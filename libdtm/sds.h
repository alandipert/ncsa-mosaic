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
**  Scientific Data Set include file - contains constants used with the
**	SDS header type.
**
**************************************************************************/

/*********************************************************************
**
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/sds.h,v 1.4 1996/02/18 23:40:18 spowers Exp $
**
**********************************************************************/

/*
 * Revision 1.1.1.1  1993/07/04  00:03:14  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:42  marca
 * I think I got it now.
 *
 * Revision 1.8  92/04/30  20:27:37  jplevyak
 * Changed Version to 2.3
 * 
 * Revision 1.7  1992/03/16  21:03:18  creiman
 * Changed SDSsetMinMax prototype to use floats. NOT related to the change in
 * sds.c for SDSgetMinMax. This is getting confusing...
 *
 * Revision 1.6  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.5  1992/01/30  19:32:29  jplevyak
 * Added prototypes and support for C++ type external definitions.
 *
 * Revision 1.4  92/01/24  19:13:51  dweber
 * Added header lenght macros
 * 
 * Revision 1.3  91/09/10  15:07:10  jefft
 * cleaned-up message class macros.
 * 
 * Revision 1.2  1991/06/11  15:22:52  sreedhar
 * disclaimer added
 *
 * Revision 1.1  1990/11/08  16:39:26  jefft
 * Initial revision
 *
*/


#define	SDSclass		"SDS"
#define SDSsize			256

/* SDS specific tags */
#define SDSdims			"DIM"
#define SDSminmax		"MM"


/* SDS specific macros */
#define SDSsetClass(h)			dtm_set_class(h, SDSclass)
#define SDScompareClass(h)		dtm_compare_class(h, SDSclass)

/* SDS rename macros */
#define SDSheaderLength			dtm_header_length
#define	SDSHL				dtm_header_length

#define SDSsetType			dtm_set_type
#define	SDSgetType			dtm_get_type

#define SDSsetTitle			dtm_set_title
#define SDSgetTitle			dtm_get_title

/* SDS routines */

#ifdef __cplusplus
extern "C" {
#endif

extern void SDSsetDimensions    DTM_PROTO(( char* h, int rank, int *dims ));
extern int SDSgetDimensions     DTM_PROTO(( char* h, int * rank, int* dims, 
									int len ));
extern int SDSgetRank           DTM_PROTO(( char* h, int * rank ));
extern int SDSnumElements       DTM_PROTO(( int rank, int *dims ));
extern void SDSsetMinMax        DTM_PROTO(( char* h, float min, float max));
extern int SDSgetMinMax         DTM_PROTO(( char* h, float* min, float* max));

#ifdef __cplusplus
};
#endif
