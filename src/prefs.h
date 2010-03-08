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

/* 
 * Created: Wed Dec 20 11:08:12 CDT 1995
 * Author: Dan Pape
 *
 */

#ifndef PREFS
#define PREFS 1

/* This include file contains enums for the variables in the following
   structures. This is so get_pref() knows what to return */

#include <X11/Intrinsic.h>
#include "prefs_defs.h"

typedef struct
{

/* anchors */

    Boolean track_visited_anchors;
    Boolean display_urls_not_titles;      
    Boolean track_pointer_motion;
    Boolean track_full_url_names;

/* annotations */
    
    Boolean annotations_on_top;
    Boolean confirm_delete_annotation;
    char *annotation_server;
    
/* audio */

    char *record_command_location;
    char *record_command;
    
/* cache */

    Boolean reload_pragma_no_cache;

/* strings (command names, file names) */
    
    char *sendmail_command;
    char *edit_command;
    char *xterm_command;
    char *mail_filter_command;
       
/* directories */

    char *private_annotation_directory;

/* document */
    
    char *home_document;                  
    char *tmp_directory;
    char *docs_directory;
    
/* fonts */
    
    char *default_font_choice;
    
/* history */
    
    char *global_history_file;
    char *history_file;
    Boolean use_global_history;           
  
/* hotlist */
    
    char *default_hotlist_file;
    char *default_hot_file;
    char *documents_menu_specfile;
    Boolean addHotlistAddsRBM;
    Boolean addRBMAddsRBM;
    
/* images */
    
    int colors_per_inlined_image;
    int image_cache_size;
    Boolean reload_reloads_images;
    Boolean reverse_inlined_bitmap_colors;
    Boolean delay_image_loads;
    
/* mail */
    
    char *default_author_name;
    char *default_author_email;
    char *signature;
    char *mail_mode;
    
/* MIME */
    
    char *print_command;
    char *uncompress_command;
    char *gunzip_command;
    Boolean use_default_extension_map;
    Boolean use_default_type_map;
    char *global_extension_map;
    char *personal_extension_map;
    char *global_type_map;
    char *personal_type_map;
    Boolean tweak_gopher_types;

/* layout info form main gui */
    char *gui_layout;
    
/* news */
    
/* printing */
    
    char *print_mode;
    Boolean print_banners;
    Boolean print_footnotes;
    Boolean print_us;
    
/* proxy */
    
    char *proxy_specfile;
    char *noproxy_specfile;
    
/* services */
    
    int cciPort;
    int max_num_of_cci_connections;
    int max_wais_responses;
    Boolean kiosk;
    Boolean kioskPrint;
    Boolean kioskNoExit;
    Boolean keepAlive;
    int ftp_timeout_val;
    
/* tables */
    
    Boolean enable_tables;
    
/* window */
    
    int default_width;                    
    int default_height;                   
    Boolean auto_place_windows;
    Boolean initial_window_iconic;
    Boolean titleIsWindowTitle;
    Boolean useIconBar;
    Boolean useTextButtonBar;
    Boolean twirling_transfer_icon;
    Boolean securityIcon;
    int twirl_increment;

/* Save file stuff */

    char *save_mode;
    
/* HDF stuff */
    
    int hdf_max_image_dimension;
    int hdf_max_displayed_datasets;
    int hdf_max_displayed_attributes;
    Boolean hdf_power_user;
    Boolean hdflongname;
    
/* miscellaneous */

    char *full_hostname;
    int  load_local_file;
    Boolean edit_command_use_xterm;
    Boolean confirm_exit;
    Boolean default_fancy_selections;
    Boolean catch_prior_and_next; 
    Boolean simple_interface; 
    Boolean protect_me_from_myself;      
    Boolean gethostbyname_is_evil;  
    Boolean useAFSKlog;
    
#ifdef __sgi
    Boolean debugging_malloc;
#endif

    /* new in 2.7 */

	Boolean clipping;
	int max_clip_x;
	int max_clip_y;
	Boolean long_text_names;
	char *toolbar_layout;
	Boolean sendReferer;
	Boolean sendAgent;
	Boolean expandUrls;
	Boolean expandUrlsWithName;
	char * defaultProtocol;
	char * meterForeground;
	char * meterBackground;
	char * meterFontForeground;
	char * meterFontBackground;
	Boolean use_meter;
	Boolean backup_files;
    char * pix_basename;
	int pix_count;
	char * acceptlanguage_str;
	int ftpRedial;
	int ftpRedialSleep;
	int ftpFilenameLength;
	int ftpEllipsisLength;
	int ftpEllipsisMode;
	Boolean useScreenGamma;
	float screen_gamma;
    Boolean disableMiddleButton;

        /* newer in 2.7 */

    Boolean httpTrace;
    Boolean www2Trace;
    Boolean htmlwTrace;
    Boolean cciTrace;
    Boolean srcTrace;
    Boolean cacheTrace;
    Boolean nutTrace;
    Boolean animateBusyIcon;

    Boolean splashScreen;
    Boolean instamap;
    Boolean imageViewInternal;
    int urlExpired;
    int popupCascadeMappingDelay;
    Boolean frame_hack;  
    
  /* newest in 2.7 (ha top that) */
    Boolean newsNoThreadJumping;
    Boolean newsShowAllArticles;
    Boolean newsShowAllGroups;
    Boolean newsShowReadGroups;
    Boolean newsConfigView;
    Boolean newsUseBackgroundFlush;
    int newsBackgroundFlushTime;

  /* newest in 2.7b5 double haha; */
    Boolean newsPrevIsUnread;
    Boolean newsNextIsUnread;
    char *newsNewsrcPrefix;
    Boolean newsUseNewsrc;
    int newsSubjectWidth;
    int newsAuthorWidth;

    Boolean focusFollowsMouse;
    Boolean sessionHistoryOnRBM;
    int numberOfItemsInRBMHistory;
    Boolean hotlistOnRBM;

    Boolean newsUseShortNewsrc;

} AppData, *AppDataPtr;



typedef struct prefs {

    int version;
    AppDataPtr RdataP;

} prefsStruct, *prefsStructP;


Boolean preferences_genesis(void);
Boolean preferences_armegeddon(void);

Boolean read_preferences_file(prefsStructP inPrefsStruct);

prefsStructP get_ptr_to_preferences(void);

void *get_pref(long pref_id);
char *get_pref_string(long pref_id);
int get_pref_int(long pref_id);
Boolean get_pref_boolean(long pref_id);
float get_pref_float(long pref_id);

void set_pref_boolean(long pref_id, int value);
void set_pref(long pref_id, void *incoming);

void mo_preferences_dialog(mo_window *win);


#endif

/*
  To add a new preference:

  1) Figure out what you want to call it. (duh)

  2) Add it to the structure in xresources.h. (This will eventually go
     away, but do it for now.

  2) Figure out where you want it to go in the above structures. For
     example, any preference which would normally be added as an xresource
     would go in the Appdata structure. If you are adding a whole bunch of
     related preferences, you might consider adding a new structure
     containing them to the main prefsStruct.

  3) Add the variable to the structure (preferably at the end).

  4) Add an enumeration for the varuable in the prefs_defs.h file.

  (the rest of the changes are in prefs.c)
  
  5) Add a write_pref_* function call to the list in
     write_preferences_file() so that your preference will be added to the
     new prefs file.

  6) Add a news case to get_pref().

  7) Add a new case to set_pref().

  8) Make sure you use the correct variant of get_pref and set_pref in your
     code when you actually want to use your new variable.

  9) IMPORTANT!!! : Before a new public release - either the
     PREFERENCES_MAJOR_VERSION or the PREFERENCES_MINOR_VERSION defines
     must be changed, or we risk mangleing our users prefs files!

  Last Change: Mar 21, 1996 - Dan X. Pape

*/
