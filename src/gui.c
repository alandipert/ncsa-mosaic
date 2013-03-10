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

#include "mosaic.h"
#include "gui.h"
#include "gui-documents.h"
#include "main.h"
#include "mo-www.h"
#include "gui-menubar.h"
#include "proxy.h"
#include "pan.h"
#include "pixmaps.h"
#include "libnut/system.h"
#include "libwww2/HTAABrow.h"

struct Proxy *noproxy_list = NULL, *proxy_list = NULL, *ReadProxies();

Widget mo_fill_toolbar(mo_window *win, Widget top, int w, int h);


#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if !defined(ultrix)
#include <netdb.h>
#endif
#include <ctype.h>
#include <sys/utsname.h>
#include <pwd.h>


#define __SRC__
#include "../libwww2/HTAAUtil.h"

extern Pixmap *tmp_pix;

/*SWP -- 9/7/95*/
char pre_title[80];
int cursorAnimCnt;
int makeBusy=0;

/*SWP -- colormap 3/19/96*/
extern int installed_colormap;
extern Colormap installed_cmap;

/* SWP -- Spoof Agent stuff */
extern int numAgents;
extern int selectedAgent;
extern char **agent;

/* PLB */
extern int newsShowAllGroups;
extern int newsShowAllArticles;
extern int newsShowReadGroups;
extern int newsNoThreadJumping;
extern int ConfigView;

void kill_splash();
int splash_cc=1; /* 1 if we need to free colors, 0 if already popped down */
XtIntervalId splashTimer;
Widget splash=NULL;

char *slab_words[] =
{"MENU","TITLE","URL","TOOLS","STATUS","VIEW","GLOBE","SMALLGLOBE","TEXTTOOLS",NULL};

int sarg[7],scount=-1,smalllogo=0,stexttools=0,pres=0;

/*SWP -- 8/14/95*/
extern int tableSupportEnabled;
extern int securityType;

/*SWP -- 10.27.95 -- No Content Length*/
extern int noLength;

/* doesn't seem to be on all X11R4 systems
#if (XtSpecificationRelease == 4)
extern void _XEditResCheckMessages();
#define EDITRES_SUPPORT
#endif
*/

#if (XtSpecificationRelease > 4)
#define EDITRES_SUPPORT
#endif

/*
 * EDITRES_SUPPORT seems to fail with the HP libraries
 */
#if defined(__hpux) && defined(EDITRES_SUPPORT)
#undef EDITRES_SUPPORT
#endif

#ifdef EDITRES_SUPPORT
#include <X11/Xmu/Editres.h>
#endif

#include <X11/Intrinsic.h>

#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>
#include <Xm/DrawnB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/DrawingA.h>
#include <Xm/MainW.h>
#include <Xm/MenuShell.h>
#include <Xm/Protocols.h>
#include <Xm/Separator.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>


#include "libhtmlw/HTML.h"
#include "xresources.h"
#include "cci.h"


#include "bitmaps/iconify.xbm"
#include "bitmaps/iconify_mask.xbm"
#include "bitmaps/xmosaic.xbm"
#include "bitmaps/xmosaic_left.xbm"
#include "bitmaps/xmosaic_right.xbm"
#include "bitmaps/xmosaic_down.xbm"
#include "bitmaps/security.xbm"
/*Busy Cursor*/
#include "bitmaps/busy_1.xbm"
#include "bitmaps/busy_2.xbm"
#include "bitmaps/busy_3.xbm"
#include "bitmaps/busy_4.xbm"
#include "bitmaps/busy_5.xbm"
#include "bitmaps/busy_6.xbm"
#include "bitmaps/busy_7.xbm"
#include "bitmaps/busy_8.xbm"
#include "bitmaps/busy_9.xbm"
#include "bitmaps/busy_1_mask.xbm"
#include "bitmaps/busy_2_mask.xbm"
#include "bitmaps/busy_3_mask.xbm"
#include "bitmaps/busy_4_mask.xbm"
#include "bitmaps/busy_5_mask.xbm"
#include "bitmaps/busy_6_mask.xbm"
#include "bitmaps/busy_7_mask.xbm"
#include "bitmaps/busy_8_mask.xbm"
#include "bitmaps/busy_9_mask.xbm"


/* for selective image loading */

char **imagedelay_sites = NULL;
char **imagekill_sites = NULL;
Boolean currently_delaying_images = 0;

/*******************************/

/* Because Sun cripples the keysym.h file. */
#ifndef XK_KP_Up
#define XK_KP_Home              0xFF95  /* Keypad Home */
#define XK_KP_Left              0xFF96  /* Keypad Left Arrow */
#define XK_KP_Up                0xFF97  /* Keypad Up Arrow */
#define XK_KP_Right             0xFF98  /* Keypad Right Arrow */
#define XK_KP_Down              0xFF99  /* Keypad Down Arrow */
#define XK_KP_Prior             0xFF9A  /* Keypad Page Up */
#define XK_KP_Next              0xFF9B  /* Keypad Page Down */
#define XK_KP_End               0xFF9C  /* Keypad End */
#endif


#define SLAB_MENU 0
#define SLAB_TITLE 1
#define SLAB_URL 2
#define SLAB_TOOLS 3
#define SLAB_STATUS 4
#define SLAB_VIEW 5
#define SLAB_GLOBE 6
#define SLAB_SMALLGLOBE 7
#define SLAB_TEXTTOOLS 8

/*
 * Globals used by the pixmaps for the animated icon.
 * Marc, I imagine you might want to do something cleaner
 * with these?
 *
 * Marc's not here, man.
 */
extern int IconWidth, IconHeight, WindowWidth, WindowHeight;

extern Pixmap *IconPix,*IconPixSmall,*IconPixBig;

void mo_post_upload_window(mo_window *win);

extern void MoCCINewConnection();
void mo_gui_update_meter(int level,char *text);

int animateCursor();
void createBusyCursors(Widget bob);
void stopBusyAnimation();

extern int force_dump_to_file;
extern char *HTAppVersion;

/* from cciBindings.c */
extern int cci_event;

/* for the -geometry fix (kludge) - DXP 8/30/95 */
int userSpecifiedGeometry = 0;
Dimension userWidth, userHeight;
Position userX, userY;

// SAM char *MakeFilename();
// SAM long GetCardCount(char *fname);

/* ------------------------------ variables ------------------------------- */

Display *dsp;
XtAppContext app_context;
Widget toplevel;
Widget view = NULL;  /* HORRIBLE HACK @@@@ */
int Vclass;  /* visual class for 24bit support hack */
/*AppData Rdata;  /* extern'd in mosaic.h */
char *global_xterm_str;  /* required for HTAccess.c now */

char *uncompress_program;
char *gunzip_program;

int use_default_extension_map;
char *global_extension_map;
char *personal_extension_map;
int use_default_type_map;
char *global_type_map;
char *personal_type_map;

int tweak_gopher_types;
int max_wais_responses;
int useAFS;
int have_hdf;
int ftp_timeout_val;
int ftpRedial;
int ftpRedialSleep;
int ftpFilenameLength;
int ftpEllipsisLength;
int ftpEllipsisMode;
int twirl_increment;


extern int sendAgent;
extern int sendReferer;
extern int imageViewInternal;

//SAM extern int do_comment;

/* --------------BalloonHelpStuff---------------------------------------- */

static void BalloonHelpMe(Widget w, XEvent *event)
{
    char *info;

    XtVaGetValues(w, XmNuserData, (XtPointer) &info, NULL);
    mo_gui_notify_progress(info);
}

static void UnBalloonHelpMe(Widget w, XEvent *event)
{
    mo_gui_notify_progress(" ");
}

static char xlattab[] = "<Enter>: BalloonHelpMe()\n<Leave>: UnBalloonHelpMe()";

static XtActionsRec balloon_action[] = {
    {"BalloonHelpMe", (XtActionProc)BalloonHelpMe},
    {"UnBalloonHelpMe", (XtActionProc)UnBalloonHelpMe}
};

/* to use balloon help, add these bits to your widget ...  BJS 2/7/96
 *    XmNtranslations, XtParseTranslationTable(xlattab),
 *    XmNuserData, (xtpointer) "Balloon Help String!",
 */

/* ------------------------------------------------------ */

/* emacs bindings to be used in text fields */

static char text_translations[] = "\
           ~Meta ~Alt Ctrl<Key>u:	beginning-of-line()		\
					delete-to-end-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>k:	delete-to-end-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>a:	beginning-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>e:	end-of-line()   		\n\
           ~Meta ~Alt Ctrl<Key>w:	key-select()			\
					delete-selection()		\n\
           ~Meta ~Alt Ctrl<Key>y:	paste-clipboard()		\n\
	 Meta ~Ctrl       <Key>d:	delete-next-word()		\n\
	  Alt ~Ctrl       <Key>d:	delete-next-word()		\n\
           ~Meta ~Alt Ctrl<Key>d:       delete-next-character()         \n\
     Meta ~Ctrl<Key>osfBackSpace:	delete-previous-word()		\n\
      Alt ~Ctrl<Key>osfBackSpace:	delete-previous-word()		\n\
	Meta ~Ctrl<Key>osfDelete:	delete-next-word()		\n\
	 Alt ~Ctrl<Key>osfDelete:	delete-next-word()              \n\
                      <Btn1Down>:       take_focus() grab-focus()";

/* this will have to be handled dynamically when we go to preferences */

static char url_translations[] = "Ctrl<Key>z:         set_focus_to_view()";

void set_focus_to_view();
void take_focus();
static XtActionsRec url_actions[] = {
    {"set_focus_to_view", (XtActionProc)set_focus_to_view},
    {"take_focus", (XtActionProc)take_focus}
};

/* this stuff is so we can properly update the current_win variable
   eliminating alot of problems with cloned windows (We love globals!)

   Globals? Where? There are no globals here! */

static char toplevel_translations[] = "\
                      <Enter>:            set_current_win() \n\
                      <Leave>:            set_current_win()";

void set_current_win();

static XtActionsRec toplevel_actions[] = {
    {"set_current_win", (XtActionProc)set_current_win}
};

/* ------------------------------------------------------ */

#ifndef DISABLE_TRACE
extern int httpTrace;
extern int www2Trace;
extern int htmlwTrace;
extern int nutTrace;

int cciTrace=0;
int srcTrace=0;
int cacheTrace=0;
#endif


/* from cciBindings.c */
extern int cci_get;

char *HTReferer = NULL;

/* This is exported to libwww, like altogether too many other
   variables here. */
int binary_transfer;
/* Now we cache the current window right before doing a binary
   transfer, too.  Sheesh, this is not pretty. */
mo_window *current_win;

/* If startup_document is set to anything but NULL, it will be the
   initial document viewed (this is separate from home_document
   below). */
char *startup_document = NULL;
/* If startup_document is NULL home_document will be the initial
   document. */
char *home_document = NULL;
char *machine;
char *shortmachine;
char *machine_with_domain;

XColor fg_color, bg_color;

static Cursor busy_cursor;
static int busy = 0;
static Widget *busylist = NULL;
char *cached_url = NULL;

/* Forward declaration of test predicate. */
int anchor_visited_predicate (Widget, char *);

/* When we first start the application, we call mo_startup()
   after creating the unmapped toplevel widget.  mo_startup()
   either sets the value of this to 1 or 0.  If 0, we don't
   open a starting window. */
int defer_initial_window;

/* Pixmaps for interrupt button. */
static Pixmap xmosaic_up_pix, xmosaic_left_pix, xmosaic_down_pix, xmosaic_right_pix;

#define MAX_BUSY_CURSORS 9
int numCursors=MAX_BUSY_CURSORS;
/* Pixmaps for the busy cursor animation */
static Cursor busyCursor[MAX_BUSY_CURSORS
];

/* Pixmaps for security button. */
static Pixmap security_pix;
extern Pixmap toolbarBack, toolbarForward, toolbarHome, toolbarReload,
    toolbarOpen, toolbarSave, toolbarClone, toolbarNew, toolbarClose,
    toolbarBackGRAY, toolbarForwardGRAY,
    toolbarSearch, toolbarPrint, toolbarPost, toolbarFollow,
    tearv, tearh, toolbarPostGRAY, toolbarFollowGRAY,
    toolbarNewsFwd, toolbarNewsFFwd, toolbarNewsRev, toolbarNewsFRev,
    toolbarNewsIndex, toolbarAddHotlist, toolbarNewsGroups,
    toolbarNewsFwdGRAY, toolbarNewsFFwdGRAY, toolbarNewsRevGRAY, toolbarNewsFRevGRAY,
    toolbarNewsIndexGRAY,
    toolbarFTPput, toolbarFTPmkdir;

extern Pixmap securityKerberos4, securityBasic, securityMd5, securityNone,
    securityUnknown, securityKerberos5, securityDomain, securityLogin,
    enc_not_secure;

extern char *HTDescribeURL (char *);
extern mo_status mo_post_access_document (mo_window *win, char *url,
                                          char *content_type,
                                          char *post_data);
XmxCallbackPrototype (menubar_cb);

struct utsname mo_uname;

/* ----------------------------- WINDOW LIST ------------------------------ */

static mo_window *winlist = NULL;
static int wincount = 0;

/* Return either the first window in the window list or the next
   window after the current window. */
mo_window *mo_next_window (mo_window *win)
{
  if (win == NULL)
    return winlist;
  else
    return win->next;
}

/* Return a window matching a specified uniqid. */
mo_window *mo_fetch_window_by_id (int id)
{
  mo_window *win;

  win = winlist;
  while (win != NULL)
    {
      if (win->id == id)
/*        goto done;*/
	return(win);
      win = win->next;
    }

/* notfound:*/
  return NULL;
/*
 done:
  return win;*/
}

/* Register a window in the window list. */
mo_status mo_add_window_to_list (mo_window *win)
{
  wincount++;

  if (winlist == NULL)
    {
      win->next = NULL;
      winlist = win;
    }
  else
    {
      win->next = winlist;
      winlist = win;
    }

  return mo_succeed;
}

/* Remove a window from the window list. */
mo_status mo_remove_window_from_list (mo_window *win)
{
  mo_window *w = NULL, *prev = NULL;

  while (w = mo_next_window (w))
    {
      if (w == win)
        {
          /* Delete w. */
          if (!prev)
            {
              /* No previous window. */
              winlist = w->next;

              free (w);
              w = NULL;

              wincount--;

              /* Maybe exit. */
              if (!winlist)
                mo_exit ();
            }
          else
            {
              /* Previous window. */
              prev->next = w->next;

              free (w);
              w = NULL;

              wincount--;

              return mo_succeed;
            }
        }
      prev = w;
    }

  /* Couldn't find it. */
  return mo_fail;
}

/*
 * THIS IS NOT USED ANYMORE!
 */

/****************************************************************************
 * name:    mo_check_for_proxy
 * purpose: Return the location of the proxy gateway for the passed access
 *	    method.
 * inputs:
 *   - char *access: access string from the URL (http, gopher, ftp, etc)
 * returns:
 *   The proxy gateway to use. (http://proxy.ncsa.uiuc.edu:911/)
 * remarks: This should really be open-ended configurable.
 ****************************************************************************/
char *mo_check_for_proxy (char *access)
{
/*
	if (access == NULL)
	{
		return((char *)NULL);
	}
	else if (strcmp(access, "http") == 0)
	{
		return(Rdata.http_proxy);
	}
	else if (strcmp(access, "ftp") == 0)
	{
		return(Rdata.ftp_proxy);
	}
	else if (strcmp(access, "wais") == 0)
	{
		return(Rdata.wais_proxy);
	}
	else if (strcmp(access, "gopher") == 0)
	{
		return(Rdata.gopher_proxy);
	}
	else if (strcmp(access, "news") == 0)
	{
		return(Rdata.news_proxy);
	}
	else if (strcmp(access, "file") == 0)
	{
		return(Rdata.file_proxy);
	}
	else
	{
*/

		return((char *)NULL);
/*
	}
*/
}

/****************************************************************************
 * name:    mo_assemble_help_url
 * purpose: Make a temporary, unique filename.
 * inputs:
 *   - char *file: Filename to be appended to Rdata.docs_directory.
 * returns:
 *   The desired help url (a malloc'd string).
 * remarks:
 ****************************************************************************/
char *mo_assemble_help_url (char *file)
{
  char *tmp;
  char *docs_directory = get_pref_string(eDOCS_DIRECTORY);

  if (!file)
    return strdup ("http://lose.lose/lose");

  tmp = (char *)malloc ((strlen (file) + strlen (docs_directory) + 4) *
                        sizeof (char));

  if (docs_directory[strlen(docs_directory)-1] == '/')
    {
      /* Trailing slash in docs_directory spec. */
      sprintf (tmp, "%s%s", docs_directory, file);
    }
  else
    {
      /* No trailing slash. */
      sprintf (tmp, "%s/%s", docs_directory, file);
    }

  return tmp;
}


/* ----------------------------- busy cursor ------------------------------ */

mo_status mo_not_busy (void)
{

	/* This is done from mo_gui_done_with_icon() */

	return mo_succeed;
}


void stopBusyAnimation() {

mo_window *win = NULL;

	if (busy) {
		XUndefineCursor (dsp, XtWindow (toplevel));
		while (win = mo_next_window (win)) {
			XUndefineCursor (dsp, XtWindow (win->base));
			if (win->history_win)
				XUndefineCursor (dsp, XtWindow (win->history_win));
			if (win->hotlist_win)
				XUndefineCursor (dsp, XtWindow (win->hotlist_win));
			if (win->searchindex_win)
				XUndefineCursor (dsp, XtWindow (win->searchindex_win));
		}

		XFlush (dsp);
		busy = 0;
	}

	return;
}


/* For lack of a better place, we do the iconify icon stuff here as well...
    --SWP */

void createBusyCursors(Widget bob) {

int i;
Pixmap pmap,mmap;
Pixmap imap,imaskmap;
XColor ccell1,ccell_fg,ccell_bg;

/*
XWMHints *whints=XAllocWMHints();

	imap = XCreatePixmapFromBitmapData
	(XtDisplay(toplevel), XtWindow(toplevel),
	 iconify_bits, iconify_width, iconify_height, 1, 0, 1);
	imaskmap = XCreatePixmapFromBitmapData
	(XtDisplay(toplevel), XtWindow(toplevel),
	 iconify_mask_bits, iconify_mask_width, iconify_mask_height, 1, 0, 1);

	whints->flags=IconPixmapHint|IconMaskHint;
	whints->icon_pixmap=imap;
	whints->icon_mask=imaskmap;
	XSetWMHints(XtDisplay(toplevel),XtWindow(toplevel),whints);
*/

	if (! get_pref_boolean(eANIMATEBUSYICON) ) {
		numCursors=1;
		busyCursor[0]=busy_cursor;

		return;
	}

	XAllocNamedColor(dsp,(installed_colormap ?
			      installed_cmap :
			      DefaultColormapOfScreen(XtScreen(bob))),
			 "black",&ccell1,&ccell_fg);
	XAllocNamedColor(dsp,(installed_colormap ?
			      installed_cmap :
			      DefaultColormapOfScreen(XtScreen(bob))),
			 "white",&ccell1,&ccell_bg);

	mmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_1_mask_bits, busy_1_mask_width, busy_1_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_1_bits, busy_1_width, busy_1_height, 1, 0, 1);

	busyCursor[0]=XCreatePixmapCursor(dsp,pmap,mmap,&ccell_fg,&ccell_bg,busy_1_x_hot,busy_1_y_hot);
	XFreePixmap(dsp,mmap);
	XFreePixmap(dsp,pmap);

	mmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_2_mask_bits, busy_2_mask_width, busy_2_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_2_bits, busy_2_width, busy_2_height, 1, 0, 1);
	busyCursor[1]=XCreatePixmapCursor(dsp,pmap,mmap,&ccell_fg,&ccell_bg,busy_2_x_hot,busy_2_y_hot);
	XFreePixmap(dsp,mmap);
	XFreePixmap(dsp,pmap);

	mmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_3_mask_bits, busy_3_mask_width, busy_3_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_3_bits, busy_3_width, busy_3_height, 1, 0, 1);
	busyCursor[2]=XCreatePixmapCursor(dsp,pmap,mmap,&ccell_fg,&ccell_bg,busy_3_x_hot,busy_3_y_hot);
	XFreePixmap(dsp,mmap);
	XFreePixmap(dsp,pmap);

	mmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_4_mask_bits, busy_4_mask_width, busy_4_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_4_bits, busy_4_width, busy_4_height, 1, 0, 1);
	busyCursor[3]=XCreatePixmapCursor(dsp,pmap,mmap,&ccell_fg,&ccell_bg,busy_4_x_hot,busy_4_y_hot);
	XFreePixmap(dsp,mmap);
	XFreePixmap(dsp,pmap);

	mmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_5_mask_bits, busy_5_mask_width, busy_5_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_5_bits, busy_5_width, busy_5_height, 1, 0, 1);
	busyCursor[4]=XCreatePixmapCursor(dsp,pmap,mmap,&ccell_fg,&ccell_bg,busy_5_x_hot,busy_5_y_hot);
	XFreePixmap(dsp,mmap);
	XFreePixmap(dsp,pmap);

	mmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_6_mask_bits, busy_6_mask_width, busy_6_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_6_bits, busy_6_width, busy_6_height, 1, 0, 1);
	busyCursor[5]=XCreatePixmapCursor(dsp,pmap,mmap,&ccell_fg,&ccell_bg,busy_6_x_hot,busy_6_y_hot);
	XFreePixmap(dsp,mmap);
	XFreePixmap(dsp,pmap);

	mmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_7_mask_bits, busy_7_mask_width, busy_7_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_7_bits, busy_7_width, busy_7_height, 1, 0, 1);
	busyCursor[6]=XCreatePixmapCursor(dsp,pmap,mmap,&ccell_fg,&ccell_bg,busy_7_x_hot,busy_7_y_hot);
	XFreePixmap(dsp,mmap);
	XFreePixmap(dsp,pmap);

	mmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_8_mask_bits, busy_8_mask_width, busy_8_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_8_bits, busy_8_width, busy_8_height, 1, 0, 1);
	busyCursor[7]=XCreatePixmapCursor(dsp,pmap,mmap,&ccell_fg,&ccell_bg,busy_8_x_hot,busy_8_y_hot);
	XFreePixmap(dsp,mmap);
	XFreePixmap(dsp,pmap);

	mmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_9_mask_bits, busy_9_mask_width, busy_9_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
	(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 busy_9_bits, busy_9_width, busy_9_height, 1, 0, 1);
	busyCursor[8]=XCreatePixmapCursor(dsp,pmap,mmap,&ccell_fg,&ccell_bg,busy_9_x_hot,busy_9_y_hot);
	XFreePixmap(dsp,mmap);
	XFreePixmap(dsp,pmap);
}


int animateCursor() {

mo_window *win = NULL;

	if (!makeBusy) {
		mo_not_busy();
		stopBusyAnimation();

		return(0);
	}

	cursorAnimCnt++;
	if (cursorAnimCnt>=numCursors) {
		cursorAnimCnt=0;
	}

	XDefineCursor(dsp, XtWindow(toplevel), busyCursor[cursorAnimCnt]);
	while (win=mo_next_window(win)) {
		XDefineCursor(dsp, XtWindow(win->base),
			      busyCursor[cursorAnimCnt]);
		if (win->history_win) {
			XDefineCursor(dsp, XtWindow(win->history_win),
				      busyCursor[cursorAnimCnt]);
		}
		if (win->hotlist_win) {
			XDefineCursor(dsp, XtWindow(win->hotlist_win),
				      busyCursor[cursorAnimCnt]);
		}
		if (win->searchindex_win) {
			XDefineCursor(dsp, XtWindow(win->searchindex_win),
				      busyCursor[cursorAnimCnt]);
		}
	}

	XFlush(dsp);
	busy=1;

	return(1);
}


mo_status mo_busy (void) {

/* This happens in mo_gui_check_icon */
/*
	if (!busy) {
	}
*/

	return mo_succeed;
}


#ifdef HAVE_DTM
/* --------------------- mo_set_dtm_menubar_functions --------------------- */

mo_status mo_set_dtm_menubar_functions (mo_window *win)
{
  if (mo_dtm_out_active_p ())
    {
      /* If we've got an active outport, then we can send a document
         but not open another outport. */
      XmxRSetSensitive
        (win->menubar, mo_dtm_open_outport, XmxNotSensitive);
      XmxRSetSensitive
        (win->menubar, mo_dtm_send_document, XmxSensitive);
    }
  else
    {
      /* If we don't have an active outport, then we can't send a document
         but we can open an outport. */
      XmxRSetSensitive
        (win->menubar, mo_dtm_open_outport, XmxSensitive);
      XmxRSetSensitive
        (win->menubar, mo_dtm_send_document, XmxNotSensitive);
    }

  return mo_succeed;
}
#endif


/****************************************************************************
 * name:    mo_redisplay_window
 * purpose: Cause the current window's HTML widget to be refreshed.
 *          This causes the anchors to be reexamined for visited status.
 * inputs:
 *   - mo_window *win: Current window.
 * returns:
 *   mo_succeed
 * remarks:
 *
 ****************************************************************************/
mo_status mo_redisplay_window (mo_window *win)
{
  char *curl = cached_url;
  cached_url = win->cached_url;

  HTMLRetestAnchors (win->scrolled_win, anchor_visited_predicate);

  cached_url = curl;

  return mo_succeed;
}


/* ---------------------- mo_set_current_cached_win ----------------------- */

mo_status mo_set_current_cached_win (mo_window *win)
{
  current_win = win;
  view = win->view;

  return mo_succeed;
}


static connect_interrupt = 0;
extern int sleep_interrupt;

XmxCallback (icon_pressed_cb)
{
  sleep_interrupt = connect_interrupt = 1;
  if (cci_event) MoCCISendEventOutput(MOSAIC_GLOBE);
}


static XmxCallback (security_pressed_cb) {

mo_window *win = current_win;
char buf[BUFSIZ];

	if (!win || !win->current_node || !win->current_node) {
		return;
	}

	if (cci_event)MoCCISendEventOutput(AUTHENTICATION_BUTTON);

	mo_gui_check_security_icon(win->current_node->authType);

	switch(win->current_node->authType) {
		case HTAA_NONE:
			strcpy(buf,"There is no authentication for this URL." );
			break;
		case HTAA_BASIC:
			strcpy(buf,"The authentication method for this URL is\nBasic (uuencode/uudecode)." );
			break;
		case HTAA_KERBEROS_V4:
			strcpy(buf,"The authentication method for this URL is\nKerberos v4." );
			break;
		case HTAA_KERBEROS_V5:
			strcpy(buf,"The authentication method for this URL is\nKerberos v5." );
			break;
		case HTAA_MD5:
			strcpy(buf,"The authentication method for this URL is\nMD5." );
			break;
		case HTAA_DOMAIN:
			strcpy(buf,"This URL is Domain Restricted." );
			break;
		case HTAA_LOGIN:
			strcpy(buf,"This FTP URL is authenticated through logging into the\nFTP server machine." );
			break;
		case HTAA_UNKNOWN:
		default:
			strcpy(buf,"The authentication method for this URL is unknown.\nA default of no authentication was used, which was okayed\nby the server." );
			break;
	}

	application_user_info_wait(buf);

	return;
}


/* ----------------------- editable URL field callback -------------------- */
/* If the user hits return in the URL text field at the top of the display, */
/* then go fetch that URL  -- amb                                           */

static XmxCallback (url_field_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  char *url,*xurl;
  XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *) call_data;

  if(!get_pref_boolean(eFOCUS_FOLLOWS_MOUSE))
    XtSetKeyboardFocus(win->base, win->view);

  if (cci_event) MoCCISendEventOutput(MOSAIC_URL_TEXT_FIELD);

  url = XmxTextGetString (win->url_text);
  if (!url || (!strlen(url)))
    return;
  mo_convert_newlines_to_spaces (url);
  xurl=mo_url_prepend_protocol(url);
  mo_load_window_text (win, xurl, NULL);

  if (xurl==url) {
	free(xurl);
  }
  else {
	free(xurl);
	free(url);
  }

  if (cci_event) MoCCISendEventOutput(LINK_LOADED);

  return;
}

/****************************************************************************
 * name:    anchor_cb
 * purpose: Callback for triggering anchor in HTML widget.
 * inputs:
 *   - as per XmxCallback
 * returns:
 *   nothing
 * remarks:
 *   This is too complex and should be broken down.
 *   We look at the button event passed in through the callback;
 *   button1 == same window, button2 == new window.
 *   Call mo_open_another_window or mo_load_window_text to get
 *   the actual work done.
 ****************************************************************************/
static XmxCallback (anchor_cb)
{
  char *href, *reftext;
/*  char *access;*/
  static char *referer = NULL;
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  XButtonReleasedEvent *event =
    (XButtonReleasedEvent *)(((WbAnchorCallbackData *)call_data)->event);
  int force_newwin = (event->button == Button2 ? 1 : 0);
  int old_binx_flag;

  if (!win)
    return;

  if (cci_event) MoCCISendEventOutput(MOSAIC_URL_TRIGGER);

  /* if shift was down, make this a Load to Local Disk -- amb */
  old_binx_flag = win->binary_transfer;
  if ( (event->state & ShiftMask) == ShiftMask)
    win->binary_transfer = 1;

  if (get_pref_boolean(eKIOSK) ||
      get_pref_boolean(eKIOSKNOEXIT) ||
      get_pref_boolean(eDISABLEMIDDLEBUTTON)) {
    /* disable new window if in kiosk mode*/
    force_newwin = 0;
    /* disable load to disk in kiosk */
    win->binary_transfer = 0;
    }

  if (get_pref_boolean(ePROTECT_ME_FROM_MYSELF))
    {
      int answer = XmxModalYesOrNo
        (win->base, app_context,
	 "BEWARE: Despite our best and most strenuous intentions to the contrary,\nabsolutely anything could be on the other end of this hyperlink,\nincluding -- quite possibly -- pornography, or even nudity.\n\nNCSA disclaims all responsibility regarding your emotional and mental health\nand specifically all responsibility for effects of viewing salacious material via Mosaic.\n\nWith that in mind, are you *sure* you want to follow this hyperlink???" , "Yup, I'm sure, really." ,
         "Ack, no!  Get me outta here." );
      if (!answer)
        return;
    }

  /* amb */
  if (referer!=NULL)
    {
      free(referer);
      referer=NULL;
    }

  /*SWP*/
  if (!my_strncasecmp(win->current_node->url, "http://", 7))
    {
      /* what if hostname is a partial local? */
      referer = strdup (win->current_node->url);
      HTReferer = referer;
    }
  else
    HTReferer = NULL;

  if (((WbAnchorCallbackData *)call_data)->href)
    href = strdup (((WbAnchorCallbackData *)call_data)->href);
  else
    href = strdup ("Unlinked");
  if (((WbAnchorCallbackData *)call_data)->text)
    reftext = strdup (((WbAnchorCallbackData *)call_data)->text);
  else
    reftext = strdup ("Untitled");

  mo_convert_newlines_to_spaces (href);

  if (!force_newwin)
    mo_load_window_text (win, href, reftext);
  else
    {
      char *target = mo_url_extract_anchor (href);
      char *url =
        mo_url_canonicalize_keep_anchor (href, win->current_node->url);
      /* @@@ should we be keeping the anchor here??? */
      if (strncmp (url, "telnet:", 7) && strncmp (url, "tn3270:", 7) &&
          strncmp (url, "rlogin:", 7))
        {
          /* Not a telnet anchor. */

          /* Open the window (generating a new cached_url). */
          mo_open_another_window (win, url, reftext, target);

          /* Now redisplay this window. */
          mo_redisplay_window (win);
        }
      else
        /* Just do mo_load_window_text go get the xterm forked off. */
        mo_load_window_text (win, url, reftext);
    }

  if (cci_event) MoCCISendEventOutput(LINK_LOADED);

  win->binary_transfer = old_binx_flag;
  free (href);
  return;
}


/****************************************************************************
 * name:    anchor_visited_predicate (PRIVATE)
 * purpose: Called by the HTML widget to determine whether a given URL
 *          has been previously visited.
 * inputs:
 *   - Widget   w: HTML widget that called this routine.
 *   - char *href: URL to test.
 * returns:
 *   1 if href has been visited previously; 0 otherwise.
 * remarks:
 *   All this does is canonicalize the URL and call
 *   mo_been_here_before_huh_dad() to figure out if we've been
 *   there before.
 ****************************************************************************/
int anchor_visited_predicate (Widget w, char *href)
{
  int rv;

  if (!get_pref_boolean(eTRACK_VISITED_ANCHORS) || !href)
    return 0;

  /* This doesn't do special things for data elements inside
     an HDF file, because it's faster this way. */
  href = mo_url_canonicalize (href, cached_url);

  rv = (mo_been_here_before_huh_dad (href) == mo_succeed ? 1 : 0);

  free (href);
  return rv;
}

static void pointer_motion_callback (Widget w, char *href)
{
  mo_window *win = NULL;
  XmString xmstr;
  char *to_free = NULL, *to_free_2 = NULL;

  if (!get_pref_boolean(eTRACK_POINTER_MOTION))
    return;

  while (win = mo_next_window (win))
    if (win->scrolled_win == w)
      break;
  if (win == NULL)
    return;

  if (href && *href) {
      href = mo_url_canonicalize_keep_anchor (href, win->cached_url);
      to_free = href;

      /* Sigh... */
      mo_convert_newlines_to_spaces (href);

      /* This is now the option wherein the URLs are just spit up there;
         else we put up something more friendly. */
      if (get_pref_boolean(eTRACK_FULL_URL_NAMES)) {
          /* Everything already done... */
      } else {
          /* This is where we go get a good description. */
          href = HTDescribeURL (href);
          to_free_2 = href;
      }
  } else
    href = " ";

  xmstr = XmStringCreateSimple (href);
  XtVaSetValues
    (win->tracker_label,
     XmNlabelString, (XtArgVal)xmstr,
     NULL);
  XmStringFree (xmstr);

  if (to_free)
    free (to_free);
  if (to_free_2)
    free (to_free_2);

  return;
}


XmxCallback (submit_form_callback)
{
  mo_window *win = NULL;
  char *url = NULL, *method = NULL, *enctype = NULL, *query;
  int len, i;
  WbFormCallbackData *cbdata = (WbFormCallbackData *)call_data;
  int do_post_urlencoded = 0;
  int plaintext=0;
  char *entity=NULL;

  if (!cbdata)
    return;

  while (win = mo_next_window (win))
    if (win->scrolled_win == w)
      goto foundit;

  /* Shit outta luck. */
  return;

 foundit:

  mo_busy ();

  if (cci_event) MoCCISendEventOutput(FORM_SUBMIT);

  /* Initial query: Breathing space. */
  len = 16;

  /* Add up lengths of strings. */
  for (i = 0; i < cbdata->attribute_count; i++)
    {
      if (cbdata->attribute_names[i])
        {
          len += strlen (cbdata->attribute_names[i]) * 3;
          if (cbdata->attribute_values[i])
            len += strlen (cbdata->attribute_values[i]) * 3;
        }
      len += 2;
    }

  /* Get the URL. */
  if (cbdata->href && *(cbdata->href))
    url = cbdata->href;
  else
    url = win->current_node->url;

  if (cbdata->method && *(cbdata->method))
    method = cbdata->method;
  else
    method = strdup ("GET");

  /* Grab enctype if it's there. */
  if (cbdata->enctype && *(cbdata->enctype))
    enctype = cbdata->enctype;
  /* Grab encentity if it's there and we have an enctype. */
  if (enctype && cbdata->enc_entity && *(cbdata->enc_entity))
    entity = cbdata->enc_entity;

#ifndef DISABLE_TRACE
  if (srcTrace) {
	fprintf (stderr, "[submit_form_callback] method is '%s'\n",
		 method);
	fprintf (stderr, "[submit_form_callback] enctype is '%s'\n",
		 enctype);
  }
#endif

  if ((my_strcasecmp (method, "POST") == 0) ||
 	(my_strcasecmp (method, "cciPOST") == 0))
    do_post_urlencoded = 1;

  len += strlen (url);

  query = (char *)malloc (sizeof (char) * len);

  if (!do_post_urlencoded)
    {
	strcpy (query, url);
	/* Clip out anchor. */
	strtok (query, "#");
	/* Clip out old query. */
	strtok (query, "?");
	if (query[strlen(query)-1] != '?')
	{
		strcat (query, "?");
	}
	plaintext=0;
    }
  else if (cbdata->format && *cbdata->format && !my_strcasecmp(cbdata->format,"PLAIN")) {
	/* Get ready for cats below. */
	query[0] = 0;
	plaintext=1;
    }
  else
    {
	/* Get ready for cats below. */
	query[0] = 0;
	plaintext=0;
    }

  /* Take isindex possibility into account. */
  if (cbdata->attribute_count == 1 &&
      strcmp (cbdata->attribute_names[0], "isindex") == 0)
    {
      if (cbdata->attribute_values[0])
        {
          char *c = mo_escape_part (cbdata->attribute_values[0]);
          strcat (query, c);
          free (c);
        }
    }
  else
    {
      for (i = 0; i < cbdata->attribute_count; i++)
        {
          /* For all but the first, lead off with an ampersand. */
          if (i != 0)
            strcat (query, "&");
          /* Rack 'em up. */
          if (cbdata->attribute_names[i])
            {
              char *c = mo_escape_part (cbdata->attribute_names[i]);
              strcat (query, c);
              free (c);

              strcat (query, "=");

              if (cbdata->attribute_values[i])
                {
                  char *c = mo_escape_part (cbdata->attribute_values[i]);
                  strcat (query, c);
                  free (c);
                }
            }
        }
    }

  if (do_post_urlencoded)
    {
	if (!my_strcasecmp(method,"cciPOST"))
		MoCCIFormToClient(NULL, NULL, NULL,NULL,1);

      	mo_post_access_document (win, url,
				 (plaintext?"text/plain":
				  "application/x-www-form-urlencoded"),
				 query);
    }
  else
    {
      mo_access_document (win, query);
    }

  if (query) free (query);

  return;
}


/* This only gets called by the widget in the middle of set_text. */
static XmxCallback (link_callback)
{
  mo_window *win = current_win;
  LinkInfo *linfo = (LinkInfo *)call_data;
  extern char *url_base_override;

  if (!linfo)
    return;

  /* Free -nothing- in linfo. */
  if (linfo->href)
    {
      url_base_override = strdup (linfo->href);

      /* Set the URL cache variables to the correct values NOW. */
      cached_url = mo_url_canonicalize (url_base_override, "");
      win->cached_url = cached_url;
    }
  if (linfo->role)
    {
      /* Do nothing with the role crap yet. */
    }

  return;
}


/* Exported to libwww2. */
void mo_gui_notify_progress (char *msg)
{
  XmString xmstr;
  mo_window *win = current_win;

  if (!get_pref_boolean(eTRACK_POINTER_MOTION))
    return;

  if (!msg)
    msg = " ";

  xmstr = XmStringCreateSimple (msg);
  XtVaSetValues
    (win->tracker_label,
     XmNlabelString, (XtArgVal)xmstr,
     NULL);
  XmStringFree (xmstr);

  XmUpdateDisplay (win->base);

  return;
}


void UpdateButtons (Widget w)
{
  XEvent event;
  Display * display = XtDisplay(w);

  XSync (display, 0);

  while (XCheckMaskEvent(display, (ButtonPressMask|ButtonReleaseMask),
                         &event))
    {
      XButtonEvent *bevent = &(event.xbutton);
      if (bevent->window == XtWindow (current_win->logo))
        {
          XtDispatchEvent(&event);
        }
      /* else just throw it away... users shouldn't be pressing buttons
         in the middle of transfers anyway... */
    }
}


/*SWP 7/3/95*/
void mo_gui_check_security_icon_in_win(int type, mo_window *win) {

int ret;
Pixmap pix;
static int current=HTAA_NONE;

	if (!get_pref_boolean(eSECURITYICON)) {
		return;
	}

	switch (type) {
		case HTAA_UNKNOWN:
			pix=securityUnknown;
			current=type;
			break;

		case HTAA_NONE:
			pix=securityNone;
			current=type;
			break;

		case HTAA_KERBEROS_V4:
			pix=securityKerberos4;
			current=type;
			break;

		case HTAA_KERBEROS_V5:
			pix=securityKerberos5;
			current=type;
			break;
		case HTAA_MD5:
			pix=securityMd5;
			current=type;
			break;
		case HTAA_BASIC:
			pix=securityBasic;
			current=type;
			break;

		case HTAA_DOMAIN:
			pix=securityDomain;
			current=type;
			break;

		case HTAA_LOGIN:
			pix=securityLogin;
			current=type;
			break;

		default:
			pix=securityUnknown;
			current=type;
			break;
	}

	if ((char *)pix != NULL) {
		DrawSecurityPixmap (win->security, pix);
	}

	UpdateButtons (win->base);
	XmUpdateDisplay (win->base);

	return;
}


void mo_gui_check_security_icon(int type) {

mo_window *win = current_win;

	if (get_pref_boolean(eSECURITYICON)) {
		mo_gui_check_security_icon_in_win(type,win);
	}

	return;
}


int logo_count = 0;
int logo_save = 0;

int mo_gui_check_icon (int twirl)
{
  mo_window *win = current_win;
  int ret;
  static int cnt=0;

  if (twirl!=(-1) && twirl>0) {
	if (!makeBusy) {
		cursorAnimCnt=(-1);
		makeBusy=1;
	}

	cnt++;
	if (cnt==2) {
		animateCursor();
		cnt=0;
	}
  }

  if (twirl>0) {
	if (get_pref_boolean(eTWIRLING_TRANSFER_ICON)) {
		if ((char *)IconPix[logo_count] != NULL) {
			AnimatePixmapInWidget(win->logo, IconPix[logo_count]);
		}
		logo_count++;
		if (logo_count >= get_pref_int(ePIX_COUNT))
			logo_count = 0;
	}
  }

  UpdateButtons (win->base);
  XmUpdateDisplay (win->base);

  ret = connect_interrupt;
  connect_interrupt = 0;

  return(ret);
}


void mo_gui_clear_icon (void)
{
  connect_interrupt = 0;
}

void mo_gui_apply_default_icon(void) {

mo_window *win = current_win;

	XmxApplyPixmapToLabelWidget(win->logo, IconPix[0]);
}


void mo_gui_done_with_icon (void)
{
  mo_window *win = current_win;
  extern void ungrab_the_____ing_pointer();

	if (tmp_pix) {
		IconPix = tmp_pix;
		logo_count = 0;
		set_pref(ePIX_COUNT, (void *)&logo_save);
		tmp_pix=NULL;
	}

	XClearArea(XtDisplay(win->logo), XtWindow(win->logo), 0, 0, 0, 0, True);
	makeBusy = 0;
	animateCursor();
	logo_count = 0;
	HTMLSetAppSensitive((Widget) win->scrolled_win);
	/* this works dammit (trust me) - TPR */
	XtAppAddTimeOut(app_context, 10,
                    (XtTimerCallbackProc)ungrab_the_____ing_pointer, NULL);
}


void mo_presentation_mode(mo_window *win) {

	if (!pres) {
		pres=1;
		scount=(-1);
		mo_duplicate_window(win);
		mo_delete_window(win);
	}
	else {
		pres=0;
		scount=(-1);
		mo_duplicate_window(win);
		mo_delete_window(win);
	}
}


/****************************************************************************
 * name:    mo_view_keypress_handler (PRIVATE)
 * purpose: This is the event handler for the HTML widget and associated
 *          scrolled window; it handles keypress events and enables the
 *          hotkey support.
 * inputs:
 *   - as per XmxEventHandler
 * returns:
 *   nothing
 * remarks:
 *   Hotkeys and their actions are currently hardcoded.  This is probably
 *   a bad idea, and Eric hates it.
 ****************************************************************************/
static XmxEventHandler (mo_view_keypress_handler)
{
/*  char url[128]; /* buffer for news io */
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  int _bufsize = 3, _count;
  char _buffer[3];
  KeySym _key;
  XComposeStatus _cs;
  Widget sb;
  String params[1];

  if (!win)
    return;

  /* Go get ascii translation. */
  _count = XLookupString (&(event->xkey), _buffer, _bufsize,
                          &_key, &_cs);

  /* I don't know why this is necessary but for some reason the rbm was making
     the above function return 0 as the _key, this fixes it -- TPR */
  if(!_key)
    _key = XKeycodeToKeysym(XtDisplay(win->view), event->xkey.keycode, 0);

  /* Insert trailing Nil. */
  _buffer[_count] = '\0';

  params[0] = "0";

  switch(_key){
  case XK_Prior: /* Page up. */
  case XK_KP_Prior:
      if(!get_pref_boolean(eCATCH_PRIOR_AND_NEXT)) break;
  case XK_BackSpace:
  case XK_Delete:
      XtVaGetValues (win->scrolled_win, XmNverticalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb)) {
          XtCallActionProc (sb, "PageUpOrLeft", event, params, 1);
      }
      break;

  case XK_Next:  /* Page down. */
  case XK_KP_Next:
      if(!get_pref_boolean(eCATCH_PRIOR_AND_NEXT)) break;
  case XK_Return:
  case XK_space:
      XtVaGetValues (win->scrolled_win, XmNverticalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb)) {
          XtCallActionProc (sb, "PageDownOrRight", event, params, 1);
      }
      break;

  case XK_Tab:
    if(!get_pref_boolean(eFOCUS_FOLLOWS_MOUSE))
       {
	 if(event->xkey.state & ControlMask)
	   {
	     XtSetKeyboardFocus(win->base, win->view);
	     HTMLTraverseTabGroups(win->view, XmTRAVERSE_HOME);
	   }
	 else
	   HTMLTraverseTabGroups(win->scrolled_win, XmTRAVERSE_NEXT_TAB_GROUP);
       }
    break;

  case XK_Home: /* Home -- Top */
      HTMLGotoId(win->scrolled_win, 0, 0);
      break;

  case XK_End: /* End -- Bottom */
      HTMLGotoId(win->scrolled_win, HTMLLastId(win->scrolled_win), 0);
      break;

  case XK_Down:
  case XK_KP_Down:
      XtVaGetValues (win->scrolled_win, XmNverticalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb)) {
          XtCallActionProc (sb, "IncrementDownOrRight", event, params, 1);
      }
      break;

  case XK_Right:
  case XK_KP_Right:
      params[0] = "1";
      XtVaGetValues (win->scrolled_win, XmNhorizontalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb)) {
          XtCallActionProc (sb, "IncrementDownOrRight", event, params, 1);
      }
      break;

  case XK_Up:
  case XK_KP_Up:
      XtVaGetValues (win->scrolled_win, XmNverticalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb)){
          XtCallActionProc (sb, "IncrementUpOrLeft", event, params, 1);
      }
      break;

  case XK_Left:
  case XK_KP_Left:
      params[0] = "1";

      XtVaGetValues (win->scrolled_win, XmNhorizontalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb)) {
          XtCallActionProc (sb, "IncrementUpOrLeft", event, params, 1);
      }
      break;
  }


  if (!(get_pref_boolean(eKIOSK) || get_pref_boolean(eKIOSKNOEXIT))) {
      switch(_key){
              /* News Hotkeys ...
                 < > = prev/next thread  , . = prev/next message */
      case XK_less:
          gui_news_prevt(win);
          break;

      case XK_greater:
          gui_news_nextt(win);
          break;

      case XK_comma:
          gui_news_prev(win);
	  break;

      case XK_period:
          gui_news_next(win);
          break;

      case XK_A: /* Annotate. */
      case XK_a:
          mo_post_annotate_win (win, 0, 0, NULL, NULL, NULL, NULL);
          break;

      case XK_B: /* Back. */
      case XK_b:
          mo_back_node (win);
          break;

      case XK_C: /* Clone */
      case XK_c:
          mo_duplicate_window (win);
          break;

      case XK_D: /* Document source. */
      case XK_d:
          mo_post_source_window (win);
          break;

      case XK_E: /* Edit */
      case XK_e:
          mo_edit_source (win);
          break;

      case XK_F:
      case XK_f:
	if(event->xkey.state & ControlMask)
	  {
	    if(XtIsManaged(win->slab[SLAB_URL]) &&
	       !get_pref_boolean(eFOCUS_FOLLOWS_MOUSE))
	      {
		XmTextFieldSetString(win->url_text, "ftp://");
		XtSetKeyboardFocus(win->base, win->url_text);
		XmTextSetInsertionPosition(win->url_text, 7);
	      }
	  }
	else
          mo_forward_node (win);
          break;

      case XK_H: /* Hotlist */
          mo_post_hotlist_win (win);
          break;

      case XK_h: /* History */
	if(event->xkey.state & ControlMask)
	  {
	    if(XtIsManaged(win->slab[SLAB_URL]) &&
	       !get_pref_boolean(eFOCUS_FOLLOWS_MOUSE))
	      {
		XmTextFieldSetString(win->url_text, "http://");
		XtSetKeyboardFocus(win->base, win->url_text);
		XmTextSetInsertionPosition(win->url_text, 8);
	      }
	  }
	else
          mo_post_history_win (win);
	break;

      case XK_L: /* Open Local */
      case XK_l:
          mo_post_open_local_window (win);
          break;

      case XK_M: /* Mailto */
      case XK_m:
          mo_post_mail_window (win);
          break;

      case XK_N: /* New */
      case XK_n:
	if(event->xkey.state & ControlMask)
	  {
	    if(XtIsManaged(win->slab[SLAB_URL]) &&
	       !get_pref_boolean(eFOCUS_FOLLOWS_MOUSE))
	      {
		XtSetKeyboardFocus(win->base, win->url_text);

		XmTextFieldSetString(win->url_text, "news://");
		XmTextSetInsertionPosition(win->url_text, 8);
	      }
	  }
	else
	  mo_open_another_window (win, home_document, NULL, NULL);
	break;

      case XK_O: /* Open */
      case XK_o:
          mo_post_open_window (win);
          break;

      case XK_P: /* Print */
      case XK_p:
/*
	  if(event->xkey.state & ControlMask) {
		mo_presentation_mode(win);
	  }
	  else
*/
	  {
		mo_post_print_window (win);
	  }
          break;

      case XK_r: /* reload */
          mo_reload_window_text (win, 0);
          break;

      case XK_R: /* Refresh */
          mo_refresh_window_text (win);
          break;

      case XK_S: /* Search. */
      case XK_s:
          mo_post_search_window (win);
          break;

	/* Tag 'n Bag */
              /*
      case XK_T:
      case XK_t:
	  mo_tagnbag_url (win);
          break;
          */

/* Not active */
#ifdef SWP_NOT_DONE
      case XK_U: /* Upload a file (method of put) */
      case XK_u:
	  mo_post_upload_window(win);
	  break;
#endif

      case XK_Z:
      case XK_z:
	if(XtIsManaged(win->slab[SLAB_URL]) &&
	   !get_pref_boolean(eFOCUS_FOLLOWS_MOUSE))
	  {
	    char *str;

	    str = XmTextFieldGetString(win->url_text);
	    XmTextFieldSetSelection(win->url_text, 0, strlen(str),
				    event->xkey.time);

	    XtSetKeyboardFocus(win->base, win->url_text);
	    XtSetKeyboardFocus(win->base, win->url_text);
	    XtFree(str);
	  }
	break;


      case XK_Escape:
            mo_delete_window (win);
            break;
      default:
          break;
      }
  }
  else { /* Kiosk */
      switch(_key) {
	case XK_B: /* Back. */
	case XK_b:
          mo_back_node (win);
          break;

	case XK_F: /* Forward */
	case XK_f:
          mo_forward_node (win);
          break;

      default:
          break;
      }
  }

  return;
}


/* Callback to redraw the meter -bjs */
static void ResizeMeter(Widget meter, XtPointer client, XtPointer call);


static void DrawMeter(Widget meter, XtPointer client, XtPointer call)
{
    mo_window *win = (mo_window *) client;
    GC gc;
    long mask = 0;
    XGCValues values;
    int l;
    char *ss;
    char s[256];
    static int last_len=0;
    int current_len;
    int resize=0;
    static char *finished="100%";

    if (!get_pref_boolean(eMETER)) {
	return;
    }

    gc = XtGetGC( meter,0,NULL);

    if(win->meter_width== -1) {
	resize=1;
	ResizeMeter(meter,(XtPointer)win,NULL);
    }

    if(win->meter_width== -1) ResizeMeter(meter,(XtPointer)win,NULL);
    current_len=(win->meter_width * win->meter_level)/100;

    XSetForeground( XtDisplay(meter),gc,win->meter_bg);
    XFillRectangle(XtDisplay(meter),XtWindow(meter),gc,
		   0,0,win->meter_width,win->meter_height);

    XSetForeground( XtDisplay(meter),gc,win->meter_fg);
    XFillRectangle(XtDisplay(meter),XtWindow(meter),gc,
		   0,0,current_len,
                   win->meter_height);

    if (win->meter_level==100 && !win->meter_text) {
	win->meter_text=finished;
    }

    if(!win->meter_notext && (win->meter_level<=100 || win->meter_text)){

	if (!win->meter_font) {
		XtVaGetValues(win->scrolled_win,
			      WbNmeterFont,
			      &(win->meter_font),
			      NULL);
		if (!win->meter_font) {
/*
 = XLoadQueryFont(XtDisplay(win->base),ftext))) {
*/
			puts("METER Cannot Get Font -- Please set 'Mosaic*MeterFont: NEW_FONT'\n  Where NEW_FONT is a 14 point font on your system.");
			win->meter_notext=1;
		}
		else {
			win->meter_notext=0;
			win->meter_fontW = win->meter_font->max_bounds.rbearing;
			win->meter_fontH = win->meter_font->max_bounds.ascent;

/* +			win->meter_font->max_bounds.descent;*/
		}
	}

	if (!win->meter_notext) {
		if(!win->meter_text) {
			l = 3;
			ss = s;
			s[0]=win->meter_level>9 ? '0'+(win->meter_level/10) : ' ';
			s[1]='0'+(win->meter_level%10);
			s[2]='%';
			s[3]=0;
		} else {
			ss = win->meter_text;
			l = strlen(ss);
		}

		values.font = win->meter_font->fid;
/*
		values.function = GXxor;
		values.background = win->meter_bg;
		mask |= GCFont | GCFunction | GCBackground;
*/
		mask = GCFont;
		XChangeGC(XtDisplay(meter), gc,  mask, &values);
		XSetForeground( XtDisplay(meter),gc,win->meter_font_bg);
		XDrawString(XtDisplay(meter), XtWindow(meter), gc,
			    (win->meter_width/2-(win->meter_fontW*l)/2)+2,
			    ((win->meter_height/2)+(win->meter_fontH/2)),
			    ss, l);
		XSetForeground( XtDisplay(meter),gc,win->meter_font_fg);
		XDrawString(XtDisplay(meter), XtWindow(meter), gc,
			    (win->meter_width/2-(win->meter_fontW*l)/2),
			    ((win->meter_height/2)+(win->meter_fontH/2))-2,
			    ss, l);
	}
    }

    last_len=current_len;

    XtReleaseGC(meter,gc);
}

static void ResizeMeter(Widget meter, XtPointer client, XtPointer call)
{
    XWindowAttributes wattr;
    mo_window *win = (mo_window *) client;

    if (!get_pref_boolean(eMETER)) {
	return;
    }

    if(!XtWindow(meter)) return;

    XGetWindowAttributes(XtDisplay(meter),XtWindow(meter),&wattr);

    win->meter_width = wattr.width;
    win->meter_height = wattr.height;

}


/* Exported to libwww2 */
void mo_gui_update_meter(int level, char *text)
{

    if (!get_pref_boolean(eMETER)) {
	return;
    }

    current_win->meter_text = text;

    if(current_win->meter_level == -1) return;
    if(level<0) level = 0;
    if(level>100) level = 100;
    current_win->meter_level = level;

    DrawMeter(current_win->meter,(XtPointer) current_win, NULL);
}

/* take a text string of the form "MENU,URL,VIEW,STATUS" and set the
   layout slab stuff from it */

/* WARNING:  The code to draw the interface expects the rules enforced herein
   to be followed... just taking out the safety checks here could cause some
   major headaches. BJS */
int parse_slabinfo(char *s)
{
    int k,j,i,done;
    char *p;

    for(p=s,i=0,done=0;!done;p++){
        if(!*p) done = 1;

        if(!*p || (*p==',')){
            *p=0;

            if(i==7){
                fprintf(stderr,"layout: too many slabs\n");
                return 0;
            }
            for(j=0;slab_words[j];j++){
                if((strlen(slab_words[j])==strlen(s)) &&
                   !strcmp(slab_words[j],s)){
                    if(j==SLAB_TEXTTOOLS){
                        j = SLAB_TOOLS;
                        stexttools=1;
                    }
                    if(j==SLAB_SMALLGLOBE) {
                        j = SLAB_GLOBE;
                        smalllogo = 1;
                    }
                    sarg[i++] = j;
                    goto next1;
                }
            }
            fprintf(stderr,"layout: bad slab name \"%s\"\n",s);
            return 0;

          next1:
            s = p+1;
            continue;
        }
        if(isalpha(*p)){
            *p = toupper(*p);
        } else {
            fprintf(stderr,"layout: bad character '%c'\n",*p);
            return 0;
        }
    }

        /* do some idiot-proofing */
    for(done=0,j=0;j<i;j++){
        if(sarg[j] == SLAB_VIEW) done = 1;
        if(sarg[j] == SLAB_GLOBE) {
            if(smalllogo){
                if(j+1 >= i){
                    fprintf(stderr,"layout: SMALLGLOBE requires one normal slab\n");
                    return 0;
                }
                if(sarg[j+1]==SLAB_VIEW || sarg[j+1]==SLAB_TOOLS){
                    fprintf(stderr,"layout: SMALLGLOBE may not be next to %s\n",
                            slab_words[sarg[j+1]]);
                    return 0;
                }
            } else {
                if(j+2 >= i){
                    fprintf(stderr,"layout: GLOBE requires two normal slabs\n");
                    return 0;
                }
                if((sarg[j+1]==SLAB_VIEW)||(sarg[j+2]==SLAB_VIEW)){
                    fprintf(stderr,"layout: GLOBE requires two normal slabs\n");
                    return 0;
                }
            }

        }
    }
    if(!done){
        fprintf(stderr,"layout: one VIEW slab required\n");
        return 0;
    }

        /* check for duplicate slabs */
    for(j=0;j<i;j++){
        for(k=0;k<i;k++){
            if((k!=j) && (sarg[j]==sarg[k])){
                fprintf(stderr,"layout: only one %s slab allowed\n",
                        slab_words[j]);
                return 0;
            }
        }
    }

        /* whew. made it. */
    scount = i;
    return 1;
}



struct tool mo_tools[] = {
    {"<-","Back","Previous page",mo_back,&toolbarBack, &toolbarBackGRAY, moMODE_ALL, 1, NULL},
    {"->","Forward","Next page",mo_forward,&toolbarForward, &toolbarForwardGRAY, moMODE_ALL, 1, NULL},
    {"Rel","Reload","Reload this page",mo_reload_document,&toolbarReload,NULL, moMODE_ALL, 0, NULL},
    {"Home","Home","Go Home!",mo_home_document,&toolbarHome,NULL, moMODE_ALL, 1, NULL},
    {"Open","Open","Open a new URL",mo_open_document,&toolbarOpen,NULL, moMODE_ALL, 0, NULL},
    {"Save","Save","Save current page as ...",mo_save_document,&toolbarSave,NULL, moMODE_ALL, 0, NULL},
    {"New","New","Create a new Mosaic window",mo_new_window,&toolbarNew,NULL, moMODE_ALL, 0, NULL},
    {"Clone","Clone","Clone this Mosaic window",mo_clone_window,&toolbarClone,NULL, moMODE_ALL, 0, NULL},
    {"Close","Close","Destroy this Mosaic window",mo_close_window,&toolbarClose,NULL, moMODE_ALL, 0, NULL},
    {"+ Hot","Add To Hotlist","Add current page to hotlist",mo_register_node_in_default_hotlist,&toolbarAddHotlist,NULL, moMODE_PLAIN, 0, NULL},
    {"Find","Find","Search this document", mo_search, &toolbarSearch, NULL, moMODE_ALL, 0, NULL},
    {"Prt","Print","Print this document", mo_print_document, &toolbarPrint, NULL, moMODE_ALL, 0, NULL},
    {"Grps","Groups","Newsgroups index",mo_news_groups,&toolbarNewsGroups, NULL, moMODE_ALL, 0, NULL},
/* News Mode */
    {"Idx","Index","Newsgroup article index",mo_news_index,&toolbarNewsIndex, NULL, moMODE_NEWS, 1, NULL},
    {"<Thr","< Thread","Go to previous thread",mo_news_prevt,&toolbarNewsFRev, &toolbarNewsFRevGRAY, moMODE_NEWS, 1, NULL},
    {"<Art","< Article","Go to previous article",mo_news_prev,&toolbarNewsRev, &toolbarNewsRevGRAY, moMODE_NEWS, 1, NULL},
    {"Art>","Article >","Go to next article",mo_news_next,&toolbarNewsFwd, &toolbarNewsFwdGRAY, moMODE_NEWS, 1, NULL},
    {"Thr>","Thread >","Go to next thread",mo_news_nextt,&toolbarNewsFFwd, &toolbarNewsFFwdGRAY, moMODE_NEWS, 1, NULL},
    {"Post","Post","Post a UseNet Article",mo_news_post,&toolbarPost, &toolbarPostGRAY, moMODE_NEWS, 1, NULL},
    {"Foll","Followup","Follow-up to UseNet Article",mo_news_follow,&toolbarFollow, &toolbarFollowGRAY, moMODE_NEWS, 1, NULL},
/* FTP Mode */
    {"Put","Put","Send file to remote host",mo_ftp_put,&toolbarFTPput, NULL, moMODE_FTP, 1, NULL},
    {"Mkdir","Mkdir","Make remote directory",mo_ftp_mkdir,&toolbarFTPmkdir, NULL, moMODE_FTP, 1, NULL},
    {NULL, NULL, NULL, 0, NULL, NULL, 0, 0, NULL}
};

/* NOTE: THESE MUST COINCIDE EXACTLY WITH mo_tools!!! */
char *tool_names[] = {
	"BACK",
	"FORWARD",
	"RELOAD",
	"HOME",
	"OPEN",
	"SAVE",
	"NEW",
	"CLONE",
	"CLOSE",
	"ADD_TO_HOTLIST",
	"FIND",
	"PRINT",
	"GROUPS",
	"INDEX",
	"PREVIOUS_THREAD",
	"PREVIOUS_ARTICLE",
	"NEXT_ARTICLE",
	"NEXT_THREAD",
	"POST",
	"FOLLOW_UP",
	"PUT",
	"MKDIR",
	NULL
};
int use_tool[BTN_COUNT];


void mo_get_tools_from_res() {

int i;
char *tools,*ptr,*start,*end;

	if (get_pref_boolean(eKIOSK)) {
		if (ptr=get_pref_string(eTOOLBAR_LAYOUT)) {
			fprintf(stderr,"Toolbar Resource is Overiding the Kiosk Toolbar.\n");
		}
		else if (get_pref_boolean(eKIOSKPRINT)) {
			ptr=strdup("BACK,FORWARD,HOME,CLOSE,PRINT");
		}
		else {
			ptr=strdup("BACK,FORWARD,HOME,CLOSE");
		}
	}
	else if (get_pref_boolean(eKIOSKNOEXIT)) {
		if (ptr=get_pref_string(eTOOLBAR_LAYOUT)) {
			fprintf(stderr,"Toolbar Resource is Overiding the Kiosk Toolbar.\n");
		}
		else if (get_pref_boolean(eKIOSKPRINT)) {
			ptr=strdup("BACK,FORWARD,HOME,PRINT");
		}
		else {
			ptr=strdup("BACK,FORWARD,HOME");
		}
	}
	else if (!(ptr=get_pref_string(eTOOLBAR_LAYOUT))) {
		ptr="BACK,FORWARD,RELOAD,HOME,OPEN,SAVE,CLONE,CLOSE,FIND,PRINT,GROUPS,INDEX,PREVIOUS_THREAD,PREVIOUS_ARTICLE,NEXT_ARTICLE,NEXT_THREAD,POST,FOLLOW_UP,PUT,MKDIR";
	}
	tools=strdup(ptr);

	for (i=0; tool_names[i]; i++) {
		use_tool[i]=0;
	}

	for (start=tools; start && *start; ) {
		ptr=start;
		for (; *ptr && isspace(*ptr); ptr++);
		if (*ptr==',') {
			ptr++;
		}
		end=strchr(ptr,',');
		if (end) {
			start=end+1;
			*end='\0';
		}
		else {
			start=NULL;
		}
		for (i=0; tool_names[i]; i++) {
			if (!strncmp(tool_names[i],ptr,strlen(tool_names[i]))) {
				use_tool[i]=1;
			}
		}
	}

	free(tools);

	return;
}


void mo_make_globe(mo_window *win, Widget parent, int small);

void mo_tool_detach_cb(Widget wx, XtPointer cli, XtPointer call)
{
    Atom WM_DELETE_WINDOW;
    mo_window *win = (mo_window *) cli;
    int h,w;

        /* Xmx sucks */
    XmxSetUniqid(win->id);

    XtUnmanageChild(XtParent(win->scrolled_win));

    if(win->toolbardetached){
        win->toolbardetached = 0;
        XtUnmanageChild(win->toolbarwin);
        XtDestroyWidget(win->toolbarwin);
        win->toolbarwin = NULL;
        win->topform = win->slab[SLAB_TOOLS];
        mo_fill_toolbar(win,win->topform,0,0);
        if(win->biglogo && !win->smalllogo){
            mo_make_globe(win, win->slab[SLAB_GLOBE], 0);
        }
    } else {
        win->toolbardetached = 1;
        XtUnmanageChild(win->button_rc);
        XtUnmanageChild(win->button2_rc);

        XtDestroyWidget(win->button_rc);
        XtDestroyWidget(win->button2_rc);

        if(win->biglogo && !win->smalllogo){
            XtUnmanageChild(win->logo);
            XtDestroyWidget(win->logo);
            XtUnmanageChild(win->security);
            XtDestroyWidget(win->security);
            XtUnmanageChild(win->encrypt);
            XtDestroyWidget(win->encrypt);
        }
        win->toolbarwin = XtVaCreatePopupShell
            ("ToolBox",
/*             topLevelShellWidgetClass,*/
             xmDialogShellWidgetClass,
             win->base,
             XmNminHeight, h = win->toolbarorientation?640:40,
             XmNminWidth, w = win->toolbarorientation?40:640,
             XmNmaxHeight, h,
             XmNmaxWidth, w,
             XmNheight, h,
             XmNwidth, w,
             XmNallowShellResize, FALSE,
             NULL);
        XtManageChild(win->toolbarwin);
        win->topform = XtVaCreateWidget
            ("slab_tools",
             xmFormWidgetClass, win->toolbarwin,
             XmNminHeight, h,
             XmNminWidth, w,
             XmNmaxHeight, h,
             XmNmaxWidth, w,
             XmNheight, h,
             XmNwidth, w,
             NULL);
        mo_fill_toolbar(win,win->topform,0,0);
        XtManageChild(win->topform);
        WM_DELETE_WINDOW = XmInternAtom(dsp, "WM_DELETE_WINDOW", False);
        XmAddWMProtocolCallback(win->toolbarwin, WM_DELETE_WINDOW,
                                mo_tool_detach_cb, (XtPointer)win);

        XtPopup(win->toolbarwin, XtGrabNone);
    }

    XtManageChild(XtParent(win->scrolled_win));

}

void mo_switch_mode(mo_window *win)
{
    int i;
    for(i=0;mo_tools[i].label;i++){
        if(use_tool[i] && win->tools[i].w){
            if(!(mo_tools[i].toolset & win->mode)) {
                if(XtIsManaged(win->tools[i].w))
                    XtUnmanageChild(win->tools[i].w);
            } else {
                if(!XtIsManaged(win->tools[i].w))
                    XtManageChild(win->tools[i].w);
            }
        }
    }
}



mo_tool_state(struct toolbar *t,int state,int index)
{
    if (use_tool[index]) {
	XmxSetSensitive (t->w, t->gray = state);
    }
}


void mo_extra_buttons(mo_window *win, Widget top)
{
    win->security = XmxMakeNamedPushButton (top, NULL, "sec",
                                            security_pressed_cb, 0);
    XmxApplyPixmapToLabelWidget (win->security, securityUnknown);

    XtVaSetValues(win->security,
                  XmNmarginWidth, 0,
                  XmNmarginHeight, 0,
                  XmNmarginTop, 0,
                  XmNmarginBottom, 0,
                  XmNmarginLeft, 0,
                  XmNmarginRight, 0,
                  XmNuserData, (XtPointer) "Security Stats Information",
		  XmNtraversalOn, False,
		  NULL);

    XtOverrideTranslations(win->security,
                            XtParseTranslationTable(xlattab));

    win->encrypt = XtVaCreateManagedWidget
        (" ", xmPushButtonWidgetClass,
         top,
         XmNmarginWidth, 0,
         XmNmarginHeight, 0,
         XmNmarginTop, 0,
         XmNmarginBottom, 0,
         XmNmarginLeft, 0,
         XmNmarginRight, 0,
         XmNuserData, (XtPointer) "Encryption Status (not in this release)",
	 XmNtraversalOn, False,
         NULL);

    XmxApplyPixmapToLabelWidget (win->encrypt, enc_not_secure);

    XtOverrideTranslations(win->encrypt,XtParseTranslationTable(xlattab));

            /* insure we set the security icon! */
    if (win->current_node) {
        mo_gui_check_security_icon_in_win(win->current_node->authType,win);
    }

}


void mo_make_globe(mo_window *win, Widget parent, int small)
{
    int tmp = 25;

    if(!small){
      IconPix = IconPixBig;
      IconWidth = IconHeight = 64;
      WindowWidth = WindowHeight = 0;
      logo_count = 0;
      set_pref(ePIX_COUNT, (void *)&logo_save);
    } else {
        IconPix = IconPixSmall;
        IconWidth = IconHeight = 32;
        logo_count = 0;
        set_pref(ePIX_COUNT, (void *)&tmp);
        WindowWidth = WindowHeight = 0;
    }

    win->logo = XmxMakeNamedPushButton
        (parent, NULL, "logo", icon_pressed_cb, 0);
    XmxApplyPixmapToLabelWidget(win->logo, IconPix[0]);
    XtVaSetValues(win->logo,
                  XmNmarginWidth, 0,
                  XmNmarginHeight, 0,
                  XmNmarginTop, 0,
                  XmNmarginBottom, 0,
                  XmNmarginLeft, 0,
                  XmNmarginRight, 0,
                  XmNtopAttachment, XmATTACH_FORM,
                  XmNbottomAttachment, XmATTACH_FORM,
                  XmNleftAttachment, !win->biglogo || (!win->smalllogo && win->toolbardetached) ? XmATTACH_FORM : XmATTACH_NONE,
                  XmNrightAttachment, XmATTACH_FORM,
		  XmNtraversalOn, False,
                  NULL);

    if(win->biglogo){
        if(win->smalllogo){
            mo_extra_buttons(win,win->slab[SLAB_GLOBE]);
            XtVaSetValues(win->security,
                          XmNtopAttachment, XmATTACH_FORM,
                          XmNbottomAttachment, XmATTACH_FORM,
                          XmNleftAttachment, XmATTACH_FORM,
                          XmNrightAttachment, XmATTACH_NONE,
                          NULL);
            XtVaSetValues(win->encrypt,
                          XmNtopAttachment, XmATTACH_FORM,
                          XmNbottomAttachment, XmATTACH_FORM,
                          XmNleftAttachment, XmATTACH_WIDGET,
                          XmNleftWidget, win->security,
                          XmNrightAttachment, XmATTACH_WIDGET,
                          XmNrightWidget, win->logo,
                          NULL);
        } else {
            if(!win->toolbardetached){
                mo_extra_buttons(win,win->slab[SLAB_GLOBE]);
                XtVaSetValues(win->security,
                              XmNtopAttachment, XmATTACH_FORM,
                              XmNbottomAttachment, XmATTACH_NONE,
                              XmNleftAttachment, XmATTACH_FORM,
                              XmNrightAttachment, XmATTACH_WIDGET,
                              XmNrightWidget, win->logo,
                              NULL);
                XtVaSetValues(win->encrypt,
                              XmNtopAttachment, XmATTACH_WIDGET,
                              XmNtopWidget, win->security,
                              XmNbottomAttachment, XmATTACH_FORM,
                              XmNleftAttachment, XmATTACH_FORM,
                              XmNrightAttachment, XmATTACH_WIDGET,
                              XmNrightWidget, win->logo,
                              NULL);
            }
        }
    }
    XtVaSetValues(win->logo,
                  XmNuserData, (XtPointer) "Logo Button - Abort a Transaction",
                  NULL);

    XtOverrideTranslations(win->logo,
                          XtParseTranslationTable(xlattab));
}

/* create topform and fill it with toolbar bits'n'pieces */
Widget mo_fill_toolbar(mo_window *win, Widget top, int w, int h)
{
    int tmp = 25;
    Widget rightform, tearbutton, btn;
    int i,vert = win->toolbarorientation && win->toolbardetached;
    int textbuttons = win->texttools;
    int long_text = get_pref_boolean(eUSE_LONG_TEXT_NAMES);
    static XFontStruct *tmpFont=NULL;
    static XmFontList tmpFontList;

    if (!tmpFont) {
	XtVaGetValues(win->scrolled_win,
		      WbNtoolbarFont,
		      &tmpFont,
		      NULL);
	if (!tmpFont) {
		fprintf(stderr,"Toolbar Font: Could not load! The X Resource is Mosaic*ToolbarFont\nDefault font is: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso8859-1\nExiting Mosaic.");

		exit(1);
	}
	tmpFontList = XmFontListCreate(tmpFont,XmSTRING_DEFAULT_CHARSET);
    }

    /* Which tools to show */
    mo_get_tools_from_res();

    win->topform = top;

        /* Xmx sucks */
    XmxSetUniqid(win->id);

    win->button2_rc = XtVaCreateWidget
        ("buttonrc2", xmRowColumnWidgetClass,
         win->topform,
         XmNorientation, vert?XmVERTICAL:XmHORIZONTAL,
         XmNmarginWidth, 0,
         XmNmarginHeight, 0,
         XmNspacing, 0,
         XmNleftOffset, 0,
         XmNrightOffset, 0,
         XmNtopOffset, 0,
         XmNbottomOffset, 0,
         XmNleftAttachment, XmATTACH_NONE,
         XmNrightAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);


    win->button_rc = XtVaCreateWidget
        ("buttonrc", xmRowColumnWidgetClass,
         win->topform,
         XmNorientation, vert?XmVERTICAL:XmHORIZONTAL,
/*         XmNpacking, XmPACK_TIGHT,*/
         XmNpacking, XmPACK_TIGHT,
/*
         XmNmarginWidth, 1,
*/
         XmNmarginWidth, 0,

         XmNmarginHeight, 1,
         XmNspacing, 0,
         XmNleftOffset, 0,
         XmNrightOffset, 0,
         XmNtopOffset, 2,
         XmNbottomOffset, 2,
         XmNleftAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, win->button2_rc,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    tearbutton = XtVaCreateManagedWidget
        ("|",xmPushButtonWidgetClass,
         win->button_rc,
         XmNuserData, (XtPointer) "Toolbar Tearoff Control",
         XmNlabelType, textbuttons ? XmSTRING : XmPIXMAP,
         XmNlabelPixmap, vert?tearh:tearv,

	 XmNfontList, tmpFontList,
	 XmNtraversalOn, False,
         NULL);

    XtOverrideTranslations(tearbutton,XtParseTranslationTable(xlattab));

    XtAddCallback(tearbutton,
                  XmNactivateCallback, mo_tool_detach_cb,
                  (XtPointer) win);

    for(i=0;mo_tools[i].label;i++) {
        if(mo_tools[i].action>0){
	    if (use_tool[i]) {
              win->tools[i].w = XtVaCreateManagedWidget
                ((long_text?mo_tools[i].long_text:mo_tools[i].text)
		 ,xmPushButtonWidgetClass,
                 win->button_rc,
                 XmNuserData, (XtPointer) mo_tools[i].label,
                 XmNmarginWidth, 0,
                 XmNmarginHeight, 0,
                 XmNmarginTop, 0,
                 XmNmarginBottom, 0,
/*
                 XmNmarginLeft, textbuttons ? 2 : 0,
                 XmNmarginRight, textbuttons ? 2 : 0,
*/
                 XmNmarginLeft, 0,
                 XmNmarginRight, 0,

                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNlabelType, textbuttons ? XmSTRING : XmPIXMAP,
                 XmNlabelPixmap, *(mo_tools[i].image),

		 XmNfontList, tmpFontList,
		 XmNtraversalOn, False,
                 NULL);

              XtOverrideTranslations(win->tools[i].w,
                                  XtParseTranslationTable(xlattab));
              if(mo_tools[i].greyimage != NULL)
                XtVaSetValues(win->tools[i].w,
                              XmNlabelInsensitivePixmap,
                              *(mo_tools[i].greyimage),
                              NULL);
              XmxSetSensitive(win->tools[i].w,win->tools[i].gray);
              XmxAddCallback(win->tools[i].w,
                           XmNactivateCallback, menubar_cb,
                           mo_tools[i].action);

              if(!(mo_tools[i].toolset & win->mode))
                XtUnmanageChild(win->tools[i].w);
	    }
	} else {
            win->tools[i].w=NULL;
            XtVaCreateManagedWidget
                (" ",xmSeparatorWidgetClass,
                 win->button_rc,
                 XmNorientation, vert?XmHORIZONTAL:XmVERTICAL,
                 vert?XmNheight:XmNwidth, vert?3:4,
                 XmNseparatorType, XmNO_LINE,
		 XmNtraversalOn, False,
                 NULL);
	}
    }


    if(!win->biglogo || (!win->smalllogo && win->toolbardetached)){
        mo_extra_buttons(win, win->button2_rc);
        mo_make_globe(win, win->button2_rc, 1);
    }

    XtManageChild(win->button2_rc);
    XtManageChild(win->button_rc);

    return (win->topform);
}



/****************************************************************************
 * name:    mo_fill_window (PRIVATE)
 * purpose: Take a new (empty) mo_window struct and fill in all the
 *          GUI elements.
 * inputs:
 *   - mo_window *win: The window.
 * returns:
 *   mo_succeed
 * remarks:
 *
 ****************************************************************************/
static mo_status mo_fill_window (mo_window *win)
{
  Widget up, dn;
  int linkup, topatt, botatt;
  Widget form,topform,botform;
  Widget rightform,title_label,url_label;
  int i,globe,height;
  char *s;
  static char *pres_slab=NULL;
  static char *kiosk_slab=NULL;

  form = XtVaCreateManagedWidget("form0", xmFormWidgetClass, win->base, NULL);

  if (scount<0 && pres) {
	if (!pres_slab) {
		pres_slab=strdup("VIEW");
	}
	s=pres_slab;
  }
  else {
	if (s=get_pref_string(eGUI_LAYOUT)) {
		if (get_pref_boolean(eKIOSK) || get_pref_boolean(eKIOSKNOEXIT)) {
			fprintf(stderr,"The Gui Layout Resource is Overiding the Kiosk Resource.\n");
		}
	}
	else if (get_pref_boolean(eKIOSK) || get_pref_boolean(eKIOSKNOEXIT)) { /*we be kiosking*/
		if (!kiosk_slab) {
			kiosk_slab=strdup("TOOLS,STATUS,VIEW");
		}
		s=kiosk_slab;
	}
  }

  if((scount < 0)  && s){
      parse_slabinfo(s);
  }
  if(scount < 0){
          /* go with the default layout */
      win->smalllogo = 0;
      win->texttools = 0;

      win->slabcount = 6;
      win->slabpart[0] = SLAB_MENU;
      win->slabpart[1] = SLAB_GLOBE;
      win->slabpart[2] = SLAB_TOOLS;
      win->slabpart[3] = SLAB_URL;
      win->slabpart[4] = SLAB_VIEW;
      win->slabpart[5] = SLAB_STATUS;
  } else {
      win->texttools = stexttools;
      win->smalllogo = smalllogo;
      win->slabcount = scount;
      for(i=0;i<scount;i++){
          win->slabpart[i] = sarg[i];
      }
  }

  win->biglogo=0;
  for(i=0;i<win->slabcount;i++){
      if(win->slabpart[i]==SLAB_GLOBE) win->biglogo=1;
  }

      /* no active toolset, horiz, not detached */
  win->toolset = 0;
  win->toolbarorientation = 0;
  win->toolbardetached = 0;
  win->toolbarwin = NULL;



      /*********************** SLAB_GLOBE ****************************/
  if(win->biglogo){
    win->slab[SLAB_GLOBE] = XtVaCreateWidget("slab_globe",
                                             xmFormWidgetClass, form, NULL);

    mo_make_globe(win,win->slab[SLAB_GLOBE],win->smalllogo);
  } else {
      win->slab[SLAB_GLOBE] = NULL;
  }


      /*********************** SLAB_MENU ****************************/
  win->menubar = mo_make_document_view_menubar (form);
  win->slab[SLAB_MENU] = win->menubar->base;
  XtUnmanageChild(win->slab[SLAB_MENU]);

      /*********************** SLAB_TITLE ****************************/
  win->slab[SLAB_TITLE] = XtVaCreateWidget("slab_title",
                                           xmFormWidgetClass, form,
                                           XmNheight, 36, NULL);
  title_label = XtVaCreateManagedWidget("Title:",xmLabelWidgetClass,
                                        win->slab[SLAB_TITLE],
                                        XmNleftOffset, 3,
                                        XmNleftAttachment, XmATTACH_FORM,
                                        XmNrightAttachment, XmATTACH_NONE,
                                        XmNtopAttachment, XmATTACH_FORM,
                                        XmNbottomAttachment, XmATTACH_FORM,
                                        NULL);
  win->title_text = XtVaCreateManagedWidget("title",xmTextFieldWidgetClass,
                                            win->slab[SLAB_TITLE],
                                            XmNrightOffset, 3,
                                            XmNleftOffset, 3,
                                            XmNtopOffset, 3,
                                            XmNleftAttachment, XmATTACH_WIDGET,
                                            XmNleftWidget, title_label,
                                            XmNrightAttachment, XmATTACH_FORM,
                                            XmNtopAttachment, XmATTACH_FORM,
                                            XmNbottomAttachment, XmATTACH_NONE,
                                            XmNeditable, False,
                                            XmNcursorPositionVisible, False,
                                            NULL);


      /*********************** SLAB_URL ****************************/
  win->slab[SLAB_URL] = XtVaCreateWidget("slab_url",
                                         xmFormWidgetClass, form,
                                         XmNheight, 36, NULL);
  url_label = XtVaCreateManagedWidget("URL:",xmLabelWidgetClass,
                                      win->slab[SLAB_URL],
                                      XmNleftOffset, 3,
                                      XmNleftAttachment, XmATTACH_FORM,
                                      XmNrightAttachment, XmATTACH_NONE,
                                      XmNtopAttachment, XmATTACH_FORM,
                                      XmNbottomAttachment, XmATTACH_FORM,
                                      NULL);
  win->url_text = XtVaCreateManagedWidget("text",xmTextFieldWidgetClass,
                                          win->slab[SLAB_URL],
                                          XmNrightOffset, 3,
                                          XmNleftOffset, 3,
                                          XmNtopOffset, 3,
                                          XmNleftAttachment, XmATTACH_WIDGET,
                                          XmNleftWidget, url_label,
                                          XmNrightAttachment, XmATTACH_FORM,
                                          XmNtopAttachment, XmATTACH_FORM,
                                          XmNbottomAttachment, XmATTACH_NONE,
                                          XmNcursorPositionVisible, True,
                                          XmNeditable, True,
					  XmNtraversalOn, False,
                                          NULL);
      /* DO THIS WITH THE SLAB MANAGER - BJS */
  if (!(get_pref_boolean(eKIOSK) || get_pref_boolean(eKIOSKNOEXIT))) {
      XmxAddCallbackToText (win->url_text, url_field_cb, 0);
  } else {
      XtUnmanageChild(url_label);
      XtUnmanageChild(win->url_text);
  }

  XtOverrideTranslations(win->url_text,
			 XtParseTranslationTable(text_translations));
  XtOverrideTranslations(win->title_text,
			 XtParseTranslationTable(text_translations));
  XtOverrideTranslations(win->url_text,
			 XtParseTranslationTable(url_translations));


      /*********************** SLAB_VIEW ****************************/
  win->slab[SLAB_VIEW] = win->scrolled_win = XtVaCreateManagedWidget
      ("view", htmlWidgetClass, form,
       WbNtext, 0,
       XmNresizePolicy, XmRESIZE_ANY,
       WbNpreviouslyVisitedTestFunction, anchor_visited_predicate,
       WbNpointerMotionCallback, pointer_motion_callback,
       WbNfancySelections, win->pretty ? True : False,
       WbNdelayImageLoads, win->delay_image_loads ? True : False,
       XmNshadowThickness, 2,
       NULL);
  mo_register_image_resolution_function (win);
  XmxAddCallback (win->scrolled_win, WbNanchorCallback, anchor_cb, 0);
  XmxAddCallback (win->scrolled_win, WbNlinkCallback, link_callback, 0);
  XmxAddCallback (win->scrolled_win, WbNsubmitFormCallback,
                  submit_form_callback, 0);
  XtVaGetValues(win->scrolled_win, WbNview, (long)(&win->view), NULL);
  XmxAddEventHandler
    (win->view, KeyPressMask, mo_view_keypress_handler, 0);
 /* now that the htmlWidget is created we can do this  */
  mo_make_popup(win->view);


      /*********************** SLAB_STATUS ****************************/
  win->slab[SLAB_STATUS] = XtVaCreateWidget("slab_status",
                                            xmFormWidgetClass, form, NULL);

  if (get_pref_boolean(eKIOSK) || get_pref_boolean(eKIOSKNOEXIT)) {
	set_pref_boolean(eMETER,False);
  }

  /* meter */
  if (get_pref_boolean(eMETER)) {
      win->meter_text = NULL;
      win->meter_notext = 0;
      win->meter_font = 0;
      win->meter_frame = XmxMakeFrame(win->slab[SLAB_STATUS], XmxShadowIn);
      XtVaSetValues(win->meter_frame,
                    XmNrightOffset, 3,
                    XmNtopOffset, 2,
                    XmNbottomOffset, 2,
                    XmNleftAttachment, XmATTACH_NONE,
                    XmNrightAttachment, XmATTACH_FORM,
                    XmNtopAttachment, XmATTACH_FORM,
                    XmNbottomAttachment, XmATTACH_FORM,
                    NULL);

      win->meter = XtVaCreateManagedWidget
          ("meter", xmDrawingAreaWidgetClass,
           win->meter_frame,
           XmNuserData, (XtPointer) "Progress Meter",
           XmNheight, 16,
           XmNwidth, 96,
           NULL);
      XtOverrideTranslations(win->meter,
                            XtParseTranslationTable(xlattab));

      win->meter_level = 0;
      win->meter_width = -1;

      XtAddCallback(win->meter, XmNexposeCallback,
                    DrawMeter, (XtPointer) win);
      XtAddCallback(win->meter, XmNresizeCallback,
                    ResizeMeter, (XtPointer) win);

          /* grab some colors */
      {
          XColor ccell1,ccell2;

          XAllocNamedColor(dsp,(installed_colormap ?
                                installed_cmap :
                                DefaultColormapOfScreen(XtScreen(win->base))),
                           get_pref_string(eMETER_FOREGROUND),
                           &ccell1,&ccell2);
          win->meter_fg = ccell2.pixel;
          XAllocNamedColor(dsp,(installed_colormap ?
                                installed_cmap :
                                DefaultColormapOfScreen(XtScreen(win->base))),
                           get_pref_string(eMETER_BACKGROUND),
                           &ccell1,&ccell2);
          win->meter_bg = ccell2.pixel;
          XAllocNamedColor(dsp,(installed_colormap ?
                                installed_cmap :
                                DefaultColormapOfScreen(XtScreen(win->base))),
                           get_pref_string(eMETER_FONT_FOREGROUND),
                           &ccell1,&ccell2);
          win->meter_font_fg = ccell2.pixel;
          XAllocNamedColor(dsp,(installed_colormap ?
                                installed_cmap :
                                DefaultColormapOfScreen(XtScreen(win->base))),
                           get_pref_string(eMETER_FONT_BACKGROUND),
                           &ccell1,&ccell2);
          win->meter_font_bg = ccell2.pixel;
      }
  } else {
      win->meter_frame = NULL;
      win->meter = NULL;
  }

  win->tracker_label = XtVaCreateManagedWidget
      (" ",xmLabelWidgetClass,
       win->slab[SLAB_STATUS],
       XmNalignment, XmALIGNMENT_BEGINNING,
       XmNleftAttachment, XmATTACH_FORM,
       XmNrightAttachment, get_pref_boolean(eMETER) ? XmATTACH_WIDGET : XmATTACH_NONE,
       XmNrightWidget, get_pref_boolean(eMETER) ? win->meter_frame : NULL,
       XmNtopAttachment, XmATTACH_FORM,
       XmNbottomAttachment, XmATTACH_NONE,
                                               NULL);


      /*********************** SLAB_TOOLS ****************************/
  win->slab[SLAB_TOOLS] = XtVaCreateWidget("slab_tools",
                                            xmFormWidgetClass, form,
                                            NULL);

  mo_fill_toolbar(win,win->slab[SLAB_TOOLS],640,32);

      /* chain those slabs together 'n stuff */
  for(globe=0,linkup=1,i=0;i<win->slabcount;i++){
      if(win->slabpart[i] == SLAB_GLOBE){
/* next two slabs have to attach to the globe */
          globe=2-win->smalllogo;
          if(linkup){
              XtVaSetValues
                  (win->slab[SLAB_GLOBE],
                   XmNleftAttachment, XmATTACH_NONE,
                   XmNrightAttachment, XmATTACH_FORM,
                   XmNtopAttachment, i ? XmATTACH_WIDGET : XmATTACH_FORM,
                   XmNtopWidget, i ?win->slab[win->slabpart[i-1]] : NULL,
                   XmNbottomAttachment, XmATTACH_NONE,
                   NULL);
          } else {
              XtVaSetValues
                  (win->slab[SLAB_GLOBE],
                   XmNleftAttachment, XmATTACH_NONE,
                   XmNrightAttachment, XmATTACH_FORM,
                   XmNbottomAttachment, i+globe+1==win->slabcount ? XmATTACH_FORM : XmATTACH_WIDGET,
                   XmNbottomWidget, i+globe+1==win->slabcount ? NULL : win->slab[win->slabpart[i+globe+1]],
                   XmNtopAttachment, XmATTACH_NONE,
                   NULL);
          }

      } else {
          if(win->slabpart[i] == SLAB_VIEW){
                  /* we change link dir here AND link this slab both ways*/
              linkup = 0;
              if(!i || ((i==1) && (win->slabpart[i-1]==SLAB_GLOBE))){
                  up = NULL;
                  topatt = XmATTACH_FORM;
              } else {
                  up = win->slab[win->slabpart[(i-1) - (globe==2 ? 1 : 0)]];
                  topatt = XmATTACH_WIDGET;
              }
              if(i==win->slabcount-1){
                  dn = NULL;
                  botatt = XmATTACH_FORM;
              } else {
                  dn = win->slab[win->slabpart[i+1]];
                  botatt = XmATTACH_WIDGET;
              }
          } else {
              if(linkup){
                  if(!i || ((i==1) && (win->slabpart[i-1]==SLAB_GLOBE))){
                      up = NULL;
                      topatt = XmATTACH_FORM;
                  } else {
                      if(globe==1 && win->smalllogo){
                          up = win->slab[win->slabpart[i-2]];
                      } else {
                          up = win->slab[win->slabpart[(i-1)-(globe==2 ? 1 : 0)]];
                      }

                      topatt = XmATTACH_WIDGET;
                  }
                  dn = NULL;
                  botatt = XmATTACH_NONE;
              } else {
                  if(i==win->slabcount-1){
                      dn = NULL;
                      botatt = XmATTACH_FORM;
                  } else {
                      dn = win->slab[win->slabpart[i+1]];
                      botatt = XmATTACH_WIDGET;
                  }
                  up = NULL;
                  topatt = XmATTACH_NONE;
              }
          }

              /*
          fprintf(stderr,"%s (0x%08X): up=0x%08X dn=0x%08X ta=%d ba=%d\n",
                  slab_words[win->slabpart[i]], win->slab[win->slabpart[i]],
                  up, dn, topatt, botatt);
                  */

          XtVaSetValues(win->slab[win->slabpart[i]],
                        XmNleftOffset, 0,
                        XmNrightOffset, 0,
                        XmNtopOffset, 0,
                        XmNbottomOffset, 0,
                        XmNtopAttachment, topatt,
                        XmNtopWidget, up,
                        XmNbottomAttachment, botatt,
                        XmNbottomWidget, dn,
                        XmNleftAttachment, XmATTACH_FORM,
                        XmNrightAttachment, globe ? XmATTACH_WIDGET : XmATTACH_FORM,
                        XmNrightWidget, win->slab[SLAB_GLOBE],
                        NULL);
          if(globe) globe--;
      }
  }
  for(i=0;i<win->slabcount;i++)
      XtManageChild(win->slab[win->slabpart[i]]);

  XtManageChild(form);

  /* Can't go back or forward if we haven't gone anywhere yet... */
  mo_back_impossible (win);
  mo_forward_impossible (win);

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_delete_window
 * purpose: Shut down a window.
 * inputs:
 *   - mo_window *win: The window.
 * returns:
 *   mo_succeed
 * remarks:
 *   This can be called, among other places, from the WM_DELETE_WINDOW
 *   handler.  By the time we get here, we must assume the window is already
 *   in the middle of being shut down.
 *   We must explicitly close every dialog that be open as a child of
 *   the window, because window managers too stupid to do that themselves
 *   will otherwise allow them to stay up.
 ****************************************************************************/
#define POPDOWN(x) \
  if (win->x) XtUnmanageChild (win->x)

mo_status mo_delete_window (mo_window *win)
{
  mo_node *node;

  if (!win)
    return mo_fail;

  node = win->history;

  POPDOWN (source_win);
  POPDOWN (save_win);
  POPDOWN (savebinary_win);
  POPDOWN (open_win);
  POPDOWN (mail_win);
  POPDOWN (mailhist_win);
  POPDOWN (print_win);
  POPDOWN (history_win);
  POPDOWN (open_local_win);
  if (win->hotlist_win)
    XtDestroyWidget(win->hotlist_win);
  POPDOWN (techsupport_win);
  POPDOWN (annotate_win);
  POPDOWN (search_win);
  POPDOWN (searchindex_win);
  POPDOWN (mailto_win);
  POPDOWN (mailto_form_win);
  POPDOWN (news_win);
  POPDOWN (links_win);
#ifdef HAVE_DTM
  POPDOWN (dtmout_win);
#endif
#ifdef HAVE_AUDIO_ANNOTATIONS
  POPDOWN (audio_annotate_win);
#endif
  XtPopdown (win->base);

      /* we really should be doing this :-) BJS */
  XtDestroyWidget(win->base);
  win->base=NULL;

  /* Free up some of the HTML Widget's state */
  if (win && win->scrolled_win) {
	HTMLFreeImageInfo (win->scrolled_win);
  }

  while (node)
    {
      mo_node *tofree = node;
      node = node->next;
      mo_free_node_data (tofree);
      free (tofree);
    }
  win->history=NULL;

  free (win->search_start);
  win->search_start=NULL;
  free (win->search_end);
  win->search_end=NULL;

  /* This will free the win structure (but none of its elements
     individually) and exit if this is the last window in the list. */
  mo_remove_window_from_list (win);

  /* Go get another current_win. */
  mo_set_current_cached_win (mo_next_window (NULL));

  return mo_succeed;
}


int mo_get_font_size_from_res(char *userfontstr,int *fontfamily)
{
  char *lowerfontstr = strdup(userfontstr);
  int   x;

  for (x=0; x<strlen(userfontstr); x++)
    lowerfontstr[x]=tolower(userfontstr[x]);

  *fontfamily = 0;
  if (strstr(lowerfontstr, "times")!=NULL)
    {
      if (strstr(lowerfontstr, "large")!=NULL)
	return mo_large_fonts;
      if (strstr(lowerfontstr, "regular")!=NULL)
	return mo_regular_fonts;
      if (strstr(lowerfontstr, "small")!=NULL)
	return mo_small_fonts;
      return mo_regular_fonts;
    }
  if (strstr(lowerfontstr, "helvetica")!=NULL)
    {
      *fontfamily = 1;
      if (strstr(lowerfontstr, "large")!=NULL)
	return mo_large_helvetica;
      if (strstr(lowerfontstr, "regular")!=NULL)
	return mo_regular_helvetica;
      if (strstr(lowerfontstr, "small")!=NULL)
	return mo_small_helvetica;
      return mo_regular_helvetica;
    }
  if (strstr(lowerfontstr, "century")!=NULL)
    {
      *fontfamily = 2;
      if (strstr(lowerfontstr, "large")!=NULL)
	return mo_large_newcentury;
      if (strstr(lowerfontstr, "regular")!=NULL)
	return mo_regular_newcentury;
      if (strstr(lowerfontstr, "small")!=NULL)
	return mo_small_newcentury;
      return mo_regular_newcentury;
    }
  if (strstr(lowerfontstr, "lucida")!=NULL)
    {
      *fontfamily = 3;
      if (strstr(lowerfontstr, "large")!=NULL)
	return mo_large_lucidabright;
      if (strstr(lowerfontstr, "regular")!=NULL)
	return mo_regular_lucidabright;
      if (strstr(lowerfontstr, "small")!=NULL)
	return mo_small_lucidabright;
      return mo_regular_lucidabright;
    }
  return mo_regular_fonts;
}


void kill_splash()
{
    if(splash_cc) {
        ReleaseSplashColors(splash);
    } else {
	XtPopdown(splash);
    }
    XtDestroyWidget(splash);
    splash=NULL;
}


extern gui_news_updateprefs (mo_window *win);
void mo_set_agents(mo_window *win, int which);

void mo_sync_windows(mo_window *win, mo_window *parent)
{

    win->font_size = parent->font_size;
    mo_set_fonts(win, parent->font_size);

    win->underlines_state = parent->underlines_state;
    mo_set_underlines (win, parent->underlines_state);

    win->agent_state = parent->agent_state;
    mo_set_agents(win, win->agent_state);


    imageViewInternal = win->image_view_internal = parent->image_view_internal;
    XmxRSetToggleState (win->menubar, mo_image_view_internal,
                      (win->image_view_internal ? XmxSet : XmxNotSet));

    tableSupportEnabled = win->table_support = parent->table_support;
    XmxRSetToggleState (win->menubar, mo_table_support,
                        win->table_support ? XmxSet : XmxNotSet);

    win->body_color = parent->body_color;
    XtVaSetValues(win->scrolled_win,
                  WbNbodyColors,
                  win->body_color,
                  NULL);
    XmxRSetToggleState (win->menubar, mo_body_color,
                        win->body_color ? XmxSet : XmxNotSet);

    win->body_images = parent->body_images;
    XtVaSetValues(win->scrolled_win,
                  WbNbodyImages,
                  win->body_images,
                  NULL);
    XmxRSetToggleState (win->menubar, mo_body_images,
                        win->body_images ? XmxSet : XmxNotSet);

    win->delay_image_loads = parent->delay_image_loads;
    XmxSetArg (WbNdelayImageLoads, win->delay_image_loads ? True : False);
    XmxSetValues (win->scrolled_win);
    XmxRSetSensitive (win->menubar, mo_expand_images_current,
                      win->delay_image_loads ? XmxSensitive : XmxNotSensitive);
    XmxRSetToggleState (win->menubar, mo_delay_image_loads,
                        win->delay_image_loads ? XmxSet : XmxNotSet);
}

/****************************************************************************
 * name:    mo_open_window_internal (PRIVATE)
 * purpose: Make a mo_window struct and fill up the GUI.
 * inputs:
 *   - Widget       base: The dialog widget on which this window is
 *                        to be based.
 *   - mo_window *parent: The parent mo_window struct for this window,
 *                        if one exists; this can be NULL.
 * returns:
 *   The new window (mo_window *).
 * remarks:
 *   This routine must set to 0 all elements in the mo_window struct
 *   that can be tested by various routines to see if various things
 *   have been done yet (popup windows created, etc.).
 ****************************************************************************/
/* FOO */
static mo_window *mo_open_window_internal (Widget base, mo_window *parent)
{
  mo_window *win;
  Widget dialog_pixmap;
  int i;

  win = (mo_window *)malloc (sizeof (mo_window));
  win->id = XmxMakeNewUniqid ();
  XmxSetUniqid (win->id);

  win->base = base;
  win->mode = moMODE_PLAIN;

  win->source_win = 0;
  win->save_win = 0;
  win->upload_win = 0;
  win->savebinary_win = 0;
  win->ftpput_win = win->ftpremove_win = win->ftpmkdir_win = 0;
/*
  win->tag_win = win->tag_list = 0;
  win->urlUnderPointer = NULL;
*/
  for(i=0;i<BTN_COUNT;i++) win->tools[i].gray = XmxSensitive;

  win->open_win = win->open_text = win->open_local_win = 0;
  win->mail_win = win->mailhot_win = win->edithot_win = win->mailhist_win =
    win->inserthot_win = 0;
  win->print_win = 0;
  win->history_win = win->history_list = 0;
  win->hotlist_win = win->hotlist_list = 0;
  win->techsupport_win = win->techsupport_text = 0;
  win->mailto_win = win->mailto_text = 0;
  win->mailto_form_win = win->mailto_form_text = 0;
  win->post_data=0;
  win->news_win = 0;
  win->links_win = 0;
  win->news_fsb_win = 0;
  win->mail_fsb_win = 0;
  win->annotate_win = 0;
  win->search_win = win->search_win_text = 0;
  win->searchindex_win = win->searchindex_win_label = win->searchindex_win_text = 0;
  win->src_search_win=0;
  win->src_search_win_text=0;
  win->cci_win = win->cci_win_text = (Widget) 0;
  win->cci_accept_toggle = win->cci_off_toggle = (Widget) 0;
#ifdef HAVE_DTM
  win->dtmout_win = win->dtmout_text = 0;
#endif
#ifdef HAVE_AUDIO_ANNOTATIONS
  win->audio_annotate_win = 0;
#endif

  win->history = NULL;
  win->current_node = 0;
  win->reloading = 0;
  win->source_text = 0;
  win->format_optmenu = 0;
  win->save_format = 0;
  if (!parent) {
    win->font_size = mo_get_font_size_from_res(get_pref_string(eDEFAULT_FONT_CHOICE),
			&(win->font_family));
    /*win->font_size = mo_regular_fonts;*/
    /*win->font_family = 0;*/
  } else {
    win->font_size = parent->font_size;
    win->font_family = parent->font_family;
  }

  win->agent_state=selectedAgent+mo_last_entry;

  win->underlines_snarfed = 0;
  if (!parent)
    win->underlines_state = mo_default_underlines;
  else
    win->underlines_state = parent->underlines_state;

  win->pretty = get_pref_boolean(eDEFAULT_FANCY_SELECTIONS);

  win->mail_format = 0;

#ifdef HAVE_AUDIO_ANNOTATIONS
  win->record_fnam = 0;
  win->record_pid = 0;
#endif

  win->print_text = 0;
  win->print_format = 0;

  win->target_anchor = 0;
  /* Create search_start and search_end. */
  win->search_start = (void *)malloc (sizeof (ElementRef));
  win->search_end = (void *)malloc (sizeof (ElementRef));
  win->src_search_pos=0;

#ifdef ISINDEX
  /* We don't know yet. */
  win->keyword_search_possible = -1;
#endif

/*SWP 7/3/95*/
  if (get_pref_boolean(eSECURITYICON)) {
        if (win->current_node) {
                mo_gui_check_security_icon_in_win(win->current_node->authType,win);
	}
  }
  win->delay_image_loads = get_pref_boolean(eDELAY_IMAGE_LOADS);
/* Install all the GUI bits & pieces. */
  mo_fill_window (win);

  XmxRSetToggleState (win->menubar, win->font_size, XmxSet);

  /* setup news default states */
   ConfigView = !get_pref_boolean (eUSETHREADVIEW);
   newsShowAllGroups = get_pref_boolean (eSHOWALLGROUPS);
   newsShowReadGroups = get_pref_boolean (eSHOWREADGROUPS);
   newsShowAllArticles = get_pref_boolean (eSHOWALLARTICLES);
   newsNoThreadJumping = get_pref_boolean (eNOTHREADJUMPING);
   gui_news_updateprefs (win);

  win->have_focus = False;

  win->binary_transfer = 0;
  XmxRSetToggleState (win->menubar, mo_binary_transfer,
                      (win->binary_transfer ? XmxSet : XmxNotSet));
  XmxRSetToggleState (win->menubar, mo_delay_image_loads,
                      (win->delay_image_loads ? XmxSet : XmxNotSet));
  XmxRSetSensitive (win->menubar, mo_expand_images_current,
                    win->delay_image_loads ? XmxSensitive : XmxNotSensitive);
  XmxRSetSensitive (win->menubar, mo_annotate, XmxSensitive);
  XmxRSetSensitive (win->menubar, mo_annotate_edit, XmxNotSensitive);
  XmxRSetSensitive (win->menubar, mo_annotate_delete, XmxNotSensitive);

  tableSupportEnabled = win->table_support = get_pref_boolean(eENABLE_TABLES);
  XmxRSetToggleState (win->menubar, mo_table_support,
                      (win->table_support ? XmxSet : XmxNotSet));

  imageViewInternal = win->image_view_internal = get_pref_boolean(eIMAGEVIEWINTERNAL);
  XmxRSetToggleState (win->menubar, mo_image_view_internal,
                      (win->image_view_internal ? XmxSet : XmxNotSet));

  /* take care of session history for rbm */

  if(get_pref_boolean(eSESSION_HISTORY_ON_RBM))
    {
      win->session_menu = NULL;
      win->num_session_items = 0;
  if(get_pref_boolean(eSESSION_HISTORY_ON_RBM))
    win->session_items = malloc(sizeof(Widget) *
				get_pref_int(eNUMBER_OF_ITEMS_IN_RBM_HISTORY));
    }

  /* Pop the window up. */
  XtPopup (win->base, XtGrabNone);
  XFlush (dsp);
  XSync (dsp, False);

  /* Register win with internal window list. */
  mo_add_window_to_list (win);

      /* Set the font size. */
  if (win->font_size != mo_regular_fonts)
    mo_set_fonts (win, win->font_size);

  /* Set the underline state. */
  mo_set_underlines (win, win->underlines_state);

  mo_set_agents(win, win->agent_state);

  /* Set the fancy selections toggle to the starting value. */
  mo_set_fancy_selections_toggle (win);

  if(parent) {
#ifndef DISABLE_TRACE
    if (srcTrace) {
      fprintf(stderr,"Window SYNCing\n");
    }
#endif
      mo_sync_windows(win,parent);
  }

  return win;
}


/****************************************************************************
 * name:    delete_cb (PRIVATE)
 * purpose: Callback for the WM_DELETE_WINDOW protocol.
 * inputs:
 *   - as per XmxCallback
 * returns:
 *   nothing
 * remarks:
 *   By the time we get called here, the window has already been popped
 *   down.  Just call mo_delete_window to clean up.
 ****************************************************************************/
static XmxCallback (delete_cb)
{
  mo_window *win = (mo_window *)client_data;
  mo_delete_window (win);
  return;
}


/****************************************************************************
 * name:    mo_make_window (PRIVATE)
 * purpose: Make a new window from scratch.
 * inputs:
 *   - Widget      parent: Parent for the new window shell.
 *   - mo_window *parentw: Parent window, if one exists (may be NULL).
 * returns:
 *   The new window (mo_window *).
 * remarks:
 *   The 'parent window' parentw is the window being cloned, or the
 *   window in which the 'new window' command was triggered, etc.
 *   Some GUI properties are inherited from it, if it exists (fonts,
 *   anchor appearance, etc.).
 ****************************************************************************/
static mo_window *mo_make_window (Widget parent, mo_window *parentw)
{
  Widget base;
  mo_window *win;
  Atom WM_DELETE_WINDOW;
  char buf[80];

  sprintf(pre_title,"NCSA X Mosaic %s",MO_VERSION_STRING);
  sprintf(buf,"%s: ",pre_title);
  XmxSetArg (XmNtitle, (long)buf);
  XmxSetArg (XmNiconName, (long)"Mosaic");
  XmxSetArg (XmNallowShellResize, False);
  if (installed_colormap) {
	XmxSetArg(XmNcolormap,installed_cmap);
  }
  base = XtCreatePopupShell ("shell", topLevelShellWidgetClass,
                             toplevel, Xmx_wargs, Xmx_n);
  Xmx_n = 0;

#ifdef EDITRES_SUPPORT
  XtAddEventHandler(base, (EventMask) 0, TRUE,
                    (XtEventHandler) _XEditResCheckMessages, NULL);
#endif

  XtOverrideTranslations(base, XtParseTranslationTable(toplevel_translations));


  win = mo_open_window_internal (base, parentw);

  WM_DELETE_WINDOW = XmInternAtom(dsp, "WM_DELETE_WINDOW", False);
  XmAddWMProtocolCallback(base, WM_DELETE_WINDOW, delete_cb, (XtPointer)win);

  return win;
}


/****************************************************************************
 * name:    mo_open_another_window_internal (PRIVATE)
 * purpose: Open a new window from an existing window.
 * inputs:
 *   - mo_window *win: The old window.
 * returns:
 *   The new window (mo_window *).
 * remarks:
 *   This routine handles (optional) autoplace of new windows.
 ****************************************************************************/
static mo_window *mo_open_another_window_internal (mo_window *win)
{
  Dimension oldx, oldy;
  Dimension scrx = WidthOfScreen (XtScreen (win->base));
  Dimension scry = HeightOfScreen (XtScreen (win->base));
  Dimension x, y;
  Dimension width, height;
  mo_window *newwin;

  XtVaGetValues (win->base, XmNx, &oldx, XmNy, &oldy,
                 XmNwidth, &width, XmNheight, &height, NULL);

  /* Ideally we open down and over 40 pixels... is this possible? */
  /* If not, deal with it... */

  /* Bug fix, thanks to Ken Shores <kss1376@pop.draper.com> */

  /* the original test did not handle the case where the old window
   * was exactly the same size as the screen.  Also, it used a looping
   * algorithm which would infinite loop under such a case. */

  if ((oldx+width) > (scrx-40))
    x = (scrx - (oldx + width));
  else
    x = oldx + 40;

  if ((oldy+height) > (scry-40))
    y = (scry - (oldy + height));
  else
    y = oldy + 40;

  if (x > scrx) x = 0;
  if (y > scry) y = 0;

  XmxSetArg (XmNdefaultPosition, False);
  if (get_pref_boolean(eAUTO_PLACE_WINDOWS))
    {
      char geom[20];
      sprintf (geom, "+%d+%d", x, y);
      XmxSetArg (XmNgeometry, (long)geom);
    }
  XmxSetArg (XmNwidth, width);
  XmxSetArg (XmNheight, height);

  newwin = mo_make_window (toplevel, win);
  mo_set_current_cached_win (newwin);
  return newwin;
}


/****************************************************************************
 * name:    mo_open_window
 * purpose: Open a new window to view a given URL.
 * inputs:
 *   - Widget      parent: The parent Widget for the new window's shell.
 *   - char          *url: The URL to view in the new window.
 *   - mo_window *parentw: The (optional) parent window of the new window.
 * returns:
 *   The new window.
 * remarks:
 *
 ****************************************************************************/
mo_window *mo_open_window (Widget parent, char *url, mo_window *parentw)
{
  mo_window *win = NULL;

  win = mo_make_window (parent, parentw);

  mo_set_current_cached_win (win);

  mo_load_window_text (win, url, NULL);

  return win;
}


/****************************************************************************
 * name:    mo_duplicate_window
 * purpose: Clone a existing window as intelligently as possible.
 * inputs:
 *   - mo_window *win: The existing window.
 * returns:
 *   The new window.
 * remarks:
 *
 ****************************************************************************/
mo_window *mo_duplicate_window (mo_window *win) {

mo_window *neww;

	if (win && win->current_node) {
		securityType=win->current_node->authType;
	}

	neww = mo_open_another_window_internal (win);

	mo_duplicate_window_text (win, neww);

	mo_gui_update_meter(100,NULL);

	return neww;
}



/****************************************************************************
 * name:    mo_open_another_window
 * purpose: Open another window to view a given URL, unless the URL
 *          indicates that it's pointless to do so
 * inputs:
 *   - mo_window      *win: The existing window.
 *   - char           *url: The URL to view in the new window.
 *   - char           *ref: The reference (hyperlink text contents) for this
 *                          URL; can be NULL.
 *   - char *target_anchor: The target anchor to view open opening the
 *                          window, if any.
 * returns:
 *   The new window.
 * remarks:
 *
 ****************************************************************************/
mo_window *mo_open_another_window (mo_window *win, char *url, char *ref,
                                   char *target_anchor)
{
  mo_window *neww;
  mo_status return_stat = mo_succeed;

  /* Check for reference to telnet.  Never open another window
     if reference to telnet exists; instead, call mo_load_window_text,
     which knows how to manage current window's access to telnet. */
  if (!strncmp (url, "telnet:", 7) || !strncmp (url, "tn3270:", 7) ||
      !strncmp (url, "rlogin:", 7))
    {
      mo_load_window_text (win, url, NULL);
      return NULL;
    }

  mo_busy ();

  neww = mo_open_another_window_internal (win);
  /* Set it here; hope it gets handled in mo_load_window_text_first
     (it probably won't, now. */
  neww->target_anchor = target_anchor;

  return_stat = mo_load_window_text (neww, url, ref);

  if ((cci_get) && (return_stat == mo_fail))
	return (mo_window *) NULL;

  return neww;
}


/* ------------------------------------------------------------------------ */

char **gargv;
int gargc;

#ifndef VMS
extern MO_SIGHANDLER_RETURNTYPE ProcessExternalDirective (MO_SIGHANDLER_ARGS);
#endif

#ifdef HAVE_DTM
static XmxCallback (blip)
{
  mo_dtm_poll_and_read ();

  XtAppAddTimeOut (app_context, 100, (XtTimerCallbackProc)blip, (XtPointer)True);

  return;
}
#endif

#ifdef HAVE_DTM
mo_status mo_register_dtm_blip (void)
{
  /* Set a timer that will poll DTM regularly. */
  XtAppAddTimeOut (app_context, 100, (XtTimerCallbackProc)blip, (XtPointer)True);

  return mo_succeed;
}
#endif


/****************************************************************************
 * name:    fire_er_up (PRIVATE)
 * purpose: Callback from timer that actually starts up the application,
 *          i.e., opens the first window.
 * inputs:
 *   - as per XmxCallback
 * returns:
 *   Nothing.
 * remarks:
 *   This routine figures out what the home document should be
 *   and then calls mo_open_window().
 ****************************************************************************/
static XmxCallback (fire_er_up)
{
  char *home_opt;
  mo_window *win;
  char *init_document;
  char *fname=NULL;
  int cnt=0;

/* Pick up default or overridden value out of X resources. */
  home_document = get_pref_string(eHOME_DOCUMENT);

  /* Value of environment variable WWW_HOME overrides that. */
  if ((home_opt = getenv ("WWW_HOME")) != NULL)
    home_document = home_opt;

#ifdef SAM
#ifdef PRERELEASE
  /*
   * If this is a pre-release, go to the help-on-version doc for three
   *   start ups. Then, go to the Pre-Release warning page for three
   *   start ups. Then go to their defined page or the NCSA home page.
   */
  if ((cnt=GetCardCount((fname=MakeFilename())))<=MO_GO_NCSA_COUNT) {
	init_document = strdup (MO_HELP_ON_VERSION_DOCUMENT);
  }
  else if (cnt<=(MO_GO_NCSA_COUNT*2)) {
	init_document = strdup ("http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/NewPrereleaseWarningPage.html");
  }
  else {
	init_document=strdup(home_document);
  }
#else
  /*
   * If this is not a pre-release, go to the help-on-version doc for three
   *   start ups. Then go to their defined page or the NCSA home page.
   */
  if (GetCardCount((fname=MakeFilename()))<=MO_GO_NCSA_COUNT) {
	init_document = strdup (MO_HELP_ON_VERSION_DOCUMENT);
  }
  else {
	init_document=strdup(home_document);
  }
#endif
#else
	init_document=strdup(home_document);
#endif

  if (fname) {
	free(fname);
  }

  /* Value of argv[1], if it exists, sets startup_document.
     (All other command-line flags will have been picked up by
     the X resource mechanism.) */
  /* Unless they are bogus options - then they will break... DXP */
  if (gargc > 1 && gargv[1] && *gargv[1])
    startup_document = mo_url_prepend_protocol(gargv[1]);

  /* Check for proper home document URL construction. */
  if (!strstr (home_document, ":"))
    home_document = mo_url_canonicalize_local (home_document);

  /* Check for proper init document URL construction. */
  if (!strstr (init_document, ":"))
    init_document = mo_url_canonicalize_local (init_document);

/* SWP -- Done in mo_url_prepend_protcol
  if (startup_document && !strstr (startup_document, ":"))
    startup_document = mo_url_canonicalize_local (startup_document);
*/

/* set the geometry values - dxp */

  if(!userSpecifiedGeometry) {
      /* then no -geometry was specified on the command line,
	   so we just use the default values from the resources */
      XmxSetArg (XmNwidth, get_pref_int(eDEFAULT_WIDTH));
      XmxSetArg (XmNheight, get_pref_int(eDEFAULT_HEIGHT));
  }
  else {
      /* the they DID specify a -geometry, so we use that */
      XmxSetArg (XmNwidth, userWidth);
      XmxSetArg (XmNheight, userHeight);

      XmxSetArg (XmNx, userX);
      XmxSetArg (XmNx, userY);
  }

  if (get_pref_boolean(eINITIAL_WINDOW_ICONIC))
    XmxSetArg (XmNiconic, True);

  win = mo_open_window
    (toplevel, startup_document ? startup_document : init_document, NULL);

#if 0
  /* Check the Comment Card */
#ifdef PRERELEASE
  do_comment=0; /* Don't actually display the cc if we aren't in final release */
#endif
  CommentCard(win);
#endif

  XtVaGetValues(win->scrolled_win,
		WbNbodyColors,
		&(win->body_color),
		NULL);

  XtVaGetValues(win->scrolled_win,
		WbNbodyImages,
		&(win->body_images),
		NULL);

  XmxRSetToggleState (win->menubar, mo_body_color,
                      (win->body_color ? XmxSet : XmxNotSet));

  XmxRSetToggleState (win->menubar, mo_body_images,
                      (win->body_images ? XmxSet : XmxNotSet));

  /* set focus policy of HTMLWidget according to preferences */
  HTMLSetFocusPolicy(win->scrolled_win,get_pref_boolean(eFOCUS_FOLLOWS_MOUSE));

  if(get_pref_boolean(eFOCUS_FOLLOWS_MOUSE))
    XtVaSetValues(toplevel, XmNkeyboardFocusPolicy, XmPOINTER, NULL);

  return;
}


/****************************************************************************
 * name:    mo_open_initial_window
 * purpose: This routine is called when we know we want to open the
 *          initial Document View window.
 * inputs:
 *   none
 * returns:
 *   mo_succeed
 * remarks: This routine is simply a stub that sets a timeout that
 *          calls fire_er_up() after 10 milliseconds, which does the
 *          actual work.
 ****************************************************************************/
mo_status mo_open_initial_window (void)
{
  /* Set a timer that will actually cause the window to open. */
  XtAppAddTimeOut (app_context, 10,
                   (XtTimerCallbackProc)fire_er_up, (XtPointer)True);

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_error_handler (PRIVATE)
 * purpose: Handle X errors.
 * inputs:
 *   - Display       *dsp: The X display.
 *   - XErrorEvent *event: The error event to handle.
 * returns:
 *   0, if it doesn't force an exit.
 * remarks:
 *   The main reason for this handler is to keep the application
 *   from crashing on BadAccess errors during calls to XFreeColors().
 ****************************************************************************/
static int mo_error_handler (Display *dsp, XErrorEvent *event)
{
  char buf[128];

  XUngrabPointer (dsp, CurrentTime);   /* in case error occurred in Grab */

  /* BadAlloc errors (on a XCreatePixmap() call)
     and BadAccess errors on XFreeColors are 'ignoreable' errors */
  if (event->error_code == BadAlloc ||
      (event->error_code == BadAccess && event->request_code == 88))
    return 0;
  else
    {
      /* All other errors are 'fatal'. */
      XGetErrorText (dsp, event->error_code, buf, 128);
      fprintf (stderr, "X Error: %s\n", buf);
      fprintf (stderr, "  Major Opcode:  %d\n", event->request_code);

      /* Try to close down gracefully. */
      mo_exit ();
    }
 return 0; /* never makes it here.... */
}


/****************************************************************************
 * name:    setup_imagekill
 * purpose: Read the imagekill file if it exists and fill in the
 *          imagekill_sites array
 *
 * returns:
 *   nothing
 * remarks:
 *
 ****************************************************************************/
#define IMAGESELECT_FILENAME ".mosaic/imageselect-sites" // SAM

void setup_imagekill(void) {

    char *home_ptr, *home;
    struct passwd *pwdent;
    char imageselect_file_pathname[512];
    FILE *fp;
    long i,j, cnt, num_delay_sites=0, num_kill_sites=0;
    char buf[512];


    if (get_home(&home)!=0 || !home) {
		fprintf(stderr,"home: Could not get your home directory.\n");
		return;
	}

    sprintf(imageselect_file_pathname, "%s/%s",
            home, IMAGESELECT_FILENAME);

    free(home);

    /* Check to see if the file exists. If it doesn't, then exit */

    if(!file_exists(imageselect_file_pathname))
        return;

    /* try to open it */
    if(!(fp=fopen(imageselect_file_pathname, "r"))) {

        fprintf(stderr,
                "Error: Can't open .mosaic-imageselect-sites file for reading\n");
        return;
    }

        /* read it */

    while(!(fgets(buf, 512, fp) == NULL)) {
        if(buf[0] == '#' || buf[0] == '\n')
            continue;
        else if(buf[0] == 'd' || buf[0] == 'D')
            num_delay_sites++;
        else if(buf[0] == 'k' || buf[0] == 'K')
            num_kill_sites++;
    }

    rewind(fp);

    imagekill_sites = (char **)malloc((num_kill_sites+1) * sizeof(char *));
    imagedelay_sites = (char **)malloc((num_delay_sites+1) * sizeof(char *));

    if(imagekill_sites == NULL)
        return;
    if(imagedelay_sites == NULL) {
        free(imagekill_sites);
        return;
    }

    i=j=0;

    while(!(fgets(buf, 512, fp) == NULL)) {

        int len;


        if(buf[0] == '#' || buf[0] == '\n')
            continue;

        else if(buf[0] == 'd' || buf[0] == 'D') {
            len = strlen(buf) - 6 - 1; /* 6 == strlen("delay ") */
            imagedelay_sites[i] = NULL;
            imagedelay_sites[i] = (char *)malloc((len+1) * sizeof(char));
            strncpy(imagedelay_sites[i], buf+6, len);
            imagedelay_sites[i][len] = '\0';
            i++;
        }

        else if(buf[0] == 'k' || buf[0] == 'K') {

            len = strlen(buf) - 5 - 1; /* 5 == strlen("kill ") */
            imagekill_sites[j] = NULL;
            imagekill_sites[j] = (char *)malloc((len+1) * sizeof(char));
            strncpy(imagekill_sites[j], buf+5, len);
            imagekill_sites[j][len] = '\0';
            j++;
        }
    }

    imagedelay_sites[i] = NULL;
    imagekill_sites[j] = NULL;


    fclose(fp);

    return;

}



/* exported from HTTP.c */
void HT_SetExtraHeaders(char **headers);

/****************************************************************************
 * name:    mo_do_gui
 * purpose: This is basically the real main routine of the application.
 * inputs:
 *   - int    argc: Number of arguments.
 *   - char **argv: The argument vector.
 * returns:
 *   nothing
 * remarks:
 *
 ****************************************************************************/
void mo_do_gui (int argc, char **argv)
{
#ifdef MONO_DEFAULT
    int use_color = 0;
#else
    int use_color = 1;
#endif
    int no_defaults = 0;
    int color_set = 0;
    char* display_name = getenv("DISPLAY");
    Display* dpy;
    XrmDatabase intDB,appDB;
    Widget intWidget;
    int i;

        /* for prefs - DXP */
    Boolean successful;
    prefsStructP thePrefsStructP;

        /* Loop through the args before passing them off to
     XtAppInitialize() in case we need to catch something first. */
    for (i = 1; i < argc; i++)
    {
        if (!strcmp (argv[i], "-mono"))
        {
            use_color = 0;
            color_set = 1;
	    continue;
        }
        if (!strcmp (argv[i], "-color"))
        {
            use_color = 1;
            color_set = 1;
	    continue;
        }
        if (!strcmp (argv[i], "-nd"))
        {
            no_defaults = 1;
	    continue;
        }
        if (!strcmp (argv[i], "-display"))
        {
            display_name = argv[i + 1];
            i++;
	    continue;
        }
        if(!strcmp(argv[i], "-geometry")) {
            userSpecifiedGeometry = 1;
	    continue;
	}
	if (!strcmp(argv[i],"-install")) {
		installed_colormap=1;
		continue;
	}
	if (!strcmp(argv[i],"-iconic")) {
		splash_cc=0;
		continue;
	}
    }


        /* Motif setup. */
    XmxStartup ();
    XmxSetArg (XmNwidth,1);
    XmxSetArg (XmNheight,1);
    XmxSetArg (XmNmappedWhenManaged, False);
    /*
     * Awful expensive to open and close the display just to find
     * rhe depth information.
     */
    if ((dpy=XOpenDisplay(display_name))!=NULL) {
	if (!color_set) {
		use_color = DisplayPlanes(dpy, DefaultScreen(dpy)) > 1;
	}
	XCloseDisplay(dpy);
    }
    else {
	fprintf(stderr,"Couldn't open display: %s\n",(!display_name?"(NULL)":display_name));
    }

    if (no_defaults)
    {
        toplevel = XtAppInitialize
            (&app_context, "Mosaic", options, XtNumber (options),
             &argc, argv, NULL, Xmx_wargs, Xmx_n);
    }
    else
    {
        if (use_color)
        {
            toplevel = XtAppInitialize
                (&app_context, "Mosaic", options, XtNumber (options),
                 &argc, argv, color_resources, Xmx_wargs, Xmx_n);
        }
        else
        {
            toplevel = XtAppInitialize
                (&app_context, "Mosaic", options, XtNumber (options),
                 &argc, argv, mono_resources, Xmx_wargs, Xmx_n);
        }
    }

    Xmx_n=0;

    dsp = XtDisplay (toplevel);

        /* initialize the preferences stuff */
    successful = preferences_genesis();
    if(!successful) { /* I should probably be generating an error here... */
        signal (SIGBUS, 0);
        signal (SIGSEGV, 0);
        signal (SIGILL, 0);
        abort ();
    }

    thePrefsStructP = get_ptr_to_preferences();

      /* First for the regular resources */
    XtVaGetApplicationResources(
        toplevel,
        (XtPointer)thePrefsStructP->RdataP,
        resources,
        XtNumber (resources), NULL);

/*
  appDB=XrmGetDatabase(dsp);

  if (Rdata.internationalFilename!=NULL) {
	if ((intDB=XrmGetFileDatabase(Rdata.internationalFilename))!=NULL) {
		XrmMergeDatabases(intDB,&appDB);
		XrmSetDatabase(dsp,appDB);
	}
	else {
		fprintf(stderr,"There was no language file called:\n  [%s]\n",Rdata.internationalFilename);
	}
  }

  intWidget=XtVaCreateWidget("international",xmRowColumnWidgetClass,toplevel,
			     NULL);
  XtVaGetApplicationResources(intWidget, (XtPointer)&Idata, intResources,
			      XtNumber (intResources), NULL);
*/

        /* read the preferences file now */
    successful = read_preferences_file(NULL);
    if(!successful) {
        signal (SIGBUS, 0);
        signal (SIGSEGV, 0);
        signal (SIGILL, 0);
        abort ();
    }

    if (get_pref_boolean(eINSTALL_COLORMAP)) {
	installed_colormap=1;
    }

    if (installed_colormap) {
	XColor bcolr;

	installed_cmap=XCreateColormap(dsp,
				       RootWindow(dsp,DefaultScreen(dsp)),
				       DefaultVisual(dsp,DefaultScreen(dsp)),
				       AllocNone);

	XtVaGetValues(toplevel,
		      XtNbackground,
		      &(bcolr.pixel),
		      NULL);
	XQueryColor(dsp,
		    DefaultColormap(dsp,
				    DefaultScreen(dsp)),
		    &bcolr);

	XtVaSetValues(toplevel,
		      XmNcolormap, installed_cmap,
		      NULL);

        XAllocColor(dsp,
		    installed_cmap,
		    &bcolr);
	XtVaSetValues(toplevel,
		      XmNbackground,
		      bcolr.pixel,
		      NULL);
    }

  /* Needed for picread.c, right now. */
    {
        XVisualInfo vinfo, *vptr;
        int cnt;

        vinfo.visualid =
            XVisualIDFromVisual
            (DefaultVisual (dsp,
                            DefaultScreen (dsp)));
        vptr = XGetVisualInfo (dsp, VisualIDMask, &vinfo, &cnt);
        Vclass = vptr->class;
        XFree((char *)vptr);
    }

        /* First get the hostname. */
    machine = (char *)malloc (sizeof (char) * 64);
    gethostname (machine, 64);

    uname(&mo_uname);
    HTAppVersion =
        (char *)malloc (sizeof(char) * (
            strlen(MO_VERSION_STRING) +
            strlen(mo_uname.sysname) +
            strlen(mo_uname.release) +
            strlen(mo_uname.machine) + 20));
    sprintf(HTAppVersion, "%s (X11;%s %s %s)",
            MO_VERSION_STRING,
            mo_uname.sysname,
            mo_uname.release,
            mo_uname.machine);

    XSetErrorHandler (mo_error_handler);

    /* Transient shell cannot be focussed, so no point in splash screen as it
       will be complete psychadelic */
    if (installed_colormap || !splash_cc) {
	set_pref_boolean(eSPLASHSCREEN,False);
    }

splash_goto:

    if (get_pref_boolean(eSPLASHSCREEN)) {
        Pixmap splashpix;
        GC gc;
        XGCValues values;
        XColor ccell1, ccell_fg;

        int x,y;
        XWindowAttributes war;
        Widget sform, spixwid;
        XFontStruct *font;
        char s[64];

        int l;
        int fontW, fontH;

        if (!XGetWindowAttributes(dsp,DefaultRootWindow(dsp),&war)) {
            fprintf(stderr,"Warning: Could not obtain your root window attributes.\n  Splash screen will not be centered.\n");
            x=y=100;
        }
        else {
            x=(war.width/2)-(320/2);
            y=(war.height/2)-(320/2);
        }

            /* GO GO MOSAIC SPLASH SCREEN - WHOOMP! */
        if (!(font =
              XLoadQueryFont(dsp,
                             "-adobe-helvetica-medium-o-normal-*-*-180-*-*-p-*-iso8859-*"))) {
		fprintf(stderr,"Warning: Cannot Get Font -adobe-helvetica-medium-o-normal-*-*-180-*-*-p-*-iso8859-*");
		fprintf(stderr,"Warning: Splash Screen has been aborted.\n  Reason: Could not load version font.\n");
		set_pref_boolean(eSPLASHSCREEN,False);
		goto splash_goto;
	}

        fontW = font->max_bounds.rbearing;
        fontH = font->max_bounds.ascent + font->max_bounds.descent;

        splash = XtVaCreatePopupShell
            ("Hello, World!",
             xmMenuShellWidgetClass,
             toplevel,
             XmNheight, 320,
             XmNwidth, 320,
             XmNx, x,
             XmNy, y,
             XmNallowShellResize, FALSE,
             NULL);

        splash_cc=180;
        splashpix = LoadSplashXPM(splash,&splash_cc);
	if (!splash_cc) {
		XtDestroyWidget(splash);
		set_pref_boolean(eSPLASHSCREEN,False);
		goto splash_goto;
	}

        sprintf(s,"version %s",MO_VERSION_STRING);

        l = strlen(s);

        sform = XtVaCreateManagedWidget("sform", xmRowColumnWidgetClass,
                                        splash,
                                        XmNheight, 320,
                                        XmNwidth, 320,
                                        XmNx, x,
                                        XmNy, y,
                                        NULL);

        XtPopup(splash, XtGrabNone);

        ccell_fg.flags = DoRed | DoGreen | DoBlue;
        ccell_fg.red = 0xF9F9;
        ccell_fg.blue = 0x0404;
        ccell_fg.green = 0x0404;

          /* we use red so we don't bother freeing it */
        if(!XAllocColor(dsp,(installed_colormap ?
			     installed_cmap :
			     DefaultColormapOfScreen(XtScreen(splash))),
                        &ccell_fg))
            ccell_fg.pixel = WhitePixelOfScreen(XtScreen(splash));


        gc = XtGetGC( splash,0,NULL);
        values.font = font->fid;
        values.foreground = ccell_fg.pixel;

        XChangeGC(dsp, gc, GCFont | GCForeground, &values);

        XDrawString(dsp, splashpix, gc,
                    320-(fontW*l/2),
                    320-fontH/2,
                    s, l);


        spixwid = XtVaCreateManagedWidget(" ", xmLabelWidgetClass,
                                          sform,
                                          XmNlabelType, XmPIXMAP,
                                          XmNlabelPixmap, splashpix,
                                          XmNalignment, XmALIGNMENT_CENTER,
                                          XmNx, x,
                                          XmNy, y,
                                          NULL);

        XFlush (dsp);
        XmUpdateDisplay (splash);
        XFlush (dsp);
        XSync (dsp, False);
        XtReleaseGC(splash,gc);
    }

    XtAppAddActions(app_context, balloon_action, 2);
    XtAppAddActions(app_context, toplevel_actions, 1);
    XtAppAddActions(app_context, url_actions, 2);

    mo_init_menubar();

#ifdef __sgi
        /* Turn on debugging malloc if necessary. */
    if (get_pref_boolean(eDEBUGGING_MALLOC))
        mallopt (M_DEBUG, 1);
#endif

    if(get_pref_string(eACCEPT_LANGUAGE_STR)) {
        char **extras;

        extras = malloc(sizeof(char *) * 2);

        extras[0] = malloc(strlen(get_pref_string(eACCEPT_LANGUAGE_STR))+19);
        sprintf(extras[0],
                "Accept-Language: %s",
                get_pref_string(eACCEPT_LANGUAGE_STR));
        extras[1] = NULL;

        HT_SetExtraHeaders(extras);
    }

    global_xterm_str = get_pref_string(eXTERM_COMMAND);

    uncompress_program = get_pref_string(eUNCOMPRESS_COMMAND);
    gunzip_program = get_pref_string(eGUNZIP_COMMAND);

    tweak_gopher_types = get_pref_boolean(eTWEAK_GOPHER_TYPES);
    max_wais_responses = get_pref_int(eMAX_WAIS_RESPONSES);
    ftp_timeout_val = get_pref_int(eFTP_TIMEOUT_VAL);
    ftpRedial=get_pref_int(eFTP_REDIAL);
    ftpRedialSleep=get_pref_int(eFTP_REDIAL_SLEEP);
    ftpFilenameLength=get_pref_int(eFTP_FILENAME_LENGTH);
    ftpEllipsisLength=get_pref_int(eFTP_ELLIPSIS_LENGTH);
    ftpEllipsisMode=get_pref_int(eFTP_ELLIPSIS_MODE);

    sendReferer=get_pref_boolean(eSEND_REFERER);
    sendAgent=get_pref_boolean(eSEND_AGENT);

#ifndef DISABLE_TRACE
    httpTrace=get_pref_boolean(eHTTPTRACE);
    www2Trace=get_pref_boolean(eWWW2TRACE);
    htmlwTrace=get_pref_boolean(eHTMLWTRACE);
    cciTrace=get_pref_boolean(eCCITRACE);
    srcTrace=get_pref_boolean(eSRCTRACE);
    cacheTrace=get_pref_boolean(eCACHETRACE);
    nutTrace=get_pref_boolean(eNUTTRACE);
#else
    if (get_pref_boolean(eHTTPTRACE) ||
        get_pref_boolean(eWWW2TRACE) ||
        get_pref_boolean(eHTMLWTRACE) ||
        get_pref_boolean(eCCITRACE) ||
        get_pref_boolean(eSRCTRACE) ||
        get_pref_boolean(eCACHETRACE) ||
        get_pref_boolean(eNUTTRACE)) {
        fprintf(stderr,"Tracing has been compiled out of this binary.\n");
    }
#endif

    useAFS = get_pref_boolean(eUSEAFSKLOG);

    proxy_list = ReadProxies(get_pref_string(ePROXY_SPECFILE));
    noproxy_list = ReadNoProxies(get_pref_string(eNOPROXY_SPECFILE));

    use_default_extension_map = get_pref_boolean(eUSE_DEFAULT_EXTENSION_MAP);
    global_extension_map = get_pref_string(eGLOBAL_EXTENSION_MAP);

    if (get_pref_string(ePERSONAL_EXTENSION_MAP))
    {
        char *home = getenv ("HOME");

        if (!home)
            home = "/tmp";

        personal_extension_map = (char *)malloc
            (strlen (home) +
             strlen (get_pref_string(ePERSONAL_EXTENSION_MAP)) +
             8);
        sprintf (personal_extension_map, "%s/%s", home,
                 get_pref_string(ePERSONAL_EXTENSION_MAP));
    }
    else
        personal_extension_map = "\0";

    use_default_type_map = get_pref_boolean(eUSE_DEFAULT_TYPE_MAP);
    global_type_map = get_pref_string(eGLOBAL_TYPE_MAP);
    if (get_pref_string(ePERSONAL_TYPE_MAP))
    {
        char *home = getenv ("HOME");

        if (!home)
            home = "/tmp";

        personal_type_map = (char *)malloc
            (strlen (home) +
             strlen (get_pref_string(ePERSONAL_TYPE_MAP)) +
             8);
        sprintf (personal_type_map, "%s/%s", home,
                 get_pref_string(ePERSONAL_TYPE_MAP));
    }
    else
        personal_type_map = "\0";

#ifdef HAVE_HDF
    have_hdf = 1;
#else
    have_hdf = 0;
#endif

    twirl_increment = get_pref_int(eTWIRL_INCREMENT);

  /* Then make a copy of the hostname for shortmachine.
     Don't even ask. */
    shortmachine = strdup (machine);

        /* Then find out the full name, if possible. */
    if (get_pref_string(eFULL_HOSTNAME))
    {
        free (machine);
        machine = get_pref_string(eFULL_HOSTNAME);
    }
    else if (!get_pref_boolean(eGETHOSTBYNAME_IS_EVIL))
    {
        struct hostent *phe;

        phe = gethostbyname (machine);
        if (phe && phe->h_name)
        {
            free (machine);
            machine = strdup (phe->h_name);
        }
    }
        /* (Otherwise machine just remains whatever gethostname returned.) */

    machine_with_domain = (strlen (machine) > strlen (shortmachine) ?
                           machine : shortmachine);

    {/* Author Name & Email init.  - bjs */
        struct passwd *pw = getpwuid (getuid ());
        char *cc;
        char *default_author_name = get_pref_string(eDEFAULT_AUTHOR_NAME);
        char *default_author_email = get_pref_string(eDEFAULT_AUTHOR_EMAIL);

        if(!default_author_name) {
	    if (!pw || !pw->pw_gecos) {
		default_author_name = strdup("Unknown");
	    }
	    else {
		default_author_name = strdup(pw->pw_gecos);
		strcpy(default_author_name,pw->pw_gecos);
		for(cc = default_author_name;*cc;cc++)
			if(*cc==',') {
				*cc=0;
				break;
			}
		}
	}
        if(!default_author_email) {
	    if (!pw || !pw->pw_name) {
		default_author_email =
			(char *) malloc(strlen("UNKNOWN")+strlen(machine)+2);
		sprintf(default_author_email,"UNKNOWN@%s",machine);
	    }
	    else {
		default_author_email =
			(char *) malloc(strlen(pw->pw_name)+strlen(machine)+2);
		sprintf(default_author_email,"%s@%s",pw->pw_name,machine);
	    }
        }
        set_pref(eDEFAULT_AUTHOR_NAME, (void *)default_author_name);
        set_pref(eDEFAULT_AUTHOR_EMAIL, (void *)default_author_email);
    }

        /* If there's no tmp directory assigned by the X resource, then
     look at TMPDIR. */
    {
        char *tmp_dir = get_pref_string(eTMP_DIRECTORY);

        if (!tmp_dir)
        {
            tmp_dir = getenv ("TMPDIR");
                /* It can still be NULL when we leave here -- then we'll just
                   let tmpnam() do what it does best. */
            set_pref(eTMP_DIRECTORY, (void *)tmp_dir);
        }
    }

        /* If there's no docs directory assigned by the X resource,
     then look at MOSAIC_DOCS_DIRECTORY environment variable
     and then at hardcoded default. */
    {
        char *docs_dir = get_pref_string(eDOCS_DIRECTORY);

        if (!docs_dir)
        {
            docs_dir = getenv ("MOSAIC_DOCS_DIRECTORY");
            if (!docs_dir)
                docs_dir = DOCS_DIRECTORY_DEFAULT;
            if (!docs_dir || !*(docs_dir))
            {
                fprintf (stderr, "fatal error: nonexistent docs directory\n");
                exit (-1);
            }
            set_pref(eDOCS_DIRECTORY, (void *)docs_dir);
        }
    }

  if (get_pref_int(eCOLORS_PER_INLINED_IMAGE)>256) {
	fprintf(stderr,"WARNING: Colors per inline image specification > 256.\n  Auto-Setting to 256.\n");
	set_pref_int(eCOLORS_PER_INLINED_IMAGE,256);
  }

  if (get_pref_boolean(eUSE_GLOBAL_HISTORY))
    mo_setup_global_history ();
  else
    mo_init_global_history ();

  mo_setup_default_hotlist ();
  mo_write_default_hotlist (); /* amb */
  mo_setup_pan_list ();

  if(get_pref_boolean(eHOTLIST_ON_RBM))
    mo_init_hotmenu();

  /* Write pid into "~/.mosaicpid". */
  {
    char *home = getenv ("HOME"), *fnam;
    FILE *fp;

        if (!home)
            home = "/tmp";

        fnam = (char *)malloc (strlen (home) + 32);
        sprintf (fnam, "%s/.mosaic/mosaicpid", home); // SAM

        fp = fopen (fnam, "w");
        if (fp)
        {
            fprintf (fp, "%d\n", getpid());
            fclose (fp);
        }

        free (fnam);
    }

    busy_cursor = XCreateFontCursor (dsp, XC_watch);

    XtRealizeWidget (toplevel);

        /* get the current geometry values */
    XtVaGetValues(toplevel,
                  XmNwidth, &userWidth,
                  XmNheight, &userHeight,
                  XmNx, &userX,
                  XmNy, &userY,
                  NULL);


    gargv = argv;
    gargc = argc;

#ifndef VMS
    signal (SIGUSR1, (void *)ProcessExternalDirective);
#endif

    if(get_pref_boolean(eSPLASHSCREEN) && splash) {
      /*Wait 3 secs, then popdown*/
      if(splash_cc) {
          splashTimer =
              XtAppAddTimeOut(app_context, 3000,
                              (XtTimerCallbackProc)kill_splash, NULL);
      } else {
          kill_splash();
      }
    }

    createBusyCursors(toplevel);
    MakePixmaps(toplevel);
    logo_save = get_pref_int(ePIX_COUNT);
    logo_count = 0;

    setup_imagekill();

    mo_open_initial_window ();

#ifndef DISABLE_TRACE
    if (srcTrace) {
        fprintf(stderr,"cciPort resourced to %d\n",get_pref_int(eCCIPORT));
    }
#endif

    if ((get_pref_int(eCCIPORT) > 1023 ) &&  (get_pref_int(eCCIPORT) < 65536))
    {
        MoCCIStartListening(toplevel,get_pref_int(eCCIPORT));
    }

    XtAppMainLoop (app_context);
}


/****************************************************************************
 * name:    mo_process_external_directive
 * purpose: Handle an external directive given to the application via
 *          a config file read in response to a SIGUSR1.
 * inputs:
 *   - char *directive: The directive; either "goto" or "newwin".
 *   - char       *url: The URL corresponding to the directive.
 * returns:
 *   nothing
 * remarks:
 *
 ****************************************************************************/
#define CLIP_TRAILING_NEWLINE(url) \
  if (url[strlen (url) - 1] == '\n') \
    url[strlen (url) - 1] = '\0';

static XEvent *mo_manufacture_dummy_event (Widget foo)
{
  /* This is fucking hilarious. */
  XAnyEvent *a = (XAnyEvent *)malloc (sizeof (XAnyEvent));
  a->type = 1; /* HAHA! */
  a->serial = 1; /* HAHA AGAIN! */
  a->send_event = False;
  a->display = XtDisplay (foo);
  a->window = XtWindow (foo);
  return (XEvent *)a;
}

void mo_process_external_directive (char *directive, char *url)
{
  /* Process a directive that we received externally. */
  mo_window *win = current_win;

  /* Make sure we have a window. */
  if (!win)
    win = mo_next_window (NULL);

  if (!strncmp (directive, "goto", 4))
    {
      CLIP_TRAILING_NEWLINE(url);

      mo_access_document (win, url);

      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "newwin", 6))
    {
      CLIP_TRAILING_NEWLINE(url);

      /* Force a new window to open. */
      mo_open_another_window (win, url, NULL, NULL);

      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "pagedown", 8))
    {
      Widget sb;
      String params[1];

      params[0] = "0";

      XtVaGetValues (win->scrolled_win, XmNverticalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb))
        {
          XEvent *event = mo_manufacture_dummy_event (sb);
          XtCallActionProc (sb, "PageDownOrRight", event, params, 1);
        }

      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "pageup", 6))
    {
      Widget sb;
      String params[1];

      params[0] = "0";

      XtVaGetValues (win->scrolled_win, XmNverticalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb))
        {
          XEvent *event = mo_manufacture_dummy_event (sb);
          XtCallActionProc (sb, "PageUpOrLeft", event, params, 1);
        }
      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "scrolldown", 9))
    {
      Widget sb;
      String params[1];

      params[0] = "0";

      XtVaGetValues (win->scrolled_win, XmNverticalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb))
        {
          XEvent *event = mo_manufacture_dummy_event (sb);
          XtCallActionProc (sb, "IncrementDownOrRight", event, params, 1);
        }
      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "scrollup", 7))
    {
      Widget sb;
      String params[1];

      params[0] = "0";

      XtVaGetValues (win->scrolled_win, XmNverticalScrollBar,
                     (long)(&sb), NULL);
      if (sb && XtIsManaged (sb))
        {
          XEvent *event = mo_manufacture_dummy_event (sb);
          XtCallActionProc (sb, "IncrementUpOrLeft", event, params, 1);
        }
      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "flushimagecache", 15))
    {
      mo_flush_image_cache (win);
    }
  else if (!strncmp (directive, "backnode", 8))
    {
      mo_back_node (win);
      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "forwardnode", 11))
    {
      mo_forward_node (win);
      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "reloaddocument", 14))
    {
      mo_reload_window_text (win, 0);
      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "reloadimages", 12))
    {
      mo_reload_window_text (win, 1);
      XmUpdateDisplay (win->base);
    }
  else if (!strncmp (directive, "refresh", 7))
    {
      mo_refresh_window_text (win);
      XmUpdateDisplay (win->base);
    }

  return;
}


void set_current_win(Widget w, XEvent *event,
	       String *params, Cardinal *num_params)
{
  Widget toplevel = w;
  mo_window *ptr = winlist;
  int i;

  while(!XtIsTopLevelShell(toplevel))
    toplevel = XtParent(toplevel);

  for(i=0;(ptr != NULL) && (i<wincount);i++)
    {
      if(ptr->base == toplevel)
	{
	  if(event->xany.type == EnterNotify)
	    {
	      current_win = ptr;
	      ptr->have_focus = True;
	    }
	  else if(event->xany.type == LeaveNotify)
	    ptr->have_focus = False;
	  break;
	}
      else
	ptr = ptr->next;
    }
  if(!ptr)
	fprintf(stderr, "Couldn't find current window. Mosaic will be crashing soon.\n");
}

void set_focus_to_view(Widget w, XEvent *event,
	       String *params, Cardinal *num_params)
{
  XtSetKeyboardFocus(current_win->base, current_win->view);
}

void take_focus(Widget w, XEvent *event,
	       String *params, Cardinal *num_params)
{
  XtSetKeyboardFocus(current_win->base,w);
}


void mo_flush_passwd_cache (mo_window *win)
{

  HTFTPClearCache ();
  HTAAServer_clear ();
  HTProgress ("Password cache flushed");
}
