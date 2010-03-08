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


/*
 * list.c:  This module contains list manipulation routines that cunstruct
 * and maintain a linked list of data items.  The record at the head of each
 * list contains pointers to the head, tail, and current list position.
 * the list itsself is doubly linked with both next and previous pointers.
 */
#include <stdio.h>
#include "listP.h"

#ifndef MALLOC
#define MALLOC  malloc
#define FREE    free
#endif

#define NIL	0

static void ListPrintErr(s)
char *s;
{
	fprintf(stderr,"%s",s);
}

/*
 * This function returns the data located at the head of the linked list,
 * or NIL if the list is empty.  As a side effect current is also set to
 * the head of the list.
 */
char *ListHead(theList)
List theList;
{
	if (!theList)
		return(NIL);
	theList->current = theList->head;
	if (theList->head)
		return(theList->head->value);
	else
		return(NIL);
}

/*
 * This function returns the data located at the tail of the linked list,
 * or NIL if the list is empty.  As a side effect current is also set to
 * the tail of the list.
 */
char *ListTail(theList)
List theList;
{
	if (!theList)
		return(NIL);
	theList->current = theList->tail;
	if (theList->tail)
		return(theList->tail->value);
	else
		return(NIL);
}

/*
 * This function returns the data located at the current position in the
 * linked list, or NIL if the list is empty.
 */
char *ListCurrent(theList)
List theList;
{
	if (!theList)
		return(NIL);
	if (theList->current)
		return(theList->current->value);
	else
		return(NIL);
}

/*
 * This function returns the data located at the next element of the linked
 * list after the current position, or NIL if the list is empty, or you
 * are at its end.
 * As a side effect current is also set to the next entry in the list.
 */
char *ListNext(theList)
List theList;
{
	if (!theList)
		return(NIL);
	if (theList->current) {
		theList->current = theList->current->next;
		return(ListCurrent(theList));
		}
	else
		return(NIL);
}

/*
 * This function returns the data located at the previous element of the linked
 * list before the current position, or NIL if the list is empty.
 * As a side effect current is also set to the previous entry in the list.
 */
char *ListPrev(theList)
List theList;
{
	if (!theList)
		return(NIL);
	if (theList->current) {
		theList->current = theList->current->prev;
		return(ListCurrent(theList));
		}
	else
		return(NIL);
}

/*
 * Cretae a list head and initialize it to NIL.
 */
List ListCreate()
{
List retVal;

	retVal = (List) MALLOC(sizeof(struct LISTSTRUCT));
	retVal->head = NIL;
	retVal->tail = NIL;
	retVal->current = NIL;
	return(retVal);
}

/*
 * Destroy a list head, and free all associated memory.
 */
void ListDestroy(theList)
List theList;
{
struct LISTINSTANCE *l;
struct LISTINSTANCE *m;

	if (!theList)
		return;
	l = theList->head;
	while(l) {
		m = l;
		l = l->next;
		FREE(m);
		}
	FREE(theList);
		
}


/*
 * Add an entry to the end of the linked list.  Current is changed to point to
 * the added element.
 */
int ListAddEntry(theList,v)
/* return 0 on failure */
List theList;
char *v; /* data to be added */
{
struct LISTINSTANCE *l;

	if (!(l =(struct LISTINSTANCE *) MALLOC(sizeof(struct LISTINSTANCE)))){
		ListPrintErr("Out of Memory\n");
		return(0);
		}

	l->value = v;

	l->next = NIL;
	l->prev = NIL;

	if (theList->head == NIL)
		theList->tail = theList->head = l;
	else {
		theList->tail->next = l;
		l->prev = theList->tail;
		theList->tail = l;
		}

	theList->current = l;
	
	return(1);
}

/*
 * Search the list for an entry with a matching value field, and return
 * a pointer to that list element.  Current is changed to point to the
 * element returned.
 */
static struct LISTINSTANCE *SearchListByValue(theList,v)
List theList;
char *v;
{
struct LISTINSTANCE *l;

	l = theList->head;
	while (l != NIL) {
		if (l->value == v) {
			theList->current = l;
			return(l);
			}
		else {
			l = l->next;
			}
		}
	theList->current = l;

	return(NIL);
	
}

/*
 * Find the list entry with a matching value field, and delete it
 * from the list.  Set current to point to the element after the deleted
 * element in the list.
 */
int ListDeleteEntry(theList,v)
/* removes the first occurance of v from the list */
/* return 0 if value not in list else 1 */
List theList;
char *v;
{
struct LISTINSTANCE *l;
char *retV;

	if (!(l = SearchListByValue(theList,v)))
		return(0);

	if (l->prev)
		l->prev->next = l->next;
	else
		theList->head = l->next;

	if (l->next)
		l->next->prev = l->prev;
	else
		theList->tail = l->prev;

	theList->current = l->next;

	retV = l->value;
	FREE(l);

	return(1);
}


int ListMakeEntryCurrent(theList,entry)
/* return 0 on failure  */
List theList;
char *entry;
{
struct LISTINSTANCE *l;

	if (theList) {
		if (!(l = SearchListByValue(theList,entry)))
			return(0);
		theList->current = l;
		return(1);
		}
	return(0);

}
