/*****************************************************************************
*
*                         NCSA DTM version 2.3
*                             May 1, 1992
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
** x.c	Contains the DTM interface to X.
**
***************************************************************************/

/*
 * $Log: x.c,v $
 * Revision 1.1.1.1  1995/01/11 00:03:04  alanb
 * New CVS source tree, Mosaic 2.5 beta 4
 *
 * Revision 2.5  1994/12/29  23:40:42  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:32  alanb
 *
 * Revision 1.1.1.1  1993/07/04  00:03:16  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.1  1993/01/18  21:50:54  marca
 * I think I got it now.
 *
 * Revision 1.4  92/05/05  22:27:50  jplevyak
 * Corrected X interface code.
 * 
 * Revision 1.3  1992/05/04  22:05:55  jplevyak
 * Declare DTMaddInput external
 *
 * Revision 1.2  1992/04/30  20:25:27  jplevyak
 * Changed Version to 2.3.
 *
 * Revision 1.1  1992/04/29  22:20:16  jplevyak
 * Initial revision
 *
 */

#include <X11/Intrinsic.h>

#include "arch.h"
#include "dtmint.h"
#include "debug.h"

#if XtInputReadMask != (1L<<0)
	Balk -	you must change the definition in dtm.c near
			the function dtm_set_Xcallback
#endif

#ifdef DTM_PROTOTYPES
void dtm_handle_new_in( caddr_t client_data, int * fd, XtInputId * id)
#else
void dtm_handle_new_in( client_data, fd, id )
	caddr_t		client_data;
	int *		fd;
	XtInputId *	id;
#endif
{
	int				p = (int) client_data;
	DTMPORT *		pp = DTMpt[p];

	dtm_accept_read_connections( pp, FALSE );
}

#ifdef DTM_PROTOTYPES
void dtm_handle_new_out( caddr_t client_data, int * fd, XtInputId * id)
#else
void dtm_handle_new_out( client_data, fd, id )
	caddr_t		client_data;
	int *		fd;
	XtInputId *	id;
#endif
{
	int				p = (int) client_data;
	DTMPORT *		pp = DTMpt[p];
	int				p_ext = p;

	dtm_map_port_external( &p_ext );
	pp->Xcallback( pp->Xcallback_data, &p_ext, id );
}


#ifdef DTM_PROTOTYPES
/*
	STATIC FUNCTION PROTOTYPES
*/
#endif

/*
	DTMaddInput()

	Add X style input handlers for DTM ports.  Currently only
	read ports are handled.

	condition
		The read/write portions of the condition value are ignored
		in favor of the Read/Write quality of the port.  Any operating-
		system-dependant options should be included here.
	proc
		The user must provide an X style callback which will be called
		when data has arrived at an input port.  New connections are
		automatically handled.
*/
#ifdef DTM_PROTOTYPES
int DTMaddInput( int p_ext, caddr_t condition, 
					XtInputCallbackProc proc, caddr_t client_data )
#else
int	DTMaddInput( p_ext, condition, proc, client_data )
	int					p_ext;
	caddr_t 			condition; 
	XtInputCallbackProc	proc;
	caddr_t				client_data;
#endif
{
	reg DTMPORT *pp;
	reg Inport  *inp;
	reg	int		p;

	DTMerrno = DTMNOERR;

	CHECK_ERR( p = dtm_map_port_internal( p_ext ));
	pp = DTMpt[p];

	pp->Xcallback_data = client_data;
	pp->Xcallback = proc;
	pp->XaddInput = (XtInputCallbackProc) XtAddInput;
	pp->XremoveInput = (XtInputCallbackProc) XtRemoveInput;
	if ( pp->porttype == INPORTTYPE ) {
		/*
			Set up callback for new connections
		*/
		XtAddInput( pp->sockfd, XtInputReadMask, dtm_handle_new_in, 
				(caddr_t) p );
		FOR_EACH_IN_PORT( inp, pp ) {
			inp->XinputId = XtAddInput( inp->fd, XtInputReadMask, 
					dtm_handle_in, (caddr_t) p );
		}
	} else {
		XtAddInput( pp->sockfd, XtInputReadMask, dtm_handle_new_out, 
				(caddr_t) p );
	}
	return DTM_OK;
}
