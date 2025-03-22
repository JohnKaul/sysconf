#include "abort.h"

/**
 *: AbortTranslation
 * @brief A fatal error occurred durring the translation. Print the abort
 *        code and then exit.
 *
 * @ param ac   abort code [-i.e. TabAbortCode]
 *
 * @return void
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
