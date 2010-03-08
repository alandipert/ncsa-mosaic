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


#ifdef RCSLOG

 $Log: dtmf77.c,v $
 Revision 1.1.1.1  1995/01/11 00:02:59  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:39:40  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:31  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:11  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:16  marca
 * I think I got it now.
 *
 * Revision 1.7  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 
 * Revision 1.6  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.5  1991/11/15  23:01:21  jefft
 * Added quality of service options to DTMMOP and DTMMIP
 *
 * Revision 1.4  1991/06/11  15:22:00  sreedhar
 * disclaimer added
 *
 * Revision 1.3  1991/05/30  15:53:35  sreedhar
 * Changes for readMsg/writeMsg internal release
 *
 * Revision 1.2  1991/02/01  10:41:40  jefft
 * Fixed bug with DTMBR, the _cptofcd was not used correctly.
 *
 * Revision 1.1  90/11/08  16:31:46  jefft
 * Initial revision
 * 

#endif


#include	<stdio.h>
#include	<fortran.h>
#include	"dtm.h"



#ifdef DTM_PROTOTYPES
int DTMMIP(_fcd s)
#else
int DTMMIP(s)
  _fcd	s;
#endif
{
  int	len;
  char	portname[128];

  len = _fcdlen(s);
  strncpy(portname, _fcdtocp(s), len);
  portname[len] = '\0';

  return DTMmakeInPort(portname, DTM_DEFAULT);
}


#ifdef DTM_PROTOTYPES
int DTMMOP(_fcd s)
#else
int DTMMOP(s)
  _fcd	s;
#endif
{
  int	len;
  char	portname[128];

  len = _fcdlen(s);
  strncpy(portname, _fcdtocp(s), len);
  portname[len] = '\0';

  return DTMmakeOutPort(portname, DTM_DEFAULT);
}


#ifdef DTM_PROTOTYPES
int DTMGPA(int *p,_fcd s,int *len)
#else
int DTMGPA(p, s, len)
  int	*p, *len;
  _fcd	s;
#endif
{  
  char	portaddr[128];


  if (DTMgetPortAddr(*p, portaddr, sizeof portaddr) == DTMERROR)
    return DTMERROR;

  if (strlen(portaddr) > *len)  {
    strncpy(_fcdtocp(s), portaddr, *len); 
    return DTMERROR;
    }

  else  {
    strncpy(_fcdtocp(s), portaddr, strlen(portaddr));
    return 0;
    }
}


#ifdef DTM_PROTOTYPES
int DTMAR(int *p)
#else
int DTMAR(p)
  int	*p;
#endif
{
  return DTMavailRead(*p);
}


#ifdef DTM_PROTOTYPES
int DTMBR(int *p,_fcd header,int *size)
#else
int DTMBR(p, header, size)
  int		*p, *size;
  _fcd		header;
#endif
{

  return DTMbeginRead(*p, _fcdtocp(header), *size);
}


#ifdef DTM_PROTOTYPES
int DTMRD(int *p,char *ds,int *size,DTMTYPE type)
#else
int DTMRD(p, ds, size, type)
  int		*p, *size;
  char		*ds;
  DTMTYPE	*type;
#endif
{
  return DTMreadDataset(*p, ds, *size, *type);
}


#ifdef DTM_PROTOTYPES
int DTMER(int *p)
#else
int DTMER(p)
  int	*p;
#endif
{
  return DTMendRead(*p);
}


#ifdef DTM_PROTOTYPES
int DTMAW(int *p)
#else
int DTMAW(p)
  int		*p;
#endif
{
  return DTMavailWrite(*p);
}


#ifdef DTM_PROTOTYPES
int DTMBW(int *p,_fcd header,int *size)
#else
int DTMBW(p, header, size)
  int		*p, *size;
  _fcd		header;
#endif
{
  int	len;
  char	buffer[DTM_MAX_HEADER];

  len = _fcdlen(header);
  strncpy(buffer, _fcdtocp(header), len);
  buffer[len] = '\0';
  
  return DTMbeginWrite(*p, buffer, len+1);
}


#ifdef DTM_PROTOTYPES
int DTMSD(int *p,char *ds,int *size,DTMTYPE type)
#else
int DTMSD(p, ds, size, type)
  int		*p, *size;
  char		*ds;
  DTMTYPE	*type;
#endif
{
  return DTMwriteDataset(*p, ds, *size, *type);
}


#ifdef DTM_PROTOTYPES
int DTMEW(int *p)
#else
int DTMEW(p)
  int		*p;
#endif
{
  return DTMendWrite(*p);
}
