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
**  $Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/dtmnserv.h,v 1.3 1995/01/12 02:33:34 spowers Exp $
**
**********************************************************************/

/*
 * Revision 1.1.1.1  1993/07/04  00:03:12  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:23  marca
 * I think I got it now.
 *
 * Revision 1.5  92/04/30  20:27:05  jplevyak
 * Changed Version to 2.3
 * 
*/


/*
	Purpose	: Header file for name server interaction 
	Notes	:

	Message format: 

	Portid	- nethostid:portid	
	Header 	- opcode

	DTM to nameserver

	Send message length.

	Mreg  	  - Header refname portname Portid
	Mackroute - Header refname portname 

	Nameserver to DTM

	Send message length.

	Mroute 	- Header delcount addcount Portid1 Portid2 ...

	Ports to be deleted should be before ports to be added.

	Nethostid is in dotted decimal notation of internet.
*/

#define	MREGID		"REGISTER"	
#define	MREG		"%s %s %s %s:%d"
#define	MROUTEID	"ROUTE"	
#define	MROUTE		"%s %d %d"		/*  %s:%d  %s:%d .... */
#define	MACKROUTEID	"ROUTE_ACK"	
#define	MACKROUTE	"%s %s %s"	
#define	MDISCARDID	"DISCARD"
#define	MDISCARD	"%s %d"
