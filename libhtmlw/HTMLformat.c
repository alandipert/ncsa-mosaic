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
#include "../config.h"
#ifndef VMS
#include <sys/time.h>
struct timeval Tv;
struct timezone Tz;
#else
#include <time.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include "HTMLP.h"


/*
 * I need my own is ispunct function because I need a closing paren
 * immediately after a word to act like punctuation.
 */
#define	MY_ISPUNCT(val)	(ispunct((int)(val)) || ((val) == ')'))

#define INDENT_SPACES	2
/* SWP -- Now using eptr->bwidth -- 02/08/96
#define IMAGE_BORDER	2 */
#define IMAGE_DEFAULT_BORDER	2

#define D_NONE		0
#define D_TITLE		1
#define D_TEXT		2
#define D_OLIST		3
#define D_ULIST		4

#ifndef DISABLE_TRACE
extern int htmlwTrace;
#endif

extern struct ele_rec *AddEle();
extern void FreeLineList();
extern void FreeObjList();
extern int SwapElements();
extern struct ele_rec **MakeLineList();
extern char *ParseMarkTag();
extern char *MaxTextWidth();
extern char *IsMapForm();
extern char *DelayedHRef();
extern int IsDelayedHRef();
extern int AnchoredHeight();
extern struct mark_up *HTMLParse();
extern struct ref_rec *FindHRef();
extern struct delay_rec *FindDelayedImage();
extern ImageInfo *NoImageData();
extern ImageInfo *DelayedImageData();
extern Pixmap NoImage();
extern Pixmap DelayedImage();
extern Pixmap InfoToImage();
extern int caseless_equal();
extern void clean_white_space();
extern void WidgetRefresh();
extern WidgetInfo *MakeWidget();
extern XFontStruct *GetWidgetFont();
extern void AddNewForm();
extern void PrepareFormEnd();
extern char *ComposeCommaList();
extern void FreeCommaList();
extern TableInfo *MakeTable();
extern void TableRefresh();

/* for selective image loading */
extern Boolean currently_delaying_images;

/*******************************/

/*SWP -- 8/14/95*/
int tableSupportEnabled;


/*
 * To allow arbitrary nesting of lists
 */
typedef struct dtype_rec {
	int type;		/* D_NONE, D_TITLE, D_TEXT, D_OLIST, D_ULIST */
	int count;
	int compact;
	struct dtype_rec *next;
} DescRec;


/*
 * To allow arbitrary nesting of font changes
 */
typedef struct font_rec {
	XFontStruct *font;
	struct font_rec *next;
} FontRec;

static DescRec BaseDesc;
static DescRec *DescType;
static DescRec *ListData;
static FontRec FontBase;
static FontRec *FontStack;
static XFontStruct *currentFont;
static XFontStruct *saveFont;
static unsigned long Fg;
static unsigned long Bg;
static int Width;
static int MaxWidth;
static int ElementId;
static int WidgetId;
static int LineNumber;
static int LineHeight;
static int LineBottom;
static int BaseLine;
static int TextIndent;
static int MarginW;
static int Ignore;
static int Preformat;
static int PF_LF_State; /* Pre-formatted linefeed state.  Hack for bad HTMLs */
static int NeedSpace;
static Boolean Internal;
static Boolean DashedUnderlines;
static Boolean Strikeout;
static int Underlines;
static int CharsInLine;
static int IndentLevel;
static struct ele_rec *Current;
static char *AnchorText;
static char *TitleText;
static char *TextAreaBuf;
static struct mark_up *Last;
static FormInfo *CurrentForm;
static MapInfo *CurrentMap=NULL; /* csi stuff -- swp */
static SelectInfo *CurrentSelect;
static int Superscript;             /* amb */
static int Subscript;
static XFontStruct *nonScriptFont;
static int InDocHead;
static int InUnderlined;
static int Centered; // SAM

/*
 * Turned out we were taking WAY too much time mallocing and freeing
 * memory when composing the lines into elements.  So this ineligent
 * method minimizes all that.
 */
#define COMP_LINE_BUF_LEN	1024
static char *CompLine = NULL;
static int CompLineLen = 0;
static char *CompWord = NULL;
static int CompWordLen = 0;



/*
 * Create a formatted element
 */
struct ele_rec *
CreateElement(hw, type, fp, x, y, edata, w, h, bw)
	HTMLWidget hw;
	int type;
	XFontStruct *fp;
	int x, y;
	char *edata, *w, *h;
	int bw;
{
	struct ele_rec *eptr;
	int baseline;

	if (fp != NULL)
	{
		baseline = fp->max_bounds.ascent;
	}
	else
	{
		baseline = LineHeight;
	}

	eptr = (struct ele_rec *)malloc(sizeof(struct ele_rec));
	if (eptr == NULL)
	{
		fprintf(stderr, "Cannot allocate space for element buffer\n");
		exit(1);
	}

	memset(eptr, 0, sizeof(struct ele_rec));

	eptr->type = type;
	eptr->pic_data = NULL;
	eptr->widget_data = NULL;
	eptr->table_data = NULL;
	eptr->font = fp;
	eptr->alignment = ALIGN_BOTTOM;
	eptr->selected = False;
	eptr->internal = Internal;
	eptr->strikeout = Strikeout;
	eptr->bwidth = bw;
	eptr->x = x;
	eptr->y = y;
	eptr->y_offset = 0;
	eptr->width = 0;
	eptr->line_number = LineNumber;
	eptr->line_height = LineHeight;
	eptr->fg = Fg;
	eptr->bg = Bg;
	eptr->underline_number = Underlines;
	eptr->dashed_underline = DashedUnderlines;
	eptr->indent_level = IndentLevel;

	if(Centered && type != E_TABLE) eptr->alignment = ALIGN_CENTER;

	switch(type)
	{
		case E_TEXT:
			/*
			 * get a unique element id
			 */
			ElementId++;
			eptr->ele_id = ElementId;

			eptr->y_offset = 0;

			eptr->edata_len = strlen(edata) + 1;
			eptr->edata = (char *)malloc(eptr->edata_len);
			if (eptr->edata == NULL)
			{
				eptr->edata_len = 0;
				fprintf(stderr, "Cannot allocate space for copy of text element data\n");
				exit(1);
			}
			strcpy(eptr->edata, edata);

			/*
			 * if this is an anchor, puts its href value into
			 * the element.
			 */
			if (AnchorText != NULL)
			{
				eptr->anchorHRef = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				eptr->anchorName = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_NAME);
				eptr->anchorSubject = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_SUBJECT);
				if (!eptr->anchorSubject) {
					eptr->anchorSubject = ParseMarkTag(AnchorText,
									   MT_ANCHOR, AT_TITLE);
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject = NULL;
			}
			break;
		case E_BULLET:
			eptr->ele_id = ElementId;

			if (BaseLine == -100)
			{
				BaseLine = baseline;
				if (LineBottom == 0)
				{
					LineBottom = LineHeight - baseline;
				}
				else
				{
					/*
					 * It is possible (with the first item
					 * in a line being a top aligned image)
					 * for LineBottom to have already been
					 * set. It now needs to be
					 * corrected as we set a real
					 * BaseLine
					 */
					if ((LineHeight - baseline) >
						(LineBottom - baseline))
					{
						LineBottom = LineHeight -
							baseline;
					}
					else
					{
						LineBottom = LineBottom -
							baseline;
					}
				}
			}
			else if (baseline < BaseLine)
			{
				eptr->y_offset = BaseLine - baseline;
			}

			/*
			 * Bullets can't be underlined!
			 */
			eptr->underline_number = 0;

			eptr->edata = NULL;
			eptr->edata_len = 0;
			eptr->anchorHRef = NULL;
			eptr->anchorName = NULL;
			eptr->anchorSubject = NULL;
			break;
		case E_HRULE:
			/*
			 * get a unique element id
			 */
			ElementId++;
			eptr->ele_id = ElementId;

			if (BaseLine == -100)
			{
				BaseLine = baseline;
				if (LineBottom == 0)
				{
					LineBottom = LineHeight - baseline;
				}
				else
				{
					/*
					 * It is possible (with the first item
					 * in a line being a top aligned image)
					 * for LineBottom to have already been
					 * set. It now needs to be
					 * corrected as we set a real
					 * BaseLine
					 */
					if ((LineHeight - baseline) >
						(LineBottom - baseline))
					{
						LineBottom = LineHeight -
							baseline;
					}
					else
					{
						LineBottom = LineBottom -
							baseline;
					}
				}
			}
			else if (baseline < BaseLine)
			{
				eptr->y_offset = BaseLine - baseline;
			}

			/*
			 * Rules can't be underlined!
			 */
			eptr->underline_number = 0;

			eptr->edata = NULL;
			eptr->edata_len = 0;
			eptr->anchorHRef = NULL;
			eptr->anchorName = NULL;
			eptr->anchorSubject = NULL;
			break;
		case E_LINEFEED:
			eptr->ele_id = ElementId;

			eptr->y_offset = 0;

			if (BaseLine == -100)
			{
				BaseLine = baseline;
				if (LineBottom == 0)
				{
					LineBottom = LineHeight - baseline;
				}
				else
				{
					/*
					 * It is possible (with the first item
					 * in a line being a top aligned image)
					 * for LineBottom to have already been
					 * set. It now needs to be
					 * corrected as we set a real
					 * BaseLine
					 */
					if ((LineHeight - baseline) >
						(LineBottom - baseline))
					{
						LineBottom = LineHeight -
							baseline;
					}
					else
					{
						LineBottom = LineBottom -
							baseline;
					}
				}
			}

			/*
			 * Linefeeds have to use the maximum line height.
			 * Deal with bad Lucidia descents.
			 */
#ifdef NO_EXTRA_FILLS
			eptr->line_height = eptr->font->ascent +
				eptr->font->descent;
#else
			eptr->line_height = LineHeight;
#endif /* NO_EXTRA_FILLS */
			if ((BaseLine + LineBottom) > eptr->line_height)
			{
				eptr->line_height = BaseLine + LineBottom;
			}

			/*
			 * Linefeeds can't be underlined!
			 */
			eptr->underline_number = 0;

			eptr->edata = NULL;
			eptr->edata_len = 0;
			/*
			 * if this linefeed is part of a broken anchor put
			 * its href value into the element so we can reconnect
			 * it when activated.
			 * If it at the beginning of an anchor, don't put
			 * the href in, and change the color back.
			 */
			if (AnchorText != NULL)
			{
				char *tptr;

				tptr = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				if ((Current != NULL)&&
				    ((Current->anchorHRef == NULL)||
				     (tptr == NULL)||
				     (strcmp(Current->anchorHRef, tptr) != 0)))
				{
                                        if (tptr)
                                          free(tptr);
					eptr->anchorHRef = NULL;
					eptr->anchorName = NULL;
					eptr->anchorSubject = NULL;
/*
 * Without motif we use our own foreground resource instead of
 * using the manager's
 */
#ifdef MOTIF
					eptr->fg = hw->manager.foreground;
#else
					eptr->fg = hw->html.foreground;
#endif /* MOTIF */
				}
				else
				{
					eptr->anchorHRef = tptr;
					eptr->anchorName =
					    ParseMarkTag(AnchorText,
						MT_ANCHOR, AT_NAME);
					eptr->anchorSubject =
					    ParseMarkTag(AnchorText,
						MT_ANCHOR, AT_SUBJECT);
					if (!eptr->anchorSubject) {
						eptr->anchorSubject =
							ParseMarkTag(AnchorText,
								     MT_ANCHOR, AT_TITLE);
					}
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject = NULL;
			}
			break;
		case E_IMAGE:
			/*
			 * get a unique element id
			 */
			ElementId++;
			eptr->ele_id = ElementId;

			/*
			 * Images can't be underlined!
			 */
			eptr->underline_number = 0;

			if (edata != NULL)
			{
				eptr->edata_len = strlen(edata) + 1;
				eptr->edata = (char *)malloc(eptr->edata_len);
				if (eptr->edata == NULL)
				{
					eptr->edata_len = 0;
					fprintf(stderr, "Cannot allocate space for copy of image element data\n");
					exit(1);
				}
				strcpy(eptr->edata, edata);
			}
			else
			{
				eptr->edata_len = 0;
				eptr->edata = NULL;
			}

			/*
			 * if this image is part of an anchor put
			 * its href and name values into the element
			 * so we can reconnect it when activated.
			 */
			if (AnchorText != NULL)
			{
				eptr->anchorHRef = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				eptr->anchorName = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_NAME);
				eptr->anchorSubject = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_SUBJECT);
				if (!eptr->anchorSubject) {
					eptr->anchorSubject =
						ParseMarkTag(AnchorText,
							     MT_ANCHOR, AT_TITLE);
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject = NULL;
			}

			/*
			 * Picture stuff
			 */
			/*
			 * if we have an image resolver, use it.
			 */
			if (hw->html.resolveImage != NULL)
			{
				int internal;

				/*
				 * See if this is a special internal image
				 */
				if ((edata != NULL)&&
					(strncmp(edata, INTERNAL_IMAGE,
					strlen(INTERNAL_IMAGE)) == 0))
				{
					internal = 1;
				}
				else
				{
					internal = 0;
				}

				/*
				 * if we delay image fetching
				 * internal images are not delayed.
				 */
				if (((hw->html.delay_images == True) &&
					(!internal)) ||
                    currently_delaying_images == 1 )
				{
				    /*
				     * see if already cached.
				     */
				    eptr->pic_data = (*(resolveImageProc)
					(hw->html.resolveImage))(hw, edata, 1, w, h);
				    if (eptr->pic_data != NULL)
				    {
					eptr->pic_data->delayed = 0;
					/*
					 * Mark images we have sucessfully
					 * loaded at least once
					 */
					if (eptr->pic_data->image_data != NULL)
					{
					    eptr->pic_data->fetched = 1;
					}
				    }
				    /*
				     * else, not cached.
				     */
				    else
				    {
				      /*
				       * just image
				       */
				      if (eptr->anchorHRef == NULL)
				      {
					eptr->pic_data = DelayedImageData(hw,
						False);
					eptr->pic_data->delayed = 1;
					eptr->anchorHRef = DelayedHRef(hw);
					eptr->fg = hw->html.anchor_fg;
				      }
				      /*
				       * else anchor and image
				       */
				      else
				      {
					eptr->pic_data = DelayedImageData(hw,
						True);
					eptr->pic_data->delayed = 1;
				      }
				    }
				}
				else
				{
				    eptr->pic_data = (*(resolveImageProc)
					(hw->html.resolveImage))(hw, edata, 0, w, h);
				    if (eptr->pic_data != NULL)
				    {
					eptr->pic_data->delayed = 0;
					/*
					 * Mark images we have sucessfully
					 * loaded at least once
					 */
					if (eptr->pic_data->image_data != NULL)
					{
					    eptr->pic_data->fetched = 1;
					}
				    }
				}
				if (eptr->pic_data != NULL)
				{
					eptr->pic_data->internal = internal;
				}
			}
			if (eptr->pic_data == NULL)
			{
				eptr->pic_data = NoImageData(hw);
				eptr->pic_data->delayed = 0;
				eptr->pic_data->internal = 0;
			}

			break;
		case E_TABLE:
			ElementId++;
			eptr->ele_id = ElementId;
			/*
			 * Table's can't be underlined
			 */
			eptr->underline_number = 0;


/*
			if (eptr->edata != NULL)
			{
				free((char *)eptr->edata);
			}
*/
			eptr->edata = NULL;
			eptr->edata_len = 0;

			/*
			 * if this table is part of an anchor put
			 * its href and name values into the element
			 * so we can reconnect it when activated.
			 */
/*
			if (eptr->anchorHRef != NULL)
			{
				free((char *)eptr->anchorHRef);
			}
			if (eptr->anchorName != NULL)
			{
				free((char *)eptr->anchorName);
			}
			if (eptr->anchorSubject != NULL)
			{
				free((char *)eptr->anchorSubject);
			}
*/
			if (AnchorText != NULL)
			{
				eptr->anchorHRef = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				eptr->anchorName = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_NAME);
				eptr->anchorSubject= ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_SUBJECT);
				if (!eptr->anchorSubject) {
					eptr->anchorSubject =
						ParseMarkTag(AnchorText,
							     MT_ANCHOR, AT_TITLE);
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject = NULL;
			}
			eptr->table_data = MakeTable (hw, edata,
				(x + IMAGE_DEFAULT_BORDER), (y + IMAGE_DEFAULT_BORDER));

			break;
		case E_WIDGET:
			/*
			 * get a unique element id
			 */
			WidgetId++;
			ElementId++;
			eptr->ele_id = ElementId;

			/*
			 * Widgets can't be underlined!
			 */
			eptr->underline_number = 0;

			eptr->edata = NULL;
			eptr->edata_len = 0;

			/*
			 * if this widget is part of an anchor put
			 * its href and name values into the element
			 * so we can reconnect it when activated.
			 */
			if (AnchorText != NULL)
			{
				eptr->anchorHRef = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				eptr->anchorName = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_NAME);
				eptr->anchorSubject = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_SUBJECT);
				if (!eptr->anchorSubject) {
					eptr->anchorSubject =
						ParseMarkTag(AnchorText,
							     MT_ANCHOR, AT_TITLE);
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject = NULL;
			}

			/*
			 * Widget stuff
			 */
			eptr->widget_data = MakeWidget(hw, edata,
				(x + IMAGE_DEFAULT_BORDER), (y + IMAGE_DEFAULT_BORDER),
				WidgetId, CurrentForm);

			/*
			 * I have no idea what to do if we can't create the
			 * widget.  It probably means we are so messed up we
			 * will soon be crashing.
			 */
			if (eptr->widget_data == NULL)
			{
			}

			break;
		default:
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr, "CreateElement:  Unknown type %d\n", type);
			}
#endif
			eptr->ele_id = ElementId;

			eptr->edata = NULL;
			eptr->edata_len = 0;
			eptr->anchorHRef = NULL;
			eptr->anchorName = NULL;
			eptr->anchorSubject = NULL;
			break;
	}
	return(eptr);
} /* CreateElement() */


/*
 * Set the formatted element into the format list.  Use a pre-allocated
 * list position if possible, otherwise allocate a new list position.
 */
void
SetElement(hw, type, fp, x, y, edata, w, h, bw)
	HTMLWidget hw;
	int type;
	XFontStruct *fp;
	int x, y;
	char *edata;
	char *w, *h;
	int bw;
{
	struct ele_rec *eptr;
	int len;
	int baseline;

	if (fp != NULL)
	{
		baseline = fp->max_bounds.ascent;
	}
	else
	{
		baseline = LineHeight;
	}

	/*
	 * There is not pre-allocated format list, or we have reached
	 * the end of the pre-allocated list.  Create a new element, and
	 * add it.
	 */
	if ((hw->html.formatted_elements == NULL)||
		((Current != NULL)&&(Current->next == NULL)))
	{
		eptr = CreateElement(hw, type, fp, x, y, edata, w, h, bw);
		Current = AddEle(&(hw->html.formatted_elements), Current, eptr);
		return;
	}

	/*
	 * If current is null, but we have a pre-allocated format list, then
	 * this is the first SetElement() call for this formated text, and
	 * we must set current to the head of the formatted list.  Otherwise
	 * we move current to the next pre-allocated list position.
	 */
	if (Current == NULL)
	{
		Current = hw->html.formatted_elements;
	}
	else
	{
		Current = Current->next;
	}

	eptr = Current;
	if (eptr == NULL)
	{
		fprintf(stderr, "SetElement: Error, setting a null element\n");
		exit(1);
	}

	eptr->type = type;
	eptr->pic_data = NULL;
	eptr->widget_data = NULL;
	eptr->table_data = NULL;
	eptr->font = fp;
	eptr->alignment = ALIGN_BOTTOM;
	eptr->selected = False;
	eptr->internal = Internal;
	eptr->strikeout = Strikeout;
	eptr->bwidth = bw;
	eptr->x = x;
	eptr->y = y;
	eptr->y_offset = 0;
	eptr->width = 0;
	eptr->line_number = LineNumber;
	eptr->line_height = LineHeight;
	eptr->fg = Fg;
	eptr->bg = Bg;
	eptr->underline_number = Underlines;
	eptr->dashed_underline = DashedUnderlines;
	eptr->indent_level = IndentLevel;

	// SAM
	if(Centered && type != E_TABLE) eptr->alignment = ALIGN_CENTER;
	// SAM

	switch(type)
	{
		case E_TEXT:
			/*
			 * get a unique element id
			 */
			ElementId++;
			eptr->ele_id = ElementId;

			eptr->y_offset = 0;

			len = strlen(edata) + 1;
			if (len > eptr->edata_len)
			{
				if (eptr->edata != NULL)
				{
					free((char *)eptr->edata);
				}
				eptr->edata = (char *)malloc(len);
				if (eptr->edata == NULL)
				{
					eptr->edata_len = 0;
					fprintf(stderr, "Cannot allocate space for copy of text element data\n");
					exit(1);
				}
			}
			eptr->edata_len = len;
			strcpy(eptr->edata, edata);

			/*
			 * if this is an anchor, puts its href and name
			 * values into the element.
			 */
			if (eptr->anchorHRef != NULL)
			{
				free((char *)eptr->anchorHRef);
			}
			if (eptr->anchorName != NULL)
			{
				free((char *)eptr->anchorName);
			}
			if (eptr->anchorSubject != NULL)
			{
				free((char *)eptr->anchorSubject);
			}
			if (AnchorText != NULL)
			{
				eptr->anchorHRef = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				eptr->anchorName = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_NAME);
				eptr->anchorSubject= ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_SUBJECT);
				if (!eptr->anchorSubject) {
					eptr->anchorSubject =
						ParseMarkTag(AnchorText,
							     MT_ANCHOR, AT_TITLE);
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject = NULL;
			}
			break;
		case E_BULLET:
			eptr->ele_id = ElementId;

			if (BaseLine == -100)
			{
				BaseLine = baseline;
				if (LineBottom == 0)
				{
					LineBottom = LineHeight - baseline;
				}
				else
				{
					/*
					 * It is possible (with the first item
					 * in a line being a top aligned image)
					 * for LineBottom to have already been
					 * set. It now needs to be
					 * corrected as we set a real
					 * BaseLine
					 */
					if ((LineHeight - baseline) >
						(LineBottom - baseline))
					{
						LineBottom = LineHeight -
							baseline;
					}
					else
					{
						LineBottom = LineBottom -
							baseline;
					}
				}
			}
			else if (baseline < BaseLine)
			{
				eptr->y_offset = BaseLine - baseline;
			}

			/*
			 * Bullets can't be underlined!
			 */
			eptr->underline_number = 0;

			if (eptr->edata != NULL)
			{
				free((char *)eptr->edata);
			}
			eptr->edata = NULL;
			eptr->edata_len = 0;
			if (eptr->anchorHRef != NULL)
			{
				free((char *)eptr->anchorHRef);
			}
			if (eptr->anchorName != NULL)
			{
				free((char *)eptr->anchorName);
			}
			if (eptr->anchorSubject != NULL)
			{
				free((char *)eptr->anchorSubject);
			}
			eptr->anchorHRef = NULL;
			eptr->anchorName = NULL;
			eptr->anchorSubject = NULL;
			break;
		case E_HRULE:
			/*
			 * get a unique element id
			 */
			ElementId++;
			eptr->ele_id = ElementId;

			if (BaseLine == -100)
			{
				BaseLine = baseline;
				if (LineBottom == 0)
				{
					LineBottom = LineHeight - baseline;
				}
				else
				{
					/*
					 * It is possible (with the first item
					 * in a line being a top aligned image)
					 * for LineBottom to have already been
					 * set. It now needs to be
					 * corrected as we set a real
					 * BaseLine
					 */
					if ((LineHeight - baseline) >
						(LineBottom - baseline))
					{
						LineBottom = LineHeight -
							baseline;
					}
					else
					{
						LineBottom = LineBottom -
							baseline;
					}
				}
			}
			else if (baseline < BaseLine)
			{
				eptr->y_offset = BaseLine - baseline;
			}

			/*
			 * Rules can't be underlined!
			 */
			eptr->underline_number = 0;

			if (eptr->edata != NULL)
			{
				free((char *)eptr->edata);
			}
			eptr->edata = NULL;
			eptr->edata_len = 0;
			if (eptr->anchorHRef != NULL)
			{
				free((char *)eptr->anchorHRef);
			}
			if (eptr->anchorName != NULL)
			{
				free((char *)eptr->anchorName);
			}
			if (eptr->anchorSubject != NULL)
			{
				free((char *)eptr->anchorSubject);
			}
			eptr->anchorHRef = NULL;
			eptr->anchorName = NULL;
			eptr->anchorSubject = NULL;
			break;
		case E_LINEFEED:
			eptr->ele_id = ElementId;

			eptr->y_offset = 0;

			if (BaseLine == -100)
			{
				BaseLine = baseline;
				if (LineBottom == 0)
				{
					LineBottom = LineHeight - baseline;
				}
				else
				{
					/*
					 * It is possible (with the first item
					 * in a line being a top aligned image)
					 * for LineBottom to have already been
					 * set. It now needs to be
					 * corrected as we set a real
					 * BaseLine
					 */
					if ((LineHeight - baseline) >
						(LineBottom - baseline))
					{
						LineBottom = LineHeight -
							baseline;
					}
					else
					{
						LineBottom = LineBottom -
							baseline;
					}
				}
			}

			/*
			 * Linefeeds have to use the maximum line height.
			 * Deal with bad Lucidia descents.
			 */
#ifdef NO_EXTRA_FILLS
			eptr->line_height = eptr->font->ascent +
				eptr->font->descent;
#else
			eptr->line_height = LineHeight;
#endif /* NO_EXTRA_FILLS */
			if ((BaseLine + LineBottom) > eptr->line_height)
			{
				eptr->line_height = (BaseLine + LineBottom);
			}

			/*
			 * Linefeeds can't be underlined!
			 */
			eptr->underline_number = 0;

			if (eptr->edata != NULL)
			{
				free((char *)eptr->edata);
			}
			eptr->edata = NULL;
			eptr->edata_len = 0;
			/*
			 * if this linefeed is part of a broken anchor put
			 * its href and name values into the element
			 * so we can reconnect it when activated.
			 * If it at the beginning of an anchor, don't put
			 * the href in and change the color back.
			 */
			if (eptr->anchorHRef != NULL)
			{
				free((char *)eptr->anchorHRef);
			}
			if (eptr->anchorName != NULL)
			{
				free((char *)eptr->anchorName);
			}
			if (eptr->anchorSubject != NULL)
			{
				free((char *)eptr->anchorSubject);
			}
			if (AnchorText != NULL)
			{
				char *tptr;

				tptr = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				if ((eptr->prev != NULL)&&
				   ((eptr->prev->anchorHRef == NULL)||
				   (tptr == NULL)||
				   (strcmp(eptr->prev->anchorHRef, tptr) != 0)))
				{
                                        if (tptr)
                                          free(tptr);
					eptr->anchorHRef = NULL;
					eptr->anchorName = NULL;
					eptr->anchorSubject = NULL;
/*
 * Without motif we use our own foreground resource instead of
 * using the manager's
 */
#ifdef MOTIF
					eptr->fg = hw->manager.foreground;
#else
					eptr->fg = hw->html.foreground;
#endif /* MOTIF */
				}
				else
				{
					eptr->anchorHRef = tptr;
					eptr->anchorName =
					    ParseMarkTag(AnchorText,
						MT_ANCHOR, AT_NAME);
					eptr->anchorSubject =
					    ParseMarkTag(AnchorText,
						MT_ANCHOR, AT_SUBJECT);
					if (!eptr->anchorSubject) {
						eptr->anchorSubject =
							ParseMarkTag(AnchorText,
								     MT_ANCHOR, AT_TITLE);
					}
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject = NULL;
			}
			break;
		case E_IMAGE:
			/*
			 * get a unique element id
			 */
			ElementId++;
			eptr->ele_id = ElementId;

			/*
			 * Images can't be underlined!
			 */
			eptr->underline_number = 0;

			if (edata != NULL)
			{
				len = strlen(edata) + 1;
				if (len > eptr->edata_len)
				{
					if (eptr->edata != NULL)
					{
						free((char *)eptr->edata);
					}
					eptr->edata = (char *)malloc(len);
					if (eptr->edata == NULL)
					{
						eptr->edata_len = 0;
						fprintf(stderr, "Cannot allocate space for copy of text element data\n");
						exit(1);
					}
				}
				eptr->edata_len = len;
				strcpy(eptr->edata, edata);
			}
			else
			{
				eptr->edata_len = 0;
				if (eptr->edata != NULL)
				{
					free((char *)eptr->edata);
				}
				eptr->edata = NULL;
			}

			/*
			 * if this image is part of an anchor put
			 * its href and name values into the element
			 * so we can reconnect it when activated.
			 */
			if (eptr->anchorHRef != NULL)
			{
				free((char *)eptr->anchorHRef);
			}
			if (eptr->anchorName != NULL)
			{
				free((char *)eptr->anchorName);
			}
			if (eptr->anchorSubject != NULL)
			{
				free((char *)eptr->anchorSubject);
			}
			if (AnchorText != NULL)
			{
				eptr->anchorHRef = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				eptr->anchorName = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_NAME);
				eptr->anchorSubject = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_SUBJECT);
				if (!eptr->anchorSubject) {
					eptr->anchorSubject =
						ParseMarkTag(AnchorText,
							     MT_ANCHOR, AT_TITLE);
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject = NULL;
			}

			/*
			 * Picture stuff
			 */
			/*
			 * if we have an image resolver, use it.
			 */
			if (hw->html.resolveImage != NULL)
			{
				int internal;

				/*
				 * See if this is a special internal image
				 */
				if ((edata != NULL)&&
					(strncmp(edata, INTERNAL_IMAGE,
					strlen(INTERNAL_IMAGE)) == 0))
				{
					internal = 1;
				}
				else
				{
					internal = 0;
				}

				/*
				 * if we delay image fetching
				 * internal images are not delayed.
				 */
				if (((hw->html.delay_images == True) &&
					(!internal)) ||
                    currently_delaying_images == 1 )
				{
				    /*
				     * see if already cached.
				     */
				    eptr->pic_data = (*(resolveImageProc)
					(hw->html.resolveImage))(hw, edata, 1, w, h);
				    if (eptr->pic_data != NULL)
				    {
					eptr->pic_data->delayed = 0;
					/*
					 * Mark images we have sucessfully
					 * loaded at least once
					 */
					if (eptr->pic_data->image_data != NULL)
					{
					    eptr->pic_data->fetched = 1;
					}
				    }
				    /*
				     * else, not cached.
				     */
				    else
				    {
				      /*
				       * just image
				       */
				      if (eptr->anchorHRef == NULL)
				      {
					eptr->pic_data = DelayedImageData(hw,
						False);
					eptr->pic_data->delayed = 1;
					eptr->anchorHRef = DelayedHRef(hw);
					eptr->fg = hw->html.anchor_fg;
				      }
				      /*
				       * else anchor and image
				       */
				      else
				      {
					eptr->pic_data = DelayedImageData(hw,
						True);
					eptr->pic_data->delayed = 1;
				      }
				    }
				}
				else
				{
				    eptr->pic_data = (*(resolveImageProc)
					(hw->html.resolveImage))(hw, edata, 0, w, h);
				    if (eptr->pic_data != NULL)
				    {
					eptr->pic_data->delayed = 0;
					/*
					 * Mark images we have sucessfully
					 * loaded at least once
					 */
					if (eptr->pic_data->image_data != NULL)
					{
					    eptr->pic_data->fetched = 1;
					}
				    }
				}
				if (eptr->pic_data != NULL)
				{
					eptr->pic_data->internal = internal;
				}
			}
			if (eptr->pic_data == NULL)
			{
				eptr->pic_data = NoImageData(hw);
				eptr->pic_data->delayed = 0;
				eptr->pic_data->internal = 0;
			}

			break;
		case E_TABLE:
			ElementId++;
			eptr->ele_id = ElementId;
			/*
			 * Table's can't be underlined
			 */
			eptr->underline_number = 0;

			if (eptr->edata != NULL)
			{
				free((char *)eptr->edata);
			}
			eptr->edata = NULL;
			eptr->edata_len = 0;

			/*
			 * if this table is part of an anchor put
			 * its href and name values into the element
			 * so we can reconnect it when activated.
			 */
			if (eptr->anchorHRef != NULL)
			{
				free((char *)eptr->anchorHRef);
			}
			if (eptr->anchorName != NULL)
			{
				free((char *)eptr->anchorName);
			}
			if (eptr->anchorSubject != NULL)
			{
				free((char *)eptr->anchorSubject);
			}
			if (AnchorText != NULL)
			{
				eptr->anchorHRef = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				eptr->anchorName = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_NAME);
				eptr->anchorSubject= ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_SUBJECT);
				if (!eptr->anchorSubject) {
					eptr->anchorSubject =
						ParseMarkTag(AnchorText,
							     MT_ANCHOR, AT_TITLE);
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject = NULL;
			}
			eptr->table_data = MakeTable (hw, edata,
				(x + IMAGE_DEFAULT_BORDER), (y + IMAGE_DEFAULT_BORDER));

			break;
		case E_WIDGET:
			/*
			 * get a unique element id
			 */
			WidgetId++;
			ElementId++;
			eptr->ele_id = ElementId;

			/*
			 * Widgets can't be underlined!
			 */
			eptr->underline_number = 0;

			if (eptr->edata != NULL)
			{
				free((char *)eptr->edata);
			}
			eptr->edata = NULL;
			eptr->edata_len = 0;

			/*
			 * if this widget is part of an anchor put
			 * its href and name values into the element
			 * so we can reconnect it when activated.
			 */
			if (eptr->anchorHRef != NULL)
			{
				free((char *)eptr->anchorHRef);
			}
			if (eptr->anchorName != NULL)
			{
				free((char *)eptr->anchorName);
			}
			if (eptr->anchorSubject != NULL)
			{
				free((char *)eptr->anchorSubject);
			}
			if (AnchorText != NULL)
			{
				eptr->anchorHRef = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_HREF);
				eptr->anchorName = ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_NAME);
				eptr->anchorSubject= ParseMarkTag(AnchorText,
					MT_ANCHOR, AT_SUBJECT);
				if (!eptr->anchorSubject) {
					eptr->anchorSubject =
						ParseMarkTag(AnchorText,
							     MT_ANCHOR, AT_TITLE);
				}
			}
			else
			{
				eptr->anchorHRef = NULL;
				eptr->anchorName = NULL;
				eptr->anchorSubject= NULL;
			}

			/*
			 * Widget stuff
			 */
			eptr->widget_data = MakeWidget(hw, edata,
				(x + IMAGE_DEFAULT_BORDER), (y + IMAGE_DEFAULT_BORDER),
				WidgetId, CurrentForm);

			/*
			 * I have no idea what to do if we can't create the
			 * widget.  It probably means we are so messed up we
			 * will soon be crashing.
			 */
			if (eptr->widget_data == NULL)
			{
			}

			break;
		default:
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr, "SetElement:  Unknown type %d\n", type);
			}
#endif
			eptr->ele_id = ElementId;

			if (eptr->edata != NULL)
			{
				free((char *)eptr->edata);
			}
			eptr->edata = NULL;
			eptr->edata_len = 0;
			if (eptr->anchorHRef != NULL)
			{
				free((char *)eptr->anchorHRef);
			}
			if (eptr->anchorName != NULL)
			{
				free((char *)eptr->anchorName);
			}
			if (eptr->anchorSubject != NULL)
			{
				free((char *)eptr->anchorSubject);
			}
			eptr->anchorHRef = NULL;
			eptr->anchorName = NULL;
			eptr->anchorSubject = NULL;
			break;
	}
} /* SetElement() */


/*
 * Change our drawing font
 */
void
NewFont(fp)
	XFontStruct *fp;
{
	/*
	 * Deal with bad Lucidia descents.
	 */
	if (fp->descent > fp->max_bounds.descent)
	{
		LineHeight = fp->max_bounds.ascent + fp->descent;
	}
	else
	{
		LineHeight = fp->max_bounds.ascent + fp->max_bounds.descent;
	}
}


/*
 * Place a linefeed at the end of a line.
 * Create and add the element record for it.
 */
void
LinefeedPlace(hw, x, y)
	HTMLWidget hw;
	int *x, *y;
{
	// SAM
#if 0
	if(Centered) {
		struct ele_rec *eptr;
		int width;

		printf("LineFeedPlace\n");
		for(eptr = Current; eptr; eptr = eptr->next) {
			printf("type %d x %d y %d width %d bwidth %d alignment %d start %d end %d\n",
				   eptr->type, eptr->x, eptr->y, eptr->width, eptr->bwidth,
				   eptr->alignment,
				   eptr->start_pos, eptr->end_pos);
			printf("\t'%.40s'\n", eptr->edata);

			// SAM Simple and wrong
			if(eptr->width) {
				// SAM How to get canvas width?
				if(eptr->width >= 640)
					printf("TOO WIDE\n");
				else {
					int offset = (640 - eptr->width) / 2;
					eptr->x += offset;
				}
			}
		}
	}
#endif
	// SAM

	/*
	 * At the end of every line check if we have a new MaxWidth
	 */
	if ((*x + hw->html.margin_width) > MaxWidth)
	{
		MaxWidth = *x + hw->html.margin_width;
	}

	SetElement(hw, E_LINEFEED, currentFont, *x, *y, (char *)NULL,NULL,NULL,IMAGE_DEFAULT_BORDER);
}


/*
 * We have encountered a line break.  Incrment the line counter,
 * and move down some space.
 */
void
LineFeed(hw, x, y)
	HTMLWidget hw;
	int *x, *y;
{
	/*
	 * Manipulate linefeed state for special pre-formatted linefeed
	 * hack for broken HTMLs
	 */
	if (Preformat)
	{
		switch(PF_LF_State)
		{
			/*
			 * First soft linefeed
			 */
			case 0:
				PF_LF_State = 1;
				break;
			/*
			 * Collapse multiple soft linefeeds within a pre
			 */
			case 1:
				return;
				break;
			/*
			 * Ignore soft linefeeds after hard linefeeds
			 * within a pre
			 */
			case 2:
				return;
				break;
			default:
				PF_LF_State = 1;
				break;
		}
	}
	/*
	 * No blank lines allowed at the start of a document.
	 */
	else if (ElementId == 0)
	{
		return;
	}
	/*
	 * For formatted documents there are 3 linefeed states.
	 * 0 = in the middle of a line.
	 * 1 = at left margin
	 * 2 = at left margin with blank line above
	 */
	else
	{
		PF_LF_State++;
		if (PF_LF_State > 2)
		{
			PF_LF_State = 2;
		}
	}

	/*
	 * sanity check to set some line height if none was specified.
	 */
	if (BaseLine <= 0)
	{
		BaseLine = LineHeight;
	}

	LinefeedPlace(hw, x, y);

	CharsInLine = 0;
	*x = TextIndent;
	*y = *y + BaseLine + LineBottom;

	LineBottom = 0;
	BaseLine = -100;

	NeedSpace = 0;
	LineNumber++;
}


/*
 * We want to make sure that future text starts at the left margin.
 * But if we are already there, don't put in a new line.
 */
void
ConditionalLineFeed(hw, x, y, state)
	HTMLWidget hw;
	int *x, *y;
	int state;
{
	if (PF_LF_State < state)
	{
		/*
		 * If this funtion is being used to insert a blank line,
		 * we need to look at the percentVerticalSpace resource
		 * to see how high to make the line.
		 */
		if ((state == 2)&&(hw->html.percent_vert_space > 0))
		{
			int l_height;

			l_height = LineHeight;
			LineHeight = LineHeight *
				hw->html.percent_vert_space / 100;
			LineFeed(hw, x, y);
			LineHeight = l_height;
		}
		else
		{
			LineFeed(hw, x, y);
		}
	}
}


/*
 * hack to make broken HTMLs within pre-formatted text have nice
 * looking linefeeds.
 */
void
HardLineFeed(hw, x, y)
	HTMLWidget hw;
	int *x, *y;
{
	/*
	 * Manipulate linefeed state for special pre-formatted linefeed
	 * hack for broken HTMLs
	 */
	if (Preformat)
	{
		switch(PF_LF_State)
		{
			/*
			 * First hard linefeed
			 */
			case 0:
				PF_LF_State = 2;
				break;
			/*
			 * Previous soft linefeed should have been ignored, so
			 * ignore this hard linefeed, but set state like it
			 * was not ignored.
			 */
			case 1:
				PF_LF_State = 2;
				return;
				break;
			/*
			 * Honor multiple hard linefeeds.
			 */
			case 2:
				break;
			default:
				PF_LF_State = 2;
				break;
		}
	}

	/*
	 * sanity check to set some line height if none was specified.
	 */
	if (BaseLine <= 0)
	{
		BaseLine = LineHeight;
	}

	LinefeedPlace(hw, x, y);

	CharsInLine = 0;
	*x = TextIndent;
	*y = *y + BaseLine + LineBottom;

	LineBottom = 0;
	BaseLine = -100;

	NeedSpace = 0;
	LineNumber++;
}


static void
AdjustBaseLine()
{
	int baseline;
	int supsubBaseline;

	baseline = Current->font->max_bounds.ascent;

        if ((Superscript>0) || (Subscript>0))
          {
            supsubBaseline = nonScriptFont->max_bounds.ascent;
            baseline += ((supsubBaseline * .4) * Superscript);
            baseline -= ((supsubBaseline * .4) * Subscript);
            baseline += 2;
          }

	if (BaseLine == -100)
	{
		BaseLine = baseline;
		Current->y_offset = 0;
		if (LineBottom == 0)
		{
			LineBottom = LineHeight - baseline;
		}
		else
		{
			/*
			 * It is possible (with the first item
			 * in a line being a top aligned image)
			 * for LineBottom to have already been
			 * set. It now needs to be
			 * corrected as we set a real
			 * BaseLine
			 */
			if ((LineHeight - baseline) >
				(LineBottom - baseline))
			{
				LineBottom = LineHeight -
					baseline;
			}
			else
			{
				LineBottom = LineBottom -
					baseline;
			}
		}
	}
	else if (baseline <= BaseLine)
	{
		if (baseline < BaseLine)
		{
			Current->y_offset = BaseLine - baseline;
		}
		else
		{
			Current->y_offset = 0;
		}

		if ((LineHeight - baseline) > LineBottom)
		{
			LineBottom = LineHeight - baseline;
		}
	}
	else
	{
		struct ele_rec *eptr;
		int line, incy;

		incy = baseline - BaseLine;
		BaseLine = baseline;

		/*
		 * Go back over this line
		 * and move everything down
		 * a little.
		 */
		eptr = Current;
		line = eptr->line_number;
		while ((eptr->prev != NULL)&&
			(eptr->prev->line_number == line))
		{
			eptr = eptr->prev;
			eptr->y_offset = eptr->y_offset + incy;
		}

		if ((LineHeight - baseline) > LineBottom)
		{
			LineBottom = LineHeight - baseline;
		}
	}
}


/*
 * Place the bullet at the beginning of an unnumbered
 * list item. Create and add the element record for it.
 */
void
BulletPlace(hw, x, y)
	HTMLWidget hw;
	int *x, *y;
{
	int width, l_height;

	/*
	 * Save the font's line height, and set your own for this
	 * element.  Restore the fonts height when done.
	 * Deal with bad Lucidia descents.
	 */
	l_height = LineHeight;
	if (hw->html.font->descent > hw->html.font->max_bounds.descent)
	{
		LineHeight = hw->html.font->max_bounds.ascent +
			hw->html.font->descent;
	}
	else
	{
		LineHeight = hw->html.font->max_bounds.ascent +
			hw->html.font->max_bounds.descent;
	}

	NeedSpace = 0;
	width = hw->html.font->max_bounds.width;
	SetElement(hw, E_BULLET, hw->html.font, *x, *y, (char *)NULL, NULL, NULL,IMAGE_DEFAULT_BORDER);
	LineHeight = l_height;
/*
 * This should reall be here, but it is a hack for headers on list
 * elements to work if we leave it out
	PF_LF_State = 0;
 */
}


/*
 * Place a horizontal rule across the page.
 * Create and add the element record for it.
 */
void
HRulePlace(hw, x, y, width)
	HTMLWidget hw;
	int *x, *y;
	unsigned int width;
{
	NeedSpace = 0;
	*x = hw->html.margin_width;
	SetElement(hw, E_HRULE, currentFont, *x, *y, (char *)NULL, NULL, NULL, IMAGE_DEFAULT_BORDER);
	*x = *x + width - (2 * hw->html.margin_width);
	NeedSpace = 1;
	PF_LF_State = 0;
}


/*
 * Place the number at the beginning of an numbered
 * list item. Create and add the element record for it.
 */
void
ListNumberPlace(hw, x, y, val)
	HTMLWidget hw;
	int *x, *y;
	int val;
{
	int width, my_x;
	int dir, ascent, descent;
	XCharStruct all;
	char buf[20];

	sprintf(buf, "%d.", val);

	width = hw->html.font->max_bounds.lbearing +
		hw->html.font->max_bounds.rbearing;
	XTextExtents(currentFont, buf, strlen(buf), &dir,
		&ascent, &descent, &all);
	my_x = *x - (width / 2) - all.width;
	/*
	 * Add a space after thenumber here so it will look right when
	 * cut and pasted from a selection.
	 */
	width = strlen(buf);
	buf[width] = ' ';
	buf[width + 1] = '\0';

	SetElement(hw, E_TEXT, currentFont, my_x, *y, buf, NULL, NULL, IMAGE_DEFAULT_BORDER);
	AdjustBaseLine();
	CharsInLine = CharsInLine + strlen(buf);

	NeedSpace = 0;
/*
 * This should reall be here, but it is a hack for headers on list
 * elements to work if we leave it out
	PF_LF_State = 0;
 */
}


/*
 * Place a piece of pre-formatted text. Add an element record for it.
 */
void
PreformatPlace(hw, mptr, x, y, width)
	HTMLWidget hw;
	struct mark_up *mptr;
	int *x, *y;
	unsigned int width;
{
	char *text;
	char *start;
	char *end;
	char *ptr;
	char tchar;
	int tab_count, char_cnt;
	int dir, ascent, descent;
        XCharStruct all;
	char *line;
	int line_x;

	text = mptr->text;

	line_x = *x;
	line = CompLine;
	if (line != NULL)
	{
		line[0] = '\0';
	}
	end = text;
	while (*end != '\0')
	{
		tab_count = 0;
		char_cnt = CharsInLine;
		/*
		 * make start and end point to one word.  A word is either
		 * a lone linefeed, or all whitespace before a word, plus
		 * the text of the word itself.
		 */
		start = end;
		/*
		 * Throw out carriage returns and form-feeds
		 */
		if ((*end == '\r')||(*end == '\f'))
		{
			start++;
			end++;
		}
		else if (*end == '\n')
		{
			end++;
			char_cnt++;
		}
		else
		{
			/*
			 * Should be only spaces and tabs here, so if it
			 * is not a tab, make it a space.
			 * Break on linefeeds, they must be done separately
			 */
			while (((int)((unsigned char)*end) < 128)&&
				(isspace(*end)))
			{
				if (*end == '\n')
				{
					break;
				}
				else if (*end == '\t')
				{
					tab_count++;
					char_cnt = ((char_cnt / 8) + 1) * 8;
				}
				else
				{
					*end = ' ';
					char_cnt++;
				}
				end++;
			}
			while (((int)((unsigned char)*end) > 127)||
				((!isspace(*end))&&(*end != '\0')))
			{
				end++;
				char_cnt++;
			}
		}

		/*
		 * Add the word to the end of this line, or insert
		 * a linefeed if the word is a lone linefeed.
		 * tabs expand to 8 spaces.
		 */
		if (start != end)
		{
			int tlen;

			tchar = *end;
			*end = '\0';

			tlen = char_cnt + 1;
			if (tlen > CompWordLen)
			{
				CompWordLen += COMP_LINE_BUF_LEN;
				if (tlen > CompWordLen)
				{
					CompWordLen = tlen;
				}
				if (CompWord != NULL)
				{
					free(CompWord);
				}
				CompWord = (char *)malloc(CompWordLen);
			}
			ptr = CompWord;

			/*
			 * If we have any tabs, expand them into spaces.
			 */
			if (tab_count)
			{
				char *p1, *p2;
				int i, new;

				char_cnt = CharsInLine;
				p1 = ptr;
				p2 = start;
				while (*p2 != '\0')
				{
					if (*p2 == '\t')
					{
						new = ((char_cnt / 8) + 1) * 8;
						for (i=0; i<(new-char_cnt); i++)
						{
							*p1++ = ' ';
						}
						p2++;
						char_cnt = new;
					}
					else
					{
						*p1++ = *p2++;
						char_cnt++;
					}
				}
				*p1 = '\0';
			}
			else
			{
				strcpy(ptr, start);
			}

#ifdef ASSUME_FIXED_WIDTH_PRE
			all.width = currentFont->max_bounds.width * strlen(ptr);
#else
			XTextExtents(currentFont, ptr, strlen(ptr), &dir,
				&ascent, &descent, &all);
#endif /* ASSUME_FIXED_WIDTH_PRE */

			if (*start == '\n')
			{
				if ((line != NULL)&&(line[0] != '\0'))
				{
					SetElement(hw, E_TEXT, currentFont,
							line_x, *y, line, NULL, NULL, IMAGE_DEFAULT_BORDER);
					/*
					 * Save width here to avoid an
					 * XTextExtents call later.
					 */
					Current->width = *x - line_x + 1;

					AdjustBaseLine();
					PF_LF_State = 0;

					line[0] = '\0';
				}

				HardLineFeed(hw, x, y);
				line_x = *x;
				NeedSpace = 0;
			}
			else
			{
				char *tptr;
				int tlen;

				if (line == NULL)
				{
					tlen = strlen(ptr) + 1;
				}
				else
				{
					tlen = strlen(line) +
						strlen(ptr) + 1;
				}
				if (tlen > CompLineLen)
				{
					CompLineLen += COMP_LINE_BUF_LEN;
					if (tlen > CompLineLen)
					{
						CompLineLen = tlen;
					}
					tptr = (char *)malloc(CompLineLen);
					if (CompLine != NULL)
					{
						strcpy(tptr, CompLine);
						free(CompLine);
					}
					else
					{
						tptr[0] = '\0';
					}
					CompLine = tptr;
				}
				line = CompLine;

				strcat(line, ptr);

				*x = *x + all.width;
				CharsInLine = CharsInLine + strlen(ptr);
				NeedSpace = 1;
			}
			*end = tchar;
		}
	}
	if ((line != NULL)&&(line[0] != '\0'))
	{
		SetElement(hw, E_TEXT, currentFont,
				line_x, *y, line, NULL, NULL, IMAGE_DEFAULT_BORDER);
		/*
		 * Save width here to avoid an
		 * XTextExtents call later.
		 */
		Current->width = *x - line_x + 1;

		AdjustBaseLine();
		PF_LF_State = 0;
		line[0] = '\0';
	}
}


/*
 * Format and place a piece of text. Add an element record for it.
 */
void
FormatPlace(hw, mptr, x, y, width)
	HTMLWidget hw;
	struct mark_up *mptr;
	int *x, *y;
	unsigned int width;
{
	char *text;
	char *start;
	char *end;
	char *ptr;
	char tchar;
#ifdef DOUBLE_SPACE_AFTER_PUNCT
	char tchar2;
#endif /* DOUBLE_SPACE_AFTER_PUNCT */
	int stripped_space;
	int added_space;
	int double_space;
	int dir, ascent, descent;
        XCharStruct all;
	char *line;
	int line_x;

	text = mptr->text;

	line_x = *x;
	line = CompLine;
	if (line != NULL)
	{
		line[0] = '\0';
	}
	end = text;
	while (*end != '\0')
	{
		/*
		 * make start and end point to one word.
		 * set flag if we removed any leading white space.
		 * set flag if we add any leading white space.
		 */
		stripped_space = 0;
		added_space = 0;
		start = end;
		while (((int)((unsigned char)*start) < 128)&&(isspace(*start)))
		{
			stripped_space = 1;
			start++;
		}

		end = start;
		while (((int)((unsigned char)*end) > 127)||
			((!isspace(*end))&&(*end != '\0')))
		{
			end++;
		}

		/*
		 * Add the word to the end of this line, or insert
		 * a linefeed an put the word at the start of the next line.
		 */
		if (start != end)
		{
			int nobreak;
			int tlen;

			/*
			 * nobreak is a horrible hack that specifies special
			 * conditions where line breaks are just not allowed
			 */
			nobreak = 0;

			tchar = *end;
			*end = '\0';

			/*
			 * Malloc temp space if needed, leave room for
			 * 2 spaces and a end of string char
			 */
			tlen = strlen(start) + 3;
			if (tlen > CompWordLen)
			{
				CompWordLen += COMP_LINE_BUF_LEN;
				if (tlen > CompWordLen)
				{
					CompWordLen = tlen;
				}
				if (CompWord != NULL)
				{
					free(CompWord);
				}
				CompWord = (char *)malloc(CompWordLen);
			}
			ptr = CompWord;

			if ((NeedSpace > 0)&&(stripped_space))
			{
				if (NeedSpace == 2)
				{
					strcpy(ptr, "  ");
					added_space = 2;
				}
				else
				{
					strcpy(ptr, " ");
					added_space = 1;
				}
			}
			else
			{
				strcpy(ptr, "");
			}
			strcat(ptr, start);

#ifdef DOUBLE_SPACE_AFTER_PUNCT
			/*
			 * If this text ends in '.', '!', or '?' we need
			 * to set up the addition of two spaces after it.
			 */
			tchar2 = ptr[strlen(ptr) - 1];
			if ((tchar2 == '.')||(tchar2 == '!')||(tchar2 == '?'))
			{
				double_space = 1;
			}
			else
			{
				double_space = 0;
			}
#else
			double_space = 0;
#endif /* DOUBLE_SPACE_AFTER_PUNCT */

			XTextExtents(currentFont, ptr, strlen(ptr), &dir,
				&ascent, &descent, &all);

			/*
			 * Horrible hack for punctuation following
			 * font changes to not go on the next line.
			 */
			if ((MY_ISPUNCT(*ptr))&&(added_space == 0))
			{
				char *tptr;

				/*
				 * Take into account whole streams of
				 * punctuation.
				 */
				nobreak = 1;
				tptr = ptr;
				while ((*tptr != '\0')&&(MY_ISPUNCT(*tptr)))
				{
					tptr++;
				}
				if (*tptr != '\0')
				{
					nobreak = 0;
				}
			}

			/*
			 * No linebreaks if this whole line is just too
			 * long.
			 */
			if (*x == TextIndent)
			{
				nobreak = 1;
			}

			if (((*x + all.width + MarginW) <= width)||(nobreak))
			{
				char *tptr;
				int tlen;

				if (line == NULL)
				{
					tlen = strlen(ptr) + 1;
				}
				else
				{
					tlen = strlen(line) +
						strlen(ptr) + 1;
				}
				if (tlen > CompLineLen)
				{
					CompLineLen += COMP_LINE_BUF_LEN;
					if (tlen > CompLineLen)
					{
						CompLineLen = tlen;
					}
					tptr = (char *)malloc(CompLineLen);
					if (CompLine != NULL)
					{
						strcpy(tptr, CompLine);
						free(CompLine);
					}
					else
					{
						tptr[0] = '\0';
					}
					CompLine = tptr;
				}
				line = CompLine;

				strcat(line, ptr);
			}
			else
			{
				char *tptr, *tptr2;
				int tlen;

				if ((line != NULL)&&(line[0] != '\0'))
				{
					SetElement(hw, E_TEXT, currentFont,
							line_x, *y, line, NULL, NULL, IMAGE_DEFAULT_BORDER);
					/*
					 * Save width here to avoid an
					 * XTextExtents call later.
					 */
					Current->width = *x - line_x + 1;

					AdjustBaseLine();
					PF_LF_State = 0;

					line[0] = '\0';
				}

				LineFeed(hw, x, y);
				line_x = *x;

				/*
				 * If we added a space before, remove it now
				 * since we are at the beginning of a new line
				 */
				if (added_space)
				{
					tptr2 = (char *)(ptr + added_space);
				}
				else
				{
					tptr2 = ptr;
				}
				XTextExtents(currentFont, tptr2,
					strlen(tptr2), &dir,
					&ascent, &descent, &all);

				if (line == NULL)
				{
					tlen = strlen(tptr2) + 1;
				}
				else
				{
					tlen = strlen(line) +
						strlen(tptr2) + 1;
				}
				if (tlen > CompLineLen)
				{
					CompLineLen += COMP_LINE_BUF_LEN;
					if (tlen > CompLineLen)
					{
						CompLineLen = tlen;
					}
					tptr = (char *)malloc(CompLineLen);
					if (CompLine != NULL)
					{
						strcpy(tptr, CompLine);
						free(CompLine);
					}
					else
					{
						tptr[0] = '\0';
					}
					CompLine = tptr;
				}
				line = CompLine;

				strcat(line, tptr2);
			}

			/*
			 * Set NeedSpace for one or 2 spaces based on
			 * whether we are after a '.', '!', or '?'
			 * or not.
			 */
			if (double_space)
			{
				NeedSpace = 2;
			}
			else
			{
				NeedSpace = 1;
			}

			*x = *x + all.width;
			*end = tchar;
		}
		/*
		 * Else if there is trailing whitespace, add it now
		 */
		else if ((NeedSpace > 0)&&(stripped_space))
		{
			char *tptr;
			char *spc;
			int tlen;

			if (NeedSpace == 2)
			{
				spc = (char *)malloc(strlen("  ") + 1);
				strcpy(spc, "  ");
			}
			else
			{
				spc = (char *)malloc(strlen(" ") + 1);
				strcpy(spc, " ");
			}

			XTextExtents(currentFont, spc, strlen(spc), &dir,
				&ascent, &descent, &all);

			/*
			 * Sigh, adding this one little space might force a
			 * line break.
			 */
			if ((*x + all.width + MarginW) <= width)
			{
				if (line == NULL)
				{
					tlen = strlen(spc) + 1;
				}
				else
				{
					tlen = strlen(line) +
						strlen(spc) + 1;
				}
				if (tlen > CompLineLen)
				{
					CompLineLen += COMP_LINE_BUF_LEN;
					if (tlen > CompLineLen)
					{
						CompLineLen = tlen;
					}
					tptr = (char *)malloc(CompLineLen);
					if (CompLine != NULL)
					{
						strcpy(tptr, CompLine);
						free(CompLine);
					}
					else
					{
						tptr[0] = '\0';
					}
					CompLine = tptr;
				}
				line = CompLine;

				strcat(line, spc);
			}
			/*
			 * Ok, the space forced a linefeed, but now we must
			 * also drop the space since we don't want it if we
			 * have a linefeed here.
			 */
			else
			{
				if ((line != NULL)&&(line[0] != '\0'))
				{
					SetElement(hw, E_TEXT, currentFont,
							line_x, *y, line, NULL, NULL, IMAGE_DEFAULT_BORDER);
					/*
					 * Save width here to avoid an
					 * XTextExtents call later.
					 */
					Current->width = *x - line_x + 1;

					AdjustBaseLine();
					PF_LF_State = 0;

					line[0] = '\0';
				}

				LineFeed(hw, x, y);
				line_x = *x;

				all.width = 0;
			}

			*x = *x + all.width;
			if (spc)
				free(spc);
			NeedSpace = 0;
		}
	}
	if ((line != NULL)&&(line[0] != '\0'))
	{
		SetElement(hw, E_TEXT, currentFont,
				line_x, *y, line, NULL, NULL, IMAGE_DEFAULT_BORDER);
		/*
		 * Save width here to avoid an
		 * XTextExtents call later.
		 */
		Current->width = *x - line_x + 1;

		AdjustBaseLine();
		PF_LF_State = 0;
		line[0] = '\0';
	}
}

TablePlace(hw,mptr,x,y,width)
HTMLWidget hw;
struct mark_up **mptr;
int *x, *y;
unsigned int width;
{
int extra;

#ifndef DISABLE_TRACE
	if (htmlwTrace) {
		fprintf(stderr,"TablePlace(hw,mptr,*x=%d,*y=%d,width = %d)\n", *x,*y,width);
	}
#endif

	if ((*mptr)->is_end) {
		/* end of table */
		return 0;
		}

	extra = 10;

	LineFeed(hw, x, y);
	SetElement(hw, E_TABLE, currentFont, *x, *y, (char *) mptr, NULL, NULL, IMAGE_DEFAULT_BORDER);
	if (!Current->table_data) {
		/* no table */
		return 0;
		}
	Current->alignment = ALIGN_MIDDLE;
	Current->width = Current->table_data->width;
	Current->y_offset = Current->table_data->height-extra;
	Current->line_height = Current->table_data->height + extra;
	BaseLine = Current->table_data->height;

	*x += Current->width + 1;
	LineFeed(hw, x, y);
}

/*
 * Place an image. Add an element record for it.
 */
void
ImagePlace(hw, mptr, x, y, width)
	HTMLWidget hw;
	struct mark_up *mptr;
	int *x, *y;
	unsigned int width;
{
	char *tptr,*tmpPtr;
	char *wTmp,*hTmp;
	int border_width;

#ifdef SPACE_HACK
	/*
	 * If we are starting an image in formatted
	 * text, and it needs a preceeding space, add
	 * that space now.
	 */
	if ((!Preformat)&&(NeedSpace > 0))
	{
		int dir, ascent, descent;
		XCharStruct all;

		if (NeedSpace == 2)
		{
			tptr = (char *)malloc(strlen("  ") + 1);
			strcpy(tptr, "  ");
		}
		else
		{
			tptr = (char *)malloc(strlen(" ") + 1);
			strcpy(tptr, " ");
		}

		XTextExtents(currentFont, tptr,
			strlen(tptr), &dir, &ascent,
			&descent, &all);
		SetElement(hw, E_TEXT, currentFont,
			*x, *y, tptr, NULL, NULL, IMAGE_DEFAULT_BORDER);
		/*
		 * Save width here to avoid an
		 * XTextExtents call later.
		 */
		Current->width = all.width;

		AdjustBaseLine();
		*x = *x + all.width;
		CharsInLine = CharsInLine + strlen(tptr);
                if (tptr)
                  free(tptr);
		PF_LF_State = 0;
		NeedSpace = 0;
	}
#endif /* SPACE_HACK */

	tptr = ParseMarkTag(mptr->start, MT_IMAGE, "SRC");
	/**temp******/
	if (!tptr) {
		tptr = ParseMarkTag(mptr->start, MT_FIGURE, "SRC");
		}
	/***********/
	tmpPtr=tptr;

	tptr = ParseMarkTag(mptr->start, MT_IMAGE, "WIDTH");
	/**temp******/
	if (!tptr) {
		tptr = ParseMarkTag(mptr->start, MT_FIGURE, "WIDTH");
		}
	/***********/
	wTmp=tptr;

	tptr = ParseMarkTag(mptr->start, MT_IMAGE, "HEIGHT");
	/**temp******/
	if (!tptr) {
		tptr = ParseMarkTag(mptr->start, MT_FIGURE, "HEIGHT");
		}
	/***********/
	hTmp=tptr;

	tptr = ParseMarkTag(mptr->start, MT_IMAGE, "BORDER");
	/**temp******/
	if (!tptr) {
		tptr = ParseMarkTag(mptr->start, MT_FIGURE, "BORDER");
		}
	/***********/
	if (!tptr || !*tptr) {
		border_width=IMAGE_DEFAULT_BORDER;
	}
	else if ((border_width=atoi(tptr))<0) {
		border_width=0;
	}
	if (tptr) {
		free(tptr);
	}

	SetElement(hw, E_IMAGE, currentFont, *x, *y, tmpPtr, wTmp, hTmp, border_width);

	/*
	 * Only after we have placed the image do we know its dimensions.
	 * So now look and see if the image is too wide, and if so go
	 * back and insert a linebreak.
	 */
	if ((Current->pic_data != NULL)&&(!Preformat))
	{
		int extra;

		if ((hw->html.border_images == True)||
			((Current->anchorHRef != NULL)&&
			(!Current->pic_data->internal)))
		{
			if (Current->pic_data->delayed) {
				extra=4;
			}
			else {
				extra = 2*border_width;
			}
		}
		else
		{
			extra = 0;
		}

		if (((*x + Current->pic_data->width + extra + MarginW) >width)&&
			(Current->prev != NULL)&&
			(Current->prev->line_number == LineNumber))
		{
			Current = Current->prev;
			LineFeed(hw, x, y);
			SetElement(hw, E_IMAGE, currentFont, *x, *y, tmpPtr, wTmp, hTmp, border_width);
		}
	}
	/*
	 * Clean up parsed SRC string
	 */
	if (tmpPtr) {
		free(tmpPtr);
	}
	if (wTmp) {
		free(wTmp);
	}
	if (hTmp) {
		free(hTmp);
	}

	/*
	 * Yank out the name field, and stick it in text.
	 * We may use this for ALT to at some later date.
	 */
	if (Current->pic_data != NULL)
	{
		tptr = ParseMarkTag(mptr->start, MT_IMAGE, "NAME");
		/*temp******/
		if (!tptr) {
			tptr = ParseMarkTag(mptr->start, MT_FIGURE, "NAME");
			}
		/*temp******/
		Current->pic_data->text = tptr;
	}

	/*
	 * Check if this image has the ISMAP attribute, so we know the
	 * x,y coordinates of the image click are important.
	 * Due to a special case (see below), this code can acutally
	 * change the size, or anchor status of the image, thus we MUST
	 * doit before we muck with the Baseline and stuff.
	 */
	if (Current->pic_data != NULL)
	{
		/*
		 * Handle the USEMAP attribute of IMG tags. This is used for
		 * client-side image map support.
		 * --SWP
		 */
		Current->pic_data->map=NULL;
		Current->pic_data->usemap=NULL;
		tptr=ParseMarkTag(mptr->start, MT_IMAGE, "USEMAP");
		if (tptr!=NULL) {
			if (*tptr) {
				Current->pic_data->usemap=tptr;
			}
			else {
				free(tptr);
			}
		}
		Current->pic_data->fptr = NULL;
		tptr = ParseMarkTag(mptr->start, MT_IMAGE, "ISMAP");
		if (tptr != NULL)
		{
			free(tptr);
			Current->pic_data->ismap = 1;
			/*
			 * SUPER SPECIAL CASE!  (Thanks Marc)
			 * If you have an ISMAP image inside a form,
			 * And that form doesn't already have an HREF
			 * by being inside an anchor,
			 * (Being a DelayedHRef is considered no href)
			 * clicking in that image will submit the form,
			 * adding the x,y coordinates of the click as part
			 * of the list of name/value pairs.
			 */
			if ((CurrentForm != NULL)&&
				((Current->anchorHRef == NULL)||
				(IsDelayedHRef(hw, Current->anchorHRef))))
			{
				Current->pic_data->fptr = CurrentForm;
				Current->anchorHRef = IsMapForm(hw);
				Current->fg = hw->html.anchor_fg;
			}
		}
		else
		{
			Current->pic_data->ismap = 0;
		}
	}

	/*
	 * Check if this image will be top aligned
	 */
	tptr = ParseMarkTag(mptr->start, MT_IMAGE, "ALIGN");
	/*temp******/
	if (!tptr)
	{
		tptr = ParseMarkTag(mptr->start, MT_FIGURE, "ALIGN");
	}
	/*temp******/
	if (caseless_equal(tptr, "TOP"))
	{
		Current->alignment = ALIGN_TOP;
	}
	else if (caseless_equal(tptr, "MIDDLE"))
	{
		Current->alignment = ALIGN_MIDDLE;
	}
	else
	{
		Current->alignment = ALIGN_BOTTOM;
	}
	/*
	 * Clean up parsed ALIGN string
	 */
	if (tptr != NULL)
	{
		free(tptr);
	}

	/*
	 * Advance x position, and check the max
	 * line height.  We need to follow this
	 * image with a space.
	 */
	if (Current->pic_data != NULL)
	{
		int extra;

                if ((hw->html.border_images == True)||
                        ((Current->anchorHRef != NULL)&&
                        (!Current->pic_data->internal)))
                {
                        if (Current->pic_data->delayed) {
                                extra=4;
			}
                        else {
                                extra = 2*border_width;
			}
		}
                else
                {
                        extra = 0;
		}

		if (BaseLine == -100)
		{
			BaseLine = 0;
		}

		*x = *x + Current->pic_data->width + extra;

		if (Current->alignment == ALIGN_TOP)
		{
			Current->y_offset = 0;

			if ((Current->pic_data->height + extra - BaseLine) >
				LineBottom)
			{
				LineBottom = Current->pic_data->height + extra -
					BaseLine;
			}
		}
		else if (Current->alignment == ALIGN_MIDDLE)
		{
			int baseline;

			baseline = (Current->pic_data->height + extra) / 2;

			if (baseline <= BaseLine)
			{
				Current->y_offset = BaseLine - baseline;
			}
			else
			{
				struct ele_rec *eptr;
				int line, incy;

				Current->y_offset = 0;

				incy = baseline - BaseLine;
				BaseLine = baseline;

				/*
				 * Go back over this line
				 * and move everything down
				 * a little.
				 */
				eptr = Current;
				line = eptr->line_number;
				while ((eptr->prev != NULL)&&
					(eptr->prev->line_number == line))
				{
					eptr = eptr->prev;
					eptr->y_offset = eptr->y_offset + incy;
				}
			}

			if ((Current->pic_data->height + extra - BaseLine) >
				LineBottom)
			{
				LineBottom = Current->pic_data->height + extra -
					BaseLine;
			}
		}
		else if ((Current->pic_data->height + extra) <= BaseLine)
		{
			Current->y_offset = BaseLine -
				(Current->pic_data->height + extra);
		}
		else if ((Current->pic_data->height + extra) > BaseLine)
		{
			struct ele_rec *eptr;
			int line, incy;

			incy = Current->pic_data->height + extra - BaseLine;
			BaseLine = Current->pic_data->height + extra;

			/*
			 * Go back over this line
			 * and move everything down
			 * a little.
			 */
			eptr = Current;
			line = eptr->line_number;
			while ((eptr->prev != NULL)&&
				(eptr->prev->line_number == line))
			{
				eptr = eptr->prev;
				eptr->y_offset = eptr->y_offset + incy;
			}
		}

		if (BaseLine == 0)
		{
			BaseLine = -100;
		}
	}
	PF_LF_State = 0;
	NeedSpace = 1;
}


/*
 * Place a Widget. Add an element record for it.
 */
void
WidgetPlace(hw, mptr, x, y, width)
	HTMLWidget hw;
	struct mark_up *mptr;
	int *x, *y;
	unsigned int width;
{
	SetElement(hw, E_WIDGET, currentFont, *x, *y, mptr->start,NULL,NULL,IMAGE_DEFAULT_BORDER);

	/*
	 * Only after we have placed the widget do we know its dimensions.
	 * So now look and see if the widget is too wide, and if so go
	 * back and insert a linebreak.
	 */
	if ((Current->widget_data != NULL)&&(!Preformat))
	{
		int extra;

		extra = 2 * IMAGE_DEFAULT_BORDER;

		if (((*x + Current->widget_data->width + extra + MarginW) >
			width)&&
			(Current->prev != NULL)&&
			(Current->prev->line_number == LineNumber))
		{
			WidgetId--;
			Current = Current->prev;
			LineFeed(hw, x, y);
			SetElement(hw, E_WIDGET, currentFont, *x, *y,
				mptr->start,NULL,NULL,IMAGE_DEFAULT_BORDER);
		}
	}

	/*
	 * Advance x position, and check BaseLine and LineBottom.
	 * We need to follow this widget with a space.
	 */
	if (Current->widget_data != NULL)
	{
		int extra;
		int baseline;
		XFontStruct *fp;

		extra = 2 * IMAGE_DEFAULT_BORDER;

		/*
		 * Find the font used in this widget.  Then find its baseline
		 */
		fp = GetWidgetFont(hw, Current->widget_data);
		if (fp == NULL)
		{
			baseline = Current->widget_data->height + extra;
		}
		/*
		 * If no font, the baseline is the bottum of the widget
		 */
		else
		{
			int border;

			border = ((Current->widget_data->height + extra) -
			    (fp->max_bounds.ascent + fp->max_bounds.descent));
			baseline = (border / 2) + fp->max_bounds.ascent;
		}

		/*
		 * Baseline == -100 is the special unset baseline value.
		 */
		if (BaseLine == -100)
		{
			BaseLine = baseline;
			Current->y_offset = 0;
			/*
			 * If linebottom isn't set, set it to
			 * whatever of the height is below the baseline.
			 */
			if (LineBottom == 0)
			{
				LineBottom = Current->widget_data->height +
					extra - baseline;
			}
			/*
			 * Else, it is possible that a linebottom has been
			 * set even when we have no baseline yet (like if
			 * the first item in the line was a top aligned image)
			 * It now needs to be corrected as we set a real
			 * BaseLine.
			 */
			else
			{
				if ((Current->widget_data->height +
					extra - baseline) >
					(LineBottom - baseline))
				{
					LineBottom =
						Current->widget_data->height +
						extra - baseline;
				}
				else
				{
					LineBottom = LineBottom - baseline;
				}
			}
		}
		/*
		 * Else we already have a baseline, and it is greater that
		 * the baseline for this widget.
		 * Set y_offset, and check linebottom.
		 */
		else if (baseline <= BaseLine)
		{
			if (baseline < BaseLine)
			{
				Current->y_offset = BaseLine - baseline;
			}
			else
			{
				Current->y_offset = 0;
			}

			/*
			 * Our line bottom may be greater than the
			 * old one.
			 */
			if ((Current->widget_data->height + extra - baseline) >
				LineBottom)
			{
				LineBottom = Current->widget_data->height +
					extra - baseline;
			}
		}
		else
		/*
		 * Else we have a new baseline greater than the old baseline.
		 */
		{
			struct ele_rec *eptr;
			int line, incy;

			/*
			 * Figure out how much to move all the old stuff
			 */
			incy = baseline - BaseLine;
			BaseLine = baseline;

			/*
			 * Go back over this line
			 * and move everything down
			 * a little.
			 */
			eptr = Current;
			line = eptr->line_number;
			while ((eptr->prev != NULL)&&
				(eptr->prev->line_number == line))
			{
				eptr = eptr->prev;
				eptr->y_offset = eptr->y_offset + incy;
			}

			/*
			 * Our line bottom may be greater than the
			 * old one.
			 */
			if ((Current->widget_data->height + extra - baseline) >
				LineBottom)
			{
				LineBottom = Current->widget_data->height +
					extra - baseline;
			}
		}

		/*
		 * Advance the X position.
		 */
		*x = *x + Current->widget_data->width + extra;
	}
	PF_LF_State = 0;
	NeedSpace = 1;
}


static void
PushFont(font)
	XFontStruct *font;
{
	FontRec *fptr;

	fptr = (FontRec *)malloc(sizeof(FontRec));
	if (fptr == NULL)
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "No memory to expand font stack!\n");
		}
#endif

		return;
	}

	fptr->font = font;
	fptr->next = FontStack;
	FontStack = fptr;
}


static XFontStruct *
PopFont()
{
	XFontStruct *font;
	FontRec *fptr;

	if (FontStack->next != NULL)
	{
		fptr = FontStack;
		FontStack = FontStack->next;
		font = fptr->font;
		free((char *)fptr);
	}
	else
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "Warning, popping empty font stack!\n");
		}
#endif

		font = FontStack->font;
	}

	return(font);
}


/*
 * We've just terminated the current OPTION.
 * Put it in the proper place in the SelectInfo structure.
 * Move option_buf into options, and maybe copy into
 * value if is_value is set.
 */
static void
ProcessOption(sptr)
	SelectInfo *sptr;
{
	int i, cnt;
	char **tarray;

	clean_white_space(sptr->option_buf);
	tarray = sptr->options;
	cnt = sptr->option_cnt + 1;
	sptr->options = (char **)malloc(sizeof(char *) * cnt);
	for (i=0; i<(cnt - 1); i++)
	{
		sptr->options[i] = tarray[i];
	}
	if (tarray != NULL)
	{
		free((char *)tarray);
	}
	sptr->options[cnt - 1] = sptr->option_buf;
	sptr->option_cnt = cnt;

	tarray = sptr->returns;
	cnt = sptr->option_cnt;
	sptr->returns = (char **)malloc(sizeof(char *) * cnt);
	for (i=0; i<(cnt - 1); i++)
	{
		sptr->returns[i] = tarray[i];
	}
	if (tarray != NULL)
	{
		free((char *)tarray);
	}
	sptr->returns[cnt - 1] = sptr->retval_buf;

	if (sptr->is_value)
	{
		tarray = sptr->value;
		cnt = sptr->value_cnt + 1;
		sptr->value = (char **)malloc(sizeof(char *) * cnt);
		for (i=0; i<(cnt - 1); i++)
		{
			sptr->value[i] = tarray[i];
		}
		if (tarray != NULL)
		{
			free((char *)tarray);
		}
		sptr->value[cnt - 1] = (char *)malloc(
			strlen(sptr->option_buf) + 1);
		strcpy(sptr->value[cnt - 1], sptr->option_buf);
		sptr->value_cnt = cnt;
	}
}


/*
 * Horrible code for the TEXTAREA element.  Escape '\' and ''' by
 * putting a '\' in front of them, then replace all '"' with '''.
 * This lets us safely put the resultant value between double quotes.
 */
char *
TextAreaAddValue(value, text)
	char *value;
	char *text;
{
	int extra;
	char *buf;
	char *bptr;
	char *tptr;

	if ((text == NULL)||(text[0] == '\0'))
	{
		return(value);
	}

	extra = 0;
	tptr = text;
	while (*tptr != '\0')
	{
		if (*tptr == '\\')
		{
			extra++;
		}
		else if (*tptr == '\'')
		{
			extra++;
		}
		tptr++;
	}

	buf = (char *)malloc(strlen(value) + strlen(text) + extra + 1);
	if (buf == NULL)
	{
		return(value);
	}
	strcpy(buf, value);

	tptr = text;
	bptr = (char *)(buf + strlen(value));
	while (*tptr != '\0')
	{
		if ((*tptr == '\\')||(*tptr == '\''))
		{
			*bptr++ = '\\';
			*bptr++ = *tptr++;
		}
		else if (*tptr == '\"')
		{
			*bptr++ = '\'';
			tptr++;
		}
		else
		{
			*bptr++ = *tptr++;
		}
	}
	*bptr = '\0';

	free(value);
	return(buf);
}


/*
 * Make necessary changes to formatting, based on the type of the
 * parsed HTML text we are formatting.
 * Some calls create elements that are added to the formatted element list.
 */
static
void
TriggerMarkChanges(hw, mptr, x, y)
	HTMLWidget hw;
struct mark_up **mptr;
int *x, *y;
{
	struct mark_up *mark;
	XFontStruct *font;
	int type, width, curCoord;
	char *tptr,*cptr,*endptr;

	mark = *mptr;
	type = mark->type;
	font = NULL;

	/* If we are not in a tag that belongs in the HEAD, end the HEAD
	   section  - amb */
	if (InDocHead)
		if ((type != M_TITLE)&&(type != M_NONE)&&(type != M_BASE)&&
			(type != M_INDEX)&&(type != M_COMMENT))
		{
			Ignore = 0;
			InDocHead = 0;
		}

	/*
	 * If Ignore is set, we ignore all further elements until we get to the
	 * end of the Ignore
	 * Let text through so we can grab the title text.
	 * Let title through so we can hit the end title.
	 * Now also used for SELECT parseing
	 * Let SELECT through so we can hit the end SELECT.
	 * Let OPTION through so we can hit the OPTIONs.
	 * Let TEXTAREA through so we can hit the TEXTAREAs.
	 */
	if ((Ignore)&&(!InDocHead)&&(type != M_TITLE)&&(type != M_NONE)&&
		(type != M_SELECT)&&(type != M_OPTION)&&
		(type != M_TEXTAREA)&&(type != M_DOC_HEAD))
	{
		return;
	}

	switch(type)
	{
		/*
		 * Place the text.  Different functions based on whether it
		 * is pre-formatted or not.
		 */
	case M_NONE:
		if ((Ignore)&&(CurrentSelect == NULL)&&
			(TextAreaBuf == NULL))
		{
			if (TitleText == NULL)
			{
				TitleText = (char *)
					malloc(strlen((*mptr)->text) + 1);
				strcpy(TitleText, (*mptr)->text);
			}
			else
			{
				char *tptr;

				tptr = (char *)
					malloc(strlen(TitleText) +
						   strlen((*mptr)->text) + 1);
				strcpy(tptr, TitleText);
				strcat(tptr, (*mptr)->text);
				free(TitleText);
				TitleText = tptr;
			}
		}
		else if ((Ignore)&&(CurrentSelect != NULL))
		{
			if (CurrentSelect->option_buf != NULL)
			{
				char *tptr;

				tptr = (char *)malloc(strlen(
										  CurrentSelect->option_buf) +
									  strlen((*mptr)->text) + 1);
				strcpy(tptr, CurrentSelect->option_buf);
				strcat(tptr, (*mptr)->text);
				free(CurrentSelect->option_buf);
				CurrentSelect->option_buf = tptr;
			}
		}
		else if ((Ignore)&&(TextAreaBuf != NULL))
		{
			TextAreaBuf = TextAreaAddValue(TextAreaBuf,
										   (*mptr)->text);
		}
		else if (Preformat)
		{
			PreformatPlace(hw, *mptr, x, y, Width);
		}
		else
		{
			FormatPlace(hw, *mptr, x, y, Width);
		}
		break;
		/*
		 * Titles are just set into the widget for retrieval by
		 * XtGetValues().
		 */
	case M_TITLE:
		if (mark->is_end)
		{
			if (!InDocHead)
				Ignore = 0;
			hw->html.title = TitleText;
			TitleText = NULL;
		}
		else
		{
			Ignore = 1;
			TitleText = NULL;
		}
		break;
		/*
		 * Formatting commands just change the current font.
		 */
	case M_CODE:
	case M_SAMPLE:
	case M_KEYBOARD:
	case M_FIXED:
		if (mark->is_end)
		{
			font = PopFont();
		}
		else
		{
			PushFont(currentFont);
			font = hw->html.fixed_font;
		}
		break;
	case M_STRONG:
	case M_BOLD:
		if (mark->is_end)
		{
			font = PopFont();
		}
		else
		{
			PushFont(currentFont);
			if (currentFont == hw->html.fixed_font ||
				currentFont == hw->html.fixeditalic_font)
				font = hw->html.fixedbold_font;
			else if (currentFont == hw->html.plain_font ||
					 currentFont == hw->html.plainitalic_font)
				font = hw->html.plainbold_font;
			else
				font = hw->html.bold_font;
		}
		break;
	case M_EMPHASIZED:
	case M_VARIABLE:
	case M_CITATION:
	case M_ITALIC:
		if (mark->is_end)
		{
			font = PopFont();
		}
		else
		{
			PushFont(currentFont);
			if (currentFont == hw->html.fixed_font ||
				currentFont == hw->html.fixedbold_font)
				font = hw->html.fixeditalic_font;
			else if (currentFont == hw->html.plain_font ||
					 currentFont == hw->html.plainbold_font)
				font = hw->html.plainitalic_font;
			else
				font = hw->html.italic_font;
		}
		break;
		/*
		 * Strikeout means draw a line through the text.
		 * Right now we just set a boolean flag which gets shoved
		 * in the element record for all elements in the
		 * strikeout zone.
		 */
	case M_STRIKEOUT:
		if (mark->is_end)
		{
			Strikeout = False;
		}
		else
		{
			Strikeout = True;
		}
		break;
	case M_SUP:
		if (mark->is_end)
		{
			Superscript--;
			if ((Superscript==0) && (Subscript==0))
				font = PopFont();
		}
		else
		{
			Superscript++;
			if ((Superscript==1) && (Subscript==0))
			{
				nonScriptFont=currentFont;
				PushFont(currentFont);
				font = hw->html.supsub_font;
			}
		}
		break;
	case M_SUB:
		if (mark->is_end)
		{
			Subscript--;
			if ((Subscript==0) && (Superscript==0))
				font = PopFont();
		}
		else
		{
			Subscript++;
			if ((Subscript==1) && (Superscript==0))
			{
				nonScriptFont=currentFont;
				PushFont(currentFont);
				font = hw->html.supsub_font;
			}
		}
		break;
	case M_CENTER:
		if (mark->is_end)
			Centered = 0;
		else
			Centered = 1;
		break;
/* amb - ignore text inside a HEAD element */
	case M_DOC_HEAD:
		if (mark->is_end)
		{
			InDocHead = 0;
			Ignore = 0;
		}
		else
		{
			InDocHead = 1;
			Ignore = 1;
		}
		break;
	case M_DOC_BODY:
		if (mark->is_end)
		{
			/* do nothing */
		}
		else
		{
			InDocHead = 0;   /* end <head> section */
			Ignore = 0;
		}
		break;
	case M_UNDERLINED:
		if (mark->is_end)
		{
			Underlines = 0;
			InUnderlined = 0;
		}
		else
		{
			Underlines = 1;
			InUnderlined = 1;
		}
		break;
		/*
		 * Headers are preceeded and followed by a linefeed,
		 * and the change the font.
		 */
	case M_HEADER_1:
		ConditionalLineFeed(hw, x, y, 1);
		if (mark->is_end)
		{
			font = PopFont();
			NewFont(font);
			currentFont = font;
			ConditionalLineFeed(hw, x, y, 2);
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 2);
			PushFont(currentFont);
			font = hw->html.header1_font;
		}
		break;
	case M_HEADER_2:
		ConditionalLineFeed(hw, x, y, 1);
		if (mark->is_end)
		{
			font = PopFont();
			NewFont(font);
			currentFont = font;
			ConditionalLineFeed(hw, x, y, 2);
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 2);
			PushFont(currentFont);
			font = hw->html.header2_font;
		}
		break;
	case M_HEADER_3:
		ConditionalLineFeed(hw, x, y, 1);
		if (mark->is_end)
		{
			font = PopFont();
			NewFont(font);
			currentFont = font;
			ConditionalLineFeed(hw, x, y, 2);
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 2);
			PushFont(currentFont);
			font = hw->html.header3_font;
		}
		break;
	case M_HEADER_4:
		ConditionalLineFeed(hw, x, y, 1);
		if (mark->is_end)
		{
			font = PopFont();
			NewFont(font);
			currentFont = font;
			ConditionalLineFeed(hw, x, y, 2);
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 2);
			PushFont(currentFont);
			font = hw->html.header4_font;
		}
		break;
	case M_HEADER_5:
		ConditionalLineFeed(hw, x, y, 1);
		if (mark->is_end)
		{
			font = PopFont();
			NewFont(font);
			currentFont = font;
			ConditionalLineFeed(hw, x, y, 2);
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 2);
			PushFont(currentFont);
			font = hw->html.header5_font;
		}
		break;
	case M_HEADER_6:
		ConditionalLineFeed(hw, x, y, 1);
		if (mark->is_end)
		{
			font = PopFont();
			NewFont(font);
			currentFont = font;
			ConditionalLineFeed(hw, x, y, 2);
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 2);
			PushFont(currentFont);
			font = hw->html.header6_font;
		}
		break;
	case M_FRAME:
		break;
		/*
		 * Anchors change the text color, and may set
		 * underlineing attributes.
		 * No linefeeds, so they can be imbedded anywhere.
		 */
	case M_ANCHOR:
		if (mark->is_end)
		{
/*
 * Without motif we use our own foreground resource instead of
 * using the manager's
 */
#ifdef MOTIF
			Fg = hw->manager.foreground;
#else
			Fg = hw->html.foreground;
#endif /* MOTIF */
			if (!InUnderlined) /* amb 2 */
				Underlines = 0;
			else
				Underlines = 1;
			DashedUnderlines = False;
			AnchorText = NULL;
		}
		else
		{
			char *tptr;

			/*
			 * Only change the color of anchors with
			 * HREF tags, because other anchors are
			 * not active.
			 */

			tptr = ParseMarkTag(mark->start,
								MT_ANCHOR, AT_HREF);
			if (tptr != NULL)
			{
				/*
				 * If internal check our internal list
				 * to change color if visited before.
				 */
				if (Internal == True)
				{
					struct ref_rec *hptr;

					hptr = FindHRef(
						hw->html.my_visited_hrefs,
						tptr);
					if (hptr != NULL)
					{
					    Fg = hw->html.visitedAnchor_fg;
					    Underlines = hw->html.num_visitedAnchor_underlines;
					    DashedUnderlines = hw->html.dashed_visitedAnchor_lines;
					}
					else
					{
					    Fg = hw->html.anchor_fg;
					    Underlines = hw->html.num_anchor_underlines;
					    DashedUnderlines = hw->html.dashed_anchor_lines;
					}
				}
				/*
				 * Else we may want to send
				 * the href back somewhere else and
				 * find out if we've visited it before
				 */
				else if (hw->html.previously_visited_test !=
						 NULL)
				{
					if ((*(visitTestProc)
						 (hw->html.previously_visited_test))
						(hw, tptr))
					{
					    Fg = hw->html.visitedAnchor_fg;
					    Underlines = hw->html.num_visitedAnchor_underlines;
					    DashedUnderlines = hw->html.dashed_visitedAnchor_lines;
					}
					else
					{
					    Fg = hw->html.anchor_fg;
					    Underlines = hw->html.num_anchor_underlines;
					    DashedUnderlines = hw->html.dashed_anchor_lines;
					}
				}
				else
				{
					Fg = hw->html.anchor_fg;
					Underlines = hw->html.num_anchor_underlines;
					DashedUnderlines = hw->html.dashed_anchor_lines;
				}
				if (tptr)
					free(tptr);
			}
			AnchorText = mark->start;
			/* amb 2 */
			if (InUnderlined)
			{
				DashedUnderlines = False;
				if (!Underlines)
					Underlines = 1;
			}
		}
		break;
		/*
		 * Just insert a linefeed, or ignore if this is prefomatted
		 * text because the <P> will be followed be a linefeed.
		 */
	case M_PARAGRAPH:
		ConditionalLineFeed(hw, x, y, 1);
		ConditionalLineFeed(hw, x, y, 2);
		break;
		/*
		 * Just insert the image for now
		 */
	case M_FIGURE:
	case M_IMAGE:
		if (mark->is_end)
		{
			/* do nothing */
		}
		else
		{
			ImagePlace(hw, *mptr, x, y, Width);
		}
		break;
		/*
		 * Can only be inside a SELECT tag.
		 */
	case M_OPTION:
		if (CurrentSelect != NULL)
		{
			char *tptr;

			if (CurrentSelect->option_buf != NULL)
			{
				ProcessOption(CurrentSelect);
			}
			CurrentSelect->option_buf = (char *)malloc(1);
			strcpy(CurrentSelect->option_buf, "");

			/*
			 * Check if this option starts selected
			 */
			tptr = ParseMarkTag(mark->start,
								MT_OPTION, "SELECTED");
			if (tptr != NULL)
			{
				CurrentSelect->is_value = 1;
				free(tptr);
			}
			else
			{
				CurrentSelect->is_value = 0;
			}

			/*
			 * Check if this option has an different
			 * return value field.
			 */
			tptr = ParseMarkTag(mark->start,
								MT_OPTION, "VALUE");
			if (tptr != NULL)
			{
				if (*tptr != '\0')
				{
					CurrentSelect->retval_buf = tptr;
				}
				else
				{
					CurrentSelect->retval_buf = NULL;
					free(tptr);
				}
			}
			else
			{
				CurrentSelect->retval_buf = NULL;
			}
		}
		break;
		/*
		 * Special INPUT tag.  Allows an option menu or
		 * a scrolled list.
		 * Due to a restriction in SGML, this can't just be a
		 * subset of the INPUT markup.  However, I can treat it
		 * that way to avoid duplicating code.
		 * As a result I combine SELECT and OPTION into a faked
		 * up INPUT mark.
		 */
	case M_SELECT:
		if (CurrentForm != NULL)
		{
			if ((mark->is_end)&&(CurrentSelect != NULL))
			{
				int len;
				char *buf;
				char *start;
				char *options, *returns, *value;

				if (CurrentSelect->option_buf != NULL)
				{
					ProcessOption(CurrentSelect);
				}

				options = ComposeCommaList(
					CurrentSelect->options,
					CurrentSelect->option_cnt);
				returns = ComposeCommaList(
					CurrentSelect->returns,
					CurrentSelect->option_cnt);
				value = ComposeCommaList(
					CurrentSelect->value,
					CurrentSelect->value_cnt);
				FreeCommaList(
					CurrentSelect->options,
					CurrentSelect->option_cnt);
				FreeCommaList(
					CurrentSelect->returns,
					CurrentSelect->option_cnt);
				FreeCommaList(
					CurrentSelect->value,
					CurrentSelect->value_cnt);

				/*
				 * Construct a fake INPUT tag.
				 */
				len = strlen(MT_INPUT) +
					strlen(options) +
					strlen(returns) +
					strlen(value) + strlen(
						" type=select options=\"\" returns=\"\" value=\"\"");
				buf = (char *)malloc(len +
									 strlen(CurrentSelect->mptr->start)
									 + 1);
				strcpy(buf, MT_INPUT);
				strcat(buf, " type=select");
				strcat(buf, " options=\"");
				strcat(buf, options);
				strcat(buf, "\" returns=\"");
				strcat(buf, returns);
				strcat(buf, "\" value=\"");
				strcat(buf, value);
				strcat(buf, "\"");
				strcat(buf, (char *)
					   (CurrentSelect->mptr->start +
						strlen(MT_SELECT)));
				/*
				 * stick the fake in, saving the
				 * real one.
				 */
				start = CurrentSelect->mptr->start;
				CurrentSelect->mptr->start = buf;
				WidgetPlace(hw, CurrentSelect->mptr,
							x, y, Width);
				/*
				 * free the fake, put the original back
				 */
				free(buf);
				free(options);
				free(returns);
				free(value);
				CurrentSelect->mptr->start = start;

				free((char *)CurrentSelect);
				CurrentSelect = NULL;
				Ignore = 0;
			}
			else if ((!mark->is_end)&&(CurrentSelect == NULL))
			{
				CurrentSelect = (SelectInfo *)malloc(
					sizeof(SelectInfo));
				CurrentSelect->hw = (Widget)hw;
				CurrentSelect->mptr = *mptr;
				CurrentSelect->option_cnt = 0;
				CurrentSelect->returns = NULL;
				CurrentSelect->retval_buf = NULL;
				CurrentSelect->options = NULL;
				CurrentSelect->option_buf = NULL;
				CurrentSelect->value_cnt = 0;
				CurrentSelect->value = NULL;
				CurrentSelect->is_value = -1;
				Ignore = 1;
			}
		}
		break;
		/*
		 * TEXTAREA is a replacement for INPUT type=text size=rows,cols
		 * because SGML will not allow an arbitrary length value
		 * field.
		 */
	case M_TEXTAREA:
		if (CurrentForm != NULL)
		{
			if ((mark->is_end)&&(TextAreaBuf != NULL))
			{
				char *start;
				char *buf;

				/*
				 * Finish a fake INPUT tag.
				 */
				buf = (char *)malloc(
					strlen(TextAreaBuf) + 2);
				strcpy(buf, TextAreaBuf);
				strcat(buf, "\"");

				/*
				 * stick the fake in, saving the
				 * real one.
				 */
				start = mark->start;
				mark->start = buf;
				mark->is_end = 0;
				WidgetPlace(hw, mark, x, y, Width);

				/*
				 * free the fake, put the original back
				 */
				free(buf);
				free(TextAreaBuf);
				mark->start = start;
				mark->is_end = 1;
				TextAreaBuf = NULL;
				Ignore = 0;
			}
			else if ((!mark->is_end)&&(TextAreaBuf == NULL))
			{
				char *buf;
				int len;

				/*
				 * Construct  the start of
				 * a fake INPUT tag.
				 */
				len = strlen(MT_INPUT) +
					strlen(
						" type=textarea value=\"\"");
				buf = (char *)malloc(len +
									 strlen(mark->start)
									 + 1);
				strcpy(buf, MT_INPUT);
				strcat(buf, (char *)
					   (mark->start +
						strlen(MT_TEXTAREA)));
				strcat(buf, " type=textarea");
				strcat(buf, " value=\"");

				TextAreaBuf = buf;
				Ignore = 1;
			}
		}
		break;
		/*
		 * Just insert the widget.
		 * Can only inside a FORM tag.
		 * Special case the type=image stuff to become a special
		 * IMG tag.
		 */
	case M_INPUT:
		if (CurrentForm != NULL)
		{
			char *tptr;
			char *tptr2;

			tptr = ParseMarkTag((*mptr)->start,
								MT_INPUT, "TYPE");
			if ((tptr != NULL)&&
				(caseless_equal(tptr, "image")))
			{
				free(tptr);
				tptr = (char *)malloc(
					strlen((*mptr)->start) +
					strlen(" ISMAP") +
					strlen(MT_IMAGE) -
					strlen(MT_INPUT) + 1);
				strcpy(tptr, MT_IMAGE);
				strcat(tptr, (char *)
					   ((*mptr)->start + strlen(MT_INPUT))
					);
				strcat(tptr, " ISMAP");
				tptr2 = (*mptr)->start;
				(*mptr)->start = tptr;
				ImagePlace(hw, *mptr, x, y, Width);
				(*mptr)->start = tptr2;
				free(tptr);
			}
			/*
			 * hidden inputs have no element associated
			 * with them, just a widget record.
			 */
			else if ((tptr != NULL)&&
					 (caseless_equal(tptr, "hidden")))
			{
				free(tptr);
				WidgetId++;
				(void)MakeWidget(hw, (*mptr)->start, x, y,
								 WidgetId, CurrentForm);
			}
			else
			{
				if (tptr != NULL)
				{
					free(tptr);
				}
				WidgetPlace(hw, *mptr, x, y, Width);
			}
		}
		break;
		/*
		 * Fillout forms.  Cannot be nested.
		 */
	case M_FORM:
		ConditionalLineFeed(hw, x, y, 1);
		if ((mark->is_end)&&(CurrentForm != NULL))
		{
			CurrentForm->end = WidgetId;
			ConditionalLineFeed(hw, x, y, 2);
			AddNewForm(hw, CurrentForm);
			CurrentForm = NULL;
		}
		else if ((!mark->is_end)&&(CurrentForm == NULL))
		{
			ConditionalLineFeed(hw, x, y, 2);
			CurrentForm = (FormInfo *)malloc(
				sizeof(FormInfo));
			CurrentForm->next = NULL;
			CurrentForm->hw = (Widget)hw;
			CurrentForm->action = ParseMarkTag(mark->start,
											   MT_FORM, "ACTION");
			CurrentForm->format = ParseMarkTag(mark->start,
											   MT_FORM, "FORMAT");
			CurrentForm->method = ParseMarkTag(mark->start,
											   MT_FORM, "METHOD");
			CurrentForm->enctype = ParseMarkTag(mark->start,
												MT_FORM, "ENCTYPE");
			CurrentForm->enc_entity = ParseMarkTag(
				mark->start, MT_FORM, "ENCENTITY");
			CurrentForm->start = WidgetId;
			CurrentForm->end = -1;
			CurrentForm->button_pressed=NULL;
		}
		break;
		/*
		 * Addresses are just like headers.  A linefeed before and
		 * after, and change the font.
		 */
	case M_ADDRESS:
		ConditionalLineFeed(hw, x, y, 1);
		if (mark->is_end)
		{
			font = PopFont();
		}
		else
		{
			PushFont(currentFont);
			font = hw->html.address_font;
		}
		break;
		/*
		 * Blockquotes increase the margin width.
		 * They cannot be nested.
		 */
	case M_BLOCKQUOTE:
		ConditionalLineFeed(hw, x, y, 1);
		if (mark->is_end)
		{
			MarginW = hw->html.margin_width;
			/*
			 * Only unindent if we think we indented
			 * when we started the blockquote
			 */
			if (TextIndent <= (2 * MarginW))
			{
				TextIndent = MarginW;
			}
			ConditionalLineFeed(hw, x, y, 2);
			/*
			 * The linefeed should have set x to TextIndent
			 * but since it is conditional, it might not
			 * have, so check it here.
			 */
			if (*x > TextIndent)
			{
				*x = TextIndent;
			}
		}
		else
		{
			MarginW = 2 * hw->html.margin_width;
			/*
			 * Only indent if the current indent
			 * is less than what we want.
			 */
			if (TextIndent < MarginW)
			{
				TextIndent = MarginW;
			}
			ConditionalLineFeed(hw, x, y, 2);
			/*
			 * The linefeed should have set x to TextIndent
			 * but since it is conditional, it might not
			 * have, so check it here.
			 */
			if (*x < TextIndent)
			{
				*x = TextIndent;
			}
		}
		break;
		/*
		 * Plain text.  A single pre-formatted chunk of text
		 * in its own font.
		 */
	case M_PLAIN_TEXT:
		if (mark->is_end)
		{
			Preformat = 0;
			/*
			 * Properly convert the Linefeed state
			 * variable from preformat to formatted
			 * state.
			 */
			if (PF_LF_State == 2)
			{
				PF_LF_State = 1;
			}
			else
			{
				PF_LF_State = 0;
			}
			ConditionalLineFeed(hw, x, y, 1);
			font = PopFont();
			NewFont(font);
			currentFont = font;
			ConditionalLineFeed(hw, x, y, 2);
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 1);
			ConditionalLineFeed(hw, x, y, 2);
			Preformat = 1;
			PF_LF_State = 0;
			PushFont(currentFont);
			font = hw->html.plain_font;
		}
		break;
		/*
		 * Listing text.  A different pre-formatted chunk of text
		 * in its own font.
		 */
	case M_LISTING_TEXT:
		if (mark->is_end)
		{
			Preformat = 0;
			/*
			 * Properly convert the Linefeed state
			 * variable from preformat to formatted
			 * state.
			 */
			if (PF_LF_State == 2)
			{
				PF_LF_State = 1;
			}
			else
			{
				PF_LF_State = 0;
			}
			ConditionalLineFeed(hw, x, y, 1);
			font = PopFont();
			NewFont(font);
			currentFont = font;
			ConditionalLineFeed(hw, x, y, 2);
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 1);
			ConditionalLineFeed(hw, x, y, 2);
			Preformat = 1;
			PF_LF_State = 0;
			PushFont(currentFont);
			font = hw->html.listing_font;
		}
		break;
		/*
		 * Plain text.  The rest of the text is pre-formatted.
		 * There is not end for this mark.
		 */
	case M_PLAIN_FILE:
		ConditionalLineFeed(hw, x, y, 1);
		ConditionalLineFeed(hw, x, y, 2);
		Preformat = 1;
		PF_LF_State = 0;
		PushFont(currentFont);
		font = hw->html.plain_font;
		break;
		/*
		 * Numbered lists, Unnumbered lists, Menus.
		 * Currently also lump directory listings into this.
		 * Save state for each indent level.
		 * Change the value of the TxtIndent (can be nested)
		 * Linefeed at the end of the list.
		 */
	case M_NUM_LIST:
	case M_UNUM_LIST:
	case M_MENU:
	case M_DIRECTORY:
		ConditionalLineFeed(hw, x, y, 1);
		width = hw->html.font->max_bounds.width;
		/*
		 * If this is the outermost level of indentation,
		 * add another linefeed for more white space.
		 */
		if ((TextIndent <= MarginW)||((mark->is_end)&&
									  ((TextIndent - ((INDENT_SPACES + 1) * width)) <=
									   MarginW)))
		{
			ConditionalLineFeed(hw, x, y, 2);
		}
		if (mark->is_end)
		{
			TextIndent = TextIndent -
				((INDENT_SPACES + 1) * width);
			if (TextIndent < MarginW)
			{
				TextIndent = MarginW;
			}
			IndentLevel--;
			if (IndentLevel < 0)
			{
				IndentLevel = 0;
			}

			/*
			 * restore the old state if there is one
			 */
			if (ListData->next != NULL)
			{
				DescRec *dptr;

				dptr = ListData;
				ListData = ListData->next;
				free((char *)dptr);
			}
		}
		else
		{
			DescRec *dptr;

			dptr = (DescRec *)malloc(sizeof(DescRec));
			/*
			 * Save the old state, and start a new
			 */
			if (type == M_NUM_LIST)
			{
				dptr->type = D_OLIST;
				dptr->count = 1;
			}
			else
			{
				dptr->type = D_ULIST;
				dptr->count = 0;
			}
			dptr->next = ListData;
			ListData = dptr;

			TextIndent = TextIndent +
				((INDENT_SPACES + 1) * width);
			IndentLevel++;
		}
		*x = TextIndent;
		break;
		/*
		 * Place the bullet element at the beginning of this item.
		 */
	case M_LIST_ITEM:
		if (!mark->is_end)
		{
			ConditionalLineFeed(hw, x, y, 1);
			/*
			 * for ordered/numbered lists
			 * put numbers in place of bullets.
			 */
			if (ListData->type == D_OLIST)
			{
				ListNumberPlace(hw, x, y,
								ListData->count);
				ListData->count++;
			}
			else
			{
				BulletPlace(hw, x, y);
			}
		}
		break;
		/*
		 * Description lists
		 */
	case M_DESC_LIST:
		ConditionalLineFeed(hw, x, y, 1);
		ConditionalLineFeed(hw, x, y, 2);
		width = hw->html.font->max_bounds.width;
		if (mark->is_end)
		{
			if (DescType->type == D_TEXT)
			{
				TextIndent = TextIndent -
					((INDENT_SPACES + 1) * width);
				if (TextIndent < MarginW)
				{
					TextIndent = MarginW;
				}
			}
			/*
			 * restore the old state if there is one
			 */
			if (DescType->next != NULL)
			{
				DescRec *dptr;

				dptr = DescType;
				DescType = DescType->next;
				free((char *)dptr);
				/*
				 * If the old state had forced an
				 * indent, outdent it now.
				 */
				if (DescType->type == D_TITLE)
				{
					TextIndent = TextIndent -
						((INDENT_SPACES + 1) * width);
					if (TextIndent < MarginW)
					{
						TextIndent = MarginW;
					}
				}
			}
		}
		else
		{
			DescRec *dptr;
			char *tptr;

			dptr = (DescRec *)malloc(sizeof(DescRec));
			/*
			 * Check is this is a compact list
			 */
			tptr = ParseMarkTag(mark->start,
								MT_DESC_LIST, "COMPACT");
			if (tptr != NULL)
			{
				free(tptr);
				dptr->compact = 1;
			}
			else
			{
				dptr->compact = 0;
			}
			/*
			 * Description list stared after a title needs
			 * a forced indentation here
			 */
			if (DescType->type == D_TITLE)
			{
				TextIndent = TextIndent +
					((INDENT_SPACES + 1) * width);
			}
			/*
			 * Save the old state, and start a new
			 */
			dptr->type = D_TITLE;
			dptr->next = DescType;
			DescType = dptr;
		}
		*x = TextIndent;
		break;
	case M_DESC_TITLE:
		ConditionalLineFeed(hw, x, y, 1);
		width = hw->html.font->max_bounds.width;
		/*
		 * Special hack.  Don't indent again for
		 * multiple <dt>'s in a row.
		 */
		if (DescType->type == D_TEXT)
		{
			TextIndent = TextIndent -
				((INDENT_SPACES + 1) * width);
			if (TextIndent < MarginW)
			{
				TextIndent = MarginW;
			}
		}
		DescType->type = D_TITLE;
		*x = TextIndent;
		break;
	case M_DESC_TEXT:
		width = hw->html.font->max_bounds.width;

		/*
		 * For a compact list we want to stay on the same
		 * line if there is room and we are the first line
		 * after a title.
		 */
		if ((DescType->compact)&&(DescType->type == D_TITLE)&&
			(*x < (TextIndent + (INDENT_SPACES * width))))
		{
			NeedSpace = 0;
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 1);
		}

		/*
		 * Special hack.  Don't indent again for
		 * multiple <dd>'s in a row.
		 */
		if (DescType->type == D_TITLE)
		{
			TextIndent = TextIndent +
				((INDENT_SPACES + 1) * width);
		}
		DescType->type = D_TEXT;
		*x = TextIndent;
		break;
	case M_PREFORMAT:
		if (mark->is_end)
		{
			Preformat = 0;
			/*
			 * Properly convert the Linefeed state
			 * variable from preformat to formatted
			 * state.
			 */
			if (PF_LF_State == 2)
			{
				PF_LF_State = 1;
			}
			else
			{
				PF_LF_State = 0;
			}
			ConditionalLineFeed(hw, x, y, 1);
			if (saveFont != NULL)
			{
				hw->html.font = saveFont;
				saveFont = NULL;
			}
			font = PopFont();
			NewFont(font);
			currentFont = font;
			ConditionalLineFeed(hw, x, y, 2);
		}
		else
		{
			ConditionalLineFeed(hw, x, y, 1);
			ConditionalLineFeed(hw, x, y, 2);
			Preformat = 1;
			PF_LF_State = 2;
			if (saveFont == NULL)
			{
				saveFont = hw->html.font;
				hw->html.font = hw->html.plain_font;
			}
			PushFont(currentFont);
			font = hw->html.font;
		}
		break;
		/*
		 * Now with forms, <INDEX> is the same as:
		 * <FORM>
		 * <HR>
		 * This is a searchable index.  Enter search keywords:
		 * <INPUT NAME="isindex">
		 * <HR>
		 * </FORM>
		 * Also, <INDEX> will take an ACTION tag to specify a
		 * different URL to submit the query to.
		 */
	case M_INDEX:
		hw->html.is_index = True;
		/*
		 * No index inside a form
		 */
		if (CurrentForm == NULL)
		{
			struct mark_up mark_tmp;

			/*
			 * Start the form
			 */
			ConditionalLineFeed(hw, x, y, 1);
			ConditionalLineFeed(hw, x, y, 2);
			CurrentForm = (FormInfo *)malloc(
				sizeof(FormInfo));
			CurrentForm->next = NULL;
			CurrentForm->hw = (Widget)hw;
			CurrentForm->action = NULL;
			CurrentForm->action = ParseMarkTag(mark->start,
											   MT_INDEX, "ACTION");
			CurrentForm->format = ParseMarkTag(mark->start,
											   MT_INDEX, "FORMAT");
			CurrentForm->method = ParseMarkTag(mark->start,
											   MT_INDEX, "METHOD");
			CurrentForm->enctype = ParseMarkTag(mark->start,
												MT_INDEX, "ENCTYPE");
			CurrentForm->enc_entity = ParseMarkTag(
				mark->start, MT_INDEX, "ENCENTITY");
			CurrentForm->start = WidgetId;
			CurrentForm->end = -1;

			/*
			 * Horizontal rule
			 */
			ConditionalLineFeed(hw, x, y, 1);
			HRulePlace(hw, x, y, Width);
			ConditionalLineFeed(hw, x, y, 1);

			/*
			 * Text: "This is a searchable index.
			 *  Enter search keywords: "
			 */
			mark_tmp.text = (char *)malloc(
				strlen("This is a searchable index.  Enter search keywords: ") + 1);
			strcpy(mark_tmp.text,"This is a searchable index.  Enter search keywords: ");
			FormatPlace(hw, &mark_tmp, x, y, Width);

			/*
			 * Fake up the text INPUT tag.
			 */
			mark_tmp.start = (char *)malloc(
				strlen("input SIZE=25 NAME=\"isindex\"") + 1);
			strcpy(mark_tmp.start,"input SIZE=25 NAME=\"isindex\"");
			WidgetPlace(hw, &mark_tmp, x, y, Width);

#ifdef ISINDEX_SUBMIT
			/*
			 * Text: ";<CR> press this button to submit
			 * the query: "
			 */
			mark_tmp.text = (char *)malloc(
				strlen(";\n press this button to submit the query: ") + 1);
			strcpy(mark_tmp.text,";\n press this button to submit the query: ");
			FormatPlace(hw, &mark_tmp, x, y, Width);

			/*
			 * Fake up the submit INPUT tag.
			 */
			mark_tmp.start = (char *)malloc(
				strlen("input TYPE=\"submit\"") + 1);
			strcpy(mark_tmp.start, "input TYPE=\"submit\"");
			WidgetPlace(hw, &mark_tmp, x, y, Width);

			/*
			 * Text: ".<CR>"
			 */
			mark_tmp.text = (char *)malloc(
				strlen(".\n") + 1);
			strcpy(mark_tmp.text, ".\n");
			FormatPlace(hw, &mark_tmp, x, y, Width);
#endif /* ISINDEX_SUBMIT */

			/*
			 * Horizontal rule
			 */
			ConditionalLineFeed(hw, x, y, 1);
			HRulePlace(hw, x, y, Width);
			ConditionalLineFeed(hw, x, y, 1);

			/*
			 * Close the form
			 */
			ConditionalLineFeed(hw, x, y, 1);
			CurrentForm->end = WidgetId;
			ConditionalLineFeed(hw, x, y, 2);
			AddNewForm(hw, CurrentForm);
			CurrentForm = NULL;
		}
		break;
	case M_HRULE:
		ConditionalLineFeed(hw, x, y, 1);
		HRulePlace(hw, x, y, Width);
		ConditionalLineFeed(hw, x, y, 1);
		break;
	case M_LINEBREAK:
		LineFeed(hw, x, y);
		break;
	case M_TABLE:
		if (tableSupportEnabled) {
			TablePlace(hw, mptr, x, y, Width);
		}
		break;
	default:
		break;
	}
	if ((font != NULL)&&(font != currentFont))
	{
		NewFont(font);
		currentFont = font;
	}

} /* TriggerMarkChanges() */


/*
 * Format all the objects in the passed Widget's
 * parsed object list to fit the locally global Width.
 * Passes in the x,y coords of where to start placing the
 * formatted text.
 * Returns the ending x,y in same variables.
 * Title objects are ignored, and not formatted.
 *
 * The locally global variables are assumed to have been initialized
 * before this function was called.
 */
void
FormatChunk(hw, x, y)
	HTMLWidget hw;
	int *x, *y;
{
	struct mark_up *mptr;

	/*
	 * Format all objects
	 */
	mptr = hw->html.html_objects;
	Last = NULL;
	while (mptr != NULL)
	{
		TriggerMarkChanges(hw, &mptr, x, y);

		/*
		 * Save last non-text mark
		 */
		 /* DDT: why is this here? it's not used anywhere? */
		if (mptr->type != M_NONE)
		{
			Last = mptr;
		}
		/*****/


		if (mptr) {
			mptr = mptr->next;
			}
	}
}


/*
 * Called by the widget to format all the objects in the
 * parsed object list to fit its current window size.
 * Returns the max_height of the entire document.
 * Title objects are ignored, and not formatted.
 */
int
FormatAll(hw, Fwidth)
	HTMLWidget hw;
	int *Fwidth;
{
	int x, y;
	int width;
	struct mark_up *msave;

#ifndef DISABLE_TRACE
	if (htmlwTrace) {
#ifndef VMS
		gettimeofday(&Tv, &Tz);
		fprintf(stderr, "FormatAll enter (%d.%d)\n", Tv.tv_sec, Tv.tv_usec);
#else
                fprintf(stderr, "FormatAll enter (%s)\n", asctime(localtime(&clock)));
#endif
	}
#endif

	width = *Fwidth;
	MaxWidth = width;

	/*
	 * Clear the is_index flag
	 */
	hw->html.is_index = False;

	/*
	 * Initialize local variables, some from the widget
	 */
	MarginW = hw->html.margin_width;
/*
 * Without motif we use our own foreground resource instead of
 * using the manager's
 */
#ifdef MOTIF
	Fg = hw->manager.foreground;
#else
	Fg = hw->html.foreground;
#endif /* MOTIF */
	Bg = hw->core.background_pixel;
	Underlines = 0;
	DashedUnderlines = False;
	Width = width;
	TextIndent = MarginW;
	ElementId = 0;
	WidgetId = 0;
	LineNumber = 1;
	LineBottom = 0;
	BaseLine = -100;
	CharsInLine = 0;
	IndentLevel = 0;
	Ignore = 0;
	Preformat = 0;
	PF_LF_State = 0;
	NeedSpace = 0;
	Internal = False;
	Strikeout = False;
	AnchorText = NULL;
	DescType = &BaseDesc;
	ListData = &BaseDesc;
	DescType->type = D_NONE;
	DescType->count = 0;
	DescType->compact = 0;
	DescType->next = NULL;
	CurrentForm = NULL;
	CurrentSelect = NULL;
	TextAreaBuf = NULL;
        Superscript = 0; /* amb */
        Subscript = 0;
	InDocHead = 0;
	InUnderlined = 0;

	/*
	 * Free the old title, if there is one.
	 */
	if (hw->html.title != NULL)
	{
		free(hw->html.title);
		hw->html.title = NULL;
	}
	TitleText = NULL;

#ifdef THROW_AWAY_OLD_LIST
	/*
	 * Free up previously formatted elements
	 */
	FreeLineList(hw->html.formatted_elements);
	hw->html.formatted_elements = NULL;
#endif

	/*
	 * Clear any previous selections
	 */
	hw->html.select_start = NULL;
	hw->html.select_end = NULL;
	hw->html.new_start = NULL;
	hw->html.new_end = NULL;

	/*
	 * Set up a starting font, and starting x, y, position
	 */
	NewFont(hw->html.font);
	currentFont = hw->html.font;
	saveFont = NULL;
	FontStack = &FontBase;
	FontStack->font = hw->html.font;

	x = TextIndent;
	y = hw->html.margin_height;

	/*
	 * Start a null element list, to be filled in as we go.
	 */
	Current = NULL;

	/*
	 * If we have parsed special header text, fill it in now.
	 */
	if (hw->html.html_header_objects != NULL)
	{
		msave = hw->html.html_objects;
		hw->html.html_objects = hw->html.html_header_objects;
		FormatChunk(hw, &x, &y);

		if (saveFont != NULL)
		{
			hw->html.font = saveFont;
			saveFont = NULL;
		}
		NewFont(hw->html.font);
		currentFont = hw->html.font;

		ConditionalLineFeed(hw, &x, &y, 1);

		hw->html.html_objects = msave;
	}

	/*
	 * Format all objects for width
	 */
	FormatChunk(hw, &x, &y);

	/*
	 * If we have parsed special footer text, fill it in now.
	 */
	if (hw->html.html_footer_objects != NULL)
	{
		if (saveFont != NULL)
		{
			hw->html.font = saveFont;
			saveFont = NULL;
		}
		NewFont(hw->html.font);
		currentFont = hw->html.font;

		Preformat = 0;
		PF_LF_State = 0;
		NeedSpace = 0;

		ConditionalLineFeed(hw, &x, &y, 1);

		msave = hw->html.html_objects;
		hw->html.html_objects = hw->html.html_footer_objects;
		FormatChunk(hw, &x, &y);

		hw->html.html_objects = msave;
	}

	/*
	 * Ensure a linefeed after the final element.
	 */
	ConditionalLineFeed(hw, &x, &y, 1);

	/*
	 * Restore the proper font from unterminated preformatted text
	 * sequences.
	 */
	if (saveFont != NULL)
	{
		hw->html.font = saveFont;
		saveFont = NULL;
	}

	/*
	 * Free any extra of the pre-allocated list.
	 * Terminate the element list.
	 */
	if ((Current != NULL)&&(Current->next != NULL))
	{
		FreeLineList(Current->next);
		Current->next = NULL;
	}
	else if ((Current == NULL)&&(hw->html.formatted_elements != NULL))
	{
		FreeLineList(hw->html.formatted_elements);
		hw->html.formatted_elements = NULL;
	}

	/*
	 * Add the bottom margin to the max height.
	 */
	y = y + hw->html.margin_height;

	/*
	 * Make the line array indexed into the element list
	 * and store it into the widget
	 */
	hw->html.line_count = LineNumber;
	if (hw->html.line_array != NULL)
	{
		free((char *)hw->html.line_array);
	}
	hw->html.line_array = MakeLineList(hw->html.formatted_elements,
						LineNumber);

	/*
	 * If the passed in MaxWidth was wrong, correct it.
	 */
	if (MaxWidth != width)
	{
		*Fwidth = MaxWidth;
	}

#ifndef DISABLE_TRACE
	if (htmlwTrace) {
#ifndef VMS
		gettimeofday(&Tv, &Tz);
		fprintf(stderr, "FormatAll exit (%d.%d)\n", Tv.tv_sec, Tv.tv_usec);
#else
                fprintf(stderr, "FormatAll exit (%s)\n", asctime(localtime(&clock)));
#endif
	}
#endif

	return(y);
}


/*
 * Redraw a linefeed.
 * Basically a filled rectangle at the end of a line.
 */
void
LinefeedRefresh(hw, eptr)
	HTMLWidget hw;
	struct ele_rec *eptr;
{
	int x1, y1;
	unsigned int width, height;

#ifdef NO_EXTRA_FILLS
	/*
	 * The actualt height of the rectangle to fill is strange, based
	 * an a differente between eptr->font->(ascent/descent) and
	 * eptr->font->max_bounds.(ascent/descent) which I don't quite
	 * understand. But it works.
	 * Deal with bad Lucidia descents.
	 */
	x1 = eptr->x;
	if (x1 > (int)hw->core.width)
	{
		width = 0;
	}
	else
	{
		width = hw->core.width - x1;
	}
#ifdef SHORT_LINEFEEDS
	y1 = eptr->y + eptr->y_offset + eptr->font->max_bounds.ascent -
		eptr->font->ascent;
	height = eptr->font->ascent + eptr->font->descent;
#else
	y1 = eptr->y + eptr->font->max_bounds.ascent - eptr->font->ascent;
	height = eptr->line_height;
#endif /* SHORT_LINEFEEDS */
#else
	x1 = eptr->x;
	if (x1 > (int)hw->core.width)
	{
		width = 0;
	}
	else
	{
		width = hw->core.width - x1;
	}
#ifdef SHORT_LINEFEEDS
	y1 = eptr->y + eptr->y_offset;
	if (eptr->font->descent > eptr->font->max_bounds.descent)
	{
		height = eptr->font->max_bounds.ascent +
			eptr->font->descent;
	}
	else
	{
		height = eptr->font->max_bounds.ascent +
			eptr->font->max_bounds.descent;
	}
#else
	y1 = eptr->y;
	height = eptr->line_height;
#endif /* SHORT_LINEFEEDS */
#endif /* NO_EXTRA_FILLS */

	x1 = x1 - hw->html.scroll_x;
	y1 = y1 - hw->html.scroll_y;

	if (eptr->selected == True)
	{
		XSetForeground(XtDisplay(hw), hw->html.drawGC, eptr->fg);
	}
	else
	{
		XSetForeground(XtDisplay(hw), hw->html.drawGC, eptr->bg);
	}

	/* Don't draw out past the end of the text if selecting */
	if (eptr->selected == False) {
		if (hw->html.bg_image) {
			HTMLDrawBackgroundImage((Widget)hw,
						x1,
						y1,
						width,
						height);
		}
		else {
			XFillRectangle(XtDisplay(hw->html.view),
				       XtWindow(hw->html.view),
				       hw->html.drawGC,
				       x1,
				       y1,
				       width,
				       height);
		}
	}
}


/*
 * Redraw part of a formatted text element, in the passed fg and bg
 */
void
PartialRefresh(hw, eptr, start_pos, end_pos, fg, bg)
	HTMLWidget hw;
	struct ele_rec *eptr;
	int start_pos, end_pos;
	unsigned long fg, bg;
{
	int ascent;
	char *tdata;
	int tlen;
	int x, y, width;
	int partial, descent;
	unsigned long valuemask;
	XGCValues values;

	XSetFont(XtDisplay(hw), hw->html.drawGC, eptr->font->fid);
	ascent = eptr->font->max_bounds.ascent;
	width = -1;
	partial = 0;

	if (start_pos != 0)
	{
		int dir, nascent;
		XCharStruct all;

#ifndef ASSUME_FIXED_WIDTH_PRE
		if (eptr->font == hw->html.plain_font)
		{
			all.width = eptr->font->max_bounds.width * start_pos;
		}
		else
		{
			XTextExtents(eptr->font, (char *)eptr->edata,
				start_pos, &dir, &nascent, &descent, &all);
		}
#else
		XTextExtents(eptr->font, (char *)eptr->edata,
			start_pos, &dir, &nascent, &descent, &all);
#endif /* ASSUME_FIXED_WIDTH_PRE */
		x = eptr->x + all.width;
		tdata = (char *)(eptr->edata + start_pos);
		partial = 1;
	}
	else
	{
		x = eptr->x;
		tdata = (char *)eptr->edata;
	}

	if (end_pos != (eptr->edata_len - 2))
	{
		tlen = end_pos - start_pos + 1;
		partial = 1;
	}
	else
	{
		tlen = eptr->edata_len - start_pos - 1;
	}

	y = eptr->y + eptr->y_offset;

	x = x - hw->html.scroll_x;
	y = y - hw->html.scroll_y;

#ifndef NO_EXTRA_FILLS
	{
		int dir, nascent, descent;
		XCharStruct all;
		int height;

		/*
		 * May be safe to used the cached full width of this
		 * string, and thus avoid a call to XTextExtents
		 */
		if ((!partial)&&(eptr->width != 0))
		{
			all.width = eptr->width;
		}
		else
		{
#ifdef ASSUME_FIXED_WIDTH_PRE
			if (eptr->font == hw->html.plain_font)
			{
				all.width = eptr->font->max_bounds.width * tlen;
			}
			else
			{
				XTextExtents(eptr->font, (char *)tdata,
					tlen, &dir, &nascent, &descent, &all);
			}
#else
			XTextExtents(eptr->font, (char *)tdata,
				tlen, &dir, &nascent, &descent, &all);
#endif /* ASSUME_FIXED_WIDTH_PRE */
		}

		XSetForeground(XtDisplay(hw), hw->html.drawGC, bg);

		height = (eptr->font->max_bounds.ascent - eptr->font->ascent);
		if (height > 0)
		{
		  if(!hw->html.bg_image)
		    XFillRectangle(XtDisplay(hw),
				   XtWindow(hw->html.view),
				   hw->html.drawGC, x, y,
				   (unsigned int)all.width, (unsigned int)height);
		}
		height = (eptr->font->max_bounds.descent - eptr->font->descent);
		if (height > 0)
		{
		  if(!hw->html.bg_image)
		    XFillRectangle(XtDisplay(hw),
				   XtWindow(hw->html.view),
				   hw->html.drawGC, x,
				   (int)(y + eptr->font->max_bounds.ascent +
					 eptr->font->descent),
				   (unsigned int)all.width, (unsigned int)height);
		}
		width = all.width;
	}
#endif /* NO_EXTRA_FILLS */

	if (bg!=hw->html.view->core.background_pixel || NoBodyImages(hw) || !hw->html.bg_image) {
		XSetForeground(XtDisplay(hw), hw->html.drawGC, bg);
		XSetBackground(XtDisplay(hw), hw->html.drawGC, fg);

		XFillRectangle(XtDisplay(hw),
			       XtWindow(hw->html.view),
			       hw->html.drawGC,
			       x,
			       y,
			       width,
			       ascent+eptr->font->descent);

		XSetForeground(XtDisplay(hw), hw->html.drawGC, fg);
		XSetBackground(XtDisplay(hw), hw->html.drawGC, bg);

		XDrawString(XtDisplay(hw),
			    XtWindow(hw->html.view),
			    hw->html.drawGC,
			    x,
			    y + ascent,
			    (char *)tdata,
			    tlen);
	}
	else {
		XSetForeground(XtDisplay(hw), hw->html.drawGC, bg);
		XSetBackground(XtDisplay(hw), hw->html.drawGC, fg);

		XFillRectangle(XtDisplay(hw),
			       XtWindow(hw->html.view),
			       hw->html.drawGC,
			       x,
			       y,
			       width,
			       ascent+eptr->font->descent);

		XSetForeground(XtDisplay(hw), hw->html.drawGC, fg);
		XSetBackground(XtDisplay(hw), hw->html.drawGC, bg);

		HTMLDrawBackgroundImage((Widget)hw,
					(x<0 ?
					 0 :
					 x),
					(y<0 ?
					 0 :
					 y),
					(x<0 ?
					 (width+x) :
					 width),
					(y<0 ?
					 (ascent+eptr->font->descent+y) :
					 (ascent+eptr->font->descent)));

		XDrawString(XtDisplay(hw),
			    XtWindow(hw->html.view),
			    hw->html.drawGC,
			    x,
			    y + ascent,
			    (char *)tdata,
			    tlen);

	}

	if (eptr->underline_number)
	{
		int i, ly;

		if (eptr->dashed_underline)
		{
			XSetLineAttributes(XtDisplay(hw), hw->html.drawGC, 1,
				LineOnOffDash, CapButt, JoinBevel);
		}
		else
		{
			XSetLineAttributes(XtDisplay(hw), hw->html.drawGC, 1,
				LineSolid, CapButt, JoinBevel);
		}

		if (width == -1)
		{
			int dir, nascent, descent;
			XCharStruct all;

#ifdef ASSUME_FIXED_WIDTH_PRE
			if (eptr->font == hw->html.plain_font)
			{
				all.width = eptr->font->max_bounds.width * tlen;
			}
			else
			{
				XTextExtents(eptr->font, (char *)tdata,
					tlen, &dir, &nascent, &descent,&all);
			}
#else
			XTextExtents(eptr->font, (char *)tdata,
				tlen, &dir, &nascent, &descent,&all);
#endif /* ASSUME_FIXED_WIDTH_PRE */
			width = all.width;
		}

		ly = (int)(y + eptr->font->max_bounds.ascent +
				eptr->font->descent - 1);

		for (i=0; i<eptr->underline_number; i++)
		{
			XDrawLine(XtDisplay(hw),
				XtWindow(hw->html.view), hw->html.drawGC,
				x, ly, (int)(x + width), ly);
			ly -= 2;
		}
	}

	if (eptr->strikeout == True)
	{
		int ly;

		XSetLineAttributes(XtDisplay(hw), hw->html.drawGC, 1,
			LineSolid, CapButt, JoinBevel);

		if (width == -1)
		{
			int dir, nascent, descent;
			XCharStruct all;

#ifdef ASSUME_FIXED_WIDTH_PRE
			if (eptr->font == hw->html.plain_font)
			{
				all.width = eptr->font->max_bounds.width * tlen;
			}
			else
			{
				XTextExtents(eptr->font, (char *)tdata,
					tlen, &dir, &nascent, &descent,&all);
			}
#else
			XTextExtents(eptr->font, (char *)tdata,
				tlen, &dir, &nascent, &descent,&all);
#endif /* ASSUME_FIXED_WIDTH_PRE */
			width = all.width;
		}

		ly = (int)(y + eptr->font->max_bounds.ascent +
				eptr->font->descent - 1);
		ly = ly - ((hw->html.font->max_bounds.ascent +
			hw->html.font->descent) / 2);

		XDrawLine(XtDisplay(hw), XtWindow(hw->html.view),
			hw->html.drawGC,
			x, ly, (int)(x + width), ly);
	}
}


/*
 * Redraw a formatted text element
 */
void
TextRefresh(hw, eptr, start_pos, end_pos)
	HTMLWidget hw;
	struct ele_rec *eptr;
	int start_pos, end_pos;
{
	if (eptr->selected == False)
	{
		PartialRefresh(hw, eptr, start_pos, end_pos,
			eptr->fg, eptr->bg);
	}
	else if ((start_pos >= eptr->start_pos)&&(end_pos <= eptr->end_pos))
	{
		PartialRefresh(hw, eptr, start_pos, end_pos,
			eptr->bg, eptr->fg);
	}
	else
	{
		if (start_pos < eptr->start_pos)
		{
			PartialRefresh(hw, eptr, start_pos, eptr->start_pos - 1,
				eptr->fg, eptr->bg);
			start_pos = eptr->start_pos;
		}
		if (end_pos > eptr->end_pos)
		{
			PartialRefresh(hw, eptr, eptr->end_pos + 1, end_pos,
				eptr->fg, eptr->bg);
			end_pos = eptr->end_pos;
		}
		PartialRefresh(hw, eptr, start_pos, end_pos,
			eptr->bg, eptr->fg);
	}
}


/*
 * Redraw a formatted bullet element
 */
void
BulletRefresh(hw, eptr)
	HTMLWidget hw;
	struct ele_rec *eptr;
{
	int width, line_height;
	int x1, y1;

/*
	width = eptr->font->max_bounds.width;
*/
	width = eptr->font->max_bounds.lbearing +
		eptr->font->max_bounds.rbearing;
	/*
	 * Deal with bad Lucidia descents.
	 */
	if (eptr->font->descent > eptr->font->max_bounds.descent)
	{
		line_height = eptr->font->max_bounds.ascent +
			eptr->font->descent;
	}
	else
	{
		line_height = eptr->font->max_bounds.ascent +
			eptr->font->max_bounds.descent;
	}
	x1 = eptr->x;
	y1 = eptr->y + eptr->y_offset + (line_height / 2) - (width / 4);
	x1 = x1 - hw->html.scroll_x;
	y1 = y1 - hw->html.scroll_y;
	XSetFont(XtDisplay(hw), hw->html.drawGC, eptr->font->fid);
	XSetForeground(XtDisplay(hw), hw->html.drawGC, eptr->fg);
	XSetBackground(XtDisplay(hw), hw->html.drawGC, eptr->bg);

/*
 * Rewritten to alternate circle, square and those paris alternate filled
 *   and not filled.
 * --SWP
 */
	if (eptr->indent_level && (eptr->indent_level % 2)) { /* odd & !0 */
		XSetLineAttributes(XtDisplay(hw),
				   hw->html.drawGC,
				   1,
				   LineSolid,
				   CapButt,
				   JoinBevel);
		if (eptr->indent_level==1 || (eptr->indent_level % 4)==1) {
			XFillArc(XtDisplay(hw),
				 XtWindow(hw->html.view),
				 hw->html.drawGC,
				 (x1 - width),
				 y1,
				 (width / 2),
				 (width / 2),
				 0,
				 23040);
		}
		else {
			XDrawArc(XtDisplay(hw),
				 XtWindow(hw->html.view),
				 hw->html.drawGC,
				 (x1 - width),
				 y1,
				 (width / 2),
				 (width / 2),
				 0,
				 23040);
		}
	}
	else { /* even */
		XSetLineAttributes(XtDisplay(hw),
				   hw->html.drawGC,
				   1,
				   LineSolid,
				   CapButt,
				   JoinBevel);
		if (eptr->indent_level==0 || (eptr->indent_level % 4)==2) {
			XFillRectangle(XtDisplay(hw),
				       XtWindow(hw->html.view),
				       hw->html.drawGC,
				       (x1 - width),
				       y1,
				       (width / 2),
				       (width / 2));
		}
		else {
			XDrawRectangle(XtDisplay(hw),
				       XtWindow(hw->html.view),
				       hw->html.drawGC,
				       (x1 - width),
				       y1,
				       (width / 2),
				       (width / 2));
		}
	}
}


/*
 * Redraw a formatted horizontal rule element
 */
void
HRuleRefresh(hw, eptr)
	HTMLWidget hw;
	struct ele_rec *eptr;
{
	int width, height;
	int x1, y1;

	width = (int)hw->html.view_width - (int)(2 * hw->html.margin_width);
	if (width < 0)
	{
		width = 0;
	}

	x1 = eptr->x;
	y1 = eptr->y;
	x1 = x1 - hw->html.scroll_x;
	y1 = y1 - hw->html.scroll_y;
	height = eptr->line_height;

	/* blank out area */
	XSetForeground(XtDisplay(hw), hw->html.drawGC, eptr->bg);
	if(!hw->html.bg_image)
	  XFillRectangle(XtDisplay(hw), XtWindow(hw->html.view),
			 hw->html.drawGC, x1, y1, width, height);
	y1 = y1 + (height / 2) - 1;

	XSetLineAttributes(XtDisplay(hw), hw->html.drawGC, 1,
		LineSolid, CapButt, JoinBevel);
#ifdef MOTIF
	XDrawLine(XtDisplay(hw), XtWindow(hw->html.view),
		hw->manager.bottom_shadow_GC,
		x1, y1, (int)(x1 + width), y1);
	XDrawLine(XtDisplay(hw), XtWindow(hw->html.view),
		hw->manager.top_shadow_GC,
		x1, y1 + 1, (int)(x1 + width), y1 + 1);
#else
	/* changing the GC back and forth is not the most efficient way.... */
	XSetForeground(XtDisplay(hw), hw->html.drawGC, eptr->fg);
	XDrawLine(XtDisplay(hw), XtWindow(hw->html.view),
		hw->html.drawGC,
		x1, y1, (int)(x1 + width), y1);
	XDrawLine(XtDisplay(hw), XtWindow(hw->html.view),
		hw->html.drawGC,
		x1, y1 + 1, (int)(x1 + width), y1 + 1);
#endif
}


/*
 * Redraw a formatted image element.
 * The color of the image border reflects whether it is an active anchor
 * or not.
 * Actual Pixmap creation was put off until now to make sure we
 * had a window.  If it hasn't been already created, make the Pixmap
 * now.
 */
void
ImageRefresh(hw, eptr)
	HTMLWidget hw;
	struct ele_rec *eptr;
{

unsigned long valuemask;
XGCValues values;

	if (eptr->pic_data != NULL)
	{
		int x, y, extra;

		x = eptr->x;
		y = eptr->y + eptr->y_offset;

		if ((hw->html.border_images == True)||
			((eptr->anchorHRef != NULL)&&
			(!eptr->pic_data->internal)))
		{
			if (eptr->pic_data->delayed) {
				extra=2;
			}
			else {
				extra = eptr->bwidth;
			}
		}
		else
		{
			extra = 0;
		}

		x = x - hw->html.scroll_x;
		y = y - hw->html.scroll_y;

		XSetForeground(XtDisplay(hw), hw->html.drawGC, eptr->fg);
		XSetBackground(XtDisplay(hw), hw->html.drawGC, eptr->bg);

		if (extra) {
			XFillRectangle(XtDisplay(hw),
				       XtWindow(hw->html.view), hw->html.drawGC,
				       x, y,
				       (eptr->pic_data->width + (2 * extra)),
				       extra);
			XFillRectangle(XtDisplay(hw),
				       XtWindow(hw->html.view), hw->html.drawGC,
				       x,
				       (y + eptr->pic_data->height + extra),
				       (eptr->pic_data->width + (2 * extra)),
				       extra);
			XFillRectangle(XtDisplay(hw),
				       XtWindow(hw->html.view), hw->html.drawGC,
				       x, y,
				       extra,
				       (eptr->pic_data->height + (2 * extra)));
			XFillRectangle(XtDisplay(hw),
				       XtWindow(hw->html.view), hw->html.drawGC,
				       (x + eptr->pic_data->width + extra),
				       y,
				       extra,
				       (eptr->pic_data->height + (2 * extra)));
		}

		if (eptr->pic_data->image == None)
		{
			if (eptr->pic_data->image_data != NULL)
			{
				eptr->pic_data->image = InfoToImage(hw,
					eptr->pic_data, 0);
				if (eptr->pic_data->transparent &&
				    eptr->pic_data->clip==None) {
					eptr->pic_data->clip =
						XCreatePixmapFromBitmapData
							(XtDisplay(hw),
							 XtWindow(hw->html.view),
							 eptr->pic_data->clip_data,
							 eptr->pic_data->width,
							 eptr->pic_data->height,
							 1,
							 0,
							 1);
				}
				else if (!eptr->pic_data->transparent) {
					eptr->pic_data->clip = None;
				}
			}
			else
			{
				if (eptr->pic_data->delayed)
				{
				    if ((eptr->anchorHRef != NULL)&&
					(!IsDelayedHRef(hw, eptr->anchorHRef))&&
					(!IsIsMapForm(hw, eptr->anchorHRef)))
				    {
					eptr->pic_data->image = DelayedImage(
						hw, True);
				    }
				    else
				    {
					eptr->pic_data->image = DelayedImage(
						hw, False);
				    }
				}
				else
				{
				    /*
				     * Could be that the user opened another
				     * window, and the Pixmap was freed, and
				     * then they overflowed the cache,
				     * and the XImage data was freed.
				     * If this image was ever successfully
				     * fetched, try again before giving up.
				     */
				    if ((eptr->pic_data->fetched)&&
					(hw->html.resolveDelayedImage != NULL))
				    {
					ImageInfo *pdata;

					pdata = eptr->pic_data;
				        eptr->pic_data = (*(resolveImageProc)
					(hw->html.resolveDelayedImage))(hw,
						eptr->edata);
					if (eptr->pic_data != NULL)
					{
					    eptr->pic_data->delayed = 0;
					    /*
					     * Mark images we have sucessfully
					     * loaded at least once
					     */
					    if (eptr->pic_data->image_data != NULL)
					    {
						eptr->pic_data->fetched = 1;
					    }
					    /*
					     * Copy over state information from
					     * the last time we had this image
					     */
					     eptr->pic_data->ismap =
						pdata->ismap;
					     eptr->pic_data->fptr =
						pdata->fptr;
					     eptr->pic_data->internal =
						pdata->internal;
					     eptr->pic_data->text =
						pdata->text;
					}
					else
					{
					    eptr->pic_data = NoImageData(hw);
					    eptr->pic_data->delayed = 0;
					    eptr->pic_data->internal = 0;
					}
				    }
				    else
				    {
					eptr->pic_data->image = NoImage(hw);
				    }
				}
			}
		}

		if (eptr->pic_data->image != None)
		{
			if (eptr->pic_data->transparent) {
				unsigned long valuemask;
				XGCValues values;

				values.clip_mask=eptr->pic_data->clip;
				values.clip_x_origin=x+extra;
				values.clip_y_origin=y+extra;
				valuemask=GCClipMask|GCClipXOrigin|GCClipYOrigin;
				XChangeGC(XtDisplay(hw),
					  hw->html.drawGC,
					  valuemask, &values);

				XCopyArea(XtDisplay(hw),
					  eptr->pic_data->image,
					  XtWindow(hw->html.view),
					  hw->html.drawGC,
					  0,
					  0,
					  eptr->pic_data->width,
					  eptr->pic_data->height,
					  (x + extra),
					  (y + extra));

				values.clip_mask=None;
				values.clip_x_origin=0;
				values.clip_y_origin=0;
				valuemask=GCClipMask|GCClipXOrigin|GCClipYOrigin;
				XChangeGC(XtDisplay(hw),
					  hw->html.drawGC,
					  valuemask, &values);
			}
			else {
				values.clip_mask=None;
				values.clip_x_origin=0;
				values.clip_y_origin=0;
				valuemask=GCClipMask|GCClipXOrigin|GCClipYOrigin;
				XChangeGC(XtDisplay(hw),
					  hw->html.drawGC,
					  valuemask, &values);

				XCopyArea(XtDisplay(hw),
					  eptr->pic_data->image,
					  XtWindow(hw->html.view),
					  hw->html.drawGC,
					  0,
					  0,
					  eptr->pic_data->width,
					  eptr->pic_data->height,
					  (x + extra),
					  (y + extra));
			}
		}
		if ((eptr->pic_data->delayed)&&(eptr->anchorHRef != NULL)&&
		    (!IsDelayedHRef(hw, eptr->anchorHRef))&&
		    (!IsIsMapForm(hw, eptr->anchorHRef))) {
			XSetForeground(XtDisplay(hw),
				       hw->html.drawGC,
				       eptr->fg);
			XFillRectangle(XtDisplay(hw->html.view),
				       XtWindow(hw->html.view),
				       hw->html.drawGC,
				       x,
				       (y + AnchoredHeight(hw)),
				       (eptr->pic_data->width + (2 * extra)),
				       extra);
		}
	}
}


void
RefreshTextRange(hw, start, end)
	HTMLWidget hw;
	struct ele_rec *start;
	struct ele_rec *end;
{
	struct ele_rec *eptr;

	eptr = start;
	while ((eptr != NULL)&&(eptr != end))
	{
		if (eptr->type == E_TEXT)
		{
			TextRefresh(hw, eptr,
				0, (eptr->edata_len - 2));
		}
		eptr = eptr->next;
	}
	if (eptr != NULL)
	{
		if (eptr->type == E_TEXT)
		{
			TextRefresh(hw, eptr,
				0, (eptr->edata_len - 2));
		}
	}
}


/*
 * Refresh all elements on a single line into the widget's window
 */
void
PlaceLine(hw, line)
	HTMLWidget hw;
	int line;
{
	struct ele_rec *eptr;
	XGCValues values;

	/*
	 * Item list for this line
	 */
	eptr = hw->html.line_array[line];

	while ((eptr != NULL)&&(eptr->line_number == (line + 1)))
	{
		switch(eptr->type)
		{
			case E_TEXT:
			        TextRefresh(hw, eptr,
					    0, (eptr->edata_len - 2));
				break;
			case E_BULLET:
				BulletRefresh(hw, eptr);
				break;
			case E_HRULE:
				HRuleRefresh(hw, eptr);
				break;
			case E_LINEFEED:
			        if(!hw->html.bg_image)
				  LinefeedRefresh(hw, eptr);
				break;
			case E_IMAGE:
				ImageRefresh(hw, eptr);
				break;
			case E_WIDGET:
				WidgetRefresh(hw, eptr);
				break;
			case E_TABLE:
				TableRefresh(hw, eptr);
				break;
		}
		eptr = eptr->next;
	}
}


/*
 * Locate the element (if any) that is at the passed location
 * in the widget.  If there is no corresponding element, return
 * NULL.  If an element is found return the position of the character
 * you are at in the pos pointer passed.
 */
struct ele_rec *
LocateElement(hw, x, y, pos)
	HTMLWidget hw;
	int x, y;
	int *pos;
{
	struct ele_rec *eptr;
	struct ele_rec *rptr;
	int i, start, end, line, guess;
	int tx1, tx2, ty1, ty2;

	x = x + hw->html.scroll_x;
	y = y + hw->html.scroll_y;

	/*
	 * Narrow the search down to a 2 line range
	 * before beginning to search element by element
	 */
	start = -1;
	end = -1;

	/*
	 * Heuristic to speed up redraws by guessing at the starting line.
	 */
	guess = y / (hw->html.font->max_bounds.ascent +
		hw->html.font->max_bounds.descent);
	if (guess > (hw->html.line_count - 1))
	{
		guess = hw->html.line_count - 1;
	}
	while (guess > 0)
	{
		if ((hw->html.line_array[guess] != NULL)&&
			(hw->html.line_array[guess]->y <= y))
		{
			break;
		}
		guess--;
	}
	if (guess < 0)
	{
		guess = 0;
	}

	for (i=guess; i<hw->html.line_count; i++)
	{
		if (hw->html.line_array[i] == NULL)
		{
			continue;
		}
		else if (hw->html.line_array[i]->y <= y)
		{
			start = i;
			continue;
		}
		else
		{
			end = i;
			break;
		}
	}

	/*
	 * Search may have already failed, or it may be a one line
	 * range.
	 */
	if ((start == -1)&&(end == -1))
	{
		return(NULL);
	}
	else if (start == -1)
	{
		start = end;
	}
	else if (end == -1)
	{
		end = start;
	}

	/*
	 * Search element by element, for now we only search
	 * text elements, images, and linefeeds.
	 */
	eptr = hw->html.line_array[start];
	ty1 = eptr->y;
	/*
	 * Deal with bad Lucidia descents.
	 */
	if (eptr->font->descent > eptr->font->max_bounds.descent)
	{
		ty2 = eptr->y + eptr->font->max_bounds.ascent +
			eptr->font->descent;
	}
	else
	{
		ty2 = eptr->y + eptr->font->max_bounds.ascent +
			eptr->font->max_bounds.descent;
	}
	line = eptr->line_number;
	/*
	 * Searches on this line should extend to the top of the
	 * next line, if possible.  Which might be far away if there
	 * is an image on this line.
	 */
	if (((line + 1) < hw->html.line_count)&&
		(hw->html.line_array[line + 1] != NULL))
	{
		ty2 = hw->html.line_array[line + 1]->y - 1;
	}
	/*
	 * Else we are at the last line, and need to find its height.
	 * The linefeed at the end should know the max height of the line.
	 */
	else
	{
		struct ele_rec *teptr;

		teptr = eptr;
		while (teptr != NULL)
		{
			if (teptr->type == E_LINEFEED)
			{
				break;
			}
			teptr = teptr->next;
		}
		if (teptr != NULL)
		{
			ty2 = teptr->y + teptr->line_height - 1;
		}
	}

	rptr = NULL;
	while ((eptr != NULL)&&(eptr->line_number <= (end + 1)))
	{
		if (eptr->line_number != line)
		{
			ty1 = ty2;
			/*
			 * Deal with bad Lucidia descents.
			 */
			if(eptr->font->descent > eptr->font->max_bounds.descent)
			{
				ty2 = eptr->y + eptr->font->max_bounds.ascent +
					eptr->font->descent;
			}
			else
			{
				ty2 = eptr->y + eptr->font->max_bounds.ascent +
					eptr->font->max_bounds.descent;
			}
			line = eptr->line_number;
			/*
			 * Searches on this line should extend to the top of
			 * the next line, if possible.  Which might be far
			 * away if there is an image on this line.
			 */
			if (((line + 1) < hw->html.line_count)&&
				(hw->html.line_array[line + 1] != NULL))
			{
				ty2 = hw->html.line_array[line + 1]->y - 1;
			}
			/*
			 * Else we are at the last line, and need to find its
			 * height.  The linefeed at the end should know the
			 * max height of the line.
			 */
			else
			{
				struct ele_rec *teptr;

				teptr = eptr;
				while (teptr != NULL)
				{
					if (teptr->type == E_LINEFEED)
					{
						break;
					}
					teptr = teptr->next;
				}
				if (teptr != NULL)
				{
					ty2 = teptr->y + teptr->line_height - 1;
				}
			}
		}

		if (eptr->type == E_TEXT)
		{
			int dir, ascent, descent;
			XCharStruct all;

			tx1 = eptr->x;
			XTextExtents(eptr->font, (char *)eptr->edata,
					eptr->edata_len - 1, &dir,
					&ascent, &descent, &all);
			tx2 = eptr->x + all.width;
			if ((x >= tx1)&&(x <= tx2)&&(y >= ty1)&&(y <= ty2))
			{
				rptr = eptr;
				break;
			}
		}
		else if ((eptr->type == E_IMAGE)&&(eptr->pic_data != NULL))
		{
			tx1 = eptr->x;
			tx2 = eptr->x + eptr->pic_data->width;
			if ((x >= tx1)&&(x <= tx2)&&(y >= ty1)&&(y <= ty2))
			{
				rptr = eptr;
				break;
			}
		}
		else if (eptr->type == E_LINEFEED)
		{
			tx1 = eptr->x;
			if ((x >= tx1)&&(y >= ty1)&&(y <= ty2))
			{
				rptr = eptr;
				break;
			}
			else if (eptr->next == NULL)
			{
				rptr = eptr;
				break;
			}
			else if (eptr->next != NULL)
			{
				int tmpy;

				tmpy = eptr->next->y + eptr->next->line_height;
				tx2 = eptr->next->x;
				if ((x < tx2)&&(y >= ty2)&&(y <= tmpy))
				{
					rptr = eptr;
					break;
				}
			}
		}
		eptr = eptr->next;
	}

	/*
	 * If we found an element, locate the exact character position within
	 * that element.
	 */
	if (rptr != NULL)
	{
		int dir, ascent, descent;
		XCharStruct all;
		int epos;

		/*
		 * Start assuming fixed width font.  The real position should
		 * always be <= to this, but just in case, start at the end
		 * of the string if it is not.
		 */
		epos = ((x - rptr->x) / rptr->font->max_bounds.width) + 1;
		if (epos >= rptr->edata_len - 1)
		{
			epos = rptr->edata_len - 2;
		}
		XTextExtents(rptr->font, (char *)rptr->edata,
				(epos + 1), &dir, &ascent, &descent, &all);
		if (x > (int)(rptr->x + all.width))
		{
			epos = rptr->edata_len - 3;
		}
		else
		{
			epos--;
		}

		while (epos >= 0)
		{
			XTextExtents(rptr->font, (char *)rptr->edata,
				(epos + 1), &dir, &ascent, &descent, &all);
			if ((int)(rptr->x + all.width) <= x)
			{
				break;
			}
			epos--;
		}
		epos++;
		*pos = epos;
	}
	return(rptr);
}


/*
 * Used by ParseTextToPrettyString to let it be sloppy about its
 * string creation, and never overflow the buffer.
 * It concatonates the passed string to the current string, managing
 * both the current string length, and the total buffer length.
 */
void
strcpy_or_grow(str, slen, blen, add)
	char **str;
	int *slen;
	int *blen;
	char *add;
{
	int newlen;
	int addlen;
	char *buf;

	/*
	 * If necessary, initialize this string buffer
	 */
	if (*str == NULL)
	{
		*str = (char *)malloc(1024 * sizeof(char));
		if (*str == NULL)
		{
			return;
		}
		*blen = 1024;
		strcpy(*str, "");
		*slen = 0;
	}

	buf = *str;
	if ((buf == NULL)||(add == NULL))
	{
		return;
	}

	addlen = strlen(add);

	newlen = *slen + addlen;
	if (newlen >= *blen)
	{
		newlen = ((newlen / 1024) + 1) * 1024;
		buf = (char *)malloc(newlen * sizeof(char));
		if (buf == NULL)
		{
			return;
		}
/*
		bcopy(*str, buf, *blen);
*/
		memcpy(buf, *str, *blen);
		free((char *)*str);
		*str = buf;
		*blen = newlen;
	}

/*
	bcopy(add, (char *)(buf + *slen), addlen + 1);
*/
	memcpy((char *)(buf + *slen), add, addlen + 1);

	*slen = *slen + addlen;
}


/*
 * Parse all the formatted text elements from start to end
 * into an ascii text string, and return it.
 * space_width and lmargin tell us how many spaces
 * to indent lines.
 */
char *
ParseTextToString(elist, startp, endp, start_pos, end_pos, space_width, lmargin)
	struct ele_rec *elist;
	struct ele_rec *startp;
	struct ele_rec *endp;
	int start_pos, end_pos;
	int space_width;
	int lmargin;
{
	int newline;
	int epos;
	char *text;
	int t_slen, t_blen;
	struct ele_rec *eptr;
	struct ele_rec *start;
	struct ele_rec *end;

	if (startp == NULL)
	{
		return(NULL);
	}

	if (SwapElements(startp, endp, start_pos, end_pos))
	{
		start = endp;
		end = startp;
		epos = start_pos;
		start_pos = end_pos;
		end_pos = epos;
	}
	else
	{
		start = startp;
		end = endp;
	}

	text = NULL;
	newline = 0;
	eptr = start;
	while ((eptr != NULL)&&(eptr != end))
	{
		/*
		 * Skip the special internal text
		 */
		if (eptr->internal == True)
		{
			eptr = eptr->next;
			continue;
		}

		if (eptr->type == E_TEXT)
		{
			int i, spaces;
			char *tptr;

			if (eptr == start)
			{
				tptr = (char *)(eptr->edata + start_pos);
			}
			else
			{
				tptr = (char *)eptr->edata;
			}

			if (newline)
			{
				spaces = (eptr->x - lmargin) / space_width;
				if (spaces < 0)
				{
					spaces = 0;
				}
				for (i=0; i<spaces; i++)
				{
					strcpy_or_grow(&text, &t_slen, &t_blen,
						" ");
				}
			}
			strcpy_or_grow(&text, &t_slen, &t_blen, tptr);
			newline = 0;
		}
		else if (eptr->type == E_LINEFEED)
		{
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			newline = 1;
		}
		eptr = eptr->next;
	}
	if ((eptr != NULL)&&(eptr->internal == False))
	{
		if (eptr->type == E_TEXT)
		{
			int i, spaces;
			char *tptr;
			char *tend, tchar;

			if (eptr == start)
			{
				tptr = (char *)(eptr->edata + start_pos);
			}
			else
			{
				tptr = (char *)eptr->edata;
			}

			if (eptr == end)
			{
				tend = (char *)(eptr->edata + end_pos + 1);
				tchar = *tend;
				*tend = '\0';
			}

			if (newline)
			{
				spaces = (eptr->x - lmargin) / space_width;
				if (spaces < 0)
				{
					spaces = 0;
				}
				for (i=0; i<spaces; i++)
				{
					strcpy_or_grow(&text, &t_slen, &t_blen,
						" ");
				}
			}
			strcpy_or_grow(&text, &t_slen, &t_blen, tptr);
			newline = 0;

			if (eptr == end)
			{
				*tend = tchar;
			}
		}
		else if (eptr->type == E_LINEFEED)
		{
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			newline = 1;
		}
	}
	return(text);
}


/*
 * Parse all the formatted text elements from start to end
 * into an ascii text string, and return it.
 * Very like ParseTextToString() except the text is prettied up
 * to show headers and the like.
 * space_width and lmargin tell us how many spaces
 * to indent lines.
 */
char *
ParseTextToPrettyString(hw, elist, startp, endp, start_pos, end_pos,
		space_width, lmargin)
	HTMLWidget hw;
	struct ele_rec *elist;
	struct ele_rec *startp;
	struct ele_rec *endp;
	int start_pos, end_pos;
	int space_width;
	int lmargin;
{
	int line;
	int newline;
	int lead_spaces;
	int epos;
	char *text;
	int t_slen, t_blen;
	char *line_buf;
	int l_slen, l_blen;
	char lchar;
	struct ele_rec *eptr;
	struct ele_rec *start;
	struct ele_rec *end;
	struct ele_rec *last;

	if (startp == NULL)
	{
		return(NULL);
	}

	if (SwapElements(startp, endp, start_pos, end_pos))
	{
		start = endp;
		end = startp;
		epos = start_pos;
		start_pos = end_pos;
		end_pos = epos;
	}
	else
	{
		start = startp;
		end = endp;
	}

	text = NULL;
	line_buf = NULL;

	/*
	 * We need to know if we should consider the indentation or bullet
	 * that might be just before the first selected element to also be
	 * selected.  This current hack looks to see if they selected the
	 * Whole line, and assumes if they did, they also wanted the beginning.
	 *
	 * If we are at the beginning of the list, or the beginning of
	 * a line, or just behind a bullett, assume this is the start of
	 * a line that we may want to include the indent for.
	 */
	if ((start_pos == 0)&&
		((start->prev == NULL)||(start->prev->type == E_BULLET)||
		(start->prev->line_number != start->line_number)))
	{
		eptr = start;
		while ((eptr != NULL)&&(eptr != end)&&
			(eptr->type != E_LINEFEED))
		{
			eptr = eptr->next;
		}
		if ((eptr != NULL)&&(eptr->type == E_LINEFEED))
		{
			newline = 1;
			if ((start->prev != NULL)&&
				(start->prev->type == E_BULLET))
			{
				start = start->prev;
			}
		}
		else
		{
			newline = 0;
		}
	}
	else
	{
		newline = 0;
	}

	lead_spaces = 0;
	last = start;
	eptr = start;
	line = eptr->line_number;
	while ((eptr != NULL)&&(eptr != end))
	{
		/*
		 * Skip the special internal text
		 */
		if (eptr->internal == True)
		{
			eptr = eptr->next;
			continue;
		}

		if (eptr->type == E_BULLET)
		{
			int i, spaces;

			if (newline)
			{
				spaces = (eptr->x - lmargin) / space_width;
				spaces -= 2;
				if (spaces < 0)
				{
					spaces = 0;
				}
				lead_spaces = spaces;
				for (i=0; i<spaces; i++)
				{
					strcpy_or_grow(&line_buf,
						&l_slen, &l_blen, " ");
				}
			}
			newline = 0;

			strcpy_or_grow(&line_buf, &l_slen, &l_blen, "o ");
			lead_spaces += 2;
		}
		else if (eptr->type == E_TEXT)
		{
			int i, spaces;
			char *tptr;

			if (eptr == start)
			{
				tptr = (char *)(eptr->edata + start_pos);
			}
			else
			{
				tptr = (char *)eptr->edata;
			}

			if (newline)
			{
				spaces = (eptr->x - lmargin) / space_width;
				if (spaces < 0)
				{
					spaces = 0;
				}
				lead_spaces = spaces;
				for (i=0; i<spaces; i++)
				{
					strcpy_or_grow(&line_buf,
						&l_slen, &l_blen, " ");
				}
			}
			strcpy_or_grow(&line_buf, &l_slen, &l_blen, tptr);
			newline = 0;
		}
		else if (eptr->type == E_LINEFEED)
		{
			strcpy_or_grow(&text, &t_slen, &t_blen, line_buf);
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			newline = 1;
			lchar = '\0';
			if (eptr->font == hw->html.header1_font)
			{
				lchar = '*';
			}
			else if (eptr->font == hw->html.header2_font)
			{
				lchar = '=';
			}
			else if (eptr->font == hw->html.header3_font)
			{
				lchar = '+';
			}
			else if (eptr->font == hw->html.header4_font)
			{
				lchar = '-';
			}
			else if (eptr->font == hw->html.header5_font)
			{
				lchar = '~';
			}
			else if (eptr->font == hw->html.header6_font)
			{
				lchar = '.';
			}
			if (lchar != '\0')
			{
				char *ptr;
				int cnt;

				cnt = 0;
				ptr = line_buf;
				while ((ptr != NULL)&&(*ptr != '\0'))
				{
					cnt++;
					if (cnt > lead_spaces)
					{
						*ptr = lchar;
					}
					ptr++;
				}
				strcpy_or_grow(&text,&t_slen,&t_blen, line_buf);
				strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			}
			if (line_buf != NULL)
			{
				free(line_buf);
				line_buf = NULL;
			}
		}
		last = eptr;
		eptr = eptr->next;
	}
	if ((eptr != NULL)&&(eptr->internal == False))
	{
		if (eptr->type == E_BULLET)
		{
			int i, spaces;

			if (newline)
			{
				spaces = (eptr->x - lmargin) / space_width;
				spaces -= 2;
				if (spaces < 0)
				{
					spaces = 0;
				}
				lead_spaces = spaces;
				for (i=0; i<spaces; i++)
				{
					strcpy_or_grow(&line_buf,
						&l_slen, &l_blen, " ");
				}
			}
			newline = 0;

			strcpy_or_grow(&line_buf, &l_slen, &l_blen, "o ");
			lead_spaces += 2;
		}
		else if (eptr->type == E_TEXT)
		{
			int i, spaces;
			char *tptr;
			char *tend, tchar;

			if (eptr == start)
			{
				tptr = (char *)(eptr->edata + start_pos);
			}
			else
			{
				tptr = (char *)eptr->edata;
			}

			if (eptr == end)
			{
				tend = (char *)(eptr->edata + end_pos + 1);
				tchar = *tend;
				*tend = '\0';
			}

			if (newline)
			{
				spaces = (eptr->x - lmargin) / space_width;
				if (spaces < 0)
				{
					spaces = 0;
				}
				lead_spaces = spaces;
				for (i=0; i<spaces; i++)
				{
					strcpy_or_grow(&line_buf,
						&l_slen, &l_blen, " ");
				}
			}
			strcpy_or_grow(&line_buf, &l_slen, &l_blen, tptr);
			newline = 0;

			if (eptr == end)
			{
				*tend = tchar;
			}
		}
		else if (eptr->type == E_LINEFEED)
		{
			strcpy_or_grow(&text, &t_slen, &t_blen, line_buf);
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			newline = 1;
			lchar = '\0';
			if (eptr->font == hw->html.header1_font)
			{
				lchar = '*';
			}
			else if (eptr->font == hw->html.header2_font)
			{
				lchar = '=';
			}
			else if (eptr->font == hw->html.header3_font)
			{
				lchar = '+';
			}
			else if (eptr->font == hw->html.header4_font)
			{
				lchar = '-';
			}
			else if (eptr->font == hw->html.header5_font)
			{
				lchar = '~';
			}
			else if (eptr->font == hw->html.header6_font)
			{
				lchar = '.';
			}
			if (lchar != '\0')
			{
				char *ptr;
				int cnt;

				cnt = 0;
				ptr = line_buf;
				while ((ptr != NULL)&&(*ptr != '\0'))
				{
					cnt++;
					if (cnt > lead_spaces)
					{
						*ptr = lchar;
					}
					ptr++;
				}
				strcpy_or_grow(&text,&t_slen,&t_blen, line_buf);
				strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			}
			if (line_buf != NULL)
			{
				free(line_buf);
				line_buf = NULL;
			}
		}
		last = eptr;
	}
	if (line_buf != NULL)
	{
		strcpy_or_grow(&text, &t_slen, &t_blen, line_buf);
		lchar = '\0';
		if (last->font == hw->html.header1_font)
		{
			lchar = '*';
		}
		else if (last->font == hw->html.header2_font)
		{
			lchar = '=';
		}
		else if (last->font == hw->html.header3_font)
		{
			lchar = '+';
		}
		else if (last->font == hw->html.header4_font)
		{
			lchar = '-';
		}
		else if (last->font == hw->html.header5_font)
		{
			lchar = '~';
		}
		else if (last->font == hw->html.header6_font)
		{
			lchar = '.';
		}
		if (lchar != '\0')
		{
			char *ptr;
			int cnt;

			cnt = 0;
			ptr = line_buf;
			while ((ptr != NULL)&&(*ptr != '\0'))
			{
				cnt++;
				if (cnt > lead_spaces)
				{
					*ptr = lchar;
				}
				ptr++;
			}
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			strcpy_or_grow(&text, &t_slen, &t_blen, line_buf);
		}
	}
	if (line_buf != NULL)
	{
		free(line_buf);
		line_buf = NULL;
	}
	return(text);
}


/*
 * Find the preferred width of a parsed HTML document
 * Currently unformatted plain text, unformatted listing text, plain files
 * and preformatted text require special width.
 * Preferred width = (width of longest plain text line in document) *
 * 	(width of that text's font)
 */
int
DocumentWidth(hw, list)
	HTMLWidget hw;
	struct mark_up *list;
{
	struct mark_up *mptr;
	int plain_text;
	int listing_text;
	int pcnt, lcnt, pwidth, lwidth;
	int width;
	char *ptr;

	/*
	 * Loop through object list looking at the plain, preformatted,
	 * and listing text
	 */
	width = 0;
	pwidth = 0;
	lwidth = 0;
	plain_text = 0;
	listing_text = 0;
	mptr = list;
	while (mptr != NULL)
	{
		/*
		 * All text blocks between the starting and ending
		 * plain and pre text markers are plain text blocks.
		 * Manipulate flags so we recognize these blocks.
		 */
		if ((mptr->type == M_PLAIN_TEXT)||
			(mptr->type == M_PLAIN_FILE)||
			(mptr->type == M_PREFORMAT))
		{
			if (mptr->is_end)
			{
				plain_text--;
				if (plain_text < 0)
				{
					plain_text = 0;
				}
			}
			else
			{
				plain_text++;
			}
			pcnt = 0;
			lcnt = 0;
		}
		/*
		 * All text blocks between the starting and ending
		 * listing markers are listing text blocks.
		 */
		else if (mptr->type == M_LISTING_TEXT)
		{
			if (mptr->is_end)
			{
				listing_text--;
				if (listing_text < 0)
				{
					listing_text = 0;
				}
			}
			else
			{
				listing_text++;
			}
			lcnt = 0;
			pcnt = 0;
		}
		/*
		 * If this is a plain text block, add to line length.
		 * Find the Max of all line lengths.
		 */
		else if ((plain_text)&&(mptr->type == M_NONE))
		{
			ptr = mptr->text;
			while ((ptr != NULL)&&(*ptr != '\0'))
			{
				ptr = MaxTextWidth(ptr, &pcnt);
				if (pcnt > pwidth)
				{
					pwidth = pcnt;
				}
			}
		}
		/*
		 * If this is a listing text block, add to line length.
		 * Find the Max of all line lengths.
		 */
		else if ((listing_text)&&(mptr->type == M_NONE))
		{
			ptr = mptr->text;
			while ((ptr != NULL)&&(*ptr != '\0'))
			{
				ptr = MaxTextWidth(ptr, &lcnt);
				if (lcnt > lwidth)
				{
					lwidth = lcnt;
				}
			}
		}
		mptr = mptr->next;
	}
	width = pwidth * hw->html.plain_font->max_bounds.width;
	lwidth = lwidth * hw->html.listing_font->max_bounds.width;
	if (lwidth > width)
	{
		width = lwidth;
	}
	return(width);
}

