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
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/craycvt.c,v 1.1.1.1 1995/01/11 00:02:57 alanb Exp $
**
**********************************************************************/


#ifdef RCSLOG

 $Log: craycvt.c,v $
 Revision 1.1.1.1  1995/01/11 00:02:57  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:39:21  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:30  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:09  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:06  marca
 * I think I got it now.
 *
 * Revision 1.4  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.3  1991/06/11  15:21:41  sreedhar
 * disclaimer added
 *
 * Revision 1.2  1991/01/31  17:08:16  jefft
 * Added 'icheck' variable to force checking of floating point values and
 * changing to 0 on underflow.
 *
 * Revision 1.1  90/11/08  16:29:13  jefft
 * Initial revision
 * 

#endif

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

  if (mode == DTMLOCAL)  {

    if (buf != NULL)  {
      int	tmp;
      tmp = size / 4;

      DTMSCI(buf, buf, &tmp);
      }

    return (size / 4);
    }

  else  {
    if (buf != NULL)
      DTMCSI(buf, buf, &size);
    return (size * 4);
    }
}


static int dtm_float(mode, buf, size)
  int	mode, size;
  VOIDPTR	buf;
{
  int	icheck = 1;

  DBGFLOW("# dtm_float called.\n");

  if (mode == DTMLOCAL)  {

    if (buf != NULL)  {
      int	tmp;
      tmp = size / 4;

      DTMSCF(buf, buf, &tmp);
      }

    return (size / 4);
    }

  else  {
    if (buf != NULL)
      DTMCSF(buf, buf, &size, &icheck);
    return (size * 4);
    }
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
  int	icheck = 1;

  DBGFLOW("# dtm_complex called.\n");

  if (mode == DTMLOCAL)  {

    if (buf != NULL)  {
      int       tmp;

      tmp = size / 8;
      DTMSCF(buf, buf, &tmp);
      }

    return (size / 8);
    }

  else  {
    if (buf != NULL)
      DTMCSF(buf, buf, &size, &icheck);
    return (size * 8);
    }
}


static int dtm_triplet(mode, buf, size)
  int	mode, size;
  VOIDPTR	buf;
{

  DBGFLOW("# dtm_triplet called.\n");

  if (mode == DTMLOCAL)  {
    if (buf != NULL)  {
      int	tmp;

      tmp = size / 16;
      DTMSCT(buf, buf, &tmp);
      }
    return (size / 16);
    }

  else  {
    if (buf != NULL)
      DTMCST(buf, buf, &size);
    return (size * 16);
    }
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
