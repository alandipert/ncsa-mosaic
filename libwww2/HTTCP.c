/*			Generic Communication Code		HTTCP.c
**			==========================
**
**	This code is in common between client and server sides.
**
**	16 Jan 92  TBL	Fix strtol() undefined on CMU Mach.
**	25 Jun 92  JFG  Added DECNET option through TCP socket emulation.
*/

/* SOCKS mods by:
 * Ying-Da Lee, <ylee@syl.dl.nec.com>
 * NEC Systems Laboratory
 * C&C Software Technology Center
 */
#include "../config.h"
#include "HTUtils.h"
#include "HTParse.h"
#include "HTAlert.h"
#include "HTAccess.h"
#include "tcp.h"		/* Defines SHORT_NAMES if necessary */
#ifdef SHORT_NAMES
#define HTInetStatus		HTInStat
#define HTInetString 		HTInStri
#define HTParseInet		HTPaInet
#endif

#ifdef __STDC__
#include <stdlib.h>
#endif

#if defined(SVR4) && !defined(SCO) && !defined(linux) && !defined(DGUX)
#include <sys/filio.h>
#endif

#if defined(DGUX)
#include <sys/file.h>
#endif

/* Apparently needed for AIX 3.2. */
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

#ifndef DISABLE_TRACE
extern int httpTrace;
extern int www2Trace;
#endif

/*	Module-Wide variables
*/

PRIVATE char *hostname=0;		/* The name of this host */


/*	PUBLIC VARIABLES
*/

/* PUBLIC SockA HTHostAddress; */	/* The internet address of the host */
					/* Valid after call to HTHostName() */

/*	Encode INET status (as in sys/errno.h)			  inet_status()
**	------------------
**
** On entry,
**	where		gives a description of what caused the error
**	global errno	gives the error number in the unix way.
**
** On return,
**	returns		a negative status in the unix way.
*/
#ifndef errno
extern int errno;
#endif /* errno */

/*	Report Internet Error
**	---------------------
*/
#ifdef __STDC__
PUBLIC int HTInetStatus(char *where)
#else
PUBLIC int HTInetStatus(where)
    char    *where;
#endif
{
#ifndef DISABLE_TRACE
    if (www2Trace) {
	fprintf(stderr, "TCP: Error %d in `errno' after call to %s() failed.\n\t%s\n",
			errno,  where, strerror(errno));
    }
#endif

    return -1;
}


/*	Parse a cardinal value				       parse_cardinal()
**	----------------------
**
** On entry,
**	*pp	    points to first character to be interpreted, terminated by
**		    non 0:9 character.
**	*pstatus    points to status already valid
**	maxvalue    gives the largest allowable value.
**
** On exit,
**	*pp	    points to first unread character
**	*pstatus    points to status updated iff bad
*/

PUBLIC unsigned int HTCardinal ARGS3
	(int *,		pstatus,
	char **,	pp,
	unsigned int,	max_value)
{
    int   n;
    if ( (**pp<'0') || (**pp>'9')) {	    /* Null string is error */
	*pstatus = -3;  /* No number where one expeceted */
	return 0;
    }

    n=0;
    while ((**pp>='0') && (**pp<='9')) n = n*10 + *((*pp)++) - '0';

    if (n>max_value) {
	*pstatus = -4;  /* Cardinal outside range */
	return 0;
    }

    return n;
}


/*	Produce a string for an Internet address
**	----------------------------------------
**
** On exit,
**	returns	a pointer to a static string which must be copied if
**		it is to be kept.
*/

PUBLIC WWW_CONST char * HTInetString ARGS1(SockA*,sin)
{
    static char string[16];
    sprintf(string, "%d.%d.%d.%d",
	    (int)*((unsigned char *)(&sin->sin_addr)+0),
	    (int)*((unsigned char *)(&sin->sin_addr)+1),
	    (int)*((unsigned char *)(&sin->sin_addr)+2),
	    (int)*((unsigned char *)(&sin->sin_addr)+3));
    return string;
}


/*	Parse a network node address and port
**	-------------------------------------
**
** On entry,
**	str	points to a string with a node name or number,
**		with optional trailing colon and port number.
**	sin	points to the binary internet or decnet address field.
**
** On exit,
**	*sin	is filled in. If no port is specified in str, that
**		field is left unchanged in *sin.
*/
PUBLIC int HTParseInet ARGS2(SockA *,sin, WWW_CONST char *,str)
{
  char *port;
  char host[256];
  struct hostent  *phost;	/* Pointer to host - See netdb.h */
  int numeric_addr;
  char *tmp;

  static char *cached_host = NULL;
  static char *cached_phost_h_addr = NULL;
  static int cached_phost_h_length = 0;

  strcpy(host, str);		/* Take a copy we can mutilate */

  /* Parse port number if present */
  if (port=strchr(host, ':'))
    {
      *port++ = 0;		/* Chop off port */
      if (port[0]>='0' && port[0]<='9')
        {
          sin->sin_port = htons(atol(port));
	}
    }

  /* Parse host number if present. */
  numeric_addr = 1;
  for (tmp = host; *tmp; tmp++)
    {
      /* If there's a non-numeric... */
      if ((*tmp < '0' || *tmp > '9') && *tmp != '.')
        {
          numeric_addr = 0;
          goto found_non_numeric_or_done;
        }
    }

 found_non_numeric_or_done:
  if (numeric_addr)
    {   /* Numeric node address: */
      sin->sin_addr.s_addr = inet_addr(host); /* See arpa/inet.h */
    }
  else
    {		    /* Alphanumeric node name: */
      if (cached_host && (strcmp (cached_host, host) == 0))
        {
#if 0
          fprintf (stderr, "=-= Matched '%s' and '%s', using cached_phost.\n",
                   cached_host, host);
#endif
          memcpy(&sin->sin_addr, cached_phost_h_addr, cached_phost_h_length);
        }
      else
        {
	  extern int h_errno;
#if 0
          fprintf (stderr, "=+= Fetching on '%s'\n", host);
#endif
          phost = gethostbyname (host);
          if (!phost)
            {
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf
                  (stderr,
                   "HTTPAccess: Can't find internet node name `%s'.\n",host);
#endif
              return -1;  /* Fail? */
            }

          /* Free previously cached strings. */
          if (cached_host) {
            free (cached_host);
	    cached_host=NULL;
	  }
          if (cached_phost_h_addr) {
            free (cached_phost_h_addr);
	    cached_phost_h_addr=NULL;
	  }

          /* Cache new stuff. */
          cached_host = strdup (host);
          cached_phost_h_addr = calloc (phost->h_length + 1, 1);
          memcpy (cached_phost_h_addr, phost->h_addr, phost->h_length);
#if 0
          cached_phost_h_addr = strdup (phost->h_addr);
#endif
          cached_phost_h_length = phost->h_length;

          memcpy(&sin->sin_addr, phost->h_addr, phost->h_length);
        }
    }

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf(stderr,
            "TCP: Parsed address as port %d, IP address %d.%d.%d.%d\n",
            (int)ntohs(sin->sin_port),
            (int)*((unsigned char *)(&sin->sin_addr)+0),
            (int)*((unsigned char *)(&sin->sin_addr)+1),
            (int)*((unsigned char *)(&sin->sin_addr)+2),
            (int)*((unsigned char *)(&sin->sin_addr)+3));
#endif

  return 0;	/* OK */
}


/*	Derive the name of the host on which we are
**	-------------------------------------------
**
*/
#ifdef __STDC__
PRIVATE void get_host_details(void)
#else
PRIVATE void get_host_details()
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64		/* Arbitrary limit */
#endif

{
    char name[MAXHOSTNAMELEN+1];	/* The name of this host */
#ifdef NEED_HOST_ADDRESS		/* no -- needs name server! */
    struct hostent * phost;		/* Pointer to host -- See netdb.h */
#endif
    int namelength = sizeof(name);

    if (hostname) return;		/* Already done */
    gethostname(name, namelength);	/* Without domain */

#ifndef DISABLE_TRACE
    if (www2Trace) {
	fprintf(stderr, "TCP: Local host name is %s\n", name);
    }
#endif

    StrAllocCopy(hostname, name);

#ifdef NEED_HOST_ADDRESS		/* no -- needs name server! */
    phost=gethostbyname(name);		/* See netdb.h */
    if (!phost) {
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr,
		"TCP: Can't find my own internet node address for `%s'!!\n",
		name);
#endif
	return;  /* Fail! */
    }
    StrAllocCopy(hostname, phost->h_name);
    memcpy(&HTHostAddress, &phost->h_addr, phost->h_length);
#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr, "     Name server says that I am `%s' = %s\n",
	    hostname, HTInetString(&HTHostAddress));
#endif
#endif
}

#ifdef __STDC__
PUBLIC char * HTHostName(void)
#else
PUBLIC char * HTHostName()
#endif
{
    get_host_details();
    return hostname;
}

#ifdef SOCKS
struct in_addr SOCKS_ftpsrv;
#endif


PUBLIC int HTDoConnect (char *url, char *protocol, int default_port, int *s)
{
  struct sockaddr_in soc_address;
  struct sockaddr_in *sin = &soc_address;
  int status;

  /* Set up defaults: */
  sin->sin_family = AF_INET;
  sin->sin_port = htons(default_port);

  /* Get node name and optional port number: */
  {
    char line[256];
    char *p1 = HTParse(url, "", PARSE_HOST);
    int status;

    sprintf (line, "Looking up %s.", p1);
    HTProgress (line);

    status = HTParseInet(sin, p1);
    if (status)
      {
        sprintf (line, "Unable to locate remote host %s.", p1);
        HTProgress(line);
        free (p1);
        return HT_NO_DATA;
      }

    sprintf (line, "Making %s connection to %s.", protocol, p1);
    HTProgress (line);
    free (p1);
  }

  /* Now, let's get a socket set up from the server for the data: */
  *s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

#ifdef SOCKS
  /* SOCKS can't yet deal with non-blocking connect request */
  HTClearActiveIcon();
  status = Rconnect(*s, (struct sockaddr*)&soc_address, sizeof(soc_address));
  if ((status == 0) && (strcmp(protocol, "FTP") == 0))
     SOCKS_ftpsrv.s_addr = soc_address.sin_addr.s_addr;
  {
    int intr;
    intr = HTCheckActiveIcon(1);
    if (intr)
      {
#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf (stderr, "*** INTERRUPTED in middle of connect.\n");
#endif
        status = HT_INTERRUPTED;
        errno = EINTR;
      }
  }
  return status;
#else /* SOCKS not defined */


  /*
   * Make the socket non-blocking, so the connect can be canceled.
   * This means that when we issue the connect we should NOT
   * have to wait for the accept on the other end.
   */
  {
    int ret;
    int val = 1;
    char line[256];

    ret = ioctl(*s, FIONBIO, &val);
    if (ret == -1)
      {
        sprintf (line, "Could not make connection non-blocking.");
        HTProgress(line);
      }
  }
  HTClearActiveIcon();

  /*
   * Issue the connect.  Since the server can't do an instantaneous accept
   * and we are non-blocking, this will almost certainly return a negative
   * status.
   */
  status = connect(*s, (struct sockaddr*)&soc_address, sizeof(soc_address));

  /*
   * According to the Sun man page for connect:
   *     EINPROGRESS         The socket is non-blocking and the  con-
   *                         nection cannot be completed immediately.
   *                         It is possible to select(2) for  comple-
   *                         tion  by  selecting the socket for writ-
   *                         ing.
   * According to the Motorola SVR4 man page for connect:
   *     EAGAIN              The socket is non-blocking and the  con-
   *                         nection cannot be completed immediately.
   *                         It is possible to select for  completion
   *                         by  selecting  the  socket  for writing.
   *                         However, this is only  possible  if  the
   *                         socket  STREAMS  module  is  the topmost
   *                         module on  the  protocol  stack  with  a
   *                         write  service  procedure.  This will be
   *                         the normal case.
   */
#ifdef SVR4
  if ((status < 0) && ((errno == EINPROGRESS)||(errno == EAGAIN)))
#else
  if ((status < 0) && (errno == EINPROGRESS))
#endif /* SVR4 */
    {
      struct timeval timeout;
      int ret;

      ret = 0;
      while (ret <= 0)
	{
          fd_set writefds;
          int intr;

          FD_ZERO(&writefds);
          FD_SET(*s, &writefds);

	  /* linux (and some other os's, I think) clear timeout...
	     let's reset it every time. bjs */
	  timeout.tv_sec = 0;
	  timeout.tv_usec = 100000;

#ifdef __hpux
          ret = select(FD_SETSIZE, NULL, (int *)&writefds, NULL, &timeout);
#else
          ret = select(FD_SETSIZE, NULL, &writefds, NULL, &timeout);
#endif
	  /*
	   * Again according to the Sun and Motorola man pagse for connect:
           *     EALREADY            The socket is non-blocking and a  previ-
           *                         ous  connection attempt has not yet been
           *                         completed.
           * Thus if the errno is NOT EALREADY we have a real error, and
	   * should break out here and return that error.
           * Otherwise if it is EALREADY keep on trying to complete the
	   * connection.
	   */
          if ((ret < 0)&&(errno != EALREADY))
            {
              status = ret;
              break;
            }
          else if (ret > 0)
            {
	      /*
	       * Extra check here for connection success, if we try to connect
	       * again, and get EISCONN, it means we have a successful
	       * connection.
	       */
              status = connect(*s, (struct sockaddr*)&soc_address,
                               sizeof(soc_address));
              if ((status < 0)&&(errno == EISCONN))
                {
                  status = 0;
                }
              break;
            }
	  /*
	   * The select says we aren't ready yet.
	   * Try to connect again to make sure.  If we don't get EALREADY
	   * or EISCONN, something has gone wrong.  Break out and report it.
	   * For some reason SVR4 returns EAGAIN here instead of EALREADY,
	   * even though the man page says it should be EALREADY.
	   */
          else
            {
              status = connect(*s, (struct sockaddr*)&soc_address,
                               sizeof(soc_address));
#ifdef SVR4
              if ((status < 0)&&(errno != EALREADY)&&(errno != EAGAIN)&&
			(errno != EISCONN))
#else
              if ((status < 0)&&(errno != EALREADY)&&(errno != EISCONN))
#endif /* SVR4 */
                {
                  break;
                }
            }
          intr = HTCheckActiveIcon(1);
          if (intr)
            {
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf (stderr, "*** INTERRUPTED in middle of connect.\n");
#endif
              status = HT_INTERRUPTED;
              errno = EINTR;
              break;
            }
	}
    }

  /*
   * Make the socket blocking again on good connect
   */
  if (status >= 0)
    {
      int ret;
      int val = 0;
      char line[256];

      ret = ioctl(*s, FIONBIO, &val);
      if (ret == -1)
	{
          sprintf (line, "Could not restore socket to blocking.");
          HTProgress(line);
	}
    }
  /*
   * Else the connect attempt failed or was interrupted.
   * so close up the socket.
   */
  else
    {
	close(*s);
    }

  return status;
#endif /* #ifdef SOCKS */
}

/* This is so interruptible reads can be implemented cleanly. */
int HTDoRead (int fildes, void *buf, unsigned nbyte)
{
  int ready, ret, intr;
  fd_set readfds;
  struct timeval timeout;
  char *adtestbuf;

  ready = 0;
  while (!ready)
    {
        FD_ZERO(&readfds);
        FD_SET(fildes, &readfds);

	  /* linux (and some other os's, I think) clear timeout...
	     let's reset it every time. bjs */
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

#ifdef __hpux
        ret = select(FD_SETSIZE, (int *)&readfds, NULL, NULL, &timeout);
#else
        ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
#endif
        if (ret < 0)
          {
                return -1;
          }
        else if (ret > 0)
          {
                ready = 1;
          }
        else
          {
                intr = HTCheckActiveIcon(1);
                if (intr)
                  {
                        return HT_INTERRUPTED;
                  }
          }
    }

  ret = read (fildes, buf, nbyte);

#ifndef DISABLE_TRACE
  if (httpTrace) {
	adtestbuf = buf;
	for (intr = 0; intr < ret; fprintf(stderr,"%c",adtestbuf[intr++]) ) ;
  }
#endif

  return ret;
}
