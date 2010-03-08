/*			Parse WAIS Source file			HTWSRC.c
**			======================
**
**	This module parses a stream with WAIS source file
**	format information on it and creates a structured stream.
**	That structured stream is then converted into whatever.
**
**	3 June 93	Bug fix: Won't crash if no description
*/
#include "../config.h"
#include "HTWSRC.h"

#include <stdio.h>
#include "HTML.h"

#include "HTUtils.h"
#include "tcp.h"
#include "HTParse.h"

#define BIG 10000		/* Arbitrary limit to value length */
#define PARAM_MAX BIG

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

struct _HTStructured {
	WWW_CONST HTStructuredClass *	isa;
	/* ... */
};

#define PUTC(c) (*me->target->isa->put_character)(me->target, c)
#define PUTS(s) (*me->target->isa->put_string)(me->target, s)
#define START(e) (*me->target->isa->start_element)(me->target, e, 0, 0)
#define END(e) (*me->target->isa->end_element)(me->target, e)


/*	Here are the parameters which can be specified in a  source file
*/
PRIVATE WWW_CONST char* par_name[] = {
	"version", 
	"ip-address",
#define PAR_IP_NAME 2
	"ip-name", 
#define PAR_TCP_PORT 3
	"tcp-port", 
#define PAR_DATABASE_NAME 4
	"database-name",
#define PAR_COST 5
	"cost", 
#define PAR_COST_UNIT 6
	"cost-unit", 
#define PAR_FREE 7
	"free",	
#define PAR_MAINTAINER 8
	"maintainer", 	
#define PAR_DESCRIPTION 9
	"description",
	"keyword-list", 	
	"source",
#define PAR_UNKNOWN 12
	"unknown",
	0,				/* Terminate list */
#define PAR_COUNT 13
} ;


enum tokenstate { beginning, before_tag, colon, before_value,
		value, bracketed_value, quoted_value, escape_in_quoted, done };


/*		Stream Object
**		------------
**
**	The target is the structured stream down which the
**	parsed results will go.
**
**	all the static stuff below should go in here to make it reentrant
*/

struct _HTStream {
	WWW_CONST HTStreamClass *	isa;
	HTStructured *		target;
	char *			par_value[PAR_COUNT];
	enum tokenstate 	state;
	char 			param[BIG+1];
	int			param_number;
	int			param_count;
};




PUBLIC WWW_CONST char * hex = "0123456789ABCDEF";

/*	Decode one hex character
*/

PUBLIC char from_hex ARGS1(char, c)
{
    return 		  (c>='0')&&(c<='9') ? c-'0'
			: (c>='A')&&(c<='F') ? c-'A'+10
			: (c>='a')&&(c<='f') ? c-'a'+10
			:		       0;
}


/*			State machine
**			-------------
**
** On entry,
**	me->state	is a valid state (see WSRC_init)
**	c		is the next character
** On exit,
** 	returns	1	Done with file
**		0	Continue. me->state is updated if necessary.
**		-1	Syntax error error
*/


/*		Treat One Character
**		-------------------
*/
PRIVATE void WSRCParser_put_character ARGS2(HTStream*, me, char, c)
{
    switch (me->state) {
    case beginning:
        if (c=='(') me->state = before_tag;
	break;
	
    case before_tag:
        if (c==')') {
	    me->state = done;
	    return;			/* Done with input file */
	} else if (c==':') {
	    me->param_count = 0;
	    me->state = colon;
	}				/* Ignore other text */
	break;

    case colon:
        if (WHITE(c)) {
	    me->param[me->param_count++] = 0;	/* Terminate */
	    for(me->param_number = 0; par_name[me->param_number]; me->param_number++) {
		if (0==strcmp(par_name[me->param_number], me->param)) {
		    break;
		}
	    }
	    if (!par_name[me->param_number]) {	/* Unknown field */
#ifndef DISABLE_TRACE
	        if (www2Trace) fprintf(stderr,
		    "HTWSRC: Unknown field `%s' in source file\n",
		    me->param);
#endif
		me->param_number = PAR_UNKNOWN;
		me->state = before_value;	/* Could be better ignore */
		return;
	    }
	    me->state = before_value;
	} else {
	    if (me->param_count < PARAM_MAX)  me->param[me->param_count++] = c;
	}
	break;
	
    case before_value:
        if (c==')') {
	    me->state = done;
	    return;			/* Done with input file */
	}
	if (WHITE(c)) return;		/* Skip white space */
	me->param_count = 0;
	if (c=='"') {
	    me->state = quoted_value;
	    break;
	}
	me->state = (c=='"') ? quoted_value : 
		    (c=='(') ? bracketed_value : value;
	me->param[me->param_count++] = c;	/* Don't miss first character */
	break;

    case value:
        if (WHITE(c)) {
	    me->param[me->param_count] = 0;
	    StrAllocCopy(me->par_value[me->param_number], me->param);
	    me->state = before_tag;
	} else {
	    if (me->param_count < PARAM_MAX)  me->param[me->param_count++] = c;
	}
	break;

    case bracketed_value:
        if (c==')') {
	    me->param[me->param_count] = 0;
	    StrAllocCopy(me->par_value[me->param_number], me->param);
	    me->state = before_tag;
	    break;
	}
        if (me->param_count < PARAM_MAX)  me->param[me->param_count++] = c;
	break;
	
    case quoted_value:
        if (c=='"') {
	    me->param[me->param_count] = 0;
	    StrAllocCopy(me->par_value[me->param_number], me->param);
	    me->state = before_tag;
	    break;
	}
	
	if (c=='\\') {		/* Ignore escape but switch state */
	    me->state = escape_in_quoted;
	    break;
	}
	if (me->param_count < PARAM_MAX)  me->param[me->param_count++] = c;
	break;

    case escape_in_quoted:
        if (me->param_count < PARAM_MAX)  me->param[me->param_count++] = c;
	me->state = quoted_value;
	break;
	
    case done:				/* Ignore anything after EOF */
	return;

    } /* switch me->state */
}


/*			Output equivalent HTML
**			----------------------
**
*/

void give_parameter ARGS2(HTStream *, me, int, p)
{
    PUTS(par_name[p]);
    if (me->par_value[p]) {
	PUTS(": ");
	PUTS(me->par_value[p]);
	PUTS("; ");
    } else {
        PUTS(" NOT GIVEN in source file; ");
    }
}


/*			Generate Outout
**			===============
*/
PRIVATE void WSRC_gen_html ARGS2(HTStream *, me, BOOL, source_file)

{
    if (me->par_value[PAR_DATABASE_NAME]) {
	char * shortname = 0;
	int l;
	StrAllocCopy(shortname, me->par_value[PAR_DATABASE_NAME]);
	l = strlen(shortname);
	if ( l > 4 && !strcasecomp(shortname + l -4, ".src")) {
	    shortname[l-4] = 0;	/* Chop of .src -- boring! */
	}
	
	START(HTML_TITLE);
	PUTS(shortname);
	PUTS(source_file ? " WAIS source file" : " index");
	END(HTML_TITLE);
    
	START(HTML_H1);
	PUTS(shortname);
	PUTS(source_file ? " description" : " index");
	END(HTML_H1);
    }
    
    START(HTML_DL);		/* Definition list of details */
    
    if (source_file) {
	START(HTML_DT);
	PUTS("Access links");
	START(HTML_DD);
	if (me->par_value[PAR_IP_NAME] &&
	    me->par_value[PAR_DATABASE_NAME]) {
    
	    char WSRC_address[256];
	    char * www_database;
	    www_database = HTEscape(me->par_value[PAR_DATABASE_NAME]);
	    sprintf(WSRC_address, "wais://%s:%s/%s",
		me->par_value[PAR_IP_NAME],
		me->par_value[PAR_TCP_PORT] ? me->par_value[PAR_TCP_PORT]
			: "210",
		www_database);
	
            PUTS ("<A HREF=\"");
            PUTS (WSRC_address);
            PUTS ("\">");
	    PUTS("Direct access");
            PUTS("</A>");
	    
	    PUTS(" or ");
	    
	    sprintf(WSRC_address, "http://www.ncsa.uiuc.edu:8001/%s:%s/%s",
		me->par_value[PAR_IP_NAME],
		me->par_value[PAR_TCP_PORT] ? me->par_value[PAR_TCP_PORT]
		: "210",
		www_database);
            PUTS ("<A HREF=\"");
            PUTS (WSRC_address);
            PUTS ("\">");
	    PUTS("through NCSA gateway");
            PUTS("</A>");
	    
	    free(www_database);
	    
	} else {
	    give_parameter(me, PAR_IP_NAME);
	    give_parameter(me, PAR_IP_NAME);
	}
    
    } /* end if source_file */
    
    if (me->par_value[PAR_MAINTAINER]) {
	START(HTML_DT);
	PUTS("Maintainer");
	START(HTML_DD);
	PUTS(me->par_value[PAR_MAINTAINER]);
    }
    START(HTML_DT);
    PUTS("Host");
    START(HTML_DD);
    PUTS(me->par_value[PAR_IP_NAME]);

    END(HTML_DL);

    if (me->par_value[PAR_DESCRIPTION]) {
	START(HTML_PRE);		/* Preformatted description */
	PUTS(me->par_value[PAR_DESCRIPTION]);
	END(HTML_PRE);
    }
    
    (*me->target->isa->end_document)(me->target);
    (*me->target->isa->free)(me->target);
    
    return;
} /* generate html */


PRIVATE void WSRCParser_put_string ARGS2(HTStream *, context, WWW_CONST char*, str)
{
    WWW_CONST char *p;
    for(p=str; *p; p++)
        WSRCParser_put_character(context, *p);
}


PRIVATE void WSRCParser_write ARGS3(
		HTStream *, 	context,
		WWW_CONST char*, 	str,
		int, 		l)
{
    WWW_CONST char *p;
    WWW_CONST char *e = str+l;
    for(p=str; p<e; p++)
        WSRCParser_put_character(context, *p);
}


PRIVATE void WSRCParser_free ARGS1(HTStream *, me)
{
    WSRC_gen_html(me, YES);
    {
	int p;
	for(p=0; par_name[p]; p++) {	/* Clear out old values */
	    if (me->par_value[p]) {
		free(me->par_value[p]);
	    }
	}
    }
    free(me);
}

PRIVATE void WSRCParser_end_document ARGS1(HTStream *, me)
{
/* Nothing */
}

PRIVATE void WSRCParser_handle_interrupt ARGS1(HTStream *, me)
{
/* Nothing */
}


/*		Stream subclass		-- method routines
**		---------------
*/

HTStreamClass WSRCParserClass = {
	"WSRCParser",
	WSRCParser_free,
	WSRCParser_end_document,
	WSRCParser_put_character,
 	WSRCParser_put_string,
	WSRCParser_write,
        WSRCParser_handle_interrupt

};


/*		Converter from WAIS Source to whatever
**		--------------------------------------
*/
PUBLIC HTStream* HTWSRCConvert ARGS5(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink,
        HTFormat,                format_in,
        int,                     compressed)
{
    HTStream * me = (HTStream*) malloc(sizeof(*me));

    me->isa = &WSRCParserClass;
    me->target = HTML_new(NULL, pres->rep_out, sink);

    {
	int p;
	for(p=0; p < PAR_COUNT; p++) {	/* Clear out parameter values */
	    me->par_value[p] = 0;
	}
    }
    me->state = beginning;

    return me;
}

