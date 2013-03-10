/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright (C) 1993, Board of Trustees of the University of Illinois      *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/
#include "../config.h"
#include "mosaic.h"
#include "comment.h"
#include "mo-www.h"
#include "gui-dialogs.h"
#include "gui.h"
#include <ctype.h>
#include <stdio.h>

/*for memcpy*/
#include <memory.h>

extern mo_window *current_win;
extern Pixmap *IconPixSmall;

/*SWP*/
Pixmap *tmp_pix=NULL;
// SAM EASTER_EXTERNS
extern int cci_docommand;
#define __MAX_HOME_LEN__ 256
#define __SRC__
#include "../libwww2/HTAAUtil.h"
extern int securityType;

extern XtAppContext app_context;
extern mo_window *current_win;

extern int is_uncompressed;

extern Pixmap *IconPixTom,*IconPixBig,*IconPix;
extern int logo_count,logo_save;

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

/*SWP -- 9.21.95 -- Binary save filename*/
char *saveFileName=NULL;

/* Grumble grumble... */
#if defined(__sgi) && !defined(__STDC__)
#define __STDC__
#endif

/* libwww includes */
#include "HTUtils.h"
#include "HTString.h"
#include "tcp.h"
#include "HTTCP.h"
#include "HTParse.h"
#include "HTAccess.h"
#include "HTML.h"
#include "HText.h"
#include "HTList.h"
#include "HTInit.h"
#include "libnut/system.h"
#include "libhtmlw/HTML.h"

#define MO_BUFFER_SIZE 8192

/* Bare minimum. */
struct _HText {
  char *expandedAddress;
  char *simpleAddress;

  /* This is what we should parse and display; it is *not*
     safe to free. */
  char *htmlSrc;
  /* This is what we should free. */
  char *htmlSrcHead;
  int srcalloc;    /* amount of space allocated */
  int srclen;      /* amount of space used */
};

/* Mosaic does NOT use either the anchor system or the style sheet
   system of libwww. */

HText* HTMainText = 0;                  /* Equivalent of main window */

/* these are used in libwww */
char *HTAppName = "NCSA_Mosaic";
char *HTAppVersion;  /* now set this in gui.c -- mo_do_gui() */
extern char *HTLibraryVersion;

int force_dump_to_file = 0;             /* hook to force dumping binary data
                                           straight to file named by... */
char *force_dump_filename = 0;          /* this filename. */

/* From gui-documents.c */
extern int interrupted;

/* From HTTP.c */
extern int do_post;
extern char *post_content_type;
extern char *post_data;
extern int do_put;
extern int put_file_size;
extern FILE *put_fp;

/* From HTMIME.c - AF */
extern char *HTTP_last_modified;

/* From cciBindings.c */
extern int cci_get;

extern char *HTTP_expires;


/* SWP -- Agent Spoofing Public Variables */
#define MAX_AGENTS 51
int numAgents;
char **agent;
int selectedAgent=0;


#define FRAME_CHECK_SIZE 2048

/* Basically we show the urls that appear within the frameset tag
   as urls and add some text explaining that these are the urls they
   were supposed to see as frames. We also show the NOFRAMES stuff. */
static void frame_hack()
{
  extern char *ParseMarkTag();
  char *start, *new_src, *place,*tmp, *url, *frame_anchors[25], *new_html;
  int num_frames=0, new_size=0, i;
  char *ptr;

  start=NULL;
  for (i=0,ptr=HTMainText->htmlSrc; ptr && i<FRAME_CHECK_SIZE; ptr++,i++) {
	if (*ptr=='<' && (*(ptr+1)=='f' || *(ptr+1)=='F')) {
		if (!my_strncasecmp("rameset",ptr+2,7)) {
			start=ptr;
			break;
		}
	}
  }
/*
  start = strstr(HTMainText->htmlSrc, "<frameset");

  if(!start)
    start = strstr(HTMainText->htmlSrc, "<FRAMESET");
*/

  if(!start)
    return;

  place = start;

  while((tmp=strstr(place, "<frame ")) || (tmp=strstr(place, "<FRAME ")))
    {
      url = ParseMarkTag(tmp, "FRAME", "SRC");
      if (url) {
	frame_anchors[num_frames] = malloc(strlen(url)*2 +
					 strlen("<LI> <A HREF=  > </A> ")+4);
	sprintf(frame_anchors[num_frames], "<LI> <A HREF=\"%s\"> %s </A>", url, url);
	new_size += strlen(frame_anchors[num_frames])+1;
	num_frames++;
      }
      place = tmp+6;
    }

  new_src = malloc(new_size+strlen(HTMainText->htmlSrc) + strlen(" <HR> ") +
		   strlen("<H2> Frame Index: </H2> <UL> </UL>") +6);

  /* copy everything up to first frameset tag to new_src */
  strncpy(new_src, HTMainText->htmlSrc, strlen(HTMainText->htmlSrc) -
	  strlen(start));

  new_src[strlen(HTMainText->htmlSrc) - strlen(start)]='\0';

  sprintf(new_src, "%s <H2> Frame Index: </H2> <UL>", new_src);

  for(i=0;i<num_frames;i++)
    {
      sprintf(new_src, "%s%s", new_src, frame_anchors[i]);
      free(frame_anchors[i]);
    }

  /* end list */
  sprintf(new_src, "%s </UL> <HR>", new_src);

  /* add in rest of document */
  strcat(new_src, start);

  free(HTMainText->htmlSrc);
  HTMainText->htmlSrc = new_src;
}

/****************************************************************************
 * name:    hack_htmlsrc (PRIVATE)
 * purpose: Do really nasty things to a stream of HTML that just got
 *          pulled over from a server.
 * inputs:
 *   - none (global HTMainText is assumed to contain current
 *           HText object)
 * returns:
 *   - HTMainText->htmlSrc (char *)
 * remarks:
 *   This is ugly but it gets the job done.
 *   The job is this:
 *     - Filter out repeated instances of <PLAINTEXT>.
 *     - Discover if <PLAINTEXT> has been improperly applied
 *       to the head of an HTML document (we discover HTML documents
 *       by checking to see if a <TITLE> element is on the next line).
 *     - Same as above but for <HEAD> and <HTML>.
 *   We advance the character pointer HTMainText->htmlSrc by the
 *   appropriate remark to make adjustments, and keep the original
 *   head of the allocated block of text in HTMainText->htmlSrcHead.
 ****************************************************************************/
static char *hack_htmlsrc (void)
{
  if (!HTMainText)
    return NULL;

  if (!HTMainText->htmlSrc)
    {
      HTMainText->htmlSrcHead = NULL;
      return NULL;
    }

  if(get_pref_boolean(eFRAME_HACK))
    frame_hack();


  /* Keep pointer to real head of htmlSrc memory block. */
  HTMainText->htmlSrcHead = HTMainText->htmlSrc;

  if (HTMainText->htmlSrc && HTMainText->srclen > 30)
    {
      if (!strncmp (HTMainText->htmlSrc, "<plaintext>", 11) ||
          !strncmp (HTMainText->htmlSrc, "<PLAINTEXT>", 11))
        {
          if (!strncmp (HTMainText->htmlSrc + 11, "<plaintext>", 11) ||
              !strncmp (HTMainText->htmlSrc + 11, "<PLAINTEXT>", 11))
            {
              HTMainText->htmlSrc += 11;
            }
          else if (!strncmp (HTMainText->htmlSrc + 11, "\n<plaintext>", 12) ||
                   !strncmp (HTMainText->htmlSrc + 11, "\n<PLAINTEXT>", 12))
            {
              HTMainText->htmlSrc += 12;
            }
          else if (!strncmp (HTMainText->htmlSrc + 11, "\n<title>", 8) ||
                   !strncmp (HTMainText->htmlSrc + 11, "\n<TITLE>", 8))
            {
              HTMainText->htmlSrc += 12;
            }
          else if (!strncmp (HTMainText->htmlSrc + 11, "\n<HEAD>", 7) ||
                   !strncmp (HTMainText->htmlSrc + 11, "\n<head>", 7) ||
                   !strncmp (HTMainText->htmlSrc + 11, "\n<HTML>", 7) ||
                   !strncmp (HTMainText->htmlSrc + 11, "\n<html>", 7) ||
                   !strncmp (HTMainText->htmlSrc + 11, "\n<BASE",  6) ||
                   !strncmp (HTMainText->htmlSrc + 11, "\n<base",  6))
            {
              HTMainText->htmlSrc += 12;
            }
        }
      if (!strncmp (HTMainText->htmlSrc,
                    "<TITLE>Document</TITLE>\n<PLAINTEXT>", 35))
        {
          if (!strncmp (HTMainText->htmlSrc + 35, "\n<title>", 8) ||
              !strncmp (HTMainText->htmlSrc + 35, "\n<TITLE>", 8))
            {
              HTMainText->htmlSrc += 36;
            }
        }
    }
  return HTMainText->htmlSrc;
}


/****************************************************************************
 * name:    doit (PRIVATE)
 * purpose: Given a URL, go fetch information.
 * inputs:
 *   - char       *url: The URL to fetch.
 *   - char **texthead: Return pointer for the head of the allocated
 *                      text block (which may be different than the
 *                      return text intended for display).
 * returns:
 *   The text intended for display (char *).
 ****************************************************************************/
static char *doit (char *url, char **texthead)
{
  char *msg;
  int rv;
  extern char *use_this_url_instead;
  mo_window *win=current_win;

  /* Hmmmmmmm... */
  if (HTMainText)
    {
      free (HTMainText);
      HTMainText = NULL;
    }

  /* Is it April? */
  if (IconPix!=IconPixSmall) { /* We aren't small... */
	if (strstr(url,"~mag/")) {
		int tmp=21;

		tmp_pix=IconPix;
		IconPix = IconPixTom;
		logo_count = 0;
		set_pref(ePIX_COUNT, (void *)&tmp);
		XmxApplyPixmapToLabelWidget(win->logo, IconPix[0]);
	}
// SAM	EASTER
  }

  XmxApplyPixmapToLabelWidget(win->logo, IconPix[0]);

  is_uncompressed=0;

  rv = HTLoadAbsolute (url);

  if (rv == 1)
    {
      char *txt = hack_htmlsrc ();
      if (HTMainText)
        *texthead = HTMainText->htmlSrcHead;
      else
        *texthead = NULL;
      return txt;
    }
  else if (rv == -1)
    {
      interrupted = 1;
      *texthead = NULL;
      return NULL;
    }

   /*
   ** Just because we errored out, doesn't mean there isn't markup to
   ** look at.  For example, an FTP site that doesn't let a user in because
   ** the maximum number of users has been reached often has a message
   ** telling about other mirror sites.  The failed FTP connection returns
   ** a message that is taken care of below.
   */
   if (HTMainText) {
	char *txt = hack_htmlsrc();
	*texthead = HTMainText->htmlSrcHead;

	if (cci_get){
		if (txt)
			return txt;
		else
			/* take care of failed local access */
			txt = strdup("<H1>ERROR</H1>");
	}
	return txt;

   }

  /* Return proper error message if we experienced redirection. */
  if (use_this_url_instead)
    url = use_this_url_instead;
  msg = (char *)malloc ((strlen (url) + 200) * sizeof (char));
  sprintf (msg, "<H1>ERROR</H1> Requested document (URL %s) could not be accessed.<p>The information server either is not accessible or is refusing to serve the document to you.<p>", url);
  *texthead = msg;
  securityType=HTAA_UNKNOWN;
  return msg;
}


/****************************************************************************
 * name:    mo_pull_er_over
 * purpose: Given a URL, pull 'er over.
 * inputs:
 *   - char       *url: The URL to pull over.
 *   - char **texthead: Return pointer to head of allocated block.
 * returns:
 *   Text to display (char *).
 * remarks:
 *
 ****************************************************************************/
char *mo_pull_er_over (char *url, char **texthead)
{
  char *rv;
  extern int binary_transfer;

  if (binary_transfer)
    {
      force_dump_to_file = 1;
      force_dump_filename = mo_tmpnam(url);
    }

  if (saveFileName!=NULL) {
	free(saveFileName);
  }
  saveFileName=strdup(url);

  if (HTTP_last_modified)
  {
      free(HTTP_last_modified);
      HTTP_last_modified = 0;
  }
  rv = doit (url, texthead);
  if (binary_transfer)
    {
      force_dump_to_file = 0;
      force_dump_filename = NULL;
    }

  return rv;
}


char *mo_post_pull_er_over (char *url, char *content_type, char *data,
                            char **texthead)
{
  char *rv;
  extern int binary_transfer;

  do_post = 1;
  post_content_type = content_type;
  post_data = data;

  if (binary_transfer)
    {
      force_dump_to_file = 1;
      force_dump_filename = mo_tmpnam(url);
    }
  if (HTTP_last_modified)
  {
      free(HTTP_last_modified);
      HTTP_last_modified = 0;
  }
  rv = doit (url, texthead);
  if (binary_transfer)
    {
      force_dump_to_file = 0;
      force_dump_filename = NULL;
    }

  do_post = 0;

  return rv;
}



/****************************************************************************
 * name:    mo_pull_er_over_virgin
 * purpose: Given a URL, pull 'er over in such a way that no format
 *          handling takes place and the data gets dumped in the filename
 *          of the calling routine's choice.
 * inputs:
 *   - char  *url: The URL to pull over.
 *   - char *fnam: Filename in which to dump the received data.
 * returns:
 *   mo_succeed on success; mo_fail otherwise.
 * remarks:
 *   This routine is called when we know there's data out there we
 *   want to get and we know we just want it dumped in a file, no
 *   questions asked, by the WWW library.  Appropriate global flags
 *   are set to make this happen.
 *   This must be made cleaner.
 ****************************************************************************/
mo_status mo_pull_er_over_virgin (char *url, char *fnam)
{
  int rv;

  /* Force dump to file. */
  force_dump_to_file = 1;
  force_dump_filename = fnam;
  if (saveFileName!=NULL) {
	free(saveFileName);
  }
  saveFileName=strdup(url);

  is_uncompressed=0;

  rv = HTLoadAbsolute (url);

  if (rv == 1)
    {
      force_dump_to_file = 0;
      return mo_succeed;
    }
  else if (rv == -1)
    {
      force_dump_to_file = 0;
      interrupted = 1;
      return mo_fail;
    }
  else
    {
      force_dump_to_file = 0;
      return mo_fail;
    }
}


mo_status mo_re_init_formats (void)
{
  HTReInit ();
  return mo_succeed;
}

/* ------------------------------------------------------------------------ */

HText *HText_new (void)
{
  HText *htObj = (HText *)malloc (sizeof (HText));

  htObj->expandedAddress = NULL;
  htObj->simpleAddress = NULL;
  htObj->htmlSrc = NULL;
  htObj->htmlSrcHead = NULL;
  htObj->srcalloc = 0;
  htObj->srclen = 0;

  /* Free the struct but not the text, as it will be handled
     by Mosaic proper -- apparently. */
  if (HTMainText)
    free (HTMainText);

  HTMainText = htObj;

  return htObj;
}

void HText_free (HText *self)
{
  if (self)
    {
      if (self->htmlSrcHead)
        free (self->htmlSrcHead);
      free (self);
    }
  return;
}

void HText_beginAppend (HText *text)
{
  HTMainText = text;
  return;
}

void HText_endAppend (HText *text)
{
  if (text)
    {
      HText_appendCharacter (text, '\0');
    }
  HTMainText = text;
  return;
}

void HText_doAbort (HText *self)
{
  /* Clean up -- we want to free htmlSrc here because htmlSrcHead
     doesn't get assigned until hack_htmlsrc, and by the time we
     reach that, this should never be called. */
  if (self)
    {
      if (self->htmlSrc)
        free (self->htmlSrc);
      self->htmlSrc = NULL;
      self->htmlSrcHead = NULL;
      self->srcalloc = 0;
      self->srclen = 0;
    }
  return;
}

void HText_clearOutForNewContents (HText *self)
{
  if (self)
    {
      if (self->htmlSrc)
        free (self->htmlSrc);
      self->htmlSrc = NULL;
      self->htmlSrcHead = NULL;
      self->srcalloc = 0;
      self->srclen = 0;
    }
  return;
}

static void new_chunk (HText *text)
{
  if (text->srcalloc == 0)
    {
      text->htmlSrc = (char *)malloc (MO_BUFFER_SIZE);
      text->htmlSrc[0] = '\0';
    }
  else
    {
      text->htmlSrc = (char *)realloc
        (text->htmlSrc, text->srcalloc + MO_BUFFER_SIZE);
    }

  text->srcalloc += MO_BUFFER_SIZE;

  return;
}

#if defined(__alpha) || defined(_IBMR2)
void HText_appendCharacter (text, ch)
HText *text;
char ch;
#else
void HText_appendCharacter (HText *text, char ch)
#endif
{
  if (text->srcalloc < text->srclen + 1)
    new_chunk (text);

  text->htmlSrc[text->srclen++] = ch;

  return;
}

void HText_appendText (HText *text, char *str)
{
  int len;

  if (!str)
    return;

  len = strlen (str);

  while (text->srcalloc < text->srclen + len + 1)
    new_chunk (text);

/*  bcopy (str, (text->htmlSrc + text->srclen), len);*/
  memcpy((text->htmlSrc + text->srclen), str, len);

  text->srclen += len;
  text->htmlSrc[text->srclen] = '\0';

  return;
}

void HText_appendBlock (HText *text, char *data, int len)
{
  if (!data)
    return;

  while (text->srcalloc < text->srclen + len + 1)
    new_chunk (text);

/*  bcopy (data, (text->htmlSrc + text->srclen), len);*/
  memcpy((text->htmlSrc + text->srclen), data, len);

  text->srclen += len;
  text->htmlSrc[text->srclen] = '\0';

  return;
}

void HText_appendParagraph (HText *text)
{
  /* Boy, talk about a misnamed function. */
  char *str = " <p> \n";

  HText_appendText (text, str);

  return;
}

void HText_beginAnchor (HText *text, char *anc)
{
  HText_appendText (text, "<A HREF=\"");
  HText_appendText (text, anc);
  HText_appendText (text, "\">");
  return;
}

void HText_endAnchor (HText * text)
{
  HText_appendText (text, "</A>");
  return;
}

void HText_dump (HText *me)
{
  return;
}

char *HText_getText (HText *me)
{
  if (me)
    return me->htmlSrc;
  else
    return NULL;
}

char **HText_getPtrToText (HText *me)
{
  if (me)
    return &(me->htmlSrc);
  else
    return NULL;
}

int HText_getTextLength (HText *me)
{
  if (me)
    return me->srclen;
  else
    return 0;
}

/*
BOOL HText_select (HText *text)
{
  return ;
}
*/


/****************************************************************************
 * name:    fileOrServer
 * purpose: Given a string, checks to see if it can stat it. If so, it is
 *   assumed the user expects to open the file, not a web site. If not, we
 *   assume it is supposed to be a server and prepend the default protocol.
 * inputs:
 *   - char    *url: URL to canonicalize.
 * returns:
 *   The canonical representation of the URL.
 * remarks:
 *   Written by spowers@ncsa.uiuc.edu
 ****************************************************************************/
char *fileOrServer(char *url) {

struct stat buf;
char *xurl;

	/*
	 * At this point we know the URL specified is of the form:
	 *   shire.ncsa.uiuc.edu[:PORT]/path/to/something
	 */

	if (!stat(url,&buf)) { /*its a file and we have access*/
		xurl=mo_url_canonicalize_local(url);
	}
	else if (!(get_pref_string(eDEFAULT_PROTOCOL)) ||
		 !*(get_pref_string(eDEFAULT_PROTOCOL))) {
		xurl=(char *)calloc(strlen(url)+15,sizeof(char));
		sprintf(xurl,"http://%s",url);
	}
	else {
		xurl=(char *)calloc(strlen(url)+strlen(get_pref_string(eDEFAULT_PROTOCOL))+10,sizeof(char));
		sprintf(xurl,"%s://%s",get_pref_string(eDEFAULT_PROTOCOL),url);
	}

	return(xurl);
}


/****************************************************************************
 * name:    mo_url_prepend_protocol
 * purpose: To prepend the proper protocol to the url if it is not present.
 * inputs:
 *   - char    *url: URL to canonicalize.
 * returns:
 *   The canonical representation of the URL.
 * remarks:
 *   Contributed by martin@gizmo.lut.ac.uk, modified by spowers@ncsa.uiuc.edu
 ****************************************************************************/
char *mo_url_prepend_protocol(char *url) {

char *xurl;

	if (!url || !*url) {
		return(NULL);
	}

	if (!get_pref_string(eEXPAND_URLS)) {
		if (!strchr(url,':')) { /*no colon found, treat as file*/
			xurl = mo_url_canonicalize_local (url);
		}
		else { /*its prolly a real url*/
			xurl=strdup(url);
		}
	}
	else if (strncasecmp(url,"mailto:",7) &&
		 strncasecmp(url,"news:",5) &&
		 strncasecmp(url,"about:",6) &&
		 !strstr(url,"://")) { /*no protocol specified, default*/
		if (get_pref_string(eEXPAND_URLS_WITH_NAME)) {
			if (!strncmp(url, "www.", 4)) {
				xurl = (char *)malloc(strlen(url) + (8 * sizeof(char)));
				sprintf(xurl, "http://%s", url);
			}
			else if (!strncmp(url, "ftp.", 4)) {
				xurl = (char *)malloc(strlen(url) + (7 * sizeof(char)));
				sprintf(xurl, "ftp://%s", url);
			}
			else if (!strncmp(url, "gopher.", 7)) {
				xurl = (char *)malloc(strlen(url) + (10 * sizeof(char)));
				sprintf(xurl, "gopher://%s", url);
			}
			else if (!strncmp(url, "wais.", 5)) {
				xurl = (char *)malloc(strlen(url) + (8 * sizeof(char)));
				sprintf(xurl, "wais://%s", url);
			}
			else {
				xurl=fileOrServer(url);
			}
		}
		else {
			xurl=fileOrServer(url);
		}
	}
	else { /*protocol was specified*/
		xurl=strdup(url);
	}

	return(xurl);
}


/****************************************************************************
 * name:    mo_url_canonicalize
 * purpose: Turn a URL into its canonical form, based on the previous
 *          URL in this context (if appropriate).
 *          INTERNAL ANCHORS ARE STRIPPED OFF.
 * inputs:
 *   - char    *url: URL to canonicalize.
 *   - char *oldurl: The previous URL in this context.
 * returns:
 *   The canonical representation of the URL.
 * remarks:
 *   All we do is call HTParse.
 ****************************************************************************/
char *mo_url_canonicalize (char *url, char *oldurl)
{
  /* We LOSE anchor information. */
  return HTParse (url, oldurl,
                  PARSE_ACCESS | PARSE_HOST | PARSE_PATH |
                  PARSE_PUNCTUATION);
}


/****************************************************************************
 * name:    mo_url_canonicalize_keep_anchor
 * purpose: Turn a URL into its canonical form, based on the previous
 *          URL in this context (if appropriate).
 *          INTERNAL ANCHORS ARE *NOT* STRIPPED OFF.
 * inputs:
 *   - char    *url: URL to canonicalize.
 *   - char *oldurl: The previous URL in this context.
 * returns:
 *   The canonical representation of the URL.
 * remarks:
 *   All we do is call HTParse.
 ****************************************************************************/
char *mo_url_canonicalize_keep_anchor (char *url, char *oldurl)
{
  char *rv;
  /* We KEEP anchor information already present in url,
     but NOT in oldurl. */
  oldurl = HTParse (oldurl, "", PARSE_ACCESS | PARSE_HOST | PARSE_PATH |
                    PARSE_PUNCTUATION);
  rv = HTParse (url, oldurl,
                PARSE_ACCESS | PARSE_HOST | PARSE_PATH |
                PARSE_PUNCTUATION | PARSE_ANCHOR);
  /* We made a new copy of oldurl, so free the new copy. */
  free (oldurl);
  return rv;
}


/****************************************************************************
 * name:    mo_url_to_unique_document
 * purpose: Given a URL that may or may not contain an internal anchor,
 *          return a form that corresponds to a unique document -- i.e.,
 *          a URL that has annotations different than all other
 *          URL's, etc.  Generally this will be the URL without the
 *          target anchor, except for automatically generated representations
 *          of internal parts of HDF files.
 * inputs:
 *   - char *url: The URL.
 * returns:
 *   URL corresponding to a unique document.
 * remarks:
 *
 ****************************************************************************/
char *mo_url_to_unique_document (char *url)
{
  char *target = mo_url_extract_anchor (url), *rv;

  if (target && !strncmp (target, "hdfref;", 7))
    rv = strdup (url);
  else
    rv = mo_url_canonicalize (url, "");

  if (target)
    free (target);

  return rv;
}


/****************************************************************************
 * name:    mo_url_extract_anchor
 * purpose: Given a URL (presumably in canonical form), extract
 *          the internal anchor, if any.
 * inputs:
 *   - char *url: URL to use.
 * returns:
 *   Internal anchor, if one exists in the URL; else NULL.
 * remarks:
 *
 ****************************************************************************/
char *mo_url_extract_anchor (char *url)
{
  return HTParse (url, "", PARSE_ANCHOR);
}


/****************************************************************************
 * name:    mo_url_extract_access
 * purpose: Given a URL (presumably in canonical form), extract
 *          the access method, if any.
 * inputs:
 *   - char *url: URL to use.
 * returns:
 *   Access method, if one exists in the URL; else NULL.
 * remarks:
 *
 ****************************************************************************/
char *mo_url_extract_access (char *url, char *oldurl)
{
  return HTParse (url, oldurl, PARSE_ACCESS);
}



char *mo_url_canonicalize_local (char *url)
{
  /* Convex OS apparently sucks. */
#ifdef CONVEX
  char blah[129];
  char *cwd = getcwd (blah, 128);
#else
  char *cwd = getcwd (NULL, 128);
#endif
  char *tmp;

  if (!url)
    return NULL;

  tmp = (char *)malloc ((strlen (url) +
                         strlen (cwd) + 32));
  if (url[0] == '/')
    sprintf (tmp, "file://localhost%s",
             url);
  else
    sprintf (tmp, "file://localhost%s/%s",
             cwd,
             url);

  /* Sigh... */
#ifndef CONVEX
  free (cwd);
#endif

  return tmp;
}


/****************************************************************************
 * name:    mo_tmpnam
 * purpose: Make a temporary, unique filename.
 * inputs:
 *   none
 * returns:
 *   The new temporary filename.
 * remarks:
 *   We call tmpnam() to get the actual filename, and use the value
 *   of Rdata.tmp_directory, if any, for the directory.
 * added code for url=NULL, bjs, 2/7/96
 ****************************************************************************/
#ifndef L_tmpnam
#define L_tmpnam 32
#endif
char *mo_tmpnam (char *url)
{
  extern void MoCCIAddFileURLToList(char *, char *);
  char *tmp = (char *)malloc (sizeof (char) * L_tmpnam);
  char *tmp_dir = get_pref_string(eTMP_DIRECTORY);

  tmpnam (tmp);

  if (!tmp_dir)
    {
      /* Fast path. */
      if(url) MoCCIAddFileURLToList(tmp,url);
      return tmp;
    }
  else
    {
      /* OK, reconstruct to go in the directory of our choice. */
      char *oldtmp = tmp;
      int i;

      /* Start at the back and work our way forward. */
      for (i = strlen(oldtmp)-1; i >= 0; i--)
        {
          if (oldtmp[i] == '/')
            goto found_it;
        }

      /* No luck, just punt. */
      if(url) MoCCIAddFileURLToList(tmp,url);
      return tmp;

    found_it:
      tmp = (char *)malloc (sizeof (char) * (strlen (tmp_dir) +
                                             strlen (&(oldtmp[i])) + 8));
      if (tmp_dir[strlen(tmp_dir)-1] == '/')
        {
          /* Trailing slash in tmp_directory spec. */
          sprintf (tmp, "%s%s", tmp_dir, &(oldtmp[i])+1);
        }
      else
        {
          /* No trailing slash. */
          sprintf (tmp, "%s%s", tmp_dir, &(oldtmp[i]));
        }

      MoCCIAddFileURLToList(tmp,url);
      free (oldtmp);
      return tmp;
    }
}


/* ------------------------------ dumb stuff ------------------------------ */

/* Grumble grumble... */
#if defined(ultrix) || defined(VMS) || defined(NeXT) || defined(M4310) || defined(vax)
char *strdup (char *str)
{
  char *dup;

  dup = (char *)malloc (strlen (str) + 1);
  dup = strcpy (dup, str);

  return dup;
}
#endif


/* Error from the library */
void application_error(char *str, char *title) {

  XmxMakeErrorDialogWait(current_win->base, app_context, str, title, "OK");

  return;
}


/* Feedback from the library. */
void application_user_feedback (char *str)
{
  extern Widget toplevel;
  XmxMakeInfoDialog (toplevel, str, "NCSA Mosaic: Application Feedback");
  XmxManageRemanage (Xmx_w);
}

void application_user_info_wait (char *str)
{

  XmxMakeInfoDialogWait(current_win->base, app_context, str, "NCSA Mosaic: Application Feedback", "OK");

}

char *prompt_for_string (char *questionstr)
{

  return XmxModalPromptForString (current_win->base, app_context,
                                  questionstr, "OK", "Cancel");
}

char *prompt_for_password (char *questionstr)
{

  return XmxModalPromptForPassword (current_win->base, app_context,
                                    questionstr, "OK", "Cancel");
}

int prompt_for_yes_or_no (char *questionstr)
{

  return XmxModalYesOrNo (current_win->base, app_context,
                          questionstr, "Yes", "No");
}

char *mo_get_html_return (char **texthead)
{
  char *txt = hack_htmlsrc ();
  *texthead = HTMainText->htmlSrcHead;
  return txt;
}


/* Simply loop through a string and convert all newlines to spaces. */
/* We now remove leading whitespace as well */
char *mo_convert_newlines_to_spaces (char *str)
{
  int i;
  char *tptr;

  if (!str)
    return NULL;

  for (i = 0; i < strlen (str); i++)
    if (str[i] == '\n')
      str[i] = ' ';

  tptr = str;
  while ((*tptr != '\0')&&(isspace((int)(*tptr))))
	tptr++;

  if (tptr != str)
  {
/*	bcopy(tptr, str, (strlen(tptr) + 1));*/
	memcpy(str, tptr, (strlen(tptr) + 1));
  }

  return str;
}

/* ---------------------------- escaping code ----------------------------- */

static unsigned char isAcceptable[96] =
/*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
{    0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,	/* 2x   !"#$%&'()*+,-./	 */
     1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,	/* 3x  0123456789:;<=>?	 */
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 4x  @ABCDEFGHIJKLMNO  */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,	/* 5x  PQRSTUVWXYZ[\]^_	 */
     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 6x  `abcdefghijklmno	 */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };	/* 7x  pqrstuvwxyz{\}~	DEL */

#define MO_HEX(i) (i < 10 ? '0'+i : 'A'+ i - 10)

/* The string returned from here, if any, can be free'd by caller. */
char *mo_escape_part (char *part)
{
  char *q/*, *u*/;
  char *p/*, *s, *e*/;		/* Pointers into keywords */
  char *escaped;

  if (!part)
    return NULL;

  escaped = (char *)malloc (strlen (part) * 3 + 1);

  for (q = escaped, p = part; *p != '\0'; p++)
    {
      /*
       * This makes sure that values 128 and over don't get
       * converted to negative values.
       */
      int c = (int)((unsigned char)(*p));
      if (*p == ' ')
        {
          *q++ = '+';
        }
      else if (c >= 32 && c <= 127 && isAcceptable[c-32])
        {
          *q++ = *p;
        }
      else
        {
          *q++ = '%';
          *q++ = MO_HEX(c / 16);
          *q++ = MO_HEX(c % 16);
        }
    }

  *q=0;

  return escaped;
}


static char mo_from_hex (char c)
{
  return ((c >= '0' && c <= '9') ? (c - '0') :
          ((c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
           (c - 'a' + 10)));
}

char *mo_unescape_part (char *str)
{
  char *p = str, *q = str;

  while (*p)
    {
      /* Plus's turn back into spaces. */
      if (*p == '+')
        {
          *q++ = ' ';
          p++;
        }
      else if (*p == '%')
        {
          p++;
          if (*p)
            *q = mo_from_hex(*p++) * 16;
          if (*p)
            *q += mo_from_hex(*p++);
          q++;
	}
      else
        {
          *q++ = *p++;
	}
    }

  *q++ = 0;
  return str;
}


/* ---------------------------- Agent Spoofing ---------------------------- */

/*
 * Agent Spoofing is simple. NCSA's real agent is always a member of the
 *   menu. Any more than that, you can add to the file in your home directory
 *   called ".mosaic-spoof-agents".
 */

void readAgents(void) {

FILE *fp;
char fname[BUFSIZ],buf[512];
char *homedir,*ptr;

	if (get_home(&homedir)!=0 || !homedir) {
		fprintf(stderr,"home: Could not get your home directory.\n");
		return;
	}
	sprintf(fname,"%s/.mosaic/spoof-agents",homedir); // SAM
	free(homedir);

	if (!(fp=fopen(fname,"r"))) {
		return;
	}

	while (!feof(fp)) {
		fgets(buf,511,fp);
		if (feof(fp)) {
			break;
		}
		if (*buf && *buf!='#') {
			buf[strlen(buf)-1]='\0';
			for (ptr=buf; *ptr && isspace(*ptr); ptr++);
			if (*ptr=='+') { /* This is to be the default*/
				if (*(ptr+1)) {
					agent[numAgents]=strdup(ptr+1);
					selectedAgent=numAgents;
				}
				else {
					continue;
				}
			}
			else if (*ptr) {
				agent[numAgents]=strdup(ptr);
			}
			else {
				continue;
			}
			numAgents++;
			if (numAgents==MAX_AGENTS) { /* limit reached */
				fprintf(stderr,"WARNING: Hard limit reached for agent spoof file.\n");
				break;
			}
		}
	}

	fclose(fp);

	return;
}


void loadAgents(void) {

char buf[512];

	agent=(char **)calloc(MAX_AGENTS+1,sizeof(char *));
	sprintf(buf,"%s/%s  libwww/%s",
		HTAppName ? HTAppName : "unknown",
		HTAppVersion ? HTAppVersion : "0.0",
		HTLibraryVersion);
	agent[0]=strdup(buf);
	numAgents=1;

	readAgents();

	return;
}


#ifdef TIMING
/* ----------------------------- Timing stuff ----------------------------- */

#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>

/* Time globals. */
static struct tms tbuf;
static int gtime;

void StartClock (void)
{
  gtime = times (&tbuf);

  return;
}

void StopClock ()
{
  int donetime;

  donetime = times(&tbuf);

#ifndef DISABLE_TRACE
  if (srcTrace) {
	fprintf (stderr, "Elapsed time %d\n", donetime - gtime);
  }
#endif

  return;
}
#endif


/* Originally in whine.c and then in techsupport.c...now it's here. - SWP */
/* ------------------------------------------------------------------------ */

static FILE *_fp = NULL;

FILE *mo_start_sending_mail_message (char *to, char *subj,
                                     char *content_type, char *url)
{
  char cmd[2048];
  char *tmp;

  if (!to)
    return NULL;

  if(!strcmp(content_type,"url_only")){
      content_type = "text/plain";
  }
#ifdef OLD
  if (get_pref_string(eMAIL_FILTER_COMMAND))
    {
      sprintf (cmd, "%s | %s -t", get_pref_string(eMAIL_FILTER_COMMAND),
               get_pref_string(eSENDMAIL_COMMAND));
    }
  else
    {
      sprintf (cmd, "%s -t", get_pref_string(eSENDMAIL_COMMAND));
    }
#else
  /* Try listing address on command line. */
  for (tmp = to; *tmp; tmp++)
    if (*tmp == ',')
      *tmp = ' ';

  if (get_pref_string(eMAIL_FILTER_COMMAND) && content_type &&
      strcmp (content_type, "application/postscript"))
    {
      sprintf (cmd, "%s | %s %s", get_pref_string(eMAIL_FILTER_COMMAND),
               get_pref_string(eSENDMAIL_COMMAND), to);
    }
  else
    {
      sprintf (cmd, "%s %s", get_pref_string(eSENDMAIL_COMMAND), to);
    }
#endif

  if ((_fp = popen (cmd, "w")) == NULL)
    return NULL;


  fprintf (_fp, "To: %s\n", to);

  fprintf (_fp, "Subject: %s\n", subj);
  fprintf (_fp, "Reply-To: %s <%s>\n",get_pref_string(eDEFAULT_AUTHOR_NAME),get_pref_string(eDEFAULT_AUTHOR_EMAIL));
  fprintf (_fp, "Content-Type: %s\n", content_type);
  fprintf (_fp, "Mime-Version: 1.0\n");
  fprintf (_fp, "X-Mailer: NCSA Mosaic %s on %s\n",
           MO_VERSION_STRING, MO_MACHINE_TYPE);
  if (url)
    fprintf (_fp, "X-URL: %s\n", url);

  fprintf (_fp, "\n");

  /* Stick in BASE tag as appropriate. */
  if (url && content_type &&
      strcmp (content_type, "text/x-html") == 0)
    fprintf (_fp, "<base href=\"%s\">\n", url);

  return _fp;
}

mo_status mo_finish_sending_mail_message (void)
{
  if (_fp)
    pclose (_fp);

  _fp = NULL;

  return mo_succeed;
}

/* ------------------------------------------------------------------------ */

mo_status mo_send_mail_message (char *text, char *to, char *subj,
                                char *content_type, char *url)
{
  FILE *fp;

  fp = mo_start_sending_mail_message (to, subj, content_type, url);
  if (!fp)
    return mo_fail;

  if(!strcmp(content_type,"url_only")){
      fputs(url,fp);
      fputs("\n\n",fp);
  } else {
      fputs (text, fp);
  }

  mo_finish_sending_mail_message ();

  return mo_succeed;
}


/* ------------------------- upload stuff -------------------------- */
char *mo_put_er_over(char *url,char **texthead) {

char *rv;

	do_put=do_post=1;

	if (saveFileName!=NULL) {
		free(saveFileName);
	}
	saveFileName=strdup(url);
	if (HTTP_last_modified) {
		free(HTTP_last_modified);
		HTTP_last_modified=0;
	}

	rv=doit(url,texthead);

	do_put=do_post=0;

	return(rv);
}


int upload(mo_window *win,FILE *fp, char *fname) {

char *put_url,*xurl;
int res=mo_fail;
char *newtext=NULL,*newtexthead=NULL;
char *last_modified=NULL,*expires=NULL;
char *ref;

	if (!win) {
		return(0);
	}

	put_url=prompt_for_string("Enter the URL you wish to upload the file as:");
	if (!put_url) {
		return(0);
	}

/*
	if (win->current_node &&
	    win->current_node->url &&
	    *(win->current_node->url)) {
		ref=strdup(win->current_node->url);
	}
	else {
*/
		ref=NULL;
/*
	}
*/

	xurl=mo_url_prepend_protocol(put_url);
	free(put_url);
	put_url=xurl;

	fseek(fp,0,SEEK_END);
	put_file_size=ftell(fp);
	rewind(fp);
	put_fp=fp;

	mo_busy ();

	if (win->target_anchor) {
		free(win->target_anchor);
	}
/*
	win->target_anchor=mo_url_extract_anchor(put_url);
*/
	win->target_anchor=NULL;

	mo_set_current_cached_win(win);

	newtext=mo_put_er_over(put_url,
			       &newtexthead);

	if (newtext) {
		if ((!strncmp(newtext,"<H1>ERROR<H1>",10)) ||
		    (!strncmp(newtext,"<HEAD><TITLE>404 Not Found</TITLE></HEAD>",28))) {
			res=mo_fail;
		}
	}

	if (HTTP_last_modified) {
		last_modified=strdup(HTTP_last_modified);
	}
	if (HTTP_expires) {
		expires=strdup(HTTP_expires);
	}

	{
		/* Check use_this_url_instead from HTAccess.c. */
		extern char *use_this_url_instead;

		if (use_this_url_instead) {
			mo_here_we_are_son(put_url);
			free(put_url);
			put_url=use_this_url_instead;

			/* Go get another target_anchor. */
			if (win->target_anchor) {
				free(win->target_anchor);
			}
/*
			win->target_anchor=mo_url_extract_anchor(put_url);
*/
			win->target_anchor=NULL;
		}
	}

	if (newtext) {
		res=mo_do_window_text(win,
				      put_url,
				      newtext,
				      newtexthead,
				      1,
				      ref,
				      last_modified,
				      expires);
		HTMLTraverseTabGroups(win->view,
				      XmTRAVERSE_HOME);
	}

	if (win->current_node) {
		mo_gui_check_security_icon(win->current_node->authType);
	}

	if (last_modified) {
		free(last_modified);
		last_modified=NULL;
	}
	if (expires) {
		free(expires);
		expires=NULL;
	}

/*
	if (xurl==put_url) {
		if (put_url) {
			free(put_url);
			put_url=NULL;
		}
	}
	else {
		if (xurl) {
			free(xurl);
			xurl=NULL;
		}
		if (put_url) {
			free(put_url);
			put_url=NULL;
		}
	}
*/

	if (ref) {
		free(ref);
		ref=NULL;
	}

	mo_gui_done_with_icon ();
	mo_not_busy ();

	return(res);
}


mo_status mo_upload_window(mo_window *win, char *fname) {

char *efname = (char *)malloc (sizeof (char) * (__MAX_HOME_LEN__ * 2));
FILE *fp;
int res;

	if (pathEval(efname, fname)<0) {
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr,"Error in evaluating the path. (gui-dialogs.c)\n");
		}
#endif
	}


	if (!(fp=fopen(efname,"r"))) {

		char *buf, *final, tmpbuf[80];
		int final_len;

		/* don't display dialog if command issued by cci application */
		if (cci_docommand) {
			return mo_fail;
		}

		buf=my_strerror(errno);
		if (!buf || !*buf || !strcmp(buf,"Error 0")) {
			sprintf(tmpbuf,"Uknown Error" );
			buf=tmpbuf;
		}

		final_len=30+((!efname || !*efname?3:strlen(efname))+13)+
			15+(strlen(buf)+3);
		final=(char *)calloc(final_len,sizeof(char));

		sprintf(final,"\nUnable to upload document:\n   %s\n\nUpload Error:\n   %s\n" ,(!efname || !*efname?" ":efname),buf);

		application_error(final,"Upload Error");

		if (final) {
			free(final);
			final=NULL;
		}

		free(efname);

		return(mo_fail);
	}

	res=upload(win,fp,efname);
	fclose (fp);
	free(efname);

	return(mo_succeed);
}


static XmxCallback (upload_win_cb) {

char *fname = (char *)malloc (sizeof (char) * 128);
mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid ((int)client_data));

	mo_busy();

	XtUnmanageChild(win->upload_win);

	XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *)call_data)->value,
			XmSTRING_DEFAULT_CHARSET,
			&fname);

	mo_upload_window(win,fname);

	mo_not_busy();

	free(fname);

	return;
}


mo_status mo_post_upload_window (mo_window *win) {

	XmxSetUniqid (win->id);

	if (!win->upload_win) {

		Widget frame, workarea, format_label;

		win->upload_win=
			XmxMakeFileSBDialog(win->base,
					    "NCSA Mosaic: Upload Document",
					    "Name of document to upload:",
					    upload_win_cb,
					    0);

		/* This makes a frame as a work area for the dialog box. */
		XmxSetArg(XmNmarginWidth,5);
		XmxSetArg(XmNmarginHeight,5);
		frame=XmxMakeFrame(win->upload_win,
				   XmxShadowEtchedIn);
		workarea=XmxMakeForm(frame);

	}
	else {
		XmFileSelectionDoSearch(win->upload_win,
					NULL);
	}

	XmxManageRemanage (win->upload_win);

	return mo_succeed;
}

