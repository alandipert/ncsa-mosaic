/*	A small List class					      HTList.c
**	==================
**
**	A list is represented as a sequence of linked nodes of type HTList.
**	The first node is a header which contains no object.
**	New nodes are inserted between the header and the rest of the list.
*/
#include "../config.h"
#include "HTList.h"

#include <stdio.h>				/* joe@athena, TBL 921019 */

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

HTList * HTList_new NOARGS
{
  HTList *newList = (HTList *)malloc (sizeof (HTList));
  if (newList == NULL) outofmem(__FILE__, "HTList_new");
  newList->object = NULL;
  newList->next = NULL;
  return newList;
}

void HTList_delete ARGS1(HTList *,me)
{
  HTList *current;
  while (current = me) {
    me = me->next;
    free (current);
  }
}

void HTList_addObject ARGS2(HTList *,me, void *,newObject)
{
  if (me) {
    HTList *newNode = (HTList *)malloc (sizeof (HTList));
    if (newNode == NULL) outofmem(__FILE__, "HTList_addObject");
    newNode->object = newObject;
    newNode->next = me->next;
    me->next = newNode;
  }
#ifndef DISABLE_TRACE
  else
    if (www2Trace) fprintf(stderr,
        "HTList: Trying to add object %p to a nonexisting list\n",
		       newObject);
#endif
}

void HTList_addObjectAtEnd ARGS2(HTList *,me, void *,newObject)
{
  if (me) 
    {
      HTList *newNode = (HTList *)malloc (sizeof (HTList));
      if (newNode == NULL) outofmem(__FILE__, "HTList_addObject");
      newNode->object = newObject;
      newNode->next = NULL;
      while (me->next) 
        me = me->next;
      me->next = newNode;
    }
#ifndef DISABLE_TRACE
  else
    if (www2Trace) fprintf(stderr,
                       "HTList: Trying to add object %p to a nonexisting list\n",
		       newObject);
#endif
}

BOOL HTList_removeObject ARGS2(HTList *,me, void *,oldObject)
{
  if (me) {
    HTList *previous;
    while (me->next) {
      previous = me;
      me = me->next;
      if (me->object == oldObject) {
	previous->next = me->next;
	free (me);
	return YES;  /* Success */
      }
    }
  }
  return NO;  /* object not found or NULL list */
}

void * HTList_removeLastObject ARGS1 (HTList *,me)
{
  if (me && me->next) {
    HTList *lastNode = me->next;
    void * lastObject = lastNode->object;
    me->next = lastNode->next;
    free (lastNode);
    return lastObject;
  } else  /* Empty list */
    return NULL;
}

void * HTList_removeFirstObject ARGS1 (HTList *,me)
{
  if (me && me->next) {
    HTList * prevNode;
    void *firstObject;
    while (me->next) {
      prevNode = me;
      me = me->next;
    }
    firstObject = me->object;
    prevNode->next = NULL;
    free (me);
    return firstObject;
  } else  /* Empty list */
    return NULL;
}

int HTList_count ARGS1 (HTList *,me)
{
  int count = 0;
  if (me)
    while (me = me->next)
      count++;
  return count;
}

int HTList_indexOf ARGS2(HTList *,me, void *,object)
{
  if (me) {
    int position = 0;
    while (me = me->next) {
      if (me->object == object)
	return position;
      position++;
    }
  }
  return -1;  /* Object not in the list */
}

void * HTList_objectAt ARGS2 (HTList *,me, int,position)
{
  if (position < 0)
    return NULL;
  if (me) {
    while (me = me->next) {
      if (position == 0)
	return me->object;
      position--;
    }
  }
  return NULL;  /* Reached the end of the list */
}
