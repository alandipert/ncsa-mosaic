/*		Chunk handling:	Flexible arrays
**		===============================
**
*/
#include "../config.h"
#include "HTUtils.h"
#include "HTChunk.h"
#include <stdio.h>

/*	Create a chunk with a certain allocation unit
**	--------------
*/
PUBLIC HTChunk * HTChunkCreate ARGS1 (int,grow)
{
    HTChunk * ch = (HTChunk *) malloc(sizeof(HTChunk));
    if (ch == NULL) outofmem(__FILE__, "cretion of chunk");

    ch->data = 0;
    ch->growby = grow;
    ch->size = 0;
    ch->allocated = 0;
    return ch;
}


/*	Clear a chunk of all data
**	--------------------------
*/
PUBLIC void HTChunkClear ARGS1 (HTChunk *,ch)
{
    if (ch->data) {
	free(ch->data);
	ch->data = 0;
    }
    ch->size = 0;
    ch->allocated = 0;
}


/*	Free a chunk
**	------------
*/
PUBLIC void HTChunkFree ARGS1 (HTChunk *,ch)
{
    if (ch->data) free(ch->data);
    free(ch);
}


/*	Append a character
**	------------------
*/
PUBLIC void HTChunkPutc ARGS2 (HTChunk *,ch, char,c)
{
    if (ch->size >= ch->allocated) {
	ch->allocated = ch->allocated + ch->growby;
        ch->data = ch->data ? (char *)realloc(ch->data, ch->allocated)
			    : (char *)malloc(ch->allocated);
      if (!ch->data) outofmem(__FILE__, "HTChunkPutc");
    }
    ch->data[ch->size++] = c;
}


/*	Ensure a certain size
**	---------------------
*/
PUBLIC void HTChunkEnsure ARGS2 (HTChunk *,ch, int,needed)
{
    if (needed <= ch->allocated) return;
    ch->allocated = needed-1 - ((needed-1) % ch->growby)
    			     + ch->growby; /* Round up */
    ch->data = ch->data ? (char *)realloc(ch->data, ch->allocated)
			: (char *)malloc(ch->allocated);
    if (ch->data == NULL) outofmem(__FILE__, "HTChunkEnsure");
}


/*	Terminate a chunk
**	-----------------
*/
PUBLIC void HTChunkTerminate ARGS1 (HTChunk *,ch)
{
    HTChunkPutc(ch, (char)0);
}


/*	Append a string
**	---------------
*/
PUBLIC void HTChunkPuts ARGS2 (HTChunk *,ch, WWW_CONST char *,s)
{
    WWW_CONST char * p;
    for (p=s; *p; p++)
        HTChunkPutc(ch, *p);
}
