#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
#define ELLIPSIS_TEST
*/

#ifdef ELLIPSIS_TEST
void usage_statement(void);
#endif
int compact_string(char *main_string, char *ellipsis_string, 
		    int num_chars, int mode, int eLength);

#ifdef ELLIPSIS_TEST
int main(int argc, char *argv[]) {

    char *main_string;
    char *ellipsis_string;
    int num_chars;
    int mode;
    int result;
    int ellipsisLength=3;

    int i,j;

        /* Check number of args */
    if(argc != 4) {
        usage_statement();
        return(1);
    }

        /* get args */
    main_string = argv[1];
    num_chars = atoi(argv[2]);
    mode = atoi(argv[3]);

        /* allocate ellipsis_string */
    ellipsis_string = (char *)calloc(num_chars,1);

    result = compact_string(main_string, ellipsis_string, num_chars, mode, ellipsisLength);

    if(result == 1) {
	printf("The original string is:\n");
	printf("\t|%s|\n\n", main_string);
	printf("And the compacted string is:\n");
	printf("\t|%s|\n\n", ellipsis_string);
    }
    else
	printf("you screwed something up...\n");

}
#endif


int compact_string(char *main_string, char *ellipsis_string, 
		    int num_chars, int mode, int eLength) {

    int string_len;
    int feem, puff, i;

    num_chars--;

    string_len = strlen(main_string);
    if(string_len <= num_chars) {
	strcpy(ellipsis_string, main_string);
	return(1);
    }

    switch(mode) {

    case 1: {
	
	puff = num_chars - eLength;
	feem = string_len - puff;

	strcpy(ellipsis_string,".");
/*
	i=1;
	while (i<eLength) {
		strcat(ellipsis_string, ".");
		i++;
	}
*/

	for (i = 0; i < eLength; i++) {
		ellipsis_string [i] = '.';
	}
	ellipsis_string [i] = '\0';

	strncat(ellipsis_string, main_string + feem, puff);

	break;

    }

    case 2: {

	int right_side, left_side;

/*
	puff = num_chars - eLength;
	right_side = puff >> 1;
	left_side = puff - right_side;

	strncpy(ellipsis_string, main_string, right_side);
	ellipsis_string[right_side]='\0';
	i=0;
	while (i<eLength) {
		strcat(ellipsis_string, ".");
		i++;
	}
	strncat(ellipsis_string, main_string+(string_len-left_side), left_side);
*/

	puff = num_chars - eLength;
	left_side = puff >> 1;
	right_side = puff - left_side;

	strncpy (ellipsis_string, main_string, left_side);
	for (i = left_side; i < num_chars-right_side; i++) {
		ellipsis_string [i] = '.';
	}
	ellipsis_string [num_chars - right_side] = '\0';
	strncat (ellipsis_string,
		 main_string + (string_len - right_side),
		 right_side);

	break;

    }
	
    case 3: {
	
	puff = num_chars - eLength;
	feem = string_len - puff;

	strncpy(ellipsis_string, main_string, puff);
/*
	ellipsis_string[puff]='\0';
	i=0;
	while (i<eLength) {
		strcat(ellipsis_string, ".");
		i++;
	}
*/

	for (i = puff; i < num_chars; i++) {
		ellipsis_string [i] = '.';
	}

	break;
	
    }
	
    default: {
#ifdef ELLIPSIS_TEST
	usage_statement();
	exit(0);
#else
	return(-1);
#endif
    }
	
    }

    ellipsis_string[num_chars]='\0';

    return(1);
}


#ifdef ELLIPSIS_TEST
void usage_statement(void) {

    printf("Usage: main <initial_string> num_chars mode; where mode is:\n");
    printf("       1=cut off start, 2=cut off middle, 3=cut off end\n");

}
#endif
