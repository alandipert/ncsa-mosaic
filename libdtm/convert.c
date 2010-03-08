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
/******************************************************************
**
** $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/convert.c,v 1.2 1995/10/13 06:33:03 spowers Exp $
**
******************************************************************/


/*
#ifdef	RCSLOG

 $Log: convert.c,v $
 Revision 1.2  1995/10/13 06:33:03  spowers
 Solaris support added.

 Revision 1.1.1.1  1995/01/11 00:02:56  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:39:19  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:30  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:09  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:05  marca
 * I think I got it now.
 *
 * Revision 1.3  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.2  1991/06/11  15:21:13  sreedhar
 * disclaimer added
 *
 * Revision 1.1  1990/11/08  16:28:46  jefft
 * Initial revision
 *

#endif
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<netinet/in.h>

#include	"dtmint.h"
#include	"debug.h"


static int dtm_char(mode, buf, size)
  int	mode, size;
  VOIDPTR	buf;
{
  DBGFLOW("# dtm_char called.\n");

  return size;
}


static int dtm_short(mode, buf, size)
  int	mode, size;
  VOIDPTR	buf;
{
  DBGFLOW("# dtm_short called.\n");

  return ((mode == DTMLOCAL) ? (size / 2) : (size * 2));
}


static int dtm_int(mode, buf, size)
  int	mode, size;
  VOIDPTR	buf;
{

  DBGFLOW("# dtm_int called.\n");

  return ((mode == DTMLOCAL) ? (size / 4) : (size * 4));
}


static int dtm_float(mode, buf, size)
  int	mode, size;
  VOIDPTR	buf;
{

  DBGFLOW("# dtm_float called.\n");

  return ((mode == DTMLOCAL) ? (size / 4) : (size * 4));
}


static int dtm_double(mode, buf, size)
  int	mode, size;
  VOIDPTR	buf;
{

  DBGFLOW("# dtm_flt64 called.\n");

  return ((mode == DTMLOCAL) ? (size / 8) : (size * 8));
}


static int dtm_complex(mode, buf, size)
  int	mode, size;
  VOIDPTR	buf;
{

  DBGFLOW("# dtm_complex called.\n");

  return ((mode == DTMLOCAL) ? (size / 8) : (size * 8));
}


static int dtm_triplet(mode, buf, size)
  int	mode, size;
  VOIDPTR	buf;
{

  DBGFLOW("# dtm_triplet called.\n");

  return  ((mode == DTMLOCAL) ? (size / 16) : (size * 16));
}


/* conversion routine function table */
int	(*DTMconvertRtns[])() = {
		dtm_char,
                dtm_short,
                dtm_int,
                dtm_float,
                dtm_double,
                dtm_complex,
		dtm_triplet
		};
