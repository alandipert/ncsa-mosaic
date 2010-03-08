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

/************************************************************************
**
**  mdd.c - Multi-Dimensional Data set routines
**	
*************************************************************************/

/*********************************************************************
**
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/mdd.c,v 1.2 1995/10/13 06:33:17 spowers Exp $
**
**********************************************************************/
/*
#ifdef RCSLOG

 $Log: mdd.c,v $
 Revision 1.2  1995/10/13 06:33:17  spowers
 Solaris support added.

 Revision 1.1.1.1  1995/01/11 00:03:00  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:40:02  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:32  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:12  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:29  marca
 * I think I got it now.
 *
 * Revision 1.5  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.4  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.3  1991/09/26  20:24:51  jplevyak
 * removed atoi() and atof() in favor of <stdlib.h>.
 *
 * Revision 1.2  1991/06/11  15:22:06  sreedhar
 * disclaimer added
 *
 * Revision 1.1  1990/11/08  16:37:13  jefft
 * Initial revision
 *

#endif
*/

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

#include	"dtmint.h"
#include	"mdd.h"



#ifdef DTM_PROTOTYPES
void MDDsetDimensions(char *h,int rank,int *dims)
#else
void MDDsetDimensions(h, rank, dims)
  char	*h;
  int	rank, *dims;
#endif
{
  char	num[8];
  int	i;

  sprintf(num, "%d ", rank);
  strcat(h, MDDdims); strcat(h, " ");
  strcat(h, num);

  for (i=0; i<rank; i+=1)  {
    sprintf(num, "%d ", dims[i]);
    strcat(h, num);
    }
}


#ifdef DTM_PROTOTYPES
int MDDgetDimensions(char *h,int *rank,int *dims,int len)
#else
int MDDgetDimensions(h, rank, dims, len)
  char	*h;
  int	*rank, *dims, len;
#endif
{
  int	i;

  if ((h = dtm_find_tag(h, MDDdims)) == NULL)
    return DTMERROR;
  else
    h = strchr(h, ' ')+1;

  *rank = atoi(h);

  for (i=0; i<*rank && i<len; i+=1)
    if ((h = strchr(h, ' ')) == NULL)
      return DTMERROR;
    else
      dims[i] = atoi(++h);

  return 0;
}


#ifdef DTM_PROTOTYPES
int MDDnumElements(int rank,int *dims)
#else
int MDDnumElements(rank, dims)
  int	rank, *dims;
#endif
{
  int	size;

  size = *dims++;
  while (--rank > 0)
    size *= *dims++;

  return size;
}


#ifdef DTM_PROTOTYPES
void MDDsetMinMax(char *h,float min,float max)
#else
void MDDsetMinMax(h, min, max)
  char	*h;
  float	min, max;
#endif
{
  char	num[12];

  strcat(h, MDDminmax);  strcat(h, " ");

  sprintf(num, "%f ", min);
  strcat(h, num);

  sprintf(num, "%f ", max);
  strcat(h, num);
}


#ifdef DTM_PROTOTYPES
int MDDgetMinMax(char *h,float *min,float *max)
#else
int MDDgetMinMax(h, min, max)
  char	*h;
  float	*min, *max;
#endif
{

  if ((h = dtm_find_tag(h, MDDminmax)) == NULL)
    return DTMERROR;
  else
    h = strchr(h, ' ')+1;


  *min = (float)atof(h);

  h = strchr(h, ' ') + 1;
  *max = (float)atof(h);
  
  return 0;
}


#ifdef DTM_PROTOTYPES
void MDDfindMinMax(char *h,float *mdd,float *min,float *max)
#else
void MDDfindMinMax(h, mdd, min, max)
  char	*h;
  float	*mdd, *min, *max;
#endif
{
  int	i, rank, dims[10];

  MDDgetDimensions(h, &rank, dims, sizeof dims);
  rank = MDDnumElements(rank, dims);

  *min = *max = *mdd++;
  for (i=1; i<rank; mdd+=1, i+=1)
    if (*mdd < *min)
      *min = *mdd;
    if (*mdd > *max)
      *max = *mdd;
}
