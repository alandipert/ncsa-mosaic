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

#ifndef UIO_H
#define UIO_H

#include <sys/types.h>

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

