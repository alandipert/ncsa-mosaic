/*			GOPHER ACCESS				HTGopher.c
**			=============
**
** History:
**	26 Sep 90	Adapted from other accesses (News, HTTP) TBL
**	29 Nov 91	Downgraded to C, for portable implementation.
*/
#include "../config.h"
/* Implements:
*/
#include "HTGopher.h"


#define GOPHER_PORT 70		/* See protocol spec */
#define BIG 1024		/* Bug */
#define LINE_LENGTH 256		/* Bug */

/*	Gopher entity types:
*/
#define GOPHER_TEXT		'0'
#define GOPHER_MENU		'1'
#define GOPHER_CSO		'2'
#define GOPHER_ERROR		'3'
#define GOPHER_MACBINHEX	'4'
#define GOPHER_PCBINHEX		'5'
#define GOPHER_UUENCODED	'6'
#define GOPHER_INDEX		'7'
#define GOPHER_TELNET		'8'
#define GOPHER_BINARY           '9'
#define GOPHER_DUPLICATE	'+'

#define GOPHER_GIF              'g'
#define GOPHER_IMAGE            'I'
#define GOPHER_TN3270           'T'

#define GOPHER_HTML		'h'		/* HTML */
#define GOPHER_WWW		'w'		/* W3 address */
#define GOPHER_SOUND            's'

#define GOPHER_PLUS_IMAGE       ':'
#define GOPHER_PLUS_MOVIE       ';'
#define GOPHER_PLUS_SOUND       '<'

#define GOPHER_INFO				'i'

#include <ctype.h>
#include "HTUtils.h"		/* Coding convention macros */
#include "tcp.h"

#include "HTAlert.h"
#include "HTParse.h"
#include "HTFormat.h"
#include "HTFile.h"
#include "HTTCP.h"

/*		Hypertext object building machinery
*/
#include "HTML.h"

#define PUTC(c) (*targetClass.put_character)(target, c)
#define PUTS(s) (*targetClass.put_string)(target, s)
#define START(e) (*targetClass.start_element)(target, e, 0, 0)
#define END(e) (*targetClass.end_element)(target, e)
#define END_TARGET (*targetClass.end_document)(target)
#define FREE_TARGET (*targetClass.free)(target)
struct _HTStructured {
	WWW_CONST HTStructuredClass *	isa;
	/* ... */
};

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

PRIVATE HTStructured *target;			/* the new hypertext */
PRIVATE HTStructuredClass targetClass;		/* Its action routines */


/*	Module-wide variables
*/
PRIVATE int s;					/* Socket for GopherHost */


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

PRIVATE WWW_CONST char hex[17] = "0123456789abcdef";


/*	Paste in an Anchor
**	------------------
**
**	The title of the destination is set, as there is no way
**	of knowing what the title is when we arrive.
**
** On entry,
**	HT 	is in append mode.
**	text 	points to the text to be put into the file, 0 terminated.
**	addr	points to the hypertext refernce address 0 terminated.
*/
PRIVATE void write_anchor ARGS3(WWW_CONST char *,text, WWW_CONST char *,addr,
                                char *, image_text)
{
    PUTS ("<A HREF=\"");
    PUTS (addr);
    PUTS ("\">");

    /* Throw in an inlined image, if one has been requested. */
    if (image_text)
      {
        PUTS ("<IMG SRC=\"");
        PUTS (image_text);
        PUTS ("\"> ");
      }

    PUTS(text);
    PUTS("</A>");
}


PRIVATE void write_non_anchor ARGS2(WWW_CONST char *,text,
                                char *, image_text)
{
    /* Throw in an inlined image, if one has been requested. */
    if (image_text)
      {
        PUTS ("<IMG SRC=\"");
        PUTS (image_text);
        PUTS ("\"> ");
      }

    PUTS(text);
}

/*	Parse a Gopher Menu document
**	============================
**
*/

PRIVATE int parse_menu ARGS2 (
	WWW_CONST char *,		arg,
	HTParentAnchor *,	anAnchor)
{
  char gtype;
  char ch;
  char line[BIG];
  char address[BIG];
  char *name, *selector = NULL;		/* Gopher menu fields */
  char *host;
  char *port;
  char *p = line;
  extern int interrupted_in_htgetcharacter;

#define TAB 		'\t'
#define HEX_ESCAPE 	'%'

  HTProgress ("Retrieving Gopher menu.");

  PUTS("<H1>Gopher Menu</H1>\n");

  START(HTML_DL);
  while ((ch=HTGetCharacter ()) != (char)EOF)
    {
      if (interrupted_in_htgetcharacter)
        {
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "parse_menu: picked up interrupt in htgc\n");
#endif
          (*targetClass.handle_interrupt)(target);
          return HT_INTERRUPTED;
        }
      if (ch != LF)
        {
          *p = ch;		/* Put character in line */
          if (p< &line[BIG-1]) p++;
        }
      else
        {
          *p++ = 0;		/* Terminate line */
          p = line;		/* Scan it to parse it */
          port = 0;		/* Flag "not parsed" */
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf(stderr, "HTGopher: Menu item: %s\n", line);
#endif
          gtype = *p++;

          /* Break on line with a dot by itself */
          if ((gtype=='.') && ((*p=='\r') || (*p==0)))
            break;

          if (gtype && *p)
            {
              name = p;
              selector = strchr(name, TAB);
              START(HTML_DD);
              if (selector)
                {
                  *selector++ = 0;	/* Terminate name */
                  host = strchr(selector, TAB);
                  if (host)
                    {
                      *host++ = 0;	/* Terminate selector */
                      port = strchr(host, TAB);
                      if (port)
                        {
                          char *junk;
                          port[0] = ':';	/* delimit host a la W3 */
                          junk = strchr(port, TAB);
                          if (junk)
                            *junk++ = 0;	/* Chop port */
                          if ((port[1]=='0') && (!port[2]))
                            port[0] = 0;	/* 0 means none */
			} /* no port */
		    } /* host ok */
		} /* selector ok */
	    } /* gtype and name ok */

          if (gtype == GOPHER_WWW)
            {	/* Gopher pointer to W3 */
              write_anchor(name, selector, "internal-gopher-text");
	    }
          else if (port)
            {		/* Other types need port */
              if (gtype == GOPHER_TELNET)
                {
                  if (*selector)
                    sprintf(address, "telnet://%s@%s/",
                            selector, host);
                  else
                    sprintf(address, "telnet://%s/", host);
                }
              else if (gtype == GOPHER_TN3270)
                {
                  if (*selector)
                    sprintf(address, "tn3270://%s@%s/",
                            selector, host);
                  else
                    sprintf(address, "tn3270://%s/", host);
                }
              else
                {			/* If parsed ok */
                  char *q;
                  unsigned char *p;
                  sprintf(address, "//%s/%c", host, gtype);
                  q = address+ strlen(address);
                  for(p=(unsigned char *)selector; *p; p++)
                    {	/* Encode selector string */
                      if (acceptable[*p]) *q++ = *p;
                      else
                        {
                          *q++ = HEX_ESCAPE;	/* Means hex coming */
                          *q++ = hex[(*p) >> 4];
                          *q++ = hex[(*p) & 15];
			}
		    }
                  *q++ = 0;			/* terminate address */
		}
              /* Error response from Gopher doesn't deserve to
                 be a hyperlink. */
              if (strcmp (address, "//error.host:1/0") != 0 &&
                  strcmp (address, "//error/0error") != 0 &&
                  strcmp (address, "//:/0") != 0 &&
                  gtype != GOPHER_ERROR)
                {
                  switch (gtype)
                    {
                    case GOPHER_MENU:
                      write_anchor(name, address, "internal-gopher-menu");
                      break;
                    case GOPHER_TEXT:
                      write_anchor(name, address, "internal-gopher-text");
                      break;
                    case GOPHER_INDEX:
                    case GOPHER_CSO:
                      write_anchor(name, address, "internal-gopher-index");
                      break;
                    case GOPHER_IMAGE:
                    case GOPHER_GIF:
                    case GOPHER_PLUS_IMAGE:
                      write_anchor(name, address, "internal-gopher-image");
                      break;
                    case GOPHER_SOUND:
                    case GOPHER_PLUS_SOUND:
                      write_anchor(name, address, "internal-gopher-sound");
                      break;
                    case GOPHER_PLUS_MOVIE:
                      write_anchor(name, address, "internal-gopher-movie");
                      break;
                    case GOPHER_TELNET:
                    case GOPHER_TN3270:
                      write_anchor(name, address, "internal-gopher-telnet");
                      break;
                    case GOPHER_BINARY:
                    case GOPHER_MACBINHEX:
                    case GOPHER_PCBINHEX:
                    case GOPHER_UUENCODED:
                      write_anchor(name, address, "internal-gopher-binary");
                      break;
					case GOPHER_INFO:
						write_non_anchor(name, NULL);
						break;
                    default:
                      write_anchor(name, address, "internal-gopher-unknown");
                      break;
                    }
                }
              else
                {
                  /* Good error handling??? */
                  PUTS(line);
                }
	    }
          else
            { /* parse error */
#ifndef DISABLE_TRACE
              if (www2Trace) fprintf(stderr,
                                 "HTGopher: Bad menu item.\n");
#endif
              PUTS(line);
	    } /* parse error */
          p = line;	/* Start again at beginning of line */
        } /* if end of line */
    } /* Loop over characters */
  if (interrupted_in_htgetcharacter)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "parse_menu: picked up interrupt in htgc\n");
#endif
      (*targetClass.handle_interrupt)(target);
      return HT_INTERRUPTED;
    }

  END(HTML_DL);
  END_TARGET;
  FREE_TARGET;

  HTProgress ("Retrieved Gopher menu.");

  return 1;
}

/*	Display a Gopher Index document
**	-------------------------------
*/

PRIVATE void display_index ARGS2 (
	WWW_CONST char *,	arg,
	HTParentAnchor *,anAnchor)
{
  PUTS("<H1>Searchable Gopher Index</H1> <ISINDEX>");

  END_TARGET;
  FREE_TARGET;
  return;
}


/*	Display a Gopher CSO document
**	-----------------------------
*/

PRIVATE void display_cso ARGS2 (
	WWW_CONST char *,	arg,
	HTParentAnchor *,anAnchor)
{
  PUTS("<H1>Searchable CSO Phonebook</H1> <ISINDEX>");

  END_TARGET;
  FREE_TARGET;
  return;
}


/*	Parse a Gopher CSO document
 **	============================
 **
 **   Accepts an open socket to a CSO server waiting to send us
 **   data and puts it on the screen in a reasonable manner.
 **
 **   Perhaps this data can be automatically linked to some
 **   other source as well???
 **
 **   Hacked into place by Lou Montulli@ukanaix.cc.ukans.edu
 **
 */
PRIVATE int parse_cso ARGS2 (WWW_CONST char *,	arg,
                             HTParentAnchor *,anAnchor)
{
  char ch;
  char line[BIG];
  char *p = line;
  char *second_colon, last_char='\0';
  extern int interrupted_in_htgetcharacter;

  HTProgress ("Retrieving CSO search results.");

  PUTS("<H1>CSO Search Results</H1>\n<PRE>");

  /* start grabbing chars from the network */
  while ((ch=HTGetCharacter ()) != (char)EOF)
    {
      if (interrupted_in_htgetcharacter)
        {
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "parse_cso: picked up interrupt in htgc\n");
#endif
          (*targetClass.handle_interrupt)(target);
          return HT_INTERRUPTED;
        }
      if (ch != '\n')
        {
          *p = ch;		/* Put character in line */
          if (p< &line[BIG-1]) p++;
        }
      else
        {
          *p++ = 0;		/* Terminate line */
          p = line;		/* Scan it to parse it */

	/* OK we now have a line in 'p' lets parse it and print it */

          /* Break on line that begins with a 2. It's the end of
           * data.
	   */
          if (*p == '2')
		break;

	  /*  lines beginning with 5 are errors,
	   *  print them and quit
	   */
          if (*p == '5') {
            START(HTML_H2);
            PUTS(p+4);
            END(HTML_H2);
            break;
          }

	  if(*p == '-') {
	     /*  data lines look like  -200:#:
              *  where # is the search result number and can be multiple
	      *  digits (infinate?)
              *  find the second colon and check the digit to the
              *  left of it to see if they are diferent
              *  if they are then a different person is starting.
	      *  make this line an <h2>
              */

		/* find the second_colon */
             second_colon = strchr( strchr(p,':')+1, ':');

             if(second_colon != NULL) {  /* error check */

                 if (*(second_colon-1) != last_char)   /* print seperator */
                   {
                     END(HTML_PRE);
                     START(HTML_H2);
                   }

		 /* right now the record appears with the alias (first line)
		  * as the header and the rest as <pre> text
		  * It might look better with the name as the
		  * header and the rest as a <ul> with <li> tags
		  * I'm not sure whether the name field comes in any
		  * special order or if its even required in a record,
		  * so for now the first line is the header no matter
		  * what it is (it's almost always the alias)
		  * A <dl> with the first line as the <DT> and
		  * the rest as some form of <DD> might good also?
		  */

                 /* print data */
                 PUTS(second_colon+1);
                 PUTS("\n");

                 if (*(second_colon-1) != last_char)   /* end seperator */
                   {
                     END(HTML_H2);
                     START(HTML_PRE);
                   }

		  /* save the char before the second colon
		   * for comparison on the next pass
		   */
                 last_char =  *(second_colon-1) ;

	     } /* end if second_colon */
	  } /* end if *p == '-' */
        } /* if end of line */

    } /* Loop over characters */
  if (interrupted_in_htgetcharacter)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "parse_cso: picked up interrupt in htgc\n");
#endif
      (*targetClass.handle_interrupt)(target);
      return HT_INTERRUPTED;
    }

  /* end the text block */
  PUTS("\n<PRE>");
  END_TARGET;
  FREE_TARGET;

  HTProgress ("Retrieved CSO search results.");

  return 1;  /* all done */
} /* end of procedure */



/*		De-escape a selector into a command
**		-----------------------------------
**
**	The % hex escapes are converted. Otheriwse, the string is copied.
*/
PRIVATE void de_escape ARGS2(char *, command, WWW_CONST char *, selector)
{
  char *p;

  if (!selector)
    return;
  if (!command)
    return;

  p = strdup (selector);
  HTUnEscape (p);

  strcpy (command, p);

  free (p);

#if 0
  for (p = command; *p; p++)
    if (*p == '+')
      *p = ' ';
#endif

  return;
}


/*		Load by name					HTLoadGopher
**		============
**
**	 Bug:	No decoding of strange data types as yet.
**
*/
PUBLIC int HTLoadGopher ARGS4(
	char *,		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
  char *command;			/* The whole command */
  int status;				/* tcp return */
  char gtype;				/* Gopher Node type */
  char * selector;			/* Selector string */
  int rv = 0;

  if (!acceptable_inited) init_acceptable();

  if (!arg)
    return -3;		/* Bad if no name sepcified	*/
  if (!*arg)
    return -2;		/* Bad if name had zero length	*/

#ifndef DISABLE_TRACE
  if (www2Trace) fprintf(stderr, "HTGopher: Looking for %s\n", arg);
#endif

  /* Get entity type, and selector string.
   */
  {
    char * p1 = HTParse(arg, "", PARSE_PATH|PARSE_PUNCTUATION);
    gtype = '1';		/* Default = menu */
    selector = p1;
    if ((*selector++=='/') && (*selector))
      {	/* Skip first slash */
        gtype = *selector++;			/* Pick up gtype */
      }
    if (gtype == GOPHER_INDEX)
      {
        char * query;
        query = strchr(selector, '?');	/* Look for search string */
        if (!query || !query[1])
          {		/* No search required */
            target = HTML_new(anAnchor, format_out, sink);
            targetClass = *target->isa;
            display_index(arg, anAnchor);	/* Display "cover page" */
            return HT_LOADED;			/* Local function only */
          }
        *query++ = 0;			/* Skip '?' 	*/
        HTUnEscape (query);
        command = malloc(strlen(selector)+ 1 + strlen(query)+ 2 + 1);

        de_escape(command, selector);

        strcat(command, "\t");
        strcat(command, query);
      }
    else if (gtype == GOPHER_CSO)
      {
        char * query;
        query = strchr(selector, '?');      /* Look for search string */
        if (!query || !query[1])
          {          /* No search required */
            target = HTML_new(anAnchor, format_out, sink);
            targetClass = *target->isa;
            display_cso(arg, anAnchor);     /* Display "cover page" */
            return HT_LOADED;                       /* Local function only */
          }
        *query++ = 0;                       /* Skip '?'     */
        HTUnEscape (query);
        command = malloc(strlen("query")+ 1 + strlen(query)+ 2 + 1);

        de_escape(command, selector);

        strcpy(command, "query ");
        strcat(command, query);
      }
    else
      {				/* Not index */
        command = malloc(strlen(selector)+2+1);
        de_escape(command, selector);
      }
    free(p1);
  }

  /* Patch security hole. */
  {
    char *tmp;
    for (tmp = command; *tmp; tmp++)
      if (*tmp == CR || *tmp == LF)
        *tmp = ' ';
#ifndef DISABLE_TRACE
    if (www2Trace)
      fprintf (stderr, "Fixed security hole: '%s'\n", command);
#endif
    *tmp++ = CR;
    *tmp++ = LF;
    *tmp++ = 0;
#ifndef DISABLE_TRACE
    if (www2Trace)
      fprintf (stderr, "Prepared command: '%s'\n", command);
#endif
  }

  status = HTDoConnect (arg, "Gopher", 70, &s);
  if (status == HT_INTERRUPTED)
    {
      /* Interrupt cleanly. */
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr,
                 "Gopher: Interrupted on connect; recovering cleanly.\n");
#endif
      HTProgress ("Connection interrupted.");
      return HT_INTERRUPTED;
    }
  if (status<0)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf(stderr,
                "HTTPAccess: Unable to connect to remote host for `%s'.\n",
                arg);
#endif
      free(command);
      return HT_NOT_LOADED;
    }

  HTInitInput(s);		/* Set up input buffering */

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf(stderr,
            "HTGopher: Connected, writing command `%s' to socket %d\n",
            command, s);
#endif

  status = NETWRITE(s, command, (int)strlen(command));
  free(command);
  if (status<0)
    {
#ifndef DISABLE_TRACE
      if (www2Trace) fprintf(stderr, "HTGopher: Unable to send command.\n");
#endif
      NETCLOSE (s);
      return HT_NOT_LOADED;
    }

  /* Now read the data from the socket: */
  switch (gtype)
    {
      int compressed;
      HTAtom *enc;
      extern int tweak_gopher_types;

    case GOPHER_MENU:
    case GOPHER_INDEX:
      target = HTML_new(anAnchor, format_out, sink);
      targetClass = *target->isa;
      rv = parse_menu(arg, anAnchor);
      break;

    case GOPHER_CSO:
      target = HTML_new(anAnchor, format_out, sink);
      targetClass = *target->isa;
      rv = parse_cso(arg, anAnchor);
      break;

    case GOPHER_MACBINHEX:
    case GOPHER_PCBINHEX:
    case GOPHER_UUENCODED:
    case GOPHER_BINARY:
      if (!tweak_gopher_types)
        rv = HTParseSocket(WWW_BINARY, format_out, anAnchor, s, sink, 0);
      else
        rv = HTParseSocket(HTFileFormat (arg, &enc, WWW_BINARY, &compressed),
                           format_out, anAnchor, s, sink, 0);
      break;

    case GOPHER_GIF:
    case GOPHER_IMAGE:
    case GOPHER_PLUS_IMAGE:
      if (!tweak_gopher_types)
        rv = HTParseSocket(HTAtom_for ("image/gif"),
                           format_out, anAnchor, s, sink, 0);
      else
        rv = HTParseSocket(HTFileFormat (arg, &enc, HTAtom_for ("image/gif"),
                                         &compressed),
                           format_out, anAnchor, s, sink, 0);
      break;

    case GOPHER_SOUND:
    case GOPHER_PLUS_SOUND:
      if (!tweak_gopher_types)
        rv = HTParseSocket(HTAtom_for ("audio/basic"),
                           format_out, anAnchor, s, sink, 0);
      else
        rv = HTParseSocket(HTFileFormat (arg, &enc,
                                         HTAtom_for ("audio/basic"),
                                         &compressed),
                           format_out, anAnchor, s, sink, 0);
      break;

    case GOPHER_PLUS_MOVIE:
      /* Sigh..... */
      if (!tweak_gopher_types)
        rv = HTParseSocket(HTAtom_for ("video/mpeg"),
                           format_out, anAnchor, s, sink, 0);
      else
        rv = HTParseSocket(HTFileFormat (arg, &enc,
                                         HTAtom_for ("video/mpeg"),
                                         &compressed),
                           format_out, anAnchor, s, sink, 0);
      break;

    case GOPHER_HTML:
      if (!tweak_gopher_types)
        rv = HTParseSocket(WWW_HTML, format_out, anAnchor, s, sink, 0);
      else
        rv = HTParseSocket(HTFileFormat (arg, &enc, WWW_HTML, &compressed),
                           format_out, anAnchor, s, sink, 0);
      break;

    case GOPHER_TEXT:
    default:			/* @@ parse as plain text */
      if (!tweak_gopher_types)
        rv = HTParseSocket(WWW_PLAINTEXT, format_out, anAnchor, s, sink, 0);
      else
        rv = HTParseSocket
          (HTFileFormat (arg, &enc, WWW_PLAINTEXT, &compressed),
           format_out, anAnchor, s, sink, 0);
      break;
    } /* switch(gtype) */

  NETCLOSE(s);
  if (rv == HT_INTERRUPTED)
    {
      HTProgress ("Connection interrupted.");
      return HT_INTERRUPTED;
    }
  else
    {
      return HT_LOADED;
    }
}

PUBLIC HTProtocol HTGopher = { "gopher", HTLoadGopher, NULL };
