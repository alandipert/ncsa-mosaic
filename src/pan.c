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
#include "pan.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "libhtmlw/HTML.h"

/*for memset*/
#include <memory.h>

#ifndef S_IRUSR
#define S_IRUSR 400
#define S_IWUSR 200
#define S_IXUSR 100
#endif

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

/* ------------------------- Personal Annotations ------------------------- */

/*
  This file contains support for personal annotations (pans).

  We have a directory called ~/.mosaic/personal-annotations (by default).
  In this directory is a file called LOG.

  The LOG file consists of the following:

  ncsa-mosaic-personal-annotation-log-format-1     [cookie]
  Personal                                         [name of log]
  url # # # #                                      [first word is url,
                                                    subsequent words are
                                                    numbers of annotations
                                                    matching this url]
  [1-line sequence for single document repeated as necessary]
  ...

  Thus, a given document can have 0 or more annotation ID's associated
  with it.  Each annotation is named pan-#.html and is stored in
  the same directory as LOG.

  Annotations are formatted like this:

  <ncsa-annotation-format-1>                        [cookie]
  <title>This is the title.</title>                 [title]
  <h1>This is the title, again.</h1>                [line skipped; expected
                                                     to be title]
  <address>This is the author.</address>            [author]
  <address>This is the date.</address>              [date of annotation/
                                                     modification]
  _______________________                           [ignored separator]
  <pre>                                             [line skipped; expected
                                                     to be pre]
  [arbitrary text]                                  [until end of message]

  So long as the annotation remains in this format, you will be
  able to operate on it later with the 'Edit This Annotation' option
  in the X Mosaic menubar.  Else, it will just be another document.
  */

/* ------------------------------------------------------------------------ */
/* --------------------------- GLOBAL PAN LIST ---------------------------- */
/* ------------------------------------------------------------------------ */

#define NCSA_PAN_LOG_FORMAT_COOKIE_ONE \
  "ncsa-mosaic-personal-annotation-log-format-1"
#define PAN_LOG_FILENAME "LOG"
#define PAN_ANNOTATION_PREFIX "PAN-"

#define NCSA_ANNOTATION_FORMAT_ONE \
  "<ncsa-annotation-format-1>"

#define HASHSIZE 100
#define MAX_PANS_PER_HREF 20

/* Cached string for home directory. */
static char *home;

typedef struct entry
{
  char *href;
  int num_pans;
  int an[MAX_PANS_PER_HREF];
  struct entry *next;
} entry;

typedef struct bucket
{
  entry *head;
  int count;
} bucket;

/* The largest ID for any pan known so far.
   The next pan created will have id max_pan_id + 1. */
static int max_pan_id = 0;

static bucket hash_table[HASHSIZE];


static int locate_id (int id, entry **lptr);
static void ensure_pan_directory_exists (void);
static int hash_url (char *url);
static void dump_bucket_counts (void);
static entry *new_entry (int buck, char *href);
static entry *fetch_entry (char *href);
static void add_an_to_entry (entry *l, int an);
static void remove_an_from_entry (entry *l, int an);
static void mo_read_pan_file (char *filename);
static mo_status mo_init_pan (void);
static mo_status mo_write_pan (int id, char *title, char *author, char *text);
static char *smart_append (char *s1, char *s2);
static char *extract_meat (char *s, int offset);



static int hash_url (char *url)
{
  int len, i, val;

  if (!url)
    return 0;
  len = strlen (url);
  val = 0;
  for (i = 0; i < 10; i++)
    val += url[(i * val + 7) % len];

  return val % HASHSIZE;
}

static void dump_bucket_counts (void)
{
  int i;

#ifndef DISABLE_TRACE
  if (srcTrace) {
	for (i = 0; i < HASHSIZE; i++)
		fprintf (stderr, "Bucket %03d, count %03d\n", i, hash_table[i].count);
  }
#endif

  return;
}

/* Presumably href isn't already in the bucket. */
static entry *new_entry (int buck, char *href)
{
  bucket *bkt = &(hash_table[buck]);
  entry *l;

  l = (entry *)malloc (sizeof (entry));
  l->href = strdup (href);
  l->num_pans = 0;
/*  bzero ((void *)(l->an), MAX_PANS_PER_HREF * 4);*/
  memset((void *)(l->an), 0, MAX_PANS_PER_HREF * 4);
  l->next = NULL;

  if (bkt->head == NULL)
    bkt->head = l;
  else
    {
      l->next = bkt->head;
      bkt->head = l;
    }

  bkt->count += 1;

  return l;
}

/* If an entry already exists for an href, then return the entry.
   Else, return NULL. */
static entry *fetch_entry (char *href)
{
  int hash = hash_url (href);
  entry *l;

  if (hash_table[hash].count)
    for (l = hash_table[hash].head; l != NULL; l = l->next)
      {
        if (!strcmp (l->href, href))
          return l;
      }

  return NULL;
}

/* Given an entry and an annotation id, do the right thing
   to the entry. */
static void add_an_to_entry (entry *l, int an)
{
  if (l->num_pans == MAX_PANS_PER_HREF)
    return;

  l->an[l->num_pans] = an;

  l->num_pans++;
}

/* Given an entry and an annotation id, remove the annotation
   from the entry. */
static void remove_an_from_entry (entry *l, int an)
{
  int i, place;
  for (i = 0; i < l->num_pans; i++)
    {
      if (l->an[i] == an)
        {
          place = i;
          goto ok;
        }
    }

  /* Couldn't find an in the list of annoations. */
  return;

 ok:
  /* Found an in the list of annotations. */
  l->an[place] = 0;
  /* If place is 3 and num_pans is 6,
     then i goes from 4 to 5; 3 gets 4's value and then 4 gets 5's. */
  for (i = place + 1; i < l->num_pans; i++)
    l->an[i-1] = l->an[i];
  l->num_pans--;

  /* Don't have to remove an empty entry, since an empty entry won't
     be written out upon exit. */

  return;
}

/* For a given ID, look through the hash table, find the corresponding
   entry, and return both it and the position of the id in it. */
int locate_id (int id, entry **lptr)
{
  entry *l;
  int i, j;

  for (i = 0; i < HASHSIZE; i++)
    {
      if (hash_table[i].count)
        for (l = hash_table[i].head; l != NULL; l = l->next)
          {
            for (j = 0; j < l->num_pans; j++)
              {
                if (l->an[j] == id)
                  {
                    *lptr = l;
                    return j;
                  }
              }
          }
    }

  *lptr = NULL;
  return -1;
}

/* ------------------------------------------------------------------------ */

static void mo_read_pan_file (char *filename)
{
  FILE *fp;
  char line[MO_LINE_LENGTH];
  char *status;
  entry *l;

  fp = fopen (filename, "r");
  if (!fp)
    goto screwed_no_file;

  status = fgets (line, MO_LINE_LENGTH, fp);
  if (!status || !(*line))
    goto screwed_with_file;

  /* See if it's our format. */
  if (strncmp (line, NCSA_PAN_LOG_FORMAT_COOKIE_ONE,
               strlen (NCSA_PAN_LOG_FORMAT_COOKIE_ONE)))
    goto screwed_with_file;

  /* Go fetch the name on the next line. */
  status = fgets (line, MO_LINE_LENGTH, fp);
  if (!status || !(*line))
    goto screwed_with_file;

  /* Start grabbing documents and lists of annotations. */
  while (1)
    {
      char *url;
      char *p;

      status = fgets (line, MO_LINE_LENGTH, fp);
      if (!status || !(*line))
        goto done;

      url = strtok (line, " ");
      if (!url)
        goto screwed_with_file;
      url = strdup (url);
      /* We don't use the last-accessed date... yet. */
      /* lastdate = strtok (NULL, "\n"); blah blah blah... */

      l = new_entry (hash_url (url), url);

      free (url);

      while (p = strtok (NULL, " "))
        {
          int a = atoi (p);
          if (a)
            {
              add_an_to_entry (l, a);
              if (a > max_pan_id)
                max_pan_id = a;
            }
        }
    }

 done:
  fclose (fp);
  return;

 screwed_with_file:
  fclose (fp);
  return;

 screwed_no_file:
  return;
}

static char *cached_global_pan_fname = NULL;

static mo_status mo_init_pan (void)
{
  int i;

  /* Initialize the pan structs. */
  for (i = 0; i < HASHSIZE; i++)
    {
      hash_table[i].count = 0;
      hash_table[i].head = 0;
    }

  return mo_succeed;
}

/* --------------------- ensure_pan_directory_exists ---------------------- */

void ensure_pan_directory_exists (void)
{
  char *default_directory = get_pref_string(ePRIVATE_ANNOTATION_DIRECTORY);
  char filename[500];
  struct stat buf;
  int r;

  sprintf (filename, "%s/%s", home, default_directory);

  r = stat (filename, &buf);
  if (r == -1)
    {
#ifdef NeXT
      mkdir (filename);
#else
      mkdir (filename, S_IRUSR | S_IWUSR | S_IXUSR);
#endif
    }

  return;
}

/* ------------------ Interface For Personal Annotations ------------------ */

mo_status mo_is_editable_pan (char *text)
{
  if (!text)
    return mo_fail;

  if (!strncmp (text, NCSA_ANNOTATION_FORMAT_ONE,
                strlen (NCSA_ANNOTATION_FORMAT_ONE)))
    return mo_succeed;
  else
    return mo_fail;
}

/* First, call mo_init_pan() to set up internal hash table.
   Then, snarf a value for home.
   Then, snarf a value for the log file.
   Then, read the log file and load up the hash table
     by calling mo_read_pan_file. */
mo_status mo_setup_pan_list (void)
{
  char *default_directory = get_pref_string(ePRIVATE_ANNOTATION_DIRECTORY);
  char *default_filename = PAN_LOG_FILENAME;
  char *filename;

  mo_init_pan ();

  home = getenv ("HOME");

  /* This shouldn't happen. */
  if (!home)
    home = "/tmp";

  filename = (char *)malloc
    ((strlen (home) + strlen (default_directory) +
      strlen (default_filename) + 8) * sizeof (char));
  sprintf (filename, "%s/%s/%s", home, default_directory, default_filename);
  cached_global_pan_fname = filename;

  mo_read_pan_file (filename);

  return mo_succeed;
}

/* Write out the log file. */
mo_status mo_write_pan_list (void)
{
  FILE *fp;
  int i;
  entry *l;

  ensure_pan_directory_exists ();

  fp = fopen (cached_global_pan_fname, "w");
  if (!fp)
    return mo_fail;

  fprintf (fp, "%s\n%s\n", NCSA_PAN_LOG_FORMAT_COOKIE_ONE, "Personal");

  for (i = 0; i < HASHSIZE; i++)
    for (l = hash_table[i].head; l != NULL; l = l->next)
      if (l->num_pans)
        {
          int j;
          fprintf (fp, "%s", l->href);
          for (j = 0; j < l->num_pans; j++)
            fprintf (fp, " %d", l->an[j]);
          fprintf (fp, "\n");
        }

  fclose (fp);

  return mo_succeed;
}

static mo_status mo_write_pan (int id, char *title, char *author, char *text)
{
  char *default_directory = get_pref_string(ePRIVATE_ANNOTATION_DIRECTORY);
  char filename[500];
  FILE *fp;
  time_t foo = time (NULL);
  char *ts = ctime (&foo);

  ts[strlen(ts)-1] = '\0';

  ensure_pan_directory_exists ();

  /* Write the new annotation to its appropriate file. */
  sprintf (filename, "%s/%s/%s%d.html", home, default_directory,
           PAN_ANNOTATION_PREFIX, id);

  fp = fopen (filename, "w");
  if (!fp)
    return mo_fail;

  fprintf (fp, "%s\n", NCSA_ANNOTATION_FORMAT_ONE);
  fprintf (fp, "<title>%s</title>\n", title);
  fprintf (fp, "<h1>%s</h1>\n", title);
  fprintf (fp, "<address>%s</address>\n", author);
  foo = time (NULL);
  fprintf (fp, "<address>%s</address>\n", ts);
  fprintf (fp, "______________________________________\n");
  fprintf (fp, "<pre>\n");
  fprintf (fp, "%s", text);

  fclose (fp);

  return mo_succeed;
}

/* We're making a new annotation to a url.  Pass in the title,
   author, and text of the annotation. */
mo_status mo_new_pan (char *url, char *title, char *author, char *text)
{
  entry *l = fetch_entry (url);
  int id = ++max_pan_id;

  if (!title || !*title)
    title = strdup ("Annotation with no title");
  if (!author || !*author)
    author = strdup ("No author name");

  /* Create a new entry if we have to. */
  if (!l)
    l = new_entry (hash_url (url), url);

  /* Register the new annotation id with the entry. */
  add_an_to_entry (l, id);

  mo_write_pan (id, title, author, text);

  return mo_succeed;
}

/* We're just outright deleting an annotation. */
mo_status mo_delete_pan (int id)
{
  entry *l;
  int place;
  char filename[500]/*, *cmd*/;
  char *default_directory = get_pref_string(ePRIVATE_ANNOTATION_DIRECTORY);

  place = locate_id (id, &l);

  if (!l)
    /* Weird -- no URL associated with the edited annotation. */
    return mo_fail;
  remove_an_from_entry (l, id);

  /* Remove the annotation itself. */
  sprintf (filename, "%s/%s/%s%d.html", home, default_directory,
           PAN_ANNOTATION_PREFIX, id);
/*
  cmd = (char *)malloc ((strlen (filename) + 32) * sizeof (char));
  sprintf (cmd, "/bin/rm -f %s &", filename);
  system (cmd);
  free (cmd);
*/
  unlink(filename);

#ifdef HAVE_AUDIO_ANNOTATIONS
#if defined(__sgi)
  /* Remove a possible audio annotation. */
  sprintf (filename, "%s/%s/%s%d.aiff", home, default_directory,
           PAN_ANNOTATION_PREFIX, id);
/*
  cmd = (char *)malloc ((strlen (filename) + 32) * sizeof (char));
  sprintf (cmd, "/bin/rm -f %s &", filename);
  system (cmd);
  free (cmd);
*/
  unlink(filename);

#else /* sun or HP, probably */
  /* Remove a possible audio annotation. */
  sprintf (filename, "%s/%s/%s%d.au", home, default_directory,
           PAN_ANNOTATION_PREFIX, id);
/*
  cmd = (char *)malloc ((strlen (filename) + 32) * sizeof (char));
  sprintf (cmd, "/bin/rm -f %s &", filename);
  system (cmd);
  free (cmd);
*/
  unlink(filename);
#endif
#endif /* HAVE_AUDIO_ANNOTATIONS */

  return mo_succeed;
}

/* We're modifying an existing pan.  Pass in the id,
   the title, author, and text.
   Check for null text -- NOT AT THE MOMENT */
mo_status mo_modify_pan (int id, char *title, char *author,
                         char *text)
{
  if (!title || !*title)
    title = strdup ("Annotation with no title");
  if (!author || !*author)
    author = strdup ("No author name");

  mo_write_pan (id, title, author, text);
  return mo_succeed;
}

/* s1 is the existing string (or NULL for a new string)
   s2 is the string to append
   (s1 is assumed free'able, s2 not) */
static char *smart_append (char *s1, char *s2)
{
  if (!s1)
    return strdup (s2);
  else
    {
      char *foo = malloc (strlen (s1) + strlen (s2) + 8);
      strcpy (foo, s1);
      strcat (foo, s2);
      free (s1);
      return foo;
    }
}

/* Given a string (s) and an offset,
   make a copy of the string starting at the offset;
   step through that string and nil'ify the first '<' we see.
   Return the resulting string.
   Example:
     extract_meat ("<foo>barblegh</foo>", 5) returns "barblegh". */
static char *extract_meat (char *s, int offset)
{
  char *foo = s + offset;
  char *bar = strdup (foo);
  char *ptr = bar;

  while (*bar != '\0' && *bar != '<')
    bar++;

  if (*bar == '<')
    *bar = '\0';

  return ptr;
}

char *mo_fetch_personal_annotations (char *url)
{
  entry *l = fetch_entry (url);
  char *msg = NULL;
  char line[MO_LINE_LENGTH];
  char *default_directory = get_pref_string(ePRIVATE_ANNOTATION_DIRECTORY);
  int i, count = 0;

  if (!l || !l->num_pans)
    {
      return NULL;
    }

  /* OK, now we've got the entry.  Basically, we step through
     and append all anotations for the given url together
     */
  sprintf (line, "Private-Annotation: %d\r\n", l->num_pans);
  msg = smart_append (msg, line);

  for (i = 0; i < l->num_pans; i++)
    {
      /* What's the annotation called? */
      char filename[500];
      FILE *fp;

      /* Do we have to assume we're opening a file with
         suffix .html??? */
      sprintf (filename, "%s/%s/%s%d.html", home, default_directory,
               PAN_ANNOTATION_PREFIX, l->an[i]);

      /* This whole routine assumes there are no NULLs in the file. */
      fp = fopen (filename, "r");
      if (fp)
	{
	  char* an_anno = NULL;
	  while (fgets (line, MO_LINE_LENGTH, fp)) {
	    an_anno = smart_append (an_anno, line);
	  }
	  sprintf (line, "Content-Length: %d\r\n", strlen(an_anno));
	  msg = smart_append (msg, line);
	  msg = smart_append (msg, an_anno);
	  if (an_anno)
	    free(an_anno);
	}

      fclose (fp);
    }

  return msg;
}

/* For the given url, fetch an HTML-format hyperlink table
   to be appended to the document text. */
char *mo_fetch_pan_links (char *url, int on_top)
{
  entry *l = fetch_entry (url);
  char *msg = NULL;
  char line[MO_LINE_LENGTH];
  char *status;
  char *default_directory = get_pref_string(ePRIVATE_ANNOTATION_DIRECTORY);
  int i, count = 0;

  if (!l)
    {
      return NULL;
    }
  if (!l->num_pans)
    {
      return NULL;
    }

  /* OK, now we've got the entry.  Basically, we step through
     the available annotations and manufacture links:

     <h2>Personal Annotations</h2>
     <ul>
     <li> Title, Date
     <li> Title, Date
     </ul>
     */

  msg = smart_append (msg, "<h2>Personal Annotations</h2>\n<ul>\n");

  for (i = 0; i < l->num_pans; i++)
    {
      /* What's the annotation called? */
      char filename[500];
      FILE *fp;

      /* Do we have to assume we're opening a file with
         suffix .html??? */
      sprintf (filename, "%s/%s/%s%d.html", home, default_directory,
               PAN_ANNOTATION_PREFIX, l->an[i]);

      fp = fopen (filename, "r");
      if (fp)
        {
          status = fgets (line, MO_LINE_LENGTH, fp);
          if (status && *line)
            {
              /* See if it's our format. */
              if (!strncmp (line, NCSA_ANNOTATION_FORMAT_ONE,
                            strlen (NCSA_ANNOTATION_FORMAT_ONE)))
                {
                  char chunk[500];

                  count++;

                  /* Second line is the title. */
                  status = fgets (line, MO_LINE_LENGTH, fp);

                  sprintf (chunk, "<li> <a href=\"file://%s%s\">%s</a>  ",
                           "localhost", filename, extract_meat (line, 7));
                  msg = smart_append (msg, chunk);

                  /* Third line is skipped. */
                  status = fgets (line, MO_LINE_LENGTH, fp);

                  /* Fourth line is the author; skipped. */
                  status = fgets (line, MO_LINE_LENGTH, fp);

                  /* Fifth line is the date. */
                  status = fgets (line, MO_LINE_LENGTH, fp);
                  sprintf (chunk, "(%s)\n", extract_meat (line, 9));
                  msg = smart_append (msg, chunk);

                  /* That's it. */
                }
            }

          fclose (fp);
        }
    }

  /* If we made it all this way and it turns out we don't actually
     have anything yet, then punt. */
  if (!count)
    {
      free (msg);
      return NULL;
    }

  msg = smart_append (msg, "</ul>\n");

  return msg;
}

mo_status mo_grok_pan_pieces (char *url, char *t,
                              char **title, char **author, char **text,
                              int *id, char **fn)
{
  char line[MO_LINE_LENGTH];
  char *status;
/*  char *default_directory = Rdata.private_annotation_directory;*/
  FILE *fp;
  char *filename;

  /* We're now including the hostname in the URL.  Jump past it. */
  filename = strstr (url, "//");
  if (!filename)
    return mo_fail;
  filename = strstr (filename + 2, "/");
  if (!filename)
    return mo_fail;

  *fn = filename;

  fp = fopen (filename, "r");
  if (fp)
    {
      status = fgets (line, MO_LINE_LENGTH, fp);
      if (status && *line)
        {
          /* See if it's our format. */
          if (!strncmp (line, NCSA_ANNOTATION_FORMAT_ONE,
                        strlen (NCSA_ANNOTATION_FORMAT_ONE)))
            {
              /*char chunk[500];*/

              /* Second line is the title. */
              status = fgets (line, MO_LINE_LENGTH, fp);
              *title = extract_meat (line, 7);

              /* Third line is skipped. */
              status = fgets (line, MO_LINE_LENGTH, fp);

              /* Fourth line is the author. */
              status = fgets (line, MO_LINE_LENGTH, fp);
              *author = extract_meat (line, 9);

              /* Fifth line is the date. */
              status = fgets (line, MO_LINE_LENGTH, fp);

              /* Sixth line is separator. */
              status = fgets (line, MO_LINE_LENGTH, fp);
              /* Sixth line is pre. */
              status = fgets (line, MO_LINE_LENGTH, fp);

              /* Remaining lines are the text. */
              *text = NULL;

              while (1)
                {
                  status = fgets (line, MO_LINE_LENGTH, fp);
                  if (status && *line)
                    *text = smart_append (*text, line);
                  else
                    goto got_it;
                }

            got_it:
              /* That's it. */
              ;
            }
        }

      fclose (fp);
    }
  else
    {
      return mo_fail;
    }

  {
    char *start = strstr (filename, "PAN-") + 4;
    char *foo = strdup (start);
    char *keepit = foo;
    while (*foo != '\0' && *foo != '.')
      foo++;
    if (*foo == '.')
      *foo = '\0';
    *id = atoi (keepit);
    free (keepit);
  }

  return mo_succeed;
}

/* Return the ID of the next pan to be issued. */
int mo_next_pan_id (void)
{
  return max_pan_id + 1;
}


