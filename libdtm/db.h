/*****************************************************************************
*
*                         NCSA DTM version 2.3
*                               May 1, 1992
*
* NCSA DTM Version 2.3 source code and documentation are in the public
* domain.  Specifically, we give to the public domain all rights for future
* licensing of the source code, all resale rights, and all publishing rights.
*
* We ask, but do not require, that the following message be included in all
* derived works:
*
* Portions developed at the National Center for Supercomputing Applications at
* the University of Illinois at Urbana-Champaign.
*
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
*
*****************************************************************************/

/*
**
**  Database Message Headers
**
**
*/

#define	DBclass			"DB"
#define	DBsize			1024

#define	DBfile			"DBF"
#define	DBauthor		"AUTH"
#define	DBscience		"SCI"
#define DBtitle			"TITLE"
#define	DBdatatype		"DT"
#define	DBkeywords		"KEYS"

#define	DBrecsize		"RS"
#define	DBmatch			"MATCH"


#define	DBsetClass(h)		DTMsetClass(h, DBclass)
#define	DBcompareClass(h)	DTMcompareClass(h, DBclass)

#define DBsetDB(h, s)		dtm_set_char(h, DBfile, s)
#define	DBgetDB(h, s, l)	dtm_get_char(h, DBfile, s, l)

#define DBsetAuthor(h, s)	dtm_set_char(h, DBauthor, s)
#define	DBgetAuthor(h, s, l)	dtm_get_char(h, DBauthor, s, l)

#define DBsetTitle(h, s)	dtm_set_char(h, DBtitle, s)
#define	DBgetTitle(h, s, l)	dtm_get_char(h, DBtitle, s, l)

#define DBsetScience(h, s)	dtm_set_char(h, DBscience, s)
#define	DBgetScience(h, s, l)	dtm_get_char(h, DBscience, s, l)

#define	DBsetKeywords(h, s)	dtm_set_char(h, DBkeywords, s)
#define	DBgetKeywords(h, s, l)	dtm_get_char(h, DBkeywords, s, l)

#define	DBsetDatatype(h, s)	dtm_set_char(h, DBdatatype, s)
#define	DBgetDatatype(h, s, l)	dtm_get_char(h, DBdatatype, s, l)

#define	DBsetRecordSize(h, i)	dtm_set_int(h, DBrecsize, i)
#define	DBgetRecordSize(h, ip)	dtm_get_int(h, DBrecsize, ip)

#define	DBsetMatches(h, i)	dtm_set_int(h, DBmatch, i)
#define	DBgetMatches(h, ip)	dtm_get_int(h, DBmatch, ip)
