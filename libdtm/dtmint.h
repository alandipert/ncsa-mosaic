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

/*********************************************************************
**
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/dtmint.h,v 1.5 1996/02/18 23:40:13 spowers Exp $
**
**********************************************************************/

/*
 * Revision 1.2  1993/10/06  06:16:06  ebina
 * Fix stupid cont annoyance
 *
 * Revision 1.1.1.1  1993/07/04  00:03:11  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:19  marca
 * I think I got it now.
 *
 * Revision 1.28  92/05/05  22:27:50  jplevyak
 * Corrected X interface code.
 * 
 * Revision 1.27  1992/04/30  20:27:05  jplevyak
 * Changed Version to 2.3
 *
 * Revision 1.26  1992/04/29  21:58:54  jplevyak
 * Add new structure elements to DTMPORT to support DTMaddInput.
 * Add prototypes for dtm_accept_read_connections and
 * dtm_set_Xcallback
 *
 * Revision 1.25  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.24  1992/03/02  18:29:41  jplevyak
 * Fixed bug in EAGAIN handling.
 *
 * Revision 1.23  1992/03/02  17:20:14  jplevyak
 * Temporary back out.
 *
 * Revision 1.21  1992/02/28  03:40:24  jplevyak
 * int/long confict fix (no diff on workstations)
 * ,
 *
 * Revision 1.20  92/02/27  23:59:56  jplevyak
 * Fixup prototype problems.
 * 
 * Revision 1.19  1992/01/30  19:28:50  jplevyak
 * Add support for c++ type external definitions.
 *
 * Revision 1.18  92/01/24  18:45:50  jefft
 * Removed prototypes for dtm_set_<type> and dtm_get_<type>, added them to dtm.h
 * 
 * Revision 1.17  1991/12/16  19:44:04  jefft
 * Fixed bug with defining of NULL macro
 *
 * Revision 1.16  1991/12/09  18:36:18  jplevyak
 * Added support for Callback ( DTMreadReady ).
 *
 * Revision 1.15  1991/11/22  21:31:37  jplevyak
 * Added fDiscard to port structure.  Removed prototype for dtm_get_nlist
 * and dtm_check_server.
 *
 * Revision 1.14  1991/10/16  11:25:23  jplevyak
 * ifdef around a duplicated include that was giving the SGIs trouble.
 *
 * Revision 1.13  1991/10/14  16:48:21  jplevyak
 * Add flag in DTMPORT indicating whether or not the address is
 * logical.
 *
 * Revision 1.12  1991/10/11  20:24:11  jplevyak
 * Changed DTMSendCTSAhead default to be 0 (no chance of deadlock) as
 * default.
 *
 * Revision 1.11  1991/10/10  14:26:16  jplevyak
 * Finished fixing naming convensions.  All prototypes are now included.
 *
 * Revision 1.10  91/09/26  20:16:46  jplevyak
 * Major reorganization.  Made this file compatible with dtm.h.  Moved
 * redundant out.  Added several configuration parameters, added
 * prototypes for internally global functions.
 * 
 * Revision 1.9  91/09/18  15:28:50  jplevyak
 * Added some external definitions for shared functions.
 * 
 * Revision 1.8  91/09/13  17:37:37  sreedhar
 * MAX132 instead of MAXBUFSIZE
 * 
 * Revision 1.7  1991/08/15  19:10:53  sreedhar
 * Changes for logical portname version
 *
 * Revision 1.6  1991/06/11  15:22:46  sreedhar
 * disclaimer added
 *
 * Revision 1.5  1991/06/07  16:06:03  sreedhar
 * Changes for sequence start message
 *
 * Revision 1.4  1991/05/30  15:50:58  sreedhar
 * Changes for readMsg/writeMsg internal release
 *
 * Revision 1.3  1991/03/14  23:26:55  jefft
 * removed call to dtm_fatal when errors occur.
 *
 * Revision 1.2  90/11/21  12:35:44  jefft
 * Removed trailing comma from DTM error enum type.
 * 
 * Revision 1.1  90/11/08  16:32:03  jefft
 * Initial revision
 * 
*/


#ifndef DTMINT_INC
#define DTMINT_INC

#ifndef DTM_INC
#include "dtm.h"
#endif

#ifndef	u_char	/* it is not clear if we can count on __sys_types_h */
#include <sys/types.h>
#endif
#ifndef	IPPROTO_IP
#ifdef _ARCH_MSDOS
# include <nmpcip.h>
#else
# include <netinet/in.h>
#endif
#endif

/*	Added to shut up the SGI compiler which quotes ANSI regulations at
	you. 
*/
#ifdef DTM_PROTOTYPES
struct sockaddr;
struct timeval;
struct iovec;
#endif

#ifdef MAIN
#define	global
#define	INIT(x)	= x
#else
#define	global	extern
#define	INIT(x)
#endif

#define	reg	register	


/*
	MACHINE SPECIFIC #defines
*/

	/*
		These at least work on a 68k architecture....
	*/
#ifdef _ARCH_MSDOS
 typedef unsigned int uint16;
 typedef unsigned long uint32;
 typedef int int16;
 typedef long int32;
#else /* _ARCH_MSDOS */
 typedef unsigned short uint16;
 typedef unsigned int uint32;
 typedef short int16;
 typedef int int32;
#endif /* _ARCH_MSDOS */

#ifdef SUN
#define BSD
#endif

#ifdef  CRAY
#  define       STDINT(x)       (x <<= 32)
#  define       LOCALINT(x)     (x >>= 32)
#else
#  define       STDINT(x)	x = htonl(x)
#  define       LOCALINT(x)	x = ntohl(x)
#endif

/*
	The SGI compiler does not like to see true floats in prototypes
*/
#ifdef SGI
#define	FLOAT	double
#else
#define	FLOAT	float
#endif

#if defined(BSD) && !defined(NEXT) && !defined(SOLARIS)
#define FREE_RETURNS_INT
#endif

#define DTM_VERSION	"2.3"

#if defined(_STDC_) || defined(__STDC__)
#define	VOID	void
#else
#define	VOID
#endif


/*
	GENERIC #defines
*/

#ifdef _ARCH_MSDOS
#define	CHECK_ERR(X)	if (((long)(X)) == DTMERROR) return DTMERROR
#else
#define	CHECK_ERR(X)	if (((int)(X)) == DTMERROR) return DTMERROR
#endif

#ifndef FALSE
#  define	FALSE	0
#  define	TRUE	!FALSE
#endif

#define	DTM_BUFF_SIZE		32768
#define	DTM_REFUSE_LIMIT	120
#define	DTM_NO_CONNECTION	-1

	/*
		Perhaps we should distinguish these
	*/
#define	DTM_CTS				0
#define	DTM_RTS				0
#define	DTM_EOT				0

#if !defined(_ARCH_MACOS) && !defined(_ARCH_MSDOS)
#  define	DISCARDSIZE	32768
#else
#  define	DISCARDSIZE	4096
#endif

#define DTMSTD		0
#define DTMLOCAL	1
#define	INPORTTYPE	0	/* input port type 	*/
#define	OUTPORTTYPE	1	/* output port type 	*/
#define	PNAMELEN	64	/* max length of portname	*/
#define	REFNAMELEN	32	/* max length of reference name given by
				   nameserver.
				*/
#define	MAX132	132		/* max space of 132 */
#define	SEP	" "		/* blank as separator */
#define	COLON	":"		/* colon as separator */	

#ifndef	NULL
# if defined( _STDC_ ) || defined( __STDC__ )
#  define 	NULL	((void *)0)
# else
#  define	NULL	0
# endif
#endif

#define	FOR_EACH_OUT_PORT( pcur, pp ) \
	for ( pcur = pp->out;  pcur != NULL ; pcur = pcur->next ) 
#define	FOR_EACH_IN_PORT( inp, pp ) \
	for ( inp = pp->in; inp != NULL ; inp = inp->next )


/* 
	PORT STRUCTURES 
*/

typedef	struct sockaddr_in	S_ADDR;

typedef	struct	Port	
{
	uint32	nethostid ;
	uint16	portid ;
} Port ;

typedef	struct	Outport	
{
	S_ADDR	sockaddr ;		/* Socket family, netid/hostid, portid */
	int32	connfd ; 			/* connection fd */ 
	int		availwrite ;		/* port availability for write */	
	int		seqstart ;			/* "Sequence start" message sent or not */ 
	struct Outport * next;	/* link to next outport */ 
} Outport ;

#define	DTM_NEW_DATASET		-1	
typedef struct Inport {
	int32		fd;				/* connection fds */
	int32		blocklen;		/* records no. of bytes read */
		/*
			Perhaps we should make these sequence, enums... that
			would allow > < comparisions
		*/
	int			fCTSsent;		/* CTS already sent */
	int			fGotHeader;		/* Already got the header */
#ifdef _XtIntrinsic_h
#ifdef __STDC__
#if sizeof( XtInputId ) != sizeof( int )
	Balk -	The following structure definition needs to be repaired
			to restore the assertion
#endif
#endif
	XtInputId	XinputId;	
#else
	int			XinputId;
#endif
	struct Inport	* next;
} Inport;

typedef	struct 
{
	S_ADDR	sockaddr ;	
				/* Socket family, netid/hostid, portid */
	int32	sockfd ; /* Main socket of port 
					Outport - UDP socket
					Inport  - TCP socket
				 */
#ifdef _XtIntrinsic_h
	XtInputId	XinputId;	
#else
	int			XinputId;
#endif
	
	int		fLogical;
	char	portname[ PNAMELEN ] ;	/* Logical portname */

	int		porttype ;	/* Input or Output port	*/
	int		qservice ;	/* Quality of service - actually qserv enum */
	int32	key ;		/* unique value used to catch stale port access */

	char *	Xcallback_data;
	void	(*Xcallback)();
#ifdef _XtIntrinsic_h
	XtInputCallbackProc	XaddInput;
	XtInputCallbackProc	XremoveInput;
#else
	int		(*XaddInput)();
	int		(*XremoveInput)();
#endif

	/*	Input port specific data	*/

	Inport	*in;
	Inport	*nextToRead;
	void  	(*callback)();

	/*	Output port specific data	*/

	Outport *out ;	/* Linked list of Out port specific structures   */
					/* If the last action on this port was a successful
					   availWrite, then when we do a beginWrite, we
					   will NOT check for new routing information */
	int		fLastWasSuccessfulAvailWrite;
	int		fGotList;	/* initially false, TRUE after any list is read */
	int		fDiscard;	/* initially false, TRUE means /dev/null output */
} DTMPORT ;


/*
	GLOBAL VARIABLES
*/

#define	DTM_INITIALIZED					(DTMpt != NULL) 
#define	DTM_PORTS_INITIAL				20
#define	DTM_PORTS_GROW					20
global DTMPORT	**DTMpt					INIT( NULL );
global int32	DTMptCount				INIT( 0 );	
global int32 	DTMportSequenceNumber	INIT( 1 );

#ifdef _ARCH_MSDOS
#include <errno.h>
#else
extern	int		errno;
#endif

global DTMerr	DTMerrno				INIT( DTM_OK );
global char	*	dtm_discard				INIT( NULL );

extern int		(*DTMconvertRtns[]) DTM_PROTO(( int, VOIDPTR, int ));

/*	global options */

	/*
		NOTE: setting this value to anything other than 0 can result
		in deadlock. However, in correctly configure graphs, this should
		not occure and performance is considerably better with values > 0.
		DO NOT SET THIS VALUE TO SOME ARBITRARILY LARGE NUMBER.
	*/
global	int		DTMSendCTSAhead			INIT( 0 );


/*
	FUNCTION PROTOTYPES
*/
#ifdef __cplusplus
extern "C" {
#endif

#define	NOT_LOGICAL_NAME	FALSE
#define	LOGICAL_NAME		TRUE
extern char * 	dtm_get_refname DTM_PROTO(( VOID ));
extern char *	dtm_get_refname DTM_PROTO(( VOID ));
extern int		dtm_nsend_ackroute	DTM_PROTO(( char * portname ));
extern int		dtm_nsend_sockaddr DTM_PROTO(( int fd, char * sendto_addr,
					char * refname, char * portname, S_ADDR * sockaddr ));
extern Outport * dtm_new_out_port DTM_PROTO(( Port * port ));
#define	DTM_PORT_MASK			0xFFFF
#define	DTM_PORT_KEY_SHIFT		16	
extern	int		dtm_map_port_internal DTM_PROTO(( int32 port ));
extern	void	dtm_map_port_external DTM_PROTO(( int32 * port ));
#define	DTM_WAIT				TRUE
#define	DTM_DONT_WAIT			FALSE
extern int		dtm_check_server	DTM_PROTO(( DTMPORT *pp, int fWait ));
extern int		dtm_send_ack		DTM_PROTO(( int fd, int32 ack ));
extern int		dtm_recv_ack		DTM_PROTO(( int fd, int32 * ack ));

extern	int		dtm_destroy_in_port	DTM_PROTO(( Inport * inp, DTMPORT * pp ));
extern	int		dtm_writev_buffer	DTM_PROTO(( int fd, struct iovec *iov,
										int32 iovlen, int32 iovsize,
										struct sockaddr * addr, int addrlen ));
extern	int		dtm_read_buffer		DTM_PROTO(( int d, int32 * blocklen, 
										void * buffer, int length ));
extern	int		dtm_recv_header		DTM_PROTO((int fd , void * header, 
										int length));
extern	int		dtm_read_header		DTM_PROTO((int fd , void * header,
										int length));
extern	int		dtm_parse_ipaddr	DTM_PROTO(( char * source, 
										unsigned long * dest ));
extern	int		dtm_quick_select	DTM_PROTO(( int socket, int32 * count ));
extern	int		dtm_select			DTM_PROTO(( int fd, int32 * count, 
										int32 time ));
extern	int		dtm_accept			DTM_PROTO(( int fd, S_ADDR * sn, 
										struct timeval * timeout ));
extern int		dtm_connect 		DTM_PROTO(( S_ADDR * sn, int * sockret ));
extern int		dtm_quick_connect	DTM_PROTO(( S_ADDR * sn, int * sockret ));
extern int		dmt_end_connect		DTM_PROTO(( int32 socket )); 
extern unsigned long	dtm_get_ipaddr	DTM_PROTO(( char * ipaddrstr ));
extern int		dtm_socket_init 	DTM_PROTO(( S_ADDR * sockaddr, int porttype, 
										int fLogicalName ));
extern int		dtm_init_sockaddr	DTM_PROTO(( S_ADDR * sockaddr, 
										char * portname,
										int	* pfLogicalName ));
extern int 		dtm_ninit			DTM_PROTO(( void ));
extern char *	dtm_get_naddr		DTM_PROTO(( S_ADDR * addr, int * sockfd ));
extern void		dtm_version			DTM_PROTO(( void ));
extern int		dtm_sigio			DTM_PROTO(( int ));
extern char * 	dtm_find_tag		DTM_PROTO(( char *, char *));
extern int		dtm_accept_read_connections DTM_PROTO(( DTMPORT *pp,int fWait ));
extern void		dtm_set_Xcallback	DTM_PROTO(( DTMPORT *pp, Inport * inp ));
#ifdef _XtIntrinsic_h
extern void		dtm_handle_in		DTM_PROTO(( caddr_t client_data,  
										int * fd, XtInputId * in ));
#else
extern void		dtm_handle_in		DTM_PROTO(( caddr_t client_data,  
										int * fd, void * in ));
#endif
 
#ifdef __cplusplus
};
#endif

#endif /* DTMINT_INC */
