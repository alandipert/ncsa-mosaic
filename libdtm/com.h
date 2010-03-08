#define COMclass	"COM"
#define COMid		"ID"
#define COMdomain	"COMDOMAIN"
#define COMmesg		"COMMAND"

#define COMsetClass(h)		dtm_set_class(h,COMclass)
#define COMcompareClass(h)	dtm_compare_class(h,COMclass)

#define COMsetID(h,s)		dtm_set_char(h,COMid,s)
#define COMgetID(h,s,l)		dtm_get_char(h,COMid,s,l)

#define COMsetDomain(h,s)	dtm_set_char(h,COMdomain,s)
#define COMgetDomain(h,s,l)	dtm_get_char(h,COMdomain,s,l)

#define COMsetMesg(h,s)		dtm_set_char(h,COMmesg,s)
#define COMgetMesg(h,s,l)	dtm_get_char(h,COMmesg,s,l)
