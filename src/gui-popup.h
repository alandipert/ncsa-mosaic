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

#ifndef __POPUP_H__
#define __POPUP_H__

#include "mosaic.h"
#include "libhtmlw/HTML.h"
#include "hotlist.h"
#include <Xm/DrawingA.h>
#include <Xm/ScrollBar.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/CascadeBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/RowColumn.h>
#include <Xm/FileSB.h>
#include <X11/Xatom.h>

#define MAX_NUM_POPUP_ITEMS 50
#define ALL_TYPES (E_TEXT | E_BULLET | E_LINEFEED | E_IMAGE | E_WIDGET | E_HRULE | E_TABLE | E_ANCHOR)
#define TIGHT 0
#define LOOSE 1
#define COPY_URL_LABEL "Copy Link URL"

#define NEWS_NOANCHOR (E_TEXT | E_BULLET | E_LINEFEED | E_WIDGET | E_HRULE |E_TABLE)

typedef enum _w_class { LastItem=1, PushButton, Separator, CascadeButton,
			ToggleButton, Widgt } w_class;
enum { I_Save, I_ViewExternal, I_ViewInternal, I_Reload,
	M_ImageData, M_LinkData, M_FileData };

typedef struct act_struct 
{
  int act_code;
  struct ele_rec *eptr;
  void *str;
} act_struct;

typedef struct PopupItem
{
  /* the top half must be filled in if this is to appear in the popup */

  w_class              class; /* this is a button, separator, label, cascade */
  unsigned long int    types; /* for which widget elements this button is to 
				 popup for (the list of elements is below) */
  int                  types_method; /* if TIGHT use == if LOOSE use & */

  unsigned long int    modes; /* news, http, ftp, etc. */
  int                  modes_method; /* if TIGHT use == if LOOSE use & */

  char *label;

  /* these are needed for a button class */

  struct act_struct    acst; /* identifies the action */
  void                 (*cbfp)(); /* callback function that takes act_struct
				     as client data */

  /* theses are optional */

  char                 mnemonic;
  char                 *accel_text;
  char                 *accel;

  /* this is needed for a cascade class */

  struct               PopupItem *sub_items; /* NULL if this isn't a 
						    pull_right */

  /* this is for internal uses */

    Widget               _w;
    int                  startup; /* are we sensitive when we start */
     
} PopupItem;

XmxCallbackPrototype (menubar_cb);
XmxCallbackPrototype (image_cb);
XmxCallbackPrototype (metadata_cb);
XmxCallbackPrototype (user_defs_cb);
XmxCallbackPrototype (ftp_rmbm_cb);
XmxCallbackPrototype (fsb_OKCallback);
XmxCallbackPrototype (fsb_CancelCallback);
XmxCallbackPrototype (copy_link_cb);
XmxCallbackPrototype (session_cb);
XmxCallbackPrototype (rbm_ballonify);
void fsb();
void ThirdButtonMenu();
char *getFileName(char *file_src);
Widget  _PopupMenuBuilder(Widget parent, int type, char *title, 
				 char mnem, PopupItem *items);
extern struct ele_rec *LocateElement();
void _set_eptr_field();
void mo_popup_set_something();
PopupItem *popup_build_user_defs();
PopupItem *build_user_defs_items();
char **user_defs_get_entries();  
extern mo_window *current_win;
extern int file_exists();
extern char *my_chop();
extern char *my_strndup();
extern char *mo_escape_part();

void mo_make_popup();
void mo_init_hotmenu();

void mo_add_to_rbm_history(mo_window *win, char *url, char *title);

static Boolean convert_selection(Widget, Atom *, Atom *, Atom *, XtPointer *,
				 unsigned long *, int *);

#ifdef GUI_POPUP_H /* this is set in gui-popup.c -- it prevents multiple 
		      instances of the following variables */

Boolean have_popup;
Widget popup = NULL;

PopupItem image_menu[] = {
  {PushButton, 0, 0, 0, 0, "Save", {I_Save, NULL, NULL}, image_cb, 0, NULL, 
   NULL,  NULL, NULL, 1},
  {PushButton, 0, 0, 0, 0, "Reload", {I_Reload, NULL, NULL}, image_cb, 0, 
   NULL, NULL, NULL,  NULL, 1},
  {PushButton, 0, 0, 0, 0, "View External", {I_ViewExternal, NULL, NULL}, 
   image_cb, 0, NULL, NULL, NULL, NULL, 1},
  {PushButton, 0, 0, 0, 0, "View Internal", {I_ViewInternal, NULL, NULL}, 
   image_cb, 0, NULL, NULL, NULL, NULL, 1},
  {PushButton, 0, 0, 0, 0, "Get Image Metadata", {M_ImageData, NULL, NULL}, 
   metadata_cb, 0, NULL, NULL, NULL, NULL, 1},
/*
  {LastItem, 0, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL}
*/
  { LastItem },
};

PopupItem pan_menu[] = {
  {PushButton, 0, 0, 0, 0, "Right", {0, NULL, NULL}, NULL, 0,
   NULL, NULL, NULL, NULL, 1},
  {PushButton, 0, 0, 0, 0, "Up", {0, NULL, NULL}, NULL,
   0, NULL, NULL, NULL, NULL, 1}, 
 {PushButton, 0,  0, 0, 0, "Left", {0, NULL, NULL}, NULL, 0,
   NULL, NULL, NULL, NULL, 1},
 {PushButton, 0, 0, 0, 0, "Down", {0, NULL, NULL},  NULL,
   0, NULL, NULL, NULL, NULL, 1},
/*
  {LastItem, 0, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL}
*/
  { LastItem },
};

PopupItem photo_cd_sub_menu[] = {
  {PushButton, 0, 0, 0,  0,"Zoom In",  {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1},
  {PushButton, 0, 0, 0,  0,"Zoom Out", {0, NULL, NULL} , NULL, 0, NULL, NULL, NULL, NULL, 1}, 
  {PushButton, 0, 0, 0, 0, "Zoom Crop", {0, NULL, NULL} , NULL, 0, NULL, NULL, NULL, NULL, 1},
  {PushButton, 0, 0, 0, 0, "Crop", {0, NULL, NULL} , NULL, 0, NULL, NULL, NULL, NULL, 1}, 
  {Separator, 0, 0, 0, 0, "Sep", {0, NULL, NULL} , NULL, 0, NULL, NULL, NULL, NULL, 1},
  {PushButton,  0, 0,0, 0, "Enlarge", {0, NULL, NULL} , NULL, 0, NULL, NULL, NULL, NULL, 1},
  {PushButton, 0, 0, 0, 0, "Reduce", {0, NULL, NULL} , NULL, 0, NULL, NULL, NULL, NULL, 1}, 
  {Separator, 0, 0, 0, 0, "Sep", {0, NULL, NULL} , NULL, 0, NULL, NULL, NULL, NULL, 1},
  {PushButton,  0, 0,0, 0, "Undo",  {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1},
  {Separator, 0, 0, 0, 0, "Sep",  {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1},
  {PushButton,  0, 0, 0, 0, "Rotate Clockwise", {0, NULL, NULL} , NULL, 0, NULL, NULL, NULL, 
   NULL, 1},
  {PushButton, 0, 0, 0, 0, "Rotate Counter-Clockwise",  {0, NULL, NULL}, NULL,
   0, NULL, NULL, NULL, NULL, 1}, 
  {CascadeButton,  0, 0,0, 0, "Pan",  {0, NULL, NULL}, NULL, 0, NULL, NULL, pan_menu, NULL, 1}, 
  {Separator,  0, 0,0, 0, "Sep", {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1},
  {PushButton, 0, 0, 0, 0, "Full Image", {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1}, 
  {PushButton,  0, 0,0,  0,"Reset To Original", {0, NULL, NULL}, NULL, 0, NULL, NULL, 
   NULL, NULL, 1}, 
  {Separator,  0, 0,0, 0, "Separator", {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL},
  {PushButton,  0, 0,0,  0,"Print This Image", {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, 
   NULL, 1},
  {ToggleButton,  0, 0,0, 0, "Display PhotoCD Icon", {0, NULL, NULL}, NULL, 0, NULL, NULL, 
   NULL, NULL, 1}, 
  {PushButton,  0, 0, 0, 0, "Options",  {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1}, 
/*
  {LastItem, 0, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL}
*/
  { LastItem },
};

PopupItem file_menu[] = {

  {PushButton, 0, 0, 0, 0, "Save Page", {mo_save_document, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, 0, 0, 0, 0, "Print", {mo_print_document, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, 0, 0, 0, 0, "Mail To", {mo_mail_document, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, 0, 0, 0, 0, "Get File Metadata", {M_FileData, NULL, NULL},
   metadata_cb, 0, NULL, NULL, NULL, NULL, 1},

  { LastItem },
};

PopupItem popup_items[] = {

  /*---------------------------------------------------------------
         Permanent stuff
    ---------------------------------------------------------------*/

  {PushButton, ALL_TYPES, LOOSE, moMODE_ALL, LOOSE,"Back", 
   {mo_back, NULL, NULL},
   menubar_cb, 0, "B", NULL, NULL, NULL, 1},

  {PushButton, ALL_TYPES, LOOSE, moMODE_ALL, LOOSE, "Forward", 
   {mo_forward, NULL, NULL},
   menubar_cb, 0, "F", NULL, NULL, NULL, 1}, 

  /* negative one means this is a hotlist */
  {CascadeButton, ALL_TYPES, LOOSE, moMODE_ALL, LOOSE, "Hotlist", 
   {-1, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1}, 

  {CascadeButton, ALL_TYPES, LOOSE, moMODE_ALL, LOOSE, "User", 
   {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1}, 

  {CascadeButton, ALL_TYPES, LOOSE, moMODE_ALL, LOOSE, "Session History", 
   {-2, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1}, 

  /*---------------------------------------------------------------
         Stuff if on a html page and not on a image or anchor
    ---------------------------------------------------------------*/

  {Separator, (E_TEXT | E_BULLET | E_LINEFEED | E_WIDGET | E_HRULE |E_TABLE),
   LOOSE, moMODE_ALL, LOOSE, "Separator", {0, NULL, NULL},
   NULL, 0, NULL, NULL, NULL, NULL, 1}, 

  {CascadeButton, (E_TEXT | E_BULLET | E_LINEFEED | E_WIDGET | E_HRULE | 
		   E_TABLE),LOOSE,  moMODE_ALL, LOOSE, "File",
   {0, NULL, NULL}, NULL, 0, NULL, NULL, file_menu, NULL, 1},

 /*---------------------------------------------------------------
         Stuff if on any page and an anchor (including image anchor)
    ---------------------------------------------------------------*/

  {Separator, E_ANCHOR | E_IMAGE, LOOSE, moMODE_ALL, LOOSE, "Separator",  
   {0, NULL, NULL},
   NULL, 0, NULL, NULL, NULL, NULL, 1}, 

  {PushButton, E_ANCHOR | E_IMAGE, LOOSE, moMODE_ALL, LOOSE, COPY_URL_LABEL, 
   {0, NULL, NULL}, copy_link_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, E_ANCHOR | E_IMAGE, LOOSE, moMODE_ALL, LOOSE, "Get Link Metadata", 
   {M_LinkData, NULL, NULL}, metadata_cb, 0, NULL, NULL, NULL, NULL, 1},

  /*---------------------------------------------------------------
         Stuff if on any page and a image (not including image link)
    ---------------------------------------------------------------*/

  {Separator, E_IMAGE, TIGHT, moMODE_ALL, LOOSE, "Separator", 
   {0, NULL, NULL}, NULL, 0, NULL, NULL, NULL, NULL, 1}, 

  {PushButton, E_IMAGE, TIGHT, moMODE_ALL, LOOSE, "Save", 
   {I_Save, NULL, NULL}, image_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, E_IMAGE, TIGHT, moMODE_ALL, LOOSE, "Reload", 
   {I_Reload, NULL, NULL},
   image_cb, 0, NULL, NULL, NULL,  NULL, 1},

  {PushButton, E_IMAGE, TIGHT, moMODE_ALL, LOOSE, "View External", 
   {I_ViewExternal, NULL, NULL}, image_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, E_IMAGE, TIGHT, moMODE_ALL, LOOSE, "View Internal", 
   {I_ViewInternal, NULL, NULL}, image_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, E_IMAGE, TIGHT, moMODE_ALL, LOOSE, "Get Image Metadata", 
   {M_ImageData, NULL, NULL}, metadata_cb, 0, NULL, NULL, NULL, NULL, 1},

 /*---------------------------------------------------------------
         Stuff if on any page and a image link
    ---------------------------------------------------------------*/

  {Separator, E_IMAGE | E_ANCHOR, TIGHT, moMODE_PLAIN, LOOSE, "Separator",  
   {0, NULL, NULL},
   NULL, 0, NULL, NULL, NULL, NULL, 1}, 

  {CascadeButton, E_IMAGE | E_ANCHOR, TIGHT, moMODE_PLAIN, LOOSE, "Image",  
   {0, NULL, NULL}, 
   NULL, 0, NULL, NULL, image_menu, NULL, 1},

 /*---------------------------------------------------------------
         Stuff if on a ftp page 
    ---------------------------------------------------------------*/

  {Separator, ALL_TYPES, LOOSE, moMODE_FTP, TIGHT, "Separator", 
   {0, NULL, NULL},
   NULL, 0, NULL, NULL, NULL, NULL, 1}, 

  {PushButton, ALL_TYPES, LOOSE, moMODE_FTP, TIGHT, "Put ...", 
   {mo_ftp_put, NULL, NULL},
   ftp_rmbm_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, ALL_TYPES, LOOSE, moMODE_FTP, TIGHT, "Make Directory", 
   {mo_ftp_mkdir, NULL, NULL}, ftp_rmbm_cb, 0, NULL, NULL, NULL,  NULL, 1},

  {PushButton, E_ANCHOR, TIGHT, moMODE_FTP, TIGHT, "Remove", 
   {mo_ftp_remove, NULL, NULL}, ftp_rmbm_cb, 0, NULL, NULL, NULL,  NULL, 1},

 /*---------------------------------------------------------------
         Stuff if on a news page and not a link
    ---------------------------------------------------------------*/

  {Separator, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, "Separator", 
   {0, NULL, NULL},
   NULL, 0, NULL, NULL, NULL, NULL, 1}, 

  {PushButton, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, "Next Article", 
   {mo_news_next, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, "Previous Article", 
   {mo_news_prev, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, "Next Thread", 
   {mo_news_nextt, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, "Previous Thread", 
   {mo_news_prevt, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, "Article Index", 
   {mo_news_index, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, "Group Index", 
   {mo_news_groups, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},
  
  {Separator, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, NULL, 
   {0, NULL, NULL},
   NULL, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, "Post", 
   {mo_news_post, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, "Followup", 
   {mo_news_follow, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  
  {Separator, ALL_TYPES, LOOSE, moMODE_NEWS, TIGHT, NULL, 
   {0, NULL, NULL},
   NULL, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, NEWS_NOANCHOR, LOOSE, moMODE_NEWS, TIGHT, "Subscribe", 
   {mo_news_sub, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, NEWS_NOANCHOR, LOOSE, moMODE_NEWS, TIGHT, "Unsubscribe", 
   {mo_news_unsub, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, E_ANCHOR, LOOSE, moMODE_NEWS, TIGHT, "Subscribe", 
   {mo_news_sub_anchor, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, E_ANCHOR, LOOSE, moMODE_NEWS, TIGHT, "Unsubscribe", 
   {mo_news_unsub_anchor, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, NEWS_NOANCHOR, LOOSE, moMODE_NEWS, TIGHT, "Mark Group Read", 
   {mo_news_mread, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  {PushButton, E_ANCHOR, LOOSE, moMODE_NEWS, TIGHT, "Mark Group Read", 
   {mo_news_mread_anchor, NULL, NULL},
   menubar_cb, 0, NULL, NULL, NULL, NULL, 1},

  /*  {Separator, 0, 0, "Separator", 0, NULL, 0,
      NULL, NULL, NULL, NULL},

      {CascadeButton, 0, 0,  "Kodak Photo CD", 0, NULL, 0,
      NULL, NULL, photo_cd_sub_menu, NULL}, */

  { LastItem },
};

#endif
#endif
