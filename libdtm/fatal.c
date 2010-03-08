/*****************************************************************************
*
*                         NCSA DTM version 2.3
*                               May 1, 1992
*
* NCSA DTM Version 2.3 source code and documentation are in the public
* domain.  Specifically, we give to the public domain all rights for future
* licensing of the source code, all resale rights, and all publishing rights.
*
* We ask, but do not require, that the following message be included in all
* derived works:
*
* Portions developed at the National Center for Supercomputing Applications at
* the University of Illinois at Urbana-Champaign.
*
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
*
*****************************************************************************/

/*********************************************************************
**
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/fatal.c,v 1.2 1995/10/13 06:33:14 spowers Exp $
**
**********************************************************************/

/*
#ifdef RCSLOG

 $Log: fatal.c,v $
 Revision 1.2  1995/10/13 06:33:14  spowers
 Solaris support added.

 Revision 1.1.1.1  1995/01/11 00:03:00  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:39:56  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:32  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:12  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:25  marca
 * I think I got it now.
 *
 * Revision 1.8  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.7  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.6  1991/10/16  23:25:00  jplevyak
 * Added new error message.
 *
 * Revision 1.5  1991/10/10  14:31:48  jplevyak
 * Added new error messages for "bad ack to internal flow control" and
 * "Bad address".  These may not end up as user error messages.
 *
 * Revision 1.4  91/09/26  20:19:14  jplevyak
 * Added several new errors, changed the DTMerrmsg function to detect
 * and return 'unknown error:' errors.  Generally encorporated the
 * good features of sherr (from the libtest directory).
 * 
 * Revision 1.3  91/06/25  20:13:28  creiman
 * Removed varargs and dtm_fatal.
 * 
 * Revision 1.2  1991/06/11  15:19:57  sreedhar
 * disclaimer added
 *
 * Revision 1.1  1990/11/08  16:33:22  jefft
 * Initial revision
 *

#endif
*/


#include	<stdio.h>
#include	<sys/types.h>
#ifdef _ARCH_MSDOS
#include    <nmpcip.h>
#else
#include	<netinet/in.h>
#endif

#include	"dtmint.h"

static char	*err_msg[] = {
	"No error",
	"Out of memory - can not create port",
	"Invalid port name - should be 'hostname:tcp port'",
	"Out of DTM ports - 256 ports max",
	"Couldn't initialize port",
	"DTM routines called in wrong order",
	"Encounted EOF",
	"Error creating socket",
	"Bad hostname",
	"Timeout waiting for connection",
	"Couldn't connect",
	"DTM read error",
	"DTM write error",
	"DTM header to long for buffer",
	"SDS error",
	"Select call error",
	"Environment not setup",
	"User buffer overflow",
	"Port table corrupted",
	"Bad port supplied to library",
	"Bad ack to internal flow control",
	"Bad address",
	"Problem communicating with server"
	};


#ifdef DTM_PROTOTYPES
void dtm_version(void )
#else
void dtm_version()
#endif
{
  fprintf(stderr, "\nDTMlib version %s.\n", DTM_VERSION);
}


#ifdef DTM_PROTOTYPES
char *DTMerrmsg(int quiet)
#else
char *DTMerrmsg(quiet)
	int	quiet;
#endif
{
	char	* 	strUnknown = "unknown error: %d"; 
	char		strOut[60];
	char	*	strErr;

	if ( DTMerrno < (sizeof(err_msg)/sizeof(char *))) 
		strErr = err_msg[(int)DTMerrno];
	else {
		sprintf( strOut, strUnknown, DTMerrno);
		strErr = strOut;
	}

	if (!quiet)
		fprintf(stderr, "\nDTMerrno = %d: %s\n", DTMerrno, 
					strErr);
	return strErr;
}
