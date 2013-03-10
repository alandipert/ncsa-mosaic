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

#include "mosaic.h"
#include "comment.h"
#include "pixmaps.h"
#include "xpmread.h"
#include "xpm.h"

/*for memcpy*/
#include <memory.h>

/*extern unsigned char *ProcessXpm3Data();*/

extern int installed_colormap;
extern Colormap installed_cmap;

Pixmap IconsBig[25], IconsSmall[25], IconsTom[21], IconsEaster[8];

/* Icons Exported to gui.c */
Pixmap *IconPix=NULL,*IconPixBig,*IconPixTom,*IconPixSmall,*IconPixEaster;
int IconsMade = 0;

Pixmap dialogError, dialogInformation, dialogQuestion, dialogWarning;

Pixmap toolbarBack, toolbarForward, toolbarHome, toolbarReload,
    toolbarOpen, toolbarSave, toolbarClone, toolbarNew, toolbarClose,
    toolbarBackGRAY, toolbarForwardGRAY, toolbarAddHotlist,
    toolbarSearch, toolbarPrint, toolbarPost, toolbarFollow,
    tearv, tearh, toolbarPostGRAY, toolbarFollowGRAY,
    toolbarNewsFwd, toolbarNewsFFwd, toolbarNewsRev, toolbarNewsFRev,
    toolbarNewsIndex, toolbarNewsGroups,
    toolbarNewsFwdGRAY, toolbarNewsFFwdGRAY, toolbarNewsRevGRAY, toolbarNewsFRevGRAY,
    toolbarNewsIndexGRAY,
    toolbarFTPput, toolbarFTPmkdir;

Pixmap securityKerberos4, securityKerberos5, securityBasic, securityMd5,
	securityNone, securityUnknown, securityDomain, securityLogin, enc_not_secure;

struct pixload_info {
    char **raw;
    Pixmap *handle;
    int gray;
} pix_info[] = {
    {icon1,&IconsBig[0],0},
    {icon2,&IconsBig[1],0},
    {icon3,&IconsBig[2],0},
    {icon4,&IconsBig[3],0},
    {icon5,&IconsBig[4],0},
    {icon6,&IconsBig[5],0},
    {icon7,&IconsBig[6],0},
    {icon8,&IconsBig[7],0},
    {icon9,&IconsBig[8],0},
    {icon10,&IconsBig[9],0},
    {icon11,&IconsBig[10],0},
    {icon12,&IconsBig[11],0},
    {icon13,&IconsBig[12],0},
    {icon14,&IconsBig[13],0},
    {icon15,&IconsBig[14],0},
    {icon16,&IconsBig[15],0},
    {icon17,&IconsBig[16],0},
    {icon18,&IconsBig[17],0},
    {icon19,&IconsBig[18],0},
    {icon20,&IconsBig[19],0},
    {icon21,&IconsBig[20],0},
    {icon22,&IconsBig[21],0},
    {icon23,&IconsBig[22],0},
    {icon24,&IconsBig[23],0},
    {icon25,&IconsBig[24],0},
    {s_icon1,&IconsSmall[0],0},
    {s_icon2,&IconsSmall[1],0},
    {s_icon3,&IconsSmall[2],0},
    {s_icon4,&IconsSmall[3],0},
    {s_icon5,&IconsSmall[4],0},
    {s_icon6,&IconsSmall[5],0},
    {s_icon7,&IconsSmall[6],0},
    {s_icon8,&IconsSmall[7],0},
    {s_icon9,&IconsSmall[8],0},
    {s_icon10,&IconsSmall[9],0},
    {s_icon11,&IconsSmall[10],0},
    {s_icon12,&IconsSmall[11],0},
    {s_icon13,&IconsSmall[12],0},
    {s_icon14,&IconsSmall[13],0},
    {s_icon15,&IconsSmall[14],0},
    {s_icon16,&IconsSmall[15],0},
    {s_icon17,&IconsSmall[16],0},
    {s_icon18,&IconsSmall[17],0},
    {s_icon19,&IconsSmall[18],0},
    {s_icon20,&IconsSmall[19],0},
    {s_icon21,&IconsSmall[20],0},
    {s_icon22,&IconsSmall[21],0},
    {s_icon23,&IconsSmall[22],0},
    {s_icon24,&IconsSmall[23],0},
    {s_icon25,&IconsSmall[24],0},

    {mag_1_xpm,&IconsTom[0],0},
    {mag_2_xpm,&IconsTom[1],0},
    {mag_3_xpm,&IconsTom[2],0},
    {mag_4_xpm,&IconsTom[3],0},
    {mag_5_xpm,&IconsTom[4],0},
    {mag_6_xpm,&IconsTom[5],0},
    {mag_7_xpm,&IconsTom[6],0},
    {mag_8_xpm,&IconsTom[7],0},
    {mag_9_xpm,&IconsTom[8],0},
    {mag_10_xpm,&IconsTom[9],0},
    {mag_11_xpm,&IconsTom[10],0},
    {mag_12_xpm,&IconsTom[11],0},
    {mag_13_xpm,&IconsTom[12],0},
    {mag_14_xpm,&IconsTom[13],0},
    {mag_15_xpm,&IconsTom[14],0},
    {mag_16_xpm,&IconsTom[15],0},
    {mag_17_xpm,&IconsTom[16],0},
    {mag_18_xpm,&IconsTom[17],0},
    {mag_19_xpm,&IconsTom[18],0},
    {mag_20_xpm,&IconsTom[19],0},
    {mag_21_xpm,&IconsTom[20],0},

// SAM    EASTER_ARRAY

    {unlock_none_xpm,&securityNone,0},
    {unlock_unknown_xpm,&securityUnknown,0},
    {lock_kerberos4_xpm,&securityKerberos4,0},
    {lock_kerberos5_xpm,&securityKerberos5,0},
    {lock_basic_xpm,&securityBasic,0},
    {lock_domain_xpm,&securityDomain,0},
    {lock_md5_xpm,&securityMd5,0},
    {lock_login_xpm,&securityLogin,0},

    {toolbar_back_1_xpm,&toolbarBack,0},
    {toolbar_forw_1_xpm,&toolbarForward,0},
    {toolbar_back_1_xpm,&toolbarBackGRAY,1},
    {toolbar_forw_1_xpm,&toolbarForwardGRAY,1},
    {toolbar_home_1_xpm,&toolbarHome,0},
    {toolbar_reload_1_xpm,&toolbarReload,0},
    {toolbar_open_1_xpm,&toolbarOpen,0},
    {toolbar_save_1_xpm,&toolbarSave,0},
    {toolbar_open_window_1_xpm,&toolbarNew,0},
    {toolbar_clone_window_1_xpm,&toolbarClone,0},
    {toolbar_close_window_1_xpm,&toolbarClose,0},
    {toolbar_hotlist_1_xpm,&toolbarAddHotlist,0},
    {toolbar_news_groups_1_xpm,&toolbarNewsGroups,0},
    {toolbar_news_list_1_xpm,&toolbarNewsIndex,0},
    {toolbar_next_art_1_xpm,&toolbarNewsFwd,0},
    {toolbar_next_thr_1_xpm,&toolbarNewsFFwd,0},
    {toolbar_prev_art_1_xpm,&toolbarNewsRev,0},
    {toolbar_prev_thr_1_xpm,&toolbarNewsFRev,0},
    {toolbar_post_1_xpm,&toolbarPost,0},
    {toolbar_followup_1_xpm,&toolbarFollow,0},
    {toolbar_next_art_1_xpm,&toolbarNewsFwdGRAY,1},
    {toolbar_next_thr_1_xpm,&toolbarNewsFFwdGRAY,1},
    {toolbar_prev_art_1_xpm,&toolbarNewsRevGRAY,1},
    {toolbar_prev_thr_1_xpm,&toolbarNewsFRevGRAY,1},
    {toolbar_post_1_xpm,&toolbarPostGRAY,1},
    {toolbar_followup_1_xpm,&toolbarFollowGRAY,1},
    {toolbar_search_1_xpm,&toolbarSearch,0},
    {toolbar_print_1_xpm,&toolbarPrint,0},
    {toolbar_ftp_put_1_xpm,&toolbarFTPput,0},
    {toolbar_ftp_mkdir_1_xpm,&toolbarFTPmkdir,0},

    {tearv_xpm,&tearv,0},
    {tearh_xpm,&tearh,0},

    {xm_error_xpm,&dialogError,0},
    {xm_question_xpm,&dialogQuestion,0},
    {xm_information_xpm,&dialogInformation,0},
    {xm_warning_xpm,&dialogWarning,0},

    {not_secure_xpm, &enc_not_secure, 0},
    {NULL, NULL,0}
};


static GC DrawGC = NULL;
int IconWidth = 0;
int IconHeight = 0;
int WindowWidth = 0;
int WindowHeight = 0;

static struct color_rec {
        int pixel[3];
        int pixelval;
        struct color_rec *hash_next;
} *Hash[256];


static char **LoadPixmapFile(char *file);
static void FindIconColor(Display *dsp, Colormap colormap, XColor *colr);
static void PixAddHash(int red, int green, int blue, int pixval);
static void InitHash(void);
static int highbit(unsigned long ul);
static Pixmap PixmapFromData(Widget wid, unsigned char *data, int width,
                             int height, XColor *colrs, int gray);


#define PBUF 1024
/* Quick 'n Dirty XPM reader */
static char **LoadPixmapFile(char *file)
{
    char **pdata;
    char buf[256],*p;
    int j;

    FILE *fp;

    int x,y,c,i;

    if(!(fp = fopen(file,"r"))) {
        return NULL;
    }

    if(!fgets(buf,PBUF,fp) && strncmp("/* XPM */",buf,9)) {
        return NULL;
    }

    while(!feof(fp)) {
        if(!fgets(buf,PBUF,fp)) return NULL;
        if(buf[0]=='"') {

            if(sscanf(&buf[1],"%d %d %d ",&x,&y,&c) != 3) {
                fclose(fp);
                return NULL;
            }


            for(p=&buf[1];*p && *p!='"';p++);
            if(!*p) {
                fclose(fp);
                return NULL;
            } else {
                *p=0;
            }

            pdata = (char **) malloc(sizeof(char *) * (y+c+2));
            pdata[0] = strdup(&buf[1]);

            for(i=1;i<(y+c+1);i++) {
                if(feof(fp) || !fgets(buf,PBUF,fp)){
                    fclose(fp);
                    return NULL;
                }

                if(buf[0]=='"') {
                    for(p=&buf[1];*p && *p!='"';p++);
                    if(!*p) {
                        while(i<0) free(pdata[--i]);
                        free(pdata);
                        fclose(fp);
                        return NULL;
                    } else {
                        *p=0;
                    }
                    pdata[i] = strdup(&buf[1]);
                } else {
                    i--; /* skip comments, etc */
                }

            }
            pdata[y+c+1]=NULL; /*for ease of deletion - trust me. -bjs */
            fclose(fp);

            return pdata;
        }
    }
    return NULL;
}





static XColor def_colrs[256];
static int init_colors = 1;

/*
 * Find the closest color by allocating it, or picking an already allocated
 * color
 */
static void
FindIconColor(Display *dsp, Colormap colormap, XColor *colr)
{
	int i, match;
	int rd, gd, bd, dist, mindist;
	int cindx;

	if (init_colors)
	{
		for (i=0; i<256; i++)
		{
			def_colrs[i].pixel = -1;
			def_colrs[i].red = 0;
			def_colrs[i].green = 0;
			def_colrs[i].blue = 0;
		}
		init_colors = 0;
	}

	match = XAllocColor(dsp, colormap, colr);
	if (match == 0)
	{
		mindist = 196608;		/* 256 * 256 * 3 */
/*
		cindx = colr->pixel;
*/
		cindx = (-1);
		for (i=0; i<256; i++)
		{
			if (def_colrs[i].pixel == -1)
			{
				continue;
			}
			rd = ((int)(def_colrs[i].red >> 8) -
				(int)(colr->red >> 8));
			gd = ((int)(def_colrs[i].green >> 8) -
				(int)(colr->green >> 8));
			bd = ((int)(def_colrs[i].blue >> 8) -
				(int)(colr->blue >> 8));
			dist = (rd * rd) +
				(gd * gd) +
				(bd * bd);
			if (dist < mindist)
			{
				mindist = dist;
				cindx = def_colrs[i].pixel;
				if (dist == 0)
				{
					break;
				}
			}
		}
		if (cindx<0) {
			colr->pixel=BlackPixel(dsp,
					       DefaultScreen(dsp));
			colr->red = colr->green = colr->blue = 0;
		}
		else {
			colr->pixel = cindx;
			colr->red = def_colrs[cindx].red;
			colr->green = def_colrs[cindx].green;
			colr->blue = def_colrs[cindx].blue;
		}
	}
	else
	{
		def_colrs[colr->pixel].pixel = colr->pixel;
		def_colrs[colr->pixel].red = colr->red;
		def_colrs[colr->pixel].green = colr->green;
		def_colrs[colr->pixel].blue = colr->blue;
	}
}


#define PixFindHash(red, green, blue, h_ptr) \
	h_ptr = Hash[((((red * 306) + (green * 601) + (blue * 117)) >> 10) >> 8)]; \
	while(h_ptr != NULL) \
	{ \
		if ((h_ptr->pixel[0] == red)&& \
		    (h_ptr->pixel[1] == green)&& \
		    (h_ptr->pixel[2] == blue)) \
		{ \
			break; \
		} \
		h_ptr = h_ptr->hash_next; \
	}


static void
PixAddHash(int red, int green, int blue, int pixval)
{
	int lum;
	struct color_rec *hash_ptr;

	lum = ((((red * 306) + (green * 601) + (blue * 117)) >> 10) >> 8);

	hash_ptr = (struct color_rec *)XtMalloc(sizeof(struct color_rec));
	if (hash_ptr == NULL)
	{
		return;
	}
	hash_ptr->pixel[0] = red;
	hash_ptr->pixel[1] = green;
	hash_ptr->pixel[2] = blue;
	hash_ptr->pixelval = pixval;
	hash_ptr->hash_next = Hash[lum];
	Hash[lum] = hash_ptr;
}


static void
InitHash(void)
{
	int i;

	for (i=0; i<256; i++)
	{
		Hash[i] = NULL;
	}
}


static int
highbit(unsigned long ul)
{
	/*
	 * returns position of highest set bit in 'ul' as an integer (0-31),
	 * or -1 if none.
	 */

	int i;
	for (i=31; ((ul&0x80000000) == 0) && i>=0;  i--, ul<<=1);
	return i;
}



static Pixmap
PixmapFromData(Widget wid, unsigned char *data, int width, int height,
               XColor *colrs, int gray)
{
	int i,t;
	int linepad, shiftnum;
	int shiftstart, shiftstop, shiftinc;
	int bytesperline;
	int temp;
	int w, h;
	XImage *newimage;
	unsigned char *bit_data, *bitp, *datap;
	unsigned char *tmpdata;
	Pixmap pix;
	int Mapping[256];
	XColor tmpcolr;
	int size;
	int depth;
	int Vclass;
	XVisualInfo vinfo, *vptr;
	Visual *theVisual;
	int bmap_order;
	unsigned long c;
	int rshift, gshift, bshift;


	if (data == NULL)
	{
		return(0);
	}

	/* find the visual class. */
	vinfo.visualid = XVisualIDFromVisual(DefaultVisual(XtDisplay(wid),
		DefaultScreen(XtDisplay(wid))));
	vptr = XGetVisualInfo(XtDisplay(wid), VisualIDMask, &vinfo, &i);
	Vclass = vptr->class;
	XFree((char *)vptr);

	depth = DefaultDepthOfScreen(XtScreen(wid));

        for (i=0; i < 256; i++)
        {
		struct color_rec *hash_ptr;

                tmpcolr.red = colrs[i].red;
                tmpcolr.green = colrs[i].green;
                tmpcolr.blue = colrs[i].blue;
                tmpcolr.flags = DoRed|DoGreen|DoBlue;
                if ((Vclass == TrueColor) || (Vclass == DirectColor))
                {
                        Mapping[i] = i;
                }
                else
                {
			PixFindHash(tmpcolr.red, tmpcolr.green, tmpcolr.blue,
				hash_ptr);
			if (hash_ptr == NULL)
			{
				FindIconColor(XtDisplay(wid),
					(installed_colormap ?
					 installed_cmap :
					 DefaultColormapOfScreen(XtScreen(wid))),
					&tmpcolr);
				PixAddHash(colrs[i].red, colrs[i].green,
					colrs[i].blue, tmpcolr.pixel);
				Mapping[i] = tmpcolr.pixel;
			}
			else
			{
				Mapping[i] = hash_ptr->pixelval;
			}
                }
        }

	size = width * height;
	tmpdata = (unsigned char *)malloc(size);
	datap = data;
	bitp = tmpdata;
        if(gray) {
            t = Mapping[(int)*datap];
            for (i=0; i < size; i++)
                {
                    *bitp++ = (i+((i/width)%2))%2?(unsigned char)Mapping[(int)*datap]:t;
                    datap++;
                }
        } else {
            for (i=0; i < size; i++)
                {
                    *bitp++ = (unsigned char)Mapping[(int)*datap];
                    datap++;
                }
        }

	free((char *)data);
	data = tmpdata;

	switch(depth)
	{
	    case 6:
	    case 8:
		bit_data = (unsigned char *)malloc(size);
/*		bcopy(data, bit_data, size);*/
		memcpy(bit_data, data, size);
		bytesperline = width;
		newimage = XCreateImage(XtDisplay(wid),
			DefaultVisual(XtDisplay(wid),
				DefaultScreen(XtDisplay(wid))),
			depth, ZPixmap, 0, (char *)bit_data,
			width, height, 8, bytesperline);
		break;
	    case 1:
	    case 2:
	    case 4:
		if (BitmapBitOrder(XtDisplay(wid)) == LSBFirst)
		{
			shiftstart = 0;
			shiftstop = 8;
			shiftinc = depth;
		}
		else
		{
			shiftstart = 8 - depth;
			shiftstop = -depth;
			shiftinc = -depth;
		}
		linepad = 8 - (width % 8);
		bit_data = (unsigned char *)malloc(((width + linepad) * height)
				+ 1);
		bitp = bit_data;
		datap = data;
		*bitp = 0;
		shiftnum = shiftstart;
		for (h=0; h<height; h++)
		{
			for (w=0; w<width; w++)
			{
				temp = *datap++ << shiftnum;
				*bitp = *bitp | temp;
				shiftnum = shiftnum + shiftinc;
				if (shiftnum == shiftstop)
				{
					shiftnum = shiftstart;
					bitp++;
					*bitp = 0;
				}
			}
			for (w=0; w<linepad; w++)
			{
				shiftnum = shiftnum + shiftinc;
				if (shiftnum == shiftstop)
				{
					shiftnum = shiftstart;
					bitp++;
					*bitp = 0;
				}
			}
		}
		bytesperline = (width + linepad) * depth / 8;
		newimage = XCreateImage(XtDisplay(wid),
			DefaultVisual(XtDisplay(wid),
				DefaultScreen(XtDisplay(wid))),
			depth, ZPixmap, 0, (char *)bit_data,
			(width + linepad), height, 8, bytesperline);
		break;
	    /*
	     * WARNING:  This depth-16 code is donated code for 16-bit
	     * TrueColor displays.  I have no access to such displays, so I
	     * can't really test it.
	     * Donated by:  andrew@icarus.demon.co.uk
	     * Fixed version donated by:  nosmo@ximage.com (Vince Kraemer)
	     * ...and patched by GRR
	     */
	    case 16:
		bit_data = (unsigned char *)malloc(size * 2);
		bitp = bit_data;
		datap = data;

		theVisual = DefaultVisual(XtDisplay(wid),
			DefaultScreen(XtDisplay(wid)));
		rshift = 15 - highbit(theVisual->red_mask);
		gshift = 15 - highbit(theVisual->green_mask);
		bshift = 15 - highbit(theVisual->blue_mask);
		bmap_order = BitmapBitOrder(XtDisplay(wid));

		for (w = size; w > 0; w--)
		{
			temp = (((colrs[(int)*datap].red >> rshift) &
					theVisual->red_mask) |
				((colrs[(int)*datap].green >> gshift) &
					theVisual->green_mask) |
				((colrs[(int)*datap].blue >> bshift) &
					theVisual->blue_mask));
			if (bmap_order == MSBFirst)
			{
				*bitp++ = (temp >> 8) & 0xff;
				*bitp++ = temp & 0xff;
			}
			else
			{
				*bitp++ = temp & 0xff;
				*bitp++ = (temp >> 8) & 0xff;
			}

			datap++;
		}

		newimage = XCreateImage(XtDisplay(wid),
			DefaultVisual(XtDisplay(wid),
				DefaultScreen(XtDisplay(wid))),
			depth, ZPixmap, 0, (char *)bit_data,
			width, height, 16, 0);
		break;
	    case 24:
		bit_data = (unsigned char *)malloc(size * 4);

		theVisual = DefaultVisual(XtDisplay(wid),
			DefaultScreen(XtDisplay(wid)));
		rshift = highbit(theVisual->red_mask) - 7;
		gshift = highbit(theVisual->green_mask) - 7;
		bshift = highbit(theVisual->blue_mask) - 7;
		bmap_order = BitmapBitOrder(XtDisplay(wid));

		bitp = bit_data;
		datap = data;
		for (w = size; w > 0; w--)
		{
			c =
			  (((colrs[(int)*datap].red >> 8) & 0xff) << rshift) |
			  (((colrs[(int)*datap].green >> 8) & 0xff) << gshift) |
			  (((colrs[(int)*datap].blue >> 8) & 0xff) << bshift);

			datap++;

			if (bmap_order == MSBFirst)
			{
				*bitp++ = (unsigned char)((c >> 24) & 0xff);
				*bitp++ = (unsigned char)((c >> 16) & 0xff);
				*bitp++ = (unsigned char)((c >> 8) & 0xff);
				*bitp++ = (unsigned char)(c & 0xff);
			}
			else
			{
				*bitp++ = (unsigned char)(c & 0xff);
				*bitp++ = (unsigned char)((c >> 8) & 0xff);
				*bitp++ = (unsigned char)((c >> 16) & 0xff);
				*bitp++ = (unsigned char)((c >> 24) & 0xff);
			}
		}

		newimage = XCreateImage(XtDisplay(wid),
			DefaultVisual(XtDisplay(wid),
				DefaultScreen(XtDisplay(wid))),
			depth, ZPixmap, 0, (char *)bit_data,
			width, height, 32, 0);
		break;
	    default:
		newimage = NULL;
	}
	free((char *)data);

	if (newimage != NULL)
	{
		GC drawGC;

		pix = XCreatePixmap(XtDisplay(wid), XtWindow(wid),
			width, height, depth);
		drawGC = XCreateGC(XtDisplay(wid), XtWindow(wid), 0, NULL);
		XSetFunction(XtDisplay(wid), drawGC, GXcopy);

		XPutImage(XtDisplay(wid), pix, drawGC, newimage, 0, 0,
			0, 0, width, height);
		XFreeGC(XtDisplay(wid), drawGC);
		XDestroyImage(newimage);
		return(pix);
	}
	else
	{
		return(0);
	}
}

static int been_here=0;

static unsigned long p[256];
static int j;

void ReleaseSplashColors(Widget wid)
{
    XFreeColors(dsp,(installed_colormap ?
		     installed_cmap :
		     DefaultColormapOfScreen(XtScreen(wid))),p,j,0);
}



/* try to alloc *colorcount colors.
   set *colorcount to 1 if ok.
   free colors if allocated.
   load a pixmap and then free its colors if *colorcount = 0*/

Pixmap LoadSplashXPM(Widget wid, int *colorcount)
{
    unsigned char *data;
    int indx, w, h, bg;
    XColor colrs[256];
    int i;

    Pixmap r;

        /* try to grab *colorcount colors */
    if(XAllocColorCells(dsp,(installed_colormap ?
			     installed_cmap :
			     DefaultColormapOfScreen(XtScreen(wid))),
                        False ,NULL,0, p, *colorcount)){
        XFreeColors(dsp,(installed_colormap ?
			 installed_cmap :
			 DefaultColormapOfScreen(XtScreen(wid))),
                    p,*colorcount,0);
        *colorcount = 1;
    } else {
	if(DefaultDepthOfScreen(XtScreen(wid))==24)
            *colorcount = 1;
        else
            *colorcount = 0;
    }

    InitHash();

    data = ProcessXpm3Data(wid, splash_xpm, &w, &h, colrs, &bg);
    r = PixmapFromData(wid, data, w, h, colrs,0);
    init_colors = 1;
    for(i=0,j=0;i<256;i++)
        if(def_colrs[i].pixel != -1)
            p[j++] = def_colrs[i].pixel;
    if(!*colorcount)
        XFreeColors(dsp,(installed_colormap ?
			 installed_cmap :
			 DefaultColormapOfScreen(XtScreen(wid))),p,j,0);
    return r;
}


void MakePixmaps(Widget wid)
{
    int i,j;
    unsigned char *data;
    char **pdata;
    int indx, w, h, bg;
    XColor colrs[256];

    int pix_count = get_pref_int(ePIX_COUNT);
    char *pix_basename = get_pref_string(ePIX_BASENAME);
    int number_of_frames = NUMBER_OF_FRAMES;


    if (!been_here) {
        been_here=1;
        InitHash();
    }

        /* load pixmaps */
    for(i=0;pix_info[i].raw;i++) {
        data = ProcessXpm3Data(wid, pix_info[i].raw, &w, &h, colrs, &bg);
        *(pix_info[i].handle) = PixmapFromData(wid, data, w, h, colrs, pix_info[i].gray);
    }

    IconPixSmall = IconsSmall;
    IconPixBig = IconsBig;
    IconPixTom = IconsTom;
    IconPixEaster = IconsEaster;

    if(pix_basename && strcmp("default",pix_basename) && (pix_count > 0)) {

        char *fname;

        IconPixBig = (Pixmap *) malloc(sizeof(Pixmap)*pix_count);
        fname = (char *) malloc(strlen(pix_basename)+8);

        for (i=0; i<pix_count; i++) {
            sprintf(fname,"%s%d.xpm",pix_basename,i+1);
            if(!(pdata=LoadPixmapFile(fname))){
                fprintf(stderr,"Could not load pixmap '%s'.\n",fname);
                free(IconPixBig);
                IconPixBig = IconsBig;
                set_pref(ePIX_COUNT, (void *)&number_of_frames);
                break;
            }

            data = ProcessXpm3Data(wid, pdata, &w, &h, colrs, &bg);
            IconPixBig[i] = PixmapFromData(wid, data, w, h, colrs,0);

            if ((IconWidth == 0)||(IconHeight == 0)) {
                IconWidth = w; IconHeight = h;
            }

                /* delete the temp pixmap data */
            for(j=0;pdata[j];j++) free(pdata[j]);
            free(pdata);
        }
        free(fname);
    }
    else {
        set_pref(ePIX_COUNT, &number_of_frames);
    }

    IconPix = IconPixBig;

}


void DrawSecurityPixmap(Widget wid, Pixmap pix) {

/*
	XmxApplyPixmapToLabelWidget(wid,pix);
*/

	XtVaSetValues(wid,
		      XmNlabelPixmap,
		      pix,
		      XmNlabelType,
		      XmPIXMAP,
		      NULL);

	return;
}


void
AnimatePixmapInWidget(Widget wid, Pixmap pix)
{
	Cardinal argcnt;
	Arg arg[5];
	int x, y;

	if ((WindowWidth == 0)||(WindowHeight == 0))
	{
		Dimension w, h;

		argcnt = 0;
		XtSetArg(arg[argcnt], XtNwidth, &w); argcnt++;
		XtSetArg(arg[argcnt], XtNheight, &h); argcnt++;
		XtGetValues(wid, arg, argcnt);
		WindowWidth = w;
		WindowHeight = h;
	}

	if (DrawGC == NULL)
	{
		DrawGC = XCreateGC(XtDisplay(wid), XtWindow(wid), 0, NULL);
		XSetFunction(XtDisplay(wid), DrawGC, GXcopy);
	}
	x = (WindowWidth - IconWidth) / 2;
	if (x < 0)
	{
		x = 0;
	}
	y = (WindowHeight - IconHeight) / 2;
	if (y < 0)
	{
		y = 0;
	}
	XCopyArea(XtDisplay(wid),
		pix, XtWindow(wid), DrawGC,
		0, 0, IconWidth, IconHeight, x, y);
}

