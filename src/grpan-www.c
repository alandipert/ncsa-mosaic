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
#include "mosaic.h"
#include "grpan-www.h"

/*for memcpy*/
#include <memory.h>

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

#if 0

/* libwww includes */
#include "tcp.h"
#include "HTAnchor.h"
#include "HTParse.h"


static int HtLoadHTTPANN(char *arg, char *data, int len, char *com);

/****************************************************************************
 * name:    HtLoadHTTPANN (PRIVATE)
 * purpose: Issue a command to a group annotation server.
 * inputs:  
 *   - char  *arg:
 *   - char *data:
 *   - int    len:
 *   - char  *com:
 * returns: 
 *   
 * remarks: 
 *   
 ****************************************************************************/
static int
HtLoadHTTPANN(char *arg, char *data, int len, char *com)
{
	int s;				/* Socket number for returned data */
	char *command;			/* The whole command */
	int status;			/* tcp return */
	SockA soc_address;		/* Binary network address */
	SockA *sin = &soc_address;
	char *tptr;
	int fmt, compressed;
	int command_len;
	HTParentAnchor *anchor;

	/*
	 * Set up defaults:
	 */
#ifdef DECNET
	sin->sdn_family = AF_DECnet;        /* Family = DECnet, host order */
	sin->sdn_objnum = DNP_OBJ;          /* Default: http object number */
#else  /* Internet */
	sin->sin_family = AF_INET;          /* Family = internet, host order */
	sin->sin_port = htons(TCP_PORT);    /* Default: http port    */
#endif

	tptr = HTParse(arg, "", PARSE_HOST);
	status = HTParseInet(sin, tptr);
	free(tptr);
	if (status)
	{
		return(status);
	}

	/*
	 * Now, let's get a socket set up from the server for the data.
	 */
#ifdef DECNET
	s = socket(AF_DECnet, SOCK_STREAM, 0);
#else
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
	status = connect
          (s, (struct sockaddr*)&soc_address,sizeof(soc_address));
	if (status < 0)
	{
		return(HTInetStatus("connect"));
	}

        /* If there's an anchor at this point, leave it in. */
	tptr = HTParse(arg, "", PARSE_PATH|PARSE_PUNCTUATION|PARSE_ANCHOR);
#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf(stderr, "HTParse(%s) returns:\n\t(%s)\n", arg, tptr);
	}
#endif
	command_len = strlen(com) + strlen(tptr);
	command = malloc(command_len + len + 1);
	if (command == NULL) outofmem(__FILE__, "HTLoadHTTP");
	strcpy(command, com);
	strcat(command, tptr);
	if (len != 0)
	{
		char *bptr;

		bptr = (char *)(command + command_len);
/*		bcopy(data, bptr, len);*/
		memcpy(bptr, data, len);
		command_len += len;
	}
	else
	{
		command_len++;
	}
	free(tptr);

	status = NETWRITE(s, command, command_len);
	free(command);
	if (status < 0)
	{
		return(HTInetStatus("send"));
	}

	tptr = HTParse(arg, "",
		  PARSE_ACCESS | PARSE_HOST | PARSE_PATH |
		  PARSE_PUNCTUATION);
#if 0
	/* fmt = HTFileFormat (tptr, WWW_HTML, &compressed); */
        fmt = WWW_HTML;
	anchor = HTAnchor_parent(HTAnchor_findAddress(arg));
	HTParseFormat(fmt, anchor, s, 0);
#endif
	free(tptr);

	status = NETCLOSE(s);

	return(HT_LOADED);
}

#endif /* 0 */

char *
grpan_doit(char *com, char *url, char *data, int len, char **texthead)
{
#if 0
	char *txt;

	if (HtLoadHTTPANN(url, data, len, com) == HT_LOADED)
	{
		txt = mo_get_html_return(texthead);
		return(txt);
	}
	*texthead = NULL;
#endif /* 0 */
	return(NULL);
}

