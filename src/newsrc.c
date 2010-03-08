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
#include "newsrc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Xm/MessageB.h>

newsgroup_t *addgroup (char *nline, long min, long max, int dri);
newsgroup_t *findgroup (char *name);
void setseq (newsgroup_t *ng, char *str);
void allocminmax (newsgroup_t *ng, long min, long max);
char *newsrc_getseq (char *, char*);
void dumpgroup (newsgroup_t *ng);
void newsrc_flushcb (XtPointer cld, XtIntervalId *id);


/* Number of bins in the newsgroup hash table */
#define ACTIVE_BINS 101

#define MAX_BUF 1024

newsgroup_t *newsrc_groups[ACTIVE_BINS];
FILE        *newsrc_fp;
char         newsrc_filename[MAX_BUF];       /* filename of the current newsrc */
char         newsrc_filenamebak[MAX_BUF];    /* filename of the last newsrc backup */
int          newsrc_active = 0;              /* This is whether newsrc hash table is valid */
int          newsrc_line;
XtIntervalId newsrc_ti;
int          newsrc_timer = 0;
int          newsrc_flushit = 0;             /* Do we need to flush? */
int newsrc_smask;

extern int newsUseNewsRC;
int newsNoNewsRC = 0;

/* Article functions */

/* isread ()
   Expects: ng -- newsgroup
            art -- article number
   Returns: 0 if not read, 1 if read
   
   Notes:  
*/
int isread (newsgroup_t *ng, long art)
{
  int b;
  if (!ng)
    return 0;
  if (art<ng->minart)
    return 1;
  if (art>ng->maxart)
    return 1;
  b = art - ng->minart;
  return (ng->read[b>>3] & (1<<(b&7)));
}

/* markread ()
   Expects: ng -- newsgroup
            art -- article number
   Returns: nothing
   
   Notes:  
*/
void markread (newsgroup_t *ng, long art)
{
  int b;

  if (!ng || !ng->read)
    return;
  if (art<ng->minart)
    return;
  if (art>ng->maxart)
    return;

  b = art - ng->minart;
  ng->attribs |= naUPDATE;
  newsrc_flushit++;
  if (!(ng->read[b>>3] & (1<<(b&7)))) /* only decrement if article is truly unread */
    ng->unread--;
  ng->read[b>>3] |= (1<<(b&7));
}

/* markunread ()
   Expects: ng -- newsgroup
            art -- article number
   Returns: nothing
   
   Notes:  
*/
void markunread (newsgroup_t *ng, long art)
{
  int b;

  if (!ng || !ng->read)
    return;
  if (art<ng->minart)
    return;
  if (art>ng->maxart)
    return;

  b = art - ng->minart;
  ng->attribs |= naUPDATE;
  newsrc_flushit++;
  if (ng->read[b>>3] & (1<<(b&7))) /* only increment if article is truly read */
    ng->unread++;
  ng->read[b>>3] &= ~(1<<(b&7));
}

/* markrangeread ()
   Expects: ng -- newsgroup
            start -- starting article
	    stop -- ending article
   Returns: nothing

   Notes: could be faster
*/
void markrangeread (newsgroup_t *ng, long start, long stop)
{
  if (!ng)
    return;
  for (;start<=stop;start++)
    markread(ng,start);
}


/* markrangeunread ()
   Expects: ng -- newsgroup
            start -- starting article
	    stop -- ending article
   Returns: nothing

   Notes: could be faster
*/
void markrangeunread (newsgroup_t *ng, long start, long stop)
{
  if (!ng)
    return;
  for (;start<=stop;start++)
    markunread(ng,start);
}



/* Group functions */

/* issubscribed ()
   Expects: name -- name of newsgroup
   Returns: pointer to group or NULL if not subscribed
   
   Notes:
*/
newsgroup_t *issubscribed (char *name)
{
  newsgroup_t *ng = findgroup (name);

  if (ng && ng->attribs&naSUBSCRIBED)
    return ng;
  else
    return NULL;
}


/* subscribegroup ()
   Expects: name -- group name
   Returns: NULL on error

   Notes:  Once the group is subscribed, resizereadbits () should be called to
           validate the minart and maxart information for the group.  updategroup ()
	   should also be called to reread the newsrc information.
*/
newsgroup_t *subscribegroup (char *name)
{
  char junk[MAX_BUF+1];
  newsgroup_t *ng = findgroup (name);

  if (!ng) {
    sprintf (junk, "%s: ", name);
    if (ng = addgroup (junk, 0, 0, 0)) { 
      ng->attribs |= naUPDATE;
      ng->attribs |= naSUBSCRIBED;
      ng->attribs |= naSHOWME;
    }
  } else {
    ng->attribs |= naUPDATE;
    ng->attribs |= naSUBSCRIBED;
    ng->attribs |= naSHOWME;

  }

  newsrc_flushit++;
  return ng;
}

/* unsubscribegroup ()
   Expects: name -- group name
   Returns: pointer to unsubed group

   Notes:  
*/
newsgroup_t *unsubscribegroup (char *name)
{
  newsgroup_t *ng = findgroup (name);

  if (ng) {
    ng->attribs &= ~(naSUBSCRIBED);
    ng->attribs |= naUPDATE;
    ng->attribs &= ~(naSHOWME);
    newsrc_flushit++;
  }
  return ng;
}


/* dumpgroup ()
   Expects: ng -- newsgroup to dump
   Returns: nothing
   
   Notes: Displays some useful info on the given group
*/
void dumpgroup (newsgroup_t *ng)
{
  long l;

  if (!ng) {
    printf  ("Name: ng is NULL\n");
    return;
  }

  printf ("Name: %s [%ld %ld] %ld %c%c%c\n", 
	  ng->name?ng->name: "NULL",
	  ng->minart,
	  ng->maxart,
	  ng->unread,
	  (ng->attribs&naUPDATE)? 'U': 'u',
	  (ng->attribs&naSUBSCRIBED)? 'S': 's',
	  (ng->attribs&naNEWGROUP)? 'N': 'n'
	  );

}


/* dumphash ()
   Expects: nothing
   Returns: nothing
   
   Notes: Displays some useful info on the hash table
*/
void dumphash ()
{
  int i;
  newsgroup_t *ng;

  fprintf (stderr, "Hash Table ......\n");
  for (i=0; i<ACTIVE_BINS; i++) {
    ng = newsrc_groups[i];
    while (ng) {
      dumpgroup (ng);
      ng = ng->next;
    }
  }
}

/* Groups hash table functions */

/* hashgroup ()
   Expects: name -- the name of the group
   Returns: the integer value of the key to begin looking at in the active array.

   Notes: hashpjw() from _Compilers_ by Aho et al.
*/
int hashgroup (char *name)
{
  unsigned int val=0, g;
  char *p;

  if (!name)
    return 0;
  for (p=name; *p!='\0'; p++) {
    val = (val<<4)+(*p);
    if (g = val&0xF0000000) {
      val = val^(g>>24);
      val = val^g;
    }
  }
  return val%ACTIVE_BINS;
}

/* findgroup ()
   Expects: name -- name of the group
   Returns: pointer to group info, NULL if it is not in the table

   Notes: 
*/
newsgroup_t *findgroup (char *name)
{
  newsgroup_t *g;
  int i;

  if (!name)
    return NULL;

  i = hashgroup (name);
  g = newsrc_groups[i];
  while (g) {
    if (strcmp (g->name, name) == 0)
      return g;
    g = g->next;
  }
  return NULL;
}


/* firstgroup ()
   Expects: nothing
   Returns: NULL if no groups

   Notes:  Returns the first group in the hash table with the given attribute.
*/
newsgroup_t *firstgroup (int mask) 
{
  int i;
  newsgroup_t *n;

  newsrc_smask = mask;
  for (i=0; i<ACTIVE_BINS; i++) {
    n = newsrc_groups[i];
    while (n) {
      if (mask == -1 || n->attribs & mask) {
	return n;
      }
      n = n->next;
    }
  }
}

/* nextgroup ()
   Expects: ng -- pointer to last newsgroup received
   Returns: NULL if no groups

   Notes:  Returns the next group in the hash table with the suggested attribute.
*/
newsgroup_t *nextgroup (newsgroup_t *ng) 
{
  int i;
  newsgroup_t *n;

  if (!ng)
    return firstgroup (naSUBSCRIBED);

  for (i=ng->h; i<ACTIVE_BINS; i++) {
    n = newsrc_groups[i];
    if (i == ng->h) {
      while (n && n!=ng) 
	n=n->next;
      if (!n) 
	continue;
      n=n->next;
    }
    if (!n) 
      continue;
    while (n) {
      if (newsrc_smask == -1 || n->attribs & newsrc_smask) {
	return n;
      }
      n = n->next;
    }
  }
  return NULL;
}


/* addgroup ()
   Expects: nline -- a line read from the newsrc to parse and add
            min, max -- minimum and maximum articles numbers according to the nntp
	                server in use.
	    dri -- if dri is nonzero, the read bitarray will be initialized (to the
	           min and max articles) otherwise read will be NULL.
   Returns: NULL if not added

   Notes:  Adds a group to the active table
*/
newsgroup_t *addgroup (char *nline, long min, long max, int dri) 
{
  char *name;
  int hash;
  int s=0;
  newsgroup_t *ng;

  /* rec.games.programmer: 1-89, 91, 93, 95-100 */

  if (!nline)
    return NULL;

  /* Pull out the name */
  name = nline;
  while (*nline && *nline != ' ' && *nline != '\t' && *nline != ':' && *nline != '!' 
	 && *nline != '\n') 
    nline++;

  if (*nline == ':')
    s = 1;

  *nline = '\0';

  if (name && *name)
    if (ng = findgroup (name))
      return ng;

  if ((ng = malloc (sizeof (newsgroup_t))) == NULL) {
    return NULL;
  }
  memset (ng, 0, sizeof (newsgroup_t));

  if (s)
    ng->attribs |= naSUBSCRIBED;

  ng->attribs |= naSHOWME;
  if (name && *name) {
    if((ng->name = strdup (name)) == NULL) {
      free (ng);
      return NULL;
    }
  } else {
    free (ng);
    return NULL;
  }

  if (dri) {
    /* allocate the bitarray */
    allocminmax (ng, min, max);
    /* pull out bitarray info from newsrc */
    if (nline && *nline)
      setseq (ng, nline);
    
    /* Do some checks to make sure numbers are valid */
    if (ng->newsrcmin < ng->minart) 
      ng->newsrcmin = ng->minart;
    if (ng->newsrcmax > ng->maxart)
      ng->newsrcmax = ng->maxart;
  } else {
    ng->read = NULL;
    ng->minart = 0;
    ng->maxart = -1;
  }
  /* Add to table */
  hash = hashgroup (ng->name);
  ng->next = newsrc_groups[hash];
  newsrc_groups[hash] = ng;
  ng->h = hash;
  return ng;
}


/* Miscellaneous & helpers */

/* getread ()
   Expects: ng -- newsgroup struct
            str -- active articles string
   Returns: nothing
   
   Notes: Updates the newsgroup read article bit array for the newgroup in ng
          according to the sequence info in the newsrc line passed in str.  If 
	  the sequence info specifies nonavailable articles in the read bitarray, 
	  the info is ignored.
*/
void setseq (newsgroup_t *ng, char *str)
{
  long min=0, max=0, lo=0, hi=0;
  char *tok, *tok2;

  if (!str)
    return;

  ng->unread = ng->maxart - ng->minart +1;
  while (*str) {

    /* 1-78, 89, 91-100 */

    /* Eat whitespace */
    while (*str && (*str < '0' || *str > '9')) str++;
    if (!(*str)) /* Done */
      break;
    
    /* Chomp out a number */
    tok = str;
    while (*str && (*str >= '0' && *str <= '9')) str++;
    lo = atol (tok);

    if (!(*str)) /* Done */
      break;

    if (*str == '-') { /* Do a range */
      str++;
      /* Chomp out another number */
      tok2 = str;
      while (*str && (*str >= '0' && *str <= '9')) str++;
      hi = atol (tok2);
      markrangeread (ng,lo,hi);
    } else {
      markread (ng,lo);
      hi = lo;
    }
    if (lo<min) min = lo;
    if (hi>max) max = hi;
  }

  ng->newsrcmin = min;
  ng->newsrcmax = max;
  return;
}

/* rereadseq ()
   Expects: ng -- newsgroup to update
   Returns: nothing

   Notes: This will update the information in the read bitarray according to the
          information in the newsrc.
*/
void rereadseq (newsgroup_t *ng)
{
  char buf[MAX_BUF+1];

  if (!ng)
    return;

  if (newsrc_getseq (ng->name, buf)) {
	setseq (ng, buf);
  }

  return;
}




/* allocminmax ()
   Expects: ng -- newsgroup to update
            min -- minimum article number
	    max -- maximum article number
   Returns: nothing

   Notes:  allocates the read bitarray for a newsgroup.
*/
void allocminmax (newsgroup_t *ng, long min, long max)
{
  char *tr;

  if (!ng)
    return;
  
  if (max<min)
    return;

  if ((tr = malloc ((max-min+7)/8+1)) == NULL) {
    return;
  }
  
  memset (tr, 0, ((max-min+7)/8)+1);
  if (ng->read)
    free (ng->read);
  ng->read = tr;
  ng->minart = min;
  ng->maxart = max;
  ng->unread = max-min+1;
  return;
}

/* setminmax ()
   Expects: ng -- newsgroup to update
            min -- new minimum article number
	    max -- new maximum article number
   Returns: nothing

   Notes:  re-allocates the read bitarray for a newsgroup.
           This is slightly slower than etching the new array in 
	   stone with a small hammer and a large peanut butter sandwich.
*/
void setminmax (newsgroup_t *ng, long min, long max)
{
  char *tr;
  long s = ((max-min+7)/8)+1;
  long b, lo, hi, l;

  if (min > max)
    return;

  if ((tr = malloc (s)) == NULL ) {
    return;
  }
  
  memset (tr, 0, s);
  lo = (min>ng->minart)? min : ng->minart; /* max (min, ng->min); */
  hi = (max<ng->maxart)? max : ng->maxart; /* min (max, ng->max); */
  for (l=lo; l<hi; l++) {
    if (isread (ng,l)) {
      b = l-min;
      tr[b>>3] |= (1<<(b&7));
    }
  }

  ng->unread = max-min+1; /* Caller should also call rereadseq() to update unread */
  ng->maxart = max;
  ng->minart = min;
  ng->attribs |= naUPDATE;
  newsrc_flushit = 1;
  if (ng->read)
    free (ng->read);
  ng->read = tr;
}



/* newsrc file functions */

/* readnewsrcline ()
   Expects: buffer -- buffer to read line into
   Returns: 0 if valid line, nonzero on error

   Notes:  Reads the next valid line of the newsrc.  
           Skips comments (as first character) and blank lines.
	   Line still has \n on end.
*/
int newsrc_readline (char *buffer)
{
  char tbuf[MAX_BUF+1], *tp;

  /* get a valid line, i.e. non blank, non comment */
  while (1) {
    if (!fgets (tbuf, MAX_BUF, newsrc_fp)) 
      return 1;
    
    newsrc_line++;
    tp = tbuf;
    while ( (*tp) && ((*tp == ' ') || (*tp == '\t')) ) 
      tp++;

    if (!(*tp) || (*tp == '#') || (*tp == '\n')) 
      continue;

    /* Good line */
    strcpy (buffer, tp);
    break;
  }
  return 0;
}


/* newsrc_getseq ()
   Expects: name -- name of group to get line for
            buf -- buffer to place line in
   Returns: pointer to buffer

   Notes:  Gets the sequence info from the newsrc associated with the given group.
*/
char *newsrc_getseq (char *name, char *buf)
{
  char *c, d, b[MAX_BUF+1];

  if ((newsrc_fp = fopen (newsrc_filename, "r")) == NULL) {
    return NULL;
  }
  newsrc_line = 0;

  while (newsrc_readline (b) == 0) {
    c = b;
    while (*c && !strchr (" :!\t\n", *c)) c++;
    d = *c;
    *c = 0;
    if (strcmp (b, name)) 
      continue;
    c++;
    strcpy (buf, c);
    fclose (newsrc_fp);
    return buf;
  }
  fclose (newsrc_fp);
  return NULL;
}


/* newsrc_backup () 
   Expects: Nothing
   Returns: Nothing

   Notes: Makes a backup of the newsrc file into newsrc.old
*/
void newsrc_backup ()
{
  FILE *old, *new;
  char b[MAX_BUF+1];

  sprintf (newsrc_filenamebak, "%s.old", newsrc_filename);
  if ((old = fopen (newsrc_filenamebak, "w")) == NULL) {
    return;
  }

  if ((new = fopen (newsrc_filename, "r")) == NULL) {
    fclose (old);
    return;
  }

  while (fgets (b,MAX_BUF,new)) 
    fprintf (old, b);

  fclose (old);
  fclose (new);
}


/* Returns ending article num of current run of read articles */
long findendrun (newsgroup_t *n, long art)
{
  long l;

  for (l=art; l<=n->maxart; l++)
    if (!isread (n,l))
      return (l-1);
  return n->maxart;
}

/* Returns beginning article num of next run of read articles */
long findnextrun (newsgroup_t *n, long art)
{
  long l;
  for (l=art; l<=n->maxart; l++)
    if (isread (n,l))
      return l;
  return 0;
}

/* newsrc_writeseq ()
   Expects: ng -- newsgroup to write info for
            buf -- a buffer to put string into
   Returns: a pointer to the buffer
   
   Notes:  Writes out the sequence information for ng
*/
char *newsrc_writeseq (newsgroup_t *ng, char *buf, int max)
{
  long lo, hi=0;
  char b[63+1], *s;
  int len = strlen (buf);

  s = buf;
  while (lo = findnextrun (ng,hi+1)) {
    hi = findendrun (ng,lo);
    if (lo==hi) 
      sprintf (b, "%ld, ", lo); 
    else 
      sprintf (b, "%ld-%ld, ", lo, hi); 
    
    strcat (s, b);
    s += strlen (b);
    len += strlen (b);
    if (len>=max) {
      buf[max-1] = 0;
      return buf;
    }
  }
  
  /* Remove the trailing comma */
  if ((s = strrchr (buf,',')) != NULL) 
    *s = 0;
  return buf;
}


/* newsrc_flush ()
   Expects: Nothing
   Returns: 0 if ok, nonzero otherwise
   
   Notes:  Writes out the data for the current groups hash table, updating the
           current newsrc as it goes.
*/
int newsrc_flush ()
{
  extern XtAppContext app_context;
  newsgroup_t *n;
  FILE *new;
  char b[2*MAX_BUF+1], seq[MAX_BUF+1];
  long del;

  if (!newsrc_flushit || !newsUseNewsRC)
    return 0;

  if (newsrc_timer)
    XtRemoveTimeOut (newsrc_ti);

  newsrc_backup ();
  if ((new = fopen (newsrc_filename, "w+")) == NULL) {
    HTProgress ("Could not open newsrc file");

    return 1;
  }
  
  n = firstgroup (-1);
  while (n) {
    seq[0] = 0;
    newsrc_writeseq (n,seq,MAX_BUF);
    sprintf (b, "%s%c %s\n", n->name, (n->attribs&naSUBSCRIBED)?':':'!', seq);
    fprintf (new, b);
    n->attribs &= ~(naUPDATE);
    n = nextgroup (n);
  }

  fclose (new);
  newsrc_flushit = 0;
  newsNoNewsRC = 0;
  
  if (newsrc_timer) {
    del = get_pref_int (eBACKGROUNDFLUSHTIME);
    newsrc_ti = XtAppAddTimeOut (app_context, 1000L*del, newsrc_flushcb, NULL);
  }
  return 0;
}


/* newsrc_flushcb ()
   Expects: Callback type stuff
   Returns: Callback type stuff

   Notes: Calls itself every newsBackgroundFlushTime seconds to flush out newsrc info
*/
void newsrc_flushcb (XtPointer cld, XtIntervalId *id)
{
  extern XtAppContext app_context;
  char buf[MAX_BUF+1];
  int del;

  newsrc_flush ();
  del = get_pref_int (eBACKGROUNDFLUSHTIME);
  newsrc_ti = XtAppAddTimeOut (app_context, 1000L*del, newsrc_flushcb, NULL);
  newsrc_timer = 1;
  return;
}

void newsrc_initflush ()
{
  extern XtAppContext app_context;
  int del = get_pref_int (eBACKGROUNDFLUSHTIME);
  int i = 1;

  newsrc_ti = XtAppAddTimeOut (app_context, 1000L*del, newsrc_flushcb, NULL);
  newsrc_timer = 1;
  set_pref (eUSEBACKGROUNDFLUSH, &i);
}



/* newsrc_kill ()
   Expects: Nothing
   Returns: 0 if ok, nonzero otherwise
   
   Notes:  Does any cleanup for the currently active newsrc.
           Runs through hash table and writes out any groups that need to be updated.
	   Frees hash table entries.
           Closes the file.
*/
int newsrc_kill (void)
{
  newsgroup_t *n, *n2;
  int i;

  if (newsrc_timer)
    XtRemoveTimeOut (newsrc_ti);

  newsrc_flush ();

  /* Kill hash table */
  for (i=0; i<ACTIVE_BINS; i++) {
    n = newsrc_groups[i];
    while (n) {
      n2 = n->next;
      if (n->read)
	free (n->read);
      if (n->name)
	free (n->name);
      free (n);
      n = n2;
    }
    newsrc_groups[i] = NULL;
  }

  newsrc_active = 0;
  return 0;
}




/* newsrc_init ()
   Expects: newshost -- name of newshost to open the newsrc for.
   Returns: 0 if ok, nonzero otherwise
   
   Notes:  Does any onetime initialization for the newsrc stuff for the given host.
           Opens .mosaic-newsrc-newshost and initializes hash table.
*/
int newsrc_init (char *newshost)
{
  extern XtAppContext app_context;
  int i;
  newsgroup_t *n, *n2;
  char *nntp, buf[MAX_BUF+1];
  long lo, hi;
  char *home = getenv ("HOME");
  char *npref = NULL;
  int spref = 1;

  if (newshost == NULL)
    return 1;

  /* If they want to reinit the current system, they have to do it the hard way */
  if (newsrc_active) 
    return 0;

  sprintf (buf, "Initializing newsrc for %s", newshost);
  HTProgress (buf);

  for (i=0; i<ACTIVE_BINS; i++) {
    newsrc_groups[i] = NULL;
  }

  newsUseNewsRC = get_pref_boolean (eUSENEWSRC);
  if (!newsUseNewsRC) {
    newsNoNewsRC = 1;
    newsrc_active = 0;
    return 0;
  }

  npref = get_pref_string (eNEWSRCPREFIX);
  spref = get_pref_boolean (eUSESHORTNEWSRC);
  sprintf (newsrc_filename, "%s/%s%s%s", home, npref, spref?"":"-",spref?"":newshost);
  if ((newsrc_fp = fopen (newsrc_filename, "r")) == NULL) {
    sprintf (buf, "News file %s/%s%s%s does not exist", 
	     home, npref, spref?"":"-",spref?"":newshost);
    HTProgress (buf);
    newsNoNewsRC = 1;
    newsrc_active = 1;
    return 0;
  }

  newsrc_line = 0;
  while (newsrc_readline (buf) == 0) {
    if (strchr (buf, ':')) 
      addgroup (buf, 0, -1, 0) ;
  }
  fclose (newsrc_fp);
  
  if (get_pref_boolean (eUSEBACKGROUNDFLUSH)) {
    newsrc_ti = XtAppAddTimeOut (app_context, 
			       1000L*get_pref_int (eBACKGROUNDFLUSHTIME),
			       newsrc_flushcb, NULL);
    newsrc_timer = 1;
  }
  newsrc_active = 1;
  return 0;
}

















