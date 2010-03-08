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
 * Created: Wed Apr 10 17:41:00 CDT 1996
 * Author: Dan Pape
 *
 */

/* this file contains stuff from the old "mosaic.h" file. I am breaking
   that file up because it was too big, and required a re-compile of all
   the source whenever something changed. */

#ifndef __MOWWW_H__
#define __MOWWW_H__ 

char *mo_pull_er_over (char *, char **);
char *mo_post_pull_er_over (char *url, char *content_type, char *data, 
                            char **texthead);
mo_status mo_pull_er_over_virgin (char *, char *);
mo_status mo_re_init_formats (void);
char *mo_tmpnam (char *);
char *mo_get_html_return (char **);
char *mo_convert_newlines_to_spaces (char *);
mo_status mo_re_init_formats (void);

char *mo_url_prepend_protocol(char *);
char *mo_url_canonicalize (char *, char *);
char *mo_url_canonicalize_keep_anchor (char *, char *);
char *mo_url_canonicalize_local (char *);
char *mo_url_to_unique_document (char *);
char *mo_url_extract_anchor (char *);
char *mo_url_extract_access (char *, char *);

void application_user_feedback (char *);
char *mo_escape_part (char *);
char *mo_unescape_part (char *);

FILE *mo_start_sending_mail_message (char *to, char *subj, 
                                     char *content_type, char *url);
mo_status mo_finish_sending_mail_message (void);
mo_status mo_send_mail_message (char *text, char *to, char *subj, 
                                char *content_type, char *url);

void application_error(char *str, char *title);
void application_error(char *str, char *title);
void application_user_feedback (char *str);
void application_user_info_wait (char *str);
char *prompt_for_string (char *questionstr);
char *prompt_for_password (char *questionstr);
int prompt_for_yes_or_no (char *questionstr);
char *mo_get_html_return (char **texthead);
char *mo_convert_newlines_to_spaces (char *str);
char *mo_escape_part (char *part);
char *mo_unescape_part (char *str);
void readAgents(void);
void loadAgents(void);

#if defined(ultrix) || defined(VMS) || defined(NeXT) || defined(M4310) || defined(vax)
char *strdup (char *str);
#endif

#endif
