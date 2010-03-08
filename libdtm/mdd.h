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

/**************************************************************************
**
**  Multi-Dimensional Data set include file - contains constants used
**	with the MDD & PAL header type.
**
**
**
**************************************************************************/

/*********************************************************************
**
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/mdd.h,v 1.4 1996/02/18 23:40:14 spowers Exp $
**
**********************************************************************/

/*
 * Revision 1.1.1.1  1993/07/04  00:03:12  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:30  marca
 * I think I got it now.
 *
 * Revision 1.4  92/04/30  20:27:05  jplevyak
 * Changed Version to 2.3
 * 
 * Revision 1.3  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.2  1991/06/11  15:23:09  sreedhar
 * disclaimer added
 *
 * Revision 1.1  1990/11/08  16:37:32  jefft
 * Initial revision
 *
*/


#define	MDDclass		"MDD"
#define MDDsize			256

#define PALclass		"PAL"
#define PALsize			64

/* MDD specific tags */
#define	MDDdims			"DIM"
#define MDDminmax		"MM"


/* MDD & PAL specific macros */
#define MDDsetClass(h)			DTMsetClass(h, MDDclass)
#define MDDcompareClass(h)		DTMcompareClass(h, MDDclass)

#define PALsetClass(h)			DTMsetClass(h, PALclass)
#define PALcompareClass(h)		DTMcompareClass(h, PALclass)

#define MDDsetPalette(h, t)		dtm_set_char(h, PALclass, t)
#define MDDgetPalette(h, t, l)		dtm_get_char(h, PALclass, t, l)

/* MDD & PAL rename macros */
#define MDDheaderLength			DTMheaderLength
#define	MDDHL				DTMheaderLength

#define PALheaderLength			DTMheaderLength
#define PALHL				DTMheaderLength

#define MDDsetTitle			DTMsetTitle
#define MDDgetTitle			DTMgetTitle

#define PALsetTitle			DTMsetTitle
#define PALgetTitle			DTMgetTitle

#define MDDsetType			DTMsetType
#define	MDDgetType			DTMgetType


/* MDD routines */
#ifdef __cplusplus
extern "C" {
#endif

extern void MDDsetDimensions DTM_PROTO((char *h, int rank, int *dims));
extern int  MDDgetDimensions DTM_PROTO((char *h, int *rank, int *dims, int len));
extern int  MDDnumElements DTM_PROTO((int rank, int *dims));
extern void MDDsetMinMax DTM_PROTO((char *h, float min, float max));
extern int  MDDgetMinMax DTM_PROTO((char *h, float *min, float *max));
extern void MDDfindMinMax DTM_PROTO((char *h, float *mdd, float *min, float *max));

#ifdef __cplusplus
};
#endif
