/*		HTML Generator
**		==============
**
**	This version of the HTML object sends HTML markup to the output stream.
**
** Bugs:	Line wrapping is not done at all.
**		All data handled as PCDATA.
**		Should convert old XMP, LISTING and PLAINTEXT to PRE.
**
**	It is not obvious to me right now whether the HEAD should be generated
**	from the incomming data or the anchor.  Currently it's from the former
**	which is cleanest.
*/
#include "../config.h"
/* Implements:
*/
#include "HTMLGen.h"

#include <stdio.h>
#include "HTMLDTD.h"
#include "HTStream.h"
#include "SGML.h"
#include "HTFormat.h"

#define PUTC(c) (*me->targetClass.put_character)(me->target, c)
#define PUTS(s) (*me->targetClass.put_string)(me->target, s)
#define PUTB(s,l) (*me->targetClass.put_block)(me->target, s, l)

/*		HTML Object
**		-----------
*/

struct _HTStream {
	WWW_CONST HTStreamClass *		isa;	
	HTStream * 			target;
	HTStreamClass			targetClass;	/* COPY for speed */
};

struct _HTStructured {
	WWW_CONST HTStructuredClass *	isa;
	HTStream * 			target;
	HTStreamClass			targetClass;	/* COPY for speed */
};


/*	Character handling
**	------------------
*/
PRIVATE void HTMLGen_put_character ARGS2(HTStructured *, me, char, c)
{
    PUTC(c);
}



/*	String handling
**	---------------
*/
PRIVATE void HTMLGen_put_string ARGS2(HTStructured *, me, WWW_CONST char*, s)
{
    PUTS(s);
}

PRIVATE void HTMLGen_write ARGS3(HTStructured *, me, WWW_CONST char*, s, int, l)
{
    PUTB(s,l);
}


/*	Start Element
**	-------------
*/
PRIVATE void HTMLGen_start_element ARGS4(
	HTStructured *, 	me,
	int,			element_number,
	WWW_CONST BOOL*,	 	present,
	WWW_CONST char **,		value)
{
    int i;

    HTTag * tag = &HTML_dtd.tags[element_number];
    PUTC('<');
    PUTS(tag->name);
    if (present) for (i=0; i< tag->number_of_attributes; i++) {
        if (present[i]) {
	    PUTC(' ');
	    PUTS(tag->attributes[i].name);
	    if (value[i]) {
	 	PUTS("=\"");
		PUTS(value[i]);
		PUTC('"');
	    }
	}
    }
    PUTC('>');
}


/*		End Element
**		-----------
**
*/
/*	When we end an element, the style must be returned to that
**	in effect before that element.  Note that anchors (etc?)
**	don't have an associated style, so that we must scan down the
**	stack for an element with a defined style. (In fact, the styles
**	should be linked to the whole stack not just the top one.)
**	TBL 921119
*/
PRIVATE void HTMLGen_end_element ARGS2(HTStructured *, me,
			int , element_number)
{
    PUTS("</");
    PUTS(HTML_dtd.tags[element_number].name);
    PUTC('>');
}


/*		Expanding entities
**		------------------
**
*/

PRIVATE void HTMLGen_put_entity ARGS2(HTStructured *, me, int, entity_number)
{
    PUTC('&');
    PUTS(HTML_dtd.entity_names[entity_number]);
    PUTC(';');
}



/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void HTMLGen_free ARGS1(HTStructured *, me)
{
    (*me->targetClass.free)(me->target);	/* ripple through */
    free(me);
}



PRIVATE void HTMLGen_end_document ARGS1(HTStructured *, me)
{
    PUTC('\n');		/* Make sure ends with newline for sed etc etc */
    (*me->targetClass.end_document)(me->target);
}


PRIVATE void HTMLGen_handle_interrupt ARGS1(HTStructured *, me)
{
    (*me->targetClass.handle_interrupt)(me->target);
}


PRIVATE void PlainToHTML_end_document ARGS1(HTStructured *, me)
{
    PUTS("</PRE></BODY>\n");/* Make sure ends with newline for sed etc etc */
    (*me->targetClass.end_document)(me->target);
}



/*	Structured Object Class
**	-----------------------
*/
PRIVATE WWW_CONST HTStructuredClass HTMLGeneration = /* As opposed to print etc */
{		
	"text/html",
	HTMLGen_free,
	HTMLGen_end_document, HTMLGen_handle_interrupt,
	HTMLGen_put_character, 	HTMLGen_put_string, HTMLGen_write,
	HTMLGen_start_element, 	HTMLGen_end_element,
	HTMLGen_put_entity
}; 


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStructured * HTMLGenerator ARGS1(HTStream *, output)
{
    HTStructured* me = (HTStructured*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTMLGenerator");
    me->isa = &HTMLGeneration;       

    me->target = output;
    me->targetClass = *me->target->isa; /* Copy pointers to routines for speed*/

    return me;
}

/*	Stream Object Class
**	-------------------
**
**	This object just converts a plain text stream into HTML
**	It is officially a structured strem but only the stream bits exist.
**	This is just the easiest way of typecasting all the routines.
*/
PRIVATE WWW_CONST HTStructuredClass PlainToHTMLConversion =
{		
	"plaintexttoHTML",
	HTMLGen_free,	
	PlainToHTML_end_document,	
        HTMLGen_handle_interrupt,
	HTMLGen_put_character,
	HTMLGen_put_string,
	HTMLGen_write,
	NULL,		/* Structured stuff */
	NULL,
	NULL
}; 


/*	HTConverter from plain text to HTML Stream
**	------------------------------------------
*/

PUBLIC HTStream* HTPlainToHTML ARGS5(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink,
        HTFormat,               format_in,
        int,                    compressed)
{
    HTStream* me = (HTStream*)malloc(sizeof(*me));
    me->isa = (HTStreamClass*) &PlainToHTMLConversion;       

    me->target = sink;
    me->targetClass = *me->target->isa;
    	/* Copy pointers to routines for speed*/
	
    PUTS("<BODY>\n<PRE>\n");
    return me;
}
