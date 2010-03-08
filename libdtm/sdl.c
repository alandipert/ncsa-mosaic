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
**  sdl.c - Surface Description Language
**	
*************************************************************************/

/*
 * $Log: sdl.c,v $
 * Revision 1.1.1.1  1995/01/11 00:03:02  alanb
 * New CVS source tree, Mosaic 2.5 beta 4
 *
 * Revision 2.5  1994/12/29  23:40:17  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:33  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:14  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:38  marca
 * I think I got it now.
 *
 * Revision 1.9  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.8  1992/04/06  15:58:56  jplevyak
 * Fixed levels of indirection on some arguments for non-prototype
 * machines.
 *
 * Revision 1.7  92/04/03  12:39:33  dweber
 * Fixed SDLbounds bug.
 * 
 * Revision 1.6  92/04/03  12:33:10  dweber
 * Added bounding box functions.
 * 
 * Revision 1.5  91/09/10  15:09:00  jefft
 * I removed all functionality from this file, HA!
 * 
 * Revision 1.4  1991/07/18  16:29:15  jefft
 * corrected the spelling of "primitive"
 *
 * Revision 1.3  1991/06/11  15:20:56  sreedhar
 * disclaimer added
 *
 * Revision 1.2  1991/05/14  14:17:06  jefft
 * cleaned-up and started using dtmmisc routines.
 *
 * Revision 1.1  90/11/08  16:38:33  jefft
 * Initial revision
 * 
 */



#include	<stdio.h>
#include	<string.h>

#include	"dtm.h"
#include	"sdl.h"


#ifdef DTM_PROTOTYPES
void SDLsetBoundingBox(char *header, float *min, float *max)
#else
void SDLsetBoundingBox(header, min, max)
char		*header;
float		*min, *max;
#endif
{
   char         num[128];

   strcat(header, SDLbounds);
   strcat(header, " ");

   sprintf(num, "%f %f %f %f %f %f ",
		min[0], min[1], min[2], max[0], max[1], max[2]);
   strcat(header, num);

}


#ifdef DTM_PROTOTYPES
int SDLgetBoundingBox(char *header, float *min, float *max)
#else
int SDLgetBoundingBox(header, min, max)
char            *header;
float           *min, *max;
#endif
{
   int   i;

   if ((header = dtm_find_tag(header, SDLbounds)) == NULL)
      return DTMERROR;
   else
      header = strchr(header, ' ')+1;

   for (i=0; i<3; i+=1)
      if ((header = strchr(header, ' ')) == NULL)
         return DTMERROR;
      else
         min[i] = atoi(++header);

   for (i=0; i<3; i+=1)
      if ((header = strchr(header, ' ')) == NULL)
         return DTMERROR;
      else
         max[i] = atoi(++header);

   return 0;
}
