/*			Atoms: Names to numbers			HTAtom.c
**			=======================
**
**	Atoms are names which are given representative pointer values
**	so that they can be stored more efficiently, and comparisons
**	for equality done more efficiently.
**
**	Atoms are kept in a hash table consisting of an array of linked lists.
**
** Authors:
**	TBL	Tim Berners-Lee, WorldWideWeb project, CERN
**	(c) Copyright CERN 1991 - See Copyright.html
**
*/
#include "../config.h"
#define HASH_SIZE	101		/* Tunable */
#include "HTAtom.h"

#include <stdio.h>				/* joe@athena, TBL 921019 */
#include "HTUtils.h"
#include "tcp.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

PRIVATE HTAtom * hash_table[HASH_SIZE];
PRIVATE BOOL initialised = NO;

#ifdef __STDC__
PUBLIC HTAtom * HTAtom_for(char * string)
#else
PUBLIC HTAtom * HTAtom_for(string)
    char * string;
#endif
{
    int hash;
    WWW_CONST char * p;
    HTAtom * a;

    /* Bug hack. */
    if (!string || !*string)
      string = strdup ("blargh");
    
    /*		First time around, clear hash table
    */
    if (!initialised) {
        int i;
	for (i=0; i<HASH_SIZE; i++)
	    hash_table[i] = (HTAtom *) 0;
	initialised = YES;
    }
    
    /*		Generate hash function
    */
    for(p=string, hash=0; *p; p++) {
        hash = (hash * 3 + *p) % HASH_SIZE;
    }
    
    /*		Search for the string in the list
    */
    for (a=hash_table[hash]; a; a=a->next) {
	if (0==strcmp(a->name, string)) {
#ifndef DISABLE_TRACE
    	    if (www2Trace) fprintf(stderr,
	    	"HTAtom: Old atom %p for `%s'\n", a, string);
#endif
	    return a;				/* Found: return it */
	}
    }
    
    /*		Generate a new entry
    */
    a = (HTAtom *)malloc(sizeof(*a));
    if (a == NULL) outofmem(__FILE__, "HTAtom_for");
    a->name = (char *)malloc(strlen(string)+1);
    if (a->name == NULL) outofmem(__FILE__, "HTAtom_for");
    strcpy(a->name, string);
    a->next = hash_table[hash];		/* Put onto the head of list */
    hash_table[hash] = a;
#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr, "HTAtom: New atom %p for `%s'\n", a, string);
#endif
    return a;
}


#ifdef __STDC__
PUBLIC HTAtom * HTAtom_exists(char * string)
#else
PUBLIC HTAtom * HTAtom_exists(string)
    char * string;
#endif
{
    int hash;
    WWW_CONST char * p;
    HTAtom * a;
    
    if (!initialised) {
      return NULL;
    }
    
    /*		Generate hash function
    */
    for(p=string, hash=0; *p; p++) {
        hash = (hash * 3 + *p) % HASH_SIZE;
    }
    
    /*		Search for the string in the list
    */
    for (a=hash_table[hash]; a; a=a->next) {
	if (0==strcmp(a->name, string)) {
	    return a;				/* Found: return it */
	}
    }
    
    return NULL;
}
