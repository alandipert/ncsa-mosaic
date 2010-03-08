/*Internationalized*/
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
 *
 * The idea for internationalization was pushed into the minds at NCSA due
 *  largely from the work put forth by the Mosaic TueV at _____ in Germany.
 *
 *
 * All of the internationalization work on NCSA Mosaic was done by Scott
 *  Powers.
 *
 */

#ifndef __INTERNATIONAL_H__
#define __INTERNATIONAL_H__

typedef enum {
	/*
	 * File
	 */
	INT_FILE=0,
	INT_FILE_NEW,
	INT_FILE_CLONE,
	INT_FILE_OPENURL,
	INT_FILE_OPENLOCAL,
	INT_FILE_RELOADCURRENT,
	INT_FILE_RELOADIMAGES,
	INT_FILE_REFRESHCURRENT,
	INT_FILE_FINDINCURRENT,
	INT_FILE_VIEWSOURCE,
	INT_FILE_EDITSOURCE,
	INT_FILE_SOURCEDATE,
	INT_FILE_SAVEAS,
	INT_FILE_PRINT,
	INT_FILE_MAILTO,
	INT_FILE_CCI,
	INT_FILE_OPENDTMOUTPORT,
	INT_FILE_BROADCASTOVERDTM,
	INT_FILE_KERBEROSV4LOGIN,
	INT_FILE_KERBEROSV5LOGIN,
	INT_FILE_PROXYLIST,
	INT_FILE_NOPROXYLIST,
	INT_FILE_CLOSE,
	INT_FILE_EXITPROGRAM,

	/*
	 * Fonts Submenu
	 */
	INT_FONTS_TIMESREGULAR,
	INT_FONTS_TIMESSMALL,
	INT_FONTS_TIMESLARGE,
	INT_FONTS_HELVETICAREGULAR,
	INT_FONTS_HELVETICASMALL,
	INT_FONTS_HELVETICALARGE,
	INT_FONTS_NEWCENTURYREGULAR,
	INT_FONTS_NEWCENTURYSMALL,
	INT_FONTS_NEWCENTURYLARGE,
	INT_FONTS_LUCIDABRIGHTREGULAR,
	INT_FONTS_LUCIDABRIGHTSMALL,
	INT_FONTS_LUCIDABRIGHTLARGE,

	/*
	 * Underline Submenu
	 */
	INT_UNDERLINE_DEFAULTUNDERLINES,
	INT_UNDERLINE_LIGHTUNDERLINES,
	INT_UNDERLINE_MEDIUMUNDERLINES,
	INT_UNDERLINE_HEAVYUNDERLINES,
	INT_UNDERLINE_NOUNDERLINES,

	/*
	 * Options Menubar
	 */
	INT_OPTIONS,
	INT_OPTIONS_FANCYSELECTIONS,
	INT_OPTIONS_LOADTOLOCALDISK,
	INT_OPTIONS_TABLESUPPORT,
	INT_OPTIONS_DELAYIMAGELOADING,
	INT_OPTIONS_LOADIMAGESINCURRENT,
	INT_OPTIONS_RELOADCONFIGFILES,
	INT_OPTIONS_FLUSHIMAGECACHE,
	INT_OPTIONS_CLEARGLOBALHISTORY,
	INT_OPTIONS_FONTS,
	INT_OPTIONS_ANCHORUNDERLINES,

	/*
	 * Navigate Menubar
	 */
	INT_NAVIGATE,
	INT_NAVIGATE_BACK,
	INT_NAVIGATE_FORWARD,
	INT_NAVIGATE_HOMEDOCUMENT,
	INT_NAVIGATE_WINDOWHISTORY,
	INT_NAVIGATE_DOCUMENTLINKS,
	INT_NAVIGATE_HOTLIST,
	INT_NAVIGATE_ADDCURRENTTOHOTLIST,
	INT_NAVIGATE_INTERNETSTARTINGPOINTS,
	INT_NAVIGATE_INTERNETRESOURCESMETAINDEX,

	/*
	 * Annotate Menubar
	 */
	INT_ANNOTATE,
	INT_ANNOTATE_ANNOTATE,
	INT_ANNOTATE_AUDIOANNOTATE,
	INT_ANNOTATE_EDITTHISANNOTATION,
	INT_ANNOTATE_DELETETHISANNOTATION,

	/*
	 * News Menubar
	 */
	INT_NEWS,
	INT_NEWS_NEXT,
	INT_NEWS_PREV,
	INT_NEWS_NEXTTHREAD,
	INT_NEWS_PREVTHREAD,
	INT_NEWS_INDEX,
	INT_NEWS_LISTGROUPS,
	INT_NEWS_POST,
	INT_NEWS_FOLLOWUP,
	INT_NEWS_REPLY,
	INT_NEWS_CANCEL,
	INT_NEWS_FORMAT,
	INT_NEWS_SUBSCRIBE,
	INT_NEWS_UNSUBSCRIBE,
	INT_NEWS_CATCHUP,
	INT_NEWS_LISTALL,
	INT_NEWS_LISTSUBSCRIBED,

	/*
	 * News Format Submenu
	 */
	INT_NEWSFORMAT_THREADVIEW,
	INT_NEWSFORMAT_GROUPVIEW,
	INT_NEWSFORMAT_TREEVIEW,

	/*
	 * Help Menubar
	 */
	INT_HELP,
	INT_HELP_ABOUT,
	INT_HELP_MANUAL,
	INT_HELP_WHATSNEW,
	INT_HELP_DEMO,
	INT_HELP_ONVERSION,
	INT_HELP_ONWINDOW,
	INT_HELP_ONFAQ,
	INT_HELP_ONHTML,
	INT_HELP_ONURLS,
	INT_HELP_MAILTECHSUPPORT,
	INT_HELP_COMMENTCARD,

	/*
	 * Documents Menubar
	 */
	INT_DOCUMENTS,

	/*
	 * Push Buttons
	 */
	INT_PB_BACK,
	INT_PB_FORWARD,
	INT_PB_HOME,
	INT_PB_RELOAD,
	INT_PB_OPEN,
	INT_PB_SAVEAS,
	INT_PB_CLONE,
	INT_PB_NEW,
	INT_PB_CLOSE,

	/*
	 * Buttons
	 */
	INT_BUTTON_OPEN,
	INT_BUTTON_CLEAR,
	INT_BUTTON_DISMISS,
	INT_BUTTON_HELP,
	INT_BUTTON_MAIL,
	INT_BUTTON_MAILTO,
	INT_BUTTON_PRINT,
	INT_BUTTON_FIND,
	INT_BUTTON_RESET,
	INT_BUTTON_CLEANSLATE,
	INT_BUTTON_INCLUDEFILE,
	INT_BUTTON_DELETE,
	INT_BUTTON_COMMIT,
	INT_BUTTON_START,
	INT_BUTTON_STOP,
	INT_BUTTON_OK,
	INT_BUTTON_GOTOURL,
	INT_BUTTON_SAVE,
	INT_BUTTON_POST,
	INT_BUTTON_QUOTE,
	INT_BUTTON_ADDCURRENT,
	INT_BUTTON_REMOVE,
	INT_BUTTON_EDIT,
	INT_BUTTON_COPY,
	INT_BUTTON_INSERT,
	INT_BUTTON_UP,
	INT_BUTTON_LOAD,

	/*
	 * Security Icon
	 */
	INT_SECURITY,
	INT_SECURITY_NONE,
	INT_SECURITY_BASIC,
	INT_SECURITY_KERBEROSV4,
	INT_SECURITY_KERBEROSV5,
	INT_SECURITY_MD5,
	INT_SECURITY_DOMAIN,
	INT_SECURITY_LOGIN,
	INT_SECURITY_UNKNOWN,

	/*
	 * Protect Me From Myself
	 */
	INT_PROTECTME,
	INT_PROTECTME_YES,
	INT_PROTECTME_NO,

	/*
	 * Window Labels
	 */
	INT_WIN_TITLELABEL,
	INT_WIN_URLLABEL,

	/*
	 * String Error
	 */
	INT_STRERR_UNKNOWN,

	/*
	 * Format Strings
	 */
	INT_FORMAT_BANNERS,
	INT_FORMAT_FOOTNOTES,
	INT_FORMAT_A4,
	INT_FORMAT_US,
	INT_FORMAT_FORMAT,
	INT_FORMAT_NOMODE,
	INT_FORMAT_INVALIDMODE,

	/*
	 * Save Dialogs
	 */
	INT_SAVE_ERROR,
	INT_SAVE_ERRORTITLE,
	INT_SAVE_TITLE,
	INT_SAVE_LABEL,
	INT_SAVE_BINARYERROR,
	INT_SAVE_BINARYERRORTITLE,
	INT_SAVE_BINARYTITLE,
	INT_SAVE_BINARYLABEL,

	/*
	 * Open Dialogs
	 */
	INT_OPEN_LOCALTITLE,
	INT_OPEN_LOCALLABEL,
	INT_OPEN_TITLE,
	INT_OPEN_LABEL,
	INT_OPEN_DTMTITLE,
	INT_OPEN_DTMLABEL,

	/*
	 * Mail Dialogs
	 */
	INT_MAIL_TITLE,
	INT_MAIL_TO,
	INT_MAIL_SUBJECT,

	/*
	 * my_system Error Strings
	 */
	INT_SYSTEM_NOCOMMAND,
	INT_SYSTEM_FORKFAIL,
	INT_SYSTEM_PROGRAMFAILED,
	INT_SYSTEM_NORETBUF,
	INT_SYSTEM_FCNTLFAILED,

	/*
	 * Print Dialogs
	 */
	INT_PRINT_INFOTITLE,
	INT_PRINT_TITLE,
	INT_PRINT_COMMAND,

	/*
	 * Source Dialogs
	 */
	INT_SOURCE_TITLE,
	INT_SOURCE_LABEL,
	INT_SOURCE_YESDATE,
	INT_SOURCE_NODATE,
	INT_SOURCE_EDITOPENERRORTITLE,
	INT_SOURCE_EDITOPENERRORSTRING,
	INT_SOURCE_EDITWRITEERRORTITLE,
	INT_SOURCE_EDITWRITEERRORSTRING,

	/*
	 * Find Dialogs
	 */
	INT_FIND_RESULTTITLE,
	INT_FIND_RESULTNONE,
	INT_FIND_RESULTNOMORE,
	INT_FIND_TITLE,
	INT_FIND_STRING,
	INT_FIND_CASELESS,
	INT_FIND_BACKWARDS,

	/*
	 * Exit
	 */
	INT_EXIT_TITLE,
	INT_EXIT_STRING,

	/*
	 * Font Loading
	 */
	INT_FONTLOAD_ERRORSTRING,
	INT_FONTLOAD_ERRORTITLE,

	/*
	 * Global History
	 */
	INT_GLOBHIST_CLEARSTRING,
	INT_GLOBHIST_CLEARTITLE,

	/*
	 * Annotations
	 */
	INT_ANNOTATION_DELETESTRING,
	INT_ANNOTATION_DELETETITLE,
	INT_ANNOTATION_NOTITLE,
	INT_ANNOTATION_NOAUTHOR,
	INT_ANNOTATION_BY,
	INT_ANNOTATION_OPENERRORTITLE,
	INT_ANNOTATION_OPENERRORSTRING,
	INT_ANNOTATION_INCLUDESTRING,
	INT_ANNOTATION_INCLUDETITLE,
	INT_ANNOTATION_TITLE,
	INT_ANNOTATION_AUTHORSTRING,
	INT_ANNOTATION_TITLESTRING,
	INT_ANNOTATION_PASSWORDTOGGLE,
	INT_ANNOTATION_PASSWORDSTRING,
	INT_ANNOTATION_TEXTSTRING,

	/*
	 * Title
	 */
	INT_TITLE_NOTITLE,

	/*
	 * Audio Annotations
	 */
	INT_AUDIO_ERRORSTRING,
	INT_AUDIO_ERRORTITLE,
	INT_AUDIO_BY,
	INT_AUDIO_COMPLETEERRORSTRING,
	INT_AUDIO_COMPLETEERRORTITLE,
	INT_AUDIO_ANCHOR,
	INT_AUDIO_TITLE,
	INT_AUDIO_INSTRUCTIONS,

	/*
	 * CCI
	 */
	INT_CCI_ERRORADDRTITLE,
	INT_CCI_ERRORADDRSTRING,
	INT_CCI_PORTSTRING,
	INT_CCI_PORTTITLE,
	INT_CCI_ERRORREQTITLE,
	INT_CCI_ERRORREQSTRING,
	INT_CCI_NOSTRING,
	INT_CCI_NOTITLE,
	INT_CCI_TITLE,
	INT_CCI_ADDR,
	INT_CCI_TOGGLEACCEPT,
	INT_CCI_TOGGLEOFF,
	INT_CCI_ERRORLISTENTITLE,
	INT_CCI_ERRORLISTENSTRING,

	/*
	 * Document Links
	 */
	INT_LINKS_TITLE,
	INT_LINKS_LABEL,
	INT_LINKS_LINKSSTRING,
	INT_LINKS_IMAGESSTRING,

	/*
	 * News
	 */
	INT_NEWS_INCLUDEERRORSTRING,
	INT_NEWS_INCLUDEERRORTITLE,
	INT_NEWS_INCLUDESTRING,
	INT_NEWS_INCLUDETITLE,
	INT_NEWS_TITLE,
	INT_NEWS_FROMSTRING,
	INT_NEWS_SUBJECTSTRING,
	INT_NEWS_GROUPSSTRING,
	INT_NEWS_FOLLOWUPSTRING,
	INT_NEWS_POSTSTRING,
	INT_NEWS_ARTICLETITLESTRING,
	INT_NEWS_GROUPTITLESTRING,

	/*
	 * Gopher
	 */
	INT_GOPHER_TITLESTRING1,
	INT_GOPHER_TITLESTRING2,

	/*
	 * WAIS
	 */
	INT_WAIS_TITLESTRING1,
	INT_WAIS_TITLESTRING2,

	/*
	 * URL
	 */
	INT_URL_LOCALTITLESTRING,
	INT_URL_REMOTETITLESTRING,
	INT_URL_UNKNOWNTITLESTRING,

	/*
	 * History
	 */
	INT_HISTORY_MAILTITLE,
	INT_HISTORY_MAILTOLABEL,
	INT_HISTORY_MAILSUBJECTLABEL,
	INT_HISTORY_TITLE,
	INT_HISTORY_HISTORYLABEL,

	/*
	 * Hotlist
	 */
	INT_HOTLIST_INSERTTITLE,
	INT_HOTLIST_EDITTITLE,
	INT_HOTLIST_TITLELABEL,
	INT_HOTLIST_URLLABEL,
	INT_HOTLIST_URLNAME,
	INT_HOTLIST_LISTNAME,
	INT_HOTLIST_INSERTNAME,
	INT_HOTLIST_APPENDNAME,
	INT_HOTLIST_SAVEERROR,
	INT_HOTLIST_SAVEERRORTITLE,
	INT_HOTLIST_OPENERROR,
	INT_HOTLIST_OPENERRORTITLE,
	INT_HOTLIST_NONAME,
	INT_HOTLIST_REMOVESTARTSTRING,
	INT_HOTLIST_REMOVEENDSTRING,
	INT_HOTLIST_REMOVETITLE,
	INT_HOTLIST_MAILTITLE,
	INT_HOTLIST_MAILTOLABEL,
	INT_HOTLIST_MAILSUBJECTLABEL,
	INT_HOTLIST_SELECTERRORTITLE,
	INT_HOTLIST_SELECTERRORSTRING,
	INT_HOTLIST_REMOVEERRORTITLE,
	INT_HOTLIST_REMOVEERRORSTRING,
	INT_HOTLIST_EDITERRORTITLE,
	INT_HOTLIST_EDITERRORSTRING,
	INT_HOTLIST_COPYERRORTITLE,
	INT_HOTLIST_COPYERRORSTRING,
	INT_HOTLIST_SAVETITLE,
	INT_HOTLIST_SAVESTRING,
	INT_HOTLIST_OPENTITLE,
	INT_HOTLIST_OPENSTRING,
	INT_HOTLIST_OPENCREATENAME,
	INT_HOTLIST_OPENLOADNAME,
	INT_HOTLIST_VIEWTITLE,
	INT_SCOTTLIST_VIEWTITLE,

	/*
	 * The End...
	 */
	MAX_INTER_TYPES
} iType;

#else

#endif
