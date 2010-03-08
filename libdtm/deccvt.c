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
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/deccvt.c,v 1.1.1.1 1995/01/11 00:02:58 alanb Exp $
**
**********************************************************************/



#ifdef RCSLOG

 $Log: deccvt.c,v $
 Revision 1.1.1.1  1995/01/11 00:02:58  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:39:34  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:30  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:10  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:12  marca
 * I think I got it now.
 *
 * Revision 1.4  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.3  1992/04/03  18:25:50  jplevyak
 * Fixed indice computation problem.
 *
 * Revision 1.2  1991/06/11  15:22:22  sreedhar
 * diclaimer added
 *
 * Revision 1.1  1991/05/30  15:52:54  sreedhar
 * Initial revision
 *
 * Revision 1.2  1990/11/08  16:48:12  jefft
 * Fixed dtm_triplet and dtm_complex conversion routines.  'size' is set
 * first instead of only on return.
 *
 * Revision 1.1  90/11/08  16:40:06  jefft
 * Initial revision
 * 

#endif


#include	<stdio.h>
#include	<sys/types.h>
#include	<netinet/in.h>

#include	"dtmint.h"
#include	"debug.h"


#define		swap(x,y)	x ^= y;  y ^= x;  x ^= y


static int dtm_char(mode, buf, size)
  int	mode, size;
  char	*buf;
{
	DBGFLOW("# dtm_char called.\n");

	return size;
}

static int dtm_short(mode, buf, size)
  int	mode, size;
  char	*buf;
{
	int		nShorts, i;
	DBGFLOW("# dtm_short called.\n");
	
	nShorts = (mode == DTMLOCAL) ? (size / 2) : size;
	if (buf)
		for ( i = 0; i < nShorts ; i++ ) {
			swap(*buf, *(buf+1));
			buf += 2;	
		}
		
	return ((mode == DTMLOCAL) ? (size / 2) : (size * 2));
}


static int dtm_int(mode, buf, size)
  int	mode, size;
  char	*buf;
{
	int		nWords, i;

	DBGFLOW("# dtm_int called.\n");

	nWords = (mode == DTMLOCAL) ? (size / 4) : size;
	size = ((mode == DTMLOCAL) ? (size / 4) : (size * 4));

	if (buf)
		for ( i=0 ; i < nWords ; i++ )  {
			swap( *buf, *(buf+3) );
			swap( *(buf+1), *(buf+2) );
			buf += 4;
		}

	return size;
}


static int dtm_float(mode, buf, size)
  int	mode, size;
  char	*buf;
{
	int		nWords, i;

	DBGFLOW("# dtm_float called.\n");

	nWords = (mode == DTMLOCAL) ? (size / 4) : size;
	size = ((mode == DTMLOCAL) ? (size / 4) : (size * 4));

	if (buf)
		for ( i=0 ; i < nWords ; i++ )  {
			swap( *buf, *(buf+3) );
			swap( *(buf+1), *(buf+2) );
			buf += 4;
		}

	return size;
}


static int dtm_double(mode, buf, size)
  int	mode, size;
  char	*buf;
{
	int		nWords, i;

	DBGFLOW("# dtm_flt64 called.\n");

	nWords = (mode == DTMLOCAL) ? (size / 4) : size * 2;
	if (buf)
		for ( i=0 ; i < nWords ; i++ )  {
			swap( *buf, *(buf+3) );
			swap( *(buf+1), *(buf+2) );
			buf += 4;
		}
	return ((mode == DTMLOCAL) ? (size / 8) : (size * 8));
}


static int dtm_complex(mode, buf, size)
  int	mode, size;
  char	*buf;
{
	int		nWords, i;

	DBGFLOW("# dtm_complex called.\n");

	nWords = (mode == DTMLOCAL) ? (size / 4) : size * 2;
	size = ((mode == DTMLOCAL) ? (size / 8) : (size * 8));

	if (buf)
		for ( i=0 ; i < nWords ; i++ )  {
			swap( *buf, *(buf+3) );
			swap( *(buf+1), *(buf+2) );
			buf += 4;
		}

	return size;
}


static int dtm_triplet(mode, buf, size)
  int	mode, size;
  char	*buf;
{
	int		nWords, i;

	DBGFLOW("# dtm_triplet called.\n");

	nWords = (mode == DTMLOCAL) ? (size / 4) : size * 4;
	size = ((mode == DTMLOCAL) ? (size / 16) : (size * 16));

	if (buf)
		for ( i=0 ; i < nWords ; i++ )  {
			swap( *buf, *(buf+3) );
			swap( *(buf+1), *(buf+2) );
			buf += 4;
		}

  return size;
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
