/*	WorldWideWeb - Wide Area Informaion Server Access	HTWAIS.c
**	==================================================
**
**	This module allows a WWW server or client to read data from a
**	remote  WAIS
**  server, and provide that data to a WWW client in hypertext form.
**  Source files, once retrieved, are stored and used to provide
**  information about the index when that is acessed.
**
** Authors
**	BK	Brewster Kahle, Thinking Machines, <Brewster@think.com>
**	TBL	Tim Berners-Lee, CERN <timbl@info.cern.ch>
**
** History
**	   Sep 91	TBL adapted shell-ui.c (BK) with HTRetrieve.c from WWW.
**	   Feb 91	TBL Generated HTML cleaned up a bit (quotes, escaping)
**			    Refers to lists of sources. 
**	   Mar 93	TBL   Lib 2.0 compatible module made.	
**
** Bugs
**	Uses C stream i/o to read and write sockets, which won't work
**	on VMS TCP systems.
**
**	Should cache connections.
**
**	ANSI C only as written
**
** WAIS comments:
**
**	1.	Separate directories for different system's .o would help
**	2.	Document ids are rather long!
**
** WWW Address mapping convention:
**
**	/servername/database/type/length/document-id
**
**	/servername/database?word+word+word
*/
/* WIDE AREA INFORMATION SERVER SOFTWARE:
   No guarantees or restrictions.  See the readme file for the full standard
   disclaimer.

   Brewster@think.com
*/
#include "../config.h"
#ifdef DIRECT_WAIS

#define BIG 10000

/*			From WAIS
**			---------
*/

#include <ui.h>

#define MAX_MESSAGE_LEN 100000
#define CHARS_PER_PAGE 4096 /* number of chars retrieved in each request */


/*			FROM WWW
**			--------
*/
#define HEX_ESCAPE '%'

#include "HTUtils.h"
#include "tcp.h"
#include "HTParse.h"
#include "HTAccess.h"		/* We implement a protocol */
#include "HTML.h"		/* The object we will generate */
#include "HTFile.h"
#include "HTAlert.h"
 
/* #include "ParseWSRC.h" */

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

extern int WWW_TraceFlag;	/* Control diagnostic output */
extern FILE * logfile;		/* Log file output */
/*char *log_file_name = "/dev/null";*/
extern char *log_file_name;     /* change for freeWAIS-0.5 - DXP */

PRIVATE char	line[2048];	/* For building strings to display */
				/* Must be able to take id */


#include "HTFormat.h"
#include "HTTCP.h"
/* #include "HTWSRC.h"	*/	/* Need some bits from here */

/*		Hypertext object building machinery
*/
#include "HTML.h"

#define PUTC(c) (*target->isa->put_character)(target, c)
#define PUTBLOCK(c, len) (*target->isa->put_block)(target, c, len)
#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)
#define END_TARGET (*target->isa->end_document)(target)
#define FREE_TARGET (*target->isa->free)(target)

struct _HTStructured {
	WWW_CONST HTStructuredClass *	isa;
	/* ... */
};

struct _HTStream {
	WWW_CONST HTStreamClass *	isa;
	/* ... */
};


/*								showDiags
*/
/* modified from Jonny G's version in ui/question.c */

void showDiags ARGS2(
	HTStream *, 		target,
	diagnosticRecord **, 	d)
{
  long i;

  for (i = 0; d[i] != NULL; i++) {
    if (d[i]->ADDINFO != NULL) {
      PUTS("Diagnostic code is ");
      PUTS(d[i]->DIAG);
      PUTC(' ');
      PUTS(d[i]->ADDINFO);
      PUTC('\n'); ;
    }
  }
}

/*	Matrix of allowed characters in filenames
**	-----------------------------------------
*/

PRIVATE BOOL acceptable[256];
PRIVATE BOOL acceptable_inited = NO;

PRIVATE void init_acceptable NOARGS
{
    unsigned int i;
    char * good = 
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./-_$";
    for(i=0; i<256; i++) acceptable[i] = NO;
    for(;*good; good++) acceptable[(unsigned int)*good] = YES;
    acceptable_inited = YES;
}

/*	Transform file identifier into WWW address
**	------------------------------------------
**
**
** On exit,
**	returns		nil if error
**			pointer to malloced string (must be freed) if ok
*/
char * WWW_from_archie ARGS1 (char *, file)
{
    char * end;
    char * result;
    char * colon;
    for(end=file; *end > ' '; end++);	/* assumes ASCII encoding*/
    result = (char *)malloc(10 + (end-file));
    if (!result) return result;		/* Malloc error */
    strcpy(result, "file://");
    strncat(result, file, end-file);
    colon = strchr(result+7, ':');	/* Expect colon after host */
    if (colon) {
	for(; colon[0]; colon[0]=colon[1], colon++);	/* move down */
    }
    return result;
} /* WWW_from_archie */

/*	Transform document identifier into URL
**	--------------------------------------
**
** Bugs: A static buffer of finite size is used!
**	The format of the docid MUST be good!
**
** On exit,
**	returns		nil if error
**			pointer to malloced string (must be freed) if ok
*/
PRIVATE char hex [17] = "0123456789ABCDEF";
extern char from_hex PARAMS((char a));			/* In HTWSRC @@ */

PRIVATE char * WWW_from_WAIS ARGS1(any *, docid)
{
  static unsigned char buf[BIG];
  char num[10];
  unsigned char * q = buf;
  char * p = (docid->bytes);
  int i, l;
#ifndef DISABLE_TRACE
  if (www2Trace) 
    {
      char *p;
      fprintf(stderr, "WAIS id (%d bytes) is ", (int)docid->size);
      for (p = docid->bytes; p < docid->bytes + docid->size; p++) 
        {
          if ((*p >= ' ') && (*p<= '~')) /* Assume ASCII! */
            fprintf(stderr, "%c", *p);
          else
            fprintf(stderr, "<%x>", (unsigned)*p);
        }
      fprintf(stderr, "\n");
    }	 
#endif

  for (p = docid->bytes; 
       (p < docid->bytes+docid->size) && 
       (q < &buf[BIG]);) 
    {
#ifndef DISABLE_TRACE
      if (www2Trace) fprintf(stderr, "    Record type %d, length %d\n",
                         p[0], p[1]);
#endif
      sprintf(num, "%d", (int)*p);
/*
      bcopy(num, q, strlen(num));
*/
      memcpy(q, num, strlen(num));
      q += strlen(num);
      p++;
      *q++ = '=';		/* Separate */
      l = (int)((unsigned char)*p);
      p++;
      if (l > 127)
        {
          l = (l - 128) * 128;
          l = l + (int)((unsigned char)*p);
          p++;
        }
      
      for (i = 0; i < l; i++, p++)
        {
          if (!acceptable[(unsigned char)*p]) 
            {
              *q++ = HEX_ESCAPE;
              *q++ = hex[((unsigned char)*p) >> 4];
              *q++ = hex[((unsigned char)*p) & 15];
            }
          else *q++ = (unsigned char)*p;
        }
      *q++= ';';		/* Terminate field */
    }
  *q++ = 0;			/* Terminate string */
#ifndef DISABLE_TRACE
  if (www2Trace) 
    fprintf(stderr, "WWW form of id: %s\n", buf); 
#endif

  {
    char * result = (char *)malloc (strlen (buf) + 1);
    strcpy (result, buf);
    return result;
  }
} /* WWW_from_WAIS */


/*	Transform URL into WAIS document identifier
**	-------------------------------------------
**
** On entry,
**	docname		points to valid name produced originally by
**			WWW_from_WAIS
** On exit,
**	docid->size	is valid
**	docid->bytes	is malloced and must later be freed.
*/
PRIVATE any * WAIS_from_WWW ARGS2 (any *, docid, char *, docname)
{
  char *z; 	/* Output pointer */
  char *sor;	/* Start of record - points to size field. */
  char *p; 	/* Input pointer */
  char *q; 	/* Poisition of "=" */
  char *s; 	/* Position of semicolon */
  int n;	/* size */

#ifndef DISABLE_TRACE
  if (www2Trace) 
    fprintf(stderr, "WWW id (to become WAIS id): %s\n", docname); 
#endif
  for(n=0, p = docname; *p; p++) 
    {	/* Count sizes of strings */
      n++;
      if (*p == ';')  n--;		/* Not converted */
      else if (*p == HEX_ESCAPE) n=n-2;	/* Save two bytes */
      docid->size = n;
    }
  
  docid->bytes = (char *) malloc(docid->size + 32); /* result record */
  z = docid->bytes;
  
  for(p = docname; *p; ) 
    {
      q = strchr(p, '=');
      if (!q) 
        return 0;
      *q = '\0';
      *z++ = atoi(p);
      *q = '=';
      s = strchr(q, ';');	/* (Check only) */
      if (!s) 
        return 0;	/* Bad! No ';';	*/
      sor = z;          /* Remember where the size field was */
      z++;              /* Skip record size for now */
      
      {
        int len;
        int tmp;
	for(p=q+1; *p!=';' ; ) 
          {
            if (*p == HEX_ESCAPE) 
              {
                char c;
                unsigned int b;
		p++;
	        c = *p++;
		b = from_hex(c);
		c = *p++;
		if (!c) 
                  break;	/* Odd number of chars! */
		*z++ = (b<<4) + from_hex(c);
              } 
            else 
              {
	        *z++ = *p++;	/* Record */
              }
          }
        len = (z-sor-1);
        
        z = sor;
        if (len > 127)
          {
            tmp = (len / 128);
            len = len - (tmp * 128);
            tmp = tmp + 128;
            *z++ = (char)tmp;
            *z = (char)len;
          }
        else
          {
            *z = (char)len;
          }
        z++;
      }
      
      for(p=q+1; *p!=';' ; ) 
        {
          if (*p == HEX_ESCAPE) 
            {
              char c;
              unsigned int b;
              p++;
              c = *p++;
              b = from_hex(c);
              c = *p++;
              if (!c) 
                break;	/* Odd number of chars! */
              *z++ = (b<<4) + from_hex(c);
	    } 
          else 
            {
              *z++ = *p++;	/* Record */
	    }
	}
      p++;			/* After semicolon: start of next record */
    }
  
#ifndef DISABLE_TRACE
  if (www2Trace) 
    {
      char *p;
      fprintf(stderr, "WAIS id (%d bytes) is ", (int)docid->size);
      for(p=docid->bytes; p<docid->bytes+docid->size; p++) {
        if ((*p >= ' ') && (*p<= '~'))
          fprintf(stderr, "%c", *p);
        else
          fprintf(stderr, "<%x>", (unsigned)*p);
      }
      fprintf(stderr, "\n");
    }	 
#endif
  return docid;		/* Ok */
  
} /* WAIS_from_WWW */


/*	Send a plain text record to the client		output_text_record()
**	--------------------------------------
*/

PRIVATE void output_text_record ARGS3(
    HTStream *,			target,
    WAISDocumentText *,		record,
    boolean,			quote_string_quotes)
{
  if (record->DocumentText->size)
    {
      /* This cast should be unnecessary, as put_block should operate
         on unsigned char from the start.  What was he thinking??? */
      PUTBLOCK((unsigned char *)record->DocumentText->bytes,
               record->DocumentText->size);
    }    
} /* output text record */



/*	Format A Search response for the client		display_search_response
**	---------------------------------------
*/
/* modified from tracy shen's version in wutil.c
 * displays either a text record or a set of headlines.
 */
void
display_search_response ARGS4(
    HTStructured *,		target,
    SearchResponseAPDU *,	response,
    char *,			database,
    char *,	 		keywords)
{
  WAISSearchResponse  *info;
  long i, k;
  BOOL archie;

  if (!response)
    {
      PUTS("Arrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrgh!");
      return;
    }

  archie = strstr(database, "archie")!=0;	/* Specical handling */
  
#ifndef DISABLE_TRACE
  if (www2Trace) 
    fprintf(stderr, "HTWAIS: Displaying search response\n");
#endif
#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "HTWAIS: database 0x%08x '%s', response 0x%08x\n",
             database, database, response);
#endif
#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "HTWAIS: keywords 0x%08x '%s'\n", keywords, keywords);
#endif
  sprintf(line,
  	"Index %s contains the following %d item%s relevant to '%s'.\n",
	 database,
	 (int)(response->NumberOfRecordsReturned),
	 response->NumberOfRecordsReturned ==1 ? "" : "s",
	 keywords);

  PUTS(line);
  PUTS("The first figure for each entry is its relative score, ");
  PUTS("the second the number of lines in the item.");
  START(HTML_MENU);

  if ( response->DatabaseDiagnosticRecords != 0 ) {
    info = (WAISSearchResponse *)response->DatabaseDiagnosticRecords;
    i =0; 

    if (info->Diagnostics != NULL)
      showDiags((HTStream*)target, info->Diagnostics);

    if ( info->DocHeaders != 0 ) {
      for (k=0; info->DocHeaders[k] != 0; k++ ) {
	WAISDocumentHeader* head = info->DocHeaders[k];
	char * headline = trim_junk(head->Headline);
	any * docid = head->DocumentID;
	char * docname;			/* printable version of docid */
	i++;

/*	Make a printable string out of the document id.
*/
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr, 
		"HTWAIS:  %2ld: Score: %4ld, lines:%4ld '%s'\n", 
	       i,
	       (long int)(info->DocHeaders[k]->Score),
	       (long int)(info->DocHeaders[k]->Lines),
	       headline);
#endif
	START(HTML_LI);
	sprintf(line, "%4ld  %4ld  ",
	    head->Score,
	    head->Lines);
	PUTS( line);

	if (archie) {
	    char * www_name = WWW_from_archie(headline);
	    if (www_name) {
                PUTS ("<A HREF=\"");
                PUTS (www_name);
                PUTS ("\">");
		PUTS(headline);
                PUTS("</A>");
		free(www_name);
	    } else {
		 PUTS(headline);
		 PUTS(" (bad file name)");
	    }
	} else { /* Not archie */
	    docname =  WWW_from_WAIS(docid);
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf (stderr, "HTWAIS: docname '%s'\n", docname);
#endif
	    if (docname) {
		char * dbname = HTEscape(database);
                char types_array[1000]; /* bad */
                char *type_escaped;

                types_array[0] = 0;

                if (head->Types)
                  {
                    int i;
                    for (i = 0; head->Types[i]; i++)
                      {
                        if (i)
                          strcat (types_array, ",");

                        type_escaped = HTEscape (head->Types[i]);
                        strcat (types_array, type_escaped);
                        free (type_escaped);
                      }
#ifndef DISABLE_TRACE
                    if (www2Trace)
                      fprintf (stderr, "Built types_array '%s'\n", types_array);
#endif
                  }
                else
                  {
                    strcat (types_array, "TEXT");
                  }
                
		sprintf(line, "%s/%s/%d/%s",
                        dbname,
                        types_array,
                        (int)(head->DocumentLength),
                        docname);

                PUTS ("<A HREF=\"");
                if (head->Types && head->Types[0] && 
                    strcmp (head->Types[0], "URL") == 0)
                  {
                    /* The real URL, maybe? */
#ifndef DISABLE_TRACE
                    if (www2Trace)
                      fprintf (stderr, "HTWAIS: Using headline '%s' as URL\n",
                               headline);
#endif
                    PUTS(headline);
                  }
                else
                  {
                    /* Our manufactured URL. */
                    PUTS (line);
                  }
                PUTS ("\">");
		PUTS(headline);
                PUTS("</A>");
		free(dbname);
		free(docname);
	    } else {
		 PUTS("(bad doc id)");
	    }
	  }
      } /* next document header */
    } /* if there were any document headers */
    
    if ( info->ShortHeaders != 0 ) {
      k =0;
      while (info->ShortHeaders[k] != 0 ) {
	i++;
	PUTS( "(Short Header record, can't display)");
      }
    }
    if ( info->LongHeaders != 0 ) {
      k =0;
      while (info->LongHeaders[k] != 0) {
	i++;
	PUTS( "\nLong Header record, can't display\n");
      }
    }
    if ( info->Text != 0 ) {
      k =0;
      while (info->Text[k] != 0) {
	i++;
	PUTS( "\nText record\n");
	output_text_record((HTStream*)target, info->Text[k++], false);
      }
    }
    if ( info->Headlines != 0 ) {
      k =0;
      while (info->Headlines[k] != 0) {
	i++;
	PUTS( "\nHeadline record, can't display\n");
	/* dsply_headline_record( info->Headlines[k++]); */
      }
    }
    if ( info->Codes != 0 ) {
      k =0;
      while (info->Codes[k] != 0) {
	i++;
	PUTS( "\nCode record, can't display\n");
	/* dsply_code_record( info->Codes[k++]); */
      }
    }
  }				/* Loop: display user info */
  END(HTML_MENU);
  PUTC('\n'); ;
}


/* ------------------------------------------------------------------------ */
/* ---------------- Local copy of connect_to_server calls ----------------- */
/* ------------------------------------------------------------------------ */

/* Returns 1 on success, 0 on fail, -1 on interrupt. */
static int fd_mosaic_connect_to_server(char *host_name, long port, long *fd)
{
  /* New version. */
  char dummy[256];
  int status;

  sprintf (dummy, "wais://%s:%d/", host_name, port);

  status = HTDoConnect (dummy, "WAIS", 210, (int *)fd);
  if (status == HT_INTERRUPTED)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "===WAIS=== interrupted in connect\n");
#endif
      HTProgress ("Connection interrupted.");
      return -1;
    }
  if (status < 0)
    return 0;
  return 1;
}

/* Returns 1 on success, 0 on fail, -1 on interrupt. */
static int mosaic_connect_to_server(char *host_name, long port, FILE **fp)
{
  FILE* file;
  long fd;
  int rv;
  
  rv = fd_mosaic_connect_to_server (host_name, port, &fd);
  if(rv == 0) 
    {
      HTProgress ("Could not connect to WAIS server.");
      return 0;
    }
  else if (rv == -1)
    {
      HTProgress ("Connection interrupted.");
      return -1;
    }

  if ((file = fdopen(fd,"r+")) == NULL) 
    {
      HTProgress ("Could not open WAIS connection for reading.");
      return 0;
    }

  *fp = file;
  return 1;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
    
/* Pulled in from gui.c; corresponds to Rdata.max_wais_responses. */
extern int max_wais_responses;

#define MAX_KEYWORDS_LENGTH 5000
#define MAX_SERVER_LENGTH 1000
#define MAX_DATABASE_LENGTH 1000
#define MAX_SERVICE_LENGTH 1000
#define MAXDOCS max_wais_responses

/*		Load by name					HTLoadWAIS
**		============
**
**	This renders any object or search as required
*/
PUBLIC int HTLoadWAIS ARGS4(
	WWW_CONST char *,		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
  char *key;			  /* pointer to keywords in URL */
  char *request_message = NULL; /* arbitrary message limit */
  char *response_message = NULL; /* arbitrary message limit */
  long request_buffer_length;	/* how of the request is left */
  SearchResponseAPDU  *retrieval_response = 0;
  char keywords[MAX_KEYWORDS_LENGTH + 1];
  char *server_name;	
  char *wais_database = NULL;		/* name of current database */
  char *www_database;			/* Same name escaped */
  char *service;
  char *doctype;
  char *doclength;
  long document_length;
  char *docname;
  FILE *connection = 0;
  char *names;		/* Copy of arg to be hacked up */
  BOOL ok = NO;
  WAISSearchResponse *response;
  diagnosticRecord **diag;
  
  if (!acceptable_inited) 
    init_acceptable();
  
  /*	Decipher and check syntax of WWW address:
   **	----------------------------------------
   **
   **	First we remove the "wais:" if it was spcified.  920110
   */  
  names = HTParse(arg, "", PARSE_HOST | PARSE_PATH | PARSE_PUNCTUATION);
  key = strchr(names, '?');
  
  if (key) 
    {
      char * p;
      *key++ = 0;	/* Split off keywords */
      for (p=key; *p; p++) if (*p == '+') *p = ' ';
      HTUnEscape(key);
    }
  if (names[0]== '/') 
    {
      server_name = names+1;
      if (*server_name == '/')
        server_name++;	/* Accept one or two */
      www_database = strchr(server_name,'/');
      if (www_database) 
        {
          *www_database++ = 0;		/* Separate database name */
          doctype = strchr(www_database, '/');
          if (key) 
            ok = YES;	/* Don't need doc details */
          else if (doctype) 
            {	/* If not search parse doc details */
              *doctype++ = 0;	/* Separate rest of doc address */
              doclength = strchr(doctype, '/');
              if(doclength) 
                {
                  *doclength++ = 0;
                  
                  /* OK, now doclength should be the rest of the URL,
                     right??? */
#ifndef DISABLE_TRACE
                  if (www2Trace)
                    fprintf (stderr, 
                             "WAIS: doctype '%s', doclength\n~~~~\n%s\n~~~~\n",
                             doctype, doclength);
#endif
                  /* Multitype! */
                  if (strchr (doctype, ','))
                    {
                      HTStructured *target = 
                        HTML_new(anAnchor, format_out, sink);
                      char *t, *oldt, *newt, *revised;
                      int first;

#ifndef DISABLE_TRACE
                      if (www2Trace)
                        {
                          fprintf (stderr,
                                   "WAIS: Hey boss, got multitype.\n");
                          fprintf (stderr,
                                   "WAIS: names is '%s'\n", names);
                        }
#endif

                      START(HTML_TITLE);
                      PUTS("Multiple Format Opportunity");
                      END(HTML_TITLE);
                      
                      START(HTML_H1);
                      PUTS("Multiple Format Opportunity");
                      END(HTML_H1);

                      PUTS("This is a multiformat WAIS response.  You may pick the format of your choice from the list that follows: <p>\n\n<ul>\n");

                      /* Get the first doctype. */
                      t = strtok (doctype, ",");

                      /* oldt is a copy of the first doctype,
                         with leading period. */
                      oldt = (char *)malloc (strlen (t) + 16);
                      sprintf (oldt, ".%s", t);
                      
                      first = 1;

                      while (t && *t)
                        {
                          /* Got a type, as t. */
                          PUTS("<li> <a href=\"wais:");
                          PUTS(names);
                          PUTS("/");
                          PUTS(www_database);
                          PUTS("/");
                          PUTS(t);
                          PUTS("/");
                          PUTS(doclength);
                          PUTS("\">");
                          
                          /* Unescape t in place; we don't need it anymore
                             after this anyway. */
                          HTUnEscape (t);
                          PUTS(t);
                          PUTS("</a>\n");
                          t = strtok (NULL, ",");
                        }

                      free (oldt);
                      
                      PUTS("</ul>");
                      
                      END_TARGET;
                      FREE_TARGET;
                      
                      /* Hey, WE'RE DONE! */
                      free (names);
                      return HT_LOADED;
                    }
                  
                  document_length = atol(doclength);
                  if (document_length) 
                    {
                      docname=strchr(doclength, '/');
                      if (docname) 
                        {
                          *docname++ = 0;
                          ok = YES;	/* To avoid a goto! */
                        } /* if docname */
                    } /* if document_length valid */
                } /* if doclength */
            } 
          else 
            { /* no doctype?  Assume index required */
              if (!key) key = "";
              ok = YES;
            } /* if doctype */
        } /* if database */
    }
  
  if (!ok)
    {
      free (names);
      return HT_NOT_LOADED;
    }

#ifndef DISABLE_TRACE
  if (www2Trace) 
    fprintf(stderr, "HTWAIS: Parsed OK; type is '%s'\n", doctype);
#endif

  service = strchr(names, ':');
  if (service)  
    *service++ = 0;
  else 
    service = "210";
  
  if (server_name[0] == 0)
    connection = NULL;
  else if (!(key && !*key))
    {
      int status;
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "===WAIS=== calling mosaic_connect_to_server\n");
#endif
      status = mosaic_connect_to_server
        (server_name, atoi(service), &connection);
      if (status == 0)
        {
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "===WAIS=== connection failed\n");
#endif
          free(names);
          return HT_NOT_LOADED;
        }
      else if (status == -1)
        {
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "===WAIS=== connection interrupted\n");
#endif
          free(names);
          return HT_INTERRUPTED;
        }
    }

  StrAllocCopy(wais_database,www_database);
  HTUnEscape(wais_database);
  
  /* This below fixed size stuff is terrible */
  request_message = (char*)s_malloc((size_t)MAX_MESSAGE_LEN * sizeof(char));
  response_message = (char*)s_malloc((size_t)MAX_MESSAGE_LEN * sizeof(char));
  
  /*	If keyword search is performed but there are no keywords,
   **	the user has followed a link to the index itself. It would be
   **	appropriate at this point to send him the .SRC file - how?
   */
  if (key && !*key) 
    {				/* I N D E X */
      HTStructured * target = HTML_new(anAnchor, format_out, sink);
      
      START(HTML_TITLE);
      PUTS(wais_database);
      PUTS(" index");
      END(HTML_TITLE);
      
      START(HTML_H1);
      PUTS(wais_database);
      END(HTML_H1);

      START(HTML_ISINDEX);
      
      START(HTML_P);
      
      END_TARGET;
      if (connection) 
        FW_close_connection(connection);
      FREE_TARGET;
    } 
  else if (key) 
    {					/* S E A R C H */
      char *p;
      HTStructured * target;
      
      strncpy(keywords, key, MAX_KEYWORDS_LENGTH);
      while(p=strchr(keywords, '+')) 
        *p = ' ';
      
      /* Send advance title to get something fast to the other end */
      
      target = HTML_new(anAnchor, format_out, sink);
      
      START(HTML_TITLE);
      PUTS(keywords);
      PUTS(" (in ");
      PUTS(wais_database);
      PUTS(")");
      END(HTML_TITLE);
      
      START(HTML_H1);
      PUTS(keywords);
      END(HTML_H1);
      
      START(HTML_ISINDEX);
      
      request_buffer_length = MAX_MESSAGE_LEN; /* Amount left */
#ifndef DISABLE_TRACE
      if (www2Trace) fprintf(stderr, "HTWAIS: Search for `%s' in `%s'\n",
                         keywords, wais_database);
#endif
      
      if(NULL == generate_search_apdu(request_message + HEADER_LENGTH, 
                                      &request_buffer_length, 
                                      keywords, wais_database, NULL, MAXDOCS))
        {
          HTProgress ("WAIS request too large; something went wrong.");
        }
      
      if(!interpret_message(request_message, 
                            MAX_MESSAGE_LEN - request_buffer_length, 
                            response_message,
                            MAX_MESSAGE_LEN,
                            connection,
                            false	/* true verbose */
                            )) 
        {
          HTProgress ("WAIS returned message too large; something went wrong.");
        } 
      else 
        {	/* returned message ok */
          SearchResponseAPDU  *query_response = 0;
          readSearchResponseAPDU(&query_response,
                                 response_message + HEADER_LENGTH);
          /* We do want this to be called if !query_response, to
             get our cute error message. */
          display_search_response(target, 
                                  query_response, wais_database, keywords);
          if (query_response)
            {
              if (query_response->DatabaseDiagnosticRecords)
                freeWAISSearchResponse(query_response->DatabaseDiagnosticRecords);
              freeSearchResponseAPDU( query_response);
            }
        }	/* returned message not too large */
      
      END_TARGET;
      if (connection) 
        FW_close_connection(connection);
      FREE_TARGET;
    } 
  else 
    {			/* D O C U M E N T    F E T C H */
      HTFormat format_in;
      HTStream * target;
      long count;
      any   doc_chunk;
      any * docid = &doc_chunk;

      if (doctype)
        HTUnEscape (doctype);

#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf(stderr,
                "===WAIS=== Retrieve document id `%s' type `%s' length %ld\n",
                docname, doctype, document_length);
#endif      
      {
        char *tmp_doctype = strdup (doctype);
        char *tmp;
        for (tmp = tmp_doctype; *tmp; tmp++)
          *tmp = TOLOWER (*tmp);
#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf (stderr,
                   "===WAIS=== Doing HTAtom_exists on '%s'\n", tmp_doctype);
#endif
        format_in = HTAtom_exists (tmp_doctype);
        free (tmp_doctype);
      }
      if (!format_in)
        {
          char dummy[256];
          HTAtom *pencoding;
          int compressed;
          
          sprintf (dummy, "foo.%s", doctype);
          format_in = HTFileFormat (dummy, &pencoding,
                                    WWW_PLAINTEXT, &compressed);
          /* Assume it will always be at *least* WWW_PLAINTEXT. */
        }
      
      target = HTStreamStack(format_in, format_out, 0, sink, anAnchor);
      if (!target) 
        {
          free (names);
          if (connection) 
            FW_close_connection(connection);
          return HT_NOT_LOADED;
        }

      /*	Decode hex or literal format for document ID
       */	
      WAIS_from_WWW(docid, docname);
      
      /*	Loop over slices of the document
       */	
      {
        int bytes = 0, intr;
        char line[256];
        
        HTClearActiveIcon ();
        
        count = 0;
        while (1)
          {
            char *type = s_strdup(doctype);	/* Gets freed I guess */
            request_buffer_length = MAX_MESSAGE_LEN; /* Amount left */
#ifndef DISABLE_TRACE
            if (www2Trace) 
              fprintf(stderr, "HTWAIS: Slice number %ld\n", count);
#endif
            
            intr = HTCheckActiveIcon (1);
            if (intr)
              {
                HTProgress ("Data transfer interrupted.");
                (*target->isa->handle_interrupt)(target);
                free (names);
                if (connection) 
                  FW_close_connection(connection);
                return HT_INTERRUPTED;
              }
            
            if(generate_retrieval_apdu
               (request_message + HEADER_LENGTH,
                &request_buffer_length, 
                docid, 
                CT_byte,
                count * CHARS_PER_PAGE,
                (count + 1) * CHARS_PER_PAGE,
                type,
                wais_database
                ) == 0)
              {
                HTProgress 
                  ("WAIS error condition; retrieval may be unsuccessful.");
              }

            free (type);
            
            /*	Actually do the transaction given by request_message */   
            if(0 ==
               interpret_message
               (request_message, 
                MAX_MESSAGE_LEN - request_buffer_length, 
                response_message,
                MAX_MESSAGE_LEN,
                connection,
                false /* true verbose */	
                ))
              {
                HTProgress ("WAIS error condition; retrieval may be unsuccessful.");
                goto no_more_data;
              }
            
            /* 	Parse the result which came back into memory.
             */
            readSearchResponseAPDU(&retrieval_response, 
                                   response_message + HEADER_LENGTH);
            
            response = 
              (WAISSearchResponse *)retrieval_response->DatabaseDiagnosticRecords;
            diag = response->Diagnostics;

            if(NULL == response->Text)
              {
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr, "WAIS: no more data (NULL response->Text)\n");
#endif
                if (retrieval_response->DatabaseDiagnosticRecords)
                  freeWAISSearchResponse 
                    (retrieval_response->DatabaseDiagnosticRecords);
                freeSearchResponseAPDU (retrieval_response);
                goto no_more_data;
              } 
            else if
              (((WAISSearchResponse *)
                retrieval_response->DatabaseDiagnosticRecords)->Text[0]->DocumentText->size)
              {
                output_text_record
                  (target,
                   ((WAISSearchResponse *)
                    retrieval_response->DatabaseDiagnosticRecords)->Text[0],
                   false);
              } /* If text existed */
            else
              {
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr, "WAIS: no more data (fell through)\n");
#endif
                if (retrieval_response->DatabaseDiagnosticRecords)
                  freeWAISSearchResponse 
                    (retrieval_response->DatabaseDiagnosticRecords);
                freeSearchResponseAPDU (retrieval_response);
                goto no_more_data;
              }

            /* Slightly inaccurate for last slice. */
            bytes += CHARS_PER_PAGE;
            sprintf (line, "Read %d bytes of data.", bytes);
            HTProgress (line);

            if (diag &&
                diag[0] &&
                diag[0]->ADDINFO != NULL &&
                !strcmp(diag[0]->DIAG, D_PresentRequestOutOfRange))
              {
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr, "WAIS: no more data (diag)\n");
#endif
                if (retrieval_response->DatabaseDiagnosticRecords)
                  freeWAISSearchResponse 
                    (retrieval_response->DatabaseDiagnosticRecords);
                freeSearchResponseAPDU (retrieval_response);
                goto no_more_data;
              }

            if (retrieval_response->DatabaseDiagnosticRecords)
              freeWAISSearchResponse 
                (retrieval_response->DatabaseDiagnosticRecords);
            freeSearchResponseAPDU (retrieval_response);

            count++;
          }	/* Loop over slices */
        
      } /* local variables */

    no_more_data:
      
      END_TARGET;
      /* Close the connection BEFORE calling system(), which can
         happen in the free method. */
      if (connection) 
        FW_close_connection(connection);
      FREE_TARGET;
      
      free (docid->bytes);
    } /* If document rather than search */
  
  if (wais_database) 
    free (wais_database);
  s_free (request_message);
  s_free(response_message);
  
  free(names);
  return HT_LOADED;
}

PUBLIC HTProtocol HTWAIS = { "wais", HTLoadWAIS, NULL };

#endif /* DIRECT_WAIS */
