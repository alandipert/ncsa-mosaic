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
#include <stdio.h>
#include <ctype.h>
#include "cci.h"

#include "mosaic.h"
#include "gui.h"
#include "cciServer.h"
#include "cciBindings.h"
#include "cciBindings2.h"
#include "pan.h"
#include "mo-www.h"
#include "annotate.h"
/* for setting some selections buttons*/
#include "libhtmlw/HTML.h"

extern mo_window *current_win;
extern char *home_document;


#ifndef DISABLE_TRACE
extern int cciTrace;
#endif


int cci_get = 0;
int cci_event = 0;
int cci_docommand = 0;

/*char *mo_post_pull_er_over (char *url, char *content_type,
                                   char *post_data,
                                   char **texthead);
char *MoReturnURLFromFileName(char *fileName);
char *mo_fetch_personal_annotations(char *url);
*/

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/*                                                                         */
/* This module contains bindings between the new cci code and the browser  */
/* If you are retrofitting the cci on to your browser strip out everything */
/* except the function headers and the return values and stick in the      */
/* appropriate calls for your browser.                                     */
/*                                                                         */
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

void MCCIRequestDoCommand(retCode, retText, command, parameter)
int *retCode;
char *retText; /* must be less MCCI_MAX_RETURN_TEXT*/
char *command;
char *parameter;
{
mo_window *win;
char *s, *end, *tmp_end, *w_id;

  /* default assume to work */
  *retCode = MCCIR_DOCOMMAND_OK;
  strcpy(retText, "Executed the command");

  if (!strcmp(command, MCCI_EXITPROGRAM)){
	mo_exit ();
	return;
	}

  if (!strcmp(command, MCCI_RELOADCONFIG)){
	mo_re_init_formats ();
	return;
	}

  if (!parameter){
	*retCode = MCCIR_DOCOMMAND_FAILED;
	strcpy(retText, "Parameters Required");
	return;
	}

  s = parameter;

/*
  GetWordFromString(s,&w_id,&end);
  if ((!w_id) || (w_id == end)) {
	*retCode = MCCIR_DOCOMMAND_FAILED;
	strcpy(retText, "You Need to Specify a window id");
	return;
	}
  s = end;
  w_id = strdup(w_id);
  tmp_end = strchr(w_id, ' ');
  if (tmp_end) *tmp_end = '\0';
*/

  /* got the window id here */
/*
  if ((win = mo_fetch_window_by_id(atoi(w_id))) == NULL){
	*retCode = MCCIR_DOCOMMAND_FAILED;
	strcpy(retText, "Bad Window Id");
  	if (w_id) free(w_id);
	return;
	}

  if (w_id) free(w_id);
*/

  if (!current_win) {
	*retCode = MCCIR_DOCOMMAND_FAILED;
	strcpy(retText, "Current Window Not Available");
	return;
  }

  win=current_win;

  if (!strcmp(command, MCCI_BACK))
	mo_back_node(win);
  else if (!strcmp(command, MCCI_FORWARD))
	mo_forward_node(win);	
  else if (!strcmp(command, MCCI_HOME))
	mo_access_document (win, home_document);
  else if (!strcmp(command, MCCI_RELOAD))
	mo_reload_window_text (win, 0);
  else if (!strcmp(command, MCCI_CLONE))
	mo_duplicate_window (win);
  else if (!strcmp(command, MCCI_CLOSEWINDOW))
	mo_delete_window (win);
  else if (!strcmp(command, MCCI_RELOADIMAGES))
	mo_reload_window_text (win, 1);
  else if (!strcmp(command, MCCI_REFRESHCURRENT))
	mo_refresh_window_text (win);
  else if (!strcmp(command, MCCI_VIEWSOURCE))
	mo_post_source_window (win);
  else if (!strcmp(command, MCCI_EDITSOURCE))
	mo_edit_source(win);
  else if (!strcmp(command, MCCI_NEWWINDOW))
	mo_open_another_window (win, home_document, NULL, NULL);
  else if (!strcmp(command, MCCI_FLUSHIMAGECACHE))
	mo_flush_image_cache (win);
  else if (!strcmp(command, MCCI_CLEARGLOBALHISTORY)){
     	mo_window *w = NULL;
	mo_wipe_global_history (win);

	while (w = mo_next_window (w))
	  	mo_redisplay_window (w);
	}
  else if (!strcmp(command, MCCI_SAVEAS)){
    /* need to get format filename */
    char *format, *filename;
    mo_status status;

/*FIX*/
	/* s is pointed pass the window id part */

	GetWordFromString(s,&format,&end); /* Get command */
	if ((!format) || (format == end)) {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify a format");
		return;
		}
	s = end;
	format = strdup(format);
	tmp_end = strchr(format, ' ');
	if (tmp_end) *tmp_end = '\0';

	GetWordFromString(s,&filename,&end); /* Get command */
	if ((!filename) || (filename == end)) {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify a filename");
		if (format) free(format);
		return;
		}
	filename = strdup(filename);
	sscanf(filename,"%s",filename); /* to get rid of /r/n */
	cci_docommand = 1;
	if (!strcmp(format, MCCI_PLAINTEXT))
		status = mo_save_window(win, filename, 0);
	else if (!strcmp(format, MCCI_FORMATTEDTEXT))
		status = mo_save_window(win, filename, 1);
	else if (!strcmp(format, MCCI_HTML)) 
		status = mo_save_window(win, filename, 2);
	else if (!strcmp(format, MCCI_POSTSCRIPT))
		status = mo_save_window(win, filename, 4);
	else{
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "Invalid Format");
		if (filename) free(filename);
		if (format) free(format);
		cci_docommand = 0;
		return;
		}
	cci_docommand = 0;

	if (filename) free(filename);
	if (format) free(format);
	if (status == mo_fail){
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "Unable to save file");
		return;
		}
	}

  else if (!strcmp(command, MCCI_FINDINCURRENT)){
    /* need to get search_string and CASE|NOCASE */
    char *s_string, *c;   
    mo_status found;

	/* s is pointed pass the window id part */
	GetWordFromString(s,&s_string,&end); /* Get command */
	if ((!s_string) || (s_string == end)) {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify a search string");
		return;
		}
	s = end;
	s_string = strdup(s_string);
	tmp_end = strchr(s_string, ' ');
	if (tmp_end) *tmp_end = '\0';

	GetWordFromString(s,&c,&end); /* Get command */
	if ((!c) || (c == end) ){ 
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify CASE");
		if (s_string) free(s_string);
		return;
		}
	c = strdup(c);
	sscanf(c,"%s",c); /* to get rid of /r/n */

	cci_docommand = 1;
	if (!strcmp(c, MCCI_NOCASE))
		found = mo_search_window(win, s_string, 0, 1, 0);
	else if (!strcmp(c, MCCI_CASE))
		found = mo_search_window(win, s_string, 0, 0, 0);
  	else{
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "Invalid CASE Specification");
		cci_docommand = 0;
		if(s_string) free(s_string);
		if (c) free(c);
		return;
		}
	cci_docommand = 0;

	/**** should I return a error when not found ****/
	if(s_string) free(s_string);
	if (c) free(c);

	}
  else if (!strcmp(command, MCCI_PRINT)){
    /* need to get format and printCommand */
    char *format, *printCommand;
    mo_status status;

	/* s is pointed pass the window id part */
	GetWordFromString(s,&format,&end); /* Get command */
	if ((!format) || (format == end)) {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify a print format");
		return;
		}
	s = end;
	format = strdup( format);
	tmp_end = strchr(format, ' ');
	if (tmp_end) *tmp_end = '\0';

	if (s == NULL){
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify a print command");
		if (format) free(format);
		return;
		}

	printCommand = s;
	while(isalnum( (int) (*s)) || (*s == ' ') || (*s == '-'))
		s++;
	if (s) *s = '\0';
	printCommand = strdup(printCommand);

	cci_docommand = 1;
	if (!strcmp(format, MCCI_PLAINTEXT))
		status = mo_print_window(win, 0, printCommand);
	else if (!strcmp(format, MCCI_FORMATTEDTEXT))
		status = mo_print_window(win, 1, printCommand);
	else if (!strcmp(format, MCCI_HTML)) 
		status = mo_print_window(win, 2, printCommand);
	else if (!strcmp(format, MCCI_POSTSCRIPT))
		status = mo_print_window(win, 4, printCommand);
	else{
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "Invalid Format");
	   	cci_docommand = 0;
		if (format) free(format);
		return;
		}
	cci_docommand = 0;
	if (format) free(format);
	if (status == mo_fail){
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "Unable to print file");
		return;
		}
	}
  else if (!strcmp(command, MCCI_FANCYSELECTIONS)){
    /* need to get ON|OFF */
    char *on_off;

	/* s is pointed pass the window id part */
	GetWordFromString(s,&on_off,&end); /* Get command */
	if ((!on_off) || (on_off == end)) {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify ON|OFF");
		return;
		}
	on_off = strdup(on_off);
	sscanf(on_off,"%s", on_off);

	if (!strcmp(on_off, MCCI_ON)){
		win->pretty = 1;
		mo_set_fancy_selections_toggle (win);
		HTMLClearSelection (win->scrolled_win);
		XmxSetArg (WbNfancySelections, True);
		XmxSetValues (win->scrolled_win);
		}
	else if(!strcmp(on_off, MCCI_OFF)){
		win->pretty = 0;
		mo_set_fancy_selections_toggle (win);
		HTMLClearSelection (win->scrolled_win);
		XmxSetArg (WbNfancySelections, False);
		XmxSetValues (win->scrolled_win);
		}
	else{
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify ON|OFF");
		if (on_off) free(on_off);
		return;
		}
	if (on_off) free(on_off);
	}
  else if (!strcmp(command, MCCI_LOADTOLOCALDISK)){
    /* need to get ON|OFF */
    char *on_off;

	/* s is pointed pass the window id part */
	GetWordFromString(s,&on_off,&end); /* Get command */
	if ((!on_off) || (on_off == end)) {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify ON|OFF");
		return;
		}
	on_off = strdup(on_off);
	sscanf(on_off,"%s", on_off);

	if (!strcmp(on_off, MCCI_ON)){
		win->binary_transfer = 1;
		XmxRSetToggleState (win->menubar, mo_binary_transfer,
			(win->binary_transfer ? XmxSet : XmxNotSet));
		}
	else if (!strcmp(on_off, MCCI_OFF)){
		win->binary_transfer = 0;
		XmxRSetToggleState (win->menubar, mo_binary_transfer,
			(win->binary_transfer ? XmxSet : XmxNotSet));
		}
	else{
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify ON|OFF");
		if (on_off) free(on_off);
		return;
		}
	if (on_off) free(on_off);
	}
  else if (!strcmp(command, MCCI_DELAYIMAGELOAD)){
    /* need to get ON|OFF */
    char *on_off;

	/* s is pointed pass the window id part */
	GetWordFromString(s,&on_off,&end); /* Get command */
	if ((!on_off) || (on_off == end)) {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify ON|OFF");
		return;
		}
	on_off = strdup(on_off);
	sscanf(on_off,"%s", on_off);

	if (!strcmp(on_off, MCCI_ON)){
		win->delay_image_loads = 1;
		XmxRSetToggleState(win->menubar, mo_delay_image_loads,
			(win->delay_image_loads?XmxSet : XmxNotSet));
/*
		XmxSetArg (WbNdelayImageLoads, True);
		XmxSetValues (win->scrolled_win);
*/
		XmxRSetSensitive (win->menubar, mo_expand_images_current, 
			win->delay_image_loads?XmxSensitive:XmxNotSensitive);
		}
	else if (!strcmp(on_off, MCCI_OFF)){
		win->delay_image_loads = 0;
		XmxRSetToggleState(win->menubar, mo_delay_image_loads,
			(win->delay_image_loads?XmxSet : XmxNotSet));
/*
		XmxSetArg (WbNdelayImageLoads, False);
		XmxSetValues (win->scrolled_win);
*/
		XmxRSetSensitive (win->menubar, mo_expand_images_current, 
			win->delay_image_loads?XmxSensitive:XmxNotSensitive);
		} 
	else{
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify ON|OFF");
		if (on_off) free(on_off);
		return;
		}
	if (on_off) free(on_off);
	}
  else if (!strcmp(command, MCCI_WINDOWHISTORY)){
	}
  else if (!strcmp(command, MCCI_HOTLIST)){
	}
  else if (!strcmp(command, MCCI_FONT)){
    /* has to get fontname */
    char *fontname;

	/* s is pointed pass the window id part */
	GetWordFromString(s,&fontname,&end); /* Get command */
	if ((!fontname) || (fontname == end)) {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify fontname");
		return;
		}
	fontname = strdup(fontname);
	sscanf(fontname,"%s", fontname);

	if (!strcmp(fontname, MCCI_TIMES_REGULAR))
		mo_set_fonts(win, mo_regular_fonts);
	else if(!strcmp(fontname, MCCI_TIMES_SMALL))
		mo_set_fonts(win, mo_small_fonts);
	else if(!strcmp(fontname, MCCI_TIMES_LARGE))
		mo_set_fonts(win, mo_large_fonts);
	else if(!strcmp(fontname, MCCI_HELVETICA_LARGE))
		mo_set_fonts(win, mo_large_helvetica);
	else if(!strcmp(fontname, MCCI_HELVETICA_SMALL))
		mo_set_fonts(win, mo_small_helvetica);
	else if(!strcmp(fontname, MCCI_HELVETICA_REGULAR))
		mo_set_fonts(win, mo_regular_helvetica);
	else if(!strcmp(fontname, MCCI_NEWCENTURY_LARGE))
		mo_set_fonts(win, mo_large_newcentury);
	else if(!strcmp(fontname, MCCI_NEWCENTURY_SMALL))
		mo_set_fonts(win, mo_small_newcentury);
	else if(!strcmp(fontname, MCCI_NEWCENTURY_REGULAR))
		mo_set_fonts(win, mo_regular_newcentury);
	else if(!strcmp(fontname, MCCI_LUCIDABRIGHT_LARGE))
		mo_set_fonts(win, mo_large_lucidabright);
	else if(!strcmp(fontname, MCCI_LUCIDABRIGHT_REGULAR))
		mo_set_fonts(win, mo_regular_lucidabright);
	else if(!strcmp(fontname, MCCI_LUCIDABRIGHT_SMALL))
		mo_set_fonts(win, mo_small_lucidabright);
	else{
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "Invalid Fontname Specification");
		if (fontname) free(fontname);
		return;
		}
	if (fontname) free(fontname);
	}
  else if (!strcmp(command, MCCI_ANCHORUNDERLINE)){
    /* need to get level */
    char *level;

	/* s is pointed pass the window id part */
	GetWordFromString(s,&level,&end); /* Get command */
	if ((!level) || (level == end)) {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "You Need to Specify a level");
		return;
		}
	level = strdup(level);
	sscanf(level,"%s", level);

	if (!strcmp(level, MCCI_UNDERLINE_DEFAULT))
		mo_set_underlines (win,mo_default_underlines);
	else if (!strcmp(level, MCCI_UNDERLINE_LIGHT))
		 mo_set_underlines (win,mo_l1_underlines);
	else if (!strcmp(level, MCCI_UNDERLINE_MEDIUM))
		 mo_set_underlines (win,mo_l2_underlines);
	else if (!strcmp(level, MCCI_UNDERLINE_HEAVY))
		 mo_set_underlines (win,mo_l3_underlines);
	else if (!strcmp(level, MCCI_UNDERLINE_NONE))
		 mo_set_underlines (win,mo_no_underlines);
	else {
		*retCode = MCCIR_DOCOMMAND_FAILED;
		strcpy(retText, "Invalid Level Specification");
		if (level) free(level);
		return;
		}
	if (level) free(level);
	}
  else {/* command not recongized */
  	*retCode = MCCIR_DOCOMMAND_FAILED;
	strcpy(retText, "Invalid Command");
	}

  return;
}


void MCCIRequestGetURL(retCode,retText,url,output,additionalHeader)
int *retCode;
char *retText; /* must be less MCCI_MAX_RETURN_TEXT*/
char *url;
int output;
char *additionalHeader; 	/* currently additional header ignored */
{
mo_status moStatus;

#ifndef DISABLE_TRACE
	if (cciTrace) {
		if (additionalHeader)
			fprintf(stderr,"MCCIRequestGetURL(url=\"%s\",output=%d,header=\"%s\")\n",
				url,output,additionalHeader);
		else
			fprintf(stderr,"MCCIRequestGetURL(url=\"%s\",output=%d)\n",
				url,output);
	}
#endif

        if (! get_pref_int(eLOAD_LOCAL_FILE))
	  if (!my_strncasecmp(url,"file:",5)) {
	    *retCode = MCCIR_GET_FAILED;
	    strcpy(retText,
		   " Can't get local file (for CCI security reasons)");
	    return;
	  }

	/*do it */
	if (!strchr (url, ':')) {
		url = mo_url_canonicalize_local (url);
		}

	switch (output) {
	    case MCCI_OUTPUT_CURRENT:
		/* turn flag on so mosaic will know to do a get*/
		cci_get = 1; 	

		moStatus = mo_load_window_text (current_win, url, NULL);
		if (moStatus == mo_succeed) {
			*retCode = MCCIR_GET_OK;
			strcpy(retText,"Got the URL");
			}
		else {
			*retCode = MCCIR_GET_FAILED;
			sprintf(retText,"Couldn't get URL %s",url);
			}
			break;

		cci_get = 0;  	/* done with get, turn flag off */
	    case MCCI_OUTPUT_NEW:
		/* turn flag on so mosaic will know to do a get*/
		cci_get = 1; 	

		if (!mo_open_another_window(current_win,url,NULL,NULL)) {
			*retCode = MCCIR_GET_FAILED;
			sprintf(retText,"Couldn't get URL %s",url);
			}
		else {
			*retCode = MCCIR_GET_OK;
			strcpy(retText,"Got the URL");
			}

		cci_get = 0;  	/* done with get, turn flag off */
		break;
	    case MCCI_OUTPUT_NONE:
		*retCode = MCCIR_GET_FAILED;
		strcpy(retText,"Sorry, OUTPUT to no where not supported yet");
		break;
	    default:
		*retCode = MCCIR_GET_FAILED;
		strcpy(retText,"Send output where???");
		break;
	    }

	return;
}

void MCCIRequestForm(client, retCode,retText,actionID,status)
MCCIPort client;
int *retCode;
char *retText; /* must be less MCCI_MAX_RETURN_TEXT*/
char *actionID;
int status;
{
mo_status moStatus;

	MoCCIForm(client, actionID, status, 0);
	*retCode = MCCIR_FORM_OK;
	strcpy(retText, "Form Submit Received");

	return;
}

void MCCIRequestSendAnchor(retCode,retText,client,status)
int *retCode;
char *retText; /* must be less MCCI_MAX_RETURN_TEXT*/
MCCIPort client;
int status; /* 0, MCCI_SEND_BEFORE, or MCCI_SEND_AFTER */
            /* or MCCI_SEND_HANDLER  ADC ZZZ */
/* anchor replies may be sent back using MCCISendAnchorHistory(client,url)*/
{

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIRequestSendAnchor(%d)\n",status);
	}
#endif

		switch (status) {
		      case MCCI_SEND_BEFORE:
			MoCCISendAnchor(client,1);
			*retCode = MCCIR_SEND_ANCH_BEF_OTHR_OK;
			strcpy(retText,"Send Anchor Before enabled");
			break;
		      case MCCI_SEND_AFTER:
		        MoCCISendAnchor(client,2);
			*retCode = MCCIR_SEND_ANCH_AFT_OTHR_OK;
			strcpy(retText,"Send Anchor After enabled");
			break;
                      case MCCI_SEND_HANDLER:           /* ADC ZZZ */
                        MoCCISendAnchor(client,3);
                        *retCode = MCCIR_SEND_ANCH_HAN_OTHR_OK;
                        strcpy(retText,"Send Anchor Handler enabled");
                        break;
		      case 0:
			MoCCISendAnchor(client,0);
			*retCode = MCCIR_SEND_A_STOP_OK;
			strcpy(retText,"Send Anchor disabled");
			break;
		}
}

void MCCIRequestSendOutput(retCode,retText,client,on,type)
int *retCode;
char *retText; /* must be less MCCI_MAX_RETURN_TEXT*/
MCCIPort client;
int on;		/* boolean value....turn on - true, off - false */
char *type;	/* if null, assume all types */
{

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIRequestSendOutput(%d,%s)\n",on,type);
	}
#endif

	if (on) {
		MoCCISendOutput(client,1,type);
		*retCode = MCCIR_SEND_OUTPUT_OK;
		strcpy(retText,"Send OUTPUT enabled");
		}
	else {
		MoCCISendOutput(client,0,type);
		*retCode = MCCIR_SEND_O_STOP_OK;
		strcpy(retText,"Send OUTPUT disabled");
		}
	return;
}


void MCCIRequestSendEvent(retCode,retText,client,on)
int *retCode;
char *retText; /* must be less MCCI_MAX_RETURN_TEXT*/
MCCIPort client;
int on;		/* boolean value....turn on - true, off - false */
{

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIRequestEvent(%d)\n",on);
	}
#endif

	if (on) {
		cci_event = 1;
		MoCCISendEvent(client,1);
		*retCode = MCCIR_SEND_EVENT_OK;
		strcpy(retText,"Send EVENT enabled");
		}
	else {
		/* set cci_event to 0 only if there are no client request */
		MoCCISendEvent(client,0);
		*retCode = MCCIR_SEND_EVENT_STOP_OK;
		strcpy(retText,"Send EVENT disabled");
		}
	return;
}

void MCCIRequestSendBrowserView(retCode,retText,client,on)
int *retCode;
char *retText; /* must be less MCCI_MAX_RETURN_TEXT*/
MCCIPort client;
int on;		/* boolean value....turn on - true, off - false */
{

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIRequestSendBrowserView(%d)\n",on);
	}
#endif

	if (on) {
		MoCCISendBrowserView(client,1);
		*retCode = MCCIR_BROWSERVIEW_OK;
		strcpy(retText,"Send BROWSERVIEW enabled");
		}
	else {
		MoCCISendBrowserView(client,0);
		*retCode = MCCIR_BROWSERVIEW_STOP_OK;
		strcpy(retText,"Send BROWSERVIEW disabled");
		}
	return;
}



void MCCIRequestPost(client,retCode,retText,url,contentType,
			postData,dataLength,output)
MCCIPort client;
int *retCode;
char *retText;
char *url;
char *contentType;
char *postData;
int dataLength;
int output;
{
char *textHead;
char *response;
char buff[256];
int length;

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIRequestPost(): about to mo_post_pull_er_over()\n");
		fprintf(stderr,"mo_post_pull_er_over(url=\"%s\",type=\"%s\",postData=\"%s\")\n"
				,url,contentType,postData);
	}
#endif

	*retCode = MCCIR_POST_OK;
	strcpy(retText,"Post Request ok");

	switch(output) {
		case MCCI_OUTPUT_NONE:
			/* do not display output of post, but send the
			   output back through the cci to the client */
			response = mo_post_pull_er_over(url, 
				contentType, postData, &textHead);

			/* send response back through cci */
			if (response && (length = strlen(response))) {
				MCCISendResponseLine(client,MCCIR_POST_OUTPUT,
							"POST output");
				sprintf(buff,"Content-Length: %d\r\n",length);
				if (length!=NetServerWrite(client,
							buff,strlen(buff))){
					/* this is pointless... I know */
					strcpy(retText,"couldn't send output");
					*retCode = MCCI_FAIL;
					}
				if (length!=NetServerWrite(client,
							response,length)) {
					/* this is pointless... I know */
					strcpy(retText,"couldn't send output");
					*retCode = MCCI_FAIL;
					}
				}
			
			break;
		case MCCI_OUTPUT_NEW:
			/* open a new window and display posting... */
			/* ...not done yet...*/

		case MCCI_DEFAULT: /* default to output current */
		case MCCI_OUTPUT_CURRENT:
		default:
			/* display in current window */
			response = mo_post_pull_er_over(url, 
				contentType, postData, &textHead);
			/*mo_decode_internal_reference(url,response,url);*/
			mo_do_window_text(current_win,url,
						response,response,1,url,0,0);
			
			break;
		}

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"result from mo_post_pull_er_over():\"%s\"\n",response);
		fprintf(stderr,"MCCIRequestPost(): returning now\n");
	}
#endif
}


void MCCIRequestDisplay(client,retCode,retText,url,contentType,
			displayData,dataLength,output)
MCCIPort client;
int *retCode;
char *retText;
char *url;
char *contentType;
char *displayData;
int dataLength;
int output;
{
char *textHead;
char *response;
char buff[256];
int length;
char *ref;
char *new_url;

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIRequestDisplay(): about to mo_post_pull_er_over()\n");
		fprintf(stderr,"mo_post_pull_er_over(url=\"%s\",type=\"%s\",	\
						     displayData=\"%s\")\n" ,url,contentType,displayData);
	}
#endif

	*retCode = MCCIR_DISPLAY_OK;
	strcpy(retText,"Display Request ok");

	switch(output) {
		case MCCI_OUTPUT_NONE:
			strcpy(retText, "OUTPUT NONE not support yet\n");
			break;
		case MCCI_OUTPUT_NEW:
			/* open a new window and display... */
			/* ...not done yet...*/
			strcpy(retText, "OUTPUT NEW not support yet\n");

		case MCCI_DEFAULT: /* default to output current */
		case MCCI_OUTPUT_CURRENT:
		default:
			/* display in current window */
			ref = strdup(url);
			new_url = strdup(url);
			if (strcmp(contentType, "text/html") == 0)
				mo_do_window_text(current_win, new_url, 
					displayData, displayData, 1, ref, current_win->current_node->last_modified, current_win->current_node->expires);
			else
				strcpy(retText, "Display text/html only");	
			break;
		}

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIRequestDisplay(): returning now\n");
	}
#endif
}

void MCCIRequestQuit()
/* time to die */
{
	mo_exit();
}

void MCCIRequestGetAnnotation(retCode,retText,retData,retDataLength,url,type)
int *retCode;
char *retText;
char **retData;
int *retDataLength;
char *url;
int type;
{
  *retDataLength = 0;
  
  if ((type == MCCI_PRIVATE_ANNOTATION) || 
      (type == MCCI_ALL_ANNOTATION)) {
    if ((*retData) = mo_fetch_personal_annotations(url)) {
      *retCode = MCCIR_PRIVATE_ANNOTATION;
      strcpy(retText,"Annotation follows");
      *retDataLength = strlen(*retData);
    }
    else {
      *retData = NULL;
      *retCode = MCCIR_NO_ANNOTATION;
      strcpy(retText,"No annotation for this URL");
    }
  }
  else {
    *retCode = MCCIR_NO_ANNOTATION;
    strcpy(retText,"Only Private annotations currently supported");
  }
  
  return;
}


MCCIRequestPutAnnotation(retCode,retText,type,url,annotation,annotationLength)
int *retCode;
char *retText;
int type;
char *url;
char *annotation;
int annotationLength;
{
  if (type == MCCI_PRIVATE_ANNOTATION) {
    *retCode = mo_new_pan(url, NULL, NULL, annotation);
  }
}

MCCIRequestFileToURL(retCode,retText,fileName)
int *retCode;
char *retText;
char *fileName;
{
char *url;

	url = MoReturnURLFromFileName(fileName);
	if (url) {
		strcpy(retText,url);
		*retCode = MCCIR_FILE_TO_URL;
		}
	else {
		strcpy(retText,"No URL for given file name");
		*retCode = MCCIR_NO_URL_FOR_FILE; 
		}
}

