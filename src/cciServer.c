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
#include "mosaic.h"
#include <sys/types.h>
#include <sys/stat.h>

#include "memStuffForPipSqueeks.h"
#include "cci.h"
#include "accept.h"

/* #define DEBUG */

static ListenAddress listenPort;

int MCCIanchorcached = 0;       /* another ugly ADC hack ZZZZ */

#ifndef DISABLE_TRACE
extern int cciTrace;
#endif

extern char *GetLine();

int MCCIReturnListenPortSocketDescriptor()
{
	return(listenPort);
}

MCCICloseConnection(clientPort)
MCCIPort clientPort;
{

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"CloseConnection(): I've been called\n");
	}
#endif

	MoCCITerminateAConnection(clientPort);
	NetCloseConnection(clientPort);

}

MCCICloseAcceptPort()
{
	NetCloseAcceptPort(listenPort);
}

int MCCIServerInitialize(portNumber)
/* return 0 on failure */
/* return listenPort on success */
int portNumber;
{

	listenPort = NetServerInitSocket(portNumber);

	if (listenPort == -1)
		return(0);
	else
		return(1);

}

int MCCIGetSocketDescriptor(clientPort)
/* this routine is platform dependent and is not assumed to be supported
 * on all platforms.  It is only here for those routines that wish to have
 * a select() external to the MCCI library.
 * this routine extracts the socket descriptor from the MCCIPort and
 * returns it */
MCCIPort clientPort;
{
        return(NetGetSocketDescriptor(clientPort));
}



int MCCISendResponseLine(client,code,text)
MCCIPort client;
int code; 	/* response code */
char *text; 	/* text response (no newline)*/
{
int length/*,lengthSent*/;
char *buff;

	if (!(buff = (char *) MALLOC(strlen(text) + 7))) {
		/* out of memory */
		return(MCCI_OUTOFMEMORY);
		}

	sprintf(buff,"%d %s\r\n",code,text);
	length = strlen(buff);
	if (length != NetServerWrite(client,buff,length)) {
		return(MCCI_FAIL);
		}

	return(MCCI_OK);
}

MCCIPort MCCICheckAndAcceptConnection()
/* return NULL if no connection */
/* return a MCCIPort if connected */
{
MCCIPort client;

	if (NetIsThereAConnection(listenPort)){
		client = NetServerAccept(listenPort);
		}
	else {
		return(NULL);
		}

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"Current cci connections: max number=%d, currentNumber=%d\n",
				MoCCIMaxNumberOfConnectionsAllowed(),
				MoCCICurrentNumberOfConnections());
	}
#endif

	if (client && MoCCIMaxNumberOfConnectionsAllowed()) {
		/* if maxNumConnections == 0, then no limit */
		if ((MoCCICurrentNumberOfConnections() + 1) >
		    MoCCIMaxNumberOfConnectionsAllowed()) {
			MCCISendResponseLine(client,MCCIR_MAX_CONNECTIONS,
			"Maximum number of allowed CCI connections exceeded");
			MCCICloseConnection(client);
			return(NULL);

			}
		}
	return(client);
}



int MCCIIsThereInput(client)
/* return 1 on true, 0 on false */
MCCIPort client;
{
        if (!client)
                return(0);
        return(NetIsThereInput(client));
}

int MCCIReadContent(client,content)
/* read from client. Next line should contain Content-Length: value 
   and then the content body. Returns the number of chars read. 
   0 on error. space is allocated and placed into 'content'*/
MCCIPort client;
char **content;
{
char *s;
int length;
char garbage;
char *line;
int x;

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIReadContent(): Just entered...about to GetLine()\n");
	}
#endif
	*content = (char *) 0;
	line = GetLine(client);
#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIReadContent(): read line \"%s\"\n",line);
	}
#endif

	/* read content length */
	s = strchr(line,':'); /* skip to length */
	if ((!s) || (!strlen(s))) {
		/* bad value */
		return(0);
		}
	s++;
	length = atoi(s);
	if ((length > 10000000) || (length < 0)) {
		/* bad value */
		return(0);
		}
	if (!((*content) = (char*) MALLOC(length+1))) {
		/* to recover protocol, this needs to be read in
			any way before returning, but if we're out of memory,
			it's likely hopeless anyway */
		for (x = 0; x < length; x++) {
			if (!NetRead(client,&garbage,1)) {
				break;
				}
			}
		return(0);
		}
#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"ReadContent(): about to read %d bytes\n",length);
	}
#endif

	length = ReadBuffer(client,*content,length);
	(*content)[length]='\0';
	return(length);
}

int MCCIHandleSend(client,line,retText)
     /* take care of the SEND request parsing */
     /* return value to send back to cci client */
     MCCIPort client;
     char *line; 	/* GET request line */
     char *retText;	/* text to be returned to cci client */
{
  int retCode;
  char *s,*end,*start;
  
  if (!(s = strchr(line,' ')))
    { /* skip over SEND */
      strcpy(retText,"Error in protocol");
      return(MCCIR_ERROR);
    }
  
  
  GetWordFromString(s,&start,&end);
  if (!my_strncasecmp(start,MCCI_S_ANCHOR,strlen(MCCI_S_ANCHOR)))
    {
/* SEND ANCHOR */
      s = end;
      GetWordFromString(s,&start,&end);
      /* ejb 9 March 1995 added BEFORE and AFTER cases */
      if (start && (start != end))
/* SEND ANCHOR STOP => turn off SEND ANCHOR */
	if (!my_strncasecmp(start,MCCI_S_STOP,strlen(MCCI_S_STOP))) 
	  MCCIRequestSendAnchor(&retCode,retText,client,0);
	else 
/* SEND ANCHOR BEFORE => Mosaic sends anchor, BEFORE done getting */
	  if (!my_strncasecmp(start,MCCI_S_BEFORE,strlen(MCCI_S_BEFORE)))
	    MCCIRequestSendAnchor(&retCode,retText,client,MCCI_SEND_BEFORE);
	  else
/* SEND ANCHOR AFTER => Mosaic sends anchor, AFTER done getting */
	    if (!my_strncasecmp(start,MCCI_S_AFTER,strlen(MCCI_S_AFTER)) ||
		(!(start)))
	      MCCIRequestSendAnchor(&retCode,retText,client,MCCI_SEND_AFTER);
	    else
/* SEND ANCHOR HANDLER => Mosaic sends anchor first then lets cci handle it  ADC ZZZ */
               if (!my_strncasecmp(start,MCCI_S_HANDLER,strlen(MCCI_S_HANDLER)) ||
                   (!(start)))
                 MCCIRequestSendAnchor(&retCode,retText,client,MCCI_SEND_HANDLER);
	       else
/* SEND ANCHOR XXXXX => Mosaic doesn't know what to do with it */
	          {
		    /* we don't know what to do with it. */
		    strcpy(retText,"what\'s this stuff after ANCHOR?");
		    return(MCCIR_ERROR);
	          }
/* SEND ANCHOR => Mosaic sends anchor, AFTER done getting*/
      else
	MCCIRequestSendAnchor(&retCode,retText,client,MCCI_SEND_AFTER);
    }
  else if (!my_strncasecmp(start,MCCI_S_OUTPUT,strlen(MCCI_S_OUTPUT))) {
/* SEND OUTPUT */
		s = end;
		GetWordFromString(s,&start,&end);
		if (start && (start != end)) {
			if (!my_strncasecmp(start, MCCI_S_STOP,
						strlen(MCCI_S_STOP))){
				/* SEND OUTPUT STOP*/
				s = end;
				/* check for mime type */
				GetWordFromString(s,&start,&end);
				if (start && (start != end)) {
					*end = '\0';
					MCCIRequestSendOutput(&retCode,retText,
							client,0,start);
					}
				else {
					/* no output type... so all types */
					MCCIRequestSendOutput(&retCode,retText,
							client,0,(char *)0);
					}
				}
			else {
				/* SEND OUTPUT type */
				*end = '\0';
				MCCIRequestSendOutput(&retCode,retText,
							client,1,start);
				}
			
			}
		else {
			/* "SEND OUTPUT" so send it all */
			MCCIRequestSendOutput(&retCode,retText,
							client,1,(char *)0);
			}
		}
	
  else if (!my_strncasecmp(start,MCCI_S_BROWSERVIEW,strlen(MCCI_S_BROWSERVIEW))){
/* SEND BROWSERVIEW */
		s = end;
		GetWordFromString(s,&start,&end);
		if (start && (start != end)) {
			if (!my_strncasecmp(start, MCCI_S_STOP,
						strlen(MCCI_S_STOP))){
				/* SEND BROWSERVIEW STOP*/
				MCCIRequestSendBrowserView(&retCode,retText,
							client,0);
				}
			else {
				/* SEND BROWSERVIEW garbageHere */
				MCCIRequestSendBrowserView(&retCode,retText,
							client,1);
				}
			}
		else {
			/* SEND BROWSERVIEW*/
			MCCIRequestSendBrowserView(&retCode,retText,
							client,1);
			}
		}
   
  else if (!my_strncasecmp(start,MCCI_S_EVENT,strlen(MCCI_S_EVENT))){
/* SEND EVENT */
		s = end;
		GetWordFromString(s,&start,&end);
		if (start && (start != end)) {
			if (!my_strncasecmp(start, MCCI_S_STOP,
						strlen(MCCI_S_STOP))){
				/* SEND EVENT STOP*/
				MCCIRequestSendEvent(&retCode,retText,
							client,0);
				}
			else {
				/* SEND EVENT garbageHere */
				MCCIRequestSendEvent(&retCode,retText,
							client,1);
				}
			}
		else {
			/* SEND EVENT*/
			MCCIRequestSendEvent(&retCode,retText,
							client,1);
			}
		}

	else {
/* SEND ??? */
		strcpy(retText,"SEND what???");
		return(MCCIR_ERROR);
		}
	return(retCode);
}


int MCCIHandlePost(client,line,retText)
/* take care of the Post request parsing */
/* return value to send back to cci client */
MCCIPort client;
char *line; 	/* GET request line */
char *retText;	/* text to be returned to cci client */
{
char *s,*end;
char *url;
char *postData;
int postDataLength;
char *mimeType;
int retCode;
int output;
char *tmpend;
char *next;

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIHandlePost(): parsing line: \"%s\"\n",line);
	}
#endif

	if (!(s = strchr(line,' '))){ /* skip over POST */
		strcpy(retText,"Error in protocol");
		return(MCCIR_ERROR);
		}

	GetWordFromString(s,&url,&end); /* Get <url> */
	if ((!url) || (url == end)) {
		strcpy(retText,"Hey bud, where's the URL for POST?");
		return(MCCIR_ERROR);
		}
	s = end;
	url++; /* skip over '<' */
	end--; /* backup over '>' */
	*end = '\0'; /* terminate url */

	url = strdup(url);

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIHandlePost(): extracted url: \"%s\"\n",url);
	}
#endif

	GetWordFromString(s,&mimeType,&end); /* Get Content Type*/
	if ((!mimeType) || (mimeType == end)) {
		strcpy(retText,"No Content-Type?");
		return(MCCIR_ERROR);
		}
	tmpend = end;

	s = end;
	GetWordFromString(s,&next,&end); /* move pointer to OUTPUT */

	*tmpend = '\0'; /* terminate the content-type */
	mimeType = strdup(mimeType);

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIHandlePost(): mimeType: \"%s\"\n",mimeType);
	}
#endif

	output = MCCI_DEFAULT;
	if (next && (next != end)) {
	    if (!my_strncasecmp(next,MCCI_S_OUTPUT,
				strlen(MCCI_S_OUTPUT))) {
		/* output tag */
		s = end;
		GetWordFromString(s,&next,&end);
		if (next && (next != end)) {
			if (!my_strncasecmp(next,MCCI_S_CURRENT,strlen(MCCI_S_CURRENT))) {
				output = MCCI_OUTPUT_CURRENT;
				}
			else if (!my_strncasecmp(next,MCCI_S_NEW,strlen(MCCI_S_NEW))) {
				output = MCCI_OUTPUT_NEW;
				}
			else if (!my_strncasecmp(next,MCCI_S_NONE,strlen(MCCI_S_NONE))) {
				output = MCCI_OUTPUT_NONE;
				}
			else {
				output = MCCI_DEFAULT;
				}
			s = end;
			GetWordFromString(s,&next,&end);
			}
		    }
	    }
	else {
		output = MCCI_DEFAULT; 
		}

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"POST url = \"%s\",mimeType=\"%s\",output=%d\n",
					url,mimeType,output);
	}
#endif

	postDataLength = MCCIReadContent(client,&postData);
	if (postDataLength < 1) {
		strcpy(retText,"No data for POST");
		return(MCCIR_ERROR);
		}

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"Got the data, datalength = %d\n",postDataLength);
	}
#endif

	MCCIRequestPost(client,&retCode, retText, url, mimeType, 
				postData, postDataLength, output);

	free(url);
	free(mimeType);
	
	return(retCode);
}


int MCCIHandleDisplay(client,line,retText)
/* take care of the Display request parsing */
/* return value to send back to cci client */
MCCIPort client;
char *line; 	/* GET request line */
char *retText;	/* text to be returned to cci client */
{
char *s,*end;
char *url;
char *displayData;
int displayDataLength;
char *mimeType;
int retCode;
int output;
char *tmpend;
char *next;


#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIHandleDisplay(): parsing line: \"%s\"\n",line);
	}
#endif

	if (!(s = strchr(line,' '))){ /* skip over DISPLAY */
		strcpy(retText,"Error in protocol");
		return(MCCIR_ERROR);
		}

	GetWordFromString(s,&url,&end); /* Get <url> */
	if ((!url) || (url == end)) {
		strcpy(retText,"Hey bud, where's the URL for Display?");
		return(MCCIR_ERROR);
		}
	s = end;
	url++; /* skip over '<' */
	end--; /* backup over '>' */
	*end = '\0'; /* terminate url */

	url = strdup(url);

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIHandleDisplay(): extracted url: \"%s\"\n",url);
	}
#endif

	GetWordFromString(s,&mimeType,&end); /* Get Content Type*/
	if ((!mimeType) || (mimeType == end)) {
		strcpy(retText,"No Content-Type?");
		return(MCCIR_ERROR);
		}
	tmpend = end;

	s = end;
	GetWordFromString(s,&next,&end); /* move pointer to OUTPUT */

	*tmpend = '\0'; /* terminate the content-type */
	mimeType = strdup(mimeType);

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIHandleDisplay(): mimeType: \"%s\"\n",mimeType);
	}
#endif

	output = MCCI_DEFAULT;
	if (next && (next != end)) {
	    if (!my_strncasecmp(next,MCCI_S_OUTPUT,
				strlen(MCCI_S_OUTPUT))) {
		/* output tag */
		s = end;
		GetWordFromString(s,&next,&end);
		if (next && (next != end)) {
			if (!my_strncasecmp(next,MCCI_S_CURRENT,strlen(MCCI_S_CURRENT))) {
				output = MCCI_OUTPUT_CURRENT;
				}
			else if (!my_strncasecmp(next,MCCI_S_NEW,strlen(MCCI_S_NEW))) {
				output = MCCI_OUTPUT_NEW;
				}
			else if (!my_strncasecmp(next,MCCI_S_NONE,strlen(MCCI_S_NONE))) {
				output = MCCI_OUTPUT_NONE;
				}
			else {
				output = MCCI_DEFAULT;
				}
			s = end;
			GetWordFromString(s,&next,&end);
			}
		    }
	    }
	else {
		output = MCCI_DEFAULT; 
		}

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"Display url = \"%s\",mimeType=\"%s\",output=%d\n",
					url,mimeType,output);
	}
#endif

	/* MCCIReadContent will malloc space for displayData */
	displayDataLength = MCCIReadContent(client,&displayData);
	if (displayDataLength < 1) {
		strcpy(retText,"No data for DISPLAY");
		return(MCCIR_ERROR);
		}

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"Got the data, datalength = %d\n",displayDataLength);
	}
#endif

	MCCIRequestDisplay(client, &retCode, retText, url, mimeType, 
				displayData, displayDataLength, output);

	free(url);
	free(mimeType);
	
	return(retCode);
}


int MCCIHandleGet(client,line,retText)
/* take care of the GET request parsing */
/* return value to send back to cci client */
MCCIPort client;
char *line; 	/* GET request line */
char *retText;	/* text to be returned to cci client */
{
char *s;
char *url/*,*start*/,*end,*next;
int output/*,absRel*/;
char *headerExt;
int headerExtLength;
int retCode;

	output = MCCI_DEFAULT;
/*	absRel = MCCI_DEFAULT;*/
	headerExt = (char *) 0;
 	headerExtLength=0;
	
	if (!(s = strchr(line,' '))){ /* skip over GET */
		strcpy(retText,"Error in protocol");
		return(MCCIR_ERROR);
		}
	GetWordFromString(s,&url,&end); /* URL */
	if (my_strncasecmp(url,"URL",3)) {
		strcpy(retText,"No URL?");
		return(MCCIR_ERROR);
		}
	s = end;
	GetWordFromString(s,&url,&end); /* actual <url> */
	if ((!url) || (url == end)) {
		strcpy(retText,"Hey bud, where's the URL?");
		return(MCCIR_ERROR);
		}
	s = end;
	url++; /* skip over '<' */
	end--; /* backup over '>' */
	*end = '\0'; /* terminate url */

	url = strdup(url);
#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"GetURL: URL=\"%s\"\n",url);
	}
#endif

	GetWordFromString(s,&next,&end);
	if (next && (next != end)) {
	    if (!my_strncasecmp(next,MCCI_S_OUTPUT,
				strlen(MCCI_S_OUTPUT))) {
		/* output tag */
		s = end;
		GetWordFromString(s,&next,&end);
		if (next && (next != end)) {
			if (!my_strncasecmp(next,MCCI_S_CURRENT,strlen(MCCI_S_CURRENT))) {
				output = MCCI_OUTPUT_CURRENT;
				}
			else if (!my_strncasecmp(next,MCCI_S_NEW,strlen(MCCI_S_NEW))) {
				output = MCCI_OUTPUT_NEW;
				}
			else if (!my_strncasecmp(next,MCCI_S_NONE,strlen(MCCI_S_NONE))) {
				output = MCCI_OUTPUT_NONE;
				}
			else {
				output = MCCI_OUTPUT_CURRENT;
				}
			s = end;
			GetWordFromString(s,&next,&end);
			}
		    }
	    }
#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"pt #2 GetURL: URL=\"%s\"\n",url);
	}
#endif

	if (next && (next != end)) {
	    if (!my_strncasecmp(next,MCCI_S_HEADER,strlen(MCCI_S_HEADER))) {
		/* get header extention */
		    headerExtLength = MCCIReadContent(client,&headerExt);
		    }
		}
#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"pt #3 GetURL: URL=\"%s\"\n",url);
	}
#endif
        /* set flag to be caught in MoCCISendAnchorToCCI */
	cciStatPreventSendAnchor(client, url); 
	MCCIRequestGetURL(&retCode,retText,url,output,headerExt);
	if ((headerExtLength > 0) && (headerExt)) {
		FREE(headerExt);
		}
	free(url);
	return(retCode);
}


int MCCIHandleDoCommand(client,line,retText)
/* take care of the GET request parsing */
/* return value to send back to cci client */
MCCIPort client;
char *line; 	/* GET request line */
char *retText;	/* text to be returned to cci client */
{
char *s;
char *end, *tmpend ;
char *command;
char *parameter;
int retCode;

	/* expected line, DOCOMMAND command parameters... */
#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"line is %s\n", line);
	}
#endif

	if (!(s = strchr(line,' '))){ /* skip over DOCOMMAND */
		strcpy(retText,"Error in protocol");
		return(MCCIR_ERROR);
		}

	GetWordFromString(s,&command,&end); /* Get command */
	if ((!command) || (command == end)) {
		strcpy(retText,"You need a command");
		return(MCCIR_ERROR);
		}
	s = end;
	tmpend = end;

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIHandleDisplay(): extracted command: \"%s\"\n",command);
	}
#endif

	parameter = strdup(s);

	*tmpend = '\0';
	command = strdup(command);

	MCCIRequestDoCommand(&retCode,retText,command, parameter);

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"MCCIHandleDisplay(): retCode: %d -- retText: [%s]\n",retCode,retText);
	}
#endif

	return(retCode);
}


int MCCIHandleForm(client,line,retText)
/* take care of the Form request parsing */
/* return value to send back to cci client */
MCCIPort client;
char *line; 	/* GET request line */
char *retText;	/* text to be returned to cci client */
{
char *s;
char *actionID,*end,*next,*startstop, *tmp;
int output/*,absRel*/;
int retCode;
int status;

	if (!(s = strchr(line,' '))){ /* skip over FORM */
		strcpy(retText,"Error in protocol");
		return(MCCIR_ERROR);
		}

	GetWordFromString(s,&actionID,&end); /* actionID */
	if ((!actionID) || (actionID == end)) {
		strcpy(retText,"Hey bud, where's the actionID?");
		return(MCCIR_ERROR);
		}
	s = end;
	actionID = strdup(actionID);
	tmp = strchr(actionID, ' ');
	*tmp = '\0';


#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"GetURL: actionID=[%s]\n",actionID);
	}
#endif

	GetWordFromString(s,&next,&end);
	if (next && (next != end)) {
		if (!my_strncasecmp(next,
			MCCI_S_STOP, strlen(MCCI_S_STOP))) {
			status = 0;
			}
		else if(!my_strncasecmp(next,
			MCCI_S_START, strlen(MCCI_S_START))){
			status = 1;
			}
		else {
			return(MCCIR_ERROR);
			}
		}
	else
		return(MCCIR_ERROR);

#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"  actionID=\"%s\"\n",actionID);
	}
#endif

        /* set flag to be caught in MoCCISendAnchorToCCI */
/*
	cciStatPreventSendAnchor(client, url); 
*/
	MCCIRequestForm(client, &retCode,retText,actionID,status);
		
/*  	free(actionID);
*/
	return(retCode);
}

#ifdef NEW
#define NUM_ANNO_CODES 3
static int annoCodes[NUM_ANNO_CODES] = {MCCI_PUBLIC_ANNOTATION, MCCI_GROUP_ANNOTATION, MCCI_PRIVATE_ANNOTATION};
static char* annoStrings[NUM_ANNO_CODES] = {MCCI_S_PUBLIC_ANN, MCCI_S_GROUP_ANN, MCCI_S_PRIVATE_ANN};
#else /* NEW */

#endif /* NEW */
int MCCIHandleGetAnnotation(client,line,retText,retData,retDataLength)
MCCIPort client;
char *line; 	/* GET request line */
char *retText;	/* text to be returned to cci client */
char **retData;
int *retDataLength;
{
char *s;
char *end;
char *type;
char *url;
int annotationType;
int retCode;

	if (!(s = strchr(line,' '))){ /* skip over GET */
		strcpy(retText,"Error in protocol");
		return(MCCIR_ERROR);
		}

	annotationType = 0;
	GetWordFromString(s,&type,&end); /* Get type (pub,priv,group)*/
	if (!my_strncasecmp(type,MCCI_S_PUBLIC_ANN,strlen(MCCI_S_PUBLIC_ANN))) {
		annotationType = MCCI_PUBLIC_ANNOTATION;
		}
	else if (!my_strncasecmp(type,MCCI_S_GROUP_ANN,
					strlen(MCCI_S_GROUP_ANN))) {
		annotationType = MCCI_GROUP_ANNOTATION;
		}
	else if (!my_strncasecmp(type,MCCI_S_PRIVATE_ANN,
					strlen(MCCI_S_PRIVATE_ANN))) {
		annotationType = MCCI_PRIVATE_ANNOTATION;
		}
	else if (!my_strncasecmp(type,MCCI_S_ALL_ANN,
					strlen(MCCI_S_ALL_ANN))) {
		annotationType = MCCI_ALL_ANNOTATION;
		}
	else {
		strcpy(retText,"PUBLIC, PRIVATE, GROUP or ALL annotation requests only");
		return(MCCIR_ERROR);
		}

	s = end;
	GetWordFromString(s,&url,&end); /* actual <url> */
	if ((!url) || (url == end)) {
		strcpy(retText,"Hey bud, where's the URL?");
		return(MCCIR_ERROR);
		}
	s = end;
	url++; /* skip over '<' */
	end--; /* backup over '>' */
	*end = '\0'; /* terminate url */

	url = strdup(url);
#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"GetAnnotation: URL=\"%s\"\n",url);
	}
#endif

#ifdef NEW
       MCCISendResponseLine(client,retCode,retText);
       for (int index = 0; index < NUM_ANNO_CODES; ++index) {
	 if (!my_strncasecmp(type,annoStrings[index],
			  strlen(annoStrings[index])) ||
	     !my_strncasecmp(type,MCCI_S_ALL_ANN,
					strlen(MCCI_S_ALL_ANN))) {
	   MCCIRequestGetAnnotation(&retCode,retText,retData,retDataLength,
			url,annoCodes[index]);
	 }
	 else
	   MCCIGetAnnotationDummyLine(&retCode,retText,retData,retDataLength,annoCodes[index]);

		if (retDataLength != 
		    NetServerWrite(client,retData,retDataLength)) {
		  return(MCCI_FAIL);
		}
#else /* NEW */
	MCCIRequestGetAnnotation(&retCode,retText,retData,retDataLength,
			url,annotationType);

#endif /* NEW */
	free(url);
	return(retCode);
}

int MCCIHandlePutAnnotation(client,line,retText)
MCCIPort client;
char *line; 	/* PUT request line */
char *retText;	/* text to be returned to cci client */
{
char *s;
char *end;
char *type;
char *url;
char *annotation;
int annotationLength;
int retCode;
int annotationType;

	if (!(s = strchr(line,' '))){ /* skip over GET */
		strcpy(retText,"Error in protocol");
		return(MCCIR_ERROR);
		}

	annotationType = 0;
	GetWordFromString(s,&type,&end); /* Get type (pub,priv,group)*/
	if (!my_strncasecmp(type,MCCI_S_PUBLIC_ANN,strlen(MCCI_S_PUBLIC_ANN))) {
		annotationType = MCCI_PUBLIC_ANNOTATION;
		}
	else if (!my_strncasecmp(type,MCCI_S_GROUP_ANN,
					strlen(MCCI_S_GROUP_ANN))) {
		annotationType = MCCI_GROUP_ANNOTATION;
		}
	else if (!my_strncasecmp(type,MCCI_S_PRIVATE_ANN,
					strlen(MCCI_S_PRIVATE_ANN))) {
		annotationType = MCCI_PRIVATE_ANNOTATION;
		}
	else {
		strcpy(retText,"PUBLIC, PRIVATE or GROUP put annotations only");
		return(MCCIR_ERROR);
		}

	s = end;
	GetWordFromString(s,&url,&end); /* actual <url> */
	if ((!url) || (url == end)) {
		strcpy(retText,"Hey bud, where's the URL?");
		return(MCCIR_ERROR);
		}
	s = end;
	url++; /* skip over '<' */
	end--; /* backup over '>' */
	*end = '\0'; /* terminate url */

	url = strdup(url);
#ifndef DISABLE_TRACE
	if (cciTrace) {
		fprintf(stderr,"GetURL: URL=\"%s\"\n",url);
	}
#endif
	annotationLength = MCCIReadContent(client,&annotation);
	if (annotationLength < 1) {
		strcpy(retText,"No annotation data");
		return(MCCIR_ERROR);
		}
	MCCIRequestPutAnnotation(&retCode,retText,annotationType,url,
						annotation,annotationLength);
	return(retCode);
}

int MCCIHandleFileToURL(client,line,retText)
MCCIPort client;
char *line;
char *retText;
{
char *fileName;
char *url;
char *s;
char *end;
int retCode;

	if (!(s = strchr(line,' '))){ /* skip over FILETOURL*/
		strcpy(retText,"Error in protocol");
		return(MCCIR_ERROR);
		}

	GetWordFromString(s,&fileName,&end); /* <fileName> */
	if ((!fileName) || (fileName == end)) {
		strcpy(retText,"I need a filename to translate");
		return(MCCIR_ERROR);
		}
	fileName++; /* skip over '<' */
	end--; /* backup over '>' */
	*end = '\0'; /* terminate fileName */

	fileName = strdup(fileName);

	MCCIRequestFileToURL(&retCode,retText,fileName);

	free(fileName);

	return(retCode);

}


int MCCIHandleInput(client)
/* read input from the client and do something with it */
/* return 1 on success, 0 on failure or disconnect */
MCCIPort client;
{
int retCode;
char retText[MCCI_MAX_RETURN_TEXT];
char *blah;
char **retData=&blah;
int  retDataLength = 0;
char *line;

	line = GetLine(client);

#ifndef DISABLE_TRACE
	if (cciTrace) {
		if (line)
			fprintf(stderr,"Server Read: %s\n",line);
		else
			fprintf(stderr,"Server Read: NULL line\n");
	}
#endif

	if (!line) {
		/* error or disconnect */
		MCCICloseConnection(client);
		return(0);
		}

/* parse the request */
/* to save speed & memory this parse is destructive to the text in 'line' */

	if (!my_strncasecmp(line,MCCI_S_DISCONNECT,strlen(MCCI_S_DISCONNECT))) {
                MCCISendResponseLine(client,MCCIR_DISCONNECT_OK,
                        "DISCONNECT request received");
		return(0);
		}
/* This has to go ahead of the simple get or else it gets snagged */
	else if (!my_strncasecmp(line,MCCI_S_GETANNOTATION,
					strlen(MCCI_S_GETANNOTATION))) {
		retDataLength = 0;

/*SWP -- 7/11/95
 * In the Original line, &retData is passed. retData is a char * to begin with
 * and it eventually gets assigned the value from mo_fetch_personal_annotations
 * which sends back all of the annotations for the specified url in one char
 * * string. Not an array of strings.
 */
		retCode = MCCIHandleGetAnnotation(client,line,retText,retData,
				&retDataLength);
/*Original
		retCode = MCCIHandleGetAnnotation(client,line,retText,&retData,
				&retDataLength);
*/
		MCCISendResponseLine(client,retCode,retText);
		if (retDataLength != 
		    NetServerWrite(client,*retData,retDataLength)) {
		  return(MCCI_FAIL);
		}

/* FINISHME */	/**** if retDataLength, send data */
		/*** if retDataLength, free retData?? */

		if (retDataLength>0) {
			free(*retData);
			}

		}
	else if (!my_strncasecmp(line,MCCI_S_GET,strlen(MCCI_S_GET))) {
		retCode = MCCIHandleGet(client,line,retText);
		MCCISendResponseLine(client,retCode,retText);
		}

	else if (!my_strncasecmp(line,MCCI_S_DOCOMMAND,strlen(MCCI_S_DOCOMMAND))){
		retCode = MCCIHandleDoCommand(client,line,retText);
		MCCISendResponseLine(client,retCode,retText);
		}

	else if (!my_strncasecmp(line,MCCI_S_DISPLAY,strlen(MCCI_S_DISPLAY))) {
		retCode = MCCIHandleDisplay(client, line, retText);
                MCCISendResponseLine(client, retCode,
                        "DISPLAY request received by Mosaic");
		}
	else if (!my_strncasecmp(line, MCCI_S_FORM, strlen(MCCI_S_FORM))) {
		retCode = MCCIHandleForm(client, line, retText);
                MCCISendResponseLine(client, MCCIR_FORM_OK,
                        "FORM request received by Mosaic");
		}
	else if (!my_strncasecmp(line,MCCI_S_QUIT,strlen(MCCI_S_QUIT))) {
                MCCISendResponseLine(client,MCCIR_QUIT_OK,
                        "QUIT request received exiting...");
		MCCIRequestQuit();
		}
	else if (!my_strncasecmp(line,MCCI_S_SEND,strlen(MCCI_S_SEND))) {
		retCode = MCCIHandleSend(client,line,retText);
                MCCISendResponseLine(client,retCode,retText);
		}
	else if (!my_strncasecmp(line,MCCI_S_POST,strlen(MCCI_S_POST))) {
		retCode = MCCIHandlePost(client,line,retText);
		MCCISendResponseLine(client,retCode,retText);
		}
	else if (!my_strncasecmp(line,MCCI_S_PUTANNOTATION,
					strlen(MCCI_S_PUTANNOTATION))) {
		retCode = MCCIHandlePutAnnotation(client,line,retText);
		MCCISendResponseLine(client,retCode,retText);
		}
	else if (!my_strncasecmp(line,MCCI_S_FILE_TO_URL,
					strlen(MCCI_S_FILE_TO_URL))) {
		retCode = MCCIHandleFileToURL(client,line,retText);
		MCCISendResponseLine(client,retCode,retText);
		}
	else {
		/* 
		MCCIRRequestUnrecognized();
		*/
		MCCISendResponseLine(client,MCCIR_UNRECOGNIZED,
			"Command not recognized");
		}
		

	return(1);
}


MCCISendAnchorHistory(client,url)
MCCIPort client;
char *url;
{
char buff[1024];
        if (MCCIanchorcached == 1)      /* ugly ADC hack ZZZ */
           sprintf(buff,"%s <%s> CACHED", MCCI_S_ANCHOR, url);
        else
	   sprintf(buff,"%s <%s>", MCCI_S_ANCHOR, url);

	return(MCCISendResponseLine(client, MCCIR_ANCHOR_INFO,buff));
}

int MCCIFormQueryToClient(client, actionID, query, contentType, post_data)  
MCCIPort client;
char *actionID;
char *query;
char *contentType;
char *post_data;
{
char buff[1024];
int length, dataLength;

	
	sprintf(buff, "%s %s ", actionID, query);
	if(MCCISendResponseLine(client, MCCIR_FORM_RESPONSE,buff)){
		return(MCCI_FAIL);
		}

	sprintf(buff,"Content-Type: %s\r\n",contentType);
	length = strlen(buff);
        if (length != NetServerWrite(client,buff,length)) {
                return(MCCI_FAIL);
                }

	dataLength = strlen(post_data); 
	sprintf(buff,"Content-Length: %d \r\n",dataLength);
	length = strlen(buff);
        if (length != NetServerWrite(client,buff,length)) {
                return(MCCI_FAIL);
                }

        if (dataLength!= NetServerWrite(client,post_data,dataLength)) {
                return(MCCI_FAIL);
		}
	
	return(MCCI_OK);
}

int MCCISendOutputFile(client,contentType,fileName)
MCCIPort client;
char *contentType;
char *fileName;
/* this routine used to send output back to the client */
{
int length;
int countDown;
char buff[1030];
struct stat fileInfo;
FILE *fp;

        if (stat(fileName,&fileInfo)) { /* get the length of the file */
                return(MCCI_FAIL);
		}

        if (!(fp = fopen(fileName,"r"))) {
                return(MCCI_FAIL);
                }

	sprintf(buff,"%d Send Data Output\r\n",MCCIR_SEND_DATA_OUTPUT);
	length = strlen(buff);
        if (length != NetServerWrite(client,buff,length)) {
                return(MCCI_FAIL);
                }

	sprintf(buff,"Content-Type: %s \r\n",contentType);
	length = strlen(buff);
        if (length != NetServerWrite(client,buff,length)) {
                return(MCCI_FAIL);
                }

	sprintf(buff,"Content-Length: %d \r\n",fileInfo.st_size);
	length = strlen(buff);
        if (length != NetServerWrite(client,buff,length)) {
                return(MCCI_FAIL);
                }

	countDown = fileInfo.st_size;
	while(countDown > 0) {
		if (0 < (length = fread(buff,1,1024,fp))) {
			if (length != NetServerWrite(client,buff,length)) {
				return(MCCI_FAIL);
				}
			countDown -= length;

			}
		else {
			/* error reading here...but we promised to send 
			   countDown number of bytes, so send nulls */
			while (countDown > 0) {
				if (1 != NetServerWrite(client,"\0",1)) {
					return(MCCI_FAIL);
					}
				countDown--;
				}
			}
		}
	fclose(fp);
	return(MCCI_OK);
	
}


int MCCISendBrowserViewOutput(client,url,contentType,data,dataLength)
/* Send BrowserView output response to client */
MCCIPort client;
char *url;
char *contentType;
char *data;
int dataLength;
{
char buff[1024];
int length;

	if (MCCI_OK!=MCCISendResponseLine(client,MCCIR_SEND_BROWSERVIEW,url)){
                return(MCCI_FAIL);
		}


	sprintf(buff,"Content-Type: %s\r\n",contentType);
	length = strlen(buff);
        if (length != NetServerWrite(client,buff,length)) {
                return(MCCI_FAIL);
                }

	sprintf(buff,"Content-Length: %d \r\n",dataLength);
	length = strlen(buff);
        if (length != NetServerWrite(client,buff,length)) {
                return(MCCI_FAIL);
                }

        if (dataLength!= NetServerWrite(client,data,dataLength)) {
                return(MCCI_FAIL);
		}
	
	return(MCCI_OK);

}


int MCCISendEventOutput(client, event_type)
/* Send Event output response to client */
MCCIPort client;
CCI_events event_type;
{
char buff[1024];
int length;

	sprintf(buff,"%d %d\r\n",MCCIR_SEND_EVENT, (int) event_type);
	length = strlen(buff);
        if (length != NetServerWrite(client,buff,length)) {
                return(MCCI_FAIL);
                }

	return(MCCI_OK);
}


int MCCISendMouseAnchorOutput(client, anchor)
/* Send MouseAnchor output response to client */
MCCIPort client;
char *anchor;
{

	if (MCCI_OK!=MCCISendResponseLine(client,MCCIR_SEND_MOUSE_ANCHOR,anchor)){
                return(MCCI_FAIL);
		}

	return(MCCI_OK);
}

