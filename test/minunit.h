//===---------------------------------------------------*- C -*---===
// BRIEF
// MinUnit is an extremely simple unit testing framework written in C.
//
// SYNOPSIS
// A MinUnit test case is a simple function, that returns 0 (null), and
// calls `mu_assert()` to check and see if a certain condition passes.
//      ...>%
//      int foo = 7;
//      static char * test_foo() {
//        /* testing if int foo is equal to 7 */
//          mu_assert(foo == 7);
//          return 0;
//      }
//      ...>%
//
// If `mu_assert()` finds an error in the condition it returns the
// filename and the linenumber where the assertion failed.
//
// SAMPLE OUTPUT:
//      [1] PASS : <testcase>
//      [2] FAIL : <filename> [line: <linenumber>] : <testcase> - <errormessage>
//      Tests run: 2
//
// EXAMPLE 1 (two tests - one pass, one fail):
//
//     //===---------------------------------------------------*- C -*---===
//     //: minunit_example
//     //
//     // DATE: January 1 1900
//     // BY  : John Kaul
//     //
//     // DESCRIPTION
//     // This test file contains several simple tests.
//     // `testfoo()`   - tests if int foo is equal to 7
//     // `testbar()`   - tests if int bar is equal to 5
//     //
//     // The function `testfoo()` should pass.
//     // The function `testbar()` should fail.
//     // This is a sample unit test.
//     //===-------------------------------------------------------------===
//      #include <stdio.h>
//      #include "minunit.h"
//
//      int strchk(char *P, char *T) {
//        if(!P || !T)
//          return 0;
//        unsigned len = strlen(P) - 1;
//        char *endP = P + len;
//        char *endT = T + len;
//
//        for(;
//            P < endP && T < endT;
//            ++P, ++T, --endT, --endP)
//          if(*P != *T || *endP != *endT)
//            return 0;
//        return 1;
//      }
//
//      int tests_run = 0;
//
//      static char * time_strchk() {
//        mu_timer(strchk("test", "testing"), 777777);
//        return 0;
//      }
//
//      static char * all_times() {
//        mu_run_test("time-test_strchck", NULL, time_strchk);
//        return 0;
//      }
//
//      int main(int argc, char **argv) {
//        char *result = all_times();
//        printf("Tests run: %d\n", tests_run);
//        return result != 0;
//      }
//     // SECOND (ALTERNATE) example with the revised macros.
//     /**
//      * @file tests.c
//      * @brief Example usage of the minunit-style macros
//      *
//      * Logic:
//      *  - Test functions return NULL on success or a malloc'd error string on failure.
//      *  - all_tests runs tests and returns the first failure (caller must free it).
//      */
//
//     #include <stdio.h>
//     #include <stdlib.h>
//     #include "minunit.h"
//
//     int tests_run = 0;
//
//     int foo = 7;
//     int bar = 4;
//
//     static char * test_foo() {
//         /* If assertion fails, mu_assert returns malloc'd error message */
//         mu_assert(foo == 7);
//         return NULL;
//     }
//
//     static char * test_bar() {
//         mu_assert(bar == 5);
//         return NULL;
//     }
//
//     static char * all_tests() {
//         char *ret;
//
//         /* Run test_foo and propagate any error back to caller */
//         ret = test_foo();
//         ++tests_run;
//         if (ret) return ret;
//
//         /* Run test_bar and propagate any error back to caller */
//         ret = test_bar();
//         ++tests_run;
//         if (ret) return ret;
//
//         return NULL;
//     }
//
//     int main(int argc, char **argv) {
//         char *result = all_tests();
//         if (result != NULL) {
//             printf("%s\n", result);
//             free(result); /* free malloc'd error string */
//         } else {
//             printf("ALL TESTS PASSED\n");
//         }
//         printf("Tests run: %d\n", tests_run);
//         return result != NULL;
//     }
//
// There is also a timing function (macro) built in which should allow
// simple profiling abilities. -i.e. a way to test a function's speed
// compared to another's.
//
// EXAMPLE 2 (a time test):
//
//     //===---------------------------------------------------*- C -*---===
//     //: minunit_example
//     //
//     // DATE: January 1 1900
//     // BY  : John Kaul
//     //
//     // DESCRIPTION
//     // This test file contains several simple time tests.
//     //
//     // I want to test the speed of my fancy string check function
//     // against the std `memcmp()` function and see which is faster
//     // (who is smarter, me--a weekend programmer--or a bunch of
//     // very experienced seasoned professional programmers who
//     // probably have doctorates in computer science. ...I think
//     // it's me.
//     //===-------------------------------------------------------------===
//
//      #include <stdio.h>
//      #include "minunit.h"
//
//      int strchk(char *P, char *T) {
//        if(!P || !T)
//          return 0;
//        unsigned len = strlen(P) - 1;
//        char *endP = P + len;
//        char *endT = T + len;
//
//        for(;
//            P < endP && T < endT;
//            ++P, ++T, --endT, --endP)
//          if(*P != *T || *endP != *endT)
//            return 0;
//        return 1;
//      }
//
//      int tests_run = 0;
//
//      static char * time_strchk() {
//        mu_timer(strchk("test", "testing"), 777777);
//        return 0;
//      }
//
//      static char * all_times() {
//        mu_run_test("time-test_strchck", NULL, time_strchk);
//        return 0;
//      }
//
//      int main(int argc, char **argv) {
//        char *result = all_times();
//        printf("Tests run: %d\n", tests_run);
//        return result != 0;
//      }
//
// LICENSE
// You may use the code for any purpose, with the understanding that
// it comes with NO WARRANTY.
//
// HISTORY
// This originally came from a blog post from John Brewer but I don't
// think much is left from his original 4 lines of unit testing
// code/macros, because I've tinkered and refactored slightly to better
// fit my needs.
//
// [ https://jera.com/techinfo/jtns/jtn002 ]
//
// APPENDIX
// About the use of do {} while in the mu_run_test macro. This is a
// standard C idiom for writing a macro that contains multiple
// statements. For more information, see:
//
// [ http://www.eskimo.com/~scs/C-faq/q10.4.html ]
//===-------------------------------------------------------------===

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

extern int tests_run;

#define mu_assert(X)                                                \
  do {                                                              \
    if (!(X)) {                                                     \
      char *msg = NULL;                                             \
      asprintf(&msg, "FAIL : %s [line: %d]", __FILE__, __LINE__);   \
      return msg;                                                   \
    }                                                               \
  } while (0)

#define mu_run_test(label, errmsg, test)                            \
  do {                                                              \
    char *message = NULL;                                           \
    char *ret = test();                                             \
    ++tests_run;                                                    \
    if (ret) {                                                      \
      asprintf(&message, "[%d] %s : %s - \"%s\"",                   \
               tests_run, ret, label, errmsg);                      \
      printf("%s\n", message);                                      \
      free(message);       /* Free malloc'd msg from asprintf() */  \
      free(ret);           /* Free malloc'd ret err from test() */  \
    } else {                                                        \
      asprintf(&message, "[%d] PASS : %s", tests_run, label);       \
      printf("%s\n", message);                                      \
      free(message);       /* Free malloc'd msg from asprintf() */  \
    }                                                               \
  } while (0)
