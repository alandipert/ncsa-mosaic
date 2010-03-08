/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright (C) 1993, Board of Trustees of the University of Illinois      *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/
#include "../config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mm.h"


/*#define MM_TEST /**/


#ifndef MM_TEST
#ifndef DISABLE_TRACE
extern int nutTrace;
#endif
#else
#ifndef DISABLE_TRACE
int nutTrace=1;
#endif
#endif


void freeBlock(mem_block *block) {

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"freeBlock: Called\n");
	}
#endif

	if (block) {
		if (block->memory) {
#ifndef DISABLE_TRACE
			if (nutTrace) {
				fprintf(stderr,"freeBlock: Freeing block->memory\n");
			}
#endif
			free(block->memory);
			block->memory=NULL;
		}
#ifndef DISABLE_TRACE
		if (nutTrace) {
			fprintf(stderr,"freeBlock: Freeing block\n");
		}
#endif
		free(block);
		block=NULL;
	}

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"freeBlock: Leaving\n");
	}
#endif

	return;
}


mem_block *allocateBlock(int type) {

mem_block *block;

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"allocateBlock: Called\n");
	}
#endif

	block=(mem_block *)calloc(1,sizeof(mem_block));
	if (!block) {
#ifndef DISABLE_TRACE
		if (nutTrace) {
			perror("allocateBlock_block");
		}
#endif

		return(NULL);
	}

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"allocateBlock: block(%d)\n",sizeof(block));
	}
#endif

	memset(block, 0, sizeof(mem_block));

	if (type>=MEM_MAX_ENTRY) {
#ifndef DISABLE_TRACE
		if (nutTrace) {
			fprintf(stderr,"allocateBlock_type: Invalid type\n");
		}
#endif

		return(NULL);
	}

	block->memoryType=type;
	block->sizeCnt=1;
	block->size=memSize[block->memoryType];
	block->fullSize=block->size*block->sizeCnt;

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"allocateBlock: block->memoryType(%d), block->sizeCnt(%d)\n",block->memoryType,block->sizeCnt);
		fprintf(stderr,"allocateBlock: block->size(%d), block->fullSize(%d)\n",block->size,block->fullSize);
	}
#endif

	block->memory=(char *)calloc((block->fullSize),sizeof(char));
	if (!block->memory) {
#ifndef DISABLE_TRACE
		if (nutTrace) {
			perror("allocateBlock_block->memory");
		}
#endif
		freeBlock(block);

		return(NULL);
	}

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"allocateBlock: block->memory(%d)\n",sizeof(block->memory));
	}
#endif

	block->nextFree=0;

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"allocateBlock: block->nextFree(%d)\n",block->nextFree);
		fprintf(stderr,"allocateBlock: Leaving\n");
	}
#endif

	return(block);
}


int reallocateBlock(mem_block *block) {

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"reallocateBlock: Called\n");
	}
#endif

	block->sizeCnt++;
	block->fullSize+=block->size;

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"reallocateBlock: block->sizeCnt(%d), block->fullSize(%d)\n",block->sizeCnt,block->fullSize);
	}
#endif

	block->memory=realloc(block->memory,(block->fullSize*sizeof(char)));
	if (!block->memory) {
#ifndef DISABLE_TRACE
		if (nutTrace) {
			perror("reallocateBlock_block->memory");
		}
#endif
		freeBlock(block);

		return(MEM_FAIL);
	}

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"reallocateBlock: block->memory(%d)\n",sizeof(block->memory));
		fprintf(stderr,"reallocateBlock: Leaving\n");
	}
#endif

	return(MEM_SUCCEED);
}


void clearBlock(mem_block *block) {

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"clearBlock: Called\n");
	}
#endif

	block->nextFree=0;

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"clearBlock: block->nextFree(%d)\n",block->nextFree);
		fprintf(stderr,"clearBlock: Leaving\n");
	}
#endif

	return;
}


void *balloc(mem_block *block, int size) {

void *ptr=NULL;

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"balloc: Called\n");
	}
#endif

	if (!block) {
#ifndef DISABLE_TRACE
		if (nutTrace) {
			fprintf(stderr,"balloc_block: Block not allocated\n");
		}
#endif

		return(NULL);
	}

	if (size<=0) {
		if (size==0) {
#ifndef DISABLE_TRACE
			if (nutTrace) {
				fprintf(stderr,"balloc_size: Size is zero\n");
			}
#endif

			return(NULL);
		}
#ifndef DISABLE_TRACE
		if (nutTrace) {
			fprintf(stderr,"balloc_size: Size is negative\n");
		}
#endif

		return(NULL);
	}

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"balloc: size(%d), (block->fullSize{%d}-block->nextFree{%d})(%d)\n",size,block->fullSize,block->nextFree,(block->fullSize-block->nextFree));
	}
#endif

	if (size>(block->fullSize-block->nextFree)) {
		/*need to reallocate*/
#ifndef DISABLE_TRACE
		if (nutTrace) {
			fprintf(stderr,"balloc: Need to reallocateBlock\n");
		}
#endif
		if (reallocateBlock(block)!=MEM_SUCCEED) {
			return(NULL);
		}
	}

	ptr = &(block->memory[block->nextFree]);
	block->nextFree+=(size+(MEM_ALIGN-(size%MEM_ALIGN)));

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"balloc: ptr(%d), block->nextFree(%d)\n",sizeof(ptr),block->nextFree);
		fprintf(stderr,"balloc: Leaving\n");
	}
#endif

	return(ptr);
}


int blockSize(mem_block *block) {

#ifndef DISABLE_TRACE
	if (nutTrace) {
		fprintf(stderr,"blockSize: Calling\n");
		fprintf(stderr,"blockSize: block->fullSize(%d)\n",block->fullSize);
		fprintf(stderr,"blockSize: Leaving\n");
	}
#endif

	return(block->fullSize);
}


#ifdef MM_TEST

int main() {

mem_block *b;
char *ptr1;
int *iptr;

	b=allocateBlock(MEM_LEX);

	ptr1=balloc(b,4095);
	ptr1=balloc(b,1);
	ptr1=balloc(b,1);
	iptr=balloc(b,sizeof(int));

	*iptr=50;

	sprintf(ptr1,"1234567890123456789 (%d)",*iptr);

	printf("[%s]\n",ptr1);

	exit(0);
}

#endif
