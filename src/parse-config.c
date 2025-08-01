#include "parse-config.h"
#include "abort.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  /* for isstring() */

/**
 *: count_tokes
 * @brief Counts the number of tokens in the input string based on the delimiters.
 *
 * @param input_string  A string to parse.
 * @param delimiters    A string of delimiters.
 *
 * @return The number of tokens found, or -1 on error.
 */
int count_tokens(const char *input_string, const char *delimiters) {/* {{{ */
    if (input_string == NULL || delimiters == NULL) {
        return -1;
    }

    // Skip leading delimiters
    const char *stash = input_string + strspn(input_string, delimiters);

    // Check if stash is empty
    if (*stash == '\0') {
        return 0;                                       // No tokens found
    }

    // Duplicate the string
    char *tokenized_string = strdup(stash);
    if (tokenized_string == NULL) {
        return -1;                                      // Memory allocation failed
    }

    char *saveptr;
    int tokens = 0;

    // Count tokens
    if (strtok_r(tokenized_string, delimiters, &saveptr) != NULL) {
        tokens = 1;
        while (strtok_r(NULL, delimiters, &saveptr) != NULL) {
            tokens++;
        }
    }

    free(tokenized_string);
    return tokens;
}
/* }}} */

/**
 *: populate_array
 * @brief Populates the argv array with tokens from the input string.
 *
 * @param input_string  A string to parse.
 * @param delimiters    A string of delimiters.
 * @param argvp        An array to store the tokens.
 * @param tokens       The number of tokens to populate.
 *
 * @return 0 on success, -1 on error.
 */
int populate_array(const char *input_string, const char *delimiters, char ***argvp, int tokens) {/* {{{ */
    const char *stash = input_string + strspn(input_string, delimiters);
    char *tokenized_string = strndup(stash, strlen(stash));
    if (tokenized_string == NULL) {
        return -1;
    }

    char *saveptr;
    char *token = strtok_r(tokenized_string, delimiters, &saveptr);
    for (int i = 0; i < tokens; i++) {
        if (token == NULL) {
            break;                                      // Exit the loop if no more tokens are available
        }

        if (((*argvp)[i] = strndup(token, strlen(token))) == NULL) {
            // Free previously allocated strings on error
            for (int j = 0; j < i; j++) {
                free((*argvp)[j]);
            }
            free(*argvp);
            free(tokenized_string);
            return -1;
        }

        token = strtok_r(NULL, delimiters, &saveptr);
    }

    free(tokenized_string);
    return 0;
}
/* }}} */

/**
 *: make_argv
 * @brief Parses a string into an array of strings based on delimiters.
 *
 * @param input_string  A string to parse.
 * @param delimiters    A string of delimiters.
 * @param argvp        An array to store the tokens.
 *
 * @return The number of elements in the array, or -1 on error.
 */
int make_argv(const char *input_string, const char *delimiters, char ***argvp) {        /* {{{ */
    int tokens = count_tokens(input_string, delimiters);
    if (tokens < 0) {
        *argvp = NULL;
        return -1;
    }

    // Allocate memory for the string array
    if ((*argvp = calloc(tokens + 1, sizeof(char *))) == NULL) {
        return -1;
    }

    // Populate the array with tokens
    if (populate_array(input_string, delimiters, argvp, tokens) < 0) {
        free(*argvp);
        *argvp = NULL;
        return -1;
    }

    return tokens;
}
/* }}} */

/**
 *: find_config_item
 * @brief               This procedure will search and array for a given value.
 *
 * @param array         An array of char arrays to search.
 * @param value         A char array to search for.
 *
 * @returns 0 = Not found. 1 = Item found.
 */
config_t* find_config_item(config_t* config, const char* name, int count) {       /*{{{*/
  for (int i = 0; i < count; i++) {
    if (config[i].values != NULL && \
        config[i].values[0] != NULL) {
      if (strncmp(config[i].values[0], name, strlen(config[i].values[0])) == 0) {
        return &config[i];
      }
    }
  }
  return NULL;
}
/*}}}*/

/**
 *: contains
 * @brief               This procedure will search and array for a given value.
 *
 * @param array         An array of char arrays to search.
 * @param size          Tthe number of array elements.
 * @param value         A char array to search for.
 *
 * @returns 0 = Not found. 1 = Item found.
 */
int contains(char **array, int size, const char *value) {       /*{{{*/
    for (int i = 0; i < size; i++) {
        if (array[i] != NULL && strncmp(array[i], value, strlen(array[i])) == 0) {
            return 1; // Found
        }
    }
    return 0; // Not found
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

      if (argc > 0) {
        config[i].values = argv;
        config[i].value_count = argc;
      }
      i++;
    }

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

