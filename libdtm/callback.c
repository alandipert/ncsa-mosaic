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


#include <sys/types.h>
#if !defined(NEXT) && !defined(_ARCH_MSDOS)
#include <unistd.h>
#endif
#ifdef __hpux
#include <termio.h>
#endif
#include <fcntl.h>
#ifdef _ARCH_BSD
#include <sys/filio.h>
#endif
#ifdef _ARCH_MSDOS
# include <signal.h>
#else
# include <sys/signal.h>
#endif
#include <sys/file.h>
#include <stdio.h>
#include "dtmint.h"
#include "debug.h"

#ifdef DTM_PROTOTYPES
static void DTMsigioHandler(int sig,int code,struct sigcontext *scp,char *addr )
#else
static void DTMsigioHandler( sig, code, scp, addr )
	int					sig;
	int					code;
	struct sigcontext 	*scp;
	char 				*addr;
#endif
{
	/* 
		Unfortunately, not one of the parameters listed above
		provides even the slightest help in determinine WHICH
		port is now ready for input, and any system calls
		screw up any other system calls in progress.
	*/
	reg		int		i;

/* DBGMSG2( "DTMsigioHandler enter %X %X\n", code, addr); */

	for ( i = 0 ; i < DTMptCount ; i++ ) {
		int	port;
		int	ready;
		if ( !DTMpt[i] ) continue;
		if ( !DTMpt[i]->callback ) continue;
		port = i;
		dtm_map_port_external(&port);
#if 0 
		if ((ready = DTMavailRead( port ))== DTMERROR) continue;
		if ( ready == DTM_PORT_READY ) {
			DBGMSG( "DTMsigioHandler calling user routine\n" );
#endif
			(*DTMpt[i]->callback)();
#if 0
		}
#endif
	}
/* DBGMSG( "DTMsigioHandler exit\n" ); */
}

#ifdef DTM_PROTOTYPES
int dtm_sigio( int fd )
#else
int dtm_sigio( fd )
	int		fd;
#endif
{
	int		flags;
        int             pid=getpid();
        int             sigio_on=1;
	DBGMSG1( "dtm_sigio on fd %d\n", fd );
#ifdef __hpux
	if (flags = ioctl( fd, FIOSSAIOOWN, &pid) == -1 ) {
#else
	if (flags = fcntl( fd, F_SETOWN, getpid()) == -1 ) {
#endif
		DTMerrno = DTMSOCK;
		return DTMERROR;
	}	
#ifdef __hpux
	if (flags = ioctl( fd, FIOSSAIOSTAT, &sigio_on ) == -1 ) {
#else
  	if (flags = fcntl( fd, F_SETFL, FASYNC ) == -1 ) {
#endif
		DTMerrno = DTMSOCK;
		return DTMERROR;
	}	
	return DTM_OK;
}

#ifdef DTM_PROTOTYPES
int DTMreadReady( int port, void (*pfn)() )
#else
int DTMreadReady( port, pfn )
	int32				port;
	void 				(*pfn)();
#endif	
{
	DTMPORT * pp;

	DBGMSG1( "DTMreadReady on port %d\n", port );
	CHECK_ERR( port = dtm_map_port_internal( port ));
	pp = DTMpt[port];

	/*
		Just replace the function
	*/
	if ( pp->callback ) {
		pp->callback = pfn;
		return DTM_OK;
	}

	if ( pp->porttype != INPORTTYPE ) {
		DTMerrno = DTMBADPORT;
		return DTMERROR;
	}	
	DBGMSG1( "DTMreadReady port has sockfd %d\n", pp->sockfd );
#ifndef _ARCH_MSDOS
	if ( (int)signal( SIGIO, DTMsigioHandler) == -1 ) {
		DBGMSG( "DTMreadReady signal failed\n" );
		DTMerrno = DTMSOCK;
		return DTMERROR;
	}	
#endif
	pp->callback = pfn;
	{
		reg	Inport	*inp;
		if( dtm_sigio( pp->sockfd )== DTMERROR) {
			DTMerrno = DTMSOCK;
			return DTMERROR;
		}	
		FOR_EACH_IN_PORT( inp, pp ) {
			if (dtm_sigio( inp->fd )== DTMERROR) {
				DTMerrno = DTMSOCK;
				return DTMERROR;
			}	
		}
	}
	return DTM_OK;
}
