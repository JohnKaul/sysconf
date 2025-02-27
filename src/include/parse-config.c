#include "parse-config.h"
#include "abort.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  /* for isstring() */

/**
 *: make_argv
 *  @brief              This function will take a string and parse it
 *                      based on a delimeter and store the string
 *                      pointers into an array for easier access.
 *
 * @param input_string  A string to parse
 * @param delimiters    A string of delimiters.
 * @param ***argvp      An array to store the array into.
 *
 * @return              The number of elements in the array.
 */
int make_argv(const char *input_string, const char *delimiters, char ***argvp) {  /*{{{*/
  int tokens = 0;
  // Construct a `stash` string and eliminate any prefix delimiters.
  const char *stash = input_string + strspn(input_string, delimiters);

  // duplicate the `stash` string into another string to count the
  // number of tokens.
  char *tokenized_string = strndup(stash, strlen(stash));
  if (tokenized_string == NULL) {
    // If there was an error allocating memory for the
    // `tokenized_string`, cleanup.
    **argvp = NULL;
    return -1;
  }

  // Memory allocation
  // ----------------------------------------------------------------

  // Pop the first token from the string.
  char *saveptr;
  if(strtok_r(tokenized_string, delimiters, &saveptr) == NULL) {
    // if error, cleanup.
    free(tokenized_string);
    **argvp = NULL;
    return -1;
  }

  // We have at least one token sofar, keep tokenizing and count the
  // number of tokens in the string.
  for (tokens = 1; (strtok_r(NULL, delimiters, &saveptr) != NULL); tokens++)
    ;

  // Allocate memory for the string array based on the number of
  // tokens in the string.
  if ((*argvp = calloc(tokens + 1, sizeof(char *))) == NULL) {
    free(tokenized_string);
    **argvp = NULL;
    return -1;
  }

  // Array construction
  // ----------------------------------------------------------------

  // Copy the `stash` string back into `tokenized_string`.
  strncpy(tokenized_string, stash, strlen(stash));
  char *token = strtok_r(tokenized_string, delimiters, &saveptr);

  // Foreach token, copy the pointer into our array.
  for (int i = 0; i < tokens; i++) {
    // NOTE:
    //      The `strndup` function copies the string into the array so
    //      that the pointers associated with `tokenized_string` can
    //      be `freed` at the end of this function.
    if ((*(*argvp + i) = strndup(token, strlen(token))) == NULL) {
      // If there was an error allocating memory for the token,
      // then free all the memory allocated (back out).
      for (int j = 0; j < i; j++) {
        free((*argvp)[j]);
      }
      free(*argvp);
      free(tokenized_string);
      **argvp = NULL;
      return -1;
    }
    token = strtok_r(NULL, delimiters, &saveptr);
  }

  // Set the last element to NULL to act as a sentinel
  (*argvp)[tokens] = NULL;  free(tokenized_string);

  return tokens;
}
/*}}}*/

/**
 *: find_config_item
 * @brief               A function to find a configuration
 *                      value given a name.
 *
 * @param config        A pointer to the configuration data.
 * @param count         The number of configuration entries.
 * @param name          The name of the value to retrieve.
 *
 * @return config_t*    A pointer to the parsed configuration data, or
 *                      NULL on error.
 */
config_t* find_config_item(config_t* config, const char* name, int count) {     /*{{{*/
    for (int i = 0; i < count; i++) {
        if (strcmp(config[i].values[0], name) == 0) {
            return &config[i];
        }
    }
    return NULL;
}
/*}}}*/

/**
 *: parse_config
 *  @brief  Parse the configuration file and store the data in a
 *          config_t array.
 *
 * @param filename      The name of the configuration file.
 * @param count         A pointer to store the number of configuration
 *                      entries.
 * @param delimiters    A char array of delimiters for tokenization.
 *
 * @return config_t*    A pointer to the parsed configuration data, or
 *                      NULL on error.
 */
config_t* parse_config(const char* filename, int* count, char *delimiters) {      /*{{{*/
    // Open the configuration file
    FILE* file = fopen(filename, "r");
    if (!file)
        return NULL;

    int lines = 0;                                      /* Count the number of lines */
    char buffer[1024];                                  /* buffer stores the line */
    char **argv;
    int argc;

    // Parse the configuration file
    while (fgets(buffer, sizeof(buffer), file)) {
      char *str = buffer;
      while (isspace(*str)) str++;
      if (*str == '\0' || \
          *str == '#' || \
          *str == ';' || \
          *str == '/' || \
          *str == '*' || \
          *str == '\n'|| \
          *str == '[') {
        continue;
      }

      lines++;
    }

    // Allocate memory for the configuration data
    config_t* config = malloc((lines * 1) * sizeof(config_t));
    if (!config) {
        fclose(file);
        return NULL;
    }

    // Reset the file pointer to the beginning of the file
    rewind(file);

    // Parse the configuration file
    int i = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
      char *str = buffer;
      while (isspace(*str)) str++;
      if (*str == '\0' || \
          *str == '#' || \
          *str == ';' || \
          *str == '/' || \
          *str == '*' || \
          *str == '\n'|| \
          *str == '[') {
        continue;
      }

      argc = make_argv(str, delimiters, &argv);
//:~        argc = makeargv(str, delimiters, &argv);

      if (argc > 0) {
        config[i].values = argv;
        config[i].value_count = argc;
      }
      i++;
    }

    // Set the last element to NULL to act as a sentinel
    (argv)[i] = NULL;

    *count = i;
    fclose(file);
    return config;
}
/*}}}*/

/**
 *: get_value
 * @brief               Function to find a configuration item by name.
 *
 * @param config        A pointer to the configuration data.
 * @param count         The number of configuration entries.
 * @param name          The name of the value to retrieve.
 *
 * @return char**       An array of char arrays.
 */
char **get_value(config_t* config, int count, const char* name) {        /*{{{*/
    for (int i = 0; i < count; i++) {
      if (strncmp(config[i].values[0], name, strlen(config[i].values[0])) == 0) {
        return config[i].values;
      }
  }
    return NULL;
}
/*}}}*/

/**
 *: print_config_item
 * @brief               Function to print a configuration item's value to STDOUT.
 *
 * This function will only print the first element in the array of
 * char arrays and is only ment as a simple printer function. Because
 * values in the config files can be complex (multile values seperated
 * by spaces) and our parser will store those values as array
 * elements, the `get_value()` function to get an array back and
 * iterate it as needed for finer grained control.
 *
 * @param config        A pointer to the configuration data.
 * @param count         The number of configuration entries.
 * @param name          The name of the value to retrieve.
 */
void print_config_item(config_t* config, int count, const char* name) {     /*{{{*/
  for (int i = 0; i < count; i++) {
    if (strcmp(config[i].values[0], name) == 0) {
//:~        printf("%s =\t %s\n", name, config[i].values[1]);
      printf("%s\n", config[i].values[1]);
    }
  }
}
/*}}}*/

/**
 *: free_config
 * @brief               Free the allocated memory for the configuration data.
 *
 * @param config        A pointer to the configuration data.
 * @param count         The number of configuration entries.
 */
void free_config(config_t *config, int count) {/*{{{*/
    for (int i = 0; i < count; i++) {
      free(config[i].values);
    }
//:~      while(count > 0) {
//:~        free(config[count--].values);
//:~      }
}/*}}}*/
