#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/**
 *: TabAbortCode --
 *      Enums for standard errors.
 */
enum TAbortCode {     /*{{{*/
  abortInvalidCommandLineArgs   = -1,
  abortRuntimeError             = -2,
  abortUnimplementedFeature     = -3
};/*}}}*/

/**
 *: Abort Messages --
 *      Keyed to ennumeration type TAbortCode
 */
const static char *abortMsg[] = {     /*{{{*/
    NULL,
    "Invalid command line arguments",
    "Runtime error",
    "Unimplemented feature",
};
/*}}}*/

/**
 *: AbortTranslation --
 *      A fatal error occurred durring the translation. Print the abort
 *      code and then exit.
 */
void AbortTranslation ( enum TAbortCode ac );
