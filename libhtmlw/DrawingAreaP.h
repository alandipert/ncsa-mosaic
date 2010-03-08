
/* DrawingArea widget private definitions */

#ifndef _XawDrawingAreaP_h
#define _XawDrawingAreaP_h

#include "DrawingArea.h"
#include <X11/ConstrainP.h>

#define XtInheritLayout ((Boolean (*)())_XtInherit)

typedef struct {
  int dummy;
} DrawingAreaClassPart;


typedef struct _DrawingAreaClassRec {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    DrawingAreaClassPart drawingArea_class;
} DrawingAreaClassRec;

extern DrawingAreaClassRec drawingAreaClassRec;

typedef struct _DrawingAreaPart {
  int dummy;
} DrawingAreaPart;

typedef struct _DrawingAreaRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    DrawingAreaPart	drawingArea;
} DrawingAreaRec;

typedef struct _DrawingAreaConstraintsPart {
  int dummy;
} DrawingAreaConstraintsPart;

typedef struct _DrawingAreaConstraintsRec {
  int dummy;
} DrawingAreaConstraintsRec, *DrawingAreaConstraints;

#endif /* _XawDrawingAreaP_h */
