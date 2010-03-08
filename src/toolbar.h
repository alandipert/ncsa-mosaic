/* Toolbar Stuff -- BJS */

struct toolbar 
{
    Widget w;
    int gray;
};

#define BTN_PREV 0
#define BTN_NEXT 1
#define BTN_PTHR 14
#define BTN_PART 15
#define BTN_NART 16
#define BTN_NTHR 17
#define BTN_POST 18
#define BTN_FOLLOW 19
#define BTN_COUNT 23

struct tool {
    char *text;            /* button text */
    char *long_text;       /* long button text */
    char *label;           /* tracker label */
    int action;            /* mo_* for menubar dispatcher */
    Pixmap *image;         /* the icon */
    Pixmap *greyimage;     /* the greyed icon or NULL */
    int toolset;           /* toolset it belongs too (0=always on) */
    int kioskok;           /* allowed in kiosk mode */
    char *unused;
    
};

int mo_tool_state(struct toolbar *t,int state, int index);




