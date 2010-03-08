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
#include "XmxP.h"

#if defined(ultrix) || defined(VMS) || defined(NeXT)
extern char *strdup ();
#endif

XmString xwt_str_array_to_xmstr(char *normal_string[], int numargs);

/*
void startbouncetimer();
void stopbouncetimer();
*/

/* Kludge for broken linux */
extern Pixmap dialogError, dialogInformation, dialogQuestion, dialogWarning;

/* ---------------------------- FILE VARIABLES ---------------------------- */

/* Variables accessed through Xmx.h as extern. */
int    Xmx_n = 0;
Arg    Xmx_wargs[XmxMaxNumArgs];
Widget Xmx_w;
int    Xmx_uniqid = 0;

/* Counter for values returned from XmxMakeNewUniqid. */
static int Xmx_uniqid_counter = 0;

/* Flag for whether or not XmxSetUniqid has ever been called. */
static int Xmx_uniqid_has_been_set = 0;


/* --------------------------- UNIQID FUNCTIONS --------------------------- */

int
XmxMakeNewUniqid (void)
{
  Xmx_uniqid_counter++;

  return Xmx_uniqid_counter;
}

void
XmxSetUniqid (int uniqid)
{
  Xmx_uniqid = uniqid;
  Xmx_uniqid_has_been_set = 1;

  return;
}

void
XmxZeroUniqid (void)
{
  Xmx_uniqid = 0;
  /* Do NOT reset Xmx_uniqid_has_been_set. */

  return;
}

int
XmxExtractUniqid (int cd)
{
  /* Pull the high 16 bits, if uniqid has been set. */
  if (Xmx_uniqid_has_been_set)
    return (cd >> 16);
  else
    return 0;
}

int
XmxExtractToken (int cd)
{
  /* Pull the low 16 bits, if uniqid has been set. */
  if (Xmx_uniqid_has_been_set)
    return ((cd << 16) >> 16);
  else
    return cd;
}

/* This function should be called by every Xmx routine
   when registering a callback or event handler. */
/* This is PRIVATE but accessible to Xmx2.c also. */
int
_XmxMakeClientData (int token)
{
  if (Xmx_uniqid_has_been_set)
    return ((Xmx_uniqid << 16) | token);
  else
    return token;
}

/* -------------------------- INTERNAL CALLBACKS -------------------------- */

/* Internal routine to unmanage file selection box on Cancel. */
static
XmxCallback (_XmxCancelCallback)
{
  XtUnmanageChild (w);

  return;
}


/* --------------------------- CALLBACK SUPPORT --------------------------- */

/* args do nothing */
void XmxAddCallback (Widget w, String name, XtCallbackProc cb, int cb_data)
{
  XtAddCallback (w, name, cb, (XtPointer)_XmxMakeClientData (cb_data));
  return;
}


/* ------------------------ EVENT HANDLER SUPPORT ------------------------- */

void
XmxAddEventHandler (Widget w, EventMask event_mask, XtEventHandler proc,
                    int client_data)
{
  XtAddEventHandler 
    (w, event_mask, False, proc, (XtPointer)_XmxMakeClientData (client_data));

  return;
}

void
XmxRemoveEventHandler (Widget w, EventMask event_mask, XtEventHandler proc,
                       int client_data)
{
  XtRemoveEventHandler
    (w, event_mask, False, proc, (XtPointer)_XmxMakeClientData (client_data));

  return;
}


/* ------------------- (nonworking) CENTERING ROUTINES -------------------- */

#ifdef NONWORKING_CENTERING

/* Adapted from Dan Heller's post in comp.windows.x.motif;
   assumes BulletinBoard with one (centered) child. */
static void
XmxCenteringResize 
  (CompositeWidget w, XConfigureEvent *event, String args[], int *num_args)
{
  WidgetList children;
  int width = event->width;
  int height = event->height;
  Dimension w_width, w_height; 
  
  /* get handle to BulletinBoard's children and marginal spacing */
  XtVaGetValues (w,
                 XmNchildren, &children,
                 NULL);
  
  XtVaGetValues (children[0],
                 XmNheight, &w_height,
                 XmNwidth, &w_width,
                 NULL);
  XtVaSetValues (children[0],
                 XmNx, width/2 - w_width/2,
                 XmNy, height/2 - w_height/2,
                 NULL);
                 
  return;
}

#endif /* NONWORKING_CENTERING */

/* -------------------------- UTILITY FUNCTIONS --------------------------- */

/* resets args */
void XmxStartup (void)
{
  Xmx_n = 0;
#ifdef MOTIF1_2
  XmRepTypeInstallTearOffModelConverter();
#endif
  return;
}

/* sets an arg */
void XmxSetArg (String arg, XtArgVal val)
{
  XtSetArg (Xmx_wargs[Xmx_n], arg, val);
  Xmx_n++;
  return;
}

void XmxSetValues (Widget w)
{
  XtSetValues (w, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return;
}

/* args do nothing */
void XmxManageRemanage (Widget w)
{
#if 0
  if (XtIsManaged (w))
    XtUnmanageChild (w);
  XtManageChild (w);
#endif
  
  if (XtIsManaged (w))
    if (XtIsShell(w))
      XRaiseWindow (XtDisplay (w), XtWindow (w));
    else
      XMapRaised (XtDisplay (w), XtWindow (XtParent(w)));
  else
    XtManageChild (w);

  return;
}

/* args do nothing */
void XmxSetSensitive (Widget w, int state)
{
  assert (state == XmxSensitive || state == XmxUnsensitive);
  XtSetSensitive (w, (state == XmxSensitive) ? True : False);

  return;
}

/* ------------------------------------------------------------------------ */
/* ---------------- WIDGET CREATION AND HANDLING ROUTINES ----------------- */
/* ------------------------------------------------------------------------ */

/* ----------------------------- PUSHBUTTONS ------------------------------ */

/* args work */
Widget XmxMakePushButton (Widget parent, String name, XtCallbackProc cb,
                          int cb_data)
{
  XmString xmstr = NULL;

  if (name)
    {
      xmstr = XmStringCreateLtoR (name, XmSTRING_DEFAULT_CHARSET);
      XmxSetArg (XmNlabelString, (XtArgVal)xmstr);
    }
  Xmx_w = XtCreateManagedWidget ("pushbutton", xmPushButtonWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  XtAddCallback (Xmx_w, XmNactivateCallback, cb, 
                 (XtPointer)_XmxMakeClientData (cb_data));

  if (xmstr)
    XmStringFree (xmstr);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeNamedPushButton (Widget parent, String name, String wname, 
                               XtCallbackProc cb,
                               int cb_data)
{
  XmString label;
  
  if (name)
    {
      label = XmStringCreateLtoR (name, XmSTRING_DEFAULT_CHARSET);
      XmxSetArg (XmNlabelString, (XtArgVal)label);
    }
  Xmx_w = XtCreateManagedWidget (wname, xmPushButtonWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  XtAddCallback (Xmx_w, XmNactivateCallback, cb, 
                 (XtPointer)_XmxMakeClientData (cb_data));
  if (name)
    XmStringFree (label);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeBlankButton (Widget parent, XtCallbackProc cb,
			   int cb_data)
{
  Xmx_w = XtCreateManagedWidget ("blankbutton", xmPushButtonWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  XtAddCallback (Xmx_w, XmNactivateCallback, cb, 
                 (XtPointer)_XmxMakeClientData (cb_data));
  
  Xmx_n = 0;
  return Xmx_w;
}

/* ------------------------------- COMMAND -------------------------------- */

/* args work */
Widget XmxMakeCommand 
  (Widget parent, String prompt, XtCallbackProc cb, int cb_data)
{
  XmString xmstr = XmxMakeXmstrFromString (prompt);

  XmxSetArg (XmNpromptString, (XtArgVal)xmstr);
  Xmx_w = XtCreateManagedWidget ("command", xmCommandWidgetClass, parent,
				 Xmx_wargs, Xmx_n);
  XtAddCallback 
    (Xmx_w, XmNcommandEnteredCallback, cb, 
     (XtPointer)_XmxMakeClientData (cb_data));

  XmStringFree (xmstr);

  Xmx_n = 0;
  return Xmx_w;
}

/* ---------------------------- SCROLLED LIST ----------------------------- */

/* args work */
Widget XmxMakeScrolledList
  (Widget parent, XtCallbackProc cb, int cb_data)
{
  Xmx_w = XmCreateScrolledList (parent, "scrolled_list", Xmx_wargs, Xmx_n);
  XtManageChild (Xmx_w);
  /* defaultAction gets triggered on double click and sends item
     along with it... */
  XtAddCallback (Xmx_w, XmNdefaultActionCallback, cb,
                 (XtPointer)_XmxMakeClientData (cb_data));
  
  Xmx_n = 0;
  return Xmx_w;
}

/* ----------------------------- DRAWING AREA ----------------------------- */

/* args work */
Widget XmxMakeDrawingArea (Widget parent, int width, int height)
{
  XmxSetArg (XmNwidth, (XtArgVal)width);
  XmxSetArg (XmNheight, (XtArgVal)height);
  Xmx_w = XtCreateManagedWidget ("drawingarea", xmDrawingAreaWidgetClass,
				 parent, Xmx_wargs, Xmx_n);

  Xmx_n = 0;
  return Xmx_w;
}

/* ------------------------ TOGGLE BUTTONS & BOXES ------------------------ */

/* args work */
Widget XmxMakeRadioBox (Widget parent)
{
  /* Could set XmxNspacing here to avoid having to play with
     margins for each togglebutton. */
  XmxSetArg (XmNspacing, (XtArgVal)0);
  XmxSetArg (XmNentryClass, (XtArgVal)xmToggleButtonGadgetClass);
  Xmx_w = XmCreateRadioBox (parent, "radiobox", Xmx_wargs, Xmx_n);
  XtManageChild (Xmx_w);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeOptionBox (Widget parent)
{
  XmxSetArg (XmNentryClass, (XtArgVal)xmToggleButtonGadgetClass);
  XmxSetArg (XmNisHomogeneous, (XtArgVal)True);
  Xmx_w = XtCreateManagedWidget ("optionbox", xmRowColumnWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeToggleButton (Widget parent, String name, XtCallbackProc cb,
                            int cb_data)
{
  XmString label = XmStringCreateLtoR (name, XmSTRING_DEFAULT_CHARSET);
  XmxSetArg (XmNlabelString, (XtArgVal)label);
  XmxSetArg (XmNmarginHeight, (XtArgVal)0);
  Xmx_w = XtCreateManagedWidget
    ("togglebutton", xmToggleButtonWidgetClass, parent, Xmx_wargs, Xmx_n);
  /* Used to be XmNarmCallback --- probably not right. */
  if (cb)
    XtAddCallback (Xmx_w, XmNvalueChangedCallback, cb, 
                   (XtPointer)_XmxMakeClientData (cb_data));

  XmStringFree (label);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
void XmxSetToggleButton (Widget button, int set_state)
{
  assert (set_state == XmxSet || set_state == XmxUnset);
  XmToggleButtonSetState 
    (button, (set_state == XmxSet) ? True : False, False);

  Xmx_n = 0;
  return;
}

/* -------------------------------- SCALES -------------------------------- */

/* args ignored if label is non-NULL, otherwise args work */
Widget XmxMakeScale (Widget parent, XtCallbackProc cb, int cb_data,
                     String label, int min, int max, int start, int dec_adj)
{
  if (label)
    {
      Xmx_n = 0;
      XmxMakeLabel (parent, label);
    }

  XmxSetArg (XmNminimum, (XtArgVal)min);
  XmxSetArg (XmNmaximum, (XtArgVal)max);
  XmxSetArg (XmNvalue, (XtArgVal)start);
  XmxSetArg (XmNorientation, (XtArgVal)XmHORIZONTAL);
  XmxSetArg (XmNprocessingDirection, (XtArgVal)XmMAX_ON_RIGHT);
  if (dec_adj != XmxNotDisplayed)
    {
      XmxSetArg (XmNshowValue, (XtArgVal)True);
      XmxSetArg (XmNdecimalPoints, (XtArgVal)dec_adj);
    }
  Xmx_w = XtCreateManagedWidget ("scale", xmScaleWidgetClass, parent,
                                 Xmx_wargs, Xmx_n);

  XtAddCallback 
    (Xmx_w, XmNvalueChangedCallback, cb, 
     (XtPointer)_XmxMakeClientData (cb_data));
  XtAddCallback (Xmx_w, XmNdragCallback, cb, 
                 (XtPointer)_XmxMakeClientData (cb_data));

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
void XmxAdjustScale (Widget scale, int val)
{
  XmxSetArg (XmNvalue, (XtArgVal)val);
  XtSetValues (scale, Xmx_wargs, Xmx_n);

  Xmx_n = 0;
  return;
}


/* args work */
Widget XmxMakeFrame (Widget parent, int shadow)
{
  assert (shadow == XmxShadowIn || shadow == XmxShadowOut || shadow == XmxShadowEtchedIn || shadow == XmxShadowEtchedOut);
  switch (shadow)
    {
    case XmxShadowIn:
      XmxSetArg (XmNshadowType, (XtArgVal)XmSHADOW_IN);  break;
    case XmxShadowOut:
      XmxSetArg (XmNshadowType, (XtArgVal)XmSHADOW_OUT);  break;
    case XmxShadowEtchedIn:
      XmxSetArg (XmNshadowType, (XtArgVal)XmSHADOW_ETCHED_IN);  break;
    case XmxShadowEtchedOut:
      XmxSetArg (XmNshadowType, (XtArgVal)XmSHADOW_ETCHED_OUT);  break;
    }
  Xmx_w = XtCreateManagedWidget ("frame", xmFrameWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);

  Xmx_n = 0;
  return Xmx_w;
}

/* -------------------------------- FORMS --------------------------------- */

/* args work */
Widget XmxMakeForm (Widget parent)
{
  Xmx_w = XtCreateManagedWidget ("form", xmFormWidgetClass, parent,
				 Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args sent to w */
void XmxSetPositions (Widget w, int top, int bottom, int left, int right)
{
  if (top != XmxNoPosition)
    {
      XmxSetArg (XmNtopAttachment, (XtArgVal)XmATTACH_POSITION);
      XmxSetArg (XmNtopPosition, (XtArgVal)top);
    }
  if (bottom != XmxNoPosition)
    {
      XmxSetArg (XmNbottomAttachment, (XtArgVal)XmATTACH_POSITION);
      XmxSetArg (XmNbottomPosition, (XtArgVal)bottom);
    }
  if (left != XmxNoPosition)
    {
      XmxSetArg (XmNleftAttachment, (XtArgVal)XmATTACH_POSITION);
      XmxSetArg (XmNleftPosition, (XtArgVal)left);
    }
  if (right != XmxNoPosition)
    {
      XmxSetArg (XmNrightAttachment, (XtArgVal)XmATTACH_POSITION);
      XmxSetArg (XmNrightPosition, (XtArgVal)right);
    }

  XmxSetValues (w);

  Xmx_n = 0;
  return;
}

/* args sent to w */
void XmxSetOffsets (Widget w, int top, int bottom, int left, int right)
{
  if (top != XmxNoOffset)
    XmxSetArg (XmNtopOffset, (XtArgVal)top);
  if (bottom != XmxNoOffset)
    XmxSetArg (XmNbottomOffset, (XtArgVal)bottom);
  if (left != XmxNoOffset)
    XmxSetArg (XmNleftOffset, (XtArgVal)left);
  if (right != XmxNoOffset)
    XmxSetArg (XmNrightOffset, (XtArgVal)right);
  
  XmxSetValues (w);

  Xmx_n = 0;
  return;
}

/* args sent to w */
void XmxSetConstraints (Widget w, 
			int top, int bottom, int left, int right,
			Widget topw, Widget botw, Widget lefw, Widget rigw)
{
  if (top != XmATTACH_NONE)
    {
      XmxSetArg (XmNtopAttachment, (XtArgVal)top);
      if (topw)
        XmxSetArg (XmNtopWidget, (XtArgVal)topw);
    }
  
  if (bottom != XmATTACH_NONE)
    {
      XmxSetArg (XmNbottomAttachment, (XtArgVal)bottom);
      if (botw)
        XmxSetArg (XmNbottomWidget, (XtArgVal)botw);
    }

  if (left != XmATTACH_NONE)
    {
      XmxSetArg (XmNleftAttachment, (XtArgVal)left);
      if (lefw)
        XmxSetArg (XmNleftWidget, (XtArgVal)lefw);
    }

  if (right != XmATTACH_NONE)
    {
      XmxSetArg (XmNrightAttachment, (XtArgVal)right);
      if (rigw)
        XmxSetArg (XmNrightWidget, (XtArgVal)rigw);
    }
  
  XmxSetValues (w);

  Xmx_n = 0;
  return;
}

/* ------------------------------ ROWCOLUMNS ------------------------------ */

/* args work */
Widget XmxMakeVerticalRowColumn (Widget parent)
{
  Xmx_w = XtCreateManagedWidget ("rowcolumn", xmRowColumnWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeHorizontalRowColumn (Widget parent)
{
  XmxSetArg (XmNorientation, (XtArgVal)XmHORIZONTAL);
  Xmx_w = XtCreateManagedWidget ("rowcolumn", xmRowColumnWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeNColumnRowColumn (Widget parent, int ncolumns)
{
  XmxSetArg (XmNorientation, (XtArgVal)XmVERTICAL);
  XmxSetArg (XmNpacking, (XtArgVal)XmPACK_COLUMN);
  XmxSetArg (XmNnumColumns, (XtArgVal)ncolumns);
  Xmx_w = XtCreateManagedWidget ("rowcolumn", xmRowColumnWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* --------------------------- BULLETIN BOARDS ---------------------------- */

/* args work */
Widget XmxMakeVerticalBboard (Widget parent)
{
  Xmx_w = XtCreateManagedWidget ("bboard", xmBulletinBoardWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeVerticalBboardWithFont (Widget parent, String fontname)
{
  XFontStruct *_font;
  XmFontList _fontlist;

  _font = XLoadQueryFont (XtDisplay (parent), fontname);
  if (_font != (XFontStruct *)NULL)
    {
      _fontlist = XmFontListCreate (_font, XmSTRING_DEFAULT_CHARSET);
      XmxSetArg (XmNbuttonFontList, (XtArgVal)_fontlist);
      XmxSetArg (XmNlabelFontList, (XtArgVal)_fontlist);
      XmxSetArg (XmNtextFontList,(XtArgVal) _fontlist);
    }
  XmxSetArg (XmNmarginWidth, (XtArgVal)0);
  XmxSetArg (XmNmarginHeight, (XtArgVal)0);
  Xmx_w = XtCreateManagedWidget ("bboard", xmBulletinBoardWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeHorizontalBboard (Widget parent)
{
  XmxSetArg (XmNorientation, (XtArgVal)XmHORIZONTAL);
  Xmx_w = XtCreateManagedWidget ("bboard", xmBulletinBoardWidgetClass,
                              parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* -------------------- (nonworking) CENTERING BBOARD --------------------- */

#ifdef NONWORKING_CENTERING

/* args work */
Widget XmxMakeCenteringBboard (Widget parent, XtAppContext app)
{
  XtActionsRec rec;

  Xmx_w = XtCreateManagedWidget 
    ("bboard", xmBulletinBoardWidgetClass, parent, Xmx_wargs, Xmx_n);

  /* Does this have to happen more than once? */
  rec.string = "resize";
  rec.proc = XmxCenteringResize;
  XtAppAddActions (app, &rec, 1);

  /* This does, for sure... */
  XtOverrideTranslations 
    (Xmx_w, XtParseTranslationTable ("<Configure>: resize()"));
  
  Xmx_n = 0;
  return Xmx_w;
}

#endif /* NONWORKING_CENTERING */

/* -------------------------------- LABELS -------------------------------- */

/* args work */
void XmxAdjustLabelText (Widget label, String text)
{
  XmString xmstr = XmStringCreateLtoR (text, XmSTRING_DEFAULT_CHARSET);
  XmxSetArg (XmNlabelString, (XtArgVal)xmstr);
  XtSetValues (label, Xmx_wargs, Xmx_n);
  XmStringFree (xmstr);

  Xmx_n = 0;
  return;
}

/* args work */
Widget XmxMakeLabel (Widget parent, String name)
{
  XmString xmstr = XmStringCreateLtoR (name, XmSTRING_DEFAULT_CHARSET);
  XmxSetArg (XmNlabelString, (XtArgVal)xmstr);
  Xmx_w = XtCreateManagedWidget ("label", xmLabelWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  XmStringFree (xmstr);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeNamedLabel (Widget parent, String name, String wname)
{
  XmString xmstr = XmStringCreateLtoR (name, XmSTRING_DEFAULT_CHARSET);
  XmxSetArg (XmNlabelString, (XtArgVal)xmstr);
  Xmx_w = XtCreateManagedWidget (wname, xmLabelWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  XmStringFree (xmstr);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeBlankLabel (Widget parent)
{
  Xmx_w = XtCreateManagedWidget ("label", xmLabelWidgetClass,
                                 parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* ------------------------------- DIALOGS -------------------------------- */

/* args work */
Widget XmxMakeErrorDialog (Widget parent, String name, String title)
{
  XmString message = XmStringCreateLtoR (name, XmSTRING_DEFAULT_CHARSET);
  XmString dialog = XmStringCreateLtoR (title, XmSTRING_DEFAULT_CHARSET);
  XmxSetArg (XmNmessageString, (XtArgVal)message);
  XmxSetArg (XmNdialogTitle, (XtArgVal)dialog);
  XmxSetArg (XmNsymbolPixmap, (XtArgVal)dialogError);

  Xmx_w = XmCreateErrorDialog (parent, "error", Xmx_wargs, Xmx_n);
  XtUnmanageChild (XmMessageBoxGetChild (Xmx_w, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (Xmx_w, XmDIALOG_HELP_BUTTON));

  XmStringFree (message);
  XmStringFree (dialog);

  Xmx_n = 0;
  return Xmx_w;
}


/* args work */
Widget XmxMakeInfoDialog (Widget parent, String name, String title)
{
  XmString message = XmStringCreateLtoR (name, XmSTRING_DEFAULT_CHARSET);
  XmString dialog = XmStringCreateLtoR (title, XmSTRING_DEFAULT_CHARSET);
  XmxSetArg (XmNmessageString, (XtArgVal)message);
  XmxSetArg (XmNdialogTitle, (XtArgVal)dialog);
  XmxSetArg (XmNsymbolPixmap, (XtArgVal)dialogInformation);

  Xmx_w = XmCreateInformationDialog (parent, "infozoid", Xmx_wargs, Xmx_n);
  XtUnmanageChild (XmMessageBoxGetChild (Xmx_w, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (Xmx_w, XmDIALOG_HELP_BUTTON));

  XmStringFree (message);
  XmStringFree (dialog);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeQuestionDialog (Widget parent, String question, String title,
			      XtCallbackProc cb, int yes_token, int no_token)
{
  XmString message = XmStringCreateLtoR (question, XmSTRING_DEFAULT_CHARSET);
  XmString dialog = XmStringCreateLtoR (title, XmSTRING_DEFAULT_CHARSET);
  XmString ok = XmStringCreateLtoR ("Yes", XmSTRING_DEFAULT_CHARSET);
  XmString cancel = XmStringCreateLtoR ("No", XmSTRING_DEFAULT_CHARSET);
  XmxSetArg (XmNmessageString, (XtArgVal)message);
  XmxSetArg (XmNdialogTitle, (XtArgVal)dialog);
  XmxSetArg (XmNokLabelString, (XtArgVal)ok);
  XmxSetArg (XmNcancelLabelString, (XtArgVal)cancel);
  XmxSetArg (XmNsymbolPixmap, (XtArgVal)dialogQuestion);

  Xmx_w = XmCreateQuestionDialog (parent, "question", Xmx_wargs, Xmx_n);
  XtUnmanageChild (XmMessageBoxGetChild (Xmx_w, XmDIALOG_HELP_BUTTON));

  XtAddCallback (Xmx_w, XmNcancelCallback, cb, 
                 (XtPointer)_XmxMakeClientData (no_token));
  XtAddCallback (Xmx_w, XmNokCallback, cb, 
                 (XtPointer)_XmxMakeClientData (yes_token));

  XmStringFree (message);
  XmStringFree (dialog);
  XmStringFree (ok);
  XmStringFree (cancel);

  Xmx_n = 0;
  return Xmx_w;
}

/* ----------------------------- STRING UTILS ----------------------------- */

/* args do nothing */
XmString XmxMakeXmstrFromFile (String filename)
{
  FILE *_f;
  char _mstr[81];
  XmString _xmstr;

  _f = fopen (filename, "r");
  assert (_f != (FILE *)NULL);

  _xmstr = (XmString)NULL;
  while (!feof (_f))
    {
      if (fgets (_mstr, 80, _f) == (char *)NULL)
        break;
      _mstr[strlen (_mstr)-1] = '\0';
      if (_xmstr != (XmString)NULL)
        _xmstr = XmStringConcat (_xmstr, XmStringSeparatorCreate ());
      /* Used to be XmStringCreate; changed to standard call. */
      _xmstr = XmStringConcat
        (_xmstr, XmStringCreateLtoR (_mstr, XmSTRING_DEFAULT_CHARSET));
    }

  fclose (_f);
  return _xmstr;
}

/* args do nothing */
XmString XmxMakeXmstrFromString (String mstr)
{
  XmString _xmstr;

  _xmstr = XmStringCreateLtoR (mstr, XmSTRING_DEFAULT_CHARSET);
  return _xmstr;
}

/* args work */
Widget XmxMakeBboardDialog (Widget parent, String title)
{
  XmString xmstr = XmStringCreateLtoR (title, XmSTRING_DEFAULT_CHARSET);
  XmxSetArg (XmNdialogTitle, (XtArgVal)xmstr);
  XmxSetArg (XmNautoUnmanage, (XtArgVal)False);
  XmxSetArg (XmNmarginWidth, (XtArgVal)0);
  XmxSetArg (XmNmarginHeight, (XtArgVal)0);

  Xmx_w = XmCreateBulletinBoardDialog (parent, "bbdialog", Xmx_wargs, Xmx_n);
  XmStringFree (xmstr);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeFormDialog (Widget parent, String title)
{
  XmString xmstr = XmStringCreateLtoR (title, XmSTRING_DEFAULT_CHARSET);
  XmxSetArg (XmNdialogTitle, (XtArgVal)xmstr);
  XmxSetArg (XmNautoUnmanage, (XtArgVal)False);

  Xmx_w = XmCreateFormDialog (parent, "formdialog", Xmx_wargs, Xmx_n);
  XmStringFree (xmstr);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeFileSBDialog (Widget parent, String title, String selection_txt,
                            XtCallbackProc cb, int cb_data)
{
  Widget _selection_label;
  XmString dialog_title = XmStringCreateLtoR (title, XmSTRING_DEFAULT_CHARSET);
  XmString label = XmStringCreateLtoR (selection_txt, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg (XmNdialogTitle, (XtArgVal)dialog_title);
  /* Can't set width of box with XmNwidth here... why not? */

  /* this will cause the dialog to only resize if needed. That 
     way it won't be growing and shrinking all the time... very annoying. - DXP */
  XmxSetArg (XmNresizePolicy, (XtArgVal)XmRESIZE_GROW);

  /* Create the FileSelectionBox with OK and Cancel buttons. */
  Xmx_w = XmCreateFileSelectionDialog (parent, "fsb", Xmx_wargs, Xmx_n);
  XtUnmanageChild (XmFileSelectionBoxGetChild (Xmx_w, XmDIALOG_HELP_BUTTON));
  XtAddCallback (Xmx_w, XmNokCallback, cb, 
                 (XtPointer)_XmxMakeClientData (cb_data));
  XtAddCallback (Xmx_w, XmNcancelCallback, 
                 (XtCallbackProc)_XmxCancelCallback, 0);

  /* Set selection label to specified selection_txt. */
  Xmx_n = 0;
  _selection_label = 
    XmFileSelectionBoxGetChild (Xmx_w, XmDIALOG_SELECTION_LABEL);
  XmxSetArg (XmNlabelString, (XtArgVal)label);
  XtSetValues (_selection_label, Xmx_wargs, Xmx_n);

  XmStringFree (dialog_title);
  XmStringFree (label);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeHelpDialog (Widget parent, XmString xmstr, String title)
{
  XmString dialog_title = XmStringCreateLtoR (title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg (XmNmessageString, (XtArgVal)xmstr);
  XmxSetArg (XmNdialogTitle, (XtArgVal)dialog_title);

  Xmx_w = XmCreateMessageDialog (parent, "helpdialog", Xmx_wargs, Xmx_n);
  XtUnmanageChild (XmMessageBoxGetChild (Xmx_w, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (Xmx_w, XmDIALOG_HELP_BUTTON));

  XmStringFree (dialog_title);

  Xmx_n = 0;
  return Xmx_w;
}

/* Boy, this is a hack. */
static
XmxCallback(_XmxHelpTextCancelCallback)
{
  /* This is highly dependent on the button being four layers
     below the dialog shell... what a ridiculous hack. */
  XtUnmanageChild (XtParent (XtParent (XtParent (XtParent (w)))));

  return;
}

/* args work */
Widget XmxMakeHelpTextDialog 
  (Widget parent, String str, String title, Widget *text_w)
{
  Widget _box, _outer_frame, _form;
  Widget _scr_text, _sep, _buttons_form;

  /* Create the dialog box. */
  _box = XmxMakeFormDialog (parent, title);

  /* Make it 3D. */
  _outer_frame = XmxMakeFrame (_box, XmxShadowOut);
  XmxSetConstraints
    (_outer_frame, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
     NULL, NULL, NULL, NULL);

  /* Put form inside that, then frame for text window. */
  _form = XmxMakeForm (_outer_frame);

  /* Make multiline non-editable text window, with scrollbars. */
  XmxSetArg (XmNscrolledWindowMarginWidth, (XtArgVal)10);
  XmxSetArg (XmNscrolledWindowMarginHeight, (XtArgVal)10);
  XmxSetArg (XmNcursorPositionVisible, (XtArgVal)False);
  XmxSetArg (XmNeditable, (XtArgVal)False);
  XmxSetArg (XmNeditMode, (XtArgVal)XmMULTI_LINE_EDIT);
  XmxSetArg (XmNrows, (XtArgVal)20);
  XmxSetArg (XmNcolumns, (XtArgVal)60);
  XmxSetArg (XmNwordWrap, (XtArgVal)True);
  XmxSetArg (XmNscrollHorizontal, (XtArgVal)False);
  _scr_text = XmxMakeScrolledText (_form);
  XmTextSetString (_scr_text, str);

  /* Separate the text window/frame and the OK button. */
  XmxSetArg (XmNtopOffset, (XtArgVal)10);
  _sep = XmxMakeHorizontalSeparator (_form);

  /* Make an OK button. */
  _buttons_form = XmxMakeFormAndOneButton 
    (_form, _XmxHelpTextCancelCallback, "OK", 0);

  /* Constraints for _form. */
  XmxSetConstraints 
    (XtParent (_scr_text), XmATTACH_FORM, XmATTACH_WIDGET, XmATTACH_FORM, 
     XmATTACH_FORM, NULL, _sep, NULL, NULL);
  XmxSetConstraints 
    (_sep, XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
     NULL, _buttons_form, NULL, NULL);
  XmxSetConstraints 
    (_buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
     NULL, NULL, NULL, NULL);

  /* Return _scr_text in text_w argument. */
  *text_w = _scr_text;
  Xmx_w = _box;
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
void XmxAdjustHelpDialogText (Widget dialog, XmString message,
                              String title)
{
  XmString dialog_title = XmStringCreateLtoR (title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg (XmNdialogTitle, (XtArgVal)dialog_title);
  XmxSetArg (XmNmessageString, (XtArgVal)message);
  XtSetValues (dialog, Xmx_wargs, Xmx_n);

  XmStringFree (dialog_title);

  Xmx_n = 0;
  return;
}

/* args work */
void XmxAdjustDialogTitle (Widget dialog, String title)
{
  XmString dialog_title = XmStringCreateLtoR (title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg (XmNdialogTitle, (XtArgVal)dialog_title);
  XmxSetValues (dialog);

  XmStringFree (dialog_title);

  Xmx_n = 0;
  return;
}

/* ------------------------------ SEPARATORS ------------------------------ */

/* args work */
Widget XmxMakeHorizontalSeparator (Widget parent)
{
  Xmx_w = XmCreateSeparatorGadget (parent, "separator", Xmx_wargs, Xmx_n);
  XtManageChild (Xmx_w);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeHorizontalSpacer (Widget parent)
{
  XmString label = XmStringCreateLtoR (" ", XmSTRING_DEFAULT_CHARSET);

  XmxSetArg (XmNlabelString, (XtArgVal)label);
  Xmx_w = XtCreateManagedWidget ("label", xmLabelGadgetClass, parent, 
                                 Xmx_wargs, Xmx_n);

  XmStringFree (label);

  Xmx_n = 0;
  return Xmx_w;
}

/* args do nothing */
Widget XmxMakeHorizontalBoundary (Widget parent)
{
  /* To avoid confusion, nullify preloaded resources first. */
  Xmx_n = 0;
  XmxMakeHorizontalSpacer (parent);
  XmxMakeHorizontalSeparator (parent);
  XmxMakeHorizontalSpacer (parent);

  Xmx_n = 0;
  return Xmx_w;
}

/* ------------------------- TEXT & SCROLLED TEXT ------------------------- */

/* args work */
Widget XmxMakeScrolledText (Widget parent)
{
  Xmx_w = XmCreateScrolledText (parent, "scrolledtext",
				Xmx_wargs, Xmx_n);
  XtManageChild (Xmx_w);

  /* Remember this returns the Text Widget, NOT the ScrolledWindow Widget, 
     which is what needs to be tied into a form.  Use XtParent to get the
     actual ScrolledWindow. */
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeText (Widget parent)
{
  Xmx_w = XmCreateText (parent, "text", Xmx_wargs, Xmx_n);
  XtManageChild (Xmx_w);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeTextField (Widget parent)
{
  Xmx_w = XmCreateTextField (parent, "textfield", Xmx_wargs, Xmx_n);
  XtManageChild (Xmx_w);

  Xmx_n = 0;
  return Xmx_w;
}

/* args do nothing */
void XmxTextSetString (Widget text, String str)
{
  XmTextSetString (text, str);
  XmTextShowPosition (text, 0);
  return;
}

/* Insert a sting into a text widget -- BJS */
void XmxTextInsertString (Widget text, String str)
{
  XmTextInsert(text, XmTextGetInsertionPosition(text), str);
  XmTextShowPosition (text, 0);
  return;
}

/* args do nothing */
String XmxTextGetString (Widget text)
{
  return XmTextGetString (text);
}

/* args do nothing */
void XmxAddCallbackToText (Widget text, XtCallbackProc cb, int cb_data)
{
  XtAddCallback (text, XmNactivateCallback, cb, 
                 (XtPointer)_XmxMakeClientData (cb_data));
  return;
}


#if 0

/* --------------------------- DRAWING VOLUMES ---------------------------- */

#ifdef __sgi
/* args work */
Widget
XmxMakeDrawingVolume 
  (Widget parent, int width, int height,
   GLXconfig *glxConfig,
   XtCallbackProc redraw_cb, XtCallbackProc resize_cb,
   XtCallbackProc ginit_cb)
{
  XmxSetArg (XmNwidth, (XtArgVal)width);
  XmxSetArg (XmNheight, (XtArgVal)height);
  XmxSetArg (GlxNglxConfig, (XtArgVal)glxConfig);

  Xmx_w = GlxCreateMDraw (parent, "drawingvolume", Xmx_wargs, Xmx_n);
  XtManageChild (Xmx_w);

  XtAddCallback (Xmx_w, GlxNexposeCallback, redraw_cb, 
                 (XtPointer)_XmxMakeClientData (0));
  XtAddCallback (Xmx_w, GlxNresizeCallback, resize_cb, 
                 (XtPointer)_XmxMakeClientData (0));
  XtAddCallback (Xmx_w, GlxNginitCallback, ginit_cb, 
                 (XtPointer)_XmxMakeClientData (0));

  Xmx_n = 0;
  return Xmx_w;
}

void XmxInstallColormaps (Widget toplevel, Widget glw)
{
  Window windows[2];

  windows[0] = XtWindow (glw);
  windows[1] = XtWindow (toplevel);
  XSetWMColormapWindows (XtDisplay (toplevel), XtWindow (toplevel),
			 windows, 2);
  return;
}

void XmxInstallColormapsWithOverlay (Widget toplevel, Widget glw)
{
  Window windows[5];
  Window overlay, popup, underlay;
  Arg args[5];
  register int i=0;
  
  i=0;
  XtSetArg(args[i], GlxNoverlayWindow, &overlay); i++;
  XtSetArg(args[i], GlxNpopupWindow, &popup); i++;
  XtSetArg(args[i], GlxNunderlayWindow, &underlay); i++;
  XtGetValues(glw, args, i);
  i = 0;
  if (overlay)
    {
      windows[i] = overlay;
      i++;
    }
  if (popup)
    {
      windows[i] = popup;
      i++;
    }
  if (underlay)
    {
      windows[i] = underlay;
      i++;
    }
  windows[i] = XtWindow(glw); i++;
  windows[i] = XtWindow(toplevel); i++;
  XSetWMColormapWindows(XtDisplay(toplevel), XtWindow(toplevel), windows, i);
  
  return;
}

void XmxWinset (Widget w)
{
  GLXwinset (XtDisplay (w), XtWindow (w));
  return;
}
#endif /* __sgi */


#ifdef _IBMR2
/* args work */
Widget
XmxMakeDrawingVolume
  (Widget parent, int width, int height,
   XtCallbackProc redraw_cb, XtCallbackProc resize_cb,
   XtCallbackProc ginit_cb)
{
  XmxSetArg (XmNwidth, (XtArgVal)width);
  XmxSetArg (XmNheight, (XtArgVal)height);

  Xmx_w = XtCreateWidget
    ("drawingvolume", glibWidgetClass, parent, Xmx_wargs, Xmx_n);
  XtManageChild (Xmx_w);

  XtAddCallback (Xmx_w, XglNexposeCallback, redraw_cb, 
                 (XtPointer)_XmxMakeClientData (0));
  XtAddCallback (Xmx_w, XglNresizeCallback, resize_cb, 
                 (XtPointer)_XmxMakeClientData (0));
  XtAddCallback (Xmx_w, XglNgconfigCallback, ginit_cb, 
                 (XtPointer)_XmxMakeClientData (0));

  Xmx_n = 0;
  return Xmx_w;
}

void XmxInstallColormaps (Widget toplevel, Widget glw)
{
  return;
}

void XmxWinset (Widget w)
{
  GlWinsetWidget (w);
  return;
}
#endif /* _IBMR2 */

#endif /* if 0 */

/* ----------------------------- BITMAP UTILS ----------------------------- */

/* args ignored and reset */
void XmxApplyBitmapToLabelWidget
  (Widget label, String data, unsigned int width, unsigned int height)
{
  Display *_disp;
  Pixel _fg, _bg;
  Pixmap _pix;

  _disp = XtDisplay (label);

  Xmx_n = 0;
  XmxSetArg (XmNforeground, (XtArgVal)(&_fg));
  XmxSetArg (XmNbackground, (XtArgVal)(&_bg));
  XtGetValues (label, Xmx_wargs, Xmx_n);
  Xmx_n = 0;

  _pix = XCreatePixmapFromBitmapData
    (_disp, DefaultRootWindow (_disp), data, width, height, _fg, _bg,
     DefaultDepthOfScreen (DefaultScreenOfDisplay (_disp)));
  XmxSetArg (XmNlabelPixmap, (XtArgVal)_pix);
  XmxSetArg (XmNlabelType, (XtArgVal)XmPIXMAP);
  XmxSetValues (label);

  Xmx_n = 0;
  return;
}

/* args ignored and reset */
Pixmap XmxCreatePixmapFromBitmap
  (Widget label, String data, unsigned int width, unsigned int height)
{
  Display *_disp;
  Pixel _fg, _bg;
  Pixmap _pix;

  _disp = XtDisplay (label);

  Xmx_n = 0;
  XmxSetArg (XmNforeground, (XtArgVal)(&_fg));
  XmxSetArg (XmNbackground, (XtArgVal)(&_bg));
  XtGetValues (label, Xmx_wargs, Xmx_n);
  Xmx_n = 0;

  _pix = XCreatePixmapFromBitmapData
    (_disp, DefaultRootWindow (_disp), data, width, height, _fg, _bg,
     DefaultDepthOfScreen (DefaultScreenOfDisplay (_disp)));

  return _pix;
}

/* args used */
void XmxApplyPixmapToLabelWidget
  (Widget label, Pixmap pix)
{
  XmxSetArg (XmNlabelPixmap, (XtArgVal)pix);
  XmxSetArg (XmNlabelType, (XtArgVal)XmPIXMAP);
  XmxSetValues (label);

  Xmx_n = 0;
  return;
}

/* ------------------------ DIALOG CONTROL BUTTONS ------------------------ */

/* args apply to form */
Widget XmxMakeFormAndOneButton (Widget parent, XtCallbackProc cb, 
                                String name1, int cb_data1)
{
  Widget _form, _button1;

  XmxSetArg (XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg (XmNfractionBase, (XtArgVal)3);
  _form = XmxMakeForm (parent);

  _button1 = XmxMakePushButton (_form, name1, cb, cb_data1);

  XmxSetConstraints 
    (_button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions (_button1, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetOffsets (_button1, XmxNoOffset, XmxNoOffset, 8, 8);
  
  Xmx_n = 0;
  Xmx_w = _form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndTwoButtons 
  (Widget parent, XtCallbackProc cb,
   String name1, String name2,
   int cb_data1, int cb_data2)
{
  Widget _form, _button1, _button2;

  XmxSetArg (XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg (XmNfractionBase, (XtArgVal)2);
  _form = XmxMakeForm (parent);

  _button1 = XmxMakePushButton (_form, name1, cb, cb_data1);
  _button2 = XmxMakePushButton (_form, name2, cb, cb_data2);

  XmxSetConstraints 
    (_button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints 
    (_button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions (_button1, XmxNoPosition, XmxNoPosition, 0, 1);
  XmxSetPositions (_button2, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetOffsets (_button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets (_button2, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = _form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndTwoButtonsSqueezed
  (Widget parent, XtCallbackProc cb,
   String name1, String name2,
   int cb_data1, int cb_data2)
{
  Widget _form, _button1, _button2;

  XmxSetArg (XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg (XmNfractionBase, (XtArgVal)5);
  _form = XmxMakeForm (parent);

  _button1 = XmxMakePushButton (_form, name1, cb, cb_data1);
  _button2 = XmxMakePushButton (_form, name2, cb, cb_data2);

  XmxSetConstraints 
    (_button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints 
    (_button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions (_button1, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions (_button2, XmxNoPosition, XmxNoPosition, 3, 4);
  XmxSetOffsets (_button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets (_button2, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = _form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndThreeButtonsSqueezed
  (Widget parent, XtCallbackProc cb,
   String name1, String name2, String name3,
   int cb_data1, int cb_data2, int cb_data3)
{
  Widget _form, _button1, _button2, _button3;

  XmxSetArg (XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg (XmNfractionBase, (XtArgVal)7);
  _form = XmxMakeForm (parent);

  _button1 = XmxMakePushButton (_form, name1, cb, cb_data1);
  _button2 = XmxMakePushButton (_form, name2, cb, cb_data2);
  _button3 = XmxMakePushButton (_form, name3, cb, cb_data3);

  XmxSetConstraints 
    (_button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints 
    (_button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints 
    (_button3, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions (_button1, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions (_button2, XmxNoPosition, XmxNoPosition, 3, 4);
  XmxSetPositions (_button3, XmxNoPosition, XmxNoPosition, 5, 6);
  XmxSetOffsets (_button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets (_button2, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets (_button3, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = _form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndThreeButtons
  (Widget parent, XtCallbackProc cb,
   String name1, String name2, String name3,
   int cb_data1, int cb_data2, int cb_data3)
{
  Widget _form, _button1, _button2, _button3;

  XmxSetArg (XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg (XmNfractionBase, (XtArgVal)3);
  _form = XmxMakeForm (parent);

  _button1 = XmxMakePushButton (_form, name1, cb, cb_data1);
  _button2 = XmxMakePushButton (_form, name2, cb, cb_data2);
  _button3 = XmxMakePushButton (_form, name3, cb, cb_data3);

  XmxSetConstraints 
    (_button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints 
    (_button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints 
    (_button3, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions (_button1, XmxNoPosition, XmxNoPosition, 0, 1);
  XmxSetPositions (_button2, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions (_button3, XmxNoPosition, XmxNoPosition, 2, 3);
  XmxSetOffsets (_button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets (_button2, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets (_button3, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = _form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndFourButtons 
  (Widget parent, XtCallbackProc cb,
   String name1, String name2, String name3, String name4,
   int cb_data1, int cb_data2, int cb_data3, int cb_data4)
{
  Widget _form, _button1, _button2, _button3, _button4;

  XmxSetArg (XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg (XmNfractionBase, (XtArgVal)4);
  _form = XmxMakeForm (parent);

  _button1 = XmxMakePushButton (_form, name1, cb, cb_data1);
  _button2 = XmxMakePushButton (_form, name2, cb, cb_data2);
  _button3 = XmxMakePushButton (_form, name3, cb, cb_data3);
  _button4 = XmxMakePushButton (_form, name4, cb, cb_data4);

  XmxSetConstraints 
    (_button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints 
    (_button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints 
    (_button3, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints 
    (_button4, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions (_button1, XmxNoPosition, XmxNoPosition, 0, 1);
  XmxSetPositions (_button2, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions (_button3, XmxNoPosition, XmxNoPosition, 2, 3);
  XmxSetPositions (_button4, XmxNoPosition, XmxNoPosition, 3, 4);
  XmxSetOffsets (_button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets (_button2, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets (_button3, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets (_button4, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = _form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndFiveButtons 
  (Widget parent, XtCallbackProc cb,
   String name1, String name2, String name3, String name4, String name5,
   int cb_data1, int cb_data2, int cb_data3, int cb_data4, int cb_data5)
{
  Widget _form, _button1, _button2, _button3, _button4, _button5;

  XmxSetArg (XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg (XmNfractionBase, (XtArgVal)5);
  _form = XmxMakeForm (parent);

  _button1 = XmxMakePushButton (_form, name1, cb, cb_data1);
  _button2 = XmxMakePushButton (_form, name2, cb, cb_data2);
  _button3 = XmxMakePushButton (_form, name3, cb, cb_data3);
  _button4 = XmxMakePushButton (_form, name4, cb, cb_data4);
  _button5 = XmxMakePushButton (_form, name5, cb, cb_data5);

  XmxSetConstraints
    (_button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (_button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (_button3, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (_button4, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (_button5, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions (_button1, XmxNoPosition, XmxNoPosition, 0, 1);
  XmxSetPositions (_button2, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions (_button3, XmxNoPosition, XmxNoPosition, 2, 3);
  XmxSetPositions (_button4, XmxNoPosition, XmxNoPosition, 3, 4);
  XmxSetPositions (_button5, XmxNoPosition, XmxNoPosition, 4, 5);
  XmxSetOffsets (_button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets (_button2, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets (_button3, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets (_button4, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets (_button5, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = _form;
  return Xmx_w;
}

static void _XmxYesOrNoResponse (Widget w, int *answer, XmAnyCallbackStruct *cbs)
{
  if (cbs->reason == XmCR_OK)
    *answer = 1;
  else if (cbs->reason == XmCR_CANCEL)
    *answer = 0;
}

int XmxModalYesOrNo (Widget parent, XtAppContext app, 
                     char *questionstr, char *yesstr, char *nostr)
{
  Widget dialog;
  XmString question, yes, no, title;
  int answer = -1;

/*  startbouncetimer();*/

  question = XmStringCreateLtoR (questionstr, XmSTRING_DEFAULT_CHARSET);
  yes = XmStringCreateLtoR (yesstr, XmSTRING_DEFAULT_CHARSET);
  no = XmStringCreateLtoR (nostr, XmSTRING_DEFAULT_CHARSET);
  title = XmStringCreateLtoR ("Prompt", XmSTRING_DEFAULT_CHARSET);
  
  XmxSetArg (XmNdialogTitle, (XtArgVal)title);
  XmxSetArg (XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
  XmxSetArg (XmNmessageString, (XtArgVal)question);
  XmxSetArg (XmNokLabelString, (XtArgVal)yes);
  XmxSetArg (XmNcancelLabelString, (XtArgVal)no);
  XmxSetArg (XmNsymbolPixmap, (XtArgVal)dialogQuestion);

  dialog = XmCreateQuestionDialog (parent, "question_dialog", Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  
  XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_HELP_BUTTON));
  XtAddCallback (dialog, XmNokCallback, 
                 (XtCallbackProc)_XmxYesOrNoResponse, &answer);
  XtAddCallback (dialog, XmNcancelCallback, 
                 (XtCallbackProc)_XmxYesOrNoResponse, &answer);

  XtManageChild (dialog);

  while (answer == -1)
    {
      XtAppProcessEvent (app, XtIMAll);
      XSync (XtDisplay (dialog), 0);
    }

  XtUnmanageChild (dialog);
  XSync (XtDisplay (dialog), 0);
  XmUpdateDisplay (dialog);

  XmStringFree (question);
  XmStringFree (yes);
  XmStringFree (no);
  XmStringFree (title);

  XtDestroyWidget (dialog);

/*  stopbouncetimer();*/

  return answer;
}

/* ------------------------------------------------------------------------ */

#define XMX_NO_ANSWER "-*-no answer, dammit, but Xmx rules, yo yo yo-*-"

/*SWP -- 7/6/95*/
static void _XmxActivate (Widget w, char **answer, 
                                         XmSelectionBoxCallbackStruct *cbs)
{
    *answer = XMX_NO_ANSWER;
}

static void _XmxPromptForStringResponse (Widget w, char **answer, 
                                         XmSelectionBoxCallbackStruct *cbs)
{
  if (!XmStringGetLtoR (cbs->value, XmSTRING_DEFAULT_CHARSET, answer))
    *answer = XMX_NO_ANSWER;
}

static void _XmxPromptForStringCancel (Widget w, char **answer, 
                                       XmSelectionBoxCallbackStruct *cbs)
{
  *answer = XMX_NO_ANSWER;
}


/*SWP -- 7/4/95*/
void XmxMakeInfoDialogWait (Widget parent, XtAppContext app, 
                               char *infostr, char *titlestr, char *yesstr)
{
  Widget dialog;
  XmString info, yes, title;
  char *answer = NULL;

  info = XmStringCreateLtoR (infostr, XmSTRING_DEFAULT_CHARSET);
  yes = XmStringCreateLtoR (yesstr, XmSTRING_DEFAULT_CHARSET);
  title = XmStringCreateLtoR (titlestr, XmSTRING_DEFAULT_CHARSET);
  
  XmxSetArg (XmNdialogTitle, (XtArgVal)title);
  XmxSetArg (XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
  XmxSetArg (XmNmessageString, (XtArgVal)info);
  XmxSetArg (XmNokLabelString, (XtArgVal)yes);
  XmxSetArg (XmNsymbolPixmap, (XtArgVal)dialogInformation);

  dialog = XmCreateInformationDialog (parent, "information_dialog", Xmx_wargs, Xmx_n);
  Xmx_n = 0;

  XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_HELP_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_CANCEL_BUTTON));

  XtAddCallback (dialog, XmNokCallback, 
                 (XtCallbackProc)_XmxActivate, &answer);

  XtManageChild (dialog);

  while (answer == NULL)
    {
      XtAppProcessEvent (app, XtIMAll);
      XSync (XtDisplay (dialog), 0);
    }

  XtUnmanageChild (dialog);
  XSync (XtDisplay (dialog), 0);
  XmUpdateDisplay (dialog);

  XmStringFree (info);
  XmStringFree (yes);
  XmStringFree (title);

  XtDestroyWidget (dialog);

  return;
}


/*SWP -- 4/15/96*/
void XmxMakeErrorDialogWait (Widget parent, XtAppContext app, 
                               char *infostr, char *titlestr, char *yesstr)
{
  Widget dialog;
  XmString info, yes, title;
  char *answer = NULL;

  info = XmStringCreateLtoR (infostr, XmSTRING_DEFAULT_CHARSET);
  yes = XmStringCreateLtoR (yesstr, XmSTRING_DEFAULT_CHARSET);
  title = XmStringCreateLtoR (titlestr, XmSTRING_DEFAULT_CHARSET);
  
  XmxSetArg (XmNdialogTitle, (XtArgVal)title);
  XmxSetArg (XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
  XmxSetArg (XmNmessageString, (XtArgVal)info);
  XmxSetArg (XmNokLabelString, (XtArgVal)yes);
  XmxSetArg (XmNsymbolPixmap, (XtArgVal)dialogError);

  dialog = XmCreateInformationDialog (parent, "information_dialog", Xmx_wargs, Xmx_n);
  Xmx_n = 0;

  XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_HELP_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_CANCEL_BUTTON));

  XtAddCallback (dialog, XmNokCallback, 
                 (XtCallbackProc)_XmxActivate, &answer);

  XtManageChild (dialog);

  while (answer == NULL)
    {
      XtAppProcessEvent (app, XtIMAll);
      XSync (XtDisplay (dialog), 0);
    }

  XtUnmanageChild (dialog);
  XSync (XtDisplay (dialog), 0);
  XmUpdateDisplay (dialog);

  XmStringFree (info);
  XmStringFree (yes);
  XmStringFree (title);

  XtDestroyWidget (dialog);

  return;
}


char *XmxModalPromptForString (Widget parent, XtAppContext app, 
                               char *questionstr, char *yesstr, char *nostr)
{
  Widget dialog;
  XmString question, yes, no, title;
  char *answer = NULL;

/*  startbouncetimer();*/

  question = XmStringCreateLtoR (questionstr, XmSTRING_DEFAULT_CHARSET);
  yes = XmStringCreateLtoR (yesstr, XmSTRING_DEFAULT_CHARSET);
  no = XmStringCreateLtoR (nostr, XmSTRING_DEFAULT_CHARSET);
  title = XmStringCreateLtoR ("Prompt", XmSTRING_DEFAULT_CHARSET);
  
  XmxSetArg (XmNdialogTitle, (XtArgVal)title);
  XmxSetArg (XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
  XmxSetArg (XmNselectionLabelString, (XtArgVal)question);
  XmxSetArg (XmNokLabelString, (XtArgVal)yes);
  XmxSetArg (XmNcancelLabelString, (XtArgVal)no);
  dialog = XmCreatePromptDialog (parent, "question_dialog", Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  
  XtUnmanageChild (XmSelectionBoxGetChild (dialog, XmDIALOG_HELP_BUTTON));
  XtAddCallback (dialog, XmNokCallback, 
                 (XtCallbackProc)_XmxPromptForStringResponse, &answer);
  XtAddCallback (dialog, XmNcancelCallback, 
                 (XtCallbackProc)_XmxPromptForStringCancel, &answer);

  XtManageChild (dialog);

  while (answer == NULL)
    {
      XtAppProcessEvent (app, XtIMAll);
      XSync (XtDisplay (dialog), 0);
    }

  XtUnmanageChild (dialog);
  XSync (XtDisplay (dialog), 0);
  XmUpdateDisplay (dialog);

  XmStringFree (question);
  XmStringFree (yes);
  XmStringFree (no);
  XmStringFree (title);

  XtDestroyWidget (dialog);

/*  stopbouncetimer();*/

  if (!answer || strcmp (answer, XMX_NO_ANSWER) == 0)
    return NULL;
  else
    return answer;
}


/* ------------------------------------------------------------------------ */

#define XMX_NO_ANSWER "-*-no answer, dammit, but Xmx rules, yo yo yo-*-"

static char *_passwd = NULL;

static void _XmxPromptForPasswordResponse (Widget w, char **answer, 
                                           XmSelectionBoxCallbackStruct *cbs)
{
  if (!XmStringGetLtoR (cbs->value, XmSTRING_DEFAULT_CHARSET, answer))
    *answer = XMX_NO_ANSWER;
}

static void _XmxPromptForPasswordCancel (Widget w, char **answer, 
                                         XmSelectionBoxCallbackStruct *cbs)
{
  *answer = XMX_NO_ANSWER;
}

static void _XmxPromptForPasswordVerify (Widget text_w, XtPointer unused, 
                                         XmTextVerifyCallbackStruct *cbs)
{
  char *new;
  int len;
  
  if (cbs->reason != XmCR_MODIFYING_TEXT_VALUE) 
    {
      return;
    }

  if (cbs->text->ptr == NULL) 
    { /* backspace */
      cbs->doit = True;
      if (_passwd && *_passwd)
        {
	  int start;
	  char *tptr;
	
	  len = strlen(_passwd);
	  /* Find the start of the delete */
	  if (cbs->startPos < len)
	    {
	      start = cbs->startPos;
	    }
	  else
	    {
	      start = len - 1;
	    }
	  /* Move up stuff after the delete */
	  if (cbs->endPos > len)
	    {
	      tptr = &(_passwd[len]);
	    }
	  else
	    {
	      tptr = &(_passwd[cbs->endPos]);
	    }
	  _passwd[start] ='\0';
	  strcat(_passwd, tptr);
        }
    }
  else if (cbs->text->length >= 1) 
    {
      int i;

      if (_passwd == NULL)
	{
	  _passwd = XtMalloc (cbs->text->length + 1);
	  strncpy(_passwd, cbs->text->ptr, cbs->text->length);
	  _passwd[cbs->text->length] = '\0';
	}
      else
	{
	  char *tptr;
	  char tchar;
	  int start;

	  len = strlen(_passwd);
	  /* Find the start of the delete */
	  if (cbs->startPos < len)
	    {
	      start = cbs->startPos;
	    }
	  else
	    {
	      start = len;
	    }
	  tptr = &(_passwd[start]);
	  tchar = *tptr;
	  *tptr = '\0';
	  new = XtMalloc (len + cbs->text->length + 1);
	  strcpy(new, _passwd);
	  strncat(new, cbs->text->ptr, cbs->text->length);
	  new[start + cbs->text->length] = '\0';
	  *tptr = tchar;
	  strcat(new, tptr);
	  XtFree(_passwd);
	  _passwd = new;
	}
      cbs->doit = True;
      /*  make a '*' show up instead of what they typed */
      for (i=0; i < cbs->text->length; i++)
      {
	cbs->text->ptr[i] = '*';
      }
    }
}

char *XmxModalPromptForPassword (Widget parent, XtAppContext app, 
                               char *questionstr, char *yesstr, char *nostr)
{
  Widget dialog;
  XmString question, yes, no, title;
  char *answer = NULL;

/*  startbouncetimer();*/

  _passwd = NULL;

  question = XmStringCreateLtoR (questionstr, XmSTRING_DEFAULT_CHARSET);
  yes = XmStringCreateLtoR (yesstr, XmSTRING_DEFAULT_CHARSET);
  no = XmStringCreateLtoR (nostr, XmSTRING_DEFAULT_CHARSET);
  title = XmStringCreateLtoR ("Prompt", XmSTRING_DEFAULT_CHARSET);
  
  XmxSetArg (XmNdialogTitle, (XtArgVal)title);
  XmxSetArg (XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
  XmxSetArg (XmNselectionLabelString, (XtArgVal)question);
  XmxSetArg (XmNokLabelString, (XtArgVal)yes);
  XmxSetArg (XmNcancelLabelString, (XtArgVal)no);
  dialog = XmCreatePromptDialog (parent, "question_dialog", Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  
  XtUnmanageChild (XmSelectionBoxGetChild (dialog, XmDIALOG_HELP_BUTTON));
  XtAddCallback
    (XmSelectionBoxGetChild (dialog, XmDIALOG_TEXT),
     XmNmodifyVerifyCallback, (XtCallbackProc)_XmxPromptForPasswordVerify, 0);
  XtAddCallback (dialog, XmNokCallback, 
                 (XtCallbackProc)_XmxPromptForPasswordResponse, &answer);
  XtAddCallback (dialog, XmNcancelCallback, 
                 (XtCallbackProc)_XmxPromptForPasswordCancel, &answer);

  XtManageChild (dialog);

  while (answer == NULL)
    {
      XtAppProcessEvent (app, XtIMAll);
      XSync (XtDisplay (dialog), 0);
    }

  XtUnmanageChild (dialog);
  XSync (XtDisplay (dialog), 0);
  XmUpdateDisplay (dialog);

  XmStringFree (question);
  XmStringFree (yes);
  XmStringFree (no);
  XmStringFree (title);

  XtDestroyWidget (dialog);

/*  stopbouncetimer();*/

  if (!answer || strcmp (answer, XMX_NO_ANSWER) == 0 ||
      !_passwd || !(*_passwd))
    return NULL;
  else
    return strdup (_passwd);
}

