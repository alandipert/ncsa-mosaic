/*                System dependencies in the W3 library
                                   SYSTEM DEPENDENCIES
                                             
   System-system differences for TCP include files and macros. This
   file includes for each system the files necessary for network and
   file I/O.
   
  AUTHORS
  
  TBL                Tim Berners-Lee, W3 project, CERN, <timbl@info.cern.ch>
  EvA                     Eelco van Asperen <evas@cs.few.eur.nl>
  MA                      Marc Andreessen NCSA
  AT                      Aleksandar Totic <atotic@ncsa.uiuc.edu>
  SCW                     Susan C. Weber <sweber@kyle.eitech.com>
                         
  HISTORY:
  22 Feb 91               Written (TBL) as part of the WWW library.
  16 Jan 92               PC code from EvA
  22 Apr 93               Merged diffs bits from xmosaic release
  29 Apr 93               Windows/NT code from SCW

  Much of the cross-system portability stuff has been intentionally
  REMOVED from this version of the library by Marc A in order to
  discourage attempts to make "easy" ports of Mosaic for X to non-Unix
  platforms.  The library needs to be rewritten from the ground up; in
  the meantime, Unix is *all* we support or intend to support with
  this set of source code.

*/

#ifndef TCP_H
#define TCP_H

/*

Default values

   These values may be reset and altered by system-specific sections
   later on.  there are also a bunch of defaults at the end .
   
 */
/* Default values of those: */
#define NETCLOSE close      /* Routine to close a TCP-IP socket         */
#define NETREAD  HTDoRead       /* Routine to read from a TCP-IP socket     */
#define NETWRITE write      /* Routine to write to a TCP-IP socket      */

/* Unless stated otherwise, */
#define SELECT                  /* Can handle >1 channel.               */
#define GOT_SYSTEM              /* Can call shell with string           */

#ifdef unix
#define GOT_PIPE
#endif

typedef struct sockaddr_in SockA;  /* See netinet/in.h */


#ifndef STDIO_H
#include <stdio.h>
#define STDIO_H
#endif

#ifdef _AIX
#define AIX
#endif
#ifdef AIX
#define unix
#endif

#ifdef _IBMR2
#define USE_DIRENT              /* sys V style directory open */
#endif

/* Solaris. */
#if defined(sun) && defined(__svr4__)
#define USE_DIRENT              /* sys V style directory open */
#endif

#if defined(__alpha)
#define USE_DIRENT
#endif

#ifndef USE_DIRENT
#ifdef SVR4
#define USE_DIRENT
#endif
#endif /* not USE_DIRENT */

#include <string.h>

/* Use builtin strdup when appropriate. */
#if defined(ultrix) || defined(VMS) || defined(NeXT)
extern char *strdup ();
#endif



/*

VAX/VMS

   Under VMS, there are many versions of TCP-IP. Define one if you do
   not use Digital's UCX product:
   
  UCX                     DEC's "Ultrix connection" (default)
  WIN_TCP                 From Wollongong, now GEC software.
  MULTINET                From SRI, now from TGV Inv.
  DECNET                  Cern's TCP socket emulation over DECnet
                           
   The last three do not interfere with the
   unix i/o library, and so they need special calls to read, write and
   close sockets. In these cases the socket number is a VMS channel
   number, so we make the @@@ HORRIBLE @@@ assumption that a channel
   number will be greater than 10 but a unix file descriptor less than
   10.  It works.
   
 */
#ifdef vms
#ifdef WIN_TCP
#undef NETREAD
#undef NETWRITE
#undef NETCLOSE
#define NETREAD(s,b,l)  ((s)>10 ? netread((s),(b),(l)) : read((s),(b),(l)))
#define NETWRITE(s,b,l) ((s)>10 ? netwrite((s),(b),(l)) : write((s),(b),(l)))
#define NETCLOSE(s)     ((s)>10 ? netclose(s) : close(s))
#endif

#ifdef MULTINET
#undef NETCLOSE
#undef NETREAD
#undef NETWRITE
#define NETREAD(s,b,l)  ((s)>10 ? socket_read((s),(b),(l)) : read((s),(b),(l)))
#define NETWRITE(s,b,l) ((s)>10 ? socket_write((s),(b),(l)) : \
                                write((s),(b),(l)))
#define NETCLOSE(s)     ((s)>10 ? socket_close(s) : close(s))
#endif

/*      Certainly this works for UCX and Multinet; not tried for Wollongong
*/
#ifdef MULTINET
#include "multinet_root:[multinet.include.sys]types.h"
#include "multinet_root:[multinet.include]errno.h"
#include "multinet_root:[multinet.include.sys]time.h"
#else
#include types
#include errno
#include time
#endif /* multinet */

#include string

#ifndef STDIO_H
#include stdio
#define STDIO_H
#endif

#include file

#include unixio

#define INCLUDES_DONE

#ifdef MULTINET  /* Include from standard Multinet directories */
#include "multinet_root:[multinet.include.sys]socket.h"
#ifdef __TIME_LOADED  /* defined by sys$library:time.h */
#define __TIME  /* to avoid double definitions in next file */
#endif
#include "multinet_root:[multinet.include.netinet]in.h"
#include "multinet_root:[multinet.include.arpa]inet.h"
#include "multinet_root:[multinet.include]netdb.h"

#else  /* not multinet */
#ifdef DECNET
#include "types.h"  /* for socket.h */
#include "socket.h"
#include "dn"
#include "dnetdb"
/* #include "vms.h" */

#else /* UCX or WIN */
#include socket
#include in
#include inet
#include netdb

#endif  /* not DECNET */
#endif  /* of Multinet or other TCP includes */

#define TCP_INCLUDES_DONE

#endif  /* vms */



/*

SCO ODT unix version

 */

#ifdef sco
#include <sys/fcntl.h>
#define USE_DIRENT
#endif

/*

MIPS unix

 */
/* Mips hack (bsd4.3/sysV mixture...) */

#ifdef mips
extern int errno;
#endif


/*

Regular BSD unix versions

   These are a default unix where not already defined specifically.
   
 */
#ifndef INCLUDES_DONE
#include <sys/types.h>
/* #include <streams/streams.h>                 not ultrix */
#include <string.h>

#include <errno.h>          /* independent */
#include <sys/time.h>       /* independent */
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/file.h>       /* For open() etc */
#define INCLUDES_DONE
#endif  /* Normal includes */

/*                      Directory reading stuff - BSD or SYS V
*/
#ifdef unix                    /* if this is to compile on a UNIX machine */
#define GOT_READ_DIR 1    /* if directory reading functions are available */
#ifdef USE_DIRENT             /* sys v version */
#include <dirent.h>
#define direct dirent
#else
#include <sys/dir.h>
#endif
#if defined(sun) && defined(__svr4__)
#include <sys/fcntl.h>
#include <limits.h>
#endif
#endif

/*

Defaults

  INCLUDE FILES FOR TCP
  
 */
#ifndef TCP_INCLUDES_DONE
#include <sys/ioctl.h> /* EJB */
#include <sys/socket.h>
#include <netinet/in.h>
#ifndef __hpux /* this may or may not be good -marc */
#include <arpa/inet.h>      /* Must be after netinet/in.h */
#endif
#include <netdb.h>
#endif  /* TCP includes */


/*

  MACROS FOR MANIPULATING MASKS FOR SELECT()
  
 */
#ifdef SELECT
#ifndef FD_SET
typedef unsigned int fd_set;
#define FD_SET(fd,pmask) (*(pmask)) |=  (1<<(fd))
#define FD_CLR(fd,pmask) (*(pmask)) &= ~(1<<(fd))
#define FD_ZERO(pmask)   (*(pmask))=0
#define FD_ISSET(fd,pmask) (*(pmask) & (1<<(fd)))
#endif  /* FD_SET */
#endif  /* SELECT */

#endif /* TCP_H */
