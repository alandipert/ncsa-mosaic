
/* MODULE							HTAAServ.c
**		SERVER SIDE ACCESS AUTHORIZATION MODULE
**
**	Contains the means for checking the user access
**	authorization for a file.
**
** IMPORTANT:
**	Routines in this module use dynamic allocation, but free
**	automatically all the memory reserved by them.
**
**	Therefore the caller never has to (and never should)
**	free() any object returned by these functions.
**
**	Therefore also all the strings returned by this package
**	are only valid until the next call to the same function
**	is made. This approach is selected, because of the nature
**	of access authorization: no string returned by the package
**	needs to be valid longer than until the next call.
**
**	This also makes it easy to plug the AA package in:
**	you don't have to ponder whether to free() something
**	here or is it done somewhere else (because it is always
**	done somewhere else).
**
**	The strings that the package needs to store are copied
**	so the original strings given as parameters to AA
**	functions may be freed or modified with no side effects.
**
**	The AA package does not free() anything else than what
**	it has itself allocated.
**
** AUTHORS:
**	AL	Ari Luotonen	luotonen@dxcern.cern.ch
**
** HISTORY:
**
**
** BUGS:
**
**
*/
#include "../config.h"
#include <stdio.h>		/* FILE */
#include <string.h>		/* strchr() */

#include "HTUtils.h"
#include "HTString.h"
#include "HTAccess.h"		/* HTSecure			*/
#include "HTFile.h"		/* HTLocalName			*/
#include "HTRules.h"		/* 				*/
#include "HTParse.h"		/* URL parsing function		*/
#include "HTList.h"		/* HTList object		*/

#include "HTAAUtil.h"		/* AA common parts		*/
#include "HTAuth.h"		/* Authentication		*/
#include "HTACL.h"		/* Access Control List		*/
#include "HTGroup.h"		/* Group handling		*/
#include "HTAAProt.h"		/* Protection file parsing	*/
#include "HTAAServ.h"		/* Implemented here		*/



/*
** Global variables
*/
PUBLIC time_t theTime;


/*
** Module-wide global variables
*/
PRIVATE FILE *  htaa_logfile	    = NULL; 		/* Log file	      */
PRIVATE HTAAUser *htaa_user = NULL;			/* Authenticated user */
PRIVATE HTAAFailReasonType HTAAFailReason = HTAA_OK;	/* AA fail reason     */



/* SERVER PUBLIC					HTAA_statusMessage()
**		RETURN A STRING EXPLAINING ACCESS
**		AUTHORIZATION FAILURE
**		(Can be used in server reply status line
**		 with 401/403 replies.)
** ON EXIT:
**	returns	a string containing the error message
**		corresponding to internal HTAAFailReason.
*/
PUBLIC char *HTAA_statusMessage NOARGS
{
    switch (HTAAFailReason) {

    /* 401 cases */
      case HTAA_NO_AUTH:
	return "Unauthorized -- authentication failed";
	break;
      case HTAA_NOT_MEMBER:
	return "Unauthorized to access the document";
	break;

    /* 403 cases */
      case HTAA_BY_RULE:
	return "Forbidden -- by rule";
	break;
      case HTAA_IP_MASK:
	return "Forbidden -- server refuses to serve to your IP address";
	break;
      case HTAA_NO_ACL:
      case HTAA_NO_ENTRY:
	return "Forbidden -- access to file is never allowed";
	break;
      case HTAA_SETUP_ERROR:
	return "Forbidden -- server protection setup error";
	break;

    /* 404 cases */
      case HTAA_NOT_FOUND:
	return "Not found -- file doesn't exist or is read protected";
	break;

    /* Success */
      case HTAA_OK:
	return "AA: Access should be ok but something went wrong"; 
	break;

    /* Others */
      default:
	return "Access denied -- unable to specify reason (bug)";

    } /* switch */
}



PRIVATE char *status_name ARGS1(HTAAFailReasonType, reason)
{
    switch (HTAAFailReason) {

    /* 401 cases */
      case HTAA_NO_AUTH:
	return "NO-AUTHENTICATION";
	break;
      case HTAA_NOT_MEMBER:
	return "NOT-AUTHORIZED";
	break;

    /* 403 cases */
      case HTAA_BY_RULE:
	return "FORB-RULE";
	break;
      case HTAA_IP_MASK:
	return "FORB-IP";
	break;
      case HTAA_NO_ACL:
	return "NO-ACL-FILE";
	break;
      case HTAA_NO_ENTRY:
	return "NO-ACL-ENTRY";
	break;
      case HTAA_SETUP_ERROR:
	return "SETUP-ERROR";
	break;

    /* 404 cases */
      case HTAA_NOT_FOUND:
	return "NOT-FOUND";
	break;

    /* Success */
      case HTAA_OK:
	return "OK";
	break;

    /* Others */
      default:
	return "SERVER-BUG";
    } /* switch */
}

    




/* PRIVATE						check_uthorization()
**		CHECK IF USER IS AUTHORIZED TO ACCESS A FILE
** ON ENTRY:
**	pathname	is the physical file pathname
**			to access.
**	method		method, e.g. METHOD_GET, METHOD_PUT, ...
**	scheme		authentication scheme.
**	scheme_specifics authentication string (or other
**			scheme specific parameters, like
**			Kerberos-ticket).
**
** ON EXIT:
**	returns		HTAA_OK on success.
**			Otherwise the reason for failing.
** NOTE:
**	This function does not check whether the file
**	exists or not -- so the status  404 Not found
**	must be returned from somewhere else (this is
**	to avoid unnecessary overhead of opening the
**	file twice).
*/
PRIVATE HTAAFailReasonType check_authorization ARGS4(WWW_CONST char *,  pathname,
						     HTAAMethod,    method,
						     HTAAScheme,    scheme,
						     char *, scheme_specifics)
{
    HTAAFailReasonType reason;
    GroupDef *allowed_groups;
    FILE *acl_file = NULL;
    HTAAProt *prot = NULL;	/* Protection mode */

    htaa_user = NULL;

    if (!pathname) {
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr,
			   "HTAA_checkAuthorization: Forbidden by rule\n");
#endif
	return HTAA_BY_RULE;
    }
#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr, "%s `%s' %s %s\n",
		       "HTAA_checkAuthorization: translated path:",
		       pathname, "method:", HTAAMethod_name(method));
#endif
    /*
    ** Get protection setting (set up by callbacks from rule system)
    ** NULL, if not protected by a "protect" rule.
    */
    prot = HTAA_getCurrentProtection();

    /*
    ** Check ACL existence
    */
    if (!(acl_file = HTAA_openAcl(pathname))) {
	if (prot) { /* protect rule, but no ACL */
	    if (prot->mask_group) {
		/*
		** Only mask enabled, check that
		*/
		GroupDefList *group_def_list =
		    HTAA_readGroupFile(HTAssocList_lookup(prot->values,
							  "group"));
		/*
		** Authenticate if authentication info given
		*/
		if (scheme != HTAA_UNKNOWN  &&  scheme != HTAA_NONE) {
		    htaa_user = HTAA_authenticate(scheme,
						  scheme_specifics,
						  prot);
#ifndef DISABLE_TRACE
		    if (www2Trace) fprintf(stderr, "Authentication returned: %s\n",
				       (htaa_user ? htaa_user->username
					          : "NOT-AUTHENTICATED"));
#endif
		}
		HTAA_resolveGroupReferences(prot->mask_group, group_def_list);
		reason = HTAA_userAndInetInGroup(prot->mask_group,
						 htaa_user
						  ? htaa_user->username : "",
						 HTClientHost,
						 NULL);
#ifndef DISABLE_TRACE
		if (www2Trace) {
		    if (reason != HTAA_OK)
			fprintf(stderr, "%s %s %s %s\n",
				"HTAA_checkAuthorization: access denied",
				"by mask (no ACL, only Protect rule)",
				"host", HTClientHost);
		    else fprintf(stderr, "%s %s %s %s\n",
				 "HTAA_checkAuthorization: request from",
				 HTClientHost, 
				 "accepted by only mask match (no ACL, only",
				 "Protect rule, and only mask enabled)");
		}
#endif
		return reason;
	    }
	    else {	/* 403 Forbidden */
#ifndef DISABLE_TRACE
		if (www2Trace) fprintf(stderr, "%s %s\n",
				   "HTAA_checkAuthorization: Protected, but",
				   "no mask group nor ACL -- forbidden");
#endif
		return HTAA_NO_ACL;
	    }
	}
	else { /* No protect rule and no ACL => OK 200 */
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr, "HTAA_checkAuthorization: %s\n",
			       "no protect rule nor ACL -- ok\n");
#endif
	    return HTAA_OK;
	}
    }

    /*
    ** Now we know that ACL exists
    */
    if (!prot) {		/* Not protected by "protect" rule */
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr,
			   "HTAA_checkAuthorization: default protection\n");
#endif
	prot = HTAA_getDefaultProtection();   /* Also sets current protection */

	if (!prot) {		/* @@ Default protection not set ?? */
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr, "%s %s\n",
			       "HTAA_checkAuthorization: default protection",
			       "not set (internal server error)!!");
#endif
	    return HTAA_SETUP_ERROR;
	}
    }

    /*
    ** Now we know that document is protected and ACL exists.
    ** Check against ACL entry.
    */
    {
	GroupDefList *group_def_list =
	    HTAA_readGroupFile(HTAssocList_lookup(prot->values, "group"));

	/*
	** Authenticate now that we know protection mode
	*/
	if (scheme != HTAA_UNKNOWN  &&  scheme != HTAA_NONE) {
	    htaa_user = HTAA_authenticate(scheme,
					  scheme_specifics,
					  prot);
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr, "Authentication returned: %s\n",
			       (htaa_user
				? htaa_user->username : "NOT-AUTHENTICATED"));
#endif
	}
	/* 
	** Check mask group
	*/
	if (prot->mask_group) {
	    HTAA_resolveGroupReferences(prot->mask_group, group_def_list);
	    reason=HTAA_userAndInetInGroup(prot->mask_group,
					   htaa_user ? htaa_user->username : "",
					   HTClientHost,
					   NULL);
	    if (reason != HTAA_OK) {
#ifndef DISABLE_TRACE
		if (www2Trace) fprintf(stderr, "%s %s %s\n",
				   "HTAA_checkAuthorization: access denied",
				   "by mask, host:", HTClientHost);
#endif
		return reason;
	    }
#ifndef DISABLE_TRACE
	    else if (www2Trace) fprintf(stderr, "%s %s %s %s %s\n",
				   "HTAA_checkAuthorization: request from",
				   HTClientHost, 
				   "accepted by just mask group match",
				   "(no ACL, only Protect rule, and only",
				   "mask enabled)");
#endif
	    /* And continue authorization checking */
	}
	/*
        ** Get ACL entries; get first one first, the loop others
	** Remember, allowed_groups is automatically freed by
	** HTAA_getAclEntry().
	*/
	allowed_groups = HTAA_getAclEntry(acl_file, pathname, method);
	if (!allowed_groups) {
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr, "%s `%s' %s\n",
			       "No entry for file", pathname, "in ACL");
#endif
	    HTAA_closeAcl(acl_file);
	    return HTAA_NO_ENTRY;	/* Forbidden -- no entry in the ACL */
	}
	else {
	    do {
		HTAA_resolveGroupReferences(allowed_groups, group_def_list);
		reason = HTAA_userAndInetInGroup(allowed_groups,
						 htaa_user
						 ? htaa_user->username : "",
						 HTClientHost,
						 NULL);
		if (reason == HTAA_OK) {
		    HTAA_closeAcl(acl_file);
		    return HTAA_OK;	/* OK */
		}
		allowed_groups = HTAA_getAclEntry(acl_file, pathname, method);
	    } while (allowed_groups);
	    HTAA_closeAcl(acl_file);
	    return HTAA_NOT_MEMBER;	/* Unauthorized */
	}
    }
}



/* PUBLIC					      HTAA_checkAuthorization()
**		CHECK IF USER IS AUTHORIZED TO ACCESS A FILE
** ON ENTRY:
**	url		is the document to be accessed.
**	method_name	name of the method, e.g. "GET"
**	scheme_name	authentication scheme name.
**	scheme_specifics authentication string (or other
**			scheme specific parameters, like
**			Kerberos-ticket).
**
** ON EXIT:
**	returns	status codes uniform with those of HTTP:
**	  200 OK	   if file access is ok.
**	  401 Unauthorized if user is not authorized to
**			   access the file.
**	  403 Forbidden	   if there is no entry for the
**			   requested file in the ACL.
**
** NOTE:
**	This function does not check whether the file
**	exists or not -- so the status  404 Not found
**	must be returned from somewhere else (this is
**	to avoid unnecessary overhead of opening the
**	file twice).
**
*/
PUBLIC int HTAA_checkAuthorization ARGS4(WWW_CONST char *,	url,
					 WWW_CONST char *,	method_name,
					 WWW_CONST char *,	scheme_name,
                                         char *,	scheme_specifics)
{
    static char *pathname = NULL;
    char *local_copy = NULL;
    HTAAMethod method = HTAAMethod_enum(method_name);
    HTAAScheme scheme = HTAAScheme_enum(scheme_name);
    
    /*
    ** Translate into absolute pathname, and
    ** check for "protect" and "defprot" rules.
    */
    FREE(pathname);		/* From previous call	*/
    StrAllocCopy(local_copy, url);
    {
	char *keywords = strchr(local_copy, '?');
	if (keywords) *keywords = (char)0;	/* Chop off keywords */
    }
    HTSimplify(local_copy);	/* Remove ".." etc. */
    pathname = HTTranslate(local_copy);
    if (!HTSecure) {
	char *localname = HTLocalName(pathname);
	free(pathname);
	pathname = localname;
    }	    
    FREE(local_copy);

    HTAAFailReason = check_authorization(pathname, method,
					 scheme, scheme_specifics);

    if (htaa_logfile) {
	time(&theTime);
	fprintf(htaa_logfile, "%24.24s %s %s %s %s %s\n",
		ctime(&theTime),
		HTClientHost ? HTClientHost : "local",
		method_name,
		url,
		status_name(HTAAFailReason),
		htaa_user && htaa_user->username
		? htaa_user->username : "");
	fflush(htaa_logfile);	/* Actually update it on disk */
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr, "Log: %24.24s %s %s %s %s %s\n",
			   ctime(&theTime),
			   HTClientHost ? HTClientHost : "local",
			   method_name,
			   url,
			   status_name(HTAAFailReason),
			   htaa_user && htaa_user->username
			   ? htaa_user->username : "");
#endif
    }

    switch (HTAAFailReason) {

      case HTAA_NO_AUTH:
      case HTAA_NOT_MEMBER:
	return 401;
	break;

      case HTAA_BY_RULE:
      case HTAA_IP_MASK:
      case HTAA_NO_ACL:
      case HTAA_NO_ENTRY:
      case HTAA_SETUP_ERROR:
	return 403;
	break;

      case HTAA_NOT_FOUND:
	return 404;
	break;

      case HTAA_OK:
	return 200;
	break;

      default:
	return 500;
    } /* switch */
}





/* PRIVATE					compose_scheme_specifics()
**		COMPOSE SCHEME-SPECIFIC PARAMETERS
**		TO BE SENT ALONG WITH SERVER REPLY
**		IN THE WWW-Authenticate: FIELD.
** ON ENTRY:
**	scheme		is the authentication scheme for which
**			parameters are asked for.
**	prot		protection setup structure.
**
** ON EXIT:
**	returns		scheme specific parameters in an
**			auto-freed string.
*/
PRIVATE char *compose_scheme_specifics ARGS2(HTAAScheme,	scheme,
					     HTAAProt *,	prot)
{
    static char *result = NULL;

    FREE(result);	/* From previous call */

    switch (scheme) {
      case HTAA_BASIC:
	{
	    char *realm = HTAssocList_lookup(prot->values, "server");
	    result = (char*)malloc(60);
	    sprintf(result, "realm=\"%s\"",
		    (realm ? realm : "UNKNOWN"));
	    return result;
	}
	break;

      case HTAA_PUBKEY:
	{
	    char *realm = HTAssocList_lookup(prot->values, "server");
	    result = (char*)malloc(200);
	    sprintf(result, "realm=\"%s\", key=\"%s\"",
		    (realm ? realm : "UNKNOWN"),
		    "PUBKEY-NOT-IMPLEMENTED");
	    return result;
	}
	break;
      default:
	return NULL;
    }
}


/* SERVER PUBLIC				    HTAA_composeAuthHeaders()
**		COMPOSE WWW-Authenticate: HEADER LINES
**		INDICATING VALID AUTHENTICATION SCHEMES
**		FOR THE REQUESTED DOCUMENT
** ON ENTRY:
**	No parameters, but HTAA_checkAuthorization() must
**	just before have failed because a wrong (or none)
**	authentication scheme was used.
**
** ON EXIT:
**	returns	a buffer containing all the WWW-Authenticate:
**		fields including CRLFs (this buffer is auto-freed).
**		NULL, if authentication won't help in accessing
**		the requested document.
**
*/
PUBLIC char *HTAA_composeAuthHeaders NOARGS
{
    static char *result = NULL;
    HTAAScheme scheme;
    char *scheme_name;
    char *scheme_params;
    HTAAProt *prot = HTAA_getCurrentProtection();

    if (!prot) {
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr, "%s %s\n",
			   "HTAA_composeAuthHeaders: Document not protected",
			   "-- why was this function called??");
#endif
	return NULL;
    }
#ifndef DISABLE_TRACE
    else if (www2Trace) fprintf(stderr, "HTAA_composeAuthHeaders: for file `%s'\n",
			    prot->filename);
#endif

    FREE(result);	/* From previous call */
    if (!(result = (char*)malloc(4096)))	/* @@ */
	outofmem(__FILE__, "HTAA_composeAuthHeaders");
    *result = (char)0;

    for (scheme=0; scheme < HTAA_MAX_SCHEMES; scheme++) {
	if (-1 < HTList_indexOf(prot->valid_schemes, (void*)scheme)) {
	    if ((scheme_name = HTAAScheme_name(scheme))) {
		scheme_params = compose_scheme_specifics(scheme,prot);
		strcat(result, "WWW-Authenticate: ");
		strcat(result, scheme_name);
		if (scheme_params) {
		    strcat(result, " ");
		    strcat(result, scheme_params);
		}
		strcat(result, "\r\n");
	    } /* scheme name found */
#ifndef DISABLE_TRACE
	    else if (www2Trace) fprintf(stderr, "HTAA_composeAuthHeaders: %s %d\n",
				    "No name found for scheme number", scheme);
#endif
	} /* scheme valid for requested document */
    } /* for every scheme */
    
    return result;
}



/* PUBLIC						HTAA_startLogging()
**		START UP ACCESS AUTHORIZATION LOGGING
** ON ENTRY:
**	fp	is the open log file.
**
*/
PUBLIC void HTAA_startLogging ARGS1(FILE *, fp)
{
    htaa_logfile = fp;
}

