/*  */

/*              Initialisation module                   HTInit.h
**
**      This module resisters all the plug&play software modules which
**      will be used in the program.  This is for a browser.
**
**      To override this, just copy it and link in your version
**      befoe you link with the library.
*/

#include "HTUtils.h"

extern void HTReInit NOPARAMS;
extern void HTFormatInit NOPARAMS;
extern void HTFileInit NOPARAMS;

extern int HTLoadExtensionsConfigFile (char *fn);
