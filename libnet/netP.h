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
/* $Id: netP.h,v 1.1.1.1 1995/01/11 00:03:39 alanb Exp $ */


#include "net.h"

/****************************/

typedef struct {
	float load1;
	float load5;
	float load15;
	int   numUsers;
	} ExecHostStatusReturn;

typedef union {
	ExecHostStatusReturn	hsReturn; 
	} ExecRetInfo;

typedef struct {
	char *id;
	char *retAddress;
	char *authentication;
	char *timeStamp;
	int  type;
	ExecRetInfo info;  /* addition info depending on type */
	} Exec;
/****************************/

static void	NetFreeDataCB PARAMS((GenericPtr data, caddr_t cbData));
