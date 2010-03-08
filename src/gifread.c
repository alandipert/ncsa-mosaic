/* +-------------------------------------------------------------------+ */
/* | Copyright 1990 - 1994, David Koblas. (koblas@netcom.com)          | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */
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
#include <stdio.h>
#include <X11/Intrinsic.h>
#include "gifread.h"

/*
#define TIMING 1
*/

#include <sys/time.h>
static struct timeval Tv;
static struct timezone Tz;


#define	MAXCOLORMAPSIZE		256

#define	TRUE	1
#define	FALSE	0

#define CM_RED		0
#define CM_GREEN	1
#define CM_BLUE		2

#define	MAX_LWZ_BITS		12

#define INTERLACE		0x40
#define LOCALCOLORMAP	0x80
#define BitSet(byte, bit)	(((byte) & (bit)) == (bit))

#define	ReadOK(file,buffer,len)	(fread(buffer, len, 1, file) != 0)

#define LM_to_uint(a,b)			(((b)<<8)|(a))

static struct {
	unsigned int	Width;
	unsigned int	Height;
	unsigned char	ColorMap[3][MAXCOLORMAPSIZE];
	unsigned int	BitPixel;
	unsigned int	ColorResolution;
	unsigned int	Background;
	unsigned int	AspectRatio;
	/*
	**
	*/
	int             xGrayScale;
} GifScreen;

static struct {
	int	transparent;
	int	delayTime;
	int	inputFlag;
	int	disposal;
} Gif89 = { -1, -1, -1, 0 };

static int	verbose = FALSE;
static int	showComment = FALSE;


static int ReadColorMap(FILE *, int, unsigned char [3][MAXCOLORMAPSIZE], int *);
static int DoExtension(FILE *, int);
static int GetDataBlock(FILE *, unsigned char *);
static unsigned char *ReadImage(FILE *, int, int, XColor *, int,
                unsigned char[3][MAXCOLORMAPSIZE], int, int, int);
/*
unsigned char *ReadGIF ARGS(( FILE	*fd, int imageNumber, int *bg ));
static int ReadColorMap ARGS(( FILE *fd, int number, unsigned char buffer[3][MAXCOLORMAPSIZE] ));
static int DoExtension ARGS(( FILE *fd, int label ));
static int GetDataBlock ARGS(( FILE *fd, unsigned char  *buf ));
static int GetCode ARGS(( FILE *fd, int code_size, int flag ));
static int LWZReadByte ARGS(( FILE *fd, int flag, int input_code_size ));
static unsigned char *ReadImage ARGS(( FILE *fd, int len, int height, unsigned char cmap[3][MAXCOLORMAPSIZE], int interlace, int ignore ));
*/

/*
static int ReadColorMap();
static int DoExtension();
static int GetDataBlock();
static int GetCode();
static int LWZReadByte();
static unsigned char *ReadImage();
*/

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

unsigned char *
ReadGIF(FILE *fd, int *w, int *h, XColor *colrs, int *bg)
{
	unsigned char	buf[16];
	unsigned char	c;
	unsigned char	localColorMap[3][MAXCOLORMAPSIZE];
	int		grayScale;
	int		useGlobalColormap;
	int		bitPixel;
	int		imageCount = 0;
	char		version[4];
	int		imageNumber = 1;
	unsigned char	*image = NULL;
	int i;

#ifndef DISABLE_TRACE
	if (srcTrace) {
		gettimeofday(&Tv, &Tz);
		fprintf(stderr, "ReadGIF_DK enter (%d.%d)\n", Tv.tv_sec, Tv.tv_usec);
	}
#endif

	verbose = FALSE;
	showComment = FALSE;

	/*
	 * Initialize GIF89 extensions
	 */
	Gif89.transparent = -1;
	Gif89.delayTime = -1;
	Gif89.inputFlag = -1;
	Gif89.disposal = 0;

	if (! ReadOK(fd,buf,6))
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "error reading magic number\n");
		}
#endif

		return(NULL);
	}

	if (strncmp((char *)buf,"GIF",3) != 0)
	{
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "not a GIF file\n");
#endif

		return(NULL);
	}

	strncpy(version, (char *)buf + 3, 3);
	version[3] = '\0';

	if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0))
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "bad version number, not '87a' or '89a'\n");
		}
#endif

		return(NULL);
	}

	if (! ReadOK(fd,buf,7))
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "failed to read screen descriptor\n");
		}
#endif

		return(NULL);
	}

	GifScreen.Width           = LM_to_uint(buf[0],buf[1]);
	GifScreen.Height          = LM_to_uint(buf[2],buf[3]);
	GifScreen.BitPixel        = 2<<(buf[4]&0x07);
	GifScreen.ColorResolution = (((buf[4]&0x70)>>3)+1);
	GifScreen.Background      = buf[5];
	GifScreen.AspectRatio     = buf[6];

	if (BitSet(buf[4], LOCALCOLORMAP)) {	/* Global Colormap */
		int scale = 65536/MAXCOLORMAPSIZE;

		if (ReadColorMap(fd,GifScreen.BitPixel,GifScreen.ColorMap,
				&GifScreen.xGrayScale))
		{

#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr, "error reading global colormap\n");
			}
#endif

			return(NULL);
		}
		for (i=0; i < GifScreen.BitPixel; i++)
		{

			colrs[i].red = GifScreen.ColorMap[0][i] * scale;
			colrs[i].green = GifScreen.ColorMap[1][i] * scale;
			colrs[i].blue = GifScreen.ColorMap[2][i] * scale;
			colrs[i].pixel = i;
			colrs[i].flags = DoRed|DoGreen|DoBlue;
		}
		for (i = GifScreen.BitPixel; i<MAXCOLORMAPSIZE; i++)
		{
			colrs[i].red = 0;
			colrs[i].green = 0;
			colrs[i].blue = 0;
			colrs[i].pixel = i;
			colrs[i].flags = DoRed|DoGreen|DoBlue;
		}
	}

	if (GifScreen.AspectRatio != 0 && GifScreen.AspectRatio != 49) {
/*		float	r;
		r = ( (float) GifScreen.AspectRatio + 15.0 ) / 64.0;*/
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "Warning:  non-square pixels!\n");
		}
#endif
	}

	while (image == NULL) {
		if (! ReadOK(fd,&c,1))
		{
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr, "EOF / read error on image data\n");
			}
#endif

			return(NULL);
		}

		if (c == ';') {		/* GIF terminator */
			if (imageCount < imageNumber)
			{
#ifndef DISABLE_TRACE
				if (srcTrace) {
					fprintf(stderr, "No images found in file\n");
				}
#endif

				return(NULL);
			}
			break;
		}

		if (c == '!') { 	/* Extension */
			if (! ReadOK(fd,&c,1))
			{
#ifndef DISABLE_TRACE
				if (srcTrace) {
					fprintf(stderr, "EOF / read error on extention function code\n");
				}
#endif

				return(NULL);
			}
			DoExtension(fd, c);
			continue;
		}

		if (c != ',') {		/* Not a valid start character */
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr, "bogus character 0x%02x, ignoring\n",
					(int)c);
			}
#endif

			continue;
		}

		++imageCount;

		if (! ReadOK(fd,buf,9))
		{
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr,"couldn't read left/top/width/height\n");
			}
#endif

			return(NULL);
		}

		useGlobalColormap = ! BitSet(buf[8], LOCALCOLORMAP);

		bitPixel = 1<<((buf[8]&0x07)+1);

		/*
		 * We only want to set width and height for the imageNumber
		 * we are requesting.
		 */
		if (imageCount == imageNumber)
		{
			*w = LM_to_uint(buf[4],buf[5]);
			*h = LM_to_uint(buf[6],buf[7]);
		}
		if (! useGlobalColormap) {
			if (ReadColorMap(fd,bitPixel,localColorMap,&grayScale))
			{
#ifndef DISABLE_TRACE
				if (srcTrace) {
					fprintf(stderr, "error reading local colormap\n");
				}
#endif

				return(NULL);
			}

			/*
			 * We only want to set the data for the
			 * imageNumber we are requesting.
			 */
			if (imageCount == imageNumber)
			{
			    image = ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				  LM_to_uint(buf[6],buf[7]), colrs,
				  bitPixel, localColorMap, grayScale,
				  BitSet(buf[8], INTERLACE),
				  imageCount != imageNumber);
			}
			else
			{
/*			    unsigned char *tdata;

			    tdata =*/ ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				  LM_to_uint(buf[6],buf[7]), colrs,
				  bitPixel, localColorMap, grayScale,
				  BitSet(buf[8], INTERLACE),
				  imageCount != imageNumber);
			}
		} else {
			/*
			 * We only want to set the data for the
			 * imageNumber we are requesting.
			 */
			if (imageCount == imageNumber)
			{
			    image = ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				  LM_to_uint(buf[6],buf[7]), colrs,
				  GifScreen.BitPixel, GifScreen.ColorMap,
				  GifScreen.xGrayScale,
				  BitSet(buf[8], INTERLACE),
				  imageCount != imageNumber);
			}
			else
			{
/*			    unsigned char *tdata;

			    tdata =*/ ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				  LM_to_uint(buf[6],buf[7]), colrs,
				  GifScreen.BitPixel, GifScreen.ColorMap,
				  GifScreen.xGrayScale,
				  BitSet(buf[8], INTERLACE),
				  imageCount != imageNumber);
			}
		}

	}
	*bg = Gif89.transparent;

#ifndef DISABLE_TRACE
	if (srcTrace) {
		gettimeofday(&Tv, &Tz);
		fprintf(stderr, "ReadGIF_DK exit (%d.%d)\n", Tv.tv_sec, Tv.tv_usec);
	}
#endif

	return(image);
}

static int
ReadColorMap(FILE *fd, int number, unsigned char buffer[3][MAXCOLORMAPSIZE],
		int *gray)
{
	int		i;
	unsigned char	rgb[3];
	int		flag;

	flag = TRUE;

	for (i = 0; i < number; ++i) {
		if (! ReadOK(fd, rgb, sizeof(rgb)))
		{
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr, "bad colormap\n");
			}
#endif

			return(TRUE);
		}

		buffer[CM_RED][i] = rgb[0] ;
		buffer[CM_GREEN][i] = rgb[1] ;
		buffer[CM_BLUE][i] = rgb[2] ;

		flag &= (rgb[0] == rgb[1] && rgb[1] == rgb[2]);
	}

	*gray = flag;

	return FALSE;
}

static int
DoExtension(FILE *fd, int label)
{
	static char	buf[256];
	char		str[256];

	switch (label) {
	case 0x01:		/* Plain Text Extension */
		strcpy(str,"Plain Text Extension");
#ifdef notdef
		if (GetDataBlock(fd, (unsigned char*) buf) <= 0)
			;

		lpos   = LM_to_uint(buf[0], buf[1]);
		tpos   = LM_to_uint(buf[2], buf[3]);
		width  = LM_to_uint(buf[4], buf[5]);
		height = LM_to_uint(buf[6], buf[7]);
		cellw  = buf[8];
		cellh  = buf[9];
		foreground = buf[10];
		background = buf[11];

		while (GetDataBlock(fd, (unsigned char*) buf) > 0) {
			PPM_ASSIGN(image[ypos][xpos],
					cmap[CM_RED][v],
					cmap[CM_GREEN][v],
					cmap[CM_BLUE][v]);
			++index;
		}

		return FALSE;
#else
		break;
#endif
	case 0xff:		/* Application Extension */
		strcpy(str,"Application Extension");
		break;
	case 0xfe:		/* Comment Extension */
		strcpy(str,"Comment Extension");
		while (GetDataBlock(fd, (unsigned char*) buf) > 0) {
			if (showComment)
			{
#ifndef DISABLE_TRACE
				if (srcTrace) {
					fprintf(stderr, "gif comment: %s\n", buf);
				}
#endif
			}
		}
		return FALSE;
	case 0xf9:		/* Graphic Control Extension */
		strcpy(str,"Graphic Control Extension");
		(void) GetDataBlock(fd, (unsigned char*) buf);
		Gif89.disposal    = (buf[0] >> 2) & 0x7;
		Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
		Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
		if ((buf[0] & 0x1) != 0)
			Gif89.transparent = (int)((unsigned char)buf[3]);

		while (GetDataBlock(fd, (unsigned char*) buf) > 0)
			;
		return FALSE;
	default:
		sprintf(str, "UNKNOWN (0x%02x)", label);
		break;
	}
#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf(stderr, "got a '%s' extension\n", str);
	}
#endif

	while (GetDataBlock(fd, (unsigned char*) buf) > 0)
		;

	return FALSE;
}

static int	ZeroDataBlock = FALSE;

static int
GetDataBlock(FILE *fd, unsigned char *buf)
{
	unsigned char	count;

	count = 0;
	if (! ReadOK(fd, &count, 1)) {
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "error in getting DataBlock size\n");
		}
#endif

		return -1;
	}

	ZeroDataBlock = count == 0;

	if ((count != 0) && (! ReadOK(fd, buf, count))) {
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "error in reading DataBlock\n");
		}
#endif

		return -1;
	}

	return((int)count);
}


/*
**  Pulled out of nextCode
*/
static  int             curbit, lastbit, get_done, last_byte;
static  int             return_clear;
/*
**  Out of nextLWZ
*/
static int      stack[(1<<(MAX_LWZ_BITS))*2], *sp;
static int      code_size, set_code_size;
static int      max_code, max_code_size;
static int      clear_code, end_code;

static void initLWZ(int input_code_size)
{
/*	static int      inited = FALSE;*/

	set_code_size = input_code_size;
	code_size     = set_code_size + 1;
	clear_code    = 1 << set_code_size ;
	end_code      = clear_code + 1;
	max_code_size = 2 * clear_code;
	max_code      = clear_code + 2;

	curbit = lastbit = 0;
	last_byte = 2;
	get_done = FALSE;

	return_clear = TRUE;

	sp = stack;
}

static int nextCode(FILE *fd, int code_size)
{
	static unsigned char    buf[280];
	static int maskTbl[16] = {
		0x0000, 0x0001, 0x0003, 0x0007,
		0x000f, 0x001f, 0x003f, 0x007f,
		0x00ff, 0x01ff, 0x03ff, 0x07ff,
		0x0fff, 0x1fff, 0x3fff, 0x7fff,
	};
	int                     i, j, ret, end;

	if (return_clear) {
		return_clear = FALSE;
		return clear_code;
	}

	end = curbit + code_size;

	if (end >= lastbit) {
		int     count;

		if (get_done) {
			if (curbit >= lastbit)
			{
#if 0
				ERROR("ran off the end of my bits" );
#endif
			}
			return -1;
		}
		buf[0] = buf[last_byte-2];
		buf[1] = buf[last_byte-1];

		if ((count = GetDataBlock(fd, &buf[2])) == 0)
			get_done = TRUE;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2+count)*8 ;

		end = curbit + code_size;
	}

	j = end / 8;
	i = curbit / 8;

	if (i == j)
		ret = (int)buf[i];
	else if (i + 1 == j)
		ret = (int)buf[i] | ((int)buf[i+1] << 8);
	else
		ret = (int)buf[i] | ((int)buf[i+1] << 8) | ((int)buf[i+2] << 16);

	ret = (ret >> (curbit % 8)) & maskTbl[code_size];

	curbit += code_size;

	return ret;
}

#define readLWZ(fd) ((sp > stack) ? *--sp : nextLWZ(fd))

static int nextLWZ(FILE *fd)
{
	static int       table[2][(1<< MAX_LWZ_BITS)];
	static int       firstcode, oldcode;
	int              code, incode;
	register int     i;

	while ((code = nextCode(fd, code_size)) >= 0) {
	       if (code == clear_code) {

			/* corrupt GIFs can make this happen */
			if (clear_code >= (1<<MAX_LWZ_BITS))
			{
				return -2;
			}

		       for (i = 0; i < clear_code; ++i) {
			       table[0][i] = 0;
			       table[1][i] = i;
		       }
		       for (; i < (1<<MAX_LWZ_BITS); ++i)
			       table[0][i] = table[1][i] = 0;
		       code_size = set_code_size+1;
		       max_code_size = 2*clear_code;
		       max_code = clear_code+2;
		       sp = stack;
			do {
			       firstcode = oldcode = nextCode(fd, code_size);
			} while (firstcode == clear_code);

			return firstcode;
	       }
	       if (code == end_code) {
		       int             count;
		       unsigned char   buf[260];

		       if (ZeroDataBlock)
			       return -2;

		       while ((count = GetDataBlock(fd, buf)) > 0)
			       ;

		       if (count != 0)
			{
#ifndef DISABLE_TRACE
				if (srcTrace) {
					fprintf(stderr,"missing EOD in data stream (common occurence)");
				}
#endif
			}
		       return -2;
	       }

	       incode = code;

	       if (code >= max_code) {
		       *sp++ = firstcode;
		       code = oldcode;
	       }

	       while (code >= clear_code) {
		       *sp++ = table[1][code];
		       if (code == table[0][code])
			{
#if 0
			       ERROR("circular table entry BIG ERROR");
#endif
			       return(code);
			}
			if ((int)sp >= ((int)stack + sizeof(stack)))
			{
#if 0
			       ERROR("circular table STACK OVERFLOW!");
#endif
			       return(code);
			}
		       code = table[0][code];
	       }

	       *sp++ = firstcode = table[1][code];

	       if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
		       table[0][code] = oldcode;
		       table[1][code] = firstcode;
		       ++max_code;
		       if ((max_code >= max_code_size) &&
			       (max_code_size < (1<<MAX_LWZ_BITS))) {
			       max_code_size *= 2;
			       ++code_size;
		       }
	       }

	       oldcode = incode;

	       if (sp > stack)
		       return *--sp;
	}
	return code;
}


static unsigned char *
ReadImage(FILE *fd, int len, int height, XColor *colrs, int cmapSize,
		unsigned char cmap[3][MAXCOLORMAPSIZE], int gray,
		int interlace, int ignore)
{
	unsigned char	*dp, c;	
	int		v;
	int		xpos = 0, ypos = 0/*, pass = 0*/;
	unsigned char 	*image;

	/*
	**  Initialize the Compression routines
	*/
	if (! ReadOK(fd,&c,1))
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "EOF / read error on image data\n");
		}
#endif

		return(NULL);
	}

	initLWZ(c);

	/*
	**  If this is an "uninteresting picture" ignore it.
	*/
	if (ignore) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "skipping image...\n" );
#endif
		while (readLWZ(fd) >= 0)
			;
		return(NULL);
	}

	image = (unsigned char *)calloc(len * height, sizeof(char));
	if (image == NULL)
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "Cannot allocate space for image data\n");
		}
#endif

		return(NULL);
	}

	for (v = 0; v < MAXCOLORMAPSIZE; v++) {
		colrs[v].red = colrs[v].green = colrs[v].blue = 0;
		colrs[v].pixel = v;
		colrs[v].flags = DoRed|DoGreen|DoBlue;
	}
	for (v = 0; v < cmapSize; v++) {
		colrs[v].red   = cmap[CM_RED][v]   * 0x101;
		colrs[v].green = cmap[CM_GREEN][v] * 0x101;
		colrs[v].blue  = cmap[CM_BLUE][v]  * 0x101;
	}

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "reading %d by %d%s GIF image\n",
			len, height, interlace ? " interlaced" : "" );
#endif

	if (interlace) {
		int     i;
		int     pass = 0, step = 8;

		for (i = 0; i < height; i++) {
			if (ypos < height)
			{
				dp = &image[len * ypos];
				for (xpos = 0; xpos < len; xpos++) {
					if ((v = readLWZ(fd)) < 0)
						goto fini;

					*dp++ = v;
				}
			}
			if ((ypos += step) >= height) {
				if (pass++ > 0)
					step /= 2;
				ypos = step / 2;
			}
		}
	} else {
		dp = image;
		for (ypos = 0; ypos < height; ypos++) {
			for (xpos = 0; xpos < len; xpos++) {
				if ((v = readLWZ(fd)) < 0)
					goto fini;

				*dp++ = v;
			}
		}
	}

fini:
	if (readLWZ(fd)>=0)
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr,"too much input data, ignoring extra...");
		}
#endif
	}

	return(image);
}

