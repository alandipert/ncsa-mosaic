/*		FILE WRITER			HTWrite.c
**		===========
**
*/
#include "../config.h"
#include "HTWriter.h"

#define BUFFER_SIZE 4096	/* Tradeoff */

#include "HTUtils.h"
#include "tcp.h"
#include <stdio.h>

char adbuf[1024];

#ifndef DISABLE_TRACE
extern int httpTrace;
extern int www2Trace;
#endif

/*		HTML Object
**		-----------
*/

struct _HTStream {
	WWW_CONST HTStreamClass *	isa;

	int	soc;
	char	*write_pointer;
	char 	buffer[BUFFER_SIZE];
};


/*	Write the buffer out to the socket
**	----------------------------------
*/

PRIVATE void flush ARGS1(HTStream *, me)
{
    char *read_pointer 	= me->buffer;
    char *write_pointer = me->write_pointer;

    while (read_pointer < write_pointer) {
        int status;
	status = NETWRITE(me->soc, me->buffer,
			write_pointer - read_pointer);

#ifndef DISABLE_TRACE
	if (httpTrace) {
		strncpy(adbuf, me->buffer, write_pointer - read_pointer);
		adbuf[write_pointer - read_pointer] = '\0';
		fprintf(stderr,"%s",adbuf);
	}
#endif

	if (status<0) {
#ifndef DISABLE_TRACE
	    if(www2Trace) fprintf(stderr,
	    "HTWrite: Error on socket output stream!!!\n");
#endif
	    return;
	}
	read_pointer = read_pointer + status;
    }
    me->write_pointer = me->buffer;
}


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

PRIVATE void HTWriter_put_character ARGS2(HTStream *, me, char, c)
{
    if (me->write_pointer == &me->buffer[BUFFER_SIZE]) flush(me);
    *me->write_pointer++ = c;
}



/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/
PRIVATE void HTWriter_put_string ARGS2(HTStream *, me, WWW_CONST char*, s)
{
    int l = strlen(s);
    if (me->write_pointer + l > &me->buffer[BUFFER_SIZE]) flush(me);
    strcpy(me->write_pointer, s);
    me->write_pointer = me->write_pointer + l;
}


/*	Buffer write.  Buffers can (and should!) be big.
**	------------
*/
PRIVATE void HTWriter_write ARGS3(HTStream *, me, WWW_CONST char*, s, int, l)
{
 
    WWW_CONST char *read_pointer 	= s;
    WWW_CONST char *write_pointer = s+l;

    flush(me);		/* First get rid of our buffer */

    while (read_pointer < write_pointer) {
        int status = NETWRITE(me->soc, read_pointer,
			write_pointer - read_pointer);

#ifndef DISABLE_TRACE
	if (httpTrace) {
		strncpy(adbuf, me->buffer, read_pointer - read_pointer);
		adbuf[write_pointer - read_pointer] = '\0';
		fprintf(stderr,"%s",adbuf);
	}
#endif

	if (status<0) {
#ifndef DISABLE_TRACE
	    if(www2Trace) fprintf(stderr,
	    "HTWriter_write: Error on socket output stream!!!\n");
#endif
	    return;
	}
	read_pointer = read_pointer + status;
    }
}




/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void HTWriter_free ARGS1(HTStream *, me)
{
    NETCLOSE(me->soc);
    flush(me);
    free(me);
}

/*	End writing
*/

PRIVATE void HTWriter_end_document ARGS1(HTStream *, me)
{
    flush(me);
}

PRIVATE void HTWriter_handle_interrupt ARGS1(HTStream *, me)
{
}



/*	Structured Object Class
**	-----------------------
*/
PRIVATE WWW_CONST HTStreamClass HTWriter = /* As opposed to print etc */
{		
	"SocketWriter",
	HTWriter_free,
	HTWriter_end_document,
	HTWriter_put_character, 	HTWriter_put_string,
	HTWriter_write,
        HTWriter_handle_interrupt
}; 


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStream* HTWriter_new ARGS1(int, soc)
{
    HTStream* me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTWriter_new");
    me->isa = &HTWriter;       
    
    me->soc = soc;
    me->write_pointer = me->buffer;
    return me;
}

/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStream* HTASCIIWriter ARGS1(int, soc)
{
    HTStream* me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTASCIIWriter");
    me->isa = &HTWriter;       

    me->soc = soc;
    me->write_pointer = me->buffer;
    return me;
}

