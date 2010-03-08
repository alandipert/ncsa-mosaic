#include "../config.h"
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include "HTMLP.h"
#include "HTML.h"
#include "list.h"
#ifdef VAXC
#include <ctype.h>
#endif /* VAXC, need for isspace, GEC */

#define	DEFAULT_FIELD_WIDTH	30
#define	DEFAULT_FIELD_HEIGHT	20

#define FIELD_BORDER_SPACE	5	/*aesthetic space around element */

#define FONTHEIGHT(font) (font->max_bounds.ascent + font->max_bounds.descent)

extern char *ParseMarkTag();

#ifndef DISABLE_TRACE
extern int htmlwTrace;
#endif

/* Allocate a TableField and initialize to default values 
 * return 0 on failure 
 */
static TableField *NewTableField()
{
TableField *tf;

	if (!(tf = (TableField *) malloc(sizeof(TableField)))) {
		return(0);
		}
	tf->alignment = ALIGN_CENTER;
	tf->colSpan = 1;
	tf->rowSpan = 1;
	tf->contVert = False;
	tf->contHoriz = False;
	tf->maxWidth = DEFAULT_FIELD_WIDTH;
	tf->minWidth = DEFAULT_FIELD_WIDTH;
	tf->maxHeight = DEFAULT_FIELD_HEIGHT; 
	tf->minHeight = DEFAULT_FIELD_HEIGHT;
	tf->header = False;

	tf->type = F_NONE;
	tf->text = (char *) 0;
	tf->font = (XFontStruct *) 0;
	tf->formattedText = (char **) 0;
	tf->numLines = 0;

	tf->image = (ImageInfo *) 0;
	tf->winfo = (WidgetInfo *) 0;

	return(tf);
}


/* return a word out of the text */
void GetWord(text,retStart,retEnd)
char *text;	 /* text to get a word out of */
char **retStart; /* RETURNED: start of word in text */
char **retEnd;	 /* RETURNED: end of word in text */
{
char *start;
char *end;

	if (!text) {
		*retStart = *retEnd = text;
		return;
		}

	start = text;
	while ((*start) && isspace(*start)){ /*skip over leading space*/
		start++;
		}

	end = start;
	while((*end) && (!isspace(*end))){ /* find next space */
		end++;
		}

	*retStart = start;
	*retEnd = end;
	return;
}



/* PourText() this routine pours a text string of a particular font into a 
   rectangular area of specified dimensions.  The return value is a list of 
   text lines that will fit within the given width.
   If a height is specified, then text will be truncated if necessary to fit.
   If height is 0, then all of the text is in the list. 
   The actual pixel height of the text is returned in variable height.
*/
int PourText(text,font,width,height,percentVertSpace,formattedText,numberOfLines)
char *text; /* assumed that text is already clean and without newlines */
XFontStruct *font;
int width;   /* width of area to pour text */
int *height; /* if passed height value is zero, then height is returned */
	     /* if passed height is non zero, then truncate to this height */
int percentVertSpace; /* likely hw->html.percent_vert_space */
		      /* needed to compute height of text */
char ***formattedText; /* RETURNED: array of text lines */
int *numberOfLines;   /* RETURNED: number of lines */
{
int fontHeight;
int stringWidth;
int builtWidth;
char *textPtr;
char *wordStart,*wordEnd;
int wordWidth;  /* in pixels */
int wordLength; /* in chars */
List textList;  /* returned list of text lines */
char tmpBuff[5120]; /*ACK!, can't have more than 5k chars on a line in a field*/
int spaceWidth; /* width of a space in this font */
char **cTextList;
int numLines;
int y;

#ifndef DISABLE_TRACE
	if (htmlwTrace) {
		fprintf(stderr,"PourText: \"%s\" width=%d,height=%d\n",text,width,height);
	}
#endif

	if (!text) {
		*height = 0;
		*formattedText = (char **) 0;
		*numberOfLines = 0;
		return(0);
		}
	
	textList = ListCreate();
        stringWidth = XTextWidth(font,text,strlen(text));
	if (stringWidth < width) {
		ListAddEntry(textList,strdup(text));
		}
	else {

	    builtWidth = 0;
	    textPtr = text;
	    spaceWidth = XTextWidth(font," ",1);
	    *tmpBuff = '\0';
	    while (*textPtr) {

#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			printf("textPtr = \"%s\"\n",textPtr);
		}
#endif

		GetWord(textPtr,&wordStart,&wordEnd);
		wordLength = (int) (wordEnd - wordStart);
		wordWidth = XTextWidth(font,wordStart, wordLength);
		if ((builtWidth + spaceWidth + wordWidth)  < width) { 
						/* then add to line */
			if (builtWidth) {
				/* only add space if something on line already*/
				strcat(tmpBuff, " ");
				builtWidth += spaceWidth;
				}
			strncat(tmpBuff, wordStart, wordLength);
			builtWidth += wordWidth;
			}
		else if (wordWidth < width) { 
			/* start new line */
			ListAddEntry(textList, strdup(tmpBuff));
			*tmpBuff = '\0';
			builtWidth = 0;

			/* and add it to the line */
			strncat(tmpBuff, wordStart, wordLength);
			builtWidth += wordWidth;
			}
		else { 
			/* word is too big to fit on a line */
			/* so break up word */
	
			/* start new line */
			ListAddEntry(textList, strdup(tmpBuff));
			*tmpBuff = '\0';
			builtWidth = 0;

			/* find the max that will fit on a line*/
			wordWidth = 0;
			wordLength = 0;
			wordEnd = wordStart;
			while ((*wordEnd) && (width > wordWidth)) {
				wordEnd++;
				wordLength = (int) (wordEnd - wordStart);
				wordWidth = XTextWidth(font,wordStart, 
							wordLength);
				}
			
			strncat(tmpBuff, wordStart, wordLength);
			builtWidth += wordWidth;

			}
		textPtr = wordEnd;
		}

	    if (*tmpBuff) {
		ListAddEntry(textList, strdup(tmpBuff));
		}
	    }

	/* ok, we haven't paid attention to height, so now we are going
	   to remove placed lines that don't fit */

	fontHeight = FONTHEIGHT(font);
	fontHeight += (fontHeight * (percentVertSpace/100));
	if (*height) {
		/* truncate list to specified height */
		while ((ListCount(textList) * fontHeight) > (*height)) {
			ListDeleteEntry(textList, ListTail(textList));
			}
		*height = ListCount(textList) * fontHeight;
		}
	else {
		/* return the height */
		*height = ListCount(textList) * fontHeight;
		}

	/* turn text list into an array */
	numLines = ListCount(textList);
	if (!(cTextList = (char **) malloc(sizeof(char *)* numLines))) {
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr,"Out of Memory\n");
		}
#endif
		*formattedText = (char **) 0;
		*numberOfLines = 0;
		return(0);
		}
	for (y = 0; y < numLines; y++) {
		cTextList[y] = ListHead(textList);
		(void) ListDeleteEntry(textList,cTextList[y]);
		}
	ListDestroy(textList);
	*formattedText = cTextList;
	*numberOfLines = numLines;

	return(numLines);
	
} /* PourText() */


/* print out the table to stdout */
TableDump(t)
TableInfo *t;
{
register int x,y;

#ifndef DISABLE_TRACE
	if (htmlwTrace) {

	fprintf(stderr,"Table dump:\n");
	fprintf(stderr,"Border width is %d\n",t->borders);
	fprintf(stderr,"numColumns=%d, numRows=%d\n",t->numColumns,t->numRows);
	fprintf(stderr,"---------------------------------------------------\n");
	for (y = 0; y < t->numRows; y++ ) {
		fprintf(stderr,"|");
		for (x = 0; x < t->numColumns; x++ ) {
			fprintf(stderr,"colWidth=%d,rowHeight=%d | ",
				t->table[y * t->numColumns + x].colWidth,
				t->table[y * t->numColumns + x].rowHeight);
			}
		fprintf(stderr,"\n---------------------------------------------------\n");
		}

	}
#endif
}


/* fill out uneven rows in table */
/* return 0 on out of memory, 1 on suceess */
static int TableCleanUp(t,tableList)
TableInfo *t;
List tableList;
{
int maxNumCols;
int numColsInRow;
List rowList;
TableField *field;
int x,y;

	/* determine number of columns all rows should have */
	rowList = (List) ListHead(tableList);
	t->numRows = 0;
	maxNumCols = 0;
	while(rowList) {
		field = (TableField *) ListHead(rowList);
		numColsInRow = 0;
		while (field) {
			numColsInRow++;
			field = (TableField *) ListNext(rowList);
			}
		maxNumCols=(maxNumCols > numColsInRow)?maxNumCols:numColsInRow;
		t->numRows++;
		rowList = (List) ListNext(tableList);
		}

	t->numColumns = maxNumCols;


	/* make all rows have correct number of columns */
	rowList = (List) ListHead(tableList);
	while(rowList) {
		numColsInRow = 0;
		field = (TableField *) ListHead(rowList);
		numColsInRow = 0;
		while (field) {
			numColsInRow++;
			field = (TableField *) ListNext(rowList);
			}

		while(numColsInRow < t->numColumns) {
			/* fill up the table with empty fields */
			if (!(field = NewTableField())) {
				return(0); /* out of memory */
				}
			ListAddEntry(rowList,field);
			numColsInRow++;
			}
		rowList = (List) ListNext(tableList);
		}


	/* move 2D link list table to an array for speed */
	if (!(t->table = (TableField *) malloc(sizeof(TableField) 
				* t->numColumns * t->numRows))) {
		return(0); /* out of memory */
		}
	y=0;
	rowList = (List) ListHead(tableList);
	while(rowList) {
		x = 0;
		field = (TableField *) ListHead(rowList);
		while (field) {
			memcpy(&(t->table[y * t->numColumns + x]), field, 
						sizeof(TableField));
			x++;
			field = (TableField *) ListNext(rowList);
			}
		y++;
		rowList = (List) ListNext(tableList);
		}
	

	return(1);
	
}

/* return the number of connected fields */
static int TableHowManyConnectedHorizFields(t,xpos,ypos)
TableInfo *t;
int xpos,ypos;
{
int count;
register int x;

	count = 0;
	for (x = xpos+1; x < t->numColumns; x++) {
		if (t->table[ypos * t->numColumns + x].contHoriz) {
			count++;
			}
		else {
			return(count);
			}
		}
	return(count);
}

/* return the number of connected fields */
static int TableHowManyConnectedVertFields(t,xpos,ypos)
TableInfo *t;
int xpos,ypos;
{
int count;
register int y;

	count = 0;
	for (y = ypos+1; y < t->numRows; y++) {
		if (t->table[y * t->numColumns + xpos].contVert) {
			count++;
			}
		else {
			return(count);
			}
		}
	return(count);
}

static int CalculateMaxWidthOfColumn(t,x)
TableInfo *t;
int x;
{
register int y;
register int maxWidth=0;
register int width;

	for (y=0; y < t->numRows; y++) {
		width = t->table[y * t->numColumns+x].maxWidth;
		maxWidth = (maxWidth > width) ? maxWidth : width;
		}
	return(maxWidth);
}

TableCalculateDimensions(hw,t,pageWidth)
HTMLWidget hw;
TableInfo *t;
int pageWidth;		/* width in pixels of output page */
{
TableField *field;
register int xx,x,y;
int sumMaxWidth;	/* summation of max widths */
int maxWidthOfColumn;
int maxHeightOfRow;
int sumMinWidth;	/* summation of max widths */
int maxWidthOfRow;
int minWidthOfRow;	
int numAdjacent;
float percentToShrink;
int accumulateColWidth;


	/* calculate max and min width for each field*/
	sumMaxWidth = 0;
	sumMinWidth = 0;
	for (y = 0; y < t->numRows; y++) {
	    maxWidthOfRow = 0;
	    minWidthOfRow = 0;
	    for (x = 0; x < t->numColumns; x++ ) {

		field = &(t->table[y * t->numColumns + x]);
		if (field->type == F_TEXT) {
			field->maxWidth = XTextWidth(field->font,
					field->text,strlen(field->text));
			field->minHeight = FONTHEIGHT(field->font);
			}
		else {
			/* non text */
			field->maxWidth = 0;
			}
		maxWidthOfRow += field->maxWidth;

		field->minWidth = 0; /* is minWidth needed? set 0 for now*/
		field->maxHeight = 0;
		/* if it's needed, set it to width of one char or maybe
		   length of longest word in text */
		minWidthOfRow += field->minWidth;
		}
	    /* save the length of the longest and shortest row */
	    sumMaxWidth = (sumMaxWidth > maxWidthOfRow) ? 
						sumMaxWidth : maxWidthOfRow;
	    sumMinWidth = (sumMinWidth > minWidthOfRow) ? 
						sumMinWidth : minWidthOfRow;
	    }


	/* add border spacing to widths */
	sumMaxWidth += (t->numColumns * 2 * FIELD_BORDER_SPACE);
	sumMinWidth += (t->numColumns * 2 * FIELD_BORDER_SPACE);


	/* divy up max width with adjacent continue Horizontal fields*/
	for (y = 0; y < t->numRows; y++) {
		    for (x = 0; x < t->numColumns; x++) {
			numAdjacent = TableHowManyConnectedHorizFields(t,x,y);
			if (numAdjacent) {
			    int xx;
			    for (xx = x; xx < x + numAdjacent; xx++) {
				t->table[y * t->numColumns+xx].maxWidth
					= t->table[y*t->numColumns+x].maxWidth
						/ (numAdjacent+ 1);
				}
			    }
			x += numAdjacent;
			}
		    }
	/* divy up min height with adjacent continue Vertical fields */
	for (x = 0; x < t->numColumns; x++) {
		    for (y = 0; y < t->numRows; y++) {
			numAdjacent = TableHowManyConnectedVertFields(t,x,y);
			if (numAdjacent) {
			    int yy;
			    for (yy = y; yy < y + numAdjacent; yy++) {
				t->table[yy * t->numColumns+x].minHeight
					= t->table[y*t->numColumns+x].minHeight
						/ (numAdjacent + 1);
				}
			    }
			y += numAdjacent;
			}
		    }



	/* fit table to page */
	if (sumMaxWidth < pageWidth ) {
		/* fits on the page, set all fields to use max width */

		for (x = 0; x < t->numColumns; x++) {
			/* find widest field in column */
			maxWidthOfColumn = 0;
			for (y = 0; y < t->numRows; y++ ) {
			    maxWidthOfColumn = 
				(maxWidthOfColumn > 
				t->table[y * t->numColumns + x].maxWidth)?
					maxWidthOfColumn : 
					t->table[y * t->numColumns+x].maxWidth;
			    }
			/* assign uniform width to column */
			for (y = 0; y < t->numRows; y++) {
			        t->table[y*t->numColumns + x].colWidth 
						= maxWidthOfColumn
						+ 2 * FIELD_BORDER_SPACE;
				}
			}
		for (y=0; y < t->numRows; y++) {
			/* find highest of minimum heights */
			maxHeightOfRow = 0;
			for (x=0; x < t->numColumns; x++) {
				maxHeightOfRow = 
				    (maxHeightOfRow > 
				    t->table[y * t->numColumns + x].minHeight)?
					maxHeightOfRow: 
					t->table[y * t->numColumns+x].minHeight;
				}
			/* assign uniform height to row */
			for (x=0; x < t->numColumns; x++) {
				t->table[y * t->numColumns + x].rowHeight
						= maxHeightOfRow
						+ 2 * FIELD_BORDER_SPACE;
				}
			}

		/* take care of formattedText */
		for (y=0; y < t->numRows; y++) {
			for (x=0; x < t->numColumns; x++) {
			    field = &(t->table[y * t->numColumns + x]);
			    field->formattedText =
				    (char **) malloc(sizeof(char *));
			    if (field->text) {
				field->formattedText[0]= strdup(field->text);
				field->numLines = 1;
				}
			    else {
				field->formattedText[0]= (char *) 0;
				field->numLines = 0;
				}
			    }
			}
		}
	else {
	/* will have to squeeze fields downward to fit on page */

		percentToShrink = ((float)pageWidth)/((float)sumMaxWidth);
		for (x = 0; x < t->numColumns; x++) {
			/*find max width of this column */
/*
			maxWidthOfColumn = 0;
			for (y = 0; y < t->numRows; y++) {
				maxWidthOfColumn = (maxWidthOfColumn >
					t->table[y*t->numColumns+x].maxWidth) ?
					maxWidthOfColumn :
					t->table[y*t->numColumns+x].maxWidth;
				}
*/

			/* format it */
			for (y = 0; y < t->numRows; y++) {
				field = &(t->table[y*t->numColumns+x]);
				field->colWidth = (int) (percentToShrink * 
				     ((float) CalculateMaxWidthOfColumn(t,x)));
				field->rowHeight = 0;
				numAdjacent = TableHowManyConnectedHorizFields
									(t,x,y);
				/* calculate the width including connected */
				accumulateColWidth = field->colWidth;
				for (xx = x+1; xx < x+numAdjacent+1; xx++) {
				    accumulateColWidth += (
					(percentToShrink *
                                        ((float) CalculateMaxWidthOfColumn(t,xx))));
				    }
				
#ifndef DISABLE_TRACE
				if (htmlwTrace) {
					fprintf(stderr,"About to call PourText\n");
				}
#endif

				PourText(field->text,field->font,
					accumulateColWidth,
					&(field->rowHeight),
					 hw->html.percent_vert_space,
					&(field->formattedText),
					&(field->numLines));

#ifndef DISABLE_TRACE
				if (htmlwTrace) {
					fprintf(stderr,"poured field %d,%d is dims %d,%d: %%shrink=%f\n",
						x,y,field->colWidth,field->rowHeight,percentToShrink);
				}
#endif

				}
			}

		/* divy up width with adjacent continue Horizontal fields*/
		for (y = 0; y < t->numRows; y++) {
		    for (x = 0; x < t->numColumns; x++) {
			numAdjacent = TableHowManyConnectedHorizFields(t,x,y);
			if (numAdjacent) {
			    int xx;
			    for (xx = x; xx < x + numAdjacent; xx++) {
				t->table[y * t->numColumns+xx].colWidth
					= t->table[y*t->numColumns+x].colWidth
						/ (numAdjacent+ 1);
				}
			    }
			x += numAdjacent;
			}
		    }
		/* divy up height with adjacent continue Vertical fields */
		for (x = 0; x < t->numColumns; x++) {
		    for (y = 0; y < t->numRows; y++) {
			numAdjacent = TableHowManyConnectedVertFields(t,x,y);
			if (numAdjacent) {
			    int yy;
			    for (yy = y; yy < y + numAdjacent; yy++) {
				t->table[yy * t->numColumns+x].rowHeight
					= t->table[y*t->numColumns+x].rowHeight
						/ (numAdjacent + 1);
				}
			    }
			y += numAdjacent;
			}
		    }
		/* assign uniform height to row*/
		for (y = 0; y < t->numRows; y++) {
			/* find max height of this row */
			maxHeightOfRow = 0;
			for (x = 0; x < t->numColumns; x++) {
				maxHeightOfRow = (maxHeightOfRow >
					t->table[y*t->numColumns+x].rowHeight) ?
					maxHeightOfRow :
					t->table[y*t->numColumns+x].rowHeight;
				}
			/* assign height */
			for (x = 0; x < t->numColumns; x++) {
				t->table[y * t->numColumns + x].rowHeight = 
					maxHeightOfRow;
				}
			}

		/* make sure all widths in a column are the same size */
		for (x = 0; x < t->numColumns; x++) {
		    maxWidthOfColumn = 0; 
		    /* find biggest Width for this column */
		    for (y = 0; y < t->numRows; y++) {
			maxWidthOfColumn = (maxWidthOfColumn > 
				t->table[y*t->numColumns+x].colWidth)?
				maxWidthOfColumn:
				t->table[y*t->numColumns+x].colWidth;
			}
		    /* make sure they are all the same */
		    for (y = 0; y < t->numRows; y++) {
			t->table[y*t->numColumns+x].colWidth = maxWidthOfColumn;
			}
		    }
		
		}


	/* calculate table width */
	t->width = 0;
	for (x = 0; x < t->numColumns; x++) {
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr,"colWidth for %d,0 = %d\n",x,t->table[x].colWidth);
		}
#endif
		t->width += t->table[x].colWidth;
		}
	/* calculate table height */
	t->height = 0;
	for (y = 0; y < t->numRows; y++) {
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			fprintf(stderr,"rowHeight for 0,%d = %d\n",y,t->table[y * t->numColumns].rowHeight);
		}
#endif
		t->height += t->table[y * t->numColumns].rowHeight;
		}
#ifndef DISABLE_TRACE

/*
	t->bwidth=t->width-t->borders;
	t->bheight=t->height-t->borders;
*/
	/*
	t->bwidth=t->width;
	t->bheight=t->height;
	*/
	t->width+=(t->borders*2);
	t->height+=(t->borders*2);

	if (htmlwTrace) {
		TableDump(t);
		fprintf(stderr,"TableCalculateDimensions(): table is %d x %d\n",
					t->width,t->height);
	}
#endif

} /* TableCalculateDimensions() */




/* expand colspans and rowspans in table */
/* return True if this routine did something */
static Boolean TableExpandFields(tableList, rowList, rowCount, columnCount)
List tableList;
List rowList;
int rowCount;
int *columnCount;
{
TableField *field;
List previousRow;		/* previous to current row */
TableField *aboveField;		/* field above current field */
TableField *fieldToTheLeft;	/* field to the left of current field */
Boolean expandedSomething;

	expandedSomething = False;
	/* check for and take care of previous rowspans */
	if (rowCount > 1) {
		/* get field above this one */
		previousRow = (List) ListGetIndexedEntry(tableList, 
					rowCount - 2);/*zero indexed*/
		aboveField =(TableField *)ListGetIndexedEntry(previousRow,
					*columnCount);
		if (aboveField) {
		    /*check if the above expands into this row*/
		    if (aboveField->rowSpan > 1) {
			if (!(field = NewTableField())) {
				return(0); /* out of memory */
				}

			field->rowSpan = aboveField->rowSpan - 1;
			field->contVert = True;
			field->contHoriz = aboveField->contHoriz;
		        field->header = aboveField->header;

			ListAddEntry(rowList, field);
			expandedSomething = True;
			(*columnCount)++;
			}
		    }
		}

	/* check for and take care of previous colspans */
	if (*columnCount) {
		/* get field above this one */
		fieldToTheLeft = (TableField *) ListTail(rowList);
		while(fieldToTheLeft->colSpan > 1) {

		    if (!(field = NewTableField())) {
			return(0); /* out of memory */
			}

		    field->colSpan = fieldToTheLeft->colSpan - 1;
		    field->contHoriz = True;
		    field->header = fieldToTheLeft->header;

		    if (fieldToTheLeft->rowSpan > 1) {
			field->rowSpan = fieldToTheLeft->rowSpan;
			}

		    ListAddEntry(rowList, field);
		    fieldToTheLeft = field;
		    (*columnCount)++;
		    expandedSomething = True;
		    }
		}

	return(expandedSomething);
}




/* set/get attributes for display from mark list for the field*/
static void TableFieldSetAttributes(hw,field,mptr)
HTMLWidget hw;
TableField *field;
struct mark_up **mptr;
{
struct mark_up *m;
/*
Boolean bold;
Boolean italic;
Boolean fixed;
*/


/* for right now, only get the text */

	field->font = hw->html.plain_font; /* default font */
	m = (*mptr)->next;
	while(m && (m->type != M_TABLE) && (m->type != M_TABLE_ROW) &&
		(m->type != M_TABLE_DATA) && (m->type != M_TABLE_HEADER)
		&& (m->type != M_NONE)) {
		if (!m->is_end) { /* effect is to only use the top on stack */
		    switch(m->type) {
			case M_ITALIC:
			case M_VARIABLE:
			case M_EMPHASIZED:
					field->font = hw->html.italic_font;
					break;
			case M_BOLD:
			case M_STRONG:
			case M_ANCHOR:
					field->font = hw->html.bold_font;
					break;
			case M_FIXED:
			case M_CODE:
			case M_SAMPLE:
			case M_KEYBOARD:
					field->font = hw->html.fixed_font;
					break;
			}
		    }
		m = m->next;
		}
	
	if (field->header) {
		field->font = hw->html.plainbold_font;
		}
	if (m->type == M_NONE){ /* text */
		field->type = F_TEXT;
		field->text = strdup(m->text);
		clean_text(field->text);
		}

}


TableInfo *MakeTable(hw, mptr, x, y)
HTMLWidget hw;
struct mark_up **mptr;
unsigned int x,y;
{
struct mark_up *m;
TableInfo *t;
TableField *field;
int columnCount;
int rowCount;
char *val;
List rowList; 			/* current row (List of TableFields)*/
List tableList;			/* list of Row Lists */
char *tptr;

	if (((*mptr)->type != M_TABLE) || ((*mptr)->is_end)) {
		return(0);
		}
	if (!(t = (TableInfo *) malloc(sizeof(TableInfo)))) {
		return(0);
		}
	t->numColumns = 0;
	t->numRows = 0;
	t->caption = (char *) 0;

	if (tptr=ParseMarkTag(((*mptr)->start),MT_TABLE,"BORDER")) {
		t->borders = atoi(tptr);
		}
	else {
		t->borders = 0;
		}
	tableList = ListCreate();
	rowList = ListCreate();
	ListAddEntry(tableList, rowList);
	columnCount = 0;
	rowCount=1;
	m = *mptr;
	field = (TableField *) 0;
	while (m && (!((m->type == M_TABLE) && (m->is_end)))) {

		if (m->type == M_CAPTION) {
			if (ParseMarkTag(m->start,MT_CAPTION,"top")) {
				t->captionAlignment = ALIGN_TOP;
				}
			else {
				t->captionAlignment = ALIGN_BOTTOM;
				}
			}

		else if ((m->type == M_TABLE_ROW)&&(!m->is_end)) {
			/* expand at end of row */
			while(TableExpandFields(tableList, rowList,
						rowCount, &columnCount));

			/* if: is this the first container <tr> or the 
			       separator */
			if (ListHead(ListHead(tableList))) {
				rowList = ListCreate();
				ListAddEntry(tableList,rowList);
				rowCount++;
				}
			columnCount = 0;
			/* expand cols at beginning of row */
			TableExpandFields(tableList, rowList,
						rowCount, &columnCount);
			}

		else if ((m->type == M_TABLE_DATA) && (!m->is_end))  {

			while(TableExpandFields(tableList, rowList,
						rowCount, &columnCount));

			if (!(field = NewTableField())) {
				return(0); /* out of memory */
				}
			field->header = False;

			/* check for colspan & rowspan */

			val = ParseMarkTag(m->start,MT_TABLE_DATA,"colspan");
			if (val) {
			    field->colSpan = atoi(val);
			    if ((field->colSpan > 100)||(field->colSpan < 1)){
				field->colSpan = 1;
				}
			    }

			val = ParseMarkTag(m->start,MT_TABLE_DATA,"rowspan");
			if (val) {
			    field->rowSpan = atoi(val);
			    if ((field->rowSpan > 100)||(field->rowSpan < 1)){
				field->rowSpan = 1;
				}
			    }

			/* check for alignment */
			val = ParseMarkTag(m->start,MT_TABLE_DATA,"align");
			if (caseless_equal(val,"left")) {
				field->alignment = ALIGN_LEFT;
				}
			else if (caseless_equal(val,"right")) {
				field->alignment = ALIGN_RIGHT;
				}
			else {
				field->alignment = ALIGN_CENTER;
				}
			TableFieldSetAttributes(hw,field,&m);

			ListAddEntry(rowList, field);
			columnCount++;
			}

		else if ((m->type == M_TABLE_HEADER) && (!m->is_end)) {

			while(TableExpandFields(tableList, rowList,
						rowCount, &columnCount));

			if (!(field = NewTableField())) {
				return(0); /*out of memory */
				}
			field->header = True;

			val = ParseMarkTag(m->start,MT_TABLE_HEADER,"colspan");
			if (val) {
				field->colSpan = atoi(val);
				if ((field->colSpan > 100)||(field->colSpan<1)){
					field->colSpan = 1;
					}
				}

			val = ParseMarkTag(m->start,MT_TABLE_HEADER,"rowspan");
			if (val) {
				field->rowSpan = atoi(val);
				if ((field->rowSpan > 100) || (field->rowSpan < 1)) {
					field->rowSpan = 1;
					}
				}

			/* check for alignment */
			val = ParseMarkTag(m->start,MT_TABLE_HEADER,"align");
			if (caseless_equal(val,"left")) {
				field->alignment = ALIGN_LEFT;
				}
			else if (caseless_equal(val,"right")) {
				field->alignment = ALIGN_RIGHT;
				}
			else {
				field->alignment = ALIGN_CENTER;
				}
			TableFieldSetAttributes(hw,field,&m);

			ListAddEntry(rowList, field);
			columnCount++;
			}
		else if (m->type == 0){ /* text */
			/* taken care of by TableFieldSetAttributes() above*/
			/* so ignore text now */
			}


		m = m->next;
		}
	*mptr = m; /* advance mark pointer to end of table */

	/* end of table has been hit, so wrap it up */
	/* clean up any at end of row */
	while(TableExpandFields(tableList, rowList, rowCount, &columnCount));
/*
	rowCount++;
	do {
		rowList = ListCreate();
		ListAddEntry(tableList,rowList);
		rowCount++;
		}
	while (TableExpandFields(tableList, ListTail(tableList),
			ListCount(tableList) - 1, 0));
	ListDeleteEntry(tableList, rowList);
	rowCount--;
*/

	if (!(TableCleanUp(t,tableList))) {
		return(0); /* out of memory */
		}

	/* free up memory from tableList since TableCleanUp
	   has already copied it into an array for speed. */
	rowList = (List) ListHead(tableList);
	while (rowList) {
		field = (TableField *) ListHead(rowList);
		while(field) {
			ListDeleteEntry(rowList,field);
			free(field);
			field = (TableField *) ListHead(rowList);
			}
		ListDeleteEntry(tableList,rowList);
		ListDestroy(rowList);
		rowList = (List) ListHead(tableList);
		}

	TableCalculateDimensions(hw,t,622);

#ifndef DISABLE_TRACE
	if (htmlwTrace) {
		TableDump(t);
	}
#endif

	return(t);

} /* MakeTable() */




TableDisplayField(hw,eptr,field,x,y,width,height)
HTMLWidget hw;
struct ele_rec *eptr;
TableField *field;
int x,y; /* field origin */
int width,height; /* space allowed for displaying */
{
int stringWidth; /* in pixels */
int placeX,placeY;
int lineHeight;
int baseLine;
int yy;

	if (field->type == F_NONE) { /* nothing to display in field */
		return;
		}

	if (field->type != F_TEXT) { /* routine only does text at this time */
		return;
		}

	/* adjust for aesthetic surounding space */
	width -= (2 * FIELD_BORDER_SPACE);
	x += FIELD_BORDER_SPACE;
	height -= (2 * FIELD_BORDER_SPACE);
	y += FIELD_BORDER_SPACE;


	lineHeight = FONTHEIGHT(field->font);
	baseLine = field->font->max_bounds.ascent;
	placeY = y + (height - (lineHeight * field->numLines))/2;
	for (yy = 0; yy < field->numLines; yy++) {
		stringWidth = XTextWidth(field->font,field->formattedText[yy],
					strlen(field->formattedText[yy]));

		switch(field->alignment) {
			case ALIGN_LEFT:
					placeX = x;
					break;
			case ALIGN_CENTER:
					placeX = x + (width - stringWidth)/2;
					break;
			case ALIGN_RIGHT:
					placeX = x + width - stringWidth;
					break;
			}
/*
		placeY = y + height/2 + 
	   			(field->font->max_bounds.ascent 
				- field->font->max_bounds.descent)/2;
*/

		XSetLineAttributes(XtDisplay(hw),hw->html.drawGC,1,LineSolid,
			CapNotLast,JoinMiter);
		XSetBackground(XtDisplay(hw), hw->html.drawGC, eptr->bg);
		XSetForeground(XtDisplay(hw), hw->html.drawGC, eptr->fg);
		XSetFont(XtDisplay(hw), hw->html.drawGC, field->font->fid);
		XDrawString(XtDisplay(hw), XtWindow(hw->html.view),
			hw->html.drawGC, placeX, placeY+baseLine,
			field->formattedText[yy],
			strlen(field->formattedText[yy]));

		placeY += lineHeight;
		}

	XSetLineAttributes(XtDisplay(hw),
			   hw->html.drawGC,
			   eptr->table_data->borders,
			   LineSolid,
			   CapNotLast,
			   JoinMiter);

} /* TableDisplayField() */



/* Find actual table field dimensions considering colspans & rowspans */
static void TableGetExpandedDimensions(t,xpos,ypos,
						expandWidth,expandHeight)
TableInfo *t;
int xpos,ypos; /* current field index */
int *expandWidth,*expandHeight; /* returned */
{
int x,y;
	x = xpos;
	y = ypos;

	*expandWidth = t->table[y * t->numColumns + x].colWidth;
	*expandHeight = t->table[y * t->numColumns + x].rowHeight;

	/* do width */
	x++;
	if (x < t->numColumns) {
		/* do width */
		while ((x < t->numColumns) && 
				t->table[y * t->numColumns + x].contHoriz) {
			(*expandWidth) += t->table[y * t->numColumns + x].colWidth;
			x++;
			}
		}

	x = xpos;
	y++;
	if (y < t->numRows) {
		/* do height */
		while ((y < t->numRows) && 
				t->table[y * t->numColumns + x].contVert) {
			(*expandHeight) += t->table[y * t->numColumns+x].rowHeight;
			y++;
			}
		}
	
	
}




/* display table */
void TableRefresh(hw,eptr)
HTMLWidget hw;
struct ele_rec *eptr;
{
int x,y; 		/* table origin */
register int xx,yy;
TableField *field;
int vertMarker,horizMarker;
int colWidth,rowHeight;
int expandedWidth,expandedHeight;

	if (eptr->table_data == NULL) {
		return;
		}

	x = eptr->x;
	y = eptr->y;

	x = x - hw->html.scroll_x;
	y = y - hw->html.scroll_y;

	XSetLineAttributes(XtDisplay(hw),
			   hw->html.drawGC,
			   eptr->table_data->borders,
			   LineSolid,
			   CapNotLast,
			   JoinMiter);
	XSetForeground(XtDisplay(hw), hw->html.drawGC, eptr->fg);
	XSetBackground(XtDisplay(hw), hw->html.drawGC, eptr->bg);

	if (eptr->table_data->borders){
	  /*
		XDrawRectangle(XtDisplay(hw), XtWindow(hw->html.view),
			hw->html.drawGC, 
			x+(eptr->table_data->borders/2),y+eptr->table_data->borders,
			eptr->table_data->bwidth,
			eptr->table_data->bheight);
			*/
/*
			eptr->table_data->height-1);
*/
		}

	field = eptr->table_data->table;
	horizMarker = y+eptr->table_data->borders;
	for (yy = 0; yy < eptr->table_data->numRows; yy++) {
		vertMarker = x+(eptr->table_data->borders/2);
		rowHeight = field->rowHeight;
		for (xx = 0; xx < eptr->table_data->numColumns; xx++) {
			colWidth = field->colWidth;

			/* draw field borders */
			if (eptr->table_data->borders){
			    if (!field->contVert) { /* draw above line */
				XDrawLine(XtDisplay(hw), 
					XtWindow(hw->html.view),
                        		hw->html.drawGC,
					/*hw->manager.bottom_shadow_GC,*/
					vertMarker, horizMarker,
					vertMarker + colWidth, horizMarker);
/*
				XDrawLine(XtDisplay(hw), 
					XtWindow(hw->html.view),
					hw->manager.top_shadow_GC,
					vertMarker, horizMarker+1,
					vertMarker + colWidth, horizMarker+1);
*/
				}
			    if (!field->contHoriz) { /* draw left side*/
				XDrawLine(XtDisplay(hw), 
					XtWindow(hw->html.view),
                        		hw->html.drawGC,
					vertMarker, horizMarker,
					vertMarker, horizMarker + rowHeight);
				}
			    }
			TableGetExpandedDimensions(eptr->table_data,
					xx,yy,&expandedWidth,&expandedHeight);
			/* fill in field */
			TableDisplayField(hw,
					  eptr,
					  field,
					  vertMarker,
					  horizMarker,
					  expandedWidth,
					  expandedHeight);

			vertMarker += colWidth;
			field++;
			}		

		horizMarker += rowHeight;
		}	

	XSetLineAttributes(XtDisplay(hw),
			   hw->html.drawGC,
			   1,
			   LineSolid,
			   CapNotLast,
			   JoinMiter);
}
