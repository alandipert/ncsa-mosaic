/*  */

/*              HTML generator
*/

#ifndef HTMLGEN_H
#define HTMLGEN_H

#include "HTML.h"
#include "HTStream.h"

/* Subclass:
*/
/* extern WWW_CONST HTStructuredClass HTMLGeneration; */

/* Special Creation:
*/
extern HTStructured * HTMLGenerator PARAMS((HTStream * output));

extern HTStream * HTPlainToHTML PARAMS((
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink,
        HTFormat                format_in,
        int                     compressed));


#endif
