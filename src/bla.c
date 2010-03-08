/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright (C) 1993, Board of Trustees of the University of Illinois      *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/

/*		CCI redirect object 
**		=================
*/

#include "../config.h"
#include <stdio.h>
#include "cciServer.h"
#include "list.h"
#include "memStuffForPipSqueeks.h"

#include "HTPlain.h"
#include "HTUtils.h"
#include "HText.h"
#include "HTFile.h"
#include "HTCompressed.h"

extern void MoCCISendOutputToClient();
extern void HTCompressedFileToFile (char *fnam, int compressed);

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

/*		HTML Object
**		-----------
*/

struct _HTStream {
	WWW_CONST HTStreamClass *	isa;

	HTAtom *dataType;
	char fileName[L_tmpnam]; /* name of temp file... kept for unlink()ing*/
	FILE *fp; 
   
        int compressed;
};

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

PRIVATE void CCI_put_character ARGS2(HTStream *, me, char, c)
{
	fputc(c,me->fp);
}



/*	String handling
**	---------------
**
*/
PRIVATE void CCI_put_string ARGS2(HTStream *, me, WWW_CONST char*, s)
{

	fwrite(s,1,strlen(s),me->fp);
}


PRIVATE void CCI_write ARGS3(HTStream *, me, WWW_CONST char*, s, int, l)
{
	fwrite(s,1,l,me->fp);
}



/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void CCI_free ARGS1(HTStream *, me)
{

#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf(stderr,"CCI_free()\n");
	}
#endif

/*
  if (me->compressed != COMPRESSED_NOT)
    {
#ifndef DISABLE_TRACE
      if (cciTrace)
        fprintf 
          (stderr, 
           "[CCI_free] OK, we're going to decompress HText\n");
#endif
      HTCompressedHText (me->text, me->compressed, 1);
    }

  free(me);
*/
}

/*	End writing
*/

PRIVATE void CCI_end_document ARGS1(HTStream *, me)
{

	fclose(me->fp);
	/* ship it */
	if ( me->compressed != COMPRESSED_NOT)
	  HTCompressedFileToFile (me->fileName, me->compressed);	
	MoCCISendOutputToClient(HTAtom_name(me->dataType),me->fileName);
/*
	unlink(me->fileName);
*/
}

PRIVATE void CCI_handle_interrupt ARGS1(HTStream *, me)
{
	fclose(me->fp);
	unlink(me->fileName);
}



/*		Structured Object Class
**		-----------------------
*/
PUBLIC WWW_CONST HTStreamClass CCIout =
{		
	"CCIout",
	CCI_free,
	CCI_end_document,
	CCI_put_character, 	CCI_put_string, CCI_write,
        CCI_handle_interrupt
}; 


/*		New object
**		----------
*/
PUBLIC HTStream* CCIPresent ARGS5(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink,
        HTFormat,               format_in,
        int,                    compressed)
{
HTStream* me = (HTStream*)malloc(sizeof(HTStream));

	me->isa = &CCIout;       

	(void) tmpnam(me->fileName);
	if (!(me->fp = fopen(me->fileName,"w"))) {
		/*error, can't open tmp file */
		return(sink);
		}
	me->dataType = pres->rep;
	me->compressed = compressed;

/*
  if (me->compressed == COMPRESSED_NOT)
    HText_appendText(me->text, "<PLAINTEXT>\n");
*/

  return (HTStream*) me;
}
