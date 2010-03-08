
/* MODULE							HTAABrow.c
**		BROWSER SIDE ACCESS AUTHORIZATION MODULE
**
**	Containts the code for keeping track on server hostnames,
**	port numbers, scheme names, usernames, passwords
**	(and servers' public keys).
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

#include <string.h>		/* strchr() */
#include "../config.h"
#include "HTUtils.h"
#include "HTString.h"
#include "HTParse.h"		/* URL parsing function		*/
#include "HTList.h"		/* HTList object		*/
#include "HTAlert.h"		/* HTConfirm(), HTPrompt()	*/
#include "HTAAUtil.h"		/* AA common to both sides	*/
#include "HTAssoc.h"		/* Assoc list			*/
#include "HTAABrow.h"		/* Implemented here		*/
#include "HTUU.h"		/* Uuencoding and uudecoding	*/
#include "../libnut/str-tools.h"
#include "../src/md5.h"         /* MD5 code  -- DXP */

/* defined in HTTP.c -- DXP */
extern int do_post;

#ifndef DISABLE_TRACE
extern int httpTrace;
extern int www2Trace;
#endif

int securityType=HTAA_NONE;
int securityDone=0;

/*
** Local datatype definitions
**
** HTAAServer contains all the information about one server.
*/
typedef struct {

    char *	hostname;	/* Host's name			*/
    int		portnumber;	/* Port number			*/
    HTList *	setups;		/* List of protection setups	*/
                                /* on this server; i.e. valid	*/
                                /* authentication schemes and	*/
                                /* templates when to use them.	*/
                                /* This is actually a list of	*/
                                /* HTAASetup objects.		*/
    HTList *	realms;		/* Information about passwords	*/
} HTAAServer;


/*
** HTAASetup contains information about one server's one
** protected tree of documents.
*/
typedef struct {
    HTAAServer *server;		/* Which server serves this tree	     */
    char *	template;	/* Template for this tree		     */
    HTList *	valid_schemes;	/* Valid authentic.schemes   		     */
    HTAssocList**scheme_specifics;/* Scheme specific params		     */
    BOOL	retry;		/* Failed last time -- reprompt (or whatever)*/
} HTAASetup;


/*
** Information about usernames and passwords in
** Basic and Pubkey authentication schemes;
*/
typedef struct {
    char *	realmname;	/* Password domain name		*/
    char *	username;	/* Username in that domain	*/
    char *	password;	/* Corresponding password	*/
} HTAARealm;



/*
** Module-wide global variables
*/

PRIVATE HTList *server_table	= NULL;	/* Browser's info about servers	     */
PRIVATE char *secret_key	= NULL;	/* Browser's latest secret key       */
PRIVATE HTAASetup *current_setup= NULL;	/* The server setup we are currently */
                                        /* talking to			     */
PRIVATE char *current_hostname	= NULL;	/* The server's name and portnumber  */
PRIVATE int current_portnumber	= 80;	/* where we are currently trying to  */
                                        /* connect.			     */
PRIVATE char *current_docname	= NULL;	/* The document's name we are        */
                                        /* trying to access.		     */


PUBLIC void HTAARealm_clearall (HTList *realm_table);
PUBLIC void HTAASetup_clearall (HTList *s);

PUBLIC void HTAASetup_clearall (HTList *s)
{
  HTList *n, *nn, *sn, *snn;
  /*  HTAssocList **ss; */
  HTAASetup *o;

  /* Walk the list of setups */
  n = s;
  while (n) {
    nn = n->next;
    /* Free all of the object data */
    o = (HTAASetup *)n->object;
    if (o) {
      if (o->template)
	free (o->template);
      sn = o->valid_schemes;
      while (sn) {
	snn = sn->next;
	free (sn);
	sn = snn;
      }
    }
    /* Should free the scheme_specifics stuff too */
    /* Free the list structure and move on */
    free (n);
    n = nn;
  }
}

/**************************** HTAAServer ***********************************/


/* PUBLIC                                               HTAAServer_clearall()
**              Clears password information for all servers
** On Entry: nothing
** On Exit:  password information is gone.
**
** May 1996  PLB    Created.
** 
*/
PUBLIC void HTAAServer_clear ()
{
  HTList *n, *nn;
  HTAAServer *s;
  
  n = server_table;
  while (n) {
    nn = n->next;
    s = (HTAAServer *)n->object;
    if (s) {
#ifndef DISABLE_TRACE
      if (www2Trace) 
	fprintf(stderr, "Clearing passwd info for %s\n", s->hostname?s->hostname:"NULL");
#endif
      HTAARealm_clearall (s->realms);
      HTAASetup_clearall (s->setups);
      if (s->hostname)
	free (s->hostname);
    }
    free (n);
    n = nn;
  }
  server_table = NULL;
  secret_key = NULL;	
  current_setup = NULL;	
  current_hostname = NULL;	
  current_docname = NULL;
}



/* PRIVATE						HTAAServer_new()
**		ALLOCATE A NEW NODE TO HOLD SERVER INFO
**		AND ADD IT TO THE LIST OF SERVERS
** ON ENTRY:
**	hostname	is the name of the host that the server
**			is running in.
**	portnumber	is the portnumber which the server listens.
**
** ON EXIT:
**	returns		the newly-allocated node with all the strings
**			duplicated.
**			Strings will be automatically freed by
**			the function HTAAServer_delete(), which also
**			frees the node itself.
*/
PRIVATE HTAAServer *HTAAServer_new ARGS2(WWW_CONST char*,	hostname,
					 int,		portnumber)
{
    HTAAServer *server;

    if (!(server = (HTAAServer *)malloc(sizeof(HTAAServer))))
	outofmem(__FILE__, "HTAAServer_new");

    server->hostname	= NULL;
    server->portnumber	= (portnumber > 0 ? portnumber : 80);
    server->setups	= HTList_new();
    server->realms	= HTList_new();

    if (hostname) StrAllocCopy(server->hostname, hostname);

    if (!server_table) server_table = HTList_new();
    
    HTList_addObject(server_table, (void*)server);

    return server;
}


/* PRIVATE						HTAAServer_delete()
**
**	DELETE THE ENTRY FOR THE SERVER FROM THE HOST TABLE,
**	AND FREE THE MEMORY USED BY IT.
**
** ON ENTRY:
**	killme		points to the HTAAServer to be freed.
**
** ON EXIT:
**	returns		nothing.
*/

/* PRIVATE						HTAAServer_lookup()
**		LOOK UP SERVER BY HOSTNAME AND PORTNUMBER
** ON ENTRY:
**	hostname	obvious.
**	portnumber	if non-positive defaults to 80.
**
**	Looks up the server in the module-global server_table.
**
** ON EXIT:
**	returns		pointer to a HTAAServer structure
**			representing the looked-up server.
**			NULL, if not found.
*/
PRIVATE HTAAServer *HTAAServer_lookup ARGS2(WWW_CONST char *, hostname,
					    int,	  portnumber)
{
    if (hostname) {
	HTList *cur = server_table;
	HTAAServer *server;

	if (portnumber <= 0) portnumber = 80;

	while (NULL != (server = (HTAAServer*)HTList_nextObject(cur))) {
	    if (server->portnumber == portnumber  &&
		0==strcmp(server->hostname, hostname))
		return server;
	}
    }
    return NULL;	/* NULL parameter, or not found */
}




/*************************** HTAASetup *******************************/    


/* PRIVATE						HTAASetup_lookup()
**	FIGURE OUT WHICH AUTHENTICATION SETUP THE SERVER
**	IS USING FOR A GIVEN FILE ON A GIVEN HOST AND PORT
**
** ON ENTRY:
**	hostname	is the name of the server host machine.
**	portnumber	is the port that the server is running in.
**	docname		is the (URL-)pathname of the document we
**			are trying to access.
**
** 	This function goes through the information known about
**	all the setups of the server, and finds out if the given
**	filename resides in one of the protected directories.
**
** ON EXIT:
**	returns		NULL if no match.
**			Otherwise, a HTAASetup structure representing
**			the protected server setup on the corresponding
**			document tree.
**			
*/
PRIVATE HTAASetup *HTAASetup_lookup ARGS3(WWW_CONST char *, hostname,
					  int,		portnumber,
					  WWW_CONST char *, docname)
{
    HTAAServer *server;
    HTAASetup *setup;

    if (portnumber <= 0) portnumber = 80;

    if (hostname && docname && *hostname && *docname &&
	NULL != (server = HTAAServer_lookup(hostname, portnumber))) {

	HTList *cur = server->setups;

#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr, "%s (%s:%d:%s)\n",
			   "HTAASetup_lookup: resolving setup for",
			   hostname, portnumber, docname);
#endif

	while (NULL != (setup = (HTAASetup*)HTList_nextObject(cur))) {
	    if (HTAA_templateMatch(setup->template, docname)) {
#ifndef DISABLE_TRACE
		if (www2Trace) fprintf(stderr, "%s `%s' %s `%s'\n",
				   "HTAASetup_lookup:", docname,
				   "matched template", setup->template);
#endif
		return setup;
	    }
#ifndef DISABLE_TRACE
	    else if (www2Trace) fprintf(stderr, "%s `%s' %s `%s'\n",
				    "HTAASetup_lookup:", docname,
				    "did NOT match template", setup->template);
#endif
	} /* while setups remain */
    } /* if valid parameters and server found */

#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr, "%s `%s' %s\n",
		       "HTAASetup_lookup: No template matched",
		       (docname ? docname : "(null)"),
		       "(so probably not protected)");
#endif

    return NULL;	/* NULL in parameters, or not found */
}




/* PRIVATE						HTAASetup_new()
**			CREATE A NEW SETUP NODE
** ON ENTRY:
**	server		is a pointer to a HTAAServer structure
**			to which this setup belongs.
**	template	documents matching this template
**			are protected according to this setup.
**	valid_schemes	a list containing all valid authentication
**			schemes for this setup.
**			If NULL, all schemes are disallowed.
**	scheme_specifics is an array of assoc lists, which
**			contain scheme specific parameters given
**			by server in Authenticate: fields.
**			If NULL, all scheme specifics are
**			set to NULL.
** ON EXIT:
**	returns		a new HTAASetup node, and also adds it as
**			part of the HTAAServer given as parameter.
*/
PRIVATE HTAASetup *HTAASetup_new ARGS4(HTAAServer *,	server,
				       char *,		template,
				       HTList *,	valid_schemes,
				       HTAssocList **,	scheme_specifics)
{
    HTAASetup *setup;

    if (!server || !template || !*template) return NULL;

    if (!(setup = (HTAASetup*)malloc(sizeof(HTAASetup))))
	outofmem(__FILE__, "HTAASetup_new");

    setup->retry = NO;
    setup->server = server;
    setup->template = NULL;
    if (template) StrAllocCopy(setup->template, template);
    setup->valid_schemes = valid_schemes;
    setup->scheme_specifics = scheme_specifics;

    HTList_addObject(server->setups, (void*)setup);

    return setup;
}



/* PRIVATE						HTAASetup_delete()
**			FREE A HTAASetup STRUCTURE
** ON ENTRY:
**	killme		is a pointer to the structure to free().
**
** ON EXIT:
**	returns		nothing.
*/


/* PRIVATE					HTAASetup_updateSpecifics()
*		COPY SCHEME SPECIFIC PARAMETERS
**		TO HTAASetup STRUCTURE
** ON ENTRY:
**	setup		destination setup structure.
**	specifics	string array containing scheme
**			specific parameters for each scheme.
**			If NULL, all the scheme specific
**			parameters are set to NULL.
**
** ON EXIT:
**	returns		nothing.
*/
PRIVATE void HTAASetup_updateSpecifics ARGS2(HTAASetup *,	setup,
					     HTAssocList **,	specifics)
{
    int scheme;

    if (setup) {
	if (setup->scheme_specifics) {
	    for (scheme=0; scheme < HTAA_MAX_SCHEMES; scheme++) {
		if (setup->scheme_specifics[scheme])
		    HTAssocList_delete(setup->scheme_specifics[scheme]);
	    }
	    free(setup->scheme_specifics);
	}
	setup->scheme_specifics = specifics;
    }
}




/*************************** HTAARealm **********************************/

/* PUBLIC                                               HTAARealm_clearall()
**              Clears all realm information.
** On Entry:
**      realm_table     a list of realm objects
**
** On Exit: 
**      returns: Nothing. realm_table is no longer valid.
*/
PUBLIC void HTAARealm_clearall ARGS1(HTList *, realm_table)
{
  HTList *n, *nn;
  HTAARealm *r;

  n = realm_table;
  while (n) {
    nn = n->next;
    r = (HTAARealm *)n->object;
    if (r) {

#ifdef PAUL_IS_A_GIMP
      if (www2Trace) 
	fprintf(stderr, "Clearing %s %s:%s\n",
		r->realmname?r->realmname:"NULL", 
		r->realmname?r->username:"NULL", 
		r->realmname?r->password:"NULL");

#endif
      if (r->realmname)
	free (r->realmname);
      if (r->username)
	free (r->username);
      if (r->password)
	free (r->password);
    }
    free (n->object);
    free (n);
    n = nn;
  }
}


/* PRIVATE 						HTAARealm_lookup()
**		LOOKUP HTAARealm STRUCTURE BY REALM NAME
** ON ENTRY:
**	realm_table	a list of realm objects.
**	realmname	is the name of realm to look for.
**
** ON EXIT:
**	returns		the realm.  NULL, if not found.
*/
PRIVATE HTAARealm *HTAARealm_lookup ARGS2(HTList *,	realm_table,
					  WWW_CONST char *, realmname)
{
    if (realm_table && realmname) {
	HTList *cur = realm_table;
	HTAARealm *realm;
	
	while (NULL != (realm = (HTAARealm*)HTList_nextObject(cur))) {
	    if (0==strcmp(realm->realmname, realmname))
		return realm;
	}
    }
    return NULL;	/* No table, NULL param, or not found */
}



/* PRIVATE						HTAARealm_new()
**		CREATE A NODE CONTAINING USERNAME AND
**		PASSWORD USED FOR THE GIVEN REALM.
**		IF REALM ALREADY EXISTS, CHANGE
**		USERNAME/PASSWORD.
** ON ENTRY:
**	realm_table	a list of realms to where to add
**			the new one, too.
**	realmname	is the name of the password domain.
**	username	and
**	password	are what you can expect them to be.
**
** ON EXIT:
**	returns		the created realm.
*/
PRIVATE HTAARealm *HTAARealm_new ARGS4(HTList *,	realm_table,
				       WWW_CONST char *,	realmname,
				       WWW_CONST char *,	username,
				       WWW_CONST char *,	password)
{
    HTAARealm *realm;

    realm = HTAARealm_lookup(realm_table, realmname);

    if (!realm) {
	if (!(realm = (HTAARealm*)malloc(sizeof(HTAARealm))))
	    outofmem(__FILE__, "HTAARealm_new");
	realm->realmname = NULL;
	realm->username = NULL;
	realm->password = NULL;
	StrAllocCopy(realm->realmname, realmname);
	if (realm_table) HTList_addObject(realm_table, (void*)realm);
    }
    if (username) StrAllocCopy(realm->username, username);
    if (password) StrAllocCopy(realm->password, password);

    return realm;
}


/* BROWSER PRIVATE					HTAA_selectScheme()
**		SELECT THE AUTHENTICATION SCHEME TO USE
** ON ENTRY:
**	setup	is the server setup structure which can
**		be used to make the decision about the
**		used scheme.
**
**	When new authentication methods are added to library
**	this function makes the decision about which one to
**	use at a given time.  This can be done by inspecting
**	environment variables etc.
**
**	Currently only searches for the first valid scheme,
**	and if nothing found suggests Basic scheme;
**
** ON EXIT:
**	returns	the authentication scheme to use.
*/
PRIVATE HTAAScheme HTAA_selectScheme ARGS1(HTAASetup *, setup)
{
    HTAAScheme scheme;

    if (setup && setup->valid_schemes) {
	for (scheme=HTAA_BASIC; scheme < HTAA_MAX_SCHEMES; scheme++)
	    if (-1 < HTList_indexOf(setup->valid_schemes, (void*)scheme))
		return scheme;
    }
    return HTAA_BASIC;
}


/***************** Basic and Pubkey Authentication ************************/

/* PRIVATE						compose_auth_string()
**
**		COMPOSE Basic OR Pubkey AUTHENTICATION STRING;
**		PROMPTS FOR USERNAME AND PASSWORD IF NEEDED
**
** ON ENTRY:
**	scheme		is either HTAA_BASIC or HTAA_PUBKEY.
**	realmname	is the password domain name.
**
** ON EXIT:
**	returns		a newly composed authorization string,
**			(with, of course, a newly generated secret
**			key and fresh timestamp, if Pubkey-scheme
**			is being used).
** NOTE:
**	Like throughout the entire AA package, no string or structure
**	returned by AA package needs to (or should) be freed.
**
*/
PRIVATE char *compose_auth_string ARGS2(HTAAScheme,	scheme,
					HTAASetup *,	setup)
{
    static char *result = NULL;	/* Uuencoded presentation, the result */
    char *cleartext = NULL;	/* Cleartext presentation */
    char *ciphertext = NULL;	/* Encrypted presentation */
    int len;
    char *username;
    char *password;
    char *realmname;
    HTAARealm *realm;
    char *inet_addr = "0.0.0.0";	/* Change... @@@@ */
    char *timestamp = "42";		/* ... these @@@@ */

   /* for MD5 -- DXP */
    char *      nonce;          /* Server specified integer value */
    char *      opaque;         /* more random MD5 junk... */
    BOOL        stale;          /* flag indicating the previous request 
                                            from the client was rejected because 
                                            the nonce value was stale */
 

    FREE(result);	/* From previous call */

    if ((scheme != HTAA_BASIC && scheme != HTAA_PUBKEY && scheme != HTAA_MD5) 
	|| !setup ||
	!setup->scheme_specifics || !setup->scheme_specifics[scheme] ||
	!setup->server  ||  !setup->server->realms)
	return "";

    realmname = HTAssocList_lookup(setup->scheme_specifics[scheme], "realm");
    if (!realmname) return "";

    realm = HTAARealm_lookup(setup->server->realms, realmname);
    if (!realm || setup->retry) {
	char msg[2048];

	if (!realm) {
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr, "%s `%s' %s\n",
			       "compose_auth_string: realm:", realmname,
			       "not found -- creating");
#endif
	    realm = HTAARealm_new(setup->server->realms, realmname, NULL,NULL);
	    sprintf(msg,
		    "Document is protected.\nEnter username for %s at %s: ",
		    realm->realmname,
		    setup->server->hostname ? setup->server->hostname : "??");
	    realm->username =
		HTPrompt(msg, realm->username);
            /* Added by marca. */
            if (!realm->username)
              return "";
	}
	else {
	    sprintf(msg,"Enter username for %s at %s: ", realm->realmname,
		    setup->server->hostname ? setup->server->hostname : "??");
	    username = HTPrompt(msg, realm->username);
	    FREE(realm->username);
	    realm->username = username;
            /* Added by marca. */
            if (!realm->username)
              return "";
	}
	password = HTPromptPassword("Enter password to authenticate yourself: ");
	FREE(realm->password);
	realm->password = password;
        /* Added by marca. */
        if (!realm->password)
          return "";
    }
    
    len = strlen(realm->username ? realm->username : "") +
	  strlen(realm->password ? realm->password : "") + 3;

    if (scheme == HTAA_PUBKEY) {
#ifdef PUBKEY
	/* Generate new secret key */
	StrAllocCopy(secret_key, HTAA_generateRandomKey());
#endif
	/* Room for secret key, timestamp and inet address */
	len += strlen(secret_key ? secret_key : "") + 30;
    }
    else
	FREE(secret_key);

    if (!(cleartext  = (char*)calloc(len, 1)))
	outofmem(__FILE__, "compose_auth_string");

    if (realm->username) strcpy(cleartext, realm->username);
    else *cleartext = (char)0;

    strcat(cleartext, ":");

    if (realm->password) strcat(cleartext, realm->password);

    if (scheme == HTAA_PUBKEY) {
	strcat(cleartext, ":");
	strcat(cleartext, inet_addr);
	strcat(cleartext, ":");
	strcat(cleartext, timestamp);
	strcat(cleartext, ":");
	if (secret_key) strcat(cleartext, secret_key);

	if (!((ciphertext = (char*)malloc(2*len)) &&
	      (result     = (char*)malloc(3*len))))
	    outofmem(__FILE__, "compose_auth_string");
#ifdef PUBKEY
	HTPK_encrypt(cleartext, ciphertext, server->public_key);
        /* marca added unsigned char * cast for HP. */
	HTUU_encode((unsigned char *)ciphertext, strlen(ciphertext), result);
#endif
	free(cleartext);
	free(ciphertext);
    }

        /* the following added by DXP */
    else if(scheme == HTAA_MD5) {

	unsigned char *md5_cleartext = NULL;	/* Cleartext presentation */
	unsigned char *md5_ciphertext = NULL;	/* Encrypted presentation */
	unsigned char *A1, *A2;
	unsigned char *digest1, *digest2;
	unsigned char *hex1, *hex2;

	if (!(result = (char*)malloc(300)))
	    outofmem(__FILE__, "compose_auth_string");
	
	nonce = HTAssocList_lookup(setup->scheme_specifics[scheme], "nonce");
	if (!nonce) 
	    return "";
	
	opaque = HTAssocList_lookup(setup->scheme_specifics[scheme], "opaque");
	if (!opaque) 
	    return "";

	if (!(A1 = (unsigned char*)malloc(strlen(realm->username) + 
				 strlen(realm->realmname) + 
				 strlen(realm->password) + 3 + 1)))
	    outofmem(__FILE__, "compose_auth_string");

	if (!(A2 = (unsigned char*)malloc(4 + strlen(current_docname) + 1 + 1 )))
	    outofmem(__FILE__, "compose_auth_string");	

	    /* make A1 */
	*A1 = (unsigned char)0;
	strcat(A1, realm->username);
	strcat(A1, ":");
	strcat(A1, realm->realmname);
	strcat(A1, ":");
	strcat(A1, realm->password);
	strcat(A1, "\0");

	    /* make A2 */
	*A2 = (unsigned char)0;
	if (do_post)
	    strcat(A2, "POST");
	else
	    strcat(A2, "GET");
	strcat(A2, ":");
	strcat(A2, current_docname);
	strcat(A2, "\0");

	if (!(md5_cleartext = (unsigned char*)malloc(100 + 1)))
	    outofmem(__FILE__, "compose_auth_string");	
	if (!(md5_ciphertext = (unsigned char*)malloc(100 + 1)))
	    outofmem(__FILE__, "compose_auth_string");	
	if (!(hex1 = (unsigned char*)malloc(32 + 1)))
	    outofmem(__FILE__, "compose_auth_string");	
	if (!(hex2 = (unsigned char*)malloc(32 + 1)))
	    outofmem(__FILE__, "compose_auth_string");	
	if (!(digest1 = (unsigned char*)malloc(16)))
	    outofmem(__FILE__, "compose_auth_string");	
	if (!(digest2 = (unsigned char*)malloc(16)))
	    outofmem(__FILE__, "compose_auth_string");	
	
	MD5Mem(A1, strlen(A1), digest1);
	MD5Mem(A2, strlen(A2), digest2);

	MD5Convert_to_Hex(digest1, hex1);
	MD5Convert_to_Hex(digest2, hex2);

	    /* make md5_cleartext */

	*md5_cleartext = (unsigned char)0;
	strcat(md5_cleartext, hex1);
	strcat(md5_cleartext, ":");
	strcat(md5_cleartext, nonce);
	strcat(md5_cleartext, ":");
	strcat(md5_cleartext, hex2);

	MD5Mem(md5_cleartext, strlen(md5_cleartext), digest1);
	MD5Convert_to_Hex(digest1, md5_ciphertext);

	*result = (char)0;
	strcat(result, "username=\"");
	strcat(result, realm->username);
	strcat(result, "\", realm=\"");
	strcat(result, realm->realmname);
	strcat(result, "\", nonce=\"");
	strcat(result, nonce);
	strcat(result, "\", uri=\"");
	strcat(result, current_docname);
	strcat(result, "\", response=\"");
	strcat(result, md5_ciphertext);
	strcat(result, "\", opaque=\"");
	strcat(result, opaque);
	strcat(result, "\"");

	/* since all we need from here on out is the result, 
	     get rid of all the rest */
	free(A1);
	free(A2);
	free(digest1);
	free(digest2);
	free(hex1);
	free(hex2);
	free(md5_cleartext);
	free(md5_ciphertext);

    }


    else { /* scheme == HTAA_BASIC */
      /* Added "+ 1" marca. */
#ifdef OLD
	if (!(result = (char*)malloc(len + len/2 + 1)))
#else
        /* Ari fix. */
	if (!(result = (char*)malloc(4 * ((len+2)/3) + 1)))
#endif
	    outofmem(__FILE__, "compose_auth_string");
        /* Added cast to unsigned char * on advice of
           erik@sockdev.uni-c.dk (Erik Bertelsen). */
	HTUU_encode((unsigned char *)cleartext, strlen(cleartext), result);
	free(cleartext);
    }
#ifndef DISABLE_TRACE
    if(www2Trace) fprintf(stderr,"sending auth line: %s\n",result);
#endif

    return result;
}




/* BROWSER PUBLIC					HTAA_composeAuth()
**
**	SELECT THE AUTHENTICATION SCHEME AND
**	COMPOSE THE ENTIRE AUTHORIZATION HEADER LINE
**	IF WE ALREADY KNOW THAT THE HOST REQUIRES AUTHENTICATION
**
** ON ENTRY:
**	hostname	is the hostname of the server.
**	portnumber	is the portnumber in which the server runs.
**	docname		is the pathname of the document (as in URL)
**
** ON EXIT:
**	returns	NULL, if no authorization seems to be needed, or
**		if it is the entire Authorization: line, e.g.
**
**		   "Authorization: Basic username:password"
**
**		As usual, this string is automatically freed.
*/
PUBLIC char *HTAA_composeAuth ARGS3(WWW_CONST char *,	hostname,
				    WWW_CONST int,		portnumber,
				    WWW_CONST char *,	docname)
{
    static char *result = NULL;
    char *auth_string;
#if defined(KRB4) || defined(KRB5)
    char *compose_kerberos_auth_string(HTAAScheme, char *);
#endif
    BOOL retry;
    HTAAScheme scheme;

    FREE(result);			/* From previous call */

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, 
		"Composing Authorization for %s:%d/%s\n",
		hostname, portnumber, docname);
#endif

    if (current_portnumber != portnumber ||
	!current_hostname || !current_docname ||
	!hostname         || !docname         ||
	0 != strcmp(current_hostname, hostname) ||
	0 != strcmp(current_docname, docname)) {

	retry = NO;

	current_portnumber = portnumber;
	
	if (hostname) StrAllocCopy(current_hostname, hostname);
	else FREE(current_hostname);

	if (docname) StrAllocCopy(current_docname, docname);
	else FREE(current_docname);
    }
    else retry = YES;
    
    if (!current_setup || !retry)
	current_setup = HTAASetup_lookup(hostname, portnumber, docname);

    if (!current_setup)
	return NULL;


    switch (scheme = HTAA_selectScheme(current_setup)) {
      case HTAA_BASIC:
      case HTAA_PUBKEY:
      case HTAA_MD5: /* DXP */
	auth_string = compose_auth_string(scheme, current_setup);
	break;
#ifdef KRB4
      case HTAA_KERBEROS_V4:
        auth_string = compose_kerberos_auth_string(scheme, hostname);
        break;
#endif
#ifdef KRB5
      case HTAA_KERBEROS_V5:
        auth_string = compose_kerberos_auth_string(scheme, hostname);
        break;
#endif
      /* OTHER AUTHENTICATION ROUTINES ARE CALLED HERE */
      default:
	{
	    char msg[2048];

	    sprintf(msg, "%s %s `%s'",
		    "This client doesn't know how to compose authentication",
		    "information for scheme", HTAAScheme_name(scheme));
	    HTAlert(msg);
	    auth_string = NULL;
	}
    } /* switch scheme */

    if (!securityDone) {
	if (!auth_string || !*auth_string) {
		securityType=HTAA_NONE;
	}
	else {
		securityType=scheme;
	}
    }
    else {
	securityDone=0;
    }

    current_setup->retry = NO;

    /* Added by marca. */
    if (!auth_string)
      return NULL;

    if (!(result = (char*)malloc(sizeof(char) * (strlen(auth_string)+40))))
	outofmem(__FILE__, "HTAA_composeAuth");
    strcpy(result, "Authorization: ");
    strcat(result, HTAAScheme_name(scheme));
    strcat(result, " ");
    strcat(result, auth_string);
    return result;
}



	    
/* BROWSER PUBLIC				HTAA_shouldRetryWithAuth()
**
**		DETERMINES IF WE SHOULD RETRY THE SERVER
**		WITH AUTHORIZATION
**		(OR IF ALREADY RETRIED, WITH A DIFFERENT
**		USERNAME AND/OR PASSWORD (IF MISSPELLED))
** ON ENTRY:
**	start_of_headers is the first block already read from socket,
**			but status line skipped; i.e. points to the
**			start of the header section.
**	length		is the remaining length of the first block.
**	soc		is the socket to read the rest of server reply.
**
**			This function should only be called when
**			server has replied with a 401 (Unauthorized)
**			status code.
** ON EXIT:
**	returns		YES, if connection should be retried.
**			     The node containing all the necessary
**			     information is
**				* either constructed if it does not exist
**				* or password is reset to NULL to indicate
**				  that username and password should be
**				  reprompted when composing Authorization:
**				  field (in function HTAA_composeAuth()).
**			NO, otherwise.
*/
PUBLIC BOOL HTAA_shouldRetryWithAuth ARGS3(char *, start_of_headers,
					   int,	   length,
					   int,	   soc)
{
    HTAAScheme scheme;
    char *line;
    int num_schemes = 0;
    HTList *valid_schemes = HTList_new();
    HTAssocList **scheme_specifics = NULL;
    char *template = NULL;


    /* Read server reply header lines */

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "Server reply header lines:\n");
#endif

    HTAA_setupReader(start_of_headers, length, soc);
    while (NULL != (line = HTAA_getUnfoldedLine())  &&  *line != (char)0) {

#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr, "%s\n", line);
#endif

	if (strchr(line, ':')) {	/* Valid header line */

	    char *p = line;
	    char *fieldname = HTNextField(&p);
	    char *arg1 = HTNextField(&p);
	    char *args = p;
	    
	    if (0==my_strcasecmp(fieldname, "WWW-Authenticate:")) {
		if (HTAA_UNKNOWN != (scheme = HTAAScheme_enum(arg1))) {
		    HTList_addObject(valid_schemes, (void*)scheme);
		    if (!scheme_specifics) {
			int i;
			scheme_specifics = (HTAssocList**)
			    malloc(HTAA_MAX_SCHEMES * sizeof(HTAssocList*));
			if (!scheme_specifics)
			    outofmem(__FILE__, "HTAA_shouldRetryWithAuth");
			for (i=0; i < HTAA_MAX_SCHEMES; i++)
			    scheme_specifics[i] = NULL;
		    }
		    scheme_specifics[scheme] = HTAA_parseArgList(args);
		    num_schemes++;
		}
#ifndef DISABLE_TRACE
		else if (www2Trace) {
		    fprintf(stderr, "Unknown scheme `%s' %s\n",
			    (arg1 ? arg1 : "(null)"),
			    "in WWW-Authenticate: field");
		}
#endif
	    }

	    else if (0==my_strcasecmp(fieldname, "WWW-Protection-Template:")) {
#ifndef DISABLE_TRACE
		if (www2Trace)
		    fprintf(stderr, "Protection template set to `%s'\n", arg1);
#endif
		StrAllocCopy(template, arg1);
	    }

	} /* if a valid header line */
#ifndef DISABLE_TRACE
	else if (www2Trace) {
	    fprintf(stderr, "Invalid header line `%s' ignored\n", line);
	} /* else invalid header line */
#endif
    } /* while header lines remain */


    /* So should we retry with authorization */

    if (num_schemes == 0) {		/* No authentication valid */
	current_setup = NULL;
	return NO;
    }

    if (current_setup && current_setup->server) {
	/* So we have already tried with authorization.	*/
	/* Either we don't have access or username or	*/
	/* password was misspelled.			*/
	    
	/* Update scheme-specific parameters	*/
	/* (in case they have expired by chance).	*/
	HTAASetup_updateSpecifics(current_setup, scheme_specifics);

	if (NO == HTConfirm("Authorization failed.  Retry?")) {
	    current_setup = NULL;
	    return NO;
	} /* HTConfirm(...) == NO */
	else { /* re-ask username+password (if misspelled) */
	    current_setup->retry = YES;
	    return YES;
	} /* HTConfirm(...) == YES */
    } /* if current_setup != NULL */

    else { /* current_setup == NULL, i.e. we have a	 */
	   /* first connection to a protected server or  */
	   /* the server serves a wider set of documents */
	   /* than we expected so far.                   */

	HTAAServer *server = HTAAServer_lookup(current_hostname,
					       current_portnumber);
	if (!server) {
	    server = HTAAServer_new(current_hostname,
				    current_portnumber);
	}
	if (!template)
	    template = HTAA_makeProtectionTemplate(current_docname);
	current_setup = HTAASetup_new(server, 
				      template,
				      valid_schemes,
				      scheme_specifics);

        HTAlert("Access without authorization denied -- retrying");
	return YES;
    } /* else current_setup == NULL */

    /* Never reached */
}

