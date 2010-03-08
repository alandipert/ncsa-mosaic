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
#include "gui.h"
#include "mo-www.h"
#include "gui-popup.h"
#include <time.h>
#include <sys/types.h>
#include "libhtmlw/HTML.h"

#define __SRC__
#include "../libwww2/HTAAUtil.h"

/*for memcpy*/
#include <memory.h>

extern Display *dsp;
void mo_add_to_rbm_history();

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

/* ------------------------------------------------------------------------ */
/* ----------------------------- HISTORY LIST ----------------------------- */
/* ------------------------------------------------------------------------ */

/* ---------------------------- kill functions ---------------------------- */

/* Free the data contained in an mo_node.  Currently we only free
   the text itself. */
mo_status mo_free_node_data (mo_node *node)
{
  if (node->texthead != NULL)
    {
      free (node->texthead);
      node->texthead = NULL;
    }
  if (node->title != NULL)
    {
      free (node->title);
      node->title = NULL;
    }
#if 0
  /* Leads to memory getting freed twice in some cases, apparently.
     Not sure why. */
  if (node->url != NULL)
    free (node->url);

  if (node->ref != NULL)
    {
      free (node->ref);
      node->ref = NULL;
    }
#endif

  if (node->cached_stuff)
    {
      HTMLFreeWidgetInfo (node->cached_stuff);
      node->cached_stuff = NULL;
    }

  return mo_succeed;
}

/* Kill a single mo_node associated with a given mo_window; it
   the history list exists we delete it from that.  In any case
   we call mo_free_node_data and return. */
mo_status mo_kill_node (mo_window *win, mo_node *node)
{
  if (win->history_list)
    XmListDeletePos (win->history_list, node->position);
  mo_free_node_data (node);

  return mo_succeed;
}

/* Iterate through all descendents of an mo_node, but not the given
   mo_node itself, and kill them.  This is equivalent to calling
   mo_kill_node on each of those nodes, except this is faster since
   all the Motif list entries can be killed at once. */
mo_status mo_kill_node_descendents (mo_window *win, mo_node *node)
{
  mo_node *foo;
  int count = 0;

  if (node == NULL)
    return mo_succeed;
  for (foo = node->next; foo != NULL; foo = foo->next)
    {
      mo_free_node_data (foo);
      count++;
    }

  /* Free count number of items from the end of the list... */
  if (win->history_list && count)
    {
      XmListDeleteItemsPos (win->history_list, count, node->position + 1);
    }

  return mo_succeed;
}

/* ------------------------ mo_add_node_to_history ------------------------ */

/* Called from mo_record_visit to insert an mo_node into the history
   list of an mo_window. */
mo_status mo_add_node_to_history (mo_window *win, mo_node *node)
{
  /* If there is no current node, this is our first time through. */
  if (win->history == NULL)
    {
      win->history = node;
      node->previous = NULL;
      node->next = NULL;
      node->position = 1;
      win->current_node = node;
    }
  else
    {
      /* Node becomes end of history list. */
      /* Point back at current node. */
      node->previous = win->current_node;
      /* Point forward to nothing. */
      node->next = NULL;
      node->position = node->previous->position + 1;
      /* Kill descendents of current node, since we'll never
         be able to go forward to them again. */
      mo_kill_node_descendents (win, win->current_node);
      /* Current node points forward to this. */
      win->current_node->next = node;
      /* Current node now becomes new node. */
      win->current_node = node;
    }

  if (win->history_list)
    {
      XmString xmstr = 
	XmxMakeXmstrFromString(
			       get_pref_boolean(eDISPLAY_URLS_NOT_TITLES) ?
			       node->url : node->title);
      XmListAddItemUnselected 
        (win->history_list, xmstr, node->position);
      XmStringFree (xmstr);
    }

  return mo_succeed;
}

/* ---------------------------- mo_grok_title ----------------------------- */

/* Make up an appropriate title for a document that does not otherwise
   have one associated with it. */
static char *mo_grok_alternate_title (char *url, char *ref)
{
  char *title, *foo1, *foo2;

  if (!strncmp (url, "gopher:", 7))
    {
      /* It's a gopher server. */
      /* Do we have a ref? */
      if (ref)
        {
          char *tmp = ref;
          while (*tmp && (*tmp == ' ' || *tmp == '\t'))
            tmp++;
          title = strdup (tmp);
          goto done;
        }
      else
        {
          /* Nope, no ref.  Make up a title. */
          foo1 = url + 9;
          foo2 = strstr (foo1, ":");
          /* If there's a trailing colon (always should be.. ??)... */
          if (foo2)
            {
              char *server = (char *) malloc ((foo2 - foo1 + 2));
              
/*              bcopy (foo1, server, (foo2 - foo1));*/
              memcpy(server, foo1, (foo2 - foo1));
              server[(foo2 - foo1)] = '\0';
              
              title = (char *) malloc ((strlen (server) + 32) * sizeof (char));
              sprintf (title, "%s %s", "Gopher server at" , server);
              
              /* OK, we got a title... */
              free (server);

              goto done;
            }
          else
            {
              /* Aw hell... */
              title = strdup ("Gopher server" );
              goto done;
            }
        }
    }

  /* If we got here, assume we should use 'ref' if possible
     for the WAIS title. */
  if (!strncmp (url, "wais:", 5) || 
      !strncmp (url, "http://info.cern.ch:8001/", 25) ||
      !strncmp (url, "http://info.cern.ch.:8001/", 26) ||
      !strncmp (url, "http://www.ncsa.uiuc.edu:8001/", 30))
    {
      /* It's a WAIS server. */
      /* Do we have a ref? */
      if (ref)
        {
          title = strdup (ref);
          goto done;
        }
      else
        {
          /* Nope, no ref.  Make up a title. */
          foo1 = url + 7;
          foo2 = strstr (foo1, ":");
          /* If there's a trailing colon (always should be.. ??)... */
          if (foo2)
            {
              char *server = (char *) malloc ((foo2 - foo1 + 2));
              
/*              bcopy (foo1, server, (foo2 - foo1));*/
              memcpy(server, foo1, (foo2 - foo1));
              server[(foo2 - foo1)] = '\0';
              
              title = (char *) malloc ((strlen (server) + 32) * sizeof (char));
              sprintf (title, "%s %s", "WAIS server at" , server);
              
              /* OK, we got a title... */
              free (server);

              goto done;
            }
          else
            {
              /* Aw hell... */
              title = strdup ("WAIS server" );
              goto done;
            }
        }
    }

  if (!strncmp (url, "news:", 5))
    {
      /* It's a news source. */
      if (strstr (url, "@"))
        {
          /* It's a news article. */
          foo1 = url + 5;
          
          title = (char *)malloc ((strlen (foo1) + 32) * sizeof (char));
          sprintf (title, "%s %s", "USENET article" , foo1);

          goto done;
        }
      else
        {
          /* It's a newsgroup. */
          foo1 = url + 5;
          
          title = (char *)malloc ((strlen (foo1) + 32) * sizeof (char));
          sprintf (title, "%s %s", "USENET newsgroup" , foo1);

          goto done;
        }
    }

  if (!strncmp (url, "file:", 5))
    {
      /* It's a file. */
      if (strncmp (url, "file:///", 8) == 0)
        {
          /* It's a local file. */
          foo1 = url + 7;
          
          title = (char *)malloc ((strlen (foo1) + 32) * sizeof (char));
          sprintf (title, "%s %s", "Local file" , foo1);
          
          goto done;
        }
      else if (strncmp (url, "file://localhost/", 17) == 0)
        {
          /* It's a local file. */
          foo1 = url + 16;
          
          title = (char *)malloc ((strlen (foo1) + 32) * sizeof (char));
          sprintf (title, "%s %s", "Local file" , foo1);
          
          goto done;
        }
      else
        {
          /* It's a remote file. */
          foo1 = url + 7;
          
          title = (char *)malloc ((strlen (foo1) + 32) * sizeof (char));
          sprintf (title, "%s %s", "Remote file" , foo1);
          
          goto done;
        }
    }
  
  if (!strncmp (url, "ftp:", 4))
    {
      {
        /* It's a remote file. */
        foo1 = url + 6;
        
        title = (char *)malloc ((strlen (foo1) + 32) * sizeof (char));
        sprintf (title, "%s %s", "Remote file" , foo1);
        
        goto done;
      }
    }
  
  /* Punt... */
  title = (char *) malloc ((strlen (url) + 24) * sizeof (char));
  sprintf (title, "%s %s", "Untitled" , url);
  
 done:
  return title;
}

/* Figure out a title for the given URL.  'ref', if it exists,
   was the text used for the anchor that pointed us to this URL;
   it is not required to exist. */
char *mo_grok_title (mo_window *win, char *url, char *ref)
{
  char *title = NULL, *t;

  XtVaGetValues (win->scrolled_win, WbNtitleText, &title, NULL);
  if (!title)
    t = mo_grok_alternate_title (url, ref);
  else if (!strcmp (title, "Document"))
    t = mo_grok_alternate_title (url, ref);
  else
    {
      char *tmp = title;
      while (*tmp && (*tmp == ' ' || *tmp == '\t'))
        tmp++;
      if (*tmp)
        t = strdup (tmp);
      else
        t = mo_grok_alternate_title (url, ref);
    }

  mo_convert_newlines_to_spaces (t);

  return t;
}

/* --------------------------- mo_record_visit ---------------------------- */

extern int securityType;

/* Called when we visit a new node (as opposed to backing up or
   going forward).  Create an mo_node entry, call mo_grok_title
   to figure out what the title is, and call mo_node_to_history
   to add the new mo_node to both the window's data structures and
   to its Motif history list. */
mo_status mo_record_visit (mo_window *win, char *url, char *newtext, 
                           char *newtexthead, char *ref,
			   char *last_modified, char *expires)
{
  mo_node *node = (mo_node *)malloc (sizeof (mo_node));
  node->url = url;
  node->text = newtext;
  node->texthead = newtexthead;
  node->ref = ref;
  /* Figure out what the title is... */
  node->title = mo_grok_title (win, url, ref);

  node->authType=securityType;
  securityType=HTAA_NONE;
  mo_gui_check_security_icon_in_win(node->authType,win);

  /* This will be recalc'd when we leave this node. */
  node->docid = 1;
  node->cached_stuff = NULL;

  /* This may or may not be filled in later! (AF) */
  node->last_modified = 0;
  if (last_modified)
      node->last_modified = strdup(last_modified);
  node->expires = 0;
  if (expires)
      node->expires = strdup(expires);

  mo_add_node_to_history (win, node);
  mo_add_to_rbm_history(win, node->url, node->title);

  return mo_succeed;
}

/* ------------------------- navigation functions ------------------------- */

/* Back up a node. */
mo_status mo_back_node (mo_window *win)
{
  /* If there is no previous node, choke. */
  if (!win->current_node || win->current_node->previous == NULL)
    return mo_fail;

  mo_gui_apply_default_icon();
  mo_set_win_current_node (win, win->current_node->previous);

  return mo_succeed;
}

/* Go forward a node. */
mo_status mo_forward_node (mo_window *win)
{
  /* If there is no next node, choke. */
  if (!win->current_node || win->current_node->next == NULL)
    return mo_fail;

  mo_gui_apply_default_icon();
  mo_set_win_current_node (win, win->current_node->next);

  return mo_succeed;
}

/* Visit an arbitrary position.  This is called when a history
   list entry is double-clicked upon.

   Iterate through the window history; find the mo_node associated
   with the given position.  Call mo_set_win_current_node. */
mo_status mo_visit_position (mo_window *win, int pos)
{
  mo_node *node;
  
  for (node = win->history; node != NULL; node = node->next)
    {
      if (node->position == pos)
        {
          mo_set_win_current_node (win, node);
          goto done;
        }
    }

#ifndef DISABLE_TRACE
  if (srcTrace) {
	fprintf (stderr, "UH OH BOSS, asked for position %d, ain't got it.\n",
		 pos);
  }
#endif

 done:
  return mo_succeed;
}

/* ---------------------------- misc functions ---------------------------- */

mo_status mo_dump_history (mo_window *win)
{
  mo_node *node;

#ifndef DISABLE_TRACE
  if (srcTrace) {

  fprintf (stderr, "----------------- history -------------- \n");
  fprintf (stderr, "HISTORY is 0x%08x\n", win->history);
  for (node = win->history; node != NULL; node = node->next)
    {
      fprintf (stderr, "NODE %d %s\n", node->position, node->url);
      fprintf (stderr, "     TITLE %s\n", node->title);
    }
  fprintf (stderr, "CURRENT NODE %d %s\n", win->current_node->position,
           win->current_node->url);
  fprintf (stderr, "----------------- history -------------- \n");

  }
#endif

  return mo_succeed;
}  

/* ------------------------------------------------------------------------ */
/* ----------------------------- HISTORY GUI ------------------------------ */
/* ------------------------------------------------------------------------ */

/* We've just init'd a new history list widget; look at the window's
   history and load 'er up. */
static void mo_load_history_list (mo_window *win, Widget list)
{
  mo_node *node;
  
  for (node = win->history; node != NULL; node = node->next)
    {
      XmString xmstr = 
	XmxMakeXmstrFromString (get_pref_boolean(eDISPLAY_URLS_NOT_TITLES) ?
                                node->url : node->title);
      XmListAddItemUnselected 
        (list, xmstr, 0);
      XmStringFree (xmstr);
    }
  
  XmListSetBottomPos (list, 0);
  if (win->current_node)
    XmListSelectPos (win->history_list, win->current_node->position, False);

  return;
}

/* ----------------------------- mail history ----------------------------- */

static XmxCallback (mailhist_win_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  char *to, *subj;
  FILE *fp;

  switch (XmxExtractToken ((int)client_data))
    {
    case 0:
      XtUnmanageChild (win->mailhist_win);

      mo_busy ();

      to = XmxTextGetString (win->mailhist_to_text);
      if (!to)
        return;
      if (to[0] == '\0')
        return;

      subj = XmxTextGetString (win->mailhist_subj_text);

      /* Open a file descriptor to sendmail. */
      fp = mo_start_sending_mail_message (to, subj, "text/x-html", NULL);
      if (!fp)
        goto oops;

      {
        mo_node *node;
        
        fprintf (fp, "<HTML>\n");
        fprintf (fp, "<H1>History Path From %s</H1>\n",
                 get_pref_string(eDEFAULT_AUTHOR_NAME));
        fprintf (fp, "<DL>\n");
        for (node = win->history; node != NULL; node = node->next)
          {
            fprintf (fp, "<DT>%s\n<DD><A HREF=\"%s\">%s</A>\n", 
                     node->title, node->url, node->url);
          }
        fprintf (fp, "</DL>\n");
        fprintf (fp, "</HTML>\n");
      }
        
      mo_finish_sending_mail_message ();

    oops:
      free (to);
      free (subj);

      mo_not_busy ();
            
      break;
    case 1:
      XtUnmanageChild (win->mailhist_win);
      /* Do nothing. */
      break;
    case 2:
      mo_open_another_window
        (win, 
         mo_assemble_help_url ("help-on-nested-hotlists.html"), 
         NULL, NULL);
      break;
    }

  return;
}

static mo_status mo_post_mailhist_win (mo_window *win)
{
  /* This shouldn't happen. */
  if (!win->history_win)
    return mo_fail;

  if (!win->mailhist_win)
    {
      Widget dialog_frame;
      Widget dialog_sep, buttons_form;
      Widget mailhist_form, to_label, subj_label;
      
      /* Create it for the first time. */
      XmxSetUniqid (win->id);
      win->mailhist_win = XmxMakeFormDialog 
        (win->history_win, "NCSA Mosaic: Mail Window History" );
      dialog_frame = XmxMakeFrame (win->mailhist_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints 
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
      
      /* Main form. */
      mailhist_form = XmxMakeForm (dialog_frame);
      
      to_label = XmxMakeLabel (mailhist_form, "Mail To:" );
      XmxSetArg (XmNwidth, 335);
      win->mailhist_to_text = XmxMakeTextField (mailhist_form);
      
      subj_label = XmxMakeLabel (mailhist_form, "Subject:" );
      win->mailhist_subj_text = XmxMakeTextField (mailhist_form);

      dialog_sep = XmxMakeHorizontalSeparator (mailhist_form);
      
      buttons_form = XmxMakeFormAndThreeButtons
        (mailhist_form, mailhist_win_cb, "Mail" ,
	 "Dismiss" , "Help..." , 0, 1, 2);

      /* Constraints for mailhist_form. */
      XmxSetOffsets (to_label, 14, 0, 10, 0);
      XmxSetConstraints
        (to_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_NONE,
         NULL, NULL, NULL, NULL);
      XmxSetOffsets (win->mailhist_to_text, 10, 0, 5, 10);
      XmxSetConstraints
        (win->mailhist_to_text, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_WIDGET,
         XmATTACH_FORM, NULL, NULL, to_label, NULL);

      XmxSetOffsets (subj_label, 14, 0, 10, 0);
      XmxSetConstraints
        (subj_label, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, 
         XmATTACH_NONE,
         win->mailhist_to_text, NULL, NULL, NULL);
      XmxSetOffsets (win->mailhist_subj_text, 10, 0, 5, 10);
      XmxSetConstraints
        (win->mailhist_subj_text, XmATTACH_WIDGET, XmATTACH_NONE, 
         XmATTACH_WIDGET,
         XmATTACH_FORM, win->mailhist_to_text, NULL, subj_label, NULL);

      XmxSetArg (XmNtopOffset, 10);
      XmxSetConstraints 
        (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, 
         XmATTACH_FORM,
         win->mailhist_subj_text, buttons_form, NULL, NULL);
      XmxSetConstraints 
        (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
    }
  
  XtManageChild (win->mailhist_win);
  
  return mo_succeed;
}

/* ---------------------------- history_win_cb ---------------------------- */

static XmxCallback (history_win_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  switch (XmxExtractToken ((int)client_data))
    {
    case 0:
      XtUnmanageChild (win->history_win);
      /* Dismissed -- do nothing. */
      break;
    case 1:
      mo_post_mailhist_win (win);
      break;
    case 2:
      mo_open_another_window
        (win, 
         mo_assemble_help_url ("docview-menubar-navigate.html#history"),
         NULL, NULL);
      break;
    }

  return;
}

static XmxCallback (history_list_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  XmListCallbackStruct *cs = (XmListCallbackStruct *)call_data;
  
  mo_visit_position (win, cs->item_position);

  return;
}

mo_status mo_post_history_win (mo_window *win)
{
  if (!win->history_win)
    {
      Widget dialog_frame;
      Widget dialog_sep, buttons_form;
      Widget history_label;
      Widget history_form;
      XtTranslations listTable;
      static char listTranslations[] =
	"~Shift ~Ctrl ~Meta ~Alt <Btn2Down>: ListKbdSelectAll() ListBeginSelect() \n\
	 ~Shift ~Ctrl ~Meta ~Alt <Btn2Up>:   ListEndSelect() ListKbdActivate()";

      listTable = XtParseTranslationTable(listTranslations);
      
      /* Create it for the first time. */
      XmxSetUniqid (win->id);
      win->history_win = XmxMakeFormDialog 
        (win->base, "NCSA Mosaic: Window History" );
      dialog_frame = XmxMakeFrame (win->history_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints 
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
      
      /* Main form. */
      history_form = XmxMakeForm (dialog_frame);

      XmxSetArg (XmNalignment, XmALIGNMENT_BEGINNING);
      history_label = XmxMakeLabel (history_form, "Where you've been:" );

      /* History list itself. */
      XmxSetArg (XmNresizable, False);
      XmxSetArg (XmNscrollBarDisplayPolicy, XmSTATIC);
      XmxSetArg (XmNlistSizePolicy, XmCONSTANT);
      XmxSetArg (XmNwidth, 380);
      XmxSetArg (XmNheight, 184);
      win->history_list = XmxMakeScrolledList 
        (history_form, history_list_cb, 0);
      XtAugmentTranslations (win->history_list, listTable);

      dialog_sep = XmxMakeHorizontalSeparator (history_form);
      
      buttons_form = XmxMakeFormAndThreeButtons(history_form,
						history_win_cb,
						"Mail To...",
						"Dismiss", 
						"Help...", 
						1,
						0,
						2);

      /* Constraints for history_form. */
      XmxSetOffsets (history_label, 8, 0, 10, 10);
      XmxSetConstraints
        (history_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM,
         XmATTACH_NONE, NULL, NULL, NULL, NULL);
      /* History list is stretchable. */
      XmxSetOffsets (XtParent (win->history_list), 0, 10, 10, 10);
      XmxSetConstraints
        (XtParent (win->history_list), 
         XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM, 
         history_label, dialog_sep, NULL, NULL);
      XmxSetArg (XmNtopOffset, 10);
      XmxSetConstraints 
        (dialog_sep, 
         XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
         NULL, buttons_form, NULL, NULL);
      XmxSetConstraints 
        (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM,
         NULL, NULL, NULL, NULL);

      /* Go get the history up to this point set up... */
      mo_load_history_list (win, win->history_list);
    }

  XmxManageRemanage (win->history_win);
  
  return mo_succeed;
}
