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
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/ninit.c,v 1.2 1995/10/13 06:33:18 spowers Exp $
**
**********************************************************************/
/*
#ifdef RCSLOG

 $Log: ninit.c,v $
 Revision 1.2  1995/10/13 06:33:18  spowers
 Solaris support added.

 Revision 1.1.1.1  1995/01/11 00:03:01  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:40:06  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:32  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:13  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:31  marca
 * I think I got it now.
 *
 * Revision 1.7  92/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 * 

#endif
*/
/*

	Purpose	: Functions to initialise name server address, fd and to
		  to return it. 
*/

#include <stdlib.h>
#include <sys/types.h>
#ifdef _ARCH_MSDOS
#include <nmpcip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <string.h>
#include "dtmnserv.h"
#include "dtmint.h"
#include "debug.h"

static	struct	sockaddr_in	nsaddr ;	/* name server's address */
static	int	nssockfd = -1 ;			/* name server's socket */
static	char	nameserver[ MAX132 ] ;		
				/* 
					   name server address -
					   dotted decimal: port number 
				*/

/*
	Function to initialise the name server's address by
	looking up the environment variable.

	Return	values	:	DTMERROR on error.
						DTM_OK  on success.
	Notes		:
				Environment variable format : 
				
				DTM_NAMESERVER=nethostid:portid
			
				e.g. DTM_NAMESERVER=141.142.221.66:9900
*/
#ifdef DTM_PROTOTYPES
int dtm_ninit(void )
#else
int dtm_ninit()
#endif
{
	char	*p ;

	DBGFLOW( "dtm_ninit called\n" );
	if( (p = getenv( DTM_NAMESERVER )) == NULL ) {
		DTMerrno = DTMENV ;
		DTMERR( "dtm_ninit: Env not setup" );
		return DTMERROR ;
	}
	
	/*	Initialise name server's address, used in send() */

	strncpy( nameserver, p, MAX132 ); 

	DBGINT( "dtm_ninit: Nameserver is %s\n", nameserver );

	nsaddr.sin_family = AF_INET ;
	{
		char *portstr;

		portstr  = strchr( p, ':' );
		if ( portstr == NULL ) {
			DTMerrno = DTMADDR;
			return DTMERROR;
		}

		*portstr++ = '\0';

		nsaddr.sin_addr.s_addr = inet_addr( p ) ; 
		nsaddr.sin_port = (unsigned short)atol( portstr ) ;

		DBGMSG1("dtm_ninit: Nethostid = %x\n", ntohl( nsaddr.sin_addr.s_addr) );
		DBGMSG1("dtm_ninit: Portid = %d\n", ntohs( nsaddr.sin_port) ); 
	}

	/*	Acquire socket to be used for sending to name server  */

	if( (nssockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP )) == -1 ){
		DTMerrno = DTMSOCK ;
		DBGFLOW( "dtm_ninit: Socket call fails" );
		return DTMERROR;
	}

	return DTM_OK;
}

/*
	Function to return name server's address and associated socket fd.
*/

#ifdef DTM_PROTOTYPES
char    *dtm_get_naddr(struct sockaddr_in *addr,int *sockfd )
#else
char    *dtm_get_naddr( addr, sockfd )
struct	sockaddr_in	*addr ;
int	*sockfd ;
#endif
{
	if( nssockfd < 0 )  if ( dtm_ninit() == DTMERROR)
		return (char *) DTMERROR;

	*addr = nsaddr ;
	*sockfd = nssockfd ;

	return (nssockfd < 0) ? NULL : nameserver ;
}
