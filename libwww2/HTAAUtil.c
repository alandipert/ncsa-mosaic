
/* MODULE							HTAAUtil.c
**		COMMON PARTS OF ACCESS AUTHORIZATION MODULE
**			FOR BOTH SERVER AND BROWSER
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
**	AA (Access Authorization) package means modules which
**	names start with HTAA.
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
#include <string.h>
#include "HTUtils.h"
#include "tcp.h"	/* NETREAD() etc.	*/
#include "HTAAUtil.h"	/* Implemented here	*/
#include "HTAssoc.h"	/* Assoc list		*/
#include "HTTCP.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/* PUBLIC						HTAAScheme_enum()
**		TRANSLATE SCHEME NAME INTO
**		A SCHEME ENUMERATION
**
** ON ENTRY:
**	name		is a string representing the scheme name.
**
** ON EXIT:
**	returns		the enumerated constant for that scheme.
*/
PUBLIC HTAAScheme HTAAScheme_enum ARGS1(WWW_CONST char*, name)
{
    static char *upcased = NULL;
    char *cur;

    if (!name) return HTAA_UNKNOWN;

    StrAllocCopy(upcased, name);
    cur = upcased;
    while (*cur) {
	*cur = TOUPPER(*cur);
	cur++;
    }
    
    if (!strncmp(upcased, "NONE", 4))
	return HTAA_NONE;
    else if (!strncmp(upcased, "BASIC", 5))
	return HTAA_BASIC;
    else if (!strncmp(upcased, "PUBKEY", 6))
	return HTAA_PUBKEY;
    else if (!strncmp(upcased, "KERBEROSV4", 10))
	return HTAA_KERBEROS_V4;
    else if (!strncmp(upcased, "KERBEROSV5", 10))
	return HTAA_KERBEROS_V5;
    else if (!strncmp(upcased, "DIGEST", 6))
	return HTAA_MD5;  /* DXP */
    else
	return HTAA_UNKNOWN;
}


/* PUBLIC						HTAAScheme_name()
**			GET THE NAME OF A GIVEN SCHEME
** ON ENTRY:
**	scheme		is one of the scheme enum values:
**			HTAA_NONE, HTAA_BASIC, HTAA_PUBKEY, ...
**
** ON EXIT:
**	returns		the name of the scheme, i.e.
**			"None", "Basic", "Pubkey", ...
*/
PUBLIC char *HTAAScheme_name ARGS1(HTAAScheme, scheme)
{
    switch (scheme) {
      case HTAA_NONE:		return "None";          break;
      case HTAA_BASIC:		return "Basic";         break;
      case HTAA_PUBKEY:		return "Pubkey";        break;
      case HTAA_KERBEROS_V4:	return "KerberosV4";	break;
      case HTAA_KERBEROS_V5:	return "KerberosV5";	break;
      case HTAA_MD5:            return "Digest";        break;   
      case HTAA_UNKNOWN:	return "UNKNOWN";       break;
      default:			return "THIS-IS-A-BUG";
    }
}


/* PUBLIC						    HTAAMethod_enum()
**		TRANSLATE METHOD NAME INTO AN ENUMERATED VALUE
** ON ENTRY:
**	name		is the method name to translate.
**
** ON EXIT:
**	returns		HTAAMethod enumerated value corresponding
**			to the given name.
*/
PUBLIC HTAAMethod HTAAMethod_enum ARGS1(WWW_CONST char *, name)
{
    char tmp[MAX_METHODNAME_LEN+1];
    WWW_CONST char *src = name;
    char *dest = tmp;

    if (!name) return METHOD_UNKNOWN;

    while (*src) {
	*dest = TOUPPER(*src);
	dest++;
	src++;
    }
    *dest = 0;

    if (0==strcmp(tmp, "GET"))
	return METHOD_GET;
    else if (0==strcmp(tmp, "PUT"))
	return METHOD_PUT;
    else if (0==strcmp(tmp, "META"))
	return METHOD_META;
    else
	return METHOD_UNKNOWN;
}


/* PUBLIC						HTAAMethod_name()
**			GET THE NAME OF A GIVEN METHOD
** ON ENTRY:
**	method		is one of the method enum values:
**			METHOD_GET, METHOD_PUT, ...
**
** ON EXIT:
**	returns		the name of the scheme, i.e.
**			"GET", "PUT", ...
*/
PUBLIC char *HTAAMethod_name ARGS1(HTAAMethod, method)
{
    switch (method) {
      case METHOD_GET:		return "GET";           break;
      case METHOD_PUT:		return "PUT";           break;
      case METHOD_META:		return "META";           break;
      case METHOD_UNKNOWN:	return "UNKNOWN";       break;
      default:			return "THIS-IS-A-BUG";
    }
}


/* PUBLIC						HTAAMethod_inList()
**		IS A METHOD IN A LIST OF METHOD NAMES
** ON ENTRY:
**	method		is the method to look for.
**	list		is a list of method names.
**
** ON EXIT:
**	returns		YES, if method was found.
**			NO, if not found.
*/
PUBLIC BOOL HTAAMethod_inList ARGS2(HTAAMethod,	method,
				    HTList *,	list)
{
    HTList *cur = list;
    char *item;

    while (NULL != (item = (char*)HTList_nextObject(cur))) {
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr, " %s", item);
#endif
	if (method == HTAAMethod_enum(item))
	    return YES;
    }

    return NO;	/* Not found */
}



/* PUBLIC						HTAA_templateMatch()
**		STRING COMPARISON FUNCTION FOR FILE NAMES
**		   WITH ONE WILDCARD * IN THE TEMPLATE
** NOTE:
**	This is essentially the same code as in HTRules.c, but it
**	cannot be used because it is embedded in between other code.
**	(In fact, HTRules.c should use this routine, but then this
**	 routine would have to be more sophisticated... why is life
**	 sometimes so hard...)
**
** ON ENTRY:
**	template	is a template string to match the file name
**			agaist, may contain a single wildcard
**			character * which matches zero or more
**			arbitrary characters.
**	filename	is the filename (or pathname) to be matched
**			agaist the template.
**
** ON EXIT:
**	returns		YES, if filename matches the template.
**			NO, otherwise.
*/
PUBLIC BOOL HTAA_templateMatch ARGS2(WWW_CONST char *, template, 
				     WWW_CONST char *, filename)
{
    WWW_CONST char *p = template;
    WWW_CONST char *q = filename;
    int m;

    for( ; *p  &&  *q  &&  *p == *q; p++, q++)	/* Find first mismatch */
	; /* do nothing else */
    
    if (!*p && !*q)	return YES;	/* Equally long equal strings */
    else if ('*' == *p) {		/* Wildcard */
	p++;				/* Skip wildcard character */
	m = strlen(q) - strlen(p);	/* Amount to match to wildcard */
	if (m < 0) return NO;		/* No match, filename too short */
	else {			/* Skip the matched characters and compare */
	    if (strcmp(p, q+m))	return NO;	/* Tail mismatch */
	    else                return YES;	/* Tail match */
	}
    }	/* if wildcard */
    else		return NO;	/* Length or character mismatch */
}


/* PUBLIC					HTAA_makeProtectionTemplate()
**		CREATE A PROTECTION TEMPLATE FOR THE FILES
**		IN THE SAME DIRECTORY AS THE GIVEN FILE
**		(Used by server if there is no fancier way for
**		it to tell the client, and by browser if server
**		didn't send WWW-ProtectionTemplate: field)
** ON ENTRY:
**	docname	is the document pathname (from URL).
**
** ON EXIT:
**	returns	a template matching docname, and other files
**		files in that directory.
**
**		E.g.  /foo/bar/x.html  =>  /foo/bar/ *
**						    ^
**				Space only to prevent it from
**				being a comment marker here,
**				there really isn't any space.
*/
PUBLIC char *HTAA_makeProtectionTemplate ARGS1(WWW_CONST char *, docname)
{
    char *template = NULL;
    char *slash = NULL;

    if (docname) {
	StrAllocCopy(template, docname);
	slash = strrchr(template, '/');
	if (slash) slash++;
	else slash = template;
	*slash = (char)0;
	StrAllocCat(template, "*");
    }
    else StrAllocCopy(template, "*");

#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr,
		       "make_template: made template `%s' for file `%s'\n",
		       template, docname);
#endif

    return template;
}




/*
** Skip leading whitespace from *s forward
*/
#define SKIPWS(s) while (*s==' ' || *s=='\t') s++;

/*
** Kill trailing whitespace starting from *(s-1) backwords
*/
#define KILLWS(s) {char *c=s-1; while (*c==' ' || *c=='\t') *(c--)=(char)0;}


/* PUBLIC						HTAA_parseArgList()
**		PARSE AN ARGUMENT LIST GIVEN IN A HEADER FIELD
** ON ENTRY:
**	str	is a comma-separated list:
**
**			item, item, item
**		where
**			item ::= value
**			       | name=value
**			       | name="value"
**
**		Leading and trailing whitespace is ignored
**		everywhere except inside quotes, so the following
**		examples are equal:
**
**			name=value,foo=bar
**			 name="value",foo="bar"
**			  name = value ,  foo = bar
**			   name = "value" ,  foo = "bar"
**
** ON EXIT:
**	returns	a list of name-value pairs (actually HTAssocList*).
**		For items with no name, just value, the name is
**		the number of order number of that item. E.g.
**		"1" for the first, etc.
*/
PUBLIC HTAssocList *HTAA_parseArgList ARGS1(char *, str)
{
    HTAssocList *assoc_list = HTAssocList_new();
    char *cur = NULL;
    char *name = NULL;
    int index = 0;

    if (!str) return assoc_list;

    while (*str) {
	SKIPWS(str);				/* Skip leading whitespace */
	cur = str;
	index++;

	while (*cur  &&  *cur != '='  &&  *cur != ',')
	    cur++;	/* Find end of name (or lonely value without a name) */
	KILLWS(cur);	/* Kill trailing whitespace */

	if (*cur == '=') {			/* Name followed by a value */
	    *(cur++) = (char)0;			/* Terminate name */
	    StrAllocCopy(name, str);
	    SKIPWS(cur);			/* Skip WS leading the value */
	    str = cur;
	    if (*str == '"') {			/* Quoted value */
		str++;
		cur = str;
		while (*cur  &&  *cur != '"') cur++;
		if (*cur == '"')
		    *(cur++) = (char)0;	/* Terminate value */
		/* else it is lacking terminating quote */
		SKIPWS(cur);			/* Skip WS leading comma */
		if (*cur == ',') cur++;		/* Skip separating colon */
	    }
	    else {				/* Unquoted value */
		while (*cur  &&  *cur != ',') cur++;
		KILLWS(cur);			/* Kill trailing whitespace */
		if (*cur == ',')
		    *(cur++) = (char)0;
		/* else *cur already NULL */
	    }
	}
	else {	/* No name, just a value */
	    if (*cur == ',') 
		*(cur++) = (char)0;		/* Terminate value */
	    /* else last value on line (already terminated by NULL) */
	    StrAllocCopy(name, "nnn");	/* Room for item order number */
	    sprintf(name, "%d", index); /* Item order number for name */
	}
	HTAssocList_add(assoc_list, name, str);
	str = cur;
    } /* while *str */

    return assoc_list;
}



/************** HEADER LINE READER -- DOES UNFOLDING *************************/

#define BUFFER_SIZE	16384

PRIVATE char buffer[BUFFER_SIZE + 1];
PRIVATE char *start_pointer = buffer;
PRIVATE char *end_pointer = buffer;
PRIVATE int in_soc = -1;

/* PUBLIC						HTAA_setupReader()
**		SET UP HEADER LINE READER, i.e. give
**		the already-read-but-not-yet-processed
**		buffer of text to be read before more
**		is read from the socket.
** ON ENTRY:
**	start_of_headers is a pointer to a buffer containing
**			the beginning of the header lines
**			(rest will be read from a socket).
**	length		is the number of valid characters in
**			'start_of_headers' buffer.
**	soc		is the socket to use when start_of_headers
**			buffer is used up.
** ON EXIT:
**	returns		nothing.
**			Subsequent calls to HTAA_getUnfoldedLine()
**			will use this buffer first and then
**			proceed to read from socket.
*/
PUBLIC void HTAA_setupReader ARGS3(char *,	start_of_headers,
				   int,		length,
				   int,		soc)
{
    start_pointer = buffer;
    if (start_of_headers) {
	strncpy(buffer, start_of_headers, length);
	buffer[length] = (char)0;
	end_pointer = buffer + length;
    }
    else {
	*start_pointer = (char)0;
	end_pointer = start_pointer;
    }
    in_soc = soc;
}


/* PUBLIC						HTAA_getUnfoldedLine()
**		READ AN UNFOLDED HEADER LINE FROM SOCKET
** ON ENTRY:
**	HTAA_setupReader must absolutely be called before
**	this function to set up internal buffer.
**
** ON EXIT:
**	returns	a newly-allocated character string representing
**		the read line.  The line is unfolded, i.e.
**		lines that begin with whitespace are appended
**		to current line.  E.g.
**
**			Field-Name: Blaa-Blaa
**			 This-Is-A-Continuation-Line
**			 Here-Is_Another
**
**		is seen by the caller as:
**
**	Field-Name: Blaa-Blaa This-Is-A-Continuation-Line Here-Is_Another
**
*/
PUBLIC char *HTAA_getUnfoldedLine NOARGS
{
    char *line = NULL;
    char *cur;
    int count;
    BOOL peek_for_folding = NO;

    if (in_soc < 0) {
	fprintf(stderr, "%s %s\n",
		"HTAA_getUnfoldedLine: buffer not initialized",
		"with function HTAA_setupReader()");
	return NULL;
    }

    for(;;) {

	/* Reading from socket */

	if (start_pointer >= end_pointer) {/*Read the next block and continue*/
	    count = NETREAD(in_soc, buffer, BUFFER_SIZE);
	    if (count <= 0) {
		in_soc = -1;
		return line;
	    }
	    start_pointer = buffer;
	    end_pointer = buffer + count;
	    *end_pointer = (char)0;
#ifdef NOT_ASCII
	    cur = start_pointer;
	    while (cur < end_pointer) {
		*cur = TOASCII(*cur);
		cur++;
	    }
#endif /*NOT_ASCII*/
	}
	cur = start_pointer;


	/* Unfolding */
	
	if (peek_for_folding) {
	    if (*cur != ' '  &&  *cur != '\t')
		return line;	/* Ok, no continuation line */
	    else		/* So this is a continuation line, continue */
		peek_for_folding = NO;
	}


	/* Finding end-of-line */

	while (cur < end_pointer && *cur != '\n') /* Find the end-of-line */
	    cur++;				  /* (or end-of-buffer).  */

	
	/* Terminating line */

	if (cur < end_pointer) {	/* So *cur==LF, terminate line */
	    *cur = (char)0;		/* Overwrite LF */
	    if (*(cur-1) == '\r')
		*(cur-1) = (char)0;	/* Overwrite CR */
	    peek_for_folding = YES;	/* Check for a continuation line */
	}


	/* Copying the result */

	if (line)
	    StrAllocCat(line, start_pointer);	/* Append */
	else
	    StrAllocCopy(line, start_pointer);	/* A new line */

	start_pointer = cur+1;	/* Skip the read line */

    } /* forever */
}




