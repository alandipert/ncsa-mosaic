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
 * Created: Wed Sep 20 11:05:19 CDT 1995
 * Modified: All the time.
 * Author: Dan Pape
 *
 */
#include "../config.h"
#include "mosaic.h"

#include <pwd.h>
#include <sys/utsname.h>


/***********/
/* Defines */
/***********/

#define PREFS_FILE_IO 0

#define PREFERENCES_FILENAME ".mosaic/preferences" // SAM
#define PREFERENCES_MAJOR_VERSION 1
#define PREFERENCES_MINOR_VERSION 0

/***************************/
/* Static Global Variables */
/***************************/

static prefsStructP thePrefsStructP;
static char prefs_file_pathname[512];


/********************************/
/* Static function declarations */
/********************************/

static Boolean revert_preferences_file(prefsStructP inPrefsStruct);
static Boolean write_preferences_file(prefsStructP inPrefsStruct);
static Boolean create_prefs_filename(char *fname);

static Boolean write_pref_string(FILE *fp, long pref_id, char *string);
static Boolean write_pref_int(FILE *fp, long pref_id, char *string);
static Boolean write_pref_boolean(FILE *fp, long pref_id, char *string);
static Boolean write_pref_float(FILE *fp, long pref_id, char *string);


/****************************************************************************
 ****************************************************************************
 *            Preference initialization and closing functions
 *
 ****************************************************************************
 ***************************************************************************/

/****************************************************************************
   Function: preferences_genesis(void)
   Desc:     Initializes the preferences manager.
 ***************************************************************************/
Boolean preferences_genesis(void) {

    Boolean successful = 1;

        /* initialize preferences structure */
    thePrefsStructP = (prefsStructP) malloc (sizeof(prefsStruct));
    if(thePrefsStructP == NULL) {
        fprintf(stderr, "Error: no memory for preferences structure\n");
        return 0;
    }

    thePrefsStructP->RdataP = (AppDataPtr) malloc (sizeof(AppData));
    if(thePrefsStructP->RdataP == NULL) {
        free(thePrefsStructP);
        fprintf(stderr, "Error: no memory for appdata structure\n");
        return 0;
    }

    return successful;
}



/****************************************************************************
   Function: preferences_armegeddon(void)
   Desc:     Kills the preferences manager.
 ***************************************************************************/
Boolean preferences_armegeddon(void) {

    Boolean successful = 1;

       /* write the prefs file just to be safe */
        /*write_preferences_file(thePrefsStructP);*/

       /* free preferences structure */
    free(thePrefsStructP);

    return(successful);

}



/****************************************************************************
 ****************************************************************************
 *                    Preference File access functions
 *
 ****************************************************************************
 ***************************************************************************/

/****************************************************************************
   Function: create_prefs_filename(char *fname)
   Desc:     Generates a full path name for the preferences file
 ***************************************************************************/
static Boolean create_prefs_filename(char *fname) {

    char *home_ptr, home[256];
    struct passwd *pwdent;

        /*
         * Try the HOME environment variable, then the password file, and
         *   finally give up.
         */
    if (!(home_ptr=getenv("HOME"))) {
        if (!(pwdent=getpwuid(getuid()))) {
            return(0);
        }
        else {
            strcpy(home,pwdent->pw_dir);
        }
    }
    else {
        strcpy(home,home_ptr);
    }

    sprintf(fname,"%s/%s",home,PREFERENCES_FILENAME);

    return(1);

}


/****************************************************************************
   Function: read_preferences_file(prefsStructP inPrefsStruct)
   Desc:     Read the prefs file into the incoming prefs struct.
 ***************************************************************************/
Boolean read_preferences_file(prefsStructP inPrefsStruct) {

    FILE *fp;
    Boolean successful = 1;


    /* if the incoming pointer is NULL, then we use the main structure */
    if(inPrefsStruct == NULL)
        inPrefsStruct = thePrefsStructP;

#if PREFS_FILE_IO

    /* look for the file */
    if(!create_prefs_filename(prefs_file_pathname)) {
        fprintf(stderr, "Error: Can't generate pathname for preferences file\n");
        return 0;
    }

    /* Check to see if the file exists. If it doesn't, then create it */

    if(!file_exists(prefs_file_pathname))
        if(!write_preferences_file(NULL)) {
            fprintf(stderr, "Error: Can't find or create preferences file\n");
            return 0;
        }


    /* open it and read all the stuff from the file into the prefs struct */
    if(!(fp=fopen(prefs_file_pathname, "r"))) {

        fprintf(stderr, "Error: Can't open preferences file for reading\n");
        return 0;
    }

    /* but first, check the version number of the prefs file */

    fclose(fp);

#endif

    return successful;
}


/****************************************************************************
   Function: revert_preferences_file(prefsStructP inPrefsStruct)
   Desc:     Reverts the incoming prefs struct to the one last saved to
                 disk.
 ***************************************************************************/
static Boolean revert_preferences_file(prefsStructP inPrefsStruct) {


}


/****************************************************************************
   Function: write_pref_string(FILE *fp, long pref_id, char *string)
   Desc:
 ***************************************************************************/
static Boolean write_pref_string(FILE *fp, long pref_id, char *string) {

#if PREFS_FILE_IO

    char *narf;

    narf = get_pref_string(pref_id);

    if(narf == NULL)
        fprintf(fp, "%s:\n", string);
    else
        fprintf(fp, "%s: %s\n", string, narf);

    return 1;

#endif

}

/****************************************************************************
   Function: write_pref_int(FILE *fp, long pref_id, char *string)
   Desc:
 ***************************************************************************/
static Boolean write_pref_int(FILE *fp, long pref_id, char *string) {

#if PREFS_FILE_IO

    int narf;

    narf = get_pref_int(pref_id);

    fprintf(fp, "%s: %d\n", string, narf);

    return 1;

#endif

}

/****************************************************************************
   Function: write_pref_boolean(FILE *fp, long pref_id, char *string)
   Desc:
 ***************************************************************************/
static Boolean write_pref_boolean(FILE *fp, long pref_id, char *string) {

#if PREFS_FILE_IO

    Boolean  narf;

    narf = get_pref_boolean(pref_id);

    if(narf == 1)
        fprintf(fp, "%s: True\n", string);
    else if (narf == 0)
        fprintf(fp, "%s: False\n", string);

    return 1;

#endif

}

/****************************************************************************
   Function: write_pref_float(FILE *fp, long pref_id, char *string)
   Desc:
 ***************************************************************************/
static Boolean write_pref_float(FILE *fp, long pref_id, char *string) {

#if PREFS_FILE_IO

    float narf;

    narf = get_pref_float(pref_id);

    fprintf(fp, "%s: %f\n", string, narf);

    return 1;

#endif

}


/****************************************************************************
   Function: write_preferences_file(void)
   Desc:     Writes the incoming preferences structure to disk. If the
             incoming pointer is NULL, then we are creating
		     the prefs file for the first time - although, the prefs struct
             is already filled in, becuase all the default values are in
             xresources.h.
 ***************************************************************************/
static Boolean write_preferences_file(prefsStructP inPrefsStruct) {


#if PREFS_FILE_IO

    Boolean successful = 1;
    FILE *fp;


    if(!(fp=fopen(prefs_file_pathname, "w"))) {
        fprintf(stderr, "Error: Can't open preferences file for writing\n");
        return 0;
    }


        /* write out our little header... */

    fprintf(fp, "# NCSA Mosaic preferences file\n");
    fprintf(fp, "# File Version: %d.%d\n",
            PREFERENCES_MAJOR_VERSION,
            PREFERENCES_MINOR_VERSION);
    fprintf(fp, "# Warning - this is NOT a user editable file!!!\n");
    fprintf(fp, "# If a character is out of place...it will be very bad.\n\n");


/* access all the fields in the prefs structure, and write them out */

    write_pref_boolean(fp, eTRACK_VISITED_ANCHORS, "TRACK_VISITED_ANCHORS");
    write_pref_boolean(fp, eDISPLAY_URLS_NOT_TITLES, "DISPLAY_URLS_NOT_TITLES");
    write_pref_boolean(fp, eTRACK_POINTER_MOTION, "TRACK_POINTER_MOTION");
    write_pref_boolean(fp, eTRACK_FULL_URL_NAMES, "TRACK_FULL_URL_NAMES");
    write_pref_boolean(fp, eANNOTATIONS_ON_TOP, "ANNOTATIONS_ON_TOP");
    write_pref_boolean(fp, eCONFIRM_DELETE_ANNOTATION, "CONFIRM_DELETE_ANNOTATION");
    write_pref_string(fp, eANNOTATION_SERVER, "ANNOTATION_SERVER");
    write_pref_string(fp, eRECORD_COMMAND_LOCATION, "RECORD_COMMAND_LOCATION");
    write_pref_string(fp, eRECORD_COMMAND, "RECORD_COMMAND");
    write_pref_boolean(fp, eRELOAD_PRAGMA_NO_CACHE, "RELOAD_PRAGMA_NO_CACHE");
    write_pref_string(fp, eSENDMAIL_COMMAND, "SENDMAIL_COMMAND");
    write_pref_string(fp, eEDIT_COMMAND, "EDIT_COMMAND");
    write_pref_string(fp, eXTERM_COMMAND, "XTERM_COMMAND");
    write_pref_string(fp, eMAIL_FILTER_COMMAND, "MAIL_FILTER_COMMAND");
    write_pref_string(fp, ePRIVATE_ANNOTATION_DIRECTORY, "PRIVATE_ANNOTATION_DIRECTORY");
    write_pref_string(fp, eHOME_DOCUMENT, "HOME_DOCUMENT");
    write_pref_string(fp, eTMP_DIRECTORY, "TMP_DIRECTORY");
    write_pref_string(fp, eDOCS_DIRECTORY, "DOCS_DIRECTORY");
    write_pref_string(fp, eDEFAULT_FONT_CHOICE, "DEFAULT_FONT_CHOICE");
    write_pref_string(fp, eGLOBAL_HISTORY_FILE, "GLOBAL_HISTORY_FILE");
    write_pref_boolean(fp, eUSE_GLOBAL_HISTORY, "USE_GLOBAL_HISTORY");
    write_pref_string(fp, eHISTORY_FILE, "HISTORY_FILE");
    write_pref_string(fp, eDEFAULT_HOTLIST_FILE, "DEFAULT_HOTLIST_FILE");
    write_pref_boolean(fp, eADD_HOTLIST_ADDS_RBM, "ADD_HOTLIST_ADDS_RBM");
    write_pref_boolean(fp, eADD_RBM_ADDS_RBM, "ADD_RBM_ADDS_RBM");
    write_pref_string(fp, eDOCUMENTS_MENU_SPECFILE, "DOCUMENTS_MENU_SPECFILE");
    write_pref_int(fp, eCOLORS_PER_INLINED_IMAGE, "COLORS_PER_INLINED_IMAGE");
    write_pref_int(fp, eIMAGE_CACHE_SIZE, "IMAGE_CACHE_SIZE");
    write_pref_boolean(fp, eRELOAD_RELOADS_IMAGES, "RELOAD_RELOADS_IMAGES");
    write_pref_boolean(fp, eREVERSE_INLINED_BITMAP_COLORS, "REVERSE_INLINED_BITMAP_COLORS");
    write_pref_boolean(fp, eDELAY_IMAGE_LOADS, "DELAY_IMAGE_LOADS");
    write_pref_float(fp, eSCREEN_GAMMA, "SCREEN_GAMMA");
    write_pref_string(fp, eDEFAULT_AUTHOR_NAME, "DEFAULT_AUTHOR_NAME");
    write_pref_string(fp, eDEFAULT_AUTHOR_EMAIL, "DEFAULT_AUTHOR_EMAIL");
    write_pref_string(fp, eSIGNATURE, "SIGNATURE");
    write_pref_string(fp, eMAIL_MODE, "MAIL_MODE");
    write_pref_string(fp, ePRINT_COMMAND, "PRINT_COMMAND");
    write_pref_string(fp, eUNCOMPRESS_COMMAND, "UNCOMPRESS_COMMAND");
    write_pref_string(fp, eGUNZIP_COMMAND, "GUNZIP_COMMAND");
    write_pref_boolean(fp, eUSE_DEFAULT_EXTENSION_MAP, "USE_DEFAULT_EXTENSION_MAP");
    write_pref_boolean(fp, eUSE_DEFAULT_TYPE_MAP, "USE_DEFAULT_TYPE_MAP");
    write_pref_string(fp, eGLOBAL_EXTENSION_MAP, "GLOBAL_EXTENSION_MAP");
    write_pref_string(fp, ePERSONAL_EXTENSION_MAP, "PERSONAL_EXTENSION_MAP");
    write_pref_string(fp, eGLOBAL_TYPE_MAP, "GLOBAL_TYPE_MAP");
    write_pref_string(fp, ePERSONAL_TYPE_MAP, "PERSONAL_TYPE_MAP");
    write_pref_boolean(fp, eTWEAK_GOPHER_TYPES, "TWEAK_GOPHER_TYPES");
    write_pref_string(fp, ePRINT_MODE, "PRINT_MODE");
    write_pref_string(fp, eGUI_LAYOUT, "GUI_LAYOUT");
    write_pref_boolean(fp, ePRINT_BANNERS, "PRINT_BANNERS");
    write_pref_boolean(fp, ePRINT_FOOTNOTES, "PRINT_FOOTNOTES");
    write_pref_boolean(fp, ePRINT_PAPER_SIZE_US, "PRINT_PAPER_SIZE_US");
    write_pref_string(fp, ePROXY_SPECFILE, "PROXY_SPECFILE");
    write_pref_string(fp, eNOPROXY_SPECFILE, "NOPROXY_SPECFILE");
    write_pref_int(fp, eCCIPORT, "CCIPORT");
    write_pref_int(fp, eMAX_NUM_OF_CCI_CONNECTIONS, "MAX_NUM_OF_CCI_CONNECTIONS");
    write_pref_int(fp, eMAX_WAIS_RESPONSES, "MAX_WAIS_RESPONSES");
    write_pref_boolean(fp, eKIOSK, "KIOSK");
    write_pref_boolean(fp, eKIOSKNOEXIT, "KIOSKNOEXIT");
    write_pref_boolean(fp, eKEEPALIVE, "KEEPALIVE");
    write_pref_int(fp, eFTP_TIMEOUT_VAL, "FTP_TIMEOUT_VAL");
    write_pref_boolean(fp, eENABLE_TABLES, "ENABLE_TABLES");
    write_pref_int(fp, eDEFAULT_WIDTH, "DEFAULT_WIDTH");
    write_pref_int(fp, eDEFAULT_HEIGHT, "DEFAULT_HEIGHT");
    write_pref_boolean(fp, eAUTO_PLACE_WINDOWS, "AUTO_PLACE_WINDOWS");
    write_pref_boolean(fp, eINITIAL_WINDOW_ICONIC, "INITIAL_WINDOW_ICONIC");
    write_pref_boolean(fp, eTITLEISWINDOWTITLE, "TITLEISWINDOWTITLE");
    write_pref_boolean(fp, eUSEICONBAR, "USEICONBAR");
    write_pref_boolean(fp, eUSETEXTBUTTONBAR, "USETEXTBUTTONBAR");
    write_pref_boolean(fp, eTWIRLING_TRANSFER_ICON, "TWIRLING_TRANSFER_ICON");
    write_pref_boolean(fp, eSECURITYICON, "SECURITYICON");
    write_pref_int(fp, eTWIRL_INCREMENT, "TWIRL_INCREMENT");
    write_pref_string(fp, eSAVE_MODE, "SAVE_MODE");
    write_pref_int(fp, eHDF_MAX_IMAGE_DIMENSION, "HDF_MAX_IMAGE_DIMENSION");
    write_pref_int(fp, eHDF_MAX_DISPLAYED_DATASETS, "HDF_MAX_DISPLAYED_DATASETS");
    write_pref_int(fp, eHDF_MAX_DISPLAYED_ATTRIBUTES, "HDF_MAX_DISPLAYED_ATTRIBUTES");
    write_pref_boolean(fp, eHDF_POWER_USER, "HDF_POWER_USER");
    write_pref_boolean(fp, eHDFLONGNAME, "HDFLONGNAME");
    write_pref_string(fp, eFULL_HOSTNAME, "FULL_HOSTNAME");
    write_pref_int(fp, eLOAD_LOCAL_FILE, "LOAD_LOCAL_FILE");
    write_pref_boolean(fp, eEDIT_COMMAND_USE_XTERM, "EDIT_COMMAND_USE_XTERM");
    write_pref_boolean(fp, eCONFIRM_EXIT, "CONFIRM_EXIT");
    write_pref_boolean(fp, eDEFAULT_FANCY_SELECTIONS, "DEFAULT_FANCY_SELECTIONS");
    write_pref_boolean(fp, eCATCH_PRIOR_AND_NEXT, "CATCH_PRIOR_AND_NEXT");
    write_pref_boolean(fp, eSIMPLE_INTERFACE, "SIMPLE_INTERFACE");
    write_pref_boolean(fp, ePROTECT_ME_FROM_MYSELF, "PROTECT_ME_FROM_MYSELF");
    write_pref_boolean(fp, eGETHOSTBYNAME_IS_EVIL, "GETHOSTBYNAME_IS_EVIL");
#ifdef __sgi
    write_pref_boolean(fp, eDEBUGGING_MALLOC, "DEBUGGING_MALLOC");
#endif
    write_pref_boolean(fp, eUSEAFSKLOG, "USEAFSKLOG");
    write_pref_boolean(fp, eSEND_REFERER, "SEND_REFERER");
    write_pref_boolean(fp, eSEND_AGENT, "SEND_AGENT");
    write_pref_boolean(fp, eEXPAND_URLS, "EXPAND_URLS");
    write_pref_boolean(fp, eEXPAND_URLS_WITH_NAME, "EXPAND_URLS_WITH_NAME");
    write_pref_string(fp, eDEFAULT_PROTOCOL, "DEFAULT_PROTOCOL");
    write_pref_string(fp, eMETER_FOREGROUND, "METER_FOREGROUND");
    write_pref_string(fp, eMETER_BACKGROUND, "METER_BACKGROUND");
    write_pref_string(fp, eMETER_FONT_FOREGROUND, "METER_FONT_FOREGROUND");
    write_pref_string(fp, eMETER_FONT_BACKGROUND, "METER_FONT_BACKGROUND");
    write_pref_boolean(fp, eMETER, "METER");
    write_pref_boolean(fp, eBACKUP_FILES, "BACKUP_FILES");
    write_pref_string(fp, ePIX_BASENAME, "PIX_BASENAME");
    write_pref_int(fp, ePIX_COUNT, "PIX_COUNT");
    write_pref_string(fp, eACCEPT_LANGUAGE_STR, "ACCEPT_LANGUAGE_STR");
    write_pref_int(fp, eFTP_REDIAL, "FTP_REDIAL");
    write_pref_int(fp, eFTP_REDIAL_SLEEP, "FTP_REDIAL_SLEEP");
    write_pref_int(fp, eFTP_FILENAME_LENGTH, "FTP_FILENAME_LENGTH");
    write_pref_int(fp, eFTP_ELLIPSIS_LENGTH, "FTP_ELLIPSIS_LENGTH");
    write_pref_int(fp, eFTP_ELLIPSIS_MODE, "FTP_ELLIPSIS_MODE");
    write_pref_boolean(fp, eTITLE_ISWINDOW_TITLE, "TITLE_ISWINDOW_TITLE");
    write_pref_boolean(fp, eUSE_SCREEN_GAMMA, "USE_SCREEN_GAMMA");
    write_pref_boolean(fp, eDISABLEMIDDLEBUTTON, "DISABLEMIDDLEBUTTON");
    write_pref_boolean(fp, eHTTPTRACE, "HTTPTRACE");
    write_pref_boolean(fp, eWWW2TRACE, "WWW2TRACE");
    write_pref_boolean(fp, eHTMLWTRACE, "HTMLWTRACE");
    write_pref_boolean(fp, eCCITRACE, "CCITRACE");
    write_pref_boolean(fp, eSRCTRACE, "SRCTRACE");
    write_pref_boolean(fp, eCACHETRACE, "CACHETRACE");
    write_pref_boolean(fp, eNUTTRACE, "NUTTRACE");
    write_pref_boolean(fp, eANIMATEBUSYICON, "ANIMATEBUSYICON");
    write_pref_boolean(fp, eSPLASHSCREEN, "SPLASHSCREEN");
    write_pref_boolean(fp, eINSTALL_COLORMAP, "INSTALL_COLORMAP");
    write_pref_boolean(fp, eIMAGEVIEWINTERNAL, "IMAGEVIEWINTERNAL");
    write_pref_int(fp, eURLEXPIRED, "URLEXPIRED");
    write_pref_int(fp, ePOPUPCASCADEMAPPINGDELAY, "POPUPCASCADEMAPPINGDELAY");
    write_pref_boolean(fp, eFRAME_HACK, "FRAME_HACK");
    write_pref_boolean(fp, eCLIPPING, "CLIPPING");
    write_pref_int(fp, eMAX_CLIPPING_SIZE_X, "MAX_CLIPPING_SIZE_X");
    write_pref_int(fp, eMAX_CLIPPING_SIZE_Y, "MAX_CLIPPING_SIZE_Y");
    write_pref_boolean(fp, eUSE_LONG_TEXT_NAMES, "USE_LONG_TEXT_NAMES");
    write_pref_string(fp, eTOOLBAR_LAYOUT, "TOOLBAR_LAYOUT");

    write_pref_boolean (fp, eUSETHREADVIEW, "USETHREADVIEW");
    write_pref_boolean (fp, eSHOWREADGROUPS, "SHOWREADGROUPS");
    write_pref_boolean (fp, eNOTHREADJUMPING, "NOTHREADJUMPING");
    write_pref_boolean (fp, eSHOWALLGROUPS, "SHOWALLGROUPS");
    write_pref_boolean (fp, eSHOWALLARTICLES, "SHOWALLARTICLES");
    write_pref_boolean (fp, eUSEBACKGROUNDFLUSH, "USEBACKGROUNDFLUSH");
    write_pref_int     (fp, eBACKGROUNDFLUSHTIME, "BACKGROUNDFLUSHTIME");
    write_pref_boolean (fp, ePREVISUNREAD, "PREVISPREVUNREAD");
    write_pref_boolean (fp, eNEXTISUNREAD, "NEXTISNEXTUNREAD");
    write_pref_boolean (fp, eUSENEWSRC, "USENEWSRC");
    write_pref_string  (fp, eNEWSRCPREFIX, "NEWSRCPREFIX");
    write_pref_int (fp, eNEWSAUTHORWIDTH, "NEWSAUTHORWIDTH");
    write_pref_int (fp, eNEWSSUBJECTWIDTH, "NEWSSUBJECTWIDTH");


    write_pref_boolean(fp, eFOCUS_FOLLOWS_MOUSE, "FOCUS_FOLLOWS_MOUSE");
    write_pref_boolean(fp, eSESSION_HISTORY_ON_RBM, "SESSION_HISTORY_ON_RBM");
    write_pref_int(fp, eNUMBER_OF_ITEMS_IN_RBM_HISTORY,
		   "NUMBER_OF_ITEMS_IN_RBM_HISTORY");

    write_pref_boolean (fp, eUSESHORTNEWSRC, "USESHORTNEWSRC");
    fclose(fp);
    return successful;

#endif

}



/****************************************************************************
 ****************************************************************************
 *                   Preference Structure access functions
 *
 ****************************************************************************
 ***************************************************************************/


/****************************************************************************
   Function: get_ptr_to_preferences(void)
   Desc:     Returns a pointer to the main preferences structure
 ***************************************************************************/
prefsStructP get_ptr_to_preferences(void) {

    return thePrefsStructP;

}


/****************************************************************************
   Function: get_pref(long pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
void *get_pref(long pref_id) {

    switch(pref_id) {

        case  eTRACK_VISITED_ANCHORS:
            return (void *)&(thePrefsStructP->RdataP->track_visited_anchors);
            break;
        case  eDISPLAY_URLS_NOT_TITLES:
            return (void *)&(thePrefsStructP->RdataP->display_urls_not_titles);
            break;
        case  eTRACK_POINTER_MOTION:
            return (void *)&(thePrefsStructP->RdataP->track_pointer_motion);
            break;
        case  eTRACK_FULL_URL_NAMES:
            return (void *)&(thePrefsStructP->RdataP->track_full_url_names);
            break;
        case  eANNOTATIONS_ON_TOP:
            return (void *)&(thePrefsStructP->RdataP->annotations_on_top);
            break;
        case  eCONFIRM_DELETE_ANNOTATION:
            return (void *)&(thePrefsStructP->RdataP->confirm_delete_annotation);
            break;
        case  eANNOTATION_SERVER:
            return (void *)(thePrefsStructP->RdataP->annotation_server);
            break;
        case  eRECORD_COMMAND_LOCATION:
            return (void *)(thePrefsStructP->RdataP->record_command_location);
            break;
        case  eRECORD_COMMAND:
            return (void *)(thePrefsStructP->RdataP->record_command);
            break;
        case  eRELOAD_PRAGMA_NO_CACHE:
            return (void *)&(thePrefsStructP->RdataP->reload_pragma_no_cache);
            break;
        case  eSENDMAIL_COMMAND:
            return (void *)(thePrefsStructP->RdataP->sendmail_command);
            break;
        case  eEDIT_COMMAND:
            return (void *)(thePrefsStructP->RdataP->edit_command);
            break;
        case  eXTERM_COMMAND:
            return (void *)(thePrefsStructP->RdataP->xterm_command);
            break;
        case  eMAIL_FILTER_COMMAND:
            return (void *)(thePrefsStructP->RdataP->mail_filter_command);
            break;
        case  ePRIVATE_ANNOTATION_DIRECTORY:
            return (void *)(thePrefsStructP->RdataP->private_annotation_directory);
            break;
        case  eHOME_DOCUMENT:
            return (void *)(thePrefsStructP->RdataP->home_document);
            break;
        case  eTMP_DIRECTORY:
            return (void *)(thePrefsStructP->RdataP->tmp_directory);
            break;
        case  eDOCS_DIRECTORY:
            return (void *)(thePrefsStructP->RdataP->docs_directory);
            break;
        case  eDEFAULT_FONT_CHOICE:
            return (void *)(thePrefsStructP->RdataP->default_font_choice);
            break;
        case  eGLOBAL_HISTORY_FILE:
            return (void *)(thePrefsStructP->RdataP->global_history_file);
            break;
        case  eHISTORY_FILE:
            return (void *)(thePrefsStructP->RdataP->history_file);
            break;
        case  eUSE_GLOBAL_HISTORY:
            return (void *)&(thePrefsStructP->RdataP->use_global_history);
            break;
        case  eDEFAULT_HOTLIST_FILE:
            return (void *)(thePrefsStructP->RdataP->default_hotlist_file);
            break;
        case  eDEFAULT_HOT_FILE:
            return (void *)(thePrefsStructP->RdataP->default_hot_file);
            break;
        case  eADD_HOTLIST_ADDS_RBM:
            return (void *)&(thePrefsStructP->RdataP->addHotlistAddsRBM);
            break;
        case  eADD_RBM_ADDS_RBM:
            return (void *)&(thePrefsStructP->RdataP->addRBMAddsRBM);
            break;
        case  eDOCUMENTS_MENU_SPECFILE:
            return (void *)(thePrefsStructP->RdataP->documents_menu_specfile);
            break;
        case  eCOLORS_PER_INLINED_IMAGE:
            return (void *)&(thePrefsStructP->RdataP->colors_per_inlined_image);
            break;
        case  eIMAGE_CACHE_SIZE:
            return (void *)&(thePrefsStructP->RdataP->image_cache_size);
            break;
        case  eRELOAD_RELOADS_IMAGES:
            return (void *)&(thePrefsStructP->RdataP->reload_reloads_images);
            break;
        case  eREVERSE_INLINED_BITMAP_COLORS:
            return (void *)&(thePrefsStructP->RdataP->reverse_inlined_bitmap_colors);
            break;
        case  eDELAY_IMAGE_LOADS:
            return (void *)&(thePrefsStructP->RdataP->delay_image_loads);
            break;
        case  eDEFAULT_AUTHOR_NAME:
            return (void *)(thePrefsStructP->RdataP->default_author_name);
            break;
        case  eDEFAULT_AUTHOR_EMAIL:
            return (void *)(thePrefsStructP->RdataP->default_author_email);
            break;
        case  eSIGNATURE:
            return (void *)(thePrefsStructP->RdataP->signature);
            break;
        case  eMAIL_MODE:
            return (void *)(thePrefsStructP->RdataP->mail_mode);
            break;
        case  ePRINT_COMMAND:
            return (void *)(thePrefsStructP->RdataP->print_command);
            break;
        case  eUNCOMPRESS_COMMAND:
            return (void *)(thePrefsStructP->RdataP->uncompress_command);
            break;
        case  eGUNZIP_COMMAND:
            return (void *)(thePrefsStructP->RdataP->gunzip_command);
            break;
        case  eUSE_DEFAULT_EXTENSION_MAP:
            return (void *)&(thePrefsStructP->RdataP->use_default_extension_map);
            break;
        case  eUSE_DEFAULT_TYPE_MAP:
            return (void *)&(thePrefsStructP->RdataP->use_default_type_map);
            break;
        case  eGLOBAL_EXTENSION_MAP:
            return (void *)(thePrefsStructP->RdataP->global_extension_map);
            break;
        case  ePERSONAL_EXTENSION_MAP:
            return (void *)(thePrefsStructP->RdataP->personal_extension_map);
            break;
        case  eGLOBAL_TYPE_MAP:
            return (void *)(thePrefsStructP->RdataP->global_type_map);
            break;
        case  ePERSONAL_TYPE_MAP:
            return (void *)(thePrefsStructP->RdataP->personal_type_map);
            break;
        case  eTWEAK_GOPHER_TYPES:
            return (void *)&(thePrefsStructP->RdataP->tweak_gopher_types);
            break;
        case eGUI_LAYOUT:
            return (void *)(thePrefsStructP->RdataP->gui_layout);
            break;
        case  ePRINT_MODE:
            return (void *)(thePrefsStructP->RdataP->print_mode);
            break;
        case  ePRINT_BANNERS:
            return (void *)&(thePrefsStructP->RdataP->print_banners);
            break;
        case  ePRINT_FOOTNOTES:
            return (void *)&(thePrefsStructP->RdataP->print_footnotes);
            break;
        case  ePRINT_PAPER_SIZE_US:
            return (void *)&(thePrefsStructP->RdataP->print_us);
            break;
        case  ePROXY_SPECFILE:
            return (void *)(thePrefsStructP->RdataP->proxy_specfile);
            break;
        case  eNOPROXY_SPECFILE:
            return (void *)(thePrefsStructP->RdataP->noproxy_specfile);
            break;
        case  eCCIPORT:
            return (void *)&(thePrefsStructP->RdataP->cciPort);
            break;
        case  eMAX_NUM_OF_CCI_CONNECTIONS:
            return (void *)&(thePrefsStructP->RdataP->max_num_of_cci_connections);
            break;
        case  eMAX_WAIS_RESPONSES:
            return (void *)&(thePrefsStructP->RdataP->max_wais_responses);
            break;
        case  eKIOSK:
            return (void *)&(thePrefsStructP->RdataP->kiosk);
            break;
        case  eKIOSKPRINT:
            return (void *)&(thePrefsStructP->RdataP->kioskPrint);
            break;
        case  eKIOSKNOEXIT:
            return (void *)&(thePrefsStructP->RdataP->kioskNoExit);
            break;
        case  eKEEPALIVE:
            return (void *)&(thePrefsStructP->RdataP->keepAlive);
            break;
        case  eFTP_TIMEOUT_VAL:
            return (void *)&(thePrefsStructP->RdataP->ftp_timeout_val);
            break;
        case  eENABLE_TABLES:
            return (void *)&(thePrefsStructP->RdataP->enable_tables);
            break;
        case  eDEFAULT_WIDTH:
            return (void *)&(thePrefsStructP->RdataP->default_width);
            break;
        case  eDEFAULT_HEIGHT:
            return (void *)&(thePrefsStructP->RdataP->default_height);
            break;
        case  eAUTO_PLACE_WINDOWS:
            return (void *)&(thePrefsStructP->RdataP->auto_place_windows);
            break;
        case  eINITIAL_WINDOW_ICONIC:
            return (void *)&(thePrefsStructP->RdataP->initial_window_iconic);
            break;
        case  eTITLEISWINDOWTITLE:
            return (void *)&(thePrefsStructP->RdataP->titleIsWindowTitle);
            break;
        case  eUSEICONBAR:
            return (void *)&(thePrefsStructP->RdataP->useIconBar);
            break;
        case  eUSETEXTBUTTONBAR:
            return (void *)&(thePrefsStructP->RdataP->useTextButtonBar);
            break;
        case  eTWIRLING_TRANSFER_ICON:
            return (void *)&(thePrefsStructP->RdataP->twirling_transfer_icon);
            break;
        case  eSECURITYICON:
            return (void *)&(thePrefsStructP->RdataP->securityIcon);
            break;
        case  eTWIRL_INCREMENT:
            return (void *)&(thePrefsStructP->RdataP->twirl_increment);
            break;
        case  eSAVE_MODE:
            return (void *)(thePrefsStructP->RdataP->save_mode);
            break;
        case  eHDF_MAX_IMAGE_DIMENSION:
            return (void *)&(thePrefsStructP->RdataP->hdf_max_image_dimension);
            break;
        case  eHDF_MAX_DISPLAYED_DATASETS:
            return (void *)&(thePrefsStructP->RdataP->hdf_max_displayed_datasets);
            break;
        case  eHDF_MAX_DISPLAYED_ATTRIBUTES:
            return (void *)&(thePrefsStructP->RdataP->hdf_max_displayed_attributes);
            break;
        case  eHDF_POWER_USER:
            return (void *)&(thePrefsStructP->RdataP->hdf_power_user);
            break;
        case  eHDFLONGNAME:
            return (void *)&(thePrefsStructP->RdataP->hdflongname);
            break;
        case  eFULL_HOSTNAME:
            return (void *)(thePrefsStructP->RdataP->full_hostname);
            break;
        case  eLOAD_LOCAL_FILE:
            return (void *)&(thePrefsStructP->RdataP->load_local_file);
            break;
        case  eEDIT_COMMAND_USE_XTERM:
            return (void *)&(thePrefsStructP->RdataP->edit_command_use_xterm);
            break;
        case  eCONFIRM_EXIT:
            return (void *)&(thePrefsStructP->RdataP->confirm_exit);
            break;
        case  eDEFAULT_FANCY_SELECTIONS:
            return (void *)&(thePrefsStructP->RdataP->default_fancy_selections);
            break;
        case  eCATCH_PRIOR_AND_NEXT:
            return (void *)&(thePrefsStructP->RdataP->catch_prior_and_next);
            break;
        case  eSIMPLE_INTERFACE:
            return (void *)&(thePrefsStructP->RdataP->simple_interface);
            break;
        case  ePROTECT_ME_FROM_MYSELF:
            return (void *)&(thePrefsStructP->RdataP->protect_me_from_myself);
            break;
        case  eGETHOSTBYNAME_IS_EVIL:
            return (void *)&(thePrefsStructP->RdataP->gethostbyname_is_evil);
            break;
#ifdef __sgi
        case  eDEBUGGING_MALLOC:
            return (void *)&(thePrefsStructP->RdataP->debugging_malloc);
            break;
#endif
        case  eUSEAFSKLOG:
            return (void *)&(thePrefsStructP->RdataP->useAFSKlog);
            break;

/* New in 2.7 */

        case eSEND_REFERER:
            return (void *)&(thePrefsStructP->RdataP->sendReferer);
            break;
        case eSEND_AGENT:
            return (void *)&(thePrefsStructP->RdataP->sendAgent);
            break;
        case eEXPAND_URLS:
            return (void *)&(thePrefsStructP->RdataP->expandUrls);
            break;
        case eEXPAND_URLS_WITH_NAME:
            return (void *)&(thePrefsStructP->RdataP->expandUrlsWithName);
            break;
        case eDEFAULT_PROTOCOL:
            return (void *)(thePrefsStructP->RdataP->defaultProtocol);
            break;
        case eMETER_FOREGROUND:
            return (void *)(thePrefsStructP->RdataP->meterForeground);
            break;
        case eMETER_BACKGROUND:
            return (void *)(thePrefsStructP->RdataP->meterBackground);
            break;
        case eMETER_FONT_FOREGROUND:
            return (void *)(thePrefsStructP->RdataP->meterFontForeground);
            break;
        case eMETER_FONT_BACKGROUND:
            return (void *)(thePrefsStructP->RdataP->meterFontBackground);
            break;
        case eMETER:
            return (void *)&(thePrefsStructP->RdataP->use_meter);
            break;
        case eBACKUP_FILES:
            return (void *)&(thePrefsStructP->RdataP->backup_files);
            break;
        case ePIX_BASENAME:
            return (void *)(thePrefsStructP->RdataP->pix_basename);
            break;
        case ePIX_COUNT:
            return (void *)&(thePrefsStructP->RdataP->pix_count);
            break;
        case eACCEPT_LANGUAGE_STR:
            return (void *)(thePrefsStructP->RdataP->acceptlanguage_str);
            break;
        case eFTP_REDIAL:
            return (void *)&(thePrefsStructP->RdataP->ftpRedial);
            break;
        case eFTP_REDIAL_SLEEP:
            return (void *)&(thePrefsStructP->RdataP->ftpRedialSleep);
            break;
        case eFTP_FILENAME_LENGTH:
            return (void *)&(thePrefsStructP->RdataP->ftpFilenameLength);
            break;
        case eFTP_ELLIPSIS_LENGTH:
            return (void *)&(thePrefsStructP->RdataP->ftpEllipsisLength);
            break;
        case eFTP_ELLIPSIS_MODE:
            return (void *)&(thePrefsStructP->RdataP->ftpEllipsisMode);
            break;
        case eTITLE_ISWINDOW_TITLE:
            return (void *)&(thePrefsStructP->RdataP->titleIsWindowTitle);
            break;
        case eUSE_SCREEN_GAMMA:
            return (void *)&(thePrefsStructP->RdataP->useScreenGamma);
            break;
        case eSCREEN_GAMMA:
            return (void *)&(thePrefsStructP->RdataP->screen_gamma);
            break;
        case eDISABLEMIDDLEBUTTON:
            return (void *)&(thePrefsStructP->RdataP->useScreenGamma);
            break;
        case eHTTPTRACE:
            return (void *)&(thePrefsStructP->RdataP->httpTrace);
            break;
        case eWWW2TRACE:
            return (void *)&(thePrefsStructP->RdataP->www2Trace);
            break;
        case eHTMLWTRACE:
            return (void *)&(thePrefsStructP->RdataP->htmlwTrace);
            break;
        case eCCITRACE:
            return (void *)&(thePrefsStructP->RdataP->cciTrace);
            break;
        case eSRCTRACE:
            return (void *)&(thePrefsStructP->RdataP->srcTrace);
            break;
        case eCACHETRACE:
            return (void *)&(thePrefsStructP->RdataP->cacheTrace);
            break;
        case eNUTTRACE:
            return (void *)&(thePrefsStructP->RdataP->nutTrace);
            break;
        case eANIMATEBUSYICON:
            return (void *)&(thePrefsStructP->RdataP->animateBusyIcon);
            break;
        case eSPLASHSCREEN:
            return (void *)&(thePrefsStructP->RdataP->splashScreen);
            break;
        case eINSTALL_COLORMAP:
            return (void *)&(thePrefsStructP->RdataP->instamap);
            break;
        case eIMAGEVIEWINTERNAL:
            return (void *)&(thePrefsStructP->RdataP->imageViewInternal);
            break;
        case eURLEXPIRED:
            return (void *)&(thePrefsStructP->RdataP->urlExpired);
            break;
        case ePOPUPCASCADEMAPPINGDELAY:
            return (void *)&(thePrefsStructP->RdataP->popupCascadeMappingDelay);
            break;
        case eFRAME_HACK:
            return (void *)&(thePrefsStructP->RdataP->frame_hack);
            break;

        case eUSETHREADVIEW:
	    return (void *)&(thePrefsStructP->RdataP->newsConfigView);
	    break;

        case eSHOWREADGROUPS:
	  return (void *)&(thePrefsStructP->RdataP->newsShowReadGroups);
	  break;

        case eNOTHREADJUMPING:
	  return (void *)&(thePrefsStructP->RdataP->newsNoThreadJumping);
          break;

        case eSHOWALLGROUPS:
	  return (void *)&(thePrefsStructP->RdataP->newsShowAllGroups);
          break;

        case eSHOWALLARTICLES:
	  return (void *)&(thePrefsStructP->RdataP->newsShowAllArticles);
          break;

        case eUSEBACKGROUNDFLUSH:
	  return (void *)&(thePrefsStructP->RdataP->newsUseBackgroundFlush);
          break;

        case eBACKGROUNDFLUSHTIME:
	  return (void *)&(thePrefsStructP->RdataP->newsBackgroundFlushTime);
          break;

        case eCLIPPING:
	  return (void *)&(thePrefsStructP->RdataP->clipping);
          break;

        case eMAX_CLIPPING_SIZE_X:
	  return (void *)&(thePrefsStructP->RdataP->max_clip_x);
          break;

        case eMAX_CLIPPING_SIZE_Y:
	  return (void *)&(thePrefsStructP->RdataP->max_clip_y);
          break;

        case eUSE_LONG_TEXT_NAMES:
	  return (void *)&(thePrefsStructP->RdataP->long_text_names);
          break;

        case eTOOLBAR_LAYOUT:
            return (void *)(thePrefsStructP->RdataP->toolbar_layout);
            break;

        case eNEXTISUNREAD:
	  return (void *)&(thePrefsStructP->RdataP->newsNextIsUnread);
          break;
        case ePREVISUNREAD:
	  return (void *)&(thePrefsStructP->RdataP->newsPrevIsUnread);
          break;
        case eUSENEWSRC:
	  return (void *)&(thePrefsStructP->RdataP->newsUseNewsrc);
          break;
        case eNEWSRCPREFIX:
	  return (void *)(thePrefsStructP->RdataP->newsNewsrcPrefix);
          break;
        case eNEWSSUBJECTWIDTH:
	  return (void *)&(thePrefsStructP->RdataP->newsSubjectWidth);
          break;
        case eNEWSAUTHORWIDTH:
	  return (void *)&(thePrefsStructP->RdataP->newsAuthorWidth);
          break;


        case eFOCUS_FOLLOWS_MOUSE:
            return (void *)&(thePrefsStructP->RdataP->focusFollowsMouse);
            break;

        case eSESSION_HISTORY_ON_RBM:
            return (void *)&(thePrefsStructP->RdataP->sessionHistoryOnRBM);
            break;

        case eNUMBER_OF_ITEMS_IN_RBM_HISTORY:
	    return (void *)&(thePrefsStructP->RdataP->numberOfItemsInRBMHistory);
            break;

        case eHOTLIST_ON_RBM:
	    return (void *)&(thePrefsStructP->RdataP->hotlistOnRBM);
	    break;

        case eUSESHORTNEWSRC:
	  return (void *)&(thePrefsStructP->RdataP->newsUseShortNewsrc);
          break;
    }

}


/****************************************************************************
   Function: get_pref_string(long pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
char *get_pref_string(long pref_id) {
    char *tmp_string = (char *)get_pref(pref_id);

    if(tmp_string == NULL)
        return (char *)NULL;
    else if(strcmp(tmp_string, "") == 0)
        return (char *)NULL;
    else
        return (char *)tmp_string;
}

/****************************************************************************
   Function: get_pref_int(long pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
int get_pref_int(long pref_id) {
    return *(int *)get_pref(pref_id);
}

/****************************************************************************
   Function: get_pref_boolean(long pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
Boolean get_pref_boolean(long pref_id) {
    return *(Boolean *)get_pref(pref_id);
}

/****************************************************************************
   Function: get_pref_float(long pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
float get_pref_float(long pref_id) {
    return *(float *)get_pref(pref_id);
}



/****************************************************************************
   Function: set_pref_boolean(long pref_id, int value)
   Desc:     Convenience for boolean setting.
 ***************************************************************************/
void set_pref_boolean(long pref_id, int value) {

int val=value;

	set_pref(pref_id,&val);
}


/****************************************************************************
   Function: set_pref_int(long pref_id, int value)
   Desc:     Convenience for integer setting.
 ***************************************************************************/
void set_pref_int(long pref_id, int value) {

int val=value;

	set_pref(pref_id,&val);
}


/****************************************************************************
   Function: set_pref(long pref_id, void *incoming)
   Desc:     set the single preference variable denoted by pref_id, to
                 whatever incoming points to.
 ***************************************************************************/
void set_pref(long pref_id, void *incoming) {


    switch(pref_id) {

        case  eTRACK_VISITED_ANCHORS:
            thePrefsStructP->RdataP->track_visited_anchors =
                *((Boolean *)incoming);
            break;
        case  eDISPLAY_URLS_NOT_TITLES:
            thePrefsStructP->RdataP->display_urls_not_titles =
                *((Boolean *)incoming);
            break;
        case  eTRACK_POINTER_MOTION:
            thePrefsStructP->RdataP->track_pointer_motion =
                *((Boolean *)incoming);
            break;
        case  eTRACK_FULL_URL_NAMES:
            thePrefsStructP->RdataP->track_full_url_names =
                *((Boolean *)incoming);
            break;
        case  eANNOTATIONS_ON_TOP:
            thePrefsStructP->RdataP->annotations_on_top =
                *((Boolean *)incoming);
            break;
        case  eCONFIRM_DELETE_ANNOTATION:
            thePrefsStructP->RdataP->confirm_delete_annotation =
                *((Boolean *)incoming);
            break;
        case  eANNOTATION_SERVER:
            thePrefsStructP->RdataP->annotation_server =
                (char *)incoming;
            break;
        case  eRECORD_COMMAND_LOCATION:
            thePrefsStructP->RdataP->record_command_location =
                (char *)incoming;
            break;
        case  eRECORD_COMMAND:
            thePrefsStructP->RdataP->record_command =
                (char *)incoming;
            break;
        case  eRELOAD_PRAGMA_NO_CACHE:
            thePrefsStructP->RdataP->reload_pragma_no_cache =
                *((Boolean *)incoming);
            break;
        case  eSENDMAIL_COMMAND:
            thePrefsStructP->RdataP->sendmail_command =
                (char *)incoming;
            break;
        case  eEDIT_COMMAND:
            thePrefsStructP->RdataP->edit_command =
                (char *)incoming;
            break;
        case  eXTERM_COMMAND:
            thePrefsStructP->RdataP->xterm_command =
                (char *)incoming;
            break;
        case  eMAIL_FILTER_COMMAND:
            thePrefsStructP->RdataP->mail_filter_command =
                (char *)incoming;
            break;
        case  ePRIVATE_ANNOTATION_DIRECTORY:
            thePrefsStructP->RdataP->private_annotation_directory =
                (char *)incoming;
            break;
        case  eHOME_DOCUMENT:
            thePrefsStructP->RdataP->home_document =
                (char *)incoming;
            break;
        case  eTMP_DIRECTORY:
            thePrefsStructP->RdataP->tmp_directory =
                (char *)incoming;
            break;
        case  eDOCS_DIRECTORY:
            thePrefsStructP->RdataP->docs_directory =
                (char *)incoming;
            break;
        case  eDEFAULT_FONT_CHOICE:
            thePrefsStructP->RdataP->default_font_choice =
                (char *)incoming;
            break;
        case  eGLOBAL_HISTORY_FILE:
            thePrefsStructP->RdataP->global_history_file =
                (char *)incoming;
            break;
        case  eHISTORY_FILE:
            thePrefsStructP->RdataP->history_file =
                (char *)incoming;
            break;
        case  eUSE_GLOBAL_HISTORY:
            thePrefsStructP->RdataP->use_global_history =
                *((Boolean *)incoming);
            break;
        case  eDEFAULT_HOTLIST_FILE:
            thePrefsStructP->RdataP->default_hotlist_file =
                (char *)incoming;
            break;
        case  eDEFAULT_HOT_FILE:
            thePrefsStructP->RdataP->default_hot_file =
                (char *)incoming;
            break;
        case  eADD_HOTLIST_ADDS_RBM:
            thePrefsStructP->RdataP->addHotlistAddsRBM =
                *((Boolean *)incoming);
            break;
        case  eADD_RBM_ADDS_RBM:
            thePrefsStructP->RdataP->addRBMAddsRBM =
                *((Boolean *)incoming);
            break;
        case  eDOCUMENTS_MENU_SPECFILE:
            thePrefsStructP->RdataP->documents_menu_specfile =
                (char *)incoming;
            break;
        case  eCOLORS_PER_INLINED_IMAGE:
            thePrefsStructP->RdataP->colors_per_inlined_image =
                *((int *)incoming);
            break;
        case  eIMAGE_CACHE_SIZE:
            thePrefsStructP->RdataP->image_cache_size =
                *((int *)incoming);
            break;
        case  eRELOAD_RELOADS_IMAGES:
            thePrefsStructP->RdataP->reload_reloads_images =
                *((Boolean *)incoming);
            break;
        case  eREVERSE_INLINED_BITMAP_COLORS:
            thePrefsStructP->RdataP->reverse_inlined_bitmap_colors =
                *((Boolean *)incoming);
            break;
        case  eDELAY_IMAGE_LOADS:
            thePrefsStructP->RdataP->delay_image_loads =
                *((Boolean *)incoming);
            break;
        case  eDEFAULT_AUTHOR_NAME:
            thePrefsStructP->RdataP->default_author_name =
                (char *)incoming;
            break;
        case  eDEFAULT_AUTHOR_EMAIL:
            thePrefsStructP->RdataP->default_author_email =
                (char *)incoming;
            break;
        case  eSIGNATURE:
            thePrefsStructP->RdataP->signature =
                (char *)incoming;
            break;
        case  eMAIL_MODE:
            thePrefsStructP->RdataP->mail_mode =
                (char *)incoming;
            break;
        case  ePRINT_COMMAND:
            thePrefsStructP->RdataP->print_command =
                (char *)incoming;
            break;
        case  eUNCOMPRESS_COMMAND:
            thePrefsStructP->RdataP->uncompress_command =
                (char *)incoming;
            break;
        case  eGUNZIP_COMMAND:
            thePrefsStructP->RdataP->gunzip_command =
                (char *)incoming;
            break;
        case  eUSE_DEFAULT_EXTENSION_MAP:
            thePrefsStructP->RdataP->use_default_extension_map =
                *((Boolean *)incoming);
            break;
        case  eUSE_DEFAULT_TYPE_MAP:
            thePrefsStructP->RdataP->use_default_type_map =
                *((Boolean *)incoming);
            break;
        case  eGLOBAL_EXTENSION_MAP:
            thePrefsStructP->RdataP->global_extension_map =
                (char *)incoming;
            break;
        case  ePERSONAL_EXTENSION_MAP:
            thePrefsStructP->RdataP->personal_extension_map =
                (char *)incoming;
            break;
        case  eGLOBAL_TYPE_MAP:
            thePrefsStructP->RdataP->global_type_map =
                (char *)incoming;
            break;
        case  ePERSONAL_TYPE_MAP:
            thePrefsStructP->RdataP->personal_type_map =
                (char *)incoming;
            break;
        case  eTWEAK_GOPHER_TYPES:
            thePrefsStructP->RdataP->tweak_gopher_types =
                *((Boolean *)incoming);
            break;
        case  ePRINT_MODE:
            thePrefsStructP->RdataP->print_mode =
                (char *)incoming;
            break;
        case  eGUI_LAYOUT:
            thePrefsStructP->RdataP->gui_layout =
                (char *)incoming;
            break;
        case  ePRINT_BANNERS:
            thePrefsStructP->RdataP->print_banners =
                *((Boolean *)incoming);
            break;
        case  ePRINT_FOOTNOTES:
            thePrefsStructP->RdataP->print_footnotes =
                *((Boolean *)incoming);
            break;
        case  ePRINT_PAPER_SIZE_US:
            thePrefsStructP->RdataP->print_us =
                *((Boolean *)incoming);
            break;
        case  ePROXY_SPECFILE:
            thePrefsStructP->RdataP->proxy_specfile =
                (char *)incoming;
            break;
        case  eNOPROXY_SPECFILE:
            thePrefsStructP->RdataP->noproxy_specfile =
                (char *)incoming;
            break;
        case  eCCIPORT:
            thePrefsStructP->RdataP->cciPort =
                *((int *)incoming);
            break;
        case  eMAX_NUM_OF_CCI_CONNECTIONS:
            thePrefsStructP->RdataP->max_num_of_cci_connections =
                *((int *)incoming);
            break;
        case  eMAX_WAIS_RESPONSES:
            thePrefsStructP->RdataP->max_wais_responses =
                *((int *)incoming);
            break;
        case  eKIOSK:
            thePrefsStructP->RdataP->kiosk =
                *((Boolean *)incoming);
            break;
        case  eKIOSKPRINT:
            thePrefsStructP->RdataP->kioskPrint =
                *((Boolean *)incoming);
            break;
        case  eKIOSKNOEXIT:
            thePrefsStructP->RdataP->kioskNoExit =
                *((Boolean *)incoming);
            break;
        case  eKEEPALIVE:
            thePrefsStructP->RdataP->keepAlive =
                *((Boolean *)incoming);
            break;
        case  eFTP_TIMEOUT_VAL:
            thePrefsStructP->RdataP->ftp_timeout_val =
                *((int *)incoming);
            break;
        case  eENABLE_TABLES:
            thePrefsStructP->RdataP->enable_tables =
                *((Boolean *)incoming);
            break;
        case  eDEFAULT_WIDTH:
            thePrefsStructP->RdataP->default_width =
                *((int *)incoming);
            break;
        case  eDEFAULT_HEIGHT:
            thePrefsStructP->RdataP->default_height =
                *((int *)incoming);
            break;
        case  eAUTO_PLACE_WINDOWS:
            thePrefsStructP->RdataP->auto_place_windows =
                *((Boolean *)incoming);
            break;
        case  eINITIAL_WINDOW_ICONIC:
            thePrefsStructP->RdataP->initial_window_iconic =
                *((Boolean *)incoming);
            break;
        case  eTITLEISWINDOWTITLE:
            thePrefsStructP->RdataP->titleIsWindowTitle =
                *((Boolean *)incoming);
            break;
        case  eUSEICONBAR:
            thePrefsStructP->RdataP->useIconBar =
                *((Boolean *)incoming);
            break;
        case  eUSETEXTBUTTONBAR:
            thePrefsStructP->RdataP->useTextButtonBar =
                *((Boolean *)incoming);
            break;
        case  eTWIRLING_TRANSFER_ICON:
            thePrefsStructP->RdataP->twirling_transfer_icon =
                *((Boolean *)incoming);
            break;
        case  eSECURITYICON:
            thePrefsStructP->RdataP->securityIcon =
                *((Boolean *)incoming);
            break;
        case  eTWIRL_INCREMENT:
            thePrefsStructP->RdataP->twirl_increment =
                *((int *)incoming);
            break;
        case  eSAVE_MODE:
            thePrefsStructP->RdataP->save_mode =
                (char *)incoming;
            break;
        case  eHDF_MAX_IMAGE_DIMENSION:
            thePrefsStructP->RdataP->hdf_max_image_dimension =
                *((int *)incoming);
            break;
        case  eHDF_MAX_DISPLAYED_DATASETS:
            thePrefsStructP->RdataP->hdf_max_displayed_datasets =
                *((int *)incoming);
            break;
        case  eHDF_MAX_DISPLAYED_ATTRIBUTES:
            thePrefsStructP->RdataP->hdf_max_displayed_attributes =
                *((int *)incoming);
            break;
        case  eHDF_POWER_USER:
            thePrefsStructP->RdataP->hdf_power_user =
                *((Boolean *)incoming);
            break;
        case  eHDFLONGNAME:
            thePrefsStructP->RdataP->hdflongname =
                *((Boolean *)incoming);
            break;
        case  eFULL_HOSTNAME:
            thePrefsStructP->RdataP->full_hostname =
                (char *)incoming;
            break;
        case  eLOAD_LOCAL_FILE:
            thePrefsStructP->RdataP->load_local_file =
                *((int *)incoming);
            break;
        case  eEDIT_COMMAND_USE_XTERM:
            thePrefsStructP->RdataP->edit_command_use_xterm =
                *((Boolean *)incoming);
            break;
        case  eCONFIRM_EXIT:
            thePrefsStructP->RdataP->confirm_exit =
                *((Boolean *)incoming);
            break;
        case  eDEFAULT_FANCY_SELECTIONS:
            thePrefsStructP->RdataP->default_fancy_selections =
                *((Boolean *)incoming);
            break;
        case  eCATCH_PRIOR_AND_NEXT:
            thePrefsStructP->RdataP->catch_prior_and_next =
                *((Boolean *)incoming);
            break;
        case  eSIMPLE_INTERFACE:
            thePrefsStructP->RdataP->simple_interface =
                *((Boolean *)incoming);
            break;
        case  ePROTECT_ME_FROM_MYSELF:
            thePrefsStructP->RdataP->protect_me_from_myself =
                *((Boolean *)incoming);
            break;
        case  eGETHOSTBYNAME_IS_EVIL:
            thePrefsStructP->RdataP->gethostbyname_is_evil =
                *((Boolean *)incoming);
            break;
#ifdef __sgi
        case  eDEBUGGING_MALLOC:
            thePrefsStructP->RdataP->debugging_malloc =
                *((Boolean *)incoming);
            break;
#endif
        case  eUSEAFSKLOG:
            thePrefsStructP->RdataP->useAFSKlog =
                *((Boolean *)incoming);
            break;

/* New in 2.7 */

        case eSEND_REFERER:
            thePrefsStructP->RdataP->sendReferer =
                *((Boolean *)incoming);
            break;
        case eSEND_AGENT:
            thePrefsStructP->RdataP->sendAgent =
                *((Boolean *)incoming);
            break;
        case eEXPAND_URLS:
            thePrefsStructP->RdataP->expandUrls =
                *((Boolean *)incoming);
            break;
        case eEXPAND_URLS_WITH_NAME:
            thePrefsStructP->RdataP->expandUrlsWithName =
                *((Boolean *)incoming);
            break;
        case eDEFAULT_PROTOCOL:
            thePrefsStructP->RdataP->defaultProtocol =
                (char *)incoming;
            break;
        case eMETER_FOREGROUND:
            thePrefsStructP->RdataP->meterForeground =
                (char *)incoming;
            break;
        case eMETER_BACKGROUND:
            thePrefsStructP->RdataP->meterBackground =
                (char *)incoming;
            break;
        case eMETER:
            thePrefsStructP->RdataP->use_meter =
                *((Boolean *)incoming);
            break;
        case eBACKUP_FILES:
            thePrefsStructP->RdataP->backup_files =
                *((Boolean *)incoming);
            break;
        case ePIX_BASENAME:
            thePrefsStructP->RdataP->pix_basename =
                (char *)incoming;
            break;
        case ePIX_COUNT:
            thePrefsStructP->RdataP->pix_count =
                *((int *)incoming);
            break;
        case eACCEPT_LANGUAGE_STR:
            thePrefsStructP->RdataP->acceptlanguage_str =
                (char *)incoming;
            break;
        case eFTP_REDIAL:
            thePrefsStructP->RdataP->ftpRedial =
                *((int *)incoming);
            break;
        case eFTP_REDIAL_SLEEP:
            thePrefsStructP->RdataP->ftpRedialSleep =
                *((int *)incoming);
            break;
        case eFTP_FILENAME_LENGTH:
            thePrefsStructP->RdataP->ftpFilenameLength =
                *((int *)incoming);
            break;
        case eFTP_ELLIPSIS_LENGTH:
            thePrefsStructP->RdataP->ftpEllipsisLength =
                *((int *)incoming);
            break;
        case eFTP_ELLIPSIS_MODE:
            thePrefsStructP->RdataP->ftpEllipsisMode =
                *((int *)incoming);
            break;
        case eTITLE_ISWINDOW_TITLE:
            thePrefsStructP->RdataP->titleIsWindowTitle =
                *((Boolean *)incoming);
            break;
        case eUSE_SCREEN_GAMMA:
            thePrefsStructP->RdataP->useScreenGamma =
                *((Boolean *)incoming);
            break;
        case eSCREEN_GAMMA:
            thePrefsStructP->RdataP->screen_gamma =
                *((float *)incoming);
            break;
        case eDISABLEMIDDLEBUTTON:
            thePrefsStructP->RdataP->disableMiddleButton =
                *((Boolean *)incoming);
            break;
        case eHTTPTRACE:
            thePrefsStructP->RdataP->httpTrace =
                *((Boolean *)incoming);
            break;
        case eWWW2TRACE:
            thePrefsStructP->RdataP->www2Trace =
                *((Boolean *)incoming);
            break;
        case eHTMLWTRACE:
            thePrefsStructP->RdataP->htmlwTrace =
                *((Boolean *)incoming);
            break;
        case eCCITRACE:
            thePrefsStructP->RdataP->cciTrace =
                *((Boolean *)incoming);
            break;
        case eSRCTRACE:
            thePrefsStructP->RdataP->srcTrace =
                *((Boolean *)incoming);
            break;
        case eCACHETRACE:
            thePrefsStructP->RdataP->cacheTrace =
                *((Boolean *)incoming);
            break;
        case eNUTTRACE:
            thePrefsStructP->RdataP->nutTrace =
                *((Boolean *)incoming);
            break;
        case eANIMATEBUSYICON:
            thePrefsStructP->RdataP->animateBusyIcon =
                *((Boolean *)incoming);
            break;
        case eIMAGEVIEWINTERNAL:
            thePrefsStructP->RdataP->imageViewInternal =
                *((Boolean *)incoming);
            break;
        case eSPLASHSCREEN:
            thePrefsStructP->RdataP->splashScreen =
                *((Boolean *)incoming);
            break;
        case eINSTALL_COLORMAP:
            thePrefsStructP->RdataP->instamap =
                *((Boolean *)incoming);
            break;
        case eURLEXPIRED:
            thePrefsStructP->RdataP->urlExpired =
                *((Boolean *)incoming);
            break;
        case ePOPUPCASCADEMAPPINGDELAY:
            thePrefsStructP->RdataP->popupCascadeMappingDelay =
                *((int *)incoming);

	    break;

        case eUSETHREADVIEW:
	    thePrefsStructP->RdataP->newsConfigView = *((int *)incoming);
	    break;

        case eSHOWREADGROUPS:
	  thePrefsStructP->RdataP->newsShowReadGroups = *((int *)incoming);
	  break;

        case eNOTHREADJUMPING:
	  thePrefsStructP->RdataP->newsNoThreadJumping = *((int *)incoming);
          break;

        case eSHOWALLGROUPS:
	  thePrefsStructP->RdataP->newsShowAllGroups = *((int *)incoming);
          break;

        case eSHOWALLARTICLES:
	  thePrefsStructP->RdataP->newsShowAllArticles = *((int *)incoming);
          break;

        case eUSEBACKGROUNDFLUSH:
	  thePrefsStructP->RdataP->newsUseBackgroundFlush = *((int *)incoming);
          break;

        case eBACKGROUNDFLUSHTIME:
	  thePrefsStructP->RdataP->newsBackgroundFlushTime = *((int *)incoming);
          break;

        case eCLIPPING:
            thePrefsStructP->RdataP->clipping =
                *((Boolean *)incoming);
          break;

        case eMAX_CLIPPING_SIZE_X:
	  thePrefsStructP->RdataP->max_clip_x = *((int *)incoming);
          break;

        case eMAX_CLIPPING_SIZE_Y:
	  thePrefsStructP->RdataP->max_clip_y = *((int *)incoming);
          break;

        case eUSE_LONG_TEXT_NAMES:
            thePrefsStructP->RdataP->long_text_names =
                *((Boolean *)incoming);
          break;

        case eTOOLBAR_LAYOUT:
            thePrefsStructP->RdataP->toolbar_layout =
                (char *)incoming;
            break;

        case eNEXTISUNREAD:
	  thePrefsStructP->RdataP->newsNextIsUnread = *((int *)incoming);
          break;
        case ePREVISUNREAD:
	  thePrefsStructP->RdataP->newsPrevIsUnread = *((int *)incoming);
          break;
        case eUSENEWSRC:
	  thePrefsStructP->RdataP->newsUseNewsrc = *((int *)incoming);
          break;
        case eNEWSRCPREFIX:
	  thePrefsStructP->RdataP->newsNewsrcPrefix = (char *)incoming;
          break;
        case eNEWSSUBJECTWIDTH:
	  thePrefsStructP->RdataP->newsSubjectWidth = *((int *)incoming);
          break;
        case eNEWSAUTHORWIDTH:
	  thePrefsStructP->RdataP->newsAuthorWidth = *((int *)incoming);
          break;


        case eFOCUS_FOLLOWS_MOUSE:
            thePrefsStructP->RdataP->focusFollowsMouse =
                *((Boolean *)incoming);
            break;
        case eSESSION_HISTORY_ON_RBM:
            thePrefsStructP->RdataP->sessionHistoryOnRBM =
                *((Boolean *)incoming);
            break;
        case eNUMBER_OF_ITEMS_IN_RBM_HISTORY:
	    thePrefsStructP->RdataP->numberOfItemsInRBMHistory = *((int *)incoming);
	    break;
        case eHOTLIST_ON_RBM:
            thePrefsStructP->RdataP->hotlistOnRBM =
                *((Boolean *)incoming);
            break;
        case eUSESHORTNEWSRC:
	  thePrefsStructP->RdataP->newsUseShortNewsrc = *((int *)incoming);
          break;
    }

}


/****************************************************************************
 ****************************************************************************
 *                         Preference Dialog functions
 *
 ****************************************************************************
 ***************************************************************************/

/****************************************************************************
   Function: mo_preferences_dialog(mo_window *win)
   Desc:     Displays the preferences dialog
 ***************************************************************************/
void mo_preferences_dialog(mo_window *win) {


}


