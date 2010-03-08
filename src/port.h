#ifndef PORT_DOT_H_BEEN_INCLUDED_BEFORE

#define PORTBUFFERSIZE 1024

typedef struct {
	int socketFD;
	char *serverAddress;
	int serverTCPPort;
	int connected;

	int numInBuffer;
	char buffer[PORTBUFFERSIZE *2 +2];

	/* callBack(void *callBackData) */
	void (*callBack) (); /* called when connection dropped*/
	void *callBackData;
	} PortDescriptor;

#define PORT_DOT_H_BEEN_INCLUDED_BEFORE
#endif
