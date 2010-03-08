/*  */

/*              List object
**
**      The list object is a generic container for storing collections
**      of things in order.
*/
#ifndef HTLIST_H
#define HTLIST_H

#include "HTUtils.h"  /* for BOOL type and PARAMS and ARGS*/

typedef struct _HTList HTList;

struct _HTList {
  void * object;
  HTList * next;
  HTList * last;
};

#ifdef SHORT_NAMES
#define HTList_new                      HTLiNew
#define HTList_delete                   HTLiDele
#define HTList_addObject                HTLiAdOb
#define HTList_removeObject             HTLiReOb
#define HTList_removeLastObject         HTLiReLa
#define HTList_removeFirstObject        HTLiReFi
#define HTList_count                    HTLiCoun
#define HTList_indexOf                  HTLiInOf
#define HTList_objectAt                 HTLiObAt
#endif

extern HTList * HTList_new NOPARAMS;
extern void     HTList_delete PARAMS((HTList *me));

/*      Add object to START of list
*/
extern void     HTList_addObject PARAMS((HTList *me, void *newObject));
extern void     HTList_addObjectAtEnd PARAMS((HTList *me, void *newObject));


extern BOOL     HTList_removeObject PARAMS((HTList *me, void *oldObject));
extern void *   HTList_removeLastObject PARAMS((HTList *me));
extern void *   HTList_removeFirstObject PARAMS((HTList *me));
#define         HTList_isEmpty(me) (me ? me->next == NULL : YES)
extern int      HTList_count PARAMS((HTList *me));
extern int      HTList_indexOf PARAMS((HTList *me, void *object));
#define         HTList_lastObject(me) \
  (me && me->next ? me->next->object : NULL)
extern void *   HTList_objectAt PARAMS((HTList *me, int position));

/* Fast macro to traverse the list. Call it first with copy of list header :
   it returns the first object and increments the passed list pointer.
   Call it with the same variable until it returns NULL. */
#define HTList_nextObject(me) \
  (me && (me = me->next) ? me->object : NULL)

#endif /* HTLIST_H */
/*

    */
