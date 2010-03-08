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
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/ris.c,v 1.2 1995/10/13 06:33:21 spowers Exp $
**
**********************************************************************/
/*
#ifdef RCSLOG

 $Log: ris.c,v $
 Revision 1.2  1995/10/13 06:33:21  spowers
 Solaris support added.

 Revision 1.1.1.1  1995/01/11 00:03:01  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:40:10  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:33  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:13  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:34  marca
 * I think I got it now.
 *
 * Revision 1.9  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.8  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.7  1992/02/18  16:18:43  dweber
 * Deleted the old-style palette functions.
 *
 * Revision 1.6  92/01/24  19:12:55  dweber
 * *** empty log message ***
 * 
 * Revision 1.5  91/11/04  12:57:55  dweber
 * Deleted (via comments) superfluous functions.  Later to be removed.
 * 
 * Revision 1.4  91/09/26  20:26:11  jplevyak
 * Use dtminit.h for access to internally global functions.
 * 
 * Revision 1.3  1991/06/11  15:21:05  sreedhar
 * disclaimer added
 *
 * Revision 1.2  1991/05/30  15:53:16  sreedhar
 * Changes for readMsg/writeMsg internal release
 *
 * Revision 1.1  1990/11/08  16:37:49  jefft
 * Initial revision
 *

#endif
*/

#include	<stdio.h>
#include	<string.h>

#include	"dtmint.h"
#include	"ris.h"


char		PAL[] = "PAL ";


#ifdef DTM_PROTOTYPES
void RISsetDimensions(char *h,int x,int y)
#else
void RISsetDimensions(h, x, y)
  char	*h;
  int	x, y;
#endif
{
  char	append[32];

  sprintf(append, "%s 2 %d %d ", RISdims, x, y);
  strcat(h, append);
}

#ifdef DTM_PROTOTYPES
int RISgetDimensions(char *h,int *x,int *y)
#else
int RISgetDimensions(h, x, y)
  char	*h;
  int	*x, *y;
#endif
{

  if ((h = dtm_find_tag(h, RISdims)) == NULL)
    return DTMERROR;
  else
    h = strchr(h, ' ')+1;

  /* skip rank */
  h = strchr(h, ' ')+1;

  *x = atoi(h);
  h = strchr(h, ' ') + 1;
  *y = atoi(h);

  return 0;
}
