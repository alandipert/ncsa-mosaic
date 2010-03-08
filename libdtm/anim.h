#define ANIMclass	"ANIM"
#define ANIMtag		"ANIMATION"
#define ANIMtitle	"TTL"
#define ANIMframeNumber	"FRN"
#define ANIMfunc	"FNC"
#define ANIMid		"ID"
#define ANIMskip	"SKIP"
#define ANIMRunType	"RType"
#define ANIMExpandX	"XEXPAND"
#define ANIMExpandY	"YEXPAND"

#define ANIM_FUNC_STOP	1
#define ANIM_FUNC_FPLAY	2	/* forward play */
#define ANIM_FUNC_RPLAY	3	/* reverse play */

#define ANIM_RUN_TYPE_SINGLE	1    /* single play */
#define ANIM_RUN_TYPE_CONT	2    /* continuous play in one direction*/
#define ANIM_RUN_TYPE_BOUNCE	3    /* continuous play alternating direction*/

#define ANIMsetClass(h)          dtm_set_class(h,ANIMclass)
#define ANIMcompareClass(h)      dtm_compare_class(h,ANIMclass)

#define ANIMsetTitle             dtm_set_title
#define ANIMgetTitle             dtm_get_title

#define ANIMsetID(h,s)           dtm_set_char(h,ANIMid,s)
#define ANIMgetID(h,s,l)         dtm_get_char(h,ANIMid,s,l)

#define ANIMsetFunc(h,i)	dtm_set_int(h,ANIMfunc,i)
#define ANIMgetFunc(h,i)	dtm_get_int(h,ANIMfunc,i)

#define ANIMsetRunType(h,i)	dtm_set_int(h,ANIMRunType,i)
#define ANIMgetRunType(h,i)	dtm_get_int(h,ANIMRunType,i)

#define ANIMsetFrame(h,i)	dtm_set_int(h,ANIMframeNumber,i)
#define ANIMgetFrame(h,i)	dtm_get_int(h,ANIMframeNumber,i)

#define ANIMsetFrameSkip(h,i)	dtm_set_int(h,ANIMskip,i)
#define ANIMgetFrameSkip(h,i)	dtm_get_int(h,ANIMskip,i)

/* used to mark SDS as animation frame */
#define ANIMmarkAnimation(h)	dtm_set_int(h,ANIMtag,1)
#define ANIMisAnimation(h,i)	((dtm_get_int(h,ANIMtag,&(i)) == -1)?0:1)

#define ANIMsetExpansion(h,x,y)	{dtm_set_float(h,ANIMExpandX,x); \
				 dtm_set_float(h,ANIMExpandY,y); }

#define ANIMgetExpansion(h,x,y)	{if (-1 == dtm_get_float(h,ANIMExpandX,x)) \
					{ (*(x)) = 1.0; (*(y)) = 1.0;} \
				if (-1 == dtm_get_float(h,ANIMExpandY,y)) \
					{ (*(x)) = 1.0; (*(y)) = 1.0;} \
				}
