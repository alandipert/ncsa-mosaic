#include "../config.h"
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>


#include "HTFormat.h"
#include "HTFile.h"
#include "HTUtils.h"
#include "tcp.h"
#include "HTML.h"
#include "HTMLDTD.h"
#include "HText.h"
#include "HTAlert.h"
#include "HTList.h"
#include "HTInit.h"
#include "HTFWriter.h"
#include "HTPlain.h"
#include "SGML.h"
#include "HTMLGen.h"

#include "../libnut/system.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

struct _HTStream 
{
  WWW_CONST HTStreamClass*	isa;
  /* ... */
};

int is_uncompressed=0;

extern char *mo_tmpnam (char *);
extern void application_user_feedback (char *);
extern char *uncompress_program, *gunzip_program;

extern void HTFileCopyToText (FILE *fp, HText *text);

/* Given a filename of a local compressed file, compress it in place.

   We assume that the file does not already have a .Z or .z extension
   at this point -- this is a little weird but it's convenient. */
void HTCompressedFileToFile (char *fnam, int compressed)
{
  char *znam;
  char *cmd;
  int len;

  cmd=NULL;

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf 
      (stderr, "[HTCompressedFileToFile] Entered; fnam '%s', compressed %d\n",
       fnam, compressed);
#endif

  /* Punt if we can't handle it. */
  if (compressed != COMPRESSED_BIGZ && compressed != COMPRESSED_GNUZIP)
    return;

  HTProgress ("Preparing to uncompress data.");
  
  znam = (char *)malloc (sizeof (char) * (strlen (fnam) + 8));

  /* Either compressed or gzipped. */
  if (compressed == COMPRESSED_BIGZ)
    sprintf (znam, "%s.Z", fnam);
  else
    sprintf (znam, "%s.gz", fnam);

/*SWP -- New "mv" fucntion to take care of these /bin/mv things*/
  {
  char retBuf[BUFSIZ];
  int status;

     if ((status=my_move(fnam,znam,retBuf,BUFSIZ,1))!=SYS_SUCCESS) {
	sprintf(retBuf,"Unable to uncompress compressed data;\nresults may be in error.\n%s",retBuf);
	application_user_info_wait(retBuf);
	free (znam);
	return;
     }
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "[HTCompressedFileToFile] Moved '%s' to '%s'\n",
             fnam, znam);
#endif

  if (compressed == COMPRESSED_BIGZ)
    {
      cmd = (char *)malloc(strlen(uncompress_program)+strlen(znam)+8);
      sprintf (cmd, "%s %s", uncompress_program, znam);
    }
  else
    {
      cmd = (char *)malloc (strlen (gunzip_program) + strlen (znam) + 8);
      sprintf (cmd, "%s %s", gunzip_program, znam);
    }

  HTProgress ("Uncompressing data.");

  {
  int status,skip_output=0;
  char retBuf[BUFSIZ];
  char final[BUFSIZ];

	*retBuf='\0';
	*final='\0';

	if ((status=my_system(cmd,retBuf,BUFSIZ))!=SYS_SUCCESS) {
		switch(status) {
			case SYS_NO_COMMAND:
				sprintf(final,"%sThere was no command to execute.\n",final);
				break;
			case SYS_FORK_FAIL:
				sprintf(final,"%sThe fork call failed.\n",final);
				break;
			case SYS_PROGRAM_FAILED:
				sprintf(final,"%sThe program specified was not able to exec.\n",final);
				break;
			case SYS_NO_RETBUF:
				sprintf(final,"%sThere was no return buffer.\n",final);
				break;
			case SYS_FCNTL_FAILED:
				sprintf(final,"%sFcntl failed to set non-block on the pipe.\n",final);
				break;
		}
		/*give them the output*/
		if (*retBuf) {
			sprintf(final,"%s%s",final,retBuf);
		}
	}
	else if (*retBuf) {
		/*give them the output*/
		sprintf(final,"%s%s",final,retBuf);
	}
	else {
		/*a-okay*/
		skip_output=1;
	}

	if (!skip_output) {
		application_user_info_wait(final);
		free (cmd);
		free (znam);
		HTProgress ("Uncompress failed.");
		return;
	}
  }

  HTProgress ("Data uncompressed.");

  is_uncompressed=1;

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf 
      (stderr, "[HTCompressedFileToFile] Uncompressed '%s' with command '%s'\n",
       znam, cmd);
#endif
  
  free (cmd);
  free (znam);

  return;
}


void HTCompressedHText (HText *text, int compressed, int plain)
{
  char *fnam;
  char *znam;
  char *cmd;
  FILE *fp;
  int rv, size_of_data;
  
#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf 
      (stderr, "[HTCompressedHText] Entered; compressed %d\n",
       compressed);
#endif

  /* Punt if we can't handle it. */
  if (compressed != COMPRESSED_BIGZ && compressed != COMPRESSED_GNUZIP)
    return;

  /* Hmmmmmmmmm, I'm not sure why we subtract 1 here, but it is
     indeed working... */
  size_of_data = HText_getTextLength (text) - 1;

  if (size_of_data == 0)
    {
      fprintf (stderr, "[HTCompressedHText] size_of_data 0; punting\n");
      return;
    }
  
  fnam = mo_tmpnam ((char *) 0);
  fp = fopen (fnam, "w");
  if (!fp)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "COULD NOT OPEN TMP FILE '%s'\n", fnam);
#endif
      application_user_feedback
        ("Unable to uncompress compressed data;\nresults may be in error.");
      free (fnam);
      return;
    }

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "[HTCmopressedHText] Going to write %d bytes.\n",
             size_of_data);
#endif
  rv = fwrite (HText_getText (text), sizeof (char), size_of_data, fp);
  if (rv != size_of_data)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "ONLY WROTE %d bytes\n", rv);
#endif
      application_user_feedback
        ("Unable to write compressed data to local disk;\nresults may be in error.");
    }
  fclose (fp);

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "HTCompressedHText: Calling CompressedFileToFile\n");
#endif
  HTCompressedFileToFile (fnam, compressed);

  HText_clearOutForNewContents (text);

  HText_beginAppend (text);
  
  if (plain)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTCompressedHText] Throwing in PLAINTEXT token...\n");
#endif
      HText_appendText(text, "<PLAINTEXT>\n");
    }

  fp = fopen (fnam, "r");
  if (!fp)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "COULD NOT OPEN TMP FILE FOR READING '%s'\n", fnam);
#endif
      /* We already get error dialog up above. */
      free (fnam);
      return;
    }

  HTFileCopyToText (fp, text);

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "[HTCompressedHText] I think we're done...\n");
#endif

/*SWP*/
/*
  cmd = (char *)malloc (sizeof (char) * (strlen (fnam) + 32));
  sprintf (cmd, "/bin/rm -f %s", fnam);
  system (cmd);
  free (cmd);
*/
  unlink(fnam);
  
  return;
}
