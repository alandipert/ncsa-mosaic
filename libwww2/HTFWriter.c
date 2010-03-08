/*		FILE WRITER				HTFWrite.h
**		===========
**
**	This version of the stream object just writes to a C file.
**	The file is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/
#include "../config.h"
#include <string.h>

#include "HTFWriter.h"

#include "HTFormat.h"
#include "HTAlert.h"
#include "HTFile.h"
#include "HText.h"
#include "tcp.h"
#include "HTCompressed.h"

extern char *currentURL;

int imageViewInternal=0;

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/*		Stream Object
**		------------
*/

struct _HTStream {
	WWW_CONST HTStreamClass *	isa;
	
	FILE *			fp;
        char * fnam;
	char * 			end_command;
        int compressed;
        int interrupted;
        int write_error;
	char *mime_type;
};

/* MOSAIC: We now pick up some external variables, handled
   in src/mo-www.c: */
extern int force_dump_to_file;
extern char *force_dump_filename;
/* If force_dump_to_file is high, we know we want to dump the
   data into a file already named by force_dump_filename and not
   do anything else. */

/* If this is high, then we just want to dump the thing to a file;
   the file is named by force_dump_filename. */
extern int binary_transfer;

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

PRIVATE void HTFWriter_put_character ARGS2(HTStream *, me, char, c)
{
  int rv;

  if (me->write_error)
    return;

  /* Make sure argument to putc is in range 0-255, to avoid weirdness
     with rv == -1 == EOF when it's not supposed to. */
  rv = putc ((int)(unsigned char)c, me->fp);

  if (rv == EOF)
    {
      HTProgress ("Error writing to temporary file.");
      me->write_error = 1;
    }
}


/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/
PRIVATE void HTFWriter_put_string ARGS2(HTStream *, me, WWW_CONST char*, s)
{
  int rv;

  if (me->write_error)
    return;

  rv = fputs(s, me->fp);
  if (rv == EOF)
    {
      HTProgress ("Error writing to temporary file.");
      me->write_error = 1;
    }
}


/*	Buffer write.  Buffers can (and should!) be big.
**	------------
*/
PRIVATE void HTFWriter_write ARGS3(HTStream *, me, WWW_CONST char*, s, int, l)
{
  int rv;

  if (me->write_error)
    return;

  rv = fwrite(s, 1, l, me->fp); 
  if (rv != l)
    {
      HTProgress ("Error writing to temporary file.");
      me->write_error = 1;
    }
}

char *supportedTypes[]={
        "image/gif",
        "image/jpeg",
        "image/jpg",
        "image/png",
        "image/x-png",
        "image/x-pcd-jpeg",
        "image/x-pcd-jycc",
	"image/xpm",
	"image/xbm",
	"image/xpixmap",
	"image/xbitmap",
	"image/x-xpixmap",
	"image/x-xbitmap",
        "\n"
};

int supportedImageType(char *mt) {

int i;

	if (!mt || !*mt) {
		return(0);
	}

	for (i=0; supportedTypes[i][0]!='\n'; i++) {
		if (!strcmp(supportedTypes[i],mt)) {
			return(1);
		}
	}

	return(0);
}


/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created
**	object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void HTFWriter_free ARGS1(HTStream *, me)
{
  HText *text;
  static char *envbuf1=NULL;
  static char *envbuf2=NULL;

  /* I dunno if this is necessary... */
  if (me->interrupted)
    {
      free (me->fnam);
      free (me);
      return;
    }

  if (me->write_error)
    {
/*
      char *cmd = (char *)malloc ((strlen (me->fnam) + 32));
      sprintf (cmd, "/bin/rm -f %s &", me->fnam);
      system (cmd);
      free (cmd);
*/
/*ddt*/unlink(me->fnam);
      
      HTProgress ("Insufficient temporary disk space; could not transfer data.");

      free (me->fnam);
      free (me);
      return;
    }

  fflush (me->fp);
  fclose (me->fp);

  /* We do want to be able to handle compressed inlined images,
     but we don't want transparent uncompression to take place
     in binary transfer mode. */
  if (!binary_transfer && me->compressed != COMPRESSED_NOT)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTFWriter] Hi there; compressed is %d, fnam is '%s'\n",
                 me->compressed, me->fnam);
#endif
      HTCompressedFileToFile (me->fnam, me->compressed);
    }

  if (force_dump_to_file)
    {
      if (!binary_transfer)
        goto done;
    }

  /* Now, me->end_command can either be something starting with
     "<mosaic-internal-reference" or it can be a real command.
     Deal with appropriately. */
  if (me->end_command)
    {
	/* Check for forced dump condition.  The left paren comes
		from the construction of me->end_command as a compound shell
		command below. */
	if (strstr (me->end_command, "mosaic-internal-dump")) {
		rename_binary_file (me->fnam);
        }
	else if (!strstr (me->end_command, "mosaic-internal-reference")) {
		if (imageViewInternal && supportedImageType(me->mime_type)) {
			char *newHTML="<html>\n<head>\n<title>Mosaic's Internal Image Display</title>\n</head>\n<body>\n<img align=center src=\"%s\">\n</body>\n</html>\n";
			char *buf;

			buf=(char *)calloc((strlen(currentURL)+strlen(newHTML)+5),sizeof(char));
			sprintf(buf,newHTML,currentURL);

			text=HText_new();
			HText_beginAppend(text);
			HText_appendText(text,buf);
			HText_endAppend(text);

			free(buf);

			buf=(char *)calloc((strlen(currentURL)+strlen(me->fnam)+5),sizeof(char));
			sprintf(buf,"%s\n%s",me->fnam,currentURL);
			ImageResolve(NULL,buf,0);

			free(buf);

			goto done;
		}

		HTProgress("Spawning external viewer.");

		/*
		 * Have to dance around putenv since it makes "envbuf*" part
		 *   of the actual environment string...*sigh* What a mess!
		 */
		if (envbuf1) {
			envbuf2=(char *)calloc((strlen(currentURL)+
						strlen("MOSAIC_CURRENT_URL=")+
						2),
					       sizeof(char));
			sprintf(envbuf2,"MOSAIC_CURRENT_URL=%s",currentURL);
			putenv(envbuf2);
			free(envbuf1);
			envbuf1=NULL;
		}
		else if (envbuf2) {
			envbuf1=(char *)calloc((strlen(currentURL)+
						strlen("MOSAIC_CURRENT_URL=")+
						2),
					       sizeof(char));
			sprintf(envbuf1,"MOSAIC_CURRENT_URL=%s",currentURL);
			putenv(envbuf1);
			free(envbuf2);
			envbuf2=NULL;
		}
		else { /* Likely it is the first time */
			envbuf1=(char *)calloc((strlen(currentURL)+
						strlen("MOSAIC_CURRENT_URL=")+
						2),
					       sizeof(char));
			sprintf(envbuf1,"MOSAIC_CURRENT_URL=%s",currentURL);
			putenv(envbuf1);
		}

		system (me->end_command);

		if (envbuf1) {
			envbuf2=(char *)calloc((strlen("MOSAIC_CURRENT_URL=")+
						2),
					       sizeof(char));
			sprintf(envbuf2,"MOSAIC_CURRENT_URL=");
			putenv(envbuf2);
			free(envbuf1);
			envbuf1=NULL;
		}
		else if (envbuf2) {
			envbuf1=(char *)calloc((strlen("MOSAIC_CURRENT_URL=")+
						2),
					       sizeof(char));
			sprintf(envbuf1,"MOSAIC_CURRENT_URL=");
			putenv(envbuf1);
			free(envbuf2);
			envbuf2=NULL;
		}
		else { /* Likely it is the first time */
			envbuf1=(char *)calloc((strlen("MOSAIC_CURRENT_URL=")+
						2),
					       sizeof(char));
			sprintf(envbuf1,"MOSAIC_CURRENT_URL=");
			putenv(envbuf1);
		}
	}
	else {
          /* Internal reference, aka HDF file.  Just close output file. */
	}
    }
  else
    {
      /* No me->end_command; just close the file. */
    }

  /* Construct dummy HText thingie so Mosaic knows
     not to try to access this "document". */
  text = HText_new ();
  HText_beginAppend (text);
  /* If it's a real internal reference, tell Mosaic. */
  if (me->end_command) {
	if (strstr (me->end_command, "mosaic-internal-reference")) {
          HText_appendText (text, me->end_command);
	}
	else {
		HText_appendText (text, "<mosaic-access-override>\n");
	}
	free (me->end_command);
  }
  else {
      /* No me->end_command; just override the access. */
      HText_appendText (text, "<mosaic-access-override>\n");
  }
  HText_endAppend (text);

 done:
  if (binary_transfer)
    rename_binary_file (me->fnam);

 really_done:
  free (me->fnam);
  if (me->mime_type) {
	free(me->mime_type);
  }
  free (me);

  return;
}

/*	End writing
*/

PRIVATE void HTFWriter_end_document ARGS1(HTStream *, me)
{
  if (me->interrupted || me->write_error)
    return;

  fflush(me->fp);
}

PRIVATE void HTFWriter_handle_interrupt ARGS1(HTStream *, me)
{
/*  char *cmd;*/

  if (me->write_error)
    goto outtahere;

  /* Close the file, then kill it. */
  fclose (me->fp);

/*
  cmd = (char *)malloc ((strlen (me->fnam) + 32) * sizeof (char));
  sprintf (cmd, "/bin/rm -f %s &", me->fnam);
  system (cmd);
  free (cmd);
*/
/*ddt*/unlink(me->fnam);

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "*** HTFWriter interrupted; killed '%s'\n", me->fnam);
#endif
  
 outtahere:
  me->interrupted = 1;

  return;
}


/*	Structured Object Class
**	-----------------------
*/
PRIVATE WWW_CONST HTStreamClass HTFWriter = /* As opposed to print etc */
{		
	"FileWriter",
	HTFWriter_free,
	HTFWriter_end_document,
	HTFWriter_put_character, 	HTFWriter_put_string,
	HTFWriter_write,
        HTFWriter_handle_interrupt
}; 


/*	Take action using a system command
**	----------------------------------
**
**	Creates temporary file, writes to it, executes system command
**	on end-document.  The suffix of the temp file can be given
**	in case the application is fussy, or so that a generic opener can
**	be used.
**
**      WARNING: If force_dump_to_file is high, pres may be NULL
**      (as we may get called directly from HTStreamStack).
*/
PUBLIC HTStream* HTSaveAndExecute ARGS5(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	/* Not used */
	HTStream *,		sink,
        HTFormat,               format_in,
        int,                    compressed)	/* Not used */
{
  char *command;
  WWW_CONST char * suffix;
  
  HTStream* me;

  me = (HTStream*)malloc(sizeof(*me));
  me->isa = &HTFWriter;  
  me->interrupted = 0;
  me->write_error = 0;
  me->fnam = NULL;
  me->end_command = NULL;
  me->compressed = compressed;
  if (!format_in || !format_in->name || !*(format_in->name)) {
	me->mime_type=NULL;
  }
  else {
	if (!strncmp(format_in->name,"image",5)) {
		me->mime_type=strdup(format_in->name);
	}
	else {
		me->mime_type=NULL;
	}
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "[HTSaveAndExecute] me->compressed is '%d'\n",
             me->compressed);
#endif
  
  /* Save the file under a suitably suffixed name */
  
  if (!force_dump_to_file)
    {
      extern char *mo_tmpnam (char *);

      suffix = HTFileSuffix(pres->rep);
      
      me->fnam = mo_tmpnam(anchor->address);
      if (suffix) 
        {
          char *freeme = me->fnam;
         
          me->fnam = (char *)malloc (strlen (me->fnam) + strlen (suffix) + 8);
          strcpy(me->fnam, freeme);
          strcat(me->fnam, suffix);
          free (freeme);
        }
    }
  else
    {
      me->fnam = strdup (force_dump_filename);
    }

  me->fp = fopen (me->fnam, "w");
  if (!me->fp) 
    {
      HTProgress("Can't open temporary file -- serious problem.");
      me->write_error = 1;
      return me;
    }

  /* If force_dump_to_file is high, we're done here. */
  if (!force_dump_to_file)
    {
      if (!strstr (pres->command, "mosaic-internal-reference"))
        {
          /* If there's a "%s" in the command, or if the command
             is magic... */
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "HTFWriter: pres->command is '%s'\n",
                     pres->command);
#endif
          if (strstr (pres->command, "%s") ||
              strstr (pres->command, "mosaic-internal"))
            {
              /* Make command to process file */
              command = (char *)malloc 
                ((strlen (pres->command) + 10 + 3*strlen(me->fnam)) * 
                 sizeof (char));
              
              /* Cute.  pres->command will be something like "xv %s"; me->fnam
                 gets filled in as many times as appropriate.  */
              sprintf (command, pres->command, me->fnam, me->fnam, me->fnam);
              
              me->end_command = (char *)malloc 
                ((strlen (command) + 32 + strlen(me->fnam)) * sizeof (char));
              sprintf (me->end_command, "(%s ; /bin/rm -f %s) &",
                       command, me->fnam);

              free (command);
            }
          else
            {
              /* Make command to process file -- but we have to cat
                 to the viewer's stdin. */
              me->end_command = (char *)malloc 
                ((strlen (pres->command) + 64 + (2 * strlen(me->fnam))) * 
                 sizeof (char));
              sprintf (me->end_command, "((cat %s | %s); /bin/rm -f %s) &",
                       me->fnam, pres->command, me->fnam);
            }
        }
      else
        {
          /* Overload me->end_command to be what we should write out as text
             to communicate back to client code. */
          me->end_command = (char *)malloc
            (strlen ("mosaic-internal-reference") + strlen (me->fnam) + 32);
          sprintf (me->end_command, "<%s \"%s\">\n", "mosaic-internal-reference", me->fnam);
        }
    }
  
  return me;
}
