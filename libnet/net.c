/*
 * Copyright (C) 1992, Board of Trustees of the University of Illinois.
 *
 * Permission is granted to copy and distribute source with out fee.
 * Commercialization of this product requires prior licensing
 * from the National Center for Supercomputing Applications of the
 * University of Illinois.  Commercialization includes the integration of this 
 * code in part or whole into a product for resale.  Free distribution of 
 * unmodified source and use of NCSA software is not considered 
 * commercialization.
 *
 */
#if ! defined(lint) && ! defined(LINT)
static char rcs_id[] = "$Id: net.c,v 1.4 1995/10/15 18:48:57 spowers Exp $";
#endif

/* $Log: net.c,v $
 * Revision 1.4  1995/10/15 18:48:57  spowers
 * bzero and bcopy iced.
 *
 * Revision 1.3  1995/10/14 22:07:34  spowers
 * Bzero and Bcopy removed...memset memcpy used instead.
 *
 * Revision 1.2  1995/10/09 21:49:23  spowers
 * i have no idea
 *
 * Revision 1.1.1.1  1995/01/11  00:03:38  alanb
 * New CVS source tree, Mosaic 2.5 beta 4
 *
 * Revision 2.5  1994/12/29  23:42:05  alanb
 * I'm committing with a new symbolic revision number.
 *
 * Revision 1.1.1.1  1994/12/28  21:37:42  alanb
 *
 * Revision 1.5  1994/03/28  17:20:49  gbourhis
 * Update to be more in synch with the collage distribution.
 *
 * Revision 1.4  1993/10/31  06:30:23  marca
 * Tweaks.
 *
 * Revision 1.3  1993/10/25  06:23:25  marca
 * Tweaks.
 *
 * Revision 1.2  1993/07/19  06:35:51  marca
 * Removed stupid if 0.
 *
 * Revision 1.1.1.1  1993/07/04  00:03:20  marca
 * Mosaic for X version 2 distribution
 *
 * Revision 1.8  1993/06/03  18:47:35  davet
 * Version name change
 *
 * Revision 1.7  1993/05/17  16:47:10  gbourhis
 * remove the warning for 3D data.
 *
 * Revision 1.6  1993/05/05  19:22:12  gbourhis
 * Add rcs Id and Log. handling of the new associated field, new param for
 * NetPALDistribute & NetSendPalette8.
 * Changes in NetArrayDistribute() & NetRISDistribute().
 * call failCB() parameter in NetClientSendMessage() if not connected.
 * give NetFreeDataCB as the failCB parameter of NetClientSendMessage in
 * most case.
 *
 *
 *  mods: 
 * 2/22/93 (ddt)  print out message if user has loaded unsupported 
 *		 dimensional array
*/

#include <stdio.h>
#include <sys/time.h>

#include <libdtm/dtm.h>
#include <libdtm/sds.h>
#include <libdtm/ris.h>
#include <libdtm/text.h>
#include <libdtm/srv.h>
#include <libdtm/col.h>
#include <libdtm/anim.h>
#include <libdtm/vdata.h>
#include <libdtm/sdl.h>
#include <libdtm/com.h>
#include <libdtm/exec.h>

#define FREE	free
#include "list.h"
#include "netdata.h"
#include "netP.h"
#include "doodle.h"
#include "collage.h"

#define TRUE 1
#define FALSE 0

/* USE_AVAIL_WRITE will allow for a send queue.  only send when reader ready*/
#define USE_AVAIL_WRITE
/* USE_WRITEMSG if set should reduce number packets sent on write */
#define USE_WRITEMSG
/* USE_FEEDBACK_CALLS if set will call UI routines when DTM traffic is present*/
/* #define USE_FEEDBACK_CALLS */

#define VERSION_STRING	"1.2"
#define VERSION_NUMBER  2

#ifndef DTM_STRING_SIZE
#define DTM_STRING_SIZE 1024
#endif

#define MAX_SDL_VERTICES	10000


/* docb(dataObject,client_data); */
typedef struct {	/* Data Object Call Back */
	char *moduleName;
	void (*newCB)();
	caddr_t newData;
	void (*changeCB)();
	caddr_t changeData;
	void (*destroyCB)();
	caddr_t destroyData;
	} DOCB;

typedef struct {	 /* Send Queue */
	NetPort *netPort;
	char	*header;
	char	*data;
	long	num;
	DTMTYPE	type;
	int	numTries;	/* number of attempted sends */
	void	(*cb)();	/* called on succes cb(data,cbData) */
	caddr_t	cbData;		/* call back data */
	void	(*failCB)();	/* called on failure failCB(data,failCBData) */
	caddr_t	failCBData;	/* fail call back data */
	} SQueue;

typedef struct {
	caddr_t internal;
	void (*cb)();
	caddr_t cbData;
	void (*failCB)();
	caddr_t failCBData;
	} ExecCBData;


static List netInList;
static List netOutList;

static List sendQueue;

static List ANIMList;
static List RIS8List;
static List SDSList;
static List PALList;
static List TXTList;
static List SRVList;
static List DTMList;
static List COLList;
static List VDATAList;
static List SDLList;
static List COMList;
static List EXECList;
static List MSGList;
static List userList;
static int NetSendConnect();
static int NetFlushPort();
static int dtmFlowControl = (int)DTM_SYNC;

static int netTimeOut;
static int netMaxAttemptsToSend;

#ifdef USE_FEEDBACK_CALLS
extern void SetReadFeedback();
extern void SetWriteFeedback();
extern void UnsetFeedback();
#endif

static char *userID = NULL;

void NetSetASync(set)
/* set DTM to behave in either async or syncronous mode */
int	set; /* Boolean */
{
	if (set)  {
		dtmFlowControl = (int)DTM_ASYNC;
#ifdef DEBUG
		printf("NetSetASync(): setting to DTM_ASYNC\n");
#endif
		}
	else {
		dtmFlowControl = (int)DTM_SYNC;
#ifdef DEBUG
		printf("NetSetASync(): setting to SYNC\n");
#endif
		}
}

void NetSetTimeOut(seconds)
int seconds;
{
	netTimeOut = seconds;
}

int NetGetTimeOut()
{
	return(netTimeOut);
}

void NetSetMaxAttemptsToSend(numberTries)
int numberTries;
{
	netMaxAttemptsToSend = numberTries;
}

int NetGetMaxAttemptsToSend()
{
	return(netMaxAttemptsToSend);
}

char *NetDTMErrorString(error)
{
    switch (error) {
	case DTMNOERR:   return(" no error ");
	case DTMMEM:     return(" (1) Out of memory ");
	case DTMHUH:     return(" (2) Unknown port definition ");
	case DTMNOPORT:  return(" (3) No DTM ports available ");
	case DTMPORTINIT:return(" (4) DTM port not initialized ");
	case DTMCALL:    return(" (5) calling routines in wrong order ");
	case DTMEOF:     return(" (6) EOF error ");
	case DTMSOCK:    return(" (7) Socket error ");
	case DTMHOST:    return(" (8) That hostname is not found/bad ");
	case DTMTIMEOUT: return(" (9) Timeout waiting for connection ");
	case DTMCCONN:   return(" (10) DTM cannot connect (network down?) ");
	case DTMREAD:    return(" (11) error returned from system read ");
	case DTMWRITE:   return(" (12) error returned from system write(v) ");
	case DTMHEADER:  return(" (13) Header to long for buffer ");
	case DTMSDS:     return(" (14) SDS error ");
	case DTMSELECT:  return(" (15) Select call error ");
	case DTMENV:     return(" (16) Environment not setup ");
	case DTMBUFOVR:  return(" (17) User buffer overflow ");
	case DTMCORPT:   return(" (18) Port table corrupted ");
	case DTMBADPORT: return(" (19) Port identifier is bad/corrupt/stale ");
	case DTMBADACK:  return(" (20) Bad ack to internal flow control ");
	case DTMADDR:    return(" (21) Bad address ");
	case DTMSERVER:  return(" (22) Problem communicating with the server ");
	default:	 return(" Unknown error ");
	};
}

NetPort *NetIsConnected()
/* is collage connected? if so, return port */
{
	return((NetPort *)ListHead(netOutList));
}

static void NetRemovePortFromSendQueue(netPort)
NetPort *netPort;
{
SQueue *sq;

	sq = (SQueue *) ListHead(sendQueue);
	while(sq) {
		if (sq->netPort == netPort) {
			ListDeleteEntry(sendQueue,sq);
			if (sq->failCB) 
				sq->failCB(sq->data,sq->failCBData);
			FREE(sq->header);
			FREE(sq);
			sq = (SQueue *) ListCurrent(sendQueue);
			}
		else {
			sq = (SQueue *) ListNext(sendQueue);
			}
		}
}

static int NetUserListAdd(name)
/* return 1 if new user else 0; -1 on error*/
char *name;
{
char *p;
	
	if ((!name) || (!strlen(name)))
		return(-1);
	p = (char *) ListHead(userList);
	while(p) {
		if (!strcmp(p,name)) {
			return(0);
			}
		p = (char *) ListNext(userList);
		}

	if (!(p = (char *) MALLOC(strlen(name)+1))) {
		ErrMesg("Out of memory adding new user\n");
		return(-1);
		}
	strcpy(p,name);
	ListAddEntry(userList,p);
	return(1);
}

static int
DEFUN(NetUserListRemove,(name),char *name)
{
	int rc;
	return name ?
		(rc = ListDeleteEntry(userList, name), FREE(name), rc) : -1;
}

int NetGetListOfUsers(max,users)
int max; 	/* size of users array */
char **users;	/* List of users put in here */
/* returns the number of users */
{
int count;
char *p;

	p = (char *) ListHead(userList);
	count= 0;
	while(p && (count < max)) {
		users[count++] = p;
		p = (char *) ListNext(userList);
		}
	return(count);
}

void DEFUN(NetSetUserID,(user), char *user)
{
	if (user && userID && strcmp(userID, user) ||
	    !user && userID)
		NetUserListRemove(userID);

	if (user && userID && strcmp(userID, user) ||
	    !userID && user) {
		if (!(userID = (char *) MALLOC(strlen(user)+1))) {
			ErrMesg("Out of memory setting user id\n");
			return;
		}
		strcpy(userID,user);
		ListAddEntry(userList,userID);
	}
	else if (!user && userID)
		userID = NULL;
}
		

static DOCB *NetSearchByName(name,list)
char *name;
List list;
{
DOCB *docb;

	docb = (DOCB *) ListHead(list);
	while (docb) {
		if (!strcmp(docb->moduleName,name)) {
			return(docb);
			}
		docb = (DOCB *) ListNext(list);
		}
	return((DOCB *) 0);
	
}


int NetRegisterModule(name,netType,new,newData,change,changeData,
		 			destroy,destroyData)
char *name;		/* module Name */
NetType netType;	/* DTM class */
void (*new)();		/* New data Object callback */
caddr_t newData;
void (*change)();	/* Data object has changed callback */
caddr_t changeData;
void (*destroy)();	/* Data object destroyed callback */
caddr_t destroyData;
{
	DOCB *docb; 
	char	itsNew;

	/*Yeah this is huge,repetitive and could easily be condensed, but it
	  wasn't when I started, and I don't feel like changing it now 
	  condensed on Feb 93 by gbourhis */
#define REGISTERMODULE(list)						\
 do {									\
	if (!(docb = NetSearchByName(name,list)) ) {			\
		if (!(docb =(DOCB *) MALLOC(sizeof(DOCB)))){		\
			ErrMesg("Out of Memory\n");			\
			return(0);					\
			}						\
		if (!(docb->moduleName = (char *)			\
					MALLOC(strlen(name)+1))) {	\
			ErrMesg("Out of Memory\n");			\
			return(0);					\
			}						\
		strcpy(docb->moduleName,name);				\
		itsNew = TRUE;						\
		}							\
	else								\
		itsNew = FALSE;						\
	docb->newCB = new;						\
	docb->changeCB = change;					\
	docb->destroyCB = destroy;					\
	docb->newData = newData;					\
	docb->changeData = changeData;					\
	docb->destroyData = destroyData;				\
	if (itsNew)							\
		ListAddEntry(list,(char *)docb);			\
    } while (0)

	switch (netType) {
		case NETRIS8:
			REGISTERMODULE(RIS8List);
			break;
		case NETSDS:
			REGISTERMODULE(SDSList);
			break;
		case NETANIM:
			REGISTERMODULE(ANIMList);
			break;
		case NETPAL:
			REGISTERMODULE(PALList);
			break;
		case NETTXT:
			REGISTERMODULE(TXTList);
			break;
		case NETCOL:
			REGISTERMODULE(COLList);
			break;
		case NETSRV:
			REGISTERMODULE(SRVList);
			break;
		case NETDTM:
			REGISTERMODULE(DTMList);
			break;
		case NETVDATA:
			REGISTERMODULE(VDATAList);
			break;
		case NETSDL:
			REGISTERMODULE(SDLList);
			break;
		case NETCOM:
			REGISTERMODULE(COMList);
			break;
		case NETEXEC:
			REGISTERMODULE(EXECList);
			break;
		case NETMSG:
			REGISTERMODULE(MSGList);
			break;
		default:
#ifdef DEBUG
			fprintf(stderr,"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
			fprintf(stderr,"Internal Error: NetRegisterModule():");
			fprintf(stderr,"Unknown type\n");
#endif
			return(0);
		};


	
	return(1);
}



static int DimensionsEqual(dim1,rank1,dim2,rank2) 
int *dim1,rank1,*dim2,rank2;
{
register int x;
	if (rank1 != rank2)
		return(0);
	for (x=0; x < rank1; x++) {
		if (dim1[x] != dim2[x])
			return(0);
		}
	return(1);
}


static void CopyDimensions(dim1,rank1,dim2,rank2) 
/* copies dim2 to dim1 */
int *dim1,*rank1;
int *dim2,rank2;
{
register int x;

	for (x = 0; x < rank2; x++) {
		dim1[x] = dim2[x];
		}
	*rank1 = rank2;
}



static NetPort *NetPortNew()
{
NetPort *n;

	if (!(n = (NetPort *) MALLOC(sizeof(NetPort)))) {
		return(0);
		}
	n->port = 0;
	n->portName[0] = (char) 0;
	n->open = FALSE;
	n->type = NET_UNDEF;
	n->queueTime = 0;
	return(n);
}

void NetDestroyPort(netPort)
NetPort *netPort;
{
NetPort *np;
SQueue *sq;

	switch (netPort->type) {
		case NET_IN:
			np = (NetPort *) ListHead(netInList);
			while (np) {
				if (np == netPort) {
					ListDeleteEntry(netInList,netPort);
					np = (NetPort *) ListCurrent(netInList);
					}
				else {
					np = (NetPort *) ListNext(netInList);
					}
				}
			break;
		case NET_OUT:
			np = (NetPort *) ListHead(netOutList);
			while (np) {
				if (np == netPort) {
					ListDeleteEntry(netOutList,netPort);
					np = (NetPort *)ListCurrent(netOutList);
					}
				else {
					np = (NetPort *) ListNext(netOutList);
					}
				}
			NetRemovePortFromSendQueue(netPort);
			break;
		};
	if (netPort->open)
	  DTMdestroyPort(netPort->port);
	FREE(netPort);
}

NetPort *NetCreateInPort(inPortAddr)
char *inPortAddr;
{
int in;
NetPort *n;

#ifdef DEBUG
	printf("NetCreateInPort(\"%s\"): I've been called\n",inPortAddr);
#endif
	if ((!inPortAddr) || (!strlen(inPortAddr))){
		inPortAddr = ":0";
		}
#ifdef DEBUG
	if (dtmFlowControl == DTM_ASYNC)
		fprintf(stderr, "DTMmakeInPort(DTM_ASYNC)\n");
	else
		fprintf(stderr, "DTMmakeInPort(DTM_SYNC)\n");
#endif
        if ( DTMERROR == (in = DTMmakeInPort(inPortAddr, dtmFlowControl))) {
                ErrMesg(stderr,"Can't make DTM in port %s: %s\n",
                                        inPortAddr,DTMerrmsg(1));
                return(0);
                }

	if (!(n = NetPortNew())) {
		ErrMesg("Out of Memory");
		return(0);
		}

	n->port = in;
	n->open = TRUE;
	DTMgetPortAddr(n->port,n->portName,PORTNAMESIZE);
	n->type = NET_IN;

#ifdef DEBUG
	if (dtmFlowControl == DTM_ASYNC) {
		printf("Just made an ASYNC in port %s (%d)\n",
				n->portName,n->port);
		}
        else if (dtmFlowControl == DTM_SYNC) {
		printf("Just made an SYNC in port %s (%d)\n",
				n->portName,n->port);
		}
	else {
		printf("********Just made an *UNKOWN* in port %s (%d)\n",
				n->portName,n->port);
		printf("yow.... check me out\n");
		}
#endif

	ListAddEntry(netInList,(char *)n);
	return(n);
}



static NetPort *NetInternalCreateOutPort(outPortAddr,sendConnect)
char *outPortAddr;
int sendConnect;	/* Send connect message */
			/* connect protocol requires this be done */
{
int out;
NetPort *n;
char **portNames;
int numPortNames;


#ifdef DEBUG
	printf("NetCreateOutPort(\"%s\"): I've been called\n",outPortAddr);
#endif
	if ((!outPortAddr) || (!strlen(outPortAddr)))
		return(0);
#ifdef DEBUG
	if (dtmFlowControl == DTM_ASYNC)
		fprintf(stderr, "DTMmakeOutPort(DTM_ASYNC)\n");
	else
		fprintf(stderr, "DTMmakeOutPort(DTM_SYNC)\n");
#endif
        if ( DTMERROR == (out = DTMmakeOutPort(outPortAddr, dtmFlowControl))) {
                ErrMesg(stderr,"Can't make DTM out port %s: %s\n",
                                        outPortAddr,DTMerrmsg(1));
                return(0);
                }

	if (!(n = NetPortNew())) {
		ErrMesg("Out of Memory");
		return(0);
		}

	n->port = out;
	n->open = TRUE;
	n->type = NET_OUT;

        DTMgetRemotePortAddr(n->port,&portNames,&numPortNames);
        if (numPortNames) {
                strncpy(n->portName,portNames[0],PORTNAMESIZE);
                }
        else {
                /* use address passed in */
                strncpy(n->portName,outPortAddr,PORTNAMESIZE);
                }

#ifdef DEBUG
        if (dtmFlowControl == DTM_ASYNC) {
                printf("Just made an ASYNC out netPort=%x port %x (%s)\n",
                                n,n->port,n->portName);
                }
        else if (dtmFlowControl == DTM_SYNC) {
                printf("Just made an SYNC out netPort=%x port %x (%s)\n",
                                n,n->port,n->portName);
                }       
        else {
                printf("********Just made an *UNKOWN* out port %s (%d)\n",
                                n->portName,n->port);
                printf("yow.... check me out\n");
                }
#endif

	ListAddEntry(netOutList,(char *)n);

	if (sendConnect) {
		NetSendConnect((NetPort *) ListHead(netInList),n,0);
		}
	return(n);
}


NetPort *NetCreateOutPort(outPortAddr)
char *outPortAddr;
{
	return(NetInternalCreateOutPort(outPortAddr,TRUE));
}

static void NetChangeOutPort(address,oldOut) 
char *address;
NetPort *oldOut;
{
NetPort *n;

#ifdef DEBUG
	printf("Changing OutPort Address from netPort=%x port=%x (%s) to %s\n",
			oldOut,oldOut->port,oldOut->portName,address);
#endif
	DTMdestroyPort(oldOut->port);
	n = NetInternalCreateOutPort(address,FALSE);
	oldOut->port = n->port;
	strcpy(oldOut->portName,n->portName);
	ListDeleteEntry(netOutList,n);
	NetSendConnect((NetPort *) ListHead(netInList),oldOut,0);
#ifdef DEBUG
	printf("NetChangeOut: now netPort = %x port %x (%s) \n",
			oldOut, oldOut->port, oldOut->portName);
#endif
#ifdef DEBUG
	{
	NetPort *bla;
	bla = (NetPort *) ListHead(netOutList);
	while (bla) {
		printf("In netOutList: now netPort = %x port %x (%s) \n",
			bla, bla->port, bla->portName);
		bla = (NetPort *) ListNext(netOutList);
		}
	}
#endif
	FREE(n);
}


int DEFUN(NetInit,(user), char *user)
{
	InitData();
	netInList = ListCreate();
	netOutList = ListCreate();

	sendQueue = ListCreate();

	ANIMList = ListCreate();
	RIS8List = ListCreate();
	SDSList = ListCreate();
	PALList = ListCreate();

	TXTList = ListCreate();
	SRVList = ListCreate();
	DTMList = ListCreate();
	COLList = ListCreate();
	VDATAList = ListCreate();
	SDLList = ListCreate();
	COMList = ListCreate();
	EXECList = ListCreate();
	MSGList = ListCreate();

	userList = ListCreate();
	NetSetUserID(user);

	NetSetTimeOut(30);
	NetSetMaxAttemptsToSend(200);

	return(1);
}


static void NetReject(in,header)
int in;
char *header;
{
char buff[DTM_MAX_HEADER+50];

#ifdef DEBUG
	sprintf(buff,"Rejecting:  %s\n",header);
	WriteMesg(buff);
#endif
	DTMendRead(in);
}

static Data *NetReadSDL(n,header)
NetPort *n;
char *header;
{
#if 0
char    title[DTM_STRING_SIZE];
char    id[DTM_STRING_SIZE];
struct	DTM_TRIPLET	primbuff[MAX_SDL_VERTICES];
Data *d;
					/* SDL header doesn't say how big */
					/* it is so have to alloc max */
int	num;

	SDLgetTitle(header,title,DTM_STRING_SIZE);
	if (!(d = DataNew()))
		return(0);
	if (!(d->label = (char *) MALLOC(strlen(title)+1))) {
		ErrMesg("Out of Memory reading in vdata label \n");
        	DTMendRead(n->port);
		return(0);
		}
	strcpy(d->label,title);
	d->dot = DOT_SDL;
	SDLgetPrimitive(header,&(d->dost));
	
	if ((num = DTMreadDataset(n->port, primbuff, 
		MAX_SDL_VERTICES, DTM_TRIPLET)) == DTMERROR){
			ErrMesg("Error reading DTM SDL");
			NetReject(n->port,header);
			return(0);
			}
       	DTMendRead(n->port);

	if (!(d->data = (char *) MALLOC(num))) {
		ErrMesg("Out of memory reading DTM SDL");
		return(0);
		}
	memcpy(d->data,(char *)primbuff,num);
	d->rank = 1;
	d->dim[0] = num / sizeof(DTM_TRIPLET);
	d->entity = ENT_Network;

	return(d);
#endif	
}

static Com *NetReadCOM(n,header)
NetPort *n;
char *header;
{
Data *d;
static char	id[DTM_STRING_SIZE];
static char	domain[DTM_STRING_SIZE];
static char	mesg[DTM_STRING_SIZE];
static Com	c;


	DTMendRead(n->port);

	COMgetID(header,id,DTM_STRING_SIZE);
	COMgetDomain(header,domain,DTM_STRING_SIZE);
	COMgetMesg(header,mesg,DTM_STRING_SIZE);
	c.id = id;
	c.domain = domain;
	c.mesg = mesg;

	return(&c);
}


static Data *NetReadVDATA(n,header)
NetPort *n;
char *header;
{
#if 0
Data *d;
char	title[DTM_STRING_SIZE];
char	id[DTM_STRING_SIZE];
char	tmp[DTM_STRING_SIZE];
DTMTYPE type;
int	elementSize;
int	x;
int	size;

        VDATAgetTitle(header,title,DTM_STRING_SIZE);
	if ((!title) || (!strlen(title)))
		 strcpy(title,"Untitled");

	if (!(d = DataNew()))
		return(0);
	if (!(d->label = (char *) MALLOC(strlen(title)+1))) {
		ErrMesg("Out of Memory reading in vdata label \n");
        	DTMendRead(n->port);
		return(0);
		}
	strcpy(d->label,title);
	d->entity = ENT_Network;
	d->dot = DOT_VData;

	if (VDATAgetType(header,&type) == -1 ) {
		ErrMesg("Error getting VDATA type\n");
		NetReject(n->port,header);
		return(0);
		}
	switch(type) {
		case DTM_CHAR:
			d->dost = DOST_Char;
			elementSize = sizeof(char);
			break;
		case DTM_FLOAT:
			d->dost = DOST_Float;
			elementSize = sizeof(float);
			break;
		case DTM_INT:
			d->dost = DOST_Int32;
			elementSize = 4;
			break;
		case DTM_SHORT:
			d->dost = DOST_Int16;
			elementSize = 2;
			break;
		case DTM_DOUBLE:
			d->dost = DOST_Double;
			elementSize = sizeof(double);
			break;
		default: 
			d->dost = DOST_Char;
			elementSize = 1;
			printf(
			"VDATA of unknown type just received casting to char\n");
		};
	VDATAgetNumElements(header,&(d->dim[0]));
	VDATAgetNumRecords(header,&(d->dim[1]));
	d->rank = 2;
	VDATAgetPathLength(header,&(d->pathLength));
	if (!(d->magicPath = (VdataPathElement **) 
				MALLOC(sizeof(VdataPathElement *) 
					* d->pathLength))){
		ErrMesg("Out of Memory reading VDATA path\n");
        	DTMendRead(n->port);
		return(0);
		}
	for (x = 0; x < d->pathLength; x++) {
		if (!(d->magicPath[x] = (VdataPathElement *)
				MALLOC(sizeof(VdataPathElement)))) {
			ErrMesg("Out of Memory reading VDATA path 2\n");
        		DTMendRead(n->port);
			return(0);
			}
		}
	VDATAgetPath(header,d->magicPath,&(d->pathLength));
	VDATAgetNodeID(header,&(d->nodeID));
	VDATAgetNodeName(header,tmp,DTM_STRING_SIZE);
	if (!(d->nodeName = (char *)MALLOC(strlen(tmp)+1))) {
		ErrMesg("Out of Memory reading VDATA node Name\n");
       		DTMendRead(n->port);
		return(0);
		}
	strcpy(d->nodeName,tmp);
	VDATAgetField(header,tmp,DTM_STRING_SIZE);
	if (!(d->fields= (char *)MALLOC(strlen(tmp)+1))) {
		ErrMesg("Out of Memory reading VDATA field\n");
       		DTMendRead(n->port);
		return(0);
		}
	strcpy(d->fields,tmp);
	size = d->dim[0] * d->dim[1];
	if (d->data = (char *) MALLOC(size * elementSize)) {
		ErrMesg("Out of Memory making space for VDATA\n");
       		DTMendRead(n->port);
		return(0);
		}

	if (DTMreadDataset(n->port, d->data, size, type) == DTMERROR){
			ErrMesg("Error reading DTM VDATA");
			NetReject(n->port,header);
			return(0);
			}
	DTMendRead(n->port);
	return(d);
	
#endif
}

static Data *NetReadSDS(n,header)
NetPort *n;
/* get n dimensional, n type SDS.  Return 0 on failure */
char *header;
{
#if 0
Data *d;
DTMCMD  cmd;
DTMTYPE type;
char	title[DTM_STRING_SIZE];
char	id[DTM_STRING_SIZE];
char	itsNew;
int 	rank,dims[MAX_ARRAY_DIM];
int 	size;
int	x;
int 	dostType;
int	stat;


#ifdef DEBUG
	printf("NetReadSDS(): going to read data for %s\n",header);
#endif


	if (-1 == SDSgetDimensions(header,&rank,dims,
			sizeof(MAX_ARRAY_DIM))) {
		ErrMesg("Failed at getting dimensions of DTM\n");
		NetReject(n->port,header);
		return(0);
		}

	for(x = 0, size = 1; x < rank; x++) 
		size *= dims[x];

        ANIMgetID(header,id,DTM_STRING_SIZE);
	NetUserListAdd(id);

	SDSgetTitle(header,title,DTM_STRING_SIZE);
	if ((!title) || (!strlen(title)))
		 strcpy(title,"Untitled");

	if (SDSgetType(header,&type) == -1 ) {
		ErrMesg("Error getting SDS type\n");
		NetReject(n->port,header);
		return(0);
		}
	switch(type) {
		case DTM_CHAR:
			dostType = DOST_Char;
			break;
		case DTM_FLOAT:
			dostType = DOST_Float;
			break;
		case DTM_INT:
			dostType = DOST_Int32;
			break;
		case DTM_SHORT:
			dostType = DOST_Int16;
			break;
		case DTM_DOUBLE:
			dostType = DOST_Double;
			break;
		default: 
			dostType = DOST_Char;
			printf(
			"SDS of unknown type just received casting to char\n");
		};
	if (d = (Data *) DataSearchByLabelAndDOTAndDOST(title,DOT_Array,
			dostType)) {
		itsNew = FALSE;
		}
	else {
		itsNew = TRUE;
		if (!(d = DataNew())) {
			ErrMesg("Out of memory reading in DTM SDS\n");
			NetReject(n->port,header);
			return(0);
			}
		d->entity = ENT_Network;
		d->dot	= DOT_Array;
		d->dost = 0;
		CopyDimensions(d->dim,&(d->rank),dims,rank);
                if (!(d->label= (char *) MALLOC(strlen(title)+1))) {
                	ErrMesg("Can't allocate memory for DTM SDS name");
			NetReject(n->port,header);
                        return(0);
                        }
		strcpy(d->label,title);
		}

	ANIMgetExpansion(header,&(d->expandX),&(d->expandY));
#if 0
	COLgetView(header, &(d->view_type));
#endif

	if (type == DTM_FLOAT || type == DTM_DOUBLE)
	  {
	    if (SDSgetMinMax(header, &d->min.f, &d->max.f) == DTMERROR)
		d->min.f = d->max.f = 0.;
	  }
	else
	  {
	    if (SDSgetMinMax(header, &d->min.f, &d->max.f) == DTMERROR)
		d->min.i = d->max.i = 0;
	    else d->min.i = (int)d->min.f, d->max.i = (int)d->max.f;
	  }
	if (d->associated)
		FREE(d->associated);
	if (COLgetAssoc(header, title, DTM_STRING_SIZE) != DTMERROR) {
		if (!(d->associated = (char *) MALLOC(strlen(title)+1))) {
			ErrMesg("Out of Memory\n");
			return(0);
			}
		strcpy(d->associated,title);
		}
	else
		d->associated = (char *)NULL;

#define READSDS(EltSize)						\
 do {									\
	if (! DimensionsEqual(d->dim,d->rank,dims,rank)) {		\
		if (!itsNew)						\
			FREE(d->data);					\
		itsNew = TRUE;						\
		CopyDimensions(d->dim,&(d->rank),dims,rank);		\
		}							\
	if (d->dost != dostType) {					\
		if (!itsNew)						\
			FREE(d->data);					\
		itsNew = TRUE;						\
		CopyDimensions(d->dim,&(d->rank),dims,rank);		\
		}							\
	d->dost = dostType;						\
	if (itsNew) {							\
		if (!(d->data = (char *) MALLOC(size * EltSize))) {	\
			ErrMesg("Out of Memory");			\
			NetReject(n->port,header);			\
			return(0);					\
			}						\
		}							\
	if (DTMreadDataset(n->port,d->data,size,			\
			   type) == DTMERROR){				\
		ErrMesg("Error reading DTM dataset");			\
		NetReject(n->port,header);				\
		return(0);						\
		}							\
    } while (0)

	if (type == DTM_CHAR) {
		READSDS(1);
		}
	else if (type == DTM_FLOAT) {
		READSDS(sizeof(float));
#ifdef DEBUG
			printf("\n\n\n\n#######\nThe first number is %f\n\n\n",
					*((float*) d->data));
#endif
		}
	else if (type == DTM_INT) {
		READSDS(4);
		}
	else if (type == DTM_SHORT) {
		READSDS(2);
		}
	else if (type == DTM_DOUBLE) {
		READSDS(sizeof(double));
		}
	else {
#ifdef DEBUG
		ErrMesg("ReadDTM(): Unknown type %d\n",type);
#endif
		NetReject(n->port,header);
		return(0);
		}

	DTMendRead(n->port);
	return(d);

#endif /* 0 */
} /* NetReadSDS() */




static Data *NetReadPal(n,header)
NetPort *n;
char *header;
{
#if 0
Data *d;
char title[DTM_STRING_SIZE];
char	id[DTM_STRING_SIZE];

#ifdef DEBUG
	printf("NetReadPal(): going to read data for %s\n",header);
#endif

        COLgetID(header,id,DTM_STRING_SIZE);
	NetUserListAdd(id);
	PALgetTitle(header,title,DTM_STRING_SIZE);
	if ((!title) || (!strlen(title)))
		strcpy(title,"Untitled");

	if (!(d = (Data *)DataSearchByLabelAndDOT(title,DOT_Palette8))) {
		if (!( d = DataNew())) {
			ErrMesg("Out of memory reading palette\n");
			return(0);
			}
		d->dot = DOT_Palette8;
		d->dost = DOST_Char;
		d->entity = ENT_Network;
		if (!( d->label = (char *) MALLOC(strlen(title) +1))) {
			ErrMesg("Out of memory reading palette\n");
			return(0);
			}
		strcpy(d->label,title);
		if (!( d->data= (char *) MALLOC(768))) {
			ErrMesg("Out of memory reading palette\n");
			return(0);
			}
		}
	if (d->associated)
		FREE(d->associated);
	if (COLgetAssoc(header, title, DTM_STRING_SIZE) != DTMERROR) {
		if (!(d->associated = (char *) MALLOC(strlen(title)+1))) {
			ErrMesg("Out of Memory\n");
			return(0);
			}
		strcpy(d->associated,title);
		}
	else
		d->associated = (char *)NULL;
	DTMreadDataset(n->port,d->data,768,DTM_CHAR);
	DTMendRead(n->port);
	return(d);

#endif /* 0 */
} /* NetReadPal() */


static Data *NetReadRIS8(n,header)
NetPort *n;
char *header;
{
#if 0
Data 	*d;
char	title[DTM_STRING_SIZE];
int	xdim,ydim;
char	itsNew;
char	id[DTM_STRING_SIZE];

#ifdef DEBUG
	printf("NetReadRIS8(): going to read data for %s\n",header);
#endif

	RISgetTitle(header,title,DTM_STRING_SIZE);
	RISgetDimensions(header,&xdim,&ydim);
        COLgetID(header,id,DTM_STRING_SIZE);
	NetUserListAdd(id);
	if ((!title) || (!strlen(title)))
		strcpy(title,"Untitled");
	if (d = (Data *) DataSearchByLabelAndDOTAndDOST(title,DOT_Array,
							DOST_Char)) {
		itsNew = FALSE;
		}
	else {
		itsNew = TRUE;
		if (!( d = DataNew())) {
			ErrMesg("Out of memory reading raster\n");
			NetReject(n->port,header);
			return(0);
			}
		d->dot = DOT_Array;
		d->dost = DOST_Char;
		d->entity = ENT_Network;
		d->rank = 2;
		d->dim[0] = xdim;
		d->dim[1] = ydim;
		if (!( d->label = (char *) MALLOC(strlen(title) +1))) {
			ErrMesg("Out of memory reading raster\n");
			NetReject(n->port,header);
			return(0);
			}
		strcpy(d->label,title);
		}

	if (SDSgetMinMax(header, &d->min.f, &d->max.f) == DTMERROR)
		d->min.f = d->max.f = 0.;
#if 0
	COLgetView(header, &(d->view_type));
#endif
	ANIMgetExpansion(header,&(d->expandX),&(d->expandY));
	if (d->associated)
		FREE(d->associated);
	if (COLgetAssoc(header, title, DTM_STRING_SIZE) != DTMERROR) {
		if (!(d->associated = (char *) MALLOC(strlen(title)+1))) {
			ErrMesg("Out of Memory\n");
			return(0);
			}
		strcpy(d->associated,title);
		}
	else
		d->associated = (char *)NULL;

	if (!(d->rank == 2 && d->dim[0] == xdim && (d->dim[1] == ydim) )) {
		if (!itsNew) FREE(d->data);
		itsNew = TRUE;
		d->rank = 2;
		d->dim[0] = xdim;
		d->dim[1] = ydim;
		}

	if (itsNew) {
		if (!( d->data = (char *) MALLOC(xdim*ydim))) {
			ErrMesg("Out of memory reading image\n");
			NetReject(n->port,header);
			return(0);
			}
		}

	if (DTMreadDataset(n->port,d->data,xdim*ydim,DTM_CHAR) == 
	    DTMERROR) {
		ErrMesg("Error reading RIS dataset");
		NetReject(n->port,header);
		return(0);
		}

	DTMendRead(n->port);

	return(d);

#endif
} /* NetReadRIS8() */




static Text *NetReadText(n,header)
/* Returns a static allocated text message.  However, t->textString is
   not static */
NetPort *n;
char *header;
{
#if 0
Data *d;
static	Text t;
static	char id[DTM_STRING_SIZE];
static	char title[DTM_STRING_SIZE];

#ifdef DEBUG
	printf("NetReadText(): I've been called.  header=%s\n",header);
#endif
	t.id = id;
	t.title = title;
	if (DTMERROR == TXTgetTitle(header,title,DTM_STRING_SIZE)) {
		strcpy(title,"Untitled");
		}
        if (DTMERROR == TXTgetID(header,id,DTM_STRING_SIZE)) {
		strcpy(id,"Unknown");
		}
	NetUserListAdd(id);

	t.selLeft = 0;
	t.selRight = 0;

	if (DTMERROR != TXTgetSelectionLeft(header,&(t.selLeft))) {
                if (DTMERROR == TXTgetSelectionRight(header,&(t.selRight))) {
                  ErrMesg("This DTM TXT select message is hosed. discarding\n");
                  }
		/*return(0);*/
                }

        if (DTMERROR == TXTgetDimension(header,&(t.dim))) {
                t.dim = 0;
                }
	if (!(t.textString = (char *) MALLOC(t.dim+1))) {
		ErrMesg("Out of Memory reading text\n");
		return(0);
		}
        if (DTMERROR == TXTgetInsertionPt(header,&(t.insertPt))) {
		t.insertPt = 0;
		}
	if (DTMERROR == TXTgetNumReplace(header,&(t.numReplace))) {
		t.numReplace = 0;
		}

        {int garbage;
        t.replaceAll = TXTshouldReplaceAll(header,garbage);
        }
#ifdef DEBUG
	printf("t.dim = %d\n",t.dim);
#endif
        if ((t.dim = DTMreadDataset(n->port,t.textString,t.dim,DTM_CHAR))
			== DTMERROR){
		ErrMesg("Error reading DTM dataset\n");
		DTMendRead(n->port);
		return(0);
		}
		
	t.textString[t.dim] = '\0';
#ifdef DEBUG
	printf("NetReadText(): *t.textString = %c dim = %d\n",
				*t.textString,t.dim);
	printf("NetReadText(): t.textString = \"%s\"\n",t.textString);
#endif
        DTMendRead(n->port);

	return(&t);
#endif
} /* NetReadText() */


static Col *NetReadCOL(n,header)
NetPort *n;
char *header;
{
#if 0
static	char title[DTM_STRING_SIZE];
static	char id[DTM_STRING_SIZE];
static	char func[DTM_STRING_SIZE];
static	Col col;
static	struct COL_TRIPLET	triplet[MAXDRAWDOODLE];
int	selType;
int	num, width;
char	buff[1024];

#ifdef DEBUG
	printf("NetReadCOL(): I've been called.  header=%s\n",header);
#endif
	col.title = title;
	col.id = id;
	col.func = func;

        COLgetTitle(header,col.title,DTM_STRING_SIZE);
        COLgetID(header,col.id,DTM_STRING_SIZE);
        COLgetFunc(header,col.func,DTM_STRING_SIZE,&(col.selType));

	if (((col.selType == COL_DOODLE_DISC)||(col.selType == COL_DOODLE_CONT))
		&&(strcmp(col.func, "DOODLE") == 0))
	{
		if (COLgetWidth(header,&width) == -1)
		{
#ifdef DEBUG
			fprintf(stderr, "NetReadCOL(): SENDER DIDN'T SET LINE WIDTH IN HEADER\n");
#endif
			width = 1;
		}
		col.width = width;
	}
	else
	{
		col.width = 1;
	}

        if (-1 == COLgetDimension(header,&(col.dim))) {
#ifdef DEBUG
		printf("NetReadCOL(): SENDER DIDN'T SET DIMENSIONS IN HEADER\n");
#endif
		col.dim = 1;
		}
	NetUserListAdd(col.id);
	selType = col.selType;
	if (selType == COL_DOODLE_DISC) {
		if ((num = DTMreadDataset(n->port,triplet,MAXDRAWDOODLE,
				COL_TRIPLET)) ==DTMERROR){
			sprintf(buff,"Error reading DTM dataset\n%s\n",
				NetDTMErrorString(DTMerrno));
			ErrMesg(buff);
			DTMendRead(n->port);
			return(0);
			}
		}
	else if (selType == COL_DOODLE_CONT) {
		if ((num = DTMreadDataset(n->port,triplet,MAXDRAWDOODLE,
				COL_TRIPLET)) ==DTMERROR){
			sprintf(buff,"Error reading DTM dataset\n%s\n",
				NetDTMErrorString(DTMerrno));
			ErrMesg(buff);
			DTMendRead(n->port);
			return(0);
			}
		}
        else if (selType == COL_AREA) {
                if ((num = DTMreadDataset(n->port,triplet,3,COL_TRIPLET))
                        ==DTMERROR){
			sprintf(buff,"Error reading DTM dataset\n%s\n",
				NetDTMErrorString(DTMerrno));
			ErrMesg(buff);
                        DTMendRead(n->port);
                        return(0);
                        }
		}
	else if (selType == COL_LINE) {
                if ((num = DTMreadDataset(n->port,triplet,2,COL_TRIPLET))
                        ==DTMERROR){
			sprintf(buff,"Error reading DTM dataset\n%s\n",
				NetDTMErrorString(DTMerrno));
			ErrMesg(buff);
                        DTMendRead(n->port);
                        return;
                        }
		}
	else if (selType == COL_POINT) {
                if ((num = DTMreadDataset(n->port,triplet,1,COL_TRIPLET))
                        ==DTMERROR){
			sprintf(buff,"Error reading DTM dataset\n%s\n",
				NetDTMErrorString(DTMerrno));
			ErrMesg(buff);
                        DTMendRead(n->port);
                        return;
                        }
		}
	else {
#ifdef DEBUG
		printf("Got a DTM COL class selType that I don't know\n");
#endif
		num = 0;
		}
	col.dim = num;
	col.data = triplet;
	DTMendRead(n->port);

#ifdef DEBUG
printf("col.dim is %d\n",col.dim);
#endif
	return(&col);
#endif
} /* NetReadCOL() */

char *NetReadMSG(n,header)
NetPort *n;
char *header;
{
static  char s[2*DTM_STRING_SIZE];

	DTMendRead(n->port);
	if (COLgetID(header, s, DTM_STRING_SIZE) != DTMERROR)
		strcat(s, ":");
	else
		s[0] = 0;
	MSGgetString(header,s+strlen(s),DTM_STRING_SIZE);
	return(s);
}

static NetPort *NetSearchListForPortName(list,portName)
List *list;
char *portName;
{
NetPort *netPort;
	
	netPort = (NetPort *) ListHead(list);
	while (netPort) {
		if (!strcmp(portName,netPort->portName)) {
#ifdef DEBUG
printf("NetSearchListForPortName:\"%s\" == \"%s\" RETURNING a netPort\n",
			portName,netPort->portName);
#endif
			return(netPort);
			}
#ifdef DEBUG
		printf("NetSearchListForPortName:\"%s\" != \"%s\"\n",
			portName,netPort->portName);
#endif
		netPort = (NetPort *) ListNext(list);
		}
	return(0);
}

static Server *NetReadSRV(n,header)
NetPort *n;
char *header;
{
static Server s;
char inPort[80];
NetPort *out;
char buff[256];

#ifdef DEBUG
	printf("NetReadSRV(): I've been called.  header=%s\n",header);
#endif

	DTMendRead(n->port);
	SRVgetID(header,s.id,80);
        SRVgetFunction(header,&(s.func));
	NetUserListAdd(s.id);

	switch (s.func) {
	    case SRV_FUNC_CONNECT:
                SRVgetInPort(header,s.inPort,80);
#ifdef DEBUG
		printf("Just got a SRV connect from id=%s, inPort=%s\n",
			s.id,s.inPort);
		printf("This was read from port=%s\n", n->portName);
#endif
		if (s.netPort = NetSearchListForPortName(netOutList,s.inPort)){
#ifdef DEBUG
			printf("Already connected to this address\n");
#endif
			return(0);
			}
		
		sprintf(buff,
			"Just established a connection with\n%s (%s)\n",
			s.id,s.inPort);
		WriteMesg(buff);
		if (s.netPort = (NetPort *) ListHead(netOutList))
			NetChangeOutPort(s.inPort,s.netPort);
		else {
			/* don't have an out port... make one */
			s.netPort = NetCreateOutPort(s.inPort);
			}
		strcpy(s.inPort,s.netPort->portName);
                break;

	    case SRV_FUNC_DISCONNECT:
#ifdef DEBUG
		printf("Just got a SRV disconnect from portName=%s\n",
			n->portName);
#endif
		s.inPort[0]='\0';
		s.netPort = n;
		out = (NetPort *) ListHead(netOutList);
                SRVgetInPort(header,s.inPort,80);
		sprintf(buff,
			"Just received a disconnect from\n(%s)\n",
			 s.inPort);
		WriteMesg(buff);
		if (strlen(s.inPort)) {
			while(out) {
				if (!strcmp(out->portName,s.inPort)) {
					NetRemovePortFromSendQueue(out);
					ListDeleteEntry(netOutList,out);
					DTMdestroyPort(out->port);
					out->open = FALSE;
					out->type = NET_UNDEF;
					break;
					}
				out = (NetPort *) ListNext(netOutList);
				}
			if (!out)
			    ErrMesg("Got a disconnect with a bogus port\n");
			}
		else {
			/***** this assumes only one out port **********/
			ListDeleteEntry(netOutList,ListHead(netOutList)); 
			}
                break;

	    case SRV_FUNC_LOCK:
	    case SRV_FUNC_UNLOCK:
	    case SRV_FUNC_ADD_USER:
	    case SRV_FUNC_REMOVE_USER:
		break;

	    default:
		printf("Just received an unknown SRV function\n");
		return(0);
	}


	return(&s);
} /* NetReadSRV() */


static Server *NetReadDTM(n,header)
NetPort *n;
char *header;
{
static Server s;
char buff[256];

#ifdef DEBUG
	printf("NetReadDTM(): I've been called.  header=%s\n",header);
#endif
	DTMendRead(n->port);
	header +=4;
	strncpy(s.inPort,header,79);
	s.inPort[79]='\0';
        s.func = SRV_FUNC_CONNECT;
#ifdef DEBUG
	printf("NetReadDTM(): s.inPort = \"%s\"\n",s.inPort);
#endif
	if (s.netPort = NetSearchListForPortName(netOutList,s.inPort)){
#ifdef DEBUG
		printf("Already connected to this address\n");
#endif
		return(0);
		}
	sprintf(buff, "Just established a connection with\nport=%s\n",
			s.inPort);
	WriteMesg(buff);
	if (s.netPort = (NetPort *) ListHead(netOutList))
		NetChangeOutPort(s.inPort,s.netPort);
	else {
		/* don't have an out port... make one */
		s.netPort = NetCreateOutPort(s.inPort);
		}
	strcpy(s.inPort,s.netPort->portName);
	return(&s);
	
} /* NetReadDTM() */


static Exec *NetReadEXEC(n,header)
NetPort *n;
char *header;
{
static	char 	id[80];
static	char	retAddress[80];
static	char	authentication[256];
static	char	timeStamp[80];
static	Exec	exec;


	EXECgetID(header,id,80);
	EXECgetAddress(header,retAddress,80);
	EXECgetAuthentication(header,authentication,256);
	EXECgetTimeStamp(header,timeStamp,80);
	exec.id = id;
	exec.retAddress = retAddress;
	exec.authentication = authentication;
	exec.timeStamp = timeStamp;
	EXECgetType(header,&(exec.type));
	switch(exec.type) {
		case EXEC_HOST_STATUS_QUERY:
			break;
		case EXEC_HOST_STATUS_RETURN:
			EXECgetLoad1(header, &(exec.info.hsReturn.load1));
			EXECgetLoad5(header, &(exec.info.hsReturn.load5));
			EXECgetLoad15(header, &(exec.info.hsReturn.load15));
			EXECgetNumUsers(header,&(exec.info.hsReturn.numUsers));
			break;
		case EXEC_EXECUTE:
			break;
		case EXEC_EXECUTE_RETURN:
			break;
		case EXEC_PROC_STATUS_QUERY:
			break;
		case EXEC_PROC_STATUS_RETURN:
			break;
		case EXEC_FILE_PUT:
			break;
		case EXEC_FILE_GET:
			break;
		};

	DTMendRead(n->port);

	return(&exec);
}




static AnimMesg *NetReadANIM(n,header)
NetPort *n;
char *header;
{
#if 0
static AnimMesg a;
static char id[80];
static char title[1024];
int func;
int runType;

	DTMendRead(n->port);
	ANIMgetTitle(header,title,1024);
	a.title = title;
	ANIMgetID(header,id,80);
	a.id = id;
	ANIMgetFrame(header,&(a.frameNumber));
	if (-1 == ANIMgetFunc(header,(&func))) 
		a.func = AF_NO_FUNC;
	else {
		switch(func) {
		    case  ANIM_FUNC_STOP:
				a.func = AF_STOP;
				break;
		    case ANIM_FUNC_FPLAY:
				a.func = AF_FPLAY;
				break;
		    case ANIM_FUNC_RPLAY:
				a.func = AF_RPLAY;
				break;
		    };
		}
	if (-1 == ANIMgetRunType(header,(&runType))) 
		a.runType= ART_NONE;
	else {
		switch (runType) {
		    case ANIM_RUN_TYPE_SINGLE:
				a.runType = ART_SINGLE;
				break;
		    case ANIM_RUN_TYPE_CONT:
				a.runType = ART_CONT;
				break;
		    case ANIM_RUN_TYPE_BOUNCE:
				a.runType = ART_BOUNCE;
				break;
		    };
		}
		
	a.data = 0;
	return(&a);
#endif
}

#define CALLCB(List, CallB, CallData, ClientData)			\
 do {									\
	while (docb) {							\
		if (ExceptModuleName)  {				\
			if (docb->CallB &&				\
			    strcmp(ExceptModuleName,docb->moduleName)) {\
				(docb->CallB)(CallData,docb->ClientData);\
			    }						\
			}						\
                else {							\
			if (docb->CallB) {				\
				(docb->CallB)(CallData,docb->ClientData);\
                        	}					\
			}						\
                docb = (DOCB *) ListNext(List);				\
		}							\
    } while (0)

static int NetTextDistribute(text,ExceptModuleName)
Text *text;
char *ExceptModuleName;
{
DOCB *docb;

	if (!(docb = (DOCB *) ListHead(TXTList))) {
		return(0);
		}
        if (ExceptModuleName &&(!strcmp(docb->moduleName,ExceptModuleName))) {
                if (!(docb = (DOCB *) ListNext(TXTList)))
                        return(0); /* none to distribute to */
                }

	CALLCB(TXTList, newCB, text, newData);

        return(1);
}



static int NetCOLDistribute(col,ExceptModuleName)
/* calls all the data object callbacks (DOCB) for COL*/
Col *col;
char *ExceptModuleName;	/* don't distribute to this moduleName */

{
DOCB *docb;

	if (!(docb = (DOCB *) ListHead(COLList))) {
		return(0); /* none to distribute to */
		}
	if (ExceptModuleName &&(!strcmp(docb->moduleName,ExceptModuleName))) {
		if (!(docb = (DOCB *) ListNext(COLList)))
			return(0); /* none to distribute to */
		}

	/* distribute */
	CALLCB(COLList, newCB, col, newData);

	return(1);

} /* NetCOLDistribute() */

static Col *NetMakeCOLFromDoodle(title,doodle,length,sendDiscrete)
char *title;
struct COL_TRIPLET *doodle;
int length;
int sendDiscrete;
{
#if 0
static Col col;

	col.title = title;
	col.id = UserID;
	col.func = "DOODLE";
	if (sendDiscrete) 
		col.selType = COL_DOODLE_DISC;
	else
		col.selType = COL_DOODLE_CONT;
	col.dim = length;
	col.data = doodle;
	return(&col);
#endif
}
	
static void NetCallDestroyCallback(list,d)
List list;
Data *d;
{
DOCB *docb;

	docb = (DOCB *) ListHead(list);
	while (docb) {
		if (docb->destroyCB)
			(docb->destroyCB)(d,docb->destroyData);
		docb = (DOCB *) ListNext(list);
		}
}
static int NetAnimationDistribute(dSend,ExceptModuleName)
Data **dSend;
char *ExceptModuleName;
{
#if 0
Data *d;
DOCB *docb;
static AnimMesg a;
        if (!(docb = (DOCB *) ListHead(ANIMList))) {
                return(0); /* none to distribute to */
                }
#ifdef DEBUG
	printf("bink\n");
#endif
        if (ExceptModuleName &&(!strcmp(docb->moduleName,ExceptModuleName))) {
                if (!(docb = (DOCB *) ListNext(ANIMList)))
                        return(0); /* none to distribute to */
                }
#ifdef DEBUG
	printf("boink\n");
#endif
	if (d = DataSearchByLabelAndDOT((*dSend)->label,DOT_Array)) {
		/* if this isn't the same dim or dost, could be trouble */
		if (d->dost == (*dSend)->dost)
		  if (DimensionsEqual(d->dim,d->rank,(*dSend)->dim,
				      (*dSend)->rank)){
			NetCallDestroyCallback(ANIMList,d);
			FREE(d->data);
			d->data = (*dSend)->data;
			}
		  else {
			NetCallDestroyCallback(ANIMList,d);
			FREE(d->data);
			d->data = (*dSend)->data;
			CopyDimensions(d->dim,&(d->rank),
					(*dSend)->dim,(*dSend)->rank);
		      }
		memcpy(d, (*dSend), sizeof(Data));
		FREE(*dSend);
		*dSend = d;
		}
	else {
		d = (*dSend);
		}
	a.title = d->label;
	a.id = userID;
	a.func = AF_NO_FUNC;
	a.runType = ART_NONE;
	a.data = d;
	
	/* distribute data */
	if (!DataInList(d)) {
		DataAddEntry(d);
		CALLCB(ANIMList, newCB, &a, newData);
		}
	else {
		CALLCB(ANIMList, changeCB, &a, changeData);
		}
#ifdef DEBUG
	printf("bonk\n");
#endif
	return(1);
#endif
}


static int NetArrayDistribute(dSend,ExceptModuleName)
Data **dSend;
char *ExceptModuleName;
{
#if 0
Data *d;
DOCB *docb;
        if (!(docb = (DOCB *) ListHead(SDSList))) {
                return(0); /* none to distribute to */
                }
#ifdef DEBUG
	printf("bink\n");
#endif
        if (ExceptModuleName &&(!strcmp(docb->moduleName,ExceptModuleName))) {
                if (!(docb = (DOCB *) ListNext(SDSList)))
                        return(0); /* none to distribute to */
                }
#ifdef DEBUG
	printf("boink\n");
#endif

	if (d = DataSearchByLabelAndDOTAndDOST((*dSend)->label,DOT_Array,
			(*dSend)->dost)) {
		/* if this isn't the same dost, could be trouble */
		if (d->dost == (*dSend)->dost) {
			if (! DimensionsEqual(d->dim,d->rank,
					(*dSend)->dim,(*dSend)->rank)) {
				NetCallDestroyCallback(SDSList,d);
				FREE(d->data);
				d->data = (*dSend)->data;
				CopyDimensions(d->dim,&(d->rank),
					(*dSend)->dim,(*dSend)->rank);
				}
			else {
				NetCallDestroyCallback(SDSList,d);
				FREE(d->data);
				d->data = (*dSend)->data;
				}
			}
		if (d->associated && d->associated != (*dSend)->associated)
			FREE(d->associated);
		memcpy(d, (*dSend), sizeof(Data));
		FREE(*dSend);
		*dSend = d;
		}
	else {
		d = *dSend;
		}
	/* distribute data */
	if (!DataInList(d)) {
		DataAddEntry(d);
		CALLCB(SDSList, newCB, d, newData);
		}
	else {
		CALLCB(SDSList, changeCB, d, changeData);
		}
#ifdef DEBUG
	printf("bonk\n");
#endif
	return(1);
#endif
} /* NetArrayDistribute() */


int NetRISDistribute(dSend,ExceptModuleName)
Data **dSend;
char *ExceptModuleName;
{
#if 0
	Data *d;
	DOCB *docb;

	if (!(docb = (DOCB *) ListHead(RIS8List)))
		return(0); /* none to distribute to */
	if (ExceptModuleName &&(!strcmp(docb->moduleName,ExceptModuleName))) {
		if (!(docb = (DOCB *) ListNext(RIS8List)))
			return(0); /* none to distribute to */
		}
	if (d = DataSearchByLabelAndDOTAndDOST((*dSend)->label,DOT_Array,
			DOST_Char)) {
		/* if this isn't the same dost, could be trouble */
		if (d->dost == (*dSend)->dost) {
			if (! DimensionsEqual(d->dim,d->rank,
					(*dSend)->dim,(*dSend)->rank)) {
				NetCallDestroyCallback(RIS8List,d);
				FREE(d->data);
				d->data = (*dSend)->data;
				CopyDimensions(d->dim,&(d->rank),
					(*dSend)->dim,(*dSend)->rank);
				}
			else {
				NetCallDestroyCallback(RIS8List,d);
				FREE(d->data);
				d->data = (*dSend)->data;
				}
			}
		if (d->associated && d->associated != (*dSend)->associated)
			FREE(d->associated);
		memcpy(d, (*dSend), sizeof(Data));
		FREE(*dSend);
		*dSend = d;
		}
	else {
		d = *dSend;
	      }
	
	/* distribute data */
	if (!DataInList(d)) {
		DataAddEntry(d);
		CALLCB(RIS8List, newCB, d, newData);
		}
	else {
		CALLCB(RIS8List, changeCB, d, changeData);
		}

	return(1);
#endif
} /* NetRISDistribute() */


int NetPALDistribute(title,rgb,associated,ExceptModuleName)
/* calls all the data object callbacks (DOCB) for palettes */
char *title;
unsigned char *rgb;
char *associated;
char *ExceptModuleName;	/* don't distribute to this moduleName */

{
#if 0
DOCB *docb;
register int x;
Data *d;
register char *p;

	if (!(docb = (DOCB *) ListHead(PALList))) {
		return(0); /* none to distribute to */
		}
	if (ExceptModuleName &&(!strcmp(docb->moduleName,ExceptModuleName))) {
		if (!(docb = (DOCB *) ListNext(PALList)))
			return(0); /* none to distribute to */
		}
	
	/* get data field make a new one if doesn't exist */
	if (!(d = DataSearchByLabelAndDOT(title,DOT_Palette8))) {
		if (!(d = DataNew())) {
			return(0); /* out of memory */
			}
		if (!(d->label = (char *) MALLOC(strlen(title)+1))) {
			ErrMesg("Out of Memory\n");
			return(0);
			}
		strcpy(d->label,title);
		d->entity = ENT_Internal;
		d->dot = DOT_Palette8;
		d->dost = DOST_Char;
		if (!(d->data = (char *) MALLOC(768))) {
			ErrMesg("Out of Memory\n");
			return(0);
			}
		}

	p = d->data;
	for (x=0; x < 768; x++)
		*p++ = *rgb++;

	if (d->associated)
		FREE(d->associated);
	if (associated) {
		if (!(d->associated = (char *) MALLOC(strlen(associated)+1))) {
			ErrMesg("Out of Memory\n");
			return(0);
			}
		strcpy(d->associated,associated);
		}
	else
		d->associated = associated;

	/* distribute data */
	if (!DataInList(d)) {
		DataAddEntry(d);
		CALLCB(PALList, newCB, d, newData);
		}
	else {
		CALLCB(PALList, changeCB, d, changeData);
		}
	return(1);
#endif
} /* NetPALDistribute() */
	
#undef CALLCB
#define CALLCB(List, CallB, CallData, ClientData)			\
 do {									\
	while (docb) {							\
		if (docb->CallB) {					\
			(docb->CallB)(CallData,docb->ClientData);	\
                       	}						\
                docb = (DOCB *) ListNext(List);				\
		}							\
    } while (0)




static void *
NetReadMessage(n)
NetPort *n;
{
char    header[DTM_MAX_HEADER];
int	length;
DOCB *docb;
Data 	*d = NULL;
Text	*t;
Col	*c;
Com	*com;
Server	*s;
Exec	*e;
char	*mesg;
static AnimMesg a;
AnimMesg *ap;
static char    id[DTM_STRING_SIZE];

char	buff[256];
int 	i;

#ifdef DEBUG
	printf("NetReadMessage(): I've been called.\n");
#endif
        if ((length =DTMbeginRead(n->port,header,DTM_MAX_HEADER)) == DTMERROR){
		sprintf(buff,"Error reading DTM header from port %s (%d) ret %d\nDTM error= %s\n",
				n->portName,n->port,length,
				NetDTMErrorString(DTMerrno));
		ErrMesg(buff);
#ifdef DEBUG
		printf(buff,"header= %s\n",header);
#endif
		DTMendRead(n->port);
                return(0);
                }

	if (SDScompareClass(header) && ANIMisAnimation(header,i)) {
#ifdef DEBUG
		printf("Treating SDS as an Animation\n");
#endif
		if (!(docb = (DOCB *) ListHead(ANIMList))) {
			NetReject(n->port,header);
			return(0);
			}
		if (d = NetReadSDS(n,header)) {
			a.data = d;
			a.title = d->label;
			a.func = AF_NO_FUNC;
			a.runType = ART_NONE;
			a.id = id;
			ANIMgetID(header,a.id,DTM_STRING_SIZE);
			if (!DataInList(d)) {
				DataAddEntry(d);
				CALLCB(ANIMList, newCB, &a, newData);
				}
			else {
				CALLCB(ANIMList, changeCB, &a, changeData);
				}
			}
		} /* SDS Animation*/
	else if (ANIMcompareClass(header)) {
		if (!(docb = (DOCB *) ListHead(ANIMList))) {
			NetReject(n->port,header);
			return(0);
			}
		if (ap = NetReadANIM(n,header)) {
			CALLCB(ANIMList, newCB, ap, newData);
			}
		} /* ANIM */
	else if (SDScompareClass(header)) {
#ifdef DEBUG
		printf("SDS is not an Animation\n");
#endif
		if (!(docb = (DOCB *) ListHead(SDSList))) {
			NetReject(n->port,header);
			return(0);
			}
		if (d = NetReadSDS(n,header)) {
			if (!DataInList(d)) {
				DataAddEntry(d);
				CALLCB(SDSList, newCB, d, newData);
				}
			else {
				CALLCB(SDSList, changeCB, d, changeData);
				}
			}
		} /* SDS */

	else if (PALcompareClass(header)) {
		if (!(docb = (DOCB *) ListHead(PALList))) {
			NetReject(n->port,header); 
			return(0);
			}
		if (d = NetReadPal(n,header)) {
			if (!DataInList(d)) {
				DataAddEntry(d);
				CALLCB(PALList, newCB, d, newData);
				}
			else {
				CALLCB(PALList, changeCB, d, changeData);
				}
			}
		
		}/* PAL */

	else if (RIScompareClass(header)) {
		if (!(docb = (DOCB *) ListHead(RIS8List))) {
			NetReject(n->port,header); 
			return(0);
			}
		if (d = NetReadRIS8(n,header)) {
			if (!DataInList(d)) {
				DataAddEntry(d);
				CALLCB(RIS8List, newCB, d, newData);
				}
			else {
				CALLCB(RIS8List, changeCB, d, changeData);
				}
			}
		
		}/* RIS */
	else if (TXTcompareClass(header)) {
		if (!(docb = (DOCB *) ListHead(TXTList))) {
			NetReject(n->port,header); 
			return(0);
			}
		if (t = NetReadText(n,header)) {
			CALLCB(TXTList, newCB, t, newData);
			if (t->textString)
				FREE(t->textString);
			}
		} /* TXT */
	else if (SRVcompareClass(header)) {
		docb = (DOCB *) ListHead(SRVList);
		if (s = NetReadSRV(n,header)) {
			CALLCB(SRVList, newCB, s, newData);
			}
		} /*SRV*/
	else if (COLcompareClass(header)) {
		if (!(docb = (DOCB *) ListHead(COLList))) {
			NetReject(n->port,header); 
			return(0);
			}
		if (c = NetReadCOL(n,header)) {
			CALLCB(COLList, newCB, c, newData);
			}
		} /*COL */
	else if (DTMcompareClass(header)) {
		docb = (DOCB *) ListHead(DTMList);
		if (s = NetReadDTM(n,header)) {
			CALLCB(COLList, newCB, s, newData);
			}
		} /*DTM*/
	else if (SDLcompareClass(header)) {
		docb = (DOCB *) ListHead(SDLList);
		if (d = NetReadSDL(n,header)) {
			CALLCB(SDLList, newCB, d, newData);
			}
		} /* SDL */
	else if (COMcompareClass(header)) {
		docb = (DOCB *) ListHead(COMList);
		if (com = NetReadCOM(n,header)) {
			CALLCB(COMList, newCB, com, newData);
			}
		} /* COM */
	else if (VDATAcompareClass(header)) {
		docb = (DOCB *) ListHead(VDATAList);
		if (d = NetReadVDATA(n,header)) {
			CALLCB(VDATAList, newCB, d, newData);
			}
		}
	else if (EXECcompareClass(header)) {
		docb = (DOCB *) ListHead(EXECList);
		if (e = NetReadEXEC(n,header)) {
			CALLCB(EXECList, newCB, e, newData);
			}
		} /* EXEC */
	else if (MSGcompareClass(header)) {
		docb = (DOCB *) ListHead(MSGList);
		if (mesg = NetReadMSG(n,header)) {
			CALLCB(MSGList, newCB, mesg, newData);
			}
		}
	else {
		NetReject(n->port,header); 
		}

	return(d);


} /* NetReadMessage() */

static NetPort *NetSearchListForDTMPort(netPortList,port)
List netPortList;
int port;
{
NetPort *netPort;
	
	netPort = (NetPort *) ListHead(netPortList);
	while (netPort) {
		if (netPort->port == port) {
			return(netPort);
			}
		netPort = (NetPort *) ListHead(netPortList);
		}
	return(0);
}

#if 0			/* not called in Collage */
int NetServPollAndRead()
/* this blocks until something is ready to read*/
/* return -1 on Error, 0 on nothing read, 1 on read */
{
NetPort *n;
int length;
Dtm_set s[64];
int num;
int x;
int retStatus;

	n = (NetPort *) ListHead(netInList);
	num = 0;
	while (n) {
		s[num++].port = n->port;
		n = (NetPort *) ListNext(netInList);
		}
	if (DTMERROR == DTMselectRead(s,num,0,0,1000)) {
		WriteMesg("Error checking for DTM input\n");
		return(-1);
		}
	retStatus = 0;
	for (x = 0; x < num; x++) {
		if (s[x].status) {
			n = NetSearchListForDTMPort(netInList,s[x].port);
			NetReadMessage(n);
			retStatus = 1;
			}
		}

	return(retStatus);
}
#endif				/* not used */

void NetClientPollAndRead()
/* Check all in ports, read data and make data callbacks */
/* *should* not block */
{
Data *d;
NetPort *n;

#ifdef DEBUG

/*	printf("NetClientPollAndRead(): I've been called\n");*/
#endif
	n = (NetPort *) ListHead(netInList); 
	while (n) {
		while (DTMavailRead(n->port)) {
#ifdef DEBUG
			fprintf( stderr, "Reading message from port %s\n",n->portName);
#endif
#ifdef USE_FEEDBACK_CALLS
			SetReadFeedback();
#endif
			(void) NetReadMessage(n);
#ifdef USE_FEEDBACK_CALLS
			UnsetFeedback();
#endif
			}
		n = (NetPort *) ListNext(netInList);
		}
	NetTryResend();
}


static int
NetOpenPort(netPort)
NetPort *netPort;
{
	if (!netPort->open) {
		int out;

		out = DTMmakeOutPort(netPort->portName, dtmFlowControl);
		if (out == DTMERROR)
			return DTMERROR;
		netPort->port = out;
		netPort->open = TRUE;
		netPort->queueTime = 0;
	}
	return 0;
}

static int
NetSend(netPort,header,data,num,type)
/* Attempt to send. return 0 on can't yet, -1 on error, 1 on success */
NetPort *netPort;
char	*header;
char	*data;
long	num;
DTMTYPE type;
{
char buff[1024];
int status;

#ifdef USE_AVAIL_WRITE
	if (DTMavailWrite(netPort->port)) {
#endif
#ifdef DEBUG
        printf("NetSend():Sending \"%s\" to %s\n",header,netPort->portName);
#endif


#ifdef USE_FEEDBACK_CALLS
	SetWriteFeedback();
#endif

#ifdef USE_WRITEMSG
		if (DTMERROR == DTMwriteMsg(netPort->port,
				header,strlen(header)+1,
				data,num,type)) 
#else
		status = DTMbeginWrite(netPort->port,header,strlen(header)+1);  
#ifdef DEBUG
        printf("NetSend():sent header \"%s\" to %s\n",header,netPort->portName);
#endif
		if ((status != DTMERROR) && num) {
			status = DTMwriteDataset(netPort->port,data,num,type);
			}
		if (status == DTMERROR)
#endif
			{
			sprintf(buff,"Error sending to %s\nError is %s\n",
				netPort->portName,NetDTMErrorString(DTMerrno));
			ErrMesg(buff);
#ifndef USE_WRITEMSG
			DTMendWrite(netPort->port);
#endif
#ifdef USE_FEEDBACK_CALLS
			UnsetFeedback();
#endif
			return(-1);
			}
#ifndef USE_WRITEMSG
		DTMendWrite(netPort->port);
#endif
#ifdef USE_FEEDBACK_CALLS
		UnsetFeedback();
#endif
#ifdef DEBUG
		printf("MESSAGE SENT:\"%s\" to %s\n",header,netPort->portName);
#endif
		netPort->queueTime = 0;
		return(1);
#ifdef USE_AVAIL_WRITE
		}
	else {
		if (netPort->queueTime == 0)
			netPort->queueTime = time(0);
		return(0);
		}
#endif
} /* NetSend() */



static int
DEFUN(NetClientSendMessage, (netPort,header,data,num,type,
		cb,cbData,failCB,failCBData,doQueue),
/* Attempt to send. return 0 on can't yet, -1 on error, 1 on success */
NetPort *netPort AND
char	*header AND
GenericPtr data AND
long	num AND
DTMTYPE type AND
void 	(*cb) PARAMS((GenericPtr data, caddr_t cbData)) AND
caddr_t	cbData AND
void 	(*failCB) PARAMS((GenericPtr data, caddr_t failCBData)) AND 
caddr_t	failCBData AND
int	doQueue)     /* TRUE -> Save and resend; FALSE -> let client resend*/

{
SQueue *sq;
int status;

	if (!netPort || ! netPort->open) {
		if (!(netPort = (NetPort *) ListHead(netOutList))) {
#ifdef DEBUG
			printf("no out port: discarding %s\n",header);
#endif
			if (failCB) 
				failCB(data,failCBData);
			return(-1);
			}
		}
#ifdef DEBUG
	printf("Attempting to send \"%s\" to netPort=%x %x (%s)\n",header,
			netPort,netPort->port,netPort->portName);
#endif

	/*
	 * Before sending this data, first we must flush any pending data
	 * on this port, to preserve sending order.
	 * If we can't flush all pending data, we must queue this data
	 * for later sending.
	 */
	status = NetFlushPort(netPort);
	if (status == 0) {
		if (doQueue) {
			if (!(sq = (SQueue *) MALLOC(sizeof(SQueue)))) {
				ErrMesg("Out of Memory\n");
				return(-1);
				}
			if (!(sq->header = (char *) MALLOC(strlen(header)+1))) {
				ErrMesg("Out of Memory when putting on send queue\n");
				return(-1);
				}
			strcpy(sq->header,header);
			sq->netPort = netPort;
			sq->data = data;
			sq->num = num;
			sq->type = type;
			sq->cb = cb;
			sq->cbData = cbData;
			sq->failCB = failCB;
			sq->failCBData = failCBData;
			sq->numTries = 1;
			ListAddEntry(sendQueue,(char *)sq);
#ifdef DEBUG
			printf(
			    "message queued for later sending on netPort %x\n",
			    sq->netPort);
#endif
			}
		else {
			if (cb) 
				cb(data,cbData);
			}
		return(0);
		}

	if (!(status = NetSend(netPort,header,data,num,type))) {
	    if (doQueue) {
		if (!(sq = (SQueue *) MALLOC(sizeof(SQueue)))) {
			ErrMesg("Out of Memory\n");
			return(-1);
			}
		if (!(sq->header = (char *) MALLOC(strlen(header)+1))) {
			ErrMesg("Out of Memory when putting on send queue\n");
			return(-1);
			}
		strcpy(sq->header,header);
		sq->netPort = netPort;
/*EJB
		if (!(sq->netPort = (NetPort *) MALLOC(sizeof(netPort)))){
			ErrMesg("Out of memory when putting on send queue\n");
			return(0);
			}
		bcopy((char *) netPort,(char *) sq->netPort,sizeof(NetPort));
*/
		sq->data = data;
		sq->num = num;
		sq->type = type;
		sq->cb = cb;
		sq->failCB = failCB;
		sq->cbData = cbData;
		sq->failCBData = failCBData;
		sq->numTries = 1;
		ListAddEntry(sendQueue,sq);
#ifdef DEBUG
		printf("message queued for later sending on netPort %x\n",
			    sq->netPort);
#endif
		}
	    else {
                /* couldn't send now and no queuing, so call failCB()*/
		if (failCB) 
			failCB(data,failCBData);
		}
		return(0);
	    }
	else if (status == 1) {
		if (cb) 
			cb(data,cbData);
		return(1);
		}
	else if (status == -1 ) {
		if (failCB)
			failCB(data,failCBData);
		return(-1);
		}
	return(-1);
		
} /* NetClientSendMessage() */


static int NetFlushPort(port)
NetPort *port;
{
SQueue *sq;
char buff[DTM_MAX_HEADER+80];
int status;
time_t t;


	sq = (SQueue *) ListHead(sendQueue);
#ifdef DEBUG
	if (sq)  {
/*
		fprintf(stderr,"Flushing queue for port %s\n",port->portName);
*/
		fprintf(stderr,"Want to send %s to %s\n",
				sq->header,sq->netPort->portName);
		fprintf(stderr,"port = %x (%s) ,sq->netPort=%x (%s)\n",
				port,
				port->portName,
				sq->netPort,
				sq->netPort->portName);
		fprintf(stderr,"*"); 
		}
#endif
	while (sq) {
		if (sq->netPort == port) {
#ifdef DEBUG
			fprintf(stderr,".");
#endif
			if (status = NetSend(sq->netPort,sq->header,
					sq->data,sq->num,sq->type)){
				if ((status == 1) &&(sq->cb))
					(sq->cb)(sq->data,sq->cbData);
				ListDeleteEntry(sendQueue,sq);
				FREE(sq->header);
				FREE(sq);
				}
			else {
                                (sq->numTries)++;
				t = time(0);
/*
	                        if (sq->numTries > netMaxAttemptsToSend) {
*/
				if (t >(sq->netPort->queueTime + netTimeOut)){
					sprintf(buff,
					    "Couldn't send %s :DISCARDING\nafter %d tries and %d seconds",
					    sq->header,sq->numTries,
					    t - sq->netPort->queueTime);
                                	ErrMesg(buff);
					DTMdestroyPort(sq->netPort->port);
					sq->netPort->open = FALSE;
                                	ListDeleteEntry(sendQueue,sq);
                                	FREE(sq->header);
					if (sq->failCB)
					    (sq->failCB)(sq->data,sq->failCBData);
                                	FREE(sq);
                                	}

				return(0);
				}
			sq = (SQueue *) ListCurrent(sendQueue);
			}
		else {
			sq = (SQueue *) ListNext(sendQueue);
			}
		}
	return(1);
} /* NetFlushPort()*/


void NetTryResend()
{
int status;
NetPort *n;

#ifdef DEBUG
/*
	if (ListHead(sendQueue))
		printf("NetTryResend(): There is something on the SendQueue\n");
	else
		printf("NetTryResend(): Nothing on the SendQueue\n");
*/
#endif
	n = (NetPort *) ListHead(netOutList);
	while (n) {
		status = NetFlushPort(n);
		ListMakeEntryCurrent(netOutList,n);
		n = (NetPort *) ListNext(netOutList);
		}
} /* NetTryResend()*/

int NetSendDisconnect(netPort,cb,failCB)
NetPort *netPort;
void 	(*cb)();
void 	(*failCB)();
{
char header[DTM_MAX_HEADER];
NetPort *myInPort;

#ifdef DEBUG
	printf("NetSendDisconnect(): I've been called\n");
#endif
        SRVsetClass(header);
        SRVsetID(header,userID);
        SRVsetFunction(header,SRV_FUNC_DISCONNECT);
	myInPort = (NetPort *) ListHead(netInList);
	if (myInPort) {
		/**** assumes only one inport */
		SRVsetInPort(header,myInPort->portName);
		}
        return(NetClientSendMessage(netPort,header,0,0,0,cb,0,failCB,0,1));
}

static int NetSendConnect(myInPort,to,cb)
NetPort *myInPort;
NetPort *to;
void 	(*cb)();
{
char header[DTM_MAX_HEADER];

#ifdef DEBUG
	printf("NetSendConnect(): I've been called \n");
#endif
	if (!myInPort)
		return(-1); /* Error, no InPort to send */
	SRVsetClass(header);
	SRVsetID(header,userID);
	SRVsetFunction(header,SRV_FUNC_CONNECT);
	SRVsetInPort(header,myInPort->portName);
	SRVsetVersionString(header,VERSION_STRING);
	SRVsetVersionNumber(header,VERSION_NUMBER);
	return(NetClientSendMessage(to,header,0,0,0,cb,0,0,0,1));
}

static void
DEFUN(NetFreeDataCB,(p, client_data),
/* Free space after it has been sent */
GenericPtr p AND
caddr_t client_data)
{
	FREE(p);
}


int NetSendDoodle(netPort,title,length,width,doodle,color,sendDiscrete,doQueue,
		  distributeInternally,moduleName)
NetPort *netPort;
char *title;    /* title of doodle (name of data set applied to */
long length;
int width;	/* the linewidth of the doodle */
POINT *doodle;
DColor *color;
int sendDiscrete;	/* TRUE -> COL_DOODLE_DISC; FALSE -> COL_DOODLE_CONT */
int doQueue;       /* TRUE -> Save and resend; FALSE -> let client resend */
int distributeInternally; /* boolean */
char *moduleName; /* Send internally to all DOCB except this one */
                  /* in most cases this would be the calling module's name*/
{
#if 0
char header[DTM_MAX_HEADER];
struct COL_TRIPLET *a;
int     status;
register long i;

        COLsetClass(header);
        COLsetTitle(header,title);
        COLsetID(header,UserID);
	if (sendDiscrete) {
	        COLsetFunc(header,"DOODLE",COL_DOODLE_DISC);
		}
	else {
	        COLsetFunc(header,"DOODLE",COL_DOODLE_CONT);
		}
        COLsetWidth(header, width);
        COLsetDimension(header,(length + 1));

        if (!(a = (struct COL_TRIPLET *)
                        MALLOC(sizeof(struct COL_TRIPLET)*(length + 1)))) {
                ErrMesg("Out of Memory.  Can't send doodle\n");
                return(0);
                }

        a[0].x = (float) color->red;
        a[0].y = (float) color->green;
        a[0].z = (float) color->blue;
        a[0].tag = DTM_FLOAT;  /*?*/
        for (i = 1; i < (length + 1); i++ ) {
                a[i].x = (float) doodle[i - 1].x;
                a[i].y = (float) doodle[i - 1].y;
                a[i].z = 0.0;
                a[i].tag = DTM_FLOAT;  /*?*/
                }

	if (distributeInternally) {
		NetCOLDistribute(
			NetMakeCOLFromDoodle(title,a,length+1,sendDiscrete),
			moduleName);
		}
		

	status = NetClientSendMessage(netPort,header,a,(length + 1),
			COL_TRIPLET, NetFreeDataCB, 0,0,0,doQueue);

	if (status == -1)
	{
		NetFreeDataCB(a, (caddr_t)NULL);
	}

       	return(status);
#endif
}

int NetSendPointSelect(netPort,title,func,x,y)
NetPort *netPort;
char *title;
char *func;
int x,y;
{
#if 0
char header[DTM_MAX_HEADER];
int     status;
struct COL_TRIPLET *p;

        COLsetClass(header);
        COLsetTitle(header,title);
        COLsetID(header,UserID);
        COLsetFunc(header,func,COL_POINT);
        COLsetDimension(header,1);
        if (!(p = (struct COL_TRIPLET *)
                        MALLOC(sizeof(struct COL_TRIPLET)))) {
                ErrMesg("Out of Memory.  Can't send point select\n");
                return(0);
                }
        p->x = (float) x;
        p->y = (float) y;
	status = NetClientSendMessage(netPort,header,p,1,COL_TRIPLET,
			NetFreeDataCB,0,NetFreeDataCB,0,1);

	return(status);
#endif
}

int NetSendLineSelect(netPort,title,func,x1,y1,x2,y2)
NetPort *netPort;
char *title;
char *func;
int x1,y1,x2,y2;
{
#if 0
char header[DTM_MAX_HEADER];
int     status;
register long i;
struct COL_TRIPLET *a;


        COLsetClass(header);
        COLsetTitle(header,title);
        COLsetID(header,UserID);
        COLsetFunc(header,"NONE",COL_LINE);
        COLsetDimension(header,2);
        if (!(a = (struct COL_TRIPLET *)
                        MALLOC(sizeof(struct COL_TRIPLET)*2))) {
                ErrMesg("Out of Memory.  Can't send line select\n");
                return(0);
                }
        a[0].x = (float) x1;
        a[0].y = (float) y1;
        a[1].x = (float) x2;
        a[1].y = (float) y2;
        a[0].z = a[1].z = 0.0;
        a[0].tag = a[1].tag = DTM_FLOAT;
	status = NetClientSendMessage(netPort,header,a,2,COL_TRIPLET,
			NetFreeDataCB,0,NetFreeDataCB,0,1);

	return(status);
#endif
}

int NetSendAreaSelect(netPort,title,func,x1,y1,x2,y2)
NetPort *netPort;
char *title;
char *func;
int x1,y1,x2,y2;
{
#if 0
char header[DTM_MAX_HEADER];
int     status;
register long i;
struct COL_TRIPLET *a;


        COLsetClass(header);
        COLsetTitle(header,title);
        COLsetID(header,UserID);
        COLsetFunc(header,"HISTOGRAM",COL_AREA);
        COLsetDimension(header,3);
        if (!(a = (struct COL_TRIPLET *)
                        MALLOC(sizeof(struct COL_TRIPLET)*3))) {
                ErrMesg("Out of Memory.  Can't send area select \n");
                return(0);
                }
        a[0].x = (float) x1;
        a[0].y = (float) y1;
        a[1].x = (float) x2;
        a[1].y = (float) y2;
        a[0].z = a[1].z = a[2].x = a[2].y = a[2].z = 0.0;
        a[0].tag = a[1].tag = a[2].tag = DTM_FLOAT;
	status = NetClientSendMessage(netPort,header,a,3,COL_TRIPLET,
			NetFreeDataCB,0,NetFreeDataCB,0,1);

        return(status);
#endif
}

int NetSendClearDoodle(netPort,title)
NetPort *netPort;
char *title;
{
#if 0
char header[DTM_MAX_HEADER];
int     status;
register long i;

        COLsetClass(header);
        COLsetTitle(header,title);
        COLsetID(header,UserID);
        COLsetFunc(header,"CLEAR",COL_DOODLE_DISC);
        COLsetDimension(header,0);
	status = NetClientSendMessage(netPort,header,0,0,0,0,0,0,0,1);
        return(status);
#endif
}


#if 0			/* not used in Collage */
int NetSendSetDoodle(netPort,title)
NetPort *netPort;
char *title;
{
        char header[DTM_MAX_HEADER];
        struct COL_TRIPLET *a;
        int status;
        register long i;

        COLsetClass(header);
        COLsetTitle(header,title);
        COLsetID(header,userID);
        COLsetFunc(header,"SET",COL_DOODLE_DISC);
        COLsetDimension(header, (long)1);

        if (!(a = (struct COL_TRIPLET *)
                MALLOC(sizeof(struct COL_TRIPLET))))
        {
                ErrMesg("Out of Memory.  Can't send set doodle\n");
                return(0);
        }

        a[0].x = 0.0;
        a[0].y = 0.0;
        a[0].z = 0.0;

        status = NetClientSendMessage(netPort,header,a,(long) 1,COL_TRIPLET,
				NetFreeDataCB,0,NetFreeDataCB,0,1);

        return(status);
}


int NetSendDoodleText(netPort,title, length, doodle, buf)
NetPort *netPort;
char *title;    /* title of doodle (name of data set applied to */
long length;
POINT *doodle;
char *buf;
{
        char header[DTM_MAX_HEADER];
        struct COL_TRIPLET *a;
        int     status;
        register long i;

        COLsetClass(header);
        COLsetTitle(header,title);
        COLsetID(header,userID);
        COLsetFunc(header,"TEXT",COL_DOODLE_DISC);
        COLsetDimension(header,length);

        if (!(a = (struct COL_TRIPLET *)
                        MALLOC(sizeof(struct COL_TRIPLET)*length))) {
                ErrMesg("Out of Memory.  Can't send doodle text\n");
                return(0);
                }

        for (i = 0; i < length; i++ ) {
                a[i].x = (float) doodle[i].x;
                a[i].y = (float) doodle[i].y;
                a[i].z = (float) buf[i];
                a[i].tag = DTM_FLOAT;  /*?*/
                }

        status = NetClientSendMessage(netPort,header,a,length,COL_TRIPLET,
				NetFreeDataCB,0,NetFreeDataCB,0,1);

        return(status);
}
#endif				/* not used in Collage */


int NetSendEraseDoodle(netPort,title,length,doodle,doQueue)
/* return 0 on failure */
NetPort *netPort;
char *title;    /* title of doodle (name of data set applied to */
long length;
POINT *doodle;
int doQueue;       /* TRUE -> Save and resend; FALSE -> let client resend */
{
#if 0
char header[DTM_MAX_HEADER];
struct COL_TRIPLET *a;
int     status;
register long i;

        COLsetClass(header);
        COLsetTitle(header,title);
        COLsetID(header,UserID);
        COLsetFunc(header,"ERASE",COL_DOODLE_DISC);
        COLsetDimension(header,length);

        if (!(a = (struct COL_TRIPLET *)
                        MALLOC(sizeof(struct COL_TRIPLET)*length))) {
                ErrMesg("Out of Memory.  Can't send erase doodle\n");
                return(0);
                }

        for (i = 0; i < length; i++ ) {
                a[i].x = (float) doodle[i].x;
                a[i].y = (float) doodle[i].y;
                a[i].z = 0.0;
                a[i].tag = DTM_FLOAT;  /*?*/
                }

        status = NetClientSendMessage(netPort,header,a,length,COL_TRIPLET,
				NetFreeDataCB,0,0,0,doQueue);

	if (status == -1)
	{
		NetFreeDataCB(a);
	}

        return(status);
#endif
}

int NetSendEraseBlockDoodle(netPort,title,length,doodle)
/* return 0 on failure */
NetPort *netPort;
char *title;    /* title of doodle (name of data set applied to */
long length;
POINT *doodle;
{
#if 0
char header[DTM_MAX_HEADER];
struct COL_TRIPLET *a;
int     status;
register long i;

        COLsetClass(header);
        COLsetTitle(header,title);
        COLsetID(header,UserID);
        COLsetFunc(header,"BERASE",COL_DOODLE_DISC);
        COLsetDimension(header,length);

        if (!(a = (struct COL_TRIPLET *)
                        MALLOC(sizeof(struct COL_TRIPLET)*length))) {
                ErrMesg("Out of Memory.  Can't send erase doodle\n");
                return(0);
                }

        for (i = 0; i < length; i++ ) {
                a[i].x = (float) doodle[i].x;
                a[i].y = (float) doodle[i].y;
                a[i].z = 0.0;
                a[i].tag = DTM_FLOAT;  /*?*/
                }

	status = NetClientSendMessage(netPort,header,a,length,COL_TRIPLET,
			NetFreeDataCB,0,NetFreeDataCB,0,1);

        return(status);
#endif
}


int NetSendTextSelection(netPort,title,left,right)
NetPort *netPort;
char *title;
int left,right;
{
#if 0
char header[DTM_MAX_HEADER];
int status;

        TXTsetClass(header);
        TXTsetTitle(header,title);
        TXTsetID(header,userID);
        TXTsetSelectionLeft(header,left);
        TXTsetSelectionRight(header,right);
	status = NetClientSendMessage(netPort,header,0,0,0,0,0,0,0,1);
	return(status);
#endif
}

int NetSendText(netPort,t,distributeInternally,moduleName)
NetPort *netPort;
Text *t;
int distributeInternally;
char *moduleName;
{
char header[DTM_MAX_HEADER];
int status;
char *buff;

	t->id = userID;
	if (distributeInternally) {
		NetTextDistribute(t,moduleName);
		}
        TXTsetClass(header);
        TXTsetTitle(header,t->title);
        TXTsetID(header,userID);
	if (t->selLeft || t->selRight) {
	        TXTsetSelectionLeft(header,t->selLeft);
		TXTsetSelectionRight(header,t->selRight);
		}
	TXTsetInsertionPt(header,t->insertPt);
	TXTsetNumReplace(header,t->numReplace);
	if (t->replaceAll)
		TXTsetReplaceAll(header);
	TXTsetDimension(header,t->dim);
#ifdef DEBUG
	printf("NetSendText(): sending textString \"%s\" dim = %d\n",
				t->textString,t->dim);
#endif
	if (!(buff = (char *) MALLOC(t->dim +1))) {
		ErrMesg("Out of Memory sending text\n");
		return(-1);
		}
	strncpy(buff,t->textString,t->dim);
	buff[t->dim] = '\0';
	status = NetClientSendMessage(netPort,header,buff,t->dim,
				DTM_CHAR,NetFreeDataCB,0,NetFreeDataCB,0,1);
	return(status);
}


int NetSendPalette8(netPort,title,rgb,associated,distributeInternally,
		    moduleName)
NetPort *netPort;
char *title;
unsigned char *rgb;
char *associated;
int	distributeInternally; /* boolean */
char *moduleName; /* Send internally to all DOCB except this one */
		  /* in most cases this would be the calling module's name*/
{
unsigned char *p;
char header[DTM_MAX_HEADER];
register unsigned char *t;
register int x;
int status;

	if (distributeInternally)
		NetPALDistribute(title,rgb,associated,moduleName);

	if (!(p = (unsigned char *) MALLOC(768))) {
		ErrMesg("Out of Memory trying to send Palette\n");
		return(-1);
		}
	t = p;
	for (x = 0; x < 768; x++)
		*t++ = *rgb++;

        PALsetClass(header);
        PALsetTitle(header,title);
	COLsetID(header,userID);
	PALsetSize(header,768/3);
	if (associated)
		COLsetAssoc(header, associated);
	status = NetClientSendMessage(netPort,header,p,768,DTM_CHAR,
			NetFreeDataCB,0,NetFreeDataCB,0,1);

	return(status);
}


int NetSendArray(netPort,d,shouldCopy,distributeInternally,moduleName,
		 isAnimation)
NetPort *netPort;
Data	*d;
int	shouldCopy;
int	distributeInternally;
char	*moduleName;
char	isAnimation;
{
char header[DTM_MAX_HEADER];
DTMTYPE dtmType;
long size,buffSize;
char *buff;
register int x;
register char *pbuff;
register char *pdata;
int status;
int	elementSize;


	SDSsetClass(header);
        SDSsetTitle(header,d->label);
	ANIMsetID(header,userID);
        if (d->expandX != 0. && d->expandY != 0.)
		ANIMsetExpansion(header,d->expandX, d->expandY);
        switch (d->dost) {
                case DOST_Float:
                        SDSsetType(header,DTM_FLOAT);
                        dtmType = DTM_FLOAT;
			elementSize = sizeof(float);
                        break;
                case DOST_Char:
                        SDSsetType(header,DTM_CHAR);
                        dtmType = DTM_CHAR;
			elementSize = sizeof(char);
                        break;
                case DOST_Int16:
                        SDSsetType(header,DTM_SHORT);
                        dtmType = DTM_SHORT;
			elementSize = sizeof(short);
                        break;
                case DOST_Int32:
                        SDSsetType(header,DTM_INT);
                        dtmType = DTM_INT;
			elementSize = sizeof(int);
                        break;
                case DOST_Double:
                        SDSsetType(header,DTM_DOUBLE);
                        dtmType = DTM_DOUBLE;
			elementSize = sizeof(double);
                        break;
                }
	if (d->dost == DOST_Float || d->dost == DOST_Double)
	  {
		if (d->min.f != d->max.f)
		  SDSsetMinMax(header, d->min.f, d->max.f);
	  }
	else
	  {
		if (d->min.i != d->max.i)
		  SDSsetMinMax(header, (float)d->min.i, (float)d->max.i);
	  }
	SDSsetDimensions(header,d->rank,d->dim);

	if (isAnimation) {
		ANIMmarkAnimation(header);
		}
#if 0
	else {
		if (d->view_type)
			COLsetView(header, d->view_type);
		}
#endif

#if 0
	if (d->associated)
		COLsetAssoc(header, d->associated);
#endif

        for (x=1,size = d->dim[0]; x < d->rank; x++)
                size *= d->dim[x];

	buffSize = size * elementSize;

	if (distributeInternally) {
		if (isAnimation)
			NetAnimationDistribute(&d,moduleName);
		else
			NetArrayDistribute(&d,moduleName);
		}

	if (shouldCopy) {
		if (!(buff = (char *) MALLOC(buffSize))) {
			ErrMesg("Couldn't allocate memory to send image\n");
			return(-1);
			}
		for (x = 0, pbuff = buff, pdata = d->data; x < buffSize; x++)
			*pbuff++ = *pdata++;
#ifdef DEBUG
	if (dtmType == DTM_FLOAT) {
		printf("\n\n\n\n###########\nThe first number is %f\n\n\n",
				*((float*) buff));
		}
#endif
		status = NetClientSendMessage(netPort,header,
				buff,size,dtmType, NetFreeDataCB,0,
					      NetFreeDataCB,0,1);
		}
	else {
		status = NetClientSendMessage(netPort,header,
				d->data,size,dtmType, 0,0,0,0,1);
#ifdef DEBUG
	if (dtmType == DTM_FLOAT) {
		printf("\n\n\n\n###########\nThe first number is %f\n\n\n",
				*((float*) buff));
		}
#endif
		}

	return(status);

} /* NetSendArray()*/


static int
DEFUN(_NetSendRaster8,(netPort,d,shouldCopy,
		       distributeInternally,moduleName,isAnimation),
      NetPort *netPort AND
      Data    *d AND
      int     shouldCopy AND
      int     distributeInternally AND
      char    *moduleName AND
      char    isAnimation)
{
	char header[DTM_MAX_HEADER];
	char *buff;
	int status;

	RISsetClass(header);
	RISsetTitle(header, d->label);
	RISsetDimensions(header, d->dim[0], d->dim[1]);
	COLsetID(header, userID);

        if (d->expandX != 0. && d->expandY != 0.)
		ANIMsetExpansion(header,d->expandX, d->expandY);
#if 0
	if (d->view_type)
		COLsetView(header, d->view_type);
#endif
#if 0
	if (d->associated)
		COLsetAssoc(header, d->associated);
#endif
	if (d->min.f != d->max.f)
		SDSsetMinMax(header, d->min.f, d->max.f);

	if (isAnimation)
		ANIMmarkAnimation(header);

	if (distributeInternally)
		if (isAnimation)
			NetAnimationDistribute(&d,moduleName);
		else
			NetRISDistribute(&d, moduleName);

	
	if (shouldCopy) {
		int buffSize = d->dim[0] * d->dim[1] * sizeof(char);
		register int x;
		register char *pbuff, *pdata;

		if (!(buff = (char *) MALLOC(buffSize))) {
			ErrMesg("Couldn't allocate memory to send image\n");
			return(-1);
			}
		for (x = 0, pbuff = buff, pdata = d->data; x < buffSize; x++)
			*pbuff++ = *pdata++;
		status = NetClientSendMessage(netPort,header,
				buff,buffSize,DTM_CHAR,NetFreeDataCB,0,
					      NetFreeDataCB,0,1);
		}
	else
		status = NetClientSendMessage(netPort,header,
				d->data,d->dim[0]*d->dim[1],DTM_CHAR,
					      0,0,0,0,1);

	return(status);
}

int NetSendAnimation(netPort,d,shouldCopy,distributeInternally,moduleName)
NetPort *netPort;
Data    *d;
int     shouldCopy;
int     distributeInternally;
char    *moduleName;
{
	int status;
	char *dName = d->label;
	Data *next = d->group;

	if (d->dot == DOT_Array)
		status = NetSendArray(netPort,d,shouldCopy,
				      distributeInternally,moduleName,
				      TRUE);
	else
		status = _NetSendRaster8(netPort,d,shouldCopy,
					 distributeInternally,moduleName,
					 TRUE);

	if (next && next->dot == DOT_Palette8)
		NetSendPalette8(netPort,next->label,next->data,dName,
				distributeInternally,moduleName);

	return(status);
}

int NetSendAnimationCommand(netPort,title,command,runType,frameNumber)
NetPort *netPort;
char *title;
AnimFunc command;
AnimRunType runType;
int frameNumber;
{
#if 0
char header[DTM_MAX_HEADER];
int status;

	ANIMsetClass(header);
	ANIMsetTitle(header,title);
	ANIMsetID(header,userID);
	
	switch (command) {
	    case AF_STOP:
			ANIMsetFunc(header,ANIM_FUNC_STOP);
			break;
	    case AF_FPLAY:
			ANIMsetFunc(header,ANIM_FUNC_FPLAY);
			break;
	    case AF_RPLAY:
			ANIMsetFunc(header,ANIM_FUNC_RPLAY);
			break;
	    case AF_NO_FUNC:
	    default:
			break;
	    };
	switch (runType) {
	    case ART_SINGLE:
			ANIMsetRunType(header,ANIM_RUN_TYPE_SINGLE);
			break;
	    case ART_CONT:
			ANIMsetRunType(header,ANIM_RUN_TYPE_CONT);
			break;
	    case ART_BOUNCE:
			ANIMsetRunType(header,ANIM_RUN_TYPE_BOUNCE);
			break;
	    case ART_NONE:
	    default:
			break;
	    };

	ANIMsetFrame(header,frameNumber);
	status = NetClientSendMessage(netPort,header,0,0,0,0,0,0,0,1);
	return(status);
#endif
}


int NetSendRaster8(netPort,d,shouldCopy,
		   distributeInternally,moduleName)
NetPort *netPort;
Data    *d;
int shouldCopy;     /* should this data be copied before returning? */
		    /* if not, charData and palette8 must not be freed or */
		    /* changed until data is sent */ 
int distributeInternally; /* boolean */
char *moduleName; /* Send internally to all DOCB except this one */
                  /* in most cases this would be the calling module's name*/
{
	return _NetSendRaster8(netPort,d,shouldCopy,
			       distributeInternally,moduleName,
			       FALSE);
}


int NetSendRaster8Group(netPort,title,charData,xdim,ydim,palette8,shouldCopy,
			distributeInternally,moduleName)
NetPort *netPort;
char *title;
unsigned char *charData;
int xdim;
int ydim;
unsigned char *palette8;
int 	shouldCopy; /* should this data be copied before returning? */
		    /* if not, charData and palette8 must not be freed or */
		    /* changed until data is sent */
int distributeInternally; /* boolean */
char *moduleName; /* Send internally to all DOCB except this one */
                  /* in most cases this would be the calling module's name*/
{
int palStatus;
int rasStatus;
Data *d;

	d = DataNew();
	d->label = title;
	d->dot = DOT_Image;
	d->dost = DOST_Char;
	d->dim[0] = xdim;
	d->dim[1] = ydim;
	d->rank = 2;
	d->data = (char *) charData;
	rasStatus = NetSendRaster8(netPort,d,shouldCopy,
				distributeInternally,moduleName);
	palStatus = NetSendPalette8(netPort,title,palette8,(char *)NULL,
				distributeInternally,moduleName);

	if ((rasStatus == -1) || (palStatus == -1)) {
		return(-1); /* error */
		}
	if ((!rasStatus) || (!palStatus)) {
		return(0); /* delayed send */
		}
	return(1); /* no prob */


}

int NetSendVData(netPort,label,magicPath,pathLength,nodeID,nodeName,field,
			numRecords, numElements,type,vdata,
			shouldCopy,distributeInternally,moduleName)
/* returns 1 on success, 0 on delayed send, -1 on error */
char	*label;
NetPort *netPort;
VdataPathElement **magicPath;
int	pathLength;
int	nodeID;
char	*nodeName;
char	*field;
int	numRecords;
int	numElements;
int	type;	
char	*vdata;
int	shouldCopy; /* copy Vdata before returning in case of delayed send */
int	distributeInternally; 
char	*moduleName; /* distribute Internally to all except */
{
#if 0
char header[DTM_MAX_HEADER];
int elementSize;
char *copyData;
DTMTYPE dtmType;
int status;
int size;

	VDATAsetClass(header);
	VDATAsetTitle(header,label);
	VDATAsetID(header,userID);
	VDATAsetPath(header,magicPath,pathLength);
	VDATAsetNodeID(header,nodeID);
	VDATAsetNodeName(header,nodeName);
	VDATAsetField(header,field);
	VDATAsetNumRecords(header,numRecords);
	VDATAsetNumElements(header,numElements);
        switch (type) {
                case DOST_Float:
                        VDATAsetType(header,DTM_FLOAT);
                        dtmType = DTM_FLOAT;
			elementSize = sizeof(float);
                        break;
                case DOST_Char:
                        VDATAsetType(header,DTM_CHAR);
                        dtmType = DTM_CHAR;
			elementSize = sizeof(char);
                        break;
                case DOST_Int16:
                        VDATAsetType(header,DTM_SHORT);
                        dtmType = DTM_SHORT;
			elementSize = sizeof(short);
                        break;
                case DOST_Int32:
                        VDATAsetType(header,DTM_INT);
                        dtmType = DTM_INT;
			elementSize = sizeof(int);
                        break;
                case DOST_Double:
                        VDATAsetType(header,DTM_DOUBLE);
                        dtmType = DTM_DOUBLE;
			elementSize = sizeof(double);
                        break;
		default:
			ErrMesg("BAD TYPE in SendVDATA\n");
			break;
                };

	if (shouldCopy)  {
		size = numRecords * numElements * elementSize;
		if (!(copyData = (char *) MALLOC(size))) {
			ErrMesg("Out of Memory\n");
			return(-1);
			}
		memcpy(copyData,vdata,size);
		status = NetClientSendMessage(netPort,header,copyData,
					numRecords * numElements,
					dtmType,NetFreeDataCB,0,0,0,1);
		}
	else {
		status = NetClientSendMessage(netPort,header,vdata,
					numRecords * numElements,
					dtmType,0,0,0,0,1);
		}
	
	return(status);
#endif
}


int NetSendDataObject(netPort,d,shouldCopy,distributeInternally,moduleName)
NetPort *netPort;
Data *d;
int shouldCopy;
int distributeInternally; /* boolean */
char *moduleName; /* Send internally to all DOCB except this one */
                  /* in most cases this would be the calling module's name*/
{
int status;
static Text t;
Data *nextD;

    while (d) {
	nextD = d->group;
	switch(d->dot) {
		case DOT_Palette8:
			status = NetSendPalette8(netPort,d->label,d->data,
						 d->associated,
					distributeInternally,moduleName);
			break;
		case DOT_Array:
			if (d->rank == 1) {
				d->rank = 2;
				d->dim[1] = 1;
				}
			if (d->rank > 3) {
				WarningMesg(
			"Collage doesn't support greater than 3D data\n");
					/* should free data here?*/
				}
			status = NetSendArray(netPort,d,
					      shouldCopy,
					      distributeInternally,
					      moduleName,FALSE);
			break;
		case DOT_Image:
			if ( (nextD) 
			    && (nextD->dot == DOT_Palette8)
			    && !nextD->associated )
				nextD->associated = d->label;
			status = NetSendRaster8(netPort,d,
						shouldCopy,
						distributeInternally,
						moduleName);
			break;
		case DOT_Text:
			t.title = d->label;
			t.id = userID;
			t.selLeft = 0;
			t.selRight= 0;
			t.numReplace = 0;
			t.replaceAll = TRUE;
			t.dim = d->dim[0];
			t.textString = d->data;
			status = NetSendText(netPort,&t,distributeInternally,
				moduleName);
			break;
		case DOT_VData:
			status = NetSendVData(netPort,d->label,d->magicPath,
					d->pathLength,d->nodeID,d->nodeName,
					d->fields, d->dim[0],d->dim[1],
					d->dost, d->data,
					shouldCopy,distributeInternally,
					moduleName);
			break;

		default:
#ifdef DEBUG
		printf("*******NetSendDataObject() doesn't know this type\n");
#endif
			status = -1;
			break;
		
		};
	    d = nextD;
	    }

	return(status);
}

int NetSendCommand(netPort,domain,message,cb,failCB)
NetPort *netPort;
char *domain;
char *message;
void 	(*cb)();	/* callback when sent */
void 	(*failCB)();	/* callback if send failed */
{
char header[DTM_MAX_HEADER];

	COMsetClass(header);
	COMsetID(header,userID);
	COMsetDomain(header,domain);
	COMsetMesg(header,message);
	
	return(NetClientSendMessage(netPort,header,0,0,0,cb,0,failCB,0,1));
	
}

#if 0				/* not used */
int NetSendExecuteProc(netPort,retAddress,argc,argv,cb,cbData,failCB,failCBData)
NetPort netPort;
char *retAddress;
int argc;
char **argv;
void (*cb)();
caddr_t cbData;
void (*failCB)();
caddr_t failCBData;
{

}

static NetExecCB(data,client_data)
caddr_t data;
caddr_t client_data;
{
ExecCBData *ecbd;

	ecbd = (ExecCBData *) client_data;

	NetDestroyPort((NetPort *) ecbd->internal);
	if (ecbd->cb) {
		ecbd->cb(data,ecbd->cbData);
		}
	FREE(ecbd);
}

static NetExecFailCB(data,client_data)
caddr_t data;
caddr_t client_data;
{
ExecCBData *ecbd;

	ecbd = (ExecCBData *) client_data;
	NetDestroyPort((NetPort *) ecbd->internal);

	if (ecbd->failCB) {
		ecbd->failCB(data,ecbd->failCBData);
		}
	FREE(ecbd);
}

int NetSendHostStatusRequest(outPortAddr,retAddress,cb,cbData,failCB,failCBData)
char *outPortAddr;
char *retAddress;
void (*cb)();
caddr_t cbData;
void (*failCB)();
caddr_t failCBData;
{
char header[DTM_MAX_HEADER];
NetPort *netPort;
NetPort *inPort;
ExecCBData *ecbd;
time_t	now;

	if (!(netPort = NetInternalCreateOutPort(outPortAddr,FALSE))) {
		return(-1);
		}
	if (!( ecbd = (ExecCBData *) MALLOC(sizeof(ExecCBData)))) {
		ErrMesg("Out of Memory\n");
		return(-1);
		}
	ecbd->internal = (caddr_t) netPort;
	ecbd->cb = cb;
	ecbd->cbData = cbData;
	ecbd->failCB = failCB;
	ecbd->failCBData = failCBData;
	

	EXECsetClass(header);
	EXECsetID(header,userID);

	if (retAddress) {
		EXECsetAddress(header,retAddress);
		}
	else {
		/* if retAddress == 0, use first in port */
		if (!(inPort = (NetPort *) ListHead(netInList))) {
			/* no in port created */
			return(-1);
			}
		EXECsetAddress(header,inPort->portName);
		}
	now = time(0);
	EXECsetTimeStamp(header,ctime(&now));
	EXECsetType(header,EXEC_HOST_STATUS_QUERY);
	return(NetClientSendMessage(netPort,header,0,0,0,NetExecCB,ecbd,
						NetExecFailCB,ecbd,1));
}


int NetSendHostStatus(outPortAddr,retAddress,timeStamp,load1,load5,load15,
			numUsers,cb,cbData,failCB,failCBData)
char *outPortAddr;
char *retAddress;
char *timeStamp;
float load1,load5,load15;
int numUsers;
void (*cb)();
caddr_t cbData;
void (*failCB)();
caddr_t failCBData;
{
char header[DTM_MAX_HEADER];
NetPort *netPort;
NetPort *inPort;
ExecCBData *ecbd;

	if (!(netPort = NetInternalCreateOutPort(outPortAddr,FALSE))) {
		return(-1);
		}
	if (!( ecbd = (ExecCBData *) MALLOC(sizeof(ExecCBData)))) {
		ErrMesg("Out of Memory\n");
		return(-1);
		}
	ecbd->internal = (caddr_t) netPort;
	ecbd->cb = cb;
	ecbd->cbData = cbData;
	ecbd->failCB = failCB;
	ecbd->failCBData = failCBData;

	EXECsetClass(header);
	EXECsetID(header,userID);

	if (retAddress) {
		EXECsetAddress(header,retAddress);
		}
	else {
		/* if retAddress == 0, use first in port */
		if (!(inPort = (NetPort *) ListHead(netInList))) {
			/* no in port created */
			return(-1);
			}
		EXECsetAddress(header,inPort->portName);
		}

	EXECsetTimeStamp(header,timeStamp);
	EXECsetType(header,EXEC_HOST_STATUS_RETURN);
	EXECsetLoad1(header,load1);
	EXECsetLoad5(header,load5);
	EXECsetLoad15(header,load15);
	EXECsetNumUsers(header,numUsers);
	return(NetClientSendMessage(netPort,header,0,0,0,NetExecCB,ecbd,
					NetExecFailCB,ecbd,1));
}
#endif				/* not used */




int NetSendMessage(netPort,message,cb,cbData,failCB,failCBData)
NetPort *netPort;
char *message;
void (*cb)();
caddr_t cbData;
void (*failCB)();
caddr_t failCBData;
{
char header[DTM_MAX_HEADER];
char tmp[DTM_STRING_SIZE];
	
	MSGsetClass(header);
	COLsetID(header, userID);
	strncpy(tmp, message, DTM_STRING_SIZE - 1);
	tmp[DTM_STRING_SIZE - 1] = 0;
	MSGsetString(header,tmp);
	return(NetClientSendMessage(netPort,header,0,0,0,cb,cbData,
					failCB,failCBData,1));
}

