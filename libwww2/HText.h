/*                                                           Rich Hypertext object for libWWW
                                  RICH HYPERTEXT OBJECT
                                             
 */

/*

   This is the C interface to the Objective-C (or whatever) HyperText class.
   
 */
#ifndef HTEXT_H
#define HTEXT_H
#include "HTAnchor.h"
#include "HTStream.h"

#ifdef SHORT_NAMES
#define HTMainText                      HTMaText
#define HText_new                       HTHTNew
#define HText_free                      HTHTFree
#define HText_beginAppend               HTHTBeAp
#define HText_endAppend                 HTHTEnAp
#define HText_appendCharacter           HTHTApCh
#define HText_appendText                HTHTApTe
#define HText_appendParagraph           HTHTApPa
#define HText_beginAnchor               HTHTBeAn
#define HText_endAnchor                 HTHTEnAn
#define HText_dump                      HTHTDump
#define HText_nodeAnchor                HTHTNoAn
#define HText_select                    HTHTSele
#define HText_selectAnchor              HTHTSeAn
#define HText_replaceSel                HTHTRepl
#define HText_applyToSimilar            HTHTApTo
#define HText_unlinkSelection           HTHTUnSe
#define HText_linkSelTo                 HTHTLiSe
#define HText_referenceSelected         HTHTRefS
#endif

#ifndef THINK_C
#ifndef HyperText               /* Objective C version defined HyperText */
typedef struct _HText HText;    /* Normal Library */
#endif
#else
class CHyperText;               /* Mac Think-C browser hook */
typedef CHyperText HText;
#endif

extern HText * HTMainText;              /* Pointer to current main text */

/*                      Creation and deletion
**
**      Create hypertext object                                 HText_new
*/
 extern HText * HText_new PARAMS(());

/*      Free hypertext object                                   HText_free
*/
extern void     HText_free PARAMS((HText * me));


/*                      Object Building methods
**                      -----------------------
**
**      These are used by a parser to build the text in an object
**      HText_beginAppend must be called, then any combination of other
**      append calls, then HText_endAppend. This allows optimised
**      handling using buffers and caches which are flushed at the end.
*/
extern void HText_beginAppend PARAMS((HText * text));

extern void HText_endAppend PARAMS((HText * text));
extern void HText_doAbort PARAMS((HText * text));
extern void HText_clearOutForNewContents PARAMS((HText * text));

/*      Add one character
*/
extern void HText_appendCharacter PARAMS((HText * text, char ch));

/*      Add a zero-terminated string
*/
extern void HText_appendText PARAMS((HText * text, WWW_CONST char * str));
/*      Add a block.
*/
extern void HText_appendBlock PARAMS((HText * text, WWW_CONST char * str, int len));

/*      New Paragraph
*/
extern void HText_appendParagraph PARAMS((HText * text));

/*      Start/end sensitive text
**
** The anchor object is created and passed to HText_beginAnchor.
** The senstive text is added to the text object, and then HText_endAnchor
** is called. Anchors may not be nested.
*/

extern void HText_beginAnchor PARAMS((HText * text, char * anc));
extern void HText_endAnchor PARAMS((HText * text));


/*      Dump diagnostics to stderr
*/
extern void HText_dump PARAMS((HText * me));

extern char *HText_getText (HText *me);
extern int HText_getTextLength (HText *me);
extern char **HText_getPtrToText (HText *me);

/*              Browsing functions
**              ------------------
*/

/*      Bring to front and highlight it
*/

extern BOOL HText_select PARAMS((HText * text));


#endif /* HTEXT_H */
/*

   end  */
