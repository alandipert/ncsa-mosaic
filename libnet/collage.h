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
/* $Id: collage.h,v 1.1.1.1 1995/01/11 00:03:37 alanb Exp $ */

#ifndef HAS_COLLAGE_H_BEEN_INCLUDED_BEFORE
#define HAS_COLLAGE_H_BEEN_INCLUDED_BEFORE

#include <stdlib.h>

#if defined  __STDC__
#ifndef FUNCPROTO
#define FUNCPROTO
#endif
#define CONST		const
#define VOLATILE	volatile
#define SIGNED		signed
#else /* ! defined __STDC__ */
#define CONST
#define VOLATILE
#define SIGNED
#endif /* defined  __STDC__ */

#ifdef FUNCPROTO
#define AND		,
#define PARAMS(paramlist)		paramlist
#define DEFUN(name, arglist, args)	name(args)
#define DEFUN_VOID(name)		name(void)
#else
#define AND		;
#define PARAMS(paramlist)		()
#define DEFUN(name, arglist, args)	name arglist args;
#define DEFUN_VOID(name)		name()
#endif /* FUNCPROTO */

#ifndef MALLOC
#if defined(SUN) && defined(LINT)
union { char *c; void *vp } mal_r_ptr;
#  define MALLOC(SIZE) (mal_r_ptr.c = malloc(SIZE), mal_r_ptr.vp)
#else
#  define MALLOC malloc
#endif
#define FREE free
#endif /* ! MALLOC */

typedef union _i_or_f_ {
	int i;
	float f;
} IntOrFloat;

#ifdef NOVOIDPTR
typedef char *GenericPtr;
#else
typedef void *GenericPtr;
#endif

#endif /* ! HAS_COLLAGE_H_BEEN_INCLUDED_BEFORE */
