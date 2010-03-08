/*		Parse HyperText Document Address		HTParse.c
**		================================
*/
#include "../config.h"
#include "HTUtils.h"
#include "HTParse.h"
#include "tcp.h"

#define HEX_ESCAPE '%'

struct struct_parts {
	char * access;
	char * host;
	char * absolute;
	char * relative;
	char * anchor;
};

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/*	Strip white space off a string
**	------------------------------
**
** On exit,
**	Return value points to first non-white character, or to 0 if none.
**	All trailing white space is OVERWRITTEN with zero.
*/

#ifdef __STDC__
char * HTStrip(char * s)
#else
char * HTStrip(s)
	char *s;
#endif
{
#define SPACE(c) ((c==' ')||(c=='\t')||(c=='\n')) 
    char * p=s;
    for(p=s;*p;p++);		        /* Find end of string */
    for(p--;p>=s;p--) {
    	if(SPACE(*p)) *p=0;	/* Zap trailing blanks */
	else break;
    }
    while(SPACE(*s))s++;	/* Strip leading blanks */
    return s;
}


/*	Scan a filename for its consituents
**	-----------------------------------
**
** On entry,
**	name	points to a document name which may be incomplete.
** On exit,
**      absolute or relative may be nonzero (but not both).
**	host, anchor and access may be nonzero if they were specified.
**	Any which are nonzero point to zero terminated strings.
*/
#ifdef __STDC__
PRIVATE void scan(char * name, struct struct_parts *parts)
#else
PRIVATE void scan(name, parts)
    char * name;
    struct struct_parts *parts;
#endif
{
    char * after_access;
    char * p;
    int length;

    if (name && *name)
      length = strlen(name);
    else
      length = 0;
    
    parts->access = 0;
    parts->host = 0;
    parts->absolute = 0;
    parts->relative = 0;
    parts->anchor = 0;

    /* Argh. */
    if (!length)
      return;
    
    after_access = name;
    for(p=name; *p; p++) {
	if (*p==':') {
		*p = 0;
		parts->access = name;	/* Access name has been specified */
		after_access = p+1;
	}
	if (*p=='/') break;
	if (*p=='#') break;
    }
    
    for(p=name+length-1; p>=name; p--) {
	if (*p =='#') {
	    parts->anchor=p+1;
	    *p=0;				/* terminate the rest */
	}
    }
    p = after_access;
    if (*p=='/'){
	if (p[1]=='/') {
	    parts->host = p+2;		/* host has been specified 	*/
	    *p=0;			/* Terminate access 		*/
	    p=strchr(parts->host,'/');	/* look for end of host name if any */
	    if(p) {
	        *p=0;			/* Terminate host */
	        parts->absolute = p+1;		/* Root has been found */
	    }
	} else {
	    parts->absolute = p+1;		/* Root found but no host */
	}	    
    } else {
        parts->relative = (*after_access) ? after_access : 0;	/* zero for "" */
    }

    /* Access specified but no host: the anchor was not really one
       e.g. news:j462#36487@foo.bar -- JFG 10/7/92, from bug report */
    if (parts->access && ! parts->host && parts->anchor) {
      *(parts->anchor - 1) = '#';  /* Restore the '#' in the address */
      parts->anchor = 0;
    }

} /*scan */    


/*	Parse a Name relative to another name
**	-------------------------------------
**
**	This returns those parts of a name which are given (and requested)
**	substituting bits from the related name where necessary.
**
** On entry,
**	aName		A filename given
**      relatedName     A name relative to which aName is to be parsed
**      wanted          A mask for the bits which are wanted.
**
** On exit,
**	returns		A pointer to a malloc'd string which MUST BE FREED
*/
#ifdef __STDC__
char * HTParse(char * aName, char * relatedName, int wanted)
#else
char * HTParse(aName, relatedName, wanted)
    char * aName;
    char * relatedName;
    int wanted;
#endif

{
    char * result = 0;
    char * return_value = 0;
    int len;
    char * name = 0;
    char * rel = 0;
    char * p;
    char *access;
    struct struct_parts given, related;
    
    if (!aName)
      aName = strdup ("\0");
    if (!relatedName)
      relatedName = strdup ("\0");
    
    /* Make working copies of input strings to cut up:
    */
    len = strlen(aName)+strlen(relatedName)+10;
    result=(char *)malloc(len);		/* Lots of space: more than enough */
    
    StrAllocCopy(name, aName);
    StrAllocCopy(rel, relatedName);
    
    scan(name, &given);
    scan(rel,  &related); 
    result[0]=0;		/* Clear string  */
    access = given.access ? given.access : related.access;
    if (wanted & PARSE_ACCESS)
        if (access) {
	    strcat(result, access);
	    if(wanted & PARSE_PUNCTUATION) strcat(result, ":");
	}
	
    if (given.access && related.access)	/* If different, inherit nothing. */
        if (strcmp(given.access, related.access)!=0) {
	    related.host=0;
	    related.absolute=0;
	    related.relative=0;
	    related.anchor=0;
	}
	
    if (wanted & PARSE_HOST)
        if(given.host || related.host) {
          char * tail = result + strlen(result);   
	    if(wanted & PARSE_PUNCTUATION) strcat(result, "//");
	    strcat(result, given.host ? given.host : related.host);
#define CLEAN_URLS
#ifdef CLEAN_URLS
	    /* Ignore default port numbers, and trailing dots on FQDNs
	       which will only cause identical adreesses to look different */
          {
            char * p;
            p = strchr(tail, ':');
            if (p && access) 
              {		/* Port specified */
                if ((strcmp(access, "http") == 0 && strcmp(p, ":80") == 0) ||
                    (strcmp(access, "gopher") == 0 && 
                     (strcmp(p, ":70") == 0 ||
                      strcmp(p, ":70+") == 0)))
                  *p = (char)0;	/* It is the default: ignore it */
                else if (p && *p && p[strlen(p)-1] == '+')
                  p[strlen(p)-1] = 0;
              }
            if (!p) 
              p = tail + strlen(tail); /* After hostname */
            p--;				/* End of hostname */
            if (strlen (tail) > 3 && (*p == '.')) 
              {
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr, "[Parse] tail '%s' p '%s'\n", tail, p);
#endif
                *p = (char)0; /* chop final . */
                
                /* OK, at this point we know that *(p+1) exists,
                   else we would not be here.

                   If it's 0, then we're done.

                   If it's not 0, then we move *(p+2) to *(p+1),
                   etc.

                   Let's try to use a bcopy... */
                if (*(p+1) != '\0')
                  {
#ifndef DISABLE_TRACE
                    if (www2Trace)
                      fprintf (stderr, "[Parse] Copying '%s' to '%s', %d bytes\n", 
                               p+1, p, strlen (p+1));
#endif
/*
                    bcopy (p+1, p, strlen(p+1));
*/
                    memcpy (p, p+1, strlen(p+1));
#ifndef DISABLE_TRACE
                    if (www2Trace)
                      fprintf (stderr, "[Parse] Setting '%c' to 0...\n",
                               *(p + strlen (p+1)));
#endif
                    *(p + strlen (p+1)) = '\0';
                  }
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr, "[Parse] tail '%s' p '%s'\n", tail, p);
#endif
              }
            {
              char *tmp;
              tmp = strchr (tail, '@');
              if (!tmp)
                tmp = tail;
              for (; *tmp; tmp++)
                *tmp = TOLOWER (*tmp);
            }
          }
#endif
	}
	
    if (given.host && related.host)  /* If different hosts, inherit no path. */
        if (strcmp(given.host, related.host)!=0) {
	    related.absolute=0;
	    related.relative=0;
	    related.anchor=0;
	}
	
    if (wanted & PARSE_PATH) {
        if(given.absolute) {				/* All is given */
	    if(wanted & PARSE_PUNCTUATION) strcat(result, "/");
	    strcat(result, given.absolute);
	} else if(related.absolute) {	/* Adopt path not name */
	    strcat(result, "/");
	    strcat(result, related.absolute);
	    if (given.relative) {
		p = strchr(result, '?');	/* Search part? */
		if (!p) p=result+strlen(result)-1;
		for (; *p!='/'; p--);	/* last / */
		p[1]=0;					/* Remove filename */
		strcat(result, given.relative);		/* Add given one */
		HTSimplify (result);
	    }
	} else if(given.relative) {
	    strcat(result, given.relative);		/* what we've got */
	} else if(related.relative) {
	    strcat(result, related.relative);
	} else {  /* No inheritance */
	    strcat(result, "/");
	}
    }
		
    if (wanted & PARSE_ANCHOR)
        if(given.anchor || related.anchor) {
	    if(wanted & PARSE_PUNCTUATION) strcat(result, "#");
	    strcat(result, given.anchor ? given.anchor : related.anchor);
	}
    if (rel)
      free(rel);
    if (name)
      free(name);
    
    StrAllocCopy(return_value, result);
    free(result);
    return return_value;		/* exactly the right length */
}


/*	        Simplify a filename
//		-------------------
//
// A unix-style file is allowed to contain the seqeunce xxx/../ which may be
// replaced by "" , and the seqeunce "/./" which may be replaced by "/".
// Simplification helps us recognize duplicate filenames.
//
//	Thus, 	/etc/junk/../fred 	becomes	/etc/fred
//		/etc/junk/./fred	becomes	/etc/junk/fred
//
//      but we should NOT change
//		http://fred.xxx.edu/../..
//
//	or	../../albert.html
*/
#ifdef __STDC__
void HTSimplify(char * filename)
#else
void HTSimplify(filename)
    char * filename;
#endif

{
  char * p;
  char * q;
  if (filename[0] && filename[1])
    {
      for(p=filename+2; *p; p++) 
        {
          if (*p=='/') 
            {
              if ((p[1]=='.') && (p[2]=='.') && (p[3]=='/' || !p[3] )) 
                {
                  /* Changed clause below to (q>filename) due to attempted
                     read to q = filename-1 below. */
                  for (q = p-1; (q>filename) && (*q!='/'); q--)
                    ; /* prev slash */
                  if (q[0]=='/' && 0!=strncmp(q, "/../", 4)
                      && !(q-1>filename && q[-1]=='/')) 
                    {
                      strcpy(q, p+3);	/* Remove  /xxx/..	*/
                      if (!*filename) strcpy(filename, "/");
                      p = q-1;		/* Start again with prev slash 	*/
                    } 
                } 
              else if ((p[1]=='.') && (p[2]=='/' || !p[2])) 
                {
                  strcpy(p, p+2);			/* Remove a slash and a dot */
                }
            }
        }
    }
}
  

/*		Make Relative Name
**		------------------
**
** This function creates and returns a string which gives an expression of
** one address as related to another. Where there is no relation, an absolute
** address is retured.
**
**  On entry,
**	Both names must be absolute, fully qualified names of nodes
**	(no anchor bits)
**
**  On exit,
**	The return result points to a newly allocated name which, if
**	parsed by HTParse relative to relatedName, will yield aName.
**	The caller is responsible for freeing the resulting name later.
**
*/
#ifdef __STDC__
char * HTRelative(char * aName, char *relatedName)
#else
char * HTRelative(aName, relatedName)
   char * aName;
   char * relatedName;
#endif
{
    char * result = 0;
    WWW_CONST char *p = aName;
    WWW_CONST char *q = relatedName;
    WWW_CONST char * after_access = 0;
    WWW_CONST char * path = 0;
    WWW_CONST char * last_slash = 0;
    int slashes = 0;
    
    for(;*p; p++, q++) {	/* Find extent of match */
    	if (*p!=*q) break;
	if (*p==':') after_access = p+1;
	if (*p=='/') {
	    last_slash = p;
	    slashes++;
	    if (slashes==3) path=p;
	}
    }
    
    /* q, p point to the first non-matching character or zero */
    
    if (!after_access) {			/* Different access */
        StrAllocCopy(result, aName);
    } else if (slashes<3){			/* Different nodes */
    	StrAllocCopy(result, after_access);
    } else if (slashes==3){			/* Same node, different path */
        StrAllocCopy(result, path);
    } else {					/* Some path in common */
        int levels= 0;
        for(; *q && (*q!='#'); q++)  if (*q=='/') levels++;
	result = (char *)malloc(3*levels + strlen(last_slash) + 1);
	result[0]=0;
	for(;levels; levels--)strcat(result, "../");
	strcat(result, last_slash+1);
    }
#ifndef DISABLE_TRACE
    if (www2Trace) 
      fprintf(stderr, "HT: `%s' expressed relative to\n    `%s' is\n   `%s'.",
              aName, relatedName, result);
#endif
    return result;
}


static unsigned char isAcceptable[96] =
/*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
{    0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,	/* 2x   !"#$%&'()*+,-./	 */
     1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,	/* 3x  0123456789:;<=>?	 */
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 4x  @ABCDEFGHIJKLMNO  */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,	/* 5x  PQRSTUVWXYZ[\]^_	 */
     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 6x  `abcdefghijklmno	 */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };	/* 7x  pqrstuvwxyz{\}~	DEL */

#define HT_HEX(i) (i < 10 ? '0'+i : 'A'+ i - 10)

/* The string returned from here, if any, can be free'd by caller. */
char *HTEscape (char *part)
{
  char *q;
  char *p;		/* Pointers into keywords */
  char *escaped;

  if (!part)
    return NULL;

  escaped = (char *)malloc (strlen (part) * 3 + 1);
  
  for (q = escaped, p = part; *p != '\0'; p++)
    {
      int c = (int)((unsigned char)(*p));
      if (c >= 32 && c <= 127 && isAcceptable[c-32])
        {
          *q++ = *p;
        }
      else
        {
          *q++ = '%';
          *q++ = HT_HEX(c / 16);
          *q++ = HT_HEX(c % 16);
        }
    }
  
  *q=0;
  
  return escaped;
}




/*		Decode %xx escaped characters			HTUnEscape()
**		-----------------------------
**
**	This function takes a pointer to a string in which some
**	characters may have been encoded in %xy form, where xy is
**	the acsii hex code for character 16x+y.
**	The string is converted in place, as it will never grow.
*/

PRIVATE char from_hex ARGS1(char, c)
{
    return  c >= '0' && c <= '9' ?  c - '0' 
    	    : c >= 'A' && c <= 'F'? c - 'A' + 10
    	    : c - 'a' + 10;	/* accept small letters just in case */
}

PUBLIC char * HTUnEscape ARGS1( char *, str)
{
    char * p = str;
    char * q = str;
    while(*p) {
        if (*p == HEX_ESCAPE) {
	    p++;
	    if (*p) *q = from_hex(*p++) * 16;
	    if (*p) *q = (*q + from_hex(*p++));
	    q++;
        } else if (*p == '+')
          {
            p++;
            *q++ = ' ';
	} else {
	    *q++ = *p++; 
	}
    }
    
    *q++ = 0;
    return str;
    
} /* HTUnEscape */
