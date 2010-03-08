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
#include "pan.h"
#include "grpan.h"
#include <pwd.h>
#include <sys/types.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>
#include "bitmaps/annotate.xbm"
#include "libnut/system.h"

typedef enum
{
  mo_new_annotation = 0, mo_edit_annotation
} mo_amode;

/* ---------------------- mo_is_editable_annotation ----------------------- */

mo_status mo_is_editable_annotation (mo_window *win, char *text)
{
  if (!win->current_node)
    return mo_fail;
/*
 * In group annotations we first need to check if this document is one,
 * and we also need to set the annotation_type in current_node so we do the
 * right thing when the Delete or Edit menus items are selected.
 */
  /* For now, it can only be a pan or a grpan. */
  if (mo_is_editable_grpan (text) == mo_succeed)
  {
	win->current_node->annotation_type = mo_annotation_workgroup;
	return mo_succeed;
  }
  else if (mo_is_editable_pan (text) == mo_succeed)
  {
	win->current_node->annotation_type = mo_annotation_private;
	return mo_succeed;
  }
  else
  {
	win->current_node->annotation_type = mo_annotation_private;
	return mo_fail;
  }
}

/* ---------------------- mo_fetch_annotation_links ----------------------- */

char *mo_fetch_annotation_links (char *url, int on_top)
{
  char *pans, *grpans;
  char *msg;	/* used to compose the merged annotation returns */
  char *uniqdoc = mo_url_to_unique_document (url);
  char *rv = NULL;

  pans = mo_fetch_pan_links (uniqdoc, on_top);
  grpans = mo_fetch_grpan_links (uniqdoc);

  /* Merge into pans */
  if (!pans && !grpans)
    {
      /* pans is already NULL, do nothing */
    }
  else if (pans && !grpans)
    {
      /* pans is all there is, do nothing */
    }
  else if (!pans && grpans)
    {
      /* grpans is all there is, swap pans and grpans */
      pans = grpans;
      grpans = NULL;
    }
  else /* both exist */
    {
      char *ret = (char *)malloc (strlen (pans) + strlen (grpans) + 8);
      strcpy (ret, grpans);
      strcat (ret, pans);
      free (grpans);
      free (pans);
      pans = ret;
    }

  /*
   * If we have no annotations, return NULL now
   */
  if (pans == NULL)
    goto done;

  /*
   * Compose the annotations into msg, adding a line at either the top or
   * bottom depending on where the annotations will appear.
   */
  msg = (char *)malloc (strlen (pans) + 40);

  if (on_top)
    {
      strcpy (msg, pans);
      strcat (msg, "<hr>\n");
    }
  else
    {
      strcpy (msg, "<hr>\n");
      strcat (msg, pans);
    }
  free(pans);
  rv = msg;

 done:
  free (uniqdoc);
  return rv;
}

/* ------------------------- set_annotation_mode -------------------------- */

static void set_annotation_mode (mo_window *win, int mode)
{
  win->annotation_mode = mode;

  XmxSetSensitive 
    (win->delete_button, 
     mode == mo_edit_annotation ? XmxSensitive : XmxNotSensitive);
  return;
}

/* ----------------------- mo_post_annotate_window ------------------------ */

static XmxCallback (annotate_win_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
/*  char *msg, *fnam, *cmd;
  FILE *fp;*/

  if (!win->current_node)
    {
      XtUnmanageChild (win->annotate_win);
      return;
    }

  switch (XmxExtractToken ((int)client_data))
    {
    case 0:
/*
 * When finished with the annotation window, do the right thing based
 * on whether this is private or group, and whether this is an edit or
 * a new annotation.
 */
      if (win->pubpri == mo_annotation_public ||
          win->pubpri == mo_annotation_workgroup)
        {
          char *txt = XmxTextGetString (win->annotate_text);

          /* Make sure text exists before actually annotating. */
          /* In the case of editing, no text should result in
             a delete. */
          if (txt && *txt)
            {
              if (win->annotation_mode == mo_new_annotation)
                {
		  char *title = XmxTextGetString (win->annotate_title);
		  char *author = XmxTextGetString (win->annotate_author);
                  int i;

                  if (!title || !*title)
                    title = strdup ("Annotation with no title");
                  if (!author || !*author)
                    author = strdup ("No Author");

                  /* This is a temporary fix because Eric's real
                     fix doesn't seem to be working. */
                  for (i = 0; i < strlen (title); i++)
                    if (title[i] == '\"')
                      title[i] = '\'';
                  for (i = 0; i < strlen (author); i++)
                    if (author[i] == '\"')
                      author[i] = '\'';
                  
		  /*
		   * Add the new annotation to the server -- with its real URL
                   * if inside an HDF file, else the canonical form.
		   */
                  mo_new_grpan(mo_url_to_unique_document 
                               (win->current_node->url), title, author, txt);

                  mo_set_win_current_node (win, win->current_node);
                }
              else /* edit annotation */
                {
		  char *title = XmxTextGetString (win->annotate_title);
		  char *author = XmxTextGetString (win->annotate_author);
                  int i;

                  if (!title || !*title)
                    title = strdup ("Annotation with no title");
                  if (!author || !*author)
                    author = strdup ("No Author");

                  /* This is a temporary fix because Eric's real
                     fix doesn't seem to be working. */
                  for (i = 0; i < strlen (title); i++)
                    if (title[i] == '\"')
                      title[i] = '\'';
                  for (i = 0; i < strlen (author); i++)
                    if (author[i] == '\"')
                      author[i] = '\'';

		  /*
		   * Change the annotation on the server
		   */
                  mo_modify_grpan(mo_url_to_unique_document 
                                  (win->current_node->url), 
                                  title, author, txt);

                  mo_reload_window_text (win, 0);
                }
            }
          XtUnmanageChild (win->annotate_win);
        }
      else
        {
          char *txt = XmxTextGetString (win->annotate_text);
          /* Make sure text exists before actually annotating. */
          /* In the case of editing, no text should result in
             a delete. */
          if (txt && *txt)
            {
              if (win->annotation_mode == mo_new_annotation)
                {
                  mo_new_pan 
                    (mo_url_to_unique_document (win->current_node->url), 
                     XmxTextGetString (win->annotate_title), 
                     XmxTextGetString (win->annotate_author), 
                     txt);
                  /* Inefficient, but safe. */
                  mo_write_pan_list ();
                  mo_set_win_current_node (win, win->current_node);
                }
              else /* edit annotation */
                {
                  mo_modify_pan (win->editing_id,
                                 XmxTextGetString (win->annotate_title), 
                                 XmxTextGetString (win->annotate_author), 
                                 txt);
                  /* Inefficient, but safe. */
                  mo_write_pan_list ();
                  mo_reload_window_text (win, 0);
                }
            }
          XtUnmanageChild (win->annotate_win);
        }
      break;
    case 1:
      XtUnmanageChild (win->annotate_win);
      /* Do nothing. */
      break;
    case 2:
      mo_open_another_window
        (win,
         mo_assemble_help_url ("help-on-annotate-win.html"),
         NULL, NULL);
      break;
    }

  return;
}

#ifdef GRPAN_PASSWD
static XmxCallback (passwd_toggle_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  int on = XmToggleButtonGetState (win->passwd_toggle);
  
  if (on)
    {
      XmxSetSensitive (win->passwd_label, XmxSensitive);
      XmxSetSensitive (win->annotate_passwd, XmxSensitive);
    }
  else
    {
      XmxSetSensitive (win->passwd_label, XmxNotSensitive);
      XmxSetSensitive (win->annotate_passwd, XmxNotSensitive);
      XmxTextSetString (win->annotate_passwd, "\0");
    }

  return;
}
#endif

/* ---------------------- clear slate & include file ---------------------- */

static void do_slate (mo_window *win)
{
  struct passwd *pw = getpwuid (getuid ());
  /* Namestr has to be long, as passwd entries are now getting absurdly long. */
  char namestr[1000];

  sprintf (namestr, "%s (%s)",
           get_pref_string(eDEFAULT_AUTHOR_EMAIL),
           get_pref_string(eDEFAULT_AUTHOR_NAME));
  
  XmxTextSetString (win->annotate_author, namestr);

  sprintf (namestr, "%s %s","Annotation by",
           get_pref_string(eDEFAULT_AUTHOR_NAME));

  XmxTextSetString (win->annotate_title, namestr);

#ifdef GRPAN_PASSWD
  XmxTextSetString (win->annotate_passwd, "\0");
#endif
  XmxTextSetString (win->annotate_text, "\0");

  return;
}

static XmxCallback (include_fsb_cb)
{
  char *fname, efname[MO_LINE_LENGTH];
  FILE *fp;
  char line[MO_LINE_LENGTH], *status;

  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  if (!win)
    return;

  XtUnmanageChild (win->include_fsb);
  fname = (char *)malloc (128 * sizeof (char));
  
  XmStringGetLtoR (((XmFileSelectionBoxCallbackStruct *)call_data)->value,
                   XmSTRING_DEFAULT_CHARSET,
                   &fname);

  pathEval (efname, fname);

  fp = fopen (efname, "r");
  if (!fp)
    {
        char *buf, *final, tmpbuf[80];
	int final_len;

        buf=my_strerror(errno);
        if (!buf || !*buf || !strcmp(buf,"Error 0")) {
                sprintf(tmpbuf,"Uknown Error");
                buf=tmpbuf;
	}

        final_len=30+((!efname || !*efname?3:strlen(efname))+13)+15+(strlen(buf)+13);
        final=(char *)calloc(final_len,sizeof(char));

	sprintf(final,"\nUnable to Open File:\n   %s\n\nOpen Error:\n   %s\n",(!efname || !*efname?" ":efname),buf);

	XmxMakeErrorDialog (win->annotate_win, 
                          final, 
                          "Annotation Open Error");
	XtManageChild (Xmx_w);

	if (final) {
		free(final);
		final=NULL;
	}
      return;
    }
  
  while (1)
    {
      long pos;
      status = fgets (line, MO_LINE_LENGTH, fp);
      if (!status || !(*line))
        goto done;
      
      XmTextInsert (win->annotate_text,
                    pos = XmTextGetInsertionPosition (win->annotate_text),
                    line);
      /* move insertion position to past this line to avoid inserting the
         lines in reverse order */
      XmTextSetInsertionPosition (win->annotate_text, pos + strlen(line));
    }

 done:
  return;
}

static XmxCallback (include_button_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  if (!win->include_fsb)
    {
      win->include_fsb = XmxMakeFileSBDialog
        (win->annotate_win,
         "NCSA Mosaic: Include File In Annotation",
         "Name of file to include:",
         include_fsb_cb, 0);
    }
  else
    {
      XmFileSelectionDoSearch (win->include_fsb, NULL);
    }
  
  XmxManageRemanage (win->include_fsb);

  return;
}

static XmxCallback (clear_button_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  do_slate (win);

  return;
}

/* ------------------------- mo_delete_annotation ------------------------- */

mo_status mo_delete_annotation (mo_window *win, int id)
{
  /* Delete the annotation currently being viewed. */
  extern char *home_document;

  if (!win->current_node)
    return mo_fail;

  mo_delete_pan (id);
  
  /* Now that we've deleted the annotation, take care of
     business.  Damn, but this shouldn't have to happen here. */
  if (win->current_node->previous)
    {
      mo_node *prev = win->current_node->previous;
      
      mo_kill_node (win, win->current_node);
      prev->next = NULL;
      win->current_node = prev;
      /* Set the text. */
      mo_reload_window_text (win, 0);
    }      
  else
    {
      /* No previous node; this only happens if someone's dumb enough
         to have his/her home document be a personal annotation which
         he/she then deletes. */
      mo_kill_node (win, win->current_node);
      win->current_node = NULL;
      win->history = NULL;

      mo_load_window_text (win, home_document, NULL);
    }
  
  return mo_succeed;
}


mo_status mo_delete_group_annotation (mo_window *win, char *url)
{
  /* Delete the annotation currently being viewed. */
  extern char *home_document;

  if (!win->current_node)
    return mo_fail;

  mo_delete_grpan (mo_url_to_unique_document (url));

  /* Now that we've deleted the annotation, take care of
     business.  Damn, but this shouldn't have to happen here. */
  if (win->current_node->previous)
    {
      mo_node *prev = win->current_node->previous;
      
      mo_kill_node (win, win->current_node);
      prev->next = NULL;
      win->current_node = prev;
      /* Set the text. */
      mo_reload_window_text (win, 0);
    }      
  else
    {
      /* No previous node; this only happens if someone's dumb enough
         to have his/her home document be a group annotation which
         he/she then deletes. */
      mo_kill_node (win, win->current_node);
      win->current_node = NULL;
      win->history = NULL;

      mo_load_window_text (win, home_document, NULL);
    }
  
  return mo_succeed;
}

static XmxCallback (delete_button_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  XtUnmanageChild (win->annotate_win);

  if (!win->current_node)
    return;

  if (win->current_node->annotation_type == mo_annotation_private)
    {
      mo_delete_annotation (win, win->editing_id);
    }
  else if (win->current_node->annotation_type == mo_annotation_workgroup)
    {
      mo_delete_group_annotation (win, win->current_node->url);
    }
  
  return;
}

/* ---------------------------- public/private ---------------------------- */

XmxOptionMenuStruct *pubpri_opts;

#ifdef GRPAN_PASSWD
static void do_privacy (mo_window *win)
{
  /* If pubpri = mo_annotation_public, unfreeze the appropriate
     GUI pieces.  Else, freeze 'em. */
  int on = XmToggleButtonGetState (win->passwd_toggle);

  if (win->pubpri == mo_annotation_public ||
      win->pubpri == mo_annotation_workgroup)
    {
      /* Match current state of on. */
      if (on)
        {
          XmxSetSensitive (win->passwd_label, XmxSensitive);
          XmxSetSensitive (win->annotate_passwd, XmxSensitive);
        }
      XmxSetSensitive (win->passwd_toggle, XmxSensitive);
      XtVaSetValues (win->passwd_toggle, XmNfillOnSelect, True, NULL);
    }
  else
    {
      /* We're private.  No password, nohow. */
      XmxSetSensitive (win->passwd_label, XmxNotSensitive);
      XmxSetSensitive (win->annotate_passwd, XmxNotSensitive);
      XmxTextSetString (win->annotate_passwd, "\0");
      XmxSetSensitive (win->passwd_toggle, XmxNotSensitive);
      XtVaSetValues (win->passwd_toggle, XmNfillOnSelect, False, NULL);
    }

  return;
}
#endif

static XmxCallback (pubpri_opts_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  
  win->pubpri = XmxExtractToken ((int)client_data);

#ifdef GRPAN_PASSWD
  do_privacy (win);
#endif

  return;
}

/* OK, here's how we're gonna work this.

   There will still only be one annotate window per document view window.
   The annotate window will either be in new_annotation or edit_annotation mode.
   This mode will be stored in win->annotation_mode.
   Only routines in this file will touch that mode.

   The mode will be set upon entry to mo_post_annotate_win.

   If mo_post_annotate_win is told it's supposed to edit,
   then it installs title, author, text, etc. as it sees fit.

   If, while in edit mode, 'Clear Slate' gets hit, 
   mode flips to mo_edit_annotation regardless -- everything's wiped out.
   */

static void make_annotate_win (mo_window *win)
{
  Widget dialog_frame;
  Widget dialog_sep, buttons_form;
  Widget annotate_form/*, yap_label*/;
  Widget logo;
  Widget author_label, title_label, text_label;
  Widget include_button, clear_button;
  Widget sep;
#ifdef GRPAN_PASSWD
  Widget passwd_frame, passwd_f;
#endif
  
  /* Create it for the first time. */
  XmxSetUniqid (win->id);
  XmxSetArg (XmNresizePolicy, XmRESIZE_GROW);
  win->annotate_win = XmxMakeFormDialog 
    (win->base, "NCSA Mosaic: Annotate Window" );
  dialog_frame = XmxMakeFrame (win->annotate_win, XmxShadowOut);
  
  /* Constraints for base. */
  XmxSetConstraints 
    (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
     XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
  
  /* Main form. */
  annotate_form = XmxMakeForm (dialog_frame);
  
  author_label = XmxMakeLabel (annotate_form, "Annotation Author:" );
  win->annotate_author = XmxMakeTextField (annotate_form);
  
  title_label = XmxMakeLabel (annotate_form, "Annotation Title:" );
  win->annotate_title = XmxMakeTextField (annotate_form);

  /* Let's just not do any of this shit if we don't have
     group annotations yet. */
  win->pubpri_menu = XmxRMakeOptionMenu
    (annotate_form, "", pubpri_opts_cb, pubpri_opts);
  XmxRSetSensitive (win->pubpri_menu, mo_annotation_public, XmxNotSensitive);
  if (!get_pref_string(eANNOTATION_SERVER))
    XmxRSetSensitive (win->pubpri_menu, mo_annotation_workgroup, 
                      XmxNotSensitive);
  win->pubpri = mo_annotation_private;

#ifdef GRPAN_PASSWD  
  /* --- Frame for the password fields. --- */
  XmxSetArg (XmNmarginWidth, 4);
  XmxSetArg (XmNmarginHeight, 4);
  XmxSetArg (XmNadjustLast, True);
  passwd_frame = XmxMakeFrame (annotate_form, XmxShadowEtchedOut);
  passwd_f = XmxMakeForm (passwd_frame);
  {
    win->passwd_toggle = XmxMakeToggleButton 
      (passwd_f, "Password Protection" , passwd_toggle_cb, 0);
    XmxSetToggleButton (win->passwd_toggle, XmxSet);
    
    win->passwd_label = XmxMakeLabel (passwd_f, "Password:" );
    XmxSetArg (XmNcolumns, 20);
    win->annotate_passwd = XmxMakeTextField (passwd_f);

    XmxSetOffsets (win->passwd_toggle, 1, 0, 2, 10);
    XmxSetConstraints 
      (win->passwd_toggle, XmATTACH_FORM, XmATTACH_FORM, 
       XmATTACH_FORM, XmATTACH_NONE, NULL, NULL, NULL, NULL);
    XmxSetOffsets (win->passwd_label, 1, 0, 15, 10);
    XmxSetConstraints
      (win->passwd_label, XmATTACH_FORM, XmATTACH_FORM,
       XmATTACH_WIDGET, XmATTACH_NONE, NULL, NULL, win->passwd_toggle, NULL);
    XmxSetOffsets (win->annotate_passwd, 1, 0, 6, 6);
    XmxSetConstraints
      (win->annotate_passwd, XmATTACH_FORM, XmATTACH_FORM,
       XmATTACH_WIDGET, XmATTACH_FORM, NULL, NULL, win->passwd_label, NULL);
  }
#endif
  
  logo = XmxMakeNamedLabel (annotate_form, NULL, "logo");
  XmxApplyBitmapToLabelWidget
    (logo, annotate_bits, annotate_width, annotate_height);
  
  sep = XmxMakeHorizontalSeparator (annotate_form);
  
  text_label = XmxMakeLabel 
    (annotate_form, "Enter the annotation text:");

  clear_button = XmxMakePushButton
    (annotate_form, "Clean Slate" , clear_button_cb, 0);
  include_button = XmxMakePushButton
    (annotate_form, "Include File..." , include_button_cb, 0);
  win->delete_button = XmxMakePushButton
    (annotate_form, "Delete" , delete_button_cb, 0);
  
  XmxSetArg (XmNscrolledWindowMarginWidth, 10);
  XmxSetArg (XmNscrolledWindowMarginHeight, 8);
  XmxSetArg (XmNcursorPositionVisible, True);
  XmxSetArg (XmNeditable, True);
  XmxSetArg (XmNeditMode, XmMULTI_LINE_EDIT);
  XmxSetArg (XmNrows, 15);
  XmxSetArg (XmNcolumns, 80);
  win->annotate_text = XmxMakeScrolledText (annotate_form);
  
  dialog_sep = XmxMakeHorizontalSeparator (annotate_form);
  
  buttons_form = XmxMakeFormAndThreeButtonsSqueezed
    (annotate_form, annotate_win_cb, "Save" ,
     "Dismiss" , "Help..." , 
     0, 1, 2);
  
  /* Constraints for annotate_form. */
  /* author_label: top form, bottom nothing,
     left form, right nothing. */
  XmxSetOffsets (author_label, 14, 10, 10, 10);
  XmxSetConstraints
    (author_label, XmATTACH_FORM, XmATTACH_NONE,
     XmATTACH_FORM, XmATTACH_NONE, NULL, NULL, NULL, NULL);
  /* annotate_author: top form, bottom nothing,
     left author_label, right logo. */
  XmxSetOffsets (win->annotate_author, 10, 10, 10, 10);
  XmxSetConstraints
    (win->annotate_author, XmATTACH_FORM, XmATTACH_NONE,
     XmATTACH_WIDGET, XmATTACH_WIDGET,
     NULL, NULL, author_label, logo);
  /* title_label: top annotate_author, bottom nothing,
     left form, right nothing */
  XmxSetOffsets (title_label, 14, 10, 10, 10);
  XmxSetConstraints
    (title_label, XmATTACH_WIDGET, XmATTACH_NONE,
     XmATTACH_FORM, XmATTACH_NONE, 
     win->annotate_author, NULL, NULL, NULL);
  /* annotate_title: top annotate_author, bottom nothing,
     left title_label, right logo. */
  XmxSetOffsets (win->annotate_title, 10, 10, 10, 10);
  XmxSetConstraints
    (win->annotate_title, XmATTACH_WIDGET, XmATTACH_NONE,
     XmATTACH_WIDGET, XmATTACH_WIDGET,
     win->annotate_author, NULL, title_label, logo);
#ifdef GRPAN_PASSWD
  XmxSetOffsets (win->pubpri_menu->base, 19, 10, 0, 10);
#else
  XmxSetOffsets (win->pubpri_menu->base, 10, 10, 0, 10);
#endif
  XmxSetConstraints
    (win->pubpri_menu->base, XmATTACH_WIDGET, XmATTACH_NONE,
     XmATTACH_FORM, XmATTACH_NONE, 
     win->annotate_title, NULL, NULL, NULL);
#ifdef GRPAN_PASSWD
  /* passwd_frame: top annotate_title, bottom nothing,
     left pubpri_menu, right form */
  XmxSetOffsets (passwd_frame, 10, 10, 10, 10);
  XmxSetConstraints
    (passwd_frame, XmATTACH_WIDGET, XmATTACH_NONE,
     XmATTACH_WIDGET, XmATTACH_FORM,
     win->annotate_title, NULL, win->pubpri_menu->base, NULL);
  XmxSetOffsets (sep, 10, 0, 0, 0);
  XmxSetConstraints
    (sep, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM,
     XmATTACH_FORM, passwd_frame, NULL, NULL, NULL);
#else /* not GRPAN_PASSWD */
  XmxSetOffsets (sep, 10, 0, 0, 0);
  XmxSetConstraints
    (sep, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM,
     XmATTACH_FORM, win->pubpri_menu->base, NULL, NULL, NULL);
#endif /* not GRPAN_PASSWD */
    
  /* text_label: top sep, bottom nothing,
     left form, right nothing */
  XmxSetOffsets (text_label, 12, 0, 10, 10);
  XmxSetConstraints
    (text_label, XmATTACH_WIDGET, XmATTACH_NONE,
     XmATTACH_FORM, XmATTACH_NONE,
     sep, NULL, NULL, NULL);
  
  /* Buttons */
  XmxSetOffsets (clear_button, 8, 0, 0, 3);
  XmxSetConstraints
    (clear_button, XmATTACH_WIDGET, XmATTACH_NONE,
     XmATTACH_NONE, XmATTACH_WIDGET,
     sep, NULL, NULL, include_button);
  XmxSetOffsets (include_button, 8, 0, 0, 3);
  XmxSetConstraints
    (include_button, XmATTACH_WIDGET, XmATTACH_NONE,
     XmATTACH_NONE, XmATTACH_WIDGET,
     sep, NULL, NULL, win->delete_button);
  XmxSetOffsets (win->delete_button, 8, 0, 10, 10);
  XmxSetConstraints
    (win->delete_button, XmATTACH_WIDGET, XmATTACH_NONE,
     XmATTACH_NONE, XmATTACH_FORM,
     sep, NULL, NULL, NULL);
  
  /* logo: top form, bottom nothing,
     left nothing, right form. */
  XmxSetOffsets (logo, 5, 10, 10, 5);
  XmxSetConstraints
    (logo, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE, XmATTACH_FORM,
     NULL, NULL, NULL, NULL);
  
  XmxSetOffsets (XtParent (win->annotate_text), 0, 2, 0, 0);
  XmxSetConstraints
    (XtParent (win->annotate_text), XmATTACH_WIDGET, XmATTACH_WIDGET, 
     XmATTACH_FORM, XmATTACH_FORM,
     text_label, dialog_sep, NULL, NULL);
  
  XmxSetArg (XmNtopOffset, 10);
  XmxSetConstraints 
    (dialog_sep, XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM, 
     XmATTACH_FORM,
     NULL, buttons_form, NULL, NULL);
  XmxSetConstraints 
    (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, 
     XmATTACH_FORM,
     NULL, NULL, NULL, NULL);

  XtRealizeWidget (win->annotate_win);
  do_slate (win);
#ifdef GRPAN_PASSWD
  do_privacy (win);
#endif
  win->include_fsb = 0;
  
  return;
}


/* For editing, we should pass in:
     (x) a flag to indicate that we're editing
     (a) the URL we're annotation
     (b) the ID of the current annotation
     (c) the starting Title
     (d) the starting Author
     (e) the starting Text.
     (f) the original filename (probably not needed). */
/* If editflag is high, then:
     * window title should change (to Edit Annotation)
     * 'Annotate' should change to 'Commit'.
   Right now this doesn't happen. */
mo_status mo_post_annotate_win 
  (mo_window *win, int edit_flag, int id, char *title, char *author, 
   char *text, char *fname)
{
  if (!win->current_node)
    return mo_fail;

  /* First thing we do is create the window if it doesn't exist yet. */
  if (!win->annotate_win)
    make_annotate_win (win);
  
  /* If we're in edit mode, then do the right thing. */
  if (edit_flag)
    {
      set_annotation_mode (win, mo_edit_annotation);
      win->editing_id = id;

      /*
       * When editing an annotation, we need to set pubpri
       * properly so the right thing happens when we hit commit.
       */
      if (win->current_node->annotation_type == mo_annotation_private)
        win->pubpri = mo_annotation_private;
      else if (win->current_node->annotation_type == mo_annotation_workgroup)
        win->pubpri = mo_annotation_workgroup;
      XmxRSetOptionMenuHistory(win->pubpri_menu, win->pubpri);
      
      XmxTextSetString (win->annotate_author, author);
      XmxTextSetString (win->annotate_title, title);
#ifdef GRPAN_PASSWD
      XmxTextSetString (win->annotate_passwd, "\0");
#endif
      XmxTextSetString (win->annotate_text, text);
      XmTextSetTopCharacter (win->annotate_text, 0);
    }
  else
    {
      set_annotation_mode (win, mo_new_annotation);
    }

  /* Finally, we manage. */
  XmxManageRemanage (win->annotate_win);
  
  return mo_succeed;
}

