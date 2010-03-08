/*              Plain text object                       HTPlain.h
**              -----------------
**
**
*/

#ifndef HTPLAIN_H
#define HTPLAIN_H

#include "HTStream.h"
#include "HTAnchor.h"

extern HTStream* HTPlainPresent PARAMS((
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink, 
        HTFormat                format_in,
        int                     compressed));

#endif
