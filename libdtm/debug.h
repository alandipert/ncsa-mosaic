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

/**********************************************************************
**
**  DEBUG macro routines
**
***********************************************************************/

/*********************************************************************
**
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/debug.h,v 1.3 1995/01/12 02:33:31 spowers Exp $
**
**********************************************************************/

/*
 * Revision 1.1.1.1  1993/07/04  00:03:10  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:11  marca
 * I think I got it now.
 *
 * Revision 1.8  92/04/30  20:27:05  jplevyak
 * Changed Version to 2.3
 * 
 * Revision 1.7  1991/11/15  19:42:34  jplevyak
 * Removed dependancy on dtmint.h
 *
 * Revision 1.6  1991/10/29  22:52:43  jplevyak
 * Added debugging test before printing DTMERR messages.
 *
 * Revision 1.5  1991/10/10  14:21:33  jplevyak
 * Added new macros for DBGMSG2 DBGMSG3.
 *
 * Revision 1.4  91/09/26  20:13:12  jplevyak
 * Changed to allow setting of debug flags at run time.
 * 
 * Revision 1.3  91/08/15  19:10:25  sreedhar
 * Changes for logical portname version
 * 
 * Revision 1.2  1991/06/11  15:22:30  sreedhar
 * disclaimer added
 *
 * Revision 1.1  1990/11/08  16:30:41  jefft
 * Initial revision
 *
*/

#ifndef DEBUG_INC
#define	DEBUG_INC	

#define	DBGFLOW(s)		if (uDTMdbg & DTM_DBG_MSG) fprintf(stderr, s)

#define	DBGINT(s, i)	if (uDTMdbg & DTM_DBG_MSG) fprintf(stderr, s, i)
#define	DBGFLT(s, f)	if (uDTMdbg & DTM_DBG_MSG) fprintf(stderr, s, f)
#define	DBGSTR(s, str)	if (uDTMdbg & DTM_DBG_MSG) fprintf(stderr, s, str)
#define	DBGPTR(s, p)	if (uDTMdbg & DTM_DBG_MSG) fprintf(stderr, s, (int)p)

/*
	The new method will be to use
		DBGXXX	for 0 argument debugging message of class XXXS
		DBGXXXN	for N argument debugging messages of calss XXX including MSG

		MSG	is the general class
*/
#define	DBGMSG(s)			if (uDTMdbg & DTM_DBG_MSG) fprintf(stderr, s)
#define	DBGMSG1(s, arg1 )	if (uDTMdbg & DTM_DBG_MSG) fprintf(stderr, s, arg1 )
#define	DBGMSG2(s, arg1, arg2 )		 if (uDTMdbg & DTM_DBG_MSG) \
										fprintf(stderr, s, arg1, arg2 )
#define	DBGMSG3(s, arg1, arg2, arg3 ) if (uDTMdbg & DTM_DBG_MSG) \
										fprintf(stderr, s, arg1, arg2, arg3 )


#define	DTMERR(x)	if (uDTMdbg & DTM_DBG_MSG) fprintf( stderr, "%s\n", x )


#define	DTM_DBG_MSG		0x0001
extern unsigned int	uDTMdbg;

#endif /* DEBUG_INC */
