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
/* $Id: doodle.h,v 1.1.1.1 1995/01/11 00:03:37 alanb Exp $ */

#ifndef HAS_DOODLE_DOT_H_BEEN_INCLUDED_BEFORE
#define HAS_DOODLE_DOT_H_BEEN_INCLUDED_BEFORE

#define	MAXDRAWDOODLE	10000

typedef struct {
        short x,y;
        } POINT;

typedef struct {
	POINT *doodle;
	int length;
	} charRec;

typedef struct DoodleColor {
        short red, green, blue;
        } DColor;
#endif /* ! HAS_DOODLE_DOT_H_BEEN_INCLUDED_BEFORE */
