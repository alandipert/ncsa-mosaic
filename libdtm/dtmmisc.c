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
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/dtmmisc.c,v 1.2 1995/10/13 06:33:11 spowers Exp $
**
**********************************************************************/

/*
#ifdef RCSLOG

 $Log: dtmmisc.c,v $
 Revision 1.2  1995/10/13 06:33:11  spowers
 Solaris support added.

 Revision 1.1.1.1  1995/01/11 00:02:59  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:39:48  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:31  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:11  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:20  marca
 * I think I got it now.
 *
 * Revision 1.8  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.7  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.6  1991/10/10  14:29:30  jplevyak
 * Included "string.h" instead of declaring externs atoi atof().
 *
 * Revision 1.5  91/09/26  20:24:21  jplevyak
 * Added debug.h as include file.
 * 
 * Revision 1.4  1991/06/11  15:19:36  sreedhar
 * disclaimer added
 *
 * Revision 1.3  1991/05/14  14:13:48  jefft
 * modified dtm_set_char to delimit string with "'"
 *
 * Revision 1.2  1991/04/29  16:44:56  jefft
 * increased length of num array in dtm_set_float
 *
 * Revision 1.1  90/11/08  16:32:18  jefft
 * Initial revision
 * 

#endif
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#ifdef _ARCH_MSDOS
#include <nmpcip.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#endif

#include "dtmint.h"
#include "debug.h"

#ifndef min
#define		min(a, b)	(((a) <= (b)) ? (a) : (b))
#endif


/*************************************************************************
**
**  Header Utilitiy Functions
**
**************************************************************************/

#ifdef DTM_PROTOTYPES
void dtm_set_char(char *h,char *tag,char *s)
#else
void dtm_set_char(h, tag, s)
  char	*h, *tag, *s;
#endif
{

  strcat(h, tag);  strcat(h, " '");
  strcat(h, s);    strcat(h, "' ");
}


#ifdef DTM_PROTOTYPES
void dtm_set_int(char *h,char *tag,int x)
#else
void dtm_set_int(h, tag, x)
  char	*h, *tag;
  int	x;
#endif
{
  char	num[8];

  strcat(h, tag);  strcat(h, " ");

  sprintf(num, "%d ", x);
  strcat(h, num);
}


#ifdef DTM_PROTOTYPES
void dtm_set_float( char * h, char * tag, float x)
#else
void dtm_set_float(h, tag, x)
  char	*h, *tag;
  float	x;
#endif
{
  char	num[20];

  strcat(h, tag);  strcat(h, " ");

  sprintf(num, "%f ", x);
  strcat(h, num);
}


#ifdef DTM_PROTOTYPES
char *dtm_find_tag(char *h,char *tag)
#else
char *dtm_find_tag(h, tag)
  char	*h, *tag;
#endif
{
  int	len;

  len = strlen(tag);

  while ((h = strchr(h, ' ')) != NULL)  {
    h++;
    if (!strncmp(h, tag, len) && *(h+len) == ' ')
      return h;
    }

  return NULL;
}


#ifdef DTM_PROTOTYPES
int dtm_get_char(char *h,char *tag,char *s,int l)
#else
int dtm_get_char(h, tag, s, l)
  char	*h, *tag, *s;
  int	l;
#endif
{

  /* set NULL string incase tag does not exist */
  *s = '\0';

  /* decrement length to save space for final NUL */
  l -= 1;

  /* no tag return error */
  if ((h = dtm_find_tag(h, tag)) == NULL)
    return DTMERROR;

  /* no field available, return error */
  if ((h = strchr(h, '\'')) == NULL)
    return DTMERROR;
  else
    h += 1;

  if ((tag = strchr(h, '\'')) == NULL)  {
    strncpy(s, h, min(strlen(h)+1, l));
    *(s+min(strlen(h)+1, l)) = '\0';
    }
  else  {
    strncpy(s, h, min(tag-h, l));
    *(s+min(tag-h, l)) = '\0';
    }

  return 0;
}


#ifdef DTM_PROTOTYPES
int dtm_get_int(char *h,char *tag,int *x)
#else
int dtm_get_int(h, tag, x)
  char	*h, *tag;
  int	*x;
#endif
{

  /* no tag return error */
  if ((h = dtm_find_tag(h, tag)) == NULL)
    return DTMERROR;

  /* no field available, return error */
  if ((h = strchr(h, ' ')) == NULL)
    return DTMERROR;
  else
    h += 1;

  *x = atoi(h);

  return 0;
}


#ifdef DTM_PROTOTYPES
int dtm_get_float(char *h,char *tag,float *x)
#else
int dtm_get_float(h, tag, x)
  char	*h, *tag;
  float	*x;
#endif
{

  /* no tag return error */
  if ((h = dtm_find_tag(h, tag)) == NULL)
    return DTMERROR;

  /* no field available, return error */
  if ((h = strchr(h, ' ')) == NULL)
    return DTMERROR;
  else
    h += 1;

  *x = atof(h);

  return 0;
}


#ifdef  NONO
#ifdef DTM_PROTOTYPES
void DTMsetType(char *h,DTMTYPE type)
#else
void DTMsetType(h, type)
  char		*h;
  DTMTYPE	type;
#endif
{
  char	num[8];

  strcat(h, DTMtype); strcat(h, " ");

  sprintf(num, "%d ", type);
  strcat(h, num);
}


#ifdef DTM_PROTOTYPES
DTMTYPE DTMgetType(char *h)
#else
DTMTYPE DTMgetType(h)
  char	*h;
#endif
{
  char	*f;

  if ((f = dtm_find_tag(h, DTMtype)) != NULL)
    return (DTMTYPE)atoi(f);
  else
    return DTM_FLOAT;
}


#ifdef DTM_PROTOTYPES
void DTMsetGroup(char *h,DTMCMD cmd,char *parent,char *self)
#else
void DTMsetGroup(h, cmd, parent, self)
  char		*h, *parent, *self;
  DTMCMD	cmd;
#endif
{
  char		num[12];

  strcat(h, "GRP ");
  sprintf(num, "%d ", (int)cmd);
  strcat(h, num);
  if (parent != NULL)
    strcat(h, parent);
  strcat(h, " ");
  if (self != NULL)
    strcat(h, self);
  strcat(h, " ");
}


#ifdef DTM_PROTOTYPES
int DTMgetGroup(char *h,DTMCMD cmd,char *parent,char *self)
#else
int DTMgetGroup(h, cmd, parent, self)
  char		*h, *parent, *self;
  DTMCMD	*cmd;
#endif
{
  int		len;

  *self = *parent = '\0';

  if ((h = dtm_find_tag(h, "GRP")) == NULL)
    return DTMERROR;

  h = strchr(h, ' ')+1;

  /* get DTM command */
  *cmd = (DTMCMD)atoi(h);
  h = strchr(h, ' ')+1;

  /* get parent name */
  if (*h != ' ')  {
    len = strchr(h, ' ')-h;
    strncpy(parent, h, len);
    *(parent+len) = '\0';
    }
  else
    *parent = '\0';
  
  h = strchr(h, ' ')+1;

  /* get self name */
  if (*h != ' ')  {
    len = strchr(h, ' ')-h;
    strncpy(self, h, len);
    *(self+len) = '\0';
    }
  else
    *self = '\0';

  return 1;
}
#endif
