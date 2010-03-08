/*		Mosaic HTML text object
**		=================
**
**	This version of the stream object just writes to a socket.
**	The socket is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/
#include "../config.h"
#include "HTMosaicHTML.h"

#define BUFFER_SIZE 4096;	/* Tradeoff */

#include "HTUtils.h"
#include "HText.h"
#include "HTFile.h"
#include "HTCompressed.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/*		HTML Object
**		-----------
*/

struct _HTStream {
	WWW_CONST HTStreamClass *	isa;

	HText * 		text;
        int interrupted;
        int compressed;
};

/*	Write the buffer out to the socket
**	----------------------------------
*/


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

PRIVATE void HTMosaicHTML_put_character ARGS2(HTStream *, me, char, c)
{
    HText_appendCharacter(me->text, c);
}



/*	String handling
**	---------------
**
*/
PRIVATE void HTMosaicHTML_put_string ARGS2(HTStream *, me, WWW_CONST char*, s)
{
    HText_appendText(me->text, s);
}


PRIVATE void HTMosaicHTML_write ARGS3(HTStream *, me, WWW_CONST char*, s, int, l)
{
    HText_appendBlock (me->text, s, l);
}



/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void HTMosaicHTML_free ARGS1(HTStream *, me)
{
  if (me->compressed != COMPRESSED_NOT)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf 
          (stderr, 
           "[HTMosaicHTMLFree] OK, we're going to decompress HText\n");
#endif
      HTCompressedHText (me->text, me->compressed, 0);
    }

  free(me);
}

/*	End writing
*/

PRIVATE void HTMosaicHTML_end_document ARGS1(HTStream *, me)
{
    HText_endAppend(me->text);
}

PRIVATE void HTMosaicHTML_handle_interrupt ARGS1(HTStream *, me)
{
  me->interrupted = 1;
  HText_doAbort(me->text);
}



/*		Structured Object Class
**		-----------------------
*/
PUBLIC WWW_CONST HTStreamClass HTMosaicHTML =
{		
	"SocketWriter",
	HTMosaicHTML_free,
	HTMosaicHTML_end_document,
	HTMosaicHTML_put_character, 	HTMosaicHTML_put_string, 
        HTMosaicHTML_write,
        HTMosaicHTML_handle_interrupt
}; 


/*		New object
**		----------
*/
PUBLIC HTStream* HTMosaicHTMLPresent ARGS5(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink,
        HTFormat,               format_in,
        int,                    compressed)
{
  HTStream* me = (HTStream*)malloc(sizeof(*me));

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "[HTMosaicHTMLPresent] Hi there!  Compressed is %d\n", 
             compressed);
#endif

  me->isa = &HTMosaicHTML;       
  me->text = HText_new();
  me->interrupted = 0;
  me->compressed = compressed;
  HText_beginAppend(me->text);
  
  return (HTStream*) me;
}
