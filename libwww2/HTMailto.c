/*			MAILTO WINDOW				HTMailTo.c
 **			=============
 ** Authors:
 **  Mike Peter Bretz (bretz@zdv.uni-tuebingen.de)
 **  Alan Braverman (alanb@ncsa.uiuc.edu)
 **
 ** History:
 **	07 Jul 94   First version  (MPB)
 **     07 Mar 95   Stuck it in NCSA Mosaic for X 2.6 (AMB)
 */
#include "../config.h"
#include "HTAccess.h"
#include "HTUtils.h"
#include "tcp.h"
#include "HTML.h"
#include "HTParse.h"
#include "HTFormat.h"
#include "../libnut/str-tools.h"
#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

struct _HTStructured 
{
  WWW_CONST HTStructuredClass *	isa;
  /* ... */
};


/*	Module-wide variables
 */
PRIVATE int s;                                  /* Socket for FingerHost */
PRIVATE HTStructured * target;			/* The output sink */
PRIVATE HTStructuredClass targetClass;		/* Copy of fn addresses */

extern int GetMailtoKludgeInfo();


/*	Initialisation for this module
 **	------------------------------
 */
PRIVATE BOOL initialized = NO;
PRIVATE BOOL initialize NOARGS
{
  s = -1;			/* Disconnected */
  return YES;
}


PUBLIC int HTSendMailTo ARGS4(
      WWW_CONST char *,     arg,
      HTParentAnchor *,	anAnchor,
      HTFormat,		format_out,
      HTStream*,	stream)
{
  char *mailtoURL;
  char *mailtoSubject;

#ifndef DISABLE_TRACE
  if (www2Trace) 
    fprintf(stderr, "HTMailto: Mailing to %s\n", arg);
#endif
  
  if (!initialized) 
    initialized = initialize();
  if (!initialized) 
    {
      HTProgress ((char *) 0);
      return HT_NOT_LOADED;
    }

  {
    WWW_CONST char * p1=arg;
    
    /*	We will ask for the document, omitting the host name & anchor.
     **
     **	Syntax of address is
     **		xxx@yyy			User xxx at site yyy (xxx is optional).
     */        
    if (!my_strncasecmp (arg, "mailto:", 7))
      p1 = arg + 7;		/* Skip "mailto:" prefix */
    
    if (!*arg) 
      {
	HTProgress ("Could not find email address");
	return HT_NOT_LOADED;	/* Ignore if no name */
      }

    GetMailtoKludgeInfo(&mailtoURL,&mailtoSubject);
    (void) mo_post_mailto_win(p1,mailtoSubject);
    return HT_LOADED;
  }
}

PUBLIC HTProtocol HTMailto = { "mailto", HTSendMailTo, NULL };
