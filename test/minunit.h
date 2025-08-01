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
//      [2] *ERROR* : <filename> [line: <linenumber>] : <testcase> - <errormessage>
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
//     #include <stdio.h>
//     #include "minunit.h"
//
//     int tests_run = 0;
//
//     int foo = 7;
//     int bar = 4;
//
//     static char * test_foo() {
//         /* Testing if int foo is equal to 7 */
//         mu_assert(foo == 7);
//         return 0;
//     }
//
//     static char * test_bar() {
//         /* Testing if int bar is equal to 5 */
//         mu_assert(bar == 5);
//         return 0;
//     }
//
//     static char * all_tests() {
//         /* set up a function to run all tests
//          * We call mu_run_test with a test case label, an error
//          * message, and the actual test case function to call.
//          */
//         mu_run_test("test_foo", "error, foo != 7", test_foo);
//         mu_run_test("test_bar", "error, bar != 5", test_bar);
//         return 0;
//     }
//
//     int main(int argc, char **argv) {
//         /* main function to run all tests and report the results */
//         char *result = all_tests();
//         if (result != 0) {
//             printf("%s\n", result);
//         }
//         else {
//             printf("ALL TESTS PASSED\n");
//         }
//         printf("Tests run: %d\n", tests_run);
//
//         return result != 0;
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
// HISTORY
// This originally came from a blog post from John Brewer but I don't
// think much is left from his original 4 lines of unit testing
// code/macros, because I've tinkered and refactored to better
// fit my needs.
//
// The blog post had a link of and the following APPENDEX note:
// [ https://jera.com/techinfo/jtns/jtn002 ]
//
// APPENDIX
// About the use of do {} while in the mu_run_test macro. This is a
// standard C idiom for writing a macro that contains multiple
// statements. For more information, see:
//
// [ http://www.eskimo.com/~scs/C-faq/q10.4.html ]
//===-------------------------------------------------------------===

#include <string.h>
#include <sys/time.h>

extern int tests_run;

#define MILLION 1000000L
#define mu_assert(X) if(!(X)){ errorlocation(__FILE__, __LINE__); } return "PASS";
#define errorlocation(zFile, iLine)                                            \
  do {                                                                         \
    char *message;                                                             \
    asprintf(&message, "*ERROR* : %s [line: %d]", zFile, iLine);               \
    return message;                                                            \
  } while (0)
#define mu_timer(test, iterations)                                             \
  do {                                                                         \
  long diftime;                                                                \
  struct itimerval ovalue, value;                                              \
  char *message;                                                               \
  ovalue.it_interval.tv_sec = 0;                                               \
  ovalue.it_interval.tv_usec = 0;                                              \
  ovalue.it_value.tv_sec = MILLION; /* a large number */                       \
  ovalue.it_value.tv_usec = 0;                                                 \
  if (setitimer(ITIMER_VIRTUAL, &ovalue, NULL) == -1) {                        \
    asprintf(&message, "Failed to set virtural timer");                        \
    return message;                                                            \
  }                                                                            \
  for (unsigned n = 0; n < iterations; ++n)                                    \
      test;                                                                    \
  if (getitimer(ITIMER_VIRTUAL, &value) == -1) {                               \
    asprintf(&message, "Failed to get virtual timer");                         \
    return message;                                                            \
  }                                                                            \
  diftime = MILLION*(ovalue.it_value.tv_sec - value.it_value.tv_sec) +         \
              ovalue.it_value.tv_usec - value.it_value.tv_usec;                \
  asprintf(&message, "%ld (microseconds) [%f (seconds)] / %d (iterations)",    \
         diftime, diftime/(double)MILLION, iterations);                        \
    return message;                                                            \
  } while (0)
#define mu_run_test(label, errmsg, test)                                       \
  do {                                                                         \
    char *message;                                                             \
    char *ret = test();                                                        \
    ++tests_run;                                                               \
    asprintf(&message, "[%d] %s : %s", tests_run, ret, label);                 \
    if(memcmp("*ERROR*", ret, 7) == 0)                                         \
       asprintf(&message, "[%d] %s : %s - \"%s\"", tests_run, ret, label, errmsg);\
    printf("%s\n", message);                                                   \
    free(message);                                                             \
  } while (0)
