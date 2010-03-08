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


#include <stdio.h>

WriteMesg(s)
char *s;
{
	printf("%s",s);
	fflush(stdout);
}

WarningMesg(s)
char *s;
{
	fprintf(stderr,"%s",s);
}

ErrMesg(s)
char *s;
{
	fprintf(stderr,"%s",s);
}
