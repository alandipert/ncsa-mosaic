#ifndef _XawDrawingArea_h
#define _XawDrawingArea_h

/* #include <X11/Constraint.h> */
/***********************************************************************
 *
 * DrawingArea Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 defaultDistance     Thickness		int		4
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	computed at realize
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	computed at realize
 x		     Position		Position	0
 y		     Position		Position	0

*/


typedef struct _DrawingAreaClassRec	*DrawingAreaWidgetClass;
typedef struct _DrawingAreaRec		*DrawingAreaWidget;

extern WidgetClass drawingAreaWidgetClass;
 
#endif /* _XawDrawingArea_h */
