
#ifndef _KCMS_H_
int KCMS_Enabled;
int KCMS_Return_Format;
#define _KCMS_H_
#else
extern int KCMS_Enabled;
extern int KCMS_Return_Format;
#endif

#define JPEG 0
#define JYCC 1
#define GIF 2


void CheckKCMS(void);
