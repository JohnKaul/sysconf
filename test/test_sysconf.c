//===---------------------------------------------------*- C -*---===
// File Last Updated: 07.04.25 19:44:38
//
//: test-one_sysconf.c
//
// BY  : John Kaul
//
// DESCRIPTION
// This is a series of tests for the tools in `sysconf()`.
//===-------------------------------------------------------------===

#include "minunit.h"
#include "parse-config.h"
#include "print-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int tests_run = 0;

//** TEST FUNCTIONS **//
/**
 *: test_make_argv
 * @brief               Tests that we can make an array of pointers.
 *
 * PASS:    if array count = 2.
 *          mu_run_test("test_make_argv", "error, arg_count != 1", test_make_argv);
 */
static char * test_make_argv() {    /* {{{ */
  int arg_count = 0;
  char delimiters[] = "=";
  char *arg_string = NULL;                              /* Used to store the argument string. */
  arg_string = "key=value";
  char **arg_array;                                     /* Used to store the argument
                                                           string pointer array */

  arg_count = make_argv(arg_string, delimiters, &arg_array);

  mu_assert(arg_count == 2);
  return 0;
}
/* }}} */

/**
 *: test_contains
 * @brief               makes an array from a string, and checks to
 *                      see if the resulted array contains a value.
 *
 * PASS:    if `contains()` returns 1 when cheking for a value.
 *          mu_run_test("test_contains", "error, array does not contain \"value\"", test_contains);
 */
static char * test_contains() {     /* {{{ */
  int arg_count = 0;
  char delimiters[] = "=";
  char *arg_string = NULL;                              /* Used to store the argument string. */
  arg_string = "key.sub=value.sub";
  char **arg_array;                                     /* Used to store the argument
                                                           string pointer array */
  arg_count = make_argv(arg_string, delimiters, &arg_array);

  mu_assert(contains(arg_array, arg_count, "value.sub") == 1);

  return 0;
}
/* }}} */

/**
 *: test_add_to_array
 * @brief               tests to see if `add_to_array()` will add a
 *                      pointer to an array of pointers.
 *
 * PASS:        if the value to add gets appended to array.
 *              mu_run_test("test_add_to_array", "error, array does not contain \"value2\"", test_add_to_array);
 */
static char * test_add_to_array() {     /* {{{ */
  int arg_count = 0;
  char delimiters[] = "=";
  char *arg_string = NULL;                              /* Used to store the argument string. */
  arg_string = "key=value";
  char **arg_array;                                     /* Used to store the argument
                                                           string pointer array */
  arg_count = make_argv(arg_string, delimiters, &arg_array);

  arg_count = add_to_array(&arg_array, arg_count, "value2");

  mu_assert(contains(arg_array, arg_count, "value2") == 1);

  return 0;
}
/* }}} */

/**
 *: test_assemble_strings
 * @brief               tests to see if one char array gets assembled
 *                      from several.
 *
 * PASS:        if one string is returned.
 *              mu_run_test("test_asseble_strings", "error, assembled string does not equal \"value\"", test_assemble_strings);
 */
static char * test_assemble_strings() {     /* {{{ */
  int arg_count = 0;
  char delimiters[] = "=";
  char *arg_string = NULL;                              /* Used to store the argument string. */
  arg_string = "key=value value2";
  char **arg_array;                                     /* Used to store the argument
                                                           string pointer array */
  // split string into tokens.
  arg_count = make_argv(arg_string, delimiters, &arg_array);

  // Add a string to the array.
  arg_count = add_to_array(&arg_array, arg_count, "value3");

  // reassemble the strings.
  char *assembled_string;
  assembled_string = assemble_strings(arg_array, arg_count);

  // assert the assembled string's value.
  mu_assert(strncmp(assembled_string, "value value2 value3", 19) == 0);

  return 0;
}
/* }}} */

//** TEST RUNNER **//
// This function just runs all test functions.
static char * all_tests() {
    /* set up a function to run all tests
     * We call mu_run_test with a test case label, an error
     * message, and the actual test case function to call.
     */
    mu_run_test("test_make_argv", "error, arg_count != 1", test_make_argv);
    mu_run_test("test_contains", "error, array does not contain \"value\"", test_contains);
    mu_run_test("test_add_to_array", "error, array does not contain \"value2\"", test_add_to_array);
    mu_run_test("test_asseble_strings", "error, assembled string does not match test string", test_assemble_strings);
    return 0;
}

int main() {
  char *result = all_tests();
  printf("Tests run: %d\n", tests_run);
  return result != 0;
}

// int main() {
//     /* main function to call `all_tests()` and report the results */
//     char *result = all_tests();
//     if (result != 0) {
//         printf("%s\n", result);
//     }
//     else {
//         printf("ALL TESTS PASSED\n");
//     }
//     printf("Tests run: %d\n", tests_run);
//
//     return result != 0;
// }
