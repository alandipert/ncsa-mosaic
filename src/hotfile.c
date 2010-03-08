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
#include "hotlist.h"
#include "mo-www.h"
#include "libhtmlw/HTML.h"

/* The new file format provides support for nested hotlists of interesting
   documents within the browser.
   It uses a subset of HTML.
   Here is a simplified BNF for this format:

<hotlist-file> ::= <start-html-tag><hotlist-document><end-html-tag>|
		<hotlist-document>

<hotlist-document> ::= <head><body>|<body>
<head> ::= <start-head-tag><title element><end-head-tag>|<title element>
<body> ::= <start-body-tag><list><end-body-tag>|<list>

<list> ::= <structured-list>|<flat-list>

<structured-list> ::= <title-part><ul-list>|<ul-list>

<title-part> ::= <title-of-list>|<header-element>
<header-element> ::= <start-header-tag><title-of-list><end-header-tag>

<ul-list> ::=  <start-ul-tag><list-of-items><end-ul-tag>|
		<start-ul-tag><end-ul-tag>
<list-of-items> ::= <list-item> | <list-item><list-of-items>
<list-item> ::= <li-tag><item>
<item> ::= <structured-list>|<url-item>

<url-item> ::= <start-anchor-tag><title><end-anchor-tag>
<start-anchor-tag> ::= '<'<anchor-name><space><anchor-attrs>'>'
<anchor-attrs> ::= <href-attr>|<title-attr><space><href-attr>|
		<href-attr><space><title-attr>

<href-attr> ::= <href-keyword>'='<url-val>
<url-val> ::= <double-quote><url><double-quote>|
		<single-quote><url><single-quote>

<title-attr> ::= <title-keyword>'='<title-val>
<title-val> ::= <double-quote><title><double-quote>|
		<single-quote><title><single-quote>

<flat-list> ::= <url-item>|<url-item><flat-list>
*/

extern char *ParseMarkTag();
extern struct mark_up *HTMLParse();


static int notSpacesOrNewLine(char *s);
static char * mo_extract_anchors(mo_hotlist *list, struct mark_up *mptr);
static void mo_parse_hotlist_list(mo_hotlist *list, struct mark_up  **current);
static void fputExpanded(char *s, FILE *fp);
static void mo_write_list_r(mo_hotlist *list, FILE *fp);



static int notSpacesOrNewLine(char *s)
{
  int retc = 0;
  for (;*s && !retc; s++)
    if (!isspace(*s)) retc = 1;
  return retc;
}

/*
 * extract a hotlist from any HTML document
 */
static char * mo_extract_anchors(mo_hotlist *list, struct mark_up *mptr)
{
  mo_hotnode *node;
  char *name = NULL;
  char *last_text = NULL;
  char *url = NULL, *title = NULL, *rbm=NULL;

  for (; mptr != NULL; mptr = mptr->next)
    switch (mptr->type)
      {
      case M_TITLE:		/* title tag */
	if (mptr->is_end && last_text)
	  /* if this is the end tag, take the last text as name */
	  name = mo_convert_newlines_to_spaces (strdup(last_text));
	break;
      case M_NONE:		/* text, not tag */
	if (notSpacesOrNewLine(mptr->text))
	  last_text = mptr->text;
	break;
      case M_ANCHOR:
	if (!mptr->is_end)
	  {			/* start anchor tag */
	    last_text = NULL;
	    url = ParseMarkTag(mptr->start, MT_ANCHOR, AT_HREF);
	    title = ParseMarkTag(mptr->start, MT_ANCHOR, "title");
	    rbm = ParseMarkTag(mptr->start, MT_ANCHOR, "RBM"); /* SWP */
	  }
	else
	  {			/* end anchor tag */
	    node = (mo_hotnode *)malloc (sizeof (mo_hotnode));
	    node->type = mo_t_url;
	    node->url = url;
	    /* if there is a title attribute in the anchor, take it,
	       otherwise take the last text */
	    node->title = title ? title :
	      (last_text ? strdup(last_text) : strdup("Unnamed"));
	    if (rbm!=NULL) {
		node->rbm=1; /* SWP */
		free(rbm);
	    }
	    else {
		node->rbm=0;
	    }
	    mo_convert_newlines_to_spaces (node->title);
	    mo_append_item_to_hotlist (list, (mo_hot_item *)node);
	    rbm = url = title = last_text = NULL;
	  }
      default:
	break;
      }

  return name;
}

/*
 * parse a structured hotlist file recursively
 */
static void mo_parse_hotlist_list(mo_hotlist *list, struct mark_up  **current)
{
  mo_hotlist *hotlist;
  mo_hotnode *node;
  char *last_text = NULL;
  char *url = NULL, *title = NULL, *rbm=NULL;
  struct mark_up *mptr;
  int done = 0;
  
  for (mptr = *current; mptr != NULL && !done; mptr && (mptr = mptr->next))
    switch (mptr->type)
      {
      case M_NONE:		/* text, not tag */
	if (notSpacesOrNewLine(mptr->text))
	  last_text = mptr->text;
	break;
      case M_ANCHOR:
	if (!mptr->is_end)
	  {			/* start anchor tag */
	    last_text = NULL;
	    url = ParseMarkTag(mptr->start, MT_ANCHOR, AT_HREF);
	    title = ParseMarkTag(mptr->start, MT_ANCHOR, "title");
	    rbm = ParseMarkTag(mptr->start, MT_ANCHOR, "RBM"); /* SWP */
	  }
	else
	  {			/* end anchor tag */
	    node = (mo_hotnode *)malloc (sizeof (mo_hotnode));
	    node->type = mo_t_url;
	    node->url = url;
	    /* if there is a title attribute in the anchor, take it,
	       otherwise take the last text */
	    node->title = title ? title :
	      (last_text ? strdup(last_text) : strdup("Unnamed"));
	    if (node->title &&
		node->title[strlen(node->title)-1] == '\n')
	      node->title[strlen(node->title)-1] = '\0';
	    if (rbm!=NULL) {
		node->rbm=1; /* SWP */
		free(rbm);
	    }
	    else {
		node->rbm=0;
	    }
	    mo_convert_newlines_to_spaces (node->title);
	    mo_append_item_to_hotlist (list, (mo_hot_item *)node);
	    rbm = url = title = last_text = NULL;
	  }
	break;
      case M_UNUM_LIST:
	if (!mptr->is_end)
	  {			/* start Unum List tag */
	    hotlist = (mo_hotlist *)malloc(sizeof(mo_hotlist));
	    hotlist->type = mo_t_list;
	    hotlist->nodelist = hotlist->nodelist_last = 0;
	    hotlist->parent = list;
	    hotlist->name = last_text ?  strdup(last_text) : strdup("Unnamed");
	    if (hotlist->name &&
		hotlist->name[strlen(hotlist->name)-1] == '\n')
	      hotlist->name[strlen(hotlist->name)-1] = '\0';
	    mo_convert_newlines_to_spaces (hotlist->name);
	    rbm=ParseMarkTag(mptr->start, MT_UNUM_LIST, "RBM");
	    if (rbm!=NULL) {
		hotlist->rbm=1; /* SWP */
		free(rbm);
	    }
	    else {
		hotlist->rbm=0;
	    }
	    rbm=NULL;
	    mo_append_item_to_hotlist(list, (mo_hot_item *)hotlist);
	    mptr = mptr->next;
	    last_text = NULL;
	    mo_parse_hotlist_list(hotlist, &mptr);
	    /* after this call, mptr is positionned on the end Unum List tag */
	  }
	else			/* end Unum List tag */
	  *current = mptr, done = 1;
      default:
	break;
      }
  if (!done)
    *current = NULL;
}

/* Read a hotlist from a file.
 * fill the hotlist list given as parameter
 */
char * mo_read_new_hotlist(mo_hotlist *list, FILE *fp)
{
/*  char *status;*/
  char *name, *ptr;
  int done, normal, has_list, depth;
  long size;
  struct mark_up *hot_mark_up, *mptr;
  char *text;

  setbuf(fp, NULL);
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  text = malloc(size+1);
  if (!text)
    return NULL;
  fseek(fp, 0L, SEEK_SET);
  fread(text, (size_t)1, (size_t)size, fp);
  text[size] = '\0';
  /* parse the HTML document */
  hot_mark_up = HTMLParse(NULL, text, NULL);
  free(text);

  /* some pre-processing to see if this is in hotlist format or if
     this is a normal document
     The algo is as follow: if an anchor is outside a list or if there
     are more than one top level list, then it is not in hotlist format.
     the 'normal' flag at the end of the pre-processing tells if it
     is a normal document or a hotlist.
   */
  done = 0;
  normal = 0;
  has_list = 0;
  depth = 0;
  for (mptr = hot_mark_up; mptr != NULL && !done; mptr = mptr->next)
    {
      switch (mptr->type)
	{
	case M_ANCHOR:
	  if (!depth)
	    done = 1, normal = 1;
	  break;
	case M_UNUM_LIST:
	  if (!mptr->is_end)	/* start unum list tag */
	    if (!depth && has_list)
	      done = 1, normal = 1;
	    else
	      depth++, has_list = 1;
	  else			/* end unum list tag */
	    depth--;
	default:
	  break;
	}
      if (done)
	break;
    }
  /* now we know what kind of file we are dealing with */
  if (normal) {
    name = mo_extract_anchors(list, hot_mark_up);
  }
  else {
      char *last_text = NULL;

      done = 0;
      for (mptr = hot_mark_up; mptr != NULL && !done; mptr = mptr->next)
	switch (mptr->type)
	  {
	  case M_NONE:		/* text, not tag */
	    if (notSpacesOrNewLine(mptr->text))
	      last_text = mptr->text;
	    break;
	  case M_UNUM_LIST:	/* Unum List tag */
	    done = 1;
	  default:
	    break;
	  }
      /* after this loop, mptr is positionned just after the
	 start anchor tag */
      name = last_text ? mo_convert_newlines_to_spaces (strdup(last_text)) :
	NULL;
      mo_parse_hotlist_list(list, &mptr);
  }

  FreeObjList(hot_mark_up);

  /*
   * SWP
   *
   * Problem with hotlist name growing by 1 space with each write. So...
   *   we chop off all the spaces on the end here.
   * We do it this way to get rid of all the people out there who already
   *   have hotlist names that are space infested.
   */
  for (ptr=(name+strlen(name)-1); ptr && *ptr==' '; ptr--) {
	*ptr='\0';
  }

  return name;
}

/*
 * This function replace '>', '<' and '&' by their entity references
 * and output them.
 */
static void fputExpanded(char *s, FILE *fp)
{
  for (;*s;s++)
    if (*s == '<')
      fputs("&lt;", fp);
    else if (*s == '>')
      fputs("&gt;", fp);
    else if (*s == '&')
      fputs("&amp;", fp);
    else
      putc(*s, fp);
}

/*
 * recursive function called to write a hotlist out to a file
 */
static void mo_write_list_r(mo_hotlist *list, FILE *fp)
{
  mo_hot_item *item;

  fputExpanded(list->name, fp);
  if (list->rbm) {
	fputs("\n<UL RBM>\n", fp);
  }
  else {
	fputs("\n<UL>\n", fp);
  }
  for (item = list->nodelist; item != NULL; item = item->any.next)
    if (item->type == mo_t_url)	/* URL item */
      {
	if (!(item->hot.url)) {
		continue;
	}
	if (item->hot.rbm) {
		fputs("<LI> <A RBM HREF=\"", fp);
	}
	else {
		fputs("<LI> <A HREF=\"", fp);
	}
	fputExpanded(item->hot.url, fp);
	fputs("\"> ", fp);
	if (!(item->hot.title)) {
		fputs("No Title\n",fp);
	}
	else {
		fputExpanded(item->hot.title, fp);
	}
	fputs("</A>\n", fp);
      }
    else			/* list item */
      {
	fputs("<LI> ", fp);
	mo_write_list_r(&(item->list), fp);
      }
  fputs("</UL>\n", fp);
}

/*
 * Write a hotlist out to a file.
 * Return mo_succeed if everything goes OK;
 * mo_fail else.
 */
mo_status mo_write_hotlist (mo_hotlist *list, FILE *fp)
{
/*  mo_hotnode *node;*/

  fputs("<HTML>\n", fp);
  fprintf(fp, "%s\n", NCSA_HOTLIST_FORMAT_COOKIE_THREE);

  fputs("<TITLE>Hotlist from ", fp);
  if (!get_pref_string(eDEFAULT_AUTHOR_NAME)) {
	fputExpanded("Unknown", fp);      
  }
  else {
	fputExpanded(get_pref_string(eDEFAULT_AUTHOR_NAME), fp);      
  }
  fputs("</TITLE>\n", fp);
  
  mo_write_list_r(list, fp);

  fputs("</HTML>\n", fp);

  return mo_succeed;
}

