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
#include "hotfile.h"
#include "gui.h"
#include "mo-www.h"
#include <time.h>
#include <Xm/List.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>
#include <sys/types.h>

#include "bitmaps/hotlist.xbm"
#include "../libnut/system.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

extern mo_window *current_win;

void mo_reinit_hotmenu();


/* This file provides support for hotlists of interesting
   documents within the browser.

   Initially there will be a single hotlist, 'Default'.

   The old hotlist file format look like this:

   ncsa-mosaic-hotlist-format-1            [identifying string]
   Default                                 [title]
   url Fri Sep 13 00:00:00 1986            [first word is url;
                                            subsequent words are
                                            last-accessed date (GMT)]
   document title cached here              [cached title for above]
   [2-line sequence for single document repeated as necessary]
   ...

   Turns out this format is bad for two reasons:
   (1) Document titles can have embedded carriage returns (usually
       on purpose).
   (2) URL's can have embedded carriage returns (usually on accident).

   Also, we should just be using an HTML-derived format for hotlists.
*/

#define LISTINDIC "-> "

#define FindHotFromPos(hotnode, list, posi)			\
  do for (hotnode = list->nodelist; hotnode != NULL;		\
	  hotnode = hotnode->any.next)				\
    {								\
      if (hotnode->any.position == posi)			\
        break;							\
    } while(0)

typedef struct edit_or_insert_hot_info
{
  Widget title_text;
  int pos;
  Widget url_lab;
  Widget url_text;
  Widget tog_url;
  Widget tog_list;
  Widget insert_tog;
} edit_or_insert_hot_info;


void URL_Include_Set(mo_window *win, int togval, int sensitive) {

	if (!win) {
		win=current_win;
		if (!win) {
			return;
		}
	}

	XmxSetToggleButton(win->hotlist_rbm_toggle,
			   togval);
	XtVaSetValues(win->hotlist_rbm_toggle,
		      XmNsensitive, sensitive,
		      NULL);

	return;
}


mo_root_hotlist *default_hotlist = NULL;

/*
 * Given a hotlist and a hotnode, append the node
 * to the hotlist.
 * Change fields nodelist and nodelist_last in the hotlist,
 * and fields next and previous in the hotnode.
 * Also fill in field position in the hotnode.
 * Return nothing.
 */
void mo_append_item_to_hotlist (mo_hotlist *list,
				mo_hot_item *node)
{
  if (node->type == mo_t_list)
    node->list.parent = list;
  if (list->nodelist == 0)
    {
      /* Nothing yet. */
      list->nodelist = node;
      list->nodelist_last = node;
      node->any.next = 0;
      node->any.previous = 0;
      node->any.position = 1;
    }
  else
    {
      /* The new node becomes nodelist_last. */
      /* But first, set up node. */
      node->any.previous = list->nodelist_last;
      node->any.next = 0;
      node->any.position = node->any.previous->any.position + 1;

      /* Now point forward from previous nodelist_last. */
      list->nodelist_last->any.next = node;

      /* Now set up new nodelist_last. */
      list->nodelist_last = node;
    }

  return;
}

/* Given a hotlist and a hotnode, rip the hotnode out of the hotlist.
   No check is made as to whether the hotnode is actually in the hotlist;
   it better be. */
static void mo_remove_hotnode_from_hotlist (mo_hotlist *list,
                                            mo_hot_item *hotnode)
{
  if (hotnode->any.previous == NULL)
    {
      /* Node was the first member of the list. */
      if (hotnode->any.next != NULL)
        {
          /* Node was the first member of the list and had
             a next node. */
          /* The next node is now the first node in the list. */
          hotnode->any.next->any.previous = NULL;
          list->nodelist = hotnode->any.next;
        }
      else
        {
          /* Node was the first member of the list and
             didn't have a next node. */
          /* The list is now empty. */
          list->nodelist = NULL;
          list->nodelist_last = NULL;
        }
    }
  else
    {
      /* Node had a previous. */
      if (hotnode->any.next != NULL)
        {
          /* Node had a previous and a next. */
          hotnode->any.previous->any.next = hotnode->any.next;
          hotnode->any.next->any.previous = hotnode->any.previous;
        }
      else
        {
          /* Node had a previous but no next. */
          hotnode->any.previous->any.next = NULL;
          list->nodelist_last = hotnode->any.previous;
        }
    }

  if (hotnode->type == mo_t_list)
    {
      mo_hot_item *item, *prev;
      for (item = hotnode->list.nodelist; item; free(prev))
	{
	  mo_remove_hotnode_from_hotlist (&(hotnode->list), item);
	  prev = item; item = hotnode->list.nodelist;
	}
    }

  return;
}

/* Go through a hotlist (sequence of hotnodes) and assign position
   numbers for all of 'em. */
static void mo_recalculate_hotlist_positions (mo_hotlist *list)
{
  mo_hot_item *hotnode;
  int count = 1;

  for (hotnode = list->nodelist; hotnode != NULL;
       hotnode = hotnode->any.next)
    hotnode->any.position = count++;

  return;
}

/* Insert an item in a list at a given position.
   if position is 0, then append it (at end of list). */
static void mo_insert_item_in_hotlist (mo_hotlist *list, mo_hot_item *node,
				       int position)
{
  if (!position)
    {
      mo_append_item_to_hotlist (list, node);
    }
  else
    {
      if (node->type == mo_t_list)
	node->list.parent = list;
      if (list->nodelist == 0)
	{
	  /* Nothing yet. */
	  list->nodelist = node;
	  list->nodelist_last = node;
	  node->any.next = 0;
	  node->any.previous = 0;
	  node->any.position = 1;
	}
      else
	{
	  mo_hot_item *item, **prevNextPtr = &list->nodelist;
	  /* search the item at position 'position' */
	  for (item = list->nodelist; item != NULL;
	       item = item->any.next)
	    {
	      if (item->any.position == position)
		break;
	      prevNextPtr = &item->any.next;
	    }

	  if (item == NULL)	/* item not found */
	    mo_append_item_to_hotlist (list, node);
	  else
	    {
	      *prevNextPtr = node;
	      node->any.previous = item->any.previous;
	      node->any.next = item;
	      item->any.previous = node;
	      mo_recalculate_hotlist_positions (list);
	    }
	}
    }
    mo_reinit_hotmenu();
}

/* Go Up The tree to check if a list is the ancestor of an item
 */
static int mo_is_ancestor (mo_hotlist *list, mo_hotlist *item)
{
  while (item && item != list)
    item = item->parent;

  return item == list;
}

/* recursive function that copy a hierarchy of hotlist */
static mo_hotlist *mo_copy_hot_hier (mo_hotlist *list)
{
  mo_hot_item *item;
  mo_hotnode *hot;
  mo_hotlist *hotlist = (mo_hotlist *)malloc(sizeof(mo_hotlist));

  hotlist->name = strdup(list->name);
  hotlist->type = mo_t_list;
  hotlist->nodelist = hotlist->nodelist_last = 0;
  for (item = list->nodelist; item; item = item->any.next)
    if (item->type == mo_t_url)
      {
	hot = (mo_hotnode *)malloc(sizeof(mo_hotnode));
	hot->type = mo_t_url;
	hot->title = strdup(item->hot.title);
	hot->url = strdup(item->hot.url);
	/*hot->lastdate = strdup(item->hot.lastdate);*/
	hot->lastdate = (char *) 0;
	mo_append_item_to_hotlist(hotlist, (mo_hot_item *)hot);
      }
    else
      {
	mo_append_item_to_hotlist
	  (hotlist, (mo_hot_item *)mo_copy_hot_hier((mo_hotlist *)item));
      }
  return hotlist;
}

static char * mo_compute_hot_path (mo_hotlist *curr)
{
  char *str;
  char *prev = curr->parent ? strdup(curr->name) : strdup("/");

  for (str = prev, curr = curr->parent; curr; curr = curr->parent)
    {
      if (curr->parent)
	{
	  str = (char *)malloc(strlen(prev)+strlen(curr->name)+2);
	  strcat(strcat(strcpy(str, curr->name), "/"),prev);
	}
      else
	{
	  str = (char *)malloc(strlen(prev)+2);
	  strcat(strcpy(str, "/"), prev);
	}
      free(prev);
      prev = str;
    }
  return str;
}

static void mo_copy_hotlist_position (mo_window *win, int position)
{
  mo_hot_item *item;

  for (item = win->current_hotlist->nodelist;
       item != NULL && item->any.position != position;
       item = item->any.next) ;

  if (item)
    win->hot_cut_buffer = item;
}

static char * mo_highlight_hotlist (mo_hotlist *list)
{
  char *str = (char *)malloc(strlen(list->name)+strlen(LISTINDIC)+1);
  return
    strcat(strcpy(str,LISTINDIC), list->name);
}

static void mo_gui_add_hot_item (mo_hotlist *list, mo_hot_item *item)
{
  mo_window *win = NULL;
  /* Now we've got to update all active hotlist_list's. */
  while (win = mo_next_window (win))
    if (win->hotlist_list && win->current_hotlist == list)
      {
	char *highlight = NULL;
	XmString xmstr =
	  XmxMakeXmstrFromString
	    (item->type == mo_t_url ?
	     (get_pref_boolean(eDISPLAY_URLS_NOT_TITLES)?item->hot.url:item->hot.title) :
	     (highlight = mo_highlight_hotlist(&item->list)));
	if (item->type == mo_t_list && highlight)
	  free(highlight);
	XmListAddItemUnselected
	  (win->hotlist_list,
	   xmstr,
	   item->any.position);
	XmStringFree (xmstr);
	XmListSetBottomPos (win->hotlist_list, 0);
      }

      mo_reinit_hotmenu();
}

mo_status mo_add_item_to_hotlist (mo_hotlist *list, mo_item_type type,
                                  char *title, char *url, int position,
				  int rbm)
{
  mo_hot_item *item;
  mo_window *win = NULL;

  if ((title == NULL || title[0] == '\0') && (url == NULL || url[0] == '\0'))
    return mo_fail;

  if (type == mo_t_url)
    {
      mo_hotnode *hotnode = (mo_hotnode *)malloc (sizeof (mo_hotnode));
      time_t foo = time (NULL);
      char *ts = ctime (&foo);

      item = (mo_hot_item *)hotnode;
      ts[strlen(ts)-1] = '\0';

      hotnode->type = mo_t_url;
      if (title)
	hotnode->title = strdup (title);
      else
	hotnode->title = strdup ("Unnamed");
      mo_convert_newlines_to_spaces (hotnode->title);

      hotnode->url = strdup (url);
      mo_convert_newlines_to_spaces (hotnode->url);

      hotnode->lastdate = strdup (ts);

      hotnode->rbm=rbm;
    }
  else
    {
      mo_hotlist *hotlist = (mo_hotlist *)malloc(sizeof(mo_hotlist));

      item = (mo_hot_item *)hotlist;
      hotlist->type = mo_t_list;
      if (title)
	hotlist->name = strdup (title);
      else
	hotlist->name = strdup ("Unnamed");
      mo_convert_newlines_to_spaces (hotlist->name);
      hotlist->nodelist = hotlist->nodelist_last = 0;
      hotlist->rbm=rbm;
    }

  if (position)
    mo_insert_item_in_hotlist(list, item, position);
  else
    mo_append_item_to_hotlist (list,  item);

  mo_gui_add_hot_item (list, item);

  return mo_succeed;
}


/* ------------------------------------------------------------------------ */
/* ------------------------- gui support routines ------------------------- */
/* ------------------------------------------------------------------------ */

/* We've just init'd a new hotlist list widget; look at the default
   hotlist and load 'er up. */
static void mo_load_hotlist_list (mo_window *win, Widget list)
{
  mo_hot_item *node;

  if (win->edithot_win && XtIsManaged(win->edithot_win))
    XtUnmanageChild (win->edithot_win);
  for (node = win->current_hotlist->nodelist; node != NULL;
       node = node->any.next)
    {
      char *highlight = NULL;
      XmString xmstr =
        XmxMakeXmstrFromString
	  (node->type == mo_t_url ?
	   (get_pref_boolean(eDISPLAY_URLS_NOT_TITLES) ? node->hot.url : node->hot.title) :
	   (highlight = mo_highlight_hotlist(&node->list)));
      if (node->type == mo_t_list && highlight)
	free(highlight);
      XmListAddItemUnselected
        (list, xmstr, 0);
      XmStringFree (xmstr);
    }

  return;
}

static void mo_visit_hotlist_position (mo_window *win, int position)
{
  mo_hot_item *hotnode;

  for (hotnode = win->current_hotlist->nodelist; hotnode != NULL;
       hotnode = hotnode->any.next)
    {
      if (hotnode->any.position == position)
	if (hotnode->type == mo_t_url)
	  mo_access_document (win, hotnode->hot.url);
	else
	  {
	    char *path = mo_compute_hot_path(&(hotnode->list));

	    win->current_hotlist = &(hotnode->list);
	    XmListDeleteAllItems(win->hotlist_list);
	    XmxTextSetString(win->hotlist_label, path);
	    free(path);
	    mo_load_hotlist_list(win, win->hotlist_list);
	    URL_Include_Set(win,0,0);
	  }
    }

  return;
}
/* ------------------------------------------------------------------------ */
/* ----------- This part deals with the Edit and Insert features ---------- */
/* ------------------------------------------------------------------------ */

/* ----------------------- edit_or_insert_hot_cb -------------------------- */

static XmxCallback (edit_or_insert_hot_cb)
{
  mo_window *win = mo_fetch_window_by_id
    (XmxExtractUniqid ((int)client_data));
  char *title;
  edit_or_insert_hot_info *eht_info;


  switch (XmxExtractToken ((int)client_data))
    {
    case 0:			/* Commit Edit */
      URL_Include_Set(win,0,0);
      XmxSetArg (XmNuserData, (XtArgVal)&eht_info);
      XtGetValues (win->edithot_win, Xmx_wargs, Xmx_n);
      Xmx_n = 0;
      XtUnmanageChild (win->edithot_win);
      title = XmxTextGetString (eht_info->title_text);

      {
        /* OK, now position is still cached in win->edithot_pos. */
        mo_hotlist *list = win->current_hotlist;
        mo_hot_item *hotnode;
	mo_window *w = NULL;

	FindHotFromPos(hotnode, list, eht_info->pos);

        if (hotnode == NULL)
          goto punt;

        /* OK, now we have the hotnode. */

	if (hotnode->type == mo_t_url)
	  hotnode->hot.url = XmxTextGetString(eht_info->url_text);
	else if (!strcmp(hotnode->any.name, title))
          goto punt;

        hotnode->any.name = title;

        /* Save the hotlist before we screw something up. */
        mo_write_default_hotlist ();

        /* Change the extant hotlists. */
	while (w = mo_next_window (w))
	  {
	    if (w->hotlist_list && w->current_hotlist == win->current_hotlist)
	      {
		char *highlight = NULL;
		XmString xmstr =
		  XmxMakeXmstrFromString
		    (hotnode->type == mo_t_url ?
		     (get_pref_boolean(eDISPLAY_URLS_NOT_TITLES)?hotnode->hot.url:title) :
		     (highlight = mo_highlight_hotlist(&hotnode->list)));
		if (hotnode->type == mo_t_list && highlight)
		  free(highlight);
		XmListDeletePos
		  (w->hotlist_list,
		   hotnode->any.position);
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf (stderr,
				 "w->hotlist_list 0x%08x, xmstr 0x%08x, hotnode->position %d\n",
				 w->hotlist_list,
				 xmstr,
				 hotnode->any.position);
		}
#endif
		/* There is what appears to be a Motif UMR here... */
		XmListAddItemUnselected
		  (w->hotlist_list,
		   xmstr,
		   hotnode->any.position);
		XmStringFree (xmstr);
              }
	    if (w->hotlist_list && hotnode->type == mo_t_list &&
		mo_is_ancestor((mo_hotlist *)hotnode, w->current_hotlist))
	      {
		char *path = mo_compute_hot_path (w->current_hotlist);
		XmxTextSetString(w->hotlist_label, path);
		free(path);
	      }
	  }

        /* That's it! */
      }
    punt:
      break;

    case 3:			/* Commit Insert */
      URL_Include_Set(win,0,0);
      XmxSetArg (XmNuserData, (XtArgVal)&eht_info);
      XtGetValues (win->inserthot_win, Xmx_wargs, Xmx_n);
      Xmx_n = 0;
      XtUnmanageChild (win->inserthot_win);
      title = XmxTextGetString (eht_info->title_text);
      {
	Boolean isUrl = XmToggleButtonGadgetGetState(eht_info->tog_url);
	Boolean useIns = XmToggleButtonGadgetGetState(eht_info->insert_tog);
        int *pos_list;
        int pos_cnt, posi = 0;
	mo_status addOk = mo_succeed;

	if (useIns)
	  if (XmListGetSelectedPos (win->hotlist_list, &pos_list, &pos_cnt) &&
	      pos_cnt)
	    {
	      posi = pos_list[0]; XtFree((char *)pos_list); /* DXP */
	    }

	if (isUrl)
	  addOk = mo_add_item_to_hotlist
	    (win->current_hotlist, mo_t_url, title,
	     XmxTextGetString(eht_info->url_text), posi,
	     get_pref_boolean(eADD_HOTLIST_ADDS_RBM));
	else
	  {
	    if (win->hot_cut_buffer &&
		win->hot_cut_buffer->type == mo_t_list &&
		(!strcmp(title, win->hot_cut_buffer->any.name)) &&
		(!mo_is_ancestor((mo_hotlist *)win->hot_cut_buffer,
				 win->current_hotlist)))
	      {
		mo_insert_item_in_hotlist
		  (win->current_hotlist,
		   (mo_hot_item *)mo_copy_hot_hier
		   ((mo_hotlist *)win->hot_cut_buffer), posi);
		/* Now we've got to update all active hotlist_list's. */
		mo_gui_add_hot_item
		  (win->current_hotlist, win->current_hotlist->nodelist_last);
	      }
	    else
	      addOk = mo_add_item_to_hotlist(win->current_hotlist, mo_t_list,
					     title, NULL, posi,
					     get_pref_boolean(eADD_HOTLIST_ADDS_RBM));
	  }
	if (addOk == mo_succeed)
	  mo_write_default_hotlist ();
      }
      win->hot_cut_buffer = NULL;
      /* AMB do a redisplay here */
      mo_compute_hot_path(win->current_hotlist);
      XmListDeleteAllItems(win->hotlist_list);
      mo_load_hotlist_list(win, win->hotlist_list);
      /* /AMB */

      break;

    case 1:			/* Dismiss Edit */
      XtUnmanageChild (win->edithot_win);
      /* Do nothing. */
      break;

    case 4:			/* Dismiss Insert */
      XtUnmanageChild (win->inserthot_win);
      win->hot_cut_buffer = NULL;
      /* Do nothing. */
      break;

    case 2:			/* Help... (Edit) */
    case 5:			/* Help... (Insert) */
      mo_open_another_window
        (win,
         mo_assemble_help_url ("help-on-hotlist-view.html"),
         NULL, NULL);
      break;
    }

  return;
}

/* this is used to destroy the edit_or_insert_hot_info structure
   called from the "destroyCallback" list. */
static XmxCallback (mo_destroy_hot)
{
  free (client_data);
}

/* show or hide the url info with respect to the URL toggle */
static XmxCallback (url_or_list_cb)
{
  edit_or_insert_hot_info *eht_info = (edit_or_insert_hot_info *)client_data;
  if (((XmToggleButtonCallbackStruct *)call_data)->set)
    {
      XtManageChild(eht_info->url_lab);
      XtManageChild(eht_info->url_text);
    }
  else
    {
      XtUnmanageChild(eht_info->url_lab);
      XtUnmanageChild(eht_info->url_text);
    }
}


/* If it don't exist, make it...
   If isInsert is True, then we create an Insert Dialog window, otherwise
   we create an Edit dialog window. */
static mo_status mo_create_ed_or_ins_hot_win (mo_window *win, int isInsert)
{
  Widget ed_or_ins_w, dialog_frame;
  Widget dialog_sep, buttons_form;
  Widget eht_form, title_label, url_label, url_val, sep2;
  edit_or_insert_hot_info *eht_info;
  Widget togm, togm2, insert_tog, append_tog;

  XmxSetUniqid (win->id);
  eht_info = (edit_or_insert_hot_info *)
    malloc(sizeof(edit_or_insert_hot_info));
  XmxSetArg (XmNuserData, (XtArgVal)eht_info);
  ed_or_ins_w = XmxMakeFormDialog
    (win->hotlist_win, isInsert ? "NCSA Mosaic: Insert Hotlist Entry"  :
     "NCSA Mosaic: Edit Hotlist Entry" );
  XtAddCallback(ed_or_ins_w, XmNdestroyCallback, mo_destroy_hot,
		eht_info);

  if (isInsert)
    win->inserthot_win = ed_or_ins_w;
  else
    win->edithot_win = ed_or_ins_w;

  dialog_frame = XmxMakeFrame (ed_or_ins_w, XmxShadowOut);

  /* Constraints for ed_or_ins_w. */
  XmxSetConstraints
    (dialog_frame, XmATTACH_FORM, XmATTACH_FORM,
     XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);

  /* Main form. */
  eht_form = XmxMakeForm (dialog_frame);

  title_label = XmxMakeLabel (eht_form, "Entry Title:" );
  XmxSetArg (XmNwidth, 335);
  eht_info->title_text = XmxMakeTextField (eht_form);
  XmxAddCallbackToText (eht_info->title_text, edit_or_insert_hot_cb,
			isInsert*3);

  eht_info->url_lab =
    url_label = XmxMakeLabel (eht_form, "URL:" );

  XmxSetArg (XmNwidth, 335);
  eht_info->url_text =
    url_val = XmxMakeTextField(eht_form);

  dialog_sep = XmxMakeHorizontalSeparator (eht_form);

  if (isInsert)
    {
      togm = XmxMakeRadioBox(eht_form);
      eht_info->tog_url =
	XtVaCreateManagedWidget("toggle", xmToggleButtonGadgetClass, togm,
				XtVaTypedArg, XmNlabelString,
				XtRString, "URL" , strlen("URL" )+1,
				XmNmarginHeight, 0,
				XmNset, True, NULL);
      XtAddCallback(eht_info->tog_url, XmNvalueChangedCallback, url_or_list_cb,
		    (XtPointer)eht_info);
      eht_info->tog_list =
	XtVaCreateManagedWidget("toggle", xmToggleButtonGadgetClass, togm,
				XtVaTypedArg, XmNlabelString,
				XtRString, "List" , strlen("List" )+1,
				XmNmarginHeight, 0, NULL);
      togm2 = XmxMakeRadioBox(eht_form);
      eht_info->insert_tog = insert_tog =
	XtVaCreateManagedWidget("toggle", xmToggleButtonGadgetClass, togm2,
				XtVaTypedArg, XmNlabelString,
				XtRString, "Insert" , strlen("Insert" )+1,
				XmNmarginHeight, 0, NULL);
      append_tog =
	XtVaCreateManagedWidget("toggle", xmToggleButtonGadgetClass, togm2,
				XtVaTypedArg, XmNlabelString,
				XtRString, "Append" , strlen("Append" )+1,
				XmNmarginHeight, 0,
				XmNset, True, NULL);
      sep2 = XmxMakeHorizontalSeparator (eht_form);
    }

  buttons_form = XmxMakeFormAndThreeButtons
    (eht_form, edit_or_insert_hot_cb, "Save" ,
     "Dismiss" , "Help..." ,
     isInsert*3, isInsert*3+1, isInsert*3+2);

  /* Constraints for eht_form. */
  XmxSetOffsets (title_label, 14, 0, 10, 0);
  XmxSetConstraints
    (title_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetOffsets (eht_info->title_text, 10, 0, 5, 10);
  XmxSetConstraints
    (eht_info->title_text, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_WIDGET,
     XmATTACH_FORM, NULL, NULL, title_label, NULL);

  XmxSetOffsets (url_label, 12, 0, 10, 0);
  XmxSetConstraints
    (url_label, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_NONE,
     title_label, NULL, NULL, NULL);
  XmxSetOffsets (url_val, 8, 10, 5, 10);
  XmxSetConstraints
    (url_val, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_WIDGET,
     XmATTACH_FORM, title_label, NULL, url_label, NULL);

  XmxSetArg (XmNtopOffset, 10);
  XmxSetConstraints
    (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM,
     XmATTACH_FORM,
     url_val, isInsert ? togm : buttons_form, NULL, NULL);
  if (isInsert)
    {
      XmxSetConstraints
	(togm, XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM,
	 XmATTACH_NONE,
	 NULL, sep2, NULL, NULL);
      XmxSetPositions
	(togm, XmxNoPosition, XmxNoPosition, XmxNoPosition, 50);
      XmxSetConstraints
	(togm2, XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_NONE,
	 XmATTACH_FORM,
	 NULL, sep2, NULL, NULL);
      XmxSetPositions
	(togm2, XmxNoPosition, XmxNoPosition, 50, XmxNoPosition);
      XmxSetConstraints
	(sep2, XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM,
	 XmATTACH_FORM, NULL, buttons_form, NULL, NULL);
    }
  XmxSetConstraints
    (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
     XmATTACH_FORM,
     NULL, NULL, NULL, NULL);

  return mo_succeed;
}


static mo_status mo_do_edit_hotnode_title_win (mo_window *win, mo_hot_item
					       *item, int position)
{
  edit_or_insert_hot_info *eht_info;

  /* This shouldn't happen. */
  if (!win->hotlist_win)
    return mo_fail;

  if (!win->edithot_win)
    mo_create_ed_or_ins_hot_win (win, 0);

  XmxSetArg (XmNuserData, (XtArgVal)&eht_info);
  XtGetValues (win->edithot_win, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  /* Cache the position. */
  eht_info->pos = position;

  /* Manage the little sucker. */
  XmxManageRemanage (win->edithot_win);

  /* Insert this title as a starting point. */
  XmxTextSetString (eht_info->title_text, item->hot.title);

  if (item->type == mo_t_url)
    {
      /* Insert URL */
      XmxTextSetString (eht_info->url_text, item->hot.url);
      XtManageChild(eht_info->url_lab);
      XtManageChild(eht_info->url_text);
    }
  else
    {
      XtUnmanageChild(eht_info->url_lab);
      XtUnmanageChild(eht_info->url_text);
    }

  return mo_succeed;
}


/*
 * Edit the title of an element of the current hotlist.
 * The element is referenced by its position.
 * Algorithm for edit:
 *   Find hotnode with the position.
 *   Change the title.
 *   Cause redisplay.
 * Return status.
 */
static mo_status mo_edit_title_in_current_hotlist (mo_window *win,
						   int position)
{
  mo_hotlist *list = win->current_hotlist;
  mo_hot_item *hotnode;

  FindHotFromPos(hotnode, list, position);

  /* OK, now we have hotnode loaded. */
  /* hotnode->hot.title is the current title.
     hotnode->hot.position is the current position. */
  return
    ((hotnode != NULL) ?
     mo_do_edit_hotnode_title_win (win, hotnode, position) :
     mo_fail);
}


void RecursiveSetList(mo_window *win, mo_hotlist *list, int val, int force) {

static char *add="Do you wish to add all items in this list to the RBM?";
static char *remove="Do you wish to remove all items in this list from the RBM?";
mo_hot_item *item;

	if (force || prompt_for_yes_or_no((val?add:remove))) {
		for (item=list->nodelist; item; item=item->any.next) {
			if (item->type==mo_t_url) {
				item->hot.rbm=val;
			}
			else {
				item->list.rbm=val;
				RecursiveSetList(win,&(item->list),val,1);
			}
		}
	}

	return;
}


/*
 * Either add or remove this entry from the RBM.
 * The element is referenced by its position.
 * Algorithm:
 *   Find hotnode with the position.
 *   Change the RBM status.
 *   Cause rebuild of RBM.
 * Return status.
 */
static mo_status mo_rbm_toggle_in_hotlist(mo_window *win,int position) {

mo_hotlist *list = win->current_hotlist;
mo_hot_item *hotnode;

	FindHotFromPos(hotnode, list, position);

	if (!hotnode) { /* How did this happen? */
		return(mo_fail);
	}

	/*
	 * Set the new rbm value.
	 * Redisplay and rebuild the RBM.
	 */
	if (hotnode->type==mo_t_url) {
		hotnode->hot.rbm=(!hotnode->hot.rbm);
	}
	else {
		hotnode->list.rbm=(!hotnode->list.rbm);
		RecursiveSetList(win,&hotnode->list,hotnode->list.rbm,0);
	}
	XmxSetToggleButton(win->hotlist_rbm_toggle,
			   (hotnode->type==mo_t_url?
			    hotnode->hot.rbm:
			    hotnode->list.rbm));

	mo_reinit_hotmenu();

	return(mo_succeed);
}

static void mo_insert_item_in_current_hotlist(mo_window *win)
{
  if (win->hotlist_win)
    {
      edit_or_insert_hot_info *eht_info;

      if (!win->inserthot_win)
	mo_create_ed_or_ins_hot_win(win, 1);

      XmxSetArg (XmNuserData, (XtArgVal)&eht_info);
      XtGetValues (win->inserthot_win, Xmx_wargs, Xmx_n);
      Xmx_n = 0;
      /* Manage the little sucker. */
      XmxManageRemanage (win->inserthot_win);

      if (win->hot_cut_buffer)
	{
	  /* Insert this title as a starting point. */
	  XmxTextSetString (eht_info->title_text,
			    win->hot_cut_buffer->any.name);
	  if (win->hot_cut_buffer->type == mo_t_url)
	    {
	      /* Insert URL */
	      XmxTextSetString (eht_info->url_text,
				win->hot_cut_buffer->hot.url);
	      XtManageChild(eht_info->url_lab);
	      XtManageChild(eht_info->url_text);
	      XmToggleButtonGadgetSetState(eht_info->tog_list, False, False);
	      XmToggleButtonGadgetSetState(eht_info->tog_url, True, False);
	    }
	  else
	    {
	      /* Insert a List */
	      XtUnmanageChild(eht_info->url_lab);
	      XtUnmanageChild(eht_info->url_text);
	      XmToggleButtonGadgetSetState(eht_info->tog_list, True, False);
	      XmToggleButtonGadgetSetState(eht_info->tog_url, False, False);
	    }
	}
      else
	{
	  XmTextFieldSetString(eht_info->title_text, "");
	  XmTextFieldSetString(eht_info->url_text, "");
	}
    }

    mo_reinit_hotmenu();

}


/*
 * Create a new mo_root_hotlist.
 * Pass in the new filename and the new title.
 */
static mo_root_hotlist *mo_new_root_hotlist (char *filename, char *title)
{
  mo_root_hotlist *list;

  list = (mo_root_hotlist *)malloc (sizeof (mo_root_hotlist));
  list->type = mo_t_list;
  list->nodelist = list->nodelist_last = 0;
  list->filename = filename;
  list->modified = 1;
  list->next = list->previous = 0;
  list->parent = 0;
  list->name = title ? strdup (title) : title;
  return list;
}

/* --------------------------- mo_read_hotlist ---------------------------- */

/*
 * Read a hotlist from a file.
 * Return pointer to a mo_hotlist structure, fully loaded
 * and ready to go.
 * Return NULL if file does not exist or is not readable.
 */
static mo_root_hotlist *mo_read_hotlist (char *filename, char *home)
{
  mo_root_hotlist *list = NULL;
  FILE *fp;
  char line[MO_LINE_LENGTH];
  char *status, *name;
  int isnew;
  char *oldfilename/*,*tmpfilename*/;
  char *hotname;
  char *tmp=get_pref_string(eDEFAULT_HOT_FILE);

  hotname=(char *)calloc(strlen(home)+strlen(tmp)+5,sizeof(char));
  sprintf(hotname,"%s/%s",home,tmp);

  oldfilename = filename;
  if (! (filename= malloc(strlen(filename) + 10)))
    goto screwed_no_file;
  sprintf(filename,"%s.html",oldfilename);
  /* for backward compatibility */

  /* oldfilename: cookie 1
     filename: cookie 2
     hotname: cookie 3 */

  /* cookie 2 and cookie 3 are very similar, but format 2 browsers cannot
	read format 3 without damaging them.
     The cool thing is if we only have format 2, format 3 will automatically
	be written out to the format 3 filename...so we don't have to worry
	about converting anything. Groovy. --SWP */

  if (!(fp=fopen(hotname,"r"))) {
	fp = fopen (filename, "r");
	if (!fp) {
		fp = fopen (oldfilename, "r");
		if (!fp) {
			goto screwed_no_file;
		}
		else if (get_pref_boolean(eBACKUP_FILES)) {
			char *tf=NULL,retBuf[BUFSIZ];

			tf=(char *)calloc(strlen(oldfilename)+strlen(".backup")+5,sizeof(char));
			sprintf(tf,"%s.backup",oldfilename);
			if (my_copy(oldfilename,tf,retBuf,BUFSIZ-1,1)!=SYS_SUCCESS) {
				fprintf(stderr,"%s\n",retBuf);
			}
			free(tf);
		}
	}
	else if (get_pref_boolean(eBACKUP_FILES)) {
		char *tf=NULL,retBuf[BUFSIZ];

		tf=(char *)calloc(strlen(filename)+strlen(".backup")+5,sizeof(char));
		sprintf(tf,"%s.backup",filename);
		if (my_copy(filename,tf,retBuf,BUFSIZ-1,1)!=SYS_SUCCESS) {
			fprintf(stderr,"%s\n",retBuf);
		}
		free(tf);
	}
  }
  else if (get_pref_boolean(eBACKUP_FILES)) {
	char *tf=NULL,retBuf[BUFSIZ];

	tf=(char *)calloc(strlen(hotname)+strlen(".backup")+5,sizeof(char));
	sprintf(tf,"%s.backup",hotname);
	if (my_copy(hotname,tf,retBuf,BUFSIZ-1,1)!=SYS_SUCCESS) {
		fprintf(stderr,"%s\n",retBuf);
	}
	free(tf);
  }

  status = fgets (line, MO_LINE_LENGTH, fp);
  if (!status || !(*line)) {
    goto screwed_open_file;
  }

  /* See if it's our format. */
  if (!strncmp (line, NCSA_HOTLIST_FORMAT_COOKIE_ONE,
		strlen (NCSA_HOTLIST_FORMAT_COOKIE_ONE))) {
    isnew = 0;
  }
  else {
	status = fgets (line, MO_LINE_LENGTH, fp);
	if (!status || !(*line)) {
		goto screwed_open_file;
	}

	if (!strncmp(line, NCSA_HOTLIST_FORMAT_COOKIE_TWO,
		     strlen (NCSA_HOTLIST_FORMAT_COOKIE_TWO))) {
		isnew = 1;
	}
	else if (!strncmp(line, NCSA_HOTLIST_FORMAT_COOKIE_THREE,
			  strlen (NCSA_HOTLIST_FORMAT_COOKIE_THREE))) {
		isnew = 2;
	}
	else {
		fprintf(stderr,"Unknown hotlist format. Attempting to parse. This\n  could result in damage to this file.\n");
		isnew = 2;
	}

	rewind(fp);
	status = fgets (line, MO_LINE_LENGTH, fp);
	if (!status || !(*line)) {
		goto screwed_open_file;
	}
  }

  if (isnew)
    {
      list = mo_new_root_hotlist(hotname, NULL);
      list->name = mo_read_new_hotlist((mo_hotlist *)list, fp);
      if (isnew==1) {
	fprintf(stderr,"Your hotlist has been updated to a new format!\n  It is now called '.mosaic/hot.html'.\n");
      }
      goto done;
    }
  /* Go fetch the name on the next line. */
  status = fgets (line, MO_LINE_LENGTH, fp);
  if (!status || (!*line))
    goto screwed_open_file;
  name = strtok (line, "\n");
  if (!name)
    goto screwed_open_file;

  /* amb - display update message for 2.4 users */
  {
    fputs("Your hotlist file has been updated and is now saved as:\n",
	  stderr);
    fputs(filename, stderr);
    putc('\n', stderr);
  }
  /* Hey, whaddaya know, it is. */
  list = mo_new_root_hotlist (filename, name);

  /* Start grabbing documents. */
  while (1)
    {
      mo_hotnode *node;

      status = fgets (line, MO_LINE_LENGTH, fp);
      if (!status || !(*line))
        goto done;

      /* We've got a new node. */
      node = (mo_hotnode *)malloc (sizeof (mo_hotnode));
      node->type = mo_t_url;
      node->url = strtok (line, " ");
      if (!node->url)
        goto screwed_open_file;
      node->url = strdup (node->url);
      mo_convert_newlines_to_spaces (node->url);

      node->lastdate = strtok (NULL, "\n");
      if (!node->lastdate) {
        goto screwed_open_file;
      }
      node->lastdate = strdup (node->lastdate);

      status = fgets (line, MO_LINE_LENGTH, fp);
      if (!status || !(*line))
        {
          /* Oops, something went wrong. */
          free (node->url);
	  if (node->lastdate) {
	    free (node->lastdate);
	    }
          free (node);
          goto done;
        }

      node->title = strtok (line, "\n");
      if (!node->title)
        goto screwed_open_file;
      node->title = strdup (node->title);
      mo_convert_newlines_to_spaces (node->title);

      mo_append_item_to_hotlist ((mo_hotlist *)list, (mo_hot_item *)node);
    }

 done:
  fclose (fp);
  return list;

 screwed_open_file:
  fclose (fp);

 screwed_no_file:
  return list;
}


#if 0
/*
 * Write a hotlist out to stdout.
 * Return mo_succeed if everything goes OK;
 * mo_fail else.
 */
mo_status mo_dump_hotlist (mo_hotlist *list)
{
  mo_write_hotlist (list, stdout);

  return mo_succeed;
}
#endif

/* ------------------------------------------------------------------------ */
/* ----------------------------- HOTLIST GUI ------------------------------ */
/* ------------------------------------------------------------------------ */

/* Initial GUI support for hotlist will work like this:

   There will be a single hotlist, called 'Default'.
   It will be persistent across all windows.

   Upon program startup an attempt will be made to load it out
   of its file; if this attempt isn't successful, it just plain
   doesn't exist yet.  Bummer.

   Upon program exit it will be stored to its file.
*/

/*
 * Called on initialization.
 * Tries to load the default hotlist.
 */
mo_status mo_setup_default_hotlist (void)
{
  char *home = getenv ("HOME");
  char *default_filename = get_pref_string(eDEFAULT_HOTLIST_FILE);
  char *hot_filename = get_pref_string(eDEFAULT_HOT_FILE);
  char *filename;

  /* This shouldn't happen. */
  if (!home)
    home = "/tmp";

  filename = (char *)malloc
    ((strlen (home) + strlen (default_filename) + 8) * sizeof (char));
  sprintf (filename, "%s/%s", home, default_filename);

  /* Try to load the default hotlist. */
  default_hotlist = mo_read_hotlist (filename, home);
  /* Doesn't exist?  Bummer.  Make a new one. */
  if (!default_hotlist)
    {
      fprintf(stderr,"Could not find a hotlit. Creating a new one.\n");
      /* amb - doesn't have any hotlist, add the .html extension (ugh) */
/*      sprintf(filename, "%s/%s.html", home, default_filename); */
/* New hotlist format... SWP */
      free(filename);
      filename = (char *)malloc
	((strlen (home) + strlen (hot_filename) + 8) * sizeof (char));
      sprintf (filename, "%s/%s", home, hot_filename);
      default_hotlist = mo_new_root_hotlist (filename, "Default");
    }

  return mo_succeed;
}

/*
 * Called on program exit.
 * Tries to write the default hotlist.
 */
mo_status mo_write_default_hotlist (void)
{
  FILE *fp = fopen (default_hotlist->filename, "w");

  if (!fp)
    return mo_fail;

  mo_write_hotlist ((mo_hotlist *)default_hotlist, fp);
  if (fclose (fp))
    return mo_fail;

  default_hotlist->modified = 0;
  return mo_succeed;
}

static XmxCallback (save_hot_cb)
{
  char *fname = NULL, efname[MO_LINE_LENGTH];
  FILE *fp;
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  XtUnmanageChild (win->save_hotlist_win);
  mo_busy ();
  XmStringGetLtoR (((XmFileSelectionBoxCallbackStruct *)call_data)->value,
		   XmSTRING_DEFAULT_CHARSET, &fname);
  pathEval (efname, fname);
  fp = fopen (efname, "w");
  if (!fp)
    {
	char *buf, *final, tmpbuf[80];
	int final_len;

	buf=my_strerror(errno);
	if (!buf || !*buf || !strcmp(buf,"Error 0")) {
		sprintf(tmpbuf,"Unknown Error");
		buf=tmpbuf;
	}

	final_len=30+((!efname || !*efname?3:strlen(efname))+13)+15+(strlen(buf)+13);
	final=(char *)calloc(final_len,sizeof(char));

	sprintf(final,"\nUnable to save hotlist:\n   %s\n\nSave Error:\n   %s\n" ,(!efname||!*efname?" ":efname),buf);

	XmxMakeErrorDialog (win->save_hotlist_win,
			  final, "Save Error" );
	XtManageChild (Xmx_w);

	if (final) {
		free(final);
		final=NULL;
	}
    }
  else
    {
      mo_write_hotlist (win->current_hotlist, fp);
      fclose(fp);
    }
  mo_not_busy ();
  free (fname);
}

static XmxCallback (load_hot_cb)
{
  char *fname = NULL, efname[MO_LINE_LENGTH];
  FILE *fp;
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  XtUnmanageChild (win->load_hotlist_win);
  mo_busy ();
  XmStringGetLtoR (((XmFileSelectionBoxCallbackStruct *)call_data)->value,
		   XmSTRING_DEFAULT_CHARSET, &fname);
  pathEval (efname, fname);
  fp = fopen (efname, "r");
  if (!fp)
    {
	char *buf, *final, tmpbuf[80];
	int final_len;

	buf=my_strerror(errno);
	if (!buf || !*buf || !strcmp(buf,"Error 0")) {
		sprintf(tmpbuf,"Unknown Error");
		buf=tmpbuf;
	}

	final_len=30+((!efname || !*efname?3:strlen(efname))+13)+15+(strlen(buf)+13);
	final=(char *)calloc(final_len,sizeof(char));

	sprintf(final,"\nUnable to open hotlist:\n   %s\n\nOpen Error:\n   %s\n" ,(!efname||!*efname?" ":efname),buf);

	XmxMakeErrorDialog (win->load_hotlist_win,
			  final, "Open Error" );
	XtManageChild (Xmx_w);

	if (final) {
		free(final);
		final=NULL;
	}
    }
  else
    {
      Widget tb;

      XmxSetArg (XmNuserData, (XtArgVal)&tb);
      XtGetValues (win->load_hotlist_win, Xmx_wargs, Xmx_n);
      Xmx_n = 0;
      if (XmToggleButtonGadgetGetState (tb))
	{
	  mo_hotlist *list = (mo_hotlist *)malloc (sizeof(mo_hotlist));

	  list->type = mo_t_list;
	  list->nodelist = list->nodelist_last = 0;
	  list->name = mo_read_new_hotlist (list, fp);
	  if (list->name == NULL)
	    list->name = strdup("Unnamed" );
	  mo_append_item_to_hotlist(win->current_hotlist, (mo_hot_item *)list);
	  mo_gui_add_hot_item (win->current_hotlist, (mo_hot_item *)list);
	}
      else
	{
	  mo_hot_item *item = win->current_hotlist->nodelist_last;

	  mo_read_new_hotlist (win->current_hotlist, fp);
	  if (item == NULL)
	    item = win->current_hotlist->nodelist;
	  else
	    item = item->any.next;
	  for (;item; item = item->any.next)
	    mo_gui_add_hot_item (win->current_hotlist, item);
	}
      fclose (fp);
      default_hotlist->modified = 1;
      mo_write_default_hotlist ();
      URL_Include_Set(win,0,0);
    }
  mo_not_busy ();
  free (fname);
}

/* --------------- mo_delete_position_from_current_hotlist ---------------- */
/*
 * Delete an element of the default hotlist.
 * The element is referenced by its position.
 * Algorithm for removal:
 *   Find hotnode with the position.
 *   If it is a list, change the current list of the windows that has hotnode
 *	as an ancestor.
 *   Remove the hotnode from the hotlist data structure.
 *   Recalculate positions of the hotlist.
 *   Remove the element in the position in the list widgets.
 * Return status.
 */
static void delete_hot_from_list (mo_hotlist *list, mo_hot_item *hotnode,
				  int position)
{
  mo_window *win = NULL;

  if (hotnode == NULL)
    return;
  if (hotnode->type == mo_t_list)
    while (win = mo_next_window (win))
      {
	if (win->hotlist_list &&
	    mo_is_ancestor (&(hotnode->list), win->current_hotlist))
	  {
	    char *path = mo_compute_hot_path(list);

	    XmListDeleteAllItems(win->hotlist_list);
	    win->current_hotlist = list;
	    XmxTextSetString(win->hotlist_label, path);
	    free(path);
	    mo_load_hotlist_list(win, win->hotlist_list);
	  }
      }
  /* Pull the hotnode out of the hotlist. */
  mo_remove_hotnode_from_hotlist (list, hotnode);
  free (hotnode);
  /* Recalculate positions in this hotlist. */
  mo_recalculate_hotlist_positions (list);

  /* Do the GUI stuff. */
  while (win = mo_next_window (win))
    {
      if (win->hotlist_list && win->current_hotlist == list)
        XmListDeletePos (win->hotlist_list, position);
      if (win->hot_cut_buffer == hotnode)
	win->hot_cut_buffer = NULL;
    }
    mo_reinit_hotmenu();

}

static XmxCallback (remove_confirm_cb)
{
  mo_window *win = mo_fetch_window_by_id
    (XmxExtractUniqid ((int)client_data));
  int position = XmxExtractToken ((int)client_data);

  if (position)
    {
      mo_hot_item *hotnode;
      FindHotFromPos(hotnode, win->current_hotlist, position);
      delete_hot_from_list(win->current_hotlist, hotnode, position);
      URL_Include_Set(win,0,0);
    }
  XtDestroyWidget(w);
}

static mo_status mo_delete_position_from_current_hotlist (mo_window *win,
							  int position)
{
  mo_hotlist *list = win->current_hotlist;
  mo_hot_item *hotnode;

  FindHotFromPos(hotnode, list, position);

  if (hotnode == NULL)
    return mo_fail;

  /* OK, now we have hotnode loaded. */

  if (hotnode->type == mo_t_list)
    {
      char *question;
      char *endquestion;
      char *buff;

      question=strdup("Are you sure you want to remove the \"" );
      endquestion=strdup("\" list?" );
      buff = (char *)malloc
	(strlen(question)+strlen(hotnode->list.name)+strlen(endquestion)+1);

      strcat(strcat(strcpy(buff, question), hotnode->list.name), endquestion);
      XmxSetUniqid (win->id);
      XmxMakeQuestionDialog
	(win->hotlist_win, buff, "NCSA Mosaic: Remove list" ,
	 remove_confirm_cb, position, 0);
      free(buff);
      free(question);
      free(endquestion);
      XtManageChild (Xmx_w);
    }
  else {
    delete_hot_from_list(list, hotnode, position);
    URL_Include_Set(win,0,0);
  }

  mo_reinit_hotmenu();
  return mo_succeed;
}


/* ----------------------------- mail hotlist ----------------------------- */

static XmxCallback (mailhot_win_cb)
{
  mo_window *win = mo_fetch_window_by_id
    (XmxExtractUniqid ((int)client_data));
  char *to, *subj;
  FILE *fp;

  switch (XmxExtractToken ((int)client_data))
    {
    case 0:
      XtUnmanageChild (win->mailhot_win);

      mo_busy ();

      to = XmxTextGetString (win->mailhot_to_text);
      if (!to)
        return;
      if (to[0] == '\0')
        return;

      subj = XmxTextGetString (win->mailhot_subj_text);

      /* Open a file descriptor to sendmail. */
      fp = mo_start_sending_mail_message (to, subj, "text/x-html", NULL);
      if (!fp)
        goto oops;

      {
	mo_write_hotlist(win->current_hotlist, fp);
      }

      mo_finish_sending_mail_message ();

    oops:
      free (to);
      free (subj);

      mo_not_busy ();

      break;
    case 1:
      XtUnmanageChild (win->mailhot_win);
      /* Do nothing. */
      break;
    case 2:
      mo_open_another_window
        (win,
         mo_assemble_help_url ("help-on-hotlist-view.html"),
         NULL, NULL);
      break;
    }

  return;
}

static mo_status mo_post_mailhot_win (mo_window *win)
{
  /* This shouldn't happen. */
  if (!win->hotlist_win)
    return mo_fail;

  if (!win->mailhot_win)
    {
      Widget dialog_frame;
      Widget dialog_sep, buttons_form;
      Widget mailhot_form, to_label, subj_label;

      /* Create it for the first time. */
      XmxSetUniqid (win->id);
      win->mailhot_win = XmxMakeFormDialog
        (win->hotlist_win, "NCSA Mosaic: Mail Hotlist" );
      dialog_frame = XmxMakeFrame (win->mailhot_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM,
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);

      /* Main form. */
      mailhot_form = XmxMakeForm (dialog_frame);

      to_label = XmxMakeLabel (mailhot_form, "Mail To:" );
      XmxSetArg (XmNwidth, 335);
      win->mailhot_to_text = XmxMakeTextField (mailhot_form);

      subj_label = XmxMakeLabel (mailhot_form, "Subject:" );
      win->mailhot_subj_text = XmxMakeTextField (mailhot_form);

      dialog_sep = XmxMakeHorizontalSeparator (mailhot_form);

      buttons_form = XmxMakeFormAndThreeButtons
        (mailhot_form, mailhot_win_cb, "Mail" ,
	 "Dismiss" , "Help..." , 0, 1, 2);

      /* Constraints for mailhot_form. */
      XmxSetOffsets (to_label, 14, 0, 10, 0);
      XmxSetConstraints
        (to_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_NONE,
         NULL, NULL, NULL, NULL);
      XmxSetOffsets (win->mailhot_to_text, 10, 0, 5, 10);
      XmxSetConstraints
        (win->mailhot_to_text, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_WIDGET,
         XmATTACH_FORM, NULL, NULL, to_label, NULL);

      XmxSetOffsets (subj_label, 14, 0, 10, 0);
      XmxSetConstraints
        (subj_label, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM,
         XmATTACH_NONE,
         win->mailhot_to_text, NULL, NULL, NULL);
      XmxSetOffsets (win->mailhot_subj_text, 10, 0, 5, 10);
      XmxSetConstraints
        (win->mailhot_subj_text, XmATTACH_WIDGET,
         XmATTACH_NONE, XmATTACH_WIDGET,
         XmATTACH_FORM, win->mailhot_to_text, NULL, subj_label, NULL);

      XmxSetArg (XmNtopOffset, 10);
      XmxSetConstraints
        (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM,
         XmATTACH_FORM,
         win->mailhot_subj_text, buttons_form, NULL, NULL);
      XmxSetConstraints
        (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
         XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
    }

  XtManageChild (win->mailhot_win);

  return mo_succeed;
}


static XmxCallback(hotlist_rbm_toggle_cb) {

mo_window *win;
Boolean rv;
int *pos_list;
int pos_cnt;
mo_hotlist *list;
mo_hot_item *hotnode;

	win=current_win;
	if (!win) {
		return;
	}

	list=win->current_hotlist;
	rv = XmListGetSelectedPos (win->hotlist_list, &pos_list, &pos_cnt);
        if (rv && pos_cnt) {
		FindHotFromPos(hotnode, list, pos_list[0]);
		if (!hotnode) {
			return;
		}
		URL_Include_Set(win,
				(hotnode->type==mo_t_url?
				 hotnode->hot.rbm:
				 hotnode->list.rbm),
				1);
	}
        else {
		XmxMakeErrorDialog
			(win->hotlist_win, "No entry in the hotlist is currently selected.\n\nTo go to an entry in the hotlist,\nselect it with a single mouse click\nand press the Go To button again." , "Error: Nothing Selected" );
		XtManageChild (Xmx_w);
	}

	return;
}

/* ---------------------------- hotlist_win_cb ---------------------------- */

static XmxCallback (hotlist_win_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  switch (XmxExtractToken ((int)client_data))
    {
    case 0:
      XtUnmanageChild (win->hotlist_win);
      /* Dismissed -- do nothing. */
      break;
    case 1:
      mo_post_mailhot_win (win);
      break;
    case 2:
      mo_open_another_window
        (win,
         mo_assemble_help_url ("help-on-hotlist-view.html"),
         NULL, NULL);
      break;
    case 3:
      /* Add current. */
      if (win->current_node)
        {
          mo_add_node_to_current_hotlist (win);
          mo_write_default_hotlist ();
	  URL_Include_Set(win,0,0);
        }
      break;
    case 4:
      /* Goto selected. */
      {
        Boolean rv;
        int *pos_list;
        int pos_cnt;
        rv = XmListGetSelectedPos (win->hotlist_list, &pos_list, &pos_cnt);
        if (rv && pos_cnt)
          {
            mo_visit_hotlist_position (win, pos_list[0]);
          }
        else
          {
            XmxMakeErrorDialog
              (win->hotlist_win, "No entry in the hotlist is currently selected.\n\nTo go to an entry in the hotlist,\nselect it with a single mouse click\nand press the Go To button again." , "Error: Nothing Selected" );
            XtManageChild (Xmx_w);
          }
      }
      break;
    case 5:
      /* Remove selected. */
      {
        Boolean rv;
        int *pos_list;
        int pos_cnt;
        rv = XmListGetSelectedPos (win->hotlist_list, &pos_list, &pos_cnt);
        if (rv && pos_cnt)
          {
            mo_delete_position_from_current_hotlist (win,
						     pos_list[0]);
            mo_write_default_hotlist ();
          }
        else
          {
            XmxMakeErrorDialog
              (win->hotlist_win, "No entry in the hotlist is currently selected.\n\nTo remove an entry in the hotlist,\nselect it with a single mouse click\nand press the Remove button again." , "Error: Nothing Selected" );
            XtManageChild (Xmx_w);
          }
      }
      break;
    case 6:
      /* Edit title of selected. */
      {
        Boolean rv;
        int *pos_list;
        int pos_cnt;
        rv = XmListGetSelectedPos (win->hotlist_list, &pos_list, &pos_cnt);
        if (rv && pos_cnt)
          {
            mo_edit_title_in_current_hotlist (win, pos_list[0]);
	    XtFree((char *)pos_list); /* DXP */
            /* Writing the default hotlist should take place in the callback. */
            /* mo_write_default_hotlist (); */
          }
        else
          {
            XmxMakeErrorDialog
              (win->hotlist_win, "No entry in the hotlist is currently selected.\n\nTo edit an entry in the hotlist,\nselect it with a single mouse click\nand press the Edit button again." , "Error: Nothing Selected" );
            XtManageChild (Xmx_w);
          }
      }
      break;
    case 7:
      /* Copy item to cut buffer */
      {
        int *pos_list;
        int pos_cnt;

        if (XmListGetSelectedPos (win->hotlist_list, &pos_list, &pos_cnt) &&
	    pos_cnt)
	  {
	    mo_copy_hotlist_position(win, pos_list[0]);
	    XtFree((char *)pos_list); /* DXP */
	  }
        else
          {
            XmxMakeErrorDialog
              (win->hotlist_win, "No entry in the hotlist is currently selected.\n\nTo copy an entry in the hotlist,\nselect it with a single mouse click\nand press the Copy button again." , "Error: Nothing Selected" );
            XtManageChild (Xmx_w);
          }
      }
      break;
    case 8:
      /* Insert an Item in the current hotlist */
      mo_insert_item_in_current_hotlist(win);
      break;
    case 9:
      /* Go Up one level */
      if (win->current_hotlist->parent != 0) {
	char *path = mo_compute_hot_path(win->current_hotlist->parent);

	XmListDeleteAllItems(win->hotlist_list);
	win->current_hotlist = win->current_hotlist->parent;
	XmxTextSetString(win->hotlist_label, path);
	free(path);
	mo_load_hotlist_list(win, win->hotlist_list);
	URL_Include_Set(win,0,0);
      }
      break;
    case 10:
      /* Save in a file */
      XmxSetUniqid (win->id);
      if (!win->save_hotlist_win)
	win->save_hotlist_win = XmxMakeFileSBDialog
	  (win->hotlist_win, "NCSA Mosaic: Save Current hotlist" ,
	   "Name for saved hotlist" , save_hot_cb, 0);
      else
	XmFileSelectionDoSearch (win->save_hotlist_win, NULL);

      XmxManageRemanage (win->save_hotlist_win);
      break;
    case 11:
      /* Load a hotlist file */
      if (!win->load_hotlist_win)
	{
	  Widget frame, workarea, tb;

	  XmxSetUniqid (win->id);
	  win->load_hotlist_win = XmxMakeFileSBDialog
	    (win->hotlist_win, "NCSA Mosaic: Load in Current hotlist" ,
	     "Name of file to open" , load_hot_cb, 0);
	  /* This makes a frame as a work area for the dialog box. */
	  XmxSetArg (XmNmarginWidth, 5);
	  XmxSetArg (XmNmarginHeight, 5);
	  frame = XmxMakeFrame (win->load_hotlist_win, XmxShadowEtchedIn);
	  XmxSetArg (XmNorientation, XmHORIZONTAL);
	  workarea = XmxMakeRadioBox (frame);
	  tb = XtVaCreateManagedWidget("toggle", xmToggleButtonGadgetClass,
				       workarea,
				       XtVaTypedArg, XmNlabelString,
				       XtRString, "Create new hotlist" , strlen("Create new hotlist" )+1,
				       XmNmarginHeight, 0,
				       XmNset, True, NULL);
	  XmxSetArg (XmNuserData, (XtArgVal)tb);
	  XmxSetValues (win->load_hotlist_win);
	  XtVaCreateManagedWidget("toggle", xmToggleButtonGadgetClass,
				  workarea,
				  XtVaTypedArg, XmNlabelString,
				  XtRString, "Load in current hotlist" , strlen("Load in current hotlist" )+1,
				  XmNmarginHeight, 0,
				  NULL);
	}
      else
	XmFileSelectionDoSearch (win->load_hotlist_win, NULL);

      XmxManageRemanage (win->load_hotlist_win);
      break;
    case 12:
      /* Add selected to the RBM or take it away... */
      {
        Boolean rv;
        int *pos_list;
        int pos_cnt;
        rv = XmListGetSelectedPos (win->hotlist_list, &pos_list, &pos_cnt);
        if (rv && pos_cnt)
          {
		mo_rbm_toggle_in_hotlist(win,pos_list[0]);
		XtFree((char *)pos_list); /* DXP */
		mo_write_default_hotlist();
	  }
        else
          {
            XmxMakeErrorDialog
              (win->hotlist_win, "No entry in the hotlist is currently selected.\n\nTo edit an entry in the hotlist,\nselect it with a single mouse click\nand press the Edit button again." , "Error: Nothing Selected" );
            XtManageChild (Xmx_w);
          }
      }
      break;
    }

  return;
}

static XmxCallback (hotlist_list_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  XmListCallbackStruct *cs = (XmListCallbackStruct *)call_data;

  URL_Include_Set(win,0,0);

  mo_visit_hotlist_position (win, cs->item_position);

  /* Don't unmanage the list. */

  return;
}

/* ------------------------- mo_post_hotlist_win -------------------------- */

/*
 * Pop up a hotlist window for an mo_window.
 */
mo_status mo_post_hotlist_win (mo_window *win)
{
  if (!win->hotlist_win)
    {
      Widget dialog_frame/*, toto*/;
      Widget dialog_sep, buttons_form, buttons1_form, buttons2_form;
      Widget hotlist_form/*, buttons1_frame*/;
      XtTranslations listTable;
      static char listTranslations[] =
	"~Shift ~Ctrl ~Meta ~Alt <Btn2Down>: ListBeginSelect() \n\
	  Button2<Motion>:		ListButtonMotion()\n\
	 ~Shift ~Ctrl ~Meta ~Alt <Btn2Up>:  ListBeginSelect() ListEndSelect()";

      listTable = XtParseTranslationTable(listTranslations);

      /* Create it for the first time. */
      XmxSetUniqid (win->id);
      XmxSetArg (XmNwidth, 475);
      XmxSetArg (XmNheight, 342);
      win->hotlist_win = XmxMakeFormDialog
        (win->base, "NCSA Mosaic: Hotlist View" );
      dialog_frame = XmxMakeFrame (win->hotlist_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM,
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);

      /* Main form. */
      hotlist_form = XmxMakeForm (dialog_frame);

      win->current_hotlist = (mo_hotlist *)default_hotlist;
      XmxSetArg (XmNcursorPositionVisible, False);
      XmxSetArg (XmNeditable, False);
      XmxSetArg (XmNvalue, (XtArgVal)"/");
      win->hotlist_label = XmxMakeTextField(hotlist_form);

      buttons1_form = XmxMakeFormAndFourButtons
        (hotlist_form, hotlist_win_cb, "Add Current" ,
	 "Goto URL" , "Remove" ,
	 "Edit" , 3, 4, 5, 6);
      buttons2_form = XmxMakeFormAndThreeButtons
        (hotlist_form, hotlist_win_cb, "Copy" ,
	 "Insert" , "Up" ,
	 7, 8, 9);
      XmxSetArg (XmNfractionBase, (XtArgVal)4);
      XmxSetArg (XmNverticalSpacing, (XtArgVal)0);
      XmxSetValues(buttons2_form);

      /* Hotlist list itself. */
      XmxSetArg (XmNresizable, False);
      XmxSetArg (XmNscrollBarDisplayPolicy, XmSTATIC);
      XmxSetArg (XmNlistSizePolicy, XmCONSTANT);
      win->hotlist_list =
        XmxMakeScrolledList (hotlist_form, hotlist_list_cb, 0);
      XtAugmentTranslations (win->hotlist_list, listTable);
      XtAddCallback(win->hotlist_list,
		    XmNbrowseSelectionCallback, hotlist_rbm_toggle_cb,
		    0);

      win->hotlist_rbm_toggle=XmxMakeToggleButton(hotlist_form,
						  "Include On Right Button Menu",
						  hotlist_win_cb,
						  12);
      URL_Include_Set(win,0,0);

      dialog_sep = XmxMakeHorizontalSeparator (hotlist_form);

      buttons_form = XmxMakeFormAndFiveButtons(hotlist_form,
					       hotlist_win_cb,
					       "Mail To...",
					       "Save",
					       "Load",
					       "Dismiss",
					       "Help...",
					       1,
					       10,
					       11,
					       0,
					       2);

      /* Constraints for hotlist_form. */
      /* buttons1_form: top to nothing, bottom to hotlist_list,
         left to form, right to form. */
      XmxSetOffsets (win->hotlist_label,  4, 0, 2, 2);
      XmxSetConstraints
	(win->hotlist_label, XmATTACH_FORM,  XmATTACH_NONE, XmATTACH_FORM,
	 XmATTACH_FORM, NULL, NULL, NULL, NULL);
      XmxSetOffsets (buttons1_form, 0, 0, 0, 0);
      XmxSetConstraints
        (buttons1_form,
         XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
         win->hotlist_label, NULL, NULL, NULL);
      XmxSetOffsets (buttons2_form, 0, 2, 0, 0);
      XmxSetConstraints
        (buttons2_form,
         XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
	 buttons1_form, NULL, NULL, NULL);
      /* list: top to form, bottom to rbm_toggle,
         etc... */
      XmxSetOffsets (XtParent (win->hotlist_list), 10, 10, 8, 8);
      XmxSetConstraints
        (XtParent (win->hotlist_list),
         XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
         buttons2_form, win->hotlist_rbm_toggle, NULL, NULL);
      XmxSetOffsets (win->hotlist_rbm_toggle, 0, 10, 6, 6);
      XmxSetConstraints
        (win->hotlist_rbm_toggle,
         XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_NONE,
         NULL, dialog_sep, NULL, NULL);
      XmxSetConstraints
        (dialog_sep,
         XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
         NULL, buttons_form, NULL, NULL);
      XmxSetConstraints
        (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
         XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
      win->save_hotlist_win = win->load_hotlist_win = NULL;
      win->hot_cut_buffer = NULL;
      /* Go get the hotlist up to this point set up... */
      mo_load_hotlist_list (win, win->hotlist_list);
    }

  XmxManageRemanage (win->hotlist_win);

  return mo_succeed;
}

/* -------------------- mo_add_node_to_current_hotlist -------------------- */

mo_status mo_add_node_to_current_hotlist (mo_window *win)
{
  if (!win->hotlist_win)
    win->current_hotlist = (mo_hotlist *)default_hotlist;
  return mo_add_item_to_hotlist (win->current_hotlist, mo_t_url,
				 win->current_node->title,
				 win->current_node->url, 0,
				 get_pref_boolean(eADD_HOTLIST_ADDS_RBM));
}


void mo_init_hotmenu()
{
    mo_init_hotlist_menu((mo_hotlist *) default_hotlist);
}

void mo_reinit_hotmenu()
{
    mo_reinit_hotlist_menu((mo_hotlist *) default_hotlist);
}

void mo_rbm_myself_to_death(mo_window *win, int val) {

	RecursiveSetList(win,(mo_hotlist *)default_hotlist,val,1);
	mo_reinit_hotmenu();
}
