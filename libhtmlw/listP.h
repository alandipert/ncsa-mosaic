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


#ifndef ___HAS_LIST_STUFF_BEEN_INCLUDED_BEFORE___
#define ___HAS_LIST_STUFF_BEEN_INCLUDED_BEFORE___

#include <stdlib.h>
#include "list.h"

struct LISTINSTANCE {
	char *value;
	struct LISTINSTANCE *next;
	struct LISTINSTANCE *prev;
	};

struct LISTSTRUCT {
	struct LISTINSTANCE  *head;
	struct LISTINSTANCE  *tail;
	struct LISTINSTANCE  *current;  
	int listCount;
	};

#endif
