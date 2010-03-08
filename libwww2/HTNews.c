/*			NEWS ACCESS				HTNews.c
**			===========
**
** History:
**	26 Sep 90	Written TBL
**	29 Nov 91	Downgraded to C, for portable implementation.
**         Mar 95       Newsreader Enhancements (in progress) -B. Swetland
**         Mar 96       Newsreader Enhancements (part two) - P. Bleisch
**         Apr 96       More cleanup for 2.7b5 - P.Bleisch
*/

/*
** WARNING: This code is under development.  The only thing good about it
** right now is that it works (most of the time). Read at your own risk.
**     -- BJS
**
** Do not take Brian's warnings lightly, this code gave me a headache, and
** caused long bouts of sleeplessness.  Be warned.  P. Bleisch
*
*
* - fixed &gt and &lt's with ;'s
* - added xhdr patch by MH 
*
*/
#include "../config.h"
char *mo_tmpnam(char *url);


#include "HTNews.h"
#include "../src/mosaic.h"
#include "../src/newsrc.h"
#include "../src/prefs.h"

#define NEWS_PORT 119		/* See rfc977 */
#define APPEND			/* Use append methods */

#ifndef DEFAULT_NEWS_HOST
#define DEFAULT_NEWS_HOST "news"
#endif
#ifndef SERVER_FILE
#define SERVER_FILE "/usr/local/lib/rn/server"
#endif

#define FAST_THRESHOLD 100	/* Above this, read IDs fast */
#define CHOP_THRESHOLD 50	/* Above this, chop off the rest */

#include <ctype.h>
#include "HTUtils.h"		/* Coding convention macros */
#include "tcp.h"

#include "HTML.h"
#include "HTParse.h"
#include "HTFormat.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

struct _HTStructured 
{
  WWW_CONST HTStructuredClass *	isa;
  /* ... */
};

#define LINE_LENGTH 512		   /* Maximum length of line of ARTICLE etc */
#define GROUP_NAME_LENGTH    256   /* Maximum length of group name */



/*	Module-wide variables
*/
PUBLIC  NewsArt *CurrentArt = NULL;
PRIVATE NewsArt *FirstArt = NULL;
PRIVATE NewsArt *LastArt = NULL;
PUBLIC  char *NewsGroup = NULL;
PUBLIC  newsgroup_t *NewsGroupS = NULL;
PRIVATE int Count = 0;
PRIVATE int GroupFirst = 0;
PRIVATE int GroupLast = 0;
PRIVATE int ReadLast = 0;
PRIVATE int ReadFirst = 0;
PRIVATE newsgroup_t *LastGroup = NULL;
PRIVATE NewsArt *NextArt = NULL;

PUBLIC char * HTNewsHost;
PRIVATE int s;					/* Socket for NewsHost */
PRIVATE char response_text[LINE_LENGTH+1];	/* Last response */


/* PRIVATE HText *	HT;	*/		/* the new hypertext */
PRIVATE HTStructured * target;			/* The output sink */
PRIVATE HTStructuredClass targetClass;		/* Copy of fn addresses */
PRIVATE HTParentAnchor *node_anchor;		/* Its anchor */
PRIVATE int	diagnostic;			/* level: 0=none 2=source */


int ConfigView = 0;                      /* view format configure */
int newsShowAllGroups = 0;
int newsShowReadGroups = 0;
int newsShowAllArticles = 0;
int newsNoThreadJumping = 0;
int newsGotList = 0;        
int newsUseNewsRC = 1;   
int newsNextIsUnread = 0;
int newsPrevIsUnread = 0;
extern int newsNoNewsRC;
int newsSubjWidth = 38;
int newsAuthWidth = 30;


#define PUTC(c) (*targetClass.put_character)(target, c)
#define PUTS(s) (*targetClass.put_string)(target, s)
#define START(e) (*targetClass.start_element)(target, e, 0, 0)
#define END(e) (*targetClass.end_element)(target, e)


/* escapeString ()
   Expects: str -- String to escape
            buf -- Buffer to store escaped string
   Returns: nothing

   Escapes all <'s and >'s and ...
*/
void escapeString (char *str, char *buf)
{
  
  while (str && *str) {

    switch (*str) {

    case '<':
      *buf = '&'; buf++; *buf = 'l'; buf++; 
      *buf = 't'; buf++; *buf = ';'; buf++;
      break;

    case '>':
      *buf = '&'; buf++; *buf = 'g'; buf++; 
      *buf = 't'; buf++; *buf = ';'; buf++;
      break;

    case '&':
      *buf = '&'; buf++; *buf = 'a'; buf++; 
      *buf = 'm'; buf++; *buf = 'p'; buf++;
      *buf = ';'; buf++;
      break;

    default:
      *buf = *str;
      buf++;
    }
    str++;
  }
  *buf = 0;
}
       


/* HTSetNewsConfig ()
   Expects: artView    -- Article View configuration: 0 = Article View, 
                          1 = Thread View
            artAll     -- Show All Articles? 0 = No, non zero = yes
	    grpAll     -- Show All Groups? 0 = no, non zero = yes
	    grpRead    -- Show Read Groups? 0 = no, non zero = yes
	    noThrJmp   -- Don't jump threads? 0 = no, non zero = yes
	    newsRC     -- Use the newsrc? 0 = no, non zero = yes
	    nxtUnread  -- Next thread should be the next unread? 
	                  0 = no, non zero = yes
	    prevUnread -- Prev thread should be the prev unread? 
	                  0 = no, non zero = yes
    Returns: Nothing

    Sets the current news config.
*/

void HTSetNewsConfig (int artView, int artAll, int grpAll, int grpRead, 
		      int noThrJmp, int newsRC, int nxtUnread, int prevUnread)
{
  if (artView != NO_CHANGE) {
    ConfigView = !artView;
    set_pref (eUSETHREADVIEW, &artView);
  }

  if (artAll != NO_CHANGE) {
    newsShowAllArticles = artAll;
    set_pref (eSHOWALLARTICLES, &newsShowAllArticles);
  }

  if (grpAll != NO_CHANGE) {
    newsShowAllGroups = grpAll;
    set_pref (eSHOWALLGROUPS, &newsShowAllGroups);
  }

  if (grpRead != NO_CHANGE) {
    newsShowReadGroups = grpRead;
    set_pref (eSHOWREADGROUPS, &newsShowReadGroups);
  }

  if (noThrJmp != NO_CHANGE) {
    newsNoThreadJumping = noThrJmp;
    set_pref (eNOTHREADJUMPING, &newsNoThreadJumping);
  }

  if (newsRC != NO_CHANGE) {
    newsUseNewsRC = newsRC;
    set_pref (eUSENEWSRC, &newsUseNewsRC);
  }

  if (nxtUnread != NO_CHANGE) {
    newsNextIsUnread = nxtUnread;
    set_pref (eNEXTISUNREAD, &newsNextIsUnread);
  }

  if (prevUnread != NO_CHANGE) {
    newsPrevIsUnread = prevUnread;
    set_pref (ePREVISUNREAD, &newsPrevIsUnread);
  }

}


/*	Case insensitive string comparisons
**	-----------------------------------
**
** On entry,
**	template must be already un upper case.
**	unknown may be in upper or lower or mixed case to match.
*/
PRIVATE BOOL match ARGS2 (WWW_CONST char *,unknown, WWW_CONST char *,template)
{
    WWW_CONST char * u = unknown;
    WWW_CONST char * t = template;
    for (;*u && *t && (TOUPPER(*u)==*t); u++, t++) /* Find mismatch or end */ ;
    return (BOOL)(*t==0);		/* OK if end of template */
}

/* parseemail ()
   Expects: str -- string to parse
            name -- buffer for name
            em -- buffer for email
   Returns: pointer to em

   Notes:
   Parse str for an email address and author name in the email@host (name) 
   or name <email@host> forms.  This destroys the source string.  If 
   either name of em is NULL, the value will not be returned.
*/
char *parseemail (char *str, char *name, char *em)
{
  char *c, d;
  char *email, *end;

  /* Pull out email address */
  if ((email=strchr(str,'<')) && (end=strrchr(str,'>'))) {
    email++;
    if (email < end) {
      *end = 0;
      if (em)
	strcpy (em, email);
      email--;
      *email = 0;
      if (name) {
	while (*str && strchr (" \t\n", *str)) str++;
	strcpy (name, str);
      }
      return em;
    }
  } else if ((email=strchr(str,'(')) && (end=strrchr(str,')'))) {
    email++;
    if (email<end) {
      *end = 0;
      if (name)
	strcpy (name, email);
      email--;
      *email = 0;
      if (em) {
	while (*str && strchr (" \t\n", *str)) str++;
	strcpy (em, str);
      }
      return em;
    }
  }

  if (name) {
	*name='\0';
  }
  if (em) {
	*em='\0';
  }
  return(NULL);
}


/* compare two strings w/out crashing SGIs, etc */
static int strmatch(char *s1, char *s2)
{
    if(!s1 || !s2) return 0;

    while(*s1){
	if(!*s2) return 0;
	if(*s1 != *s2) return 0;
	s1++; s2++;
    }
    return 1;
}


/******************* UUDECODE STUFF ********************/
FILE *startuudecode(char *s)
{
    return fopen(s,"w");
}

#define DEC(Char) (((Char) - ' ') & 077)

int uudecodeline(FILE *fp, char *buf)
{
    int n;
    char *p;
    char ch;

    if(!buf){
        fclose(fp);
        return 1;
    }

    if(!strncmp(buf,"end",3)) {
        fclose(fp);
        return 1;
    }
    
    p = buf;
        /* N is used to avoid writing out all the characters at the end of
           the file.  */

    n = DEC (*p);
    if (n > 0) {
        for (++p; n > 0; p += 4, n -= 3){
            if (n >= 3){
                ch = DEC (p[0]) << 2 | DEC (p[1]) >> 4;
                fputc(ch,fp);
                ch = DEC (p[1]) << 4 | DEC (p[2]) >> 2;
                fputc(ch,fp);
                ch = DEC (p[2]) << 6 | DEC (p[3]);
                fputc(ch,fp);
            } else {
                if (n >= 1) {
                    ch = DEC (p[0]) << 2 | DEC (p[1]) >> 4;
                    fputc(ch,fp);
                }
                if (n >= 2) {
                    ch = DEC (p[1]) << 4 | DEC (p[2]) >> 2;
                    fputc(ch,fp);
                }
            }
        }
    }
    return 0;
}

static char b64_tab[256] = {
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*000-007*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*010-017*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*020-027*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*030-037*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*040-047*/
    '\177', '\177', '\177', '\76',  '\177', '\177', '\177', '\77',  /*050-057*/
    '\64',  '\65',  '\66',  '\67',  '\70',  '\71',  '\72',  '\73',  /*060-067*/
    '\74',  '\75',  '\177', '\177', '\177', '\100', '\177', '\177', /*070-077*/
    '\177', '\0',   '\1',   '\2',   '\3',   '\4',   '\5',   '\6',   /*100-107*/
    '\7',   '\10',  '\11',  '\12',  '\13',  '\14',  '\15',  '\16',  /*110-117*/
    '\17',  '\20',  '\21',  '\22',  '\23',  '\24',  '\25',  '\26',  /*120-127*/
    '\27',  '\30',  '\31',  '\177', '\177', '\177', '\177', '\177', /*130-137*/
    '\177', '\32',  '\33',  '\34',  '\35',  '\36',  '\37',  '\40',  /*140-147*/
    '\41',  '\42',  '\43',  '\44',  '\45',  '\46',  '\47',  '\50',  /*150-157*/
    '\51',  '\52',  '\53',  '\54',  '\55',  '\56',  '\57',  '\60',  /*160-167*/
    '\61',  '\62',  '\63',  '\177', '\177', '\177', '\177', '\177', /*170-177*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*200-207*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*210-217*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*220-227*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*230-237*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*240-247*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*250-257*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*260-267*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*270-277*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*300-307*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*310-317*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*320-327*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*330-337*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*340-347*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*350-357*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*360-367*/
    '\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*370-377*/
};

int base64line(FILE *fp, char *buf)
{
    int last_data = 0;
    unsigned char *p = (unsigned char *) buf;

    if(!buf || !*buf || isspace(*buf)){
        fclose(fp);
        return 1;
    }
            
            /* The following implementation of the base64 decoding might look
               a bit clumsy but I only try to follow the POSIX standard:
               ``All line breaks or other characters not found in the table
               [with base64 characters] shall be ignored by decoding
               software.''  */
    while (*p){
        char c1, c2, c3;

        while ((b64_tab[*p] & '\100') != 0) 
            if (!*p || *p++ == '=') break;
        
        if (!*p) continue;          /* This leaves the loop.  */
        
        c1 = b64_tab[*p++];
        
        while ((b64_tab[*p] & '\100') != 0) {
            if (!*p || *p++ == '=') {
                HTProgress("illegal base64 line");
                return 1;
            }
        }
        
        c2 = b64_tab[*p++];
        
        while (b64_tab[*p] == '\177') {
            if (!*p++) {
                HTProgress("illegal base64 line");
                return 1;
            }
        }
          
        if (*p == '=') {
            fputc(c1 << 2 | c2 >> 4,fp);
            last_data = 1;
            break;
        }
            
        c3 = b64_tab[*p++];

        while (b64_tab[*p] == '\177') {
            if (!*p++) {
                HTProgress("illegal base64 line");
                return 1;
            }
        }
        
        fputc(c1 << 2 | c2 >> 4,fp);
        fputc(c2 << 4 | c3 >> 2,fp);
        if (*p == '=') {
/*            return 1;*/
            break;
        } else {
            fputc(c3 << 6 | b64_tab[*p++],fp);
        }
    }
    return 0;
    
}

/*******************************************************/

/* Article list management */

/* freeart ()
   Walks the article list passed in and frees the important stuff.
*/
void freeart(NewsArt *art)
{
    if(art->ID) free(art->ID);
    if(art->SUBJ) free(art->SUBJ);
    if(art->FROM) free(art->FROM);
    if(art->FirstRef) free(art->FirstRef);
    if(art->LastRef) free(art->LastRef);
    free(art);
}


/* ClearArtList ()
   Walks the global thread information and frees the important stuff.
*/
PRIVATE void ClearArtList NOARGS
{
    NewsArt *temp,*tnext,*temp2,*tnext2;
    
    temp = FirstArt;
    while(temp) {
	tnext = temp->nextt;
	if(temp->next){
	    temp2 = temp->next;
	    while(temp2){
		tnext2 = temp2->next;
		freeart(temp2);
		temp2 = tnext2;
	    }
	}
	freeart(temp);
	temp = tnext;
    }

    FirstArt = NULL;
    LastArt = NULL;
    CurrentArt = NULL;

    GroupLast = GroupFirst = ReadLast = ReadFirst = 0;

    if(NewsGroup) free(NewsGroup);
    NewsGroup = NULL;
    Count = 0;
}

/* NewArt ()
   Allocates a new article list.
*/
PRIVATE NewsArt *NewArt NOARGS
{
    NewsArt *temp;

    if( !(temp = (NewsArt *) malloc(sizeof(NewsArt))) )
	outofmem(__FILE__, "NewArt");

    /* wipe potentially unused fields */
    temp->LastRef = NULL;
    temp->FirstRef = NULL;

    temp->ID = NULL;
    temp->FROM = NULL;
    temp->SUBJ = NULL;
    return temp;
}

/* AddArtTop ()
   Add an Article to the thread chain 
*/
PRIVATE void AddArtTop ARGS1(WWW_CONST NewsArt *, add)
{
    NewsArt *temp;

    /* Easy case ... */
    if(!FirstArt){
	add->prev = NULL;
	add->next = NULL;
	add->prevt = NULL;
	add->nextt = NULL;
	FirstArt = add;
	LastArt = add;
	return;
    }

    /* If threaded, try to find some more of me ... */
    if(add->FirstRef){
	for(temp = LastArt; temp; temp = temp->prevt)
	    if( strmatch(add->FirstRef,temp->FirstRef) 
		|| strmatch(add->FirstRef,temp->ID) ) break;
    } else { 
	for(temp = LastArt; temp; temp = temp->prevt)
	    if(strmatch(add->ID,temp->FirstRef)) break;
    }

    /* If we found a thread point ... */
    if(temp){
	/* follow the thread on down... */
	while(temp->next) temp = temp->next;

	add->prevt = add->nextt = NULL;
	add->next = NULL;
	add->prev = temp;
	
	temp->next = add;
    } else {
	/* Otherwise, tack it onto the back of the list */
	add->prev = NULL;
	add->next = NULL;

	add->prevt = LastArt;
	add->nextt = NULL;
	LastArt->nextt = add;
	LastArt = add;
    }
}


/* start_anchor ()
   Start anchor element.
*/
PRIVATE void start_anchor ARGS1(WWW_CONST char *,  href)
{
  PUTS ("<A HREF=\"");
  PUTS (href);
  PUTS ("\">");
}

/*	Paste in an Anchor
**	------------------
**
**
** On entry,
**	HT 	has a selection of zero length at the end.
**	text 	points to the text to be put into the file, 0 terminated.
**	addr	points to the hypertext refernce address,
**		terminated by white space, comma, NULL or '>' 
*/
PRIVATE void write_anchor ARGS2(WWW_CONST char *,text, WWW_CONST char *,addr)
{
    char href[LINE_LENGTH+1];
    WWW_CONST char * p;
    strcpy(href,"news:");
    for(p=addr; *p && (*p!='>') && !WHITE(*p) && (*p!=','); p++);
    strncat(href, addr, p-addr);	/* Make complete hypertext reference */
    start_anchor(href);
    PUTS(text);
    PUTS("</A>");
}


/*	Write list of anchors
**	---------------------
**
**	We take a pointer to a list of objects, and write out each,
**	generating an anchor for each.
**
** On entry,
**	HT 	has a selection of zero length at the end.
**	text 	points to a comma or space separated list of addresses.
** On exit,
**	*text	is NOT any more chopped up into substrings.
*/
PRIVATE void write_anchors ARGS1 (char *,text)
{
    char * start = text;
    char * end;
    char c;
    for (;;) {
        for(;*start && (WHITE(*start)); start++);  /* Find start */
	if (!*start) return;			/* (Done) */
        for(end=start; *end && (*end!=' ') && (*end!=','); end++);/* Find end */
	if (*end) end++;	/* Include comma or space but not NULL */
	c = *end;
	*end = 0;
	write_anchor(start, start);
	*end = c;
	start = end;			/* Point to next one */
    }
}

/* abort_socket ()
   Aborts the current connection.
*/
PRIVATE void abort_socket NOARGS
{
#ifndef DISABLE_TRACE
  if (www2Trace) fprintf(stderr,
			 "HTNews: EOF on read, closing socket %d\n", s);
#endif
  NETCLOSE(s);	/* End of file, close socket */
  PUTS("Network Error: connection lost");
  PUTC('\n');
  s = -1;		/* End of file on response */
  return;
}


/* HTGetNewsHost () && HTSetNewsHost ()
   Return/Set the newshost name.
*/
PUBLIC WWW_CONST char * HTGetNewsHost NOARGS
{
  return HTNewsHost;
}

PUBLIC void HTSetNewsHost ARGS1(WWW_CONST char *, value)
{
  StrAllocCopy(HTNewsHost, value);
}

/*	Initialisation for this module
**	------------------------------
**
**	We pick up the NewsHost name from
**
**	1.	Environment variable NNTPSERVER
**	2.	File SERVER_FILE
**	3.	Compilation time macro DEFAULT_NEWS_HOST
**	4.	Default to "news"
*/
PRIVATE BOOL initialized = NO;
PRIVATE BOOL initialize NOARGS
{
  /*   Get name of Host  */
  if (getenv("NNTPSERVER")) {
    StrAllocCopy(HTNewsHost, (char *)getenv("NNTPSERVER"));
#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr, "HTNews: NNTPSERVER defined as `%s'\n",
			   HTNewsHost);
#endif
  } else {
    char server_name[256];
    FILE* fp = fopen(SERVER_FILE, "r");
    if (fp) {
      if (fscanf(fp, "%s", server_name)==1) {
	StrAllocCopy(HTNewsHost, server_name);
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr,
			       "HTNews: File %s defines news host as `%s'\n",
			       SERVER_FILE, HTNewsHost);
#endif
      }
      fclose(fp);
    }
  }
  if (!HTNewsHost) 
    HTNewsHost = DEFAULT_NEWS_HOST;
  
  
#ifndef DISABLE_TRACE
  if (www2Trace) fprintf(stderr, "HTNews: initialising newsrc for host\n");
#endif
  
  s = -1;		/* Disconnected */
  return YES;
}



/*	Send NNTP Command line to remote host & Check Response
**	------------------------------------------------------
**
** On entry,
**	command	points to the command to be sent, including CRLF, or is null
**		pointer if no command to be sent.
** On exit,
**	Negative status indicates transmission error, socket closed.
**	Positive status is an NNTP status.
*/

PRIVATE int newswrite ARGS1(WWW_CONST char *, msg)
{
    int status;
    if( (status = NETWRITE(s, msg, strlen(msg))) <0){
#ifndef DISABLE_TRACE
	if (www2Trace) 
	    fprintf(stderr, "HTNews: Unable to send command. Disconnecting.\n");
#endif
	NETCLOSE(s);
	s = -1;
    } /* if bad status */
    return status;
}


PRIVATE int response ARGS1(WWW_CONST char *,command)
{
  int result;    
  char * p = response_text;
  if (command) {
    int status;
    int length = strlen(command);
#ifndef DISABLE_TRACE
    if (www2Trace) 
      fprintf(stderr, "NNTP command to be sent: %s", command);
#endif
    status = NETWRITE(s, command, length);
    if (status<0) {
#ifndef DISABLE_TRACE
      if (www2Trace) fprintf(stderr,
                             "HTNews: Unable to send command. Disconnecting.\n");
#endif
      NETCLOSE(s);
      s = -1;
      return status;
    } /* if bad status */
  } /* if command to be sent */
  
  for(;;) {  
    if (((*p++=HTGetCharacter ()) == LF) || (p == &response_text[LINE_LENGTH]))  {
      *p++=0;				/* Terminate the string */
#ifndef DISABLE_TRACE
      if (www2Trace) fprintf(stderr, "NNTP Response: %s\n", response_text);
#endif
      sscanf(response_text, "%d", &result);
      return result;	    
    } /* if end of line */
    
    if (*(p-1) < 0) {
#ifndef DISABLE_TRACE
      if (www2Trace) fprintf(stderr,
                             "HTNews: EOF on read, closing socket %d\n", s);
#endif
      NETCLOSE(s);	/* End of file, close socket */
      return s = -1;	/* End of file on response */
    }
  } /* Loop over characters */
}


/* Setup our networking */
PRIVATE int OpenNNTP NOARGS 
{
    /* CONNECTING to news host */
    char url[1024];
    int status;

    sprintf (url, "lose://%s/", HTNewsHost);

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf (stderr, "News: doing HTDoConnect on '%s'\n", url);
#endif

    status = HTDoConnect (url, "NNTP", NEWS_PORT, &s);

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf (stderr, "News: Done DoConnect; status %d\n", status);
#endif

    if (status == HT_INTERRUPTED) {
	/* Interrupt cleanly. */
	return 3;
    }

    if (status < 0) {
	NETCLOSE(s);
	s = -1;
#ifndef DISABLE_TRACE
	if (www2Trace) 
	    fprintf(stderr, "HTNews: Unable to connect to news host.\n");
#endif
	return 2;

    } else {
#ifndef DISABLE_TRACE
	if (www2Trace) 
	    fprintf(stderr, "HTNews: Connected to news host %s.\n",HTNewsHost);
#endif
	HTInitInput(s);		/* set up buffering */
	if ((response(NULL) / 100) !=2) {
	    NETCLOSE(s);
	    s = -1;
	    
	    return 1;
	}
    }
    return 0;
} 


/* Interface with news-gui.c and various others... */
void NNTPconfig(int viewtype)
{
    ConfigView = viewtype;
}

/* this is VERY non-reentrant.... */
static char qline[LINE_LENGTH+1];
char *NNTPgetquoteline(char *art)
{
    char *p;
    int i,f,status ;

    if (!initialized)
	initialized = initialize();
    if (!initialized){
#ifndef DISABLE_TRACE
	if(www2Trace) fprintf(stderr,"No init?\n");
#endif
	HTProgress ("Could not set up news connection.");
	return NULL;
    }
    
    if(s < 0) {
	HTProgress("Attempting to connect to news server");
	if(OpenNNTP()){
#ifndef DISABLE_TRACE
	    if(www2Trace) fprintf(stderr,"No OpenNNTP?\n");
#endif
	    HTProgress ("Could not connect to news server.");
	    return NULL;
	}
    }

    if(art){
	/* FLUSH!!! */
	HTInitInput(s);
	sprintf(qline, "BODY <%s>%c%c", art, CR, LF);
	status = response(qline);
	
	if (status != 222) return NULL;
    }

    qline[0] = '>';
    qline[1] = ' ';

    for(p = &qline[2],i=0;;p++,i++){
	*p = HTGetCharacter();

	if (*p==(char)EOF) {
	    abort_socket();	/* End of file, close socket */
	    return NULL;	/* End of file on response */
	}

	if(*p == '\n'){
	    *++p = 0;
	    break;
	}

	if(i == LINE_LENGTH-4){
	    *p = 0;
	    break;
	}
    }
    
    if(qline[2]=='.' && qline[3] < ' ') return NULL;
    return qline;
}
 
int NNTPgetarthdrs(char *art,char **ref, char **grp, char **subj, char **from)
{
    int status, done;
    char *aname,*p;
    char line[LINE_LENGTH+1];
    char buffer[LINE_LENGTH+1];

    *ref = *grp = *subj = *from = NULL;
 
    if (!initialized)
	initialized = initialize();
    if (!initialized){
#ifndef DISABLE_TRACE
	if(www2Trace) fprintf(stderr,"No init?\n");
#endif
	HTProgress ("Could not set up news connection.");
	return HT_NOT_LOADED;	/* FAIL */
    }
    
    if(s < 0) {
	HTProgress("Attempting to connect to news server");
	if(OpenNNTP()){
#ifndef DISABLE_TRACE
	    if(www2Trace) fprintf(stderr,"No OpenNNTP?\n");
#endif
	    HTProgress ("Could not connect to news server.");
	    return HT_NOT_LOADED;	/* FAIL */
	}
    }

    /* FLUSH!!! */
    HTInitInput(s);
    sprintf(buffer, "HEAD <%s>%c%c", art, CR, LF);
    status = response(buffer);
	
    if (status == 221) {	/* Head follows - parse it:*/
	
	p = line;				/* Write pointer */
	done = NO;
	while(!done){
	    char ch = *p++ = HTGetCharacter ();
	    if (ch==(char)EOF) {
		abort_socket();	/* End of file, close socket */
		return;		/* End of file on response */
	    }
	    
	    if ((ch == LF)
		|| (p == &line[LINE_LENGTH]) ) {
		
		*--p=0;		/* Terminate  & chop LF*/
		p = line;		/* Restart at beginning */
#ifndef DISABLE_TRACE
		if (www2Trace) fprintf(stderr, "G %s\n", line);
#endif
		switch(line[0]) {
		    
		case '.':
		    done = (line[1]<' ');	/* End of article? */
		    break;
		    
		case 'S':
		case 's':
		    if (match(line, "SUBJECT:"))
			StrAllocCopy(*subj, line+9);/* Save subject */
		    break;
		    
		case 'R':
		case 'r':
		    if (match(line, "REFERENCES:")) {
			p = line + 12;
			StrAllocCopy(*ref,p+1);
		    }
		    break;
		    
		case 'N':
		case 'n':
		    if (match(line, "NEWSGROUPS:")) {
			p = line + 11;
			StrAllocCopy(*grp,p+1);
		    }
		    break;
		    
		case 'f':
		case 'F':
		    if (match(line, "FROM:")) {
		      char author[1024+1];
			parseemail (strchr(line,':')+1, author, NULL);
			aname = author;
			if (aname && *aname){
			  StrAllocCopy(*from, aname);
			  p = *from + strlen(*from) - 1;
			  if (*p==LF) *p = 0;	/* Chop off newline */
			} else {
			  StrAllocCopy(*from, "Unknown");
			}
		    }
		    break;
		    
		} /* end switch on first character */
		
		p = line;		/* Restart at beginning */
	    } /* if end of line */
	} /* Loop over characters */
    } /* If good response */
}
 
int NNTPpost(char *from, char *subj, char *ref, char *groups, char *msg)
{
    char buf[1024];
  
    if (!initialized)
	initialized = initialize();
    if (!initialized){
#ifndef DISABLE_TRACE
	if(www2Trace) fprintf(stderr,"No init?\n");
#endif
	HTProgress ("Could not set up news connection.");
	return HT_NOT_LOADED;	/* FAIL */
    }
    
    if(s < 0) {
	HTProgress("Attempting to connect to news server");
	if(OpenNNTP()){
#ifndef DISABLE_TRACE
	    if(www2Trace) fprintf(stderr,"No OpenNNTP?\n");
#endif
	    HTProgress ("Could not connect to news server.");
	    return HT_NOT_LOADED;	/* FAIL */
	}
    }
    
    if(response("POST\r\n") != 340) {
	HTProgress("Server does not allow posting.");
	return 0;
    }

    HTProgress("Posting your article...");
    sprintf(buf,"From: %s\r\n",from);
    newswrite(buf);
    sprintf(buf,"Subject: %s\r\n",subj);
    newswrite(buf);
    if(ref){
	sprintf(buf,"References: %s\r\n",ref);
	newswrite(buf);
    }
    sprintf(buf,"Newsgroups: %s\r\n",groups);
    newswrite(buf);
    sprintf(buf,"X-Newsreader: NCSA Mosaic\r\n\r\n");
    newswrite(buf);
    newswrite(msg);
    if(response("\r\n.\r\n") != 240)
	HTProgress("Article was not posted.");
    else
	HTProgress("Article was posted successfully.");

    HTDoneWithIcon ();
}


/* take a url and return the news article for it if its in the news cache */
NewsArt *is_news_url(char *s)
{
    NewsArt *art, *art2;

    if (!s)
      return NULL;

    if (strchr (s, '*'))
      return NULL;
    if((strlen(s) > 5) && !strncmp("news:", s, 5)){
	s = &s[5];
	/* check the obvious */
	if(CurrentArt && strmatch(s,CurrentArt->ID)) return CurrentArt;
	for(art = FirstArt; art; art = art->nextt){
	    if(strmatch(s,art->ID)) return art;
	    if(art->next)
		for(art2 = art->next; art2; art2 = art2->next)
		    if(strmatch(s,art2->ID)) return art2;
	}
    }
    return NULL;
}

/* These are called by their gui_news_* counterparts in gui-news.c */

/* Beginning in 2.7b4, these now return the next/prev unread article/thread ,
   unless newsShowAllArticles is True.  The previous unread article/thread 
   is set in the news_next functions.  

   news_next will now continue onto the next thread unless newsNoThreadJumping 
   is True.
*/

NewsArt *nextUnreadThread (NewsArt *art);
NewsArt *prevUnreadThread (NewsArt *art);

/* Return first unread article in list (thread) */
NewsArt *firstUnread (NewsArt *art) 
{
  NewsArt *a;
  newsgroup_t *tempNewsGroupS = NULL;

  if (!art)
    return NULL;

  if (!newsUseNewsRC || newsShowAllArticles || !newsNextIsUnread ||
      (!(tempNewsGroupS = findgroup (NewsGroup))))
    return art;

  while (art && art->prev) art = art->prev;
  a = art;
  while (a) {
    if (!isread (tempNewsGroupS, a->num))
      return a;
    a = a->next;
  }
  return NULL;
}


/* return next unread article after art */
NewsArt *nextUnread (NewsArt *art, int probe) 
{
  NewsArt *a;
  newsgroup_t *tempNewsGroupS = NULL;

  if (!art)
    return NULL;

  if (!newsUseNewsRC || newsShowAllArticles || !newsNextIsUnread ||
      (!(tempNewsGroupS = findgroup (NewsGroup))))
    return art->next;

  a = art;
  art = art->next;
  while (art) {
    if (!isread (tempNewsGroupS, art->num))
      break;
    art = art->next;
  }

  if (probe && !art && !newsNoThreadJumping)
    art = nextUnreadThread (a);

  return art;
}

/* Return first unread thread in list */
NewsArt *firstUnreadThread (NewsArt *art) 
{
  NewsArt *t, *a;

  if (!art) 
    return NULL;

  if (!newsUseNewsRC || newsShowAllArticles || !newsNextIsUnread)
    return art;

  while (art && art->prev) art=art->prev;
  t = art;
  while (t) {
    if ((a = firstUnread (t)) != NULL)
      return a;
    t = t->nextt;
  }
  return NULL;
}

/* Return next unread thread in list */
NewsArt *nextUnreadThread (NewsArt *art) 
{
  NewsArt *t;

  if (!art)
    return NULL;

  while (art && art->prev) art=art->prev;
  if (!newsUseNewsRC || newsShowAllArticles || !newsNextIsUnread)
    return art?art->nextt : NULL;

  t = art->nextt;
  while (t) {
    if (art = firstUnread (t))
      return art;
    t = t->nextt;
  }
  return t;
}


NewsArt *prevUnread (NewsArt *art, int probe)
{
  NewsArt *a;
  newsgroup_t *tempNewsGroupS = NULL;

  if (!art)
    return NULL;

  if (!newsUseNewsRC || newsShowAllArticles || !newsPrevIsUnread ||
      (!(tempNewsGroupS = findgroup (NewsGroup))))
    return art->prev;

  a = art;
  art = art->prev;
  while (art) {
    if (!isread (tempNewsGroupS, art->num))
      break;
    art = art->prev;
  }

  if (probe && !art && !newsNoThreadJumping)
    art = prevUnreadThread (a);

  return art;
}

NewsArt *prevUnreadThread (NewsArt *art)
{
  NewsArt *t;

  if (!art)
    return NULL;
  while (art && art->prev) art=art->prev;
  if (!newsUseNewsRC || newsShowAllArticles || !newsPrevIsUnread)
    return art->prevt;

  t = art->prevt;
  while (t) {
    if (art = firstUnread (t))
      return art;
    t = t->prevt;
  }
  return t;
}


/* Goto the previous (unread) thread */
void news_prevt(char *url)
{	
  NewsArt *art, *p;
  
  if (art = is_news_url (url)) {
    if ((p = prevUnreadThread (art)) != NULL) {
      sprintf (url, "news:%s", p->ID);
      return;
    }
  } 
  url[0] = 0;
  return;
}

/* Goto first (unread) article in next (unread) thread */
void news_nextt(char *url)
{
  NewsArt *art, *p;
  
  if ((art = is_news_url(url)) != NULL) {
    if ((p=nextUnreadThread (art))) {
      sprintf (url, "news:%s", p->ID);
      return;
    }
  }
  url[0] = 0;
  return;
}


/* Goto the previous (unread) article */
void news_prev(char *url)
{
    NewsArt *art, *p;

    if ((art = is_news_url(url)) == NULL) { 
      url[0] = 0;
      return;
    }

    url[0] = 0;
    if ((p = prevUnread (art,0)) != NULL) {
      sprintf (url, "news:%s", p->ID);
    } else if (!newsNoThreadJumping) {
      if ((p=prevUnreadThread (art))) {
	sprintf (url, "news:%s", p->ID);
      }
    }
    return;
}

/* Goto next (unread) article in this thread */
void news_next(char *url)
{
  NewsArt *art, *p;

  if ((art = is_news_url(url)) == NULL) {
    url[0] = 0;
    return;
  }
  url[0] = 0;
  if ((p=nextUnread (art, 0))) {
    sprintf (url, "news:%s", p->ID);
  } else if (!newsNoThreadJumping) {
    if ((p = nextUnreadThread (art))) {
      sprintf (url, "news:%s", p->ID);
    }
  }
  return;
}

void news_index(char *url)
{
    if(NewsGroup && is_news_url(url))
 	sprintf(url,"news:%s",NewsGroup);
    else
	url[0] = 0;
}


/* Returns the status of the news buttons */
void news_status(char *url, int *prevt, int *nextt, int *prev, int *next, int *follow)
{
    NewsArt *art,*tmp;

    if( art = is_news_url(url) ) {
      if(prevUnread(art,!newsNoThreadJumping)) 
	*prev = 1;
      else 
	*prev = 0;
      
      if(prevUnreadThread(art)) 
	*prevt = 1;
      else
	*prevt = 0;
      
      if (nextUnread (art,!newsNoThreadJumping))
	*next = 1;
      else
	*next = 0;
      
      if (nextUnreadThread (art))
	*nextt = 1;
      else
	*nextt = 0;

      *follow = 1;
    } else {
      *follow=0;
      *prevt=0;
      *nextt=0;
      *next=0;
      *prev=0;
    }

    return;
}


/* makespaces ()
   Expects: str -- a string to figure out the number of spaces.
            len -- number of spaces to pad to.

   Returns: pointer to a static spaces string, each call to make spaces will
            overwrite this buffer.
   
   Notes: this takes the string in str and makes a string of spaces that will
          (when concatenated with str) form a string len spaces long.
*/
char *makespaces (char *str, int len)
{
  static char spaces[300+1];
  char *p;
  int l = strlen (str);

  if (l < len) {
    p = spaces;
    len -= l;
    while (len--) {
      *p = ' ';
      p++;
    }
    *p = 0;
  } else if (l > len) { 
    spaces[0] = 0;
  }
  return spaces;
}

		
/*	Read in an Article					read_article
**	------------------
**
**
**	Note the termination condition of a single dot on a line by itself.
**	RFC 977 specifies that the line "folding" of RFC850 is not used, so we
**	do not handle it here.
**
** On entry,
**	s	Global socket number is OK
**	HT	Global hypertext object is ready for appending text
*/       
PRIVATE void read_article ARGS1 (char *, artID)
{
    int i;
    int linecount=0,linenum=1,lineinc=0;
    char line[LINE_LENGTH+1];
    char buf[LINE_LENGTH+1], duff[LINE_LENGTH+1];
    char *references=NULL;			/* Hrefs for other articles */
    char *newsgroups=NULL;			/* Newsgroups list */
    char *from=NULL,*subj=NULL,*org=NULL,*date=NULL;
    char *filename;
    char *l = line;
    int f; /* ':' flag */
    int decode=0; /*uudecoding...*/
    FILE *fp;
        
    char *p = line,*pp,*m;
    BOOL done = NO;

    NewsArt *art,*art2,*art_t, *next;
    int ll;
    

    HTMeter(0,NULL);
    
    ll= strlen(artID)-3; /* ">\n\r" should be stripped outside !!! */
    for(art = FirstArt; art; art = art -> nextt){
      if(!strncmp(art->ID,artID,ll)) break;
      if(art->next){
	for(art2 = art->next; art2; art2 = art2->next)
	  if(!strncmp(art2->ID,artID,ll)) break;
	if(art2) {
	  art = art2;
	  break;
	}
      }
    }
    
    if(art) {
      CurrentArt = art;
    } else {
      CurrentArt = NULL;
    }
    
/*	Read in the HEADer of the article:
**
**	The header fields are either ignored, or formatted 
**      and put into the text.
*/
    while(!done){
      char ch = *p++ = HTGetCharacter ();
      if (ch==(char)EOF) {
	abort_socket();	/* End of file, close socket */
	return;		/* End of file on response */
      }
      if ((ch == LF) || (p == &line[LINE_LENGTH])) {            
	*--p=0;				/* Terminate the string */
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr, "H %s\n", line);
#endif
	if(line[0]<' ') {
	  done = 1;
	} else {
	  switch(line[0]) {
	  case '.':
	    if (line[1]<' ') 
	      done = 1;
	    break;
	  case 'S':
	  case 's':
	    if(match(line, "SUBJECT:")) 
	      subj = strdup(&line[8]);
	    break;
	  case 'D':
	  case 'd':
	    if(match(line, "DATE:")) 
	      date = strdup(&line[5]);
	    break;
	  case 'L':
	  case 'l':
	    if(match(line,"LINES:"))
	      linecount = atoi(&line[6]);
	    break;
	  case 'F':
	  case 'f':
	    if(match(line, "FROM:")) 
	      from = strdup(&line[5]);
	    break;
	  case 'O':
	  case 'o':
	    if(match(line, "ORGANIZATION:")) 
	      org = strdup(&line[13]);
	    break;
	  case 'N':
	  case 'n':
	    if(match(line, "NEWSGROUPS:")) 
	      newsgroups = strdup(&line[11]);
	    break;
	  case 'R':
	  case 'r':
	    if(match(line, "REFERENCES:")) 
	      references = strdup(&line[11]);
	    break;
	  default:
	    
	    /* unknown headers ignored */
	    break;
	  }
	}
	
	p = line;			/* Restart at beginning */
      } /* if end of line */
    } /* Loop over characters */
    
    if(subj) {
      PUTS("<H2>");
      PUTS(subj);
      PUTS("</H2>\n");
      START (HTML_TITLE);
      sprintf (buf, "Article: %s", subj);
      PUTS (buf);
      END (HTML_TITLE);
      free(subj);
    }
    if(date) {
      PUTS("<I>");
      PUTS(date);
      free(date);
      if(org) {
	PUTS(", ");
	PUTS(org);
	free(org);
      }       
      PUTS("</I><BR>\n");
    }
    if(from) {
      PUTS("<B>From:</B> <I>");
      if (parseemail (from,duff,buf)) {
	sprintf (line, "<A HREF=\"mailto:%s\"> %s </A>   ", buf, duff);  
	PUTS (line);
      } else 
	PUTS (from);
      PUTS("</I><BR>");
      free(from);
    }
    if(newsgroups) {
      PUTS("<B>Newsgroups:</B> <I>");
      write_anchors(newsgroups);
      PUTS("</I><BR>\n");
    }
    if(references){
      PUTS("<B>References:</B> <I>");
      i = 1;
      for(p = references; *p; p++) {
	if(*p=='<') {
	  for(pp = ++p; *p; p++) {
	    if(*p=='>') {
	      *p=0;
                        p++;
                        if(strlen(pp)<LINE_LENGTH) {
			  if(i>1)
			    PUTS(", ");
			  sprintf(line,"<A HREF=\"news:&lt;%s&gt;\">%d</A>",
				  pp,i);
			  PUTS(line);
			  i++;
                        }
                        break;   
	    }
	  }
	  if(!*p) break;
            }
      }
      free(references);
      PUTS("</I><BR>\n");
    }
    
    if(linecount) {
      lineinc = linecount/100;
      if(lineinc < 1) lineinc = 1;
    }
    
    PUTS("<HR>\n");
    
    /*	Read in the BODY of the Article:
     */
    (*targetClass.start_element)(target, HTML_PRE , 0, 0);
    
    p = line;
    done = 0;
    while(!done){
      char ch = *p++ = HTGetCharacter ();
      if (ch==(char)EOF) {
	if(decode) {
	  fclose(fp);
	}
	abort_socket();	/* End of file, close socket */
	return;		/* End of file on response */
      }
      if ((ch == LF) || (p == &line[LINE_LENGTH])) {
	    *p++=0;				/* Terminate the string */
#ifndef DISABLE_TRACE
	    if (www2Trace) fprintf(stderr, "B %s", line);
#endif
	    if (line[0]=='.') {
	      if (line[1]<' ') {		/* End of article? */
		done = YES;
		switch(decode){
		case 1:
		  uudecodeline(fp,NULL);
		  sprintf(line,"%s\n%s",filename,filename);
		  ImageResolve(NULL,line,0);
		  PUTS("<BR><IMG SRC=\"");
		  PUTS(filename);
		  PUTS("\"><BR>");
                        break;
		case 2:
		  base64line(fp,NULL);
		  sprintf(line,"%s\n%s",filename,filename);
		  ImageResolve(NULL,line,0);
		  PUTS("<BR><IMG SRC=\"");
		  PUTS(filename);
		  PUTS("\"><BR>");
		  break;
                    }
		break;
	      }
            }
            linenum++;
            if(linecount && !(linenum%lineinc)) {
	      HTMeter((linenum*100)/(linecount),NULL);            
            }
            switch(decode) {
            case 1:                        
	      /* uuencoded */
	      if(uudecodeline(fp,line)){
		decode=6;
		sprintf(line,"%s\n%s",filename,filename);
		ImageResolve(NULL,line,0);
		PUTS("<BR><IMG SRC=\"");
		PUTS(filename);
                    PUTS("\"><BR>");
	      }
	      f++;
	      p = line;
	      continue;
            case 2:
	      /* base64 encoded */
	      if(base64line(fp,line)){
		decode=6;
		sprintf(line,"%s\n%s",filename,filename);
		ImageResolve(NULL,line,0);
		PUTS("<BR><IMG SRC=\"");
		PUTS(filename);
		PUTS("\"><BR>");
	      }
	      p = line;
	      continue;                       
            case 3:
	      /* is mime, looking for encoding... */
	      if(match(line,"CONTENT-TRANSFER-ENCODING: BASE64")){
		decode = 4;
		HTProgress("base64 image decoding");
	      } else {
		if(!line[0] || isspace(line[0])) {
		  decode = 0; /* possible begin */
		}
	      }
	      break;
            case 4:
	      /* base64, looking for blank start line */
	      if(!line[0] || isspace(line[0])) {
		fp = startuudecode(filename = mo_tmpnam(NULL));
		decode=2;
		p=line;
		continue;
	      }
	      break;
	      
            case 5: /* possible mime encap encoding crud */
	      if(match(line,"CONTENT-TYPE: IMAGE")) {
		decode = 3;
		break;
	      } else {   
		decode = 0;
	      }
	      break;
            case 6: /* reg text, don't search */
	      break;
            default: /* regular text, look for encoding start tags */
	      if(match(line,"CONTENT-TYPE: IMAGE")){
		decode = 3;
		break;
	      }
	      if(!strncmp(line,"begin",5)){    
		decode=1;
		fp = startuudecode(filename = mo_tmpnam(NULL));
		HTProgress("uudecoding image data...");
		p = line;
		continue;
	      }
            }
            
	    /* HTTP, FTP, MAILTO, GOPHER,  */
            for(f=0, l=pp=line;*pp;pp++) { 
	      if(f) {
		if(isspace(*pp) || (*pp=='"') || (*pp=='<') || (*pp=='>')){
		  ll = *pp;
		  *pp=0;
		  PUTS("<A HREF=\"");
		  PUTS(l);
		  PUTS("\">");
		  PUTS(l);
		  PUTS("</A>");
		  *pp=ll;
		  l = pp;
		  f=0;
		}
	      }           
	      if(*pp=='<') {
		*pp=0;
		PUTS(l);
		PUTS("&lt;");
		l=pp+1;
		continue;
	      }
	      if(*pp=='>') {
		*pp=0;
		PUTS(l);
		PUTS("&gt;");
		l=pp+1;
		continue;
	      }
	      if(*pp==':') {
		m = pp;
		while(m > l) {
		  m--;
		  if(!isalpha(*m)) {
		    m++;
		    break;
		  }
		}
		if((pp-m)>2) {
		  if(match(m,"HTTP:") || match(m,"FTP:") ||
		     match(m,"MAILTO:") || match(m,"NEWS:") ||
		     match(m,"GOPHER")) {
		    ll=*m;
		    *m=0;
		    PUTS(l);
		    f = 1;
		    *m=ll;
		    l=m;
		  }
		}
		continue;
	      }                        
            }
            PUTS(l);
#ifdef OLD            
            for(f=1,l=pp=line;*pp;pp++){
	      if(*pp == '<'){
		if(strchr(pp,'>')){
		  f = 0;
		} else {
		  *pp = 0;
		  PUTS(l);
		  PUTS("&lt;");
		  l=pp+1; /* step over */
		}
	      }
	      if(*pp == '>'){
		if(f){
		  *pp = 0;
		  PUTS(l);
		  PUTS("&gt;");
		  l=pp+1; /* step over */
		}
                }
            }
            PUTS(l);	/* Last bit of the line */
#endif
            p = line;				/* Restart at beginning */
      }
      
    } /* Loop over characters */
    
    (*targetClass.end_element)(target, HTML_PRE);
    
    /* Mark this article read in all the groups we care about 
       Also figure out the next article to read 
     */
   
    if ((next = nextUnread (CurrentArt,0)) == NULL) {
      if ((next = nextUnreadThread (CurrentArt)) == NULL) {
	next = CurrentArt;
	while (next && next->prev) next=next->prev;
	while (next && next->prevt) next=next->prevt;
	next = firstUnread (next);
      }
    }

    NextArt = next;
    if (CurrentArt) {
      char *tok, d, *last;
      newsgroup_t *ng;

      if (newsgroups) {
	tok = strtok (newsgroups, ", ;:\t\n");
	while (tok) {
	  if (ng = findgroup (tok)) {
	    markread (ng, CurrentArt->num);
	  }
	  tok = strtok (NULL, ", ;:\t\n");
	}
      } else if (NewsGroup) {
	if (ng = findgroup (NewsGroup)) { 
	  markread (ng, CurrentArt->num);
	}
      }
    }
      
    HTMeter(100,NULL);
}



/* read_list ()
   Expects: Nothing.
   Returns Nothing.

   Notes:
   pre 2.7b4: Note the termination condition of a single dot on a line by itself.
	      RFC 977 specifies that the line "folding" of RFC850 is not used, so we
	      do not handle it here.
       2.7b4: Added support for newsrc and subscribed news.
       2.7b5: Made it faster.
*/        
PRIVATE void read_list NOARGS
{
  char line[LINE_LENGTH+1], group[LINE_LENGTH], elgroup[LINE_LENGTH], 
    postable, *p;
  int first, last, junk, m=0, next_m=20, done=0, intr, g=0, next_g = 50, l=0, lastg=0,mark=0;  
  newsgroup_t *n=NULL, *nn=NULL;
  extern int twirl_increment;

  START (HTML_TITLE);
  PUTS ("Newsgroup Listing");
  END (HTML_TITLE);
    
  HTMeter (0,NULL);
  HTProgress ("Getting newsgroup information from NNTP server");

  /* Display our list of groups */
  START (HTML_H1);
  PUTS ("Newsgroup Listing");
  END (HTML_H1);

  START (HTML_PRE);
  if (newsNoNewsRC || !newsUseNewsRC || newsShowAllGroups) {

    if (response ("LIST\r\n") < 0) {
      START (HTML_H1);
      PUTS ("Error retrieving newsgroup listing from server");
      END (HTML_H1);
      NETCLOSE (s);
      s = -1;
      END (HTML_PRE);
      return;
    }

    p = line;
    while(!done){
      char ch = *p++ = HTGetCharacter ();

      if (ch==(char)EOF) {
	abort_socket();	/* End of file, close socket */
	return;		/* End of file on response */
      }

      if ((ch == LF) || (p == &line[LINE_LENGTH])) {
	*p++=0;				/* Terminate the string */
	/* Do globe twirly */
	if (g++>next_g) {
	  next_g = g+50;
	  intr = HTCheckActiveIcon(1);
	} else {
	  intr = HTCheckActiveIcon(0);
	}
	
	if (intr) {
	  HTProgress ("Transfer Interrupted");
	  return;
	}

	/* Do progress meter */
	if (m++ > next_m) {
	  next_m = m+20;
	  /* Attempt to estimate where we are at in the big list */
	  if ((m*100/5000) < 100)
	    HTMeter (m*100/8000,NULL);
	  else 
	    HTMeter (99, "99%");
	}

	/* Check for end of transfer */
	if (line[0] == '.') {
	  if (line[1] < ' ') {		/* End of list */
	    done = 1;
	    break;
	  } else {			/* Line starts with dot */
	    PUTS (&line[1]);
	  }
	} else {
	  /*  Normal lines are scanned for references to newsgroups. */
	  if (sscanf(line, "%s %d %d %c", group, &last, &first, &postable) == 4) {
	    /* Make a short version of the group name */
	    if (compact_string (group, elgroup, newsSubjWidth, 3, 3))
	      strcpy (elgroup, group);

	    n = findgroup (group);
	    if (!LastGroup)
	      LastGroup = n;
	    if (n) {   
	      if (LastGroup == n)
		lastg = 1;

	      if (!(n->attribs & naSEQUENCED)) {
		setminmax (n, first, last); /* Update sequencer info */
		rereadseq (n);
		n->attribs |= naSHOWME;
		n->attribs |= naSEQUENCED;
	      }
	      if (postable == 'y')
		n->attribs |= naPOST;
	      
      	      if (n->attribs&naSUBSCRIBED && 
		  (newsShowAllGroups || n->unread>0 
		   || newsShowReadGroups)) {
		sprintf(line,"%s % 7d S <A HREF=\"news:%s\">%s</A> \n",
			(lastg==1)?"<b>&gt;&gt;&gt;</b>":"   ",
			n->unread, n->name, elgroup);
		PUTS (line);
		if (lastg==1)
		  lastg=2;
	      } 
	    } else {
	      sprintf(line,"    % 7d U <A HREF=\"news:%s\">%s</A> \n",
		      last<first?0:last-first, group, elgroup);
	      PUTS(line);
	    }
	    l++;
	  }
	} /* if not dot */
	p = line;			/* Restart at beginning */
      }
    } /* Loop over characters */

  } else { /* Pull down info on our subscribed groups. */

    nn = NULL;
    n = firstgroup (naSUBSCRIBED);
    while (n) {
      if (n->unread > 0 || newsShowAllGroups)
	nn = n;
      n = nextgroup (n);
    }

    n = firstgroup (naSUBSCRIBED);
    if (!LastGroup)
      LastGroup = n;
    while (n) {

      /* Make a short version of the group name */
      compact_string(n->name, elgroup, newsSubjWidth, 3, 3);
      /* contact the server about this group */
      sprintf (line, "GROUP %s\r\n", n->name);
      if ((first = response (line)) != 211) {
	sprintf(line,"??????? ? %s <I>Group not found on server </I>\n", elgroup);
	PUTS(line);
	n = nextgroup (n);
	continue;
      }

      /* Reset the sequencer data and set some flags  for this group */
      sscanf (response_text, "%d %d %d %d", &junk, &junk, &first, &last);
      if (!(n->attribs & naSEQUENCED)) {
	setminmax (n, first, last); /* Update sequencer info */
	rereadseq (n);
	n->attribs |= naSHOWME;
	n->attribs |= naSEQUENCED;
      }

      
      if (LastGroup == n && (n->unread>0 || newsShowAllGroups)) 
	lastg = 1;
      else if (LastGroup == n)
	lastg = 2;
      else if (nn == n && !mark)
	lastg = 1;
      if (newsShowAllGroups  || n->unread>0 || newsShowReadGroups) {
	sprintf(line,"%s % 7d %s <A HREF=\"news:%s\">%s</A> \n",
		(lastg==1)? "<b>&gt;&gt;&gt;</b>":"   ",
		n->unread, n->attribs&naSUBSCRIBED?"S":"U", 
		n->name, elgroup);
	PUTS(line);
	l++;
	if (lastg == 1)
	  mark = 1;
	lastg--;
      } 
      n = nextgroup (n);
    }
  }

  if (!l) {
    sprintf (line, "No %snewsgroups on server\n", newsShowAllGroups?"":"unread ");
    PUTS (line);
  }
  HTMeter (100,NULL);
  END (HTML_PRE);
}





int parsexover(char *x, char **num, char **title, char **from,
	       char **date, char **msgid, char **ref, char **bytes, char **lines)
{
  *num = x;

  while(*x && *x != '\t') x++;      /* step to next tab */
  if(!*x) return 0;                 /* early end of string - bad record */
  *x = 0;                           /* terminate */
  x++;                              /* bump to start of next field */
  *title = x;

  while(*x && *x != '\t') x++;      /* step to next tab */
  if(!*x) return 0;                 /* early end of string - bad record */
  *x = 0;                           /* terminate */
  x++;                              /* bump to start of next field */
  *from = x;

  while(*x && *x != '\t') x++;      /* step to next tab */
  if(!*x) return 0;                 /* early end of string - bad record */
  *x = 0;                           /* terminate */
  x++;                              /* bump to start of next field */
  *date = x;

  while(*x && *x != '\t') x++;      /* step to next tab */
  if(!*x) return 0;                 /* early end of string - bad record */
  *x = 0;                           /* terminate */
  x++;                              /* bump to start of next field */
  *msgid = x;

  while(*x && *x != '\t') x++;      /* step to next tab */
  if(!*x) return 0;                 /* early end of string - bad record */
  *x = 0;                           /* terminate */
  x++;                              /* bump to start of next field */
  *ref = x;
  
  while(*x && *x != '\t') x++;      /* step to next tab */
  if(!*x) return 0;                 /* early end of string - bad record */
  *x = 0;                           /* terminate */
  x++;                              /* bump to start of next field */
  *bytes = x;

  while(*x && *x != '\t') x++;      /* step to next tab */
  if(!*x) return 0;                 /* early end of string - bad record */
  *x = 0;                           /* terminate */
  x++;                              /* bump to start of next field */
  *lines = x;

  while(*x && *x != '\t') x++;      /* step to next tab */
  *x = 0;                           /* terminate */

  return 1;
}

PRIVATE void XBuildArtList ARGS3(
  WWW_CONST char *,groupName,
  int,first_required,
  int,last_required
)
{
    NewsArt *art;
    char *p,*aname=NULL, *author=NULL, *aref, abuf[1024+1];
    BOOL done;

    char *num,*title,*date,*msgid,*ref,*bytes,*lines,*from=NULL;

    char buf[2048];

    char *reference=0;			/* Href for article */
    int status, count, first, last;	/* Response fields */
					/* count is only an upper limit */
    int c,i,lineinc;

    HTMeter(0,NULL);
    
#ifndef DISABLE_TRACE
    if(www2Trace) fprintf(stderr,"[%s]\n",response_text);
#endif
    sscanf(response_text, "%d %d %d %d", &status, &count, &first, &last);
#ifndef DISABLE_TRACE
    if(www2Trace) fprintf(stderr,"Newsgroup status=%d, count=%d, (%d-%d)",
    			status, count, first, last);
#endif
    Count = 0;

    if(NewsGroup && (strlen(NewsGroup)==strlen(groupName)) && !strcmp(NewsGroup,groupName)){
	last_required = last;
	first_required = ReadLast +1;
    } else {
	first_required = first;
	last_required = last;
	ClearArtList();

	StrAllocCopy(NewsGroup, groupName);
    }

    GroupFirst = first;
    GroupLast = last; 
    ReadLast = last;

    if (first_required<GroupFirst) 
 	first_required = GroupFirst;     
    if ((last_required==0) || (last_required > GroupLast)) 
	last_required = GroupLast;

    if(first_required > last_required) return;

    /* FLUSH!!! */
    HTInitInput(s);

    sprintf(buf, "XOVER %d-%d\r\n", first_required, last_required);
    if(response(buf) != 224) return; 

    HTProgress("Threading Articles");

    lineinc = count/100;
    if(lineinc < 1) lineinc = 1;
    
    
    for(;;){
        if(!(Count%lineinc) && count) {
            HTMeter((Count*100)/(count),NULL);            
        }
	
	/* EOS test needed */
	for(p = buf;*p = HTGetCharacter();p++){
	    if(*p=='\r' || *p=='\n'){
		*p = 0;
		break;
	    }
	    if(*p == (char)EOF){
		abort_socket();	/* End of file, close socket */
		return;		/* End of file on response */
	    }
	}

	if(buf[0]=='.') break; /* end of list */

	art = NewArt();

	parsexover(buf,&num,&title,&from,&date,&msgid,&ref,&bytes,&lines);

	art->num = atoi(num);
	Count++;
	     	
	StrAllocCopy(art->SUBJ, title);
	
	if(ref[0]){
	    p = ref;
	    aref = p;
	    if(*p=='<'){
		while(*aref && *aref!='>') aref++;
		aref++;
		*(aref-1) = 0;
		StrAllocCopy(art->FirstRef, p+1);
	    }
	    do aref++; while(*aref);
	    p = aref-1;
	    while(*p) {
		if(*p == '>'){
		    *p = 0;
		    while(*--p && *p != '<');
		    if(*p=='<')
			StrAllocCopy(art->LastRef,p+1);
		    break;
		}
		p--;
	    }
	}
	
	msgid++; /* Chop < */
	msgid[strlen(msgid)-1]=0;		/* Chop > */
	StrAllocCopy(art->ID, msgid);

	parseemail (from, abuf, NULL);
	aname = abuf;
	if (aname && *aname){
	    StrAllocCopy(art->FROM, aname);
	    p = art->FROM + strlen(art->FROM) - 1;
	    if (*p==LF) *p = 0;	/* Chop off newline */
	} else {
	    StrAllocCopy(art->FROM, "Unknown");
	}
	AddArtTop(art);

    }

    HTMeter(100,NULL);
    
    HTProgress("Done Threading Articles");
}


PRIVATE void BuildArtList ARGS3(
  WWW_CONST char *,groupName,
  int,first_required,
  int,last_required
)
{
    NewsArt *art;
    char *p,*aname, *author, *aref, abuf[1024+1];
    BOOL done;

    char buffer[LINE_LENGTH];
    char line[LINE_LENGTH];
    char *reference=0;			/* Href for article */
    int artno;				/* Article number WITHIN GROUP */
    int status, count, first, last;	/* Response fields */
					/* count is only an upper limit */

    int *artlist,c,i;

#ifndef DISABLE_TRACE
    if(www2Trace) fprintf(stderr,"[%s]",response_text);
#endif
    sscanf(response_text, "%d %d %d %d", &status, &count, &first, &last);
#ifndef DISABLE_TRACE
    if(www2Trace) fprintf(stderr,"Newsgroup status=%d, count=%d, (%d-%d)",
    			status, count, first, last);
#endif

    if(NewsGroup && (strlen(NewsGroup)==strlen(groupName)) && !strcmp(NewsGroup,groupName)){
	last_required = last;
	first_required = ReadLast +1;
    } else {
	ClearArtList();
	StrAllocCopy(NewsGroup, groupName);
    }


    if (first_required<GroupFirst) 
	first_required = GroupFirst;           /* clip */
    if ((last_required==0) || (last_required > GroupLast)) 
	last_required = GroupLast;

/*	Read newsgroup using individual fields:
*/
    c = 0; 
    artlist = NULL;

    if(count){
	if(!(artlist = (int *) malloc(sizeof(int) * (count+2))))
	    outofmem(__FILE__, "BuildArtList");
	
	if(response("listgroup\r\n") != 211){
	    /* try XHDR if LISTGROUP fails */
	    /* thanks to Martin Hamilton for this bit 'o code...
               his choice of header, not mine 
            */
	    sprintf(buffer, "xhdr anarchy-in-the-uk %d-%d\r\n", first, last);
 
	    if(response(buffer) != 221) {
		HTProgress("Cannot get article list from news server");    
		return;
	    }
	}

	/* read the list of available articles from the NNTP server */
	artlist[0]=0;
	while(c<(count+2)){
	    char ch = HTGetCharacter ();
	    
	    if (ch==(char)EOF) {
		abort_socket();	/* End of file, close socket */
		return;		/* End of file on response */
	    }
	    if(ch == '.') break;
	    if(ch == LF){
#ifndef DISABLE_TRACE
		if(www2Trace) fprintf(stderr,"[%d]",artlist[c]);
#endif
		c++;
		artlist[c]=0;
	    } else {
		if (isdigit(ch))
		    artlist[c] = artlist[c]*10 + ch-'0';	   
	    } 
	} /* Loop over characters */
    }


    for(i=0;i<c;i++){
	artno = artlist[i];

	if(artno <= GroupLast) 
	  continue;
	/* FLUSH!!! */
	HTInitInput(s);
	sprintf(buffer, "HEAD %d%c%c", artno, CR, LF);
	status = response(buffer);
	
	/*	fprintf(stderr,"%d:[%d]:%s\n",artno,status,buffer);*/
	
	if (status == 221) {	/* Head follows - parse it:*/
	  
	  art = NewArt();
	  art->num = artno;
	  Count++;
	  
	  if(!(Count % 25) ) {
	    sprintf(buffer, "Threading Article %d of %d",Count,count);
	    HTProgress (buffer);
	  }
	  
	  if(!ReadFirst || artno<ReadFirst) ReadFirst = artno;
	  if(!ReadLast || artno>ReadLast) ReadLast = artno;
	  
	  p = line;				/* Write pointer */
	  done = NO;
	  while(!done){
	    char ch = *p++ = HTGetCharacter ();
	    if (ch==(char)EOF) {
	      abort_socket();	/* End of file, close socket */
	      return;		/* End of file on response */
	    }
	    
	    if ((ch == LF) || (p == &line[LINE_LENGTH]) ) {
	      
	      *--p=0;		/* Terminate  & chop LF*/
	      p = line;		/* Restart at beginning */
#ifndef DISABLE_TRACE
	      if (www2Trace) fprintf(stderr, "G %s\n", line);
#endif
	      switch(line[0]) {
		
	      case '.':
		done = (line[1]<' ');	/* End of article? */
		break;
		
	      case 'S':
	      case 's':
		if (match(line, "SUBJECT:"))
		  StrAllocCopy(art->SUBJ, line+9);/* Save subject */
		break;
		
	      case 'R':
	      case 'r':
		if (match(line, "REFERENCES:")) {
		  p = line + 12;
		  aref = p;
		  if(*p=='<'){
		    while(*aref && *aref!='>') aref++;
		    aref++;
		    *(aref-1) = 0;
		    StrAllocCopy(art->FirstRef, p+1);
		  }
		  do aref++; while(*aref);
		  p = aref-1;
		  while(*p) {
		    if(*p == '>'){
		      *p = 0;
		      while(*--p && *p != '<');
		      if(*p=='<')
			StrAllocCopy(art->LastRef,p+1);
		      break;
		    }
		    p--;
		  }
		}
		break;
		
	      case 'M':
	      case 'm':
		if (match(line, "MESSAGE-ID:")) {
		  char * addr = HTStrip(line+11) +1; /* Chop < */
		  addr[strlen(addr)-1]=0;		/* Chop > */
		  StrAllocCopy(art->ID, addr);
		}
		break;
		
	      case 'f':
	      case 'F':
		if (match(line, "FROM:")) {
		  parseemail (strchr(line,':')+1,abuf,NULL);
		  aname = abuf;
		  if (aname && *aname)
		    {
		      StrAllocCopy(art->FROM, aname);
		      p = art->FROM + strlen(art->FROM) - 1;
		      if (*p==LF) *p = 0;	/* Chop off newline */
		    }
		  else
		    {
		      StrAllocCopy(art->FROM, "Unknown");
		    }
		}
		break;
		
	      } /* end switch on first character */
	      
	      p = line;		/* Restart at beginning */
	    } /* if end of line */
	  } /* Loop over characters */
	  AddArtTop(art);

	  /*	 indicate progress!   @@@@@@
	   */
	  
	} /* If good response */
    } /* Loop over article */	 
    
    if(artlist) free(artlist);
    
    GroupLast = last;
    GroupFirst = first;
    
    HTProgress("Done Threading Articles");
}




/*	Read in a Newsgroup
**	-------------------
**	Unfortunately, we have to ask for each article one by one if we
**	want more than one field.
**
*/
PRIVATE void read_group ARGS3(
  WWW_CONST char *,groupName,
  int,first,
  int,last
)
{
    NewsArt *art,*art2, *f;
    char *p;
    BOOL done;
    char buffer[LINE_LENGTH], subj[LINE_LENGTH];
    char line[LINE_LENGTH], from[LINE_LENGTH];
    char efrom[LINE_LENGTH], esubj[LINE_LENGTH];
    char *reference=0;			
    int i,artno, mark=0;			
    int tc=0;
    int status, count;	                 /* Response fields */
			 /* count is only an upper limit */
    char em[255+1];
    
    if ((NewsGroupS = findgroup (groupName)) == NULL) {
      /* Add group unsub'd to hash table */
      if((NewsGroupS = addgroup (groupName, first, last, 1))==NULL) {
	sprintf (buffer, "\nMosaic appears to be out of memory.\n");
	PUTS (buffer);
	return;
      }
      NewsGroupS->attribs |= naUPDATE;
    } 

    if (NewsGroup) {
      free (NewsGroup);
      NewsGroup = strdup (groupName);
    }

    if ((f = firstUnreadThread (FirstArt)) == NULL) {
      sprintf (buffer, "\nNo %sarticles in this group.\n", 
	       newsShowAllArticles?"":"unread ");
      markrangeread (NewsGroupS, NewsGroupS->minart, NewsGroupS->maxart);
      PUTS (buffer);
      return;
    }
    
    /*	Set window title */
    START (HTML_TITLE);
    sprintf (buffer, "Newsgroup: %s", groupName);
    PUTS (buffer);
    END (HTML_TITLE);
    

    /* If !ConfigView then the format is:

	   [THREADCOUNT] <A HREF="news:msgID"> SUBJECT </A>

       otherwise:

	   <A HREF="news:msgID"> SUBJECT </A>        Author One's Name
	   <A HREF="news:msgID2"> SUBJECT </A>       Author Two's Name
	   <A HREF="news:msgID2"> SUBJECT </A>       Author Three's Name
    */

    /* nextUnreadThread determines the next thread to go to.
       it returns the next article in the next thread 
       It looks at a whole bunch of globals to see what the next article
       should be ...
    */

    if (!CurrentArt)
      NextArt = f;
    START(HTML_PRE);
    for(art=f; art; art=nextUnreadThread(art)) {

      compact_string (art->SUBJ, subj, newsSubjWidth, 3,3);
      compact_string (art->FROM, from, newsAuthWidth, 3,3);
      escapeString (subj, esubj);
      escapeString (from, efrom);

      if(!ConfigView) { /* Thread view */
	/* Get article count */
	for (i=0,art2=art; art2; art2=nextUnread(art2,0)) {
	  if (NextArt == art2)
	    mark = 1;
	  if (!newsShowAllArticles && isread (NewsGroupS,art2->num))
	    continue;
	  i++;
	}
	if (!i && !newsShowAllArticles) 
	  continue;
	/* Write summary */
	sprintf (buffer, "%s % 4d <A HREF=\"news:%s\">%s</A>%s %s\n", 
		 (mark==1)?"<b>&gt;&gt;&gt;</b>":"   ", i, art->ID, esubj, 
		 makespaces(subj,newsSubjWidth), 
		 efrom);
	PUTS (buffer);
	if (mark==1)
	  mark=2;
      } else {
	/* Write out subject info for each article */
	for(art2=art;art2;art2=nextUnread(art2,0)) {
	  if (!newsShowAllArticles && isread (NewsGroupS,art2->num))
	    continue;
	  if (NextArt == art2)
	    mark = 1;
	  compact_string (art2->SUBJ, subj, newsSubjWidth, 3,3);
	  compact_string (art2->FROM, from, newsAuthWidth, 3,3);
	  escapeString (subj, esubj);
	  escapeString (from, efrom);
	  sprintf (buffer, "%s      <A HREF=\"news:%s\">%s</A>%s %s\n", 
		   (mark==1)?"<b>&gt;&gt;&gt;</b>":"   ", art2->ID, esubj, 
		   makespaces(subj,newsSubjWidth), 
		   efrom);
	  PUTS (buffer);
	  if (mark==1)
	    mark=2;
	}
      }
    }
    END (HTML_PRE);
    sprintf (buffer, "Done listing %s", NewsGroup?NewsGroup : "newsgroup");
    HTProgress (buffer);
    LastGroup = NewsGroupS;
    return;
}




/*		Load by name					HTLoadNews
**		============
*/
PUBLIC int HTLoadNews ARGS4(
	WWW_CONST char *,		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		stream)
{
  char command[257];			/* The whole command */
  char groupName[GROUP_NAME_LENGTH];	/* Just the group name */
  char buf[LINE_LENGTH+1], *pt, *p1;
  int status;				/* tcp return */
  int retries;			        /* A count of how hard we have tried */ 
  BOOL group_wanted;			/* Flag: group was asked for, not article */
  BOOL list_wanted;			/* Flag: group was asked for, not article */
  long first, last;			/* First and last articles asked for */
  
  int has_xover;
  diagnostic = (format_out == WWW_SOURCE);	/* set global flag */
  

#ifndef DISABLE_TRACE
  if (www2Trace) 
    fprintf(stderr, "HTNews: Looking for %s\n", arg);
#endif
  
  if (!initialized) 
    initialized = initialize();
  if (!initialized) {
    HTProgress ("Could not set up news connection.");
    return HT_NOT_LOADED;	/* FAIL */
  }

  /* Pull in the newsrc data if necessary */
  if (newsrc_init (HTNewsHost) != 0) { 
    HTProgress ("Not using newsrc data...");
  }
    
  /* Update the preferences so we don't have to make a function call for
     every newsShow** resource access.
   */
  newsShowAllGroups = get_pref_boolean (eSHOWALLGROUPS);
  newsShowReadGroups = get_pref_boolean (eSHOWREADGROUPS);
  newsShowAllArticles = get_pref_boolean (eSHOWALLARTICLES);
  newsNoThreadJumping = get_pref_boolean (eNOTHREADJUMPING);
  ConfigView = !get_pref_boolean (eUSETHREADVIEW);
  newsAuthWidth = get_pref_int (eNEWSAUTHORWIDTH);
  newsSubjWidth = get_pref_int (eNEWSSUBJECTWIDTH);
  newsPrevIsUnread = get_pref_boolean (ePREVISUNREAD);
  newsNextIsUnread = get_pref_boolean (eNEXTISUNREAD);

  /*	We will ask for the document, omitting the host name & anchor.
  **
  **	Syntax of address is
  **		xxx@yyy			Article
  **		<xxx@yyy>		Same article
  **		xxxxx			News group (no "@")
  */        
  group_wanted = (strchr(arg, '@')==0) && (strchr(arg, '*')==0);
  list_wanted  = (strchr(arg, '@')==0) && (strchr(arg, '*')!=0);
    
  /* Don't use HTParse because news: access doesn't follow traditional
     rules. For instance, if the article reference contains a '#',
     the rest of it is lost -- JFG 10/7/92, from a bug report 
  */
  if (!my_strncasecmp (arg, "news:", 5))
    p1 = arg + 5;  /* Skip "news:" prefix */
  if (group_wanted) {
    strcpy (command, "GROUP ");
    first = 0;
    last = 0;
    strcpy (groupName, p1);
    strcat (command, groupName);
  } else if (!list_wanted) {
    strcpy (command, "ARTICLE ");
    if (strchr(p1, '<')==0) 
      strcat(command,"<");
    strcat(command, p1);
    if (strchr(p1, '>')==0) 
	strcat(command,">");
  } else {
	command[0] = 0;
  }
  
  {
    char * p = command + strlen(command);
    *p++ = CR;		/* Macros to be correct on Mac */
    *p++ = LF;
    *p++ = 0;
  }
  
  if (!*arg) {
    HTProgress ("Could not load data.");
    return HT_NOT_LOADED;			/* Ignore if no name */
  }
  
  /*  Make a hypertext object with an anchor list. */       
  node_anchor = anAnchor;
  target = HTML_new(anAnchor, format_out, stream);
  targetClass = *target->isa;	
  
  /*  Now, let's get a stream setup up from the NewsHost: */       
  for (retries=0; retries<2; retries++) {
    target = HTML_new(anAnchor, format_out, stream);
    targetClass = *target->isa;	/* Copy routine entry points */
      if (s < 0) 
	if(status = OpenNNTP()){
	  char message[256];
	  switch(status){
	  case 1:
	    /* Couldn't get it. */
	    START(HTML_TITLE);
	    PUTS("Could Not Retrieve Information");
	    END(HTML_TITLE);
	    PUTS("Sorry, could not retrieve information.");
	    (*targetClass.end_document)(target);
	    (*targetClass.free)(target);
	    return HT_LOADED;

	  case 2:
	    if (retries<=1) {
	      /* Since we reallocate on each retry, free here. */
	      (*targetClass.end_document)(target);
		  (*targetClass.free)(target);
		  continue;
	    }
	    HTProgress ("Could not access news host.");
	    sprintf(message,"\nCould not access news host %s.  "
		    "Try setting environment variable <code>NNTPSERVER</code> "
		    "to the name of your news host, and restart Mosaic.", 
		    HTNewsHost);
	    
	    PUTS(message);
	    (*targetClass.end_document)(target);
	    (*targetClass.free)(target);
	    return HT_LOADED;
	  case 3:
	    HTProgress ("Connection interrupted.");
	    (*targetClass.handle_interrupt)(target);	    
	    
	    return HT_INTERRUPTED;
	  }
	}
      
      status = response("XOVER\r\n");
      if(status != 500) 
	has_xover = 1; 
      else 
	has_xover = 0;

      /* FLUSH!!! */
      HTInitInput(s);

      /* read_list () will actually take care of its own command stuff */
      if (!list_wanted) {
	status = response(command);
	strcpy (buf, response_text);
      } else { 
	status = 211;
      }

#ifndef DISABLE_TRACE
      if (www2Trace)
	fprintf (stderr, "News: Sent '%s', status %d\n", command, status);
#endif
      if (status < 0) 
	break;
      if ((status/100) != 2) {
	PUTS(response_text);
	(*targetClass.end_document)(target);
	(*targetClass.free)(target);
	NETCLOSE(s);
	s = -1;
	continue;	/* Try again */
      }
      
      /*  Load a group, article, etc */
      if (list_wanted) {
	/* Destroy all the article stuff */
	ClearArtList ();
	if (NewsGroupS) 
	  NewsGroupS = NULL;
	if (NewsGroup)
	  free (NewsGroup);
	NewsGroup = NULL;
	NextArt = NULL;
	read_list();
	HTMeter (100,NULL);
	HTDoneWithIcon ();
	HTProgress ("Rendering newsgroup listing... one moment please");
      } else if (group_wanted){ 
	int l,h,j;
	
	if(has_xover) 
	  XBuildArtList(groupName, first, last);
	else 
	  BuildArtList(groupName, first, last);

	if (sscanf (buf, "%d %d %d %d", &j, &j, &l, &h) == 4) 
	  read_group(groupName, l, h);
	else 
	  read_group(groupName, 0, -1);
	HTMeter (100,NULL);
	HTDoneWithIcon ();
      } else { 
	read_article(&command[9]);
	HTMeter (100,NULL);
	HTDoneWithIcon ();
      }
      (*targetClass.end_document)(target);
      (*targetClass.free)(target);
      return HT_LOADED;
    } /* Retry loop */
  return HT_LOADED;
}

PUBLIC HTProtocol HTNews = { "news", HTLoadNews, NULL };
PUBLIC HTProtocol HTNNTP = { "nntp", HTLoadNews, NULL };













