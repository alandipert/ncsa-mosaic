#define SRVclass		"SRV"
#define SRVid			"ID"
#define SRVFunc			"FNC"
#define SRVInPort		"INPORT"
#define SRVVString		"VERSION"
#define SRVVnumber		"VERSIONNUM"
#define SRVLock			"LOCK"

#define	SRV_FUNC_CONNECT	1
#define SRV_FUNC_DISCONNECT	2
#define SRV_FUNC_LOCK		3
#define SRV_FUNC_UNLOCK		4
#define SRV_FUNC_ADD_USER	5
#define SRV_FUNC_REMOVE_USER	6

/*  for future use */
#define SRV_FUNC_LOCK_REQUEST	7

/**************************************************************
 * 
 * for a connect message:
 *	SRVclass SRVid SRV_FUNC_CONNECT SRVInPort SRVVString SRVnumber
 * for a disconnect message:
 *	SRVclass SRVid SRV_FUNC_DISCONNECT SRVInPort 
 * for a lock:
 *	SRVclass SRVid SRV_FUNC_LOCK SRVLock
 * for a lock:
 *	SRVclass SRVid SRV_FUNC_UNLOCK SRVLock
 * for a user joining:
 *	SRVclass SRVid SRV_FUNC_ADD_USER SRVInPort
 * for a user leaving:
 *	SRVclass SRVid SRV_FUNC_REMOVE_USER SRVInPort
 */
 

/* SRVsetClass(char *header,char *title) */
/* SRVcompareClass(char *header,char *title, int maxTitleLength) */
#ifdef DTM1
#define SRVsetClass(h)          DTMsetClass((h), SRVclass)
#define SRVcompareClass(h)      DTMcompareClass((h), SRVclass)
#else
#define SRVsetClass(h)          dtm_set_class((h), SRVclass)
#define SRVcompareClass(h)      dtm_compare_class((h), SRVclass)
#endif


/* SRVsetID(char *header,char *id) */
/* SRVgetID(char *header,char *id, int maxIdLength) */
#define SRVsetID(h,s)   dtm_set_char(h,SRVid,s)
#define SRVgetID(h,s,l) dtm_get_char(h,SRVid,s,l)


/* SRVsetFunction(char *header,int function) */
/* SRVgetFunction(char *header,int &function) */
/*	function is either SRV_FUNC_CONNECT or SRV_FUNC_DISCONNECT */
#define SRVsetFunction(h,i)   dtm_set_int(h,SRVFunc,i)
#define SRVgetFunction(h,i)   dtm_get_int(h,SRVFunc,i)


/* SRVsetInPort(char *header,char *InPort) */
/* SRVgetInPort(char *header,char *InPort,int maxInPortStringLength) */
#define SRVsetInPort(h,s)   dtm_set_char(h,SRVInPort,s)
#define SRVgetInPort(h,s,l) dtm_get_char(h,SRVInPort,s,l)

/* SRVsetVersionString(char *header,char *version) */
/* SRVgetVersionString(char *header,char *version, int maxIdLength) */
#define SRVsetVersionString(h,s)   dtm_set_char(h,SRVVString,s)
#define SRVgetVersionString(h,s,l) dtm_get_char(h,SRVVSTRING,s,l)

/* SRVsetVersionNumber(char *header,int number) */
/* SRVgetVersionNumber(char *header,int &number) */
#define SRVsetVersionNumber(h,i)   dtm_set_int(h,SRVVnumber,i)
#define SRVgetVersionNumber(h,i)   dtm_get_int(h,SRVVnumber,i)

/* SRVsetLockClass(char *header,int mask) */
/* SRVgetLockClass(char *header,int &mask) */
/* 1 is locked 0 is unlocked */

#define SRV_SDS_MASK	(1L << 0)
#define SRV_RIS8_MASK	(1L << 1)
#define SRV_PAL_MASK	(1L << 2)
#define SRV_ANIM_MASK	(1L << 3)
#define SRV_COL_MASK	(1L << 4)
#define SRV_TXT_MASK	(1L << 5)
#define SRV_VDATA_MASK	(1L << 6)
#define SRV_MESG_MASK	(1L << 7)
#define SRV_COM_MASK	(1L << 8)
#define SRV_EXEC_MASK	(1L << 9)

#define SRVsetLockClass(h,i)	dtm_set_int(h,SRVLock,i)
#define SRVgetLockClass(h,i)	dtm_get_int(h,SRVLock,i)
