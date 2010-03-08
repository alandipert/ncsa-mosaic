#ifndef __CCISERVER_H__
#define __CCISERVER_H__

#include "cci.h"

extern int MCCIServerInitialize();
extern MCCIPort MCCICheckAndAcceptConnection();
extern int MCCISendResponseLine();
extern int MCCIIsThereInput();
extern int MCCIReadInputMessage();

typedef struct {
	MCCIPort client;
	int status;
	char *url;
} cciStat;			/* ejb 03/09/95 */

#endif
