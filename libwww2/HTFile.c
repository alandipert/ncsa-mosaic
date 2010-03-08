/*			File Access				HTFile.c
**			===========
**
**	This is unix-specific code in general, with some VMS bits.
**	These are routines for file access used by browsers.
**
** History:
**	   Feb 91	Written Tim Berners-Lee CERN/CN
**	   Apr 91	vms-vms access included using DECnet syntax
**	26 Jun 92 (JFG) When running over DECnet, suppressed FTP.
**			Fixed access bug for relative names on VMS.
**
** Bugs:
**	FTP: Cannot access VMS files from a unix machine.
**      How can we know that the
**	target machine runs VMS?
*/
#include "../config.h"
#include "HTFile.h"		/* Implemented here */

#define INFINITY 512		/* file name length @@ FIXME */
#define MULTI_SUFFIX ".multi"   /* Extension for scanning formats */

#include <stdio.h>
#include <sys/param.h>
#include "HText.h"
#include "HTUtils.h"

#include "HTParse.h"
#include "tcp.h"
#include "HTTCP.h"
#include "HTFTP.h"
#include "HTAnchor.h"
#include "HTAtom.h"
#include "HTWriter.h"
#include "HTFWriter.h"
#include "HTInit.h"
#include "HTSort.h"
#include "../libnut/system.h"

typedef struct _HTSuffix {
	char *		suffix;
	HTAtom *	rep;
	HTAtom *	encoding;
	float		quality;
} HTSuffix;


#ifdef USE_DIRENT		/* Set this for Sys V systems */
#define STRUCT_DIRENT struct dirent
#else
#define STRUCT_DIRENT struct direct
#endif

#include "HTML.h"		/* For directory object building */

#define PUTC(c) (*target->isa->put_character)(target, c)
#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)
#define END_TARGET (*target->isa->end_document)(target)
#define FREE_TARGET (*target->isa->free)(target)
struct _HTStructured {
	WWW_CONST HTStructuredClass *	isa;
	/* ... */
};

/* For, e.g., Solaris. */
#ifndef NGROUPS
#define NGROUPS 64
#endif

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/*                   Controlling globals
**
*/

PUBLIC int HTDirAccess = HT_DIR_OK;
PUBLIC int HTDirReadme = HT_DIR_README_TOP;

#ifdef vms
PRIVATE char *HTCacheRoot = "/WWW$SCRATCH/";   /* Where to cache things */
#else
PRIVATE char *HTCacheRoot = "/tmp/W3_Cache_";   /* Where to cache things */
#endif

/* PRIVATE char *HTSaveRoot  = "$(HOME)/WWW/";*/    /* Where to save things */


/*	Suffix registration
*/

PUBLIC HTList * HTSuffixes = 0;
PRIVATE HTSuffix no_suffix = { "*", NULL, NULL, 1.0 };
PRIVATE HTSuffix unknown_suffix = { "*.*", NULL, NULL, 1.0};


/*	Define the representation associated with a file suffix
**	-------------------------------------------------------
**
**	Calling this with suffix set to "*" will set the default
**	representation.
**	Calling this with suffix set to "*.*" will set the default
**	representation for unknown suffix files which contain a ".".
*/
PUBLIC void HTSetSuffix ARGS4(
	WWW_CONST char *,	suffix,
	WWW_CONST char *,	representation,
	WWW_CONST char *,	encoding,
	float,		value)
{
    
    HTSuffix * suff;
    
    if (strcmp(suffix, "*")==0) suff = &no_suffix;
    else if (strcmp(suffix, "*.*")==0) suff = &unknown_suffix;
    else {
	suff = (HTSuffix*) calloc(1, sizeof(HTSuffix));
	if (suff == NULL) outofmem(__FILE__, "HTSetSuffix");
	
	if (!HTSuffixes) HTSuffixes = HTList_new();
	HTList_addObject(HTSuffixes, suff);
	
	StrAllocCopy(suff->suffix, suffix);
    }

    suff->rep = HTAtom_for(representation);
    
    {
    	char *enc = NULL, *p;
	StrAllocCopy(enc, encoding);
	for (p=enc; *p; p++) *p = TOLOWER(*p);
	suff->encoding = HTAtom_for(enc);
        free (enc);
    }
    
    suff->quality = value;
}




#ifdef vms
/*	Convert unix-style name into VMS name
**	-------------------------------------
**
** Bug:	Returns pointer to static -- non-reentrant
*/
PRIVATE char * vms_name(WWW_CONST char * nn, WWW_CONST char * fn)
{

/*	We try converting the filename into Files-11 syntax. That is, we assume
**	first that the file is, like us, on a VMS node. We try remote
**	(or local) DECnet access. Files-11, VMS, VAX and DECnet
**	are trademarks of Digital Equipment Corporation. 
**	The node is assumed to be local if the hostname WITHOUT DOMAIN
**	matches the local one. @@@
*/
    static char vmsname[INFINITY];	/* returned */
    char * filename = (char*)malloc(strlen(fn)+1);
    char * nodename = (char*)malloc(strlen(nn)+2+1);	/* Copies to hack */
    char *second;		/* 2nd slash */
    char *last;			/* last slash */
    
    char * hostname = HTHostName();

    if (!filename || !nodename) outofmem(__FILE__, "vms_name");
    strcpy(filename, fn);
    strcpy(nodename, "");	/* On same node? Yes if node names match */
    {
        char *p, *q;
        for (p=hostname, q=nn; *p && *p!='.' && *q && *q!='.'; p++, q++){
	    if (TOUPPER(*p)!=TOUPPER(*q)) {
	        strcpy(nodename, nn);
		q = strchr(nodename, '.');	/* Mismatch */
		if (q) *q=0;			/* Chop domain */
		strcat(nodename, "::");		/* Try decnet anyway */
		break;
	    }
	}
    }

    second = strchr(filename+1, '/');		/* 2nd slash */
    last = strrchr(filename, '/');	/* last slash */
        
    if (!second) {				/* Only one slash */
	sprintf(vmsname, "%s%s", nodename, filename + 1);
    } else if(second==last) {		/* Exactly two slashes */
	*second = 0;		/* Split filename from disk */
	sprintf(vmsname, "%s%s:%s", nodename, filename+1, second+1);
	*second = '/';	/* restore */
    } else { 				/* More than two slashes */
	char * p;
	*second = 0;		/* Split disk from directories */
	*last = 0;		/* Split dir from filename */
	sprintf(vmsname, "%s%s:[%s]%s",
		nodename, filename+1, second+1, last+1);
	*second = *last = '/';	/* restore filename */
	for (p=strchr(vmsname, '['); *p!=']'; p++)
	    if (*p=='/') *p='.';	/* Convert dir sep.  to dots */
    }
    free(nodename);
    free(filename);
    return vmsname;
}


#endif /* vms */



/*	Send README file
**
**  If a README file exists, then it is inserted into the document here.
*/

#ifdef GOT_READ_DIR
PRIVATE void do_readme ARGS2(HTStructured *, target, WWW_CONST char *, localname)
{ 
    FILE * fp;
    char * readme_file_name = 
	malloc(strlen(localname)+ 1 + strlen(HT_DIR_README_FILE) + 1);
    strcpy(readme_file_name, localname);
    strcat(readme_file_name, "/");
    strcat(readme_file_name, HT_DIR_README_FILE);
    
    fp = fopen(readme_file_name,  "r");
    
    if (fp) {
	HTStructuredClass targetClass;
	
	targetClass =  *target->isa;	/* (Can't init agregate in K&R) */
	START(HTML_PRE);
	for(;;){
	    char c = fgetc(fp);
	    if (c == (char)EOF) break;
	    switch (c) {
	    	case '&':
		case '<':
		case '>':
			PUTC('&');
			PUTC('#');
			PUTC((char)(c / 10));
			PUTC((char) (c % 10));
			PUTC(';');
			break;
/*	    	case '\n':
			PUTC('\r');    
Bug removed thanks to joe@athena.mit.edu */			
		default:
			PUTC(c);
	    }
	}
	END(HTML_PRE);
	fclose(fp);
    } 
}
#endif


/*	Make the cache file name for a W3 document
**	------------------------------------------
**	Make up a suitable name for saving the node in
**
**	E.g.	/tmp/WWW_Cache_news/1234@cernvax.cern.ch
**		/tmp/WWW_Cache_http/crnvmc/FIND/xx.xxx.xx
**
** On exit,
**	returns	a malloc'ed string which must be freed by the caller.
*/
PUBLIC char * HTCacheFileName ARGS1(WWW_CONST char *,name)
{
    char * access = HTParse(name, "", PARSE_ACCESS);
    char * host = HTParse(name, "", PARSE_HOST);
    char * path = HTParse(name, "", PARSE_PATH+PARSE_PUNCTUATION);
    
    char * result;
    result = (char *)malloc(
	    strlen(HTCacheRoot)+strlen(access)
	    +strlen(host)+strlen(path)+6+1);
    if (result == NULL) outofmem(__FILE__, "HTCacheFileName");
    sprintf(result, "%s/WWW/%s/%s%s", HTCacheRoot, access, host, path);
    free(path);
    free(access);
    free(host);
    return result;
}


/*	Open a file for write, creating the path
**	----------------------------------------
*/
#ifdef NOT_IMPLEMENTED
PRIVATE int HTCreatePath ARGS1(WWW_CONST char *,path)
{
    return -1;
}
#endif

/*	Convert filenames between local and WWW formats
**	-----------------------------------------------
** On exit,
**	returns	a malloc'ed string which must be freed by the caller.
*/
PUBLIC char * HTLocalName ARGS1(WWW_CONST char *,name)
{
  char * access = HTParse(name, "", PARSE_ACCESS);
  char * host = HTParse(name, "", PARSE_HOST);
  char * path = HTParse(name, "", PARSE_PATH+PARSE_PUNCTUATION);
  
  HTUnEscape(path);	/* Interpret % signs */
  
  if (0==strcmp(access, "file")) 
    {
      free(access);	
      if (!host || !*host || (0==my_strcasecmp(host, HTHostName())) ||
          (0==my_strcasecmp(host, "localhost"))) 
        {
          if (host)
            free(host);
#ifndef DISABLE_TRACE
          if (www2Trace) 
            fprintf(stderr, "Node `%s' means path `%s'\n", name, path);
#endif
          return(path);
        }
      else
        {
          free (host);
          if (path)
            free (path);
          return NULL;
        }
    }
  
  /* not file */
  if (host)
    free (host);
  free (access);
  if (path)
    free (path);
  return NULL;
}


/*	Make a WWW name from a full local path name
**
** Bugs:
**	At present, only the names of two network root nodes are hand-coded
**	in and valid for the NeXT only. This should be configurable in
**	the general case.
*/

PUBLIC char * WWW_nameOfFile ARGS1 (WWW_CONST char *,name)
{
    char * result;
    result = (char *)malloc(7+strlen(HTHostName())+strlen(name)+1);
    if (result == NULL) outofmem(__FILE__, "WWW_nameOfFile");
    sprintf(result, "file://%s%s", HTHostName(), name);
#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr, "File `%s'\n\tmeans node `%s'\n", name, result);
#endif
    return result;
}


/*	Determine a suitable suffix, given the representation
**	-----------------------------------------------------
**
** On entry,
**	rep	is the atomized MIME style representation
**
** On exit,
**	returns	a pointer to a suitable suffix string if one has been
**		found, else "".
*/
PUBLIC WWW_CONST char * HTFileSuffix ARGS1(HTAtom*, rep)
{
    HTSuffix * suff;
    int n;
    int i;

    if (!HTSuffixes) HTFileInit();
    n = HTList_count(HTSuffixes);
    for(i=0; i<n; i++) {
	suff = HTList_objectAt(HTSuffixes, i);
	if (suff->rep == rep) {
	    return suff->suffix;		/* OK -- found */
	}
    }
    return "";		/* Dunno */
}


/*	Determine file format from file name
**	------------------------------------
**
**	This version will return the representation and also set
**	a variable for the encoding.
**
**	It will handle for example  x.txt, x.txt.Z, x.Z
*/

PUBLIC HTFormat HTFileFormat ARGS4 (
			char *,	filename,
			HTAtom **,	pencoding,
                        HTAtom *,       default_type,
                        int *, compressed)
{
  HTSuffix *suff;
  int n, i, lf;

  if (!filename)
    return NULL;

  /* Make a copy to hack and slash. */
  filename = strdup (filename);

  lf = strlen (filename);

  /* Step backward through filename, looking for '?'. */
  for (i = lf - 1; i >= 0; i--)
    {
      if (filename[i] == '?')
        {
          /* Clip query. */
          filename[i] = '\0';
          /* Get new strlen, since we just changed it. */
          lf = strlen (filename);
          goto ok_ready;
        }
    }

  *compressed = 0;

  /* Check for .Z and .z. */
  if (lf > 2)
    {
      if (strcmp (&(filename[lf-2]), ".Z") == 0)
        {
          *compressed = COMPRESSED_BIGZ;
          filename[lf-2] = '\0';
          lf = strlen (filename);
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "[HTFileFormat] Got hit on .Z; filename '%s'\n",
                     filename);
#endif
          goto ok_ready;
        }
      else if (strcmp (&(filename[lf-2]), ".z") == 0)
        {
          *compressed = COMPRESSED_GNUZIP;
          filename[lf-2] = '\0';
          lf = strlen (filename);
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "[HTFileFormat] Got hit on .z; filename '%s'\n",
                     filename);
#endif
          goto ok_ready;
        }
      else if (lf > 3)
        {
          if (strcmp (&(filename[lf-3]), ".gz") == 0)
            {
              *compressed = COMPRESSED_GNUZIP;
              filename[lf-3] = '\0';
              lf = strlen (filename);
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf (stderr, 
                         "[HTFileFormat] Got hit on .gz; filename '%s'\n",
                         filename);
#endif
              goto ok_ready;
            }
        }      
    }      
  
 ok_ready:
  if (!HTSuffixes) 
    HTFileInit();

  *pencoding = NULL;

  n = HTList_count(HTSuffixes);

  for(i=0; i<n; i++) 
    {
      int ls;
      suff = HTList_objectAt(HTSuffixes, i);
      ls = strlen(suff->suffix);
      if ((ls <= lf) && 0==my_strcasecmp(suff->suffix, filename + lf - ls)) 
        {
          int j;
          *pencoding = suff->encoding;
          if (suff->rep) 
            goto done;
          
          for(j=0; j<n; j++) 
            {  /* Got encoding, need representation */
              int ls2;
              suff = HTList_objectAt(HTSuffixes, j);
              ls2 = strlen(suff->suffix);
              if ((ls <= lf) && 
                  0==my_strncasecmp(suff->suffix, filename + lf - ls -ls2, ls2)) 
                if (suff->rep) 
                  goto done;
	    }
        }
    }
  
  suff = strchr(filename, '.') ? 	/* Unknown suffix */
    ( unknown_suffix.rep ? &unknown_suffix : &no_suffix)
      : &no_suffix;
  
  /* For now, assuming default is 8bit text/plain.
     We also want default 8bit text/html for http connections. */
  
  /* set default encoding unless found with suffix already */
  if (!*pencoding) *pencoding = suff->encoding ? suff->encoding
    : HTAtom_for("8bit");

 done:

  /* Free our copy. */
  free (filename);
  return suff->rep ? suff->rep : default_type;
}


/*	Determine file format from file name -- string version
**	------------------------------------------------------
*/
PUBLIC char *HTFileMimeType ARGS2 (
			WWW_CONST char *,	filename,
                        WWW_CONST char *,   default_type)
{
  HTAtom *pencoding;
  HTFormat format;
  int compressed;

  format = HTFileFormat (filename, &pencoding, HTAtom_for (default_type),
                         &compressed);

  if (HTAtom_name (format))
    return HTAtom_name (format);
  else
    return default_type;
}

/* This doesn't do Gopher typing yet. */
/* This assumes we get a canonical URL and that HTParse works. */
char *HTDescribeURL (char *url)
{
  char line[512];
  char *type, *t, *st = NULL;
  char *host;
  char *access;
  int i;

  if (!url || !*url)
    return "Completely content-free.";

  if (strncmp ("http:", url, 5) == 0)
    type = HTFileMimeType (url, "text/html");
  else
    type = HTFileMimeType (url, "text/plain");

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "DESCRIBE: type '%s'\n", type);
#endif

  t = strdup (type);
  for (i = 0; i < strlen (t); i++)
    {
      if (t[i] == '/')
        {
          t[i] = '\0';
          if (t[i+1] != '\0' && t[i+1] != '*')
            st = &(t[i+1]);
          goto got_subtype;
        }
    }
 got_subtype:
  
  access = HTParse (url, "", PARSE_ACCESS);
  if (strcmp (access, "http") == 0)
    {
      access[0] = 'H';
      access[1] = 'T';
      access[2] = 'T';
      access[3] = 'P';
    }
  else if (strcmp (access, "ftp") == 0)
    {
      access[0] = 'F';
      access[1] = 'T';
      access[2] = 'P';
    }
  else
    {
      access[0] = toupper(access[0]);
    }

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "DESCRIBE: url '%s'\n", url);
#endif

  host = HTParse (url, "", PARSE_HOST);

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "DESCRIBE: host '%s'\n", host);
#endif

#if 0
  for (i = 0; i < strlen (host); i++)
    if (host[i] == ':')
      host[i] = '\0';
#endif
  
  if (st)
    {
      /* Uppercase type, to start sentence. */
      t[0] = toupper(t[0]);
      /* Crop x- from subtype. */
      if (st[0] == 'x' && st[1] == '-')
        st = &(st[2]);
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, 
                 "DESCRIBE: in if (st); pasting together %s %s %s %s %s\n",
                 t, 
                 (strcmp (t, "Application") == 0 ? " data" : ""), 
                 st, host, access);
#endif
      sprintf (line, "%s%s, type %s, on host %s, via %s.", 
               t, 
               (strcmp (t, "Application") == 0 ? " data" : ""), 
               st, host, access);
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "DESCRIBE: pasted together '%s'\n", line);
#endif
    }
  else
    {
      sprintf (line, "Type %s, on host %s, via %s.", type, host, access);
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "DESCRIBE: pasted together '%s'\n", line);
#endif
    }

  free (access);
  free (host);
  free (t);

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "DESCRIBE: returning '%s'\n", line);
#endif

  return strdup (line);
}


/*	Determine value from file name
**	------------------------------
**
*/

PUBLIC float HTFileValue ARGS1 (WWW_CONST char *,filename)

{
    HTSuffix * suff;
    int n;
    int i;
    int lf = strlen(filename);

    if (!HTSuffixes) HTFileInit();
    n = HTList_count(HTSuffixes);
    for(i=0; i<n; i++) {
        int ls;
	suff = HTList_objectAt(HTSuffixes, i);
	ls = strlen(suff->suffix);
	if ((ls <= lf) && 0==strcmp(suff->suffix, filename + lf - ls)) {
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr, "File: Value of %s is %.3f\n",
			       filename, suff->quality);
#endif
	    return suff->quality;		/* OK -- found */
	}
    }
    return 0.3;		/* Dunno! */
}


/*	Determine write access to a file
**	--------------------------------
**
** On exit,
**	return value	YES if file can be accessed and can be written to.
**
** Bugs:
**	1.	No code for non-unix systems.
**	2.	Isn't there a quicker way?
*/

#ifdef vms
#define NO_GROUPS
#endif
#ifdef NO_UNIX_IO
#define NO_GROUPS
#endif
#ifdef PCNFS
#define NO_GROUPS
#endif

PUBLIC BOOL HTEditable ARGS1 (WWW_CONST char *,filename)
{
#ifdef NO_GROUPS
    return NO;		/* Safe answer till we find the correct algorithm */
#else
    int 	groups[NGROUPS];	
    uid_t	myUid;
    int		ngroups;			/* The number of groups  */
    struct stat	fileStatus;
    int		i;
        
    if (stat(filename, &fileStatus))		/* Get details of filename */
    	return NO;				/* Can't even access file! */

    /* The group stuff appears to be coming back garbage on IRIX... why? */
    ngroups = getgroups(NGROUPS, groups);	/* Groups to which I belong  */
    myUid = geteuid();				/* Get my user identifier */

#ifndef DISABLE_TRACE
    if (www2Trace) {
        int i;
	fprintf(stderr, 
	    "File mode is 0%o, uid=%d, gid=%d. My uid=%d, %d groups (",
    	    (unsigned int) fileStatus.st_mode, fileStatus.st_uid,
	    fileStatus.st_gid,
	    myUid, ngroups);
	for (i=0; i<ngroups; i++) fprintf(stderr, " %d", groups[i]);
	fprintf(stderr, ")\n");
    }
#endif
    
    if (fileStatus.st_mode & 0002)		/* I can write anyway? */
    	return YES;
	
    if ((fileStatus.st_mode & 0200)		/* I can write my own file? */
     && (fileStatus.st_uid == myUid))
    	return YES;

    if (fileStatus.st_mode & 0020)		/* Group I am in can write? */
    {
   	for (i=0; i<ngroups; i++) {
            if (groups[i] == fileStatus.st_gid)
	        return YES;
	}
    }
#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr, "\tFile is not editable.\n");
#endif
    return NO;					/* If no excuse, can't do */
#endif
}



/*      Output one directory entry
**
*/
PUBLIC void HTDirEntry ARGS3(HTStructured *, target,
		 WWW_CONST char * , tail,
		 WWW_CONST char *,  entry)
{
    char * relative;
    char * escaped = HTEscape(entry);

    /* If empty tail, gives absolute ref below */
    relative = (char*) malloc(strlen(tail) + strlen(escaped)+2);
    sprintf(relative, "%s/%s", tail, escaped);
    PUTS("<A HREF=\"");
    PUTS(relative);
    PUTS("\">");
    free(escaped);
    free(relative);
    PUTS(entry);
    PUTS("</A>");
}
 
/*      Output parent directory entry
**
**    This gives the TITLE and H1 header, and also a link
**    to the parent directory if appropriate.
*/
PUBLIC void HTDirTitles ARGS2(HTStructured *, target,
		 HTAnchor * , anchor)

{
    char * logical = HTAnchor_address(anchor);
    char * path = HTParse(logical, "", PARSE_PATH + PARSE_PUNCTUATION);
    char * current;

    current = strrchr(path, '/');	/* last part or "" */
    free(logical);

    {
      char * printable = NULL;
      StrAllocCopy(printable, (current + 1));
      HTUnEscape(printable);
      START(HTML_TITLE);
      PUTS(*printable ? printable : "Welcome ");
      PUTS(" directory");
      END(HTML_TITLE);    
    
      START(HTML_H1);
      PUTS(*printable ? printable : "Welcome");
      END(HTML_H1);
      free(printable);
    }

    /*  Make link back to parent directory
     */

    if (current && current[1]) {   /* was a slash AND something else too */
        char * parent;
	char * relative;
	*current++ = 0;
      parent = strrchr(path, '/');  /* penultimate slash */

	relative = (char*) malloc(strlen(current) + 4);
	if (relative == NULL) outofmem(__FILE__, "DirRead");
	sprintf(relative, "%s/..", current);
        PUTS ("<A HREF=\"");
        PUTS (relative);
        PUTS ("\">");
	free(relative);

	PUTS("Up to ");
	if (parent) {
	  char * printable = NULL;
	  StrAllocCopy(printable, parent + 1);
	  HTUnEscape(printable);
	  PUTS(printable);
	  free(printable);
	} else {
	  PUTS("/");
	}

        PUTS("</A>");
      }
    free(path);
}
		


/*	Load a document
**	---------------
**
** On entry,
**	addr		must point to the fully qualified hypertext reference.
**			This is the physsical address of the file
**
** On exit,
**	returns		<0		Error has occured.
**			HTLOADED	OK 
**
*/
PUBLIC int HTLoadFile ARGS4 (
	WWW_CONST char *,		addr,
	HTParentAnchor *,	anchor,
	HTFormat,		format_out,
	HTStream *,		sink
)
{
    char * filename;
    HTFormat format;
    int fd = -1;		/* Unix file descriptor number = INVALID */
    char * nodename = 0;
    char * newname=0;	/* Simplified name of file */
    HTAtom * encoding;	/* @@ not used yet */
    int compressed;
    extern char *HTgeticonname(HTFormat, char *);
    
/*	Reduce the filename to a basic form (hopefully unique!)
*/
    StrAllocCopy(newname, addr);
    filename=HTParse(newname, "", PARSE_PATH|PARSE_PUNCTUATION);
    nodename=HTParse(newname, "", PARSE_HOST);
    free(newname);
    
    format = HTFileFormat(filename, &encoding, WWW_PLAINTEXT, &compressed);


#ifdef vms
/* Assume that the file is in Unix-style syntax if it contains a '/'
   after the leading one @@ */
    {
	char * vmsname = strchr(filename + 1, '/') ?
	  vms_name(nodename, filename) : filename + 1;
	fd = open(vmsname, O_RDONLY, 0);
	
/*	If the file wasn't VMS syntax, then perhaps it is ultrix
*/
	if (fd<0) {
	    char ultrixname[INFINITY];
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr, "HTFile: Can't open as %s\n", vmsname);
#endif
	    sprintf(ultrixname, "%s::\"%s\"", nodename, filename);
	    fd = open(ultrixname, O_RDONLY, 0);
	    if (fd<0) {
#ifndef DISABLE_TRACE
		if (www2Trace) fprintf(stderr, 
				   "HTFile: Can't open as %s\n", ultrixname);
#endif
	    }
	}
    }
#else

    free(filename);
    
/*	For unix, we try to translate the name into the name of a transparently
**	mounted file.
**
**	Not allowed in secure (HTClienntHost) situations TBL 921019
*/
#ifndef NO_UNIX_IO
    /*  Need protection here for telnet server but not httpd server */
	 
    {		/* try local file system */
	char * localname = HTLocalName(addr);
	struct stat dir_info;

        if (!localname)
          goto suicide;
	
#ifdef GOT_READ_DIR

/*			  Multiformat handling
**
**	If needed, scan directory to find a good file.
**  Bug:  we don't stat the file to find the length
*/
	if ( (strlen(localname) > strlen(MULTI_SUFFIX))
	   && (0==strcmp(localname + strlen(localname) - strlen(MULTI_SUFFIX),
	                  MULTI_SUFFIX))) {
	    DIR *dp;

	    STRUCT_DIRENT * dirbuf;
	    float best = NO_VALUE_FOUND;	/* So far best is bad */
	    HTFormat best_rep = NULL;	/* Set when rep found */
	    STRUCT_DIRENT best_dirbuf;	/* Best dir entry so far */

	    char * base = strrchr(localname, '/');
	    int baselen;

	    if (!base || base == localname) goto forget_multi;
	    *base++ = 0;		/* Just got directory name */
	    baselen = strlen(base)- strlen(MULTI_SUFFIX);
	    base[baselen] = 0;	/* Chop off suffix */

	    dp = opendir(localname);
	    if (!dp) {
forget_multi:
		free(localname);
		return HTLoadError(sink, 500,
			"Multiformat: directory scan failed.");
	    }
	    
	    while (dirbuf = readdir(dp)) {
			/* while there are directory entries to be read */
		if (dirbuf->d_ino == 0) continue;
				/* if the entry is not being used, skip it */
		
		if (!strncmp(dirbuf->d_name, base, baselen)) {	
		    HTFormat rep = HTFileFormat(dirbuf->d_name, &encoding,
                                                WWW_PLAINTEXT, &compressed);
		    float value = HTStackValue(rep, format_out,
		    				HTFileValue(dirbuf->d_name),
						0.0  /* @@@@@@ */);
		    if (value != NO_VALUE_FOUND) {
#ifndef DISABLE_TRACE
		        if (www2Trace) fprintf(stderr,
				"HTFile: value of presenting %s is %f\n",
				HTAtom_name(rep), value);
#endif
			if  (value > best) {
			    best_rep = rep;
			    best = value;
			    best_dirbuf = *dirbuf;
		       }
		    }	/* if best so far */ 		    
		 } /* if match */  
		    
	    } /* end while directory entries left to read */
	    closedir(dp);
	    
	    if (best_rep) {
		format = best_rep;
		base[-1] = '/';		/* Restore directory name */
		base[0] = 0;
		StrAllocCat(localname, best_dirbuf.d_name);
		goto open_file;
		
	    } else { 			/* If not found suitable file */
		free(localname);
		return HTLoadError(sink, 403,	/* List formats? */
		   "Could not find suitable representation for transmission.");
	    }
	    /*NOTREACHED*/
	} /* if multi suffix */
/*
**	Check to see if the 'localname' is in fact a directory.  If it is
**	create a new hypertext object containing a list of files and 
**	subdirectories contained in the directory.  All of these are links
**      to the directories or files listed.
**      NB This assumes the existance of a type 'STRUCT_DIRENT', which will
**      hold the directory entry, and a type 'DIR' which is used to point to
**      the current directory being read.
*/
	
	
	if (stat(localname,&dir_info) == -1) {     /* get file information */
	                               /* if can't read file information */
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr, "HTFile: can't stat %s\n", localname);
#endif
	}  else {		/* Stat was OK */
		

	    if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) {
		/* if localname is a directory */	

/*
**
** Read the localdirectory and present a nicely formatted list to the user
** Re-wrote most of the read directory code here, excepting for the checking
** access.
**
** Author: Charles Henrich (henrich@crh.cl.msu.edu)   10-09-93
**
** This is still pretty messy, need to go through and clean it up at some point
**
*/

/* Define some parameters that everyone should already have */

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

                char filepath[MAXPATHLEN];
                char buffer[4096];
     
                char *ptr;
                char *dataptr;

                HText * HT;
                HTFormat format;
                HTAtom *pencoding;

		struct stat statbuf;
		STRUCT_DIRENT * dp;
		DIR *dfp;
 
                int cmpr;
                int count;

#ifndef DISABLE_TRACE
		if (www2Trace)
		    fprintf(stderr,"%s is a directory\n",localname);
#endif			
/*	Check directory access.
**	Selective access means only those directories containing a
**	marker file can be browsed
*/
		if (HTDirAccess == HT_DIR_FORBID) {
		    free(localname);
		    return HTLoadError(sink, 403,
		    "Directory browsing is not allowed.");
		}


		if (HTDirAccess == HT_DIR_SELECTIVE) {
		    char * enable_file_name = 
			malloc(strlen(localname)+ 1 +
			 strlen(HT_DIR_ENABLE_FILE) + 1);
		    strcpy(enable_file_name, localname);
		    strcat(enable_file_name, "/");
		    strcat(enable_file_name, HT_DIR_ENABLE_FILE);
		    if (stat(enable_file_name, &statbuf) != 0) {
			free(localname);
			return HTLoadError(sink, 403,
			"Selective access is not enabled for this directory.");
		    }
		}

 
		dfp = opendir(localname);
		if (!dfp) {
		    free(localname);
		    return HTLoadError(sink, 403, "This directory is not readable.");
		}

/* Suck the directory up into a list to be sorted */

                HTSortInit();

                for(dp=readdir(dfp);dp != NULL;dp=readdir(dfp))
                    {
                    ptr = malloc(strlen(dp->d_name)+1);
                    if(ptr == NULL)
                        {
		        return HTLoadError(sink, 403, "Ran out of memory in directory read!");
                        }
                    strcpy(ptr,dp->d_name);
                     
                    HTSortAdd(ptr);
                    }

                closedir(dfp);

/* Sort the dir list */

                HTSortSort();

/* Start a new HTML page */

                HT = HText_new();
                HText_beginAppend(HT);
                HText_appendText(HT, "<H1>Local Directory ");
                HText_appendText(HT, localname);
                HText_appendText(HT, "</H1>\n");
                HText_appendText(HT,"<DL>\n"); 

/* Sort the list and then spit it out in a nice form */

/* How this for a disgusting loop :) */

                for(count=0,dataptr=HTSortFetch(count); 
                    dataptr != NULL; 
                    free(dataptr), count++, dataptr=HTSortFetch(count))
                    {

/* We dont want to see . */

                    if(strcmp(dataptr,".") == 0) continue;
 
/* If its .. *and* the current directory is / dont show anything, otherwise
/* print out a nice Parent Directory entry.
/* */

                    if(strcmp(dataptr,"..") == 0)
                        {
                        if(strcmp(localname,"/") != 0)
                            {
                            strcpy(buffer,localname);

                            ptr = strrchr(buffer, '/');

                            if(ptr != NULL) *ptr='\0'; 

                            if(buffer[0] == '\0') strcpy(buffer,"/");

                            HText_appendText(HT,"<DD><A HREF=\"");
                            HText_appendText(HT, buffer);

                            HText_appendText(HT,"\"><IMG SRC=\"");
                            HText_appendText(HT, HTgeticonname(NULL, "directory"));

                            HText_appendText(HT,"\"> Parent Directory</a>");
                            continue;
                            }
                        else
                            {
                            continue;
                            }
                        }
                      
/* Get the filesize information from a stat, if we cant stat it, we probably */
/* cant read it either, so ignore it. */

                    sprintf(filepath,"%s/%s",localname, dataptr);

                    if(stat(filepath, &statbuf) == -1) continue;
     
                    HText_appendText(HT,"<DD><A HREF=\"");
                    HText_appendText (HT, localname);

                    if(localname[strlen(localname)-1] != '/') 
                        {
                        HText_appendText (HT, "/");
                        }

                    HText_appendText (HT, dataptr);
                    HText_appendText (HT, "\">");

/* If its a directory, dump out a dir icon, dont bother with anything else */
/* if it is a file try and figure out what type of file it is, and grab    */
/* the appropriate icon.  If we cant figure it out, call it text.  If its  */
/* a compressed file, call it binary no matter what                        */

                    if(statbuf.st_mode & S_IFDIR)
                        {
                        sprintf(buffer,"%s",dataptr);
                        HText_appendText(HT, "<IMG SRC=\"");
                        HText_appendText(HT, HTgeticonname(NULL, "directory"));
                        HText_appendText(HT, "\"> ");
                        }
                    else
                        {
                        sprintf(buffer,"%s (%d bytes)", 
                            dataptr, statbuf.st_size);
              
                        format = HTFileFormat(dataptr, &pencoding, 
                                     WWW_SOURCE, &cmpr);

/* If its executable then call it application, else it might as well be text */

                        if(cmpr == 0)
                            {
                            HText_appendText(HT, "<IMG SRC=\"");
                            if((statbuf.st_mode & S_IXUSR) ||
                               (statbuf.st_mode & S_IXGRP) || 
                               (statbuf.st_mode & S_IXOTH))
                                {
                                HText_appendText(HT, 
                                    HTgeticonname(format, "application"));
                                }
                            else
                                {
                                HText_appendText(HT, 
                                    HTgeticonname(format, "text"));
                                }
                            HText_appendText(HT, "\"> ");
                            }
                        else
                            {
                            HText_appendText(HT, "<IMG SRC=\"");
                            HText_appendText(HT, HTgeticonname(NULL, "application"));
                            HText_appendText(HT, "\"> ");
                            }
                        }

/* Spit out the anchor */

                    HText_appendText (HT, buffer);
                    HText_appendText (HT, "</A>\n");
		    }

/* End of list, clean up and we are done */

            HText_appendText (HT, "</DL>\n");
            HText_endAppend (HT);
            free(localname);
            return HT_LOADED;
	    } /* end if localname is directory */
	
	} /* end if file stat worked */
	
/* End of directory reading section
*/
#endif
open_file:
	{
	    FILE * fp = fopen(localname,"r");
#ifndef DISABLE_TRACE
	    if(www2Trace) fprintf (stderr, "HTFile: Opening `%s' gives %p\n",
				localname, (void*)fp);
#endif
	    if (fp) {		/* Good! */
		if (HTEditable(localname)) {
		    HTAtom * put = HTAtom_for("PUT");
		    HTList * methods = HTAnchor_methods(anchor);
		    if (HTList_indexOf(methods, put) == (-1)) {
			HTList_addObject(methods, put);
		    }
		}
		free(localname);
		HTParseFile(format, format_out, anchor, fp, sink, compressed);
/*
This is closed elsewhere...SWP
		fclose(fp);
*/
		return HT_LOADED;
	    }  /* If succesfull open */
	}    /* scope of fp */
    }  /* local unix file system */    
#endif
#endif

/*	Now, as transparently mounted access has failed, we try FTP.
*/
  suicide:
/*
    return HTFTPLoad(addr, anchor, format_out, sink);
*/
  /* Sorry Charlie...if we are given a file:// URL and it fails, then it
	fails! Do NOT FTP!! */
    return HT_NOT_LOADED;
}

/*		Protocol descriptors
*/
PUBLIC HTProtocol HTFTP  = { "ftp", HTFTPLoad, 0 };
PUBLIC HTProtocol HTFile = { "file", HTLoadFile, 0 };
