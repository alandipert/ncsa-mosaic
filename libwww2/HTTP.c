/*	HyperText Tranfer Protocol	- Client implementation		HTTP.c
**	==========================
*/
#include "../config.h"
#include "HTTP.h"

#include "../src/mosaic.h"
#include "../src/mo-www.h"

#define HTTP_VERSION	"HTTP/1.0"

#define INIT_LINE_SIZE		1024	/* Start with line buffer this big */
#define LINE_EXTEND_THRESH	256	/* Minimum read size */
#define VERSION_LENGTH 		20	/* for returned protocol version */

#include "HTParse.h"
#include "HTUtils.h"
#include "tcp.h"
#include "HTTCP.h"
#include "HTFormat.h"
#include "HTFile.h"
#include <ctype.h>
#include "HTAlert.h"
#include "HTMIME.h"
#include "HTML.h"
#include "HTInit.h"
#include "HTAABrow.h"


int useKeepAlive=1;
extern int securityType;
int sendAgent=1;
int sendReferer=1;
extern int selectedAgent;
extern char **agent;

#ifndef DISABLE_TRACE
/* new logging stuff */
int httpTrace=0;
int www2Trace=0;
#endif

char **extra_headers=NULL;

/* I know...I know...a lib should not depend on a program... */
#define _LIBWWW2
#include "../src/kcms.h"

struct _HTStream
{
  HTStreamClass * isa;
};

/* for browser to call -- BJS */
void HT_SetExtraHeaders(char **headers)
{
	extra_headers = headers;
}

/* defined in src/mo-www.c */
extern char * HTAppName;	/* Application name: please supply */
extern char * HTAppVersion;	/* Application version: please supply */
extern char * HTReferer;        /* HTTP referer field - amb */

/* Variables that control whether we do a POST or a GET,
   and if a POST, what and how we POST. And HEAD -- SWP*/
int do_head = 0;
char *headData=NULL;
int do_post = 0;
int do_put = 0;
int do_meta = 0;
int put_file_size=0;
FILE *put_fp;
char *post_content_type = NULL;
char *post_data = NULL;
extern BOOL using_gateway;    /* are we using an HTTP gateway? */
extern char *proxy_host_fix;  /* for the Host: header */
extern BOOL using_proxy;      /* are we using an HTTP proxy gateway? */
PUBLIC BOOL reloading = NO;   /* did someone say, "RELOAD!?!?!" swp */

/*		Load Document from HTTP Server			HTLoadHTTP()
**		==============================
**
**	Given a hypertext address, this routine loads a document.
**
**
** On entry,
**	arg	is the hypertext reference of the article to be loaded.
**
** On exit,
**	returns	>=0	If no error, a good socket number
**		<0	Error.
**
**	The socket must be closed by the caller after the document has been
**	read.
**
*/

/* where was our last connection to? */
static int lsocket = -1;
static char *addr = NULL;

PUBLIC int HTLoadHTTP ARGS4 (
	char *, 		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
  int s;				/* Socket number for returned data */
  char *command;			/* The whole command */
  char *eol;			/* End of line if found */
  char *start_of_data;		/* Start of body of reply */
  int status;				/* tcp return */
  int bytes_already_read;
  char crlf[3];			/* A CR LF equivalent string */
  HTStream *target;		/* Unconverted data */
  HTFormat format_in;			/* Format arriving in the message */

  char *line_buffer;
  char *line_kept_clean;
  BOOL extensions;		/* Assume good HTTP server */
  int compressed;
  char line[2048];	/* bumped up to cover Kerb huge headers */

  int length, doing_redirect, rv;
  int already_retrying = 0;
  int return_nothing;

  int i;
  int keepingalive = 0;
  char *p;

/*SWP*/
  int statusError=0;

  if (!arg)
    {
      status = -3;
      HTProgress ("Bad request.");
      goto done;
    }
  if (!*arg)
    {
      status = -2;
      HTProgress ("Bad request.");
      goto done;
    }

  sprintf(crlf, "%c%c", CR, LF);

  /* At this point, we're talking HTTP/1.0. */
  extensions = YES;

 try_again:
  /* All initializations are moved down here from up above,
     so we can start over here... */
  eol = 0;
  bytes_already_read = 0;
  length = 0;
  doing_redirect = 0;
  compressed = 0;
  target = NULL;
  line_buffer = NULL;
  line_kept_clean = NULL;
  return_nothing = 0;

	/* okay... addr looks like http://hagbard.ncsa.uiuc.edu/blah/etc.html
	lets crop it at the 3rd '/' */
	for(p = arg,i=0;*p && i!=3;p++)
		if(*p=='/') i++;

	if(i==3)
		i = p-arg; /* i = length not counting last '/' */
	else
		i = 0;

	if((lsocket != -1) && i && addr && !strncmp(addr,arg,i)){
		/* keepalive is active and addresses match -- try the old socket */
		s = lsocket;
		keepingalive = 1; /* flag in case of network error due to server timeout*/
		lsocket = -1; /* prevent looping on failure */
#ifndef DISABLE_TRACE
		if (www2Trace)
			fprintf (stderr,
				 "HTTP: Keep-Alive reusing '%s'\n",addr);
#endif
/*
		if (addr && *addr) {
			sprintf(tmpbuf,"Reusing socket from %s.",addr);
			HTProgress(tmpbuf);
		}
*/
	} else {
		if(addr) free(addr);
		/* save the address for next time around */
		addr = malloc(i+1);
		strncpy(addr,arg,i);
		*(addr+i)=0;

		keepingalive = 0; /* just normal opening of the socket */
		if(lsocket != -1) NETCLOSE(lsocket); /* no socket leaks here */
		lsocket = -1; /*dont assign until we know the server says okay */

#ifndef DISABLE_TRACE
		if (www2Trace)
			fprintf (stderr,
				 "HTTP: Keep-Alive saving '%s'\n",addr);
#endif
/*
		if (addr && *addr) {
			sprintf(tmpbuf,"Saving %s for possible socket reuse.",addr);
			HTProgress(tmpbuf);
		}
*/
	}

  if (!keepingalive) {
      status = HTDoConnect (arg, "HTTP", TCP_PORT, &s);
      if (status == HT_INTERRUPTED){
	  /* Interrupt cleanly. */
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf (stderr,
		       "HTTP: Interrupted on connect; recovering cleanly.\n");
#endif
	  HTProgress ("Connection interrupted.");
	  /* status already == HT_INTERRUPTED */
	  goto done;
      }
      if (status < 0) {
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr,
		      "HTTP: Unable to connect to remote host for `%s' (errno = %d).\n", arg, errno);
#endif
	  HTProgress ("Unable to connect to remote host.");
	  status = HT_NO_DATA;
	  goto done;
      }
  }
  /*	Ask that node for the document,
   **	omitting the host name & anchor
   */
  {
    char * p1 = HTParse(arg, "", PARSE_PATH|PARSE_PUNCTUATION);
    command = malloc(5 + strlen(p1)+ 2 + 31);

    if (do_post && !do_put)
      strcpy(command, "POST ");
    else if (do_post && do_put)
      strcpy(command, "PUT ");
    else if (do_head)
      strcpy(command, "HEAD ");
    else if (do_meta)
      strcpy(command, "META ");
    else
      strcpy(command, "GET ");

    /*
     * For a gateway, the beginning '/' on the request must
     * be stripped before appending to the gateway address.
     */
    if ((using_gateway)||(using_proxy)) {
        strcat(command, p1+1);
    }
    else
        strcat(command, p1);
    free(p1);
  }
  if (extensions)
    {
      strcat(command, " ");
      strcat(command, HTTP_VERSION);
    }

  strcat(command, crlf);	/* CR LF, as in rfc 977 */

  if (extensions)
    {
#ifdef SAM_NOT_YET
		/* SAM This produces an absolutely huge Accept: line.  While
		   there is probably a better way to turn this off, just
		   compiling it out works for now.
		*/
      int n, i;

      if (!HTPresentations) HTFormatInit();
      n = HTList_count(HTPresentations);

      begin_ptr=command+strlen(command);
      env_length=0;

      sprintf(line, "Accept:");
      env_length+=strlen(line);
      StrAllocCat(command, line);

	/* KCMS Accept Header - swp */
	if (KCMS_Return_Format==JPEG) {
		sprintf(line," image/x-pcd-jpeg,");
		StrAllocCat(command, line);
		env_length+=strlen(line);
	}
	else if (KCMS_Return_Format==JYCC) {
		sprintf(line," image/x-pcd-jycc,");
		StrAllocCat(command, line);
		env_length+=strlen(line);
	}
	else if (KCMS_Return_Format==GIF) {
		sprintf(line," image/x-pcd-gif,");
		StrAllocCat(command, line);
		env_length+=strlen(line);
	}

      for(i=0; i<n; i++)
        {
          HTPresentation * pres = HTList_objectAt(HTPresentations, i);
          if (pres->rep_out == WWW_PRESENT)
            {
		sprintf(line, " %s,",HTAtom_name(pres->rep));
		env_length+=strlen(line);
		StrAllocCat(command, line);
		if (env_length>200) {
			if ((tmp_ptr=strrchr(command,','))!=NULL) {
				*tmp_ptr='\0';
			}
			sprintf(line, "%c%c",CR,LF);
			StrAllocCat(command, line);

			begin_ptr=command+strlen(command);
			sprintf(line, "Accept:");
			env_length=strlen(line);
			StrAllocCat(command, line);
		}
            }
        }

      /* This gets rid of the last comma. */
      if ((tmp_ptr=strrchr(command,','))!=NULL) {
	*tmp_ptr='\0';
	sprintf(line, "%c%c",CR,LF);
	StrAllocCat(command, line);
      }
      else { /* No accept stuff...get rid of "Accept:" */
	begin_ptr='\0';
      }
#endif

      /* if reloading, send no-cache pragma to proxy servers. --swp */
      /* original patch from Ari L. <luotonen@dxcern.cern.ch> */
      if (reloading) {
	sprintf(line, "Pragma: no-cache%c%c", CR, LF);
	StrAllocCat(command, line);
      }

      /*This is just used for "not" sending this header on a proxy request*/
      if (useKeepAlive) {
	sprintf(line, "Connection: keep-alive%c%c", CR, LF);
	StrAllocCat(command, line);
      }

      if (sendAgent) {
	sprintf(line, "User-Agent: %s%c%c",agent[selectedAgent],CR,LF);
/*
	sprintf(line, "User-Agent:  %s/%s  libwww/%s%c%c",
		HTAppName ? HTAppName : "unknown",
		HTAppVersion ? HTAppVersion : "0.0",
		HTLibraryVersion, CR, LF);
*/
	StrAllocCat(command, line);
      }

      if (sendReferer) {
	/* HTTP Referer field, specifies back-link URL   - amb */
	if (HTReferer) {
		sprintf(line, "Referer: %s%c%c", HTReferer, CR, LF);
		StrAllocCat(command, line);
		HTReferer = NULL;
	}
      }

      {
	char *tmp,*startPtr,*endPtr;

	/* addr is always in URL form */

	if (addr && !using_proxy && !using_gateway) {
		tmp=strdup(addr);
		startPtr=strchr(tmp,'/');
		if (startPtr) {
			startPtr+=2; /*now at begining of hostname*/
			if (*startPtr) {
				endPtr=strchr(startPtr,':');
				if (!endPtr) {
					endPtr=strchr(startPtr,'/');
					if (endPtr && *endPtr) {
						*endPtr='\0';
					}
				}
				else {
					*endPtr='\0';
				}

				sprintf(line, "Host: %s%c%c", startPtr, CR, LF);
				StrAllocCat(command, line);

				free(tmp);
				tmp=startPtr=endPtr=NULL;
			}
		}
	}
	else if (using_proxy || using_gateway) {
		sprintf(line, "Host: %s%c%c", proxy_host_fix, CR, LF);
		StrAllocCat(command, line);
	}
      }

      /* SWP -- 7/10/95 */
      /* HTTP Extension headers */
      /* Domain Restriction */
      sprintf(line,"Extension: Notify-Domain-Restriction%c%c",CR,LF);
      StrAllocCat(command,line);

      /* BJS -- 12/05/95 -- allow arbitrary headers sent from browser */
      if(extra_headers){
		int h;
		for(h=0;extra_headers[h];h++){
			sprintf(line,"%s%c%c",extra_headers[h],CR,LF);
			StrAllocCat(command,line);
		}
      }

      {
        char *docname;
        char *hostname;
        char *colon;
        int portnumber;
        char *auth;

        docname = HTParse(arg, "", PARSE_PATH);
        hostname = HTParse(arg, "", PARSE_HOST);
        if (hostname &&
            NULL != (colon = strchr(hostname, ':')))
          {
            *(colon++) = '\0';	/* Chop off port number */
            portnumber = atoi(colon);
          }
        else portnumber = 80;

        if (NULL!=(auth=HTAA_composeAuth(hostname, portnumber, docname)))
          {
            sprintf(line, "%s%c%c", auth, CR, LF);
            StrAllocCat(command, line);
          }
#ifndef DISABLE_TRACE
        if (www2Trace)
          {
            if (auth)
              fprintf(stderr, "HTTP: Sending authorization: %s\n", auth);
            else
              fprintf(stderr, "HTTP: Not sending authorization (yet)\n");
          }
#endif
        FREE(hostname);
        FREE(docname);
      }
    }

  if (do_post && !do_put)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "HTTP: Doing post, content-type '%s'\n",
                 post_content_type);
#endif
      sprintf (line, "Content-type: %s%c%c",
               post_content_type ? post_content_type : "lose", CR, LF);
      StrAllocCat(command, line);
      {
        int content_length;
        if (!post_data)
          content_length = 4; /* 4 == "lose" :-) */
        else
          content_length = strlen (post_data);
        sprintf (line, "Content-length: %d%c%c",
                 content_length, CR, LF);
        StrAllocCat(command, line);
      }

      StrAllocCat(command, crlf);	/* Blank line means "end" */

      if (post_data)
        StrAllocCat(command, post_data);
      else
        StrAllocCat(command, "lose");
    }
  else if (do_post && do_put)
    {
      sprintf (line, "Content-length: %d%c%c",
	       put_file_size, CR, LF);
      StrAllocCat(command, line);
      StrAllocCat(command, crlf);	/* Blank line means "end" */
    }
  else {
      StrAllocCat(command, crlf);	/* Blank line means "end" */
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "Writing:\n%s----------------------------------\n",
             command);
#endif

/*
  HTProgress ("Sending HTTP request.");
*/

  status = NETWRITE(s, command, (int)strlen(command));
  if (do_post && do_put) {
      char buf[BUFSIZ];
      int upcnt=0,n;

      while (status>0) {
	n=fread(buf,1,BUFSIZ-1,put_fp);

	upcnt+= status = NETWRITE(s, buf, n);
#ifndef DISABLE_TRACE
	if (www2Trace) {
		fprintf(stderr,"[%d](%d) %s",status,n,buf);
	}
#endif
	if (feof(put_fp)) {
		break;
	}
      }

      if (status<0 || !feof(put_fp) || upcnt!=put_file_size) {
	char tmpbuf[BUFSIZ];

	sprintf(tmpbuf,"Status: %d  --  EOF: %d  --  UpCnt/FileSize: %d/%d\n\nThe server you connected to either does not support\nthe PUT method, or an error occurred.\n\nYour upload was corrupted! Please try again!",status,(feof(put_fp)?1:0),upcnt,put_file_size);
	application_error(tmpbuf,"Upload Error!");
      }
  }

  /* Twirl on each request to make things look nicer -- SWP */
  HTCheckActiveIcon(1);

#ifndef DISABLE_TRACE
  if (httpTrace) {
	fprintf(stderr,"%s",command);
  }
#endif

  free (command);
  if (status <= 0)
    {
      if (status == 0)
        {
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "HTTP: Got status 0 in initial write\n");
#endif
          /* Do nothing. */
        }
      else if
        ((errno == ENOTCONN || errno == ECONNRESET || errno == EPIPE) &&
         !already_retrying &&
         /* Don't retry if we're posting. */ !do_post)
          {
            /* Arrrrgh, HTTP 0/1 compability problem, maybe. */
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf
                (stderr,
                 "HTTP: BONZO ON WRITE Trying again with HTTP0 request.\n");
#endif
/*
            HTProgress ("Retrying as HTTP0 request.");
*/
            NETCLOSE(s);
// SAM            extensions = NO;
            already_retrying = 1;
            goto try_again;
          }
      else
        {
		if(keepingalive){
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr, "HTTP: Timeout on Keep-Alive. Retrying.\n");
#endif
			HTProgress("Server Timeout: Reconnecting");
			goto try_again;
		}
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "HTTP: Hit unexpected network WRITE error; aborting connection.\n");
#endif
          NETCLOSE (s);
          status = -1;
          HTProgress ("Unexpected network write error; connection aborted.");
          goto done;
        }
    }

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "HTTP: WRITE delivered OK\n");
#endif
  HTProgress ("Done sending HTTP request; waiting for response.");

  /*	Read the first line of the response
   **	-----------------------------------
   */

  {
      /* Get numeric status etc */
      BOOL end_of_file = NO;
      int buffer_length = INIT_LINE_SIZE;

      line_buffer = (char *) malloc(buffer_length * sizeof(char));

      do {
	  /* Loop to read in the first line */
	  /* Extend line buffer if necessary for those crazy WAIS URLs ;-) */
	  if (buffer_length - length < LINE_EXTEND_THRESH) {
	      buffer_length = buffer_length + buffer_length;
	      line_buffer =
		  (char *) realloc(line_buffer, buffer_length * sizeof(char));
          }
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf (stderr, "HTTP: Trying to read %d\n",
		       buffer_length - length - 1);
#endif
	  status = NETREAD(s, line_buffer + length,
			   buffer_length - length - 1);
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf (stderr, "HTTP: Read %d\n", status);
#endif
	  if (status <= 0) {
	      /* Retry if we get nothing back too;
		 bomb out if we get nothing twice. */
	      if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
		  if (www2Trace)
		      fprintf (stderr, "HTTP: Interrupted initial read.\n");
#endif
		  HTProgress ("Connection interrupted.");
		  status = HT_INTERRUPTED;
		  NETCLOSE (s);
		  goto clean_up;
              } else
		  if
		  (status < 0 &&
		   (errno == ENOTCONN || errno == ECONNRESET || errno == EPIPE)
		   && !already_retrying && !do_post)
		      {
			  /* Arrrrgh, HTTP 0/1 compability problem, maybe. */
#ifndef DISABLE_TRACE
			  if (www2Trace)
			      fprintf (stderr, "HTTP: BONZO Trying again with HTTP0 request.\n");
#endif
			  NETCLOSE(s);
			  if (line_buffer)
			      free(line_buffer);
			  if (line_kept_clean)
			      free(line_kept_clean);

			  extensions = NO;
			  already_retrying = 1;
			  HTProgress ("Retrying as HTTP0 request.");
			  goto try_again;
		      } else {
			      if(keepingalive){
#ifndef DISABLE_TRACE
					if (www2Trace)
						fprintf (stderr, "HTTP: Timeout on Keep-Alive. Retrying.\n");
#endif
					HTProgress("Server Timeout: Reconnecting");
					goto try_again;
			      }
#ifndef DISABLE_TRACE
			  if (www2Trace)
			      fprintf (stderr, "HTTP: Hit unexpected network read error; aborting connection; status %d.\n", status);
#endif
			  HTProgress
			      ("Unexpected network read error; connection aborted.");

			  NETCLOSE (s);
			  status = -1;
			  goto clean_up;
		      }
          }

	  bytes_already_read += status;
	  {
	      char line[256];
	      sprintf (line, "Read %d bytes of data.", bytes_already_read);
	      HTProgress (line);
	  }

	  if (status == 0) {
	      end_of_file = YES;
	      break;
          }
	  line_buffer[length+status] = 0;

	  if (line_buffer) {
	      if (line_kept_clean)
		  free (line_kept_clean);
	      line_kept_clean = (char *)malloc (buffer_length * sizeof (char));
/*
	      bcopy (line_buffer, line_kept_clean, buffer_length);
*/
	      memcpy (line_kept_clean, line_buffer, buffer_length);
          }

	  eol = strchr(line_buffer + length, LF);
	  /* Do we *really* want to do this? */
	  if (eol && eol != line_buffer && *(eol-1) == CR)
	      *(eol-1) = ' ';

	  length = length + status;

	  /* Do we really want to do *this*? */
	  if (eol)
	      *eol = 0;		/* Terminate the line */
      /* All we need is the first line of the response.  If it's a HTTP/1.0
	 response, then the first line will be absurdly short and therefore
	 we can safely gate the number of bytes read through this code
	 (as opposed to below) to ~1000. */
      /* Well, let's try 100. */
      } while (!eol && !end_of_file && bytes_already_read < 100);
  } /* Scope of loop variables */


  /*	We now have a terminated unfolded line. Parse it.
   **	-------------------------------------------------
   */
#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf(stderr, "HTTP: Rx: %s\n", line_buffer);
#endif

  {
    int fields;
    char server_version[VERSION_LENGTH+1];
    int server_status;

	/*SWP*/
	statusError=0;

    server_version[0] = 0;

    fields = sscanf(line_buffer, "%20s %d",
                    server_version,
                    &server_status);

#ifndef DISABLE_TRACE
    if (www2Trace)
      fprintf (stderr, "HTTP: Scanned %d fields from line_buffer\n", fields);
#endif
#ifndef DISABLE_TRACE
    if (www2Trace)
      fprintf (stderr, "HTTP: line_buffer '%s'\n", line_buffer);
#endif

    /* Rule out HTTP/1.0 reply as best we can. */
    if (fields < 2 || !server_version[0] || server_version[0] != 'H' ||
        server_version[1] != 'T' || server_version[2] != 'T' ||
        server_version[3] != 'P' || server_version[4] != '/' ||
        server_version[6] != '.')
      {
        /* HTTP0 reply */
        HTAtom * encoding;

#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf (stderr, "--- Talking HTTP0.\n");
#endif

        format_in = HTFileFormat(arg, &encoding, WWW_HTML, &compressed);
        start_of_data = line_kept_clean;
      }
    else
      {
        /* Decode full HTTP response */
        format_in = HTAtom_for("www/mime");
        /* We set start_of_data to "" when !eol here because there
           will be a put_block done below; we do *not* use the value
           of start_of_data (as a pointer) in the computation of
           length or anything else in this situation. */
        start_of_data = eol ? eol + 1 : "";
        length = eol ? length - (start_of_data - line_buffer) : 0;

#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf (stderr, "--- Talking HTTP1.\n");
#endif

        switch (server_status / 100)
          {
          case 3:		/* Various forms of redirection */
            /* We now support this in the parser, at least. */
            doing_redirect = 1;
            break;

          case 4:		/* "I think I goofed" */
            switch (server_status)
              {
              case 403:
		statusError=1;
                /* 403 is "forbidden"; display returned text. */
                break;

              case 401:
                /* length -= start_of_data - text_buffer; */
                if (HTAA_shouldRetryWithAuth(start_of_data, length, s))
                  {
                    (void)NETCLOSE(s);
			lsocket = -1;
                    if (line_buffer)
                      free(line_buffer);
                    if (line_kept_clean)
                      free(line_kept_clean);

#ifndef DISABLE_TRACE
                    if (www2Trace)
                      fprintf(stderr, "%s %d %s\n",
                              "HTTP: close socket", s,
                              "to retry with Access Authorization");
#endif

                    HTProgress ("Retrying with access authorization information.");
                    goto try_again;
                    break;
                  }
                else
                  {
		    statusError=1;
                    /* Fall through. */
                  }

              default:
		statusError=1;
                break;
              } /* case 4 switch */
            break;

          case 5:		/* I think you goofed */
	    statusError=1;
            break;

          case 2:		/* Good: Got MIME object */
            switch (server_status)
              {
              case 204:
                return_nothing = 1;
                format_in = HTAtom_for("text/html");
                break;
	      case 200:
		if (do_head) {
			if (!start_of_data || !*start_of_data) {
				headData=NULL;
			}
			else {
				char *ptr;

				headData=strdup(start_of_data);
				ptr=strchr(headData,'\n');
				*ptr='\0';
			}
		}
		break;
              default:
                break;
              }
            break;

          default:		/* bad number */
	    statusError=1;
            HTAlert("Unknown status reply from server!");
            break;
          } /* Switch on server_status/100 */

      }	/* Full HTTP reply */
  } /* scope of fields */

  /* Set up the stream stack to handle the body of the message */
  target = HTStreamStack(format_in,
                         format_out,
                         compressed,
                         sink, anAnchor);

  if (!target)
    {
      char buffer[1024];	/* @@@@@@@@ */
      sprintf(buffer, "Sorry, no known way of converting %s to %s.",
              HTAtom_name(format_in), HTAtom_name(format_out));
      HTProgress (buffer);
      status = -1;
      NETCLOSE (s);
	lsocket = -1;
      goto clean_up;
    }

  if (!return_nothing)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "HTTP: Doing put_block, '%s'\n", start_of_data);
#endif
/* BJS: parse start_of_data...*/
	for(p=start_of_data;*p;p++){
/*		if(*p=='C' && !strncmp("Content-length: ",p,16)){
		i = 0;
		p+=16;
		while(*p && isdigit(*p)){
			i = i*10 + *p-'0';
			p++;
		}
		p--;
#ifndef DISABLE_TRACE
		if(www2Trace)
			fprintf(stderr, "HTTP: Content Length is %d\n",i);
#endif
	}
*/
		if(*p=='K' && !strncmp("Keep-Alive:",p,11)){
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr, "HTTP: Server Agrees to Keep-Alive\n");
#endif
			lsocket = s;
			p+=10;
		}
	}

#ifndef DISABLE_TRACE
        if (www2Trace && lsocket == -1)
		fprintf (stderr, "HTTP: Server does not agree to Keep-Alive\n");
#endif
      /* Recycle the first chunk of data, in all cases. */
      (*target->isa->put_block)(target, start_of_data, length);

      /* Go pull the bulk of the data down. */
	/* if we dont use length, header length is wrong due to the
	    discarded first line - bjs*/
      rv = HTCopy(s, target, length /*bytes_already_read*/);
      if (rv == -1)
        {
          (*target->isa->handle_interrupt) (target);
          status = HT_INTERRUPTED;
	  NETCLOSE (s);
		lsocket = -1;
          goto clean_up;
        }
      if (rv == -2 && !already_retrying && !do_post)
        {
          /* Aw hell. */
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "HTTP: Trying again with HTTP0 request.\n");
#endif
          /* May as well consider it an interrupt -- right? */
          (*target->isa->handle_interrupt) (target);
          NETCLOSE(s);
          if (line_buffer)
            free(line_buffer);
          if (line_kept_clean)
            free(line_kept_clean);

          extensions = NO;
          already_retrying = 1;
          HTProgress ("Retrying as HTTP0 request.");
          goto try_again;
        }
    }
  else
    {
      /* return_nothing is high. */
      (*target->isa->put_string) (target, "<mosaic-access-override>\n");
      HTProgress ("And silence filled the night.");
    }

  (*target->isa->end_document)(target);

  /* Close socket before doing free. */
	if(lsocket == -1){
		NETCLOSE(s);
#ifndef DISABLE_TRACE
		if(www2Trace)
			fprintf(stderr,"HTTP: Closing connection\n");
#endif
	} else {
		HTProgress("Leaving Server Connection Open");
#ifndef DISABLE_TRACE
		if(www2Trace)
			fprintf(stderr,"HTTP: Keeping connection alive\n");
#endif
	}
/*
  else {
      NETCLOSE(s);
#ifndef DISABLE_TRACE
      if(www2Trace)
	fprintf(stderr,"HTTP: Closing connection\n");
#endif
  }
*/

  (*target->isa->free)(target);

  if (doing_redirect)
    {
      /* OK, now we've got the redirection URL temporarily stored
         in external variable redirecting_url, exported from HTMIME.c,
         since there's no straightforward way to do this in the library
         currently.  Do the right thing. */
      status = HT_REDIRECTING;
    }
  else
    {
      status = HT_LOADED;
    }

  /*	Clean up
   */

 clean_up:
  if (line_buffer)
    free(line_buffer);
  if (line_kept_clean)
    free(line_kept_clean);

 done:
  /* Clear out on exit, just in case. */
  do_post = 0;

  if (statusError) {
	securityType=HTAA_NONE;
#ifndef DISABLE_TRACE
	if (www2Trace)
		fprintf(stderr,"Resetting security type to NONE.\n");
#endif
  }

  return status;
}


/*	Protocol descriptor
*/

PUBLIC HTProtocol HTTP = { "http", HTLoadHTTP, 0 };














