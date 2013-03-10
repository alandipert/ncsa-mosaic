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

#ifndef __MOSAIC_H__
#define __MOSAIC_H__

/* --------------------------- SYSTEM INCLUDES ---------------------------- */

#ifdef __sgi
#ifndef _SVR4_SOURCE
#define _SVR4_SOURCE
#include <stdio.h>
#undef _SVR4_SOURCE
#else
#include <stdio.h>
#endif
#else
#include <stdio.h>
#endif

#ifdef __sgi
#ifndef _SVR4_SOURCE
#define _SVR4_SOURCE
#include <string.h>
#undef _SVR4_SOURCE
#else
#include <string.h>
#endif
#else
#include <string.h>
#endif

#include <ctype.h>
#if !defined(VMS) && !defined(NeXT)
#include <unistd.h>
#endif
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

#ifdef __sgi
#ifndef _SGI_SOURCE
#define _SGI_SOURCE
#include <malloc.h>
#undef _SGI_SOURCE
#else
#include <malloc.h>
#endif
#endif

#include "../libXmx/Xmx.h"
#include "toolbar.h"

typedef enum
{
  mo_plaintext = 0, mo_formatted_text, mo_html, mo_latex, mo_postscript,
  mo_mif
} mo_format_token;

/* ------------------------------------------------------------------------ */
/* -------------------------------- ICONS --------------------------------- */
/* ------------------------------------------------------------------------ */
#define NUMBER_OF_FRAMES	25
#define ANIMATION_PIXMAPS	0
#define SECURITY_PIXMAPS	1
#define DIALOG_PIXMAPS		2

/* ------------------------------------------------------------------------ */
/* -------------------------------- MACROS -------------------------------- */
/* ------------------------------------------------------------------------ */

#define MO_VERSION_STRING "2.7b6" // SAM
#define MO_GO_NCSA_COUNT 3  /* Go to the NCSA home page thrice*/
#define MO_HELP_ON_VERSION_DOCUMENT \
  mo_assemble_help_url ("help-on-version-2.7b5.html")
#define MO_DEVELOPER_ADDRESS "mosaic-x@ncsa.uiuc.edu"

#ifndef DOCS_DIRECTORY_DEFAULT
#define DOCS_DIRECTORY_DEFAULT \
  "http://www.ncsa.uiuc.edu/SDG/Software/XMosaic"
#endif

#ifndef HOME_PAGE_DEFAULT
/* This must be a straight string as it is included into a struct;
   no tricks. */
#define HOME_PAGE_DEFAULT \
  "http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/NCSAMosaicHome.html"
#endif /* not HOME_PAGE_DEFAULT */

#ifndef WHATSNEW_PAGE_DEFAULT
#define WHATSNEW_PAGE_DEFAULT \
  "http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/Docs/whats-new.html"
#endif /* not WHATSNEW_PAGE_DEFAULT */

#ifndef DEMO_PAGE_DEFAULT
#define DEMO_PAGE_DEFAULT \
  "http://www.ncsa.uiuc.edu/demoweb/demo.html"
#endif /* not DEMO_PAGE_DEFAULT */

#ifndef HTMLPRIMER_PAGE_DEFAULT
#define HTMLPRIMER_PAGE_DEFAULT \
  "http://www.ncsa.uiuc.edu/General/Internet/WWW/HTMLPrimer.html"
#endif /* not HTMLPRIMER_PAGE_DEFAULT */

#ifndef URLPRIMER_PAGE_DEFAULT
#define URLPRIMER_PAGE_DEFAULT \
  "http://www.ncsa.uiuc.edu/demoweb/url-primer.html"
#endif /* not URLPRIMER_PAGE_DEFAULT */

#ifndef NETWORK_STARTING_POINTS_DEFAULT
#define NETWORK_STARTING_POINTS_DEFAULT \
  "http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/StartingPoints/NetworkStartingPoints.html"
#endif /* not NETWORK_STARTING_POINTS_DEFAULT */

#ifndef INTERNET_METAINDEX_DEFAULT
#define INTERNET_METAINDEX_DEFAULT \
  "http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/MetaIndex.html"
#endif /* not INTERNET_METAINDEX_DEFAULT */

#ifndef DOCUMENTS_MENU_SPECFILE
#define DOCUMENTS_MENU_SPECFILE \
  "/usr/local/lib/mosaic/documents.menu"
#endif /* not DOCUMENTS_MENU_SPECFILE */

#ifndef GLOBAL_EXTENSION_MAP
#define GLOBAL_EXTENSION_MAP "/usr/local/lib/mosaic/mime.types"
#endif
#ifndef GLOBAL_TYPE_MAP
#define GLOBAL_TYPE_MAP "/usr/local/lib/mosaic/mailcap"
#endif


#if defined(bsdi)
#define MO_MACHINE_TYPE "BSD/OS"
#endif
#if defined(__hpux)
#define MO_MACHINE_TYPE "HP-UX"
#endif
#if defined(__sgi)
#define MO_MACHINE_TYPE "Silicon Graphics"
#endif
#if defined(ultrix)
#define MO_MACHINE_TYPE "DEC Ultrix"
#endif
#if defined(linux)
#define MO_MACHINE_TYPE "Linux"
#endif
#if defined(_IBMR2)
#define MO_MACHINE_TYPE "RS/6000 AIX"
#endif
#if defined(sun) && !defined(SOLARIS)
#define MO_MACHINE_TYPE "Sun"
#else
#if defined(SOLARIS)
#define MO_MACHINE_TYPE "SOLARIS"
#endif
#endif
#if defined(__alpha)
#define MO_MACHINE_TYPE "DEC Alpha"
#endif
#if defined(NEXT)
#define MO_MACHINE_TYPE "NeXT BSD"
#endif
#if defined(cray)
#define MO_MACHINE_TYPE "Cray"
#endif
#if defined(VMS)
#define MO_MACHINE_TYPE "VMS"
#endif
#if defined(NeXT)
#define MO_MACHINE_TYPE "NeXT"
#endif
#if defined (SCO)
#if defined (_SCO_DS)
#define MO_MACHINE_TYPE "SCO OpenServer 5"
#else /* _SCO_DS */
#define MO_MACHINE_TYPE "SCO Unix"
#endif /* _SCO_DS */
#endif /* SCO */

#ifndef MO_MACHINE_TYPE
#define MO_MACHINE_TYPE "Unknown Platform"
#endif

#ifdef __hpux
#define HAVE_AUDIO_ANNOTATIONS
#else
#if defined(__sgi) || defined(sun)
#define HAVE_AUDIO_ANNOTATIONS
#endif /* if */
#endif /* ifdef */

/* Be safe... some URL's get very long. */
#define MO_LINE_LENGTH 2048

#define MO_MAX(x,y) ((x) > (y) ? (x) : (y))
#define MO_MIN(x,y) ((x) > (y) ? (y) : (x))

/* Use builtin strdup when appropriate -- code duplicated in tcp.h. */
#if defined(ultrix) || defined(VMS) || defined(NeXT)
extern char *strdup ();
#endif

#define public
#define private static

/*String #defs for Print/Mail/Save*/
#ifndef MODE_HTML
#define MODE_HTML "html"
#endif

#ifndef MODE_POSTSCRIPT
#define MODE_POSTSCRIPT "postscript"
#endif

#ifndef MODE_FORMATTED
#define MODE_FORMATTED "formatted"
#endif

#ifndef MODE_PLAIN
#define MODE_PLAIN "plain"
#endif


/* ------------------------------------------------------------------------ */
/* ------------------------------ MAIN TYPES ------------------------------ */
/* ------------------------------------------------------------------------ */

/* ------------------------------ mo_window ------------------------------- */

#define moMODE_PLAIN  0x0001
#define moMODE_FTP    0x0002
#define moMODE_NEWS   0x0004
#define moMODE_ALL    0x0007


/* mo_window contains everything related to a single Document View
   window, including subwindow details. */
typedef struct mo_window
{
  int id;
  Widget base;
    int mode;


  /* Subwindows. */
  Widget source_win;
  Widget save_win;
  Widget upload_win;
  Widget savebinary_win;  /* for binary transfer mode */
  Widget open_win;
  Widget mail_fsb_win;
  Widget mail_win;
  Widget mailhot_win;
  Widget edithot_win;
  Widget inserthot_win;
  Widget mailhist_win;
  Widget print_win;
  Widget history_win;
  Widget open_local_win;
  Widget hotlist_win;
  Widget techsupport_win;
  Widget news_win;           /* News Post/Followup*/
  Widget news_fsb_win;
  Widget news_sub_win;       /* News Subscribe Window */
  Widget annotate_win;
  Widget src_search_win;         /* source window document search */
  Widget search_win;         /* internal document search */
  Widget searchindex_win;    /* network index search */
  Widget cci_win;	     /* common client interface control window */
  Widget mailto_win;
  Widget mailto_form_win;
    Widget links_win;     /* window with list of links */
    Widget links_list; /* widget holding the list itself */
    XmString *links_items;
    int links_count;

  Widget ftpput_win, ftpremove_win, ftpremove_text, ftpmkdir_win, ftpmkdir_text;
  char *ftp_site;

  Widget session_menu;
  Widget *session_items;
  int num_session_items;

  /* Tag 'n Bag
  Widget tag_win;
  Widget tag_list;
  */


#ifdef HAVE_DTM
  Widget dtmout_win;
#endif
#ifdef HAVE_AUDIO_ANNOTATIONS
  Widget audio_annotate_win;
#endif

        /* USER INTERFACE BITS 'n PIECES */
    struct toolbar tools[BTN_COUNT];

    Widget slab[7];
    int slabpart[8];
    int slabcount,biglogo,smalllogo,texttools;

    XmxMenuRecord *menubar;

    Widget url_text;
    Widget title_text;
    Widget scrolled_win, view;
    Widget rightform;
    Widget tracker_label, logo, security;
    Widget button_rc, button2_rc, encrypt;
    Widget toolbarwin, topform;
    int toolset;
    int toolbardetached;
    int toolbarorientation;


    Widget meter, meter_frame;
    int meter_level,meter_width,meter_height;
    int meter_notext;
    Pixel meter_fg, meter_bg, meter_font_fg, meter_font_bg;
    int meter_fontW, meter_fontH;
    char *meter_text;
    XFontStruct *meter_font;

  Widget searchindex_button;   /* pushbutton, says "Search Index" */
  Widget searchindex_win_label, searchindex_win_text;
  Widget searchindex_win_searchbut;


  Widget home_button;

  int last_width;

  struct mo_node *history;
  struct mo_node *current_node;
  int reloading;

  char *target_anchor;

  /* Document source window. */
  Widget source_text;
  Widget source_url_text;
  Widget source_date_text;
  XmxMenuRecord *format_optmenu;
  int save_format; /* starts at 0 */

  Widget open_text;

  Widget mail_to_text;
  Widget mail_subj_text;
  XmxMenuRecord *mail_fmtmenu;
  int mail_format;

  Widget mailhot_to_text;
  Widget mailhot_subj_text;
  Widget mailhist_to_text;
  Widget mailhist_subj_text;

  Widget print_text;
  XmxMenuRecord *print_fmtmenu;
  int print_format;
  /*swp*/
  Widget hotlist_rbm_toggle;
  Widget print_header_toggle_save;
  Widget print_header_toggle_print;
  Widget print_header_toggle_mail;
  Widget print_footer_toggle_save;
  Widget print_footer_toggle_print;
  Widget print_footer_toggle_mail;
  Widget print_a4_toggle_save;
  Widget print_a4_toggle_print;
  Widget print_a4_toggle_mail;
  Widget print_us_toggle_save;
  Widget print_us_toggle_print;
  Widget print_us_toggle_mail;
    Widget print_url_only;
    Widget print_doc_only;

  Widget history_list;

  Widget hotlist_list;
  Widget hotlist_label;
  Widget save_hotlist_win;
  Widget load_hotlist_win;
  struct mo_hotlist *current_hotlist;
  union mo_hot_item *hot_cut_buffer;

  Widget techsupport_text;

  Widget news_text;
  Widget news_text_from, news_text_subj, news_text_group;
    /* news followup storage */
  char *newsfollow_artid;
  char *newsfollow_grp, *newsfollow_subj, *newsfollow_ref, *newsfollow_from;

  Widget mailto_text;
  Widget mailto_fromfield;
  Widget mailto_tofield;
  Widget mailto_subfield;

  Widget mailto_form_text;
  Widget mailto_form_fromfield;
  Widget mailto_form_tofield;
  Widget mailto_form_subfield;

  char *post_data;

  int font_size;
  int font_family;
  int pretty;

  int underlines_snarfed;
  int underlines_state;
  /* Default values only, mind you. */
  int underlines;
  int visited_underlines;
  Boolean dashed_underlines;
  Boolean dashed_visited_underlines;

#ifdef HAVE_DTM
  Widget dtmout_text;
#endif /* HAVE_DTM */

#ifdef HAVE_AUDIO_ANNOTATIONS
  Widget audio_start_button;
  Widget audio_stop_button;
  pid_t record_pid;
  char *record_fnam;
#endif

  Widget annotate_author;
  Widget annotate_title;
  Widget annotate_text;
  Widget delete_button;
  Widget include_fsb;
  int annotation_mode;
  int editing_id;

  char *cached_url;

  Widget search_win_text;
  Widget search_caseless_toggle;
  Widget search_backwards_toggle;
  void *search_start;
  void *search_end;

  Widget src_search_win_text;
  Widget src_search_caseless_toggle;
  Widget src_search_backwards_toggle;
  int src_search_pos;

  Widget cci_win_text;
  Widget cci_accept_toggle;
  Widget cci_off_toggle;

  int binary_transfer;
  int delay_image_loads;
/*SWP*/
  int table_support;
  Boolean body_color;
  Boolean body_images;
  int image_view_internal;

/* PLB */
  Widget subgroup;
  Widget unsubgroup;

  struct mo_window *next;

#ifdef GRPAN_PASSWD
  Widget passwd_label;
  Widget annotate_passwd;
  Widget passwd_toggle;
#endif
  XmxMenuRecord *pubpri_menu;
  int pubpri;  /* one of mo_annotation_[public,private] */
  XmxMenuRecord *audio_pubpri_menu;
  int audio_pubpri;  /* one of mo_annotation_[public,private] */
#ifdef NOPE_NOPE_NOPE
  XmxMenuRecord *title_menu;
  int title_opt;  /* mo_document_title or mo_document_url */
  Widget annotate_toggle;
  Widget crossref_toggle;
  Widget checkout_toggle;
  Widget checkin_toggle;
#endif

  int agent_state;
  Boolean have_focus;

} mo_window;

/* ------------------------------- mo_node -------------------------------- */

/* mo_node is a component of the linear history list.  A single
   mo_node will never be effective across multiple mo_window's;
   each window has its own linear history list. */
typedef struct mo_node
{
  char *title;
  char *url;
  char *last_modified;
  char *expires;
  char *ref;  /* how the node was referred to from a previous anchor,
                 if such an anchor existed. */
  char *text;
  char *texthead;   /* head of the alloc'd text -- this should
                       be freed, NOT text */
  /* Position in the list, starting at 1; last item is
     effectively 0 (according to the XmList widget). */
  int position;

  /* The type of annotation this is (if any) */
  int annotation_type;

  /* This is returned from HTMLPositionToId. */
  int docid;

  /* This is returned from HTMLGetWidgetInfo. */
  void *cached_stuff;

  /* Type of authorization */
  int authType;

  struct mo_node *previous;
  struct mo_node *next;
} mo_node;


/* ------------------------------------------------------------------------ */
/* ------------------------------ MISC TYPES ------------------------------ */
/* ------------------------------------------------------------------------ */

typedef enum
{
  mo_fail = 0, mo_succeed
} mo_status;

typedef enum
{
  mo_annotation_public = 0, mo_annotation_workgroup, mo_annotation_private
} mo_pubpri_token;


/* ---------------------------- a few globals ----------------------------- */

extern Display *dsp;

/* ------------------------------- menubar -------------------------------- */

typedef enum
{
#ifdef HAVE_DTM
  mo_dtm_open_outport, mo_dtm_send_document,
#endif
#ifdef KRB4
  mo_kerberosv4_login,
#endif
#ifdef KRB5
  mo_kerberosv5_login,
#endif
  mo_proxy, mo_no_proxy,
  mo_reload_document, mo_reload_document_and_images,
  mo_refresh_document, mo_clear_image_cache,
  mo_cci,
  mo_document_source, mo_document_edit, mo_document_date, mo_search,
  mo_open_document, mo_open_local_document, mo_save_document,
  mo_mail_document, mo_print_document,
  mo_new_window, mo_clone_window,
  mo_close_window, mo_exit_program,
  mo_home_document, mo_ncsa_document,
  mo_mosaic_manual, mo_mosaic_demopage,
  mo_back, mo_forward, mo_history_list,
  mo_clear_global_history,
  mo_hotlist_postit, mo_register_node_in_default_hotlist,
  mo_all_hotlist_to_rbm, mo_all_hotlist_from_rbm,
  mo_network_starting_points, mo_internet_metaindex, mo_search_index,
  mo_large_fonts, mo_regular_fonts, mo_small_fonts,
  mo_large_helvetica, mo_regular_helvetica, mo_small_helvetica,
  mo_large_newcentury, mo_regular_newcentury, mo_small_newcentury,
  mo_large_lucidabright, mo_regular_lucidabright, mo_small_lucidabright,
  mo_help_about, mo_help_onwindow, mo_help_onversion, mo_help_faq,
  mo_techsupport, mo_help_html, mo_help_url, mo_cc,
  mo_whats_new,
  mo_annotate,
#ifdef HAVE_AUDIO_ANNOTATIONS
  mo_audio_annotate,
#endif
  mo_annotate_edit, mo_annotate_delete,
  mo_checkout, mo_checkin,
  mo_fancy_selections,
  mo_default_underlines, mo_l1_underlines, mo_l2_underlines, mo_l3_underlines,
  mo_no_underlines, mo_binary_transfer,
/* links window */
  mo_links_window,
/* News Menu & Stuff */
  mo_news_prev, mo_news_next, mo_news_prevt, mo_news_nextt,
  mo_news_post, mo_news_cancel, mo_news_reply, mo_news_follow,
  mo_news_fmt0, mo_news_fmt1, mo_news_index, mo_news_list,
  mo_news_groups, mo_news_flush, mo_news_flushgroup,
  mo_news_grp0, mo_news_grp1, mo_news_grp2,
  mo_news_art0, mo_news_art1, mo_use_flush,
  mo_news_sub, mo_news_unsub,   mo_news_sub_anchor, mo_news_unsub_anchor,
  mo_news_mread, mo_news_mread_anchor, mo_news_munread, mo_news_maunread,

/* Other stuff */
  mo_re_init, mo_delay_image_loads, mo_table_support, mo_expand_images_current,
  mo_image_view_internal,
/* FTP */
  mo_ftp_put, mo_ftp_remove, mo_ftp_mkdir, mo_body_color, mo_body_images,

/* Password cash stuff */
  mo_clear_passwd_cache,

/* NOTE!!!!!! THIS MUST ALWAYS BE LAST!!!!!! */
  mo_last_entry
} mo_token;


#include "prefs.h"


/* ----------------------------- END OF FILE ------------------------------ */

#endif /* not __MOSAIC_H__ */





