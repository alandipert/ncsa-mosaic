/*		Manage different file formats			HTFormat.c
**		=============================
**
*/
#include <unistd.h>

#include "../config.h"
/* Connection: Keep-Alive support -bjs */
#include "HTMIME.h"

#include "HTFormat.h"

PUBLIC float HTMaxSecs = 1e10;		/* No effective limit */
PUBLIC float HTMaxLength = 1e10;	/* No effective limit */

#include "HTUtils.h"
#include "tcp.h"

#include "HTMLDTD.h"
#include "HText.h"
#include "HTAlert.h"
#include "HTList.h"
#include "HTInit.h"
#include "HTFWriter.h"
#include "HTPlain.h"
#include "SGML.h"
#include "HTML.h"
#include "HTMLGen.h"
#include "HTTCP.h"

/* From gui-documents.c. */
extern int loading_inlined_images;

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

PUBLIC	BOOL HTOutputSource = NO;	/* Flag: shortcut parser to stdout */
extern  BOOL interactive;

struct _HTStream {
      WWW_CONST HTStreamClass*	isa;
      /* ... */
};


/* Whoooooooooooooa ugly!!! */
int loading_length = -1;
int noLength=1;

/*SWP -- Even Uglier*/
extern int ftpKludge;

/*	Presentation methods
**	--------------------
*/

PUBLIC  HTList * HTPresentations = 0;
PUBLIC  HTPresentation* default_presentation = 0;


/*	Define a presentation system command for a content-type
**	-------------------------------------------------------
*/
PUBLIC void HTSetPresentation ARGS5(
	WWW_CONST char *, representation,
	WWW_CONST char *, command,
	float,	quality,
	float,	secs,
	float,	secs_per_byte
){

    HTPresentation * pres = (HTPresentation *)malloc(sizeof(HTPresentation));

    pres->rep = HTAtom_for(representation);
    pres->rep_out = WWW_PRESENT;		/* Fixed for now ... :-) */
    pres->converter = HTSaveAndExecute;		/* Fixed for now ...     */
    pres->quality = quality;
    pres->secs = secs;
    pres->secs_per_byte = secs_per_byte;
    pres->rep = HTAtom_for(representation);
    pres->command = 0;
    StrAllocCopy(pres->command, command);

    if (!HTPresentations) HTPresentations = HTList_new();

    if (strcmp(representation, "*")==0) {
        if (default_presentation) free(default_presentation);
	default_presentation = pres;
    } else {
        HTList_addObjectAtEnd(HTPresentations, pres);
    }
}


/*	Define a built-in function for a content-type
**	---------------------------------------------
*/
PUBLIC void HTSetConversion ARGS6(
	WWW_CONST char *, representation_in,
	WWW_CONST char *, representation_out,
	HTConverter*,	converter,
	float,	quality,
	float,	secs,
	float,	secs_per_byte
){

    HTPresentation * pres = (HTPresentation *)malloc(sizeof(HTPresentation));

    pres->rep = HTAtom_for(representation_in);
    pres->rep_out = HTAtom_for(representation_out);
    pres->converter = converter;
    pres->command = NULL;		/* Fixed */
    pres->quality = quality;
    pres->secs = secs;
    pres->secs_per_byte = secs_per_byte;
    pres->command = 0;

    if (!HTPresentations) HTPresentations = HTList_new();

    if (strcmp(representation_in, "*")==0) {
        if (default_presentation) free(default_presentation);
	default_presentation = pres;
    } else {
        HTList_addObject(HTPresentations, pres);
    }
}


/********************ddt*/
/*
** Remove a conversion routine from the presentation list.
** The conversion routine must match up with the given args.
*/
PUBLIC void HTRemoveConversion ARGS3(
	WWW_CONST char *, representation_in,
	WWW_CONST char *, representation_out,
	HTConverter*,	converter
){
int numberOfPresentations;
HTPresentation * pres;
HTAtom *rep_in, *rep_out;
int x;


    numberOfPresentations = HTList_count(HTPresentations);

    rep_in = HTAtom_for(representation_in);
    rep_out = HTAtom_for(representation_out);

    for (x = 0; x < numberOfPresentations; x++) {
        pres = HTList_objectAt(HTPresentations, x);
	if (pres) {
		if ((!strcmp(pres->rep->name,rep_in->name)) &&
		    (!strcmp(pres->rep_out->name,rep_out->name)) &&
		    (pres->converter == converter)) {
			HTList_removeObject(HTPresentations,pres);
			}
		}

	}
}

/***************** end ddt*/


/*	File buffering
**	--------------
**
**	The input file is read using the macro which can read from
**	a socket or a file.
**	The input buffer size, if large will give greater efficiency and
**	release the server faster, and if small will save space on PCs etc.
*/
#define INPUT_BUFFER_SIZE 65536
PRIVATE char input_buffer[INPUT_BUFFER_SIZE];
PRIVATE char * input_pointer;
PRIVATE char * input_limit;
PRIVATE int input_file_number;


/*	Set up the buffering
**
**	These routines are public because they are in fact needed by
**	many parsers, and on PCs and Macs we should not duplicate
**	the static buffer area.
*/
PUBLIC void HTInitInput ARGS1 (int,file_number)
{
    input_file_number = file_number;
    input_pointer = input_limit = input_buffer;
}

PUBLIC int interrupted_in_htgetcharacter = 0;
PUBLIC char HTGetCharacter NOARGS
{
  char ch;
  interrupted_in_htgetcharacter = 0;
  do
    {
      if (input_pointer >= input_limit)
        {
          int status =
            NETREAD(input_file_number, input_buffer, INPUT_BUFFER_SIZE);
          if (status <= 0)
            {
              if (status == 0)
                return (char)EOF;
              if (status == HT_INTERRUPTED)
                {
#ifndef DISABLE_TRACE
                  if (www2Trace)
                    fprintf (stderr, "HTFormat: Interrupted in HTGetCharacter\n");
#endif
                  interrupted_in_htgetcharacter = 1;
                  return (char)EOF;
                }
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf(stderr,
                        "HTFormat: File read error %d\n", status);
#endif
              return (char)EOF;
	    }
          input_pointer = input_buffer;
          input_limit = input_buffer + status;
	}
      ch = *input_pointer++;
    }
  while (ch == (char) 13); /* Ignore ASCII carriage return */

  return ch;
}

/*	Stream the data to an ouput file as binary
*/
PUBLIC int HTOutputBinary ARGS2( int, 		input,
				  FILE *, 	output)
{
  do
    {
      int status = NETREAD(input, input_buffer, INPUT_BUFFER_SIZE);
      if (status <= 0)
        {
          if (status == 0)
            return 0;
#ifndef DISABLE_TRACE
          if (www2Trace) fprintf(stderr,
                             "HTFormat: File read error %d\n", status);
#endif
          return 2;			/* Error */
        }
      fwrite(input_buffer, sizeof(char), status, output);
    } while (YES);
}


static int partial_wildcard_matches (HTFormat r1, HTFormat r2)
{
  /* r1 is the presentation format we're currently looking at out
     of the list we understand.  r2 is the one we need to get to. */
  char *s1, *s2, *subtype1 = NULL, *subtype2 = NULL;
  int i;

  s1 = HTAtom_name (r1);
  s2 = HTAtom_name (r2);

  if (!s1 || !s2)
    return 0;

  s1 = strdup (s1);
  s2 = strdup (s2);

  for (i = 0; i < strlen (s1); i++)
    if (s1[i] == '/')
      {
        s1[i] = '\0';
        subtype1 = &(s1[i+1]);
        /* Now s1 contains the main type and subtype1 contains
           the subtype. */
        goto done1;
      }

 done1:
  if (!subtype1)
    goto nope;

  /* Bail if we don't have a wildcard possibility. */
  if (subtype1[0] != '*')
    goto nope;

  for (i = 0; i < strlen (s2); i++)
    if (s2[i] == '/')
      {
        s2[i] = '\0';
        subtype2 = &(s2[i+1]);
        /* Now s2 contains the main type and subtype2 contains
           the subtype. */
        goto done2;
      }

 done2:
  if (!subtype2)
    goto nope;

  /* Bail if s1 and s2 aren't the same and s1[0] isn't '*'. */
  if (strcmp (s1, s2) && s1[0] != '*')
    goto nope;

  /* OK, so now either we have the same main types or we have a wildcard
     type for s1.  We also know that we have a wildcard possibility in
     s1.  Therefore, at this point, we have a match. */
  free (s1);
  free (s2);
  return 1;

 nope:
  free (s1);
  free (s2);
  return 0;
}


/*		Create a filter stack
**		---------------------
**
**	If a wildcard match is made, a temporary HTPresentation
**	structure is made to hold the destination format while the
**	new stack is generated. This is just to pass the out format to
**	MIME so far.  Storing the format of a stream in the stream might
**	be a lot neater.
*/
PUBLIC HTStream * HTStreamStack ARGS5(
	HTFormat,		format_in,
	HTFormat,		rep_out,
        int,                    compressed,
	HTStream*,		sink,
	HTParentAnchor*,	anchor)
{
  HTAtom * wildcard = HTAtom_for("*");
  HTPresentation temp;

  /* Inherit force_dump_to_file from mo-www.c. */
  extern int force_dump_to_file;

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf(stderr,
            "[HTStreamStack] Constructing stream stack for %s to %s\n",
            HTAtom_name(format_in),
            HTAtom_name(rep_out));
#endif
#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr,
             "               Compressed is %d\n", compressed);
#endif

  if (rep_out == WWW_SOURCE ||
      rep_out == format_in)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr,
                 "[HTStreamStack] rep_out == WWW_SOURCE | rep_out == format_in; returning sink\n");
#endif
      return sink;
    }

  if (!HTPresentations)
    HTFormatInit();	/* set up the list */

  if (force_dump_to_file && format_in != WWW_MIME)
    {
      return HTSaveAndExecute (NULL, anchor, sink, format_in, compressed);
    }

  {
    int n = HTList_count(HTPresentations);
    int i;
    HTPresentation * pres;
    for(i=0; i<n; i++)
      {
        pres = HTList_objectAt(HTPresentations, i);
#ifndef DISABLE_TRACE
        if (www2Trace)
          {
            fprintf (stderr, "HTFormat: looking at pres '%s'\n",
                     HTAtom_name (pres->rep));
            if (pres->command)
              fprintf (stderr, "HTFormat: pres->command is '%s'\n",
                       pres->command);
            else
              fprintf (stderr, "HTFormat: pres->command doesn't exist\n");
          }
#endif
        if (pres->rep == format_in ||
            partial_wildcard_matches (pres->rep, format_in))
          {
            if (pres->command && strstr (pres->command, "mosaic-internal-present"))
              {
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr, "[HTStreamStack] HEY HEY HEY caught internal-present\n");
#endif
                return HTPlainPresent (pres, anchor, sink, format_in, compressed);
              }
            if (pres->rep_out == rep_out)
              {
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr,
                           "[HTStreamStack] pres->rep_out == rep_out\n");
#endif
                return (*pres->converter)(pres, anchor, sink, format_in, compressed);
              }
            if (pres->rep_out == wildcard)
              {
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr,
                           "[HTStreamStack] pres->rep_out == wildcard\n");
#endif
                temp = *pres;/* make temp conversion to needed fmt */
                temp.rep_out = rep_out;		/* yuk */
                return (*pres->converter)(&temp, anchor, sink, format_in, compressed);
              }
          }
      }
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
    {
      fprintf (stderr, "[HTStreamStack] Returning NULL at bottom.\n");
    }
#endif

  return NULL;
}


/*		Find the cost of a filter stack
**		-------------------------------
**
**	Must return the cost of the same stack which StreamStack would set up.
**
** On entry,
**	length	The size of the data to be converted
*/
PUBLIC float HTStackValue ARGS4(
	HTFormat,		format_in,
	HTFormat,		rep_out,
	float,			initial_value,
	long int,		length)
{
    HTAtom * wildcard = HTAtom_for("*");

#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr,
    	"HTFormat: Evaluating stream stack for %s worth %.3f to %s\n",
	HTAtom_name(format_in),	initial_value,
	HTAtom_name(rep_out));
#endif

    if (rep_out == WWW_SOURCE ||
    	rep_out == format_in) return 0.0;

    if (!HTPresentations) HTFormatInit();	/* set up the list */

    {
	int n = HTList_count(HTPresentations);
	int i;
	HTPresentation * pres;
	for(i=0; i<n; i++) {
	    pres = HTList_objectAt(HTPresentations, i);
	    if (pres->rep == format_in && (
	    		pres->rep_out == rep_out ||
			pres->rep_out == wildcard)) {
	        float value = initial_value * pres->quality;
		if (HTMaxSecs != 0.0)
		value = value - (length*pres->secs_per_byte + pres->secs)
			                 /HTMaxSecs;
		return value;
	    }
	}
    }

    return -1e30;		/* Really bad */

}


/*	Push data from a socket down a stream
**	-------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/

#define SWP_HACK

PUBLIC int HTCopy ARGS3(int,			file_number,
                         HTStream*,		sink,
                         int,                   bytes_already_read)
{
  HTStreamClass targetClass;
  char line[256];
  char *msg;
  int bytes = bytes_already_read;
  extern int twirl_increment;
  int next_twirl = twirl_increment;
  int rv = 0;

  int left = -1, total_read = bytes_already_read, hdr_len = 0;

/*  if(loading_length != -1) left = loading_length;*/

  HTClearActiveIcon();

  /*	Push the data down the stream
   **
   */
  targetClass = *(sink->isa);	/* Copy pointers to procedures */

  hdr_len = HTMIME_get_header_length(sink);

      /*	Push binary from socket down sink */
  for(;;)
    {
      int status, intr;

      if (bytes > next_twirl)
        {
          intr = HTCheckActiveIcon(1);
          next_twirl += twirl_increment;
        }
      else
        {
          intr = HTCheckActiveIcon(0);
        }
      if (intr)
        {
#ifdef SWP_HACK
	  loading_length=(-1);
#endif
          HTProgress ("Data transfer interrupted.");
	  noLength=0;
	  HTMeter(100,NULL);
	  noLength=1;
          (*targetClass.handle_interrupt)(sink);
          rv = -1;
          goto ready_to_leave;
        }

      if(loading_length == -1) {
	  left = -1;
	  status = NETREAD(file_number, input_buffer, INPUT_BUFFER_SIZE);
      } else {
	  left = (loading_length+hdr_len)-total_read;
	  if(left>0) status = NETREAD(file_number, input_buffer,
                                      (left>INPUT_BUFFER_SIZE?
                                       INPUT_BUFFER_SIZE:left));
          else status=0;
      }

      if (status > 0)
	  total_read += status;

/*      fprintf(stderr,"ll = %d status = %d left = %d hdr = %d tr = %d\n",
              loading_length,status,left,hdr_len,total_read);
              */

/*
      status = NETREAD(file_number, input_buffer, INPUT_BUFFER_SIZE);
*/
      if (status <= 0)
        {
          if (status == 0)
            break;
          if (status == HT_INTERRUPTED)
            {
#ifdef SWP_HACK
	      loading_length=(-1);
#endif
              HTProgress ("Data transfer interrupted.");
	      noLength=0;
	      HTMeter(100,NULL);
	      noLength=1;
              (*targetClass.handle_interrupt)(sink);
              rv = -1;
              goto ready_to_leave;
            }
          if (errno == ENOTCONN || errno == ECONNRESET || errno == EPIPE)
            {
              /* Arrrrgh, HTTP 0/1 compability problem, maybe. */
              rv = -2;
              goto ready_to_leave;
            }
          break;
	}

#ifndef DISABLE_TRACE
      if (www2Trace)
	  fprintf (stderr, "HTCopy: put_block on input_buffer '%s'\n", input_buffer);
#endif

      (*targetClass.put_block)(sink, input_buffer, status);

      if (ftpKludge) {
	hdr_len=0;
      }
      else {
	hdr_len = HTMIME_get_header_length(sink);
      }

/*	  left = loading_length - total_read;*/

      bytes += status;

      /* moved msg stuff here as loading_length may change midstream -bjs*/
      if (loading_length == -1){
	  msg = (loading_inlined_images ?
		 "Read %d bytes of inlined image data." :
		 "Read %d bytes of data.");
        sprintf (line, msg, bytes);
/*	HTMeter(0,NULL);*/
      }else{
	  msg = (loading_inlined_images ?
		 "Read %d of %d bytes of inlined image data." :
		 "Read %d of %d bytes of data.");
        sprintf (line, msg, bytes, loading_length+hdr_len);
	HTMeter((bytes*100)/(loading_length+hdr_len),NULL);
      }
      HTProgress (line);
      if((loading_length != -1) && (total_read>=(loading_length+hdr_len))) {
/*	  fprintf(stderr,"done\n");*/
	  break;
      }
    } /* next bufferload */

/*
  HTProgress (loading_inlined_images ?
              "Data transfer complete." : "Data transfer complete.");
*/
  HTProgress("Data transfer complete.");
  noLength=0;
  HTMeter(100,NULL);
  noLength=1;

/*  fprintf(stderr,"HTFormat: KeepAlive Exit\n");*/
/*
  NETCLOSE (file_number);
*/

  /* Success. */
  rv = 0;

 ready_to_leave:
  /* Reset ourselves so we don't get confused. */
  loading_length = -1;

  return rv;
}



/*	Push data from a file pointer down a stream
**	-------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**
*/
PUBLIC void HTFileCopy ARGS2(
	FILE *,			fp,
	HTStream*,		sink)
{
    HTStreamClass targetClass;

    targetClass = *(sink->isa);	/* Copy pointers to procedures */

    for(;;) {
	int status = fread(input_buffer, 1, INPUT_BUFFER_SIZE, fp);
	if (status == 0) { /* EOF or error */
	    if (ferror(fp) == 0) break;
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr,
		"HTFormat: Read error, read returns %d\n", ferror(fp));
#endif
	    break;
	}
	(*targetClass.put_block)(sink, input_buffer, status);
    } /* next bufferload */

    fclose (fp);
    return;
}


PUBLIC void HTFileCopyToText ARGS2(
	FILE *,			fp,
	HText *,		text)
{
  for(;;)
    {
      int status = fread(input_buffer, 1, INPUT_BUFFER_SIZE, fp);
      if (status == 0)
        { /* EOF or error */
          if (ferror(fp) == 0) break;
#ifndef DISABLE_TRACE
          if (www2Trace) fprintf(stderr,
                             "HTFormat: Read error, read returns %d\n", ferror(fp));
#endif
          break;
        }
      HText_appendBlock (text, input_buffer, status);
  } /* next bufferload */

  fclose (fp);
  return;
}


/*	Parse a socket given format and file number
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/
PUBLIC int HTParseSocket ARGS6(
	HTFormat,		format_in,
	HTFormat,		format_out,
	HTParentAnchor *,	anchor,
	int,			file_number,
	HTStream*,		sink,
        int,                    compressed)
{
  HTStream * stream;
  HTStreamClass targetClass;
  int rv;

  stream = HTStreamStack(format_in,
                         format_out,
                         compressed,
                         sink, anchor);

  if (!stream)
    {
      char buffer[1024];	/* @@@@@@@@ */
      sprintf(buffer, "Sorry, can't convert from %s to %s.",
              HTAtom_name(format_in), HTAtom_name(format_out));
#ifndef DISABLE_TRACE
      if (www2Trace) fprintf(stderr, "HTFormat: %s\n", buffer);
#endif
      return HTLoadError(sink, 501, buffer);
    }

  targetClass = *(stream->isa);	/* Copy pointers to procedures */
  rv = HTCopy(file_number, stream, 0);
  if (rv == -1)
    {
      /* handle_interrupt should have been done in HTCopy */
      /* (*targetClass.handle_interrupt)(stream); */
      return HT_INTERRUPTED;
    }

  (*targetClass.end_document)(stream);

  /* New thing: we force close the data socket here, so that if
     an external viewer gets forked off in the free method below,
     the connection doesn't remain upon until the child exits --
     which it does if we don't do this. */
  NETCLOSE (file_number);

  (*targetClass.free)(stream);

  return HT_LOADED;
}



/*	Parse a file given format and file pointer
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream ie containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/
PUBLIC int HTParseFile ARGS6(
	HTFormat,		format_in,
	HTFormat,		format_out,
	HTParentAnchor *,	anchor,
	FILE *,			fp,
	HTStream*,		sink,
        int,                    compressed)
{
    HTStream * stream;
    HTStreamClass targetClass;

    stream = HTStreamStack(format_in,
                           format_out,
                           compressed,
                           sink , anchor);

    if (!stream) {
        char buffer[1024];	/* @@@@@@@@ */
	sprintf(buffer, "Sorry, can't convert from %s to %s.",
		HTAtom_name(format_in), HTAtom_name(format_out));
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr, "HTFormat(in HTParseFile): %s\n", buffer);
#endif
        return HTLoadError(sink, 501, buffer);
    }

    targetClass = *(stream->isa);	/* Copy pointers to procedures */
    HTFileCopy(fp, stream);
    (*targetClass.end_document)(stream);
    (*targetClass.free)(stream);

    return HT_LOADED;
}
