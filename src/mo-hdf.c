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
#include "mo-hdf.h"
#include "mo-dtm.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_HDF

static char *pull_guts_from_hdfref (char *target)
{
  if (target)
    {
      /* If the first seven characters don't match "hdfref;", then we know
         it's not a target. */
      if (strncmp (target, "hdfref;", 7))
        {
          /* It's not an hdfref; we don't know what the hell it is. */
          target = NULL;
        }
      else
        {
          /* OK, it's an hdfref.  Or, more to the point, &(target[7]) is. */
          target = &(target[7]);
        }
      return target;
    }

  return NULL;
}
  


/****************************************************************************
 * name:    mo_hdf_fetch_local_filename
 * purpose: Given an HDF URL (we will canonicalize it), return the
 *          filename of the local cached copy of the file.
 * inputs:  
 *   - char *url: The URL to use.
 * returns: 
 *   The local filename (char *).
 * remarks: 
 *   
 ****************************************************************************/
char *mo_hdf_fetch_local_filename (char *url)
{
  char *cache_url = mo_url_canonicalize (url, "");
  char *rv = (char *)mo_fetch_cached_local_name (cache_url);
  free (cache_url);
  return rv;
}


/****************************************************************************
 * name:    mo_decode_internal_reference
 * purpose: Given a URL, a block of HTML that came out of libwww
 *          (assumed to be in this format:
 *             <mosaic-internal-reference "filename">\n
 *          ), and a target anchor (can be NULL), pull the filename out
 *          of the internal HTML reference and do the right thing
 *          with the resulting HDF file.
 * inputs:  
 *   - char     *url: The URL we're visiting.
 *   - char *newtext: The block of HTML containing the internal
 *                    HTML reference.
 *   - char  *target: The target anchor in the URL, or NULL.
 * returns: 
 *   HTML (char *)
 * remarks: 
 *   When we're called, we've already pulled over something from the net.
 *   So no matter what, nuke any existing file and cache the current
 *   file.
 ****************************************************************************/
char *mo_decode_internal_reference (char *url, char *newtext, char *target)
{
  char *text, *fname;
  int i;

  /* We always reload here, since we already sucked it over the network.
     However, we want to clean up after ourselves if this is in fact a reload,
     i.e., another local copy of this URL already exists. */
  fname = mo_hdf_fetch_local_filename (url);
  if (fname)
    {
      /* Nuke an existing file, since we just got a new one. */
      /*
      char *cmd = (char *)malloc ((strlen (fname) + 32) * sizeof (char));
      sprintf (cmd, "/bin/rm -f %s &", fname);
      system (cmd);
      free (cmd);
      */
      unlink(fname); 
    }

  /* Now go get the new filename. */
  fname = (char *)malloc (sizeof (char) * (strlen (newtext) - 20));
  sscanf (newtext, "<mosaic-internal-reference \"%s", fname);

  /* We certainly don't need newtext anymore. */
  free (newtext);

  for (i = 0; i < strlen (fname); i++)
    if (fname[i] == '\"')
      fname[i] = '\0';
  {
    char *cache_url = mo_url_canonicalize (url, "");
    mo_cache_data (cache_url, fname, 1);
    free (cache_url);
  }

  target = pull_guts_from_hdfref (target);

  if (!target)
    {
      text = (char *)hdfGrokFile (fname, url);
    }
  else
    {
      text = (char *)hdfGrokReference (fname, target, url);
    }

  return text;
}
  

/****************************************************************************
 * name:    mo_decode_hdfref
 * purpose: Given a URL (canonicalized with or without target still
 *          attached) and a target anchor which is an hdfref,
 *          look up the cached local file and decode the href.
 * inputs:  
 *   - char    *url: URL for the data file on the net.
 *   - char *target: Target anchor (hdfref).
 * returns: 
 *   HTML for the hdfref.
 * remarks: 
 *   The URL is canonicalized (target anchor stripped out) for the call
 *   to mo_fetch_cached_data(), so it can return the appropriate local 
 *   filename for all internal references.
 *   --> We assume the file is already stored locally. ???
 ****************************************************************************/
char *mo_decode_hdfref (char *url, char *target)
{
  char *text, *fname;
  struct stat buf;
  int i;

  fname = mo_hdf_fetch_local_filename (url);
  if (!fname || stat (fname, &buf))
    {
      /* Go fetch the file anew. */
      fname = mo_tmpnam (url);
      if (mo_pull_er_over_virgin (url, fname) == mo_fail)
        {
          return strdup ("Whoops, something went wrong -- back up and try again.");
        }
      else
        {
          char *cache_url = mo_url_canonicalize (url, "");
          mo_cache_data (cache_url, fname, 1);
          free (cache_url);
        }
    }

  target = pull_guts_from_hdfref (target);

  if (!target)
    {
      return strdup ("Whoops, something went wrong -- badly wrong.");
    }
  else
    {
      text = (char *)hdfGrokReference (fname, target, url);
    }

  return text;
}


#ifdef HAVE_DTM
mo_status mo_do_hdf_dtm_thang (char *url, char *hdfdtmref)
{
  char *fname = mo_hdf_fetch_local_filename (url);
  struct stat buf;

  if (!fname || stat (fname, &buf))
    {
      /* Go fetch the file anew. */
      fname = mo_tmpnam (url);
      if (mo_pull_er_over_virgin (url, fname) == mo_fail)
        {
          return mo_fail;
        }
      else
        {
          char *cache_url = mo_url_canonicalize (url, "");
          mo_cache_data (cache_url, fname, 1);
          free (cache_url);
        }
    }
  
  hdfDtmThang (fname, hdfdtmref);

  return mo_succeed;
}
#endif /* HAVE_DTM */

#endif /* HAVE_HDF */
