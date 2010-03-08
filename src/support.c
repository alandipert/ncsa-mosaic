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
#include <string.h>

/*for memcpy*/
#include <memory.h>

#include "cci.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

int ReadBuffer(s,data,numBytesToRead)
/* this routine reads from the specified port, but also considers contents
   read and in buffer from the GetLine() routine.
/* return the number of chars read */
MCCIPort s;
char *data;
int numBytesToRead;
{
int numRead = 0;

	if (numBytesToRead <= s->numInBuffer) {
		memcpy(data,s->buffer,numBytesToRead);
		s->numInBuffer -= numBytesToRead;
		return(numBytesToRead);
		}
	if (s->numInBuffer > 0) {
		memcpy(data,s->buffer,s->numInBuffer);
		data += s->numInBuffer;
		numBytesToRead -= s->numInBuffer;
		numRead = s->numInBuffer;
		s->numInBuffer = 0;
		}

	numRead += NetRead(s, data, numBytesToRead);
	return(numRead);
}



char *GetLine(s) /****** this routine needs an overhaul.... */
MCCIPort s;
/* This routine returns a line read in from the socket file descriptor. 
 * The location of the string returned is good until the next call.
 * Limitation of this routine: A line read in must not be bigger than
 * the buffersize.
 * 0 returned on error
 */

{
int numBytes;
char buf2[PORTBUFFERSIZE +1];
char *endptr;
static char returnLine[PORTBUFFERSIZE * 2 +2];
register char *rptr,*ptr;
register int count;


	if (s->numInBuffer < 1) {
		/* no character in s->buffer, so fill it up */
/*
		if (!connectedToServer) {
			return(0);
			}
*/
		if (1 > (numBytes = NetRead(s, s->buffer, PORTBUFFERSIZE))) {
			/*  End Of Connection */
/*
			DisconnectFromServer(s);
*/

#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr,"GetLine: End of Connection\n");
			}
#endif

			return(0);
			}
		s->numInBuffer = numBytes;
		}

	s->buffer[s->numInBuffer]='\0';
	if (!(endptr = strstr(s->buffer, "\r\n"))) {
		/* There is no <CRLF> in s->buffer */
/*
		if (!connectedToServer) {
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr,"GetLine: return 0 at point 3\n");
			}
#endif

			return(0);
			}
*/
/*
		if (! NetIsThereInput(s)) {
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr,"GetLine: return 0 at point 4\n");
			}
#endif

			return(0);
			}
*/

		/* read in <CRLF> */
		if (1 > (numBytes = NetRead(s, buf2, PORTBUFFERSIZE))) {
			/*  End Of Connection */
/*
			NNTPDisconnectFromServer(s);
*/
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr,"GetLine: return 0 at point 5\n");
			}
#endif

			return(0);
			}
		memcpy(&(s->buffer[s->numInBuffer]),buf2,numBytes);
		s->numInBuffer += numBytes;
		s->buffer[s->numInBuffer]='\0';
		if (!(endptr = strstr(s->buffer, "\r\n"))) {
			/* protocol error on server end 
			   Everything sent should be terminated with
			   a <CRLF>... just return for now */
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr,"GetLine: return NULL at point 6\n");
			}
#endif

			return(NULL);
			}
		}
	endptr++;endptr++; /* <CRLF> should be included in line*/

	/* copy the line to the returnLine s->buffer */
	count = 0;
	rptr = returnLine;
	ptr = s->buffer;
	while (ptr != endptr) {
		*rptr++ =  *ptr++;
		count++;
		}
	*rptr = '\0'; /* null terminate the return line */

	/* shift the s->buffer contents to the front */
	s->numInBuffer -= count;
/*	bcopy(ptr,s->buffer,s->numInBuffer);*/
	memcpy(s->buffer,ptr,s->numInBuffer);

	return(returnLine);
	
} /* NNTPGetLine() */


/* return a word out of the text */
void GetWordFromString(text,retStart,retEnd)
char *text;	 /* text to get a word out of */
char **retStart; /* RETURNED: start of word in text */
char **retEnd;	 /* RETURNED: end of word in text */
{
char *start;
char *end;

	if (!text) {
		*retStart = *retEnd = text;
		return;
		}

	start = text;
	while ((*start) && isspace(*start)){ /*skip over leading space*/
		start++;
		}

	end = start;
	while((*end) && (!isspace(*end))){ /* find next space */
		end++;
		}

	*retStart = start;
	*retEnd = end;
	return;
}


