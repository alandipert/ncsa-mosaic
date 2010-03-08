/*                                                                 String handling for libwww
                                         STRINGS
                                             
   Case-independent string comparison and allocations with copies etc
   
 */
#ifndef HTSTRING_H
#define HTSTRING_H

#include "HTUtils.h"

extern int WWW_TraceFlag;       /* Global flag for all W3 trace */

extern WWW_CONST char * HTLibraryVersion;   /* String for help screen etc */

/*

Case-insensitive string comparison

   The usual routines (comp instead of cmp) had some problem.
   
 */
extern int strcasecomp  PARAMS((WWW_CONST char *a, WWW_CONST char *b));
extern int strncasecomp PARAMS((WWW_CONST char *a, WWW_CONST char *b, int n));

/*

Malloced string manipulation

 */
#define StrAllocCopy(dest, src) HTSACopy (&(dest), src)
#define StrAllocCat(dest, src)  HTSACat  (&(dest), src)
extern char * HTSACopy PARAMS ((char **dest, WWW_CONST char *src));
extern char * HTSACat  PARAMS ((char **dest, WWW_CONST char *src));

/*

Next word or quoted string

 */
extern char * HTNextField PARAMS ((char** pstr));


#endif
/*

   end
   
    */
