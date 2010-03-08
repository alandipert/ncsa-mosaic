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
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/sdsF.c,v 1.1.1.1 1995/01/11 00:03:03 alanb Exp $
**
**********************************************************************/

#ifdef RCSLOG

 $Log: sdsF.c,v $
 Revision 1.1.1.1  1995/01/11 00:03:03  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:40:25  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:34  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:14  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:43  marca
 * I think I got it now.
 *
 * Revision 1.5  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 

#endif


#include        <stdio.h>
#include        <string.h>
#include		<fortran.h>

#include        "dtmint.h"
#include        "sds.h"


extern int      atoi();
extern double   atof();
extern	char *	dtm_find_tag();



/*************************************************************************
**
** Cray version of SDS macros
**
*************************************************************************/

/* SDSsetClass */
void SDSSC(s)
  _fcd	s;
{
  strcat(strcpy(_fcdtocp(s), SDSclass), " ");
}

/* SDScompareClass */
long SDSCC(s)
  _fcd	s;
{
  return _btol(!strncmp(_fcdtocp(s), SDSclass, strlen(SDSclass)));
}


/* SDSheaderLength */
#undef SDSHL
long SDSHL(s)
  _fcd	s;
{
  return _fcdlen(s);
}


/* SDSsetType */
void SDSSDT(s, type)
  _fcd	s;
  int	*type;
{
  dtm_set_int(_fcdtocp(s), DTMtype, *type);
}


/* SDSgetType */
void SDSGDT(s, type)
  _fcd	s;
  int	*type;
{
  dtm_get_int(_fcdtocp(s), DTMtype, *type);
}


/* SDSsetTitle */
void SDSST(s, t)
  _fcd  s, t;
{
  int	i;
  char	*tptr = _fcdtocp(t);

  strcat(strcat(_fcdtocp(s), DTMtitle), " '");

  for (i=0; i< _fcdlen(t); i+=1)
    if (*tptr == ' ' && *(tptr+1) == ' ')
      break;

  strcat(strncat(_fcdtocp(s), _fcdtocp(t), i), "' ");
}


/* SDSgetTitle */
void SDSGT(s, t)
  _fcd  s, t;
{
  dtm_get_int(_fcdtocp(s), DTMtype, _fcdtocp(t), _fcdlen(t));;
}


/*************************************************************************
**
** Cray version of SDS functions
**
*************************************************************************/

/* SDSsetDimensions */
void SDSSD(s, rank, dims)
  _fcd	s;
  int   *rank, *dims;
{
  char  *h = _fcdtocp(s);
  char	num[8];
  int   i;


  sprintf(num, "%d ", *rank);
  strcat(h, SDSdims); strcat(h, " ");
  strcat(h, num);

  for (i=0; i< *rank; i+=1)  {
    sprintf(num, "%d ", dims[i]);
    strcat(h, num);
    }
}

/* SDSgetDimensions */
int SDSGD(s, rank, dims, len)
  _fcd	s;
  int   *rank, *dims, *len;
{
  char	*h = _fcdtocp(s);
  int   i;


  if ((h = dtm_find_tag(h, SDSdims)) == NULL)
    return DTMERROR;
  else
    h = strchr(h, ' ')+1;

  *rank = atoi(h);

  for (i=0; i<*rank && i<*len; i+=1)
    if ((h = strchr(h, ' ')) == NULL)
      return DTMERROR;
    else
      dims[i] = atoi(++h);

  return 0;
}


/* SDSsetMinMax */
void SDSSMM(s, min, max)
  _fcd	s;
  float *min, *max;
{
  char  *h = _fcdtocp(s);
  char	num[12];


  strcat(h, SDSminmax);  strcat(h, " ");

  sprintf(num, "%f ", *min);
  strcat(h, num);

  sprintf(num, "%f ", *max);
  strcat(h, num);
}


/* SDSgetMinMax */
long SDSGMM(s, min, max)
  _fcd	s;
  float *min, *max;
{
  char	*h = _fcdtocp(s);

  if ((h = dtm_find_tag(h, SDSminmax)) == NULL)
    return _btol(0);
  else
    h = strchr(h, ' ')+1;


  *min = atof(h);

  h = strchr(h, ' ') + 1;
  *max = atof(h);

  return _btol(1);
}


/* SDSnumElements */
long SDSNE(rank, dims)
  int	*rank, *dims;
{
  int   size;

  size = *dims++;
  while (--(*rank) > 0)
    size *= *dims++;

  return size;
}
