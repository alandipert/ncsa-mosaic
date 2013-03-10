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
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef VMS
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include <errno.h>
#include <string.h>

#ifndef VMS
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <pwd.h>
#endif

#ifdef RS6000
#include <sys/select.h>
#endif

#ifdef VMS
#include <ctype.h>
#include <descrip.h>
#include <file.h>
#include <lib$routines.h>
#include <lnmdef.h>
#include <starlet.h>
#include <stat.h>
#include <types.h>
#include <unixio.h>

#define $NEW_DESCRIPTOR(name) \
	struct dsc$descriptor_s name = { \
		0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 \
	}
#endif

#include "system.h"

/* Use builtin strdup when appropriate -- code duplicated in tcp.h. PGE */
#if defined(ultrix) || defined(VMS) || defined(NeXT)
extern char *strdup(char *str);
#endif

#ifndef VMS
extern int errno;
#endif


#ifndef DISABLE_TRACE
int nutTrace=0;
#endif


int sleep_interrupt=0;


int my_system(char *cmd, char *retBuf, int bufsize);
char *my_strerror(int errornum);
char **buildArgv(char *cmd, int *new_argc);
char *findProgram(char *pname,char *spath);
int my_sleep(int length, int interrupt);


/*#define STANDALONE /* */
#undef STANDALONE /* */


#ifdef STANDALONE

#ifndef VMS /* Completely different tests needed under VMS. PGE */
char *userPath={"/bin:/usr/bin:/sbin:/usr/sbin"};


int main(int argc, char **argv) {

int retValue;
char *cmd, *fnam, *lpr;
char buf[BUFSIZ];

	if (argc==3) {
		if (my_sleep(atoi(argv[1]),atoi(argv[2]))) {
			printf("Interrupted\n");
		}
		else {
			printf("Not Interrupted\n");
		}

		exit(0);
	}

	lpr = (char *)malloc (50);
	strcpy(lpr,"dsfs/usr/ucb/lpr");

	fnam = (char *)malloc (50);
	strcpy(fnam,"/sdlfjsdusr5/spowers/.cshrc");

	cmd = (char *)malloc ((strlen (lpr) + strlen (fnam) + 24));
	sprintf (cmd, "%s %s", lpr, fnam);

	printf("Executing '%s'.\n",cmd);

	if ((retValue=my_system(cmd,buf,BUFSIZ))!=SYS_SUCCESS) {
		printf("-----\nError Code [%d]\n-----\n",retValue);
	}

	if (buf && *buf) {
		printf("------\n%s-----\n",buf);
	}
	else {
		printf("------\nNo output.\n------\n");
	}
}

#else
/* Copied from mo-www.c PGE */
#if defined(ultrix) || defined(VMS) || defined(NeXT) || defined(M4310) || defined(vax)
char *strdup(char *str) {
  char *dup;

  if(!str)
    return NULL;

  dup = (char *)malloc(strlen (str) + 1);
  if(!dup)
    return NULL;

  dup = strcpy(dup, str);

  return dup;
}
#endif

int main(int argc, char **argv) {

   int    retValue;
   char  *home_dir;
   char   buffer [256];
   char **argv2;
   int    argc2;

   retValue = get_home (&home_dir);
   printf ("Home = \"%s\"\n  retValue = %d\n\n", home_dir, retValue);
   free (home_dir);

   if (file_exists ("sys$login:login.com"))
      printf ("The file sys$login:login.com exists\n");
   else
      printf ("The file sys$login:login.com does not exist\n");

   if (file_exists ("sys$login:ipserngipnfb.sadfghth"))
      printf ("The file sys$login:ipserngipnfb.sadfghth exists\n\n");
   else
      printf ("The file sys$login:ipserngipnfb.sadfghth does not exist\n\n");

   buffer [0] = '\0';
   retValue = my_system ("kdshlkw", buffer, 256);
   printf ("my_system kdshlkw\n");
   printf ("  gave \"%s\"\n  retValue = %d\n\n", buffer, retValue);

   buffer [0] = '\0';
   retValue = my_system ("dir *.*;0", buffer, 256);
   printf ("my_system dir *.*;0\n");
   printf ("  gave \"%s\"\n  retValue = %d\n\n", buffer, retValue);

   buffer [0] = '\0';
   retValue = my_copy ("argjhsrghjajh", "oijfdgath",
                       buffer, 256, TRUE);
   printf ("my_copy argjhsrghjajh to oijfdgath\n");
   printf ("  gave \"%s\"\n  retValue = %d\n\n", buffer, retValue);

   buffer [0] = '\0';
   retValue = my_copy ("sys$login:login.com", "sys$login:login.copy",
                       buffer, 256, TRUE);
   printf ("my_copy sys$login:login.com to sys$login:login.copy\n");
   printf ("  gave \"%s\"\n  retValue = %d\n\n", buffer, retValue);

   printf ("Starting 5 second sleep\n");
   retValue = my_sleep (5, TRUE);
   printf ("Ending 5 second sleep\n  retValue = %d\n\n", retValue);

   printf ("errno == 1 => \"%s\"\n\n", my_strerror (1));

   argv2 = buildArgv("abc def", &argc2);
   printf ("\"abc def\" => argc %d, argv [0] \"%s\", argv [1] \"%s\"\n\n",
           argc2, argv2 [0], argv2 [1]);
   free (*argv);
   free (argv);

   buffer [0] = '\0';
   retValue = my_move ("sys$login:login.copy", "sys$login:login.move",
                       buffer, 256, TRUE);
   printf ("my_move sys$login:login.copy to sys$login:login.move\n");
   printf ("  gave %s\n  retValue = %d\n\n", buffer, retValue);

   retValue = my_system ("diff sys$login:login.com sys$login:login.move", buffer, 256);
   retValue = my_system ("delete/log/noconf sys$login:login.move;*", buffer, 256);

   buffer [0] = '\0';
   retValue = my_move ("sys$manager:sylogin.com", "sys$login:sylogin.move",
                       buffer, 256, TRUE);
   printf ("my_move sys$manager:sylogin.com to sys$login:sylogin.move\n");
   printf ("  gave %s\n  retValue = %d\n\n", buffer, retValue);
   retValue = my_system ("diff sys$manager:sylogin.com sys$login:sylogin.move", buffer, 256);
   retValue = my_system ("dir/full sys$login:sylogin.move", buffer, 256);
   retValue = my_system ("delete/log/noconf sys$login:sylogin.move;*", buffer, 256);

}
#endif /*!VMS*/

#else


extern char *userPath;


#endif


/*
 * Written by: Scott Powers and Brad Viviano
 *
 * Takes a string command, executes the command, returns the output of
 *   the command (if any) in retBuf (passed in and pre-allocated).
 *
 * Returns one of the following:
 *   SYS_SUCCESS - The command executed without incident. Note, this does not
 *     necessarily mean the command was successful...e.g. some systems have
 *     a shell script for "lpr". In this case, the shell that runs "lpr" will
 *     execute fine, but the command "lp" which "lpr" calls may still fail.
 *   SYS_NO_COMMAND - There  was no command provided.
 *   SYS_FORK_FAIL - The fork failed.
 *   SYS_PROGRAM_FAILED - The exec could not start the program.
 *   SYS_NO_RETBUF - There was no retBuf allocated.
 *   SYS_FCNTL_FAILED - The set of NON_BLOCK on the parent end of the pipe
 *     failed.
 */
int my_system(char *cmd, char *retBuf, int bufsize) {

#ifndef VMS  /* PGE */

char **sys_argv=NULL;
int sys_argc;
pid_t pid;
int status,statusloc;
int fds[2];
char buf[BUFSIZ];
char *path=NULL;

	if (!retBuf) {
		return(SYS_NO_RETBUF);
	}

	*retBuf='\0';

	if (!cmd || !*cmd) {
		return(SYS_NO_COMMAND);
	}

	pipe(fds);
	if (fcntl(fds[0],F_SETFL,O_NONBLOCK)==(-1)) {
#ifndef DISABLE_TRACE
		if (nutTrace) {
			perror("fcntl-nonblock");
		}
#endif

		return(SYS_FCNTL_FAILED);
	}

	if ((pid=fork())==(-1)) {
		return(SYS_FORK_FAIL);
	}
	else if (pid==0) {
		/*in child -- so don't worry about frees*/
		sys_argv=buildArgv(cmd, &sys_argc);
		dup2(fds[1],1);
		dup2(fds[1],2);
		if (sys_argv!=NULL) {
			if (sys_argv[0] && sys_argv[0][0] && sys_argv[0][0]=='/') {
				path=strdup(sys_argv[0]);
			}
			else {
				path=findProgram(sys_argv[0],userPath);
			}
			execv(path,sys_argv);
#ifndef DISABLE_TRACE
			if (nutTrace) {
				fprintf(stderr,"Exec of %s failed!\n",cmd);
				perror("exec");
			}
#endif
		}
		else {
#ifndef DISABLE_TRACE
			if (nutTrace) {
				fprintf(stderr,"Could not build argv for [%s].\n",cmd);
			}
#endif
		}
		exit(1); /*child*/
	}
	else {
		int n;

		/*in parent*/
		status=wait(&statusloc);
		n=read(fds[0],retBuf,bufsize-1);
		if (n>0) {
			retBuf[n]='\0';
		}
		else {
			*retBuf='\0';
		}
		close(fds[0]);
		close(fds[1]);

		if (*retBuf) {
			return(SYS_PROGRAM_FAILED);
		}

		return(SYS_SUCCESS);
	}

#else /* VMS -- PGE */
   unsigned int status;
   unsigned int completion_status;

   if (!retBuf) {
      return(SYS_NO_RETBUF);
   }

   *retBuf='\0';

   if (!cmd || !*cmd) {
      return(SYS_NO_COMMAND);
   }

   {
      $NEW_DESCRIPTOR (cmd_desc);
      $NEW_DESCRIPTOR (retBuf_desc);
      unsigned short int string_end;

      cmd_desc.dsc$w_length = strlen (cmd);
      cmd_desc.dsc$a_pointer = cmd;
      retBuf_desc.dsc$w_length = bufsize - 1;
      retBuf_desc.dsc$a_pointer = retBuf;

      status = lib$spawn (&cmd_desc, 0, 0, 0, 0, 0, &completion_status);
      if (status != 1)
         return(SYS_FORK_FAIL);

      if ((completion_status & 1) != 1)
      {
         status = sys$getmsg (completion_status, &string_end, &retBuf_desc, 15, 0);
         retBuf [string_end] = '\0';
      }
   }

   return(SYS_SUCCESS);
#endif
}


/*
 * Written by: Scott Powers
 *
 * findProgram takes a program name and a path and searches it until:
 *   a) The program name is found, at which time the full path is returned.
 *   b) The end of the search path comes about, at which time NULL is returned.
 *
 */
char *findProgram(char *pname,char *spath) {

#ifndef VMS  /* Always return NULL on VMS. PGE */

char *start=NULL,*ptr=NULL,*endptr=NULL;
char tryit[BUFSIZ];
struct stat buf;

	if (!spath || !*spath || !pname || !*pname) {
		return(NULL);
	}

	start=spath;
	while (start && *start) {
		ptr=start;
		endptr=strchr(start,':');
		if (endptr) {
			start=endptr+1;
			*endptr='\0';
		}
		else {
			start=NULL;
		}
		sprintf(tryit,"%s/%s",ptr,pname);
		if (!stat(tryit,&buf)) {
			return(strdup(tryit));
		}
	}

#endif /* VMS, PGE */

	return(NULL);
}


/*
 * Written by: Scott Powers
 *
 * This will effectively get rid of the problem with using "/bin/mv" as it
 * first tries to use "rename". If that fails, it stores the error report
 * and then tries to actually copy the file. If that fails, it stores the
 * error report. If the copy fails then we return an error as well as copying
 * both error reports into "retBuf" so they can be displayed to the user.
 *
 * If "overwrite" is true, the destination file will automatically be
 * overwritten. If it is false and the file exists, my_move will return
 * SYS_FILE_EXISTS. It is up to the programmer to tell the user this.
 *
 * Return Values:
 *   SYS_NO_SRC_FILE -- There was no source filename specified.
 *   SYS_NO_DEST_FILE -- There was no destination filename specified.
 *   SYS_NO_RETBUF -- There was no retBuf specified (not allocated).
 *   SYS_DEST_EXISTS -- Overwrite was off and the destination exists.
 *   SYS_NO_MEMORY -- No memory to allocate with.
 *   SYS_SRC_OPEN_FAIL -- Open failed on the source file.
 *   SYS_DEST_OPEN_FAIL -- Open failed on the destination file.
 *   SYS_READ_FAIL -- The read call failed.
 *   SYS_WRITE_FAIL -- The write call failed.
 *   SYS_SUCCESS -- Success.
 */
int my_move(char *src, char *dest, char *retBuf, int bufsize, int overwrite) {

int status, n_src=1, n_dest=1, fd_src, fd_dest, ret;
char *rename_error=NULL, *copy_error=NULL;
struct stat dest_stat;

        if (!retBuf) {
		return(SYS_NO_RETBUF);
	}
	if (!src || !*src) {
		strcpy(retBuf,"There was no source file specified.\n");
		return(SYS_NO_SRC_FILE);
	}
	if (!dest || !*dest) {
		strcpy(retBuf,"There was no destination file specified.\n");
		return(SYS_NO_DEST_FILE);
	}

	*retBuf='\0';

	if (!overwrite) {
#if defined(MULTINET) && defined(__alpha)
                if (decc$stat(dest,&dest_stat)) {
#else
		if (stat(dest,&dest_stat)) {
#endif /* Alpha DEC C couldn't find it otherwise ?????, GEC */
			sprintf(retBuf,"Stat [%s] error:\n     File already exists.\n",dest);
			return(SYS_DEST_EXISTS);
		}
	}

	if ((status=rename(src,dest))==(-1)) {
		/*manual copy -- prolly accross partitions*/
		rename_error=strdup(my_strerror(errno));
		if (!rename_error) {
			strcpy(retBuf,"There was no enough memory allocate.\n");
			return(SYS_NO_MEMORY);
		}

#if 0
		if ((fd_src=open(src,O_RDONLY))==(-1)) {
			copy_error=strdup(my_strerror(errno));
			if (!copy_error) {
				free(rename_error);
				strcpy(retBuf,"There was no enough memory allocate.\n");
				return(SYS_NO_MEMORY);
			}

			if (strlen(rename_error)>bufsize) {
				fprintf(stderr,"%s\n",rename_error);
			}
			else {
				sprintf(retBuf,"Rename([%s] to [%s]) error:\n     %s\n\n",src,dest,rename_error);
			}
			free(rename_error);

			if (strlen(copy_error)>(bufsize-strlen(retBuf))) {
				fprintf(stderr,"%s\n",copy_error);
			}
			else {
				sprintf(retBuf,"%sCopy([%s] to [%s]) error:\n     %s\n\n",retBuf,src,dest,copy_error);
			}
			free(copy_error);

			return(SYS_SRC_OPEN_FAIL);
		}

		if ((fd_dest=open(dest,O_WRONLY|O_CREAT,0644))==(-1)) {
			copy_error=strdup(my_strerror(errno));
			if (!copy_error) {
				free(rename_error);
				strcpy(retBuf,"There was no enough memory allocate.\n");
				return(SYS_NO_MEMORY);
			}

			if (strlen(rename_error)>bufsize) {
				fprintf(stderr,"%s\n",rename_error);
			}
			else {
				sprintf(retBuf,"Rename([%s] to [%s]) error:\n     %s\n\n",src,dest,rename_error);
			}
			free(rename_error);

			if (strlen(copy_error)>(bufsize-strlen(retBuf))) {
				fprintf(stderr,"%s\n",copy_error);
			}
			else {
				sprintf(retBuf,"%sCopy([%s] to [%s]) error:\n     %s\n\n",retBuf,src,dest,copy_error);
			}
			free(copy_error);

			close(fd_src);

			return(SYS_DEST_OPEN_FAIL);
		}

		/*both file open and ready*/
		while (n_src>0) {
			n_src=read(fd_src,buf,BUFSIZ-1);
			if (n_src>0) {
				n_dest=write(fd_dest,buf,n_src);
				if (n_dest>0) {
					continue;
				}
				close(fd_src);
				close(fd_dest);
				sprintf(retBuf,"Write([%s]) error:\n     %s\n\n",dest,my_strerror(errno));
				return(SYS_WRITE_FAIL);
			}
			if (!n_src) {
				continue;
			}
			close(fd_src);
			close(fd_dest);
			sprintf(retBuf,"Read([%s]) error:\n     %s\n\n",src,my_strerror(errno));
			return(SYS_READ_FAIL);
		}

		close(fd_src);
		close(fd_dest);
#endif

		if ((ret=my_copy(src, dest, retBuf, bufsize, overwrite))==SYS_SUCCESS) {
			/* Now get rid of previous file */
			unlink(src);
		}

		return(ret);
	}

	/* Now get rid of previous file */
	unlink(src);

	return(SYS_SUCCESS);
}


/*
 * Written by: Scott Powers
 *
 * Some systems do not have a "strerror" function. This covers all the bases.
 */
char *my_strerror(int errornum)
{
	return(strerror(errornum));
}


/*
 * Written by: Brad Viviano and Scott Powers
 *
 * Takes a 1d string and turns it into a 2d array of strings.
 *
 * Watch out for the frees! You must free(*argv) and then free(argv)! NOTHING
 *   ELSE!! Do _NOT_ free the individual args of argv.
 */
char **buildArgv(char *cmd, int *new_argc) {

char **new_argv=NULL;
char *buf=NULL,*tmp=NULL;
int i=0;

	if (!cmd && !*cmd) {
		*new_argc=0;
		return(NULL);
	}

	for(tmp=cmd; isspace(*tmp); tmp++);
	buf=strdup(tmp);
	if (!buf) {
		*new_argc=0;
		return(NULL);
	}

	tmp=buf;

	new_argv=(char **)calloc(1,sizeof(char *));
	if (!new_argv) {
		free(buf);
		*new_argc=0;
		return(NULL);
	}

	new_argv[0]=NULL;

	while (*tmp) {
		if (!isspace(*tmp)) { /*found the begining of a word*/
			new_argv[i]=tmp;
			for (; *tmp && !isspace(*tmp); tmp++);
			if (*tmp) {
				*tmp='\0';
				tmp++;
			}
			i++;
			new_argv=(char **)realloc(new_argv,((i+1)*sizeof(char *)));
			new_argv[i]=NULL;
		}
		else {
			tmp++;
		}
	}

	*new_argc=i;

	return(new_argv);
}


/*
 * Written by: Scott Powers
 *
 * Takes an integer which is the number of seconds to sleep and an integer
 *   which is a boolean for whether to interrupt the sleep or not.
 *
 * This function sleeps for X seconds. It is interruptable.
 *
 * Returns a 1 of interrupted (and allowed to interrupt) or 0 when done
 *   sleeping.
 *
 * Note that this is not _really_ _truly_ _exact_ as it is does perform some
 *   condition checking inbetween each 100 milliseconds. But...it's pretty
 *   darn close.
 */
int my_sleep(int length, int interrupt) {

int count=0;
int num;

#ifndef VMS  /* PGE */
struct timeval timeout;
#else
unsigned char interval [8];   /* 64 bit VMS time */
char string_interval_data [] = "0 00:00:00.10";   /* 100msec. */
$NEW_DESCRIPTOR (string_interval);

  string_interval.dsc$w_length = strlen (string_interval_data);
  string_interval.dsc$a_pointer = string_interval_data;
  /* Convert string time into binary format. */
  sys$bintim (&string_interval, &interval);
#endif

	sleep_interrupt=0;

	length*=1000000;

	while (count<length) {
#ifndef VMS  /* PGE */
		timeout.tv_sec=0;
		timeout.tv_usec=100000;
		select(0, NULL, NULL, NULL, &timeout);
#else
                /* Sleep for 100 msec. */
                sys$schdwk (0, 0, &interval, 0);
                sys$hiber ();
#endif
		count+=100000;
		if (interrupt && sleep_interrupt) {
			return(1);
		}
	}

	return(0);
}


/*
 * Written by: Tommy Reilly
 *
 * Simple function that stat's a file to see if it exists.
 *
 * Simple returns 1 or 0.
 */
int file_exists(char* name)
{
  struct stat buf;

  if(!name)
    return(0);

#if defined(MULTINET) && defined(__alpha)
  if(!decc$stat(name, &buf)) {
#else
  if(!stat(name, &buf)) {
#endif
    return(1);
  }

  return(0);
}

/*
 * Written by: Tommy Reilly (with major code snarfing from Scott Powers)
 *
 * This is essentially Scott's my_move function re-written without rename
 * and without erasing the source file.
 *
 * If "overwrite" is true, the destination file will automatically be
 * overwritten. If it is false and the file exists, my_move will return
 * SYS_FILE_EXISTS. It is up to the programmer to tell the user this.
 *
 * Return Values:
 *   SYS_NO_SRC_FILE -- There was no source filename specified.
 *   SYS_NO_DEST_FILE -- There was no destination filename specified.
 *   SYS_DEST_EXISTS -- Overwrite was off and the destination exists.
 *   SYS_NO_MEMORY -- No memory to allocate with.
 *   SYS_SRC_OPEN_FAIL -- Open failed on the source file.
 *   SYS_INTERNAL_FAIL -- Error occured that user doesn't want to know about.
 *   SYS_SUCCESS -- Success.
 */
int my_copy(char *src, char *dest, char *retBuf, int bufsize, int overwrite)
{
  int status, n_src=1, n_dest=1, fd_src, fd_dest;
  char *copy_error=NULL;
#ifndef VMS  /* No need to allocate large variables if not used. PGE */
  char buf[BUFSIZ];
#endif
  struct stat dest_stat;
#ifdef VMS  /* PGE, Added for new code below. */
  char *cmd;
  int ret;
#endif

  if (!retBuf)
    {
      return(SYS_NO_RETBUF);
    }

  if (!src || !*src)
    {
      strcpy(retBuf,"There was no source file specified.\n");
      return(SYS_NO_SRC_FILE);
    }

  if (!dest || !*dest)
    {
      strcpy(retBuf,"There was no destination file specified.\n");
      return(SYS_NO_DEST_FILE);
    }

  *retBuf='\0';

  if (!overwrite)
    {
#if defined(MULTINET) && defined(__alpha)
      if (decc$stat(dest,&dest_stat))
#else
      if (stat(dest,&dest_stat))
#endif
	{
	  sprintf(retBuf,"Stat [%s] error:\n     File already exists.\n",dest);
	  return(SYS_DEST_EXISTS);
	}
    }


#ifndef VMS  /* Must copy file header as well as contents. PGE */
  if ((fd_src=open(src,O_RDONLY))==(-1))
    {
      copy_error=strdup(my_strerror(errno));

      if (!copy_error)
	{
	  strcpy(retBuf,"There was not enough memory allocate.\n");
	  return(SYS_NO_MEMORY);
	}

      if (strlen(copy_error)>(bufsize-strlen(retBuf)))
	{
	  fprintf(stderr,"%s\n",copy_error);
	}
      else
	{
	  sprintf(retBuf,"%sCopy([%s] to [%s]) error:\n     %s\n\n",retBuf,src,dest,copy_error);
	}
      free(copy_error);

      return(SYS_SRC_OPEN_FAIL);
    }

  if ((fd_dest=open(dest,O_WRONLY|O_CREAT,0644))==(-1))
    {
      copy_error=strdup(my_strerror(errno));

      if (!copy_error)
	{
	  strcpy(retBuf,"There was not enough memory allocate.\n");
	  return(SYS_NO_MEMORY);
	}

      if (strlen(copy_error)>(bufsize-strlen(retBuf)))
	{
	  fprintf(stderr,"%s\n",copy_error);
	}
      else
	{
	  sprintf(retBuf,"%sCopy([%s] to [%s]) error:\n     %s\n\n",retBuf,src,dest,copy_error);
	}
      free(copy_error);

      close(fd_src);

      return(SYS_DEST_OPEN_FAIL);
    }

  /*both files open and ready*/
  while (n_src>0)
    {
      n_src=read(fd_src,buf,BUFSIZ-1);
      if (n_src>0)
	{
	  n_dest=write(fd_dest,buf,n_src);
	  if (n_dest>0)
	    {
	      continue;
	    }
	  close(fd_src);
	  close(fd_dest);
	  sprintf(retBuf,"Write([%s]) error:\n     %s\n\n",dest,my_strerror(errno));
	  return(SYS_WRITE_FAIL);
	}
      if (!n_src)
	{
	  continue;
	}
      close(fd_src);
      close(fd_dest);
      sprintf(retBuf,"Read([%s]) error:\n     %s\n\n",src,my_strerror(errno));
      return(SYS_READ_FAIL);
    }

  close(fd_src);
  close(fd_dest);

#else   /* VMS, PGE */
  cmd = malloc(sizeof(char) * (20 + strlen(src) + strlen(dest)));
  if(!cmd)
    return(SYS_NO_MEMORY);

  sprintf(cmd, "copy$$/noconf/nolog %s %s", src, dest);
  ret = my_system(cmd, retBuf, bufsize);
  free(cmd);

  return ret;
#endif  /* VMS, PGE */

  return(SYS_SUCCESS);
}


/*
 * Written by: Tommy Reilly (originally by Scott Powers I think)
 *
 * This is a function that finds the users home directory.
 *
 * Return Values:
 *   SYS_NO_MEMORY -- No memory to allocate with.
 *   SYS_INTERNAL_FAIL -- Not an error that user would want to know about.
 *   SYS_SUCCESS -- Success.
 */
int get_home(char **ret)
{
#ifndef VMS  /* PGE */
  char *home = NULL;
  struct passwd *pwdent;

  if(!(home = getenv("HOME")))
    {
      if (!(pwdent=getpwuid(getuid())))
	{
	  fprintf(stderr, "Could not find home dir.\n");
	  *ret=NULL;
	  return(SYS_INTERNAL_FAIL);
	}
      else
	{
/*
	  home = malloc(sizeof(char) * (strlen(pwdent->pw_dir) + 1));

	  if(!home) {
	    *ret=NULL;
	    return(SYS_NO_MEMORY);
	  }

	  strcpy(home, pwdent->pw_dir);
*/
	  home=strdup(pwdent->pw_dir);
	}
    }
  else {
	home=strdup(home);
  }

  if(home)
    *ret = home; /* he better free it */
  else
    {
      *ret = NULL;
      return(SYS_INTERNAL_FAIL);
    }

#else  /* VMS, PGE */
  typedef struct {
    unsigned short int  length;
    unsigned short int  code;
    char               *buffer;
    unsigned short int *return_length;
  } ITEMLIST;

  int                status;
  ITEMLIST           item_list [2];
  char               translated_home_data [256];
  unsigned short int translated_home_length;
  char               logical_table_data [] = "LNM$FILE_DEV";
  char               home_data [] = "SYS$LOGIN";
  $NEW_DESCRIPTOR (logical_table);
  $NEW_DESCRIPTOR (home_logical);

  /* Setup values to pass into sys$trnlnm */
  logical_table.dsc$w_length = strlen(logical_table_data);
  logical_table.dsc$a_pointer = logical_table_data;
  home_logical.dsc$w_length = strlen(home_data);
  home_logical.dsc$a_pointer = home_data;

  /* Setup values to return from sys$trnlnm */
  item_list[0].code = LNM$_STRING;
  item_list[0].length = 256;
  item_list[0].buffer = translated_home_data;
  item_list[0].return_length = &translated_home_length;
  item_list[1].code = 0;
  item_list[1].length = 0;

  /* translate SYS$LOGIN logical to DISK:[DIRECTORY] */
  status = sys$trnlnm (0, &logical_table, &home_logical, 0, item_list);
  if (status != 1)
  {
    printf ("Status = %d", status);
    return(SYS_INTERNAL_FAIL);
  }

  /* Convert to null terminated C style string. */
  translated_home_data [translated_home_length] = '\0';

  *ret = strdup(translated_home_data);   /* he better free it */
  if(!(*ret))
    return(SYS_NO_MEMORY);
#endif /* VMS, PGE */

return(SYS_SUCCESS);
}
