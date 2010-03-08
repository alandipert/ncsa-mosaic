/* HTML-PSformat.c -  Module for NCSA's Mosaic software
 *
 * Purpose:	to parse Hypertext widget contents into appropriate PostScript
 *
 * Author:	Ameet A. Raval & Frans van Hoesel & Andrew Ford
 *		(aar@gfdl.gov & hoesel@chem.rug.nl). 
 *		send bugreports to hoesel@chem.rug.nl
 *
 * Institution: for Ameet A. Raval:
 *			Geophysical Fluid Dynamics Laboratory,
 *			National Oceanic and Atmospheric Administration,
 *			U.S. Department of Commerce
 *			P.O. Box 308
 *			Princeton, NJ 08542
 *		for Frans van Hoesel:
 *			Xtreme graphics software
 *			Herepoortenmolendrift 36
 *			9711 DH  Groningen
 *			The Netherlands
 *		also:
 *			Andrew Ford
 *			Independent Software Consultant
 *			30 Upper Cheltenham Place,
 *			Montpelier, Bristol, BS6 5HR, GB
 *			E-mail: andrew@icarus.demon.co.uk
 *
 * Date:		1 aug 1993 
 * Modification:	8 nov 1993
 *				o added support for bold/italics courier
 *				o removed unused or no longer needed stuff
 *				o fixed the font alignment problem
 *		 	23 nov 1993
 *				o added support for horizontal ruler
 *				o on request of Ameet, added a specific
 *					line about whome to send bugreports to
 *			15 jun 1994
 *				o add headers, footers and footnotes to convey
 *				  title, page number, url of document, date (of
 *				  printing) and urls of links.
 *				o use A4 or US Letter size paper (currently hard coded)
 *
 *			9 may 1995 (Andrew Ford)
 *				o general overhaul
 *
 * Copyright:   This work is the product of the United States Government,
 *		and is precluded from copyright protection.  It is hereby
 *		released into the public domain.
 *
 * WE MAKE NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED
 * WARRANTY. WE SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE
 * USERS OF THIS SOFTWARE. 
 * 
 *		pieces of code are taken from xvps by kind
 *		permission of John Bradley.
 *
 * To-do:
 *		* printing of tables and forms fields (and any new features that appear)
 *
 */
#include "../config.h"
#include <varargs.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#ifdef __bsdi__
#include <sys/malloc.h>
#else
#ifndef VAXC
#include <malloc.h>
#endif
#endif
#include <time.h>
#include <sys/types.h>
#include "HTMLP.h"

/* Fix thanks to robm. */
#ifdef __alpha
#include <Xm/VaSimpleP.h>
#endif

#define CR '\015'
#define LF '\012'

#ifndef DISABLE_TRACE
extern int htmlwTrace;
#endif

extern int SwapElements();

#define USLETTER 	0
#define A4 		1

#if !defined(DEFAULT_PAGE_SIZE)
#define DEFAULT_PAGE_SIZE USLETTER
#endif

#define F_FULLCOLOR	0
#define F_GREYSCALE	1
#define F_BWDITHER 	2
#define F_REDUCED  	3

#define L_PAREN		'('
#define R_PAREN		')'
#define B_SLASH		'\\'
#define MAX_ASCII	'\177'


/* MONO returns total intensity of r,g,b components .33R+ .5G+ .17B */
#define MONO(rd,gn,bl) (((rd)*11 + (gn)*16 + (bl)*5) >> 13)

/* PSconst_out outputs to the postscript buffer an array of constant
 * strings
 */
#define PSconst_out(txt) {				\
	int n=(sizeof txt)/(sizeof txt[0]); 		\
	  	int i; 					\
	  	for (i=0; i<n; i++) { 			\
			PSprintf("%s\n", txt[i]) ; 	\
		} 					\
}
	
/* STREQ tests whether two strings are equal. */
#define STREQ(a, b)	(strcmp((a), (b)) == 0)

/* for regular-font, bold-font, italic-font, fixed-font */
typedef enum { RF, BF, IF, FF, FB, FI } PS_fontstyle;

static int PS_size, PS_len, PS_offset, PS_curr_page, PS_start_y, PS_hexi;
static int PS_page_offset;
static char *PS_string;
static float Points_Pixel;
static int Pixels_Page;
static PS_fontstyle PS_oldfn = RF;
static int PS_fontascent = 0;
static int PS_oldfs = 0;

static XColor fg_color, bg_color;


static int footnote_space  = 8;		/* Space from main text to footnotes */
static int footnote_ptsize = 8;		/* Point size for footnote text */
static int cur_ftn_no;			/* Current footnote number */
static int n_saved_ftns;		/* Number of saved footnotes on page */
static int ftn_array_size=0;		/* Size of allocated footnote array */
static char **footnotes=NULL;		/* Pointer to array of footnote pointers */

typedef struct {
    double	page_height;
    double	page_width;
    double	top_margin;
    double	bot_margin;
    double	left_margin;
    double	right_margin;
    double	text_height;
    double	text_width;
}
PAGE_DIMENS_T;

#define INCH	72
#define MM	INCH / 25.4

PAGE_DIMENS_T	page_dimens;
PAGE_DIMENS_T	a4_page_dimens = {
    297 * MM,
    210 * MM, 
     20 * MM,
     20 * MM,
     20 * MM,
     20 * MM
};

PAGE_DIMENS_T	us_letter_page_dimens = {
    11  * INCH,		/* page_height */
    8.5 * INCH,		/* page_width  */
    0.9 * INCH,		/* top_margin  */
    0.7 * INCH,
    0.9 * INCH,
    0.9 * INCH
};


/* Globals to get button value in print dialog -- swp/AF */

int HTML_Print_Headers       = 1;	/* Flag whether page headers enabled */
int HTML_Print_Footers     = 1;	/* Flag whether footnote printing enabled */

/* Paper format (currently either A4 or letter).  This should be generalized. */

int HTML_Print_Paper_Size_A4 = DEFAULT_PAGE_SIZE;

extern int installed_colormap;
extern Colormap installed_cmap;

/*
 * GetDpi - return Dots-per-inch of the screen
 *
 * calculate the pixel density in dots per inch on the current widget
 * screen
 *
 */
static float 
GetDpi(HTMLWidget hw)
{
    Screen *s = XtScreen(hw);
    float dpi;

    dpi = 25.4 * WidthOfScreen(s) / WidthMMOfScreen(s);
    if (dpi<1.0 || dpi>10000.0)
	dpi = 72.0;
    return dpi;
}


/*
 * PSprintf - dynamic string concatenation function.
 *
 *  In successive calls, the formatted string will be appended to the global
 *  output string Sp.
 *  It assumes that on each call, the length of the text appended to Sp
 *  is less than 1024.
 *  The format string is used as in printf, so you can use additional
 *  arguments.
 *
 *  When successful, PSprintf returns the number of characters printed
 *  in this call, otherwise it returns EOF (just as printf does)
 *
 */
static int 
PSprintf(format, va_alist)
    char* format;
    va_dcl
{
    int 	len;
    char 	*s;
    va_list	args;

    if (PS_size - PS_len < 1024) 
    {
	PS_size += 1024;
	if ((s = (char *) realloc(PS_string, PS_size)) == NULL) 
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "PSprintf malloc failed\n");
		}
#endif
		return(EOF);
	}
	PS_string = s;
    }
    va_start(args);
    len = vsprintf(PS_string+PS_len, format, args);
    /* this is a hack to make it work on systems were vsprintf(s,...)
     * returns s, instead of the len.
     */
    if (len != EOF && len != 0) 
	PS_len += strlen(PS_string+PS_len);
    va_end(args);
    return(len);
}


/*
 * PShex - output hex byte
 *
 * Append the byte "val" to an internal string buffer in hexadecimal
 * format.  If the argument "flush" is True, or if the buffer has filled
 * up, flush the buffer to the larger postscript output buffer (using
 * PSprintf).
 *
 */
static int 
PShex(unsigned char val, int flush)
{
    static unsigned char hexline[80];
    static char digit[] = "0123456789abcdef";

    if (!flush) 
    {
	hexline[PS_hexi++] = (char) digit[((unsigned) val >>
					   (unsigned) 4) & (unsigned) 0x0f];
	hexline[PS_hexi++] = (char) digit[(unsigned) val &
					  (unsigned) 0x0f];
    }

    /* Changed from ">78" to ">77" on advice of
       debra@info.win.tue.nl (Paul De Bra). */

    if ((flush && PS_hexi) || (PS_hexi>77)) 
    {
	hexline[PS_hexi] = '\0';
	PS_hexi=0;
	return (PSprintf("%s\n", hexline));
    }
    return (0);
}


/*
 * PSfont - change font
 *
 * change local font in buf to "font"
 * fontfamily indicates if the overall style is times, helvetica, century
 * schoolbook or lucida.
 *
 */
static void
PSfont(HTMLWidget hw, XFontStruct *font, int fontfamily) 
{
    PS_fontstyle fn;
    int style, size;
    int fs;

    static PS_fontstyle fontstyle[17] = 
    {
	RF, IF, BF, FF, BF, BF, BF, BF, BF,
	BF, IF, FF, FF, FB, FI, FB, FI
    };

    static char fnchar[6][3] = {"RF", "BF", "IF", "FF", "FB", "FI"};

    /* fontsizes as set in gui.c and in HTML.c (listing font is only
     * defined in HTML.c)
     */
    static int fontsizes[4][3][17] = 
    {
	/* times font sizes */
	{
	    {14, 14, 14, 14, 18, 17, 14, 12, 10, 8, 14, 12, 12, 14, 14, 12, 12},
	    {17, 17, 17, 17, 24, 18, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14},
	    {20, 20, 20, 20, 25, 24, 20, 18, 17, 14, 20, 18, 12, 20, 20, 18, 18}
	},
	/* helvetica sizes */
	{
	    {14, 14, 14, 14, 18, 17, 14, 12, 10, 8, 14, 12, 12, 14, 14, 12, 12},
	    {17, 17, 17, 17, 24, 18, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14},
	    {20, 20, 20, 20, 25, 24, 20, 18, 17, 14, 20, 18, 12, 20, 20, 18, 18}
	},
	/* new century schoolbook sizes */
	{
	    {14, 14, 14, 14, 18, 17, 14, 12, 10, 8, 14, 12, 12, 14, 14, 12, 12},
	    {18, 18, 18, 18, 24, 18, 17, 14, 12, 10, 18, 14, 12, 18, 18, 14, 14},
	    {20, 20, 20, 20, 25, 24, 20, 18, 17, 14, 20, 18, 12, 20, 20, 18, 18}
	},
	/* lucida sizes */
	{
	    {14, 14, 14, 14, 18, 17, 14, 12, 11, 10, 14, 12, 12, 14, 14, 12, 12},
	    {17, 17, 17, 17, 24, 18, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14},
	    {20, 20, 20, 20, 25, 24, 20, 18, 17, 14, 20, 18, 12, 20, 20, 18, 18}
	}
    };

    /* next is for each fontfamily the ascent value as given by the 
     * medium sized bold x-font (the regular font has the same
     * ascent value for both the medium and the large size Century
     * font).
     * it is use in the check for the fontsize (small, medium, large)
     */
    static int medium_fontascent[4] = {
	14, 14, 16, 15
    };

    /* for each fontfamily, for each fontsize, and for each font style
     * give the ascent value, so the output from Postscript is correct.
     * If the value is given between parenthesis, then it is different
     * from the value as stored in the x-font.
     * Note that this is a fix, and need to be changed, if the browser
     * is fixed (in the current version 1.2 the baseline of various fonts
     * is not aligned very well).
     */
    static int fontascent[4][3][17] = {
	/*rg, itl, bld, fix,  h1,  h2,  h3,  h4,  h5,  h6,
	  add, pla, lis, fixbold, fixital, plabold, plaital, */
	/* times */
	{
	    {12, 11, 12, 10, 15, 14, 12, 10, 8, 6, 11, 9, 10, 10, 10, 9, 9},
	    {13, 13, 14, 12, 20, 15, 14, 12, 10, 8, 13, 10, 10, 12, 12, 10, 10},
	    {16, 15, 15, 13, 21, 20, 15, 15, 14, 12, 15, 13, 10, 13, 13, 13, 13}
	},
	/* helvetica */
	{
	    {12, 12, 12, 10, 15, 14, 12, 10, 9, 7, 12, 9, 10, 10, 10, 9, 9},
	    {14, 14, 14, 12, 22, 15, 14, 12, 10, 9, 14, 10, 10, 12, 12, 10, 10},
	    {16, 16, 16, 13, 22, 22, 16, 15, 14, 12, 16, 13, 10, 13, 13, 13, 13}
	},
	/* new century schoolbook */
	{
	    {12, 12, 13, 10, 16, 14, 13, 10, 9, 7, 12, 9, 10, 10, 10, 9, 9},
	    {16, 14, 16, 13, 22, 16, 14, 13, 10, 9, 14, 10, 10, 13, 13, 10, 10},
	    {17, 16, 17, 13, 22, 22, 17, 16, 14, 13, 16, 13, 10, 13, 13, 13, 13}
	},
	/* lucida bright */
	{
	    {11, 11, 11, 11, 15, 14, 11, 10, 9, 7, 11, 9, 10, 11, 10, 9, 9},
	    {14, 15, 14, 13, 20, 15, 14, 11, 10, 7, 15, 11, 10, 13, 13, 11, 10},
	    {17, 17, 17, 16, 21, 20, 17, 15, 14, 11, 17, 14, 10, 16, 13, 14, 13}
	}
    };

    /* NULL case - reflush old font or the builtin default: */
    if ((hw == NULL) || (font == NULL))
    {
	if (PS_oldfs != 0)
	    PSprintf( "%2s %d SF\n", fnchar[PS_oldfn], PS_oldfs);
	return;
    }
    /* added the next line in case xmosaic version 199.4 has more fonts */
    style = 3;

    if (font == hw->html.font) {
	style = 0;
    } else if (font == hw->html.italic_font) {
	style = 1;
    } else if (font == hw->html.bold_font) {
	style = 2;
    } else if (font == hw->html.fixed_font) {
	style = 3;
    } else if (font == hw->html.header1_font) {
	style = 4;
    } else if (font == hw->html.header2_font) {
	style = 5;
    } else if (font == hw->html.header3_font) {
	style = 6;
    } else if (font == hw->html.header4_font) {
	style = 7;
    } else if (font == hw->html.header5_font) {
	style = 8;
    } else if (font == hw->html.header6_font) {
	style = 9;
    } else if (font == hw->html.address_font) {
	style = 10;
    } else if (font == hw->html.plain_font) {
	style = 11;
    } else if (font == hw->html.listing_font) {
	style = 12;
    } else if (font == hw->html.fixedbold_font) {
	style = 13;
    } else if (font == hw->html.fixeditalic_font) {
	style = 14;
    } else if (font == hw->html.plainbold_font) {
	style = 15;
    } else if (font == hw->html.plainitalic_font) {
	style = 16;
    }

    /* check size, by looking at the size of the regular font */
    size = 1;
    if (hw->html.bold_font->ascent > medium_fontascent[fontfamily]) 
    {
	/* large font */
	size = 2;
    }
    else if (hw->html.bold_font->ascent < medium_fontascent[fontfamily]) 
    {
	/* small font */
	size = 0;
    }
    fn = fontstyle[style];
    fs = fontsizes[fontfamily][size][style];
    PS_fontascent = fontascent[fontfamily][size][style];

    if (fn != PS_oldfn || fs != PS_oldfs) 
    {
	PSprintf( "%2s %d SF\n", fnchar[fn], fs);
	PS_oldfn=fn, PS_oldfs=fs;
    }
}


/*
 * PSshowpage - end of page function
 *
 * show the current page and restore any changes to the printer state.
 * Any accumulated footnotes are output and the outstanding footnote count
 * reset to zero.  Footnotes are preceded by a footnote rule and each footnote
 * is consists of a raised mark and the footnote text (i.e. the url).  The mark
 * is in a smaller font than the text.  The ideas are filched from LaTeX. 
 */
static void
PSshowpage(void)
{
    PSprintf("restore\n");
    if (n_saved_ftns > 0) 
    {
	int	i;

	PSprintf("gsave 0.2 setlinewidth newpath %.2f %.2f M %.2f 0 RL stroke\n", 
		 page_dimens.left_margin,
		 (page_dimens.bot_margin + (footnote_ptsize * n_saved_ftns) + 4),
		 (page_dimens.text_width * 0.4));
	for (i = 0; n_saved_ftns; n_saved_ftns--, i++) 
	{
	    PSprintf("newpath %.2f %.2f M RF %.2f SF (%d) S 3 -2 R RF %d SF (%s) S\n", 
		     page_dimens.left_margin,
		     page_dimens.bot_margin + 5 + (n_saved_ftns - 1) * footnote_ptsize,
		     (0.7 * footnote_ptsize), cur_ftn_no - n_saved_ftns, 
		     footnote_ptsize, footnotes[i]);
	}
	PSprintf("grestore\n");
    }
    PSprintf("showpage\n");
}



/*
 * PSnewpage - begin a fresh page
 *
 * increment the page count and handle the structured comment
 * conventions
 *
 */
static void
PSnewpage(void) 
{
    PS_curr_page++;
    
    /* the PostScript reference Manual states that the Page: Tag
       should have a label and a ordinal; otherwise programs like
       psutils fail    -gustaf */

    PSprintf("%%%%Page: %d %d\n", PS_curr_page, PS_curr_page);
    PSprintf("save\n");
    if (HTML_Print_Headers)
	PSprintf("%d ", PS_curr_page);
    PSprintf("NP\n");
    PSfont( NULL, NULL, 0);	/* force re-flush of last font used */
}



/*
 * PSinit_latin1 - handle ISO encoding
 *
 * print out initializing PostScript text for ISO Latin1 font encoding
 * This code is copied from the Idraw program (from Stanford's InterViews 
 * package), courtesy of Steinar Kjaernsr|d, steinar@ifi.uio.no
 *
 */
static void
PSinit_latin1(void)
{

    static char *txt[] = {

	"/reencodeISO {",
	"dup dup findfont dup length dict begin",
	"{ 1 index /FID ne { def }{ pop pop } ifelse } forall",
	"/Encoding ISOLatin1Encoding D",
	"currentdict end definefont",
	"} D",
	"/ISOLatin1Encoding [",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/space/exclam/quotedbl/numbersign/dollar/percent/ampersand/quoteright",
	"/parenleft/parenright/asterisk/plus/comma/minus/period/slash",
	"/zero/one/two/three/four/five/six/seven/eight/nine/colon/semicolon",
	"/less/equal/greater/question/at/A/B/C/D/E/F/G/H/I/J/K/L/M/N",
	"/O/P/Q/R/S/T/U/V/W/X/Y/Z/bracketleft/backslash/bracketright",
	"/asciicircum/underscore/quoteleft/a/b/c/d/e/f/g/h/i/j/k/l/m",
	"/n/o/p/q/r/s/t/u/v/w/x/y/z/braceleft/bar/braceright/asciitilde",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/dotlessi/grave/acute/circumflex/tilde/macron/breve",
	"/dotaccent/dieresis/.notdef/ring/cedilla/.notdef/hungarumlaut",
	"/ogonek/caron/space/exclamdown/cent/sterling/currency/yen/brokenbar",
	"/section/dieresis/copyright/ordfeminine/guillemotleft/logicalnot",
	"/hyphen/registered/macron/degree/plusminus/twosuperior/threesuperior",
	"/acute/mu/paragraph/periodcentered/cedilla/onesuperior/ordmasculine",
	"/guillemotright/onequarter/onehalf/threequarters/questiondown",
	"/Agrave/Aacute/Acircumflex/Atilde/Adieresis/Aring/AE/Ccedilla",
	"/Egrave/Eacute/Ecircumflex/Edieresis/Igrave/Iacute/Icircumflex",
	"/Idieresis/Eth/Ntilde/Ograve/Oacute/Ocircumflex/Otilde/Odieresis",
	"/multiply/Oslash/Ugrave/Uacute/Ucircumflex/Udieresis/Yacute",
	"/Thorn/germandbls/agrave/aacute/acircumflex/atilde/adieresis",
	"/aring/ae/ccedilla/egrave/eacute/ecircumflex/edieresis/igrave",
	"/iacute/icircumflex/idieresis/eth/ntilde/ograve/oacute/ocircumflex",
	"/otilde/odieresis/divide/oslash/ugrave/uacute/ucircumflex/udieresis",
	"/yacute/thorn/ydieresis",
	"] D",
	"[RF BF IF FF FB FI] {reencodeISO D} forall"
    };

    PSconst_out(txt);
}


/*
 * PSinit - initialize Postscript output
 *
 * does the initialization per html document
 *
 */
static void
PSinit(void) 
{
    PS_size = PS_len = PS_offset = PS_hexi = PS_page_offset = 0;
    PS_start_y = 0;
    PS_string = (char *) malloc(1);
    PS_oldfs = 0;
    PS_oldfn = RF;
    PS_curr_page = 0 ;
    n_saved_ftns = 0;
    cur_ftn_no = 1;
}



/*
 * PSheader - initialize Postscript output
 *
 * Prints out the prolog.  The following PostScript macros are defined
 *	D	def - define a macro
 *	E	exch - exhange parameters
 *	M	moveto
 *	R	rmoveto
 *	L	lineto
 *	RL	rlineto
 *	SQ	draw a unit square
 *	U	underline a string
 *	B	draw a bullet
 *	OB	draw an open bullet
 *	HR	draw a horizontal rule
 *	SF	set font
 *	RF	roman font (dependent on font family)
 *	BF	bold font (dependent on font family)
 *	IF	italic font (dependent on font family)
 *	FF	fixed font (courier)
 *	FB	fixed bold font (courier bold)
 *	FI	fixed italic font (courier oblique)
 *	nstr	buffer for creating page number string
 *	pageno	literal "Page "
 *	url	URL of document
 *	title	title of document
 *	date	date modified/printed
 */
static void 
PSheader(char *title, int font, char *url, char *time_str) 
{
    static char *notitle="Untitled";
    int set_to_null=0;
    static char *fontname[] = {
	/* in order: regular, bold, italic */
	"Times-Roman", "Times-Bold", "Times-Italic",
	"Helvetica", "Helvetica-Bold", "Helvetica-Oblique",
	"NewCenturySchlbk-Roman", "NewCenturySchlbk-Bold",
	"NewCenturySchlbk-Italic",
	/* this is a nasty trick, I have put Times in place of
	 * Lucida, because most printers don't have Lucida font
	 */
	"Times-Roman", "Times-Bold", "Times-Italic"
	    /* "Lucida", "Lucida-Bold", "Lucida-Italic" */
    };

    static char *txt[] = {
	"/M {moveto} D",
	"/S {show} D",
	"/R {rmoveto} D",
	"/L {lineto} D",
	"/RL {rlineto} D",
	"/SQ {newpath 0 0 M 0 1 L 1 1 L 1 0 L closepath} D",
	"/U {gsave currentpoint currentfont /FontInfo get /UnderlinePosition get",
	" 0 E currentfont /FontMatrix get dtransform E pop add newpath moveto",
	" dup stringwidth rlineto stroke grestore S } D",
	"/B {/r E D gsave -13 0  R currentpoint ",
	"  newpath r 0 360 arc closepath fill grestore } D",
	"/OB {/r E D gsave -13 0  R currentpoint ",
	"  newpath r 0 360 arc closepath stroke grestore } D",
	"/HR {/l E D gsave 0 1 RL l 0 RL 0 -1 RL l neg 0 RL stroke grestore } D",
	"/SF {E findfont E scalefont setfont } D",
	"/FF {/Courier } D",
	"/FB {/Courier-Bold } D",
	"/FI {/Courier-Oblique } D"
	};

    char		time_buf[40];
    time_t		clock = time(NULL);

#if !defined(VMS) || defined (__DECC)
    strftime(time_buf, sizeof(time_buf), 
	     "Printed %a %b %e %T %Y", localtime(&clock));
#else
    sprintf(time_buf,"Printed %s",asctime(localtime(&clock)));
#endif

/* Always show the print date -- SWP */
    time_str = time_buf;

/*
    if (!time_str || (time_str[0] == '\0'))
    {
	time_str = time_buf;
    }
*/
    PSprintf("%%!PS-Adobe-1.0\n");
    PSprintf("%%%%Creator: NCSA Mosaic, Postscript by Ameet Raval, Frans van Hoesel\n");
    PSprintf("%%%%         and Andrew Ford\n");

    if (!title) {
	title=notitle;
	set_to_null=1;
    }

    {
	char *tmp;
	for (tmp = title; *tmp; tmp++)
	    if (*tmp == CR || *tmp == LF)
                *tmp = ' ';
	PSprintf("%%%%Title: %s\n", title);
    }

    PSprintf("%%%%CreationDate: %s\n", time_buf + 8);
    PSprintf("%%%%Pages: (atend)\n");
    PSprintf("%%%%PageOrder: Ascend\n");
    PSprintf("%%%%BoundingBox: %d %d %d %d\n",
	     (int)page_dimens.left_margin,
	     (int)(page_dimens.bot_margin - 12),
	     (int)(page_dimens.left_margin + page_dimens.text_width + 0.5),
	     (int)(page_dimens.bot_margin + page_dimens.text_height + 12.5));
    PSprintf("%%%%DocumentFonts: %s %s %s Courier Courier-Bold Courier-Oblique\n",
	     fontname[font*3], fontname[font*3+1], fontname[font*3+2]);
    PSprintf("%%%%EndComments\n");
    PSprintf("save /D {def} def /E {exch} D\n");
    PSprintf("/RF {/%s} D\n", fontname[font*3]);
    PSprintf("/BF {/%s} D\n", fontname[font*3+1]);
    PSprintf("/IF {/%s} D\n", fontname[font*3+2]);
    PSprintf("/nstr 6 string D /pgno (Page ) D\n");
    PSprintf("/url (%s) D\n", url);
    PSprintf("/title (%s) D\n", title);
    PSprintf("/date (%s) D\n", time_str);
    PSconst_out(txt);
    
    /* Output the newpage definition. */
    
    PSprintf("/NP {");
    if (HTML_Print_Headers) 
    {
	PSprintf("gsave 0.4 setlinewidth\n");
	PSprintf("  newpath %.2f %.2f M %.2f 0 RL stroke", 
		 page_dimens.left_margin,
		 (page_dimens.bot_margin + page_dimens.text_height), 
		 page_dimens.text_width);
	PSprintf("  newpath %.2f %.2f M %.2f 0 RL stroke\n",
		 page_dimens.left_margin, page_dimens.bot_margin,
		 page_dimens.text_width);
	PSprintf("  BF 12 SF %.2f %.2f M (%s) S\n",
		 page_dimens.left_margin,
		 (page_dimens.bot_margin + page_dimens.text_height + 6), title);
	PSprintf("  nstr cvs dup stringwidth pop pgno stringwidth pop add\n");
	PSprintf("  %.2f E sub %.2f M pgno S S\n",
		 (page_dimens.left_margin + page_dimens.text_width), 
		 (page_dimens.bot_margin + page_dimens.text_height + 6));
	PSprintf("  BF 10 SF %.2f %.2f M (%s) S\n",
		 page_dimens.left_margin, page_dimens.bot_margin - 12, url);
	PSprintf("  (%s) dup stringwidth pop %.2f E sub %.2f M S grestore\n",
		 time_str, page_dimens.left_margin + page_dimens.text_width,
		 page_dimens.bot_margin - 12);
    }
    PSprintf("  %.2f %.2f translate %.5f %.5f scale } D\n",
	     page_dimens.left_margin, 
	     page_dimens.bot_margin + page_dimens.text_height,
	     Points_Pixel, Points_Pixel);
    PSinit_latin1();
    
    PSprintf("%%%%EndProlog\n");

    if (set_to_null) {
	title=NULL;
    }
}



/*
 * PStrailer - write postscript trailer
 *
 */
static void
PStrailer(void)
{

    PSprintf("%%%%Trailer\n");
    PSprintf("restore\n");
    PSprintf("%%%%Pages: %d\n", PS_curr_page);
    if (footnotes)
    {
	free(footnotes);
	footnotes = NULL;
	ftn_array_size = 0;
    }
}


/*
 * PSmoveto - move to new x,y location
 *
 * if the Y value does not fit on the current page, begin a new page
 * (I think in the current implementation, this never happens)
 *
 */
static void
PSmoveto(int x, int y) 
{
    if (y > PS_start_y + Pixels_Page) 
    {
	PS_start_y = y;
	PSshowpage();
	PSnewpage();
    }
    PS_offset = 0;
    PSprintf( "%d %d M\n", x, -(y - PS_start_y));
}


/*
 * PSmove_offset - set Y-offset
 *
 * do a relative vertical move, whenever the offset changes
 *
 */
static void
PSmove_offset(int offset)
{
    if (offset != PS_offset) 
    {
	PSprintf("0 %d R\n", PS_offset - offset );
	PS_offset = offset;
    }
}


/*
 * Return an indication of whether or not the current element has a footnote.
 *
 * an element has a footnote if it is text or an image and its anchorHRef is not null.
 * If the element is a textual element with an anchorHRef, that has been split across 
 * lines then it should be followed by a linefeed element and a text element with the
 * same anchorHRef.  In this case say that the element doesn't have a footnote so as
 * to avoid duplicate footnotes.
 */
static int 
has_footnote(struct ele_rec *el)
{
    int			rc = 0;
    struct ele_rec 	*next;
    char		*anchorHRef;

    if (el == NULL) {
	return 0;
    }

    anchorHRef = el->anchorHRef;

    if (anchorHRef != NULL)
    {
	switch (el->type)
	{
	case E_TEXT:
	case E_IMAGE:
	    for (next = el->next; el; el = next, next = el->next)
	    {
		if (next == NULL)
		{
		    rc = 1;
		    break;
		}
		else if (next->anchorHRef == NULL) {
			rc = 1;
			break;
		}
		else if (!STREQ(next->anchorHRef, anchorHRef))
		{
		    rc = 1;
		    break;
		}
		else if (   (next->type == E_TEXT)
			 || (next->type == E_IMAGE))
		{
		    rc = 0;
		    break;
		}
	    }
	    break;

	default:
	    break;
	}
    }
    return rc;
}


/*
 * PSfootnote - output a footnote mark and store the footnote
 *
 * the footnote mark is placed at the current point, enclosed in a gsave/grestore
 * pair so that the position of the following output is not affected.
 * The reference is stored in a malloced array (which may need to be expanded), to
 * be output at the end of the page.
 */
static void
PSfootnote(char *href, double height)
{
    PSprintf("gsave 0 %.2f R RF %d SF (%d) S grestore\n", 
	     height, footnote_ptsize, cur_ftn_no++);

    if (n_saved_ftns == ftn_array_size) 
    {
	ftn_array_size += 16;
	if (!footnotes) {
		footnotes = (char **)calloc(ftn_array_size,sizeof(char *));
	}
	else {
		footnotes = (char **)realloc((void *)footnotes,
				     (ftn_array_size * sizeof(char *)));
	}
	if (footnotes == NULL) 
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "PSfootnote realloc failed\n");
		}
#endif

		return;
	}
    }

    footnotes[n_saved_ftns++] = href;
}


/*
 * PStext - output text
 *
 * show text "t", and protect special characters if needed
 * if Underline is non-zero, the text is underlined.
 *
 */
static void
PStext(HTMLWidget hw, struct ele_rec *eptr, int fontfamily, String s)
{
    String 		s2;
    String 		stmp;
    unsigned char	ch;
    int 		underline = eptr->underline_number;
    int		 	ascent;
			  
    PSfont(hw, eptr->font, fontfamily);	/* set font */
    if (PS_fontascent == 0) 
	ascent = eptr->font->ascent;
    else
	ascent = PS_fontascent;
    PSmove_offset(eptr->y_offset + ascent);


    /* Allocate a string long enough to hold the original string with
       every character stored as an octal escape (worst case scenario). */

    s2 = (String) malloc(strlen(s) * 4 + 1);
    if (s2 == NULL) 
    {
#ifndef DISABLE_TRACE
	if (htmlwTrace) {
		fprintf(stderr, "PStext malloc failed\n");
	}
#endif

	return;
    }

    /*  For each char in s, if it is a special char, insert "\"
     *  into the new string s2, then insert the actual char
     */
    for (stmp = s2; (ch = *s++) != '\0';)
    {
	if ((ch == L_PAREN) || (ch == R_PAREN) || (ch == B_SLASH))
	{
	    *stmp++ = B_SLASH;
	    *stmp++ = ch;
	}
	else if (ch > (unsigned char) MAX_ASCII) 
	{
	    /*  convert to octal */
	    *stmp++ = B_SLASH;
	    *stmp++ = ((ch >> 6) & 007) + '0';
	    *stmp++ = ((ch >> 3) & 007) + '0';
	    *stmp++ = (ch & 007) + '0';
	} 
	else
	{
	    *stmp++ = ch;
	}
    }
    *(stmp) = '\0';
    PSprintf("(%s)%c\n", s2, (underline)?'U':'S');
    if (HTML_Print_Footers && has_footnote(eptr)) 
    {
	PSfootnote(eptr->anchorHRef, 0.7 * ascent);
    }
    free(s2);
}


/*
 * PSbullet - output a bullet
 *
 * The bullet is normally filled, except for a bullet with an indent level
 * of two. The size of the higher level bullets is just somewhat smaller
 *
 */
static void 
PSbullet(HTMLWidget hw, struct ele_rec *eptr, int fontfamily)
{
    int 	width  = (  eptr->font->max_bounds.lbearing 
		          + eptr->font->max_bounds.rbearing);
    int 	offset = eptr->y_offset + eptr->font->ascent;
    int		level  = eptr->indent_level;
    double 	size   = eptr->line_height / 5.5;

    if (size < 1.1) size = 1.1;
    if (level > 2)  size /= 1.33333;

    /* the next line is a hack to get a good position of the
     * bullet in most practical cases, otherwise the
     * bullet may appear just a bit too low (for large fonts)
     * What is does is to compare the lineheight with
     * the lineheight of the next element, to correct
     * for the possibly too large y_offset
     */

    if (   (eptr->next != NULL)
	&& (   (eptr->next->type == E_TEXT)
	    || (eptr->next->type == E_IMAGE)))
    {
	offset += (eptr->line_height - eptr->next->line_height);
    }

    PSfont(hw, eptr->font, fontfamily);
    PSmove_offset(offset - width/4);
    PSprintf(" %f %s\n", size, (level == 2) ? "OB" : "B");
}


/*
 * PShrule - draw a horizontal line with the given width
 *
 * nothing special, just draw a line, from the current position to
 * the right side of the paper.
 *
 */
static void
PShrule(HTMLWidget hw, struct ele_rec *eptr, int fontfamily)
{
    int length = hw->html.doc_width;

    PSmove_offset(eptr->y_offset);
    PSprintf("%d HR\n", length);
}




/*
 * PStable - draw a table
 *
 * Currently just draw a box of the dimensions of the table.
 *
 */
static void
PStable(HTMLWidget hw, struct ele_rec *eptr, int fontfamily)
{
    struct table_rec	*tptr = eptr->table_data;
    int width  = tptr->width;
    int height = tptr->height;

#if 0
    PSmove_offset(eptr->y_offset);
    PSprintf("gsave currentpoint %d sub translate ", height);
    PSprintf("%d %d scale\n", width, height);
    PSprintf("SQ stroke\n");
    PSprintf("grestore\n");
#endif
}


/*
 * PSwidget - draw a widget (form field)
 *
 * Currently just draw a grey box of the dimensions of the field.
 * This is nowhere near complete but is a first step.
 * The widget record type field gives the type of field:
 *	0	textfield
 *	1	checkbox
 *	2	radiobox
 *	3	pushbutton
 *	4	password
 *	5	option menu
 */
static void
PSwidget(HTMLWidget hw, struct ele_rec *eptr, int fontfamily)
{
    struct wid_rec *wptr = eptr->widget_data;
    int w = wptr->width;
    int h = wptr->height;

#if 1	/* Comment out? */
    PSmove_offset(eptr->y_offset);
    PSprintf("gsave currentpoint %d sub translate ", h);
    PSprintf("%d %d scale\n", w, h);
    PSprintf("SQ 0.9 setgray fill\n");
    PSprintf("grestore\n");
#endif
}


/*
 * PSrle_encode - perform run length encoding
 *
 * does the run-length encoding. This is done to reduce the file size and
 * therefore the time to send the file to the printer. You get longer
 * processing time instead.
 * 
 * rle is encoded as such:
 *  <count> <value>			# 'run' of count+1 equal pixels
 *  <count | 0x80> <count+1 data bytes>	# count+1 non-equal pixels
 * count can range between 0 and 127
 *
 * returns length of the rleline vector
 *
*/ 
static int
PSrle_encode(unsigned char *scanline, 
	     unsigned char *rleline,
	     int wide) 
{
    int  i, j, blocklen, isrun, rlen;
    unsigned char block[256], pix;

    blocklen = isrun = rlen = 0;

    for (i = 0; i < wide; i++) 
    {
	/*  there are 5 possible states:
	 *   0: block empty.
	 *   1: block is a run, current pix == previous pix
	 *   2: block is a run, current pix != previous pix
	 *   3: block not a run, current pix == previous pix
	 *   4: block not a run, current pix != previous pix
	 */

	pix = scanline[i];

	if (!blocklen) 
	{
	    /* case 0:  empty */
	    block[blocklen++] = pix;
	    isrun = 1;
	}
	else if (isrun) 
	{
	    if (pix == block[blocklen-1])
	    { 
		/*  case 1:  isrun, prev==cur */
		block[blocklen++] = pix;
	    }
	    else 
	    {
		/*  case 2:  isrun, prev!=cur */
		if (blocklen>1) 
		{
		    /*  we have a run block to flush */
		    rleline[rlen++] = blocklen-1;
		    rleline[rlen++] = block[0];
		    /*  start new run block with pix */
		    block[0] = pix;
		    blocklen = 1;
		}
		else
		{
		    /*  blocklen<=1, turn into non-run */
		    isrun = 0;
		    block[blocklen++] = pix;
		}
	    }
	}
	else
	{ 
	    /* not a run */
	    if (pix == block[blocklen-1]) 
	    {
		/* case 3: non-run, prev==cur */
		if (blocklen>1) 
		{
		    /*  have a non-run block to flush */
		    rleline[rlen++] = (blocklen-1) | 0x80;
		    for (j=0; j<blocklen; j++)
			rleline[rlen++] = block[j];
		    /*  start new run block with pix */
		    block[0] = pix;
		    blocklen = isrun = 1;
		} 
		else
		{
		    /*  blocklen<=1 turn into a run */
		    isrun = 1;
		    block[blocklen++] = pix;
		}
	    }
	    else 
	    {
		/* case 4:  non-run, prev!=cur */
		block[blocklen++] = pix;
	    }
	}
	
	/* max block length.  flush */
	if (blocklen == 128)
	{
	    if (isrun) 
	    {
		rleline[rlen++] = blocklen-1;
		rleline[rlen++] = block[0];
	    }
	    else
	    {
		rleline[rlen++] = (blocklen-1) | 0x80;
		for (j=0; j<blocklen; j++)
		    rleline[rlen++] = block[j];
	    }
	    blocklen = 0;
	}
    }

    /* flush last block */
    if (blocklen)
    {
	if (isrun)
	{
	    rleline[rlen++] = blocklen-1;
	    rleline[rlen++] = block[0];
	}
	else
	{
	    rleline[rlen++] = (blocklen-1) | 0x80;
	    for (j=0; j<blocklen; j++)
		rleline[rlen++] = block[j];
	}
    }

    return rlen;
}


/*
 * PScolor_image - created postscript colorimage operator 
 *
 * spits out code that checks if the PostScript device in question
 * knows about the 'colorimage' operator.  If it doesn't, it defines
 * 'colorimage' in terms of image (ie, generates a greyscale image from
 * RGB data)
 *
*/

static void 
PScolor_image(void) 
{
    static char *txt[] = {

	"% define 'colorimage' if it isn't defined",
	"%   ('colortogray' and 'mergeprocs' come from xwd2ps",
	"%	 via xgrab)",
	"/colorimage where   % do we know about 'colorimage'?",
	"  { pop }		   % yes: pop off the 'dict' returned",
	"  {				 % no:  define one",
	"	/colortogray {  % define an RGB->I function",
	"	  /rgbdata exch store	% call input 'rgbdata'",
	"	  rgbdata length 3 idiv",
	"	  /npixls exch store",
	"	  /rgbindx 0 store",
	"	  /grays npixls string store  % str to hold the result",
	"	  0 1 npixls 1 sub {",
	"		grays exch",
	"		rgbdata rgbindx	   get 20 mul	% Red",
	"		rgbdata rgbindx 1 add get 32 mul	% Green",
	"		rgbdata rgbindx 2 add get 12 mul	% Blue",
	"		add add 64 idiv	  % I = .5G + .31R + .18B",
	"		put",
	"		/rgbindx rgbindx 3 add store",
	"	  } for",
	"	  grays",
	"	} bind def\n",
	/* Utility procedure for colorimage operator.
	 * This procedure takes two procedures off the
	 * stack and merges them into a single procedure
	 */
	"	/mergeprocs { % def",
	"	  dup length",
	"	  3 -1 roll dup length dup 5 1 roll",
	"	  3 -1 roll add array cvx dup",
	"	  3 -1 roll 0 exch putinterval",
	"	  dup 4 2 roll putinterval",
	"	} bind def\n",
	"	/colorimage { % def",
	/* remove 'false 3' operands */
	"	  pop pop",
	"	  {colortogray} mergeprocs",
	"	  image",
	"	} bind def",
	/* end of 'false' case */
	"  } ifelse"
    };

    PSconst_out(txt);
}
 

/*
 * PScolormap - write colormap
 *
 * spits out code for the colormap of the following image
 * if !color, it spits out a mono-ized graymap
 * 
*/
static void
PScolormap(int color, 
	   int nc, 
	   int *rmap, 
	   int *gmap, 
	   int *bmap) 
{
    int i;

    /*  define the colormap */
    PSprintf("/cmap %d string def\n\n\n", nc * ((color) ? 3 : 1));

    /*  load up the colormap */
    PSprintf("currentfile cmap readhexstring\n");

    for (i=0; i<nc; i++) 
    {
	if (color)  
	    PSprintf("%02x%02x%02x ", rmap[i]>>8,
		     gmap[i]>>8, bmap[i]>>8);
	else  
	    PSprintf("%02x ", MONO(rmap[i], gmap[i], bmap[i]));
	if ((i%10) == 9)
	    PSprintf("\n");
    }
    PSprintf("\n");
    PSprintf("pop pop\n"); /* lose return values from readhexstring */
}


/*
 * PSrle_cmapimage - define rlecmapimage operator
 * 
 */
static void
PSrle_cmapimage(int color) 
{

    static char *txt[] = {

	/* rlecmapimage expects to have 'w h bits matrix' on stack */
	"/rlecmapimage {",
	"  /buffer 1 string def",
	"  /rgbval 3 string def",
	"  /block  384 string def",
	"  { currentfile buffer readhexstring pop",
	"	/bcount exch 0 get store",
	"	bcount 128 ge",
	"	{ ",
	"	  0 1 bcount 128 sub",
	"	{ currentfile buffer readhexstring pop pop"
    };

    static char *txt_color[] = {
	"		/rgbval cmap buffer 0 get 3 mul 3 getinterval store",
	"		block exch 3 mul rgbval putinterval",
	"	  } for",
	"	  block  0  bcount 127 sub 3 mul  getinterval",
	"	}",
	"	{ ",
	"	  currentfile buffer readhexstring pop pop",
	"	  /rgbval cmap buffer 0 get 3 mul 3 getinterval store",
	"	  0 1 bcount { block exch 3 mul rgbval putinterval } for",
	"	  block 0 bcount 1 add 3 mul getinterval",
	"	} ifelse",
	"  }",
	"  false 3 colorimage",
	"} bind def"
    };

    static char *txt_gray[] = {
	"		/rgbval cmap buffer 0 get 1 getinterval store",
	"		block exch rgbval putinterval",
	"	  } for",
	"	  block  0  bcount 127 sub  getinterval",
	"	}",
	"	{ ",
	"	  currentfile buffer readhexstring pop pop",
	"	  /rgbval cmap buffer 0 get 1 getinterval store",
	"	  0 1 bcount { block exch rgbval putinterval } for",
	"	  block 0 bcount 1 add getinterval",
	"	} ifelse",
	"  }",
	"  image",
	"} bind def"
    };

    PSconst_out(txt);
    if (color) 
    {
	PSconst_out(txt_color);
    } 
    else 
    {
	PSconst_out(txt_gray);
    }
}


/*
 * PSwrite_bw - write B&W image
 *
 * Write the given image array 'pic' (B/W stippled, 1 byte per pixel,
 * 0=blk,1=wht) out as hexadecimal, max of 72 hex chars per line.  If
 * 'flipbw', then 0=white, 1=black.  Returns '0' if everythings fine,
 * 'EOF' if writing failed.
 *
*/
static int
PSwrite_bw(unsigned char *pic, int w, int h, int flipbw)
{
    int	i, j;
    int	err=0;
    unsigned char outbyte, bitnum, bit;
    
    outbyte = bitnum = 0;
    for (i=0; i<h && err != EOF; i++) {
	for (j=0; j<w && err != EOF; j++) {
	    bit = *(pic++);
	    outbyte = (outbyte<<1) | ((bit)&0x01);
	    bitnum++;
	    
	    if (bitnum==8) {
		if (flipbw)
		    outbyte = ~outbyte & 0xff;
		err=PShex(outbyte, False);
		outbyte = bitnum = 0;
	    }
	}
	if (bitnum) {	/*  few bits left over in this row */
	    outbyte <<= 8-bitnum;
	    if (flipbw)
		outbyte = ~outbyte & 0xff;
	    err=PShex(outbyte, False);
	    outbyte = bitnum = 0;
	}
    }
    err=PShex('\0', True);	/*  Flush the hex buffer if needed */
    
    return err;
}


/*
 * PSimage - generate image Postscript code
 *
 * Draw the image, unless there was no image, in which case an empty grey
 * rectangle is shown.
 * If anchor is set, a black border is shown around the image.
 * Positioning is not exactly that of Xmosaic's screen, but close enough.
 * 
*/

static void 
PSimage(HTMLWidget hw, struct ele_rec *eptr, int fontfamily)
{
    ImageInfo 		*img = eptr->pic_data;
    unsigned char 	*imgp = img->image_data;
    int 		anchor = (eptr->anchorHRef != NULL); 	
    int 		ncolors = img->num_colors;
    int 		i, j;
    int 		w = img->width;
    int 		h = img->height;
    int 		slen, colorps, colortype, bits;
    int 		err=0;
    int 		extra = 0;


    /* Isgray returns true if the nth color index is a gray value */
#	define Isgray(i,n) (i->reds[n]==i->greens[n] && i->reds[n]==i->blues[n])
    /* Is_bg returns true if the nth color index is the screen background */
#	define Is_bg(i,n)  (i->reds[n]==bg_color.red &&			\
		i->greens[n]==bg_color.green && i->blues[n]==bg_color.blue)
    /* Is_fg returns true if the nth color index is the screen foreground */
#	define Is_fg(i,n)  (i->reds[n]==fg_color.red &&			\
		i->greens[n]==fg_color.green && i->blues[n]==fg_color.blue)


    PSmove_offset(eptr->y_offset);
    if (anchor) 
    {
	/*  draw an outline by drawing a slightly larger black square
	 *  below the actual image
	 */
	PSprintf("gsave currentpoint %d sub translate ", h);
	PSprintf("0 -2 translate %d %d scale\n", w+4, h+4);
	PSprintf("SQ fill\n");
	PSprintf("grestore\n");
	extra = 4;
    }
	
    if (imgp == NULL) 
    {
	/*  image was not available... do something instead
	 *  draw an empty square for example
	 */
	PSprintf("gsave currentpoint %d sub translate", h);
	if (anchor)
	    PSprintf(" 2 0 translate");
	else
	    PSprintf(" 0 2 translate");
	PSprintf(" %d %d scale\n", w, h);
	PSprintf("0.9 setgray SQ fill\n");
	PSprintf("grestore\n");
	/*  move currentpoint just right of image */
	PSprintf("%d 0 R\n", w+extra);
	return;
    }

    /*  this is a hack to see if the image is Black & White, 
     *  Greyscale or 8 bit color
     *  assume it's bw if it has only one or two colors, both some grey's
     *  assume it's greyscale if all the colors (>2) are grey
     *  Images with only one color do occur too.
     */
    
    if (   (   (ncolors == 2) 
	    && (   (Isgray(img,0) && Isgray(img,1))
	        || (Is_bg(img,0) && Is_fg(img,1)) 
	        || (Is_fg(img,0) && Is_bg(img,1)) ))
	|| (   (ncolors == 1)
	    && (Isgray(img,0)
		|| Is_bg(img,0) 
		|| Is_fg(img,0))))
    {
	colortype = F_BWDITHER;
	slen = (w+7)/8;
	bits = 1;
	colorps = 0;
    } else {
	colortype = F_GREYSCALE;
	slen = w;
	bits = 8;
	colorps = 0;
	for (i=0; i<ncolors; i++) {
	    if (!Isgray(img,i)) {
		colortype = F_REDUCED;
		slen = w*3;
		bits = 8;
		colorps = 1;
		break;
	    }
	}
    }
	
    /*  build a temporary dictionary */
    PSprintf("20 dict begin\n\n");

    /*  define string to hold a scanline's worth of data */
    PSprintf("/pix %d string def\n\n", slen);
    
    /*  position and scaling */
    PSprintf("gsave currentpoint %d sub translate", h);
    if (anchor)
	PSprintf(" 2 0 translate");
    else
	PSprintf(" 0 2 translate");
    PSprintf(" %d %d scale\n", w, h);
    
    if (colortype == F_BWDITHER) 
    {
	/*  1-bit dither code uses 'image' */
	int flipbw = 0;
	
	/*  set if color#0 is 'white' */
	if ((ncolors == 2 &&
	     MONO(img->reds[0], img->greens[0],img->blues[0]) >
	     MONO(img->reds[1], img->greens[1], img->blues[1])) ||
	    (ncolors == 1 && 
	     MONO(img->reds[0], img->greens[0],img->blues[0]) >
	     MONO(127, 127, 127) ))
	{
	    flipbw=1; 
	}
	
	/*  dimensions of data */
	PSprintf("%d %d %d\n", w, h, bits);
	
	/*  mapping matrix */
	PSprintf("[%d 0 0 %d 0 %d]\n\n", w, -h, h);
	
	PSprintf("{currentfile pix readhexstring pop}\n");
	PSprintf("image\n");

	/*  write the actual image data */
	err = PSwrite_bw(imgp, w, h, flipbw);
    } 
    else
    {
	/*  all other formats */
	unsigned char *rleline = (unsigned char *) NULL;
	int rlen;
	
	/*  if we're using color, make sure 'colorimage' is defined */
	if (colorps)
	    PScolor_image();
	
	PScolormap(colorps, ncolors, img->reds, img->greens, img->blues);
	PSrle_cmapimage(colorps);

	/*  dimensions of data */
	PSprintf("%d %d %d\n", w, h, bits);
	/*  mapping matrix */
	PSprintf("[%d 0 0 %d 0 %d]\n", w, -h, h);
	PSprintf("rlecmapimage\n");
	
	rleline = (unsigned char *) malloc(w * 2);
	if (!rleline) 
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr,"failed to malloc space for rleline\n");
		}
#endif

		return;
	}

	for (i=0; i<h && err != EOF; i++) 
	{
	    rlen = PSrle_encode(imgp, rleline, w);
	    imgp += w;
	    for (j=0; j<rlen && err != EOF; j++)
		err=PShex(rleline[j], False);
	    err=PShex('\0', True);	/*  Flush the hex buffer */
	}
	free(rleline);
    }
	
    /*  stop using temporary dictionary */
    PSprintf("end\n");
    PSprintf("grestore\n");
	
    /*  move currentpoint just right of image */
    PSprintf("%d 0 R\n", w + extra); 
    if (HTML_Print_Footers && has_footnote(eptr)) 
    {
	PSmove_offset(0);
	PSfootnote(eptr->anchorHRef, 2.0);
    }


    /* forget about the macro's */
#	undef Isgray
#	undef Is_fg
#	undef Is_bg
}



/*
 * ParseTextToPSString - entry point for postscript output
 *
 * Parse all the formatted text elements from start to end
 * into an ascii text string, and return it.
 * Very like ParseTextToString() except the text is prettied up
 * into Postscript to show headers and the like.
 * space_width and lmargin tell us how many spaces
 * to indent lines.
 * Because this routine is only used to print whole documents,
 * some parameters are not needed at all!
 * Also it assumes that you are indeed printing the whole document, and
 * not just a selected portion of it. It therefore can assume that
 * only for the first page the initialization is needed, and only
 * the last page has the trailers. You cannot use ParseTextToPSString()
 * as you can use ParseTextToString() because of this initialization code.
 *
 * the fontfamily parameter is new
 * The font is encoded as:
 *	0: times (default for now)
 *	1: helvetica
 *	2: new century schoolbook
 *	3: lucida
 */
String ParseTextToPSString(HTMLWidget	 	hw, 
			   struct ele_rec	*elist,
			   struct ele_rec	*startp,
			   struct ele_rec	*endp,
			   int			start_pos, 
			   int			end_pos,
			   int			space_width,
			   int			lmargin,
			   int			fontfamily,
			   char   		*url,
			   char	   		*time_str)
{
    int			xpos, ypos, epos;
    int			height;
    double		pagewidth;
    int			line = -1;
    struct		ele_rec	*eptr;
    struct		ele_rec	*start;
    struct		ele_rec	*end;
    struct		ele_rec	*last;
    struct		ele_rec	*tmpptr;
    unsigned long 	fg_pixel, bg_pixel;    
    int			footnotes_this_page = 0;
    int			footnotes_this_line;
    int			reserved_space;
 	
    if (startp == NULL)
	return(NULL);


    /*
     * Get the foreground and background colors so we can check later
     * for black&white documents
     */
    XtVaGetValues (hw->html.view, 
#ifdef MOTIF
		   XtNforeground, &fg_pixel,
#endif
		   XtNbackground, &bg_pixel, 
		   NULL);
#ifndef MOTIF
    XtVaGetValues ((Widget)hw, 
		   XtNforeground, &fg_pixel,
		   NULL);
#endif	
    fg_color.pixel = fg_pixel;
    bg_color.pixel = bg_pixel;
    XQueryColor(XtDisplay(hw->html.view),
		(installed_colormap ?
		 installed_cmap :
		 DefaultColormap(XtDisplay(hw->html.view),
				DefaultScreen(XtDisplay(hw->html.view)))),
		&fg_color);
    XQueryColor(XtDisplay(hw->html.view),
		(installed_colormap ?
		 installed_cmap :
		 DefaultColormap(XtDisplay(hw->html.view),
				 DefaultScreen(XtDisplay(hw->html.view)))),
		&bg_color);
    
    /*  this piece of code is needed if the user selects a portion
     *  of the document with the mouse.
     *  I think it will never be used, but I left it in anyway. F.
     */
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
	
    /* Setup page size according to user preference. */

    if (HTML_Print_Paper_Size_A4)
	page_dimens = a4_page_dimens;
    else
	page_dimens = us_letter_page_dimens;

    page_dimens.text_height = (  page_dimens.page_height
			       - page_dimens.top_margin 
			       - page_dimens.bot_margin);
    page_dimens.text_width  = (  page_dimens.page_width
			       - page_dimens.left_margin 
			       - page_dimens.right_margin);

    /* Calculate the number of Postscript points per pixel of current
     * screen, and the height of the page in pixels (used in figuring
     * when we've hit the bottom of the page).
     */
    Points_Pixel = 72.0 / GetDpi(hw);
#ifdef OLD
    pagewidth = hw->html.doc_width;
#else /* gustaf fix */
    pagewidth = hw->html.view_width;
#endif /* gustaf fix */

    /* reduce the scaling if the width used for formatting is greater
     * than 8 * 72 pixels (8 inch)
     * In theory, this is not what you want for A4 paper (only 8.27 inch
     * wide), but I guess that the hw->html.doc_width includes some
     * left and right margins, so it seems to work in practice.
     */
    if (pagewidth > page_dimens.text_width) 
	Points_Pixel = Points_Pixel * page_dimens.text_width / pagewidth;
    Pixels_Page = (int) (page_dimens.text_height / Points_Pixel);		
	
    PSinit();
    PSheader(hw->html.title, fontfamily, url, time_str);
    PSnewpage();

    last = start;
    eptr = start;

    while ((eptr != NULL) && (eptr != end)) 
    {
	/* Skip the special internal text added for multi-page
	 * documents.
	 */
	if (eptr->internal == True) 
	{
	    if (eptr->type == E_LINEFEED) 
	    {
		PS_page_offset += eptr->line_height;
	    }
	    eptr = eptr->next;
	    continue;
	}

	/* check if this is a newline */
	if (line != eptr->line_number) 
	{
	    /* calculate max height */
	    height = 0;
	    footnotes_this_line = 0;
	    line = eptr->line_number;
	    tmpptr = eptr;
	    while (tmpptr != NULL && tmpptr->line_number == line) 
	    {
		if (tmpptr->line_height > height)
		    height = tmpptr->line_height;
		tmpptr = tmpptr->next;
		if (HTML_Print_Footers && has_footnote(tmpptr))
		    footnotes_this_line++;
	    }
	    ypos = eptr->y - PS_page_offset ;
	    xpos = eptr->x - lmargin;
	    if (xpos < 0)
		xpos = 0;

	    /* check if line fits completly on page */
	    
	    reserved_space = 0;
	    if (footnotes_this_page || footnotes_this_line) 
	    {
		reserved_space = (  (  footnote_space
				     + (  footnote_ptsize
					* (  footnotes_this_page 
					   + footnotes_this_line)))
				  / Points_Pixel);
	    }

	    if (ypos + height + reserved_space > PS_start_y + Pixels_Page) 
	    {
		PS_start_y = ypos;
		PSshowpage();
		PSnewpage();
		footnotes_this_page = 0;
	    }
	    footnotes_this_page += footnotes_this_line;
	    PSmoveto( xpos, ypos);
	}
		

	switch (eptr->type) 
	{
	case E_TEXT:
	    PStext(hw, eptr, fontfamily,
		   (String)((eptr == start) ? (eptr->edata + start_pos) : eptr->edata));
	    break;

	case E_BULLET: 
	    PSbullet(hw, eptr, fontfamily);
	    break;

	case E_IMAGE: 
	    PSimage(hw, eptr, fontfamily);
	    break;

	case E_LINEFEED: 
	    break;

	case E_HRULE:
	    PShrule(hw, eptr, fontfamily);
	    break;

	case E_TABLE:
	    PStable(hw, eptr, fontfamily);
	    break;

	case E_WIDGET:
	    PSwidget(hw, eptr, fontfamily);
	    break;

	}
	last = eptr;
	eptr = eptr->next;
    }
	
    PSshowpage();
    PStrailer();

    return (PS_string);
}

