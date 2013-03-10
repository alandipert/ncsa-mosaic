/*			MIME Message Parse			HTMIME.c
**			==================
**
**	This is RFC 1341-specific code.
**	The input stream pushed into this parser is assumed to be
**	stripped on CRs, ie lines end with LF, not CR LF.
**	(It is easy to change this except for the body part where
**	conversion can be slow.)
**
** History:
**	   Feb 92	Written Tim Berners-Lee, CERN
**
*/
#include "../config.h"
#include "HTMIME.h"		/* Implemented here */
#include "HTAlert.h"
#include "HTFile.h"
#include "tcp.h"
#include "../libnut/str-tools.h"
#if defined(KRB4) || defined(KRB5)              /* ADC, 6/28/95 */
#define HAVE_KERBEROS
#endif

/*SWP*/
#include "HTAAUtil.h"
extern int securityType;

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/* This is UGLY. */
char *redirecting_url = NULL;

/* This is almost as ugly. */
extern int loading_length;
extern int noLength;

/* As is this - AF */
char  *HTTP_last_modified;
char  *HTTP_expires;

/*		MIME Object
**		-----------
*/
typedef enum _MIME_state
{
  BEGINNING_OF_LINE,
  CONTENT_,
  CONTENT_T,
  CONTENT_TRANSFER_ENCODING,
  CONTENT_TYPE,
  CONTENT_ENCODING,
  CONTENT_LENGTH,
  EXPIRES,
  E,
  EX,
  L,
  LOCATION,
  LAST_MODIFIED,
  EXTENSION,
  SKIP_GET_VALUE,		/* Skip space then get value */
  GET_VALUE,		        /* Get value till white space */
  JUNK_LINE,		        /* Ignore the rest of this folded line */
  NEWLINE,		        /* Just found a LF .. maybe continuation */
  CHECK,			/* check against check_pointer */
  MIME_TRANSPARENT,	        /* put straight through to target ASAP! */
  MIME_IGNORE,		        /* ignore entire file */
  /* TRANSPARENT and IGNORE are defined as stg else in _WINDOWS */
#ifdef HAVE_KERBEROS
  WWW_AUTHENTICATE,             /* for kerberos mutual authentication */
#endif
} MIME_state;

#define VALUE_SIZE 8192		/* @@@@@@@ Arbitrary? */
struct _HTStream
{
  WWW_CONST HTStreamClass *	isa;

  MIME_state		state;		/* current state */
  MIME_state		if_ok;		/* got this state if match */
  MIME_state		field;		/* remember which field */
  MIME_state		fold_state;	/* state on a fold */
  WWW_CONST char *		check_pointer;	/* checking input */

  char *		value_pointer;	/* storing values */
  char 			value[VALUE_SIZE];

  HTParentAnchor *	anchor;		/* Given on creation */
  HTStream *		sink;		/* Given on creation */

  char *	        boundary;	/* For multipart */

  HTFormat		encoding;	/* Content-Transfer-Encoding */
  char *                compression_encoding;
  int                   content_length;
  int                   header_length;  /* for io accounting -bjs */
  HTFormat		format;		/* Content-Type */
  HTStream *		target;		/* While writing out */
  HTStreamClass		targetClass;

  HTAtom *		targetRep;	/* Converting into? */

  char *                location;
  char *		expires;
  char *		last_modified;

  int interrupted;
};


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
**
**	This is a FSM parser which is tolerant as it can be of all
**	syntax errors.  It ignores field names it does not understand,
**	and resynchronises on line beginnings.
*/

PRIVATE void HTMIME_put_character ARGS2(HTStream *, me, char, c)
{
#ifdef HAVE_KERBEROS
    static int got_kerb = 0;
    static HTAAScheme kscheme;
    extern int validate_kerberos_server_auth();
#endif


    if(me->state==MIME_TRANSPARENT){
	(*me->targetClass.put_character)(me->target, c);    /* MUST BE FAST */
	return;
    } else {
	me->header_length ++; /* bjs - update this first */
    }

  switch(me->state)
    {
    case MIME_IGNORE:
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTMIME_put_character] Got MIME_IGNORE; returning...\n");
#endif
      return;

/*    case MIME_TRANSPARENT:*/

    case NEWLINE:
      if (c != '\n' && WHITE(c))
        {
          /* Folded line */
          me->state = me->fold_state;	/* pop state before newline */
          break;
        }
      /* else Falls through */

    case BEGINNING_OF_LINE:
      switch(c)
        {
        case 'c':
        case 'C':
          me->check_pointer = "ontent-";
          me->if_ok = CONTENT_;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr,
                     "[MIME] Got C at beginning of line; checking for 'ontent-'\n");
#endif
          break;

	/* SWP -- 7/10/95 */
	case 'E':	/* Extension or Expires */
	case 'e':
          {
		me->state = E;
#ifndef DISABLE_TRACE
		if (www2Trace)
		fprintf (stderr,
			 "[MIME] Got E at beginning of line; checking for 'X'\n");
#endif
	  }
	  break;
        case 'l':
        case 'L':
          me->state = L;
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr,
                     "[MIME] Got L at beginning of line\n");
#endif
          break;

#ifdef HAVE_KERBEROS
          /*  for kerberos mutual authentication  */
        case 'w':
        case 'W':
          me->check_pointer = "ww-authenticate:";
          me->if_ok = WWW_AUTHENTICATE;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr,
                  "[MIME] Got W at beginning of line; checking for 'ww-authenticate'\n");
#endif
          break;
#endif

        case '\n':			/* Blank line: End of Header! */
          {
            int compressed = COMPRESSED_NOT;
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf (stderr,
                       "HTMIME: DOING STREAMSTACK: MIME content type is %s, converting to %s\n",
                       HTAtom_name(me->format), HTAtom_name(me->targetRep));
#endif
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf (stderr,
                       "                           Compression encoding '%s'\n",
                       (!me->compression_encoding || !*me->compression_encoding?"Undefined":me->compression_encoding));
#endif
            if (me->compression_encoding)
              {
                if (strcmp (me->compression_encoding, "x-compress") == 0)
                  {
                    compressed = COMPRESSED_BIGZ;
                  }
                else if (strcmp (me->compression_encoding, "x-gzip") == 0)
                  {
                    compressed = COMPRESSED_GNUZIP;
                  }
                else
                  {
#ifndef DISABLE_TRACE
                    if (www2Trace)
                      fprintf (stderr, "HTMIME: Unknown compression_encoding '%s'\n",
                               me->compression_encoding);
#endif
                  }
              }

#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf (stderr, "HTMIME: compressed == %d\n", compressed);
#endif
            me->target = HTStreamStack(me->format, me->targetRep, compressed,
                                       me->sink, me->anchor);
            if (!me->target)
              {
#ifndef DISABLE_TRACE
                if (www2Trace)
                  {
                    fprintf(stderr, "MIME: Can't translate! ** \n");
                    fprintf(stderr, "HTMIME: Defaulting to HTML.\n");
                  }
#endif
                /* Default to HTML. */
                me->target = HTStreamStack(HTAtom_for("text/html"),
                                           me->targetRep,
                                           compressed,
                                           me->sink,
                                           me->anchor);
              }
            if (me->target)
              {
                me->targetClass = *me->target->isa;
		/* Check for encoding and select state from there @@ */
                /* From now push straigh through */
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr, "[MIME] Entering MIME_TRANSPARENT\n");
#endif
                me->state = MIME_TRANSPARENT;
		/* bjs note: header is now completely read */

              }
            else
              {
                /* This is HIGHLY EVIL -- the browser WILL BREAK
                   if it ever reaches here.  Thus the default to
                   HTML above, which should always happen... */
#ifndef DISABLE_TRACE
                if (www2Trace)
                  fprintf (stderr, "MIME: HIT HIGHLY EVIL!!! ***\n");
#endif
                me->state = MIME_IGNORE;		/* What else to do? */
              }
          }
          break;

	default:
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "[MIME] Got nothing at beginning of line; bleah.\n");
#endif
          goto bad_field_name;
          break;

	} /* switch on character */
      break;

    case CHECK:				/* Check against string */
      if (TOLOWER(c) == *(me->check_pointer)++)
        {
          if (!*me->check_pointer)
            me->state = me->if_ok;
        }
      else
        {		/* Error */
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf(stderr,
                    "HTMIME: Bad character `%c' found where `%s' expected\n",
                    c, me->check_pointer - 1);
#endif
          goto bad_field_name;
        }
      break;

    case CONTENT_:
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr,
                 "[MIME] in case CONTENT_\n");
#endif
      switch(c)
        {
	case 't':
	case 'T':
          me->state = CONTENT_T;
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr,
                     "[MIME] Was CONTENT_, found T, state now CONTENT_T\n");
#endif
          break;

	case 'e':
	case 'E':
          me->check_pointer = "ncoding:";
          me->if_ok = CONTENT_ENCODING;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr,
                     "[MIME] Was CONTENT_, found E, checking for 'ncoding:'\n");
#endif
          break;

	case 'l':
	case 'L':
          me->check_pointer = "ength:";
          me->if_ok = CONTENT_LENGTH;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr,
                     "[MIME] Was CONTENT_, found L, checking for 'ength:'\n");
#endif
          break;

	default:
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr,
                     "[MIME] Was CONTENT_, found nothing; bleah\n");
#endif
          goto bad_field_name;

	} /* switch on character */
      break;

    case CONTENT_T:
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr,
                 "[MIME] in case CONTENT_T\n");
#endif
      switch(c)
        {
	case 'r':
	case 'R':
          me->check_pointer = "ansfer-encoding:";
          me->if_ok = CONTENT_TRANSFER_ENCODING;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr,
                     "[MIME] Was CONTENT_T; going to check for ansfer-encoding:\n");
#endif
          break;

	case 'y':
	case 'Y':
          me->check_pointer = "pe:";
          me->if_ok = CONTENT_TYPE;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr, "[MIME] Was CONTENT_T; going to check for pe:\n");
#endif
          break;

	default:
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf (stderr,
                     "[MIME] Was CONTENT_T; found nothing; bleah\n");
#endif
          goto bad_field_name;
        } /* switch on character */
      break;

    case L:
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr,
                 "[MIME] in case L\n");
#endif
      switch(c)
        {
		case 'a':
		case 'A':
			me->check_pointer = "st-modified:";
			me->if_ok = LAST_MODIFIED;
			me->state = CHECK;
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr,
					 "[MIME] Was L; going to check for st-modified:\n");
#endif
			break;

		case 'o':
		case 'O':
			me->check_pointer = "cation:";
			me->if_ok = LOCATION;
			me->state = CHECK;
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr,
					 "[MIME] Was L; going to check for ocation:\n");
#endif
			break;

		default:
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr,
					 "[MIME] Was L; found nothing; bleah\n");
#endif
			goto bad_field_name;
	} /* switch on character */
      break;

    case E:
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr,
                 "[MIME] in case E\n");
#endif
      switch(c)
        {
		case 'x':
		case 'X':
			me->state = EX;
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr,
					 "[MIME] Was EX; going to check for EXP or EXT:\n");
#endif
			break;

		default:
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr,
					 "[MIME] Was E; found nothing; bleah\n");
#endif
			goto bad_field_name;
	} /* switch on character */
      break;

    case EX:
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr,
                 "[MIME] in case EX\n");
#endif
      switch(c)
        {
		case 'p':
		case 'P':
			me->check_pointer = "ires";
			me->if_ok = EXPIRES;
			me->state = CHECK;
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr,
					 "[MIME] Was EXP; going to check for 'ires'\n");
#endif
			break;

		case 't':
		case 'T':
			me->check_pointer = "ension:";
			me->if_ok = EXTENSION;
			me->state = CHECK;
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr,
					 "[MIME] Was EXT; going to check for 'ension:'\n");
#endif
			break;

		default:
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf (stderr,
					 "[MIME] Was EX; found nothing; bleah\n");
#endif
			goto bad_field_name;
	} /* switch on character */
      break;

#ifdef HAVE_KERBEROS
    case WWW_AUTHENTICATE:
#endif
    case EXTENSION:
    case CONTENT_TYPE:
    case CONTENT_TRANSFER_ENCODING:
    case CONTENT_ENCODING:
    case CONTENT_LENGTH:
    case LOCATION:
    case EXPIRES:
    case LAST_MODIFIED:
      me->field = me->state;		/* remember it */
      me->state = SKIP_GET_VALUE;
      /* Fall through! (no break!) */
    case SKIP_GET_VALUE:
      if (c == '\n')
        {
          me->fold_state = me->state;
          me->state = NEWLINE;
          break;
        }
      if (WHITE(c))
        break;	/* Skip white space */

      me->value_pointer = me->value;
      me->state = GET_VALUE;
      /* Fall through to store first character */

    case GET_VALUE:
      if (WHITE(c))
        {
          /* End of field */
          *me->value_pointer = 0;
          switch (me->field)
            {
            case CONTENT_TYPE:
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf (stderr, "[MIME_put_char] Got content-type value '%s'\n", me->value);
#endif
              /* Lowercase it. */
              {
                char *tmp;
				// SAM
				if((tmp = strchr(me->value, ';'))) *tmp = '\0';
				// SAM
                for (tmp = me->value; *tmp; tmp++)
                  *tmp = TOLOWER (*tmp);
              }
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf (stderr, "[MIME_put_char] Lowercased to '%s'\n", me->value);
#endif
              me->format = HTAtom_for(me->value);
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf (stderr, "[MIME_put_char] Got content-type value atom %p\n",
                         me->format);
#endif
              break;
	    case CONTENT_TRANSFER_ENCODING:
              me->encoding = HTAtom_for(me->value);
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf (stderr,
                         "[MIME_put_char] Picked up transfer_encoding '%s'\n",
                         (char*)me->encoding);
#endif
              break;
            case CONTENT_ENCODING:
              me->compression_encoding = strdup (me->value);
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf (stderr,
                         "[MIME_put_char] Picked up compression encoding '%s'\n",
                         me->compression_encoding);
#endif
              break;
            case CONTENT_LENGTH:
              me->content_length = atoi (me->value);
              /* This is TEMPORARY. */
              loading_length = me->content_length;
	      noLength=0;
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf (stderr,
                         "[MIME_put_char] Picked up content length '%d'\n",
                         me->content_length);
#endif
              break;
            case EXPIRES:
		if (me->value_pointer < me->value + VALUE_SIZE - 1)
		{
			*me->value_pointer++ = c;
			*me->value_pointer = 0;
		}
		else
		{
			goto value_too_long;
		}
		if (me->expires)
			free(me->expires);
		me->expires = strdup(me->value);
#ifndef DISABLE_TRACE
		if (www2Trace)
			fprintf(stderr,
				"[MIME_put_char] Picked up expires '%s'\n", me->value);
#endif
		break;
            case LAST_MODIFIED:
		if (me->value_pointer < me->value + VALUE_SIZE - 1)
		{
			*me->value_pointer++ = c;
			*me->value_pointer = 0;
		}
		else
		{
			goto value_too_long;
		}
		if (me->last_modified)
			free(me->last_modified);
		me->last_modified = strdup(me->value);
#ifndef DISABLE_TRACE
		if (www2Trace)
			fprintf(stderr,
				"[MIME_put_char] Picked up last modified '%s'\n", me->value);
#endif
		break;
            case LOCATION:
		me->location = me->value;
		redirecting_url = strdup (me->location);
#ifndef DISABLE_TRACE
		if (www2Trace)
			fprintf(stderr,
				"[MIME_put_char] Picked up location '%s'\n", me->location);
#endif
		break;

#ifdef HAVE_KERBEROS
            case WWW_AUTHENTICATE:
                /*
                 * msg from server looks like:
                 * WWW-Authenticate: KerberosV4 [strified ktext]
                 * also allowed: KerberosV5, KerbV4-Encrypted, KerbV5-Encrypted
                 *
                 * This code is ugly: we have to keep this got_kerb static around because
                 * the FSM isn't really designed to have fields with values that
                 * include whitespace.  got_kerb tells us that we've been in this code
                 * before, and that we saw the word "kerberos"
                 */
#ifndef DISABLE_TRACE
                if (www2Trace) fprintf(stderr, "[MIME put char] picked up Auth. arg '%s'\n",
                                   me->value);
#endif
                if (got_kerb) {
                    validate_kerberos_server_auth(kscheme, me->value);
                    got_kerb = 0;       /* reset kerb state */
                    me->state = me->field;
		} else if (!my_strncasecmp(me->value, "kerb", 4)) {
                    if (0) {    /* just to get things started */
		    }
#ifdef KRB4
                    else if (!my_strncasecmp(me->value, "KerberosV4", 10)) {
                        kscheme = HTAA_KERBEROS_V4;
                        got_kerb = 1;
                        me->state = SKIP_GET_VALUE;
		    }
#endif
#ifdef KRB5
                    else if (!my_strncasecmp(me->value, "KerberosV5", 10)) {
                        kscheme = HTAA_KERBEROS_V5;
                        got_kerb = 1;
                        me->state = SKIP_GET_VALUE;
		    }
#endif
                    else {
                        fprintf(stderr, "Unrecognized field in WWW-Authenticate header\n");
                        me->state = me->field;
		    }

		}
                break;
#endif

            case EXTENSION:
#ifndef DISABLE_TRACE
		if (www2Trace)
			fprintf (stderr, "[MIME_put_char] Got Extension value '%s'\n", me->value);
#endif
		/* Lowercase it. */
		{
                char *tmp;
			for (tmp = me->value; *tmp; tmp++)
				*tmp = TOLOWER (*tmp);
		}
#ifndef DISABLE_TRACE
		if (www2Trace)
			fprintf (stderr, "[MIME_put_char] Lowercased to '%s'\n", me->value);
#endif
		switch(*(me->value)) {
			case 'd': /*Domain*/
				if (!strcmp(me->value,"domain-restricted")) {
					securityType=HTAA_DOMAIN;
#ifndef DISABLE_TRACE
					if (www2Trace)
						fprintf (stderr, "[MIME_put_char] Domain restricted extension header found.\n");
#endif
					break;
				}
				/*fall through*/
			default: /*Unknown*/
#ifndef DISABLE_TRACE
				if (www2Trace)
					fprintf (stderr, "[MIME_put_char] Unknown extension header: '%s'\n", me->value);
#endif
				me->state=me->field;
				break;
		}
		break;

	    default:		/* Should never get here */
              break;
	    }
	}
      else
        {
          if (me->value_pointer < me->value + VALUE_SIZE - 1)
            {
              *me->value_pointer++ = c;
              break;
            }
          else
            {
              goto value_too_long;
	    }
	}
      /* Fall through */

    case JUNK_LINE:
      if (c == '\n')
        {
          me->state = NEWLINE;
          me->fold_state = me->state;
	}
      break;

    } /* switch on state*/

  return;

 value_too_long:
#ifndef DISABLE_TRACE
  if (www2Trace) fprintf(stderr,
                     "HTMIME: *** Syntax error. (string too long)\n");
#endif

 bad_field_name:				/* Ignore it */
  me->state = JUNK_LINE;
  return;
}



/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/
PRIVATE void HTMIME_put_string ARGS2(HTStream *, me, WWW_CONST char*, s)
{
  WWW_CONST char * p;
#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "[HTMIME_put_string] Putting '%s'\n", s);
#endif
  if (me->state == MIME_TRANSPARENT)		/* Optimisation */
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTMIME_put_string] Doing transparent put_string\n");
#endif
      (*me->targetClass.put_string)(me->target,s);
    }
  else if (me->state != MIME_IGNORE)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTMIME_put_string] Doing char-by-char put_character\n");
#endif
      for (p=s; *p; p++)
        HTMIME_put_character(me, *p);
    }
  else
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTMIME_put_string] DOING NOTHING!\n");
#endif
    }
  return;
}


/*	Buffer write.  Buffers can (and should!) be big.
**	------------
*/
PRIVATE void HTMIME_write ARGS3(HTStream *, me, WWW_CONST char*, s, int, l)
{
  WWW_CONST char * p;
#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf (stderr, "[HTMIME_write] Putting %d bytes\n", l);
#endif
  if (me->state == MIME_TRANSPARENT)		/* Optimisation */
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTMIME_write] Doing transparent put_block\n");
#endif
      (*me->targetClass.put_block)(me->target, s, l);
    }
  else if (me->state != MIME_IGNORE)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTMIME_write] Doing char-by-char put_character\n");
#endif

      for (p=s; p < s+l; p++)
        HTMIME_put_character(me, *p);
    }
  else
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTMIME_write] DOING NOTHING!\n");
#endif
    }
  return;
}




/*	Free an HTML object
**	-------------------
**
*/
PRIVATE void HTMIME_free ARGS1(HTStream *, me)
{
  if (!me->target)
    {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "[HTMIME_free] Caught case where we didn't get a target.\n");
#endif
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "  me %p, me->target %p\n", me, me->target);
#endif
      me->format = HTAtom_for ("text/html");
      me->target = HTStreamStack(me->format, me->targetRep, 0,
                                 me->sink, me->anchor);
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "  me->target->isa %p\n", me->target->isa);
#endif
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf (stderr, "  me->target->isa->name %s\n", me->target->isa->name);
#endif
      me->targetClass = *me->target->isa;
      (*me->targetClass.put_string) (me->target, "<H1>ERROR IN HTTP/1.0 RESPONSE</H1> The remote server returned a HTTP/1.0 response that Mosaic's MIME parser could not understand.  Please contact the server maintainer.<P> Sorry for the inconvenience,<P> <ADDRESS>The Management</ADDRESS>");
      securityType=HTAA_UNKNOWN;
    }
  if (me->target)
    (*me->targetClass.free)(me->target);

  if (me->expires)
     {
       char *p;

       if (HTTP_expires)
       free(HTTP_expires);
       HTTP_expires = me->expires;
       for (p = HTTP_expires + strlen(HTTP_expires) - 1;
          p > HTTP_expires && isspace(*p); p--)
       {
         *p = '\0';
       }

     }

   if (me->last_modified)
     {
       char *p;

       if (HTTP_last_modified)
       free(HTTP_last_modified);
       HTTP_last_modified = me->last_modified;
       for (p = HTTP_last_modified + strlen(HTTP_last_modified) - 1;
          p > HTTP_last_modified && isspace(*p); p--)
       {
         *p = '\0';
       }

     }

  free(me);

  return;
}

/*	End writing
*/

PRIVATE void HTMIME_end_document ARGS1(HTStream *, me)
{
  if (me->target)
    (*me->targetClass.end_document)(me->target);
}

PRIVATE void HTMIME_handle_interrupt ARGS1(HTStream *, me)
{
  me->interrupted = 1;

  /* Propagate interrupt message down. */
  if (me->target)
    (*me->targetClass.handle_interrupt)(me->target);

  return;
}



/*	Structured Object Class
**	-----------------------
*/
PUBLIC WWW_CONST HTStreamClass HTMIME =
{
  "MIMEParser",
  HTMIME_free,
  HTMIME_end_document,
  HTMIME_put_character, 	HTMIME_put_string,
  HTMIME_write,
  HTMIME_handle_interrupt
  };


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStream* HTMIMEConvert ARGS5(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,
	HTStream *,		sink,
        HTFormat,               format_in,
        int,                    compressed)
{
    HTStream* me;

    me = malloc(sizeof(*me));
    me->isa = &HTMIME;

#ifndef DISABLE_TRACE
    if (www2Trace)
      fprintf (stderr, "[HTMIMEConvert] HELLO!\n");
#endif

    me->sink = sink;
    me->anchor = anchor;
    me->target = NULL;
    me->state = BEGINNING_OF_LINE;
    me->format = WWW_PLAINTEXT;
    me->targetRep = pres->rep_out;
    me->boundary = 0;		/* Not set yet */
    me->location = 0;
    me->interrupted = 0;
    me->encoding = 0;
    me->compression_encoding = 0;
    me->content_length = -1;
    me->header_length = 0; /* bjs - to allow differentiation between
			      content and header for read length */
    me->expires = 0;
    me->last_modified = 0;
    return me;
}

/* bjs - a kludge for HTFormat.c */
int HTMIME_get_header_length(HTStream *me)
{
    if(me->isa != &HTMIME) return 0; /* in case we screw up */
    return me->header_length;
}
