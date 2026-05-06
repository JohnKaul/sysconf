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
#include <string.h>

int tests_run = 0;

//** TEST FUNCTIONS **//
/**
 *: test_make_argv
 * @brief               Tests that we can make an array of pointers.
 *
 * PASS:    if array count = 2.
 *          mu_run_test("test_make_argv", "error, arg_count != 1", test_make_argv);
 */
static char * test_make_argv() {
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

/**
 *: test_contains
 * @brief               makes an array from a string, and checks to
 *                      see if the resulted array contains a value.
 *
 * PASS:    if `contains()` returns 1 when cheking for a value.
 *          mu_run_test("test_contains", "error, array does not contain \"value\"", test_contains);
 */
static char * test_contains() {
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

/**
 *: test_contains_exact
 * @brief               makes an array from a string, and checks to
 *                      see if the resulted array contains an exact value.
 *
 * PASS:    if `contains()` returns 1 when a value is found.
 *          mu_run_test("test_contains", "error, array does not contain \"value\"", test_contains);
 */
static char * test_contains_exact() {
  int arg_count = 0;
  char delimiters[] = "=";
  char *arg_string = NULL;                              /* Used to store the argument string. */
  arg_string = "key.sub=value.sub.sub";
  char **arg_array;                                     /* Used to store the argument
                                                           string pointer array */
  arg_count = make_argv(arg_string, delimiters, &arg_array);

  mu_assert(contains(arg_array, arg_count, "value.sub") == 0);
  /* `contains()` returns 0 = Not found. 1 = found. So, we do not want
   * a match for a substring of a string.
   */

  arg_string = "key.sub=value";
  mu_assert(contains(arg_array, arg_count, "value.sub") == 0);

  return 0;
}

/**
 *: test_add_to_array
 * @brief               tests to see if `add_to_array()` will add a
 *                      pointer to an array of pointers.
 *
 * PASS:        if the value to add gets appended to array.
 *              mu_run_test("test_add_to_array", "error, array does not contain \"value2\"", test_add_to_array);
 */
static char * test_add_to_array() {
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

/**
 *: test_count_tokens
 * @brief               Tests counting tokens in a string.
 *
 * PASS:    if token count matches expected value.
 */
static char * test_count_tokens() {     /* {{{ */
  int token_count = count_tokens("key=value", "=");
  mu_assert(token_count == 2);
  
  int token_count2 = count_tokens("a b c", " ");
  mu_assert(token_count2 == 3);
  
  int token_count3 = count_tokens("", " ");
  mu_assert(token_count3 == 0);
  
  return 0;
}
/* }}} */

/**
 *: test_parse_config
 * @brief               Tests parsing a configuration file.
 *
 * PASS:    if config is parsed and count is set correctly.
 */
static char * test_parse_config() {     /* {{{ */
  int count = 0;
  char delimiters[] = "= \n";
  
  config_t* config = parse_config("test/test.conf", &count, delimiters);
  
  mu_assert(config != NULL);
  mu_assert(count > 0);
  
  free_config(config, count);
  free(config);
  
  return 0;
}
/* }}} */

/**
 *: test_get_value
 * @brief               Tests retrieving a value from config.
 *
 * PASS:    if the correct value is returned.
 */
static char * test_get_value() {        /* {{{ */
  int arg_count = 0;
  char delimiters[] = "=";
  char *arg_string = "key=value1 value2";
  char **arg_array;
  
  arg_count = make_argv(arg_string, delimiters, &arg_array);
  
  config_t config[1];
  config[0].values = arg_array;
  config[0].value_count = arg_count;
  
  char **result = get_value(config, 1, "key");
  mu_assert(result != NULL);
  mu_assert(strcmp(result[0], "key") == 0);
  mu_assert(strcmp(result[1], "value1") == 0);
  
  return 0;
}
/* }}} */

/**
 *: test_find_config_item
 * @brief               Tests finding a config item by name.
 *
 * PASS:    if the correct config item is found.
 */
static char * test_find_config_item() {/* {{{ */
  char *v1[] = {"setting1", "val1", NULL};
  char *v2[] = {"setting2", "val2", NULL};
  char *v3[] = {"setting3", "val3", NULL};
  
  config_t config[3];
  config[0].values = v1;
  config[0].value_count = 2;
  config[1].values = v2;
  config[1].value_count = 2;
  config[2].values = v3;
  config[2].value_count = 2;
  
  config_t *found = find_config_item(config, "setting2", 3);
  mu_assert(found != NULL);
  mu_assert(strcmp(found->values[0], "setting2") == 0);
  
  config_t *not_found = find_config_item(config, "nonexistent", 3);
  mu_assert(not_found == NULL);
  
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
static char * test_assemble_strings() {
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

//** TEST RUNNER **//
// This function just runs all test functions.
static char * all_tests() {
    /* set up a function to run all tests
     * We call mu_run_test with a test case label, an error
     * message, and the actual test case function to call.
     */
    mu_run_test("test_make_argv", "error, arg_count != 1", test_make_argv);
    mu_run_test("test_contains", "error, array does not contain \"value\"", test_contains);
    mu_run_test("test_contains_exact", "error, contains() found a match for: \"value.sub\" in: \"key.sub=value.sub.sub\"", test_contains_exact);
    mu_run_test("test_add_to_array", "error, array does not contain \"value2\"", test_add_to_array);
    mu_run_test("test_count_tokens", "error, token count mismatch", test_count_tokens);
    mu_run_test("test_parse_config", "error, failed to parse config", test_parse_config);
    mu_run_test("test_get_value", "error, failed to get config value", test_get_value);
    mu_run_test("test_find_config_item", "error, failed to find config item", test_find_config_item);
    mu_run_test("test_asseble_strings", "error, assembled string does not match test string", test_assemble_strings);
    return 0;
}

int main() {
  char *result = all_tests();
  printf("1..%d\n", tests_run);
  return result != 0;
}
