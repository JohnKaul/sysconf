#include "parse-config.h"
#include "print-config.h"
#include "abort.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/**
 *: Printconfifile
 * @brief               Iterates the `config_array` and prints the items.
 *
 * @param config_array  The array to pull data from.
 * @param array_count   The number of items in `config_array`.
 */
void printconfigfile(config_t* config_array, int array_count ){     /*{{{*/

  // No argument (key = value or key) given so just print the config values.
  for (int i = 0; i < array_count; i++) {

    // Get the values associated with the argument passed to this function.
    char **config_line_array = get_value(config_array, array_count, config_array[i].values[0]);

    // If the value cannot be found, just exit.
    if (config_line_array == NULL) return;

    printf("%-10s\t=\t", config_array[i].values[0]);
    // just itterate the line array.
    config_line_array += 1;                             /* strip the 'key' from the array */
    while(*config_line_array) {
      if(memcmp(*config_line_array, "#", 1) == 0) {     /* do not print comments. */
        break;
      }
      printf("%s ", *config_line_array++);
    }
    printf("\n");
  }
}
/*}}}*/

/**
 *: add_to_array
 * @brief               Appends a char array (aka: `string`) to the
 *                      end of an array of pointers (aka: `argvp`).
 *
 * @param ***argvp      An array to store the array into.
 * @param size          number of elements in `argvp`.
 * @param string        The string to append to `argvp`.
 *
 * @return int          New size of array.
 */
int add_to_array(char ***argvp, int size, const char *string) {     /*{{{*/
    // Reallocate memory for the new size of the array
    char **new_array = realloc(*argvp, (size + 1) * sizeof(char *));
    if (new_array == NULL) {
        return -1; // Memory allocation failed
    }

    *argvp = new_array;                                 /* Update the original pointer to point to the new array */

    // Allocate memory for the new string and add it to the array
    (*argvp)[size] = strndup(string, strlen(string));
    if ((*argvp)[size] == NULL) {
        return -1;                                      /* Memory allocation for the string failed */
    }

    return size + 1;                                    /* Return the new size of the array */
}
/*}}}*/

/**
 *: assemble_strings
 * @brief               This function assebles an array of char
 *                      arrays into a string (ommiting the first char
 *                      array which should be the 'key' in a key/value
 *                      string).
 * @param value         The array to assemble.
 *                      NOTE: the first entry is assumed to be an
 *                            unwanted entry and is skipped.
 * @param count         The number of elements in the array.
 *
 * @return char*        Assembled char array.
 */
char* assemble_strings(char **value, int count) {       /*{{{*/
    // Calculate the total length needed
    int total_length = 0;
    for (int i = 1; i < count; i++) {
        total_length += strlen(value[i]);
    }

    // Add space for separators (-e.g., spaces)
    total_length += count - 1;                          /* For spaces between strings */

    // Allocate memory for the final string
    char *result = malloc(total_length + 1);            /* +1 for the null terminator */
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Use a buffer to concatenate the strings
    char *ptr = result;                                 /* Pointer to the current position in the buffer */
    for (int i = 1; i < count; i++) {
        // Copy the current string into the buffer
        int len = strlen(value[i]);
        memcpy(ptr, value[i], len);
        // Move the pointer forward by the the length of the copied string.
        ptr += len;

        // Add a space if it's not the last string
        if (i < count - 1) {
            *ptr = ' ';
            ptr++;
        }
    }

    *ptr = '\0';                                        /* Null-terminate the final string */

    return result;
}
/*}}}*/

/**
 *: replacevariable
 * @brief               Replaces a items value in the config file.
 *
 * @param key           The key in the key/value array.
 * @param value         The value (array) in the key/value array.
 * @param count         The value array count.
 * @param filename      The config file to change.
 *
 * @return int          The number of lines in config file.
 */
int replacevariable(const char *key, char **value, int count, const char *filename) {       /*{{{*/
    FILE* conf_file = fopen(filename, "r");             /* Open config file READONLY */
    FILE* temp_file = fopen(".sys.conf.file.tmp", "w"); /* Open a temp file READ/WRITE */
    int found = 0;
    if (!conf_file || !temp_file) {
        fprintf(stderr, "Unable to create temp file or read config file\n");
        AbortTranslation(abortRuntimeError);
    }

    int lines = 0;                                      /* Count the number of lines; this will be the return value. */
    char buffer[1024];                                  /* buffer stores the line */

    while (fgets(buffer, sizeof(buffer), conf_file)) {
        char *str = buffer;
        if (memcmp(key, str, strlen(key)) == 0) {
            // If we've found this key before, skip it and move on.
            if (found == 1)
                continue;

            // If we haven't found the key yet.
            if (found != 1) {
                // Find the position of the separator
                char *sep_pos = str;
                int spaces_before = 0;                  /* used to count spaces before seperator */
                int spaces_after = 0;                   /* used to count spaces after seperator */
                char separator = ' ';                   /* char to use as a seperator (-i.e., space). */
                char terminator = ' ';                  /* char to store the terminator symbol if one used. */
                char quote_char[2] = "";                /* array to store the quotes. */
                int dquote = 0;                         /* used as a flag when a double quote is found. */
                int squote = 0;                         /* used as a flag when a single quote is found. */
                char *value_assembled;                  /* used to store final output string. */
                int comment = 0;                        /* used a a flag when an inline comment is found. */
                int comment_pos = 0;                    /* used as a starting point in a loop counter to
                                                           add any inline comments back to string. */

                if (strnstr(str, "=", strlen(str))) separator = '=';
                if (strnstr(str, ":", strlen(str))) separator = ':';
                if (strnstr(str, ";", strlen(str))) terminator = ';';

                // Skip non-space characters until a space is reached
                while (*sep_pos != '\0' && !isspace(*sep_pos) && *sep_pos != separator) {
                    sep_pos++;
                }

                // Count the spaces before the separator
                while (isspace(*sep_pos) && *sep_pos != '\0' && *sep_pos != separator) {
                    spaces_before++;
                    sep_pos++;
//:~                      switch (*sep_pos) {
//:~                        case '\t': separator = '\t';
//:~                                   break;
//:~                        default : break;
//:~                      }
                }

                if (*sep_pos == separator) sep_pos++;

                // Count the spaces after the separator
                while (*sep_pos != '\0' && isspace(*sep_pos)) {
                    spaces_after++;
                    sep_pos++;
//:~                      switch (*sep_pos) {
//:~                        case '\t': separator = '\t';
//:~                                   break;
//:~                        default : break;
//:~                      }
                }

                // Check if the value is enclosed in quotes
                for (char *p = sep_pos; *p != '\0'; p++) {
                  if (*p == '"' ) dquote++;
                  if (*p == '\'') squote++;
                }
                if (dquote == 2) quote_char[0] = '\"';
                if (squote == 2) quote_char[0] = '\'';

                // -Check the `value` array for the plus (+) sign;
                //  if found, then:
                //   1. Check the entries in `value` are not already in
                //      the config file value array.
                //   2. Assemble the arrays.
                char **current_config_array = NULL;
                int argc;
                int i = 1;
                if (strstr(value[0], "+") != NULL) {
                  char delimiters[] = " \t\n\"\':=;";
                  // 1. tokenize the buffer,
                  // 2. Add the rest of the values to the token array `current_config_array'
                  // 3. Remove the `key` postion from the `current_config_array` array.
                  // 4. Replace the `value` array with the new array `current_config_array`.
                  argc = make_argv(str, delimiters, &current_config_array);
                  for (; i < argc; i++) {

                    // Do not add any "inline comments".
                    if(memcmp(current_config_array[i], "#", 1) == 0) {
                      comment = 1;
                      comment_pos = i;
                      break;
                    }

                    count = add_to_array(&value, count, current_config_array[i]);
                  }
                }

                if (strstr(value[0], "-") != NULL) {
                char **new_config_array = NULL;         /* Array used to add only items in current config_array
                                                         * that are not called out to be removed.
                                                         */

                int new_count = 1;
                  // XXX: make delimiters a global variable which can be referenced here.
                  char delimiters[] = " \t\n\"\':=;";
                  argc = make_argv(str, delimiters, &current_config_array);

                  /* If there are only two items in the line, and the
                   * argument to remove is the same as what is there,
                   * skip tring to recreate the string for output.
                   */
                  if (argc == 2 && \
                      strncmp(value[1], current_config_array[1], strlen(value[1])) == 0) {
                    printf("Last value for key removed. Key removed from file.\n");
                    continue;
                  }

                  for (; i < argc; i++) {
                    if (count >= 1 && \
                        strncmp(value[1], current_config_array[i], strlen(current_config_array[i])) != 0) {

                      // Do not add any "inline comments".
                      if(memcmp(current_config_array[i], "#", 1) == 0) {
                        comment = 1;
                        comment_pos = i;
                        break;
                      }

                      new_count = add_to_array(&new_config_array, new_count, current_config_array[i]);
                    }
                  }
                  value = new_config_array;
                  count = new_count;
                }

                // Assemble the new value string
                value_assembled = assemble_strings(value, count);
                if (value_assembled == NULL) {
                    fclose(conf_file);
                    fclose(temp_file);
                    fprintf(stderr, "Unable to create final value string for config file writing.\n");
                    AbortTranslation(abortRuntimeError);
                }

                // Calculate the length of the new line
                int new_line_length = strlen(key) + spaces_before + 1 + strlen(value_assembled) + strlen(quote_char) * 2 + spaces_after + strlen(quote_char) + 1; // last 1+ for terminator
                char *new_line = malloc(new_line_length + 1); // +1 for null terminator
                if (new_line == NULL) {
                    free(value_assembled);
                    fclose(conf_file);
                    fclose(temp_file);
                    fprintf(stderr, "Unable to allocate memory for new replacement string.\n");
                    AbortTranslation(abortRuntimeError);
                }

                // Construct the new line
                char *ptr = new_line;
                ptr += snprintf(ptr, new_line_length, "%s%*s%c%*s%s%s%s%c\n",
                    key,
                    spaces_before, "",
                    separator,
                    spaces_after, "",
                    quote_char,
                    value_assembled,
                    quote_char,
                    terminator);
                // Write the new line to the temp file
                fputs(new_line, temp_file);

                // Add any inline comments back into the string.
                if (comment != 0) {
                    fputs("     ", temp_file);
                    for (int j = comment_pos; current_config_array[j] != NULL; j++) {
                        fputs(current_config_array[j], temp_file);
                        fputs(" ", temp_file);
                    }
                }

                fputs("\n", temp_file);
                free(new_line);
                free(value_assembled);
                if (current_config_array) {
                    for (int j = 0; current_config_array[j] != NULL; j++) {
                        free(current_config_array[j]);  /* Free each string */
                    }
                    free(current_config_array);         /* Free the array itself */
                    current_config_array = NULL;        /* avoid dangling pointer */
                }
                free(value);
                found = 1;
            }
        } else {
            // Write the original line to the temp file
            fputs(str, temp_file);
        }
        lines += 1;
    }
    fclose(conf_file);
    fclose(temp_file);
    remove(filename);
    rename(".sys.conf.file.tmp", filename);
    return lines;
}
/*}}}*/

/**
 *: writevariable
 * @brief               Writes items value in the config file.
 *
 * @param key           The key in the key/value array.
 * @param value         The value (array) in the key/value array.
 * @param count         The value array count.
 * @param filename      The config file to change.
 */
void writevariable(const char *key, char **value, int count, const char *filename) {       /*{{{*/
  FILE* conf_file = fopen(filename, "a");
  int spaces_before = 0;
  int spaces_after = 0;
  char separator = '=';
  char terminator = ' ';

  char quote_char[2] = "";
  quote_char[0] = '"';

  char *value_assembled = assemble_strings(value, count);

  // Construct the new line
  fprintf(conf_file, "%s%*s%c%*s%s%s%s%c\n", key, spaces_before, "", separator, spaces_after, "", quote_char, value_assembled, quote_char, terminator);

  fclose(conf_file);
}
/*}}}*/
