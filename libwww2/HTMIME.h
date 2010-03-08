/*  */

/*              MIME Parser                     HTMIME.h
**              -----------
**
**   The MIME parser stream presents a MIME document.
**
**
*/

#ifndef HTMIME_H
#define HTMIME_H

#include "HTStream.h"
#include "HTAnchor.h"

int HTMIME_get_header_length(HTStream *me);

extern HTStream * HTMIMEConvert PARAMS((HTPresentation * pres,
                                        HTParentAnchor * anchor,
                                        HTStream *              sink,
                                        HTFormat                format_in,
                                        int                     compressed));

#endif
