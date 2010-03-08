/*****************************************************************************
*
*                         NCSA DTM version 2.3
*                               May 1, 1992
*
* NCSA DTM Version 2.3 source code and documentation are in the public
* domain.  Specifically, we give to the public domain all rights for future
* licensing of the source code, all resale rights, and all publishing rights.
*
* We ask, but do not require, that the following message be included in all
* derived works:
*
* Portions developed at the National Center for Supercomputing Applications at
* the University of Illinois at Urbana-Champaign.
*
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
*
*****************************************************************************/

/*************************************************************************
**
**  dtm.h - 
**
*************************************************************************/

/*********************************************************************
**
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/dtm.h,v 1.4 1996/02/18 23:40:11 spowers Exp $
**
**********************************************************************/

/*
 * Revision 1.1.1.1  1993/07/04  00:03:11  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:15  marca
 * I think I got it now.
 *
 * Revision 1.32  92/05/06  16:40:17  jefft
 * Modified prototype on DTMaddInput
 * 
 * Revision 1.31  1992/04/30  20:27:05  jplevyak
 * Changed Version to 2.3
 *
 * Revision 1.30  1992/04/29  21:56:53  jplevyak
 * Add prototypes for DTMgetConnectionCount and DTMaddInput.  Remove
 * Cray #define for VOIDPTR since they have fixed their compiler.
 *
 * Revision 1.29  92/03/20  21:14:40  jplevyak
 * Add DTMgetPortAddr and remove DTMgetPortName
 * 
 * Revision 1.28  1992/03/13  18:20:31  jefft
 * dtm.h depended on TRUE being defined before it was included.  This word
 * TRUE was changed to 1 and FALSE to 0
 *
 * Revision 1.27  1992/02/27  23:43:34  jplevyak
 * New prototype DTMgetReturnPortName.
 *
 * Revision 1.26  1992/02/21  13:46:12  dweber
 * Fixed prototype problems in dtm_get_type.
 *
 * Revision 1.25  92/02/18  14:02:36  jplevyak
 * Added prototype for DTMaddInPortSocket.
 * 
 * Revision 1.24  92/01/30  19:28:24  jplevyak
 * add prototypes
 * 
 * Revision 1.23  1992/01/02  16:31:56  dweber
 * Reorganized the DTM class and MSG class
 *
 * Revision 1.22  91/12/13  22:34:48  jefft
 * Removed DTMtitle and DTMtype constants.  Added dtm_(sg)et_address macros.
 * Added MSG(sg)etString macros.
 * 
 * Revision 1.21  91/12/13  15:37:55  dweber
 * Fixed DTMHL bugs and re-ordered some macros for clarification
 * 
 * Revision 1.20  91/12/12  22:55:27  jefft
 * Add numbers to the comments beside the DTM errors
 * 
 * Revision 1.19  1991/12/10  22:15:56  jefft
 * fixed DTMgetAddress macro (it was calling dtm_set_char).
 *
 * Revision 1.18  1991/12/09  18:36:18  jplevyak
 *  Added support for Callback ( DTMreadReady ).
 *  Use arch.h as a general architecture definition header.
 *
 * Revision 1.17  1991/10/29  23:48:17  jefft
 * move environmental variables definition from dtmnserv.h to dtm.h
 *
 * Revision 1.16  1991/10/16  23:23:56  jplevyak
 * Added new error message and more parameters to DTMsendRoute
 *
 * Revision 1.15  91/10/11  20:23:31  jplevyak
 * Added prototype for DTMcheckRoute,
 * Changed DTMSYNC, DTMNOSYNC to DTM_SYNC, DTM_ASYNC.
 * 
 * Revision 1.14  1991/10/10  14:23:43  jplevyak
 * All prototypes are now included and tested.
 *
 * Revision 1.13  1991/09/26  20:13:54  jplevyak
 * Added many prototypes.  Moved some internal function out to dtmint.h
 *
 * Revision 1.12  91/09/18  15:27:54  jplevyak
 * Added extern definition for DTMinit()
 * 
 * Revision 1.11  91/09/13  17:36:05  sreedhar
 * DTMSYNC, DTMNOSYNC added
 * 
 * Revision 1.10  1991/09/13  15:31:45  jefft
 * fixed DTMHL macro, it should not have taken an arguement.
 *
 * Revision 1.9  1991/09/11  21:09:20  jplevyak
 * Fixed typeo dtm_set_type -> dtm_get_type.
 *
 * Revision 1.8  1991/09/10  15:06:08  jefft
 * cleaned-up message class macros and added standard macros for passing
 * port address (DTMsetAddress and DTMgetAddress).
 *
 * Revision 1.7  1991/08/15  19:10:39  sreedhar
 * Changes for logical portname version
 *
 * Revision 1.5  1991/06/11  15:22:39  sreedhar
 * disclaimer added
 *
 * Revision 1.4  1991/06/07  16:04:51  sreedhar
 * Changes for "Sequence start" message
 *
 * Revision 1.3  1991/05/30  15:43:27  sreedhar
 * recvDataset, sendDataset replaced by readDataset/writeDataset
 *
 * Revision 1.2  1990/11/16  16:04:32  jefft
 * fixed typecast problem with DTMgetType
 *
 * Revision 1.1  90/11/08  16:31:19  jefft
 * Initial revision
 * 
*/


#ifndef DTM_INC
#define DTM_INC

#include <sys/types.h>

#ifndef ARCH_INC
#include "arch.h"
#endif

#if defined( _ARCH_PROTO )
#define	DTM_PROTOTYPES
#define	DTM_PROTO(x)	x
#else
#define	DTM_PROTO(x)	()	
#endif

/* DTM constants */
#define	DTM_WAIT_TIMEOUT	30	/* Seconds 		*/
#define DTM_MAX_HEADER		1024	/* Max. header length	*/

/* general macros used by other message classes */
#define	dtm_set_class(h, c)		(strcpy((h), (c)), strcat((h), " "))
#define	dtm_compare_class(h, c)		!strncmp((h), (c), strlen(c))

#define	dtm_header_length(h)		(strlen(h)+1)

#define	dtm_set_type(h, t)		dtm_set_int((h), "DT", (t))
#define	dtm_get_type(h, t)   		((*(t)=(int)DTM_FLOAT), \
					    (dtm_get_int((h), "DT",((int *)t))))

#define	dtm_set_title(h, c)		dtm_set_char((h), "TTL", (c))
#define	dtm_get_title(h, c, l)		dtm_get_char((h), "TTL", (c), (l))

#define	dtm_set_address(h, c)		dtm_set_char((h), "PORT", (c))
#define	dtm_get_address(h, c, l)	dtm_get_char((h), "PORT", (c), (l))


/* standard types supported by DTM */
typedef enum  {
	DTM_CHAR = 0,
	DTM_SHORT,
	DTM_INT,
	DTM_FLOAT,
	DTM_DOUBLE,
	DTM_COMPLEX,
	DTM_TRIPLET
} DTMTYPE;


/* DTM triplet type */
struct DTM_TRIPLET {
	int	tag;
	float	x, y, z;
};


#define	DTM_DEFAULT		DTM_SYNC
typedef	enum	{
	DTM_SYNC=0,
	DTM_ASYNC
} DTMqserv ;

/* Environmental variables used by DTM name server */
#define DTM_NAMESERVER		"_DTM_NAMESERVER"
#define DTM_REFNAME		"_DTM_REFNAME"


/* commands supported for groups */
typedef enum  {
	DTM_NEW = 1,
	DTM_APPEND,
	DTM_DELETE,
	DTM_DONE
} DTMCMD;


/*
	NOTE: the strings that describe the errors in DTMerr
		are located in fatal.c.  Any changes to this list
		must be accompanied by a corresponding change there.
*/	
#define		DTMERROR	-1
#define		DTM_OK		DTMNOERR

typedef enum  {
	DTMNOERR=0,		/* no error */
	DTMMEM,			/* (1) Out of memory */
	DTMHUH,			/* (2) Unknown port definition */
	DTMNOPORT,		/* (3) No DTM ports available */
	DTMPORTINIT,		/* (4) DTM port not initialized */
	DTMCALL,		/* (5) calling routines in wrong order */
	DTMEOF,			/* (6) EOF error */
	DTMSOCK,		/* (7) Socket error */
	DTMHOST,		/* (8) That hostname is not found/bad */
	DTMTIMEOUT,		/* (9) Timeout waiting for connection */
	DTMCCONN,		/* (10) DTM cannot connect (network down?) */
	DTMREAD,		/* (11) error returned from system read */
	DTMWRITE,		/* (12) error returned from system write(v) */
	DTMHEADER,		/* (13) Header to long for buffer */
	DTMSDS,			/* (14) SDS error */
	DTMSELECT,		/* (15) Select call error */
	DTMENV,			/* (16) Environment not setup */
	DTMBUFOVR,		/* (17) User buffer overflow */
	DTMCORPT,		/* (18) Port table corrupted */
	DTMBADPORT,		/* (19) Port identifier is bad/corrupt/stale */
	DTMBADACK,		/* (20) Bad ack to internal flow control */
	DTMADDR,		/* (21) Bad address */
	DTMSERVER		/* (22) Problem communicating with the server */
} DTMerr;

typedef	struct	Dtm_set	{
	int	port ;
	int	status ;
} Dtm_set ;

typedef	struct	Sock_set {
	int	sockfd ;
	int	status ;
} Sock_set ;

typedef void *	VOIDPTR;


/* function definitions and extern references */

#ifdef __cplusplus
extern "C" {
#endif

extern int	DTMmakeInPort		DTM_PROTO((char * portname, int qservice));
extern int	DTMaddInPortSocket	DTM_PROTO((int port, int socket ));
extern int	DTMmakeOutPort		DTM_PROTO((char * portname, int qservice));
#define	DTM_PORT_READY		1
#define	DTM_PORT_NOT_READY	0
extern int	DTMavailWrite		DTM_PROTO(( int port ));
extern int	DTMavailRead		DTM_PROTO(( int port ));
extern int	DTMbeginRead		DTM_PROTO(( int port, VOIDPTR header, int size));
extern int	DTMbeginWrite		DTM_PROTO(( int port, VOIDPTR header, int size));
extern int	DTMreadDataset	DTM_PROTO(( int p, VOIDPTR ds, int size, DTMTYPE type));
extern int	DTMwriteDataset	DTM_PROTO(( int p, VOIDPTR ds, int size, DTMTYPE type));
extern int	DTMendRead		DTM_PROTO(( int port ));
extern int	DTMendWrite		DTM_PROTO(( int port ));
extern int	DTMreadMsg		DTM_PROTO(( int p, char *hdr, int hdrsize, 
								VOIDPTR data, int datasize, int datatype ));
extern int	DTMdestroyPort	DTM_PROTO(( int port));
extern char	*DTMerrmsg();
extern int	DTMgetPortAddr	DTM_PROTO(( int port, char * addr, int length ));
extern int  DTMgetReturnPortName    DTM_PROTO(( int port, char *** addrs,
										int * n_addrs));
extern int  DTMselectRead	DTM_PROTO(( Dtm_set *dtmset, int dtmnum, 
								Sock_set *sockset, int socknum, int period ));
extern void	DTMsetGroup		DTM_PROTO(( char * header, DTMCMD cmd, char * parent, 
								char * self ));
extern int	DTMgetGroup		DTM_PROTO(( char * header, DTMCMD cmd, char * parent, 
								char * self ));
extern int	DTMsendRoute 	DTM_PROTO(( int fd, char * sendto_addr, int addcount,
								char **add_addresses, int delcount, 
								char **del_addresses ));
extern int	DTMcheckRoute	DTM_PROTO(( int port ));
	/*	Not implemented Yet	*/
extern int	DTMrecvServerMsg	DTM_PROTO(( int fd, int len, void * buffer ));
extern int	DTMreadReady		DTM_PROTO(( int port, void  (*func)() ));

extern int	DTMgetConnectionCount DTM_PROTO(( int port, int * n_connects ));

/*	If you do not have X included you are not likely to use this function */
typedef	(*DTMfuncPtr)();
#ifdef _XtIntrinsic_h
extern	int	DTMaddInput DTM_PROTO(( int port, long condition,
					XtInputCallbackProc proc, caddr_t client_data));
#else
extern	int	DTMaddInput DTM_PROTO(( int port, long condition,
							DTMfuncPtr proc, caddr_t client_data));
#endif

extern DTMerr	DTMerrno;

extern char     *dtm_find_tag       DTM_PROTO(( char * h, char * tag ));
extern void     dtm_set_char    DTM_PROTO(( char * h, char * tag, char * s));
extern int      dtm_get_char    DTM_PROTO(( char * h, char * tag,
									char * destination, int length ));
extern void     dtm_set_int     DTM_PROTO(( char * h, char * tag, int i ));
extern int      dtm_get_int     DTM_PROTO(( char * h, char * tag, int *x));
extern void     dtm_set_float   DTM_PROTO(( char * h, char * tag, float f));
extern int      dtm_get_float   DTM_PROTO(( char * h, char * tag, float *f));

#ifdef __cplusplus
};
#endif



/*
 * The following macros are defined for the specific DTM class.  They are
 * included to provide a standard framework for other classes to emulate.
 */

/*
 * DTM class specific macros
 */
#define		DTMclass		"DTM"

#define		DTMsetClass(h)		dtm_set_class((h), DTMclass)
#define		DTMcompareClass(h)	dtm_compare_class((h), DTMclass)

#define		DTMheaderLength		dtm_header_length
#define		DTMHL			dtm_header_length

#define		DTMsetAddress		dtm_set_address
#define		DTMgetAddress		dtm_get_address



/*
 * MSG class specific macros
 */
#define		MSGclass		"MSG"

#define		MSGsetClass(h)		dtm_set_class((h), MSGclass)
#define		MSGcompareClass(h)	dtm_compare_class((h), MSGclass)

#define		MSGsetString(h, s)	dtm_set_char((h), "STR", (s))
#define		MSGgetString(h, s, l)	dtm_get_char((h), "STR", (s), (l))

#define		MSGheaderLength		dtm_header_length
#define		MSGHL			dtm_header_length


#endif /* DTM_INC */
