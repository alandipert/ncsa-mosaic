/*	Displaying messages and getting input for LineMode Browser
**	==========================================================
**
**	REPLACE THIS MODULE with a GUI version in a GUI environment!
**
** History:
**	   Jun 92 Created May 1992 By C.T. Barker
**	   Feb 93 Simplified, portablised (ha!) TBL
**
*/

#include "../config.h"
#include "HTAlert.h"

#include "tcp.h"		/* for TOUPPER */
#include <ctype.h> 		/* for toupper - should be in tcp.h */

extern void mo_gui_notify_progress (char *);
extern int mo_gui_check_icon (int);
extern void mo_gui_clear_icon (void);
extern void mo_gui_update_meter(int,char *);

PUBLIC void HTAlert ARGS1(WWW_CONST char *, Msg)
{
  mo_gui_notify_progress (Msg);
  return;
}

PUBLIC void HTProgress ARGS1(WWW_CONST char *, Msg)
{
  mo_gui_notify_progress (Msg);
  return;
}

PUBLIC void HTMeter ARGS2(WWW_CONST int, level, WWW_CONST char *, text)
{
    mo_gui_update_meter(level,text);
  return;
}

PUBLIC int HTCheckActiveIcon ARGS1(int, twirl)
{
  int ret;

  ret = mo_gui_check_icon (twirl);
  return(ret);
}

PUBLIC void HTClearActiveIcon NOARGS
{
  mo_gui_clear_icon ();
  return;
}

PUBLIC void HTDoneWithIcon NOARGS
{
  mo_gui_done_with_icon ();
  return;
}

PUBLIC BOOL HTConfirm ARGS1(WWW_CONST char *, Msg)
{
  extern int prompt_for_yes_or_no (char *);

  if (prompt_for_yes_or_no (Msg))
    return(YES);
  else
    return(NO);
}

PUBLIC char * HTPrompt ARGS2(WWW_CONST char *, Msg, WWW_CONST char *, deflt)
{
  extern char *prompt_for_string (char *);
  char *Tmp = prompt_for_string (Msg);
  char *rep = 0;

  StrAllocCopy (rep, (Tmp && *Tmp) ? Tmp : deflt);
  return rep;
}

PUBLIC char * HTPromptPassword ARGS1(WWW_CONST char *, Msg)
{
  extern char *prompt_for_password (char *);
  char *Tmp = prompt_for_password (Msg);

  return Tmp;
}
