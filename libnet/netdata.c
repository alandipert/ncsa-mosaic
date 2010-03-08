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
static char rcs_id[] = "$Id: netdata.c,v 1.1.1.1 1995/01/11 00:03:39 alanb Exp $";
#endif


#include <stdlib.h>
#include <string.h>
#include "netdata.h"
#include "list.h"

static List dataList;

InitData()
{
	if (!(dataList = ListCreate()))
		return(0);

	return(1);
}


Data *DataNew()
{
	Data *d;
	if (!(d = (Data *)calloc(1,sizeof(Data))))
	  {
		ErrMesg("Out of Memory\n");
	  }
	else
	  {
		d->label = d->associated = (char *) 0;
		d->group = (Data *) 0;
		d->magicPath = (VdataPathElement **) 0;
		d->nodeName = d->fields = (char *) 0;

		d->expandX = d->expandY = 1.0;
	  }
	
	return(d);
}

void DataDestroy(d)
Data *d;
{
	(void)ListDeleteEntry(dataList,d);
	if (d->label) 
		FREE(d->label);
	if (d->data) 
		FREE(d->data);
	FREE(d);
}

void DataAddEntry(d)
Data *d;
{
	ListAddEntry(dataList,d);
}


Data *DataSearchByLabel(s)
char *s;
{
Data *d;

	d = (Data *) ListHead(dataList);
	while (d) {
		if (d->label && (!strcmp(s,d->label))) {
			return(d);
			}
		d = (Data *) ListNext(dataList);
		}
	return( (Data *) 0);
}

Data *DataSearchByLabelAndDOT(s,dot)
char *s;
int dot;	/*data object type */
{
Data *d;

	d = (Data *) ListHead(dataList);
	while (d) {
		if ((d->label) && (!strcmp(s,d->label)) && (d->dot == dot)) {
			return(d);
			}
		d = (Data *) ListNext(dataList);
		}
	return( (Data *) 0);
}

Data *DataSearchByLabelAndDOTAndDOST(s,dot,dost)
char *s;
int dot;	/*data object type */
int dost;	/* data object sub type */
{
Data *d;

	d = (Data *) ListHead(dataList);
	while (d) {
		if ((d->label)&&((!strcmp(s,d->label)) && (d->dot == dot) 
				&& (d->dost == dost))) {
			return(d);
			}
		d = (Data *) ListNext(dataList);
		}
	return( (Data *) 0);
}

int DataInList(inList)
/* is this Data set in the list */
Data *inList;
{
Data *d;
        d = (Data *) ListHead(dataList);
        while (d) {
		if (d == inList) {
			return(1);
			}
        	d = (Data *) ListNext(dataList);
		}
	return(0);
}



