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

/* this module intended to handle child process clean up through callbacks*/
/* written for version 2.5 */
#include "../config.h"
#include "child.h"
#include "list.h"
#include <stdio.h>


#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

List childProcessList;

typedef struct {
	pid_t pid;
	void (*callback)();
	void *callBackData;
	} ProcessHandle;


static ProcessHandle *SearchForChildRecordByPID(pid_t pid);



void InitChildProcessor(void)
{
	childProcessList = ListCreate();
}


/* Add a child process handler.  Callback is made when child dies */
/* callback is of the form callback(callBackData,pid); */
void AddChildProcessHandler(pid_t pid,void (*callback)(), void *callBackData)
{
ProcessHandle *p;

	if (!(p = (ProcessHandle *) malloc(sizeof(ProcessHandle)))) {
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr,"Out of Memory\n");
		}
#endif

		return;
		}
	p->pid = pid;
	p->callback = callback;
	p->callBackData = callBackData;

	ListAddEntry(childProcessList,p);
}



static ProcessHandle *SearchForChildRecordByPID(pid_t pid)
{
ProcessHandle *p;

	p = (ProcessHandle *) ListHead(childProcessList);
	while(p) {
		if (p->pid == pid) {
			return(p);
			}
		p = (ProcessHandle *) ListNext(childProcessList);
		}

	return(NULL);

}

/* terminate the children... 
   you may want to remove SIGCHLD signal handler before calling this routine
*/
void KillAllChildren(void)
{
ProcessHandle *p;

	/* first, be nice and send SIGHUP */
	p = (ProcessHandle *) ListHead(childProcessList);
	while(p) {
		kill(p->pid,SIGHUP);
		p = (ProcessHandle *) ListNext(childProcessList);
		}

	/* hack and slash */
	p = (ProcessHandle *) ListHead(childProcessList);
	while(p) {
		kill(p->pid,SIGKILL);
		p = (ProcessHandle *) ListNext(childProcessList);
		}
}


/* callback routine for SIGCHLD signal handler */
void ChildTerminated(void)
{
pid_t pid;
ProcessHandle *p;
#ifdef __sgi
union wait stat_loc;
#else
int stat_loc;
#endif

#ifdef SVR4
	pid = waitpid((pid_t)(-1),NULL,WNOHANG);
	signal(SIGCHLD, (void (*)())ChildTerminated); /*Solaris resets the signal on a catch*/
#else
	pid = wait3(&stat_loc,WNOHANG,NULL);
#endif

	p = SearchForChildRecordByPID(pid);
	if (!p) {
		/* un registered child process */
		return;
		}

	(p->callback)(p->callBackData,p->pid);

	ListDeleteEntry(childProcessList,p);
	free(p);

	return;
}

