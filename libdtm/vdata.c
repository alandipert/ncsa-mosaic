#include <stdio.h>
#include "dtm.h"
#include "vdata.h"

#ifndef MALLOC
#define MALLOC(x)	malloc(x)
#define FREE(x)		free(x)
#endif

int VDATAsetPath(header,magicPath,pathLength)
char *header;
VdataPathElement **magicPath;
int pathLength;
{
int x;
char pathString[DTM_MAX_HEADER];
char idString[DTM_MAX_HEADER];
char buff[80];

	pathString[0]='\0';
	for (x=0; x < pathLength; x++) {
		strcat(pathString,magicPath[x]->nodeName);
		strcat(pathString," ");
		}

	idString[0]='\0';
	for (x=0; x < pathLength; x++) {
		sprintf(buff,"%d ",magicPath[x]->nodeID);
		strcat(idString,buff);
		}
	dtm_set_int(header,VDATApathLength,pathLength);
	dtm_set_char(header,VDATApathName,pathString);
	dtm_set_char(header,VDATApathID,idString);
	return(1);
}


int VDATAgetPath(header,magicPath,pathLength)
char *header;
VdataPathElement **magicPath;
int *pathLength;
{
char pathString[DTM_MAX_HEADER];
char pathID[DTM_MAX_HEADER];
int numPath;
int integer[10];
char pathStep[DTM_MAX_HEADER];
char *ptr;
int x;

	dtm_get_int(header,VDATApathLength,&numPath);
	dtm_get_char(header,VDATApathName,pathString,DTM_MAX_HEADER);
	dtm_get_char(header,VDATApathID,pathID,DTM_MAX_HEADER);

	numPath = (numPath > *pathLength)? (*pathLength) : numPath;
	*pathLength = numPath;

	ptr = pathID;
	for (x = 0; x < numPath; x++) {
		sscanf(ptr,"%s ",integer);
		ptr += (strlen(ptr) + 1);
		magicPath[x]->nodeID = atoi(integer);
		}

	ptr = pathString;
	for (x = 0; x < numPath; x++) {
		sscanf(ptr,"%s ",pathStep);
		ptr += (strlen(ptr) + 1);
		if (!(magicPath[x]->nodeName = (char *)
				MALLOC( strlen(pathStep) + 1))) {
			fprintf(stderr,"VDATAgetPath: Out of Memory\n");
			return(-1);
			}
		strcpy(magicPath[x]->nodeName,pathStep);
		}
	return(1);
}


