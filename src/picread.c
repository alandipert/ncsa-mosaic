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
#include "mosaic.h"
#include "picread.h"
#include "gifread.h"
#include "xpmread.h"

#ifdef HAVE_JPEG
#include "readJPEG.h"
#endif

#ifdef HAVE_PNG
#include "readPNG.h"
#endif

#include <X11/Xos.h>

#define DEF_BLACK       BlackPixel(dsp, DefaultScreen(dsp))
#define DEF_WHITE       WhitePixel(dsp, DefaultScreen(dsp))
#define	MAX_LINE	81


/*extern unsigned char *ReadGIF();
extern unsigned char *ReadXpm3Pixmap();
extern unsigned char *ReadJPEG();*/


extern Display *dsp;

extern int installed_colormap;
extern Colormap installed_cmap;

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

char nibMask[8] = {
	1, 2, 4, 8, 16, 32, 64, 128
};



unsigned char *ReadXpmPixmap(fp, datafile, w, h, colrs, Colors, CharsPP)
FILE *fp;
char *datafile;
int *w, *h;
XColor *colrs;
int Colors, CharsPP;
{
	unsigned char *pixels;
	char **Color_Vals;
	XColor tmpcolr;
	int i, j, k;
	int /*value,*/ found;
	char line[BUFSIZ], name_and_type[MAX_LINE];
	unsigned char *dataP;
	unsigned char *bitp;
	int tchar;
	char *t;
	char *t2;

	if (Colors == 0)
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "Can't find Colors.\n");
		}
#endif

		return((unsigned char *)NULL);
	}
	if (*w == 0)
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "Can't read image.\n");
		}
#endif

		return((unsigned char *)NULL);
	}
	if (*h == 0)
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "Can't read image.\n");
		}
#endif

		return((unsigned char *)NULL);
	}

	Color_Vals = (char **)malloc(sizeof(char *) * Colors);
	for (i=0; i<Colors; i++)
	{
		tchar = getc(fp);
		while ((tchar != '"')&&(tchar != EOF))
		{
			tchar = getc(fp);
		}
		Color_Vals[i] = (char *)malloc(sizeof(char) * (CharsPP + 1));
		j = 0;
		tchar = getc(fp);
		while ((tchar != '"')&&(tchar != EOF)&&(j < CharsPP))
		{
			Color_Vals[i][j] = (char)tchar;
			tchar = getc(fp);
			j++;
		}
		Color_Vals[i][j] = '\0';
		if (tchar != '"')
		{
			tchar = getc(fp);
			while ((tchar != '"')&&(tchar != EOF))
			{
				tchar = getc(fp);
			}
		}
		tchar = getc(fp);
		while ((tchar != '"')&&(tchar != EOF))
		{
			tchar = getc(fp);
		}
		j = 0;
		tchar = getc(fp);
		while ((tchar != '"')&&(tchar != EOF))
		{
			line[j] = (char)tchar;
			tchar = getc(fp);
			j++;
		}
		line[j] = '\0';
		XParseColor(dsp, (installed_colormap ?
				  installed_cmap :
				  DefaultColormap(dsp, DefaultScreen(dsp))),
			line, &tmpcolr);
		colrs[i].red = tmpcolr.red;
		colrs[i].green = tmpcolr.green;
		colrs[i].blue = tmpcolr.blue;
		colrs[i].pixel = i;
		colrs[i].flags = DoRed|DoGreen|DoBlue;
	}
	for (i=Colors; i<256; i++)
	{
		colrs[i].red = 0;
		colrs[i].green = 0;
		colrs[i].blue = 0;
		colrs[i].pixel = i;
		colrs[i].flags = DoRed|DoGreen|DoBlue;
	}
	tchar = getc(fp);
	while ((tchar != ';')&&(tchar != EOF))
	{
		tchar = getc(fp);
	}

	for ( ; ; )
	{
		if (!(fgets(line, MAX_LINE, fp)))
		{
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr, "Can't find Pixels\n");
			}
#endif

			return((unsigned char *)NULL);
		}
		else if (sscanf(line,"static char * %s = {",name_and_type) == 1)
		{
			if ((t = strrchr(name_and_type, '_')) == NULL)
			{
				t = name_and_type;
			}
			else
			{
				t++;
			}
			if ((t2 = strchr(name_and_type, '[')) != NULL)
			{
				*t2 = '\0';
			}
			if (!strcmp("pixels", t))
			{
				break;
			}
		}
	}
	pixels = (unsigned char *)malloc((*w) * (*h));
	if (pixels == NULL)
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "Not enough memory for data.\n");
		}
#endif

		return((unsigned char *)NULL);
	}

	line[0] = '\0';
	t = line;
	dataP = pixels;
	tchar = getc(fp);
	while ((tchar != '"')&&(tchar != EOF))
	{
		tchar = getc(fp);
	}
	tchar = getc(fp);
	for (j=0; j<(*h); j++)
	{
		for (i=0; i<(*w); i++)
		{
			k = 0;
			while ((tchar != '"')&&(tchar != EOF)&&(k < CharsPP))
			{
				line[k] = (char)tchar;
				tchar = getc(fp);
				k++;
			}
			if ((k == 0)&&(tchar == '"'))
			{
				tchar = getc(fp);
				while ((tchar != '"')&&(tchar != EOF))
				{
					tchar = getc(fp);
				}
				k = 0;
				tchar = getc(fp);
				while ((tchar != '"')&&(tchar != EOF)&&
					(k < CharsPP))
				{
					line[k] = (char)tchar;
					tchar = getc(fp);
					k++;
				}
			}
			line[k] = '\0';
			found = 0;
			for (k=0; k<Colors; k++)
			{
				if (strncmp(Color_Vals[k], line, CharsPP) == 0)
				{
					*dataP++ = (unsigned char)k;
					found = 1;
					break;
				}
			}
			if (found == 0)
			{
#ifndef DISABLE_TRACE
				if (srcTrace) {
					fprintf(stderr, "Invalid Pixel (%2s) in file %s\n", line, datafile);
				}
#endif

				*dataP++ = (unsigned char)0;
			}
		}
	}

	bitp = pixels;
	for (i=0; i<((*w) * (*h)); i++)
	{
		if ((int)*bitp > (256 - 1))
			*bitp = (unsigned char)0;
		bitp++;
	}

	for (i=0; i<Colors; i++)
	{
		free((char *)Color_Vals[i]);
	}
	free((char *)Color_Vals);
	return(pixels);
}


unsigned char *ReadXbmBitmap(fp, datafile, w, h, colrs)
FILE *fp;
char *datafile;
int *w, *h;
XColor *colrs;
{
	char line[MAX_LINE], name_and_type[MAX_LINE];
	char *t;
	char *t2;
	unsigned char *ptr, *dataP;
	int bytes_per_line, version10p, raster_length, padding;
	int i, bytes, temp, value;
	int Ncolors, charspp, xpmformat;
        static unsigned long fg_pixel, bg_pixel;
        static int done_fetch_colors = 0;
        extern XColor fg_color, bg_color;
        extern Widget view;
        extern int Vclass;
	int blackbit;
	int whitebit;

        if (!done_fetch_colors)
          {
            /* First, go fetch the pixels. */
            XtVaGetValues (view, XtNforeground, &fg_pixel,
                         XtNbackground, &bg_pixel, NULL);
            
            /* Now, load up fg_color and bg_color. */
            fg_color.pixel = fg_pixel;
            bg_color.pixel = bg_pixel;
            
            /* Now query for the full color info. */
            XQueryColor 
              (XtDisplay (view), 
               (installed_colormap ?
		installed_cmap :
		DefaultColormap (XtDisplay (view),
                                DefaultScreen (XtDisplay (view)))),
               &fg_color);
            XQueryColor 
              (XtDisplay (view), 
               (installed_colormap ?
		installed_cmap :
		DefaultColormap (XtDisplay (view),
                                DefaultScreen (XtDisplay (view)))),
               &bg_color);

            done_fetch_colors = 1;

	    /*
	     * For a TrueColor visual, we can't use the pixel value as
	     * the color index because it is > 255.  Arbitrarily assign
	     * 0 to foreground, and 1 to background.
	     */
	    if ((Vclass == TrueColor) ||(Vclass == DirectColor))
	      {
		fg_color.pixel = 0;
		bg_color.pixel = 1;
	      }

          }

        if (get_pref_boolean(eREVERSE_INLINED_BITMAP_COLORS))
          {
            blackbit = bg_color.pixel;
            whitebit = fg_color.pixel;
          }
        else
          {
            blackbit = fg_color.pixel;
            whitebit = bg_color.pixel;
          }
  
	/*
	 * Error out here on visuals we can't handle so we won't core dump
	 * later.
	 */
	if (((blackbit > 255)||(whitebit > 255))&&(Vclass != TrueColor))
	  {
		fprintf(stderr, "Error:  cannot deal with default colormap that is deeper than 8, and not TrueColor\n");
                fprintf(stderr, "        If you actually have such a system, please notify mosaic-x@ncsa.uiuc.edu.\n");
                fprintf(stderr, "        We thank you for your support.\n");
		exit(1);
	  }

        if (get_pref_boolean(eREVERSE_INLINED_BITMAP_COLORS))
          {
            colrs[blackbit].red = bg_color.red;
            colrs[blackbit].green = bg_color.green;
            colrs[blackbit].blue = bg_color.blue;
            colrs[blackbit].pixel = bg_color.pixel;
            colrs[blackbit].flags = DoRed|DoGreen|DoBlue;
            
            colrs[whitebit].red = fg_color.red;
            colrs[whitebit].green = fg_color.green;
            colrs[whitebit].blue = fg_color.blue;
            colrs[whitebit].pixel = fg_color.pixel;
            colrs[whitebit].flags = DoRed|DoGreen|DoBlue;
          }
        else
          {
            colrs[blackbit].red = fg_color.red;
            colrs[blackbit].green = fg_color.green;
            colrs[blackbit].blue = fg_color.blue;
            colrs[blackbit].pixel = fg_color.pixel;
            colrs[blackbit].flags = DoRed|DoGreen|DoBlue;
            
            colrs[whitebit].red = bg_color.red;
            colrs[whitebit].green = bg_color.green;
            colrs[whitebit].blue = bg_color.blue;
            colrs[whitebit].pixel = bg_color.pixel;
            colrs[whitebit].flags = DoRed|DoGreen|DoBlue;
          }

	*w = 0;
	*h = 0;
	Ncolors = 0;
	charspp = 0;
	xpmformat = 0;
	for ( ; ; )
	{
		if (!(fgets(line, MAX_LINE, fp)))
			break;
		if (strlen(line) == (MAX_LINE - 1))
		{
#ifndef DISABLE_TRACE
			if (srcTrace) {
				fprintf(stderr, "Line too long.\n");
			}
#endif

			return((unsigned char *)NULL);
		}
		if (sscanf(line, "#define %s %d", name_and_type, &value) == 2)
		{
			if (!(t = strrchr(name_and_type, '_')))
				t = name_and_type;
			else
				t++;
			if (!strcmp("width", t))
				*w= value;
			if (!strcmp("height", t))
				*h= value;
			if (!strcmp("ncolors", t))
				Ncolors = value;
			if (!strcmp("pixel", t))
				charspp = value;
			continue;
		}
		if (sscanf(line, "static short %s = {", name_and_type) == 1)
		{
			version10p = 1;
			break;
		}
		else if (sscanf(line,"static char * %s = {",name_and_type) == 1)
		{
			xpmformat = 1;
			if (!(t = strrchr(name_and_type, '_')))
				t = name_and_type;
			else
				t++;
			if ((t2 = strchr(name_and_type, '[')) != NULL)
				*t2 = '\0';
			if (!strcmp("mono", t))
				continue;
			else
				break;
		}
		else if (sscanf(line, "static char %s = {", name_and_type) == 1)
		{
			version10p = 0;
			break;
		}
		else if (sscanf(line, "static unsigned char %s = {", name_and_type) == 1)
		{
			version10p = 0;
			break;
		}
		else
			continue;
	}
	if (xpmformat)
	{
		dataP = ReadXpmPixmap(fp, datafile, w, h, colrs, Ncolors, charspp);
		return(dataP);
	}
	if (*w == 0)
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "Can't read image.\n");
		}
#endif

		return((unsigned char *)NULL);
	}
	if (*h == 0)
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "Can't read image.\n");
		}
#endif

		return((unsigned char *)NULL);
	}
	padding = 0;
	if (((*w % 16) >= 1)&&((*w % 16) <= 8)&&version10p)
	{
		padding = 1;
	}
	bytes_per_line = ((*w + 7) / 8) + padding;
	raster_length =  bytes_per_line * *h;
	dataP = (unsigned char *)malloc((*w) * (*h));
	if (dataP == NULL)
	{
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr, "Not enough memory.\n");
		}
#endif

		return((unsigned char *)NULL);
	}
	ptr = dataP;
	if (version10p)
	{
		int cnt = 0;
		int lim = (bytes_per_line - padding) * 8;
		for (bytes = 0; bytes < raster_length; bytes += 2)
		{
			if (fscanf(fp, " 0x%x%*[,}]%*[ \r\n]", &value) != 1)
			{
#ifndef DISABLE_TRACE
				if (srcTrace) {
					fprintf(stderr, "Error scanning bits item.\n");
				}
#endif

				return((unsigned char *)NULL);
			}
			temp = value;
			value = temp & 0xff;
			for (i = 0; i < 8; i++)
			{
				if (cnt < (*w))
				{
					if (value & nibMask[i])
						*ptr++ = blackbit;
					else
						*ptr++ = whitebit;
				}
				if (++cnt >= lim)
					cnt = 0;
			}
			if ((!padding)||((bytes+2) % bytes_per_line))
			{
				value = temp >> 8;
				for (i = 0; i < 8; i++)
				{
					if (cnt < (*w))
					{
						if (value & nibMask[i])
							*ptr++ = blackbit;
						else
							*ptr++ = whitebit;
					}
					if (++cnt >= lim)
						cnt = 0;
				}
			}
		}
	}
	else
	{
		int cnt = 0;
		int lim = bytes_per_line * 8;
		for (bytes = 0; bytes < raster_length; bytes++)
		{
			if (fscanf(fp, " 0x%x%*[,}]%*[ \r\n]", &value) != 1)
			{
#ifndef DISABLE_TRACE
				if (srcTrace) {
					fprintf(stderr, "Error scanning bits item.\n");
				}
#endif

				return((unsigned char *)NULL);
			}
			for (i = 0; i < 8; i++)
			{
				if (cnt < (*w))
				{
					if (value & nibMask[i])
						*ptr++ = blackbit;
					else
						*ptr++ = whitebit;
				}
				if (++cnt >= lim)
					cnt = 0;
			}
		}
	}
	return(dataP);
}


unsigned char *ReadBitmap(datafile, w, h, colrs, bg)
char *datafile;
int *w, *h;
XColor *colrs;
int *bg;
{
    unsigned char *bit_data;
    FILE *fp;
    
    *bg = -1;
    
    /* Obviously this isn't going to work. */
    if ((datafile == NULL)||(datafile[0] == '\0'))
	{
	    fp = NULL;
	}
    else
	{
	    fp = fopen(datafile, "r");
	}
    
    if (fp != NULL)
	{
	    
	    bit_data = ReadGIF(fp, w, h, colrs, bg);
	    if (bit_data != NULL)
		{
		    if (fp != stdin) fclose(fp);
		    return(bit_data);
		}
	    rewind(fp);
	    
	    bit_data = ReadXbmBitmap(fp, datafile, w, h, colrs);
	    if (bit_data != NULL)
		{
		    if (fp != stdin) fclose(fp);
		    return(bit_data);
		}
	    rewind(fp);
	    
	    bit_data = ReadXpm3Pixmap(fp, datafile, w, h, colrs, bg);
	    if (bit_data != NULL)
		{
		    if (fp != stdin) fclose(fp);
		    return(bit_data);
		}
	    rewind(fp);
	    
#ifdef HAVE_PNG
/* I can't believe Mosaic works this way... - DXP */
/* I have to put this BEFORE ReadJPEG, because that code 
   screws up the file pointer by closing it if there is an error - go fig. */
	    bit_data = ReadPNG(fp, w, h, colrs);
	    if (bit_data != NULL) /* ie. it was able to read the image */
		{
		    if (fp != stdin) fclose(fp);
		    return(bit_data);
		}
	    rewind(fp);
#endif
#ifdef HAVE_JPEG
	    bit_data = ReadJPEG(fp, w, h, colrs);
	    if (bit_data != NULL)
		{
		    if (fp != stdin) fclose(fp);
		    return(bit_data);
		}
#endif

	}
    if ((fp != NULL) && (fp != stdin)) fclose(fp);
    return((unsigned char *)NULL);
}

