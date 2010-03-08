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
**  sds.c - Scientific Data Set utility routines.
**	
*************************************************************************/
/*
#ifdef RCSLOG

 $Log: sds.c,v $
 Revision 1.2  1995/10/13 06:33:25  spowers
 Solaris support added.

 Revision 1.1.1.1  1995/01/11 00:03:02  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:40:20  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:33  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:14  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:40  marca
 * I think I got it now.
 *
 * Revision 1.12  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.11  1992/03/16  20:39:10  creiman
 * Changed declaration of SDSgetMinMax to match prototype. (Changed doubles
 * to floats)
 *
 * Revision 1.10  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.9  1992/01/30  19:31:14  jplevyak
 * Change float to double (standard coersion).
 * Needed to correct problem with protot
 * types.
 *
 * Revision 1.8  1991/12/19  09:32:18  jefft
 * Removed the single quote from the log message.  Even though the line
 * is #ifdefed out.  The SGI still pukes.
 *
 * Revision 1.7  91/12/18  02:42:41  dweber
 * Oops.  Guess I forgot to change getRanks name.
 * 
 * Revision 1.6  1991/12/18  02:41:53  dweber
 * Added SDSgetRank
 *
 * Revision 1.5  91/12/09  12:12:27  dweber
 * Deleted the function SDSconvert
 * 
 * Revision 1.4  91/09/26  20:20:52  jplevyak
 * Minor header file reorganization.
 * 
 * Revision 1.3  91/09/13  18:01:21  jplevyak
 * Fixed minor bug w/extra argument in call.
 * 
 * Revision 1.2  91/06/11  15:20:50  sreedhar
 * disclaimer added
 * 
 * Revision 1.1  1990/11/08  16:39:09  jefft
 * Initial revision
 *

#endif
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef _ARCH_MSDOS
#include <nmpcip.h>
#else
#include <netinet/in.h>
#endif

#include "dtm.h"
#include "dtmint.h"
#include "sds.h"


#ifdef DTM_PROTOTYPES
void SDSsetDimensions(char *h,int rank,int *dims)
#else
void SDSsetDimensions(h, rank, dims)
  char	*h;
  int	rank, *dims;
#endif
{
  char	num[8];
  int	i;

  sprintf(num, "%d ", rank);
  strcat(h, SDSdims); strcat(h, " ");
  strcat(h, num);

  for (i=0; i<rank; i+=1)  {
    sprintf(num, "%d ", dims[i]);
    strcat(h, num);
    }
}


#ifdef DTM_PROTOTYPES
int SDSgetDimensions(char *h,int *rank,int *dims,int len)
#else
int SDSgetDimensions(h, rank, dims, len)
  char	*h;
  int	*rank, *dims, len;
#endif
{
  int	i;

  if ((h = dtm_find_tag(h, SDSdims)) == NULL)
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
int SDSgetRank(char *h,int *rank)
#else
int SDSgetRank(h, rank)
  char  *h;
  int   *rank;
#endif
{
  if ((h = dtm_find_tag(h, SDSdims)) == NULL)
    return DTMERROR;
  else
    h = strchr(h, ' ')+1;

  *rank = atoi(h);

  return 0;
}



#ifdef DTM_PROTOTYPES
void SDSsetMinMax(char *h,float min,float max)
#else
void SDSsetMinMax(h, min, max)
  char      *h;
  float    min, max;
#endif
{
  char	num[12];

  strcat(h, SDSminmax);  strcat(h, " ");

  sprintf(num, "%f ", min);
  strcat(h, num);

  sprintf(num, "%f ", max);
  strcat(h, num);
}


#ifdef DTM_PROTOTYPES
int SDSgetMinMax(char *h,float *min,float *max)
#else
int SDSgetMinMax(h, min, max)
  char	*h;
  float	*min, *max;
#endif
{

  if ((h = dtm_find_tag(h, SDSminmax)) == NULL)
    return DTMERROR;
  else
    h = strchr(h, ' ')+1;


  *min = (float)atof(h);

  h = strchr(h, ' ') + 1;
  *max = (float)atof(h);
  
  return 0;
}


#ifdef DTM_PROTOTYPES
int SDSnumElements(int rank,int *dims)
#else
int SDSnumElements(rank, dims)
  int	rank, *dims;
#endif
{
  int	size;

  size = *dims++;
  while (--rank > 0)
    size *= *dims++;

  return size;
}
