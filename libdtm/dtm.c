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

/***************************************************************************
**
** dtm.c	Contains the user routines for the DTM library.  These
**			routines provide inter-machine message passing for IPC.
**		Some of the features include:
**		  - automatic type conversion
**		  - synchronous and asynchronous operation
**		  - flexible message format, which is user configurable
**		  - high-level message types (SDS, RIS, SDL)
**
***************************************************************************/

/*
 * $Log: dtm.c,v $
 * Revision 1.5  1996/06/06 19:47:32  spowers
 * Linux is brain ded.
 *
 * Revision 1.4  1996/02/18 23:40:10  spowers
 * PROTO -> DTM_PROTO
 *
 * Revision 1.3  1995/10/14 22:07:23  spowers
 * Bzero and Bcopy removed...memset memcpy used instead.
 *
 * Revision 1.2  1995/10/13 06:33:05  spowers
 * Solaris support added.
 *
 * Revision 1.1.1.1  1995/01/11 00:02:58  alanb
 * New CVS source tree, Mosaic 2.5 beta 4
 *
 * Revision 2.5  1994/12/29  23:39:36  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:30  alanb
 *
 * Revision 1.2  1993/10/29  03:46:46  marca
 * Tweaks.
 *
 * Revision 1.1.1.1  1993/07/04  00:03:10  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:13  marca
 * I think I got it now.
 *
 * Revision 1.46  92/05/05  22:27:50  jplevyak
 * Corrected X interface code.
 * 
 * Revision 1.45  1992/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 *
 * Revision 1.44  1992/04/29  21:55:22  jplevyak
 * Add new function DTMgetConnectionCount.  Add support for DTMaddInput.
 *
 * Revision 1.43  1992/03/20  21:14:40  jplevyak
 * Logical outports are now deleted from the list when the connection
 * is dropped instead of trying to reconnect.
 *
 * Revision 1.42  1992/03/19  00:21:31  jplevyak
 * Use disconnect_out_port instead of just dropping the connection in
 * availWrite.  Fix Fix.
 *
 * Revision 1.41  92/03/18  22:25:44  jplevyak
 * Fix DTMEOF bug with absolute addressed.
 * 
 * Revision 1.40  1992/03/16  20:38:36  creiman
 * Added #include "arch.h"
 *
 * Revision 1.39  1992/03/16  17:30:07  jplevyak
 * Fix selectRead to work with outports.
 *
 * Revision 1.38  1992/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 *
 * Revision 1.37  1992/03/02  18:29:41  jplevyak
 * Fixed bug in EAGAIN handling.
 *
 * Revision 1.36  1992/03/02  17:20:14  jplevyak
 * Temporary back out.
 *
 * Revision 1.34  1992/02/28  03:40:24  jplevyak
 * Enhancement for ASYNC.  int/long conflict (makes no difference on workstations).
 *
 * Revision 1.33  1992/02/27  23:41:46  jplevyak
 * Ensure that the seqstart flag is always set when the seq start
 * message is sent.  Return an error if endWrite is called before
 * begin write.
 * Fix bug in writeMsg.
 *
 * Revision 1.32  1992/02/18  21:33:53  jefft
 * DTMerrno was not being cleared at the start of DTMavailRead.  Now it is.
 *
 * Revision 1.31  1992/02/18  14:01:39  jplevyak
 * Added DTMaddInPortSocket. Use _ARCH_MACOS for macintosh.
 *
 * Revision 1.30  1992/01/30  19:27:01  jplevyak
 * Don't write ack to dropped connections.  Use timeout of -1 as
 * an infinite wait to DTMselectRead.
 *
 * Revision 1.29  92/01/25  14:44:04  jplevyak
 * Fixed minor bug with DTM_ASYNC and another in AvailWrite
 * 
 * Revision 1.28  1992/01/14  19:40:09  creiman
 * #ifndef macintosh for accept_read_connections call to dtm_sigio
 *
 * Revision 1.27  1992/01/14  19:35:47  creiman
 * Removed malloc.h from mac version
 * Made clear_write_flags return DTMNOERR
 *
 * Revision 1.26  1992/01/14  16:31:40  creiman
 * Removed mac #include
 *
 * Revision 1.25  1992/01/02  21:06:13  jefft
 * Whoops.  Checked-in with a bug in DTMcheckRoute.  Fixed it!
 *
 * Revision 1.24  1992/01/02  20:43:55  jefft
 * Modified DTMcheckRoute when used with absolute port address.  It now
 * returns TRUE the first time called and FALSE there after.
 *
 * Revision 1.23  1991/12/09  18:36:18  jplevyak
 * Added support for Callback ( DTMreadReady ).
 *
 * Revision 1.22  1991/11/22  21:30:06  jplevyak
 * Added handling for fDiscard flag which allows the server to
 * tell a writer to send the data to /dev/null
 *
 * Revision 1.21  1991/11/15  19:42:34  jplevyak
 * Fixed Writing so that that connections are dropped silently on EOF.
 *
 * Revision 1.20  1991/11/01  23:07:55  jplevyak
 * New version of DTMselectRead.
 *
 * Revision 1.19  1991/11/01  22:42:10  jplevyak
 * Testing a new DTMselectRead.
 *
 * Revision 1.18  1991/11/01  21:11:30  jplevyak
 * More fixes for physical ports.
 *
 * Revision 1.17  1991/11/01  20:42:14  jplevyak
 * Fixed bug with physical addressing.
 *
 * Revision 1.16  1991/10/29  22:05:15  sreedhar
 * #include <malloc.h> added
 *
 * Revision 1.15  1991/10/29  16:37:41  jplevyak
 * Made DTMreadSelect retry when it gets a new connnection.
 *
 * Revision 1.14  1991/10/15  18:19:55  jplevyak
 * Reorder some functions to make the SGIs happy.
 *
 * Revision 1.13  91/10/14  16:46:18  jplevyak
 * Added loop for detecting dropped connections during beginRead.
 * 
 * Revision 1.12  1991/10/11  20:21:50  jplevyak
 * Fixed bug with multiple senders one receiver.
 * Added function DTMcheckRoute.
 *
 * Revision 1.11  1991/10/10  14:22:36  jplevyak
 * Complete major rework.  Complete testing when the test
 * suite is done.
 *
 * Revision 1.10  91/09/26  20:22:43  jplevyak
 * First stage of reorganization.  Use external/internal port mapping.
 * Use repackaged dtm_get_nlist and select_one.  Fix bugs with availWrite.
 * 
 * Revision 1.9  1991/09/13  17:34:10  sreedhar
 * changes for DTMSYNC, DTMNOSYNC quality of service
 *
 * Revision 1.7  1991/08/15  18:57:06  sreedhar
 * Changes for logical portname version
 *
 * Revision 1.5  1991/06/11  15:19:04  sreedhar
 * WRITEMSG option - sequence start, end of message also put in same writev call.
 *
 * Revision 1.4  1991/06/07  15:58:10  sreedhar
 * Changes for "Sequence start" message
 *
 * Revision 1.3  1991/05/30  15:51:31  sreedhar
 * Changes for readMsg/writeMsg internal release
 *
 * Revision 1.2  1990/12/19  17:28:50  jefft
 * DTMerrno is now cleared at the beginning of each DTM routine.  This
 * allows application to continue when possible.
 *
 * Revision 1.1  90/11/08  16:12:20  jefft
 * Initial revision
 * 
 */

/*
	Overview of DTM message transfer protocol.

	1. TCP connection is established between sender and receiver.

	2. Sender sends the "sequence starts" message.

	3. Sender awaits "ack from seq start" from the receiver.
	   Receipt of ack by sender guarantees the sender that
	   receiver will definitely accept at least the first user message 
	   sent by the sender.  Sender can then send as many user messages
	   as it wants to ( they would be accepted by receiver ).

	4. Sender sends the user's header message and user data messages. 

	3. Receiver will keep accepting user messages on current 
	   connection unless a new connection request is received, which
	   would be accepted after bumping the current connection.

	4. Sender would send "Messages over" message after it sends all user
	   messages.  Receiver would accept same.

	Graphic picture 

	Sender			Receiver

		Connect request			
		-------------->			

		Sequence starts		|
		-------------->		|
					|
		Ack for seq start	|
		<----------------	|
					|	
		User header		|	
		-------------->		|
					|	--> a sequence of
					|	    BEGINWRITE,
					|	    WRITEDATASET,
		User message 1		|	    ENDWRITE
		-------------->		|
		...............		|	    or equivalently,
		...............		|	    WRITEMSG
					|
		User message n		|
		-------------->		|
					|
		Messages over		|
		-------------->		|
					|

		...............			More sequences as above
		...............


	A "sequence starts" message can be sent in availWrite or 
	beginWrite. 

	When no "Ack for header" is received or a write fails.

	Note that the "ack for header", "message over" and "sequence starts"
	messages are called called "ack" in DTM terminology ( send_ack, 
	recv_ack calls used for all these ).
*/


#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "arch.h"
#if defined(_ARCH_MSDOS)
#include <time.h>
#include <io.h>
#include "uio.h"
#else
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#endif

#ifdef SOLARIS
#include <sys/filio.h>
#endif

#ifdef	RS6000
#include <sys/select.h>
#endif

#ifndef _ARCH_MACOS
# if defined( _ARCH_CONVEX ) || defined( NEXT )
#  include <sys/malloc.h>
# else
#  include <malloc.h>
# endif
#endif

#define	MAIN
#include "dtm.h"
#include "dtmint.h"
#include "debug.h"

#define	DTM_IOV_WRITE_SIZE		6
typedef struct {
	struct	iovec	iovec[ DTM_IOV_WRITE_SIZE ];
	int32				iovsize;
	int32				iovlen;
	int32				rts_data;
	int32				hdr_size;
	int32				data_size;
	int32				end_data;
} IOV_BUF;

unsigned int	uDTMdbg;

#ifdef DTM_PROTOTYPES
/*
	STATIC FUNCTION PROTOTYPES
*/
static int	destroy_out_port DTM_PROTO(( DTMPORT *pp, Outport **));
static int  select_one_connection DTM_PROTO(( int ));
static Inport *  new_in_port DTM_PROTO(( DTMPORT *pp,int fd ));
static int32 select_one DTM_PROTO(( int connfd ));
static Inport * inc_in_port DTM_PROTO(( DTMPORT *pp, Inport *inp ));
static void inc_nextToRead DTM_PROTO(( DTMPORT *pp ));
static int send_cts DTM_PROTO(( DTMPORT *pp, int fWait ));
static int accept_one_header DTM_PROTO(( DTMPORT *pp, void *header, int size ));
static int make_out_connections DTM_PROTO(( DTMPORT *pp ));
static void make_write_iov DTM_PROTO(( IOV_BUF *iov, int fStartSeq, int fEndSeq,
				char *hdr, int hdrsize, VOIDPTR data, int datasize ));
static int outp_count DTM_PROTO(( DTMPORT *pp ));
static int writev_buffer DTM_PROTO(( DTMPORT *pp, IOV_BUF *iov_buf,
				int fStartSeq ));
static int check_header_write_ack DTM_PROTO(( DTMPORT *pp ));
static int verify_out_connections DTM_PROTO(( DTMPORT *pp ));
static int clear_write_flags DTM_PROTO(( DTMPORT *pp ));
#endif

/*
	destroy_out_port()
	destroy an out port, including status flags.
*/
#ifdef DTM_PROTOTYPES
static int destroy_out_port( DTMPORT *pp, Outport **ppcur )
#else
static int	destroy_out_port( pp, ppcur )
	DTMPORT *	pp;
	Outport **	ppcur;
#endif
{
	static	Outport		dummyForLoops;
	Outport		*		pcur = *ppcur;
	/*
		What should we do with a close error here?
		Send to the server...?
			-john
	*/
	(void) dtm_end_connect( pcur->connfd );
	pcur->seqstart		= FALSE ;
	pcur->availwrite	= FALSE ;
	pcur->connfd		= DTM_NO_CONNECTION ;

	/*
		Delete the port from the list
	*/
	if ( pp->out == pcur )
		pp->out = pcur->next;
	else {
		Outport * outp;
		FOR_EACH_OUT_PORT( outp, pp ) {
			if ( outp->next == pcur ) {
				outp->next = pcur->next;
				break;
			}
		}
	}

	/*
		Most (if not all) of the users for this function
		will have it embedded in a FOR_EACH_OUT_PORT loop.
		In order that this loop is not endangered, the loop
		counter is temporarily redirected to dummyForLoops
		(a copy of the outport.
	*/
	dummyForLoops = *pcur;
	*ppcur = &dummyForLoops;

	free( pcur );

	if ( !pp->fLogical ) {
		DTMerrno = DTMEOF;
		return DTMERROR;
	}
	return DTM_OK;
}

/*
	outp_count()
	Returns the number of outports attached to the given DTM port
*/
#ifdef DTM_PROTOTYPES
static int outp_count(DTMPORT *pp )
#else
static	int	outp_count( pp )
	DTMPORT	*pp;
#endif
{
	int	i = 0;
	Outport * inp;
	FOR_EACH_OUT_PORT( inp, pp ) {
		i++;
	}
	return i;
}

/*
	make_out_connections()
	For each new out port, attmpt to make the connection.
*/
#ifdef DTM_PROTOTYPES
static  int make_out_connections(DTMPORT *pp )
#else
static	int make_out_connections( pp )
	DTMPORT	*pp;
#endif
{
	Outport	* pcur;

	FOR_EACH_OUT_PORT( pcur, pp ) {
		if( pcur->connfd == DTM_NO_CONNECTION) {
			if( dtm_connect( &pcur->sockaddr, &pcur->connfd ) 
					== DTMERROR ) {
				DBGFLOW( "make_out_connections: dtm_connect fails \n" );
				return DTMERROR;
			}
		} 
	}
	return DTM_OK;
}

/*
	clear_write_flags()
	Reset the sequence flags on write ports.
*/
#ifdef DTM_PROTOTYPES
static  int clear_write_flags(DTMPORT *pp )
#else
static	int clear_write_flags( pp )
	DTMPORT	*pp;
#endif	
{
	Outport	* pcur;

	FOR_EACH_OUT_PORT( pcur, pp ) {
		pcur->seqstart = FALSE ;
		pcur->availwrite = FALSE ;
	}
	return DTMNOERR;
}

/*
	verify_out_connections()
	Ensure that their is a connection on each out port.
*/	
#ifdef DTM_PROTOTYPES
static  int verify_out_connections(DTMPORT *pp )
#else
static	int verify_out_connections( pp )
	DTMPORT	*pp;
#endif
{
	Outport	* pcur;

	FOR_EACH_OUT_PORT( pcur, pp ) {
		if( pcur->connfd == DTM_NO_CONNECTION) {
			DTMerrno = DTMPORTINIT;
			return DTMERROR;
		} 
	}
	return DTM_OK;
}

/*
	check_header_write_ack()
	!!!! Check to see whether a header write acknowledge is required,
		or has come and read it.
	Since there are no more header ack, this function clears the
		CTS.
*/
#ifdef DTM_PROTOTYPES
static  int check_header_write_ack(DTMPORT *pp )
#else
static	int check_header_write_ack( pp )
	DTMPORT	*pp;
#endif
{
	Outport	* pcur;

	FOR_EACH_OUT_PORT( pcur, pp ) {
		if ( pcur->connfd == DTM_NO_CONNECTION ) continue;
		if( !( pcur->availwrite ) ) {
			int32	tmp;
			if( (pp->qservice == DTM_SYNC) || ((pp->qservice == DTM_ASYNC) && 
				  (dtm_select( pcur->connfd, &tmp, 0 ) == TRUE && tmp >= 4))) {
				do {
					int temp;
					if( dtm_recv_ack( pcur->connfd, &temp ) == DTMERROR) {
						DBGFLOW( "Incorrect ack for header\n" );
						CHECK_ERR( destroy_out_port( pp,  &pcur ));
						/*
							Do we return an error here?
							Do we unlink the port?
							return DTMERROR ;
						*/
					}
				} while ( dtm_select( pcur->connfd, &tmp, 0 ) == TRUE &&
					tmp >= 4);	
				pcur->availwrite = TRUE;
			}
		}
	}
	return DTM_OK;
}


/*
	make_write_iov()
	returns: The iov[0] = start seq, iov[1,2] = headersize, header
		iov[3,4] = datasetsize, dataset iov[5] = end seq
*/
#define	START_SEQ				TRUE
#define	NO_START_SEQ			FALSE
#define	END_SEQ					TRUE
#define	NO_END_SEQ				FALSE
#ifdef DTM_PROTOTYPES
static void make_write_iov(IOV_BUF *iov,int fStartSeq,int fEndSeq,char *hdr,
		int hdrsize,VOIDPTR data,int datasize )
#else
static void make_write_iov( iov, fStartSeq, fEndSeq, hdr, hdrsize, 
		data, datasize )
	IOV_BUF			*iov;
	int				fStartSeq;
	int				fEndSeq;
	char			*hdr ;
	int				hdrsize ;
	VOIDPTR			data ;
	int				datasize ;
#endif
{
	int				status ;
	int				i ;

	/*	Send "Sequence starts" message	*/

	i = 0 ;
	iov->iovsize = 0;

	if ( fStartSeq ) {
		DBGMSG( "make_write_iov: making start seq\n" );
		iov->rts_data = DTM_RTS;	
		STDINT( iov->rts_data );
		iov->iovec[ i ].iov_base = (char *)&iov->rts_data ;
#define	SEQ_START_LEN	4
		iov->iovec[ i ].iov_len = SEQ_START_LEN;
		i += 1 ;
		iov->iovsize += 4;
	}

	/*	Prepare header size and header	*/

	if ( hdrsize != 0 ) {
		DBGMSG( "make_write_iov: making header\n" );
		iov->hdr_size = hdrsize ;
		STDINT( iov->hdr_size ); 
		iov->iovec[ i ].iov_base = (char *)&iov->hdr_size ;
		iov->iovec[ i ].iov_len = 4 ;
		i += 1 ;
		iov->iovec[ i ].iov_base = hdr ;
		iov->iovec[ i ].iov_len = hdrsize ;
		i += 1 ;
		iov->iovsize += 4 + hdrsize;
	}

	/* 	Prepare data size and data	*/

	if ( datasize != 0 ) {
		DBGMSG( "make_write_iov: making dataset\n" );
		iov->data_size = datasize ;
		STDINT( iov->data_size );

		iov->iovec[ i ].iov_base = (char *)&iov->data_size ;
		iov->iovec[ i ].iov_len = 4 ;
		i += 1 ;
		iov->iovec[ i ].iov_base = (char *)data ;
		iov->iovec[ i ].iov_len = datasize ;
		i += 1 ;
		iov->iovsize += 4 + datasize;
	}

	if ( fEndSeq ) {
		/* End sequence */
		DBGMSG( "make_write_iov: making endseq\n" );
		iov->end_data = DTM_EOT ;
		STDINT( iov->end_data );
		iov->iovec[ i ].iov_base = (char *)&iov->end_data ; 
		iov->iovec[ i ].iov_len = 4 ; 
		i += 1 ;
		iov->iovsize += 4;
	}
	iov->iovlen = i;
}

#ifdef DTM_PROTOTYPES
static int  writev_buffer(DTMPORT *pp,IOV_BUF *iov_buf,int fStartSeq )
#else
static int	writev_buffer( pp, iov_buf, fStartSeq )
	DTMPORT		*	pp;
	IOV_BUF		*	iov_buf;
	int				fStartSeq;
#endif
{
	Outport		*	pcur;
	struct iovec *	iov;
	int32			iovlen;
	int32			iovsize;

	FOR_EACH_OUT_PORT( pcur, pp ) {
		int status;

		if ( pcur->connfd == DTM_NO_CONNECTION )
			continue;

		iov = &iov_buf->iovec[0];
		iovlen = iov_buf->iovlen;
		iovsize = iov_buf->iovsize;

		if ( fStartSeq ) {
			if (pcur->availwrite || pcur->seqstart ) {
				DBGMSG1( "writev: dropping start seq = %x\n", 
						(pcur->availwrite?1:0) | (pcur->seqstart?10:0));
				/* we have already sent the sequence start, skip it */
				iov++;
				iovsize -= SEQ_START_LEN;
				iovlen -= 1;
			} 
			pcur->seqstart = TRUE;	
		}
		DBGMSG1( "writev_buffer: iovlen = %d\n", iovlen );
		DBGMSG1( "writev_buffer: iovsize = %d\n", iovsize );
		DBGMSG1( "writev_buffer: ptr iov = %X\n", iov );
		DBGMSG1( "writev_buffer: first ptr word = %X\n", iov[0].iov_base );
		DBGMSG1( "writev_buffer: first word = %d\n", 
				*(int *)((iov[0]).iov_base));
		status = dtm_writev_buffer( pcur->connfd, iov, iovlen, iovsize,
				NULL, 0);

		DBGINT( "writev_buffer - status = %d\n", status); 

		if( status < 0 ) {		
			DBGINT( "dtm_writev_buffer - errno = %d\n", errno );
			if( DTMerrno == DTMEOF ) {
				CHECK_ERR( destroy_out_port(  pp, &pcur ));
				continue;
			}
			/*
				What do we do with the other errors?
				It is unclear whether we want to bail or not
						-john
				return DTMERROR;
			*/
		}
	}	/* end while	*/
	return DTM_OK;
}

/*
	select_one()
	Select on a single socket.
*/
#ifdef DTM_PROTOTYPES
static int32  select_one(int connfd )
#else
static int32  select_one( connfd )
	int	connfd;
#endif
{
	static struct	timeval	timeout = {0L, 0L};
	fd_set	readmask;
	int32	ret;

	FD_ZERO( &readmask );
	FD_SET( connfd, &readmask );

#ifdef __hpux
	ret = select( FD_SETSIZE, (int *)&readmask, (int *)0, (int *)0, 
#else
  	ret = select( FD_SETSIZE, &readmask, (fd_set *)0, (fd_set *)0, 
#endif
				&timeout );	
	if ( ret > 0 ) {
		int32		count;
		ioctl( connfd, FIONREAD, &count );
		DBGMSG1(  "select_one: got count = %d\n", count );
		ret = count; 
	}
	return ret;
}

/*
	select_one()
	Select on a single socket.
*/
#ifdef DTM_PROTOTYPES
static int  select_one_connection(int connfd )
#else
static int  select_one_connection( connfd )
	int	connfd;
#endif
{
	static struct	timeval	timeout = {0L, 0L};
	fd_set	readmask;
	int		ret;

	FD_ZERO( &readmask );
	FD_SET( connfd, &readmask );

#ifdef __hpux
	return select( FD_SETSIZE, (int *)&readmask, (int *)0, (int *)0, 
#else
  	return select( FD_SETSIZE, &readmask, (fd_set *)0, (fd_set *)0, 
#endif
				&timeout );	
}

/*
	new_in_ports()
	Add a new inport on a DTMPORT.
*/
#ifdef DTM_PROTOTYPES
static Inport *  new_in_port(DTMPORT *pp,int fd )
#else
static Inport *  new_in_port( pp, fd ) 
	DTMPORT *	pp;
	int			fd;
#endif	
{
	Inport * inp;

	if ( (inp = (Inport *) malloc( sizeof(Inport) )) == NULL ) {
		DTMerrno = DTMMEM;	
		return (Inport *) DTMERROR;
	}
	memset(inp,0,sizeof(Inport));
/*
#ifdef SOLARIS
	memset(inp,0,sizeof(Inport));
#else
	bzero( inp, sizeof(Inport) );
#endif
*/
	inp->fd 		= fd;
	inp->blocklen 	= DTM_NEW_DATASET;
	inp->fCTSsent	= FALSE;
#define	PUT_NEW_IN_PORTS_AT_END
#ifdef PUT_NEW_IN_PORTS_AT_END
	{
		Inport	* endp;
		endp = pp->in;	
		if ( endp == NULL )	
			pp->in = inp;
		else {
			while ( endp->next != NULL ) endp = endp->next;
			endp->next = inp;
		}	
	}	
#else
	inp->next 		= pp->in;
	pp->in			= inp;
#endif
	return inp;
}

#ifdef DTM_PROTOTYPES
void	dtm_handle_in( caddr_t client_data, int * fd, void * id)
#else
void	dtm_handle_in( client_data, fd, id )
	caddr_t		client_data; 
	int *		fd; 
	void *		id;
#endif
{
	int				p = (int) client_data;
	DTMPORT *		pp = DTMpt[p];
	int				p_ext = p;

	dtm_map_port_external( &p_ext );
	if ( DTMavailRead( p_ext ) == DTM_PORT_READY )
		pp->Xcallback( pp->Xcallback_data, &p_ext, id );
}

/*
	dtm_set_Xcallback

	This function may seem a little strange, after all why have a variable
	(pp->XaddInput) which has only one valid value (XtAddInput).  
	The problem is that we don't want to include the X libraries 
	unless we have to.  By using this variable which is only set 
	if the function that will cause this function to get called 
	is included... which causes the inclusion of the X libraries, we
	avoid the undefined external error.
*/

		/*	Change this if the test in x.c fails! */

#define	XtInputReadMask	(1L<<0)

#ifdef DTM_PROTOTYPES
void dtm_set_Xcallback( DTMPORT * pp, Inport * inp )
#else
void dtm_set_Xcallback( pp, inp )
	DTMPORT *   pp;
	Inport *	inp;
#endif
{
	/* you didn't see this */
	int	p; for ( p = 0; p < DTMptCount ; p++ ) if ( pp == DTMpt[p] ) break;
	if ( pp->porttype == INPORTTYPE && pp->XaddInput ) {
		inp->XinputId = pp->XaddInput( inp->fd, XtInputReadMask, 
				dtm_handle_in, (caddr_t) p);
	}
}

/*
	dtm_accept_read_connections()
		If fWait is TRUE:
			then if there are no connections
			wait for 2 minutes for a connection before failing.
			If there are some connections, add any new ones
			without waiting.
		If fWait is FALSE:
			Add any new connections without waiting.
*/
#ifdef DTM_PROTOTYPES
int  dtm_accept_read_connections(DTMPORT *pp,int fWait )
#else
int	dtm_accept_read_connections( pp, fWait )
	DTMPORT *	pp;
	int			fWait;	
#endif
{
	struct	timeval	timeout ;
	reg	int			fd;

	while ( TRUE ) {
		Inport	* inp;

		fWait = fWait && (pp->in == NULL);

		/* Do we have any waiting or will be wait? */

		if ( !fWait && (select_one_connection( pp->sockfd ) < 1 ))
			break;

		/* Wait only 2 minutes for the first connection request */

		timeout.tv_sec = 120 ;
		timeout.tv_usec = 0 ;

		/* No connection yet, await one and accept */

		DBGINT(  "dtm_accept_read_connection: pp -> sockfd = %d\n", 
			pp -> sockfd );
		if( (fd = dtm_accept( pp->sockfd, &pp->sockaddr, fWait ? &timeout : 0)) 
				== DTMERROR ) {
			if ( !fWait ) return DTM_OK;
			DTMerrno = DTMTIMEOUT ;
			return DTMERROR;
		}
		DBGINT(  "dtm_accept_read_connection: got fd = %d\n", fd );
		CHECK_ERR( inp = new_in_port( pp, fd )); 
#ifndef _ARCH_MACOS
		if ( pp->callback ) dtm_sigio( fd );
		if ( pp->Xcallback ) dtm_set_Xcallback( pp, inp );
#endif
	}

	/*
		If we have accepted new read connections, reset the
		nextToRead pointer.
	*/
	pp->nextToRead = pp->in;
	return DTM_OK;
}

/*
	DTMselectRead()
	Function to test 
	a) for existence of a new connection or a new 
	   message header to be read on a set of DTM ports OR
	b) for whether data is available to be read on a 
	   set of sockets.

	Return	values	:	
			DTM_PORT_READY		if at least a DTM port or
								socket has something to be
								read.
			DTM_PORT_NOT_READY	if no DTM port or socket
								has something to be read.
			DTMERROR			if select system call returns
								error.

			Each port has status field. Possible values
			for status field are - 

			DTM_PORT_READY		something available to be read.
			DTM_PORT_NOT_READY	nothing available to be read.
			DTMERROR			port not initialised.
*/
#ifdef DTM_PROTOTYPES
int DTMselectRead( Dtm_set *dtmset,int dtmnum,Sock_set *sockset,int socknum,
			int period )
#else
int	DTMselectRead( dtmset, dtmnum, sockset, socknum, period )
	Dtm_set		*dtmset ;		/* Pointer to set of DTM ports 	*/
	int			dtmnum ;		/* Number of DTM ports to be checked for */
	Sock_set	*sockset ;		/* Pointer to set of sockets	*/
	int			socknum ;		/* Number of sockets to be checked for	*/
	int			period ;
#endif
{
	fd_set		readmask ;
	fd_set		*fchk = &readmask ;
	int			nf ;
	int			index ;
	int			fReady;
	Dtm_set		*p1 ;
	Sock_set	*p2 ;
	struct	timeval	timeout ;
	int			fNewConnections;
	int			fFalsePositive;

	DBGFLOW( "DTMselectRead called\n" );
	timeout.tv_sec = period/1000 ;
	timeout.tv_usec = (period - (period/1000)*1000)*1000 ;

	do {
		fNewConnections = FALSE;
		fFalsePositive = FALSE;
		fReady = DTM_PORT_NOT_READY;

		FD_ZERO( fchk );

		/*	Set up DTM ports to be selected on	*/

		for( p1 = dtmset, index = 0 ; index < dtmnum ; index++, p1++ ) {
			reg	DTMPORT	*pp ; 
			int	port_internal; 
			reg	Inport	*inp;

			/* Select status is error if port entry is 
					not initialised. 
			*/
			if( (port_internal = dtm_map_port_internal( p1->port )) 
						== DTMERROR ) {
				p1->status = DTMERROR ;
				continue ;
			}
			pp = DTMpt[ port_internal ];

			CHECK_ERR( dtm_accept_read_connections( pp, DTM_DONT_WAIT));

			/* look for new connection request */

			FD_SET( pp -> sockfd, fchk );     	

			/* look for data in existing connection (if it exists) */
			FOR_EACH_IN_PORT( inp, pp ) {
				FD_SET( inp->fd, fchk );	
			}
			p1->status = DTM_PORT_NOT_READY ;
		}

		/*	Set up socket fds to be selected on	*/

		for( p2 = sockset, index = 0 ; index < socknum ; index++, p2++ ) {
			FD_SET( p2 -> sockfd, fchk );
			p2 -> status = DTM_PORT_NOT_READY ; 
		} 
#ifdef __hpux
		nf = select( FD_SETSIZE, (int *)fchk, (int *)0, (int *)0, 
#else
  		nf = select( FD_SETSIZE, fchk, (fd_set *)0, (fd_set *)0, 
#endif
			period < 0 ? NULL : &timeout );

		/* Select returns error	*/ 

		if( nf < 0 ) { 
			DBGINT( "DTMselectRead: select error %d \n", errno  ); 
			DTMerrno = DTMSELECT ; 
			return DTMERROR ; 
		}

		/* None of the DTM ports or sockets have anything to be read	*/

		if( nf == 0 ) {
			DBGFLOW( "DTMselectRead: Nothing to read\n" );
			return DTM_PORT_NOT_READY ;
		} 

		/*	Check whether any DTM port has something to be read */

		for( p1 = dtmset, index = 0 ; index < dtmnum ; index++, p1++ ) {
			reg		DTMPORT	*pp ;
			auto	int		port_internal;
			reg		Inport	*inp;

			if ((port_internal= dtm_map_port_internal( p1->port )) == DTMERROR)
				continue;
					
			pp = DTMpt[ port_internal ];
			if (pp->porttype == INPORTTYPE) {
				fNewConnections = fNewConnections ||
					(select_one_connection( pp->sockfd ) > 0);
				p1->status = DTM_PORT_NOT_READY;
			} else {
				if (select_one_connection( pp->sockfd ) > 0) 
					fReady = p1->status = DTM_PORT_READY;
				else p1->status = DTM_PORT_NOT_READY;
				continue;
			}
			inp = pp->in;
			while ( inp != NULL ) {
				if ( FD_ISSET( inp->fd, fchk )) {
					if ( select_one( inp->fd ) < 1 ) {
						dtm_destroy_in_port( inp, pp );
						fFalsePositive = TRUE;
						inp = pp->in;
						continue;
					}
					p1->status = DTM_PORT_READY;
				}	
				inp = inp->next;
			}
			if ( p1->status == DTM_PORT_READY ) fReady = DTM_PORT_READY;
		}

		/*	Check whether any socket has something to be read */

		for( p2 = sockset, index = 0 ; index < socknum ; index++, p2++ ) {
			p2 -> status = FD_ISSET( p2 -> sockfd, fchk ) ? 
				DTM_PORT_READY : DTM_PORT_NOT_READY ;
			if ( p2->status == DTM_PORT_READY ) fReady = DTM_PORT_READY;
		}

		DBGFLOW( "DTMselectRead done loop\n" );

	} while (!fReady && (fNewConnections || fFalsePositive));	

	return fReady ;
}

#ifdef DTM_PROTOTYPES
static Inport * inc_in_port(DTMPORT *pp,Inport *inp )
#else
static Inport *	inc_in_port( pp, inp )
	DTMPORT	* 	pp;
	Inport	* 	inp;
#endif	
{
	if ( inp == NULL || inp->next == NULL )
		return pp->in;
	else return inp->next;
}

#ifdef DTM_PROTOTYPES
static void inc_nextToRead(DTMPORT *pp )
#else
static void inc_nextToRead( pp )
	DTMPORT * 	pp;
#endif
{ 
	pp->nextToRead = inc_in_port( pp, pp->nextToRead );
}

/*
	dtm_destory_in_port()
	Close, unlink and delete an inport.
*/
int dtm_destroy_in_port( inp, pp )
	Inport *	inp;
	DTMPORT *	pp;
{
	DBGMSG1( "dtm_destroy_in_port on %d\n", inp->fd );

	if ( pp->Xcallback ) pp->XremoveInput( inp->XinputId );

	close( inp->fd );
	
	if ( pp->nextToRead == inp )
		inc_nextToRead( pp );
	if ( pp->nextToRead == inp )
		pp->nextToRead = NULL;

	if ( pp->in == inp )
		pp->in = inp->next;
	else {
		Inport * inpTemp;
		FOR_EACH_IN_PORT( inpTemp, pp ) {
			if ( inpTemp->next == inp ) {
				inpTemp->next = inp->next;
				break;
			}
		}
	}
#ifdef FREE_RETURNS_INT
	if ( free( inp ) != 1 ) {
		DBGMSG( "dtm_destroy_in_port free error\n" );
		DTMerrno = DTMCORPT;
		return DTMERROR;
	}
#else
	free( inp );
#endif
	DBGMSG( "dtm_destroy_in_port done\n" );
	return DTM_OK;
}

			

/*
	send_cts()
	Send CTS to the next port depending on the option DTMSendRTSAhead.
*/
#ifdef DTM_PROTOTYPES
static int send_cts(DTMPORT *pp,int fWait )
#else
static int send_cts( pp, fWait )
	DTMPORT * 	pp;
	int			fWait;
#endif	
{
	Inport * 	inp = pp->nextToRead;
	Inport * 	endp;
	int 		iSent = 0;

	DBGMSG( "send_cts: <[\n" );

	/*
		If we have no ports return OK 
	*/
	if ( inp == NULL ) inp = pp->in;
	if ( inp == NULL ) return DTM_OK;

	/*
		If we have no ports with RTS pending return OK
	*/
	endp = inp;
	while ( !inp->fCTSsent && (select_one( inp->fd ) < 1 ) ) {
		inp = inc_in_port( pp, inp );
		if ( inp == endp ) {
			if ( !fWait ) return DTM_OK;
				else break;
		}	
	}
	pp->nextToRead = inp;

	while ( iSent++ <= DTMSendCTSAhead ) {
		int32 tmp;
		/*
			If we need to send CTS and we are supposed to send
			it and we are on the first one or if it is ready, send it.
		*/
		DBGMSG1( "send_cts: while loop port %X\n", inp );
		DBGMSG1( "send_cts: while loop port fd %d\n", inp->fd );
		if ( !inp->fCTSsent && ((fWait && (iSent == 1 )) ||
				 (dtm_select( inp->fd, &tmp, 0 ) == TRUE && tmp >= 4))) {
			if ( dtm_send_ack( inp->fd, DTM_CTS ) == DTMERROR) {
				CHECK_ERR( dtm_destroy_in_port( inp, pp )); 
				/*
					Never hurts to start at the top.
				*/
				inp = pp->nextToRead;
					/*
						We just lost our last port
					*/
				if ( inp == NULL ) {
					DBGMSG( "send_cts: done ]>\n" );
					DTMerrno = DTMEOF;
					return DTMERROR;
				}
				iSent = 0;
				continue;
			}
			inp->fCTSsent = TRUE;	
			inp->blocklen = DTM_NEW_DATASET;
		}	
		inp = inc_in_port( pp, inp );
	}
	DBGMSG( "send_cts: ]>\n" );
	return DTM_OK;
}

#ifdef DTM_PROTOTYPES
static  int accept_one_header(DTMPORT *pp,void *header,int size )
#else
static	int	accept_one_header( pp, header, size )
	DTMPORT	*	pp;
	void *		header;		
	int			size;
#endif
{
	Inport * 	inp = pp->nextToRead;
	int			count;
	int			ack ;

	if ( inp == NULL || !inp->fCTSsent || inp->fGotHeader ) {
		DTMerrno = DTMCALL;
		return DTMERROR;
	}	

	DBGMSG1( "Accepting RTS on %d\n", inp->fd );
	if (dtm_recv_ack( inp->fd, &ack ) == DTMERROR ) {
		dtm_destroy_in_port( inp, pp ); 
		return DTMERROR;
	}
	if( ack != DTM_RTS ) {
		DTMerrno = DTMBADACK;
		DBGMSG1( "Something other than RTS received %d\n", ack );
		dtm_destroy_in_port( inp, pp ); 
		return DTMERROR;
	}
#if 0
	/*	There are no header ack */
	if ( dtm_send_ack( inp->fd, DTM_CTS ) == DTMERROR) {
		dtm_destroy_in_port( inp, pp ); 
		return DTMERROR;
	}
#endif
	DBGINT( "Accepting header on %d\n", inp->fd ) ;
	if( (count = dtm_read_header( inp->fd, header, size )) < 0 ) {
		DBGINT( "Recv header error = %d\n", errno );
		if( DTMerrno != DTMHEADER ) {
			dtm_destroy_in_port( inp, pp );
		}
		return DTMERROR;
	}
	inp->fGotHeader = TRUE;
	return count;
}

/*
	DTMcheckRoute()
	Check whether new routing information has come in.
	Returns: 
*/
#ifdef DTM_PROTOTYPES
int DTMcheckRoute(int port )
#else
int	DTMcheckRoute( port )
	int	port;
#endif
{

	CHECK_ERR( port = dtm_map_port_internal( port ));

	/* if logical port, check for routing message from server */
	if (DTMpt[port]->fLogical)
		return dtm_check_server( DTMpt[port], DTM_DONT_WAIT );

	/* if absolute port, return TRUE the first time and FALSE thereafter */
	else if (DTMpt[port]->fGotList)
		return FALSE;
	else
		return DTMpt[port]->fGotList = TRUE;
}

/*
	Function to test for existence of a new connection or
	a new message header to be read on a given DTM port.

	Return	values	:	TRUE		if either new connection 
						request or something new 
						to be read is available on
						given port.
				DTMERROR	on select error.	
				FALSE		otherwise.

	Notes	:

		DTMavailRead is basically call to DTMselectRead for 
		given port with 0 timeout.
*/
#ifdef DTM_PROTOTYPES
int DTMavailRead(int p )
#else
int	DTMavailRead( p )
	int	p ;
#endif
{
	Dtm_set		dtmset ;
	int			dtmnum ;
	Sock_set	sockset ;
	int			socknum ;
	int			fAnyReady;

	DBGFLOW( "DTMavailRead called\n" );
	DTMerrno = DTMNOERR;

	/* Note: the port here is an external port since that
			is what selectRead expects 						
	*/
	dtmnum 		= 1 ;
	dtmset.port = p ;
	socknum 	= 0 ;
	CHECK_ERR(fAnyReady = DTMselectRead( &dtmset,dtmnum,&sockset,socknum,0));
	DBGMSG1( "DTMselectRead returned %d\n", fAnyReady );
	CHECK_ERR( p = dtm_map_port_internal( p ));
	if ( fAnyReady ) {
		if ( send_cts( DTMpt[p], DTM_DONT_WAIT ) == DTMERROR ) {
            DBGMSG1( "DTMavailRead send_cts returned error %d\n", DTMerrno );
			if ( DTMerrno == DTMEOF ) fAnyReady = FALSE ;
				else return DTMERROR;
		}	
	}
	DBGMSG( "DTMavailRead done\n" );
	return fAnyReady;
}

/*
	Function to add a socket to a DTM inport.
*/
#ifdef DTM_PROTOTYPES
int   DTMaddInPortSocket(int p,int socket )
#else
int	DTMaddInPortSocket( p, socket )
	int		p;
	int		socket;
#endif
{
	CHECK_ERR( p = dtm_map_port_internal( p ));
	CHECK_ERR( new_in_port( DTMpt[p], socket ));
	return DTM_OK;
}

#ifdef DTM_PROTOTYPES
int DTMgetConnectionCount(int port,int *n_connections)
#else
int DTMgetConnectionCount(port, n_connections)
	int		port;
	int	* 	n_connections;
#endif
{
    int             count = 0;
    reg DTMPORT *   pp;

    CHECK_ERR( port = dtm_map_port_internal( port ));
    pp = DTMpt[port];

    if ( pp->porttype != INPORTTYPE ) {
        reg Outport *   pcur;
        FOR_EACH_OUT_PORT( pcur, pp ) {
            count++;
        }
    } else {
        reg Inport *    pcur;
        FOR_EACH_IN_PORT( pcur, pp ) {
            count++;
        }
    }
    return count;
}

/*
	Function to begin reading on a DTM port. The header of the
	message is read.

	Return	values	:	>= 0 		on success.
						DTMERROR	on some error.	
*/
#ifdef DTM_PROTOTYPES
int DTMbeginRead(int p,VOIDPTR header,int size )
#else
int	DTMbeginRead( p, header, size )
	int		p ;
	VOIDPTR	header ;
	int		size ;
#endif
{
	int			count = 0;
	reg DTMPORT	*pp;
	reg	Inport	*inp;

  	DBGFLOW( "DTMbeginRead called.\n" );
  	DTMerrno = DTMNOERR;

	CHECK_ERR( p = dtm_map_port_internal( p ));
	pp = DTMpt[p];
		
	while ( TRUE ) { 
		CHECK_ERR( dtm_accept_read_connections( pp, DTM_WAIT));
		if ( send_cts( pp, DTM_WAIT ) == DTMERROR) {
			if ( DTMerrno == DTMEOF ) continue;
			return DTMERROR;
		}
		if ( pp->nextToRead == NULL ) continue;
		DBGFLOW( "DTMbeginRead before accept_one_header.\n" );
		if (( count = accept_one_header( pp, header, size )) == DTMERROR ) { 
			if ( DTMerrno == DTMEOF ) continue;	
			return DTMERROR;
		}
		break;
	}

  	DBGFLOW( "DTMbeginRead done.\n" );
  	return count;
}

/*
	DTMreadDataset()
	Function to read user messages. 

	Return	values	: 	number of bytes read,	on success
				DTMERROR		on error
*/
#ifdef DTM_PROTOTYPES
int DTMreadDataset(int p,VOIDPTR ds,int size,DTMTYPE type)
#else
int DTMreadDataset(p, ds, size, type)
	int		p ;
	VOIDPTR	ds;
	int		size ;
	DTMTYPE	type;
#endif
{
	DTMPORT 	* pp;
	Inport		* inp;

  	DBGFLOW("DTMreadDataset called.\n");
	DTMerrno = DTMNOERR;

	CHECK_ERR( p = dtm_map_port_internal( p ));
	pp = DTMpt[p];
	inp = pp->nextToRead;

	if ( inp == NULL || !inp->fCTSsent || !inp->fGotHeader ) {
		DTMerrno = DTMCALL;
		DTMERR( "DTMreadDataset: DTMbeginRead required before DTMreadDataset.");
		return DTMERROR;
	}

  	/* determine max number of bytes that can be read */

	size = (*DTMconvertRtns[(int)type])(DTMSTD, NULL, size);

  	/* fill buffer from network */

	/*
		Assume that the caller has checked for EOT 
	*/
	CHECK_ERR( size = dtm_read_buffer( inp->fd, &inp->blocklen, ds, size));

  	/* convert dataset to local representation */

  	return (*DTMconvertRtns[(int)type])(DTMLOCAL, ds, size);
}

/*
	DTMendRead()
	Function to end reading user messages.
	Return	values	:	0		on no error
				DTMERROR	on error
*/
#ifdef DTM_PROTOTYPES
int DTMendRead(int p )
#else
int DTMendRead( p )
	int	p ;
#endif
{
	DTMPORT 	* pp;
	Inport		* inp;

	DBGFLOW("DTMendRead called.\n");
	DTMerrno = DTMNOERR;

	CHECK_ERR( p = dtm_map_port_internal( p ));
	pp = DTMpt[p];
	inp = pp->nextToRead;

  	/* check that DTMbeginRead has been called */

	if ( inp == NULL || !inp->fCTSsent || !inp->fGotHeader ) {
		DTMerrno = DTMCALL;
		DTMERR( "DTMendRead: DTMbeginRead required before DTMendRead.");
		return DTMERROR;
	}

  	/* discard any remaining data */

  	while ( dtm_read_buffer( inp->fd, &inp->blocklen,
      		dtm_discard, DISCARDSIZE) > 0 );
	inp->fCTSsent = FALSE;
	inp->fGotHeader = FALSE;
  	return DTM_OK ; 
}

/*
	Function to combine reading of header/data.
	
	Return	values	:	number of bytes read,	on success
				DTMERROR		on error	

	Notes	: This function is really there for completeness
		  sake ( it complements DTMwriteMsg ). It is not
		  very clear how a user can use it effectively
		  ( since he has to know data size beforehand,
		    in which case he need not have a header ).
		
		  Hence, implementation of this function is to 
		  just call beginRead, readDataset and endRead
		  in that order.
*/
#ifdef DTM_PROTOTYPES
int DTMreadMsg(int p,char *hdr,int hdrsize,VOIDPTR data,int datasize,
			int datatype )
#else
int	DTMreadMsg( p, hdr, hdrsize, data, datasize, datatype )
	int		p ;
	char	*hdr ;
	int		hdrsize ;
	VOIDPTR	data ;
	int		datasize ;
	int		datatype ;
#endif
{
	int		count;

	DTMerrno = DTMNOERR ;
	/*
		Note: all ports given here are external ports
	*/
	CHECK_ERR( count = DTMbeginRead( p, hdr, hdrsize ));
	DBGMSG1( "readMsg got header = %d\n", count );
	CHECK_ERR( count = DTMreadDataset( p, data, datasize, datatype ));
	CHECK_ERR( DTMendRead( p ));

	return count ;
}

/*
	DTMavailWrite()
	Test whether a subsequent beginWrite( or writeMsg )
	will (definitely) succeed or not.

	Return	values	: 	TRUE		if subsequent write will
									succeed.
						FALSE		subsequent write will wait/fail.	
						DTMERROR	if port is not initialised or
							server ( UDP/TCP ) port not
							yet acquired etc.
*/
#ifdef DTM_PROTOTYPES
int DTMavailWrite(int port )
#else
int	DTMavailWrite( port )
	int		port ;
#endif
{
	int				rstatus = DTM_PORT_NOT_READY;
	int				err_count ;
	Outport			*pcur ;
	reg DTMPORT		*pp;

  	DBGFLOW( "DTMavailWrite called.\n" );
  	DTMerrno = DTMNOERR;

	CHECK_ERR( port = dtm_map_port_internal( port ));
	pp = DTMpt[port];

	CHECK_ERR( dtm_check_server( pp, DTM_DONT_WAIT ));

	/* If we have been told to discard the output, we are ready */

	if ( pp->fDiscard ) return TRUE;

	/* If we have no connections, then we are not ready.  */

	if ( pp->out == NULL ) return FALSE;

	/*	For all ports in list	*/

	err_count = 0 ;
	rstatus = DTM_PORT_READY ;

	FOR_EACH_OUT_PORT( pcur, pp ) { 

		/* Connect to all new active sockets 	*/

		if( pcur->connfd == DTM_NO_CONNECTION ) {
			if( dtm_quick_connect( &pcur->sockaddr, &pcur->connfd ) 
					== DTMERROR ) {
				++err_count ;
				continue ;
			}
		}

		DBGINT( "DTMavailWrite: availWrite = %d\n", pcur -> availwrite );
		DBGINT( "DTMavailWrite: seqstart = %d\n", pcur -> seqstart );

		if( !(pcur->availwrite) ) {

			int	ack ;
			int	nf ;

			if ( pp->qservice == DTM_SYNC && !(pcur->seqstart) ) {

				/* send RTS to new sockets */

				if( dtm_send_ack( pcur->connfd, DTM_RTS ) == DTMERROR ) {
					if( DTMerrno == DTMEOF ) 
						CHECK_ERR( destroy_out_port( pp, &pcur ));
					++err_count ;
					continue ;
				} 
				pcur->seqstart = TRUE ;
			}
		
			nf = select_one( pcur->connfd );

			if( nf < 0 ) {
				CHECK_ERR( destroy_out_port(  pp, &pcur ));
				DTMerrno 	= DTMSELECT ;
				++err_count ;
				continue ;
			}

			/* No ack yet	*/

			if( nf == 0 ) {
				if( pp->qservice == DTM_SYNC ) 
					rstatus = DTM_PORT_NOT_READY ;
				continue ;
			} 

			/* Receive ack */

			if ( dtm_recv_ack( pcur->connfd, &ack ) == DTMERROR)
			{
				DBGFLOW( "Incorrect ack for header\n" );
				CHECK_ERR( destroy_out_port(  pp, &pcur ));
				++err_count ;
				continue ;
			}	

			/* port is available for write */

			if ( pp->qservice == DTM_SYNC )
				pcur->availwrite = TRUE;
		}
	}	/* end while */

	/*
		At some future point we may want to send the status
		of err_count to the server.
	*/
	pp->fLastWasSuccessfulAvailWrite = ( err_count == 0 ) 
			&& ( rstatus == DTM_PORT_READY );
	return ( err_count != 0 ) ? DTM_PORT_NOT_READY : rstatus ;
}

/*
	Function to write user's header.

	Return	values	: same as intWriteMsg().
*/
#ifdef DTM_PROTOTYPES
int DTMbeginWrite(int port,VOIDPTR header,int size )
#else
int	DTMbeginWrite( port, header, size )
	int		port ; 
	VOIDPTR	header;
	int		size ;
#endif
{
	DTMPORT	*pp;
	IOV_BUF	iov_buf;

	CHECK_ERR( port = dtm_map_port_internal( port ));
  	pp = DTMpt[port];

	if ( pp->fDiscard ) {
		CHECK_ERR( dtm_check_server( pp, DTM_DONT_WAIT ));
		if ( pp->fDiscard ) return DTM_OK;
	}	
	if ( !pp->fLastWasSuccessfulAvailWrite ) 
		CHECK_ERR( dtm_check_server( pp, DTM_WAIT ));
	CHECK_ERR( make_out_connections( pp ));
	make_write_iov( &iov_buf, START_SEQ, NO_END_SEQ, header, size, NULL, 0 );
	DBGMSG1( "DTMbeginWrite: before writev_buffer with %d ports\n",
			outp_count( pp ));
	CHECK_ERR( writev_buffer( pp, &iov_buf, START_SEQ ));
	pp->fLastWasSuccessfulAvailWrite = FALSE;
	CHECK_ERR( check_header_write_ack( pp ));
	return DTM_OK;
}

/*
	Function to write user's data.

	Return	values	: same as intWriteMsg().
*/
#ifdef DTM_PROTOTYPES
int DTMwriteDataset(int p,VOIDPTR ds,int size,DTMTYPE type)
#else
int DTMwriteDataset(p, ds, size, type)
	int		p;
	VOIDPTR	ds;
	int		size;
	DTMTYPE	type;
#endif
{
	DTMPORT *	pp;
	IOV_BUF	iov_buf;

	CHECK_ERR( p = dtm_map_port_internal( p ));
	pp = DTMpt[p]; 
	if ( pp->fDiscard ) return DTM_OK;
	CHECK_ERR( verify_out_connections( pp ));
	size = (*DTMconvertRtns[(int)type]) ( DTMSTD, ds, size );
	make_write_iov( &iov_buf, NO_START_SEQ, NO_END_SEQ, NULL, 0, ds, size);
	CHECK_ERR( writev_buffer( pp, &iov_buf, NO_START_SEQ));
	return DTM_OK;
}

/*
	Function to end user's write.

	Return	values	: same as intWriteMsg().
*/
#ifdef DTM_PROTOTYPES
int DTMendWrite(int port )
#else
int DTMendWrite( port )
	int	port;
#endif
{
	DTMPORT *		pp;
	IOV_BUF			iov_buf;
	reg Outport	*	pcur ;

	CHECK_ERR( port = dtm_map_port_internal( port ));
	pp = DTMpt[port]; 
	/*
		Check for endWrite before begin
	*/
	FOR_EACH_OUT_PORT( pcur, pp ) { 
		if( pcur->connfd == DTM_NO_CONNECTION ) continue; 
		if ((pp->qservice == DTM_SYNC && !pcur->availwrite)  ||
				!pcur->seqstart ) {
			DTMerrno = DTMCALL;
			return DTMERROR;
		}
	}
	if ( pp->fDiscard ) return DTM_OK;
	CHECK_ERR( verify_out_connections( pp ));
	make_write_iov( &iov_buf, NO_START_SEQ, END_SEQ, NULL, 0, NULL, 0 ); 
	CHECK_ERR( writev_buffer( pp, &iov_buf, NO_START_SEQ ));
	CHECK_ERR( clear_write_flags( pp ));
	return DTM_OK;
}

#ifdef DTM_PROTOTYPES
int DTMsizeof(DTMTYPE type )
#else
int	DTMsizeof( type )
	DTMTYPE	type;
#endif
{
	int	size = 1;

  	return (*DTMconvertRtns[(int)type])(DTMSTD, NULL, size);
}

/*
	Function to do a complete/composite write in which
	the user header/data is written and the write ended.

	Return	values	: same as intWriteMsg().
*/
#ifdef DTM_PROTOTYPES
int DTMwriteMsg(int p,char *hdr,int hdrsize,VOIDPTR data,int datasize,
			DTMTYPE datatype )
#else
int	DTMwriteMsg( p, hdr, hdrsize, data, datasize, datatype )
	int		p;
	char	*hdr ;
	int		hdrsize ;
	VOIDPTR	data ;
	int		datasize ;
	DTMTYPE	datatype ;
#endif
{
	DTMPORT	*pp;
	IOV_BUF	iov_buf;

	CHECK_ERR( p = dtm_map_port_internal( p ));
  	pp = DTMpt[p];

	if ( pp->fDiscard ) {
		CHECK_ERR( dtm_check_server( pp, DTM_DONT_WAIT ));
		if ( pp->fDiscard ) return DTM_OK;
	}	
	if ( !pp->fLastWasSuccessfulAvailWrite ) 
		CHECK_ERR( dtm_check_server( pp, DTM_WAIT ));
	CHECK_ERR( make_out_connections( pp ));
	CHECK_ERR( verify_out_connections( pp ));
	datasize = (*DTMconvertRtns[(int)datatype]) ( DTMSTD, data, datasize );
	make_write_iov( &iov_buf, START_SEQ, END_SEQ, hdr, hdrsize, data, datasize);
	CHECK_ERR( writev_buffer( pp, &iov_buf, START_SEQ ));
	CHECK_ERR( check_header_write_ack( pp ));
	CHECK_ERR( clear_write_flags( pp ));
	return DTM_OK;
}
