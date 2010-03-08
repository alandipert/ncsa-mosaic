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
#include "gui-ftp.h"
#include "gui-popup.h"
#include "libhtmlw/HTML.h"
#include "libnut/system.h"

#define MAX_BUF_LEN 512  /* Length of all of the buffers used for dialog message output */


static mo_status mo_handle_ftpremove (mo_window *win, char *urlNsite);


/* gui-ftp handles all of the gui bits of the FTP send, remove, and mkdir 
   functionality.  All of the actual transfer stuff is in libwww2/HTFTP.c   
*/

/*---------------------  mo_handle_ftpput ---------------------------------*/
mo_status mo_handle_ftpput(mo_window *win)
{
 int length;
 char site[MAX_BUF_LEN+1], *cptr;
 char tbuf[MAX_BUF_LEN+1];

 if (!win->current_node)
   return mo_fail;

 /* Check to see if the url is somethin' like ftp://somewarez.31337.com */
 if((strlen(win->current_node->url)>4) && strncmp("ftp:", win->current_node->url, 4)==0) {
   win->ftp_site = strdup(win->current_node->url);
   mo_post_ftpput_window(win);
 } else {
   sprintf(tbuf, "FTP Send requires you to be on a page with an FTP url."); 
   application_user_info_wait(tbuf);
   return mo_fail;
 }
 return mo_succeed;  
} 

/* ---------------------- mo_post_ftpput_window ----------------------- */
static XmxCallback (ftpput_win_cb)
{
  char *fname = NULL, efname[MO_LINE_LENGTH];
  char tbuf[MAX_BUF_LEN+1];
  int i, count, ret;
  Widget fsbList;
  XmString st;
  XmStringTable selected_items;
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
	
  XtUnmanageChild (win->ftpput_win);  /* Down with the box */
  fsbList = XmFileSelectionBoxGetChild (win->ftpput_win, XmDIALOG_LIST);
  XtVaGetValues (fsbList,
		 XmNselectedItems, &selected_items,
		 XmNselectedItemCount, &count,
		 NULL);

  if (count) {
    for (i=0; i<count; i++) {
      XmStringGetLtoR (selected_items[i], XmSTRING_DEFAULT_CHARSET, &fname);
      pathEval (efname, fname);
      XtFree (fname);                  
      /* Make the url something HTFTPSend will understand */
      sprintf (tbuf, "%s&%s", win->ftp_site, efname);
      if ((ret = HTFTPSend (tbuf)) != 0) {   /* !HT_LOADED */
	if (ret != -2) { /* If the user interrupted us, forget about telling them */
	  sprintf(tbuf, "FTP Send Failed!  The file %s could not be sent.", efname);
	  application_user_info_wait(tbuf);
	}
	break;
      } else {
	mo_reload_window_text (win,0);
      }
    }
  } else {
    /* Get the filename out of the filespec box in case they typed something in */
    XtVaGetValues (win->ftpput_win,
		 XmNdirSpec, &st,
		 NULL);
    XmStringGetLtoR (st, XmSTRING_DEFAULT_CHARSET, &fname);
    pathEval (efname, fname);
    XtFree (fname);                  
    sprintf (tbuf, "%s&%s", win->ftp_site, efname);
    if ((ret = HTFTPSend (tbuf)) != 0) {   /* !HT_LOADED */
      if (ret != -2) { /* If the user interrupted us, forget about telling them */
	sprintf(tbuf, "FTP Send Failed!  The file %s could not be sent.", efname);
	application_user_info_wait(tbuf);
      }
    } else {
      mo_reload_window_text (win,0);
    }
  }

  
  /* Clear out the selections, we have to do this because the XmFSB has no clue 
     it is being used in extended selection mode. */
  XmListDeselectAllItems(fsbList);
  free(win->ftp_site);
  win->ftp_site = NULL;
  return;
}

mo_status mo_post_ftpput_window (mo_window *win)
{
 char tbuf[MAX_BUF_LEN+1];
 Widget fsbList;
 	
  if( win->ftp_site == NULL ) {
   return mo_fail;
  }

  sprintf( tbuf, "NCSA Mosaic: Send file to %s", win->ftp_site);
  XmxSetUniqid (win->id);
  if (!win->ftpput_win)
  {
    win->ftpput_win = XmxMakeFileSBDialog(win->base, tbuf, "Name of local file to send:", ftpput_win_cb, 0);
    /* Change the selection mode */
    fsbList = XmFileSelectionBoxGetChild (win->ftpput_win, XmDIALOG_LIST); 
    XtVaSetValues (fsbList,
		   XmNselectionPolicy, XmEXTENDED_SELECT,
 		   NULL);
  } else {
      XmFileSelectionDoSearch (win->ftpput_win, NULL);
  }
  
  XmxManageRemanage (win->ftpput_win);
  return mo_succeed;
}


/*---------------------  mo_handle_ftpmkdir ---------------------------------*/
mo_status mo_handle_ftpmkdir(mo_window *win)
{
 char tbuf[MAX_BUF_LEN+1];

 if (!win->current_node) 
   return mo_fail;

 /* Check to see if the url is somethin' like ftp://somewarez.31337.com */
 if((strlen(win->current_node->url)>4) && strncmp("ftp:", win->current_node->url, 4)==0) {
   win->ftp_site = strdup(win->current_node->url);
   mo_post_ftpmkdir_window(win);
 } else {
   sprintf(tbuf, "FTP MkDir requires you to be on a page with an FTP url."); 
   application_user_info_wait(tbuf);
   return mo_fail;
 }
 return mo_succeed;  
} 

/*---------------------  mo_post_ftpmkdir_window ---------------------------------*/
static XmxCallback (ftpmkdir_win_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  char *dirpath, tbuf[MAX_BUF_LEN+1];
  int ret;

  switch (XmxExtractToken ((int)client_data)) {
  
  case 0: /* Create dir */
    XtUnmanageChild (win->ftpmkdir_win);
    dirpath = XmxTextGetString (win->ftpmkdir_text);
    if (!dirpath || !(*dirpath)) {
      /* nothing here so do nothing */
      return;
    }
    sprintf (tbuf, "%s&%s", win->ftp_site, dirpath);
    if( (ret = HTFTPMkDir (tbuf)) != 0) {
      if (ret != -2) { /* If the user interrupted us, forget about telling them */
	sprintf(tbuf, "FTP MkDir Failed!  The directory %s could not be created.", dirpath);
	application_user_info_wait(tbuf);
      }
    } else {
      mo_reload_window_text (win, 0);
    }
    free(win->ftp_site);
    win->ftp_site = NULL;
      break;
  case 1: /* Dismiss */
      XtUnmanageChild (win->ftpmkdir_win);
      break;
  case 2: /* Help */
    /* mo_open_another_window (win, mo_assemble_help_url ("docview-menubar-file.html"), NULL, NULL); */
    break;
  case 3: /* Clear */
    XmxTextSetString (win->ftpmkdir_text, "");
    break;
  }
  return;
}

mo_status mo_post_ftpmkdir_window (mo_window *win)
{
  if (!win->ftpmkdir_win)
    {
      Widget dialog_frame;
      Widget dialog_sep, buttons_form;
      Widget form, label;
      
      /* Create it for the first time. */
      XmxSetUniqid (win->id);
      win->ftpmkdir_win = XmxMakeFormDialog (win->base, "NCSA Mosaic: FTP MkDir");
      dialog_frame = XmxMakeFrame (win->ftpmkdir_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints 
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
      
      /* Main form. */
      form = XmxMakeForm (dialog_frame);
      
      label = XmxMakeLabel (form, "Directory to Create: ");
      XmxSetArg (XmNwidth, 310);
      win->ftpmkdir_text = XmxMakeTextField (form);
      XmxAddCallbackToText (win->ftpmkdir_text, ftpmkdir_win_cb, 0);
      
      dialog_sep = XmxMakeHorizontalSeparator (form);
      
      buttons_form = XmxMakeFormAndFourButtons
        (form, ftpmkdir_win_cb, "Create", "Clear", "Dismiss", "Help...", 
         0, 3, 1, 2);

      /* Constraints for form. */
      XmxSetOffsets (label, 14, 0, 10, 0);
      XmxSetConstraints
        (label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_NONE,
         NULL, NULL, NULL, NULL);
      XmxSetOffsets (win->ftpmkdir_text, 10, 0, 5, 10);
      XmxSetConstraints
        (win->ftpmkdir_text, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_WIDGET,
         XmATTACH_FORM, NULL, NULL, label, NULL);
      XmxSetArg (XmNtopOffset, 10);
      XmxSetConstraints 
        (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, 
         XmATTACH_FORM,
         win->ftpmkdir_text, buttons_form, NULL, NULL);
      XmxSetConstraints 
        (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
    }
  
  XmxManageRemanage (win->ftpmkdir_win);
  
  return mo_succeed;
}


/* ---------------------- mo_handle_ftpremove ----------------------- */
static mo_status mo_handle_ftpremove (mo_window *win, char *urlNsite) 
{
 int ret;
 char tbuf[MAX_BUF_LEN+1];

    if ((ret = HTFTPRemove (urlNsite)) != 0) {
      if (ret != -2) { /* If the user interrupted us, forget about telling them */
	sprintf(tbuf, "FTP Remove Failed!  The file could not be removed.");
	application_user_info_wait(tbuf);
      }
    } else {
      mo_reload_window_text (win, 0);
    }
}


/* Ftp callback for the right mouse button menu */
void ftp_rmbm_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct act_struct *acst = (struct act_struct *) client_data;
  int which;
  char *xurl, tbuf[MAX_BUF_LEN+1];
  extern mo_window *current_win;
  struct ele_rec *eptr;
 
  which = acst->act_code;
  eptr = acst->eptr;
  
  switch(which) {

  case mo_ftp_put:
    mo_handle_ftpput (current_win);
    break;
    
  case mo_ftp_mkdir:
    mo_handle_ftpmkdir (current_win);
    break;

  case mo_ftp_remove:
    xurl = strrchr (eptr->anchorHRef, '/');
    sprintf (tbuf, "%s%s", current_win->current_node->url, xurl);
    mo_handle_ftpremove (current_win, tbuf);
    break;
  }
}

