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

/* SOCKS mods by:
 * Ying-Da Lee, <ylee@syl.dl.nec.com>
 * NEC Systems Laboratory
 * C&C Software Technology Center
 */
#include "../config.h"
#include "mosaic.h"
#include "main.h"
#include "gui.h"
#include "pan.h"
#include "child.h"
#include "newsrc.h"
#include "hotlist.h"
#include "globalhist.h"
#include "cciBindings2.h"

#include <signal.h>
#include <sys/utsname.h>
#include <string.h>

/* swp */
#define _KCMS_H_
#include "kcms.h"

char *userPath=NULL;

void mo_exit (void)
{
  mo_write_default_hotlist ();
  newsrc_kill ();
  if (get_pref_boolean(eUSE_GLOBAL_HISTORY))
    mo_write_global_history ();
  mo_write_pan_list ();

  preferences_armegeddon();

#ifdef HAVE_DTM
  mo_dtm_disconnect ();
#endif

  exit (0);
}

#ifndef VMS
MO_SIGHANDLER_RETURNTYPE ProcessExternalDirective (MO_SIGHANDLER_ARGS)
{
  char filename[64];
  char line[MO_LINE_LENGTH], *status, *directive, *url;
  FILE *fp;
  

  signal (SIGUSR1, SIG_IGN);

  /* Construct filename from our pid. */
  sprintf (filename, "/tmp/Mosaic.%d", getpid ());

  fp = fopen (filename, "r");
  if (!fp)
    goto done;

  status = fgets (line, MO_LINE_LENGTH, fp);
  if (!status || !(*line)) {
    fclose(fp);
    goto done;
  }
  directive = strdup (line);

  /* We now allow URL to not exist, since some directives
     don't need it. */
  status = fgets (line, MO_LINE_LENGTH, fp);
  if (!status || !(*line))
    url = strdup ("dummy");
  else
    url = strdup (line);
  
  mo_process_external_directive (directive, url);

  free (directive);
  free (url);

  fclose(fp);

 done:
  signal (SIGUSR1, (void *)ProcessExternalDirective);
  return;
}  
#endif

static void RealFatal (void)
{
  signal (SIGBUS, 0);
  signal (SIGSEGV, 0);
  signal (SIGILL, 0);
  abort ();
}

#ifdef __STDC__
static void FatalProblem (int sig)
#else /* not __STDC__ */
#ifdef _HPUX_SOURCE
static MO_SIGHANDLER_RETURNTYPE FatalProblem
  (int sig, int code, struct sigcontext *scp,
                        char *addr)
#else
static MO_SIGHANDLER_RETURNTYPE FatalProblem
  (int sig, int code, struct sigcontext *scp, char *addr)
#endif
#endif /* not __STDC__ */
{
  fprintf (stderr, "\nCongratulations, you have found a bug in\n");
  fprintf (stderr, "NCSA Mosaic %s on %s.\n\n", MO_VERSION_STRING, 
           MO_MACHINE_TYPE);
  fprintf (stderr, "If a core file was generated in your directory,\n");
  fprintf (stderr, "please do one of the following:\n\n");
  fprintf (stderr, "  %% dbx /path/to/Mosaic /path/to/core\n");
  fprintf (stderr, "  dbx> where\n\n");
  fprintf (stderr, "OR\n\n");
  fprintf (stderr, "  %% gdb /path/to/Mosaic /path/to/core\n");
  fprintf (stderr, "  gdb> where\n\n");
  fprintf (stderr, "Mail the results, and a description of what you were doing at the time,\n");
  fprintf (stderr, "(include any URLs involved!) to %s.\n\nWe thank you for your support.\n\n", 
           MO_DEVELOPER_ADDRESS);
  fprintf (stderr, "...exiting NCSA Mosaic now.\n\n");

  RealFatal ();
}


main (int argc, char **argv, char **envp)
{
  struct utsname u;
  FILE *fp;
  int i;

	userPath=getenv("PATH");

/*
	if (getenv("XKEYSYMDB")==NULL) {
		fprintf(stderr,"If you have key binding problems, set the environment variable XKEYSYMDB\nto the location of the correct XKeysymDB file on your system.\n");
	}
*/

/*
	if (uname(&u)<0) {
		perror("uname");
	}
	else {
		if (!strcmp(u.sysname,"SunOS") && 
		    (!strcmp(u.release,"5.0")
		     || !strcmp(u.release,"5.1")
		     || !strcmp(u.release,"5.2")
		     || !strcmp(u.release,"5.3")
		     || !strcmp(u.release,"5.4")
		     || !strcmp(u.release,"5.5"))) {
			if (getenv("XKEYSYMDB")==NULL) {
				if (!(fp=fopen("/usr/openwin/lib/X11/XKeysymDB","r"))) {
					if (!(fp=fopen("/usr/openwin/lib/XKeysymDB","r"))) {
					}
					else {
						fclose(fp);
						putenv("XKEYSYMDB=/usr/openwin/lib/XKeysymDB");
					}
				}
				else {
					fclose(fp);
					putenv("XKEYSYMDB=/usr/openwin/lib/X11/XKeysymDB");
				}
			}
		}
	}
*/

  signal (SIGBUS, FatalProblem);
  signal (SIGSEGV, FatalProblem);
  signal (SIGILL, FatalProblem);

  /* Since we're doing lots of TCP, just ignore SIGPIPE altogether. */
  signal (SIGPIPE, SIG_IGN);

  InitChildProcessor();
  MoCCIPreInitialize();

#ifdef SVR4
  signal(SIGCLD, (void (*)())ChildTerminated);
#else
  signal(SIGCHLD, (void (*)())ChildTerminated);
#endif



#ifdef SOCKS
  SOCKSinit(argv[0]);
#endif

  CheckKCMS();

  mo_do_gui (argc, argv);
}
