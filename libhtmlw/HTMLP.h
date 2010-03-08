/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright (C) 1993, Board of Trustees of the University of Illinois      *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/

#ifndef HTMLP_H
#define HTMLP_H

#include "HTML.h"

#ifdef MOTIF
#include <Xm/XmP.h>
# ifdef MOTIF1_2
#  include <Xm/ManagerP.h>
# endif /* MOTIF1_2 */
#else
#include <X11/IntrinsicP.h>
#include <X11/ConstrainP.h>
#endif /* MOTIF */

#include <X11/Xatom.h>
#ifndef VMS
#include <X11/Xmu/Atoms.h>
#else
#include <XMU/Atoms.h>
#endif

/*
 * Used for special images
 */
#define INTERNAL_IMAGE	"internal-"


/*  New fields for the HTML widget class */
typedef struct _HTMLClassPart
{
	int none;	/* no extra HTML class stuff */
} HTMLClassPart;


typedef struct _HTMLClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
#ifdef MOTIF
	XmManagerClassPart	manager_class;
#endif /* MOTIF */
	HTMLClassPart		html_class;
} HTMLClassRec;


extern HTMLClassRec htmlClassRec;


/* New fields for the HTML widget */
typedef struct _HTMLPart
{
	/* Resources */
	Dimension		margin_width;
	Dimension		margin_height;

	Widget			view;
	Widget			hbar;
	Widget			vbar;
        Widget                  frame;
	Boolean			hbar_top;
	Boolean			vbar_right;

	XtCallbackList		anchor_callback;
	XtCallbackList		link_callback;
	XtCallbackList		form_callback;

	char			*title;
	char			*raw_text;
	char			*header_text;
	char			*footer_text;
/*
 * Without motif we have to define our own forground resource
 * instead of using the manager's
 */
#ifndef MOTIF
	Pixel			foreground;
#endif
	Pixel			anchor_fg;
	Pixel			visitedAnchor_fg;
	Pixel			activeAnchor_fg;
	Pixel			activeAnchor_bg;

        Boolean                 body_colors;
        Boolean                 body_images;

	int			bg_image;

	Pixmap			bgmap_SAVE;
	Pixmap			bgclip_SAVE;
        int                     bg_height;
        int                     bg_width; 

        Pixel                   foreground_SAVE;
	Pixel			anchor_fg_SAVE;
	Pixel			visitedAnchor_fg_SAVE;
	Pixel			activeAnchor_fg_SAVE;
	Pixel			activeAnchor_bg_SAVE;
	Pixel			top_color_SAVE;
	Pixel			bottom_color_SAVE;    
        Pixel                   background_SAVE;
    
	int			num_anchor_underlines;
	int			num_visitedAnchor_underlines;
	Boolean			dashed_anchor_lines;
	Boolean			dashed_visitedAnchor_lines;
	Boolean			fancy_selections;
	Boolean			border_images;
	Boolean			delay_images;
	Boolean			is_index;
	int			percent_vert_space;

	XFontStruct		*font;
	XFontStruct		*italic_font;
	XFontStruct		*bold_font;
	XFontStruct		*meter_font;
	XFontStruct		*toolbar_font;
	XFontStruct		*fixed_font;
	XFontStruct		*fixedbold_font;
	XFontStruct		*fixeditalic_font;
	XFontStruct		*header1_font;
	XFontStruct		*header2_font;
	XFontStruct		*header3_font;
	XFontStruct		*header4_font;
	XFontStruct		*header5_font;
	XFontStruct		*header6_font;
	XFontStruct		*address_font;
	XFontStruct		*plain_font;
	XFontStruct		*plainbold_font;
	XFontStruct		*plainitalic_font;
	XFontStruct		*listing_font;
/* amb */
        XFontStruct             *supsub_font;
/* end amb */

        XtPointer		previously_visited_test;
        XtPointer		resolveImage;
        XtPointer		resolveDelayedImage;
        
        XtPointer               pointer_motion_callback;

	/* PRIVATE */
	Dimension		max_pre_width;
	Dimension		view_width;
	Dimension		view_height;
	int			doc_width;
	int			doc_height;
	int			scroll_x;
	int			scroll_y;
	Boolean			use_hbar;
	Boolean			use_vbar;
	struct ele_rec		*formatted_elements;
	int			line_count;
	struct ele_rec		**line_array;
	struct ele_rec		*select_start;
	struct ele_rec		*select_end;
	int			sel_start_pos;
	int			sel_end_pos;
	struct ele_rec		*new_start;
	struct ele_rec		*new_end;
	int			new_start_pos;
	int			new_end_pos;
	struct ele_rec		*active_anchor;
	GC			drawGC;
	int			press_x;
	int			press_y;
	Time			but_press_time;
	Time			selection_time;
	struct mark_up		*html_objects;
	struct mark_up		*html_header_objects;
	struct mark_up		*html_footer_objects;
	struct ref_rec		*my_visited_hrefs;
	struct delay_rec	*my_delayed_images;
	WidgetInfo		*widget_list;
	FormInfo		*form_list;
	MapInfo			*map_list;
        struct ele_rec          *cached_tracked_ele;
        Boolean                 focus_follows_mouse;
	Boolean			obscured;
} HTMLPart;


typedef struct _HTMLRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
#ifdef MOTIF
	XmManagerPart		manager;
#endif /* MOTIF */
	HTMLPart		html;
} HTMLRec;

/*
 * to reduce the number of MOTIF/ATHENA ifdefs around the code
 * we use some generalized constants
x */
#ifdef MOTIF
#   define XxNx      XmNx
#   define XxNy      XmNy
#   define XxNwidth  XmNwidth
#   define XxNheight XmNheight
#   define XxNset    XmNset
#   define XxNvalue  XmNvalue
#else
#   define XxNx      XtNx
#   define XxNy      XtNy
#   define XxNwidth  XtNwidth
#   define XxNheight XtNheight
#   define XxNset    XtNstate
#   define XxNvalue  XtNstring
#endif /* MOTIF */


#endif /* HTMLP_H */
