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
#ifndef VMS
#include <sys/time.h>
struct timeval Tv;
struct timezone Tz;
#else
#include <time.h>
#endif

#include <stdio.h>
#include <ctype.h>
#ifndef sun
/* To get atoi. */
#include <stdlib.h>
#endif
#include "HTML.h"
#include "HTMLamp.h"


extern void FreeObjList();
extern struct mark_up *AddObj();

int NoBodyColors(Widget w);
int NoBodyImages(Widget w);

char *ParseMarkTag();

extern int tableSupportEnabled;

#ifndef DISABLE_TRACE
extern int htmlwTrace;
#endif

#ifdef NOT_ASCII
#define TOLOWER(x)	(tolower(x))
#else

/*
 * A hack to speed up caseless_equal.  Thanks to Quincey Koziol for
 * developing it for me
 */
unsigned char map_table[256]={
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
    24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
    45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,97,98,
    99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,
    116,117,118,119,120,121,122,91,92,93,94,95,96,97,98,99,100,101,102,
    103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,
    120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,
    137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,
    154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,
    171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,
    188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,
    205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,
    222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,
    239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};

#define TOLOWER(x)	(map_table[x])
#endif /* NOT_ASCII */


/*
 * Check if two strings are equal, ignoring case.
 * The strings must be of the same length to be equal.
 * return 1 if equal, 0 otherwise.
 */
int
caseless_equal(str1, str2)
	char *str1;
	char *str2;
{
	if ((str1 == NULL)||(str2 == NULL))
	{
		return(0);
	}

	while ((*str1 != '\0')&&(*str2 != '\0'))
	{
		if (TOLOWER(*str1) != TOLOWER(*str2))
		{
			return(0);
		}
		str1++;
		str2++;
	}

	if ((*str1 == '\0')&&(*str2 == '\0'))
	{
		return(1);
	}
	else
	{
		return(0);
	}
}


/*
 * Check if two strings are equal in the first count characters, ignoring case.
 * The strings must both be at least of length count to be equal.
 * return 1 if equal, 0 otherwise.
 */
int
caseless_equal_prefix(str1, str2, cnt)
	char *str1;
	char *str2;
	int cnt;
{
	int i;

	if ((str1 == NULL)||(str2 == NULL))
	{
		return(0);
	}

	if (cnt < 1)
	{
		return(1);
	}

	for (i=0; i < cnt; i++)
	{
		if (TOLOWER(*str1) != TOLOWER(*str2))
		{
			return(0);
		}
		str1++;
		str2++;
	}

	return(1);
}


/*
 * Clean up the white space in a string.
 * Remove all leading and trailing whitespace, and turn all
 * internal whitespace into single spaces separating words.
 * The cleaning is done by rearranging the chars in the passed
 * txt buffer.  The resultant string will probably be shorter,
 * it can never get longer.
 */
void
clean_white_space(txt)
	char *txt;
{
	char *ptr;
	char *start;

	start = txt;
	ptr = txt;

	/*
	 * Remove leading white space
	 */
	while (isspace((int)*ptr))
	{
		ptr++;
	}

	/*
	 * find a word, copying if we removed some space already
	 */
	if (start == ptr)
	{
		while ((!isspace((int)*ptr))&&(*ptr != '\0'))
		{
			ptr++;
		}
		start = ptr;
	}
	else
	{
		while ((!isspace((int)*ptr))&&(*ptr != '\0'))
		{
			*start++ = *ptr++;
		}
	}

	while (*ptr != '\0')
	{
		/*
		 * Remove trailing whitespace.
		 */
		while (isspace((int)*ptr))
		{
			ptr++;
		}
		if (*ptr == '\0')
		{
			break;
		}

		/*
		 * If there are more words, insert a space and if space was
		 * removed move up remaining text.
		 */
		*start++ = ' ';
		if (start == ptr)
		{
			while ((!isspace((int)*ptr))&&(*ptr != '\0'))
			{
				ptr++;
			}
			start = ptr;
		}
		else
		{
			while ((!isspace((int)*ptr))&&(*ptr != '\0'))
			{
				*start++ = *ptr++;
			}
		}
	}

	*start = '\0';
}


/*
 * parse an amperstand escape, and return the appropriate character, or
 * '\0' on error.
 * we should really only use caseless_equal_prefix for unterminated, and use
 * caseless_equal otherwise, but since there are so many escapes, and I
 * don't want to type everything twice, I always use caseless_equal_prefix
 * Turns out the escapes are case sensitive, use strncmp.
 * termination states:
 *	0: terminated with a ';'
 *	1: unterminated
 *	2: terminated with whitespace
 */
char
ExpandEscapes(esc, endp, termination)
	char *esc;
	char **endp;
	int termination;
{
	int cnt;
	char val;
	int unterminated;

	unterminated = (termination & 0x01);

	esc++;
	if (*esc == '#')
	{
		if (unterminated)
		{
			char *tptr;
			char tchar;

			tptr = (char *)(esc + 1);
			while (isdigit((int)*tptr))
			{
				tptr++;
			}
			tchar = *tptr;
			*tptr = '\0';
			val = (char)atoi((esc + 1));
			*tptr = tchar;
			*endp = tptr;
		}
		else
		{
			val = (char)atoi((esc + 1));
			*endp = (char *)(esc + strlen(esc));
		}
	}
	else
	{
		int escLen, ampLen;
		cnt = 0;
		escLen = strlen(esc);
		while (AmpEscapes[cnt].tag != NULL)
		{
			ampLen = strlen(AmpEscapes[cnt].tag);
			if ((escLen == ampLen) && (strncmp(esc, AmpEscapes[cnt].tag, ampLen) == 0))
			{
				val = AmpEscapes[cnt].value;
				*endp = (char *)(esc +
					strlen(AmpEscapes[cnt].tag));
				break;
			}
			cnt++;
		}
		if (AmpEscapes[cnt].tag == NULL)
		{
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr, "Error bad & string\n");
			}
#endif
			val = '\0';
			*endp = (char *)NULL;
		}
	}

	return(val);
}


/*
 * Clean the special HTML character escapes out of the text and replace
 * them with the appropriate characters "&lt;" = "<", "&gt;" = ">",
 * "&amp;" = "&"
 * GAG:  apperantly &lt etc. can be left unterminated, what a nightmare.
 * Ok, better, they have to be terminated with white-space or ';'.
 * the '&' character must be immediately followed by a letter to be
 * a valid escape sequence.  Other &'s are left alone.
 * The cleaning is done by rearranging chars in the passed txt buffer.
 * if any escapes are replaced, the string becomes shorter.
 */
void
clean_text(txt)
	char *txt;
{
	int unterminated;
	int space_terminated;
	char *ptr;
	char *ptr2;
	char *start;
	char *text;
	char *tend;
	char tchar;
	char val;

	if (txt == NULL)
	{
		return;
	}

	/*
	 * Quick scan to find escape sequences.
	 * Escape is '&' followed by a letter (or a hash mark).
	 * return if there are none.
	 */
	ptr = txt;
	while (*ptr != '\0')
	{
		if ((*ptr == '&')&&
			((isalpha((int)*(ptr + 1)))||(*(ptr + 1) == '#')))
		{
			break;
		}
		ptr++;
	}
	if (*ptr == '\0')
	{
		return;
	}

	/*
	 * Loop, replaceing escape sequences, and moving up remaining
	 * text.
	 */
	ptr2 = ptr;
	while (*ptr != '\0')
	{

		unterminated = 0;
		space_terminated = 0;
		/*
		 * Extract the escape sequence from start to ptr
		 */
		start = ptr;
		while ((*ptr != ';')&&(!isspace((int)*ptr))&&(*ptr != '\0'))
		{
			ptr++;
		}
		if (*ptr == '\0')
		{
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr, "warning:  unterminated & (%s)\n",
					start);
			}
#endif
			unterminated = 1;
		}
		else if (isspace((int)*ptr))
		{
			space_terminated = 1;
		}

		/*
		 * Copy the escape sequence into a separate buffer.
		 * Then clean spaces so the "& lt ;" = "&lt;" etc.
		 * The cleaning should be unnecessary.
		 */
		tchar = *ptr;
		*ptr = '\0';
		text = (char *)malloc(strlen(start) + 1);
		if (text == NULL)
		{
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr, "Cannot malloc space for & text\n");
			}
#endif
			*ptr = tchar;
			return;
		}
		strcpy(text, start);
		*ptr = tchar;
		clean_white_space(text);

		/*
		 * Replace escape sequence with appropriate character
		 */
		val = ExpandEscapes(text, &tend,
			((space_terminated << 1) + unterminated));
		if (val != '\0')
		{
			if (unterminated)
			{
				tchar = *tend;
				*tend = '\0';
				ptr = (char *)(start + strlen(text) - 1);
				*tend = tchar;
			}
			else if (space_terminated)
			{
				ptr--;
			}
			*ptr2 = val;
			unterminated = 0;
			space_terminated = 0;
		}
		/*
		 * invalid escape sequence. skip it.
		 */
		else
		{
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr, "Error bad & string\n");
			}
#endif
			ptr = start;
			*ptr2 = *ptr;
		}
		free(text);

		/*
		 * Copy forward remaining text until you find the next
		 * escape sequence
		 */
		ptr2++;
		ptr++;
		while (*ptr != '\0')
		{
			if ((*ptr == '&')&&
			    ((isalpha((int)*(ptr + 1)))||(*(ptr + 1) == '#')))
			{
				break;
			}
			*ptr2++ = *ptr++;
		}
	}
	*ptr2 = '\0';
}


/*
 * Get a block of text from a HTML document.
 * All text from start to the end, or the first mark
 * (a mark is '<' or '</' followed by any letter or a '!')
 * is returned in a malloced buffer.  Also, endp returns
 * a pointer to the next '<' or '\0'
 * The returned text has already expanded '&' escapes.
 */
char *
get_text(start, endp)
	char *start;
	char **endp;
{
	char *ptr;
	char *text;
	char tchar;

	if (start == NULL)
	{
		return(NULL);
	}

	/*
	 * Copy text up to beginning of a mark, or the end
	 */
	ptr = start;
	while (*ptr != '\0')
	{
		if (*ptr == '<')
		{
			if (isalpha((int)(*(ptr + 1))))
			{
				break;
			}
			else if (*(ptr + 1) == '/')
			{
				if (isalpha((int)(*(ptr + 2))))
				{
					break;
				}
			}
			else if (*(ptr + 1) == '!')  /* a comment */
			{
				break;
			}
		}
		ptr++;
	}
	*endp = ptr;

	if (ptr == start)
	{
		return(NULL);
	}

	/*
	 * Copy the text into its own buffer, and clean it
	 * of escape sequences.
	 */
	tchar = *ptr;
	*ptr = '\0';
	text = (char *)malloc(strlen(start) + 1);
	if (text == NULL)
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "Cannot malloc space for text\n");
		}
#endif
		*ptr = tchar;
		return(NULL);
	}
	strcpy(text, start);
	*ptr = tchar;
	clean_text(text);

	return(text);
}


/*
 * Get the mark text between '<' and '>'.  From the text, determine
 * its type, and fill in a mark_up structure to return.  Also returns
 * endp pointing to the ttrailing '>' in the original string.
 */
struct mark_up *
get_mark(start, endp)
	char *start;
	char **endp;
{
	char *ptr;
	char *text;
	char tchar;
	struct mark_up *mark;
	int  comment=0;       /* amb - comment==1 if we are in a comment */
	char *first_gt=NULL;  /* keep track of ">" for old broken comments */

	if (start == NULL)
	{
		return(NULL);
	}

	if (*start != '<')
	{
		return(NULL);
	}

	/* amb - check if we are in a comment, start tag is <!-- */
	if (strncmp (start, "<!--", 4)==0)
	  comment=1;

	start++;
	first_gt = NULL;

	mark = (struct mark_up *)malloc(sizeof(struct mark_up));
	if (mark == NULL)
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "Cannot malloc space for mark_up struct\n");
		}
#endif
		return(NULL);
	}

	/*
	 * Grab the mark text
	 */
	ptr = start;

	/* amb - skip over the comment text */
	/* end tag is --*>, where * is zero or more spaces (ugh) */
	if (comment)
	  {
	    while (*ptr != '\0')
	      {
		if ( (*ptr == '>') && (!first_gt) )
		  first_gt = ptr;
		if (strncmp (ptr, "--", 2) == 0)   /* found double dash (--) */
		  {
		    ptr += 2;
		    while ((*ptr != '\0') && ((*ptr == ' ') || (*ptr == '\n')
					      || (*ptr == '-') ))
		      ptr++;                   /* skip spaces and newlines */
		    if (*ptr == '>')                /* completed end comment */
		      {
			*endp = ptr;
			mark->is_end = 1;
			mark->type = M_COMMENT;
			mark->start = NULL;
			mark->text = NULL;
			mark->end = NULL;
			mark->next = NULL;
			return(mark);
		      }
		  }
		else                         /* if no double dash (--) found */
		  ptr++;
	      }
	    /* if we get here, this document must use the old broken
	       comment style */
	    if (first_gt) {
		ptr = first_gt;
	    }
	  } /* end of: if (comment) */

	while (ptr&&(*ptr != '>')&&(*ptr != '\0'))
	{
		ptr++;
	}
	if (ptr) {
		*endp=ptr;
	}
	else {
		return(NULL); /*only if EOF and no close comment -- SWP*/
	}

	if (*ptr != '>')
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "error: bad mark format\n");
		}
#endif
		return(NULL);
	}

	/*
	 * Copy the mark text to its own buffer, and
	 * clean it of escapes, and odd white space.
	 */
	tchar = *ptr;
	*ptr = '\0';
	text = (char *)malloc(strlen(start) + 1);
	if (text == NULL)
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "Cannot malloc space for mark\n");
		}
#endif
		*ptr = tchar;
		return(NULL);
	}
	strcpy(text, start);
	*ptr = tchar;
	clean_text(text);
/*
 * No longer needed because the parsing code is now smarter
 *
	clean_white_space(text);
 *
 */

	/*
	 * Set whether this is the start or end of a mark
	 * block, as well as determining its type.
	 */
	if (*text == '/')
	{
		mark->is_end = 1;
		mark->type = ParseMarkType((char *)(text + 1));
		mark->start = NULL;
		mark->text = NULL;
		mark->end = text;
	}
	else
	{
		mark->is_end = 0;
		mark->type = ParseMarkType(text);
		mark->start = text;
		mark->text = NULL;
		mark->end = NULL;
	}

	mark->text = NULL;
	mark->next = NULL;

	return(mark);
}


/*
 * Special version of get_text.  It reads all text up to the
 * end of the plain text mark, or the end of the file.
 */
char *
get_plain_text(start, endp)
	char *start;
	char **endp;
{
	char *ptr;
	char *text;
	char tchar;

	if (start == NULL)
	{
		return(NULL);
	}

	/*
	 * Read until stopped by end plain text mark.
	 */
	ptr = start;
	while (*ptr != '\0')
	{
		/*
		 * Beginning of a mark is '<' followed by any letter,
		 * or followed by '!' for a comment,
		 * or '</' followed by any letter.
		 */
		if ((*ptr == '<')&&
			((isalpha((int)(*(ptr + 1))))||
			(*(ptr + 1) == '!')||
			((*(ptr + 1) == '/')&&(isalpha((int)(*(ptr + 2)))))))
		{
			struct mark_up *mp;
			char *ep;

			/*
			 * We think we found a mark.  If it is the
			 * end of plain text, break out
			 */
			mp = get_mark(ptr, &ep);
			if (mp != NULL)
			{
				if (((mp->type == M_PLAIN_TEXT)||
				    (mp->type == M_LISTING_TEXT))&&(mp->is_end))
				{
					if (mp->end != NULL)
					{
						free((char *)mp->end);
					}
					free((char *)mp);
					break;
				}
				if (mp->start != NULL)
				{
					free((char *)mp->start);
				}
				if (mp->end != NULL)
				{
					free((char *)mp->end);
				}
				free((char *)mp);
			}
		}
		ptr++;
	}
	*endp = ptr;

	if (ptr == start)
	{
		return(NULL);
	}

	/*
	 * Copy text to its own malloced buffer, and clean it of
	 * HTML escapes.
	 */
	tchar = *ptr;
	*ptr = '\0';
	text = (char *)malloc(strlen(start) + 1);
	if (text == NULL)
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "Cannot malloc space for text\n");
		}
#endif
		*ptr = tchar;
		return(NULL);
	}
	strcpy(text, start);
	*ptr = tchar;
	clean_text(text);

	return(text);
}


static char *atts[]={"text","bgcolor","alink","vlink","link",NULL};

/*
 * Main parser of HTML text.  Takes raw text, and produces a linked
 * list of mark objects.  Mark objects are either text strings, or
 * starting and ending mark delimiters.
 * The old list is passed in so it can be freed, and in the future we
 * may want to add code to append to the old list.
 */
struct mark_up *
HTMLParse(old_list, str, hw)
	struct mark_up *old_list;
	char *str;
        void *hw;
{
	int preformat;
	char *start, *end;
	char *text, *tptr;
	struct mark_up *mark;
	struct mark_up *list;
	struct mark_up *current;

#ifndef DISABLE_TRACE
	if (htmlwTrace) {
#ifndef VMS
		gettimeofday(&Tv, &Tz);
		fprintf(stderr, "HTMLParse enter (%d.%d)\n", Tv.tv_sec, Tv.tv_usec);
#else
                fprintf(stderr, "HTMLParse enter (%s)\n", asctime(localtime(&clock)));
#endif
	}
#endif

	preformat = 0;

	/*
	 * Free up the previous Object List if one exists
	 */
	FreeObjList(old_list);

	if (str == NULL)
	{
		return(NULL);
	}

	list = NULL;
	current = NULL;

	start = str;
	end = str;

	mark = NULL;
	while (*start != '\0')
	{
		/*
		 * Get some text (if any).  If our last mark was
		 * a begin plain text we call different function
		 * If last mark was <PLAINTEXT> we lump all the rest of
		 * the text in.
		 */
		if ((mark != NULL)&&(mark->type == M_PLAIN_FILE)&&
			(!mark->is_end))
		{
			text = start;
			end = text;
			while (*end != '\0')
			{
				end++;
			}
			/*
			 * Copy text to its own malloced buffer, and clean it of
			 * HTML escapes.
			 */
			tptr = (char *)malloc(strlen(text) + 1);
			if (tptr == NULL)
			{
#ifndef DISABLE_TRACE
				if (htmlwTrace) {
					fprintf(stderr,
						"Cannot malloc space for text\n");
				}
#endif

				return(list);
			}
			strcpy(tptr, text);
			text = tptr;
		}
		else if ((mark != NULL)&&
			 ((mark->type == M_PLAIN_TEXT)||
			  (mark->type == M_LISTING_TEXT))&&
			 (!mark->is_end))
		{
			text = get_plain_text(start, &end);
		}
		else
		{
			text = get_text(start, &end);
		}

		/*
		 * If text is OK, put it into a mark structure, and add
		 * it to the linked list.
		 */
		if (text == NULL)
		{
			if (start != end)
			{
#ifndef DISABLE_TRACE
				if (htmlwTrace) {
					fprintf(stderr, "error parsing text, bailing out\n");
				}
#endif

				return(list);
			}
		}
		else
		{
			mark = (struct mark_up *)malloc(sizeof(struct mark_up));
			if (mark == NULL)
			{
#ifndef DISABLE_TRACE
				if (htmlwTrace) {
					fprintf(stderr, "Cannot malloc for mark_up struct\n");
				}
#endif

				return(list);
			}
			mark->type = M_NONE;
			mark->is_end = 0;
			mark->start = NULL;
			mark->text = text;
			mark->end = NULL;
			mark->next = NULL;
			current = AddObj(&list, current, mark, preformat);
		}
		start = end;

		if (*start == '\0')
		{
			break;
		}

		/*
		 * Get the next mark if any, and if it is
		 * valid, add it to the linked list.
		 */
		mark = get_mark(start, &end);
		if (mark == NULL)
		{
			if (start != end)
			{
#ifndef DISABLE_TRACE
				if (htmlwTrace) {
					fprintf(stderr, "error parsing mark, bailing out\n");
				}
#endif

				return(list);
			}
		}
		else
		{
/* WE SUCK.  We're a bunch of pathetic followers. */
/* ABSOLUTE CHEEZE OF THE FINEST KIND - bjs - 9/21/95 */
                    if(mark->type==M_DOC_BODY && mark->start){
			char *tmp=NULL,*tmp_bgname=NULL;
			int i;

			if (!NoBodyImages(hw)) {
				tmp_bgname=ParseMarkTag(mark->start,
							MT_DOC_BODY,
							"background");
			}
			if (!NoBodyColors(hw)) {
				for(i=0;atts[i];i++) {
					tmp=ParseMarkTag(mark->start,
							 MT_DOC_BODY,atts[i]);
					if (tmp) {
						hw_do_color(hw,atts[i],tmp);
						free(tmp);
						tmp=NULL;
					}
				}
			}
			if (tmp_bgname) {
				hw_do_bg(hw,tmp_bgname);
				free(tmp_bgname);
				tmp_bgname=NULL;
			}
		    }

		    mark->next = NULL;
                    current = AddObj(&list, current, mark, preformat);

		}



		start = (char *)(end + 1);

		if ((mark != NULL)&&(mark->type == M_PLAIN_FILE)&&
			(!mark->is_end))
		{
			/*
			 * A linefeed immediately after the <PLAINTEXT>
			 * mark is to be ignored.
			 */
			if (*start == '\n')
			{
				start++;
			}
		}
		else if ((mark != NULL)&&((mark->type == M_PLAIN_TEXT)||
			(mark->type == M_LISTING_TEXT))&&
			(!mark->is_end))
		{
			/*
			 * A linefeed immediately after the <XMP>
			 * or <LISTING> mark is to be ignored.
			 */
			if (*start == '\n')
			{
				start++;
			}
		}
		/*
		 * If we are parsing pre-formatted text we need to set a
		 * flag so we don't throw out needed linefeeds.
		 */
		else if ((mark != NULL)&&(mark->type == M_PREFORMAT))
		{
			if (mark->is_end)
			{
				preformat = 0;
			}
			else
			{
				preformat = 1;
				/*
				 * A linefeed immediately after the <PRE>
				 * mark is to be ignored.
				 */
				if (*start == '\n')
				{
					start++;
				}
			}
		}
	}

#ifndef DISABLE_TRACE
	if (htmlwTrace) {
#ifndef VMS
		gettimeofday(&Tv, &Tz);
		fprintf(stderr, "HTMLParse exit (%d.%d)\n", Tv.tv_sec, Tv.tv_usec);
#else
                fprintf(stderr, "HTMLParse exit (%s)\n", asctime(localtime(&clock)));
#endif
	}
#endif

	return(list);
}



/*
 * Determine mark type from the identifying string passed
 */

int
ParseMarkType(str)
	char *str;
{
	int type;
	char *tptr;
	char tchar;

	if (str == NULL)
	{
		return(M_NONE);
	}

	type = M_UNKNOWN;
	tptr = str;
	while (*tptr != '\0')
	{
		if (isspace((int)*tptr))
		{
			break;
		}
		tptr++;
	}
	tchar = *tptr;
	*tptr = '\0';

	if (caseless_equal(str, MT_ANCHOR))
	{
		type = M_ANCHOR;
	}
	else if (caseless_equal(str, MT_FRAME))
	{
		type = M_FRAME;
	}
	else if (caseless_equal(str, MT_TITLE))
	{
		type = M_TITLE;
	}
	else if (caseless_equal(str, MT_FIXED))
	{
		type = M_FIXED;
	}
	else if (caseless_equal(str, MT_BOLD))
	{
		type = M_BOLD;
	}
	else if (caseless_equal(str, MT_ITALIC))
	{
		type = M_ITALIC;
	}
	else if (caseless_equal(str, MT_EMPHASIZED))
	{
		type = M_EMPHASIZED;
	}
	else if (caseless_equal(str, MT_STRONG))
	{
		type = M_STRONG;
	}
	else if (caseless_equal(str, MT_CODE))
	{
		type = M_CODE;
	}
	else if (caseless_equal(str, MT_SAMPLE))
	{
		type = M_SAMPLE;
	}
	else if (caseless_equal(str, MT_KEYBOARD))
	{
		type = M_KEYBOARD;
	}
	else if (caseless_equal(str, MT_VARIABLE))
	{
		type = M_VARIABLE;
	}
	else if (caseless_equal(str, MT_CITATION))
	{
		type = M_CITATION;
	}
	else if (caseless_equal(str, MT_STRIKEOUT))
	{
		type = M_STRIKEOUT;
	}
	else if (caseless_equal(str, MT_HEADER_1))
	{
		type = M_HEADER_1;
	}
	else if (caseless_equal(str, MT_HEADER_2))
	{
		type = M_HEADER_2;
	}
	else if (caseless_equal(str, MT_HEADER_3))
	{
		type = M_HEADER_3;
	}
	else if (caseless_equal(str, MT_HEADER_4))
	{
		type = M_HEADER_4;
	}
	else if (caseless_equal(str, MT_HEADER_5))
	{
		type = M_HEADER_5;
	}
	else if (caseless_equal(str, MT_HEADER_6))
	{
		type = M_HEADER_6;
	}
	else if (caseless_equal(str, MT_ADDRESS))
	{
		type = M_ADDRESS;
	}
	else if (caseless_equal(str, MT_PLAIN_TEXT))
	{
		type = M_PLAIN_TEXT;
	}
	else if (caseless_equal(str, MT_LISTING_TEXT))
	{
		type = M_LISTING_TEXT;
	}
	else if (caseless_equal(str, MT_PLAIN_FILE))
	{
		type = M_PLAIN_FILE;
	}
	else if (caseless_equal(str, MT_PARAGRAPH))
	{
		type = M_PARAGRAPH;
	}
	else if (caseless_equal(str, MT_UNUM_LIST))
	{
		type = M_UNUM_LIST;
	}
	else if (caseless_equal(str, MT_NUM_LIST))
	{
		type = M_NUM_LIST;
	}
	else if (caseless_equal(str, MT_MENU))
	{
		type = M_MENU;
	}
	else if (caseless_equal(str, MT_DIRECTORY))
	{
		type = M_DIRECTORY;
	}
	else if (caseless_equal(str, MT_LIST_ITEM))
	{
		type = M_LIST_ITEM;
	}
	else if (caseless_equal(str, MT_DESC_LIST))
	{
		type = M_DESC_LIST;
	}
	else if (caseless_equal(str, MT_DESC_TITLE))
	{
		type = M_DESC_TITLE;
	}
	else if (caseless_equal(str, MT_DESC_TEXT))
	{
		type = M_DESC_TEXT;
	}
	else if (caseless_equal(str, MT_PREFORMAT))
	{
		type = M_PREFORMAT;
	}
	else if (caseless_equal(str, MT_BLOCKQUOTE))
	{
		type = M_BLOCKQUOTE;
	}
	else if (caseless_equal(str, MT_INDEX))
	{
		type = M_INDEX;
	}
	else if (caseless_equal(str, MT_HRULE))
	{
		type = M_HRULE;
	}
	else if (caseless_equal(str, MT_BASE))
	{
		type = M_BASE;
	}
	else if (caseless_equal(str, MT_LINEBREAK))
	{
		type = M_LINEBREAK;
	}
	else if (caseless_equal(str, MT_IMAGE))
	{
		type = M_IMAGE;
	}
	else if (caseless_equal(str, MT_FIGURE))
	{
		type = M_FIGURE;
	}
	else if (caseless_equal(str, MT_SELECT))
	{
		type = M_SELECT;
	}
	else if (caseless_equal(str, MT_OPTION))
	{
		type = M_OPTION;
	}
	else if (caseless_equal(str, MT_INPUT))
	{
		type = M_INPUT;
	}
	else if (caseless_equal(str, MT_TEXTAREA))
	{
		type = M_TEXTAREA;
	}
	else if (caseless_equal(str, MT_FORM))
	{
		type = M_FORM;
	}
/*amb*/
        else if (caseless_equal(str, MT_SUP))
        {
                type = M_SUP;
        }
        else if (caseless_equal(str, MT_SUB))
        {
                type = M_SUB;
        }
	else if (caseless_equal(str, MT_DOC_HEAD))
        {
	        type = M_DOC_HEAD;
        }
	else if (caseless_equal(str, MT_UNDERLINED))
        {
	        type = M_UNDERLINED;
        }
	else if (caseless_equal(str, MT_DOC_BODY))
        {
	        type = M_DOC_BODY;
        }
	else if (caseless_equal(str, MT_TABLE))
	{
		if (tableSupportEnabled) {
			type = M_TABLE;
		}
		else {
			type = M_UNKNOWN;
		}
	}
	else if (caseless_equal(str, MT_CAPTION))
	{
		type = M_CAPTION;
	}
	else if (caseless_equal(str, MT_TABLE_ROW))
	{
		if (tableSupportEnabled) {
			type = M_TABLE_ROW;
		}
		else {
			type = M_LINEBREAK;
		}
	}
	else if (caseless_equal(str, MT_TABLE_HEADER))
	{
		if (tableSupportEnabled) {
			type = M_TABLE_HEADER;
		}
		else {
			type = M_UNKNOWN;
		}
	}
	else if (caseless_equal(str, MT_TABLE_DATA))
	{
		if (tableSupportEnabled) {
			type = M_TABLE_DATA;
		}
		else {
			type = M_UNKNOWN;
		}
	}
	else if (caseless_equal(str, MT_MAP))
	{
		type=M_MAP;
	}
	else if (caseless_equal(str, MT_CENTER))
	{
		type = M_CENTER;
	}
	else
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "warning: unknown mark (%s)\n", str);
		}
#endif
		type = M_UNKNOWN;
	}

	*tptr = tchar;
	return(type);
}


/*
 * Parse a single anchor tag.  ptrp is a pointer to a pointer to the
 * string to be parsed.  On return, the ptr should be changed to
 * point to after the text we have parsed.
 * On return start and end should point to the beginning, and just
 * after the end of the tag's name in the original anchor string.
 * Finally the function returns the tag value in a malloced buffer.
 */
char *
AnchorTag(ptrp, startp, endp)
	char **ptrp;
	char **startp;
	char **endp;
{
	char *tag_val;
	char *ptr;
	char *start;
	char tchar;
	int quoted;
	int has_value;

	quoted = 0;

	/*
	 * remove leading spaces, and set start
	 */
	ptr = *ptrp;
	while (isspace((int)*ptr))
	{
		ptr++;
	}
	*startp = ptr;

	/*
	 * Find and set the end of the tag
	 */
	while ((!isspace((int)*ptr))&&(*ptr != '=')&&(*ptr != '\0'))
	{
		ptr++;
	}
	*endp = ptr;

        has_value=0;
	if (*ptr == '\0')
	{
		*ptrp = ptr;
/*		return(NULL);*/
                    /* try to handle <A NAME=blah></A> correctly -bjs*/
	} else {
	/*
	 * Move to the start of the tag value, if there is one.
	 */
            while ((isspace((int)*ptr))||(*ptr == '='))
            {
		if (*ptr == '=')
		{
                    has_value = 1;
		}
		ptr++;
            }
        }

	/*
	 * For a tag with no value, this is a boolean flag.
	 * Return the string "1" so we know the tag is there.
	 */
	if (!has_value)
	{
		*ptrp = *endp;
		/*
		 * set a tag value of 1.
		 */
		tag_val = (char *)malloc(strlen("1") + 1);
		if (tag_val == NULL)
		{
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr, "can't malloc space for tag value\n");
			}
#endif

			return(NULL);
		}
		strcpy(tag_val, "1");

		return(tag_val);
	}

	if (*ptr == '\"')
	{
		quoted = 1;
		ptr++;
	}

	start = ptr;
	/*
	 * Get tag value.  Either a quoted string or a single word
	 */
	if (quoted)
	{
		while ((*ptr != '\"')&&(*ptr != '\0'))
		{
			ptr++;
		}
	}
	else
	{
		while ((!isspace((int)*ptr))&&(*ptr != '\0'))
		{
			ptr++;
		}
	}
/* amb - everyone forgets the end quotes on anchor
   attributes, so we'll let it slide */

/*
	if ((quoted)&&(*ptr == '\0'))
	{
		*ptrp = ptr;
		return(NULL);
	}
*/

	/*
	 * Copy the tag value out into a malloced string
	 */
	tchar = *ptr;
	*ptr = '\0';
	tag_val = (char *)malloc(strlen(start) + 1);
	if (tag_val == NULL)
	{
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr, "can't malloc space for tag value\n");
		}
#endif
		*ptr = tchar;
		*ptrp = ptr;

		return(NULL);
	}
	strcpy(tag_val, start);
	*ptr = tchar;

	/* If you forgot the end quote, you need to make sure you aren't
		indexing ptr past the end of its own array -- SWP */
	if (quoted && *ptr!='\0')
	{
		ptr++;
	}
	*ptrp = ptr;

	return(tag_val);
}


/*
 * Parse mark text for the value associated with the
 * passed mark tag.
 * If the passed tag is not found, return NULL.
 * If the passed tag is found but has no value, return "".
 */
char* ParseMarkTag(text, mtext, mtag)
	char *text;
	char *mtext;
	char *mtag;
{
	char *ptr;
	char *start;
	char *end;
	char *tag_val;
	char tchar;

	if ((text == NULL)||(mtext == NULL)||(mtag == NULL))
	{
		return(NULL);
	}

	ptr = (char *)(text + strlen(mtext));

	while (*ptr != '\0')
	{
		tag_val = AnchorTag(&ptr, &start, &end);

		tchar = *end;
		*end = '\0';
		if (caseless_equal(start, mtag))
		{
			*end = tchar;
			if (tag_val == NULL)
			{
				tag_val = (char *)malloc(1);
				*tag_val = '\0';
				return(tag_val);
			}
			else
			{
				return(tag_val);
			}
		}
		*end = tchar;
		if (tag_val != NULL)
		{
			free(tag_val);
		}
	}
	return(NULL);
}

