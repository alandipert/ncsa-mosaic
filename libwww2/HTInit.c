#include "../config.h"
#include "HTInit.h"

#include "HTML.h"
#include "HTPlain.h"
#include "HTMosaicHTML.h"
#include "HTMLGen.h"
#include "HTFile.h"
#include "HTFormat.h"
#include "HTMIME.h"
#include "HTWSRC.h"

#include "tcp.h"
#include "HTUtils.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

int HTLoadTypesConfigFile(char *);

/* Reread config files. */
PUBLIC void HTReInit NOARGS
{
  if (HTPresentations)
    {
      HTList_delete (HTPresentations);
      HTPresentations = 0;
    }
  HTFormatInit ();

  if (HTSuffixes)
    {
      HTList_delete (HTSuffixes);
      HTSuffixes = 0;
    }
  HTFileInit ();

  return;
}

PUBLIC void HTFormatInit NOARGS
{
  extern int use_default_type_map;
  extern char *global_type_map;
  extern char *personal_type_map;
  extern int have_hdf;

  /* Conversions aren't customizable. */
  HTSetConversion("www/mime",  "*", HTMIMEConvert, 1.0, 0.0, 0.0);

  /* Wonder what HTML will end up as? */
  HTSetConversion("text/html", "www/present", HTMosaicHTMLPresent, 1.0, 0.0, 0.0);
  HTSetConversion("text/x-html", "www/present", HTMosaicHTMLPresent, 1.0, 0.0, 0.0);
  HTSetConversion("application/html", "www/present", HTMosaicHTMLPresent, 1.0, 0.0, 0.0);
  HTSetConversion("application/x-html", "www/present", HTMosaicHTMLPresent, 1.0, 0.0, 0.0);

  HTSetConversion("text/plain", "www/present", HTPlainPresent,	1.0, 0.0, 0.0);
  HTSetConversion("application/x-wais-source", "*", HTWSRCConvert, 1.0, 0.0, 0.0);

  /* These should override everything else. */
  HTLoadTypesConfigFile (personal_type_map);

  /* These should override the default types as necessary. */
  HTLoadTypesConfigFile (global_type_map);

  /* These should always be installed if we have internal support;
     can be overridden by users. */
  if (have_hdf)
    {
      HTSetPresentation("application/x-hdf",      "mosaic-internal-reference",
                        1.0, 3.0, 0.0);
      HTSetPresentation("application/x-netcdf",   "mosaic-internal-reference",
                        1.0, 3.0, 0.0);
      /* Jumping the gun, but still... */
      HTSetPresentation("application/hdf",      "mosaic-internal-reference",
                        1.0, 3.0, 0.0);
      HTSetPresentation("application/netcdf",   "mosaic-internal-reference",
                        1.0, 3.0, 0.0);
    }

  if (use_default_type_map)
    {
#if defined(__sgi)
      HTSetPresentation("audio/basic", "sfplay %s", 1.0, 3.0, 0.0);
      HTSetPresentation("audio/x-aiff", "sfplay %s", 1.0, 3.0, 0.0);
#else /* not __sgi */
#if defined(ultrix) || defined(__alpha)
      HTSetPresentation("audio/basic", "aplay %s", 1.0, 3.0, 0.0);
      HTSetPresentation("audio/x-aiff", "aplay %s", 1.0, 3.0, 0.0);
#else /* not ultrix or __alpha */
      HTSetPresentation("audio/basic", "showaudio %s", 1.0, 3.0, 0.0);
      HTSetPresentation("audio/x-aiff", "showaudio %s", 1.0, 3.0, 0.0);
#endif /* not ultrix or __alpha */
#endif /* not __sgi */

      HTSetPresentation("image/gif", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/jpeg", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/png", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-png", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/tiff", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-portable-anymap", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-portable-bitmap", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-portable-graymap", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-portable-pixmap", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-rgb", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/rgb", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-xbitmap", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-xpixmap", "xv %s", 1.0, 3.0, 0.0); /* ?? */

      HTSetPresentation("image/xwd", "xwud -in %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-xwd", "xwud -in %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-xwindowdump", "xwud -in %s", 1.0, 3.0, 0.0);

      HTSetPresentation("video/mpeg", "mpeg_play %s", 1.0, 3.0, 0.0);
#ifdef __sgi
      HTSetPresentation("video/quicktime", "movieplayer -f %s", 1.0, 3.0, 0.0); /* sgi */
      HTSetPresentation("video/x-sgi-movie", "movieplayer -f %s", 1.0, 3.0, 0.0); /* sgi */
#endif

      HTSetPresentation("application/postscript", "ghostview %s", 1.0, 3.0, 0.0);
      HTSetPresentation("application/x-dvi", "xdvi %s", 1.0, 3.0, 0.0);

      HTSetPresentation("message/rfc822",
                        "xterm -e metamail %s", 1.0, 3.0, 0.0);

      HTSetPresentation("application/x-latex", "mosaic-internal-present", 1.0, 3.0, 0.0);
      HTSetPresentation("application/x-tex", "mosaic-internal-present", 1.0, 3.0, 0.0);
      HTSetPresentation("application/x-texinfo", "mosaic-internal-present", 1.0, 3.0, 0.0);
      HTSetPresentation("application/x-troff", "mosaic-internal-present", 1.0, 3.0, 0.0);
      HTSetPresentation("application/x-troff-man", "mosaic-internal-present", 1.0, 3.0, 0.0);
      HTSetPresentation("application/x-troff-me", "mosaic-internal-present", 1.0, 3.0, 0.0);
      HTSetPresentation("application/x-troff-ms", "mosaic-internal-present", 1.0, 3.0, 0.0);
      HTSetPresentation("text/richtext", "mosaic-internal-present", 1.0, 3.0, 0.0);
      HTSetPresentation("text/tab-separated-values", "mosaic-internal-present", 1.0, 3.0, 0.0);
      HTSetPresentation("text/x-setext", "mosaic-internal-present", 1.0, 3.0, 0.0);
    }

  /* Fallthrough clauses. */

  HTSetPresentation ("*/*", "mosaic-internal-dump", 1.0, 3.0, 0.0);
  HTSetPresentation ("*", "mosaic-internal-dump", 1.0, 3.0, 0.0);

}


/* Some of the following is taken from: */

/*
Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)

Permission to use, copy, modify, and distribute this material
for any purpose and without fee is hereby granted, provided
that the above copyright notice and this permission notice
appear in all copies, and that the name of Bellcore not be
used in advertising or publicity pertaining to this
material without the specific, prior written permission
of an authorized representative of Bellcore.  BELLCORE
MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
*/
/******************************************************
    Metamail -- A tool to help diverse mail readers
                cope with diverse multimedia mail formats.

    Author:  Nathaniel S. Borenstein, Bellcore

 ******************************************************* */

struct MailcapEntry {
    char *contenttype;
    char *command;
    int needsterminal;
};

#define LINE_BUF_SIZE       2000

static char *GetCommand(char *s, char **t)
{
    char *s2;
    int quoted = 0;
    /* marca -- added + 1 for error case -- oct 24, 1993. */
    s2 = malloc(strlen(s)*2 + 1); /* absolute max, if all % signs */
    *t = s2;
    while (s && *s) {
	if (quoted) {
            if (*s == '%') *s2++ = '%'; /* Quote through next level, ugh! */

            *s2++ = *s++;
	    quoted = 0;
	} else {
	    if (*s == ';') {
                *s2 = 0;
		return(++s);
	    }
	    if (*s == '\\') {
		quoted = 1;
		++s;
	    } else {
		*s2++ = *s++;
	    }
	}
    }
    *s2 = 0;
    return(NULL);
}

static char *Cleanse(char *s) /* no leading or trailing space, all lower case */
{
    char *tmp, *news;

    /* strip leading white space */
    while (*s && isspace((unsigned char) *s)) ++s;
    news = s;
    /* put in lower case */
    for (tmp=s; *tmp; ++tmp) {
      *tmp = TOLOWER ((unsigned char)*tmp);
    }
    /* strip trailing white space */
    while (*--tmp && isspace((unsigned char) *tmp)) *tmp = 0;
    return(news);
}

static int ProcessMailcapEntry(FILE *fp, struct MailcapEntry *mc)
{
    int rawentryalloc = 2000, len;
    char *rawentry, *s, *t, *LineBuf;

    LineBuf = malloc(LINE_BUF_SIZE);
    rawentry = malloc(1 + rawentryalloc);
    *rawentry = 0;
    while (fgets(LineBuf, LINE_BUF_SIZE, fp)) {
	if (LineBuf[0] == '#') continue;
	len = strlen(LineBuf);
        if (LineBuf[len-1] == '\n') LineBuf[--len] = 0;
	if ((len + strlen(rawentry)) > rawentryalloc) {
	    rawentryalloc += 2000;
	    rawentry = realloc(rawentry, rawentryalloc+1);
	}
	if (len > 0 && LineBuf[len-1] == '\\') {
            LineBuf[len-1] = 0;
	    strcat(rawentry, LineBuf);
	} else {
	    strcat(rawentry, LineBuf);
	    break;
	}
    }
    free(LineBuf);
    for (s=rawentry; *s && isspace((unsigned char) *s); ++s) ;
    if (!*s) {
	/* totally blank entry -- quietly ignore */
	free(rawentry);
	return(0);
    }
    s = strchr(rawentry, ';');
    if (!s) {
#ifndef DISABLE_TRACE
      if (www2Trace)
	fprintf(stderr, "Ignoring invalid mailcap entry: %s\n", rawentry);
#endif
      free(rawentry);
      return(0);
    }
    *s++ = 0;
    mc->needsterminal = 0;
    mc->contenttype = malloc(1+strlen(rawentry));
    strcpy(mc->contenttype, rawentry);

    t = GetCommand(s, &mc->command);
    if (!t) {
        free(rawentry);
        goto do_presentation;
    }
    while (s && *s && isspace((unsigned char) *s)) ++s;
    s = t;
    while (s) {
	char *arg, *eq;

        t = GetCommand(s, &arg);
        eq = strchr(arg, '=');
        if (eq) *eq++ = 0;
        /* Error check added by marca, oct 24 1993. */
        if (arg && *arg)
          arg = Cleanse(arg);
	if (!strcmp(arg, "needsterminal")) {
	    mc->needsterminal = 1;
	}
	s = t;
    }

    free(rawentry);
  do_presentation:
    HTSetPresentation(mc->contenttype, mc->command, 1.0, 3.0, 0.0);
    return(1);
}


static int ProcessMailcapFile(char *file)
{
    struct MailcapEntry mc;
    FILE *fp;

#ifndef DISABLE_TRACE
    if (www2Trace)
      fprintf (stderr, "Loading types config file '%s'\n",
               file);
#endif

    fp = fopen(file, "r");

    while (fp && !feof(fp)) {
        ProcessMailcapEntry(fp, &mc);
    }
    if (fp) fclose(fp);
    return(-1);
}



int HTLoadTypesConfigFile (char *fn)
{
  return ProcessMailcapFile (fn);
}




/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

/*	Define a basic set of suffixes
**	------------------------------
**
**	The LAST suffix for a type is that used for temporary files
**	of that type.
**	The quality is an apriori bias as to whether the file should be
**	used.  Not that different suffixes can be used to represent files
**	which are of the same format but are originals or regenerated,
**	with different values.
*/

PUBLIC void HTFileInit NOARGS
{
  extern int use_default_extension_map;
  extern char *global_extension_map;
  extern char *personal_extension_map;

  if (use_default_extension_map)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "@@@ Using default extension map\n");
#endif

      HTSetSuffix(".uu",	"application/octet-stream", "binary", 1.0); /* xtra */
      HTSetSuffix(".saveme",	"application/octet-stream", "binary", 1.0); /* xtra */
      HTSetSuffix(".dump",	"application/octet-stream", "binary", 1.0); /* xtra */
      HTSetSuffix(".hqx",       "application/octet-stream", "binary", 1.0);
      HTSetSuffix(".arc",       "application/octet-stream", "binary", 1.0);
      HTSetSuffix(".o",         "application/octet-stream", "binary", 1.0);
      HTSetSuffix(".a",         "application/octet-stream", "binary", 1.0);
      HTSetSuffix(".bin",	"application/octet-stream", "binary", 1.0);
      HTSetSuffix(".exe",	"application/octet-stream", "binary", 1.0);
      /* Temporary only. */
      HTSetSuffix(".z",	        "application/octet-stream", "binary", 1.0);
      HTSetSuffix(".gz",	"application/octet-stream", "binary", 1.0);

      HTSetSuffix(".oda",       "application/oda", "binary", 1.0);

      HTSetSuffix(".pdf",	"application/pdf", "binary", 1.0);

      HTSetSuffix(".eps",	"application/postscript", "binary", 1.0);
      HTSetSuffix(".ai",	"application/postscript", "binary", 1.0);
      HTSetSuffix(".ps",	"application/postscript", "binary", 1.0);

      HTSetSuffix(".rtf",	"application/rtf", "binary", 1.0);

      HTSetSuffix(".dvi","application/x-dvi", "binary", 1.0);

      HTSetSuffix(".hdf","application/x-hdf", "binary", 1.0);

      HTSetSuffix(".latex", "application/x-latex", "binary", 1.0);

      HTSetSuffix(".cdf","application/x-netcdf", "binary", 1.0);
      HTSetSuffix(".nc","application/x-netcdf", "binary", 1.0);

      HTSetSuffix(".tex",  "application/x-tex", "binary", 1.0);

      HTSetSuffix(".texinfo",       "application/x-texinfo",    "binary", 1.0);
      HTSetSuffix(".texi",          "application/x-texinfo",    "binary", 1.0);

      HTSetSuffix(".t",    "application/x-troff", "binary", 1.0);
      HTSetSuffix(".tr",   "application/x-troff", "binary", 1.0);
      HTSetSuffix(".roff", "application/x-troff", "binary", 1.0);
      HTSetSuffix(".man",  "application/x-troff-man", "binary", 1.0);
      HTSetSuffix(".me",   "application/x-troff-me", "binary", 1.0);
      HTSetSuffix(".ms",   "application/x-troff-ms", "binary", 1.0);

      HTSetSuffix(".src",	"application/x-wais-source", "binary", 1.0);
      HTSetSuffix(".wsrc",	"application/x-wais-source", "binary", 1.0); /* xtra */

      HTSetSuffix(".zip", "application/zip", "binary", 1.0);

      HTSetSuffix(".bcpio",   "application/x-bcpio", "binary", 1.0);
      HTSetSuffix(".cpio",    "application/x-cpio", "binary", 1.0);
      HTSetSuffix(".gtar",    "application/x-gtar", "binary", 1.0);
      HTSetSuffix(".shar",    "application/x-shar", "binary", 1.0);
      HTSetSuffix(".sh",      "application/x-shar", "binary", 1.0); /* xtra */
      HTSetSuffix(".sv4cpio", "application/x-sv4cpio", "binary", 1.0);
      HTSetSuffix(".sv4crc",  "application/x-sv4crc", "binary", 1.0);
      HTSetSuffix(".tar",     "application/x-tar", "binary", 1.0);
      HTSetSuffix(".ustar",   "application/x-ustar", "binary", 1.0);

      HTSetSuffix(".snd",  "audio/basic", "binary", 1.0);
      HTSetSuffix(".au",   "audio/basic", "binary", 1.0);
      HTSetSuffix(".aifc", "audio/x-aiff", "binary", 1.0);
      HTSetSuffix(".aif",  "audio/x-aiff", "binary", 1.0);
      HTSetSuffix(".aiff", "audio/x-aiff", "binary", 1.0);
      HTSetSuffix(".wav",  "audio/x-wav", "binary", 1.0);

      HTSetSuffix(".gif", "image/gif", "binary", 1.0);

      HTSetSuffix(".png", "image/png", "binary", 1.0);

      HTSetSuffix(".ief", "image/ief", "binary", 1.0);

      HTSetSuffix(".jfif","image/jpeg", "binary", 1.0); /* xtra */
      HTSetSuffix(".jfif-tbnl","image/jpeg", "binary", 1.0); /* xtra */
      HTSetSuffix(".jpe", "image/jpeg", "binary", 1.0);
      HTSetSuffix(".jpg", "image/jpeg", "binary", 1.0);
      HTSetSuffix(".jpeg","image/jpeg", "binary", 1.0);

      HTSetSuffix(".tif", "image/tiff", "binary", 1.0);
      HTSetSuffix(".tiff","image/tiff", "binary", 1.0);

      HTSetSuffix(".ras", "image/x-cmu-rast", "binary", 1.0);
      HTSetSuffix(".pnm", "image/x-portable-anymap", "binary", 1.0);
      HTSetSuffix(".pbm", "image/x-portable-bitmap", "binary", 1.0);
      HTSetSuffix(".pgm", "image/x-portable-graymap", "binary", 1.0);
      HTSetSuffix(".ppm", "image/x-portable-pixmap", "binary", 1.0);
      HTSetSuffix(".rgb", "image/x-rgb", "binary", 1.0);
      HTSetSuffix(".xbm", "image/x-xbitmap", "binary", 1.0);
      HTSetSuffix(".xpm", "image/x-xpixmap", "binary", 1.0);
      HTSetSuffix(".xwd", "image/x-xwindowdump", "binary", 1.0);

      HTSetSuffix(".htm",   "text/html", "binary", 1.0);
      HTSetSuffix(".html",  "text/html", "binary", 1.0);

      HTSetSuffix(".text",     "text/plain", "binary", 1.0);
      HTSetSuffix(".c",	       "text/plain", "binary", 1.0);
      HTSetSuffix(".cc",       "text/plain", "binary", 1.0);
      HTSetSuffix(".c++",      "text/plain", "binary", 1.0);
      HTSetSuffix(".h",	       "text/plain", "binary", 1.0);
      HTSetSuffix(".pl",       "text/plain", "binary", 1.0);
      HTSetSuffix(".txt",      "text/plain", "binary", 1.0);

      HTSetSuffix(".rtx", "text/richtext", "binary", 1.0); /* MIME richtext */
      HTSetSuffix(".tsv", "text/tab-separated-values", "binary", 1.0);
      HTSetSuffix(".etx", "text/x-setext", "binary", 1.0);

      HTSetSuffix(".mpg",  "video/mpeg", "binary", 1.0);
      HTSetSuffix(".mpe",  "video/mpeg", "binary", 1.0);
      HTSetSuffix(".mpeg", "video/mpeg", "binary", 1.0);

      HTSetSuffix(".mov", "video/quicktime", "binary", 1.0);
      HTSetSuffix(".qt",  "video/quicktime", "binary", 1.0);

      HTSetSuffix(".avi", "video/x-msvideo", "binary", 1.0);

      HTSetSuffix(".movie", "video/x-sgi-movie", "binary", 1.0);
      HTSetSuffix(".mv",    "video/x-sgi-movie", "binary", 1.0);

      HTSetSuffix(".mime", "message/rfc822", "binary", 1.0);
    }

  /* These should override the default extensions as necessary. */
  HTLoadExtensionsConfigFile (global_extension_map);

  /* These should override everything else. */
  HTLoadExtensionsConfigFile (personal_extension_map);
}



/* -------------------- Extension config file reading --------------------- */

/* The following is lifted from NCSA httpd 1.0a1, by Rob McCool;
   NCSA httpd is in the public domain, as is this code. */

#define MAX_STRING_LEN 256

/*static int getline(char *s, int n, FILE *f) */
/*{*/
/*register int i=0;*/

/*while(1) */
/*{*/
/*s[i] = (char)fgetc(f);*/

/*if(s[i] == CR)*/
/*s[i] = fgetc(f);*/

/*if((s[i] == EOF) || (s[i] == LF) || (i == (n-1)))*/
/*{*/
/*s[i] = '\0';*/
/*return (feof(f) ? 1 : 0);*/
/*}*/
/*++i;*/
/*}*/

/**//* NOTREACHED */
/*}*/

static void getword(char *word, char *line, char stop, char stop2)
{
  int x = 0, y;

  for (x = 0; line[x] && line[x] != stop && line[x] != stop2; x++)
    {
      word[x] = line[x];
    }

  word[x] = '\0';
  if (line[x])
    ++x;
  y=0;

  while (line[y++] = line[x++])
    ;

  return;
}

int HTLoadExtensionsConfigFile (char *fn)
{
  char l[MAX_STRING_LEN],w[MAX_STRING_LEN],*ct,*ptr;
  size_t len = MAX_STRING_LEN;
  FILE *f;
  int x, count = 0;

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "Loading extensions config file '%s'\n",
             fn);
#endif

  if(!(f = fopen(fn,"r")))
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "Could not open extensions config file '%s'\n",fn);
#endif

      return -1;
    }

  while(!(getline((char**)&l,&len,f)))
    {
      /* always get rid of leading white space for "line" -- SWP */
      for (ptr=l; *ptr && isspace(*ptr); ptr++);

      getword(w,ptr,' ','\t');
      if(ptr[0] == '\0' || w[0] == '#')
        continue;
      ct = (char *)malloc(sizeof(char) * (strlen(w) + 1));
      strcpy(ct,w);

      while(ptr[0])
        {
          getword(w,ptr,' ','\t');
          if(w[0] && (w[0] != ' '))
            {
              char *ext = (char *)malloc(sizeof(char) * (strlen(w)+1+1));

              for(x=0; w[x]; x++)
                ext[x+1] = TOLOWER(w[x]);
              ext[0] = '.';
              ext[strlen(w)+1] = 0;

#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf (stderr, "SETTING SUFFIX '%s' to '%s'\n", ext, ct);
#endif

              HTSetSuffix (ext, ct, "binary", 1.0);
              count++;

              free (ext);
            }
        }
      free(ct);
    }

  fclose(f);

  return count;
}
