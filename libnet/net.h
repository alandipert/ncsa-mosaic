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
/* $Id: net.h,v 1.1.1.1 1995/01/11 00:03:39 alanb Exp $ */

#include <sys/types.h>
#include "netdata.h"
#include "doodle.h"

#define PORTNAMESIZE	25

#define	NET_UNDEF	0
#define	NET_IN		1
#define	NET_OUT		2


typedef enum { NETRIS8, NETSDS, NETPAL, NETTXT, NETSRV, NETCOL, NETDTM,
		NETANIM, NETVDATA, NETSDL, NETCOM, NETEXEC, NETMSG,
	       NETDOL} NetType;

typedef struct {
	int	port;
	char	portName[PORTNAMESIZE];
	char	open;	/*boolean*/
	int	type;
	time_t	queueTime;
	} NetPort;


typedef struct {
	char	*title;
	char	*id;
	int 	selLeft;	/* selection Left */
	int 	selRight;	/* selection Right */
	int 	insertPt;	/* insertion point */
        int     numReplace;     /* number to replace */
	int	replaceAll;	/* boolean should reaplace All text */
	int 	dim;		/* dimensions */
	char	*textString;
	} Text;

typedef struct {
	char	*title;
	char	*id;
	char	*func;
	int	selType;
	int	width;
	int	dim;
	struct	COL_TRIPLET *data;
	} Col;

typedef struct {
	char	*title;
	char	*id;
	char	*func;
	int	selType;
	int	width;
	int	dim;
	short int *data;
	} Dol;

typedef enum { AF_NO_FUNC, AF_STOP, AF_FPLAY, AF_RPLAY 
		} AnimFunc;

typedef enum { ART_NONE, ART_SINGLE, ART_CONT, ART_BOUNCE
                } AnimRunType;

typedef struct {
	char		*title;
	char		*id;
	AnimFunc 	func;
	int		frameNumber;
	AnimRunType	runType;
	Data 		*data;
	int		axis;
	} AnimMesg;

typedef struct {
	char 	id[80];
	char	inPort[80];
	int	func;
	NetPort	*netPort;
	} Server;

typedef struct {
	char *id;
	char *domain;
	char *mesg;
	} Com;


extern	void	NetSetASync PARAMS((int set));
extern	int	NetRegisterModule
	PARAMS((char *name, NetType netType, void (*new)(), caddr_t newData,
		void (*change)(), caddr_t changeData, void (*destroy)(),
		caddr_t destroyData));
extern	void	NetDestroyPort PARAMS((NetPort *netPort));
extern	NetPort *NetCreateInPort PARAMS((char *inPortAddr));
extern	NetPort *NetCreateOutPort PARAMS((char *outPortAddr));
extern	NetPort *NetIsConnected PARAMS((void));
extern	int	NetInit PARAMS((char *));
extern	void	NetClientPollAndRead PARAMS((void));
extern	void	NetTryResend PARAMS((void));
extern	int	NetPALDistribute
	PARAMS((char *title, unsigned char *rgb, char *assoc, char *excepMod));
extern	int	NetRISDistribute
	PARAMS((Data **dSend, char *exceptMod));
extern	int	NetSendDisconnect
	PARAMS((NetPort *netPort, void 	(*cb)(), void 	(*failCB)()));
extern	int	NetSendDoodle
	PARAMS((NetPort *netPort, char *title, long length, int width,
		POINT *doodle, DColor *color, int sendDiscrete, int doQueue,
		int distributeInternally, char *moduleName));
extern	int	NetSendPointSelect
	PARAMS((NetPort *netPort, char *title, char *func, int x, int y));
extern	int	NetSendLineSelect
	PARAMS((NetPort *netPort, char *ti, char *fu, int x1, int y1,
		int x2, int y2));
extern	int	NetSendAreaSelect
	PARAMS((NetPort *netPort, char *ti, char *fu, int x1, int y1,
		int x2, int y2));
extern	int	NetSendClearDoodle PARAMS((NetPort *netPort, char *title));
#if 0
extern	int	NetSendSetDoodle();
extern	int	NetSendDoodleText();
#endif
extern	int	NetSendEraseDoodle
	PARAMS((NetPort *netPort, char *title, long length, POINT *doodle,
	       int doQueue));
extern	int	NetSendEraseBlockDoodle
	PARAMS((NetPort *netPort, char *title, long length, POINT *doodle));
extern	int	NetSendTextSelection
	PARAMS((NetPort *netPort, char *title, int left, int right));
extern	int	NetSendText
	PARAMS((NetPort *netPort, Text *t, int distributeInternally,
		char *moduleName));
extern	int	NetSendPalette8
	PARAMS((NetPort *netPort, char *title, unsigned char *rgb,
		char *associated, int distributeInternally, char *moduleName));
extern	int	NetSendAnimation
	PARAMS((NetPort *netPort, Data *d, int shouldCopy,
		int distributeInternally, char    *moduleName));
extern	int	NetSendAnimationCommand
	PARAMS((NetPort *netPort, char *title, AnimFunc command,
		AnimRunType runType, int frameNumbe));
extern	int	NetSendRaster8
	PARAMS((NetPort *netPort, Data *d, int shouldCopy,
		int distributeInternally, char *moduleName));
extern	int	NetSendRaster8Group
	PARAMS((NetPort *netPort, char *title, unsigned char *charDat,
		int xdim, int ydim, unsigned char *palette8, int shouldCopy,
		int distributeInternally, char *moduleName));
extern	int	NetSendVData
	PARAMS((NetPort *netPort, char	*label, VdataPathElement **magicPath,
		int pathLength, int nodeID, char *nodeName, char *field,
		int numRecords, int numElements, int type, char	*vdata,
		int shouldCopy, int distributeInternally, char *moduleName));
extern	int	NetSendDataObject
	PARAMS((NetPort *netPort, Data *d, int shouldCopy,
		int distributeInternally, char *moduleName));
extern	int	NetSendCommand
	PARAMS((NetPort *netPort, char *domain, char *message, void (*cb)(),
		void 	(*failCB)()));
extern	int	NetGetListOfUsers PARAMS((int max, char **users));
extern	int	NetSendMessage
	PARAMS((NetPort *netPort, char *message, void (*cb)(), caddr_t cbData,
		void (*failCB)(), caddr_t failCBData));
extern	void NetSetTimeOut
	PARAMS((int seconds));
extern	int	NetGetTimeOut PARAMS((void));
extern	void NetSetMaxAttemptsToSend
	PARAMS((int numberTries));
extern	int NetGetMaxAttemptsToSend PARAMS((void));
