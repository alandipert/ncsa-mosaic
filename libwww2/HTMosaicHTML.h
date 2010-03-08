/*              MosaicHTML text object                       HTMosaicHTML.h
**              -----------------
**
**
*/

#ifndef HTMosaicHTML_H
#define HTMosaicHTML_H

#include "HTStream.h"
#include "HTAnchor.h"

extern HTStream* HTMosaicHTMLPresent PARAMS((
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink, 
        HTFormat                format_in,
        int                     compressed));

#endif
