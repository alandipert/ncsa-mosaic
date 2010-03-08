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
#include <ctype.h>
#include <string.h>
#ifdef MOTIF
#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/List.h>
#else
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#endif /* MOTIF */
#include "HTMLP.h"


#ifdef MOTIF
#define STRING XmString
#else
#define STRING String
#endif /* MOTIF */

#define X_NAME		"x"
#define Y_NAME		"y"

#define	W_TEXTFIELD	0
#define	W_CHECKBOX	1
#define	W_RADIOBOX	2
#define	W_PUSHBUTTON	3
#define	W_PASSWORD	4
#define	W_OPTIONMENU	5
#define	W_TEXTAREA	6
#define	W_LIST		7
#define	W_JOT		8
#define	W_HIDDEN	9


extern void NewJot();
extern void ClearJot();
extern void EVJotExpose();
extern void EVJotPress();
extern void EVJotMove();
extern void EVJotRelease();
extern char *EJB_JOTfromJot();
extern char *ParseMarkTag();

#ifdef MOTIF
static Boolean ModifyIgnore = False;
#endif /* MOTIF */

char **ParseCommaList();
void FreeCommaList();
char *MapOptionReturn();

static char traversal_table[] = 
"\
~Shift ~Meta ~Ctrl <Key> Tab:  traversal_forward()\n\
Shift ~Meta ~Ctrl <Key> Tab:   traversal_back()\n\
Ctrl <Key> Tab:    traversal_end()\
";

static char text_translations[] = "\
           ~Meta ~Alt Ctrl<Key>u:	beginning-of-line()		\
					delete-to-end-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>k:	delete-to-end-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>a:	beginning-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>e:	end-of-line()   		\n\
           ~Meta ~Alt Ctrl<Key>w:	key-select()			\
					delete-selection()		\n\
           ~Meta ~Alt Ctrl<Key>y:	paste-clipboard()		\n\
	 Meta ~Ctrl       <Key>d:	delete-next-word()		\n\
	  Alt ~Ctrl       <Key>d:	delete-next-word()		\n\
           ~Meta ~Alt Ctrl<Key>d:       delete-next-character()         \n\
     Meta ~Ctrl<Key>osfBackSpace:	delete-previous-word()		\n\
      Alt ~Ctrl<Key>osfBackSpace:	delete-previous-word()		\n\
	Meta ~Ctrl<Key>osfDelete:	delete-next-word()		\n\
	 Alt ~Ctrl<Key>osfDelete:	delete-next-word()              \n\
                      <Btn1Down>:       take_focus() grab-focus() traversal_current()";


#ifndef MOTIF
#define FONTHEIGHT(font) (font->max_bounds.ascent + font->max_bounds.descent)

#ifndef DISABLE_TRACE
extern int htmlwTrace;
#endif

void
setTextSize(w, columns, lines)
	Widget w;
	int columns;
	int lines;
{
	XFontStruct *font;
	Position lm, rm, tm, bm;
	int width, height;

	XtVaGetValues(w, XtNfont, &font,
		XtNleftMargin, &lm,
		XtNrightMargin, &rm,
		XtNtopMargin, &tm,
		XtNbottomMargin, &bm,
		NULL);
	width  = rm + lm + columns * XTextWidth(font, "0", 1);
	height = tm + bm + lines * FONTHEIGHT(font);
	XtVaSetValues(w,
		XtNwidth, width,
		XtNheight, height,
		NULL);
}

void
CBListDestroy(w, client_data, call_data)
	Widget w;
	caddr_t client_data;
	caddr_t call_data;
{
	char **string_list=NULL, **p;
	int item_count;

	XtVaGetValues(w,
		XtNlist, string_list,
		XtNnumberStrings, &item_count, /*ddt4/3/95*/
		NULL);

	p = string_list;
	while(item_count > 0)
	{
		free(*p++);
		item_count--;
	}
	free(string_list);
}


void
CBTextDestroy(w, client_data, call_data)
	Widget w;
	caddr_t client_data;
	caddr_t call_data;
{
	char *txt = (char *)client_data;
	free(txt);
}


void
CBoption(w, client_data, call_data)
	Widget w;
	caddr_t client_data;
	caddr_t call_data;
{
	Widget menuButton = (Widget)client_data;
	char *label;

	XtVaGetValues(menuButton, XtNlabel, &label, NULL);
	XtVaGetValues(w, XtNlabel, &label, NULL);
	XtVaSetValues(menuButton, XtNlabel, label, NULL);
}
#endif /* not MOTIF */


void
AddNewForm(hw, fptr)
	HTMLWidget hw;
	FormInfo *fptr;
{
	FormInfo *ptr;

	ptr = hw->html.form_list;
	if (ptr == NULL)
	{
		hw->html.form_list = fptr;
		fptr->next = NULL;
	}
	else
	{
		while (ptr->next != NULL)
		{
			ptr = ptr->next;
		}
		ptr->next = fptr;
		fptr->next = NULL;
	}
}


int
CollectSubmitInfo(fptr, name_list, value_list)
	FormInfo *fptr;
	char ***name_list;
	char ***value_list;
{
	HTMLWidget hw = (HTMLWidget)(fptr->hw);
	WbFormCallbackData cbdata;
	WidgetInfo *wptr;
	int cnt;
#ifndef MOTIF
	Boolean state;
#endif

	if (fptr->end == -1)  /* unterminated FORM tag */
	{
		wptr = hw->html.widget_list;
		cnt = 0;
		while (wptr != NULL)
		{
			cnt++;
			wptr = wptr->next;
		}
		cbdata.attribute_count = cnt;
	}
	else
	{
		cbdata.attribute_count = fptr->end - fptr->start;
	}
	cbdata.attribute_names = (char **)malloc(cbdata.attribute_count *
		sizeof(char *));
	cbdata.attribute_values = (char **)malloc(cbdata.attribute_count *
		sizeof(char *));

	if (fptr->start == 0)
	{
		wptr = hw->html.widget_list;
	}
	else
	{
		wptr = hw->html.widget_list;
		while (wptr != NULL)
		{
			if (wptr->id == fptr->start)
			{
				wptr = wptr->next;
				break;
			}
			wptr = wptr->next;
		}
	}

	cnt = 0;

	while ((wptr != NULL)&&(cnt < cbdata.attribute_count))
	{
	/***   cvarela@ncsa.uiuc.edu:  August 17, 1994
               Adding multiple submit buttons support
         ***   changed to match widgets -- amb ***/
           if (wptr->name)

	    {
		Widget child;
		STRING *str_list;
		int list_cnt;
		char *val;
#ifdef MOTIF
		STRING label;
		Cardinal argcnt;
		Arg arg[5];
#else
		XawListReturnStruct *currentSelection;
#endif /* MOTIF */

		cbdata.attribute_names[cnt] = wptr->name;
		switch(wptr->type)
		{
			case W_TEXTFIELD:
#ifdef MOTIF
				cbdata.attribute_values[cnt] =
					XmTextFieldGetString(wptr->w);
#else
				XtVaGetValues(wptr->w, XtNstring,
					&(cbdata.attribute_values[cnt]),
					NULL);
#endif /* MOTIF */
				if ((cbdata.attribute_values[cnt] != NULL)&&
				    (cbdata.attribute_values[cnt][0] == '\0'))
				{
					cbdata.attribute_values[cnt] = NULL;
				}
				break;
			case W_TEXTAREA:
#ifdef MOTIF
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNworkWindow, &child);
				argcnt++;
				XtGetValues(wptr->w, arg, argcnt);
				cbdata.attribute_values[cnt] =
					XmTextGetString(child);
#else
				XtVaGetValues(wptr->w, XtNstring,
					&(cbdata.attribute_values[cnt]),
					NULL);
#endif /* MOTIF */
				if ((cbdata.attribute_values[cnt] != NULL)&&
				    (cbdata.attribute_values[cnt][0] == '\0'))
				{
					cbdata.attribute_values[cnt] = NULL;
				}
				break;
			case W_PASSWORD:
				cbdata.attribute_values[cnt] = wptr->password;
				if ((cbdata.attribute_values[cnt] != NULL)&&
				    (cbdata.attribute_values[cnt][0] == '\0'))
				{
					cbdata.attribute_values[cnt] = NULL;
				}
				break;
			case W_LIST:
				/*
				 * First get the Widget ID of the proper
				 * list element
				 */
#ifdef MOTIF
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNworkWindow, &child);
				argcnt++;
				XtGetValues(wptr->w, arg, argcnt);
#else
				{
					WidgetList wl;
					XtVaGetValues(wptr->w, XtNchildren,
						&wl, NULL);
					child = *++wl;
				}
#endif /* MOTIF */

				/*
				 * Now get the list of selected items.
				 */
#ifdef MOTIF
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNselectedItemCount,
					&list_cnt);
				argcnt++;
				XtSetArg(arg[argcnt], XmNselectedItems,
					&str_list);
				argcnt++;
				XtGetValues(child, arg, argcnt);
#else
				currentSelection = XawListShowCurrent(child);
				list_cnt =
				  currentSelection->list_index == XAW_LIST_NONE?
					0 : 1;
				str_list = &(currentSelection->string);
#endif /* MOTIF */

				if (list_cnt == 0)
				{
					cnt--;
					cbdata.attribute_count--;
				}
				else /* list_cnt >= 1 */
				{
					int j, new_cnt;
					char **names;
					char **values;

					if (list_cnt > 1)
					{
					    new_cnt = cbdata.attribute_count +
						list_cnt - 1;
					    names = (char **)malloc(new_cnt *
						sizeof(char *));
					    values = (char **)malloc(new_cnt *
						sizeof(char *));
					    for (j=0; j<cnt; j++)
					    {
						names[j] =
						    cbdata.attribute_names[j];
						values[j] =
						    cbdata.attribute_values[j];
					    }
					    free((char *)
						cbdata.attribute_names);
					    free((char *)
						cbdata.attribute_values);
					    cbdata.attribute_names = names;
					    cbdata.attribute_values = values;
					    cbdata.attribute_count = new_cnt;
					}

					for (j=0; j<list_cnt; j++)
					{
						cbdata.attribute_names[cnt + j]
							= wptr->name;
#ifdef MOTIF
						XmStringGetLtoR(str_list[j],
						    XmSTRING_DEFAULT_CHARSET,
						    &val);
#else
						val = str_list[j];
#endif /* MOTIF */
						if ((val != NULL)&&
							(val[0] == '\0'))
						{
							val = NULL;
						}
						else if (val != NULL)
						{
							val = MapOptionReturn(
								val,
								wptr->mapping);
						}
						cbdata.attribute_values[cnt + j]
							= val;
					}
					cnt = cnt + list_cnt - 1;
				}
				break;
			/*
			 * For an option menu, first get the label gadget
			 * which holds the current value.
			 * Now get the text from that label as a character
			 * string.
			 */
			case W_OPTIONMENU:
#ifdef MOTIF
				child = XmOptionButtonGadget(wptr->w);
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNlabelString, &label);
				argcnt++;
				XtGetValues(child, arg, argcnt);
				val = NULL;
				XmStringGetLtoR(label, XmSTRING_DEFAULT_CHARSET,
					&val);
#else
				XtVaGetValues(wptr->w, XtNlabel, &val, NULL);
#endif /* MOTIF */
				if ((val != NULL)&&(val[0] == '\0'))
				{
					val = NULL;
				}
				else if (val != NULL)
				{
					val = MapOptionReturn(val,
						wptr->mapping);
				}
				cbdata.attribute_values[cnt] = val;
				if ((cbdata.attribute_values[cnt] != NULL)&&
				    (cbdata.attribute_values[cnt][0] == '\0'))
				{
					cbdata.attribute_values[cnt] = NULL;
				}
				break;
			case W_CHECKBOX:
			case W_RADIOBOX:
#ifdef MOTIF
				if (XmToggleButtonGetState(wptr->w) == True)
#else
				XtVaGetValues(wptr->w, XtNstate, &state, NULL);
				if (state)
#endif /* MOTIF */
				{
				    cbdata.attribute_values[cnt] = wptr->value;
				}
				else
				{
				    cnt--;
				    cbdata.attribute_count--;
				}
				break;
 
		       /*** cvarela@ncsa.uiuc.edu:  August 17, 1994
		            Adding multiple submit buttons support ***/
				/* mods 3/11/95  -- amb */
                       case W_PUSHBUTTON:
                               if (fptr->button_pressed == wptr->w){
                                   cbdata.attribute_values[cnt] = wptr->value;
                               }
                               else
                               {
                                   cnt--;
                                   cbdata.attribute_count--;
                               }
                               break;
			/**/

			case W_HIDDEN:
				cbdata.attribute_values[cnt] = wptr->value;
				break;
#ifdef MOTIF
			case W_JOT:
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNuserData,
					(XtPointer *)&child);
				argcnt++;
				XtGetValues(wptr->w, arg, argcnt);
				cbdata.attribute_values[cnt] =
					EJB_JOTfromJot(child);
				break;
#endif /* MOTIF */
			default:
				cbdata.attribute_values[cnt] = NULL;
				break;
		}
		cnt++;
	    }
	    else
	    {
		cbdata.attribute_count--;
	    }
	    wptr = wptr->next;
	}
	cbdata.attribute_count = cnt;

	*name_list = cbdata.attribute_names;
	*value_list = cbdata.attribute_values;
	return(cbdata.attribute_count);
}


void
ImageSubmitForm(fptr, event, name, x, y)
	FormInfo *fptr;
	XEvent *event;
	char *name;
	int x, y;
{
	HTMLWidget hw = (HTMLWidget)(fptr->hw);
	WbFormCallbackData cbdata;
	int i, cnt;
	char **name_list;
	char **value_list;
	char valstr[100];

	cbdata.event = event;
	cbdata.href = fptr->action;
	cbdata.format = fptr->format;
        cbdata.method = fptr->method;
        cbdata.enctype = fptr->enctype;
        cbdata.enc_entity = fptr->enc_entity;

	name_list = NULL;
	value_list = NULL;
	cnt = CollectSubmitInfo(fptr, &name_list, &value_list);

	cbdata.attribute_count = cnt + 2;
	cbdata.attribute_names = (char **)malloc(cbdata.attribute_count *
		sizeof(char *));
	cbdata.attribute_values = (char **)malloc(cbdata.attribute_count *
		sizeof(char *));
	for (i=0; i<cnt; i++)
	{
		cbdata.attribute_names[i] = name_list[i];
		cbdata.attribute_values[i] = value_list[i];
	}
	if (name_list != NULL)
	{
		free((char *)name_list);
	}
	if (value_list != NULL)
	{
		free((char *)value_list);
	}

	if ((name != NULL)&&(name[0] != '\0'))
	{
		cbdata.attribute_names[cnt] = (char *)malloc(strlen(name) +
			strlen(X_NAME) + 2);
		strcpy(cbdata.attribute_names[cnt], name);
		strcat(cbdata.attribute_names[cnt], ".");
		strcat(cbdata.attribute_names[cnt], X_NAME);
	}
	else
	{
		cbdata.attribute_names[cnt] = (char *)malloc(strlen(X_NAME) +1);
		strcpy(cbdata.attribute_names[cnt], X_NAME);
	}
	sprintf(valstr, "%d", x);
	cbdata.attribute_values[cnt] = (char *)malloc(strlen(valstr) + 1);
	strcpy(cbdata.attribute_values[cnt], valstr);

	cnt++;
	if ((name != NULL)&&(name[0] != '\0'))
	{
		cbdata.attribute_names[cnt] = (char *)malloc(strlen(name) +
			strlen(Y_NAME) + 2);
		strcpy(cbdata.attribute_names[cnt], name);
		strcat(cbdata.attribute_names[cnt], ".");
		strcat(cbdata.attribute_names[cnt], Y_NAME);
	}
	else
	{
		cbdata.attribute_names[cnt] = (char *)malloc(strlen(Y_NAME) +1);
		strcpy(cbdata.attribute_names[cnt], Y_NAME);
	}
	sprintf(valstr, "%d", y);
	cbdata.attribute_values[cnt] = (char *)malloc(strlen(valstr) + 1);
	strcpy(cbdata.attribute_values[cnt], valstr);

	XtCallCallbackList ((Widget)hw, hw->html.form_callback,
		(XtPointer)&cbdata);
}


void
CBSubmitForm(w, client_data, call_data)
	Widget w;
	caddr_t client_data;
	caddr_t call_data;
{
	FormInfo *fptr = (FormInfo *)client_data;
	HTMLWidget hw = (HTMLWidget)(fptr->hw);
	WbFormCallbackData cbdata;
#ifdef MOTIF
	XmPushButtonCallbackStruct *pb =
		(XmPushButtonCallbackStruct *)call_data;
#endif /* MOTIF */

#ifdef MOTIF
	cbdata.event = pb->event;
#else
	/******* WE HAVE NO EVENT in ATHENA *******/
	cbdata.event = NULL;
#endif /* MOTIF */
	cbdata.href = fptr->action;
        cbdata.method = fptr->method;
	cbdata.format = fptr->format;
        cbdata.enctype = fptr->enctype;
        cbdata.enc_entity = fptr->enc_entity;
	fptr->button_pressed = w;

	cbdata.attribute_count = CollectSubmitInfo(fptr,
		&cbdata.attribute_names, &cbdata.attribute_values);

	XtCallCallbackList ((Widget)hw, hw->html.form_callback,
		(XtPointer)&cbdata);
}


/*
 * A radio buttom was toggled on in a form.
 * If there are other radios of the same name, turn them off.
 */
void
CBChangeRadio(w, client_data, call_data)
	Widget w;
	caddr_t client_data;
	caddr_t call_data;
{
	FormInfo *fptr = (FormInfo *)client_data;
	HTMLWidget hw = (HTMLWidget)(fptr->hw);
	WidgetInfo *wptr;
	WidgetInfo *wtmp;
	char *name;
	int cnt, count;
#ifdef MOTIF
	XmToggleButtonCallbackStruct *tb =
		(XmToggleButtonCallbackStruct *)call_data;
#else
	Boolean state;
#endif /* MOTIF */

#ifdef MOTIF
	/*
	 * Bad button
	 */
	if (tb == NULL)
	{
		return;
	}
#endif /* MOTIF */

	/*
	 * Only do stuff when the button is turned on.
	 * Don't let the button be turned off, by clicking on
	 * it, as that would leave all buttons off.
	 */
#ifdef MOTIF
	if ((tb == NULL)||(tb->set == False))
	{
		XmToggleButtonSetState(w, True, False);
		return;
	}
#else
	XtVaGetValues(w, XtNstate, &state, NULL);
	if (!state)
	{
		XtVaSetValues(w, XtNstate, 1, NULL);
		return;
	}
#endif /* MOTIF */

	/*
	 * Terminate the form if it was never properly terminated.
	 */
	if (fptr->end == -1)  /* unterminated FORM tag */
	{
		wptr = hw->html.widget_list;
		cnt = 0;
		while (wptr != NULL)
		{
			cnt++;
			wptr = wptr->next;
		}
		count = cnt;
	}
	else
	{
		count = fptr->end - fptr->start;
	}

	/*
	 * Locate the start of the form.
	 */
	if (fptr->start == 0)
	{
		wptr = hw->html.widget_list;
	}
	else
	{
		wptr = hw->html.widget_list;
		while (wptr != NULL)
		{
			if (wptr->id == fptr->start)
			{
				wptr = wptr->next;
				break;
			}
			wptr = wptr->next;
		}
	}

	/*
	 * Find the name of the toggle button just pressed.
	 */
	name = NULL;
	wtmp = wptr;
	while (wtmp != NULL)
	{
		if (wtmp->w == w)
		{
			name = wtmp->name;
			break;
		}
		wtmp = wtmp->next;
	}

	/*
	 * Check for other checked radioboxes of the same name.
	 */
	cnt = 0;
	while ((wptr != NULL)&&(cnt < count))
	{
#ifdef MOTIF
		if ((wptr->type == W_RADIOBOX)&&
			(wptr->w != w)&&
			(XmToggleButtonGetState(wptr->w) == True)&&
			(wptr->name != NULL)&&
			(name != NULL)&&
			(strcmp(wptr->name, name) == 0))
		{
			XmToggleButtonSetState(wptr->w, False, False);
		}
#else
		if ((wptr->type == W_RADIOBOX)&&
			(wptr->w != w)&&
			(wptr->name != NULL)&&
			(name != NULL)&&
			(strcmp(wptr->name, name) == 0))
		{
			XtVaGetValues(wptr->w, XtNstate, &state, NULL);
			if (state)
			{
				XtVaSetValues(wptr->w, XtNstate, 0, NULL);
			}
		}
#endif /* MOTIF */
		cnt++;
		wptr = wptr->next;
	}
}


#ifdef MOTIF
/*
 * Catch all attempted modifications to the textfield for password
 * entry.  This is so we can prevent the password from showing
 * uponm the screen.
 * I would prefer that for all insereted characters a random 1-3 '*'s
 * were added, and any delete deleted the whole string, but due to
 * bugs in somve version of Motif 1.1 this won't work.
 */
void
CBPasswordModify(w, client_data, call_data)
	Widget w;
	caddr_t client_data;
	caddr_t call_data;
{
	FormInfo *fptr = (FormInfo *)client_data;
	XmTextVerifyCallbackStruct *tv =(XmTextVerifyCallbackStruct *)call_data;
	HTMLWidget hw = (HTMLWidget)(fptr->hw);
	WidgetInfo *wptr;
	int i, len;

	/*
	 * by default accept nothing
	tv->doit = False;
	 */

	/*
	 * Ignore when ModifyIgnore is true
	 */
	if (ModifyIgnore == True)
	{
		return;
	}

	/*
	 * only accept text modification of password fields
	 */
	if (tv->reason != XmCR_MODIFYING_TEXT_VALUE)
	{
		return;
	}

	/*
	 * find the structure for this widget
	 */
	wptr = hw->html.widget_list;
	while (wptr != NULL)
	{
		if (wptr->w == w)
		{
			break;
		}
		wptr = wptr->next;
	}
	if (wptr == NULL)
	{
		return;
	}

	/*
	 * Deletion.
	 */
	if (tv->text->ptr == NULL)
	{
		tv->doit = True;

		/*
		 * Only can delete if we have stuff to delete.
		 */
		if ((wptr->password != NULL)&&(wptr->password[0] != '\0'))
		{
			int start;
			char *tptr;

			len = strlen(wptr->password);
			/*
			 * Find the start of the chunk of text to
			 * delete.
			 */
			if (tv->startPos < len)
			{
				start = tv->startPos;
			}
			else
			{
				start = len - 1;
			}

			/*
			 * might be more stuff after the end that we
			 * want to move up
			 */
			if (tv->endPos > len)
			{
				tptr = &(wptr->password[len]);
			}
			else
			{
				tptr = &(wptr->password[tv->endPos]);
			}
			wptr->password[start] = '\0';
			strcat(wptr->password, tptr);
		}
	}
	/*
	 * Else insert character.
	 */
	else if (tv->text->length >= 1)
	{
		int maxlength, plen;
		Cardinal argcnt;
		Arg arg[5];

		/*
		 * No insertion if it makes you exceed maxLength
		 */
		if (wptr->password == NULL)
		{
			plen = 0;
		}
		else
		{
			plen = strlen(wptr->password);
		}
		maxlength = 1000000;
		argcnt = 0;
		XtSetArg(arg[argcnt], XmNmaxLength, &maxlength); argcnt++;
		XtGetValues(w, arg, argcnt);
		if ((plen + tv->text->length) > maxlength)
		{
			return;
		}

		if (wptr->password == NULL)
		{
			wptr->password = (char *)malloc(tv->text->length + 1);
			for (i=0; i < tv->text->length; i++)
			{
				wptr->password[i] = tv->text->ptr[i];
			}
			wptr->password[tv->text->length] = '\0';
		}
		/*
		 * else insert a char somewhere.
		 * Make a new buffer.  Put everything from before the insert
		 * postion into it.  Now insert the character.
		 * Finally append any remaining text.
		 */
		else
		{
			char *buf;
			char *tptr;
			char tchar;
			int start;

			len = strlen(wptr->password);
			if (tv->startPos < len)
			{
				start = tv->startPos;
			}
			else
			{
				start = len;
			}
			tptr = &(wptr->password[start]);
			tchar = *tptr;
			*tptr = '\0';
			buf = (char *)malloc(len + tv->text->length + 1);
			strcpy(buf, wptr->password);
			for (i=0; i < tv->text->length; i++)
			{
				buf[start + i] = tv->text->ptr[i];
			}
			buf[start + tv->text->length] = '\0';
			*tptr = tchar;
			strcat(buf, tptr);
			free(wptr->password);
			wptr->password = buf;
		}

		tv->doit = True;
		/*
		 * make a '*' show up instead of what they typed
		 */
		for (i=0; i < tv->text->length; i++)
		{
			tv->text->ptr[i] = '*';
		}
	}
}
#endif /* MOTIF */



/*
 * RETURN was hit in a textfield in a form.
 * If this is the only textfield in this form, submit the form.
 */
void
CBActivateField(w, client_data, call_data)
	Widget w;
	caddr_t client_data;
	caddr_t call_data;
{
	FormInfo *fptr = (FormInfo *)client_data;
	HTMLWidget hw = (HTMLWidget)(fptr->hw);
	WidgetInfo *wptr;
	int cnt, count;
#ifdef MOTIF
	XmAnyCallbackStruct *cb = (XmAnyCallbackStruct *)call_data;
#endif /* MOTIF */

	/*
	 * Terminate the form if it was never properly terminated.
	 */
	if (fptr->end == -1)  /* unterminated FORM tag */
	{
		wptr = hw->html.widget_list;
		cnt = 0;
		while (wptr != NULL)
		{
			cnt++;
			wptr = wptr->next;
		}
		count = cnt;
	}
	else
	{
		count = fptr->end - fptr->start;
	}

	/*
	 * Locate the start of the form.
	 */
	if (fptr->start == 0)
	{
		wptr = hw->html.widget_list;
	}
	else
	{
		wptr = hw->html.widget_list;
		while (wptr != NULL)
		{
			if (wptr->id == fptr->start)
			{
				wptr = wptr->next;
				break;
			}
			wptr = wptr->next;
		}
	}

	/*
	 * Count the textfields in this form.
	 */
	cnt = 0;
	while ((wptr != NULL)&&(cnt < count))
	{
		if ((wptr->type == W_TEXTFIELD)||(wptr->type == W_PASSWORD))
		{
			cnt++;
		}
		wptr = wptr->next;
	}

	/*
	 * If this is the only textfield in this form, submit the form.
	 */
	if (cnt == 1)
	{
		CBSubmitForm(w, client_data, call_data);
	}
}


void
CBResetForm(w, client_data, call_data)
	Widget w;
	caddr_t client_data;
	caddr_t call_data;
{
	FormInfo *fptr = (FormInfo *)client_data;
	HTMLWidget hw = (HTMLWidget)(fptr->hw);
	WidgetInfo *wptr;
	int widget_count, cnt;
#ifdef MOTIF
	XmPushButtonCallbackStruct *pb =
		(XmPushButtonCallbackStruct *)call_data;
#endif /* MOTIF */

	if (fptr->end == -1)  /* unterminated FORM tag */
	{
		wptr = hw->html.widget_list;
		cnt = 0;
		while (wptr != NULL)
		{
			cnt++;
			wptr = wptr->next;
		}
		widget_count = cnt;
	}
	else
	{
		widget_count = fptr->end - fptr->start;
	}

	if (fptr->start == 0)
	{
		wptr = hw->html.widget_list;
	}
	else
	{
		wptr = hw->html.widget_list;
		while (wptr != NULL)
		{
			if (wptr->id == fptr->start)
			{
				wptr = wptr->next;
				break;
			}
			wptr = wptr->next;
		}
	}

	cnt = 0;
	while ((wptr != NULL)&&(cnt < widget_count))
	{
		Widget child;
		STRING label;
#ifdef MOTIF
		Cardinal argcnt;
		Arg arg[5];
#else
		char *txt = NULL;
		int length = 0;
		Boolean stringInPlace;
#endif /* MOTIF */

		switch(wptr->type)
		{
			case W_TEXTFIELD:
#ifdef MOTIF
				if (wptr->value == NULL)
				{
				    XmTextFieldSetString(wptr->w, "");
				}
				else
				{
				    XmTextFieldSetString(wptr->w, wptr->value);
				}
#else
				XtVaGetValues(wptr->w,
					XtNuseStringInPlace, &stringInPlace,
					XtNlength, &length,
					NULL);
				if (stringInPlace)
				{
					XtVaGetValues(wptr->w,
						XtNstring, &txt,
						NULL);
				}
				if (wptr->value == NULL)
				{
					if (stringInPlace)
					{
						if (txt) *txt = '\0';
						XtVaSetValues(wptr->w,
							XtNstring, txt, NULL);
					}
					else
					{
						XtVaSetValues(wptr->w,
							XtNstring, "", NULL);
					}
				}
				else
				{
					if (stringInPlace)
					{
						strncpy(txt,wptr->value,length);
						XtVaSetValues(wptr->w,
							XtNstring, txt, NULL);
					}
					else
					{
						XtVaSetValues(wptr->w,
							XtNstring, wptr->value,
							NULL);
					}
				}
#endif /* MOTIF */
				break;
			case W_TEXTAREA:
#ifdef MOTIF
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNworkWindow, &child);
				argcnt++;
				XtGetValues(wptr->w, arg, argcnt);
				if (wptr->value == NULL)
				{
				    XmTextSetString(child, "");
				}
				else
				{
				    XmTextSetString(child, wptr->value);
				}
#else
				XtVaSetValues(wptr->w, XtNstring,
					wptr->value ? wptr->value : "",
					NULL);
#endif /* MOTIF */
				break;
			case W_PASSWORD:
				if (wptr->value == NULL)
				{
#ifdef MOTIF
				    /*
				     * Due to errors in Motif1.1, I can't
				     * call XmTextFieldSetString() here.
				     * Because I have a modifyVerify callback
				     * registered for this widget.
				     * I don't know if this error exists
				     * in Motif1.2 or not.
				     */
				    argcnt = 0;
				    XtSetArg(arg[argcnt], XmNvalue, "");
				    argcnt++;
				    XtSetValues(wptr->w, arg, argcnt);
#else
				    XtVaSetValues(wptr->w,
					XtNstring, "",  NULL);
#endif /* MOTIF */
				    if (wptr->password != NULL)
				    {
					free(wptr->password);
					wptr->password = NULL;
				    }
				}
				else
				{
				    int i, len;

				    if (wptr->password != NULL)
				    {
					free(wptr->password);
					wptr->password = NULL;
				    }
				    len = strlen(wptr->value);
				    wptr->password = (char *)malloc(len + 1);
				    for (i=0; i<len; i++)
				    {
					wptr->password[i] = '*';
				    }
				    wptr->password[len] = '\0';
#ifdef MOTIF
				    XmTextFieldSetString(wptr->w,
					wptr->password);
#else
				    XtVaSetValues(wptr->w,
					XtNstring, wptr->password,
					NULL);
#endif /* MOTIF */
				    strcpy(wptr->password, wptr->value);
				}
				break;
			case W_LIST:
			    {
				char **vlist;
				int vlist_cnt;
				STRING *val_list;
				int i;

#ifdef MOTIF
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNworkWindow, &child);
				argcnt++;
				XtGetValues(wptr->w, arg, argcnt);
#else
				WidgetList wl;
				char **string_list;
				int list_cnt;

				XtVaGetValues(wptr->w, XtNchildren, &wl, NULL);
				child = *++wl;
				XtVaGetValues(child,
					XtNlist, &string_list,
					XtNnumberStrings, &list_cnt, NULL);
#endif /* MOTIF */

				if (wptr->value != NULL)
				{
				    vlist = ParseCommaList(wptr->value,
					&vlist_cnt);
				    val_list = (STRING *)malloc(vlist_cnt *
					sizeof(STRING));
#ifdef MOTIF
				    XmListDeselectAllItems(child);
				    for (i=0; i<vlist_cnt; i++)
				    {
					val_list[i] =
						XmStringCreateSimple(vlist[i]);
				    }
#else
				    XawListUnhighlight(child);
				    for (i=0; i<vlist_cnt; i++)
				    {
					val_list[i] =
						XtNewString(vlist[i]);
				    }
#endif /* MOTIF */
				    FreeCommaList(vlist, vlist_cnt);
#ifdef MOTIF
				    if (vlist_cnt > 0)
				    {
					argcnt = 0;
					XtSetArg(arg[argcnt], XmNselectedItems,
						val_list);
					argcnt++;
					XtSetArg(arg[argcnt],
						XmNselectedItemCount,
						vlist_cnt);
					argcnt++;
					XtSetValues(child, arg, argcnt);
				    }
				    for (i=0; i<vlist_cnt; i++)
				    {
					XmStringFree(val_list[i]);
				    }
#else
				    if (vlist_cnt > 0)
				    {
					if (vlist_cnt > 1)
					{
#ifndef DISABLE_TRACE
						if (htmlwTrace) {
							fprintf(stderr,
								"HTML: only a single selection allowed!\n");
						}
#endif
					}

					for (i=0; i<list_cnt; i++)
					{
					    if (!strcmp(string_list[i],
						val_list[0]))
					    {
						XawListHighlight(child, i);
						break;
					    }
					}
				    }
				    for (i=0; i<vlist_cnt; i++)
				    {
					free(val_list[i]);
				    }
#endif /* MOTIF */
				    if (val_list != NULL)
				    {
					free((char *)val_list);
				    }
				}
				else
				{
#ifdef MOTIF
					XmListDeselectAllItems(child);
#else
					XawListUnhighlight(child);
#endif /* MOTIF */
				}
			    }
				break;
			/*
			 * gack, we saved the widget id of the starting default
			 * into the value character pointer, just so we could
			 * yank it out here, and restore the default.
			 */
			case W_OPTIONMENU:
				if (wptr->value != NULL)
				{
					Widget hist = (Widget)wptr->value;
#ifdef MOTIF
					Cardinal argcnt;
					Arg arg[5];

					argcnt = 0;
					XtSetArg(arg[argcnt], XmNmenuHistory,
						hist);
					argcnt++;
					XtSetValues(wptr->w, arg, argcnt);
#else
					char *txt;

					XtVaGetValues(hist, XtNlabel,&txt,NULL);
					XtVaSetValues(wptr->w,XtNlabel,txt,NULL);
#endif /* MOTIF */
				}
				break;
			case W_CHECKBOX:
			case W_RADIOBOX:
#ifdef MOTIF
				if (wptr->checked == True)
				{
				  XmToggleButtonSetState(wptr->w, True, False);
				}
				else
				{
				  XmToggleButtonSetState(wptr->w, False, False);
				}
#else
				XtVaSetValues(wptr->w,
					XtNstate, wptr->checked, NULL);
#endif /* MOTIF */
				break;
			case W_HIDDEN:
				break;
#ifdef MOTIF
			case W_JOT:
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNuserData,
					(XtPointer *)&child);
				argcnt++;
				XtGetValues(wptr->w, arg, argcnt);
				ClearJot(hw, child, wptr->width, wptr->height);
				break;
#endif /* MOTIF */
			default:
				break;
		}
		cnt++;
		wptr = wptr->next;
	}
}


void
PrepareFormEnd(hw, w, fptr)
	HTMLWidget hw;
	Widget w;
	FormInfo *fptr;
{
#ifdef MOTIF
	XtAddCallback(w, XmNactivateCallback, 
                      (XtCallbackProc)CBSubmitForm, (caddr_t)fptr);
#else
	XtAddCallback(w, XtNcallback,
		      (XtCallbackProc)CBSubmitForm, (caddr_t)fptr);
#endif /* MOTIF */
}


void
PrepareFormReset(hw, w, fptr)
	HTMLWidget hw;
	Widget w;
	FormInfo *fptr;
{
#ifdef MOTIF
	XtAddCallback(w, XmNactivateCallback, 
                      (XtCallbackProc)CBResetForm, (caddr_t)fptr);
#else
	XtAddCallback(w, XtNcallback,
		     (XtCallbackProc)CBResetForm, (caddr_t)fptr);
#endif /* MOTIF */
}


void
HideWidgets(hw)
	HTMLWidget hw;
{
	WidgetInfo *wptr;
	XEvent event;

#ifdef MOTIF
	/*
	 * Make sure all expose events have been dealt with first.
	 */
	XmUpdateDisplay((Widget)hw);
#endif /* MOTIF */

	wptr = hw->html.widget_list;
	while (wptr != NULL)
	{
		if ((wptr->w != NULL)&&(wptr->mapped == True))
		{
			XtSetMappedWhenManaged(wptr->w, False);
			wptr->mapped = False;
		}
		wptr = wptr->next;
	}

	/*
	 * Force the exposure events into the queue
	 */
	XSync(XtDisplay(hw), False);

	/*
	 * Remove all Expose events for the view window
	 */
	while (XCheckWindowEvent(XtDisplay(hw->html.view),
		XtWindow(hw->html.view), ExposureMask, &event) == True)
	{
	}
}


void
MapWidgets(hw)
	HTMLWidget hw;
{
	WidgetInfo *wptr;

	wptr = hw->html.widget_list;
	while (wptr != NULL)
	{
		if ((wptr->w != NULL)&&(wptr->mapped == False))
		{
			wptr->mapped = True;
			XtSetMappedWhenManaged(wptr->w, True);
		}
		wptr = wptr->next;
	}
}


Boolean
AlreadyChecked(hw, fptr, name)
	HTMLWidget hw;
	FormInfo *fptr;
	char *name;
{
	WidgetInfo *wptr;
	Boolean radio_checked;

	radio_checked = False;
	wptr = hw->html.widget_list;
	while (wptr != NULL)
	{
		if ((wptr->id >= fptr->start)&&
			(wptr->type == W_RADIOBOX)&&
			(wptr->checked == True)&&
			(wptr->name != NULL)&&
			(name != NULL)&&
			(strcmp(wptr->name, name) == 0))
		{
			radio_checked = True;
			break;
		}
		wptr = wptr->next;
	}
	return(radio_checked);
}


WidgetInfo *
AddNewWidget(hw, fptr, w, type, id, x, y, width, height, name, value, mapping, checked)
	HTMLWidget hw;
	FormInfo *fptr;
	Widget w;
	int type;
	int id;
	int x, y;
	int width, height;
	char *name;
	char *value;
	char **mapping;
	Boolean checked;
{
	WidgetInfo *wptr, *lptr;

	wptr = hw->html.widget_list;
	if (wptr == NULL)
	{
		wptr = (WidgetInfo *)malloc(sizeof(WidgetInfo));
		wptr->w = w;
		wptr->type = type;
		wptr->id = id;
		wptr->x = x;
		wptr->y = y;
		wptr->width = width;
		wptr->height = height;
		wptr->seeable=0;
		wptr->name = name;
		wptr->value = value;
		wptr->password = NULL;
		wptr->mapping = mapping;
		wptr->checked = checked;
		wptr->mapped = False;
		wptr->next = NULL;
		wptr->prev = NULL;
		hw->html.widget_list = wptr;
	}
	else
	{
		while (wptr->next != NULL)
		{
			wptr = wptr->next;
		}
		wptr->next = (WidgetInfo *)malloc(sizeof(WidgetInfo));
		lptr = wptr; /* save this to fill in prev field */
		wptr = wptr->next;
		wptr->prev = lptr;
		wptr->w = w;
		wptr->type = type;
		wptr->id = id;
		wptr->x = x;
		wptr->y = y;
		wptr->width = width;
		wptr->height = height;
		wptr->seeable=0;
		wptr->name = name;
		wptr->value = value;
		wptr->password = NULL;
		wptr->mapping = mapping;
		wptr->checked = checked;
		wptr->mapped = False;
		wptr->next = NULL;
	}

	if ((wptr->type == W_PASSWORD)&&(wptr->value != NULL))
	{
		wptr->password = (char *)malloc(strlen(wptr->value) + 1);
		strcpy(wptr->password, wptr->value);
	}

	return(wptr);
}


/*
 * For the various widgets, return their fon structures so
 * we can use the font's baseline to place them.
 */
XFontStruct *
GetWidgetFont(hw, wptr)
	HTMLWidget hw;
	WidgetInfo *wptr;
{
	Widget child;
	XFontStruct *font;
#ifdef MOTIF
	Boolean ret;
	Cardinal argcnt;
	Arg arg[5];
	XmFontList font_list = (XmFontList)NULL;
	XmFontContext font_context;
	XmStringCharSet charset;
#endif /* MOTIF */

	/*
	 * For option menus we have to first get the child that has the
	 * font info.
	 */
	if (wptr->type == W_OPTIONMENU)
	{
#ifdef MOTIF
		child = XmOptionButtonGadget(wptr->w);

		argcnt = 0;
		XtSetArg(arg[argcnt], XmNfontList, &font_list); argcnt++;
		XtGetValues(child, arg, argcnt);
#else
		XtVaGetValues(wptr->w, XtNfont, &font, NULL);
#endif /* MOTIF */
	}
	else
	{
#ifdef MOTIF
		if ((wptr->type == W_TEXTAREA)||(wptr->type == W_LIST))
		{
			child = NULL;
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNworkWindow, &child); argcnt++;
			XtGetValues(wptr->w, arg, argcnt);
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNfontList,&font_list); argcnt++;
			XtGetValues(child, arg, argcnt);
		}
#else
		if (wptr->type == W_LIST)
		{
			WidgetList wl;
			int nc;
			XtVaGetValues(wptr->w,
				XtNchildren, &wl, XtNnumChildren, &nc, NULL);
			child = *++wl;
			XtVaGetValues(child, XtNfont, &font, NULL);
                }
#endif /* MOTIF */
		else
		{
#ifdef MOTIF
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNfontList,&font_list); argcnt++;
			if (wptr->w == NULL)
			  return((XFontStruct *)NULL);
			XtGetValues(wptr->w, arg, argcnt);
#else
			XtVaGetValues(wptr->w, XtNfont, &font, NULL);
#endif /* MOTIF */
		}
	}

#ifdef MOTIF
	if (font_list == (XmFontList)NULL)
	{
		return((XFontStruct *)NULL);
	}

	ret = XmFontListInitFontContext(&font_context, font_list);
	if (ret == False)
	{
		return((XFontStruct *)NULL);
	}

	ret = XmFontListGetNextFont(font_context, &charset, &font);
	if (ret == False)
	{
		return((XFontStruct *)NULL);
	}
	else
	{
		XmFontListFreeFontContext(font_context);
		free((char *)charset);
		return(font);
	}
#else
	return(font);
#endif /* MOTIF */
}


/*
 * Get the next value in a comma separated list.
 * Also unescape the '\' escaping done in ComposeCommaList
 * and convert the single ''' characters back to '"'
 * characters
 */
char *
NextComma(string)
        char *string;
{
        char *tptr;

        tptr = string;
        while (*tptr != '\0')
        {
                if (*tptr == '\\')
                {
                        *tptr = '\0';
                        strcat(string, (char *)(tptr + 1));
                        tptr++;
                }
                else if (*tptr == '\'')
                {
                        *tptr = '\"';
                        tptr++;
                }
                else if (*tptr == ',')
                {
                        return(tptr);
                }
                else
                {
                        tptr++;
                }
        }
        return(tptr);
}


char **
ParseCommaList(str, count)
	char *str;
	int *count;
{
	char *str_copy;
	char **list;
	char **tlist;
	char *tptr;
	char *val;
	int i, cnt;
	int max_cnt;

	*count = 0;
	if ((str == NULL)||(*str == '\0'))
	{
		return((char **)NULL);
	}
	str_copy = (char *)malloc(strlen(str) + 1);
	if (str_copy == NULL)
	{
		return((char **)NULL);
	}
	strcpy(str_copy, str);

	list = (char **)malloc(50 * sizeof(char *));
	if (list == NULL)
	{
		return((char **)NULL);
	}
	max_cnt = 50;

	/*
	 * This loop counts the number of objects
	 * in this list.
	 * As a side effect, NextComma() unescapes in place so
	 * "\\" becomes '\' and "\," becomes ',' and "\"" becomes '"'
	 */
	cnt = 0;
	val = str_copy;
	tptr = NextComma(val);
	while (*tptr != '\0')
	{
		if ((cnt + 1) == max_cnt)
		{
			tlist = (char **)malloc((max_cnt +50) * sizeof(char *));
			if (tlist == NULL)
			{
				return((char **)NULL);
			}
			for (i=0; i<cnt; i++)
			{
				tlist[i] = list[i];
			}
			free((char *)list);
			list = tlist;
			max_cnt += 50;
		}
		*tptr = '\0';
		list[cnt] = (char *)malloc(strlen(val) + 1);
		if (list[cnt] == NULL)
		{
			return((char **)NULL);
		}
		strcpy(list[cnt], val);
		cnt++;

		val = (char *)(tptr + 1);
		tptr = NextComma(val);
	}
	list[cnt] = (char *)malloc(strlen(val) + 1);
	if (list[cnt] == NULL)
	{
		return((char **)NULL);
	}
	strcpy(list[cnt], val);
	cnt++;

	free(str_copy);
	tlist = (char **)malloc(cnt * sizeof(char *));
	if (tlist == NULL)
	{
		return((char **)NULL);
	}
	for (i=0; i<cnt; i++)
	{
		tlist[i] = list[i];
	}
	free((char *)list);
	list = tlist;

	*count = cnt;
	return(list);
}


/*
 * Compose a single string comma separated list from
 * an array of strings.  Any '\', or ',' in the
 * list are escaped with a prepending '\'.
 * So they become '\\' and '\,'
 * Also we want to allow '"' characters in the list, but
 * they would get eaten by the later parsing code, so we will
 * turn '"' into ''', and turn ''' into '\''
 */
char *
ComposeCommaList(list, cnt)
	char **list;
	int cnt;
{
	int i;
	char *fail;
	char *buf;
	char *tbuf;
	int len, max_len;

	fail = (char *)malloc(1);
	*fail = '\0';

	if (cnt == 0)
	{
		return(fail);
	}

	buf = (char *)malloc(1024);
	if (buf == NULL)
	{
		return(fail);
	}
	max_len = 1024;
	len = 0;
	buf[0] = '\0';

	for (i=0; i<cnt; i++)
	{
		char *option;
		char *tptr;
		int olen;

		option = list[i];
		if (option == NULL)
		{
			olen = 0;
		}
		else
		{
			olen = strlen(option);
		}
		if ((len + (olen * 2)) >= (max_len-2)) /* amb 12/24/94 */
		{
			tbuf = (char *)malloc(max_len + olen + 1024);
			if (tbuf == NULL)
			{
				return(fail);
			}
			strcpy(tbuf, buf);
			free(buf);
			buf = tbuf;
			max_len = max_len + olen + 1024;
		}
		tptr = (char *)(buf + len);
		while ((option != NULL)&&(*option != '\0'))
		{
			if ((*option == '\\')||(*option == ',')||
				(*option == '\''))
			{
				*tptr++ = '\\';
				*tptr++ = *option++;
				len += 2;
			}
			else if (*option == '\"')
			{
				*tptr++ = '\'';
				option++;
				len++;
			}
			else
			{
				*tptr++ = *option++;
				len++;
			}
		}
		if (i != (cnt - 1))
		{
			*tptr++ = ',';
			len++;
		}
		*tptr = '\0';
	}

	tbuf = (char *)malloc(len + 1);
	if (tbuf == NULL)
	{
		return(fail);
	}
	strcpy(tbuf, buf);
	free(buf);
	buf = tbuf;
	free(fail);
	return(buf);
}


void
FreeCommaList(list, cnt)
	char **list;
	int cnt;
{
	int i;

	for (i=0; i<cnt; i++)
	{
		if (list[i] != NULL)
		{
			free(list[i]);
		}
	}
	if (list != NULL)
	{
		free((char *)list);
	}
}


/*
 * Clean up the mucked value field for a TEXTAREA.
 * Unescape the things with '\' in front of them, and transform
 * lone ' back to "
 */
void
UnMuckTextAreaValue(value)
	char *value;
{
	char *tptr;

	if ((value == NULL)||(value[0] == '\0'))
	{
		return;
	}

	tptr = value;
        while (*tptr != '\0')
        {
                if (*tptr == '\\')
                {
                        *tptr = '\0';
                        strcat(value, (char *)(tptr + 1));
                        tptr++;
                }
                else if (*tptr == '\'')
                {
                        *tptr = '\"';
                        tptr++;
                }
                else
                {
                        tptr++;
                }
        }
}


char *
MapOptionReturn(val, mapping)
	char *val;
	char **mapping;
{
	int cnt;

	if (mapping == NULL)
	{
		return(val);
	}

	cnt = 0;
	while (mapping[cnt] != NULL)
	{
		if (strcmp(mapping[cnt], val) == 0)
		{
			return(mapping[cnt + 1]);
		}
		cnt += 2;
	}
	return(val);
}


char **
MakeOptionMappings(list1, list2, list_cnt)
	char **list1;
	char **list2;
	int list_cnt;
{
	int i, cnt;
	char **list;

	/*
	 * pass through to see how many mappings we have.
	 */
	cnt = 0;
	for (i=0; i<list_cnt; i++)
	{
		if ((list2[i] != NULL)&&(*list2[i] != '\0'))
		{
			cnt++;
		}
	}

	if (cnt == 0)
	{
		return(NULL);
	}

	list = (char **)malloc(((2 * cnt) + 1) * sizeof(char *));
	if (list == NULL)
	{
		return(NULL);
	}

	cnt = 0;
	for (i=0; i<list_cnt; i++)
	{
		if ((list2[i] != NULL)&&(*list2[i] != '\0'))
		{
			list[cnt] = (char *)malloc(strlen(list1[i]) + 1);
			list[cnt + 1] = (char *)malloc(strlen(list2[i]) + 1);
			if ((list[cnt] == NULL)||(list[cnt + 1] == NULL))
			{
				return(NULL);
			}
			strcpy(list[cnt], list1[i]);
			strcpy(list[cnt + 1], list2[i]);
			cnt += 2;
		}
	}
	list[cnt] = NULL;

	return(list);
}


#ifdef MOTIF
/********** MOTIF VERSION *************/
/*
 * Make the appropriate widget for this tag, and fill in an
 * WidgetInfo structure and return it.
 */
WidgetInfo *
MakeWidget(hw, text, x, y, id, fptr)
	HTMLWidget hw;
	char *text;
	int x, y;
	int id;
	FormInfo *fptr;
{
	Arg arg[30];
	Cardinal argcnt;
	Widget w;
	WidgetInfo *wlist;
	WidgetInfo *wptr;
	Dimension width, height;


	wlist = hw->html.widget_list;
	while (wlist != NULL)
	{
		if (wlist->id == id)
		{
			break;
		}
		wlist = wlist->next;
	}

	/*
	 * If this widget is not on the list, we have never
	 * used it before.  Create it now.
	 */
	if (wlist == NULL)
	{
		char widget_name[100];
		char **mapping;
		char *tptr;
		char *value;
		char *name;
		char *type_str;
		int type;
		short size;
		int maxlength;
		Boolean checked;

		mapping = NULL;

		checked = False;
		name = ParseMarkTag(text, MT_INPUT, "NAME");

		/*
		 * We may need to shorten the name for the widgets,
		 * which can't handle long names.
		 */
		if (name == NULL)
		{
			widget_name[0] = '\0';
		}
		else if (strlen(name) > 99)
		{
			strncpy(widget_name, name, 99);
			widget_name[99] = '\0';
		}
		else
		{
			strcpy(widget_name, name);
		}

		type_str = ParseMarkTag(text, MT_INPUT, "TYPE");
		if ((type_str != NULL)&&(my_strcasecmp(type_str, "checkbox") == 0))
		{
			XmString label;

			type = W_CHECKBOX;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if (value == NULL)
			{
				value = (char *)malloc(strlen("on") + 1);
				strcpy(value, "on");
			}

			tptr = ParseMarkTag(text, MT_INPUT, "CHECKED");

			/* We want no text on our toggles */
			label = XmStringCreateSimple("");

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNlabelString, label); argcnt++;
			XtSetArg(arg[argcnt], XmNx, x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, y); argcnt++;
			/*XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
			argcnt++;*/
			if (tptr != NULL)
			{
				XtSetArg(arg[argcnt], XmNset, True); argcnt++;
				checked = True;
				free(tptr);
			}
			w = XmCreateToggleButton(hw->html.view, widget_name,
				arg, argcnt);

			if(!hw->html.focus_follows_mouse)
			  {
			    XtOverrideTranslations(w, 
				    XtParseTranslationTable(traversal_table));
			    XtOverrideTranslations(w, 
				    XtParseTranslationTable("<Btn1Down>: Arm() traversal_current()"));
			  }

			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);

			XmStringFree(label);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "hidden") == 0))
		{
			type = W_HIDDEN;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if (value == NULL)
			{
				value = (char *)malloc(1);
				value[0] = '\0';
			}

			w = NULL;
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "radio") == 0))
		{
			XmString label;

			type = W_RADIOBOX;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if (value == NULL)
			{
				value = (char *)malloc(strlen("on") + 1);
				strcpy(value, "on");
			}

			/*
			 * Only one checked radio button with the
			 * same name per form
			 */
			tptr = ParseMarkTag(text, MT_INPUT, "CHECKED");
			if ((tptr != NULL)&&
				(AlreadyChecked(hw, fptr, name) == True))
			{
				free(tptr);
				tptr = NULL;
			}

			/* We want no text on our toggles */
			label = XmStringCreateSimple("");

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNlabelString, label); argcnt++;
			XtSetArg(arg[argcnt], XmNx, x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, y); argcnt++;
			XtSetArg(arg[argcnt], XmNindicatorType, XmONE_OF_MANY);
			argcnt++;
			/*XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
			argcnt++;*/

			if (tptr != NULL)
			{
				XtSetArg(arg[argcnt], XmNset, True); argcnt++;
				checked = True;
				free(tptr);
			}
			w = XmCreateToggleButton(hw->html.view, widget_name,
				arg, argcnt);
			if(!hw->html.focus_follows_mouse)
			  XtOverrideTranslations(w, 
				  XtParseTranslationTable(traversal_table));

			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			XtAddCallback(w, XmNvalueChangedCallback,
				(XtCallbackProc)CBChangeRadio, (caddr_t)fptr);

			XmStringFree(label);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "submit") == 0))
		{
			XmString label;

			type = W_PUSHBUTTON;
			label = NULL;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if ((value == NULL)||(*value == '\0'))
			{
				value = (char *)malloc(strlen("Submit Query") +
					1);
				strcpy(value, "Submit Query");
			}

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, y); argcnt++;
			/*XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
			argcnt++;*/
			if (value != NULL)
			{
				label = XmStringCreateSimple(value);
				XtSetArg(arg[argcnt], XmNlabelString, label);
				argcnt++;
			}
			w = XmCreatePushButton(hw->html.view, widget_name,
				arg, argcnt);
			if(!hw->html.focus_follows_mouse)
			  XtOverrideTranslations(w, 
				  XtParseTranslationTable(traversal_table));
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			if (label != NULL)
			{
				XmStringFree(label);
			}
                        PrepareFormEnd(hw, w, fptr);

		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "reset") == 0))
		{
			XmString label;

			type = W_PUSHBUTTON;
			label = NULL;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if ((value == NULL)||(*value == '\0'))
			{
				value = (char *)malloc(strlen("Reset") + 1);
				strcpy(value, "Reset");
			}

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, y); argcnt++;
			/*XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
			argcnt++;*/
			if (value != NULL)
			{
				label = XmStringCreateSimple(value);
				XtSetArg(arg[argcnt], XmNlabelString, label);
				argcnt++;
			}
			w = XmCreatePushButton(hw->html.view, widget_name,
				arg, argcnt);
			if(!hw->html.focus_follows_mouse)
			  XtOverrideTranslations(w, 
				  XtParseTranslationTable(traversal_table));
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			if (label != NULL)
			{
				XmStringFree(label);
			}
			PrepareFormReset(hw, w, fptr);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "button") == 0))
		{
			XmString label;

			type = W_PUSHBUTTON;
			label = NULL;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, y); argcnt++;
			/*XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
			argcnt++; */
			if (value != NULL)
			{
				label = XmStringCreateSimple(value);
				XtSetArg(arg[argcnt], XmNlabelString, label);
				argcnt++;
			}
			w = XmCreatePushButton(hw->html.view, widget_name,
				arg, argcnt);
			if(!hw->html.focus_follows_mouse)
			  XtOverrideTranslations(w, 
				  XtParseTranslationTable(traversal_table));
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			if (label != NULL)
			{
				XmStringFree(label);
			}
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "jot") == 0))
		{
			XmString label;
			Dimension width, height;
			Widget frame;
			char **list;
			int list_cnt;

			type = W_JOT;
			label = NULL;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");

			/*
			 * SIZE is WIDTH,HEIGHT
			 */
			tptr = ParseMarkTag(text, MT_INPUT, "SIZE");
			list = ParseCommaList(tptr, &list_cnt);
			if (tptr != NULL)
			{
				free(tptr);
			}

			width = 200;
			height = 50;
			if (list_cnt == 1)
			{
				width = atoi(list[0]);
			}
			else if (list_cnt > 1)
			{
				width = atoi(list[0]);
				height = atoi(list[1]);
			}
			FreeCommaList(list, list_cnt);

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, y); argcnt++;
			XtSetArg(arg[argcnt], XmNshadowType, XmSHADOW_IN);
			argcnt++;
			frame = XmCreateFrame(hw->html.view, "Frame",
				arg, argcnt);

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNwidth, width); argcnt++;
			XtSetArg(arg[argcnt], XmNheight, height); argcnt++;
			w = XmCreateDrawingArea(frame, widget_name,
				arg, argcnt);
			XtManageChild(w);

			NewJot(w, width, height);
			XtAddEventHandler(w, ExposureMask, 0,
				EVJotExpose, (XtPointer)hw);
			XtAddEventHandler(w, ButtonPressMask, 0,
				EVJotPress, (XtPointer)hw);
			XtAddEventHandler(w, ButtonMotionMask, 0,
				EVJotMove, (XtPointer)hw);
			XtAddEventHandler(w, ButtonReleaseMask, 0,
				EVJotRelease, (XtPointer)hw);

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNuserData, (XtPointer)w);
			argcnt++;
			XtSetValues(frame, arg, argcnt);

			w = frame;

			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			if (label != NULL)
			{
				XmStringFree(label);
			}
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "select") == 0))
		{
			XmString label;
			Widget scroll;
			Widget pulldown, button, hist;
			char *options;
			char *returns;
			char **list;
			int list_cnt;
			char **ret_list;
			int return_cnt;
			char **vlist;
			int vlist_cnt;
			int i, mult, size;

			type = -1;
			tptr = ParseMarkTag(text, MT_INPUT, "HINT");
			if ((tptr != NULL)&&(my_strcasecmp(tptr, "list") == 0))
			{
				type = W_LIST;
			}
			else if ((tptr != NULL)&&(my_strcasecmp(tptr, "menu") == 0))
			{
				type = W_OPTIONMENU;
			}
			if (tptr != NULL)
			{
				free(tptr);
			}

			size = 5;
			tptr = ParseMarkTag(text, MT_INPUT, "SIZE");
			if (tptr != NULL)
			{
				size = atoi(tptr);
				if ((size > 1)&&(type == -1))
				{
					type = W_LIST;
				}
				free(tptr);
			}

			mult = 0;
			tptr = ParseMarkTag(text, MT_INPUT, "MULTIPLE");
			if (tptr != NULL)
			{
				if (type == -1)
				{
					type = W_LIST;
				}
				mult = 1;
				free(tptr);
			}

			if (type == -1)
			{
				type = W_OPTIONMENU;
			}

			label = NULL;
			hist = NULL;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			options = ParseMarkTag(text, MT_INPUT, "OPTIONS");
			returns = ParseMarkTag(text, MT_INPUT, "RETURNS");
			list = ParseCommaList(options, &list_cnt);
			if (options != NULL)
			{
				free(options);
			}

			ret_list = ParseCommaList(returns, &return_cnt);
			if (returns != NULL)
			{
				free(returns);
			}

			/*
			 * If return_cnt is less than list_cnt, the user made
			 * a serious error.  Try to recover by padding out
			 * ret_list with NULLs
			 */
			if (list_cnt > return_cnt)
			{
				int rcnt;
				char **rlist;

				rlist = (char **)malloc(list_cnt *
					sizeof(char *));
				for (rcnt = 0; rcnt < return_cnt; rcnt++)
				{
					rlist[rcnt] = ret_list[rcnt];
				}
				for (rcnt = return_cnt; rcnt < list_cnt; rcnt++)
				{
					rlist[rcnt] = NULL;
				}
				if (ret_list != NULL)
				{
					free((char *)ret_list);
				}
				ret_list = rlist;
			}

			vlist = ParseCommaList(value, &vlist_cnt);

			if (size > list_cnt)
			{
				size = list_cnt;
			}
			if (size < 1)
			{
				size = 1;
			}

			mapping = MakeOptionMappings(list, ret_list, list_cnt);

			if (type == W_OPTIONMENU)
			{
                                Widget child;
                                XmString xmstr;
				argcnt = 0;
				pulldown = XmCreatePulldownMenu(
					(Widget) hw->html.view,
					widget_name, arg, argcnt);

				for (i=0; i<list_cnt; i++)
				{
					char bname[30];

					sprintf(bname, "Button%d", (i + 1));
					label = XmStringCreateSimple(list[i]);
					argcnt = 0;
					XtSetArg(arg[argcnt], XmNlabelString,
						label);
					argcnt++;
					button = XmCreatePushButton(pulldown,
						bname, arg, argcnt);
					XtManageChild(button);
					XmStringFree(label);
					if(!hw->html.focus_follows_mouse)
					  XtOverrideTranslations(button, 
				            XtParseTranslationTable(traversal_table));
					if ((vlist_cnt > 0)&&
						(vlist[0] != NULL)&&
						(strcmp(vlist[0], list[i]) ==0))
					{
						hist = button;
					}

					/*
					 * Start hist out as the first button
					 * so that if the user didn't set a
					 * default we always default to the
					 * first element.
					 */
					if ((i == 0)&&(hist == NULL))
					{
      						hist = button;
					}
				}

				FreeCommaList(list, list_cnt);
				FreeCommaList(ret_list, list_cnt);
				FreeCommaList(vlist, vlist_cnt);
				if (value != NULL)
				{
					free(value);
				}

				argcnt = 0;
				XtSetArg(arg[argcnt], XmNx, x); argcnt++;
				XtSetArg(arg[argcnt], XmNy, y); argcnt++;
				/* kill margins */
				XtSetArg(arg[argcnt], XmNmarginWidth, 0); 
				argcnt++;
				XtSetArg(arg[argcnt], XmNmarginHeight, 0); 
				argcnt++;
				XtSetArg(arg[argcnt], XmNsubMenuId, pulldown);
					argcnt++;
					/*XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
				argcnt++;*/
				if (hist != NULL)
				{
					XtSetArg(arg[argcnt], XmNmenuHistory,
						hist);
					argcnt++;
					/*
					 * A gaggage.  Value is used to later
					 * restore defaults.  For option menu
					 * this means we need to save a child
					 * widget id as opposed to the
					 * character string everyone else uses.
					 */
					value = (char *)hist;
				}
				w = XmCreateOptionMenu((Widget) hw->html.view,
					widget_name, arg, argcnt);
				if(!hw->html.focus_follows_mouse)
				  {
				    XtOverrideTranslations(w, 
				     XtParseTranslationTable(traversal_table));
				    XtOverrideTranslations(pulldown, 
				     XtParseTranslationTable(traversal_table));
				  }
				
                                argcnt = 0;

				xmstr = XmStringCreateSimple ("");
                                XtSetArg(arg[argcnt], XmNlabelString,
                                         (XtArgVal)xmstr);
                                argcnt++;
				XtSetArg(arg[argcnt], XmNwidth, 0);
                                argcnt++;

                                child = XmOptionLabelGadget (w);

                                XtSetValues (child, arg, argcnt);
                                XmStringFree (xmstr);

/* back to original because of coredump on initial load of forms page SWP*/
				/* we unmange first to avoid a flicker
				   caused by Destroy taking so long */
/*
				XtUnmanageChild(child);
				XtDestroyWidget(child);
*/
                        }
			else /* type == W_LIST */
			{
				XmString *string_list;
				XmString *val_list;

				if ((!mult)&&(vlist_cnt > 1))
				{
					free(value);
					value = (char *)malloc(
						strlen(vlist[0]) + 1);
					strcpy(value, vlist[0]);
				}

				string_list = (XmString *)malloc(list_cnt *
					sizeof(XmString));
				val_list = (XmString *)malloc(vlist_cnt *
					sizeof(XmString));

				for (i=0; i<list_cnt; i++)
				{
					string_list[i] =
						XmStringCreateSimple(list[i]);
				}
				for (i=0; i<vlist_cnt; i++)
				{
					val_list[i] =
						XmStringCreateSimple(vlist[i]);
				}

				FreeCommaList(list, list_cnt);
				FreeCommaList(ret_list, list_cnt);
				FreeCommaList(vlist, vlist_cnt);

				argcnt = 0;
				XtSetArg(arg[argcnt], XmNx, x); argcnt++;
				XtSetArg(arg[argcnt], XmNy, y); argcnt++;
				scroll = XmCreateScrolledWindow((Widget) hw->html.view,
					"Scroll", arg, argcnt);

				argcnt = 0;
				XtSetArg(arg[argcnt], XmNitems, string_list);
				argcnt++;
				XtSetArg(arg[argcnt], XmNitemCount, list_cnt);
				argcnt++;
				XtSetArg(arg[argcnt], XmNvisibleItemCount,size);
				argcnt++;
				if (mult)
				{
					XtSetArg(arg[argcnt],XmNselectionPolicy,
						XmEXTENDED_SELECT);
					argcnt++;
				}
				else
				{
					XtSetArg(arg[argcnt],XmNselectionPolicy,
						XmBROWSE_SELECT);
					argcnt++;
				}
				if ((vlist_cnt > 0)&&(mult))
				{
					XtSetArg(arg[argcnt], XmNselectedItems,
						val_list);
					argcnt++;
					XtSetArg(arg[argcnt],
						XmNselectedItemCount,
						vlist_cnt);
					argcnt++;
				}
				else if ((vlist_cnt > 0)&&(!mult))
				{
					XtSetArg(arg[argcnt], XmNselectedItems,
						&val_list[0]);
					argcnt++;
					XtSetArg(arg[argcnt],
						XmNselectedItemCount, 1);
					argcnt++;
				}
				w = XmCreateList(scroll, widget_name,
					arg, argcnt);
				if(!hw->html.focus_follows_mouse)
				  XtOverrideTranslations(w, 
				     XtParseTranslationTable(traversal_table));
				XtManageChild(w);
				
				w = scroll;

				for (i=0; i<list_cnt; i++)
				{
					XmStringFree(string_list[i]);
				}
				if (string_list != NULL)
				{
					free((char *)string_list);
				}
				for (i=0; i<vlist_cnt; i++)
				{
					XmStringFree(val_list[i]);
				}
				if (val_list != NULL)
				{
					free((char *)val_list);
				}
			}
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "password") ==0))
		{
			type = W_PASSWORD;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");

			size = -1;
			maxlength = -1;

			tptr = ParseMarkTag(text, MT_INPUT, "SIZE");
			if (tptr != NULL)
			{
				size = atoi(tptr);
				free(tptr);
			}

			tptr = ParseMarkTag(text, MT_INPUT, "MAXLENGTH");
			if (tptr != NULL)
			{
				maxlength = atoi(tptr);
				free(tptr);
			}

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, y); argcnt++;
			if (size > 0)
			{
				XtSetArg(arg[argcnt], XmNcolumns, size);
				argcnt++;
			}
			if (maxlength > 0)
			{
				XtSetArg(arg[argcnt], XmNmaxLength, maxlength);
				argcnt++;
			}
			if (value != NULL)
			{
				int i, len;
				char *bval;

				len = strlen(value);
				bval = (char *)malloc(len + 1);
				for (i=0; i<len; i++)
				{
					bval[i] = '*';
				}
				bval[len] = '\0';
				XtSetArg(arg[argcnt], XmNvalue, bval);
				argcnt++;
			}
			/*XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
			argcnt++;*/
			w = XmCreateTextField(hw->html.view, widget_name,
				arg, argcnt);
			XtOverrideTranslations(w, XtParseTranslationTable(text_translations));
									 

			if(!hw->html.focus_follows_mouse)
			  XtOverrideTranslations(w, 
				  XtParseTranslationTable(traversal_table));
/*
 * The proper order here is XtSetMappedWhenManaged, XtManageChild.  But a bug
 * in some versions of Motif1.1 makes us do it the other way.  All versions
 * of 1.2 should have this fixed
 */
#ifdef MOTIF1_2
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
#else
			XtManageChild(w);
			XtSetMappedWhenManaged(w, False);
#endif /* MOTIF1_2 */
			XtAddCallback(w, XmNactivateCallback,
				(XtCallbackProc)CBActivateField, (caddr_t)fptr);
			XtAddCallback(w, XmNmodifyVerifyCallback,
				(XtCallbackProc)CBPasswordModify, (caddr_t)fptr);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "textarea") ==0))
		{
			char **list;
			int list_cnt;
			int rows, cols;
			Widget scroll;

			type = W_TEXTAREA;

			/*
			 * If there is no SIZE, look for ROWS and COLS
			 * directly.
			 * SIZE is COLUMNS,ROWS parse the list
			 */
			rows = -1;
			cols = -1;
			tptr = ParseMarkTag(text, MT_INPUT, "SIZE");
			if (tptr == NULL)
			{
				tptr = ParseMarkTag(text, MT_INPUT, "ROWS");
				if (tptr != NULL)
				{
					rows = atoi(tptr);
					free(tptr);
				}
				tptr = ParseMarkTag(text, MT_INPUT, "COLS");
				if (tptr != NULL)
				{
					cols = atoi(tptr);
					free(tptr);
				}
			}
			else
			{
				list = ParseCommaList(tptr, &list_cnt);
				free(tptr);

				if (list_cnt == 1)
				{
					cols = atoi(list[0]);
				}
				else if (list_cnt > 1)
				{
					cols = atoi(list[0]);
					rows = atoi(list[1]);
				}
				FreeCommaList(list, list_cnt);
			}

			/*
			 * Grab the starting value of the text here.
			 * NULL if none.
			 */
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			UnMuckTextAreaValue(value);

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, y); argcnt++;
			scroll = XmCreateScrolledWindow((Widget) hw->html.view,
				"Scroll", arg, argcnt);

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNeditMode, XmMULTI_LINE_EDIT);
			argcnt++;
			if (cols > 0)
			{
				XtSetArg(arg[argcnt], XmNcolumns, cols);
				argcnt++;
			}
			if (rows > 0)
			{
				XtSetArg(arg[argcnt], XmNrows, rows);
				argcnt++;
			}
			if (value != NULL)
			{
				XtSetArg(arg[argcnt], XmNvalue, value);
				argcnt++;
			}
			/*XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
			argcnt++;*/
			w = XmCreateText(scroll, widget_name, arg, argcnt);
			XtManageChild(w);
			XtOverrideTranslations(w, XtParseTranslationTable(text_translations));

			if(!hw->html.focus_follows_mouse)
			  XtOverrideTranslations(w, 
		       	     XtParseTranslationTable(traversal_table));
			

			w = scroll;

			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
		}
		else /* if no type, assume type=text */
		{
			char **list;
			int list_cnt;
			int rows, cols;
			Widget scroll;

			/*
			 * SIZE can be either COLUMNS or COLUMNS,ROWS
			 * we assume COLUMNS,ROWS and parse the list
			 */
			tptr = ParseMarkTag(text, MT_INPUT, "SIZE");
			list = ParseCommaList(tptr, &list_cnt);
			if (tptr != NULL)
			{
				free(tptr);
			}

			/*
			 * If only COLUMNS specified, or SIZE not specified
			 * assume a TEXTFIELD
			 * Otherwise a TEXTAREA.
			 */
			if (list_cnt <= 1)
			{
				type = W_TEXTFIELD;
				if (list_cnt == 1)
				{
					cols = atoi(list[0]);
				}
				else
				{
					cols = -1;
				}
			}
			else
			{
				type = W_TEXTAREA;
				cols = atoi(list[0]);
				rows = atoi(list[1]);
				
				/* be a textfield if only one row */
				if(rows==1)
				  type=W_TEXTFIELD;
			}
			/*
			 * Now that we have cols, and maybe rows, free the list
			 */
			FreeCommaList(list, list_cnt);

			/*
			 * Grab the starting value of the text here.
			 * NULL if none.
			 */
			value = ParseMarkTag(text, MT_INPUT, "VALUE");

			/*
			 * For textfileds parse maxlength and
			 * set up the widget.
			 */
			if (type == W_TEXTFIELD)
			{
				maxlength = -1;
				tptr = ParseMarkTag(text, MT_INPUT,"MAXLENGTH");
				if (tptr != NULL)
				{
					maxlength = atoi(tptr);
					free(tptr);
				}

				argcnt = 0;
				XtSetArg(arg[argcnt], XmNx, x); argcnt++;
				XtSetArg(arg[argcnt], XmNy, y); argcnt++;
				if (cols > 0)
				{
					XtSetArg(arg[argcnt], XmNcolumns, cols);
					argcnt++;
				}
				if (maxlength > 0)
				{
					XtSetArg(arg[argcnt], XmNmaxLength,
						maxlength);
					argcnt++;
				}
				if (value != NULL)
				{
					XtSetArg(arg[argcnt], XmNvalue, value);
					argcnt++;
				}
				/*XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
				argcnt++;*/

				w = XmCreateTextField((Widget) hw->html.view,
					widget_name, arg, argcnt);
				XtOverrideTranslations(w, XtParseTranslationTable(text_translations));
				if(!hw->html.focus_follows_mouse)
				  XtOverrideTranslations(w, 
				     XtParseTranslationTable(traversal_table));
			}
			/*
			 * Else this is a TEXTAREA.  Maxlength is ignored,
			 * and we set up the scrolled window
			 */
			else
			{
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNx, x); argcnt++;
				XtSetArg(arg[argcnt], XmNy, y); argcnt++;
				scroll = XmCreateScrolledWindow(hw->html.view,
					"Scroll", arg, argcnt);

				argcnt = 0;
				XtSetArg(arg[argcnt], XmNeditMode,
					XmMULTI_LINE_EDIT);
				argcnt++;
				if (cols > 0)
				{
					XtSetArg(arg[argcnt], XmNcolumns, cols);
					argcnt++;
				}
				if (rows > 0)
				{
					XtSetArg(arg[argcnt], XmNrows, rows);
					argcnt++;
				}
				if (value != NULL)
				{
					XtSetArg(arg[argcnt], XmNvalue, value);
					argcnt++;
				}
				XtSetArg(arg[argcnt], XmNnavigationType, XmNONE);
				argcnt++;
				w = XmCreateText(scroll, widget_name,
					arg, argcnt);
				XtManageChild(w);
				XtOverrideTranslations(w, XtParseTranslationTable(text_translations));
				if(!hw->html.focus_follows_mouse)
				  XtOverrideTranslations(w, 
				     XtParseTranslationTable(traversal_table));


				w = scroll;
			}

/*
 * The proper order here is XtSetMappedWhenManaged, XtManageChild.  But a bug
 * in some versions of Motif1.1 makes us do it the other way.  All versions
 * of 1.2 should have this fixed
 */
#ifdef MOTIF1_2
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
#else
			XtManageChild(w);
			XtSetMappedWhenManaged(w, False);
#endif /* MOTIF1_2 */

			/*
			 * For textfields, a CR might be an activate
			 */
			if (type == W_TEXTFIELD)
			{
				XtAddCallback(w, XmNactivateCallback,
					(XtCallbackProc)CBActivateField, (caddr_t)fptr);
			}
		}

		if (type_str != NULL)
		{
			free(type_str);
		}

		/*
		 * Don't want to do GetValues if this is HIDDEN input
		 * tag with no widget.
		 */
		if (w != NULL)
		{
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNwidth, &width); argcnt++;
			XtSetArg(arg[argcnt], XmNheight, &height); argcnt++;
			XtGetValues(w, arg, argcnt);
			/* Set it to default so we don't lose it on "back"*/
                        XtVaSetValues(w,
                          XmNbackground, hw->html.background_SAVE,
                          XmNtopShadowColor, hw->html.top_color_SAVE,
                          XmNbottomShadowColor, hw->html.bottom_color_SAVE,
                          NULL);
		}
		else
		{
			width = 0;
			height = 0;
		}

		wptr = AddNewWidget(hw, fptr, w, type, id, x, y, width, 
			height, name, value, mapping, checked);
	}
	else
	/*
	 * We found this widget on the list of already created widgets.
	 * Put it in place for reuse.
	 */
	{
		wlist->x = x;
		wlist->y = y;

		/*
		 * Don't want to SetValues if type HIDDEN which
		 * has no widget.
		 */
		if (wlist->w != NULL)
		{
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, y); argcnt++;
			XtSetValues(wlist->w, arg, argcnt);

			/* Set it to default so we don't lose it on "back"*/
                        XtVaSetValues(wlist->w,
                          XmNbackground, hw->html.background_SAVE,
                          XmNtopShadowColor, hw->html.top_color_SAVE,
                          XmNbottomShadowColor, hw->html.bottom_color_SAVE,
                          NULL);
/*
                        XtVaSetValues(wlist->w,
                          XmNbackground, hw->core.background_pixel,
                          XmNtopShadowColor, hw->manager.top_shadow_color,
                          XmNbottomShadowColor, hw->manager.bottom_shadow_color,
                          NULL);
*/
		}

		wptr = wlist;
	}

	return(wptr);
}
#else
/********** ATHENA VERSION *************/
/*
 * Make the appropriate widget for this tag, and fill in an
 * WidgetInfo structure and return it.
 */
WidgetInfo *
MakeWidget(hw, text, x, y, id, fptr)
	HTMLWidget hw;
	char *text;
	int x, y;
	int id;
	FormInfo *fptr;
{
	Arg arg[30];
	Cardinal argcnt;
	Widget w;
	WidgetInfo *wlist;
	WidgetInfo *wptr;
	Dimension width, height;

        
	wlist = hw->html.widget_list;
	while (wlist != NULL)
	{
		if (wlist->id == id)
		{
			break;
		}
		wlist = wlist->next;
	}

	/*
	 * If this widget is not on the list, we have never
	 * used it before.  Create it now.
	 */
	if (wlist == NULL)
	{
		char *tptr;
		char *value;
		char *name;
		char *type_str;
		int type;
		short size;
		int maxlength;
		Boolean checked;

		checked = False;
		name = ParseMarkTag(text, MT_INPUT, "NAME");

		type_str = ParseMarkTag(text, MT_INPUT, "TYPE");
		if ((type_str != NULL)&&(my_strcasecmp(type_str, "checkbox") == 0))
		{
			type = W_CHECKBOX;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if (value == NULL)
			{
				value = (char *)malloc(strlen("on") + 1);
				strcpy(value, "on");
			}

			tptr = ParseMarkTag(text, MT_INPUT, "CHECKED");

			argcnt = 0;
			XtSetArg(arg[argcnt], XtNx, x); argcnt++;
			XtSetArg(arg[argcnt], XtNy, y); argcnt++;
			if (tptr != NULL)
			{
				XtSetArg(arg[argcnt], XtNstate, True); argcnt++;
				checked = True;
				free(tptr);
			}
			XtSetArg(arg[argcnt], XtNlabel, ""); argcnt++;
			w = XtCreateWidget(name, toggleWidgetClass,
				hw->html.view, arg, argcnt);

			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "hidden") == 0))
		{
			type = W_HIDDEN;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if (value == NULL)
			{
				value = (char *)malloc(1);
				value[0] = '\0';
			}

			w = NULL;
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "radio") == 0))
		{
			type = W_RADIOBOX;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if (value == NULL)
			{
				value = (char *)malloc(strlen("on") + 1);
				strcpy(value, "on");
			}

			/*
			 * Only one checked radio button with the
			 * same name per form
			 */
			tptr = ParseMarkTag(text, MT_INPUT, "CHECKED");
			if ((tptr != NULL)&&
				(AlreadyChecked(hw, fptr, name) == True))
			{
				free(tptr);
				tptr = NULL;
			}

			argcnt = 0;
			XtSetArg(arg[argcnt], XtNx, x); argcnt++;
			XtSetArg(arg[argcnt], XtNy, y); argcnt++;
			if (tptr != NULL)
			{
				XtSetArg(arg[argcnt], XtNstate, True); argcnt++;
				checked = True;
				free(tptr);
			}
			XtSetArg(arg[argcnt], XtNlabel, ""); argcnt++;
			w = XtCreateWidget(name, toggleWidgetClass,
				hw->html.view, arg, argcnt);
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			XtAddCallback(w, XtNcallback,
				(XtCallbackProc)CBChangeRadio, (caddr_t)fptr);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "submit") == 0))
		{
			type = W_PUSHBUTTON;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if ((value == NULL)||(*value == '\0'))
			{
				value = (char *)malloc(strlen("Submit Query") +
					1);
				strcpy(value, "Submit Query");
			}

			argcnt = 0;
			XtSetArg(arg[argcnt], XtNx, x); argcnt++;
			XtSetArg(arg[argcnt], XtNy, y); argcnt++;
			if (value != NULL)
			{
				XtSetArg(arg[argcnt], XtNlabel, value);
				argcnt++;
			}
			w = XtCreateWidget(name, commandWidgetClass,
				hw->html.view, arg, argcnt);
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			PrepareFormEnd(hw, w, fptr);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "reset") == 0))
		{
			type = W_PUSHBUTTON;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if ((value == NULL)||(*value == '\0'))
			{
				value = (char *)malloc(strlen("Reset") + 1);
				strcpy(value, "Reset");
			}

			argcnt = 0;
			XtSetArg(arg[argcnt], XtNx, x); argcnt++;
			XtSetArg(arg[argcnt], XtNy, y); argcnt++;
			if (value != NULL)
			{
				XtSetArg(arg[argcnt], XtNlabel, value);
				argcnt++;
			}
			w = XtCreateWidget(name, commandWidgetClass,
				hw->html.view, arg, argcnt);
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			PrepareFormReset(hw, w, fptr);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "button") == 0))
		{
			type = W_PUSHBUTTON;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");

			argcnt = 0;
			XtSetArg(arg[argcnt], XtNx, x); argcnt++;
			XtSetArg(arg[argcnt], XtNy, y); argcnt++;
			if (value != NULL)
			{
				XtSetArg(arg[argcnt], XtNlabel, value);
				argcnt++;
			}
			w = XtCreateWidget(name, commandWidgetClass,
				hw->html.view, arg, argcnt);
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "select") == 0))
		{
			STRING label;
			Widget scroll;
			Widget pulldown, button, hist;
			char *options;
			char **list;
			int list_cnt;
			char **vlist;
			int vlist_cnt;
			int i, mult, size;

			type = -1;
			tptr = ParseMarkTag(text, MT_INPUT, "HINT");
			if ((tptr != NULL)&&(my_strcasecmp(tptr, "list") == 0))
			{
				type = W_LIST;
			}
			else if ((tptr != NULL)&&(my_strcasecmp(tptr, "menu") == 0))
			{
				type = W_OPTIONMENU;
			}
			if (tptr != NULL)
			{
				free(tptr);
			}

			size = 5;
			tptr = ParseMarkTag(text, MT_INPUT, "SIZE");
			if (tptr != NULL)
			{
				size = atoi(tptr);
				if ((size > 1)&&(type == -1))
				{
					type = W_LIST;
				}
				free(tptr);
			}

			mult = 0;
			tptr = ParseMarkTag(text, MT_INPUT, "MULTIPLE");
			if (tptr != NULL)
			{
				if (type == -1)
				{
					type = W_LIST;
				}
				mult = 1;
				free(tptr);
			}

			if (type == -1)
			{
				type = W_OPTIONMENU;
			}

			label = NULL;
			hist = NULL;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			options = ParseMarkTag(text, MT_INPUT, "OPTIONS");
			list = ParseCommaList(options, &list_cnt);
			if (options != NULL)
			{
				free(options);
			}

			vlist = ParseCommaList(value, &vlist_cnt);

			if (size > list_cnt)
			{
				size = list_cnt;
			}
			if (size < 1)
			{
				size = 1;
			}

			if (type == W_OPTIONMENU)
			{
				XFontStruct *font;
				Dimension maxWidth = 0, width, iW;

				argcnt = 0;
				XtSetArg(arg[argcnt], XtNx, x); argcnt++;
				XtSetArg(arg[argcnt], XtNy, y); argcnt++;
				w = XtCreateWidget(name,
					menuButtonWidgetClass,
					hw->html.view, arg, argcnt);
				argcnt = 0;
				pulldown = XtCreatePopupShell("menu",
					simpleMenuWidgetClass, w,
					arg, argcnt);
				for (i=0; i<list_cnt; i++)
				{
					char bname[30];

					sprintf(bname, "Button%d", (i + 1));
					argcnt = 0;
					XtSetArg(arg[argcnt], XtNlabel,
						list[i]);
					argcnt++;
					button = XtCreateWidget(bname,
						smeBSBObjectClass,
						pulldown, arg, argcnt);
					XtManageChild(button);
					XtAddCallback(button, XtNcallback,
						CBoption, (XtPointer)w);

					if (i==0)
					{
						XtVaGetValues(w,
							XtNfont, &font,
							XtNinternalWidth, &iW,
							NULL);
					}

					width = XTextWidth(font, list[i],
						strlen(list[i]));

					if (width > maxWidth) maxWidth = width;

					if ((vlist_cnt > 0)&&
						(vlist[0] != NULL)&&
						(strcmp(vlist[0], list[i]) ==0))
					{
						hist = button;
						XtVaSetValues(w,
							XtNlabel,
							XtNewString(list[i]),
							NULL);
					}

					/*
					 * Start hist out as the first button
					 * so that if the user didn't set a
					 * default we always default to the
					 * first element.
					 */
					if ((i == 0)&&(hist == NULL))
					{
						hist = button;
					}
				}

				XtVaSetValues(w, XtNwidth, maxWidth + (4 * iW),
					NULL);

				FreeCommaList(vlist, vlist_cnt);
				if (value != NULL)
				{
					free(value);
				}

				if (hist != NULL)
				{
					/*
					 * A gaggage.  Value is used to later
					 * restore defaults.  For option menu
					 * this means we need to save a child
					 * widget id as opposed to the
					 * character string everyone else uses.
					 */
					value = (char *)hist;
				}
                        }
			else /* type == W_LIST */
			{
				STRING *string_list;
				STRING *val_list;

				if ((!mult)&&(vlist_cnt > 1))
				{
					free(value);
					value = (char *)malloc(
						strlen(vlist[0]) + 1);
					strcpy(value, vlist[0]);
				}

				string_list = (STRING *)malloc(list_cnt *
					sizeof(STRING));
				val_list = (STRING *)malloc(vlist_cnt *
					sizeof(STRING));

				for (i=0; i<list_cnt; i++)
				{
					string_list[i] =
						XtNewString(list[i]);
				}
				for (i=0; i<vlist_cnt; i++)
				{
					val_list[i] =
						XtNewString(vlist[i]);
				}

				FreeCommaList(list, list_cnt);
				FreeCommaList(vlist, vlist_cnt);

				argcnt = 0;
				XtSetArg(arg[argcnt], XtNx, x); argcnt++;
				XtSetArg(arg[argcnt], XtNy, y); argcnt++;
				XtSetArg(arg[argcnt], XtNallowVert, True);
				argcnt++;
				scroll = XtCreateWidget("Scroll",
					viewportWidgetClass,
					hw->html.view, arg, argcnt);
				argcnt = 0;
				XtSetArg(arg[argcnt], XtNdefaultColumns, 1);
				argcnt++;
				w = XtCreateWidget(name,
					listWidgetClass,
					scroll, arg, argcnt);
				XtManageChild(w);
				XtAddCallback(w, XtNdestroyCallback,
					CBListDestroy, NULL);

				XawListChange(w, string_list, list_cnt,
					0, True);

				if (vlist_cnt > 0)
				{
				    if (vlist_cnt > 1)
				    {
#ifndef DISABLE_TRACE
					if (htmlwTrace) {
						fprintf(stderr,
							"HTML: only a single selection allowed!\n");
					}
#endif
				    }

				    for (i=0; i<list_cnt; i++)
				    {
					if (!strcmp(string_list[i],val_list[0]))
					{
					    XawListHighlight(w, i);
					    break;
					}
				    }
				}

				if (size>list_cnt) size=list_cnt;
				if (size>1)
				{
					XFontStruct *font;
					Dimension h,width, s;

					XtVaGetValues(w, XtNfont, &font,
						XtNinternalHeight, &h,
						XtNwidth, &width,
						XtNrowSpacing, &s,
						NULL);
					XtVaSetValues(scroll,
						XtNheight,
						h + size*(s+FONTHEIGHT(font)),
						XtNwidth, width + 20,
						NULL);
				}

				w = scroll;

				for (i=0; i<vlist_cnt; i++)
				{
					free(val_list[i]);
				}
				if (val_list != NULL)
				{
					free((char *)val_list);
				}
			}
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "password") ==0))
		{
			char *txt;

			type = W_PASSWORD;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");

			size = -1;
			maxlength = -1;

			tptr = ParseMarkTag(text, MT_INPUT, "SIZE");
			if (tptr != NULL)
			{
				size = atoi(tptr);
				free(tptr);
			}

			tptr = ParseMarkTag(text, MT_INPUT, "MAXLENGTH");
			if (tptr != NULL)
			{
				maxlength = atoi(tptr);
				free(tptr);
			}

			argcnt = 0;
			XtSetArg(arg[argcnt], XtNx, x); argcnt++;
			XtSetArg(arg[argcnt], XtNy, y); argcnt++;
			XtSetArg(arg[argcnt], XtNeditType, XawtextEdit);
			argcnt++;

			if (maxlength > 0)
			{
				if (value)
				{
					txt = XtNewString(value);
					txt = (char*)realloc(txt,
						sizeof(char)*(maxlength+1));
				}
				else
				{
					txt = (char *)malloc(sizeof(char)*
						(maxlength+1));
					*txt = '\0';
				}
				XtSetArg(arg[argcnt], XtNuseStringInPlace, 1);
				argcnt++;
				XtSetArg(arg[argcnt], XtNlength, maxlength);
				argcnt++;
			}
			else
			{
				XtSetArg(arg[argcnt], XtNuseStringInPlace, 0);
				argcnt++;
			}


			if (value != NULL)
			{
				int i, len;
				char *bval;

				len = strlen(value);
				if (maxlength > 0)
				{
					bval = txt;
					if (maxlength<len) len = maxlength+1;
				}
				else
				{
					bval = (char *)malloc(len + 1);
				}
				for (i=0; i<len; i++)
				{
					bval[i] = '*';
				}
				bval[len] = '\0';
				XtSetArg(arg[argcnt], XtNstring, bval);
				argcnt++;
			}
			else  /* value == NULL */
			{
				if (maxlength>0)  /* stringInPlace */
				{
					XtSetArg(arg[argcnt], XtNstring, txt);
					argcnt++;
				}
			}

			w = XtCreateWidget(name, asciiTextWidgetClass,
				hw->html.view, arg, argcnt);
                        if (maxlength > 0)
			{
				XtAddCallback(w, XtNdestroyCallback,
					(XtCallbackProc)CBTextDestroy,
					(caddr_t)txt);
			}

			XtOverrideTranslations(w,
			  XtParseTranslationTable("<Key>: HTMLpwdInput()"));
			XtOverrideTranslations(w,
			  XtParseTranslationTable("<Key>Return: no-op(RingBell)"));

			setTextSize(w,size<1?20:size,1);
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
		}
		else if ((type_str != NULL)&&(my_strcasecmp(type_str, "textarea") ==0))
		{
			char **list;
			int list_cnt;
			int rows, cols;

			type = W_TEXTAREA;

			/*
			 * If there is no SIZE, look for ROWS and COLS
			 * directly.
			 * SIZE is COLUMNS,ROWS parse the list
			 */
			rows = -1;
			cols = -1;
			tptr = ParseMarkTag(text, MT_INPUT, "SIZE");
			if (tptr == NULL)
			{
				tptr = ParseMarkTag(text, MT_INPUT, "ROWS");
				if (tptr != NULL)
				{
					rows = atoi(tptr);
					free(tptr);
				}
				tptr = ParseMarkTag(text, MT_INPUT, "COLS");
				if (tptr != NULL)
				{
					cols = atoi(tptr);
					free(tptr);
				}
			}
			else
			{
				list = ParseCommaList(tptr, &list_cnt);
				free(tptr);

				if (list_cnt == 1)
				{
					cols = atoi(list[0]);
				}
				else if (list_cnt > 1)
				{
					cols = atoi(list[0]);
					rows = atoi(list[1]);
				}
				FreeCommaList(list, list_cnt);
			}

			/*
			 * Grab the starting value of the text here.
			 * NULL if none.
			 */
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			UnMuckTextAreaValue(value);

			argcnt = 0;
			XtSetArg(arg[argcnt], XtNx, x); argcnt++;
			XtSetArg(arg[argcnt], XtNy, y); argcnt++;
			XtSetArg(arg[argcnt], XtNeditType, XawtextEdit);
			argcnt++;

			if (value != NULL)
			{
				XtSetArg(arg[argcnt], XtNstring, value);
				argcnt++;
			}
			w = XtCreateWidget(name, asciiTextWidgetClass,
				hw->html.view, arg, argcnt);
			setTextSize(w,cols>0?cols:20,rows>0?rows:1);

			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
		}
		else /* if no type, assume type=text */
		{
			char **list;
			int list_cnt;
			int rows, cols;

			/*
			 * SIZE can be either COLUMNS or COLUMNS,ROWS
			 * we assume COLUMNS,ROWS and parse the list
			 */
			tptr = ParseMarkTag(text, MT_INPUT, "SIZE");
			list = ParseCommaList(tptr, &list_cnt);
			if (tptr != NULL)
			{
				free(tptr);
			}

			/*
			 * If only COLUMNS specified, or SIZE not specified
			 * assume a TEXTFIELD
			 * Otherwise a TEXTAREA.
			 */
			if (list_cnt <= 1)
			{
				type = W_TEXTFIELD;
				if (list_cnt == 1)
				{
					cols = atoi(list[0]);
				}
				else
				{
					cols = -1;
				}
			}
			else
			{
				type = W_TEXTAREA;
				cols = atoi(list[0]);
				rows = atoi(list[1]);
			}
			/*
			 * Now that we have cols, and maybe rows, free the list
			 */
			FreeCommaList(list, list_cnt);

			/*
			 * Grab the starting value of the text here.
			 * NULL if none.
			 */
			value = ParseMarkTag(text, MT_INPUT, "VALUE");

			/*
			 * For textfileds parse maxlength and
			 * set up the widget.
			 */
			if (type == W_TEXTFIELD)
			{
				char *txt;

				maxlength = -1;
				tptr = ParseMarkTag(text, MT_INPUT,"MAXLENGTH");
				if (tptr != NULL)
				{
					maxlength = atoi(tptr);
					free(tptr);
				}

				argcnt = 0;
				XtSetArg(arg[argcnt], XtNx, x); argcnt++;
				XtSetArg(arg[argcnt], XtNy, y); argcnt++;

				if (maxlength > 0)
				{
					if (value)
					{
						txt = XtNewString(value);
						txt = (char *)realloc(txt,
							maxlength);
					}
					else
					{
						txt = (char *)malloc(maxlength);
						*txt = '\0';
					}
					XtSetArg(arg[argcnt],
						XtNuseStringInPlace,1);
					argcnt++;
					XtSetArg(arg[argcnt],
						XtNlength, maxlength);
					argcnt++;
					XtSetArg(arg[argcnt], XtNstring, txt);
					argcnt++;
				}
				else
				{
					if (value != NULL)
					{
						XtSetArg(arg[argcnt],
							XtNuseStringInPlace,0);
						argcnt++;
						txt = value;
						XtSetArg(arg[argcnt],
							XtNstring, txt);
						argcnt++;
					}
				}

				XtSetArg(arg[argcnt], XtNeditType, XawtextEdit);
				argcnt++;
				w = XtCreateWidget(name,
					asciiTextWidgetClass,
					hw->html.view, arg, argcnt);
				if (maxlength > 0)
				{
					XtAddCallback(w, XtNdestroyCallback,
						CBTextDestroy, (caddr_t)txt);
				}

				XtOverrideTranslations(w,
					XtParseTranslationTable(
					 "<Key>Return: no-op(RingBell)"));
				setTextSize(w,cols>0?cols:20,1);
/* Twice??? -- SWP */
/*
				XtSetMappedWhenManaged(w, False);
				XtManageChild(w);
*/
			}
			/*
			 * Else this is a TEXTAREA.  Maxlength is ignored,
			 * and we set up the scrolled window
			 */
			else
			{
				argcnt = 0;
				XtSetArg(arg[argcnt], XtNx, x); argcnt++;
				XtSetArg(arg[argcnt], XtNy, y); argcnt++;
				XtSetArg(arg[argcnt], XtNeditType, XawtextEdit);
				argcnt++;

				if (value != NULL)
				{
					XtSetArg(arg[argcnt], XtNstring, value);
					argcnt++;
				}
				w = XtCreateWidget(name,
					asciiTextWidgetClass,
					hw->html.view, arg, argcnt);
				setTextSize(w,cols>0?cols:20,rows>0?rows:1);
/* Twice??? -- SWP */
/*
				XtSetMappedWhenManaged(w, False);
				XtManageChild(w);
*/
			}

/* Do it only here instead of both places above and here... -- SWP */
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
		}
		if (type_str != NULL)
		{
			free(type_str);
		}

		/*
		 * Don't want to do GetValues if this is HIDDEN input
		 * tag with no widget.
		 */
		if (w != NULL)
		{
			argcnt = 0;
			XtSetArg(arg[argcnt], XtNwidth, &width); argcnt++;
			XtSetArg(arg[argcnt], XtNheight, &height); argcnt++;
			XtGetValues(w, arg, argcnt);
		}
		else
		{
			width = 0;
			height = 0;
		}

		wptr = AddNewWidget(hw, fptr, w, type, id, x, y, width, height,
			name, value, (char **) 0,checked);
					/* ^^^ ddt 4/3/95 */
	}
	else
	/*
	 * We found this widget on the list of already created widgets.
	 * Put it in place for reuse.
	 */
	{
		wlist->x = x;
		wlist->y = y;

		/*
		 * Don't want to SetValues if type HIDDEN which
		 * has no widget.
		 */
		if (wlist->w != NULL)
		{
			XtUnmanageChild(wlist->w);
			argcnt = 0;
			XtSetArg(arg[argcnt], XtNx, x); argcnt++;
			XtSetArg(arg[argcnt], XtNy, y); argcnt++;
			XtSetValues(wlist->w, arg, argcnt);
			XtManageChild(wlist->w);
		}

		wptr = wlist;
	}

	return(wptr);
}
#endif /* MOTIF */


void
WidgetRefresh(hw, eptr)
        HTMLWidget hw;
        struct ele_rec *eptr;
{

/*
unsigned long wp=WhitePixel(XtDisplay(hw),DefaultScreen(XtDisplay(hw)));
unsigned long bp=BlackPixel(XtDisplay(hw),DefaultScreen(XtDisplay(hw)));
*/

	if ((eptr->widget_data != NULL)&&(eptr->widget_data->mapped == False)&&
		(eptr->widget_data->w != NULL))
	{
		XSetForeground(XtDisplay(hw), hw->html.drawGC, eptr->fg);
		XSetBackground(XtDisplay(hw), hw->html.drawGC, eptr->bg);

		eptr->widget_data->mapped = True;
		XtSetMappedWhenManaged(eptr->widget_data->w, True);
	}
}

void traversal_forward(Widget w, XEvent *event,
	       String *params, Cardinal *num_params)
{ 
  HTMLTraverseTabGroups(w, XmTRAVERSE_NEXT_TAB_GROUP);
}

void traversal_back(Widget w, XEvent *event,
	       String *params, Cardinal *num_params)
{ 
  HTMLTraverseTabGroups(w, XmTRAVERSE_PREV_TAB_GROUP);
}

void traversal_current(Widget w, XEvent *event,
	       String *params, Cardinal *num_params)
{
  HTMLTraverseTabGroups(w, XmTRAVERSE_CURRENT);
}

void traversal_end(Widget w, XEvent *event,
	       String *params, Cardinal *num_params)
{
  Widget top;
  HTMLWidget hw = (HTMLWidget) w;
  int i=0;

  while(i < 5)
    {
      if(XtClass((Widget) hw) != htmlWidgetClass)
	hw = (HTMLWidget) XtParent((Widget) hw);
      else
	break;
      i++;
    }

  top = (Widget) hw;
  while(!XtIsTopLevelShell(top))
    top = XtParent(top);

  if(XtClass((Widget) hw) != htmlWidgetClass)
    {
      fprintf(stderr, "Error in traversal_end action.");
      return;
    }

  if(hw->html.focus_follows_mouse)
    return;
  else
    {
      XtSetKeyboardFocus(top, hw->html.view);
      HTMLTraverseTabGroups(w, XmTRAVERSE_HOME);
    }
}

/* this function is intended to imitate XmProcessTraversal */
void HTMLTraverseTabGroups(Widget w, int how)
{
  static WidgetInfo *lptr;
  Widget top;
  Boolean ret;
  int i=0;
  HTMLWidget hw = (HTMLWidget) w;

  /* due to the generality of this function the HTMLwidget could be anywhere */
  for(i=0;i<4;i++)
    {
      if((hw != NULL) && (XtClass((Widget) hw) != htmlWidgetClass))
	hw = (HTMLWidget) XtParent((Widget) hw);
      else
	break;
    }

  if(!hw || XtClass((Widget) hw) != htmlWidgetClass)
    return;

  /* make sure we have business to do */
  if(!hw->html.widget_list || hw->html.focus_follows_mouse)
    return;

  top = (Widget) hw;
  while(!XtIsTopLevelShell(top))
    top = XtParent(top);
  
  switch(how)
    {
    case XmTRAVERSE_NEXT_TAB_GROUP:
      if(!lptr)
	lptr=hw->html.widget_list;
      else if(lptr->next && lptr->next->w && XtIsManaged(lptr->next->w))
	lptr = lptr->next;

      /* Patch for hidden fields... SWP */
      while (lptr && !lptr->w) {
	if (lptr->next) {
	  lptr=lptr->next;
	}
	else {
	  lptr=NULL;
	}
      }

      if (!lptr) {
	break;
      }

      /* automagickly scroll */
      if(XtIsManaged(hw->html.vbar) && 
	 (lptr->y > (hw->html.view_height+hw->html.scroll_y-10)))
	{
	  int val, ss, in, pg_in, amount;

	  amount = lptr->y-hw->html.view_height/2;
	  if(amount<0)
	    amount=0;
	  XmScrollBarGetValues(hw->html.vbar, &val, &ss, &in, &pg_in);

	  if(amount > (hw->html.doc_height-ss-5))
	    amount = hw->html.doc_height-ss-5;

	  XmScrollBarSetValues(hw->html.vbar, amount,
			       ss, in, pg_in);
	}

      if(XtClass(lptr->w) == xmScrolledWindowWidgetClass)
	{
	  Widget text;
	  XtVaGetValues(lptr->w, XmNworkWindow, &text, NULL);
	  XtSetKeyboardFocus(top, text);
	} 
      else
	XtSetKeyboardFocus(top, lptr->w);
      break;
    case XmTRAVERSE_PREV_TAB_GROUP:
      if(!lptr)
	lptr=hw->html.widget_list;
      else if(lptr->prev)
	lptr = lptr->prev;

      /* Patch for hidden fields... SWP */
      while (lptr && !lptr->w) {
	if (lptr->prev) {
	  lptr=lptr->prev;
	}
	else {
	  lptr=NULL;
	}
      }

      if (!lptr) {
	break;
      }

      /* automagickly scroll */
      if(XtIsManaged(hw->html.vbar) && 
	 (lptr->y < hw->html.scroll_y+10))
	{
	  int val, ss, in, pg_in, amount;

	  XmScrollBarGetValues(hw->html.vbar, &val, &ss, &in, &pg_in);

	  amount = lptr->y - hw->html.view_height/2;

	  if(amount<0)
	    amount=0;

	  XmScrollBarSetValues(hw->html.vbar, amount,
			       ss, in, pg_in);
	}

      if(XtClass(lptr->w) == xmScrolledWindowWidgetClass)
	{
	  Widget text;

	  XtVaGetValues(lptr->w, XmNworkWindow, &text, NULL);
	  XtSetKeyboardFocus(top, text);
	}
      else
	XtSetKeyboardFocus(top, lptr->w);
      break;
    case XmTRAVERSE_HOME:
      lptr=NULL;
      break;
    case XmTRAVERSE_CURRENT:
      lptr = hw->html.widget_list;
      
      /* check parent to allow for text areas (lptr->w would be scroll) */
      while(lptr!=NULL) 
	{
	  if((lptr->w == w) || (lptr->w == XtParent(w)))
	    break;
	  lptr = lptr->next;
	}

      XtSetKeyboardFocus(top, w);

      break;
    }
}
