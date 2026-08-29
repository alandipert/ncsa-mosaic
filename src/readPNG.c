/* Changes for Mosaic-CK (C)2009 Cameron Kaiser */

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

/* Author: DXP 

 A lot of this is copied from the PNGLIB file example.c

 Modified:

    August   1995 - Glenn Randers-Pehrson <glennrp@arl.mil>
                    Changed dithering to use a 6x6x6 color cube.

    March 21 1996 - DXP
                    Fixed some interlacing problems.
                  
*/

#include "config.h"
#ifdef HAVE_PNG

#include <stdio.h>
#include <X11/Intrinsic.h>

#include "mosaic.h"
#include "readPNG.h"

#include <setjmp.h>

#define MAX(x,y)  (((x) > (y)) ? (x) : (y))

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

unsigned char *
ReadPNG(FILE *infile,int *width, int *height, XColor *colrs)
{

    unsigned char *pixmap;
    unsigned char *p;
    png_byte *q;

    png_struct *png_ptr;
    png_info *info_ptr;

    double screen_gamma;
    png_color palette[256]; 
    int num_palette;
    png_uint_16p hist;
    double file_gamma; 
    png_byte *png_pixels=NULL, **row_pointers=NULL;
    int i, j;

    unsigned int packets;

    png_color std_color_cube[216];

    
        /* first check to see if its a valid PNG file. If not, return. */
        /* we assume that infile is a valid filepointer */
    {
        int ret;
        png_byte buf[8];
        
        ret = fread(buf, 1, 8, infile);
        
        if(ret != 8)
            return 0;
        
        ret = png_check_sig(buf, 8);
        
        if(!ret)
            return(0);
    }

        /* OK, it is a valid PNG file, so let's rewind it, and start 
           decoding it */
    rewind(infile);

        /* allocate the structures */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr)
        return 0;
    /*
    info_ptr = (png_info *)malloc(sizeof(png_info));
    if(!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 0;
    }
    */ 
        /* Establish the setjmp return context for png_error to use. */
    if (setjmp(png_jmpbuf(png_ptr))) {
        
#ifndef DISABLE_TRACE
        if (srcTrace) {
            fprintf(stderr, "\n!!!libpng read error!!!\n");
        }
#endif

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        if(png_pixels != NULL)
            free((char *)png_pixels);
        if(row_pointers != NULL)
            free((png_byte **)row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        
        return 0;
    }

        /* initialize the structure */
    /* png_info_init(info_ptr); */ 
     info_ptr=png_create_info_struct (png_ptr);
    
        /* set up the input control */
    png_init_io(png_ptr, infile);
    
        /* read the file information */
    png_read_info(png_ptr, info_ptr);
    
        /* setup other stuff using the fields of png_info. */
    
    *width = (int)png_get_image_width(png_ptr,info_ptr);
    *height = (int)png_get_image_height(png_ptr,info_ptr);

    /* #ifndef DISABLE_TRACE */ 
#if 0     
    if (srcTrace) {
        fprintf(stderr,"\n\nBEFORE\nheight = %d\n", (int)png_ptr->width);
        fprintf(stderr,"width = %d\n", (int)png_ptr->height);
        fprintf(stderr,"bit depth = %d\n", info_ptr->bit_depth);
        fprintf(stderr,"color type = %d\n", info_ptr->color_type);
        fprintf(stderr,"compression type = %d\n", info_ptr->compression_type);
        fprintf(stderr,"filter type = %d\n", info_ptr->filter_type);
        fprintf(stderr,"interlace type = %d\n", info_ptr->interlace_type);
        fprintf(stderr,"num colors = %d\n",info_ptr->num_palette);
        fprintf(stderr,"rowbytes = %d\n", info_ptr->rowbytes);
    }
#endif


#if 0
        /* This handles alpha and transparency by replacing it with 
           a background value. */
        /* its #if'ed out for now cause I don't have anything to 
           test it with */
    {
      png_color_16 my_background,background;
	
        if (png_get_bKGD(png_ptr, info_ptr, &background) & PNG_INFO_bKGD)
            png_set_background(png_ptr, &background,
                               PNG_GAMMA_FILE, 1, 1.0);
        else
            png_set_background(png_ptr, &my_background,
                               PNG_GAMMA_SCREEN, 0, 1.0);
    }
#endif

        /* strip pixels in 16-bit images down to 8 bits */
    if (png_get_bit_depth(png_ptr,info_ptr) == 16)
        png_set_strip_16(png_ptr);


        /* If it is a color image then check if it has a palette. If not
           then dither the image to 256 colors, and make up a palette */
    if (png_get_color_type(png_ptr,info_ptr)==PNG_COLOR_TYPE_RGB ||
        png_get_color_type(png_ptr,info_ptr)==PNG_COLOR_TYPE_RGB_ALPHA) {

      /* replace with png_get_PLTE(png_ptr, info_ptr, &palette,
	 &num_palette); */ 
        if(!png_get_PLTE(png_ptr, info_ptr, &palette,
	 &num_palette)) {

#ifndef DISABLE_TRACE
            if (srcTrace) {
                fprintf(stderr,"dithering (RGB->palette)...\n");
            }
#endif
                /* if there is is no valid palette, then we need to make
                   one up */
            for(i=0;i<216;i++) {
                    /* 255.0/5 = 51 */
                std_color_cube[i].red=(i%6)*51;
                std_color_cube[i].green=((i/6)%6)*51;
                std_color_cube[i].blue=(i/36)*51;
            }

                /* this should probably be dithering to 
                   Rdata.colors_per_inlined_image colors */
	    /* EO: replaced png_set_dither with png_set_quantize to work with libpng 1.6 that ships with Ubuntu 22.04 LTS on 3 November 2024 */ 
            png_set_quantize(png_ptr, std_color_cube, 
                           216, 
                           216, NULL, 1);
            
        } else {
#ifndef DISABLE_TRACE
            if (srcTrace) {
                fprintf(stderr,"dithering (RGB->file supplied palette)...\n");
            }
#endif
	    /* replace with png_get_PLTE(png_ptr, info_ptr, &palette,
	       &num_palette); */
	    png_get_hIST(png_ptr, info_ptr, &hist);
	    /* EO: replaced png_set_dither with png_set_quantize */ 
            png_set_quantize(png_ptr, palette, 
                           num_palette,
                           get_pref_int(eCOLORS_PER_INLINED_IMAGE), 
                           hist, 1);
            
        }
    }

        /* PNG files pack pixels of bit depths 1, 2, and 4 into bytes as
           small as they can. This expands pixels to 1 pixel per byte, and
           if a transparency value is supplied, an alpha channel is
           built.*/
    if (png_get_bit_depth(png_ptr,info_ptr) < 8)
        png_set_packing(png_ptr);


        /* have libpng handle the gamma conversion */

    if (get_pref_boolean(eUSE_SCREEN_GAMMA)) { /*SWP*/
        if (png_get_bit_depth(png_ptr,info_ptr) != 16) {  /* temporary .. glennrp */
            screen_gamma=(double)(get_pref_float(eSCREEN_GAMMA));
            
#ifndef DISABLE_TRACE
            if (srcTrace) {
                fprintf(stderr,"screen gamma=%f\n",screen_gamma);
            }
#endif
            if (png_get_gAMA(png_ptr,info_ptr,&file_gamma) & PNG_INFO_gAMA) {
#ifndef DISABLE_TRACE
                if (srcTrace) {
                    printf("setting gamma=%f\n",file_gamma);
                }
#endif
                png_set_gamma(png_ptr, screen_gamma, file_gamma);
            }
            else {
#ifndef DISABLE_TRACE
                if (srcTrace) {
                    fprintf(stderr,"setting gamma=%f\n",0.45);
                }
#endif
                png_set_gamma(png_ptr, screen_gamma, (double)0.45);
            }
        }
    }
    
    if (png_get_interlace_type(png_ptr,info_ptr))
        png_set_interlace_handling(png_ptr);

    png_read_update_info(png_ptr, info_ptr);
    
    /* #ifndef DISABLE_TRACE */
#if 0     
    if (srcTrace) {
        fprintf(stderr,"\n\nAFTER\nheight = %d\n", (int)png_ptr->width);
        fprintf(stderr,"width = %d\n", (int)png_ptr->height);
        fprintf(stderr,"bit depth = %d\n", info_ptr->bit_depth);
        fprintf(stderr,"color type = %d\n", info_ptr->color_type);
        fprintf(stderr,"compression type = %d\n", info_ptr->compression_type);
        fprintf(stderr,"filter type = %d\n", info_ptr->filter_type);
        fprintf(stderr,"interlace type = %d\n", info_ptr->interlace_type);
        fprintf(stderr,"num colors = %d\n",info_ptr->num_palette);
        fprintf(stderr,"rowbytes = %d\n", info_ptr->rowbytes);
    }
#endif

        /* allocate the pixel grid which we will need to send to 
           png_read_image(). */
    png_pixels = (png_byte *)malloc(png_get_rowbytes(png_ptr,info_ptr) * 
                                    (*height) * sizeof(png_byte));
    

    row_pointers = (png_byte **) malloc((*height) * sizeof(png_byte *));
    for (i=0; i < *height; i++)
      row_pointers[i]=png_pixels+(png_get_rowbytes(png_ptr,info_ptr)*i);

    
        /* FINALLY - read the darn thing. */
    png_read_image(png_ptr, row_pointers);
    
    
        /* now that we have the (transformed to 8-bit RGB) image, we have
           to copy the resulting palette to our colormap. */
    if (png_get_color_type(png_ptr,info_ptr) & PNG_COLOR_MASK_COLOR) {
      if (png_get_PLTE(png_ptr, info_ptr, &palette,&num_palette) & PNG_INFO_PLTE) { 
            
            for (i=0; i < num_palette; i++) {
                colrs[i].red = palette[i].red << 8;
                colrs[i].green = palette[i].green << 8;
                colrs[i].blue = palette[i].blue << 8;
                colrs[i].pixel = i;
                colrs[i].flags = DoRed|DoGreen|DoBlue;
            }
            
        }
        else {
            for (i=0; i < 216; i++) {
                colrs[i].red = std_color_cube[i].red << 8;
                colrs[i].green = std_color_cube[i].green << 8;
                colrs[i].blue = std_color_cube[i].blue << 8;
                colrs[i].pixel = i;
                colrs[i].flags = DoRed|DoGreen|DoBlue;
            }	    
        }
    } else {
            /* grayscale image */
        
        for(i=0; i < 256; i++ ) {
            colrs[i].red = i << 8;
            colrs[i].green = i << 8; 	    
            colrs[i].blue = i << 8;
            colrs[i].pixel = i;
            colrs[i].flags = DoRed|DoGreen|DoBlue;    
        }
    }
    
        /* Now copy the pixel data from png_pixels to pixmap */
 
    pixmap = (png_byte *)malloc((*width) * (*height) * sizeof(png_byte));
    
    p = pixmap; q = png_pixels;

        /* if there is an alpha channel, we have to get rid of it in the
           pixmap, since I don't do anything with it yet */
   if (png_get_color_type(png_ptr,info_ptr) & PNG_COLOR_MASK_ALPHA) {

#ifndef DISABLE_TRACE
        if (srcTrace) {
            fprintf(stderr,"Getting rid of alpha channel\n");
        }
#endif
        for(i=0; i<*height; i++) {
            q = row_pointers[i];
            for(j=0; j<*width; j++) {
                *p++ = *q++; /*palette index*/
                q++; /* skip the alpha pixel */
            }
        }
        
        free((char *)png_pixels);
    }
    else {
        
#ifndef DISABLE_TRACE
        if (srcTrace) {
            fprintf(stderr,"No alpha channel\n");
        }
#endif
        
        for(i=0; i<*height; i++) {
            q = row_pointers[i];
            for(j=0; j<*width; j++) {
                *p++ = *q++; /*palette index*/
            }
        }
        
        free((char *)png_pixels);
        
    }

    free((png_byte **)row_pointers);
    
        /* clean up after the read, and free any memory allocated */

        /* free the structure */
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return pixmap;
}


#endif
