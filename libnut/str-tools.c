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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef VMS   /* PGE */
#include <ctype.h>
#endif

#ifndef DEBUG

/* Use builtin strdup when appropriate -- code duplicated in tcp.h. */
#if defined(ultrix) || defined(VMS) || defined(NeXT)
extern char *strdup(char *str);
#endif

#else

#include "str-tools.h"

/* Copied from mo-www.c PGE */
#if defined(ultrix) || defined(VMS) || defined(NeXT) || defined(M4310) || defined(vax)
char *strdup(char *str) {
  char *dup;

  if(!str)
    return NULL;

  dup = (char *)malloc(strlen (str) + 1);
  if(!dup)
    return NULL;

  dup = strcpy(dup, str);

  return dup;
}
#endif

int main() {

char *bob,*newstr;

	bob=strdup("This is test %d.");
	newstr=strstrdup(bob,"%d","1");

	printf("bob[%s]\nnew[%s]\n\n",bob,newstr);

	free(bob);
	free(newstr);

	bob=strdup("%d) This is test %d.");
	newstr=strstrdup(bob,"%d","2");

	printf("bob[%s]\nnew[%s]\n\n",bob,newstr);

	free(bob);
	free(newstr);

	bob=strdup("This is test %d.");
	newstr=strstrdup(bob,"%d","003");

	printf("bob[%s]\nnew[%s]\n\n",bob,newstr);

	bob=strdup("%d) This is test %d.");
	newstr=strstrdup(bob,"%d","004");

	printf("bob[%s]\nnew[%s]\n\n",bob,newstr);

	bob=strdup("qwerty");
	printf("src[%s]\n",bob);
	newstr=my_chop (bob);
	printf("chopped[%s]\n\n",newstr);

	bob=strdup("qwerty  ");
	printf("src[%s]\n",bob);
	newstr=my_chop (bob);
	printf("chopped[%s]\n\n",newstr);

	bob=strdup("  qwerty  ");
	printf("src[%s]\n",bob);
	newstr=my_chop (bob);
	printf("chopped[%s]\n\n",newstr);

	exit(0);
}
#endif


char *getFileName(char *file_src) {

char *ptr;

	if (!file_src || !*file_src) {
		return(NULL);
	}

	ptr=strrchr(file_src,'/');

	if (!ptr || !*ptr) {
		return(file_src);
	}

	if (*ptr=='/' && *(ptr+1)) {
		ptr++;
	}

#ifdef VMS /* GEC */
        /* I don't understand this, it can never happen. PGE */
        if (*ptr=='/') {
                return("\0");
	}
#endif

	return(ptr);
}


/*
 * Will casefully search forward through a string for a character.
 *
 * Must be a null-terminated string.
 *
 * SWP
 */
char *strcasechr(char *src, char srch) {

char *ptr=NULL;
char tmp;

	if (!src || !*src) {
		return(NULL);
	}

	tmp=toupper(srch);

	for (ptr=src; (*ptr && toupper(*ptr)!=tmp); ptr++);

	/*
	 * At this point, either *ptr == \0 (failure) or toupper(*ptr) is
	 *   == to tmp (success). Return accordingly.
	 */
	if (*ptr) {
		return(ptr);
	}

	return(NULL);
}


/*
 * Will casefully search backward through a string for a character.
 *
 * Must be a null-terminated string.
 *
 * SWP
 */
char *strrcasechr(char *src, char srch) {

char *ptr=NULL;
char tmp;

	if (!src || !*src) {
		return(NULL);
	}

	tmp=toupper(srch);

	for (ptr=(src+strlen(src)-1); (ptr>src && toupper(*ptr)!=tmp); ptr--);

	/*
	 * At this point we have either found toupper(*ptr) == to tmp, or we
	 *   are at the very begining of the string. So, if ptr is != to src,
	 *   we found a match...or...we need to test to make sure the first
	 *   char in the string is not the match. Return accordingly.
	 */
	if (ptr!=src || toupper(*ptr)==tmp) {
		return(ptr);
	}

	return(NULL);
}


char *strstrdup(char *src, char *srch, char *rplc) {

char *dest=NULL,*local=NULL,*start=NULL,*found=NULL,*next=NULL;
int rplcLen=0,i,srchLen;

	if (!src || !*src || !srch || !*srch) {
		return(NULL);
	}

	if (rplc && *rplc) {
		rplcLen=strlen(rplc);
	}
	srchLen=strlen(srch);

	if (rplcLen>srchLen) {
		dest=(char *)calloc(1,sizeof(char));
	}
	else {
		dest=strdup(src);
	}
	*dest='\0';

	local=strdup(src);
	start=local;
	while (*start) {
		if (!(found=strstr(start,srch))) {
			if (rplcLen>srchLen) {
				realloc((void *)dest,((strlen(dest)+strlen(start)+4)*sizeof(char)));
				strcat(dest,start);
			}
			else {
				strcat(dest,start);
			}
			free(local);

			return(dest);
		}

		for (i=0,next=found; i<srchLen; i++,next++);
		*found='\0';
		if (rplcLen>srchLen) {
			realloc((void *)dest,((rplcLen+strlen(dest)+strlen(start)+4)*sizeof(char)));
			strcat(dest,start);
			if (rplcLen) {
				strcat(dest,rplc);
			}
		}
		else {
			strcat(dest,start);
			strcat(dest,rplc);
		}
		start=next;
	}

	return(dest);
}


char **string_to_token_array(char *str, char *delimiter)
{
  char **array, *tmp;
  int num=0, i=0;

  if(!str || !*str || !delimiter || !*delimiter)
    return NULL;

  /* first get number of tokens */
  tmp = strstr(str, delimiter);
  num++; tmp++;
  while((tmp = strstr(tmp, delimiter)) != NULL)
    {
      tmp++; num++;
    }

  array = malloc(sizeof(char *) * (num+2));
  array[0] = strdup(strtok(str, delimiter));

  i++;
  while((array[i++] = strdup(strtok((char *) NULL, delimiter))) != NULL);


  free(str);
  return array;
}

char *my_strndup(char *str, int num)
{
  char *nstr = NULL;

  if(!str || !*str)
    return NULL;

  nstr = malloc(sizeof(char) * (num + 1));

  strncpy(nstr, str, num);
  nstr[num] = '\0'; /* shouldn't strcpy do this ?? */
  return nstr;
}

char *my_chop(char *str)
{
  char *ptr;

  if(!str || !*str)
    return str;

/*
  while(isspace(str[0]))
    str++;

  while(isspace(str[strlen(str)-1]) || isspace(str[strlen(str)-1]))
    {
      str[strlen(str)-1] = '\0';
    }
*/

  /* Remove blank space from end of string. */
  ptr = str + strlen(str) - 1;
  while((ptr >= str) && isspace(*ptr))
    {
      *ptr = '\0';
      ptr--;
    }

  /* Remove blank space from start of string. */
  ptr = str;
  while(isspace(ptr[0]))
    ptr++;

  /*
  ** If there was blank space at start of string then move string back to the
  ** beginning. This prevents memory freeing problems later if pointer is
  ** moved. memmove is used because it is safe for overlapping regions.
  */
  if (ptr != str)
    memmove (str, ptr, strlen (ptr) + 1);

  return str;
}

int my_strcasecmp(char *str1, char *str2)
{
  int i, min, offset1, offset2;

  if(!str1 || !str2 || !*str1 || !*str2)
    return 1;

  /* find shortest to string to make sure we don't go past null */
  min = strlen(str1);
  if(strlen(str2) < min)
    min = strlen(str2);

  for(i=0;i<min;i++)
    {
      /* use offsets to make everything lower case */

      if(str1[i]>='A' && str1[i]<='Z')
	offset1=32;
      else
	offset1=0;

      if(str2[i]>='A' && str2[i]<='Z')
	offset2=32;
      else
	offset2=0;

      if(str1[i]+offset1<str2[i]+offset2)
	return -1;
      if(str1[i]+offset1>str2[i]+offset2)
	return 1;
      /*
      if (toupper(str1[i])<toupper(str2[i])) {
        return(-1);
      }
      if (toupper(str1[i])>toupper(str2[i])) {
        return(1);
      }*/
    }
  /* they're equal */
  return 0;
}

int my_strncasecmp(char *str1, char *str2, int n)
{
  int i, min, offset1, offset2;

  if(!str1 || !str2 || !*str1 || !*str2 || !n)
    return 1;

  min = n;

  if(strlen(str1) < min)
    min = strlen(str1);

  if(strlen(str2) < min)
    min = strlen(str2);

  for(i=0;i<min;i++)
    {
      if(str1[i]>='A' && str1[i]<='Z')
	offset1=32;
      else
	offset1=0;

      if(str2[i]>='A' && str2[i]<='Z')
	offset2=32;
      else
	offset2=0;

      if(str1[i]+offset1<str2[i]+offset2)
	return -1;
      if(str1[i]+offset1>str2[i]+offset2)
	return 1;
/* 
      if (toupper(str1[i])<toupper(str2[i])) {
        return(-1);
      }
      if (toupper(str1[i])>toupper(str2[i])) {
        return(1);
      }*/
    }

  return 0;
}
