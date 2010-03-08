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

/*#include "mosaic.h"*/
#ifndef __HOTLIST_H__
#define __HOTLIST_H__

#include <pwd.h>

#define NCSA_HOTLIST_FORMAT_COOKIE_ONE \
  "ncsa-xmosaic-hotlist-format-1"

#define NCSA_HOTLIST_FORMAT_COOKIE_TWO \
  "<!-- ncsa-xmosaic-hotlist-format-2 -->"

#define NCSA_HOTLIST_FORMAT_COOKIE_THREE \
  "<!-- ncsa-xmosaic-hotlist-format-3 -->"

/* ------------------------------ mo_item_type ---------------------------- */
typedef enum _mo_item_type
{
  mo_t_url, mo_t_list
} mo_item_type;

/* ------------------------------ mo_any_item ----------------------------- */

/* mo_any_item is any item in a mo_hotlist. */
typedef struct mo_any_item
{
  mo_item_type type;
  char *name; /* title for an URL, name for a hotlist */
  /* Position in the list; starting at 1... */
  int position;
  union mo_hot_item *previous;
  union mo_hot_item *next;
} mo_any_item;

/* ------------------------------ mo_hotnode ------------------------------ */

/* mo_hotnode is a single item in a mo_hotlist. */
typedef struct mo_hotnode
{
  mo_item_type type;
  char *title; /* title for an URL */
  /* Position in the list; starting at 1... */
  int position;
  union mo_hot_item *previous;
  union mo_hot_item *next;
  char *url;
  char *lastdate;
  int rbm; /* is it on the rbm? */
} mo_hotnode;

/* ------------------------------ mo_hotlist ------------------------------ */

/* mo_hotlist is a list of URL's and (cached) titles that can be
   added to and deleted from freely, and stored and maintained across
   sessions. */
typedef struct mo_hotlist
{
  mo_item_type type;
  char *name; /* name for a hotlist */
  /* Position in the list; starting at 1... */
  int position;
  union mo_hot_item *previous;
  union mo_hot_item *next;

  /* specific to mo_hotlist */
  struct mo_hotlist *parent;

  union mo_hot_item *nodelist;
  /* Point to last element in nodelist for fast appends. */
  union mo_hot_item *nodelist_last;
  int rbm; /* Is this list on the RBM? */
} mo_hotlist;

/* ------------------------------ mo_root_hotlist ------------------------- */

/* mo_root_hotlist is the root hotlist */
typedef struct mo_root_hotlist
{
  mo_item_type type;
  char *name; /* name for a hotlist */
  /* Position in the list; starting at 1... */
  int position;
  union mo_hot_item *previous;
  union mo_hot_item *next;

  /* specific to mo_hotlist */
  struct mo_hotlist *parent;

  union mo_hot_item *nodelist;
  /* Point to last element in nodelist for fast appends. */
  union mo_hot_item *nodelist_last;
  /* Filename for storing this hotlist to local disk; example is
     $HOME/.mosaic-hotlist-default. */
  char *filename;

  /* Flag set to indicate whether this hotlist has to be written
     back out to disk at some point. */
  int modified;
} mo_root_hotlist;

/* ------------------------------ mo_hot_item ----------------------------- */

/* mo_hot_item is the union of all item type */
typedef union mo_hot_item
{
  mo_item_type type;
  mo_any_item any;
  mo_hotnode hot;
  mo_hotlist list;
  mo_root_hotlist root;
} mo_hot_item;

void mo_append_item_to_hotlist (mo_hotlist *list, mo_hot_item *node);
#if 0
mo_status mo_dump_hotlist (mo_hotlist *);
#endif /* 0 */
mo_status mo_setup_default_hotlist (void);
mo_status mo_write_default_hotlist (void);
mo_status mo_post_hotlist_win (mo_window *);
mo_status mo_add_node_to_current_hotlist (mo_window *);

#endif
