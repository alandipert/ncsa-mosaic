/*                                                    Utitlity macros for the W3 code library
                                  MACROS FOR GENERAL USE
                                             
   Generates: HTUtils.h
   
   See also: the system dependent file "tcp.h"
   
 */

#ifndef DEBUG
#define DEBUG   /* Noone ever turns this off as trace is too important */
#endif          /* Keeep option for really small memory applications tho */
                
#ifndef HTUTILS_H
#define HTUTILS_H

#ifdef SHORT_NAMES
#define WWW_TraceFlag HTTrFlag
#endif

/*

Debug message control.

 */
#ifndef STDIO_H
#include <stdio.h>
#define STDIO_H
#endif

/*
 * Tracing now works as a boolean from a resource. No, there are no
 *   more if's than before...
 *
 * SWP -- 02/08/96
 */

/*
#ifdef DEBUG
#define TRACE (WWW_TraceFlag)
#define PROGRESS(str) printf(str)
        extern int WWW_TraceFlag;
#else
#define TRACE 0
#define PROGRESS(str)
#endif


#undef TRACE
#define TRACE 1
#ifdef TRACE
#define HTTP_TRACE 1
#endif

#define CTRACE if(TRACE)fprintf
#define tfp stderr
*/

/*

Standard C library for malloc() etc

 */
#ifdef vax
#ifdef unix
#define ultrix  /* Assume vax+unix=ultrix */
#endif
#endif

#ifndef VMS
#ifndef ultrix
#ifdef NeXT
#include <libc.h>       /* NeXT */
#endif
#ifndef MACH /* Vincent.Cate@furmint.nectar.cs.cmu.edu */
#include <stdlib.h>     /* ANSI */
#endif
#else /* ultrix */
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#endif

#else   /* VMS */
#include <stdio.h>
#include <ctype.h>
#endif

#ifdef __sgi
#include <malloc.h>
#endif


/*

Macros for declarations

 */
#define PUBLIC                  /* Accessible outside this module     */
#define PRIVATE static          /* Accessible only within this module */

#ifdef __STDC__
#if 0
#define WWW_CONST const             /* "const" only exists in STDC */
#endif
#define WWW_CONST
#define NOPARAMS (void)
#define PARAMS(parameter_list) parameter_list
#define NOARGS (void)
#define ARGS1(t,a) \
                (t a)
#define ARGS2(t,a,u,b) \
                (t a, u b)
#define ARGS3(t,a,u,b,v,c) \
                (t a, u b, v c)
#define ARGS4(t,a,u,b,v,c,w,d) \
                (t a, u b, v c, w d)
#define ARGS5(t,a,u,b,v,c,w,d,x,e) \
                (t a, u b, v c, w d, x e)
#define ARGS6(t,a,u,b,v,c,w,d,x,e,y,f) \
                (t a, u b, v c, w d, x e, y f)
#define ARGS7(t,a,u,b,v,c,w,d,x,e,y,f,z,g) \
                (t a, u b, v c, w d, x e, y f, z g)
#define ARGS8(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h) \
                (t a, u b, v c, w d, x e, y f, z g, s h)
#define ARGS9(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h,r,i) \
                (t a, u b, v c, w d, x e, y f, z g, s h, r i)
#define ARGS10(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h,r,i,q,j) \
                (t a, u b, v c, w d, x e, y f, z g, s h, r i, q j)

#else  /* not ANSI */

#define WWW_CONST
#define NOPARAMS ()
#define PARAMS(parameter_list) ()
#define NOARGS ()
#define ARGS1(t,a) (a) \
                t a;
#define ARGS2(t,a,u,b) (a,b) \
                t a; u b;
#define ARGS3(t,a,u,b,v,c) (a,b,c) \
                t a; u b; v c;
#define ARGS4(t,a,u,b,v,c,w,d) (a,b,c,d) \
                t a; u b; v c; w d;
#define ARGS5(t,a,u,b,v,c,w,d,x,e) (a,b,c,d,e) \
                t a; u b; v c; w d; x e;
#define ARGS6(t,a,u,b,v,c,w,d,x,e,y,f) (a,b,c,d,e,f) \
                t a; u b; v c; w d; x e; y f;
#define ARGS7(t,a,u,b,v,c,w,d,x,e,y,f,z,g) (a,b,c,d,e,f,g) \
                t a; u b; v c; w d; x e; y f; z g;
#define ARGS8(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h) (a,b,c,d,e,f,g,h) \
                t a; u b; v c; w d; x e; y f; z g; s h;
#define ARGS9(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h,r,i) (a,b,c,d,e,f,g,h,i) \
                t a; u b; v c; w d; x e; y f; z g; s h; r i;
#define ARGS10(t,a,u,b,v,c,w,d,x,e,y,f,z,g,s,h,r,i,q,j) (a,b,c,d,e,f,g,h,i,j) \
                t a; u b; v c; w d; x e; y f; z g; s h; r i; q j;
                
        
#endif /* __STDC__ (ANSI) */

#ifndef NULL
#define NULL ((void *)0)
#endif

/*

Booleans

 */
/* Note: GOOD and BAD are already defined (differently) on RS6000 aix */
/* #define GOOD(status) ((status)38;1)   VMS style status: test bit 0         */
/* #define BAD(status)  (!GOOD(status))  Bit 0 set if OK, otherwise clear   */

#ifndef BOOLEAN_DEFINED
        typedef char    BOOLEAN;                /* Logical value */
#ifndef TRUE
#define TRUE    (BOOLEAN)1
#define FALSE   (BOOLEAN)0
#endif
#define BOOLEAN_DEFINED
#endif

#ifndef BOOL
#define BOOL BOOLEAN
#endif
#ifndef YES
#define YES (BOOLEAN)1
#define NO (BOOLEAN)0
#endif

#ifndef min
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#endif

#define TCP_PORT 80     /* Allocated to http by Jon Postel/ISI 24-Jan-92 */

/*      Inline Function WHITE: Is character c white space? */
/*      For speed, include all control characters */

#define WHITE(c) (((unsigned char)(c)) <= 32)


/*

Sucess (>=0) and failure (<0) codes

 */

#define HT_REDIRECTING 29998
#define HT_LOADED 29999                 /* Instead of a socket */
#define HT_INTERRUPTED -29998
#define HT_NOT_LOADED -29999
#define HT_OK           0               /* Generic success*/

#define HT_NO_ACCESS    -10             /* Access not available */
#define HT_FORBIDDEN    -11             /* Access forbidden */
#define HT_INTERNAL     -12             /* Weird -- should never happen. */
#define HT_BAD_EOF      -12             /* Premature EOF */

#include "HTString.h"   /* String utilities */

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/*

Out Of Memory checking for malloc() return:

 */
#ifndef __FILE__
#define __FILE__ ""
#define __LINE__ ""
#endif

#define outofmem(file, func) \
 { fprintf(stderr, "%s %s: out of memory.\nProgram aborted.\n", file, func); \
  exit(1);}


/*

Upper- and Lowercase macros

   The problem here is that toupper(x) is not defined officially unless isupper(x) is.
   These macros are CERTAINLY needed on #if defined(pyr) || define(mips) or BDSI
   platforms. For safefy, we make them mandatory.
   
 */
#include <ctype.h>

#ifndef TOLOWER
  /* Pyramid and Mips can't uppercase non-alpha */
#define TOLOWER(c) (isupper(c) ? tolower(c) : (c))
#define TOUPPER(c) (islower(c) ? toupper(c) : (c))
#endif /* ndef TOLOWER */

#define CR '\015'	/* Must be converted to ^M for transmission */
#define LF '\012'	/* Must be converted to ^J for transmission */

#endif /* HTUTILS_H */

/*

   end of utilities  */
