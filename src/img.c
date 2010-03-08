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
#include "gui.h"
#include "img.h"
#include "mo-www.h"
#include "globalhist.h"
#include "picread.h"
#include "libhtmlw/HTML.h"
#include "cci.h"
extern int cci_event;

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

#include "bitmaps/gopher_image.xbm"
#include "bitmaps/gopher_movie.xbm"
#include "bitmaps/gopher_menu.xbm"
#include "bitmaps/gopher_text.xbm"
#include "bitmaps/gopher_sound.xbm"
#include "bitmaps/gopher_index.xbm"
#include "bitmaps/gopher_telnet.xbm"
#include "bitmaps/gopher_binary.xbm"
#include "bitmaps/gopher_unknown.xbm"

ImageInfo *scaleImage(ImageInfo *img_data,char *width,char *height);

static ImageInfo *gopher_image = NULL;
static ImageInfo *gopher_movie = NULL;
static ImageInfo *gopher_menu = NULL;
static ImageInfo *gopher_text = NULL;
static ImageInfo *gopher_sound = NULL;
static ImageInfo *gopher_index = NULL;
static ImageInfo *gopher_telnet = NULL;
static ImageInfo *gopher_binary = NULL;
static ImageInfo *gopher_unknown = NULL;


/* for selective image loading */
#define blank_width 8
#define blank_height 8
static unsigned char blank_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
static ImageInfo *blank = NULL;

extern char **imagekill_sites;

/*******************************/


/* Defined in gui.c */
extern char *cached_url;
extern mo_window *current_win;
/* Defined in gui-documents.c */
extern int interrupted;
extern int loading_inlined_images;
extern int installed_colormap;
extern Colormap installed_cmap;

/*
          (dsp, DefaultRootWindow (dsp), \
*/

#if defined(__STDC__) || defined(__sgi)
#define MAKE_IMGINFO_FROM_BITMAP(name) \
  if (!name) \
    { \
      name = (ImageInfo *)malloc (sizeof (ImageInfo)); \
      name->ismap = 0; \
      name->width = name##_width; \
      name->height = name##_height; \
      name->image_data = NULL; \
      /* Bandaid for bug in Eric's code. */ \
      name->internal = 1; \
      name->transparent = 0; \
      name->image = \
        XCreatePixmapFromBitmapData \
          (XtDisplay(swin), XtWindow(view), \
           name##_bits,  \
           name##_width, name##_height, \
           fg_pixel, bg_pixel, DefaultDepth(dsp, DefaultScreen(dsp))); \
    } \
  if (!name->image) \
    { \
      name->image = \
        XCreatePixmapFromBitmapData \
          (XtDisplay(swin), XtWindow(view), \
           name##_bits,  \
           name##_width, name##_height, \
           fg_pixel, bg_pixel, DefaultDepth(dsp, DefaultScreen(dsp))); \
    }
#else
#define MAKE_IMGINFO_FROM_BITMAP(name) \
  if (!name) \
    { \
      name = (ImageInfo *)malloc (sizeof (ImageInfo)); \
      name->ismap = 0; \
      name->width = name/**/_width; \
      name->height = name/**/_height; \
      name->image_data = NULL; \
      /* Bandaid for bug in Eric's code. */ \
      name->internal = 1; \
      name->transparent = 0; \
      name->image = \
        XCreatePixmapFromBitmapData \
          (XtDisplay(swin), XtWindow(view), \
           name/**/_bits,  \
           name/**/_width, name/**/_height, \
           fg_pixel, bg_pixel, DefaultDepth(dsp, DefaultScreen(dsp))); \
    } \
  if (!name->image) \
    { \
      name->image = \
        XCreatePixmapFromBitmapData \
          (XtDisplay(swin), XtWindow(view), \
           name/**/_bits,  \
           name/**/_width, name/**/_height, \
           fg_pixel, bg_pixel, DefaultDepth(dsp, DefaultScreen(dsp))); \
    }
#endif /* __STDC__ */

#define RETURN_IMGINFO_FROM_BITMAP(x) \
{ \
  MAKE_IMGINFO_FROM_BITMAP(x); \
  if (cci_event) MoCCISendEventOutput(IMAGE_LOADED); \
  return x; \
}

/* ------------------------------------------------------------------------ */


ImageInfo *scaleImage(ImageInfo *img_data,char *width,char *height) {
/*
int widthInt,heightInt;

        if (!width) {
                widthInt=img_data->width;
	}
        else {
                if (strchr(width,'%')) {
                        widthInt=(atoi(width)/100)*img_data->width;
		}
                else {
                        widthInt=atoi(width);
		}
	}

        if (!height) {
                heightInt=img_data->height;
	}
        else {
                heightInt=atoi(height);
	}

        for (i=0; i<256; i++) {
                convertRed[i]=(byte)img_data->reds[i];
                convertGreen[i]=(byte)img_data->greens[i];
                convertBlue[i]=(byte)img_data->blues[i];
	}

        img_data->image_data=SmoothResize(img_data->image_data,0,
                                          img_data->width,img_data->height,
                                          widthInt,heightInt,
                                          convertRed,convertGreen,convertBlue,
                                          newRed,newGreen,newBlue,256);
*/

	return NULL;
}

/* ------------------------------------------------------------------------ */

unsigned char nums[]={
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128
};

/* Cache Load Hack: (BJS)
   char *ir_hack_string, *diskfile, *url;
   sprintf(ir_hack_string,"%s\0%s",diskfile,url);
   ImageResolve(NULL,ir_hack_string,0,NULL,NULL);
   */

/* Width and Height Hack: (SWP)
	Added "width" and "height" to the parameter list. These are used to
	resize the image when we are done decoding. This'll be fun when we
	start processing on a line-by-line basis.
	If NULLs are passed, the width and height are not used.
	*/

/* Image resolution function. */
/*static*/
ImageInfo *ImageResolve (Widget w, char *src, int noload, char *wid, char *hei)
{
    extern Widget view; /*hw->html.view*/
    Widget swin = current_win->scrolled_win;
    int i, cnt;
    unsigned char *data;
    unsigned char *bg_map;
    unsigned char *bgptr;
    unsigned char *cptr;
    unsigned char *ptr;
    int width, height;
    int Used[256];
    XColor colrs[256];
    ImageInfo *img_data;
/*  char *txt;*/
    int widthbyheight = 0;
    char *fnam;
    int rc;
    int bg, bg_red, bg_green, bg_blue;
#ifdef HAVE_HDF
    int ishdf = 0;
    char *hdfref = NULL;
#endif
    mo_window *win = NULL;
    extern int Vclass;
    static Pixel fg_pixel, bg_pixel;
    static int done_fetch_colors = 0;
    int j,bcnt;

  /* Cache Load Hack - BJS 1/18/96 */ /* Fixed - SWP 02/01/96 */
    if(!w) {
        fnam = src;
        while(*src != '\n') src++;
        *src='\0';
        src++;
        
        goto stuffcache;
    }
          
#ifndef DISABLE_TRACE
    if (srcTrace)
        fprintf (stderr,
                 "[ImageResolve] I'm entering, src '%s', noload %d!\n",
                 src, noload);
#endif
  
    if (!src)
        return NULL;

    if (current_win && current_win->scrolled_win == w)
    {
#ifndef DISABLE_TRACE
        if (srcTrace)
            fprintf (stderr, "[ImageResolve] already have current_win\n");
#endif

        goto foundit;
    }

    while (win = mo_next_window (win))
    {
        if (win->scrolled_win == w)
        {
            mo_set_current_cached_win (win);
            
            cached_url = win->current_node ? win->current_node->url : "lose";
            win->cached_url = cached_url;
          
            goto foundit;
        }
    }
  
  /* Shit outta luck. */
#ifndef DISABLE_TRACE
    if (srcTrace)
        fprintf (stderr, "[ImageResolve] aaaaaaaaaaaaaaaaaaaaagh\n");
#endif

    return NULL;
  
foundit:
#ifndef DISABLE_TRACE
    if (srcTrace)
        fprintf (stderr, "[ImageResolve] hello! win 0x%08x\n",
                 win);
#endif


        /* Here is one place we do image_kill */
    if(src){
        if(imagekill_sites != NULL) {
            for(i = 0; imagekill_sites[i]; i++) {
                if(strstr(cached_url, imagekill_sites[i])) {
                    extern Widget view; /*hw->html.view*/
                    Widget swin = current_win->scrolled_win;
              
                    if (!done_fetch_colors){
                        if (!view)
                            return NULL;
                            /* First, go fetch the pixels. */
                        XtVaGetValues(view,
                                      XtNforeground, &fg_pixel,
                                      XtNbackground, &bg_pixel,
                                      NULL);
                        done_fetch_colors = 1;
                    }    
                    RETURN_IMGINFO_FROM_BITMAP(blank);
                }
            }
        }
    }
  
  
  /* Internal images. */
  if (strncmp (src, "internal-", 9) == 0)
    {    

      if (!done_fetch_colors)
        {

          if (!view)
            return NULL;
          
          /* First, go fetch the pixels. */
	  XtVaGetValues(view,
			XtNforeground, &fg_pixel,
			XtNbackground, &bg_pixel,
			NULL);

          done_fetch_colors = 1;
        }

      if (strcmp (src, "internal-gopher-image") == 0)
        RETURN_IMGINFO_FROM_BITMAP(gopher_image);
      if (strcmp (src, "internal-gopher-movie") == 0)
        RETURN_IMGINFO_FROM_BITMAP(gopher_movie);
      if (strcmp (src, "internal-gopher-menu") == 0)
        RETURN_IMGINFO_FROM_BITMAP(gopher_menu);
      if (strcmp (src, "internal-gopher-text") == 0)
        RETURN_IMGINFO_FROM_BITMAP(gopher_text);
      if (strcmp (src, "internal-gopher-sound") == 0)
        RETURN_IMGINFO_FROM_BITMAP(gopher_sound);
      if (strcmp (src, "internal-gopher-index") == 0)
        RETURN_IMGINFO_FROM_BITMAP(gopher_index);
      if (strcmp (src, "internal-gopher-telnet") == 0)
        RETURN_IMGINFO_FROM_BITMAP(gopher_telnet);
      if (strcmp (src, "internal-gopher-binary") == 0)
        RETURN_IMGINFO_FROM_BITMAP(gopher_binary);
      if (strcmp (src, "internal-gopher-unknown") == 0)
        RETURN_IMGINFO_FROM_BITMAP(gopher_unknown);
    }

#ifdef HAVE_HDF
  /* Incoming HDF image references will only be anchors,
     so we can determine that they are in fact HDF image references
     by doing strncmp here. */
  if (strncmp (src, "#hdfref;", 8) == 0)
    ishdf = 1;

  /* OK, src is the URL we have to go hunt down.
     First, we go get it. */
  /* We can use cached_url here, since we set it in do_window_text. */
  if (ishdf)
    {
      hdfref = &(src[8]);
      src = mo_url_canonicalize_keep_anchor (src, cached_url);
    }
  else
#endif

stuffcache:
  src = mo_url_canonicalize (src, cached_url);

  /* Go see if we already have the image info hanging around. */
  img_data = mo_fetch_cached_image_data (src);
  if (img_data && img_data->image_data)
  {
	unsigned long bg_pixel;
	XColor tmpcolr;

	if (img_data->bg_index>=0) {
		/* This code copied from xpmread.c.  I could almost
		 * delete the code from there, but I suppose an XPM
		 * file could pathalogially have multiple transparent
		 * colour indicies. -- GWP
		 */
		XtVaGetValues(view, XtNbackground, &bg_pixel, NULL);
		tmpcolr.pixel = bg_pixel;
		XQueryColor(XtDisplay(view),
			    (installed_colormap ?
			     installed_cmap :
			     DefaultColormap(XtDisplay(view), DefaultScreen(XtDisplay(view)))),
			    &tmpcolr);
		img_data->reds[img_data->bg_index]=tmpcolr.red;
		img_data->greens[img_data->bg_index]=tmpcolr.green;
		img_data->blues[img_data->bg_index]=tmpcolr.blue;
	}

        img_data->src = strdup(src);
	free (src);
	if (cci_event) MoCCISendEventOutput(IMAGE_LOADED);

	return (ImageInfo *)img_data;
/*
	if (!wid && !hei) {
		return (ImageInfo *)img_data;
	}

	return(scaleImage(img_data,wid,hei));
*/
  }

  /* If we don't have the image cached and noload is high,
     then just return NULL to avoid doing a network load. */
  /* Also return if interrupted is high. */
  if (noload || interrupted)
    {
#ifndef DISABLE_TRACE
      if (srcTrace)
        fprintf (stderr, "RETURNING Null noload %d interrupted %d\n",
                 noload, interrupted);
#endif
      free (src);

      return NULL;
    }

    
#ifndef DISABLE_TRACE
  if (srcTrace)
    fprintf (stderr, "GOING ON THROUGH...\n");
#endif

  /*
   * No transparent background by default
   */
  bg = -1;
  bg_map = NULL;
#ifdef HAVE_HDF
  if (ishdf)
    {
      img_data =
        (ImageInfo *)hdfGrokImage 
          (mo_hdf_fetch_local_filename (src), 
           hdfref,
           &bg);

#ifndef DISABLE_TRACE
      if (srcTrace)
        fprintf (stderr, "[ImageResolve] Did hdfGrokImage, got back 0x%08x\n", 
                 img_data);
#endif

      if (!img_data)
        return NULL;

      /* Yet another bandaid... */
      img_data->internal = 0;

      /* Fill out colrs array. */
      for (i = 0; i < 256; i++)
        {
          colrs[i].red = img_data->reds[i];
          colrs[i].green = img_data->greens[i];
          colrs[i].blue = img_data->blues[i];
          colrs[i].pixel = i;
          colrs[i].flags = DoRed|DoGreen|DoBlue;
        }

      if (bg >= 0)
          {
              bg_red = colrs[bg].red;
              bg_green = colrs[bg].green;
              bg_blue = colrs[bg].blue;
              bg_map = (unsigned char *)malloc(img_data->width * img_data->height);
          }
  }
  else
#endif /* HAVE_HDF */
  {

            /* if w is NULL we're stuffing the cache with our own info...
                BJS */
        if(w) {
                
		/* We have to load the image. */
		fnam = mo_tmpnam(src);

		interrupted = 0;
		rc = mo_pull_er_over_virgin (src, fnam);
		if (!rc)
		{
#ifndef DISABLE_TRACE
			if (srcTrace)
			fprintf (stderr, "mo_pull_er_over_virgin returned %d; bonging\n",
				 rc);
#endif
			free (fnam);

			return NULL;
		}
      
#ifndef DISABLE_TRACE
		if (srcTrace)
		fprintf (stderr, 
			 "[ImageResolve] Got through mo_pull_er_over_virgin, rc %d\n", 
			 rc);
#endif

#if 0
		/* This causes problems. */
		{
			/* Check use_this_url_instead from HTAccess.c. */
			extern char *use_this_url_instead;
			if (use_this_url_instead)
			{
				free (src);
				src = use_this_url_instead;
			}
		}
#endif

		/*********************************************/
		/* Send it through CCI if need be            */
		MoCCISendBrowserViewFile(src, "unknown", fnam);
	}
        
	data = ReadBitmap(fnam, &width, &height, colrs, &bg);

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf (stderr,
			 "[ImageResolve] Did ReadBitmap, got 0x%08x\n", data);
#endif

	/* if we have a transparent background, prepare for it */
	if ((bg >= 0)&&(data != NULL))
        {
		unsigned long bg_pixel;
		XColor tmpcolr;

		/* This code copied from xpmread.c.  I could almost
		 * delete the code from there, but I suppose an XPM
		 * file could pathalogially have multiple transparent
		 * colour indicies. -- GWP
		 */
		XtVaGetValues(view, XtNbackground, &bg_pixel, NULL);
		tmpcolr.pixel = bg_pixel;
		XQueryColor(XtDisplay(view),
			    (installed_colormap ?
			     installed_cmap :
			     DefaultColormap(XtDisplay(view), DefaultScreen(XtDisplay(view)))),
			    &tmpcolr);

		bg_red = colrs[bg].red = tmpcolr.red;
		bg_green = colrs[bg].green = tmpcolr.green;
		bg_blue = colrs[bg].blue = tmpcolr.blue;
		colrs[bg].flags = DoRed|DoGreen|DoBlue;

		bg_map = (unsigned char *)malloc(width * height);
	}
      
	/* Now delete the file. */
	unlink(fnam); 
	{
		char *hfnam = (char *)malloc ((strlen (fnam) + strlen(".html") + 5) * sizeof (char));
		sprintf (hfnam, "%s.html", fnam);
		unlink(hfnam); 
		free(hfnam);
	}

	if (w) {
		free (fnam);
	}

	if (data == NULL)
	{
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf (stderr, "[ImageResolve] data == NULL; punting...\n");
#endif

		return NULL;
	}

{
int found_bg=0;

	if (data!=NULL) {
		for (i=0; i<width*height; i++) {
			if ((int)(data[i])==bg) {
				found_bg=1;
				break;
			}
		}
		if (!found_bg) {
			bg=(-1);
		}
	}
}

      img_data = (ImageInfo *)malloc(sizeof(ImageInfo));
      if ((bg >= 0)&&(data != NULL) &&
	  get_pref_boolean(eCLIPPING) &&
	  (get_pref_int(eMAX_CLIPPING_SIZE_X)==(-1) ||
	   get_pref_int(eMAX_CLIPPING_SIZE_X)>=width) &&
	  (get_pref_int(eMAX_CLIPPING_SIZE_Y)==(-1) ||
	   get_pref_int(eMAX_CLIPPING_SIZE_Y)>=height)) {
	img_data->transparent=1;
	img_data->clip_data=(unsigned char *)malloc(width * height);
	memset(img_data->clip_data,0,(width*height));
	img_data->bg_index=bg;
      }
      else {
	img_data->transparent=0;
	img_data->clip_data=NULL;
	img_data->bg_index=(-1);
      }

      img_data->width = width;
      img_data->height = height;
      img_data->image_data = data;
      img_data->image = 0;
      img_data->clip = 0;
      img_data->src = strdup(src);
      /* Bandaid for bug afflicting Eric's code, apparently. */
      img_data->internal = 0;
    }

	widthbyheight = img_data->width * img_data->height;

	/* Fill out used array. */
	for (i=0; i < 256; i++) {
		Used[i] = 0;
	}

	cnt = 1;
	bgptr = bg_map;
	cptr = img_data->clip_data;
	ptr = img_data->image_data;

	/*This sets the bg map and also creates bitmap data for the
		clip mask when there is a bg image */
	for (i=0; i<img_data->height; i++) {
		for (j=0,bcnt=0; j<img_data->width; j++) {
			if (Used[(int)*ptr] == 0) {
				Used[(int)*ptr] = cnt;
				cnt++;
			}
			if (bg>=0) {
				if (*ptr == bg) {
					*bgptr = 1;
				}
				else {
					*bgptr = 0;
					if (img_data->transparent) {
						*cptr += nums[(bcnt % 8)];
					}
				}
				if (img_data->transparent &&
				    ((bcnt % 8)==7 ||
				     j==(img_data->width-1))) {
					cptr++;
				}
				bgptr++;
				bcnt++;
			}
			ptr++;
		}
	}
	cnt--;

  /*
   * If the image has too many colors, apply a median cut algorithm to
   * reduce the color usage, and then reprocess it.
   * Don't cut colors for direct mapped visuals like TrueColor.
   */
  if ((cnt > get_pref_int(eCOLORS_PER_INLINED_IMAGE))&&(Vclass != TrueColor))
    {
      MedianCut(img_data->image_data, &img_data->width, 
                &img_data->height, colrs, 256, 
                get_pref_int(eCOLORS_PER_INLINED_IMAGE));
      
      for (i=0; i < 256; i++)
        Used[i] = 0;
      cnt = 1;
      ptr = img_data->image_data;
      for (i=0; i < widthbyheight; i++)
        {
          if (Used[(int)*ptr] == 0)
            {
              Used[(int)*ptr] = cnt;
              cnt++;
            }
          ptr++;
        }
      cnt--;

      /* if we had a transparent bg, MedianCut used it.  Get a new one */
      if (bg >= 0)
      {
	cnt++;
	bg = 256;
      }
    }

  img_data->num_colors = cnt;

#ifdef HAVE_HDF
  if (!ishdf)
#endif
    {
      img_data->reds = (int *)malloc(sizeof(int) * cnt);
      img_data->greens = (int *)malloc(sizeof(int) * cnt);
      img_data->blues = (int *)malloc(sizeof(int) * cnt);
    }

  /* bg is not set in here if it gets munged by MedCut */
  for (i=0; i < 256; i++)
    {
      int indx;
      
      if (Used[i] != 0)
        {
          indx = Used[i] - 1;
          img_data->reds[indx] = colrs[i].red;
          img_data->greens[indx] = colrs[i].green;
          img_data->blues[indx] = colrs[i].blue;
	  /* squeegee in the background color */
	  if ((bg >= 0)&&(i == bg))
            {
              img_data->reds[indx] = bg_red;
              img_data->greens[indx] = bg_green;
              img_data->blues[indx] = bg_blue;
	      img_data->bg_index=indx;
            }
        }
    }

    /* if MedianCut ate our background, add the new one now. */
    if (bg == 256)
      {
	img_data->reds[cnt - 1] = bg_red;
	img_data->greens[cnt - 1] = bg_green;
	img_data->blues[cnt - 1] = bg_blue;
	img_data->bg_index=(cnt-1);
      }
  
  bgptr = bg_map;
  cptr = img_data->clip_data;
  ptr = img_data->image_data;
  for (i=0; i < widthbyheight; i++)
    {
      *ptr = (unsigned char)(Used[(int)*ptr] - 1);
      /* if MedianCut ate the background, enforce it here */
      if (bg == 256)
        {
          if (*bgptr)
            {
              *ptr = (unsigned char)(cnt - 1);
            }
          bgptr++;
        }
      ptr++;
    }

  /* free the background map if we have one */
  if (bg_map != NULL)
  {
    free (bg_map);
  }

#ifndef DISABLE_TRACE
  if (srcTrace)
    fprintf (stderr, "[ImageResolve] Doing mo_cache_data on '%s', 0x%08x\n",
             src, img_data);
#endif

  mo_cache_data (src, (void *)img_data, 0);

  free (src);

#ifndef DISABLE_TRACE
  if (srcTrace)
    fprintf (stderr, "[ImageResolve] Leaving...\n");
#endif

  if (cci_event) MoCCISendEventOutput(IMAGE_LOADED);

  return img_data;
/*
  if (!wid && !hei) {
	return img_data;
  }

  return(scaleImage(img_data,wid,hei));
*/
}

static ImageInfo *DelayedImageResolve (Widget w, char *src)
{
  ImageInfo *img;

#ifndef DISABLE_TRACE
  if (srcTrace)
    fprintf (stderr, "[DelayedImageResolve] I'm entering, src '%s'\n", src);
#endif

  interrupted = 0;

  loading_inlined_images = 1;
  img = ImageResolve (w, src, 0, NULL, NULL);
  loading_inlined_images = 0;

  if (interrupted == 1)
    {
#ifndef DISABLE_TRACE
      if (srcTrace)
        fprintf (stderr, "[DelayedImageResolve] Interrupted...\n");
#endif

      interrupted = 0;
    }

#ifndef DISABLE_TRACE
  if (srcTrace)
    fprintf (stderr, "[DelayedImageResolve] Returning 0x%08x\n", img);
#endif

/* need this here now...but may make other stuff look cheeeezy -- SWP */
  mo_gui_done_with_icon ();
  mo_gui_check_icon(-1);

  return img;
}

mo_status mo_free_image_data (void *ptr)
{
  ImageInfo *img = (ImageInfo *)ptr;
  Widget swin=current_win->scrolled_win;

#ifndef DISABLE_TRACE
  if (srcTrace)
    fprintf (stderr, "[mo_free_image_info] Freeing 0x%08x\n", img);
#endif

  if (!img)
    return mo_fail;

  if (img->reds)
    {
      free (img->reds);
      img->reds = NULL;
    }
  if (img->greens)
    {
      free (img->greens);
      img->greens = NULL;
    }
  if (img->blues)
    {
      free (img->blues);
      img->blues = NULL;
    }
  if (img->image_data)
    {
      free (img->image_data);
      img->image_data = NULL;
    }
  if (img->clip_data) {
      free (img->clip_data);
      img->clip_data = NULL;
  }
  if (img->src) {
	free(img->src);
	img->src=NULL;
  }
  if (img->image!=None) {
	XFreePixmap(XtDisplay(swin),img->image);
	img->image=None;
  }
  if (img->clip!=None) {
	XFreePixmap(XtDisplay(swin),img->clip);
	img->clip=None;
  }

  return mo_succeed;
}

mo_status mo_register_image_resolution_function (mo_window *win)
{
  XmxSetArg (WbNresolveImageFunction, (long)ImageResolve);
  XmxSetArg (WbNresolveDelayedImage, (long)DelayedImageResolve);
  XmxSetValues (win->scrolled_win);
  return mo_succeed;
}

