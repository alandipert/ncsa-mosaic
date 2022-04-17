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

/* This document should be included in whatever source document
   sets up the Intrinsics.  It is in a separate file so it doesn't
   clutter up that file.  So sue me. */

#ifndef __MOSAIC_XRESOURCES_H__

/* ----------------------------- X Resources ------------------------------ */
#define offset(x) XtOffset (AppDataPtr, x)

static XtResource resources[] = {

  /* default font choice from Options menu choices */
  { "defaultFontChoice", "DefaultFontChoice", XtRString, sizeof (char *),
    offset (default_font_choice), XtRString, "TimesRegular" },
  { "kiosk", "Kiosk", XtRBoolean, sizeof (Boolean),
      offset (kiosk), XtRString, "False" },
  { "kioskPrint", "KioskPrint", XtRBoolean, sizeof (Boolean),
      offset (kioskPrint), XtRString, "False" },
  { "kioskNoExit", "KioskNoExit", XtRBoolean, sizeof (Boolean),
      offset (kioskNoExit), XtRString, "False" },
  /* Whether Mosaic reads and writes global history from
     ~/.mosaic-global-history
     and thus provides persistent history tracking. */
  { "useGlobalHistory", "UseGlobalHistory", XtRBoolean, sizeof (Boolean),
    offset (use_global_history), XtRString, "True" },
  /* Whether titles will be displayed wherever URL\'s are normally
     displayed. */
  { "displayURLsNotTitles", "DisplayURLsNotTitles", XtRBoolean,
    sizeof (Boolean),
    offset (display_urls_not_titles), XtRString, "False" },
  /* Default width for a Document View window.  This will change as windows
     are cloned. */
  { "defaultWidth", "DefaultWidth", XtRInt, sizeof (int),
      offset (default_width), XtRString, "640" },
  /* Default height for a Document View window. */
  { "defaultHeight", "DefaultHeight", XtRInt, sizeof (int),
      offset (default_height), XtRString, "700" },
  /* Startup document. */
  { "homeDocument", "HomeDocument", XtRString, sizeof (char *),
      offset (home_document), XtRString,
      HOME_PAGE_DEFAULT },
  { "confirmExit", "ConfirmExit", XtRBoolean, sizeof (Boolean),
      offset (confirm_exit), XtRString, "True" },
  /* THIS USED TO BE mailCommand BUT IS NOW sendmailCommand. */
#ifdef __bsdi__
  { "sendmailCommand", "SendmailCommand", XtRString, sizeof (char *),
      offset (sendmail_command), XtRString, "/usr/sbin/sendmail -t" },
#else
  { "sendmailCommand", "SendmailCommand", XtRString, sizeof (char *),
      offset (sendmail_command), XtRString, "/usr/lib/sendmail -t" },
#endif
  /* Ignore this.  Stealth feature. */
  { "mailFilterCommand", "MailFilterCommand", XtRString, sizeof (char *),
      offset (mail_filter_command), XtRString, NULL },
  { "printCommand", "PrintCommand", XtRString, sizeof (char *),
      offset (print_command), XtRString, "lpr" },
  { "cciPort","CCIPort",XtRInt,sizeof (int),
      offset (cciPort), XtRString, "0" },
  { "maxNumCCIConnect","MaxNumCCIConnect",XtRInt,sizeof (int),
      offset (max_num_of_cci_connections), XtRString, "0" },
  { "loadLocalFile","LoadLocalFile",XtRInt,sizeof(int),
      offset (load_local_file), XtRString, "1"},
  { "editCommand", "EditCommand", XtRString, sizeof (char *),
      offset (edit_command), XtRString, NULL },
  { "editCommandUseXterm", "EditCommandUseXterm", XtRBoolean, sizeof (Boolean),
      offset (edit_command_use_xterm), XtRString, "True" },
#ifdef _AIX
  { "xtermCommand", "XtermCommand", XtRString, sizeof (char *),
      offset (xterm_command), XtRString,
      "aixterm -v",
    },
#else /* not _AIX */
  { "xtermCommand", "XtermCommand", XtRString, sizeof (char *),
      offset (xterm_command), XtRString,
      "xterm",
    },
#endif /* not _AIX */
  { "globalHistoryFile", "GlobalHistoryFile", XtRString,
      sizeof (char *),
	offset (global_history_file), XtRString, ".mosaic/global-history" }, // SAM
  { "historyFile", "HistoryFile", XtRString,
      sizeof (char *),
	offset (history_file), XtRString, ".mosaic/x-history" }, // SAM
  { "defaultHotlistFile", "DefaultHotlistFile", XtRString,
      sizeof (char *),
	offset (default_hotlist_file), XtRString, ".mosaic/hotlist-default" }, // SAM
  { "defaultHotFile", "DefaultHotFile", XtRString,
      sizeof (char *),
	offset (default_hot_file), XtRString, ".mosaic/hot.html" }, // SAM
  { "addHotlistAddsRBM", "AddHotlistAddsRBM", XtRBoolean, sizeof (Boolean),
      offset (addHotlistAddsRBM), XtRString, "True" },
  { "addRBMAddsRBM", "AddRBMAddsRBM", XtRBoolean, sizeof (Boolean),
      offset (addRBMAddsRBM), XtRString, "True" },
  { "personalAnnotationDirectory", "PersonalAnnotationDirectory", XtRString,
      sizeof (char *),
      offset (private_annotation_directory), XtRString,
	".mosaic/personal-annotations" }, // SAM
  /* Whether selections should be fancy, by default. */
  { "fancySelections", "FancySelections", XtRBoolean,
      sizeof (Boolean), offset (default_fancy_selections),
      XtRString, "False" },
  { "defaultAuthorName", "DefaultAuthorName", XtRString, sizeof (char *),
      offset (default_author_name), XtRString, NULL },
  { "defaultAuthorEmail", "DefaultAuthorEmail", XtRString, sizeof (char *),
      offset (default_author_email), XtRString, NULL },
  { "signature", "Signature", XtRString, sizeof (char *),
      offset (signature), XtRString, NULL },
  { "annotationsOnTop", "AnnotationsOnTop", XtRBoolean, sizeof (Boolean),
      offset (annotations_on_top), XtRString, "False" },

  { "colorsPerInlinedImage", "ColorsPerInlinedImage", XtRInt, sizeof (int),
      offset (colors_per_inlined_image), XtRString, "50" },
  { "trackVisitedAnchors", "TrackVisitedAnchors", XtRBoolean, sizeof (Boolean),
      offset (track_visited_anchors), XtRString, "True" },

  { "uncompressCommand", "UncompressCommand", XtRString, sizeof (char *),
      offset (uncompress_command), XtRString, "uncompress" },
  { "gunzipCommand", "GunzipCommand", XtRString,
      sizeof (char *), offset (gunzip_command), XtRString, "gunzip -f -n" },

#if defined(__hpux)
  { "recordCommandLocation", "RecordCommandLocation", XtRString,
      sizeof (char *), offset (record_command_location), XtRString,
      "/usr/audio/bin/srecorder" },
#else
#if defined(__sgi)
  { "recordCommandLocation", "RecordCommandLocation", XtRString,
      sizeof (char *), offset (record_command_location), XtRString,
      "/usr/sbin/recordaiff" },
#else
#if defined (sun)
  { "recordCommandLocation", "RecordCommandLocation", XtRString,
      sizeof (char *), offset (record_command_location), XtRString,
      "/usr/demo/SOUND/record" },
#else
  { "recordCommandLocation", "RecordCommandLocation", XtRString,
      sizeof (char *), offset (record_command_location), XtRString,
      "/bin/true" },
#endif /* if sun */
#endif /* if sgi */
#endif /* ifdef */

#ifdef __hpux
  { "recordCommand", "RecordCommand", XtRString, sizeof (char *),
      offset (record_command), XtRString,
      "srecorder -au" },
#else
#if defined(__sgi)
  { "recordCommand", "RecordCommand", XtRString, sizeof (char *),
      offset (record_command), XtRString,
      "recordaiff -n 1 -s 8 -r 8000" },
#else
#if defined (sun)
  { "recordCommand", "RecordCommand", XtRString, sizeof (char *),
      offset (record_command), XtRString,
      "record" },
#else
  { "recordCommand", "RecordCommand", XtRString, sizeof (char *),
      offset (record_command), XtRString,
      "true" },
#endif /* if sun */
#endif /* if sgi */
#endif /* ifdef */

  { "gethostbynameIsEvil", "GethostbynameIsEvil", XtRBoolean, sizeof (Boolean),
      offset (gethostbyname_is_evil), XtRString, "False" },
  { "autoPlaceWindows", "AutoPlaceWindows", XtRBoolean, sizeof (Boolean),
      offset (auto_place_windows), XtRString, "True" },
  { "initialWindowIconic", "InitialWindowIconic", XtRBoolean, sizeof (Boolean),
      offset (initial_window_iconic), XtRString, "False" },

  { "tmpDirectory", "TmpDirectory", XtRString, sizeof (char *),
      offset (tmp_directory), XtRString, NULL },
  { "annotationServer", "AnnotationServer", XtRString, sizeof (char *),
      offset (annotation_server), XtRString, NULL },
  { "catchPriorAndNext", "CatchPriorAndNext", XtRBoolean, sizeof (Boolean),
      offset (catch_prior_and_next), XtRString, "True" },

  { "fullHostname", "FullHostname", XtRString, sizeof (char *),
      offset (full_hostname), XtRString, NULL },
  { "reverseInlinedBitmapColors", "ReverseInlinedBitmapColors", XtRBoolean,
      sizeof (Boolean),
      offset (reverse_inlined_bitmap_colors), XtRString, "False" },
  { "confirmDeleteAnnotation", "ConfirmDeleteAnnotation",
      XtRBoolean, sizeof (Boolean),
      offset (confirm_delete_annotation), XtRString, "True"},
  { "tweakGopherTypes", "TweakGopherTypes", XtRBoolean, sizeof (Boolean),
      offset (tweak_gopher_types), XtRString, "True" },

  { "guiLayout", "GuiLayout", XtRString, sizeof (char *),
    offset(gui_layout), XtRString, NULL},
  /* --- new in 2.0 --- */
  { "trackPointerMotion", "TrackPointerMotion", XtRBoolean, sizeof (Boolean),
      offset (track_pointer_motion), XtRString, "True" },
  { "trackFullURLs", "TrackFullURLs", XtRBoolean, sizeof (Boolean),
      offset (track_full_url_names), XtRString, "True" },

  { "hdfMaxImageDimension", "HdfMaxImageDimension", XtRInt, sizeof (int),
      offset (hdf_max_image_dimension), XtRString, "400" },
  { "hdfMaxDisplayedDatasets", "HdfMaxDisplayedDatasets", XtRInt, sizeof (int),
      offset (hdf_max_displayed_datasets), XtRString, "15" },
  { "hdfMaxDisplayedAttributes", "HdfMaxDisplayedAttributes", XtRInt, sizeof (int),
      offset (hdf_max_displayed_attributes), XtRString, "10" },
  { "hdfPowerUser", "HdfPowerUser", XtRBoolean, sizeof (Boolean),
      offset (hdf_power_user), XtRString, "False" },

  { "docsDirectory", "DocsDirectory", XtRString, sizeof (char *),
      offset (docs_directory), XtRString, NULL },

  { "documentsMenuSpecfile", "DocumentsMenuSpecfile", XtRString, sizeof (char *),
      offset (documents_menu_specfile), XtRString,
      "/usr/local/lib/mosaic/documents.menu" },

  { "reloadReloadsImages", "ReloadReloadsImages", XtRBoolean, sizeof (Boolean),
      offset (reload_reloads_images), XtRString, "False" },
  { "reloadPragmaNoCache", "ReloadPragmaNoCache", XtRBoolean, sizeof (Boolean),
      offset (reload_pragma_no_cache), XtRString, "False" },
  { "simpleInterface", "SimpleInterface", XtRBoolean, sizeof (Boolean),
      offset (simple_interface), XtRString, "False" },

  { "maxWaisResponses", "MaxWaisResponses", XtRInt, sizeof (int),
      offset (max_wais_responses), XtRString, "200" },
  { "delayImageLoads", "DelayImageLoads", XtRBoolean, sizeof (Boolean),
      offset (delay_image_loads), XtRString, "False" },
/*SWP*/
  { "enableTables", "EnableTables", XtRBoolean, sizeof(Boolean),
      offset(enable_tables), XtRString, "False"},
  { "disableMiddleButton", "DisableMiddleButton", XtRBoolean, sizeof(Boolean),
      offset(disableMiddleButton), XtRString, "False"},

  { "useDefaultExtensionMap", "UseDefaultExtensionMap",
      XtRBoolean, sizeof (Boolean),
      offset (use_default_extension_map), XtRString, "True" },
  { "globalExtensionMap", "GlobalExtensionMap",
      XtRString, sizeof (char *),
      offset (global_extension_map), XtRString, GLOBAL_EXTENSION_MAP },
  { "personalExtensionMap", "PersonalExtensionMap",
      XtRString, sizeof (char *),
      offset (personal_extension_map), XtRString, ".mime.types" },

  { "useDefaultTypeMap", "UseDefaultTypeMap",
      XtRBoolean, sizeof (Boolean),
      offset (use_default_type_map), XtRString, "True" },
  { "globalTypeMap", "GlobalTypeMap",
      XtRString, sizeof (char *),
      offset (global_type_map), XtRString, GLOBAL_TYPE_MAP },
  { "personalTypeMap", "PersonalTypeMap",
      XtRString, sizeof (char *),
      offset (personal_type_map), XtRString, ".mailcap" },

  { "twirlingTransferIcon", "TwirlingTransferIcon",
      XtRBoolean, sizeof (Boolean),
      offset (twirling_transfer_icon), XtRString, "True" },
  { "twirlIncrement", "TwirlIncrement", XtRInt, sizeof (int),
      offset (twirl_increment), XtRString, "4096" },

  { "securityIcon", "securityIcon",
      XtRBoolean, sizeof (Boolean),
      offset (securityIcon), XtRString, "True" },

  { "imageCacheSize", "ImageCacheSize", XtRInt, sizeof (int),
      offset (image_cache_size), XtRString, "2048" },

  { "protectMeFromMyself", "ProtectMeFromMyself",
      XtRBoolean, sizeof (Boolean),
      offset (protect_me_from_myself), XtRString, "False" },

  { "printMode", "PrintMode", XtRString, sizeof (char *),
      offset (print_mode), XtRString, "plain" },

  { "mailMode", "MailMode", XtRString, sizeof (char *),
      offset (mail_mode), XtRString, "plain" },

  { "saveMode", "SaveMode", XtRString, sizeof (char *),
      offset (save_mode), XtRString, "plain" },

  { "printBanners", "PrintBanners",
      XtRBoolean, sizeof (Boolean),
      offset (print_banners), XtRString, "True" },

  { "printFootnotes", "PrintFootnotes",
      XtRBoolean, sizeof (Boolean),
      offset (print_footnotes), XtRString, "True" },

  { "printPaperSizeUS", "PrintPaperSizeUS",
      XtRBoolean, sizeof (Boolean),
      offset (print_us), XtRString, "True" },

  { "useAFSKlog", "UseAFSKlog",
      XtRBoolean, sizeof(Boolean),
      offset(useAFSKlog), XtRString,"False" },

#ifdef __sgi
  { "debuggingMalloc", "DebuggingMalloc", XtRBoolean, sizeof (Boolean),
      offset (debugging_malloc), XtRString, "False" },
#endif

          /* new in 2.7 */
  { "clipping", "Clipping", XtRBoolean, sizeof (Boolean),
      offset (clipping), XtRString, "True" },

  { "maxClippingSizeX", "MaxClippingSizeX", XtRInt, sizeof (int),
      offset (max_clip_x), XtRString, "-1" },

  { "maxClippingSizeY", "MaxClippingSizeY", XtRInt, sizeof (int),
      offset (max_clip_y), XtRString, "-1" },

  { "useLongTextNames", "UseLongTextNames", XtRBoolean, sizeof (Boolean),
      offset (long_text_names), XtRString, "False" },

  { "toolbarLayout", "ToolbarLayout", XtRString, sizeof (char *),
      offset (toolbar_layout), XtRString, NULL},

  { "installColormap", "InstallColormap", XtRBoolean, sizeof (Boolean),
      offset (instamap), XtRString, "False" },

  { "splashScreen", "SplashScreen", XtRBoolean, sizeof (Boolean),
      offset (splashScreen), XtRString, "True" },

  { "imageViewInternal", "ImageViewInternal", XtRBoolean, sizeof (Boolean),
      offset (imageViewInternal), XtRString, "False" },

  { "urlExpired", "UrlExpired", XtRInt, sizeof (int),
      offset (urlExpired), XtRString, "30" },

  { "httpTrace", "HttpTrace", XtRBoolean, sizeof (Boolean),
      offset (httpTrace), XtRString, "False" },

  { "www2Trace", "Www2Trace", XtRBoolean, sizeof (Boolean),
      offset (www2Trace), XtRString, "False" },

  { "htmlwTrace", "HtmlwTrace", XtRBoolean, sizeof (Boolean),
      offset (htmlwTrace), XtRString, "False" },

  { "cciTrace", "CciTrace", XtRBoolean, sizeof (Boolean),
      offset (cciTrace), XtRString, "False" },

  { "srcTrace", "SrcTrace", XtRBoolean, sizeof (Boolean),
      offset (srcTrace), XtRString, "False" },

  { "cacheTrace", "CacheTrace", XtRBoolean, sizeof (Boolean),
      offset (cacheTrace), XtRString, "False" },

  { "nutTrace", "NutTrace", XtRBoolean, sizeof (Boolean),
      offset (nutTrace), XtRString, "False" },

  { "animateBusyIcon", "AnimateBusyIcon", XtRBoolean, sizeof (Boolean),
      offset (animateBusyIcon), XtRString, "True" },

  { "sendReferer", "SendReferer", XtRBoolean, sizeof (Boolean),
      offset (sendReferer), XtRString, "True" },

  { "sendAgent", "SendAgent", XtRBoolean, sizeof (Boolean),
      offset (sendAgent), XtRString, "True" },

  { "expandUrls", "ExpandUrls", XtRBoolean, sizeof (Boolean),
      offset (expandUrls), XtRString, "True" },

  { "expandUrlsWithName", "expandUrlsWithName", XtRBoolean, sizeof (Boolean),
      offset (expandUrlsWithName), XtRString, "True" },

  { "defaultProtocol", "DefaultProtocol", XtRString, sizeof (char *),
      offset (defaultProtocol), XtRString, "http"},

  { "meterForeground", "MeterForeground", XtRString, sizeof (char *),
      offset (meterForeground), XtRString, "#FFFF00000000"},

  { "meterBackground", "MeterBackground", XtRString, sizeof (char *),
      offset (meterBackground), XtRString, "#2F2F4F4F4F4F"},

  { "meterFontForeground", "MeterFontForeground", XtRString, sizeof (char *),
      offset (meterFontForeground), XtRString, "#FFFFFFFFFFFF"},

  { "meterFontBackground", "MeterFontBackground", XtRString, sizeof (char *),
      offset (meterFontBackground), XtRString, "#000000000000"},

  { "meter", "Meter", XtRBoolean, sizeof (Boolean),
      offset (use_meter), XtRString, "True" },

  { "backupDataFiles", "BackupDataFiles", XtRBoolean, sizeof (Boolean),
      offset (backup_files), XtRString, "True" },

/* Icon Animation Stuff - BJS */
  { "iconPixBasename", "IconPixBasename", XtRString, sizeof (char *),
      offset (pix_basename), XtRString, "default"},

  { "iconPixCount", "IconPixCount", XtRInt, sizeof (int),
      offset (pix_count), XtRString, "0" },

/* Accept-Language stuff - BJS */
  { "acceptLanguage", "AcceptLanguage", XtRString, sizeof (char *),
      offset (acceptlanguage_str), XtRString, NULL },

  { "ftpTimeoutVal", "FtpTimeoutVal", XtRInt, sizeof (int),
      offset (ftp_timeout_val), XtRString, "90" },

  { "ftpRedial", "FtpRedial", XtRInt, sizeof (int),
      offset (ftpRedial), XtRString, "10" },

  { "ftpRedialSleep", "FtpRedialSleep", XtRInt, sizeof (int),
      offset (ftpRedialSleep), XtRString, "3" },

  { "ftpFilenameLength", "FtpFilenameLength", XtRInt, sizeof (int),
      offset (ftpFilenameLength), XtRString, "18" },

  { "ftpEllipsisLength", "FtpEllipsisLength", XtRInt, sizeof (int),
      offset (ftpEllipsisLength), XtRString, "3" },

  { "ftpEllipsisMode", "FtpEllipsisMode", XtRInt, sizeof (int),
      offset (ftpEllipsisMode), XtRString, "2" },

  { "hdfLongName", "HdfLongName", XtRBoolean, sizeof (Boolean),
      offset (hdflongname), XtRString, "False" },

  { "titleIsWindowTitle", "TitleIsWindowTitle", XtRBoolean, sizeof (Boolean),
      offset (titleIsWindowTitle), XtRString, "True" },

  { "useIconBar", "UseIconBar", XtRBoolean, sizeof (Boolean),
      offset (useIconBar), XtRString, "False" },

  { "useTextButtonBar", "UseTextButtonBar", XtRBoolean, sizeof (Boolean),
      offset (useTextButtonBar), XtRString, "True" },

  { "proxySpecfile", "ProxySpecfile", XtRString, sizeof (char *),
      offset (proxy_specfile), XtRString,
      "/usr/local/lib/mosaic/proxy" },

  { "noproxySpecfile", "NoproxySpecfile", XtRString, sizeof (char *),
      offset (noproxy_specfile), XtRString,
      "/usr/local/lib/mosaic/no_proxy" },

  { "useScreenGamma", "UseScreenGamma", XtRBoolean, sizeof (Boolean),
      offset (useScreenGamma), XtRString, "False" },

  { "screenGamma", "ScreenGamma", XtRFloat, sizeof (float),
      offset (screen_gamma), XtRString, "2.2" },

  { "popupCascadeMappingDelay", "PopupCascadeMappingDelay", XtRInt,
    sizeof(int), offset(popupCascadeMappingDelay), XtRString, "500" },

  { "frameHack", "FrameHack", XtRBoolean, sizeof (Boolean),
    offset (frame_hack), XtRString, "False" },

  /* New news stuff in B4 */
  { "newsUseThreadView", "NewsUseThreadView", XtRBoolean,
    sizeof(Boolean), offset(newsConfigView), XtRString, "True" },

  { "newsNoThreadJumping", "NewsNoThreadJumping", XtRBoolean,
    sizeof(Boolean), offset(newsNoThreadJumping), XtRString, "True" },

  { "newsShowAllGroups", "NewsShowAllGroups", XtRBoolean,
    sizeof(Boolean), offset(newsShowAllGroups), XtRString, "False" },

  { "newsShowReadGroups", "NewsShowReadGroups", XtRBoolean,
    sizeof(Boolean), offset(newsShowReadGroups), XtRString, "False" },

  { "newsShowAllArticles", "NewsShowAllArticles", XtRBoolean,
    sizeof(Boolean), offset(newsShowAllArticles), XtRString, "True" },

  { "newsUseBackgroundFlush", "NewsUseBackgroundFlush", XtRBoolean,
    sizeof(Boolean), offset(newsUseBackgroundFlush), XtRString, "True" },

  { "newsBackgroundFlushTime", "NewsBackgroundFlushTime", XtRInt,
    sizeof(int), offset(newsBackgroundFlushTime), XtRString, "300" },

  /* New in 2.7b5 */
  { "newsPrevIsPrevUnread", "NewsPrevIsPrevUnread", XtRBoolean,
    sizeof(Boolean), offset(newsPrevIsUnread), XtRString, "False" },

  { "newsNextIsNextUnread", "NewsNextIsNextUnread", XtRBoolean,
    sizeof(Boolean), offset(newsNextIsUnread), XtRString, "True" },

  { "newsNewsrcPrefix", "NewsNewsrcPrefix", XtRString,
    sizeof(char *), offset(newsNewsrcPrefix), XtRString, ".newsrc" },

  { "newsUseNewsrc", "NewsUseNewsrc", XtRBoolean,
    sizeof(Boolean), offset(newsUseNewsrc), XtRString, "True" },

  { "newsSubjectWidth", "NewsSubjectWidth", XtRInt,
    sizeof(int), offset(newsSubjectWidth), XtRString, "38" },

  { "newsAuthorWidth", "NewsAuthorWidth", XtRInt,
    sizeof(int), offset(newsAuthorWidth), XtRString, "30" },

  { "focusFollowsMouse", "FocusFollowsMouse", XtRBoolean,
    sizeof(Boolean), offset(focusFollowsMouse), XtRString, "False" },

  { "sessionHistoryOnRBM", "SessionHistoryOnRBM", XtRBoolean,
    sizeof(Boolean), offset(sessionHistoryOnRBM), XtRString, "True" },

  { "numberOfItemsInRBMHistory", "NumberOfItemsInRBMHistory", XtRInt,
    sizeof(int), offset(numberOfItemsInRBMHistory), XtRString, "12" },

  { "hotlistOnRBM", "HotlistOnRBM", XtRBoolean,
    sizeof(Boolean), offset(hotlistOnRBM), XtRString, "True" },

  { "newsUseShortNewsrc", "NewsUseShortNewsrc", XtRBoolean,
    sizeof(Boolean), offset(newsUseShortNewsrc), XtRString, "False" },
};

#undef offset

static XrmOptionDescRec options[] = {
  {"-fn",     "*fontList",            XrmoptionSepArg, NULL},
  {"-ft",     "*XmText*fontList",     XrmoptionSepArg, NULL},
  {"-fm",     "*menubar*fontList",    XrmoptionSepArg, NULL},
  {"-home",   "*homeDocument",        XrmoptionSepArg, NULL},
  {"-ngh",    "*useGlobalHistory",    XrmoptionNoArg,  "False"},
  /* Let Xt strip out -mono from stuff it considers interesting. */
  {"-mono",   "*nothingUseful",       XrmoptionNoArg,  "True"},
  {"-color",  "*nothingUseful",       XrmoptionNoArg,  "True"},
  {"-ghbnie", "*gethostbynameIsEvil", XrmoptionNoArg,  "True"},
  {"-iconic", "*initialWindowIconic", XrmoptionNoArg,  "True"},
  {"-i",      "*initialWindowIconic", XrmoptionNoArg,  "True"},
  /* New in 1.1 */
  /* -nd isn't documented since defaults in the widget still take effect,
     so the benefits of using it are kinda iffy (as if they weren't
     anyway)... */
  {"-nd",     "*nothingUseful",       XrmoptionNoArg,  "True"},
  {"-tmpdir", "*tmpDirectory",        XrmoptionSepArg, NULL},
  {"-dil",    "*delayImageLoads",     XrmoptionNoArg,  "True"},
  {"-ics",    "*imageCacheSize",      XrmoptionSepArg, NULL},
  {"-protect","*protectMeFromMyself", XrmoptionNoArg,  "True"},
  {"-kraut",  "*mailFilterCommand",   XrmoptionNoArg,  "kraut"},
#ifdef __sgi
  {"-dm",     "*debuggingMalloc",     XrmoptionNoArg,  "True"},
#endif
  {"-kiosk",  "*kiosk",               XrmoptionNoArg,  "True"},
  {"-kioskPrint",  "*kioskPrint",     XrmoptionNoArg,  "True"},
  {"-kioskNoExit",  "*kioskNoExit",   XrmoptionNoArg,  "True"},
  {"-cciPort",  "*cciPort",   	      XrmoptionSepArg,  "0"},
  {"-maxNumCCIConnect",  "*maxNumCCIConnect",  XrmoptionSepArg,  "0"},
  {"-install",  "*nothingUseful",     XrmoptionNoArg,  "True"},
};

static String color_resources[] = {
  "*XmLabel*fontList:   		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*XmLabelGadget*fontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*XmScale*fontList:   		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*XmBulletinBoard*labelFontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*optionmenu.XmLabelGadget*fontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*XmPushButton*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmPushButtonGadget*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmToggleButton*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmToggleButtonGadget*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*optionmenu*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmIconGadget*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmBulletinBoard*buttonFontList: -*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*menubar*fontList:   		-*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmPushButton*fontList:  -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmLabelGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmPushButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmCascadeButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmCascadeButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmCascadeButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmCascadeButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmToggleButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmToggleButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*pulldownmenu*fontList:	-*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmList*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmText.fontList:      -*-lucidatypewriter-medium-r-normal-*-14-*-iso10646-1",
  "*XmTextField.fontList: -*-lucidatypewriter-medium-r-normal-*-14-*-iso10646-1",

  "*optionmenu*marginHeight: 	0",
  "*optionmenu*marginTop: 		5",
  "*optionmenu*marginBottom: 	5",
  "*optionmenu*marginWidth: 	5",
  "*pulldownmenu*XmPushButton*marginHeight:	1",
  "*pulldownmenu*XmPushButton*marginWidth:	1",
  "*pulldownmenu*XmPushButton*marginLeft:	3",
  "*pulldownmenu*XmPushButton*marginRight:	3",
  "*XmList*listMarginWidth:        3",
  "*menubar*marginHeight: 		1",
  "*menubar.marginHeight: 		0",
  "*menubar*marginLeft:  		1",
  "*menubar.spacing:  		7",
  "*XmMenuShell*marginLeft:  	3",
  "*XmMenuShell*marginRight:  	4",
  "*XmMenuShell*XmToggleButtonGadget*spacing: 	 2",
  "*XmMenuShell*XmToggleButtonGadget*marginHeight:  0",
  "*XmMenuShell*XmToggleButtonGadget*indicatorSize: 12",
  "*XmMenuShell*XmLabelGadget*marginHeight: 4",
  "*XmToggleButtonGadget*spacing: 	4",
  "*XmToggleButton*spacing: 	4",
  "*XmScrolledWindow*spacing: 	0",
  "*XmScrollBar*width: 		        18",
  "*XmScrollBar*height: 		18",
  "*Hbar*height:                        22",
  "*Vbar*width:                         22",
  "*XmScale*scaleHeight: 		20",
  "*XmText*marginHeight:		4",
  "*fsb*XmText*width:                   420",
  "*fsb*XmTextField*width:                   420",
  "*fillOnSelect:			True",
  "*visibleWhenOff:		        True",
  "*XmText*highlightThickness:		0",
  "*XmTextField*highlightThickness:	0",
  "*XmPushButton*highlightThickness:	0",
  "*XmScrollBar*highlightThickness:     0",
  "*highlightThickness:	                0",
  /* "*geometry:                           +400+200", */
  /*  "*keyboardFocusPolicy:                pointer",*/

  "*TitleFont: -adobe-times-bold-r-normal-*-24-*-*-*-*-*-iso10646-1",
  "*Font: -adobe-times-medium-r-normal-*-17-*-*-*-*-*-iso10646-1",
  "*ItalicFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso10646-1",
  "*BoldFont: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso10646-1",
  "*FixedFont: -adobe-courier-medium-r-normal-*-17-*-*-*-*-*-iso10646-1",
  "*Header1Font: -adobe-times-bold-r-normal-*-24-*-*-*-*-*-iso10646-1",
  "*Header2Font: -adobe-times-bold-r-normal-*-18-*-*-*-*-*-iso10646-1",
  "*Header3Font: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso10646-1",
  "*Header4Font: -adobe-times-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*Header5Font: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso10646-1",
  "*Header6Font: -adobe-times-bold-r-normal-*-10-*-*-*-*-*-iso10646-1",
  "*AddressFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso10646-1",
  "*PlainFont: -adobe-courier-medium-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*ListingFont: -adobe-courier-medium-r-normal-*-12-*-*-*-*-*-iso10646-1",
  "*SupSubFont: -adobe-times-medium-r-normal-*-10-*-*-*-*-*-iso10646-1",
  "*MeterFont: -adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*",
  "*ToolbarFont: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso10646-1",
  "*AnchorUnderlines:                   1",
  "*VisitedAnchorUnderlines:            1",
  "*DashedVisitedAnchorUnderlines:      True",
  "*VerticalScrollOnRight:              True",

#ifdef __sgi
  "*Foreground:			 	#000000000000",
  "*XmScrollBar*Foreground:             #afafafafafaf",
  "*Background:                         #afafafafafaf",
  "*XmList*Background:     		#afafafafafaf",
  "*XmText*Background:	 	        #afafafafafaf",
  "*TroughColor:                        #545454545454",
  "*XmSelectionBox*Background:	 	#afafafafafaf",
  "*XmMessageBox*Background:	 	#afafafafafaf",
  "*XmLabel*Foreground:                 #1d1d15155b5b",
  "*XmToggleButton*Foreground:          #1d1d15155b5b",
  "*XmPushButton*Foreground:            #5b5b00000000",
  "*logo*Foreground:                    #1d1d15155b5b",
  "*searchindex_button*Foreground:      #1d1d15155b5b",
  "*XmTextField*Background: 		#8c8c8c8c8c8c",
  "*SelectColor:			#ffffffff0000",
  "*HighlightColor:		 	#afafafafafaf",

  "*TopShadowColor:                     #dfdfdfdfdfdf",
  "*XmList*TopShadowColor:              #dfdfdfdfdfdf",
  "*XmText*TopShadowColor:              #dfdfdfdfdfdf",
  "*XmSelectionBox*TopShadowColor:      #dfdfdfdfdfdf",
  "*XmMessageBox*TopShadowColor:        #dfdfdfdfdfdf",

  "*visitedAnchorColor:                 #272705055b5b",
  "*anchorColor:                        #00000000b0b0",
  "*activeAnchorFG:                     #ffff00000000",
  "*activeAnchorBG:                     #afafafafafaf",
#else /* not sgi */
  "*Foreground:			 	#000000000000",
  "*XmScrollBar*Foreground:             #bfbfbfbfbfbf",
  "*XmLabel*Foreground:                 #1d1d15155b5b",
  "*XmToggleButton*Foreground:          #1d1d15155b5b",
  "*XmPushButton*Foreground:            #5b5b00000000",
  "*logo*Foreground:                    #1d1d15155b5b",
  "*searchindex_button*Foreground:      #1d1d15155b5b",

  "*Background:                         #bfbfbfbfbfbf",

  "*XmList*Background:     		#bfbfbfbfbfbf",
  "*XmText*Background:	 	        #bfbfbfbfbfbf",
  "*XmSelectionBox*Background:	 	#bfbfbfbfbfbf",
  "*XmMessageBox*Background:	 	#bfbfbfbfbfbf",
  "*XmTextField*Background: 		#9c9c9c9c9c9c",

  "*TopShadowColor:                     #e7e7e7e7e7e7",
  "*XmList*TopShadowColor:              #e7e7e7e7e7e7",
  "*XmText*TopShadowColor:              #e7e7e7e7e7e7",
  "*XmSelectionBox*TopShadowColor:      #e7e7e7e7e7e7",
  "*XmMessageBox*TopShadowColor:        #e7e7e7e7e7e7",

  "*TroughColor:                        #646464646464",
  "*SelectColor:			#ffffffff0000",
  "*HighlightColor:		 	#bfbfbfbfbfbf",

  /* Remember to update this in the app-defaults file. */
  "*visitedAnchorColor:                 #3f3f0f0f7b7b",
  "*anchorColor:                        #00000000b0b0",
  "*activeAnchorFG:                     #ffff00000000",
  "*activeAnchorBG:                     #bfbfbfbfbfbf",
#endif
  /* Disable Motif Drag-N-Drop - BJS */
  "*dragInitiatorProtocolStyle: XmDRAG_NONE",
  "*dragReceiverProtocolStyle:  XmDRAG_NONE",

  NULL,
};

static String mono_resources[] = {
  "*XmLabel*fontList:   		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*XmLabelGadget*fontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*XmScale*fontList:   		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*XmBulletinBoard*labelFontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*optionmenu.XmLabelGadget*fontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*XmPushButton*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmPushButtonGadget*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmToggleButton*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmToggleButtonGadget*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*optionmenu*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmIconGadget*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmBulletinBoard*buttonFontList: -*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*menubar*fontList:   		-*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmPushButton*fontList:  -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmLabelGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmPushButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmCascadeButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmCascadeButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmCascadeButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmCascadeButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmToggleButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmMenuShell*XmToggleButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*pulldownmenu*fontList:	-*-helvetica-bold-o-normal-*-14-*-iso10646-1",
  "*XmList*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso10646-1",
  "*XmText.fontList:      -*-lucidatypewriter-medium-r-normal-*-14-*-iso10646-1",
  "*XmTextField.fontList: -*-lucidatypewriter-medium-r-normal-*-14-*-iso10646-1",

  "*optionmenu*marginHeight: 	0",
  "*optionmenu*marginTop: 		5",
  "*optionmenu*marginBottom: 	5",
  "*optionmenu*marginWidth: 	5",
  "*pulldownmenu*XmPushButton*marginHeight:	1",
  "*pulldownmenu*XmPushButton*marginWidth:	1",
  "*pulldownmenu*XmPushButton*marginLeft:	3",
  "*pulldownmenu*XmPushButton*marginRight:	3",
  "*XmList*listMarginWidth:        3",
  "*menubar*marginHeight: 		1",
  "*menubar.marginHeight: 		0",
  "*menubar*marginLeft:  		1",
  "*menubar.spacing:  		7",
  "*XmMenuShell*marginLeft:  	3",
  "*XmMenuShell*marginRight:  	4",
  "*XmMenuShell*XmToggleButtonGadget*spacing: 	 2",
  "*XmMenuShell*XmToggleButtonGadget*marginHeight:  0",
  "*XmMenuShell*XmToggleButtonGadget*indicatorSize: 12",
  "*XmMenuShell*XmLabelGadget*marginHeight: 4",
  "*XmToggleButtonGadget*spacing: 	4",
  "*XmToggleButton*spacing: 	4",
  "*XmScrolledWindow*spacing: 	0",
  "*XmScrollBar*width: 		        18",
  "*XmScrollBar*height: 		18",
  "*Hbar*height:                        22",
  "*Vbar*width:                         22",
  "*XmScale*scaleHeight: 		20",
  "*XmText*marginHeight:		4",
  "*fsb*XmText*width:                   420",
  "*fsb*XmTextField*width:                   420",
  "*fillOnSelect:			True",
  "*visibleWhenOff:		        True",
  "*XmText*highlightThickness:		0",
  "*XmTextField*highlightThickness:	0",
  "*XmPushButton*highlightThickness:	0",
  "*XmScrollBar*highlightThickness:     0",
  "*highlightThickness:	                0",
  /* "*geometry:                           +400+200", */
  /*   "*keyboardFocusPolicy:                pointer", */

  "*TitleFont: -adobe-times-bold-r-normal-*-24-*-*-*-*-*-iso10646-1",
  "*Font: -adobe-times-medium-r-normal-*-17-*-*-*-*-*-iso10646-1",
  "*ItalicFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso10646-1",
  "*BoldFont: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso10646-1",
  "*FixedFont: -adobe-courier-medium-r-normal-*-17-*-*-*-*-*-iso10646-1",
  "*Header1Font: -adobe-times-bold-r-normal-*-24-*-*-*-*-*-iso10646-1",
  "*Header2Font: -adobe-times-bold-r-normal-*-18-*-*-*-*-*-iso10646-1",
  "*Header3Font: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso10646-1",
  "*Header4Font: -adobe-times-bold-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*Header5Font: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso10646-1",
  "*Header6Font: -adobe-times-bold-r-normal-*-10-*-*-*-*-*-iso10646-1",
  "*AddressFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso10646-1",
  "*PlainFont: -adobe-courier-medium-r-normal-*-14-*-*-*-*-*-iso10646-1",
  "*ListingFont: -adobe-courier-medium-r-normal-*-12-*-*-*-*-*-iso10646-1",
  "*SupSubFont: -adobe-courier-medium-r-normal-*-10-*-*-*-*-*-iso10646-1",
  "*MeterFont: -adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*",
  "*ToolbarFont: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso10646-1",

  "*Foreground:                         black",
  "*Background:                         white",
  "*TopShadowColor:                     black",
  "*BottomShadowColor:                  black",
  "*anchorColor:                        black",
  "*visitedAnchorColor:                 black",
  "*activeAnchorFG:                     black",
  "*activeAnchorBG:                     white",
  "*TroughColor:                        black",
  "*SelectColor:                        black",
  "*AnchorUnderlines:                   1",
  "*VisitedAnchorUnderlines:            1",
  "*DashedVisitedAnchorUnderlines:      True",
  "*VerticalScrollOnRight:              True",
  /* Disable Motif Drag-N-Drop - BJS */
  "*dragInitiatorProtocolStyle: XmDRAG_NONE",
  "*dragReceiverProtocolStyle:  XmDRAG_NONE",

  NULL,
};

#define __MOSAIC_XRESOURCES_H__
#endif /* __MOSAIC_XRESOURCES_H__ */











