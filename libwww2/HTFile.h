/*                                                                      File access in libwww
                                       FILE ACCESS
                                             
   These are routines for local file access used by WWW browsers and
   servers. Implemented by HTFile.c.
   
   If the file is not a local file, then we pass it on to HTFTP in
   case it can be reached by FTP.
   
 */
#ifndef HTFILE_H
#define HTFILE_H

#include "HTFormat.h"
#include "HTAccess.h"
#include "HTML.h"               /* SCW */



/*

Controlling globals

   These flags control how directories and files are represented as
   hypertext, and are typically set by the application from command
   line options, etc.
   
 */
extern int HTDirAccess; /* Directory access level */

#define HT_DIR_FORBID           0       /* Altogether forbidden */
#define HT_DIR_SELECTIVE        1       /* If HT_DIR_ENABLE_FILE exists */
#define HT_DIR_OK               2       /* Any accesible directory */

#define HT_DIR_ENABLE_FILE      ".www_browsable" /* If exists, can browse */

extern int HTDirReadme;         /* Include readme files in listing? */
                                        /* Values: */
#define HT_DIR_README_NONE      0       /* No */
#define HT_DIR_README_TOP       1       /* Yes, first */
#define HT_DIR_README_BOTTOM    2       /* Yes, at the end */

#define HT_DIR_README_FILE              "README"

extern HTList *HTSuffixes;

/*

Convert filenames between local and WWW formats

 */
extern char * HTLocalName PARAMS((WWW_CONST char * name));


/*

Make a WWW name from a full local path name

 */
extern char * WWW_nameOfFile PARAMS((char * name));


/*

Generate the name of a cache file

 */
extern char * HTCacheFileName PARAMS((WWW_CONST char * name));


/*

Output directory titles

   This is (like the next one) used by HTFTP. It is common code to
   generate the title and heading 1 and the parent directory link for
   any anchor.
   
 */
extern void HTDirTitles PARAMS((
        HTStructured *  target,
        HTAnchor *      anchor));

/*

Output a directory entry

   This is used by HTFTP.c for example -- it is a common routine for
   generating a linked directory entry.
   
 */
extern void HTDirEntry PARAMS((
        HTStructured *  target,         /* in which to put the linked text */
        WWW_CONST char *    tail,           /* last part of directory name */
        WWW_CONST char *    entry));        /* name of this entry */

/*

HTSetSuffix: Define the representation for a file suffix

   This defines a mapping between local file suffixes and file content
   types and encodings.
   
  ON ENTRY,
  
  suffix includes the "." if that is important (normally, yes!)
                         
  representation is MIME-style content-type
                         
  encoding is MIME-style content-transfer-encoding (8bit, 7bit, etc)
                         
  quality an a priori judgement of the quality of such files
  (0.0..1.0)
                         
 */
/* Example:   HTSetSuffix(".ps", "application/postscript", "8bit", 1.0);
**
*/

extern void HTSetSuffix PARAMS((
        WWW_CONST char *    suffix,
        WWW_CONST char *    representation,
        WWW_CONST char *    encoding,
        float           quality));
        

/*

HTFileFormat: Get Representation and Encoding from file name

  ON EXIT,
  
  return                 The represntation it imagines the file is in
                         
  *pEncoding             The encoding (binary, 7bit, etc). See HTSetSuffix.
                         
 */

#define COMPRESSED_NOT    0
#define COMPRESSED_BIGZ   1
#define COMPRESSED_GNUZIP 2

extern HTFormat HTFileFormat PARAMS((
                char *    filename,
                HTAtom **       pEncoding,
                HTAtom *,
                int *compressed));
extern char * HTFileMimeType PARAMS((
                WWW_CONST char *    filename,
                WWW_CONST char *    default_type));
extern char *HTDescribeURL (char *);

/*

Determine file value from file name

 */


extern float HTFileValue PARAMS((
                WWW_CONST char * filename));


/*

Determine write access to a file

  ON EXIT,
  
  return value YES if file can be accessed and can be written to.
                         
 */

/*

  BUGS
  
   Isn't there a quicker way?
   
 */


extern BOOL HTEditable PARAMS((WWW_CONST char * filename));


/*

Determine a suitable suffix, given the representation

  ON ENTRY,
  
  rep                     is the atomized MIME style representation
                         
  ON EXIT,
  
  returns a pointer to a suitable suffix string if one has been found,
                         else NULL.
                         
 */
extern WWW_CONST char * HTFileSuffix PARAMS((
                HTAtom* rep));



/*

The Protocols

 */
extern HTProtocol HTFTP, HTFile;

#endif /* HTFILE_H */
