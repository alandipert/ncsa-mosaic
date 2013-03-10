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
#include "gui-ftp.h"
#include "mo-www.h"

#ifndef GUI_POPUP_H
#define GUI_POPUP_H
#include "gui-popup.h"
#endif

#include "gui-documents.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

extern mo_window *current_win;
extern int imageViewInternal;
extern char *getFileName();
extern int do_meta;

void ThirdButtonMenu(w, client_data, event, ctd)
     Widget w;
     XtPointer *client_data;
     XEvent *event;
     Boolean *ctd;
{
  XButtonPressedEvent *BuEvent = (XButtonPressedEvent *) event;
  static struct ele_rec *eptr;
  int *epos, mode, type, i, del=12;
  int sens=False;

  if(have_popup && (BuEvent->button == Button3))
    {
      HTMLWidget hw = (HTMLWidget) w;

      while((hw != NULL) && (XtClass((Widget) hw) != htmlWidgetClass))
	hw = (HTMLWidget) XtParent((Widget) hw);

      if(hw == NULL)
	return;

      eptr = LocateElement(hw, BuEvent->x, BuEvent->y,
			   &epos);

      if(!popup)
	{
	  /* before we build the popup see if the user has a .mosiac/user-defs
	     file and if so use it to build a user menu */
	  popup_items[3].sub_items = popup_build_user_defs();
	  /* before we build the popup see if the user has a .mosiac/user-defs
	     file and if so use it to build a user menu */

	  /* if we didn't have any turn it off */
	  if(popup_items[3].sub_items == NULL)
	    popup_items[3].types = 0;

	  popup = _PopupMenuBuilder((Widget) w, XmMENU_POPUP,
				    "popup", 0, popup_items);
	}

      mode = current_win->mode;

      if(eptr)
	{
	  type = eptr->type;
	  if((type == E_IMAGE) && (eptr->anchorHRef))
	    { /* turn on anchor and off text */
	      type |= E_ANCHOR;
	      if(type & E_TEXT)
		type -= E_TEXT;
	    }
	  if((type == E_TEXT) && (eptr->anchorHRef))
	    type = E_ANCHOR;
	}
      else
	type = E_HRULE; /* pick a good normal little element */

      for(i = 0; popup_items[i].class != LastItem; i++)
	{
	  if(popup_items[i]._w) /* anything is possible in Mosaic */
	    {
	      int good = True;

	      /* take care of session menu */
	      if(popup_items[i].acst.act_code == -2)
		XtVaSetValues(popup_items[i]._w, XmNsubMenuId,
			      current_win->session_menu, NULL);

	      /* determine if we want this guy */
	      if(popup_items[i].types_method == LOOSE)
		good = good && (popup_items[i].types & type);
	      else
		good = good && (popup_items[i].types == type);

	      if(popup_items[i].modes_method == LOOSE)
		good = good && (popup_items[i].modes & mode);
	      else
		good = good && (popup_items[i].modes == mode);

	      if(good)
		{
		  if(popup_items[i].class == Separator)
		    del+=4;
		  else
		    {
		      switch(mode)
			{
			case moMODE_PLAIN:
			  if(!(type & E_IMAGE))
			    {
			      if(!XtIsSensitive(popup_items[i]._w) && !sens)
				del+=24;
			      else
				sens=True;
			      break;
			    }
			case moMODE_NEWS:
			case moMODE_FTP:
			  if(i > 3) /* skip forward, backward, hotlist, and sep */
			    {
			      if(!XtIsSensitive(popup_items[i]._w) && !sens)
				del+=24;
			      else
				sens=True;
			    }
			  else
			    del+=24;
			  break;
			}
		    }
		  XtManageChild(popup_items[i]._w);
		}
	      else
		XtUnmanageChild(popup_items[i]._w);
	    }
	}

      /* set all the widgets eptr data */
      _set_eptr_field(popup_items, eptr);

      /* motif puts the menu in a boring place lets fix it */
      /*   BuEvent->x_root -= 40; */ /* middle of buttons */
      BuEvent->y_root -= del; /* first active button or first specialty
				      item if we're over element that has em */

      XmMenuPosition(popup, BuEvent);
      XtManageChild(popup);
    }
}

PopupItem *popup_build_user_defs()
{
  PopupItem *items;
  char *str, *file, **entries;
  FILE *fp;
  int num, i;

  if ((num=get_home(&str))!=0)
    {
      return NULL;
    }

  file = malloc(sizeof(char) * (strlen(str)+strlen("/.mosaic/user-defs")+1)); // SAM
  sprintf(file, "%s/.mosaic/user-defs", str); // SAM

  free(str);

  if(!file_exists(file))
    {
      free(file);
      return NULL;
    }

  fp = fopen(file, "r");
  free(file);

  if(fp)
    {
      fseek(fp,0,SEEK_SET);
      entries = user_defs_get_entries(fp, &num);
      fclose(fp);
    }
  else
    return NULL;

  items = build_user_defs_items(entries, num);

  for(i=0;i<(num+2);i++)
    free(entries[i]);
  free(entries);

  if(items)
    return items;
  else
    return NULL;
}

void _set_eptr_field(items, eptr)
     PopupItem *items;
     struct ele_rec *eptr;
{
  int i;

  for(i=0; items[i].class != LastItem; i++)
    {
      if(items[i].sub_items)
	_set_eptr_field(items[i].sub_items, eptr);
      items[i].acst.eptr = eptr;
    }
}

void mo_make_popup(Widget view)
{
  have_popup = True; /* this will cause it to be created later */
  popup = NULL;

  XtInsertEventHandler(view, ButtonPressMask, False,
                       (XtEventHandler)ThirdButtonMenu, NULL,
                       XtListHead);
}

void mo_popup_set_something(char *what, int to, PopupItem *items)
{
  Widget w=NULL;
  int i;

  if(items == NULL)
    items = popup_items;

  for(i=0; (items[i].class != LastItem) && !w; i++)
    {
      if(items[i].label && (items[i].label[0] == *what))
	{
	  if(!strcmp(items[i].label, what))
	    {
	      w=items[i]._w;
	      if(!w)
		{
		  items[i].startup = to;
		  break;
		}
	    }
	}

      if(items[i].sub_items)
	mo_popup_set_something(what, to,items[i].sub_items);
    }

  if(w)
    XtSetSensitive(w, to);
}

Widget _PopupMenuBuilder(Widget parent, int type, char *title,
				char mnem, PopupItem *items)
{
  Widget menu, cascade;
  XmString str;
  int i, mapping_del;

  if(type == XmMENU_POPUP)
    {
      menu =  XmCreatePopupMenu(parent, title, NULL, 0);
    }
  else if(type == XmMENU_PULLDOWN)
    {
      menu = XmCreatePulldownMenu(parent, title, NULL, 0);
      str = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
      mapping_del = get_pref_int(ePOPUPCASCADEMAPPINGDELAY);
      cascade = XtVaCreateManagedWidget(title, xmCascadeButtonGadgetClass,
					parent, XmNsubMenuId, menu,
					XmNlabelString, str,
					XmNmnemonic, mnem,
					XmNmappingDelay, mapping_del, NULL);
      XmStringFree(str);
    }
  else
    return (Widget) NULL; /* this shouldn't happen */

  for(i=0;(items[i].class!=LastItem); i++)
    {
      switch(items[i].class)
	{
	case PushButton:
	  {
	    items[i]._w = XtVaCreateManagedWidget(items[i].label,
						  xmPushButtonGadgetClass,
						  menu, NULL);
	    if(items[i].mnemonic)
	      XtVaSetValues(items[i]._w, XmNmnemonic, items[i].mnemonic,
			    NULL);
	    if(items[i].accel)
	      {
		XtVaSetValues(items[i]._w,
			      XmNaccelerator , items[i].accel,
			      NULL);
	      }
	    if(items[i].accel_text)
	      {
		str = XmStringCreateLtoR(items[i].accel_text, XmSTRING_DEFAULT_CHARSET);
		XtVaSetValues(items[i]._w,
			      XmNacceleratorText, str,
			      NULL);
		XmStringFree(str);
	      }
	    if(items[i].cbfp)
	      XtAddCallback(items[i]._w, XmNactivateCallback,
			    items[i].cbfp, &(items[i].acst));


	    XtSetSensitive(items[i]._w, items[i].startup);

	    if(items[i].acst.str && items[i].acst.act_code==69)
	      {
		XtAddCallback(items[i]._w,
			      XmNarmCallback, rbm_ballonify,
			      items[i].acst.str);
		XtAddCallback(items[i]._w,
			      XmNdisarmCallback, rbm_ballonify, NULL);
	      }
	  }
	  break;
	case Separator:
	  {
	    items[i]._w = XtVaCreateManagedWidget(items[i].label,
						  xmSeparatorGadgetClass,
						  menu, NULL);
	  }
	  break;
	case ToggleButton:
	  {
	    items[i]._w = XtVaCreateManagedWidget(items[i].label,
						  xmToggleButtonGadgetClass,
						  menu, NULL);
	  }
	  break;
	case CascadeButton:
	  {
	    if(items[i].sub_items && (items[i].acst.act_code != -2))
	      items[i]._w = _PopupMenuBuilder(menu, XmMENU_PULLDOWN,
					      items[i].label,
					      items[i].mnemonic,
					      items[i].sub_items);
	    else
	      {
		int mapping_del;

		if(get_pref_boolean(eSESSION_HISTORY_ON_RBM))
		  {
		    mapping_del = get_pref_int(ePOPUPCASCADEMAPPINGDELAY);
		    items[i]._w = XtVaCreateManagedWidget("Session History",
					       xmCascadeButtonGadgetClass,
					       	  menu, XmNsubMenuId,
					current_win->session_menu,
					XmNmappingDelay, mapping_del, NULL);
		  }
	      }
	  }
	}

    }
  return type == XmMENU_POPUP ? menu : cascade;
}

void metadata_cb(Widget w, XtPointer client_data, XtPointer call_data) {

act_struct *acst = (act_struct *) client_data;
char *xurl;
struct ele_rec *eptr;
int which;

	if(!acst || !acst->eptr) {
		return;
	}

	eptr = acst->eptr;
	which = acst->act_code;

	if(!eptr) {
		return; /* oh, well */
	}

	switch (which) {
		case M_ImageData: {
			if (!eptr->pic_data) { /* do what? */
				return;
			}
			xurl=mo_url_prepend_protocol(eptr->pic_data->src);
			do_meta=1;
			mo_load_window_text(current_win, xurl, NULL);
			do_meta=0;

			break;
		}
		case M_FileData: {
			do_meta=1;
			mo_load_window_text(current_win,
					    current_win->current_node->url,
					    NULL);
			do_meta=0;

			break;
		}
		case M_LinkData: {
			if (!eptr->anchorHRef || !*eptr->anchorHRef) {
				return;
			}

			xurl=mo_url_canonicalize(eptr->anchorHRef,
						strdup(current_win->current_node->url));
			do_meta=2;
			mo_load_window_text(current_win, xurl, NULL);
			do_meta=0;

			break;
		}
		default: {
			fprintf(stderr,"Smells like an error...\n");

			break;
		}
	}

	return;
}


void image_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  act_struct *acst = (act_struct *) client_data;
  char *src, *xurl;
  struct ele_rec *eptr;
  int which,tmp;

  eptr = acst->eptr;
  which = acst->act_code;

  /* somewhere we lose source on rare ocassions... SWP */
  if(!eptr || !eptr->pic_data || !eptr->pic_data->src)
    {
      printf("Lost source.\n");
      return; /* oh, well */
    }

  switch(which)
    {
    case I_Save:
      {
	/* FIXME: this should be
	   fsb(eptr->edata); */
	fsb(eptr->pic_data->src);
      }
      break;
    case I_ViewExternal:
      {
	xurl=mo_url_prepend_protocol(eptr->pic_data->src);
	tmp=imageViewInternal;
	imageViewInternal=0;
	mo_load_window_text (current_win, xurl, NULL);
	imageViewInternal=tmp;
      }
      break;
    case I_ViewInternal:
      {
	xurl=mo_url_prepend_protocol(eptr->pic_data->src);
	tmp=imageViewInternal;
	imageViewInternal=1;
	mo_load_window_text (current_win, xurl, NULL);
	imageViewInternal=tmp;
      }
      break;
    case I_Reload:
      {
	mo_reload_window_text (current_win, 1);
      }
      break;
    }
}

void fsb(char *src)
{
  static Widget dialog;
  XmString str,fbfn;
  char *fname, *tmpstr, fBuf[1024];
  static char *last_src=NULL;

  if ( !dialog ) {
      last_src=strdup(src);
      dialog = XmCreateFileSelectionDialog (current_win->view,
					    "Save Image File",
					    NULL, 0 );
      XtAddCallback(dialog,
		    XmNcancelCallback,
		    fsb_CancelCallback,
		    NULL);
      XtAddCallback(dialog,
		    XmNokCallback,
		    fsb_OKCallback,
		    last_src);

      XtSetSensitive(XmFileSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON),
		     False);

      XtVaSetValues(dialog,
		    XmNfileTypeMask, XmFILE_REGULAR,
		    NULL);

  }
  else {
      /* Dance with the callbacks so we get the correct URL later --SWP */
      XtRemoveCallback(dialog,
		       XmNokCallback,
		       fsb_OKCallback,
		       last_src);
      if (last_src) {
	free(last_src);
      }
      last_src=strdup(src);
      XtAddCallback(dialog,
		    XmNokCallback,
		    fsb_OKCallback,
		    last_src);

      /* Re-Init the Stupid Box -- SWP */
      XmFileSelectionDoSearch(dialog,NULL);
  }

  /* set the save file string */
  XtVaGetValues(dialog,
		XmNdirSpec, &str,
		NULL);

  XmStringGetLtoR(str,XmSTRING_DEFAULT_CHARSET,&fname);
  XmStringFree(str);

  if (fname)
    {
      if(src && *src)
	sprintf(fBuf,"%s%s",fname,getFileName(src));
      else
	sprintf(fBuf,"%s",fname);

      str=XmStringCreateLtoR(fBuf,XmSTRING_DEFAULT_CHARSET);

      XtVaSetValues(dialog,
		    XmNdirSpec, str,
		    NULL);

      XmStringFree(str);
      free(fname);
    }

  XtManageChild ( dialog );
}


void fsb_OKCallback (
    Widget w, XtPointer client_data, XtPointer call_data)
{
  XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;
  char *filename, *url = (char *) client_data, efilename[MO_LINE_LENGTH];

  /* Remove the widget from the screen, and kill it.  */

  XtUnmanageChild ( w );

  /* Retrieve the character string from the compound string format.  */

  XmStringGetLtoR ( cbs->value, XmSTRING_DEFAULT_CHARSET, &filename );

  /* Expand any ~ */
  pathEval (efilename, filename);

  /* FIXME: the code below should just copy a file but since
     we don't keep the files around we have to beam it down again.
     This should be fixed with the disk cache */

  /* now copy src to filename */
  /*  if(!dest || !filename)
    {
#ifndef DISABLE_TRACE
      if (srcTrace) {
	fprintf(stderr, "Couldn't save file, internal error.\n");
      }
#endif
      return;
    }*/

  mo_pull_er_over_virgin(url, efilename);

  /* We need to reset the icons and let the user know -- SWP */
  mo_gui_done_with_icon();
  mo_gui_notify_progress("Image has been downloaded and saved.");

}

void fsb_CancelCallback (
    Widget    w,
    XtPointer clientData,
    XtPointer callData )
{
    XtUnmanageChild ( w );
}

void ungrab_the_____ing_pointer(XtPointer client_data)
{
  XUngrabPointer (dsp, CurrentTime);
}

mo_status mo_add_item_to_hotlist (mo_hotlist *list, mo_item_type type,
                                  char *title, char *url, int position,
				  int rbm);

void hot_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    act_struct *acst = (act_struct *) client_data;
    char *xurl;
    int i;
    mo_hot_item *hn = (mo_hot_item *) acst->str;

    switch(acst->act_code) {
    case 1: /* add item */
        mo_add_item_to_hotlist (acst->str, mo_t_url,
                                current_win->current_node->title,
                                current_win->current_node->url, 0,
				get_pref_boolean(eADD_RBM_ADDS_RBM));
        break;
    case 2: /* add list */
        if(xurl = XmxModalPromptForString (current_win->base,
                                           XtWidgetToApplicationContext(current_win->base),
                                        "Enter New List Name:",
                                        "Okay","Dismiss")){
            mo_add_item_to_hotlist (acst->str, mo_t_list, xurl, NULL, 0,
				    get_pref_boolean(eADD_RBM_ADDS_RBM));
        }

        break;

    default: /* goto link */
        if(acst->str) {
            xurl=mo_url_prepend_protocol(acst->str);
            mo_load_window_text (current_win, xurl, NULL);
        }
        break;
    }
}

void mo_destroy_hot_menu(PopupItem *pmenu)
{
    int i;
    for(i=0;pmenu[i].class != LastItem;i++) {
        if((pmenu[i].class != Separator) &&
           (pmenu[i].acst.act_code != 1) &&
	   (pmenu[i].acst.act_code != 2))
            free(pmenu[i].label);

        if(pmenu[i].class == CascadeButton)
            mo_destroy_hot_menu(pmenu[i].sub_items);
    }
    free(pmenu);
}

PopupItem *mo_assemble_hot_menu(mo_hotlist *list)
{
    mo_hot_item *item;
    char str[32];
    PopupItem *pmenu;
    int i;

        /* have to count it first. sigh */
    for(i = 0, item = list->nodelist; item ; item = item->any.next) {
	if ((item->type==mo_t_url && item->hot.rbm) ||
	    (item->type==mo_t_list && item->list.rbm)) {
		i++;
	}
    }

    pmenu = (PopupItem *) malloc(sizeof(PopupItem) * (i+5));
    for(i = 0, item = list->nodelist; item ; item = item->any.next) {

	if ((item->type==mo_t_url && !item->hot.rbm) ||
	    (item->type==mo_t_list && !item->list.rbm)) {
		continue;
	}

        pmenu[i].types = 0;
        pmenu[i].modes = 0;
        compact_string(item->hot.title, str, 31, 3, 3);
        pmenu[i].label = strdup(str);
	if(item->type == mo_t_url){
	  pmenu[i].acst.str = item->hot.url;
	  pmenu[i].acst.act_code = 69; /* identifies this as a hotlist
					  button so we can ballon it */
        }
        pmenu[i].cbfp = hot_cb;
        pmenu[i].mnemonic = 0;
        pmenu[i].accel_text = NULL;
        pmenu[i].accel = NULL;
        pmenu[i]._w = NULL;
	pmenu[i].startup=1;
        if (item->type == mo_t_url) {	/* URL item */
            pmenu[i].class = PushButton;
            pmenu[i].sub_items = NULL;
        } else {
            pmenu[i].class = CascadeButton;
            pmenu[i].sub_items = mo_assemble_hot_menu(&(item->list));
            pmenu[i].acst.act_code = 0;
        }
        i++;
    }
    pmenu[i].class = Separator;
    pmenu[i].sub_items = NULL;
    pmenu[i].label = strdup("Sep");
    i++;

    pmenu[i].class = PushButton;
    pmenu[i].label = "Add current URL...";
    pmenu[i].types = 0;
    pmenu[i].modes = 0;
    pmenu[i].cbfp = hot_cb;
    pmenu[i].acst.str = list;
    pmenu[i].acst.act_code = 1;
    pmenu[i].mnemonic = 0;
    pmenu[i].accel_text = NULL;
    pmenu[i].accel = NULL;
    pmenu[i]._w = NULL;
    pmenu[i].startup=1;
    pmenu[i].sub_items = NULL;
    i++;

    pmenu[i].class = PushButton;
    pmenu[i].label = "Add New List...";
    pmenu[i].types = 0;
    pmenu[i].modes = 0;
    pmenu[i].cbfp = hot_cb;
    pmenu[i].acst.str = list;
    pmenu[i].acst.act_code = 2;
    pmenu[i].mnemonic = 0;
    pmenu[i].accel_text = NULL;
    pmenu[i].accel = NULL;
    pmenu[i]._w = NULL;
    pmenu[i].startup=1;
    pmenu[i].sub_items = NULL;
    i++;

    pmenu[i].class = LastItem;
    return pmenu;
}

static int hot_button = 0;

void mo_init_hotlist_menu(mo_hotlist *list)
{
  /* this doesn't check the first button but that is okay because
     the first two buttons are always back and forward */
  while(popup_items[hot_button].acst.act_code != -1)
    hot_button++;

  popup_items[hot_button].sub_items = mo_assemble_hot_menu(list);
  /*  popup_items[hot_button].class = CascadeButton; not necassary */
}

void mo_reinit_hotlist_menu(mo_hotlist *list)
{
    short pos;

    if(!popup) return;

    mo_destroy_hot_menu(popup_items[hot_button].sub_items);

    popup_items[hot_button].sub_items = mo_assemble_hot_menu(list);

/*
    XtVaGetValues(popup_items[hot_button]._w,XmNpositionIndex, &pos, NULL);
*/

    XtDestroyWidget(popup);

    popup = _PopupMenuBuilder(current_win->view, XmMENU_POPUP,
			      "popup", 0, popup_items);

    /*    popup_items[hot_button]._w =
	  _PopupMenuBuilder(popup, XmMENU_PULLDOWN,
	  popup_items[hot_button].label,
	  popup_items[hot_button].mnemonic,
	  popup_items[hot_button].sub_items);*/

/*
    XtVaSetValues(popup_items[hot_button]._w,XmNpositionIndex, pos, NULL);
*/
}


char **user_defs_get_entries(FILE *fp, int *num)
{
  char **entries, str[512];
  int i=0;

  entries = malloc(sizeof(char *) * 100);

  while(fgets(str, 512, fp) != NULL)
    {
      int index=0;

      while(isspace(str[index]))
	index++;

      if(str[index] != '#' && str[index] != '\n' && str[index] != '\0')
	{
	  if(i%2)
	    { /* url spec line */
	      switch(str[index])
		{
		case 'G': /* GET: */
		case 'P': /* POST: */
		case 'F': /* FETCH: */
		  entries[i]=strdup(&(str[index]));
		  entries[i]=my_chop(entries[i]);
		  i++;
		  break;
		default: /* error */
		  fprintf(stderr, "User defined field wrong:%s. Ignoring it\n",
			  &(str[index]));
		}
	    }
	  else
	    { /* button name */
	      if(strlen(str) > 50)
		{
		  fprintf(stderr, "User defined button name too long. Ignoring it\n");
		}
	      else
		{
		  entries[i]=strdup(&(str[index]));
		  entries[i]=my_chop(entries[i]);
		  i++;
		}
	    }
	}
    }

  if(i%2 == 1)
    {
      fprintf(stderr, "Problem in gui_popup.c:%d \n", __LINE__);
      return NULL;
    }

  *num = i/2;
  return entries; /* they better free this */
}

PopupItem *build_user_defs_items(char **entries, int num)
{
  PopupItem *items = malloc(sizeof(PopupItem) * (num+1));
  int i;

  if(!items || !entries || num<=0)
    return NULL;

  for(i=0;i<num;i++)
    {
      items[i].class = PushButton;
      items[i].label = strdup(entries[i*2]);
      items[i].types = 0;
      items[i].types_method = 0;
      items[i].modes = 0;
      items[i].modes_method = 0;
      items[i].cbfp = user_defs_cb;
      items[i].acst.str = strdup(entries[i*2+1]);
      items[i].acst.act_code = 0;
      items[i].acst.eptr = NULL;
      items[i].mnemonic = 0;
      items[i].accel_text = NULL;
      items[i].accel = NULL;
      items[i]._w = NULL;
      items[i].startup=1;
      items[i].sub_items = NULL;
    }

  items[num].class = LastItem;
  items[num].sub_items = NULL;

  return items;
}

void select_cb(Widget w, XtPointer client_data, Atom *sel, Atom *type,
	       XtPointer value, unsigned long *length, int *format)
{
  char *pt, *end = NULL, *bptr,
    *select = (char *) value,
    *str = (char *) client_data,
    *nselect, *begin;
  char mode;

  begin = strdup(str); /* we don't want to fuck with str */
  bptr=begin;

  /* this filters out empty strings and most possible errors */
  if(*type != XA_STRING || !select || !*select || !begin || !*begin ||
     *length <= 0)
    return;

  /* do this cause select is not null terminated sometimes */
  pt = my_strndup(select, *length);

  if(pt)
    {
      select = pt;
      pt=NULL;
    }
  else
    {
      XtFree(value);
      free(begin);
      return;
    }

  switch(bptr[0])
    {
    case 'G': /* GET: */
      bptr+=4;
      mode='G';
      bptr=my_chop(bptr);
      break;
    case 'P': /* POST: */
      bptr+=5;
      mode='P';
      bptr=my_chop(bptr);
      break;
    case 'F': /* FETCH: */
      bptr+=6;
      mode='F';
      bptr=my_chop(bptr);
      break;
    default: /* error */
      fprintf(stderr, "User defined field wrong:%s. Ignoring it\n",
	      begin);
      XtFree(value);
      free(begin);
      return;
    }

  if(mode == 'F')
    {
      /* expand url */
      nselect = mo_url_prepend_protocol(select);
      XtFree(value); /* this frees select */
    }
  else
    {
      /* make search string sendable */
      nselect = mo_escape_part(select);
      XtFree(value); /* this frees select */
    }

  pt = strchr(bptr, '_');
  while(!end && pt && *pt)
    {
      if(!strncmp(pt, "__string__", 10))
	end = pt+10;
      else
	{
	  pt++;
	  if(pt && *pt)
	    pt = strchr(pt, '_');
	}
    }

  if(pt && *pt && end && nselect)
    {
#ifndef DISABLE_TRACE
      if(srcTrace)
	fprintf(stderr, "Popup getting %s from user menu.\n", pt);
#endif

      if(mode=='P')
	{
	  char *ptr;

	  ptr = strrchr(bptr, ' '); /* This shouldn't fail because bptr is
				       chopped */
	  ptr[0]='\0'; /* make bptr not have name value pair */
	  ptr++; /* get back to a real string */
	  bptr=my_chop(bptr);
	  ptr=my_chop(ptr);

	  pt[0] = '\0'; /* make __string__ go away from ptr */
	  pt= malloc(sizeof(char) * (strlen(end)+strlen(nselect)+strlen(ptr)+1));
	  sprintf(pt,"%s%s%s", ptr, nselect, end);

	  mo_post_access_document (current_win, bptr,
				   "application/x-www-form-urlencoded",
				   pt);
	  free(pt);
	}
      else if(mode=='G')
	{
	  pt[0] = '\0';
	  pt= malloc(sizeof(char) * (strlen(end)+strlen(bptr)+strlen(nselect)+1));
	  sprintf(pt,"%s%s%s", bptr, nselect, end);
	  mo_load_window_text(current_win, pt, NULL);
	  free(pt);
	}
      else if(mode=='F')
	{
	  mo_load_window_text(current_win, nselect, NULL);
	}
    }
  free(begin);
  free(nselect);
}

void user_defs_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  act_struct *acst = (act_struct *) client_data; /* acst->str is the url */
  XmPushButtonCallbackStruct *cbs = (XmPushButtonCallbackStruct *) call_data;
  char *str = acst->str;

  if(!str)
    return;

  XtGetSelectionValue(current_win->scrolled_win, XA_PRIMARY, XA_STRING,
		      select_cb, str, cbs->event->xbutton.time);
}

void copy_link_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  act_struct *acst = (act_struct *) client_data;
  XmPushButtonCallbackStruct *cbs = (XmPushButtonCallbackStruct *) call_data;
  char *url;

  if(!acst || !acst->eptr || !acst->eptr->anchorHRef ||
     !*acst->eptr->anchorHRef)
    return;

  url = mo_url_canonicalize(acst->eptr->anchorHRef, strdup(current_win->current_node->url));

  if(XtOwnSelection((Widget) current_win->scrolled_win, XA_PRIMARY,
		    cbs->event->xbutton.time, convert_selection,
		    NULL, NULL) == False)
    {
      fprintf(stderr, "Mosaic: Error: Could not copy selection, try again.\n");
      if (url)
	{
	  free(url);
	}
    }
  else
    {
      int i;

      for(i=0;popup_items[i].class!=LastItem; i++)
	{
	  if(!strcmp(popup_items[i].label, COPY_URL_LABEL) && url)
	    {
	      char *copy_str = malloc((strlen(url) +
			 strlen("URL:   has been copied  ")) * sizeof(char));
	      if(popup_items[i].acst.str)
		free(popup_items[i].acst.str);
	      popup_items[i].acst.str = url;

	      sprintf(copy_str, "URL: %s  has been copied", url);
	      mo_gui_notify_progress(copy_str);
	      break;
	    }
	  else if (!strcmp(popup_items[i].label, COPY_URL_LABEL) && !url)
	    {
	      if(popup_items[i].acst.str)
		free(popup_items[i].acst.str);
	      popup_items[i].acst.str=NULL;
	      break;
	    }
	}
    }
}

static Boolean convert_selection(Widget w, Atom *sel, Atom *tar, Atom *typ_ret,
				 XtPointer *val_ret, unsigned long *val_len,
				 int *format)
{
  char *url;
  int i;

  if(*tar == XA_STRING)
    {
#ifndef DISABLE_TRACE
      if (srcTrace)
	{
	  fprintf (stderr, "Pasting text selection.\n");
	}
#endif

      for(i=0;popup_items[i].class!=LastItem;i++)
	{
	  if(!strcmp(popup_items[i].label, COPY_URL_LABEL))
	    {
	      if(popup_items[i].acst.str)
		url = (char *) popup_items[i].acst.str;
	      else
		return False;
	      break;
	    }
	}

      *val_ret = strdup(url);
      *val_len = strlen(url);
      *typ_ret = XA_STRING;
      *format = 8;
      return(True);
    }
    return(False);
}

void mo_add_to_rbm_history(mo_window *win, char *url, char *title)
{
  char label[32];
  int max = get_pref_int(eNUMBER_OF_ITEMS_IN_RBM_HISTORY);
  int i;

  if(!get_pref_boolean(eSESSION_HISTORY_ON_RBM))
    return;
  else if(!win->session_menu)
    win->session_menu = XmCreatePulldownMenu(win->view, "session_menu",
					     NULL, 0);

  compact_string(title, label, 31, 3, 3);

  if(win->num_session_items < max)
    {
      win->session_items[win->num_session_items] =
	XtVaCreateManagedWidget(label, xmPushButtonGadgetClass,
				win->session_menu,NULL);

      XtAddCallback(win->session_items[win->num_session_items],
		    XmNactivateCallback, session_cb, url);
      XtAddCallback(win->session_items[win->num_session_items],
		    XmNarmCallback, rbm_ballonify, url);
      XtAddCallback(win->session_items[win->num_session_items],
		    XmNdisarmCallback, rbm_ballonify, " ");
      win->num_session_items++;
    }
  else if (win && win->session_items)
    {
      XtDestroyWidget(win->session_items[0]);

      /* scoot the widget pointers */
      for(i=0;i<max-1;i++)
	win->session_items[i] = win->session_items[i+1];

      win->session_items[max-1] =
	XtVaCreateManagedWidget(label, xmPushButtonGadgetClass,
				win->session_menu, NULL);
      XtAddCallback(win->session_items[max-1],
		    XmNactivateCallback, session_cb, url);
      XtAddCallback(win->session_items[max-1],
		    XmNarmCallback, rbm_ballonify, url);
      XtAddCallback(win->session_items[max-1],
		    XmNdisarmCallback, rbm_ballonify, " ");
    }
}

void session_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  char *xurl = (char *) client_data;

  mo_load_window_text (current_win, xurl, NULL);
}


void rbm_ballonify(Widget w, XtPointer client_data, XtPointer call_data)
{
  char *url = (char *) client_data;

  mo_gui_notify_progress(url);
}
