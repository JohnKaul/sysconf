#include "parse-config.h"
#include "print-config.h"
#include "abort.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

// TODO: Make function headers constant.

/*
 * Printconfifile
 *      Iterates the `config_array` and prints the items.
 *
 * ARGS
 * config_t config_array    :   The array to pull data from.
 * array_count              :   The number of items in `config_array`.
 *
 * RETURN
 *  void
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
    config_line_array += 1;                        // strip the 'key' from the array
    while(*config_line_array) {
      // do not print comments
      if(memcmp(*config_line_array, "#", 1) == 0) {
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
 * @brief               appends `string` to the end of `argvp`.
 *
 * @param ***argvp      An array to store the array into.
 * @param size          number of elements in `argvp`.
 * @param string        The string to append to `argvp`.
 */
int add_to_array(char ***argvp, int size, const char *string) {
    // Reallocate memory for the new size of the array
    char **new_array = realloc(*argvp, (size + 1) * sizeof(char *));
    if (new_array == NULL) {
        return -1; // Memory allocation failed
    }

    *argvp = new_array; // Update the original pointer to point to the new array

    // Allocate memory for the new string and add it to the array
    (*argvp)[size] = strndup(string, strlen(string));
    if ((*argvp)[size] == NULL) {
        return -1; // Memory allocation for the string failed
    }

    return size + 1; // Return the new size of the array
}

/**
 *: assemble_strings
 *      This function assebles the array of char arrays into a string
 *      (ommiting the first char array which should be the 'key' in a
 *      key/value string).
 *
 * NOTE
 *      This method is (more) efficient--than using strcpy()--because
 *      it minimizes the number of memory operations and avoids the
 *      overhead of multiple string concatenation calls.
 *
 * ARGS
 * value            :   The array to assemble (the first entry is
 *                      assumed to be an unwanted entry and is
 *                      skipped).
 * count            :   The number of elements in the array.
 */
char* assemble_strings(char **value, int count) {       /*{{{*/
    // Calculate the total length needed
    int total_length = 0;
    for (int i = 1; i < count; i++) {
        total_length += strlen(value[i]);
    }

    // Add space for separators (e.g., spaces)
    total_length += count - 1; // For spaces between strings

    // Allocate memory for the final string
    char *result = malloc(total_length + 1); // +1 for the null terminator
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Use a buffer to concatenate the strings
    char *ptr = result; // Pointer to the current position in the buffer
    for (int i = 1; i < count; i++) {
        // Copy the current string into the buffer
        int len = strlen(value[i]);
        memcpy(ptr, value[i], len);
        ptr += len; // Move the pointer forward by the length of the copied string

        // Add a space if it's not the last string
        if (i < count - 1) {
            *ptr = ' '; // Add a space
            ptr++; // Move the pointer forward
        }
    }

    *ptr = '\0'; // Null-terminate the final string

    return result;
}
/*}}}*/

/*
 * replacevariable
 *      Replaces a items value in the config file.
 *
 * ARGS
 * key              :   the key in the key/value array.
 * value            :   the value (array) in the key/value array.
 * count            :   The value array count.
 * filename         :   the config file to change.
 */
int replacevariable(const char *key, char **value, int count, const char *filename) {       /*{{{*/
    FILE* conf_file = fopen(filename, "r");
    FILE* temp_file = fopen(".sys.conf.file.tmp", "w");
    int found = 0;
    if (!conf_file || !temp_file) {
        fprintf(stderr, "Unable to create temp file or read config file\n");
        // Handle error appropriately (e.g., abort or return an error code)
        AbortTranslation(abortRuntimeError);
    }

    int lines = 0; // Count the number of lines
    char buffer[1024]; // buffer stores the line

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
                int spaces_before = 0;
                int spaces_after = 0;
                char separator;
                char terminator = ' ';

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
                }

                if (*sep_pos == separator) sep_pos++;

                // Count the spaces after the separator
                while (*sep_pos != '\0' && isspace(*sep_pos)) {
                    spaces_after++;
                    sep_pos++;
                }

                // Check if the value is enclosed in quotes
                int quotes = 0;
                for (char *p = sep_pos; *p != '\0'; p++) {
                    if (*p == '"') quotes++;
                }

                char quote_char[2] = "";
                if (quotes == 2) {
                    quote_char[0] = '"';
                }

                // check the `value` array for the plus (+) sign;
                // if found, then:
                //  1. Check the entries in `value` are not already in
                //     the config file value array.
                //  2. Assemble the arrays.
                //
                char **current_config_array;
                int argc;
                if (strstr(value[0], "+") != NULL) {
//:~                    printf("\t I found a plus sign! %s\n", value[0]);
                  // XXX: make delimiters a global variable which can be referenced here.
                  char delimiters[] = " \t\n\":=;";
                  // 1. tokenize the buffer,
                  // 2. Add the rest of the values to the token array `current_config_array'
                  // 3. Remove the `key` postion from the `current_config_array` array.
                  // 4. Replace the `value` array with the new array `current_config_array`.
                  argc = make_argv(str, delimiters, &current_config_array);
                  for (int i = 1; i < argc; i++) {
//:~                      printf("\tvalue[%d] = %s\n", i, value[i]);
//:~                      printf("\tcurrent_config_array[%d] = %s\n", i, current_config_array[i]);
                    count = add_to_array(&value, count, current_config_array[i]);
                  }
//:~                    printf("Added element(s) to array, here are the results\n");
                }

//:~                  // just print the value array.
//:~                  for (int i = 0; i < count; i++)
//:~                    printf("\t==> value[%d]:%s\n", i, value[i]);

                // Assemble the new value string
                char *value_assembled = assemble_strings(value, count);
//:~                  printf("Assembled String: %s\n", value_assembled);
                if (value_assembled == NULL) {
                    fclose(conf_file);
                    fclose(temp_file);
                    fprintf(stderr, "Unable to create final value string for config file writing.\n");
                    AbortTranslation(abortRuntimeError);
                }

                // Calculate the length of the new line
                int new_line_length = strlen(key) + spaces_before + 1 + strlen(value_assembled) + strlen(quote_char) * 2 + spaces_after + 1; // +1 for terminator
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
                ptr += sprintf(ptr, "%s%*s%c%*s%s%s%s%c\n", key, spaces_before, "", separator, spaces_after, "", quote_char, value_assembled, quote_char, terminator);

                // Write the new line to the temp file
                fputs(new_line, temp_file);
                free(new_line);
                free(value_assembled);

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
