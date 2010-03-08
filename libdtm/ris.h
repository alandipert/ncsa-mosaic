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
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/ris.h,v 1.3 1995/01/12 02:33:37 spowers Exp $
**
**********************************************************************/

/*
 * Revision 1.1.1.1  1993/07/04  00:03:13  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:36  marca
 * I think I got it now.
 *
 * Revision 1.11  92/04/30  20:27:05  jplevyak
 * Changed Version to 2.3
 * 
 * Revision 1.10  1992/02/21  13:48:04  dweber
 * Fixed prototyping problem in RISgetType.
 *
 * Revision 1.9  92/02/18  16:22:04  dweber
 * Added functions for setting/getting associated palette names.
 * (RISsetPaletteName and RISgetPaletteName)
 * 
 * Revision 1.8  92/01/24  19:12:55  dweber
 * *** empty log message ***
 * 
 * Revision 1.7  91/11/18  11:48:57  dweber
 * Returned the RISsize definition and changed PALsize to PALelements for same reason
 * 
 * Revision 1.6  91/11/04  15:06:08  dweber
 * Added PAL size commands
 * 
 * Revision 1.5  91/11/04  12:58:36  dweber
 * Added RIS types and removed 8/24 bit distinction from class level.
 * 
 * Revision 1.4  91/09/10  15:07:32  jefft
 * cleaned-up message class macros.
 * 
 * Revision 1.3  1991/06/11  15:23:04  sreedhar
 * disclaimer added
 *
 * Revision 1.2  1991/03/14  23:39:38  jefft
 * add dtm_compare_class calls
 *
 * Revision 1.1  90/11/08  16:38:04  jefft
 * Initial revision
 * 
*/


/*
 * PALETTE constants and macros
 */
#define	PALclass	"PAL"
#define PALelements	"PE"
#define	PALsize		128

#define	PALsetClass(h)		dtm_set_class(h, PALclass)
#define	PALcompareClass(h)	dtm_compare_class(h, PALclass)

#define PALsetSize(h, s)	dtm_set_int((h), PALelements, (s))
#define PALgetSize(h, s)	((*(s)=(int)256), \
				 (dtm_get_int((h), PALelements, (s))))

/*
 * PALETTE rename macros
 */
#define PALheaderLength		dtm_header_length
#define PALHL			dtm_header_length

#define PALsetTitle		dtm_set_title
#define PALgetTitle		dtm_get_title



/*
 * RASTER IMAGE constants and macros
 */


/*
 * standard image types: 8 bit colormap entries or 24-bit RGB entries.
 */
typedef	enum {
	RIS8BIT = 0,
	RIS24BIT
} RISTYPE;

#define	RISclass	"RIS"
#define RIStype		"RT"
#define	RISdims		"DIM"
#define RISpalette	"APN"
#define RISsize		256

#define RISsetClass(h)		dtm_set_class(h, RISclass)
#define	RIScompareClass(h)	dtm_compare_class(h, RISclass)

#define RISsetType(h, t)	((dtm_set_int((h), RIStype, (t))), \
				 (dtm_set_type((h), DTM_CHAR)))
#define RISgetType(h, t)	((*(t)=(int)RIS24BIT), \
				 (dtm_get_int((h), RIStype, (int *)(t))))

/*
 * RIS rename macros
 */
#define RISheaderLength		dtm_header_length
#define RISHL			dtm_header_length

#define RISsetTitle		dtm_set_title
#define RISgetTitle		dtm_get_title

#define RISsetPaletteName(h, c)		dtm_set_char((h), RISpalette, (c))
#define RISgetPaletteName(h, c, l)	dtm_get_char((h), RISpalette, (c), (l))
