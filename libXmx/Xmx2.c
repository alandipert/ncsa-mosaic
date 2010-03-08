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

/* ------------------------------------------------------------------------ */
/* --------------------------- PRIVATE ROUTINES --------------------------- */
/* ------------------------------------------------------------------------ */

/* ----------------------- _XmxMenuAddEntryToRecord ----------------------- */

/* Create a new MenuEntry and add it to the head of a MenuRecord list. */
private void 
_XmxMenuAddEntryToRecord (XmxMenuRecord *rec, Widget w, int token)
{
  XmxMenuEntry *_ent;

  /* Create new menu entry. */
  _ent = (XmxMenuEntry *)malloc (sizeof (XmxMenuEntry));
  _ent->w = w;
  _ent->token = token;

  /* Add rest of list to tail of this entry. */
  _ent->next = rec->first_entry;

  /* Make this entry head of list. */
  rec->first_entry = _ent;

  return;
}

/* ---------------------- _XmxMenuGetEntryFromRecord ---------------------- */

private XmxMenuEntry *
_XmxMenuGetEntryFromRecord (XmxMenuRecord *rec, int token)
{
  /* Given token, fetch the corresponding entry. */
  XmxMenuEntry *_ent = NULL;
  int _done;

  /* Search the linked list. */
  _ent = rec->first_entry;
  _done = 0;
  while (_ent != NULL && !_done)
    {
      if (_ent->token == token)
        _done = 1;
      else
        _ent = _ent->next;
    }

  /* Punish the application for asking for a nonexistent entry. */
  /* assert (_done); */
  return _ent;
}

/* ------------------------- _XmxMenuCreateRecord ------------------------- */

/* Create a new MenuRecord and clear out its list. */
private XmxMenuRecord *
_XmxMenuCreateRecord (Widget base)
{
  XmxMenuRecord *_rec;

  /* Create the new XmxMenuRecord. */
  _rec = (XmxMenuRecord *)malloc (sizeof (XmxMenuRecord));
  _rec->base = base;
  _rec->first_entry = NULL;

  return _rec;
}

/* ------------------------------------------------------------------------ */
/* --------------------------- PUBLIC ROUTINES ---------------------------- */
/* ------------------------------------------------------------------------ */

/* --------------------------- XmxRSetSensitive --------------------------- */

/* args NOT used on Widget */
public void
XmxRSetSensitive (XmxMenuRecord *rec, int token, int state)
{
  XmxMenuEntry *_entry;

  assert (state == XmxSensitive || state == XmxUnsensitive);
  _entry = _XmxMenuGetEntryFromRecord (rec, XmxExtractToken (token));
  /* XtSetSensitive propagates down Widget hierarchy. */
  if (_entry)
    XtSetSensitive (_entry->w, (state == XmxSensitive) ? True : False);
  
  return;
}

/* -------------------------- XmxRSetToggleState -------------------------- */

/* args not used */
public void
XmxRSetToggleState (XmxMenuRecord *rec, int token, int state)
{
  XmxMenuEntry *_entry;

  assert (state == XmxSet || state == XmxUnset);
  _entry = _XmxMenuGetEntryFromRecord (rec, XmxExtractToken (token));
  if (_entry)
    XmToggleButtonGadgetSetState 
      (_entry->w, (state == XmxSet) ? True : False, False);
  
  return;
}

/* ------------------------- XmxRUnsetAllToggles -------------------------- */

/* args not used */
public void
XmxRUnsetAllToggles (XmxMenuRecord *rec)
{
  XmxMenuEntry *_ent;

  for (_ent = rec->first_entry; _ent != NULL; _ent = _ent->next)
    XmToggleButtonGadgetSetState
      (_ent->w, False, False);

  return;
}

/* ----------------------- XmxRSetOptionMenuHistory ----------------------- */

/* args used on Widget */
public void
XmxRSetOptionMenuHistory (XmxMenuRecord *rec, int token)
{
  XmxMenuEntry *_entry;

  _entry = _XmxMenuGetEntryFromRecord (rec, XmxExtractToken (token));
  if (_entry)
    {
      XmxSetArg (XmNmenuHistory, (XtArgVal)(_entry->w));
      XtSetValues (rec->base, Xmx_wargs, Xmx_n);
    }

  Xmx_n = 0;
  return;
}

/* ---------------------------- XmxRSetValues ----------------------------- */

/* args used on Widget */
public void
XmxRSetValues (XmxMenuRecord *rec, int token)
{
  XmxMenuEntry *_entry;

  /* Strip out uniqid, if present. */
  _entry = _XmxMenuGetEntryFromRecord (rec, XmxExtractToken (token));
  if (_entry)
    XtSetValues (_entry->w, Xmx_wargs, Xmx_n);

  Xmx_n = 0;
  return;
}

/* ---------------------------- XmxRGetWidget ----------------------------- */

/* args irrelevant */
public Widget
XmxRGetWidget (XmxMenuRecord *rec, int token)
{
  XmxMenuEntry *_entry;

  /* Strip out uniqid, if present. */
  _entry = _XmxMenuGetEntryFromRecord (rec, XmxExtractToken (token));

  if (_entry)
    return _entry->w;
  else
    return NULL;
}

/* -------------------------- XmxRMakeOptionMenu -------------------------- */

/* args apply to pulldown menu */
public XmxMenuRecord *
XmxRMakeOptionMenu (Widget parent, String name, XtCallbackProc cb,
                    XmxOptionMenuStruct *opts)
{
  XmxMenuRecord *_rec;
  Widget _pulldown, _button, _menuhist = (Widget)NULL;
  int _i;

  /* Create a pulldown menupane to attach to the option menu;
     preloaded wargs affect this. */
  _pulldown = XmCreatePulldownMenu (parent, "pulldownmenu", Xmx_wargs, Xmx_n);

  /* menuHistory will not be applied to _pulldown, so we'll modify
     _rec directly after creating the option menu. */
  _rec = _XmxMenuCreateRecord (_pulldown);

  /* Create pushbutton gadgets as childen of the pulldown menu. */
  _i = 0;
  while (opts[_i].namestr)
    {
      Xmx_n = 0;
      XmxSetArg (XmNlabelString,
                 (XtArgVal)XmStringCreateLtoR (opts[_i].namestr,
                                               XmSTRING_DEFAULT_CHARSET));
      _button = XmCreatePushButtonGadget (_pulldown, "pushbutton",
                                          Xmx_wargs, Xmx_n);
      XtManageChild (_button);
      XtAddCallback (_button, XmNactivateCallback, cb,
                     (XtPointer)_XmxMakeClientData (opts[_i].data));
      if (opts[_i].set_state == XmxSet)
        _menuhist = _button;

      _XmxMenuAddEntryToRecord (_rec, _button, opts[_i].data);

      _i++;
    }

  /* Create the option menu itself; tie in the pulldown menu. */
  Xmx_n = 0;
  XmxSetArg (XmNsubMenuId, (XtArgVal)_pulldown);
  if (_menuhist != (Widget)NULL)
    XmxSetArg (XmNmenuHistory, (XtArgVal)_menuhist);
  Xmx_w = XmCreateOptionMenu (parent, "optionmenu", Xmx_wargs, Xmx_n);
  XtManageChild (Xmx_w);

  XmxSetArg (XmNalignment, (XtArgVal)XmALIGNMENT_BEGINNING);
  XmxSetValues (XmOptionButtonGadget (Xmx_w));

  if (name)
    {
      XmxSetArg (XmNlabelString,
                 (XtArgVal)XmStringCreateLtoR (name, XmSTRING_DEFAULT_CHARSET));
      XmxSetValues (XmOptionLabelGadget (Xmx_w));
    }
  else
    {
      XmxSetArg (XmNspacing, (XtArgVal)0);
      XmxSetArg (XmNmarginWidth, (XtArgVal)0);
      XmxSetValues (Xmx_w);
      XmxSetArg (XmNlabelString, (XtArgVal)NULL);
      XmxSetValues (XmOptionLabelGadget (Xmx_w));
    }

  /* Explicitly set base Widget of record. */
  _rec->base = Xmx_w;

  Xmx_n = 0;
  return _rec;
}

/* -------------------------- XmxRMakeToggleMenu -------------------------- */

/* args apply to radiobox or optionbox */
public XmxMenuRecord *
XmxRMakeToggleMenu (Widget parent, int behavior, XtCallbackProc cb,
                    XmxToggleMenuStruct *opts)
{
  XmxMenuRecord *_rec;
  Widget _box;
  int _i;

  assert (behavior == XmxOneOfMany || behavior == XmxNOfMany);
  switch (behavior)
    {
    case XmxOneOfMany:
      _box = XmxMakeRadioBox (parent);
      break;
    case XmxNOfMany:
      _box = XmxMakeOptionBox (parent);
      break;
    }

  _rec = _XmxMenuCreateRecord (_box);

  _i = 0;
  while (opts[_i].namestr)
    {
      XmxMakeToggleButton (_box, opts[_i].namestr, cb, opts[_i].data);
      XmxSetToggleButton (Xmx_w, opts[_i].set_state);
      _XmxMenuAddEntryToRecord (_rec, Xmx_w, opts[_i].data);

      _i++;
    }

  Xmx_w = _box;

  Xmx_n = 0;
  return _rec;
}

/* -------------------------- _XmxRCreateMenubar -------------------------- */

/* Possible deficiency: will not be able to grey out a submenu
   (cascade button). */
private void 
_XmxRCreateMenubar (Widget menu, XmxMenubarStruct *menulist, 
                    XmxMenuRecord *rec)
{
  int _i;
  Widget *_buttons;
  int _separators = 0, _nitems;

  _nitems = 0;
  while (menulist[_nitems].namestr)
    _nitems++;

  _buttons = (Widget *)XtMalloc (_nitems * sizeof (Widget));

  for (_i = 0; _i < _nitems; _i++)
    {
      /* Name of "----" means make a separator. */
      if (strcmp(menulist[_i].namestr, "----") == 0)
        {
          XtCreateManagedWidget ("separator", xmSeparatorGadgetClass,
                                 menu, NULL, 0);
          _separators++;
        }
      /* A function means it's an ordinary entry with callback. */
      else if (menulist[_i].func)
        {
          Xmx_n = 0;
          if (menulist[_i].mnemonic)
            XmxSetArg (XmNmnemonic, (XtArgVal)(menulist[_i].mnemonic));
          if (menulist[_i].namestr[0] == '#' ||
              menulist[_i].namestr[0] == '<') /* option/toggle button */
            {
              XmString xmstr;

              /* A toggle button is diamond-shaped. */
              if (menulist[_i].namestr[0] == '<')
                XmxSetArg (XmNindicatorType, (XtArgVal)XmONE_OF_MANY);

	      /* Make sure the button shows up even when toggled off. */
              if (menulist[_i].namestr[0] == '#')
                XmxSetArg (XmNvisibleWhenOff, (XtArgVal)True);

              /* Ignore first character of label. */
              xmstr = XmxMakeXmstrFromString (&(menulist[_i].namestr[1]));
              XmxSetArg (XmNlabelString, (XtArgVal)xmstr);
              _buttons[_i - _separators] = XtCreateManagedWidget
                ("togglebutton", xmToggleButtonGadgetClass,
                 menu, Xmx_wargs, Xmx_n);
              XmStringFree (xmstr);

              XtAddCallback
                (_buttons[_i - _separators], XmNvalueChangedCallback,
                 menulist[_i].func, 
                 (XtPointer)_XmxMakeClientData (menulist[_i].data));

              /* Add thie button to the menu record. */
              _XmxMenuAddEntryToRecord 
                (rec, _buttons[_i - _separators], menulist[_i].data);
            }
          else /* regular button */
            {
              XmString xmstr = 
                XmStringCreateLtoR
                  (menulist[_i].namestr, XmSTRING_DEFAULT_CHARSET);
              XmxSetArg (XmNlabelString, (XtArgVal)xmstr);
              _buttons[_i - _separators] = XtCreateManagedWidget
                ("pushbutton", xmPushButtonGadgetClass,
                 menu, Xmx_wargs, Xmx_n);
              XmStringFree (xmstr);
              XtAddCallback 
                (_buttons[_i - _separators], XmNactivateCallback,
                 menulist[_i].func, 
                 (XtPointer)_XmxMakeClientData (menulist[_i].data));

              /* Add thie button to the menu record. */
              _XmxMenuAddEntryToRecord 
                (rec, _buttons[_i - _separators], menulist[_i].data);
            }
        }

      /* No function and no submenu entry means it's just a label. */
      else if (menulist[_i].sub_menu == (XmxMenubarStruct *)NULL)
        {
          Xmx_n = 0;
          XmxSetArg (XmNlabelString, (XtArgVal)XmStringCreateLtoR
                     (menulist[_i].namestr, XmSTRING_DEFAULT_CHARSET));
          _buttons[_i - _separators] = XtCreateManagedWidget
            ("label", xmLabelGadgetClass, menu, Xmx_wargs, Xmx_n);
        }

      /* If all else fails, it's a submenu. */
      else
        {
          XmString xmstr;
          Widget _sub_menu;
          _sub_menu = XmCreatePulldownMenu (menu, "pulldownmenu", NULL, 0);

          Xmx_n = 0;
          XmxSetArg (XmNsubMenuId, (XtArgVal)_sub_menu);
          if (menulist[_i].mnemonic)
            XmxSetArg (XmNmnemonic, (XtArgVal)(menulist[_i].mnemonic));
          xmstr = XmStringCreateLtoR
            (menulist[_i].namestr, XmSTRING_DEFAULT_CHARSET);
          XmxSetArg (XmNlabelString, (XtArgVal)xmstr);
          _buttons[_i - _separators] = XtCreateWidget
            ("cascadebutton", xmCascadeButtonGadgetClass,
             menu, Xmx_wargs, Xmx_n);
          XmStringFree (xmstr);

          /* If name is "Help", put on far right. */
          if (strcmp (menulist[_i].namestr, "Help") == 0)
            {
              Xmx_n = 0;
              XmxSetArg (XmNmenuHelpWidget, (XtArgVal)_buttons[_i - _separators]);
              XtSetValues (menu, Xmx_wargs, Xmx_n);
            }
          /* Recursively create new submenu. */
          _XmxRCreateMenubar (_sub_menu, menulist[_i].sub_menu, rec);
        }
    }

  XtManageChildren (_buttons, _nitems - _separators);

  XtFree ((char *)_buttons);

  return;
}

/* --------------------------- XmxRMakeMenubar ---------------------------- */

/* args apply to menubar */
public XmxMenuRecord *
XmxRMakeMenubar (Widget parent, XmxMenubarStruct *mainmenu)
{
  Widget _menubar;
  XmxMenuRecord *_rec;

  /* Preset resources applied to main menubar only. */
  _menubar = XmCreateMenuBar (parent, "menubar", Xmx_wargs, Xmx_n);
  XtManageChild (_menubar);

  /* Create the new XmxMenuRecord. */
  _rec = _XmxMenuCreateRecord (_menubar);

  Xmx_n = 0;
  _XmxRCreateMenubar (_menubar, mainmenu, _rec);

  Xmx_n = 0;
  Xmx_w = _menubar;
  return _rec;
}
