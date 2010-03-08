#ifndef VDATA_DTM_INCLUDE_BEFORE
#define VDATA_DTM_INCLUDE_BEFORE

#define VDATAclass	"VDATA"
#define VDATAid		"ID"
#define VDATApathLength	"PLEN"
#define VDATApathName	"PNAME"
#define VDATApathID	"PID"
#define VDATAnodeID	"NID"
#define VDATAnodeName	"NNAME"
#define VDATAfield	"FIELD"
#define VDATAnumRec	"NREC"
#define VDATAnumElem	"NELEM"

#define VDATAsetClass(h)	dtm_set_class(h,VDATAclass)
#define VDATAcompareClass(h)	dtm_compare_class(h,VDATAclass)

#define VDATAsetTitle		dtm_set_title
#define VDATAgetTitle		dtm_get_title

#define VDATAsetID(h,s)		dtm_set_char(h,VDATAid,s)
#define VDATAgetID(h,s,l)	dtm_get_char(h,VDATAid,s,l)

#define VDATAsetType		dtm_set_type
#define VDATAgetType		dtm_get_type

#define VDATAgetPathLength(h,i)	dtm_get_int(h,VDATApathLength,i)

#define VDATAsetNodeID(h,i)	dtm_set_int(h,VDATAnodeID,i);
#define VDATAgetNodeID(h,i)	dtm_get_int(h,VDATAnodeID,i);

#define VDATAsetNodeName(h,s)	dtm_set_char(h,VDATAnodeName,s)
#define VDATAgetNodeName(h,s,l)	dtm_get_char(h,VDATAnodeName,s,l)

#define VDATAsetField(h,s)	dtm_set_char(h,VDATAfield,s)
#define VDATAgetField(h,s,l)	dtm_get_char(h,VDATAfield,s,l)

#define VDATAsetNumRecords(h,i)	dtm_set_int(h,VDATAnumRec,i);
#define VDATAgetNumRecords(h,i)	dtm_get_int(h,VDATAnumRec,i);

#define VDATAsetNumElements(h,i)	dtm_set_int(h,VDATAnumElem,i);
#define VDATAgetNumElements(h,i)	dtm_get_int(h,VDATAnumElem,i);

typedef struct {
        int nodeID;
        char *nodeName;
        } VdataPathElement;

#endif /* VDATA_DTM_INCLUDE_BEFORE */
