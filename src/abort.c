#include "abort.h"

/*
 * AbortTranslation --
 *      A fatal error occurred durring the translation. Print the abort
 *      code and then exit.
 *
 * ARGS
 *  ac              :   abort code [-i.e. TabAbortCode]
 *
 * RETURN
 *  void
 *
 * EXAMPLE USAGE
 *  // --Check the command line arguments.
 *  //   if there are not enough arguments, exit.
 *  if (argc != 2) {
 *      fprintf(stderr, "Usage: %s <ARGUMENT>\n", argv[0]);
 *      AbortTranslation(abortInvalidCommandLineArgs);
 *  }
 */
void AbortTranslation ( enum TAbortCode ac ) {     /*{{{*/
    fprintf(stderr, "**** Fatal translation error: %s\n", abortMsg[-ac]);
    exit ( ac );
} /*}}}*/

