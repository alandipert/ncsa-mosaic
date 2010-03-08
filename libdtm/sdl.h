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
**  Surface Description Language include file - contains constants used
**     with the SDL header type.  
**
**
**
**************************************************************************/

/*
 * $Log: sdl.h,v $
 * Revision 1.3  1996/02/18 23:40:17  spowers
 * PROTO -> DTM_PROTO
 *
 * Revision 1.2  1995/01/12 02:33:38  spowers
 * Finally...Dave complained that I blew away the RCSLOGs without leaving the
 * actual entries there as comments...so I put them back in as COMMENTS! Not as
 * RCSLOG. Hopefully our woeful begining with cvs will now become a happy one.
 *
 * Revision 1.1.1.1  1993/07/04  00:03:14  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:39  marca
 * I think I got it now.
 *
 * Revision 1.13  92/04/30  20:27:05  jplevyak
 * Changed Version to 2.3
 * 
 * Revision 1.12  1992/04/03  12:39:42  dweber
 * Fixed SDLbounds bug.
 *
 * Revision 1.11  92/04/03  12:33:23  dweber
 * Added bounding box functions.
 * 
 * Revision 1.10  1992/02/25  20:36:35  dweber
 * Fixed another prototyping clash, SDLgetPrimitive.
 *
 * Revision 1.9  1992/01/31  09:46:35  dweber
 * Added #define SDL_INC to avoid duplicate inclusions.
 *
 * Revision 1.8  1992/01/24  19:14:33  dweber
 * Added header length macros.
 *
 * Revision 1.7  91/11/01  10:10:28  dweber
 * Added SDLtristrip
 * 
 * Revision 1.6  91/09/10  15:07:55  jefft
 * cleaned-up message class macros.
 * 
 * Revision 1.5  1991/07/25  22:22:27  jefft
 * Added transformation triplet enum types
 *
 * Revision 1.4  1991/07/18  16:29:43  jefft
 * corrected the spelling of "primitive" and added macros for backwards
 * compatibility.
 *
 * Revision 1.3  1991/06/11  15:22:58  sreedhar
 * disclaimer added
 *
 * Revision 1.2  1991/03/01  10:27:13  jefft
 * Added SDLcompareClass functions and enum for triplet types
 *
 * Revision 1.1  90/11/08  16:38:51  jefft
 * Initial revision
 * 
 */

#ifndef SDL_INC
#define SDL_INC

/*
 * Surface description language class typedefs and macros
 */

#define	SDLclass		"SDL"
#define SDLsize			256
#define SDLprimitive		"PT"
#define SDLbounds		"BB"

typedef enum {
   SDLpoint = 1, SDLlineseg, SDLtriangle, SDLtristrip, SDLquad, SDLsphere
   } SDLprim_t;

typedef enum {
   SDLposition, SDLcolor, SDLnormal, SDLtranslate, SDLrotate, SDLscale
   } SDLtriplet;

typedef enum {
   SDLflat, SDLgouraud, SDLphong
   } SDLshade_t;

typedef enum {
   SDLpoints, SDLlines, SDLpolygons
   } SDLdraw_t;

#define SDLsetClass(h)		dtm_set_class((h), SDLclass)
#define	SDLcompareClass(h)	dtm_compare_class((h), SDLclass)

#define SDLsetPrimitive(h, p)	dtm_set_int((h), SDLprimitive, (p))
#define SDLgetPrimitive(h, p)	((*(p)=(int)SDLpoint), \
				 (dtm_get_int((h), SDLprimitive, (int *)(p))))

#define	SDLsetTitle		dtm_set_title
#define	SDLgetTitle		dtm_get_title

#define SDLheaderLength		dtm_header_length
#define SDLHL			dtm_header_length

#ifdef __cplusplus
extern "C" {
#endif

extern void SDLsetBoundingBox	DTM_PROTO(( char* h, float *min, float *max));
extern int SDLgetBoundingBox	DTM_PROTO(( char* h, float *min, float *max));

#ifdef __cplusplus
};
#endif


/*
 * View control matrix class typedefs and macros
 */
#define VCTRLclass		"VCTRL"
#define VCTRLsize		256
#define VCTRLgtm		"GTM "

#define VCTRLsetClass(h)	dtm_set_class((h), VCTRLclass)
#define	VCTRLcompareClass(h)	dtm_compare_class((h), VCTRLclass)

#define VCTRLsetGTM(h)		strcat(h, VCTRLgtm)
#define VCTRLgetGTM(h)		((dtm_find_tag(h, VCTRLgtm) == NULL) ? 0 : 1)

#define	VCTRLsetTitle		dtm_set_title
#define	VCTRLgetTitle		dtm_get_title

#define VCTRLheaderLength	dtm_header_length
#define VCTRLHL			dtm_header_length


#endif /* SDL_INC */
