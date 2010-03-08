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


typedef struct LISTSTRUCT  *List;

extern List ListCreate();
extern void ListDestroy();
extern int  ListAddEntry();
extern int  ListDeleteEntry();
extern int  ListMakeEntryCurrent();
extern int  ListCount();

extern char *ListHead();
extern char *ListTail();
extern char *ListCurrent();
extern char *ListNext();
extern char *ListPrev();
extern char *ListGetIndexedEntry();
