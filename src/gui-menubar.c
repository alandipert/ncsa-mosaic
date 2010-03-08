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
#include "main.h"
#include "libhtmlw/HTML.h"
#include "gui.h"
#include "grpan.h"
#include "gui-ftp.h"
#include "gui-popup.h" /* for callback struct definition */
#include "gui-dialogs.h"
#include "gui-news.h"
#include "cci.h"
#include "cciBindings.h"
#include "cciBindings2.h"
#include "history.h"
#include "pan.h"
#include "annotate.h"
#include "mo-www.h"
#include "globalhist.h"
#include "proxy.h"
#include "libwww2/HTNews.h"

#include <sys/types.h>
#include <sys/stat.h>

/*SWP -- 7/17/95*/
#define __SRC__
#include "../libwww2/HTAAUtil.h"

/*SWP -- 8/14/95*/
extern int tableSupportEnabled;
extern int imageViewInternal;

/* SWP -- Spoof Agents Stuff */
extern int selectedAgent;
extern int numAgents;
extern char **agent;

extern mo_root_hotlist *default_hotlist;

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

extern mo_window *current_win;

extern char *home_document;
extern Widget toplevel;
extern mo_window *current_win;

/* from cciBindings.c */
extern int cci_event;	/* send window event to application?? */

#ifndef PRERELEASE
extern char do_comment;
#endif
static Widget exitbox = NULL;

#define MAX_DOCUMENTS_MENU_ITEMS 120
#define DOCUMENTS_MENU_COUNT_OFFSET 5000
/* List of URL's matching items in documents menu. */
static char *urllist[MAX_DOCUMENTS_MENU_ITEMS];

static XmxCallback (exit_confirm_cb);
static void mo_post_exitbox (void);
static long wrapFont (char *name);
static XmxCallback (clear_history_confirm_cb);
static mo_status mo_do_delete_annotation (mo_window *win);
static XmxCallback (delete_annotation_confirm_cb);
static XmxCallback (agent_menubar_cb);
static mo_status mo_file_exists (char *filename);
static void mo_grok_menubar (char *filename);
static void mo_try_to_grok_menubar (void);


/* --------------------------- mo_post_exitbox ---------------------------- */

static XmxCallback (exit_confirm_cb)
{
  if (XmxExtractToken ((int)client_data))
    mo_exit ();
  else
    XtUnmanageChild (w);
  
  return;
}

static void mo_post_exitbox (void)
{
  if (get_pref_boolean(eCONFIRM_EXIT))
    {
      if (exitbox == NULL)
        {
          exitbox = XmxMakeQuestionDialog
            (toplevel, "Are you sure you want to exit NCSA Mosaic?" ,
             "NCSA Mosaic: Exit Confirmation" , exit_confirm_cb, 1, 0);
          XtManageChild (exitbox);
        }
      else
        {
          XmxManageRemanage (exitbox);
        }
    }
  else
    {
      /* Don't confirm exit; just zap it. */
      mo_exit ();
    }

  return;
}

/* -------------------- mo_set_fancy_selections_toggle -------------------- */

mo_status mo_set_fancy_selections_toggle (mo_window *win)
{
  XmxRSetToggleState (win->menubar, mo_fancy_selections,
                      win->pretty ? XmxSet : XmxNotSet);
  return mo_succeed;
}

/* ---------------------------- mo_set_fonts ---------------------------- */

static long wrapFont (char *name)
{
  XFontStruct *font = XLoadQueryFont (dsp, name);
  if (font == NULL)
    {
	char buf[BUFSIZ];

	sprintf(buf, "Could not open font '%s'. Using fixed instead." , name);
	XmxMakeErrorDialog(current_win->base,buf,"Load Font Error" );
	font = XLoadQueryFont (dsp, "fixed");
    }
  return ((long)font);
}

mo_status mo_set_fonts (mo_window *win, int size)
{
  switch (size)
    {
    case mo_large_fonts:
      XmxSetArg (XtNfont, wrapFont("-adobe-times-medium-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-adobe-times-medium-i-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-adobe-times-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-adobe-courier-medium-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-adobe-courier-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-adobe-times-bold-r-normal-*-25-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-adobe-times-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-adobe-times-medium-i-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-adobe-courier-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-adobe-times-medium-r-normal-*-14-*-*-*-*-*-*-*"));

      XmxSetValues (win->scrolled_win);
      win->font_family = 0;
      break;
    case mo_regular_fonts:
      XmxSetArg (XtNfont, wrapFont("-adobe-times-medium-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-adobe-times-medium-i-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-adobe-courier-medium-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-adobe-courier-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-adobe-times-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-adobe-times-bold-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-adobe-times-medium-i-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-adobe-times-medium-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetValues (win->scrolled_win);
      win->font_family = 0;
      break;
    case mo_small_fonts:
      XmxSetArg (XtNfont, wrapFont("-adobe-times-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-adobe-times-medium-i-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-adobe-times-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-adobe-times-bold-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-adobe-times-bold-r-normal-*-8-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-adobe-times-medium-i-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-adobe-courier-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-adobe-times-medium-r-normal-*-8-*-*-*-*-*-*-*"));

      XmxSetValues (win->scrolled_win);
      win->font_family = 0;
      break;
    case mo_large_helvetica:
      XmxSetArg (XtNfont, wrapFont("-adobe-helvetica-medium-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-adobe-helvetica-medium-o-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-adobe-helvetica-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-adobe-courier-medium-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-adobe-courier-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-adobe-helvetica-bold-r-normal-*-25-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-adobe-helvetica-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-adobe-helvetica-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-adobe-helvetica-medium-o-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-adobe-courier-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-adobe-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetValues (win->scrolled_win);
      win->font_family = 1;
      break;
    case mo_regular_helvetica:
      XmxSetArg (XtNfont, wrapFont("-adobe-helvetica-medium-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-adobe-helvetica-medium-o-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-adobe-helvetica-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-adobe-courier-medium-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-adobe-courier-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-adobe-helvetica-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-adobe-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-adobe-helvetica-bold-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-adobe-helvetica-medium-o-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-adobe-helvetica-medium-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetValues (win->scrolled_win);
      win->font_family = 1;
      break;
    case mo_small_helvetica:
      XmxSetArg (XtNfont, wrapFont("-adobe-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-adobe-helvetica-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-adobe-helvetica-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-adobe-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-adobe-helvetica-bold-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-adobe-helvetica-bold-r-normal-*-8-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-adobe-helvetica-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-adobe-courier-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-adobe-helvetica-medium-r-normal-*-8-*-*-*-*-*-*-*"));

      XmxSetValues (win->scrolled_win);
      win->font_family = 1;
      break;
    case mo_large_newcentury:
      XmxSetArg (XtNfont, wrapFont("-adobe-new century schoolbook-medium-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-adobe-new century schoolbook-medium-i-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-adobe-courier-medium-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-adobe-courier-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-25-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-adobe-new century schoolbook-medium-i-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-adobe-courier-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-adobe-new century schoolbook-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetValues (win->scrolled_win);
      win->font_family = 2;
      break;
    case mo_small_newcentury:
      XmxSetArg (XtNfont, wrapFont("-adobe-new century schoolbook-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-adobe-new century schoolbook-medium-i-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-8-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-adobe-new century schoolbook-medium-i-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-adobe-courier-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-adobe-new century schoolbook-medium-r-normal-*-8-*-*-*-*-*-*-*"));
      XmxSetValues (win->scrolled_win);
      win->font_family = 2;
      break;
    case mo_regular_newcentury:
      XmxSetArg (XtNfont, wrapFont("-adobe-new century schoolbook-medium-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-adobe-new century schoolbook-medium-i-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-adobe-courier-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-adobe-new century schoolbook-bold-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-adobe-new century schoolbook-medium-i-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-adobe-new century schoolbook-medium-r-normal-*-10-*-*-*-*-*-*-*"));

      XmxSetValues (win->scrolled_win);
      win->font_family = 2;
      break;
    case mo_large_lucidabright:
      XmxSetArg (XtNfont, wrapFont("-b&h-lucidabright-medium-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-b&h-lucidabright-medium-i-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-b&h-lucidabright-demibold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-25-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-b&h-lucidabright-medium-i-normal-*-20-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-b&h-lucidabright-medium-r-normal-*-14-*-*-*-*-*-*-*"));

      XmxSetValues (win->scrolled_win);
      win->font_family = 3;
      break;
    case mo_regular_lucidabright:
      XmxSetArg (XtNfont, wrapFont("-b&h-lucidabright-medium-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-b&h-lucidabright-medium-i-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-b&h-lucidabright-demibold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-b&h-lucidabright-medium-i-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-b&h-lucidabright-medium-r-normal-*-10-*-*-*-*-*-*-*"));

      XmxSetValues (win->scrolled_win);
      win->font_family = 3;
      break;
    case mo_small_lucidabright:
      XmxSetArg (XtNfont, wrapFont("-b&h-lucidabright-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNitalicFont, wrapFont("-b&h-lucidabright-medium-i-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNboldFont, wrapFont("-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedFont, wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixedboldFont, wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNfixeditalicFont, wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader1Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader2Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-17-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader3Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader4Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader5Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-11-*-*-*-*-*-*-*"));
      XmxSetArg (WbNheader6Font, wrapFont("-b&h-lucidabright-demibold-r-normal-*-10-*-*-*-*-*-*-*"));
      XmxSetArg (WbNaddressFont, wrapFont("-b&h-lucidabright-medium-i-normal-*-14-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainFont, wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainboldFont, wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNplainitalicFont, wrapFont("-adobe-courier-medium-o-normal-*-12-*-*-*-*-*-*-*"));
      XmxSetArg (WbNsupSubFont, wrapFont("-b&h-lucidabright-medium-r-normal-*-8-*-*-*-*-*-*-*"));

      XmxSetValues (win->scrolled_win);
      win->font_family = 3;
      break;
    }

  XmxRSetToggleState (win->menubar, win->font_size, XmxNotSet);
  XmxRSetToggleState (win->menubar, size, XmxSet);

  win->font_size = size;

  return mo_succeed;
}

/* -------------------------- mo_set_underlines --------------------------- */

mo_status mo_set_underlines (mo_window *win, int choice)
{
  if (!win->underlines_snarfed)
    {
      XtVaGetValues (win->scrolled_win,
                     WbNanchorUnderlines, &(win->underlines),
                     WbNvisitedAnchorUnderlines, &(win->visited_underlines),
                     WbNdashedAnchorUnderlines, &(win->dashed_underlines),
                     WbNdashedVisitedAnchorUnderlines, 
                     &(win->dashed_visited_underlines),
                     NULL);
      win->underlines_snarfed = 1;
    }

  switch (choice)
    {
    case mo_default_underlines:
      XmxSetArg (WbNanchorUnderlines, win->underlines);
      XmxSetArg (WbNvisitedAnchorUnderlines, win->visited_underlines);
      XmxSetArg (WbNdashedAnchorUnderlines, win->dashed_underlines);
      XmxSetArg (WbNdashedVisitedAnchorUnderlines, 
                 win->dashed_visited_underlines);
      XmxSetValues (win->scrolled_win);
      break;
    case mo_l1_underlines:
      XmxSetArg (WbNanchorUnderlines, 1);
      XmxSetArg (WbNvisitedAnchorUnderlines, 1);
      XmxSetArg (WbNdashedAnchorUnderlines, False);
      XmxSetArg (WbNdashedVisitedAnchorUnderlines, True);
      XmxSetValues (win->scrolled_win);
      break;
    case mo_l2_underlines:
      XmxSetArg (WbNanchorUnderlines, 1);
      XmxSetArg (WbNvisitedAnchorUnderlines, 1);
      XmxSetArg (WbNdashedAnchorUnderlines, False);
      XmxSetArg (WbNdashedVisitedAnchorUnderlines, False);
      XmxSetValues (win->scrolled_win);
      break;
    case mo_l3_underlines:
      XmxSetArg (WbNanchorUnderlines, 2);
      XmxSetArg (WbNvisitedAnchorUnderlines, 1);
      XmxSetArg (WbNdashedAnchorUnderlines, False);
      XmxSetArg (WbNdashedVisitedAnchorUnderlines, False);
      XmxSetValues (win->scrolled_win);
      break;
    case mo_no_underlines:
      XmxSetArg (WbNanchorUnderlines, 0);
      XmxSetArg (WbNvisitedAnchorUnderlines, 0);
      XmxSetArg (WbNdashedAnchorUnderlines, False);
      XmxSetArg (WbNdashedVisitedAnchorUnderlines, False);
      XmxSetValues (win->scrolled_win);
      break;
    }

  XmxRSetToggleState (win->menubar, win->underlines_state, XmxNotSet);
  XmxRSetToggleState (win->menubar, choice, XmxSet);
  win->underlines_state = choice;
  
  return mo_succeed;
}

/* --------------------------- exit_confirm_cb ---------------------------- */

static XmxCallback (clear_history_confirm_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  
  if (XmxExtractToken ((int)client_data))
    {
      mo_window *w = NULL;
      mo_wipe_global_history (win);

      while (w = mo_next_window (w))
        mo_redisplay_window (w);
    }
  else
    XtUnmanageChild (w);
  
  return;
}

/* ----------------------- mo_do_delete_annotation ------------------------ */

/* Presumably we're on an annotation. */
static mo_status mo_do_delete_annotation (mo_window *win)
{
  char *author, *title, *text, *fname;
  int id;

  if (!win->current_node)
    return mo_fail;

  if (win->current_node->annotation_type == mo_annotation_private)
    {
      mo_grok_pan_pieces (win->current_node->url,
                          win->current_node->text,
                          &title, &author, &text, 
                          &id, &fname);
      
      mo_delete_annotation (win, id);
    }
  else if (win->current_node->annotation_type == mo_annotation_workgroup)
    {
      mo_delete_group_annotation (win, win->current_node->url);
    }

  return mo_succeed;
}

static XmxCallback (delete_annotation_confirm_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  if (!win->current_node)
    return;

  if (!mo_is_editable_annotation (win, win->current_node->text))
    return;
  
  if (XmxExtractToken ((int)client_data))
    mo_do_delete_annotation (win);
  
  return;
}


/* --------------------------agent menubar_cb ------------------------------ */

void mo_set_agents(mo_window *win, int which) {

	XmxRSetToggleState(win->menubar,
			   win->agent_state,
			   XmxNotSet);
	XmxRSetToggleState(win->menubar,
			   which,
			   XmxSet);
	win->agent_state=which;
	selectedAgent=which-mo_last_entry;
}


static XmxCallback (agent_menubar_cb) {

mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
int i = XmxExtractToken ((int)client_data);

	mo_set_agents(win,i);
	return;
}
  

/* ------------------------------ menubar_cb ------------------------------ */

XmxCallback (menubar_cb)
{
  struct ele_rec *eptr = NULL;
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  int i = XmxExtractToken ((int)client_data);
  int j=0;
  char *grp, buf[512];

  if(!win)
    { /* this may be from the popup menu */
      act_struct *acst = (act_struct *) client_data;
      i = acst->act_code;
      win = current_win;
      eptr = acst->eptr;
    }
  
  switch (i)
    {
    case mo_reload_document:
      if (cci_event) MoCCISendEventOutput(MOSAIC_RELOAD_CURRENT);
      mo_reload_window_text (win, 0);
      break;
    case mo_reload_document_and_images:
      if (cci_event) MoCCISendEventOutput(FILE_RELOAD_IMAGES);
      mo_reload_window_text (win, 1);
      break;
    case mo_refresh_document:
      if (cci_event) MoCCISendEventOutput(FILE_REFRESH_CURRENT);
      mo_refresh_window_text (win);
      break;
    case mo_re_init:
      if (cci_event) MoCCISendEventOutput(OPTIONS_RELOAD_CONFIG_FILES);
      mo_re_init_formats ();
      break;
    case mo_clear_image_cache:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FLUSH_IMAGE_CACHE);
      XmUpdateDisplay (win->base);
      mo_flush_image_cache (win);
      /* Force a complete reload...nothing else we can do -- SWP */
      mo_reload_window_text (win, 1);
      break;
    case mo_clear_passwd_cache:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FLUSH_PASSWD_CACHE);
      mo_flush_passwd_cache (win);
      break;
    case mo_cci:
      if (cci_event) MoCCISendEventOutput(FILE_CCI);
      MoDisplayCCIWindow(win);
      break;
    case mo_document_source:
      if (cci_event) MoCCISendEventOutput(FILE_VIEW_SOURCE);
      mo_post_source_window (win);
      break;
    case mo_document_edit:
      if (cci_event) MoCCISendEventOutput(FILE_EDIT_SOURCE);
      mo_edit_source(win);
      break;
    case mo_search:
      if (cci_event) MoCCISendEventOutput(FILE_FIND_IN_CURRENT);
      mo_post_search_window (win);
      break;
    case mo_open_document:
      if (cci_event) MoCCISendEventOutput(MOSAIC_OPEN_URL);
      mo_post_open_window (win);
      break;
    case mo_open_local_document:
      if (cci_event) MoCCISendEventOutput(FILE_OPEN_LOCAL);
      mo_post_open_local_window (win);
      break;
    case mo_save_document:
      if (cci_event) MoCCISendEventOutput(MOSAIC_SAVE_AS);
      mo_post_save_window (win);
      break;
    case mo_mail_document:
      if (cci_event) MoCCISendEventOutput(FILE_MAIL_TO);
      mo_post_mail_window (win);
      break;
    case mo_print_document:
      if (cci_event) MoCCISendEventOutput(FILE_PRINT);
      mo_post_print_window (win);
      break;
    case mo_new_window:
      if (cci_event) MoCCISendEventOutput(MOSAIC_NEW);
      mo_open_another_window (win, home_document, NULL, NULL);
      break;
    case mo_clone_window:
      if (cci_event) MoCCISendEventOutput(MOSAIC_CLONE);
      mo_duplicate_window (win);
      break;
    case mo_close_window:
      if (cci_event) MoCCISendEventOutput(MOSAIC_CLOSE);
      mo_delete_window (win);
      break;
    case mo_exit_program:
      if (cci_event) MoCCISendEventOutput(FILE_EXIT_PROGRAM);
      mo_post_exitbox ();
      break;
#ifdef HAVE_DTM
    case mo_dtm_open_outport:
      if (cci_event) MoCCISendEventOutput(FILE_OPEN_DTM_OUTPORT);
      mo_post_dtmout_window (win);
      break;
    case mo_dtm_send_document:
      if (cci_event) MoCCISendEventOutput(FILE_BROADCAST_DOCUMENT);
      mo_send_document_over_dtm (win);
      break;
#endif
#ifdef KRB4
    case mo_kerberosv4_login:
      if (cci_event) MoCCISendEventOutput(FILE_KERBEROS_V4_LOGIN);
      scheme_login(HTAA_KERBEROS_V4);
      break;
#endif
#ifdef KRB5
    case mo_kerberosv5_login:
      if (cci_event) MoCCISendEventOutput(FILE_KERBEROS_V5_LOGIN);
      scheme_login(HTAA_KERBEROS_V5);
      break;
#endif
    case mo_proxy:
	ShowProxyDialog(win);
	break;
    case mo_no_proxy:
	ShowNoProxyDialog(win);
	break;
    case mo_home_document:
      if (cci_event) MoCCISendEventOutput(MOSAIC_HOME_DOCUMENT);
      mo_access_document (win, home_document);
      break;
    case mo_network_starting_points:
      if (cci_event) MoCCISendEventOutput(NAVIGATE_INTERNET_STARTING_POINTS);
      mo_access_document (win, NETWORK_STARTING_POINTS_DEFAULT);
      break;
    case mo_internet_metaindex:
      if (cci_event) 
	MoCCISendEventOutput(NAVIGATE_INTERNET_RESOURCES_META_INDEX);
      mo_access_document (win, INTERNET_METAINDEX_DEFAULT);
      break;
    case mo_mosaic_demopage:
      if (cci_event) MoCCISendEventOutput(HELP_DEMO);
      mo_open_another_window
        (win, DEMO_PAGE_DEFAULT,
         NULL, NULL);
      break;
    case mo_mosaic_manual:
      if (cci_event) MoCCISendEventOutput(HELP_MANUAL);
      mo_open_another_window
        (win, mo_assemble_help_url ("mosaic-docs.html"),
         NULL, NULL);
      break;

    case mo_back:
      if (cci_event) MoCCISendEventOutput(MOSAIC_BACK);
      mo_back_node (win);
      break;
    case mo_forward:
      if (cci_event) MoCCISendEventOutput(MOSAIC_FORWARD);
      mo_forward_node (win);
      break;
    case mo_history_list:
      if (cci_event) MoCCISendEventOutput(NAVIGATE_WINDOW_HISTORY);
      mo_post_history_win (win);
      break;
    case mo_clear_global_history:
      if (cci_event) MoCCISendEventOutput(OPTIONS_CLEAR_GLOBAL_HISTORY);
      XmxSetUniqid (win->id);
      XmxMakeQuestionDialog
        (win->base, "Are you sure you want to clear the global history?" ,
         "NCSA Mosaic: Clear Global History" , clear_history_confirm_cb, 1, 0);
      XtManageChild (Xmx_w);
      break;
    case mo_hotlist_postit:
      if (cci_event) MoCCISendEventOutput(NAVIGATE_HOTLIST);
      mo_post_hotlist_win (win);
      break;
    case mo_register_node_in_default_hotlist:
      if (cci_event) MoCCISendEventOutput(NAVIGATE_ADD_CURRENT_TO_HOTLIST);
      if (win->current_node)
        {
          mo_add_node_to_current_hotlist (win);
          mo_write_default_hotlist ();
        }
      break;
    case mo_all_hotlist_to_rbm:
	if (!win->hotlist_win) {
		win->current_hotlist = (mo_hotlist *)default_hotlist;
	}
	mo_rbm_myself_to_death(win,1);
	break;
    case mo_all_hotlist_from_rbm:
	if (!win->hotlist_win) {
		win->current_hotlist = (mo_hotlist *)default_hotlist;
	}
	mo_rbm_myself_to_death(win,0);
	break;
/* removed 5/17/96 - bjs 
    case mo_fancy_selections:
      win->pretty = 1 - win->pretty;
      mo_set_fancy_selections_toggle (win);
      HTMLClearSelection (win->scrolled_win);
      XmxSetArg (WbNfancySelections, win->pretty ? True : False);
      XmxSetValues (win->scrolled_win);
      if (cci_event)
      {
	if (win->pretty) MoCCISendEventOutput(OPTIONS_FANCY_SELECTIONS_ON);
	else MoCCISendEventOutput(OPTIONS_FANCY_SELECTIONS_OFF);
      }
      break;

      */
          /*SWP*/
    case mo_table_support:
      tableSupportEnabled = win->table_support = (win->table_support ? 0 : 1);
      break;
    case mo_body_color:
	win->body_color = (win->body_color ? 0 : 1);
	XtVaSetValues(win->scrolled_win,
		      WbNbodyColors,
		      win->body_color,
		      NULL);
      break;
    case mo_body_images:
	win->body_images = (win->body_images ? 0 : 1);
	XtVaSetValues(win->scrolled_win,
		      WbNbodyImages,
		      win->body_images,
		      NULL);
      break;
    case mo_binary_transfer:
      win->binary_transfer =
        (win->binary_transfer ? 0 : 1);
      if (cci_event)
      {
	if (win->binary_transfer) 
		MoCCISendEventOutput(OPTIONS_LOAD_TO_LOCAL_DISK_ON);  
	else
		MoCCISendEventOutput(OPTIONS_LOAD_TO_LOCAL_DISK_OFF);
      }
      break;
    case mo_delay_image_loads:
      win->delay_image_loads =
        (win->delay_image_loads ? 0 : 1);
      XmxSetArg (WbNdelayImageLoads, win->delay_image_loads ? True : False);
      XmxSetValues (win->scrolled_win);
      XmxRSetSensitive (win->menubar, mo_expand_images_current,
                        win->delay_image_loads ? XmxSensitive : XmxNotSensitive);
      if (cci_event)
      {
	if (win->delay_image_loads)
		MoCCISendEventOutput(OPTIONS_DELAY_IMAGE_LOADING_ON);
	else
		MoCCISendEventOutput(OPTIONS_DELAY_IMAGE_LOADING_OFF);
      }
      break;
    case mo_expand_images_current:
      if (cci_event) MoCCISendEventOutput(OPTIONS_LOAD_IMAGES_IN_CURRENT);
      XmxSetArg (WbNdelayImageLoads, False);
      XmxSetValues (win->scrolled_win);
      mo_refresh_window_text (win);
      XmxSetArg (WbNdelayImageLoads, win->delay_image_loads ? True : False);
      XmxSetValues (win->scrolled_win);
      break;
    case mo_image_view_internal:
      imageViewInternal = win->image_view_internal = (win->image_view_internal ? 0 : 1);
      break;
    case mo_large_fonts:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_TL);
      mo_set_fonts (win, i);
    break;
    case mo_regular_fonts:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_TR);
      mo_set_fonts (win, i);
    break;
    case mo_small_fonts:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_TS);
      mo_set_fonts (win, i);
    break;
    case mo_large_helvetica:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_HL);
      mo_set_fonts (win, i);
    break;
    case mo_regular_helvetica:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_HR);
      mo_set_fonts (win, i);
    break;
    case mo_small_helvetica:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_HS);
      mo_set_fonts (win, i);
    break;
    case mo_large_newcentury:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_NCL);
      mo_set_fonts (win, i);
    break;
    case mo_regular_newcentury:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_NCR);
      mo_set_fonts (win, i);
    break;
    case mo_small_newcentury:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_NCS);
      mo_set_fonts (win, i);
    break;
    case mo_large_lucidabright:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_LBL);
      mo_set_fonts (win, i);
    break;
    case mo_regular_lucidabright:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_LBR);
      mo_set_fonts (win, i);
    break;
    case mo_small_lucidabright:
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_LBS);
      mo_set_fonts (win, i);
    break;

    case mo_default_underlines:
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_DU);
      mo_set_underlines (win, i);
    break;
    case mo_l1_underlines:
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_LU);
      mo_set_underlines (win, i);
    break;
    case mo_l2_underlines:
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_MU);
      mo_set_underlines (win, i);
    break;
    case mo_l3_underlines:
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_HU);
      mo_set_underlines (win, i);
    break;
    case mo_no_underlines:
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_NU);
      mo_set_underlines (win, i);
    break;
    case mo_help_about:
      if (cci_event) MoCCISendEventOutput(HELP_ABOUT);
      mo_open_another_window
        (win, mo_assemble_help_url ("help-about.html"),
         NULL, NULL);
      break;
    case mo_help_onwindow:
      if (cci_event) MoCCISendEventOutput(HELP_ON_WINDOW);
      mo_open_another_window
        (win, mo_assemble_help_url ("help-on-docview-window.html"),
         NULL, NULL);
      break;
    case mo_whats_new:
      if (cci_event) MoCCISendEventOutput(HELP_WHATS_NEW);
      mo_open_another_window
        (win, WHATSNEW_PAGE_DEFAULT,
         NULL, NULL);
      break;
    case mo_help_onversion:
      if (cci_event) MoCCISendEventOutput(HELP_ON_VERSION);
      mo_open_another_window
        (win, MO_HELP_ON_VERSION_DOCUMENT,
         NULL, NULL);
      break;
    case mo_help_faq:
      if (cci_event) MoCCISendEventOutput(HELP_ON_FAQ);
      mo_open_another_window (win, mo_assemble_help_url ("mosaic-faq.html"), 
                              NULL, NULL);
      break;
    case mo_help_html:
      if (cci_event) MoCCISendEventOutput(HELP_ON_HTML);
      mo_open_another_window (win, HTMLPRIMER_PAGE_DEFAULT, 
                              NULL, NULL);
      break;
    case mo_help_url:
      if (cci_event) MoCCISendEventOutput(HELP_ON_URLS);
      mo_open_another_window (win, URLPRIMER_PAGE_DEFAULT, 
                              NULL, NULL);
      break;
#ifndef PRERELEASE
    case mo_cc:
	if (cci_event) MoCCISendEventOutput(HELP_COMMENT_CARD);
	do_comment=1;
	CommentCard(win);
      break;
#endif
    case mo_techsupport:        
        if (cci_event) MoCCISendEventOutput(HELP_MAIL_TECH_SUPPORT);

        {
            char subj[128];
            
            sprintf (subj, "User Feedback -- NCSA Mosaic %s on %s.",
                     MO_VERSION_STRING, MO_MACHINE_TYPE);
      
            mo_post_mailto_win(MO_DEVELOPER_ADDRESS,subj);
        }
        
/*      mo_post_techsupport_win (win);*/
      break;
    case mo_annotate:
      if (cci_event) MoCCISendEventOutput(ANNOTATE_ANNOTATE);
      mo_post_annotate_win (win, 0, 0, NULL, NULL, NULL, NULL);
      break;
    case mo_news_prev:
      if (cci_event) MoCCISendEventOutput(NEWS_PREV);
      gui_news_prev(win);
      break;
    case mo_news_next:
      if (cci_event) MoCCISendEventOutput(NEWS_NEXT);
      gui_news_next(win);
      break;
    case mo_news_prevt:
      if (cci_event) MoCCISendEventOutput(NEWS_PREV_THREAD);
      gui_news_prevt(win);
      break;
    case mo_news_nextt:
      if (cci_event) MoCCISendEventOutput(NEWS_NEXT_THREAD);
      gui_news_nextt(win);
      break;
    case mo_news_index:
      if (cci_event) MoCCISendEventOutput(NEWS_INDEX);
      gui_news_index(win);
      break;
    case mo_news_flush:
      gui_news_flush(win);
      break;
    case mo_news_flushgroup:
      gui_news_flushgroup(win);
      break;
    case mo_news_sub_anchor:
      if (NewsGroupS) {
	gui_news_subgroup (win);
	break;
      }
      if (!eptr)
	break;
      grp = &eptr->anchorHRef[5];
      subscribegroup (grp);
      sprintf (buf, "%s successfully subscribed", grp);
      HTProgress (buf);
      break;
    case mo_news_sub:
      gui_news_subgroup(win);
      break;
    case mo_news_unsub_anchor:
      if (NewsGroupS) {
	gui_news_unsubgroup (win);
	break;
      }
      if (!eptr)
	break;
      grp = &eptr->anchorHRef[5];
      unsubscribegroup (grp);
      sprintf (buf, "%s successfully unsubscribed", grp);
      HTProgress (buf);
      break;
    case mo_news_unsub:
      gui_news_unsubgroup(win);
      break;
    case mo_news_grp0:
      gui_news_showAllGroups (win);
      break;
    case mo_news_grp1:
      gui_news_showGroups (win);
      break;
    case mo_news_grp2:
      gui_news_showReadGroups (win);
      break;
    case mo_news_art0:
      gui_news_showAllArticles (win);
      break;
    case mo_news_art1:
      gui_news_showArticles (win);
      break;
    case mo_news_mread_anchor:
      if (NewsGroupS)
	gui_news_markGroupRead (win);
      if (!eptr)
	break;
      grp = &eptr->anchorHRef[5];
      NewsGroupS = findgroup (grp);
      if (!NewsGroupS)
	break;
      markrangeread (NewsGroupS, NewsGroupS->minart, NewsGroupS->maxart);
      sprintf (buf, "All articles in %s marked read", NewsGroupS->name);
      HTProgress (buf);
      NewsGroupS = NULL;
      /* Return to newsgroup list */
      sprintf (buf, "news:*");
      mo_load_window_text (win, buf, NULL);
      break;
      
    case mo_news_mread:
      gui_news_markGroupRead (win);
      break;
    case mo_news_munread:
      gui_news_markGroupUnread (win);
      break;
    case mo_news_maunread:
      gui_news_markArticleUnread (win);
      break;
    case mo_news_groups:
    case mo_news_list:
      if (cci_event) MoCCISendEventOutput(NEWS_LIST_GROUPS);
      gui_news_list(win);
      break;
    case mo_news_fmt0:
      if (cci_event) MoCCISendEventOutput(NEWS_FORMAT_TV);
      HTSetNewsConfig (1,-1,-1,-1,-1,-1,-1,-1);
      XmxRSetToggleState (win->menubar, mo_news_fmt1, XmxNotSet);
      XmxRSetToggleState (win->menubar, mo_news_fmt0, XmxSet);
      mo_reload_window_text (win, 0);
      break;
    case mo_news_fmt1:
      if (cci_event) MoCCISendEventOutput(NEWS_FORMAT_GV);
      HTSetNewsConfig (0,-1,-1,-1,-1,-1,-1,-1);
      XmxRSetToggleState (win->menubar, mo_news_fmt0, XmxNotSet);
      XmxRSetToggleState (win->menubar, mo_news_fmt1, XmxSet);
      mo_reload_window_text (win, 0);
      break;
    case mo_news_post:
      if (cci_event) MoCCISendEventOutput(NEWS_POST);
      mo_post_news_win(win);
      break;
    case mo_news_follow:
      if (cci_event) MoCCISendEventOutput(NEWS_FOLLOW_UP);
      mo_post_follow_win(win);
      break;


      /* Handle FTP stuff here */
    case mo_ftp_put:
      if (cci_event) MoCCISendEventOutput (FTP_PUT);
      mo_handle_ftpput (win);
      break;
    case mo_ftp_mkdir:
      if (cci_event) MoCCISendEventOutput (FTP_MKDIR);
      mo_handle_ftpmkdir (win);
      break;

    /* Tag and Bag */
    /*
    case mo_tag_current:
      mo_tagnbag_current (win);
      break;
    case mo_tag_url:
      mo_tagnbag_url (win);
      break;
    */

    case mo_links_window:
        mo_post_links_window(win);
        break;
#ifdef HAVE_AUDIO_ANNOTATIONS
    case mo_audio_annotate:
      if (cci_event) MoCCISendEventOutput(ANNOTATE_AUDIO_ANNOTATE);
      mo_post_audio_annotate_win (win);
      break;
#endif
    case mo_annotate_edit:
      /* OK, let's be smart.
         If we get here, we know we're viewing an editable
         annotation.
         We also know the filename (just strip the leading
         file: off the URL).
         We also know the ID, by virtue of the filename
         (just look for PAN-#.html. */
      if (cci_event) MoCCISendEventOutput(ANNOTATE_EDIT_THIS_ANNOTATION);
      if (win->current_node)
        {
          char *author, *title, *text, *fname;
          int id;
          
          if (win->current_node->annotation_type == mo_annotation_private)
            {
              mo_grok_pan_pieces (win->current_node->url,
                                  win->current_node->text,
                                  &title, &author, &text, 
                                  &id, &fname);
              
              mo_post_annotate_win (win, 1, id, title, author, text, fname);
            }
          else if (win->current_node->annotation_type == mo_annotation_workgroup)
            {
              mo_grok_grpan_pieces (win->current_node->url,
                                    win->current_node->text,
                                    &title, &author, &text, 
                                    &id, &fname);
              mo_post_annotate_win (win, 1, id, title, author, text, fname);
            }
        }
      break;
    case mo_annotate_delete:
      if (cci_event) MoCCISendEventOutput(ANNOTATE_DELETE_THIS_ANNOTATION);
      if (get_pref_boolean(eCONFIRM_DELETE_ANNOTATION))
        {
          XmxSetUniqid (win->id);
          XmxMakeQuestionDialog
            (win->base, "Are you sure you want to delete this annotation?" ,
             "NCSA Mosaic: Delete Annotation" , delete_annotation_confirm_cb, 1, 0);
          XtManageChild (Xmx_w);
        }
      else
        mo_do_delete_annotation (win);
      break;
    default:
      if (i >= DOCUMENTS_MENU_COUNT_OFFSET)
        mo_access_document (win, urllist[i - DOCUMENTS_MENU_COUNT_OFFSET]);
      break;
    }

  return;
}

/* ------------------------------------------------------------------------ */
/* --------------------------- Colleen menubar ---------------------------- */
/* ------------------------------------------------------------------------ */
static XmxMenubarStruct *file_menuspec;
static XmxMenubarStruct *fnts_menuspec;
static XmxMenubarStruct *undr_menuspec;
static XmxMenubarStruct *agent_menuspec;
static XmxMenubarStruct *opts_menuspec;
static XmxMenubarStruct *navi_menuspec;
static XmxMenubarStruct *help_menuspec;
static XmxMenubarStruct *anno_menuspec;
static XmxMenubarStruct *newsfmt_menuspec;
static XmxMenubarStruct *newsgrpfmt_menuspec;
static XmxMenubarStruct *newsartfmt_menuspec;
static XmxMenubarStruct *news_menuspec;
static XmxMenubarStruct *menuspec;

/* ----------------------- simple menubar interface ----------------------- */
static XmxMenubarStruct *file_simple_menuspec;
static XmxMenubarStruct *navi_simple_menuspec;
static XmxMenubarStruct *opts_simple_menuspec;
static XmxMenubarStruct *help_simple_menuspec;
static XmxMenubarStruct *anno_simple_menuspec;
static XmxMenubarStruct *simple_menuspec;

/* --------------------------- format options ----------------------------- */
extern XmxOptionMenuStruct *format_opts;

/* -------------------------- annotation options -------------------------- */
extern XmxOptionMenuStruct *pubpri_opts;


#ifndef DISABLE_TRACE
/* ----------------------- macros for menubar stuff ----------------------- */
#define ALLOC_MENUBAR(menuPtr,numEntries) \
{ \
	(menuPtr)=(XmxMenubarStruct *)calloc((numEntries),sizeof(XmxMenubarStruct)); \
	memset((menuPtr),0,((numEntries)*sizeof(XmxMenubarStruct))); \
	maxMenuCnt=(numEntries); \
	menuCnt=0; \
	current=(menuPtr); \
}
#define ALLOC_OPTIONS(optPtr,numOpts) \
{ \
	(optPtr)=(XmxOptionMenuStruct *)calloc((numOpts),sizeof(XmxOptionMenuStruct)); \
	memset((optPtr),0,((numOpts)*sizeof(XmxOptionMenuStruct))); \
	maxMenuCnt=(numOpts); \
	menuCnt=0; \
	ocurrent=(optPtr); \
}
#define DEFINE_MENUBAR(nameStr,mnemonicStr,cb,cbData,subMenu) \
{ \
	if (menuCnt>=maxMenuCnt) { \
		if (srcTrace) { \
			fprintf(stderr,"Trying to allocate more option menu entries than allowed!\n\n"); \
		} \
		exit(1); \
	} \
	if ((nameStr) && *(nameStr)) { \
		current[menuCnt].namestr=strdup((nameStr)); \
	} \
	else { \
		current[menuCnt].namestr=NULL; \
	} \
	if ((mnemonicStr) && *(mnemonicStr)) { \
		current[menuCnt].mnemonic=(*(mnemonicStr)); \
	} \
	else { \
		current[menuCnt].mnemonic='\0'; \
	} \
	if ((cb)!=NULL) { \
		current[menuCnt].func=(cb); \
	} \
	current[menuCnt].data=(cbData); \
	current[menuCnt].sub_menu=(subMenu); \
	menuCnt++; \
}
#define DEFINE_OPTIONS(nameStr,optData,optState) \
{ \
	if (menuCnt>=maxMenuCnt) { \
		if (srcTrace) { \
			fprintf(stderr,"Trying to allocate more menu entries than allowed!\n\n"); \
		} \
		exit(1); \
	} \
	if ((nameStr) && *(nameStr)) { \
		ocurrent[menuCnt].namestr=strdup((nameStr)); \
	} \
	else { \
		ocurrent[menuCnt].namestr=NULL; \
	} \
	ocurrent[menuCnt].data=(optData); \
	ocurrent[menuCnt].set_state=(optState); \
	menuCnt++; \
}
#define NULL_MENUBAR() \
{ \
	current[menuCnt].namestr=NULL; \
	current[menuCnt].mnemonic='\0'; \
	current[menuCnt].func=NULL; \
	current[menuCnt].data=0; \
	current[menuCnt].sub_menu=NULL; \
	menuCnt++; \
}
#define NULL_OPTIONS() \
{ \
	ocurrent[menuCnt].namestr=NULL; \
	ocurrent[menuCnt].data=0; \
	ocurrent[menuCnt].set_state=XmxNotSet; \
	menuCnt++; \
}
#define SPACER() \
{ \
	current[menuCnt].namestr=strdup("----"); \
	current[menuCnt].mnemonic='\0'; \
	current[menuCnt].func=NULL; \
	current[menuCnt].data=0; \
	current[menuCnt].sub_menu=NULL; \
	menuCnt++; \
}

#else /*Take out the srcTrace stuff*/

/* ----------------------- macros for menubar stuff ----------------------- */
#define ALLOC_MENUBAR(menuPtr,numEntries) \
{ \
	(menuPtr)=(XmxMenubarStruct *)calloc((numEntries),sizeof(XmxMenubarStruct)); \
	memset((menuPtr),0,((numEntries)*sizeof(XmxMenubarStruct))); \
	maxMenuCnt=(numEntries); \
	menuCnt=0; \
	current=(menuPtr); \
}
#define ALLOC_OPTIONS(optPtr,numOpts) \
{ \
	(optPtr)=(XmxOptionMenuStruct *)calloc((numOpts),sizeof(XmxOptionMenuStruct)); \
	memset((optPtr),0,((numOpts)*sizeof(XmxOptionMenuStruct))); \
	maxMenuCnt=(numOpts); \
	menuCnt=0; \
	ocurrent=(optPtr); \
}
#define DEFINE_MENUBAR(nameStr,mnemonicStr,cb,cbData,subMenu) \
{ \
	if (menuCnt>=maxMenuCnt) { \
		exit(1); \
	} \
	if ((nameStr) && *(nameStr)) { \
		current[menuCnt].namestr=strdup((nameStr)); \
	} \
	else { \
		current[menuCnt].namestr=NULL; \
	} \
	if ((mnemonicStr) && *(mnemonicStr)) { \
		current[menuCnt].mnemonic=(*(mnemonicStr)); \
	} \
	else { \
		current[menuCnt].mnemonic='\0'; \
	} \
	if ((cb)!=NULL) { \
		current[menuCnt].func=(cb); \
	} \
	current[menuCnt].data=(cbData); \
	current[menuCnt].sub_menu=(subMenu); \
	menuCnt++; \
}
#define DEFINE_OPTIONS(nameStr,optData,optState) \
{ \
	if (menuCnt>=maxMenuCnt) { \
		exit(1); \
	} \
	if ((nameStr) && *(nameStr)) { \
		ocurrent[menuCnt].namestr=strdup((nameStr)); \
	} \
	else { \
		ocurrent[menuCnt].namestr=NULL; \
	} \
	ocurrent[menuCnt].data=(optData); \
	ocurrent[menuCnt].set_state=(optState); \
	menuCnt++; \
}
#define NULL_MENUBAR() \
{ \
	current[menuCnt].namestr=NULL; \
	current[menuCnt].mnemonic='\0'; \
	current[menuCnt].func=NULL; \
	current[menuCnt].data=0; \
	current[menuCnt].sub_menu=NULL; \
	menuCnt++; \
}
#define NULL_OPTIONS() \
{ \
	ocurrent[menuCnt].namestr=NULL; \
	ocurrent[menuCnt].data=0; \
	ocurrent[menuCnt].set_state=XmxNotSet; \
	menuCnt++; \
}
#define SPACER() \
{ \
	current[menuCnt].namestr=strdup("----"); \
	current[menuCnt].mnemonic='\0'; \
	current[menuCnt].func=NULL; \
	current[menuCnt].data=0; \
	current[menuCnt].sub_menu=NULL; \
	menuCnt++; \
}
#endif
/* -------------------------- mo_init_menubar ----------------------------- */
/*
   This function allocates the menubar variables and properly defines them
   according to the international resources set.

   ALLOC_MENUBAR(menuPtr,numEntries) allows you to give it an address and
     it will autocate the specified numbber of pointers for the menubar.
     menuPtr -- XmxMenubarStruct *
     numEntries -- int

   ALLOC_OPTIONS(optPtr,numOpts) allows you to autocate the number of options
     in the option menu.
     optPtr -- XmxOptionMenuStruct *
     numOpts -- int

   DEFINE_MENUBAR(nameStr,mnemonic,cb,cbData,subMenu) allows you to
     actually fill in the menubar struct.
     nameStr -- char *
     mnemonic -- char *   (only first character is used)
     cb -- void (*func)()
     cbData -- int
     subMenu -- XmxMenubarStruct *

   DEFINE_OPTIONS(nameStr,optData,optState) allows you to
     actually fill in the option menu struct.
     nameStr -- char *
     optData -- int
     optState -- int

   NULL_MENUBAR() defines the current menu entry to be NULL, thus ending
     the current definition.

   NULL_OPTIONS() defines the current menu entry to be NULL, thus ending
     the current definition.

   SPACER() defines a <hr> for a menu.

   Note: To create submenus, you use ALLOC_MENUBAR on the "sub_menu" attribute
     of the XmxMenubarStruct (on an already allocated menubar). Also, the
     XmxMenubarStruct for the sub_menu must already be allocated...
*/
mo_status mo_init_menubar(void) {

int maxMenuCnt,menuCnt,i;
XmxMenubarStruct *current;
XmxOptionMenuStruct *ocurrent;
char buf[BUFSIZ];

/* --------------------------- format options ------------------------------ */
	ALLOC_OPTIONS(format_opts,5)
	DEFINE_OPTIONS("Plain Text",mo_plaintext,XmxNotSet)
	DEFINE_OPTIONS("Formatted Text",mo_formatted_text,XmxNotSet)
	DEFINE_OPTIONS("PostScript",mo_postscript,XmxNotSet)
	DEFINE_OPTIONS("HTML",mo_html,XmxNotSet)
	NULL_OPTIONS()

/* -------------------------- annotation options --------------------------- */
	ALLOC_OPTIONS(pubpri_opts,4)
	DEFINE_OPTIONS("Personal Annotation",mo_annotation_private,XmxSet)
	DEFINE_OPTIONS("Workgroup Annotation",mo_annotation_workgroup,XmxNotSet)
	DEFINE_OPTIONS("Public Annotation",mo_annotation_public,XmxNotSet)
	NULL_OPTIONS()

/* ----------------------- full menubar interface -------------------------- */
	/* File Menu */
	ALLOC_MENUBAR(file_menuspec,32)
	DEFINE_MENUBAR("New" ,"N" ,menubar_cb,mo_new_window,NULL)
	DEFINE_MENUBAR("Clone" ,"C",menubar_cb,mo_clone_window,NULL)
	SPACER()
	DEFINE_MENUBAR("Open URL..." ,"O",menubar_cb,mo_open_document,NULL)
	DEFINE_MENUBAR("Open Local..." ,"L",menubar_cb,mo_open_local_document,NULL)
	SPACER()
	DEFINE_MENUBAR("Reload Current" ,"R",menubar_cb,mo_reload_document,NULL)
	DEFINE_MENUBAR("Reload Images" ,"a",menubar_cb,mo_reload_document_and_images,NULL)
	DEFINE_MENUBAR("Refresh Current" ,"f",menubar_cb,mo_refresh_document,NULL)
	SPACER()
	DEFINE_MENUBAR("Find In Current" ,"I",menubar_cb,mo_search,NULL)
	DEFINE_MENUBAR("View Source..." ,"V",menubar_cb,mo_document_source,NULL)
	DEFINE_MENUBAR("Edit Source..." ,"E",menubar_cb,mo_document_edit,NULL)
	SPACER()
	DEFINE_MENUBAR("Save As..." ,"S",menubar_cb,mo_save_document,NULL)
	DEFINE_MENUBAR("Print..." ,"P",menubar_cb,mo_print_document,NULL)
	DEFINE_MENUBAR("Mail To..." ,"M",menubar_cb,mo_mail_document,NULL)
	SPACER()
	DEFINE_MENUBAR("CCI..." ,"D",menubar_cb,mo_cci,NULL)
#ifdef HAVE_DTM
	SPACER()
	DEFINE_MENUBAR("Open DTM Outport..." ,"u",menubar_cb,mo_dtm_open_outport,NULL)
	DEFINE_MENUBAR("Broadcast Over DTM" ,"B",menubar_cb,mo_dtm_send_document,NULL)
#endif /* HAVE_DTM */
/*SWP -- 7/17/95*/
#if defined(KRB4) || defined(KRB5)
	SPACER()
#ifdef KRB4
	DEFINE_MENUBAR("Kerberos v4 Login..." ,"4",menubar_cb,mo_kerberosv4_login,NULL)
#endif
#ifdef KRB5
	DEFINE_MENUBAR("Kerberos v5 Login..." ,"5",menubar_cb,mo_kerberosv5_login,NULL)
#endif
#endif /*defined(KRB4) || defined(KRB5)*/
	SPACER()
	DEFINE_MENUBAR("Proxy List..." ,"0",menubar_cb,mo_proxy,NULL)
	DEFINE_MENUBAR("No Proxy List..." ,"1",menubar_cb,mo_no_proxy,NULL)
	SPACER()
	DEFINE_MENUBAR("Close" ,"W",menubar_cb,mo_close_window,NULL)
	DEFINE_MENUBAR("Exit Program..." ,"x",menubar_cb,mo_exit_program,NULL)
	NULL_MENUBAR()

	/* Fonts Sub-Menu */
	ALLOC_MENUBAR(fnts_menuspec,16);
	DEFINE_MENUBAR("<Times Regular" ,"T",menubar_cb,mo_regular_fonts,NULL)
	DEFINE_MENUBAR("<Times Small" ,"S",menubar_cb,mo_small_fonts,NULL)
	DEFINE_MENUBAR("<Times Large" ,"L",menubar_cb,mo_large_fonts,NULL)
	SPACER()
	DEFINE_MENUBAR("<Helvetica Regular" ,"H",menubar_cb,mo_regular_helvetica,NULL)
	DEFINE_MENUBAR("<Helvetica Small" ,"e",menubar_cb,mo_small_helvetica,NULL)
	DEFINE_MENUBAR("<Helvetica Large" ,"v",menubar_cb,mo_large_helvetica,NULL)
	SPACER()
	DEFINE_MENUBAR("<New Century Regular" ,"N",menubar_cb,mo_regular_newcentury,NULL)
	DEFINE_MENUBAR("<New Century Small" ,"w",menubar_cb,mo_small_newcentury,NULL)
	DEFINE_MENUBAR("<New Century Large" ,"C",menubar_cb,mo_large_newcentury,NULL)
	SPACER()
	DEFINE_MENUBAR("<Lucida Bright Regular" ,"L",menubar_cb,mo_regular_lucidabright,NULL)
	DEFINE_MENUBAR("<Lucida Bright Small" ,"u",menubar_cb,mo_small_lucidabright,NULL)
	DEFINE_MENUBAR("<Lucida Bright Large" ,"i",menubar_cb,mo_large_lucidabright,NULL)
	NULL_MENUBAR()

	/* Underline Sub-Menu */
	ALLOC_MENUBAR(undr_menuspec,6)
	DEFINE_MENUBAR("<Default Underlines" ,"D",menubar_cb,mo_default_underlines,NULL)
	DEFINE_MENUBAR("<Light Underlines" ,"L",menubar_cb,mo_l1_underlines,NULL)
	DEFINE_MENUBAR("<Medium Underlines" ,"M",menubar_cb,mo_l2_underlines,NULL)
	DEFINE_MENUBAR("<Heavy Underlines" ,"H",menubar_cb,mo_l3_underlines,NULL)
	DEFINE_MENUBAR("<No Underlines" ,"N",menubar_cb,mo_no_underlines,NULL)
	NULL_MENUBAR()

	/* Agent Spoofing Sub-Menu */
	loadAgents();
	ALLOC_MENUBAR(agent_menuspec,numAgents+1);
	for (i=0; i<numAgents; i++) {
		if (agent[i][0]=='-') {
			SPACER()
		}
		else {
			sprintf(buf,"<%s",agent[i]);
			DEFINE_MENUBAR(buf," ",agent_menubar_cb,i+mo_last_entry,NULL)
		}
	}
	NULL_MENUBAR()

	/* Options Menu */
	ALLOC_MENUBAR(opts_menuspec,23)
/*	DEFINE_MENUBAR("#Fancy Selections" ,"S",menubar_cb,mo_fancy_selections,NULL)
	SPACER()
        */
	DEFINE_MENUBAR("#Load to Local Disk" ,"T",menubar_cb,mo_binary_transfer,NULL)
	SPACER()
	DEFINE_MENUBAR("#Table Support" ,"b",menubar_cb,mo_table_support,NULL)
	SPACER()
	DEFINE_MENUBAR("#Body Color" ,"y",menubar_cb,mo_body_color,NULL)
	DEFINE_MENUBAR("#Body (Background) Images" ,"k",menubar_cb,mo_body_images,NULL)
	SPACER()
	DEFINE_MENUBAR("#View Images Internally" ,"V",menubar_cb,mo_image_view_internal,NULL)
	DEFINE_MENUBAR("#Delay Image Loading" ,"D",menubar_cb,mo_delay_image_loads,NULL)
	DEFINE_MENUBAR("Load Images In Current" ,"L",menubar_cb,mo_expand_images_current,NULL)
	SPACER()
	DEFINE_MENUBAR("Reload Config Files" ,"R",menubar_cb,mo_re_init,NULL)
	SPACER()
	DEFINE_MENUBAR("Flush Image Cache" ,"I",menubar_cb,mo_clear_image_cache,NULL)
	DEFINE_MENUBAR("Flush Password Cache" ,"P",menubar_cb,mo_clear_passwd_cache,NULL)
	DEFINE_MENUBAR("Clear Global History..." ,"C",menubar_cb,mo_clear_global_history,NULL)
	SPACER()
	DEFINE_MENUBAR("Fonts" ,"F",NULL,0,fnts_menuspec)
	DEFINE_MENUBAR("Anchor Underlines" ,"A",NULL,0,undr_menuspec)
	DEFINE_MENUBAR("Agent Spoofs","g",NULL,0,agent_menuspec)
	NULL_MENUBAR()

	/* Navigation Menu */
	ALLOC_MENUBAR(navi_menuspec,15)
	DEFINE_MENUBAR("Back" ,"B",menubar_cb,mo_back,NULL)
	DEFINE_MENUBAR("Forward" ,"F",menubar_cb,mo_forward,NULL)
	SPACER()
	DEFINE_MENUBAR("Home Document" ,"D",menubar_cb,mo_home_document,NULL)
	DEFINE_MENUBAR("Window History..." ,"W",menubar_cb,mo_history_list,NULL)
	DEFINE_MENUBAR("Document Links..." ,"L",menubar_cb,mo_links_window,NULL)
	SPACER()
	DEFINE_MENUBAR("Hotlist..." ,"H",menubar_cb,mo_hotlist_postit,NULL)
	DEFINE_MENUBAR("Add Current To Hotlist" ,"A",menubar_cb,mo_register_node_in_default_hotlist,NULL)
	DEFINE_MENUBAR("Add All Hotlist Entries to RBM" ,"E",menubar_cb,mo_all_hotlist_to_rbm,NULL);
	DEFINE_MENUBAR("Remove All Hotlist Entries from RBM" ,"R",menubar_cb,mo_all_hotlist_from_rbm,NULL);
	SPACER()
	DEFINE_MENUBAR("Internet Starting Points" ,"I",menubar_cb,mo_network_starting_points,NULL)
	DEFINE_MENUBAR("Internet Resource Meta-Index" ,"M",menubar_cb,mo_internet_metaindex,NULL)
	NULL_MENUBAR()

	/* Help Menu */
	ALLOC_MENUBAR(help_menuspec,17)
	DEFINE_MENUBAR("About..." ,"A",menubar_cb,mo_help_about,NULL)
	DEFINE_MENUBAR("Manual..." ,"M",menubar_cb,mo_mosaic_manual,NULL)
	SPACER()
	DEFINE_MENUBAR("What's New..." ,"W",menubar_cb,mo_whats_new,NULL)
	DEFINE_MENUBAR("Demo..." ,"D",menubar_cb,mo_mosaic_demopage,NULL)
	SPACER()
	DEFINE_MENUBAR("Help on Version 2.7b5..." ,"V",menubar_cb,mo_help_onversion,NULL)
	DEFINE_MENUBAR("On Window..." ,"O",menubar_cb,mo_help_onwindow,NULL)
	DEFINE_MENUBAR("On FAQ..." ,"F",menubar_cb,mo_help_faq,NULL)
	SPACER()
	DEFINE_MENUBAR("On HTML..." ,"H",menubar_cb,mo_help_html,NULL)
	DEFINE_MENUBAR("On URLS..." ,"U",menubar_cb,mo_help_url,NULL)
	SPACER()
	DEFINE_MENUBAR("Mail Tech Support..." ,"M",menubar_cb,mo_techsupport,NULL)
#ifndef PRERELEASE
	SPACER()
	DEFINE_MENUBAR("Comment Card..." ,"C",menubar_cb,mo_cc,NULL)
#endif
	NULL_MENUBAR()

	/* Annotation Menu */
	ALLOC_MENUBAR(anno_menuspec,6)
	DEFINE_MENUBAR("Annotate..." ,"A",menubar_cb,mo_annotate,NULL)
#ifdef HAVE_AUDIO_ANNOTATIONS
	DEFINE_MENUBAR("Audio Annotate..." ,"u",menubar_cb,mo_audio_annotate,NULL)
#endif
	SPACER()
	DEFINE_MENUBAR("Edit This Annotation..." ,"E",menubar_cb,mo_annotate_edit,NULL)
	DEFINE_MENUBAR("Delete This Annotation..." ,"D",menubar_cb,mo_annotate_delete,NULL)
	NULL_MENUBAR()

	/* News Format Sub-Menu */
	ALLOC_MENUBAR(newsfmt_menuspec,3)
	DEFINE_MENUBAR("<Thread View" ,"T",menubar_cb,mo_news_fmt0,NULL)
	DEFINE_MENUBAR("<Article View" ,"G",menubar_cb,mo_news_fmt1,NULL)
	NULL_MENUBAR()

	/* News Menu */
	ALLOC_MENUBAR(news_menuspec,27)
	DEFINE_MENUBAR("Next" ,"N",menubar_cb,mo_news_next,NULL)
	DEFINE_MENUBAR("Prev" ,"P",menubar_cb,mo_news_prev,NULL)
	DEFINE_MENUBAR("Next Thread" ,"t",menubar_cb,mo_news_nextt,NULL)
	DEFINE_MENUBAR("Prev Thread" ,"v",menubar_cb,mo_news_prevt,NULL)
	DEFINE_MENUBAR("Article Index" ,"I",menubar_cb,mo_news_index,NULL)
	DEFINE_MENUBAR("Group Index" ,"G",menubar_cb,mo_news_groups,NULL)
	SPACER()
	DEFINE_MENUBAR("Post" ,"o",menubar_cb,mo_news_post,NULL)
	DEFINE_MENUBAR("Followup" ,"F",menubar_cb,mo_news_follow,NULL)
	SPACER()
	DEFINE_MENUBAR("Subscribe to Group" ,"s",menubar_cb,mo_news_sub,NULL)
	DEFINE_MENUBAR("Unsubscribe Group" ,"u",menubar_cb,mo_news_unsub,NULL)
	SPACER()
	DEFINE_MENUBAR("<Show All Groups" ,"A",menubar_cb,mo_news_grp0,NULL)
	DEFINE_MENUBAR("<Show Unread Subscribed Groups" ,"S",menubar_cb,mo_news_grp1,NULL)
	DEFINE_MENUBAR("<Show All Subscribed Groups" ,"R",menubar_cb,mo_news_grp2,NULL)
	SPACER()
	DEFINE_MENUBAR("<Show All Articles" ,"l",menubar_cb,mo_news_art0,NULL)
	DEFINE_MENUBAR("<Show Only Unread Articles" ,"n",menubar_cb,mo_news_art1,NULL)
	SPACER()
	DEFINE_MENUBAR("Mark Group Read" ,"e",menubar_cb,mo_news_mread,NULL)
	DEFINE_MENUBAR("Mark Group Unread" ,"d",menubar_cb,mo_news_munread,NULL)
	DEFINE_MENUBAR("Mark Article Unread" ,"M",menubar_cb,mo_news_maunread,NULL)
	SPACER()
	DEFINE_MENUBAR("Flush News Data" ,"F",menubar_cb,mo_news_flush,NULL)
	DEFINE_MENUBAR("Thread Style" ,"T",NULL,0,newsfmt_menuspec)
	NULL_MENUBAR()

	/* The Menubar */
	ALLOC_MENUBAR(menuspec,8)
	DEFINE_MENUBAR("File" ,"F",NULL,0,file_menuspec)
	DEFINE_MENUBAR("Options" ,"O",NULL,0,opts_menuspec)
	DEFINE_MENUBAR("Navigate" ,"N",NULL,0,navi_menuspec)
	DEFINE_MENUBAR("Annotate" ,"A",NULL,0,anno_menuspec)
	DEFINE_MENUBAR("News" ,"w",NULL,0,news_menuspec)
	DEFINE_MENUBAR("Help" ,"H",NULL,0,help_menuspec)
	/* Dummy submenu. */
	NULL_MENUBAR()
	NULL_MENUBAR()

/* ----------------------- simple menubar interface ----------------------- */
	/* File Menu */
	ALLOC_MENUBAR(file_simple_menuspec,7)
	DEFINE_MENUBAR("Clone" ,"C",menubar_cb,mo_clone_window,NULL)
	SPACER()
	DEFINE_MENUBAR("Find In Current" ,"I",menubar_cb,mo_search,NULL)
	SPACER()
	DEFINE_MENUBAR("Close" ,"W",menubar_cb,mo_close_window,NULL)
	DEFINE_MENUBAR("Exit Program..." ,"x",menubar_cb,mo_exit_program,NULL)
	NULL_MENUBAR()

	/* Options Menu */
	ALLOC_MENUBAR(opts_simple_menuspec,2)
	DEFINE_MENUBAR("#Load to Local Disk" ,"T",menubar_cb,mo_binary_transfer,NULL)
	NULL_MENUBAR()

	/* Navigation Menu */
	ALLOC_MENUBAR(navi_simple_menuspec,12)
	DEFINE_MENUBAR("Back" ,"B",menubar_cb,mo_back,NULL)
	DEFINE_MENUBAR("Forward" ,"F",menubar_cb,mo_forward,NULL)
	SPACER()
	DEFINE_MENUBAR("Home Document" ,"D",menubar_cb,mo_home_document,NULL)
	SPACER()
	DEFINE_MENUBAR("Window History..." ,"W",menubar_cb,mo_history_list,NULL)
	SPACER()
	DEFINE_MENUBAR("Hotlist..." ,"H",menubar_cb,mo_hotlist_postit,NULL)
	DEFINE_MENUBAR("Add Current To Hotlist" ,"A",menubar_cb,mo_register_node_in_default_hotlist,NULL)
	DEFINE_MENUBAR("Add All Hotlist Entries to RBM" ,"E",menubar_cb,mo_all_hotlist_to_rbm,NULL);
	DEFINE_MENUBAR("Remove All Hotlist Entries from RBM" ,"R",menubar_cb,mo_all_hotlist_from_rbm,NULL);
	NULL_MENUBAR()

	/* Help Menu */
	ALLOC_MENUBAR(help_simple_menuspec,12)
	DEFINE_MENUBAR("About..." ,"A",menubar_cb,mo_help_about,NULL)
	DEFINE_MENUBAR("Manual..." ,"M",menubar_cb,mo_mosaic_manual,NULL)
	SPACER()
	DEFINE_MENUBAR("What's New..." ,"W",menubar_cb,mo_whats_new,NULL)
	DEFINE_MENUBAR("Demo..." ,"D",menubar_cb,mo_mosaic_demopage,NULL)
	SPACER()
	DEFINE_MENUBAR("Help on Version 2.7b5..." ,"V",menubar_cb,mo_help_onversion,NULL)
	DEFINE_MENUBAR("On Window..." ,"O",menubar_cb,mo_help_onwindow,NULL)
	DEFINE_MENUBAR("On FAQ..." ,"F",menubar_cb,mo_help_faq,NULL)
#ifndef PRERELEASE
	SPACER()
	DEFINE_MENUBAR("Comment Card..." ,"C",menubar_cb,mo_cc,NULL)
#endif
	NULL_MENUBAR()

	/* Annotation Menu */
	ALLOC_MENUBAR(anno_simple_menuspec,6)
	DEFINE_MENUBAR("Annotate..." ,"A",menubar_cb,mo_annotate,NULL)
#ifdef HAVE_AUDIO_ANNOTATIONS
	DEFINE_MENUBAR("Audio Annotate..." ,"u",menubar_cb,mo_audio_annotate,NULL)
#endif
	SPACER()
	DEFINE_MENUBAR("Edit This Annotation..." ,"E",menubar_cb,mo_annotate_edit,NULL)
	DEFINE_MENUBAR("Delete This Annotation..." ,"D",menubar_cb,mo_annotate_delete,NULL)
	NULL_MENUBAR()

	/* The Simple Menubar */
	ALLOC_MENUBAR(simple_menuspec,7)
	DEFINE_MENUBAR("File" ,"F",NULL,0,file_simple_menuspec)
	DEFINE_MENUBAR("Options" ,"O",NULL,0,opts_simple_menuspec)
	DEFINE_MENUBAR("Navigate" ,"N",NULL,0,navi_simple_menuspec)
	DEFINE_MENUBAR("Annotate" ,"A",NULL,0,anno_simple_menuspec)
	DEFINE_MENUBAR("Help" ,"H",NULL,0,help_simple_menuspec)
	/* Dummy submenu. */
	NULL_MENUBAR()
	NULL_MENUBAR()
}


/* -------------------- mo_make_document_view_menubar --------------------- */

/* We now allow a single customizable menu.  

   First choice for the spec file is the value of the resource
   documentsMenuSpecfile.
   If that doesn't exist, second choice is the value of the
   environment variable MOSAIC_DOCUMENTS_MENU_SPECFILE.
   If *that* doesn't exist, third choice is specified in 
   #define DOCUMENTS_MENU_SPECFILE. */

static mo_status mo_file_exists (char *filename)
{
  struct stat buf;
  int r;

  r = stat (filename, &buf); 
  if (r != -1)
    return mo_succeed;
  else
    return mo_fail;
}

static void mo_grok_menubar (char *filename)
{
  FILE *fp;
  char line[MO_LINE_LENGTH];
  char *status;
  XmxMenubarStruct *menu;
  int count;

  fp = fopen (filename, "r");
  if (!fp)
    return;
/*    goto screwed_no_file;*/

  /* Make the menu. */
  menu = (XmxMenubarStruct *) malloc 
    (MAX_DOCUMENTS_MENU_ITEMS * sizeof (XmxMenubarStruct));
  count = 0;
  
  /* File consists of alternating titles and URL's.
     A title consisting of at least two leading dashes
     is a separator. */
  while (1)
    {
      status = fgets (line, MO_LINE_LENGTH, fp);
      if (!status || !(*line))
/*        goto done;*/
	  break;

      if (strlen (line) >= 2 &&
          line[0] == '-' &&
          line[1] == '-')
        {
          /* It's a separator. */
          menu[count].namestr = "----";
          menu[count].mnemonic = 0;
          menu[count].func = 0;
          menu[count].data = 0;
          menu[count].sub_menu = 0;
        }
      else
        {
          /* That's the title. */
          menu[count].namestr = strdup (line);
          /* Wipe out trailing newline. */
          menu[count].namestr[strlen(line)-1] = '\0';
          menu[count].mnemonic = 0;
          menu[count].func = (void (*)())menubar_cb;
          menu[count].data = count + DOCUMENTS_MENU_COUNT_OFFSET;
          menu[count].sub_menu = 0;
          
          status = fgets (line, MO_LINE_LENGTH, fp);
          if (!status || !(*line))
            {
              /* Oops, something went wrong. */
              menu[count].namestr = 0;
              
/*              goto done;*/
		break;
            }
          
          /* There's a URL. */
          urllist[count] = strdup (line);
          urllist[count][strlen(line)-1] = '\0';
        }

      /* Count increases. */
      count++;
    }

/* done:
  /* Make sure we get a NULL in the right place. */
  menu[count].namestr = 0;
  fclose (fp);
  if (count > 0)
    {
      if (get_pref_boolean(eSIMPLE_INTERFACE))
        {
          simple_menuspec[5].namestr = strdup("Documents" );
          simple_menuspec[5].mnemonic = 'D';
          simple_menuspec[5].func = 0;
          simple_menuspec[5].data = 0;
          simple_menuspec[5].sub_menu = menu;
        }
      else
        {
          menuspec[6].namestr = strdup("Documents" );
          menuspec[6].mnemonic = 'D';
          menuspec[6].func = 0;
          menuspec[6].data = 0;
          menuspec[6].sub_menu = menu;
        }
    }
  return;

/*
 screwed_open_file:
  fclose (fp);
  return;

 screwed_no_file:
  return;
*/
}


static void mo_try_to_grok_menubar (void)
{
  char *filename = get_pref_string(eDOCUMENTS_MENU_SPECFILE);
  
  if (filename && mo_file_exists (filename))
    {
      mo_grok_menubar (filename);
    }
  else
    {
      filename = getenv ("MOSAIC_DOCUMENTS_MENU_SPECFILE");
      if (filename && mo_file_exists (filename))
        {
          mo_grok_menubar (filename);
        }
      else
        {
          filename = DOCUMENTS_MENU_SPECFILE;
          if (filename && mo_file_exists (filename))
            {
              mo_grok_menubar (filename);
            }
        }
    }

  return;
}

static int grokked_menubar = 0;

XmxMenuRecord *mo_make_document_view_menubar (Widget form)
{
XmxMenuRecord *toBeReturned;

  /* If we've never tried to set up the user's configurable menubar by
     loading menuspec[5], give it a shot now. */
  if (!grokked_menubar)
    {
      grokked_menubar = 1;

      mo_try_to_grok_menubar ();
    }

  toBeReturned = XmxRMakeMenubar(form, 
				 get_pref_boolean(eSIMPLE_INTERFACE) ?
				 simple_menuspec : menuspec);

  if (get_pref_boolean(eKIOSK) || get_pref_boolean(eKIOSKNOEXIT)) {
		/* won't appear */
		XtUnmanageChild(toBeReturned->base);
		}

  return toBeReturned;
}
