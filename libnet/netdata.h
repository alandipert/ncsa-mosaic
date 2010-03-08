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
/* $Id: netdata.h,v 1.1.1.1 1995/01/11 00:03:40 alanb Exp $ */


#ifndef HAS_NET_DATA_DOT_H_BEEN_INCLUDED_BEFORE
#define HAS_NET_DATA_DOT_H_BEEN_INCLUDED_BEFORE
#include <libdtm/dtm.h>
#include <libdtm/vdata.h>
#include "collage.h"

/* Entity type */
#define ENT_Internal	1
#define ENT_File	2
#define ENT_Network	3
#define ENT_Interactive 4

/* Data Object Type */
#define DOT_Unknown	100
#define DOT_VData	101
#define DOT_Array	102
#define DOT_Text	103
#define DOT_Image	104
#define DOT_Palette8	105
#define DOT_SDL		106

/* Data Object SubType */
#define DOST_Float	200
#define DOST_Char	201
#define DOST_Int16	202
#define DOST_Int32	203
#define DOST_Double	204


#define anim_axis	view_type

#define MAX_ARRAY_DIM	50

/*
defined in vdata.h
typedef struct {
	int nodeID;
	char *nodeName;
	} VdataPathElement;
*/

typedef struct DATA {
	char *label;				/* data object label*/
	int entity;				/* entity type */
	int dot;				/* Data Object Type */
	int dost;				/* Data Object Subtype */
	int dim[MAX_ARRAY_DIM];			/* array of dimensions */
	int rank;				/* number of dimensions */
	GenericPtr data;			/* data */
	IntOrFloat min;
	IntOrFloat max;
	int view_type;

	VdataPathElement **magicPath;		/* Vdata path */
	int  pathLength;			/* do we want this Marc? */
	int  nodeID;				/* this Vdata's ID */
	char *nodeName;				/* this Vdata's name */
	char *fields;				/* ? */

	float	expandX;			/* expand X image */
	float	expandY;			/* expand Y image */
	char *associated;			/* associated data */
	struct DATA *group;			/* group with any */
	} Data;

extern Data *DataNew PARAMS((void));
extern void DataDestroy PARAMS((Data *d));
extern int InitData PARAMS((void));
extern void DataAddEntry PARAMS((Data *d));
extern Data *DataSearchByLabel PARAMS((char *s));
extern Data *DataSearchByLabelAndDOT PARAMS((char *s, int dot));
extern Data *DataSearchByLabelAndDOTAndDOST
		PARAMS((char *s, int dot, int dost));
extern int DataInList PARAMS((Data *d));

#endif 
