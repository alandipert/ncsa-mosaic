
/* MODULE							HTAssoc.c
**	    ASSOCIATION LIST FOR STORING NAME-VALUE PAIRS.
**	    NAMES NOT CASE SENSITIVE, AND ONLY COMMON LENGTH
**	    IS CHECKED (allows abbreviations; well, length is
**	    taken from lookup-up name, so if table contains
**	    a shorter abbrev it is not found).
** AUTHORS:
**	AL	Ari Luotonen	luotonen@dxcern.cern.ch
**
** HISTORY:
**
**
** BUGS:
**
**
*/
#include "../config.h"

#include <string.h>
#include "../libnut/str-tools.h"
#include "HTAAUtil.h"
#include "HTAssoc.h"
#include "HTString.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

PUBLIC HTAssocList *HTAssocList_new NOARGS
{
    return HTList_new();
}


PUBLIC void HTAssocList_delete ARGS1(HTAssocList *, alist)
{
    if (alist) {
	HTAssocList *cur = alist;
	HTAssoc *assoc;
	while (NULL != (assoc = (HTAssoc*)HTList_nextObject(cur))) {
	    if (assoc->name) free(assoc->name);
	    if (assoc->value) free(assoc->value);
	    free(assoc);
	}
	HTList_delete(alist);
    }
}


PUBLIC void HTAssocList_add ARGS3(HTAssocList *,	alist,
				  WWW_CONST char *,		name,
				  WWW_CONST char *,		value)
{
    HTAssoc *assoc;

    if (alist) {
	if (!(assoc = (HTAssoc*)malloc(sizeof(HTAssoc))))
	    outofmem(__FILE__, "HTAssoc_add");
	assoc->name = NULL;
	assoc->value = NULL;

	if (name) StrAllocCopy(assoc->name, name);
	if (value) StrAllocCopy(assoc->value, value);
	HTList_addObject(alist, (void*)assoc);
    }
#ifndef DISABLE_TRACE
    else if (www2Trace) fprintf(stderr, "HTAssoc_add: ERROR: assoc list NULL!!\n");
#endif
}


PUBLIC char *HTAssocList_lookup ARGS2(HTAssocList *,	alist,
				      WWW_CONST char *,	name)
{
    HTAssocList *cur = alist;
    HTAssoc *assoc;

    while (NULL != (assoc = (HTAssoc*)HTList_nextObject(cur))) {
	if (!my_strncasecmp(assoc->name, name, strlen(name)))
	    return assoc->value;
    }
    return NULL;
}

