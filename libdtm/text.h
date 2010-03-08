#define TXTclass	"TXT"
#define	TXTid		"ID"
#define	TXTdims		"DIM"
#define	TXTip		"IPT"	/* Insertion Point */
#define	TXTncr		"NCR"	/* Number of characters to Replace */
#define	TXTra		"TRA"	/* Text should replace entire file*/
#define	TXTsl		"TSL"   /* Text selection left position */
#define	TXTsr		"TSR"   /* Text selection right position */

/* TXTsetClass(char *header) */
#ifdef DTM1
#define	TXTsetClass(h)		DTMsetClass((h), TXTclass)
#define	TXTcompareClass(h)	DTMcompareClass((h), TXTclass)
#else
#define	TXTsetClass(h)		dtm_set_class((h), TXTclass)
#define	TXTcompareClass(h)	dtm_compare_class((h), TXTclass)
#endif


/* TXTsetTitle(char *header,char *title) */
/* TXTgetTitle(char *header,char *title, int maxTitleLength) */
#ifdef DTM1
#define TXTsetTitle	DTMsetTitle
#define	TXTgetTitle	DTMgetTitle
#else
#define TXTsetTitle	dtm_set_title
#define	TXTgetTitle	dtm_get_title
#endif


/* TXTsetID(char *header,char *id) */
/* TXTgetID(char *header,char *id, int maxIdLength) */
#define	TXTsetID(h,s)	dtm_set_char(h,TXTid,s)
#define TXTgetID(h,s,l)	dtm_get_char(h,TXTid,s,l)

/* TXTsetInsertionPt(char *header,int InsertionPoint) */
/* TXTsetInsertionPt(char *header,int &InsertionPoint) */
#define TXTsetInsertionPt(h,i)	dtm_set_int(h,TXTip,i)
#define TXTgetInsertionPt(h,i)	dtm_get_int(h,TXTip,i)

/* TXTsetNumReplace(char *header,int numberToReplace) */
/* 		if (numberToReplace == 0) then no replace just insert */
/* TXTgetNumReplace(char *header,int &numberToReplace) */
#define TXTsetNumReplace(h,i)	dtm_set_int(h,TXTncr,i)
#define TXTgetNumReplace(h,i)	dtm_get_int(h,TXTncr,i)

/* TXTsetReplaceAll(char *header) */
/* Boolean TXTshouldReplaceAll(char *header,int garbage) */
#define TXTsetReplaceAll(h)	dtm_set_int(h,TXTra,1)
#define TXTshouldReplaceAll(h,i)	(dtm_get_int(h,TXTra,&(i)) == -1)?0:1 

/* TXTsetDimension(char *header,int numBytes) */
/* TXTgetDimension(char *header,int &numBytes) */
#define	TXTsetDimension(h,i)	dtm_set_int(h,TXTdims,i)
#define	TXTgetDimension(h,i)	dtm_get_int(h,TXTdims,i)

/* TXTsetSelectionLeft(char *header, int leftPosition) */
/* TXTsetSelectionLeft(char *header, int &leftPosition) */
#define TXTsetSelectionLeft(h,i)	dtm_set_int(h,TXTsl,i)
#define TXTgetSelectionLeft(h,i)	dtm_get_int(h,TXTsl,i)

/* TXTsetSelectionRight(char *header, int rightPosition) */
/* TXTsetSelectionRight(char *header, int &rightPosition) */
#define TXTsetSelectionRight(h,i)	dtm_set_int(h,TXTsr,i)
#define TXTgetSelectionRight(h,i)	dtm_get_int(h,TXTsr,i)

#define	TXTheaderLength		dtm_header_length
#define TXTHL(h)		dtm_header_length

