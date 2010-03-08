#define EXECclass	"EXEC"
#define EXECid		"ID"
#define EXECtime	"TIME"
#define EXECaddress	"ADDRESS"
#define EXECauth	"AUTHENTICATION"
#define EXECtype	"TYPE"

#define EXEC_HOST_STATUS_QUERY	1
#define EXEC_HOST_STATUS_RETURN 2
#define EXEC_EXECUTE		3
#define EXEC_EXECUTE_RETURN	4
#define EXEC_PROC_STATUS_QUERY	5
#define EXEC_PROC_STATUS_RETURN 6
#define EXEC_FILE_PUT		7
#define EXEC_FILE_GET		8

#define EXEC_HS_LOAD1		"LOAD1"
#define EXEC_HS_LOAD5		"LOAD5"
#define EXEC_HS_LOAD15		"LOAD15"
#define EXEC_HS_NUM_USERS	"NUMUSERS"


#define EXECsetClass(h)			dtm_set_class(h,EXECclass)
#define EXECcompareClass(h)		dtm_compare_class(h,EXECclass)

#define EXECsetID(h,s)			dtm_set_char(h,EXECid,s)
#define EXECgetID(h,s,l)		dtm_get_char(h,EXECid,s,l)

#define EXECsetAddress(h,s)		dtm_set_char(h,EXECaddress,s)
#define EXECgetAddress(h,s,l)		dtm_get_char(h,EXECaddress,s,l)

#define EXECsetTimeStamp(h,s)		dtm_set_char(h,EXECtime,s)
#define EXECgetTimeStamp(h,s,l)		dtm_get_char(h,EXECtime,s,l)

#define EXECsetAuthentication(h,s)	dtm_set_char(h,EXECauth,s)
#define EXECgetAuthentication(h,s,l)	dtm_get_char(h,EXECauth,s,l)

#define EXECsetType(h,i)		dtm_set_int(h,EXECtype,i)
#define EXECgetType(h,i)		dtm_get_int(h,EXECtype,i)


/*************************/
/* if type is EXEC_HOST_STATUS_RETURN */

#define EXECsetLoad1(h,f)		dtm_set_float(h,EXEC_HS_LOAD1,f)
#define EXECgetLoad1(h,f)		dtm_get_float(h,EXEC_HS_LOAD1,f)

#define EXECsetLoad5(h,f)		dtm_set_float(h,EXEC_HS_LOAD5,f)
#define EXECgetLoad5(h,f)		dtm_get_float(h,EXEC_HS_LOAD5,f)

#define EXECsetLoad15(h,f)		dtm_set_float(h,EXEC_HS_LOAD15,f)
#define EXECgetLoad15(h,f)		dtm_get_float(h,EXEC_HS_LOAD15,f)

#define EXECsetNumUsers(h,i)		dtm_set_int(h,EXEC_HS_NUM_USERS,i)
#define EXECgetNumUsers(h,i)		dtm_get_int(h,EXEC_HS_NUM_USERS,i)

/*************************/



