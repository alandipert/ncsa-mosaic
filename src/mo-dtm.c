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
#include "mo-dtm.h"

#ifdef HAVE_DTM
/* Interface between Mosaic and DTM lives here.
   Right now all we want to do is sent text to a Collage session... */

#include "net.h"
#include "netdata.h"
#include "libhtmlw/HTML.h" /* for ImageInfo */

/* Creation of an input port implies done_init and done_register
   as well as done_inport.  Creation of an output port implies 
   all of these (since an input port is always created prior to
   creating an output port). */
static int done_init     = 0;   /* called NetInit?? */
static int done_register = 0;   /* called NetRegisterModule?? */
static int done_outport  = 0;   /* called NetCreateOutPort?? */
static int done_inport   = 0;   /* called NetCreateInPort?? */

/* ------------------------------ mo_dtm_in ------------------------------- */

mo_status mo_dtm_in (char *path)
{
  NetPort *inport;
  
  if (!done_init)
    {
      NetInit ("Mosaic");
      done_init = 1;
    }
  
  if (!done_register)
    {
#if 0
      NetRegisterModule
        ("Mosaic", NETCOM,
         mo_receive_com, (caddr_t) state,
         NULL, (caddr_t) 0,
         NULL, (caddr_t) 0);
#endif
      done_register = 1;
    }
  
  if (!done_inport)
    {
      inport = NetCreateInPort (path);
      done_inport = 1;
    }

  mo_register_dtm_blip ();
  
  return mo_succeed;
}

/* -------------------------- mo_dtm_disconnect --------------------------- */

mo_status mo_dtm_disconnect (void)
{
  if (done_init)
    if (!NetSendDisconnect (NULL, NULL, NULL)) {
      sleep(1);
      NetTryResend();
    }

  return;
}

/* ------------------------------ mo_dtm_out ------------------------------ */

mo_status mo_dtm_out (char *port)
{
  if (!done_outport)
    {
      mo_dtm_in (":0");
      
      /* Make the output port. */
      NetCreateOutPort (port);

      done_outport = 1;
    }
  
  return mo_succeed;
}

/* ------------------------- mo_dtm_out_active_p -------------------------- */

mo_status mo_dtm_out_active_p ()
{
  if (done_outport)
    return mo_succeed;
  else
    return mo_fail;
}

/* ------------------------- mo_dtm_poll_and_read ------------------------- */

mo_status mo_dtm_poll_and_read ()
{
  if (done_inport)
    NetClientPollAndRead ();
  
  return mo_succeed;
}

/* --------------------------- mo_dtm_send_text --------------------------- */

mo_status mo_dtm_send_text (mo_window *win, char *url, char *text)
{
  Text *t;
  char *title;
  int rv;

  if (!mo_dtm_out_active_p ())
    return mo_fail;

  title = (char *)malloc (strlen (url) + 16);
  sprintf (title, "Mosaic: %s\0", url);

  t = (Text *)malloc (sizeof (Text));
  t->title = title;
  t->id = strdup ("Mosaic");
  t->selLeft = t->selRight = t->insertPt = 0;
  t->numReplace = t->dim = strlen (text);
  t->replaceAll = TRUE;
  t->textString = strdup (text);

  rv = NetSendText (NULL, t, FALSE, "NewText");

  return mo_succeed;
}

mo_status mo_dtm_send_image (void *data)
{
  ImageInfo *img = (ImageInfo *)data;
  int rv, i;
  char palette[768];

  if (!mo_dtm_out_active_p ())
    return mo_fail;

  for (i = 0; i < 256; i++)
    {
      if (i < img->num_colors)
        {
          palette[i*3+0] = img->reds[i]   >> 8;
          palette[i*3+1] = img->greens[i] >> 8;
          palette[i*3+2] = img->blues[i]  >> 8;
        }
      else
        {
          palette[i*3+0] = i;
          palette[i*3+1] = i;
          palette[i*3+2] = i;
        }
    }

  rv = NetSendRaster8Group 
    (NULL, "Mosaic Image", img->image_data,
     img->width, img->height, palette, TRUE, FALSE, NULL);

  return mo_succeed;
}


mo_status mo_dtm_send_palette (void *data)
{
  ImageInfo *img = (ImageInfo *)data;
  int rv, i;
  char palette[768];

  if (!mo_dtm_out_active_p ())
    return mo_fail;

  for (i = 0; i < 256; i++)
    {
      if (i < img->num_colors)
        {
          palette[i*3+0] = img->reds[i]   >> 8;
          palette[i*3+1] = img->greens[i] >> 8;
          palette[i*3+2] = img->blues[i]  >> 8;
        }
      else
        {
          palette[i*3+0] = i;
          palette[i*3+1] = i;
          palette[i*3+2] = i;
        }
    }

  rv = NetSendPalette8
    (NULL, "Mosaic Palette", palette, NULL, FALSE, NULL);

  return mo_succeed;
}


mo_status mo_dtm_send_dataset (void *spanker)
{
    Data *d = (Data *) spanker;
    int rv, i;
    char palette[768];
    
    if (!mo_dtm_out_active_p ())
        return mo_fail;
    
/*

    for (i = 0; i < 256; i++)
        {
            if (i < img->num_colors)
                {
                    palette[i*3+0] = img->reds[i]   >> 8;
                    palette[i*3+1] = img->greens[i] >> 8;
                    palette[i*3+2] = img->blues[i]  >> 8;
                }
            else
                {
                    palette[i*3+0] = i;
                    palette[i*3+1] = i;
                    palette[i*3+2] = i;
                }
        }
*/
   
#define COLLAGE_SUCKS 
#ifdef COLLAGE_SUCKS
    rv = NetSendArray
        (NULL, d, TRUE, FALSE, NULL, (d->rank == 3 ? TRUE : FALSE));
#else
    rv = NetSendArray
        (NULL, d, TRUE, FALSE, NULL, TRUE);
#endif
    
    return mo_succeed;

}
#endif /* HAVE_DTM */
