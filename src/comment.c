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
 * Written By:  Scott Powers
 * Date:        April 26, 1995
 * Purpose:     Creates a comment card for Mosaic. Submittal is via form
 *		  and works in conjunction with	a "C" cgi-bin program.
 */
#include "../config.h"
#include "mosaic.h"
#include "gui.h"
#include "comment.h"


#include <pwd.h>
#include <sys/utsname.h>


#ifdef DEBUG_CC
int do_comment=1;
#else
int do_comment=0;
#endif


extern mo_window *current_win;
extern char *machine;
extern struct utsname mo_uname;


void CommentCard(mo_window *win);
int DumpHtml(char *htmlname);
char *MakeFilename();
void InitCard(char *fname);
void PutCardCount(long *num, char *fname);
long GetCardCount(char *fname);


void CommentCard(mo_window *win) {

FILE *fp;
long num[10];
char *fname;
int n;
char *htmlname, *htmlurl;

	if (!win) {
		win=mo_next_window(NULL);
	}
	if (!win) {
		return;
	}

	for (n=0; n<10; n++) {
		num[n]=0;
	}

	if (!do_comment) {
		if (!(fname=MakeFilename())) {
			return;
		}

		num[0]=GetCardCount(fname);
		num[0]++;
	}

#ifndef PRERELEASE
	if (num[0]==COMMENT_TIME || do_comment) {
		if (!(htmlname=tmpnam(NULL))) {
			free(fname);

			return;
		}
		if (!DumpHtml(htmlname)) {
			free(fname);

			return;
		}
		htmlurl=(char *)calloc(strlen(htmlname)+strlen("file://localhost")+10,sizeof(char));
		sprintf(htmlurl,"file://localhost%s",htmlname);
		mo_open_another_window(win,htmlurl,NULL,NULL);
		free(htmlurl);
	}
#endif

	if (!do_comment) {
		PutCardCount(num,fname);
	}

	free(fname);

	return;
}


int DumpHtml(char *htmlname) {

FILE *fp;

	if (!(fp=fopen(htmlname,"w"))) {
		return(0);
	}
	fprintf(fp,"%s\n",comment_card_html_top);
	fprintf(fp,"					Mosaic Compiled OS: %s<br>\n",MO_COMMENT_OS);
	fprintf(fp,"					<input type=\"hidden\" name=\"os\" value=\"%s\">\n",MO_COMMENT_OS);
	fprintf(fp,"					Sysname: %s<br>\n",mo_uname.sysname);
	fprintf(fp,"					<input type=\"hidden\" name=\"sysname\" value=\"%s\">\n",mo_uname.sysname);
	fprintf(fp,"					Release: %s<br>\n",mo_uname.release);
	fprintf(fp,"					<input type=\"hidden\" name=\"release\" value=\"%s\">\n",mo_uname.release);
	fprintf(fp,"%s\n",comment_card_html_bot);
	fclose(fp);

	return(1);
}


char *MakeFilename() {

char *hptr, home[256], *fname;
struct passwd *pwdent;

	/*
	 * Try the HOME environment variable, then the password file, and
	 *   finally give up.
	 */
	if (!(hptr=getenv("HOME"))) {
		if (!(pwdent=getpwuid(getuid()))) {
			return(NULL);
		}
		else {
			strcpy(home,pwdent->pw_dir);
		}
	}
	else {
		strcpy(home,hptr);
	}

	fname=(char *)calloc(strlen(home)+strlen(COMMENT_CARD_FILENAME)+
			     strlen(MO_VERSION_STRING)+5,sizeof(char));
	sprintf(fname,"%s/%s%s",home,COMMENT_CARD_FILENAME,MO_VERSION_STRING);

	return(fname);
}


void InitCard(char *fname) {

FILE *fp;
long num[10];
int n;

	if (!(fp=fopen(fname,"w"))) {
		return;
	}

	num[0]=1;
	n=fwrite(num,sizeof(long),2,fp);

	fclose(fp);

	return;
}


void PutCardCount(long *num, char *fname) {

FILE *fp;
int n;

	if (!(fp=fopen(fname,"w"))) {
		return;
	}

	n=fwrite(num,sizeof(long),2,fp);

	fclose(fp);

	return;
}


long GetCardCount(char *fname) {

FILE *fp;
long num[10];
int n;

	if (!(fp=fopen(fname,"r"))) {
		InitCard(fname);
		return((long)0);
	}
	fseek(fp,0L,SEEK_SET);
	n=fread(num,sizeof(long),2,fp);

	fclose(fp);

	return(num[0]);
}
