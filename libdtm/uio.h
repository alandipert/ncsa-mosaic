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
* $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/uio.h,v 1.3 1995/01/12 02:33:40 spowers Exp $ 
**
**********************************************************************/

/*
 * Revision 1.1.1.1  1993/07/04  00:03:15  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:49  marca
 * I think I got it now.
 *
 * Revision 1.2  92/04/30  20:27:37  jplevyak
 * Changed Version to 2.3
 * 
 * Revision 1.1  1992/03/06  22:51:22  jplevyak
 * Initial revision
 *
*/

#ifndef UIO_H
#define UIO_H

#include <sys\types.h>

typedef char far *caddr_t;

struct iovec {
	caddr_t	iov_base;
    long int iov_len;
};

struct uio {
    struct iovec *uio_iov;
    long int uio_iovcnt;
	off_t	uio_offset;
    long int uio_segflg;
	short	uio_fmode;
	int	uio_resid;
};

struct msghdr {
    caddr_t   msg_name;         /* optional address */
    long int  msg_namelen;      /* size of address */
    struct    iovec *msg_iov;   /* scatter/gather array */
    long int  msg_iovlen;       /* # elements in msg_iov */
    caddr_t   msg_accrights;    /* access rights sent/received */
    long int  msg_accrightslen;
};

#ifndef EFAULT
#define EFAULT  14          /* for UNIX compability */
#endif

enum	uio_rw { UIO_READ, UIO_WRITE };

/*
 * Segment flag values (should be enum).
 */
#define UIO_USERSPACE	0		/* from user data space */
#define UIO_SYSSPACE	1		/* from system space */
#define UIO_USERISPACE	2		/* from user I space */

#if defined(__STDC__) | defined (_STDC_)
extern long int readv(int ,struct iovec *,int );
extern long int writev(int ,struct iovec *,int );

extern long int recvmsg(int ,struct msghdr *,int );
extern long int sendmsg(int ,struct msghdr *,int );
#endif  /* defined(__STDC__) */

#endif  /* UIO_H */

