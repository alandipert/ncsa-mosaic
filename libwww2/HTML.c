/*		Structured stream to Rich hypertext converter
**		============================================
**
**	This generates a hypertext object.  It converts from the
**	structured stream interface from HTML events into the style-
**	oriented interface of the HText.h interface.  This module is
**	only used in clients and should not be linked into servers.
**
**	Override this module if you are making a new GUI browser.
**
*/
#include "../config.h"
#include "HTML.h"

#include <ctype.h>
#include <stdio.h>

#include "HTAtom.h"
#include "HTChunk.h"
#include "HText.h"

#include "HTAlert.h"
#include "HTMLGen.h"
#include "HTParse.h"

/*		HTML Object
**		-----------
*/
#define MAX_NESTING 20		/* Should be checked by parser */

struct _HTStructured {
    WWW_CONST HTStructuredClass * 	isa;
    HTParentAnchor * 		node_anchor;
    HText * 			text;

    HTStream*			target;			/* Output stream */
    HTStreamClass		targetClass;		/* Output routines */
};

struct _HTStream {
    WWW_CONST HTStreamClass *	isa;
    /* .... */
};

/* 	Entity values -- for ISO Latin 1 local representation
**
**	This MUST match exactly the table referred to in the DTD!
*/
static char * ISO_Latin1[] = {
  	"\306",	/* capital AE diphthong (ligature) */ 
  	"\301",	/* capital A, acute accent */ 
  	"\302",	/* capital A, circumflex accent */ 
  	"\300",	/* capital A, grave accent */ 
  	"\305",	/* capital A, ring */ 
  	"\303",	/* capital A, tilde */ 
  	"\304",	/* capital A, dieresis or umlaut mark */ 
  	"\307",	/* capital C, cedilla */ 
  	"\320",	/* capital Eth, Icelandic */ 
  	"\311",	/* capital E, acute accent */ 
  	"\312",	/* capital E, circumflex accent */ 
  	"\310",	/* capital E, grave accent */ 
  	"\313",	/* capital E, dieresis or umlaut mark */ 
  	"\315",	/* capital I, acute accent */ 
  	"\316",	/* capital I, circumflex accent */ 
  	"\314",	/* capital I, grave accent */ 
  	"\317",	/* capital I, dieresis or umlaut mark */ 
  	"\321",	/* capital N, tilde */ 
  	"\323",	/* capital O, acute accent */ 
  	"\324",	/* capital O, circumflex accent */ 
  	"\322",	/* capital O, grave accent */ 
  	"\330",	/* capital O, slash */ 
  	"\325",	/* capital O, tilde */ 
  	"\326",	/* capital O, dieresis or umlaut mark */ 
  	"\336",	/* capital THORN, Icelandic */ 
  	"\332",	/* capital U, acute accent */ 
  	"\333",	/* capital U, circumflex accent */ 
  	"\331",	/* capital U, grave accent */ 
  	"\334",	/* capital U, dieresis or umlaut mark */ 
  	"\335",	/* capital Y, acute accent */ 
  	"\341",	/* small a, acute accent */ 
  	"\342",	/* small a, circumflex accent */ 
  	"\346",	/* small ae diphthong (ligature) */ 
  	"\340",	/* small a, grave accent */ 
  	"\046",	/* ampersand */ 
  	"\345",	/* small a, ring */ 
  	"\343",	/* small a, tilde */ 
  	"\344",	/* small a, dieresis or umlaut mark */ 
  	"\347",	/* small c, cedilla */ 
  	"\351",	/* small e, acute accent */ 
  	"\352",	/* small e, circumflex accent */ 
  	"\350",	/* small e, grave accent */ 
  	"\360",	/* small eth, Icelandic */ 
  	"\353",	/* small e, dieresis or umlaut mark */ 
  	"\076",	/* greater than */ 
  	"\355",	/* small i, acute accent */ 
  	"\356",	/* small i, circumflex accent */ 
  	"\354",	/* small i, grave accent */ 
  	"\357",	/* small i, dieresis or umlaut mark */ 
  	"\074",	/* less than */ 
  	"\361",	/* small n, tilde */ 
  	"\363",	/* small o, acute accent */ 
  	"\364",	/* small o, circumflex accent */ 
  	"\362",	/* small o, grave accent */ 
  	"\370",	/* small o, slash */ 
  	"\365",	/* small o, tilde */ 
  	"\366",	/* small o, dieresis or umlaut mark */ 
  	"\337",	/* small sharp s, German (sz ligature) */ 
  	"\376",	/* small thorn, Icelandic */ 
  	"\372",	/* small u, acute accent */ 
  	"\373",	/* small u, circumflex accent */ 
  	"\371",	/* small u, grave accent */ 
  	"\374",	/* small u, dieresis or umlaut mark */ 
  	"\375",	/* small y, acute accent */ 
  	"\377",	/* small y, dieresis or umlaut mark */ 
};



/*		Set character set
**		----------------
*/

PRIVATE char** p_entity_values = ISO_Latin1;	/* Pointer to translation */

PUBLIC void HTMLUseCharacterSet ARGS1(HTMLCharacterSet, i)
{
    p_entity_values = ISO_Latin1;
}


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/
PRIVATE void HTML_put_character ARGS2(HTStructured *, me, char, c)
{
  if (!me->text) 
    {
      me->text = HText_new();
      HText_beginAppend(me->text);
    }
  HText_appendCharacter(me->text, c);
}



/*	String handling
**	---------------
**
**	This is written separately from put_character becuase the loop can
**	in some cases be promoted to a higher function call level for speed.
*/
PRIVATE void HTML_put_string ARGS2(HTStructured *, me, WWW_CONST char*, s)
{
  if (!me->text) 
    {
      me->text = HText_new();
      HText_beginAppend(me->text);
    }
  HText_appendText(me->text, s);
}


/*	Buffer write
**	------------
*/
PRIVATE void HTML_write ARGS3(HTStructured *, me, WWW_CONST char*, s, int, l)
{
    WWW_CONST char* p;
    WWW_CONST char* e = s+l;
    for (p=s; s<e; p++) HTML_put_character(me, *p);
}


/*	Start Element
**	-------------
*/
PRIVATE void HTML_start_element ARGS4(
	HTStructured *, 	me,
	int,		element_number,
	WWW_CONST BOOL*,	 	present,
	WWW_CONST char **,	value)
{
  if (!me->text) 
    {
      me->text = HText_new();
      HText_beginAppend(me->text);
    }
  switch (element_number) 
    {
    case HTML_A:
      {
        char *href = NULL;
        if (present[HTML_A_HREF]) 
          {
            StrAllocCopy(href, value[HTML_A_HREF]);
            HTSimplify(href);
          }
        HText_beginAnchor(me->text, href);
        free (href);
      }
      break;
      
    case HTML_TITLE:
      HText_appendText(me->text, "<TITLE>");
      break;
    case HTML_ISINDEX:
      HText_appendText(me->text, "<ISINDEX>\n");
      break;
    case HTML_P:
      HText_appendText(me->text, "<P>\n");
      break;
    case HTML_DL:
      HText_appendText(me->text, "\n<DL>\n");
      break;
    case HTML_DT:
      HText_appendText(me->text, "\n<DT> ");
      break;
    case HTML_DD:
      HText_appendText(me->text, "\n<DD> ");
      break;
    case HTML_UL:
      HText_appendText(me->text, "\n<UL>\n");
      break;
    case HTML_OL:
      HText_appendText(me->text, "\n<OL>\n");
      break;
    case HTML_MENU:
      HText_appendText(me->text, "\n<MENU>\n");
      break;
    case HTML_DIR:
      HText_appendText(me->text, "\n<DIR>\n");
      break;
    case HTML_LI:
      HText_appendText(me->text, "\n<LI> ");
      break;
    case HTML_LISTING:
      HText_appendText(me->text, "<LISTING>");
      break;
    case HTML_XMP:
      HText_appendText(me->text, "<XMP>");
      break;
    case HTML_PLAINTEXT:
      HText_appendText(me->text, "<PLAINTEXT>");
      break;
    case HTML_PRE:
      HText_appendText(me->text, "<PRE>");
      break;
    case HTML_IMG:
      {
        char *href = NULL;
        if (present[HTML_A_HREF]) 
          {
            StrAllocCopy(href, value[HTML_A_HREF]);
            HTSimplify(href);
          }
        if (href)
          {
            HText_appendText(me->text, "<IMG SRC=\"");
            HText_appendText(me->text, href);
            HText_appendText(me->text, "\">");
            free (href);
          }
      }
      break;
    case HTML_H1:
      HText_appendText(me->text, "<H1>");
      break;
    case HTML_H2:
      HText_appendText(me->text, "<H2>");
      break;
    case HTML_H3:
      HText_appendText(me->text, "<H3>");
      break;
    case HTML_H4:
      HText_appendText(me->text, "<H4>");
      break;
    case HTML_H5:
      HText_appendText(me->text, "<H5>");
      break;
    case HTML_H6:
      HText_appendText(me->text, "<H6>");
      break;
    case HTML_ADDRESS:
      HText_appendText(me->text, "<ADDRESS>");
      break;
    case HTML_BLOCKQUOTE:
      HText_appendText(me->text, "<BLOCKQUOTE>");
      break;
    
    case HTML_TT:			/* Physical character highlighting */
    case HTML_B:			/* Currently ignored */
    case HTML_I:
    case HTML_U:
    case HTML_EM:			/* Logical character highlighting */
    case HTML_STRONG:			/* Currently ignored */
    case HTML_CODE:
    case HTML_SAMP:
    case HTML_KBD:
    case HTML_VAR:
    case HTML_DFN:
    case HTML_CITE:
      break;

    default:
      break;
      
    } /* end switch */
}


/*		End Element
**		-----------
**
*/
PRIVATE void HTML_end_element ARGS2(HTStructured *, me, int , element_number)
{
  switch(element_number) 
    {
    case HTML_A:
      HText_endAnchor(me->text);
      break;
    case HTML_TITLE:
      HText_appendText(me->text, "</TITLE>\n");
      break;
    case HTML_LISTING:
      HText_appendText(me->text, "</LISTING>\n");
      break;
    case HTML_XMP:
      HText_appendText(me->text, "</XMP>\n");
      break;
    case HTML_PRE:
      HText_appendText(me->text, "</PRE>\n");
      break;
    case HTML_DL:
      HText_appendText(me->text, "\n</DL>\n");
      break;
    case HTML_UL:
      HText_appendText(me->text, "\n</UL>\n");
      break;
    case HTML_OL:
      HText_appendText(me->text, "\n</OL>\n");
      break;
    case HTML_MENU:
      HText_appendText(me->text, "\n</MENU>\n");
      break;
    case HTML_DIR:
      HText_appendText(me->text, "\n</DIR>\n");
      break;
    case HTML_H1:
      HText_appendText(me->text, "</H1>\n");
      break;
    case HTML_H2:
      HText_appendText(me->text, "</H2>\n");
      break;
    case HTML_H3:
      HText_appendText(me->text, "</H3>\n");
      break;
    case HTML_H4:
      HText_appendText(me->text, "</H4>\n");
      break;
    case HTML_H5:
      HText_appendText(me->text, "</H5>\n");
      break;
    case HTML_H6:
      HText_appendText(me->text, "</H6>\n");
      break;
    case HTML_ADDRESS:
      HText_appendText(me->text, "</ADDRESS>\n");
      break;
    case HTML_BLOCKQUOTE:
      HText_appendText(me->text, "</BLOCKQUOTE>\n");
      break;
    default:
      break;
    } /* switch */
}


/*		Expanding entities
**		------------------
*/
/*	(In fact, they all shrink!)
*/

PRIVATE void HTML_put_entity ARGS2(HTStructured *, me, int, entity_number)
{
    HTML_put_string(me, ISO_Latin1[entity_number]);	/* @@ Other representations */
}



/*	Free an HTML object
**	-------------------
**
** If the document is empty, the text object will not yet exist.
   So we could in fact abandon creating the document and return
   an error code.  In fact an empty document is an important type
   of document, so we don't.
**
**	If non-interactive, everything is freed off.   No: crashes -listrefs
**	Otherwise, the interactive object is left.	
*/
PUBLIC void HTML_free ARGS1(HTStructured *, me)
{
  if (me->text)
    HText_endAppend(me->text);
  
  if (me->target) 
    {
      (*me->targetClass.end_document)(me->target);
      (*me->targetClass.free)(me->target);
    }
  free(me);
}


PUBLIC void HTML_handle_interrupt ARGS1(HTStructured *, me)
{
  if (me->text)
    HText_doAbort (me->text);
  
  if (me->target) 
    {
      (*me->targetClass.handle_interrupt)(me->target);
    }
  /* Not necessarily safe... */
  /* free(me); */
}


PRIVATE void HTML_end_document ARGS1(HTStructured *, me)
{			/* Obsolete */
}


/*	Structured Object Class
**	-----------------------
*/
PUBLIC WWW_CONST HTStructuredClass HTMLPresentation = /* As opposed to print etc */
{		
	"text/html",
	HTML_free,
	HTML_end_document, HTML_handle_interrupt,
	HTML_put_character, 	HTML_put_string,  HTML_write,
	HTML_start_element, 	HTML_end_element,
	HTML_put_entity
}; 


/*		New Structured Text object
**		--------------------------
**
**	The strutcured stream can generate either presentation,
**	or plain text, or HTML.
*/
PUBLIC HTStructured* HTML_new ARGS3(
	HTParentAnchor *, 	anchor,
	HTFormat,		format_out,
	HTStream*,		stream)
{

    HTStructured * me;
    
#if 0
    if (format_out != WWW_PLAINTEXT && format_out != WWW_PRESENT) {
        HTStream * intermediate = HTStreamStack(WWW_HTML, format_out, 0,
		stream, anchor);
        fprintf (stderr, "+++ YO in HTML_new\n");
	if (intermediate) return HTMLGenerator(intermediate);
        fprintf(stderr, "** Internal error: can't parse HTML to %s\n",
       		HTAtom_name(format_out));
	exit (-99);
    }
#endif

    me = (HTStructured*) malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTML_new");

    me->isa = &HTMLPresentation;
    me->node_anchor =  anchor;
    me->text = 0;
    me->target = stream;
    if (stream) 
      me->targetClass = *stream->isa;	/* Copy pointers */
    
    return (HTStructured*) me;
}


/*	Record error message as a hypertext object
**	------------------------------------------
**
**	The error message should be marked as an error so that
**	it can be reloaded later.
**	This implementation just throws up an error message
**	and leaves the document unloaded.
**	A smarter implementation would load an error document,
**	marking at such so that it is retried on reload.
**
** On entry,
**	sink 	is a stream to the output device if any
**	number	is the HTTP error number
**	message	is the human readable message.
**
** On exit,
**	returns	a negative number to indicate lack of success in the load.
*/

PUBLIC int HTLoadError ARGS3(
	HTStream *, 	sink,
	int,		number,
	WWW_CONST char *,	message)
{
    HTAlert(message);		/* @@@@@@@@@@@@@@@@@@@ */
#if 0
    return -number;
#endif
    return -1;
} 

