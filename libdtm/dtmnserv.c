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
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/dtmnserv.c,v 1.4 1996/02/18 23:40:14 spowers Exp $
**
**********************************************************************/
/*
#ifdef RCSLOG

 $Log: dtmnserv.c,v $
 Revision 1.4  1996/02/18 23:40:14  spowers
 PROTO -> DTM_PROTO

 Revision 1.3  1995/10/14 22:07:27  spowers
 Bzero and Bcopy removed...memset memcpy used instead.

 Revision 1.2  1995/10/13 06:33:12  spowers
 Solaris support added.

 Revision 1.1.1.1  1995/01/11 00:02:59  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:39:50  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:31  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:11  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:22  marca
 * I think I got it now.
 *
 * Revision 1.19  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 

#endif
*/
/*
	Purpose	: Functions to interact with name server.
*/

#include "arch.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef _ARCH_MSDOS
#include <nmpcip.h>
#include "uio.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#endif

#if !defined _ARCH_MACOS & !defined NEXT
# ifdef	CONVEX 
#  include <sys/malloc.h>
# else
#  include <malloc.h>
# endif
#endif

#include "dtmint.h"
#include "dtmnserv.h"
#include <stdio.h>
#include "debug.h"

/*
    STATIC FUNCTION PROTOTYPES
*/
static int init_refname DTM_PROTO(( char *refname, int len ));
static void del_nlist DTM_PROTO(( DTMPORT *pp, int count ));
static void add_nlist DTM_PROTO(( DTMPORT *pp, int count ));


/*	
	Function to initialise self's reference name from environment
	variable passed by nameserver.

	Return	value	:	0 on success,
				-1 on error.

	Notes	:	Environment variable	- DTM_REFNAME
*/
 
#ifdef DTM_PROTOTYPES
static int init_refname(char *refname,int len )
#else
static int init_refname( refname, len )
	char *	refname;
	int		len;
#endif
{
	char	*p ;

	DBGFLOW( "init_refname called\n" );
	if( (p = getenv( DTM_REFNAME )) == NULL )
	{
		DTMerrno = DTMENV ;
		DTMERR( "init_refname: Env not setup" );
		return DTMERROR ;
	}
	strncpy( refname, p, (len - 1) );
	refname[ len - 1 ] = '\0' ;

	DBGINT( "init_refname: user process reference name is %s\n",
		 refname );
	return DTM_OK;
}

/*
	Function to return process reference name.
*/

#ifdef DTM_PROTOTYPES
char    *dtm_get_refname(void )
#else
char    *dtm_get_refname()
#endif
{
	static	char	refname[ REFNAMELEN ] = {0};

	if( refname[0] == '\0' ) init_refname( refname, REFNAMELEN ) ;
	return refname ;
}


/*
	I am not sure that this function still works, so it is
	not in the header
*/
#ifdef DTM_PROTOTYPES
void dtm_display_buf(Outport *outp,char *portname )
#else
void dtm_display_buf( outp, portname )
	Outport		*outp ;
	char		*portname ;
#endif
{
	DBGINT( "dtm_display_buf: Logical port %s\n", portname );
	while ( outp != NULL );
	{
		DBGINT( "dtm_display_buf: Nethostid = %x\n", 
			ntohl( outp -> sockaddr.sin_addr.s_addr) );
		DBGINT( "dtm_display_buf: Portid = %d\n", 
			ntohs( outp -> sockaddr.sin_port) );
		outp = outp->next;	
	}
}


/*
	dtm_new_out_port()
	Allocate and initialize a new outport.
*/
#ifdef DTM_PROTOTYPES
Outport *   dtm_new_out_port(Port *port )
#else
Outport *   dtm_new_out_port( port )
	Port	* port;
#endif
{	
	Outport	* p;

	if ( (p = (Outport *) malloc( sizeof( Outport ))) == NULL ) {
		DTMerrno = DTMMEM;
		return (Outport *)DTMERROR;
	}
	memset(p,0,sizeof(Outport));
/*
#ifdef SOLARIS
	memset(p,0,sizeof(Outport));
#else
	bzero( p, sizeof( Outport ));
#endif
*/
	DBGFLOW( "dtm_new_out_port called\n" );
	p -> sockaddr.sin_family = AF_INET ;
	p -> sockaddr.sin_port = port -> portid ;
	p -> sockaddr.sin_addr.s_addr = port -> nethostid ;
	p -> connfd = DTM_NO_CONNECTION ;
	p -> availwrite = FALSE ;
	p -> seqstart = FALSE ;

	return p;
}



#ifdef DTM_PROTOTYPES
static void del_nlist(DTMPORT *pp,int count )
#else
static void del_nlist( pp, count )
	DTMPORT	*pp;
	int		count;
#endif
{
	Outport	*	outp, * outpLast, * outpNext;
	Port		port;
	while( count-- ) {
		port.nethostid = inet_addr( strtok( NULL, COLON) );
		port.portid = (unsigned short)atol( strtok( NULL, SEP));
		outpLast = NULL;
		for ( outp = pp->out ; outp != NULL ; outp = outpNext ) {
			outpNext = outp->next;
			if ((outp->sockaddr.sin_port == port.portid ) && 
				(outp-> sockaddr.sin_addr.s_addr == port.nethostid)) { 
					if ( outpLast==NULL ) pp->out = outp->next;
						else outpLast->next = outp->next;
					free( outp );
				}
			else outpLast = outp;
		}
	}
}

#ifdef DTM_PROTOTYPES
static void add_nlist(DTMPORT *pp,int count )
#else
static void add_nlist( pp, count )
	DTMPORT	*pp;
    int     count;
#endif
{
	Port	port;
	Outport	* outp;
	while( count-- ) {
		port.nethostid = inet_addr( strtok(NULL, COLON));
		port.portid = (unsigned short)atol( strtok( NULL, SEP));
		outp = dtm_new_out_port( &port );
		if ( pp->out != NULL ) outp->next = pp->out;
		pp->out = outp;
	}
}

/*
	dtm_check_server()
	Service messages from the server, including the new route list.
	We only wait if we were asked to and we have not gotten ANY list yet.
	Returns the number of added ports.
*/
#ifdef DTM_PROTOTYPES
int dtm_check_server( DTMPORT *pp,int fWait )
#else
int dtm_check_server( pp, fWait )
	DTMPORT	*pp;
	int		fWait;
#endif
{
	char	*portname = pp->portname;
    int32	tmp;
	char	mbuf[MAX132];
	int		addcount;

	/* If it is not a logical port is is not listed with the server */
		
	if ( !pp->fLogical ) return DTM_OK;
	
	/* Check for new or first routing list */

	fWait = fWait && (pp->out == NULL);
	if( dtm_select( pp->sockfd, &tmp, !fWait ? 0:DTM_WAIT_TIMEOUT ) ==FALSE){
		if ( !fWait ) return 0;
		DBGFLOW( "dtm_get_nlist: timeout waiting on server\n") ;
		DTMerrno = DTMTIMEOUT;
		return DTMERROR;
	}

	while ( dtm_select( pp->sockfd, &tmp, 0 ) ) {

		/* Get the new server message */

		if( dtm_recv_header( pp->sockfd, mbuf, MAX132 ) == DTMERROR ) {
			DBGFLOW( "dtm_get_nlist: No Mport msg from name server\n") ;
			DTMerrno = DTMPORTINIT;
			return DTMERROR;
		}

		/* Ack the route message */
#if 0
		CHECK_ERR( dtm_nsend_ackroute( portname )); 
#endif

		/* Process the routing message */
			
		{
			char * msg_type = strtok( mbuf, SEP );
			if (!strcmp( msg_type, MROUTEID)) {
				int delcount = atoi( strtok( NULL, SEP ));
				DBGMSG1( "dtm_get_nlist: got routing: %s\n", mbuf ) ;
				addcount = atoi( strtok( NULL, SEP ));
				del_nlist( pp, delcount );
				add_nlist( pp, addcount );
				pp->fGotList = TRUE;
				return addcount;
			} else if ( !strcmp( msg_type, MDISCARD ) ) {
				pp->fDiscard = atoi( strtok( NULL, SEP));
			} 
		}
	}
	return 0;
}
