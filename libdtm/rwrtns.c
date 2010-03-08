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
** rwrtns.c - provides very low level routines for reading and writing
**		buffer.  This code should be independent of communication
**		channel as long as the descriptor will work with system
**		read and write routines.
**
***************************************************************************/

/*********************************************************************
**
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/rwrtns.c,v 1.5 1996/06/06 19:48:08 spowers Exp $
**
**********************************************************************/

/*
#ifdef RCSLOG

 $Log: rwrtns.c,v $
 Revision 1.5  1996/06/06 19:48:08  spowers
 Linux is brain ded.

 Revision 1.4  1996/02/18 23:40:16  spowers
 PROTO -> DTM_PROTO

 Revision 1.3  1995/11/10 12:03:32  spowers
 dumb warning messages

 Revision 1.2  1995/10/13 06:33:23  spowers
 Solaris support added.

 Revision 1.1.1.1  1995/01/11 00:03:02  alanb
 New CVS source tree, Mosaic 2.5 beta 4

 * Revision 2.5  1994/12/29  23:40:14  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:33  alanb
 *
 * Revision 1.2  1993/10/29  03:46:49  marca
 * Tweaks.
 *
 * Revision 1.1.1.1  1993/07/04  00:03:13  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:37  marca
 * I think I got it now.
 *
 * Revision 1.19  92/05/14  19:27:48  jefft
 * modified dtm_recv_reliable
 * 
 * Revision 1.18  1992/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 *
 * Revision 1.17  1992/04/29  22:01:34  jplevyak
 * Fix big with exact size buffers.  Remove dead code.
 *
 * Revision 1.16  1992/04/13  16:07:10  jplevyak
 * Changes for DEC and RS6000.
 *
 * Revision 1.15  92/03/10  22:07:10  jplevyak
 * Added changed for PC/MAC from Quincey Koziol (koziol@ncsa.uiuc.edu)
 * with modification.
 * 
 * Revision 1.14  1992/03/02  18:29:41  jplevyak
 * Fixed bug in EAGAIN handling.
 *
 * Revision 1.13  1992/03/02  17:20:14  jplevyak
 * Temporary back out.
 *
 * Revision 1.11  1992/02/27  23:44:31  jplevyak
 * Surrounded writes by code intended to recover from signal interruptions.
 *
 * Revision 1.10  1992/01/14  16:31:40  creiman
 * Removed mac #include
 *
 * Revision 1.9  1991/10/11  20:26:23  jplevyak
 * Fixed incorrect #def use.
 *
 * Revision 1.8  1991/10/10  15:14:01  jplevyak
 * Fixed naming convensions.
 *
 * Revision 1.7  91/08/20  15:56:06  sreedhar
 * Removed unused functions - dtm_write_buffer, dtm_send, dtm_recv
 * 
 * Revision 1.6  1991/08/15  18:56:52  sreedhar
 * Changes for logical portname version
 *
 * Revision 1.4  1991/06/11  15:19:51  sreedhar
 * disclaimer added
 *
 * Revision 1.3  1991/06/07  16:06:29  sreedhar
 * sizeof( int ) replaced by 4 for message to be sent out
 *
 * Revision 1.2  1991/05/30  15:51:50  sreedhar
 * Changes for readMsg/writeMsg internal release
 *
 * Revision 1.1  1990/11/08  16:38:13  jefft
 * Initial revision
 *

#endif
*/

#include	<stdio.h>
#include	<sys/types.h>
#if defined(_ARCH_MSDOS)
#include    <nmpcip.h>
#include    <time.h>
#include    "uio.h"
#else
#include	<sys/socket.h>
#include	<sys/ioctl.h>
#ifdef LINUX
#include "linux-uio.h"
#else
#include	<sys/uio.h>
#endif
#include	<netinet/in.h>
#endif
#include	<fcntl.h>
#include	<errno.h>
#include	<sys/time.h>

#ifdef SOLARIS
#include <sys/filio.h>
#endif

#ifdef RS6000
#include <sys/select.h>
#endif

#include	"dtmint.h"
#include	"debug.h"


/*
	CONTENTS


	dtm_read_buffer() 	- attempts to fill the next dtm buffer. 
	dtm_recv_header() 	- Function to read header and return size.  
 	dtm_recv_ack() 		- receive message ackowledgement
	tm_send_ack() 		- send message acknowledgement
	dtm_writev_buffer()	- sends the buffers to receiving process.
*/

/*
	STATIC FUNCTION PROTOTYPES
*/
#ifdef DTM_PROTOTYPES
static  int dtm_recv_reliable DTM_PROTO((int ,char *,int ));
static  int dtm_writev_failed DTM_PROTO((int ,struct msghdr *,int ));
static  int dtm_send_some DTM_PROTO((int d, char *buf, int bufsize ));
#endif

static int	padding[] = {0, 3, 2, 1};

/*		Technique from XlibInt.c
*/
#if defined(EAGAIN) && defined(EWOULDBLOCK)
#define ERRTEST(err) (err == EAGAIN || err == EWOULDBLOCK)
#else
#if defined(EAGAIN)
#define ERRTEST(err) (err == EAGAIN)
#else
#define ERRTEST(err) (err == EWOULDBLOCK)
#endif
#endif

#ifdef DTM_PROTOTYPES
static int   ready_bytes(int d, int length )
#else
static int	ready_bytes( d, length )
	int	d, length;
#endif
{
	int		num;
	fd_set	mask;
	struct timeval  timeout ;

	
	/* set the select timeout value */
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	FD_ZERO(&mask);
	FD_SET(d, &mask);
#ifdef __hpux
	num = select(FD_SETSIZE, (int *)&mask, (int *)0, (int *)0, &timeout);
#else
  	num = select(FD_SETSIZE, &mask, (fd_set *)0, (fd_set *)0, &timeout);
#endif

	if (num < 0)  {
		DTMerrno = DTMSELECT;
		return DTMERROR;
	}

	else if (num == 0)  {
		DTMerrno = DTMTIMEOUT;
		return DTMERROR;
	}

	else {
		ioctl(d, FIONREAD, &num);
		if (num < length)  {
			DTMerrno = DTMTIMEOUT;
			return DTMERROR;
		}

		else
			return DTM_OK;
	}
}

        

/*
	Reliably read from a port in the face of signals and other
	'errors' produced by the operating system.
*/
#ifdef DTM_PROTOTYPES
static int   dtm_recv_reliable(int d,char *buffer,int length )
#else
int	dtm_recv_reliable( d, buffer, length )
	int		d;
	char *	buffer;
	int		length;
#endif
{
	int		bytes_read;
	while ( (bytes_read = recv( d, buffer, length, 0)) !=  length ) {
		if ( bytes_read > 0) {
			length -= bytes_read;
			buffer += bytes_read;
		} else if (ERRTEST(errno)) {
			fd_set  filedes;
			int		got;

			/* FD_ZERO and FD_SET were moved into the select loop */
			/* just in case the select is clearing filedes */
			do {
				FD_ZERO( &filedes );
				FD_SET( d, &filedes );
#ifdef __hpux
				got = select( d, (int *)&filedes, (int *)NULL, (int *)NULL,
#else
  				got = select( d, &filedes, (fd_set *)NULL, (fd_set *)NULL,
#endif
						NULL );
				if (got < 0 &&  errno != EINTR ) {
					DTMerrno = DTMREAD;
					return DTMERROR;
				}
			} while ( got <= 0 );
			continue;
		} else if (bytes_read == 0) {
			DTMerrno = DTMEOF;
			return DTMERROR;
		} else if (errno != EINTR) {
			DTMerrno = DTMREAD;
			return DTMERROR;
		}
	}
	return DTM_OK;
}

/*
 * dtm_read_buffer() - attempts to fill the next dtm buffer.  The 
 *	blocklen variable must be set to DTM_NEW_DATASET after each dataset
 *	to force recv_buffer to move the next dataset.
 */
#ifdef DTM_PROTOTYPES
int dtm_read_buffer(int d,int32 *blocklen,VOIDPTR buffer,int length)
#else
int dtm_read_buffer(d, blocklen, buffer, length)
  int		d, *blocklen;
  VOIDPTR	buffer;
  int		length;
#endif
{
  reg int	tmp, readcnt, count = 0;

  DBGFLOW("# dtm_read_buffer called.\n");
  DBGMSG1("dtm_recv_buffer: attempting to read %d bytes.\n", length);
  DBGMSG1("dtm_recv_buffer: initial blocklen = %d\n", *blocklen);

  /* if block length is DTM_NEW_DATASET this is a new dataset 
   * get initial block count 
   */
  if (*blocklen == DTM_NEW_DATASET)  {
    CHECK_ERR(dtm_recv_reliable(d, (char *)blocklen, 4));
    LOCALINT(*blocklen);
    DBGINT("initial blocklen = %d\n", *blocklen);
  }

  /* attempt to get a full buffer */
  while (TRUE)  {

    /* if block length is 0, because last call to fill_buffer hit
     * the EOS or because this dataset is zero length, return 0  
     * to indicate the end of dataset.				 
	 */
    if (*blocklen == 0)
      return 0;

    /* if block length is greater than buffer size then... */
    if (*blocklen >= length - count)  {

      readcnt = length - count;
      CHECK_ERR( dtm_recv_reliable( d, ((char *)buffer) + length - readcnt,
         readcnt));

		/* decrement block length, if 0 get next block length */
		*blocklen -= (length - count);
		if (*blocklen == 0)  
			*blocklen = DTM_NEW_DATASET;

      /* if block length is 0 now, the EOS will be returned on */
      /* the next call to fill_buffer */

      /* return full buffer count */
      DBGINT("recv_buffer: buffer full, returning %d\n", length);
      return length;
      }

    /* else block length is less than buffer size */
    else  {

      readcnt = *blocklen;
      CHECK_ERR( dtm_recv_reliable( d, (char *)buffer + count +
         *blocklen - readcnt, readcnt));

      /* increment count */
      count += *blocklen;

      /* get next block length */
      CHECK_ERR( dtm_recv_reliable(d, (char *)blocklen, 4));
      LOCALINT(*blocklen);
      DBGINT("blocklen = %d\n", *blocklen);

      /* if block length is 0 now, the correct count will be */
      /* returned now, and EOS on the next call to fill_buffer */ 
      if (*blocklen == 0)
        return count;

      }
    } /* end while */
}

/*
       Replaces dtm_recv_header for nornal communication.
*/
#ifdef DTM_PROTOTYPES
int   dtm_read_header(int fd,void *buf,int buflen )
#else
int   dtm_read_header( fd, buf, buflen )
	int		fd;
	void * 	buf;
	int		buflen;
#endif
{
   int32		hdrsize;


   CHECK_ERR(ready_bytes(fd, 4));

   CHECK_ERR( dtm_recv_reliable( fd, (char *)&hdrsize, 4 ));
   LOCALINT(hdrsize);
   if ( hdrsize <= buflen ) {
        CHECK_ERR( dtm_recv_reliable( fd, buf, hdrsize ));
        return hdrsize;
   } else {
        CHECK_ERR( dtm_recv_reliable( fd, buf, buflen ));
        {
            int                     left  = hdrsize - buflen;
            int                     readcnt = left % DISCARDSIZE;
            if (!readcnt) readcnt = DISCARDSIZE;
            while (left) {
                CHECK_ERR(dtm_recv_reliable( fd, dtm_discard, readcnt ));
                left -= readcnt;
                readcnt = DISCARDSIZE;
             }
        }
    }
	DTMerrno = DTMHEADER;
	return DTMERROR;
}

/*
	dtm_recv_header()
	Function to read header and return size.  

	Notes	: If buffer is too small, dump remainder of header 
		  and return error.
		  Actually, this is function to read length of data and
		  then to receive that much data - the data is called header
		  everywhere since that was the first usage of the function.
*/
#ifdef DTM_PROTOTYPES
int dtm_recv_header(int d,VOIDPTR header,int length )
#else
int dtm_recv_header( d, header, length )
	int		d;
	int		length;
	VOIDPTR	header;
#endif
{
	int	readcnt, headerlen, tmp;
	struct	sockaddr_in from ;
	int	fromlen = sizeof( struct sockaddr_in ) ;

	DBGFLOW("# dtm_recv_header called.\n");
	DBGMSG1("dtm_recv_header: fd = %d.\n", d);
	DBGMSG1("dtm_recv_header: buf length = %d.\n", length);

  	/* get header length */
	
  	if( (readcnt = recvfrom(d, (char *)&headerlen, 4, 0, ( struct sockaddr *)&from,
			( int *)&fromlen)) != 4) {
    		/* somehow hit EOF, return DTMEOF instead */

		if( readcnt == 0 ) {
			DTMerrno = DTMEOF;
			DBGMSG("dtm_recv_header: EOF1.\n");
			return DTMERROR;
		} else {
			if( errno == ECONNRESET ) {
				/* connection closed by writer, return EOF */

				DBGMSG("dtm_recv_header: EOF2.\n");
				DTMerrno = DTMEOF;
				return DTMERROR;
			} else {
				/* don't know what the problem is, punt... */
				DBGMSG("dtm_recv_header: EOF3.\n");
				DTMerrno = DTMREAD;
				return DTMERROR;
			}
		}
  	}    

  	LOCALINT(headerlen);
	DBGMSG("dtm_recv_header: got length.\n");

	/*  read the header */ 

  	readcnt = (length > headerlen) ? headerlen : length ;
  	header = (void *) (((char *) header) + readcnt);

  	while(readcnt) {
		if( (tmp = recvfrom(d, ((char *)header) - readcnt, readcnt, 0, 
		( struct sockaddr *)&from, ( int *)&fromlen)) > 0) 
			readcnt -= tmp;
		else {
      			DTMerrno = DTMREAD;
      			return DTMERROR;
		}
	}

   	/* check for header greater than buffer size provided */ 

  	if( length >= headerlen ) 
		return headerlen;
  	else {
  		/* discard remaining header */

    		readcnt = headerlen - length;
		while (readcnt) {
			if ((tmp = recvfrom(d, dtm_discard, readcnt, 0, 
					(struct sockaddr *)&from, (int *)&fromlen)) > 0) 
				readcnt -= tmp;
			else {
      				DTMerrno = DTMREAD;
      				return DTMERROR;
			}
		}
    
		DTMerrno = DTMHEADER;
		return DTMERROR;
	}
}

/*
 	dtm_recv_ack() - receive message ackowledgement

	Notes	: Berkeley implementation returns 0 from recv
		  if socket connection breaks while waiting in
		  recv system call.  System V returns -1 and 
		  ECONNRESET in errno for same error.

		  For historical reasons, DTMEOF is returned when
		  socket connection breaks in middle instead of
		  say DTMFCONN ( DTM connection failed error )
*/
#ifdef DTM_PROTOTYPES
int dtm_recv_ack(int d,int *ack )
#else
int	dtm_recv_ack( d, ack )
	int	d;
	int	*ack;
#endif
{
	int	tmp ;

	DBGFLOW("# dtm_recv_ack called.\n");

	/* there should be no possibility of blocking after this call */
	CHECK_ERR(ready_bytes(d, 4));

  	if( (tmp = recv( d, (char *)ack, 4, 0 )) != 4 ) {
		DBGINT( "Recv_ack errno = %d\n", errno ) ;
		if( tmp == 0 ) 
			/* Courtesy Berkeley */

			DTMerrno = DTMEOF ;
		else {
			if( errno == ECONNRESET ) 
				/* Courtesy system V */

				DTMerrno = DTMEOF;
			else 
				DTMerrno = DTMREAD;
		}
		return DTMERROR;
	}

	DBGMSG1( "ack received, tmp = %d\n", tmp );
  	LOCALINT(*ack);
  	return DTM_OK;
}

#ifdef DTM_PROTOTYPES
static int dtm_send_some(int d, char *buf, int bufsize )
#else
int	dtm_send_some( d, buf, bufsize )
	int		d;
	char *	buf;
	int		bufsize;
#endif
{
	int	tmp ;

	while (bufsize ) {
		tmp = send(d, buf, bufsize, 0);
		if ( tmp >= 0 ) {
			bufsize -= tmp;
			buf += tmp;
			continue;
		}
		if (errno == EPIPE) {
				/* socket connection broke in middle */
			DTMerrno = DTMEOF ;
			return DTMERROR;
		} else if ( ERRTEST( errno ) ) {
			fd_set  filedes;
			int		got;

			FD_ZERO( &filedes );
			FD_SET( d, &filedes );
			do {
#ifdef __hpux
				got = select( 32, (int *)&filedes, (int *)NULL, (int *)NULL,
#else
  				got = select( 32, &filedes, (fd_set *)NULL, (fd_set *)NULL,
#endif
						NULL );
				if (got < 0 &&  errno != EINTR ) {
					DTMerrno = DTMWRITE;
					return DTMERROR;
				}
			} while ( got <= 0 );
			continue;
		} else DTMerrno = DTMWRITE ;
		return DTMERROR;
	}
	return DTM_OK;
}

/*
 * dtm_send_ack() - send message acknowledgement
 */
#ifdef DTM_PROTOTYPES
int dtm_send_ack(int d, int32 ack)
#else
int	dtm_send_ack(d, ack)
	int		d;
	int32	ack;
#endif
{
	DBGFLOW("# dtm_send_ack called.\n");

  	STDINT(ack);
	return dtm_send_some( d, (char *)&ack, 4 );
}


#ifdef DTM_PROTOTYPES
static int dtm_writev_failed(int fd,struct msghdr *msgbuf,int tmp )
#else
int dtm_writev_failed( fd, msgbuf, tmp )
	int					fd;
	struct msghdr * 	msgbuf;
	int					tmp;
#endif
{
	int					done = tmp;
	int					i;
	struct	iovec	*	iov;

	iov=msgbuf->msg_iov;

	if ( tmp < 0 ) done = 0;
	for ( i = 0; i < msgbuf->msg_iovlen; i++ ) {
		done -= iov[i].iov_len;
		if ( done > 0 ) continue;
		if ( dtm_send_some( fd, iov[i].iov_base + done + iov[i].iov_len, 
				(- done )) == DTMERROR )
			return DTMERROR;
		done = 0;
	}
}

/*
	dtm_writev_buffer() - sends the buffers to receiving process.
*/
#ifdef DTM_PROTOTYPES
int dtm_writev_buffer(int fd,struct iovec *iov,int32 iovlen,int32 iovsize,
			struct sockaddr *addr,int addrlen )
#else
int	dtm_writev_buffer( fd, iov, iovlen, iovsize, addr, addrlen )
	int				fd ;
	struct	iovec	*iov ;
	int32			iovlen ;
	int32			iovsize ;
	struct sockaddr	*addr ;
	int				addrlen ;
#endif
{
	int	tmp;
	struct	msghdr	msgbuf ;
	int	todo;

  	DBGINT("# dtm_writev_buffer called, fd %d.\n", fd );
	
	msgbuf.msg_name = (caddr_t)addr ; 
	msgbuf.msg_namelen = addrlen ;
	msgbuf.msg_iov = iov ;
	msgbuf.msg_iovlen = iovlen ;
	msgbuf.msg_accrights = 0 ;

	if( (tmp = sendmsg( fd, &msgbuf, 0 )) != iovsize ) 
		return dtm_writev_failed( fd, &msgbuf, tmp );

	DBGINT( "dtm_writev_buffer tmp = %d\n", tmp );
	
	return	DTM_OK ;
}
