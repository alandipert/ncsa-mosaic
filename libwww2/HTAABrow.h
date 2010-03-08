/*                          BROWSER SIDE ACCESS AUTHORIZATION MODULE
                                             
   This module is the browser side interface to Access Authorization (AA) package.  It
   contains code only for browser.
   
   Important to know about memory allocation:
   
   Routines in this module use dynamic allocation, but free automatically all the memory
   reserved by them.
   
   Therefore the caller never has to (and never should) free() any object returned by
   these functions.
   
   Therefore also all the strings returned by this package are only valid until the next
   call to the same function is made. This approach is selected, because of the nature of
   access authorization: no string returned by the package needs to be valid longer than
   until the next call.
   
   This also makes it easy to plug the AA package in: you don't have to ponder whether to
   free()something here or is it done somewhere else (because it is always done somewhere
   else).
   
   The strings that the package needs to store are copied so the original strings given as
   parameters to AA functions may be freed or modified with no side effects.
   
   Also note:The AA package does not free() anything else than what it has itself
   allocated.
   
 */

#ifndef HTAABROW_H
#define HTAABROW_H

#include "HTUtils.h"            /* BOOL, PARAMS, ARGS */
#include "HTAAUtil.h"           /* Common parts of AA */


#ifdef SHORT_NAMES
#define HTAAcoAu        HTAA_composeAuth
#define HTAAsRWA        HTAA_shouldRetryWithAuth
#define HTAA_TWA        HTAA_TryWithAuth
#endif /*SHORT_NAMES*/

/*

Routines for Browser Side Recording of AA Info

   Most of the browser-side AA is done by the following two functions (which are called
   from file HTTP.c so the browsers using libwww only need to be linked with the new
   library and not be changed at all):
   
      HTAA_composeAuth() composes the Authorization: line contents, if the AA package
      thinks that the given document is protected. Otherwise this function returns NULL.
      This function also calls the functions HTPrompt(),HTPromptPassword() and HTConfirm()
      to get the username, password and some confirmation from the user.
      
      HTAA_shouldRetryWithAuth() determines whether to retry the request with AA or with a
      new AA (in case username or password was misspelled).

      HTAA_TryWithAuth() sets up everything for an automatic first try with authentication.

      HTAA_ClearAuth() clears the currently allocated authentication record.
      
 */

/* PUBLIC                                               HTAA_composeAuth()
**
**      COMPOSE THE ENTIRE AUTHORIZATION HEADER LINE IF WE
**      ALREADY KNOW, THAT THE HOST MIGHT REQUIRE AUTHORIZATION
**
** ON ENTRY:
**      hostname        is the hostname of the server.
**      portnumber      is the portnumber in which the server runs.
**      docname         is the pathname of the document (as in URL)
**
** ON EXIT:
**      returns NULL, if no authorization seems to be needed, or
**              if it is the entire Authorization: line, e.g.
**
**                 "Authorization: basic username:password"
**
**              As usual, this string is automatically freed.
*/
PUBLIC char *HTAA_composeAuth PARAMS((WWW_CONST char * hostname,
                                      WWW_CONST int   portnumber,
                                      WWW_CONST char * docname));


/* BROWSER PUBLIC                               HTAA_shouldRetryWithAuth()
**
**              DETERMINES IF WE SHOULD RETRY THE SERVER
**              WITH AUTHORIZATION
**              (OR IF ALREADY RETRIED, WITH A DIFFERENT
**              USERNAME AND/OR PASSWORD (IF MISSPELLED))
** ON ENTRY:
**      start_of_headers is the first block already read from socket,
**                      but status line skipped; i.e. points to the
**                      start of the header section.
**      length          is the remaining length of the first block.
**      soc             is the socket to read the rest of server reply.
**
**                      This function should only be called when
**                      server has replied with a 401 (Unauthorized)
**                      status code.
** ON EXIT:
**      returns         YES, if connection should be retried.
**                           The node containing all the necessary
**                           information is
**                              * either constructed if it does not exist
**                              * or password is reset to NULL to indicate
**                                that username and password should be
**                                reprompted when composing Authorization:
**                                field (in function HTAA_composeAuth()).
**                      NO, otherwise.
*/
PUBLIC BOOL HTAA_shouldRetryWithAuth PARAMS((char *     start_of_headers,
                                             int        length,
                                             int        soc));


#ifdef PEM_AUTH
/* BROWSER PUBLIC                               HTAA_TryWithAuth()
**
**              SAYS WE KNOW WE SHOULD TRY THE SERVER
**              WITH AUTHORIZATION RIGHT FROM THE START
** ON ENTRY:
**      enctype         is the string we were given to determine
**                      just what type of authorization we should ask for
**                      from the start.
**      entity          is the server identifier needed by some
**                      types of authorization.
**      action          is the url we are GETing or POSTing to.
**
**                      This function should only be called when
**                      when we are responding to a form with ENCTYPE set.
** ON EXIT:
**      returns         YES
**                           The node containing all the necessary
**                           information is constructed.
**			NO
**			     Client can't do this encryption type.
*/
PUBLIC BOOL HTAA_TryWithAuth PARAMS((char *	enctype,
				     char *	entity,
				     char *	action));


/*

 */

PUBLIC void HTAA_ClearAuth NOPARAMS;
#endif /* PEM_AUTH */


#ifdef PEM_AUTH
/*
 * PUBLIC                                               HTAA_makecommand()
 * 
 *              ENCRYPT AN HTTP REQUEST, AND ENCAPSULATE IT IN
 *              A NEW HTTP PEM AUTHORIZED REQUEST
 * 
 * ON ENTRY:
 *      command         the HTTP request
 * 
 * ON EXIT:
 *      returns         the new HTTP request with PEM
 * 
 * Do not free this string. This function *requires* that the 
 * HTAA_composeAuth function has been called prior to it.
 * 
 */
PUBLIC char *HTAA_makecommand PARAMS((char * command, char **body, int *bl));
#endif /* PEM_AUTH */


PUBLIC void HTAAServer_clear ();

#endif  /* NOT HTAABROW_H */
/*

   End of file HTAABrow.h.  */
