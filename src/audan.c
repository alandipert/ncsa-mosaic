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
#include "audan.h"
#include "gui.h"
#include "pan.h"
#include "mo-www.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>

#include "libnut/system.h"

#ifdef HAVE_AUDIO_ANNOTATIONS

/* Check and make sure that the recording command we named
   in the resource is actually there and executable. */
mo_status mo_audio_capable (void)
{
  char *filename = get_pref_string(eRECORD_COMMAND_LOCATION);
  struct stat buf;
  int r;

  r = stat (filename, &buf); 
  if (r != -1 && buf.st_mode & S_IXOTH)
    return mo_succeed;
  else
    return mo_fail;
}

static XmxCallback (start_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  /* Take ourselves out of circulation. */
  XmxSetSensitive (win->audio_start_button, XmxNotSensitive);

  /* Come up with a new tmpnam. */
  win->record_fnam = mo_tmpnam(win->current_node->url);

  /* Fork off the recording process. */
  if ((win->record_pid = fork ()) < 0)
    {
      /* Uh oh, no process. */
      XmxMakeErrorDialog (win->audio_annotate_win, 
                          "Unable to start audio recording process." , 
                          "Audio Annotation Error" );
      XtManageChild (Xmx_w);
    }
  else if (win->record_pid == 0)
    {
      /* We're in the child. */
      execl (get_pref_string(eRECORD_COMMAND_LOCATION),
	     get_pref_string(eRECORD_COMMAND),
             win->record_fnam, (char *)0);
    }

  /* Let the user stop the record process. */
  XmxSetSensitive (win->audio_stop_button, XmxSensitive);
}

static XmxCallback (stop_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  /* Take ourselves out of circulation. */
  XmxSetSensitive (win->audio_stop_button, XmxNotSensitive);

  /* Stop the record process.  This works for both SGI recordaiff
     and Sun record, apparently. */
  kill (win->record_pid, SIGINT);
  
  /* Wait until the process is dead. */
  waitpid (win->record_pid, NULL, 0);

  /* No more process. */
  win->record_pid = 0;

  /* Let the user make another recording. */
  XmxSetSensitive (win->audio_start_button, XmxSensitive);
}

static XmxCallback (buttons_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));

  if (!win->current_node)
    {
      XtUnmanageChild (win->audio_annotate_win);
      return;
    }

  switch (XmxExtractToken ((int)client_data))
    {
    case 0:
      if (win->record_pid)
        {
          /* Stop the record process. */
          kill (win->record_pid, SIGINT);
          
          /* Wait until the process is dead. */
          waitpid (win->record_pid, NULL, 0);
        }
      if (win->record_fnam)
        {
          if (win->audio_pubpri == mo_annotation_workgroup)
            {
              char namestr[200], titlestr[200];
              unsigned char *data;
              FILE *fp;
              int len;
              extern char *machine;
              
              sprintf (namestr, "%s <%s>",
                       get_pref_string(eDEFAULT_AUTHOR_NAME),
                       get_pref_string(eDEFAULT_AUTHOR_EMAIL));

              sprintf (titlestr, "%s %s",
                       "Audio Annotation by" ,
                       get_pref_string(eDEFAULT_AUTHOR_NAME));
              
              len = 0;
              fp = fopen(win->record_fnam, "r");
              if (fp != NULL)
                {
                  /*
                   * Fine the length of the file the really cheesy way!
                   */
                  fseek(fp, 0L, 0);
                  fseek(fp, 0L, 2);
                  len = ftell(fp);
                  fseek(fp, 0L, 0);
                  data = (unsigned char *)malloc(len * sizeof(unsigned char));
                  if (data != NULL)
                    {
                      len = fread(data, sizeof(char), len, fp);
                    }
                  else
                    {
                      len = 0;
                    }
                  fclose(fp);
                }
              
              if (len <= 0)
                {
                  XmxMakeErrorDialog 
                    (win->base, "Unable to complete audio annotation." , 
                     "Audio Annotation Error" );
                  XtManageChild (Xmx_w);
                }
              
              mo_audio_grpan(win->current_node->url, titlestr, namestr,
                             (char *)data, len);
              if (data)
		free((char *)data);
              
              mo_set_win_current_node (win, win->current_node);
            }
          else
            {
              /* Do the right thing. */
              int pan_id = mo_next_pan_id ();
              char *default_directory = 
		get_pref_string(ePRIVATE_ANNOTATION_DIRECTORY);
              char filename[500], *cmd, namestr[200], textstr[500], titlestr[200];
              extern char *machine;
              
#ifdef __sgi
              sprintf (filename, "%s/%s/%s%d.aiff", getenv ("HOME"), 
                       default_directory,
                       "PAN-", pan_id);
#else /* sun or hp */
              sprintf (filename, "%s/%s/%s%d.au", getenv ("HOME"), 
                       default_directory,
                       "PAN-", pan_id);
#endif
              
/*SWP -- New "mv" fucntion to take care of these /bin/mv things*/
		{
		char retBuf[BUFSIZ];
		int status;

			if ((status=my_move(win->record_fnam,filename,retBuf,BUFSIZ,1))!=SYS_SUCCESS) {
				application_user_info_wait(retBuf);
				return;
			}
		}

              sprintf (titlestr, "%s %s",
		       "Audio Annotation by" ,
                       get_pref_string(eDEFAULT_AUTHOR_NAME));

              sprintf (namestr, "%s <%s>",
                       get_pref_string(eDEFAULT_AUTHOR_NAME),
                       get_pref_string(eDEFAULT_AUTHOR_EMAIL));
              sprintf (textstr, 
		       "This is an audio annotation. <P>\n\nTo hear the annotation, click <A HREF=\"file:%s\">here</A>. <P>\n" , filename);

              mo_new_pan (win->current_node->url, titlestr, namestr,
                          textstr);
              
              /* Inefficient, but safe. */
              mo_write_pan_list ();
              
              mo_set_win_current_node (win, win->current_node);
            }
        }
      
      win->record_pid = 0;
      win->record_fnam = 0;

      XtUnmanageChild (win->audio_annotate_win);
      break;
    case 1:
      if (win->record_pid)
        {
          /* Stop the record process. */
          kill (win->record_pid, SIGINT);
          
          /* Wait until the process is dead. */
          waitpid (win->record_pid, NULL, 0);
          
          win->record_pid = 0;
          win->record_fnam = 0;
        }
      XtUnmanageChild (win->audio_annotate_win);
      break;
    case 2:
      mo_open_another_window
        (win,
         mo_assemble_help_url ("help-on-audio-annotate.html"),
         NULL, NULL);
      break;
    }
  
  return;
}


extern XmxOptionMenuStruct *pubpri_opts;


static XmxCallback (pubpri_opts_cb)
{
  mo_window *win = mo_fetch_window_by_id (XmxExtractUniqid ((int)client_data));
  
  win->audio_pubpri = XmxExtractToken ((int)client_data);

  return;
}

mo_status mo_post_audio_annotate_win (mo_window *win)
{
  Widget dialog_frame;
  Widget dialog_sep, buttons_form;
  Widget audio_annotate_form, yap_label;

  if (!win->current_node)
    return mo_fail;

  if (!win->audio_annotate_win)
    {
      /* Create it for the first time. */
      XmxSetUniqid (win->id);
      XmxSetArg (XmNresizePolicy, XmRESIZE_GROW);
      win->audio_annotate_win = XmxMakeFormDialog 
        (win->base, "NCSA Mosaic: Audio Annotate Window" );
      dialog_frame = XmxMakeFrame (win->audio_annotate_win, XmxShadowOut);
      
      /* Constraints for base. */
      XmxSetConstraints 
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
      
      /* Main form. */
      XmxSetArg (XmNfractionBase, 2);
      audio_annotate_form = XmxMakeForm (dialog_frame);
      
      yap_label = XmxMakeLabel 
        (audio_annotate_form, "Press Start to start recording; Stop to stop recording.\nRepeat until you're satisfied with the annotation.\nThen either Save or Dismiss the annotation." );
      
      win->audio_start_button = XmxMakePushButton (audio_annotate_form,
                                                   "Start" ,
                                                   start_cb,
                                                   0);
      win->audio_stop_button = XmxMakePushButton (audio_annotate_form,
                                                  "Stop" ,
                                                  stop_cb,
                                                  0);

      win->audio_pubpri_menu = XmxRMakeOptionMenu
        (audio_annotate_form, "", pubpri_opts_cb, pubpri_opts);
      XmxRSetSensitive (win->audio_pubpri_menu, mo_annotation_public, 
                        XmxNotSensitive);

      if (! get_pref_string(eANNOTATION_SERVER))
        XmxRSetSensitive (win->audio_pubpri_menu, mo_annotation_workgroup, 
                          XmxNotSensitive);

      win->audio_pubpri = mo_annotation_private;

      dialog_sep = XmxMakeHorizontalSeparator (audio_annotate_form);
      
      buttons_form = XmxMakeFormAndThreeButtons
        (audio_annotate_form, buttons_cb, "Save" ,
	 "Dismiss" , "Help..." , 
         0, 1, 2);
      
      /* Constraints for audio_annotate_form. */
      XmxSetOffsets (yap_label, 10, 10, 10, 10);
      XmxSetConstraints
        (yap_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
      XmxSetOffsets (win->audio_start_button, 10, 10, 10, 10);
      XmxSetConstraints 
        (win->audio_start_button, XmATTACH_WIDGET, XmATTACH_NONE,
         XmATTACH_FORM, XmATTACH_NONE,
         yap_label, NULL, NULL, NULL);
      XmxSetOffsets (win->audio_stop_button, 10, 10, 10, 10);
      XmxSetConstraints 
        (win->audio_stop_button, XmATTACH_WIDGET, XmATTACH_NONE,
         XmATTACH_WIDGET, XmATTACH_NONE,
         yap_label, NULL, win->audio_start_button, NULL);
      XmxSetOffsets (win->audio_pubpri_menu->base, 10, 10, 10, 10);
      XmxSetConstraints
        (win->audio_pubpri_menu->base, XmATTACH_WIDGET, XmATTACH_NONE,
         XmATTACH_WIDGET, XmATTACH_FORM,
         yap_label, NULL, win->audio_stop_button, NULL);
      
      XmxSetArg (XmNtopOffset, 10);
      XmxSetConstraints 
        (dialog_sep, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, 
         XmATTACH_FORM,
         win->audio_pubpri_menu->base, NULL, NULL, NULL);
      XmxSetConstraints 
        (buttons_form, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, 
         XmATTACH_FORM,
         dialog_sep, NULL, NULL, NULL);
    }

  XmxManageRemanage (win->audio_annotate_win);
  XmxSetSensitive (win->audio_stop_button, XmxNotSensitive);
  XmxSetSensitive (win->audio_start_button, XmxSensitive);
  
  return mo_succeed;
}


#endif
