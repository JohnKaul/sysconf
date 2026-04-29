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
void printconfigfile(config_t* config_array, int array_count ){

  // No argument (key = value or key) given so just print the config values.
  for (int i = 0; i < array_count; i++) {

    // Get the values associated with the argument passed to this function.
    char **config_line_array = get_value(config_array, array_count, config_array[i].values[0]);

    // If the value cannot be found, just continue.
    if (config_line_array == NULL) continue;

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
int add_to_array(char ***argvp, int size, const char *string) {
    // Reallocate memory for the new size of the array (size + 1 for new element + 1 for NULL)
    char **new_array = realloc(*argvp, (size + 2) * sizeof(char *));
    if (new_array == NULL) {
        return -1;
    }

    *argvp = new_array;

    // Allocate memory for the new string and add it to the array
    (*argvp)[size] = strdup(string);
    if ((*argvp)[size] == NULL) {
        return -1;
    }

    // Ensure NULL termination
    (*argvp)[size + 1] = NULL;

    return size + 1;
}

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
char* assemble_strings(char **value, int count) {
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
int replacevariable(const char *key, char **value, int count, const char *filename) {
    FILE* conf_file = fopen(filename, "r");             /* Open config file READONLY */
    FILE* temp_file = fopen(".sys.conf.file.tmp", "w"); /* Open a temp file READ/WRITE */
    int found = 0;
    if (!conf_file || !temp_file) {
        fprintf(stderr, "Unable to create temp file or read config file\n");
        if (conf_file) fclose(conf_file);
        if (temp_file) fclose(temp_file);
        return 1;
    }

    char buffer[1024];                                  /* buffer stores the line */

    while (fgets(buffer, sizeof(buffer), conf_file)) {
        char *str = buffer;
        size_t key_len = strlen(key);
        if (strncmp(key, str, key_len) == 0 && (isspace((unsigned char)str[key_len]) || str[key_len] == '=' || str[key_len] == ':' || str[key_len] == '\0')) {
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
                char **work_value_array = NULL;         /* Local array for assembly */
                int work_value_count = count;
                int argc;

                // tokenize the current line to check for inline comments
                {
                  char delimiters[] = " \t\n\"\':=;";
                  argc = make_argv(str, delimiters, &current_config_array);
                  for (int j = 0; j < argc; j++) {
                    if (memcmp(current_config_array[j], "#", 1) == 0) {
                      comment = 1;
                      comment_pos = j;
                      break;
                    }
                  }
                }

                // Initialize work_value_array with current value array
                work_value_array = calloc(count + 1, sizeof(char *));
                if (!work_value_array) {
                    fclose(conf_file);
                    fclose(temp_file);
                    return 1;
                }
                for (int j = 0; j < count; j++) {
                    work_value_array[j] = strdup(value[j]);
                }
                work_value_array[count] = NULL;

                int i = 1;
                if (strstr(work_value_array[0], "+") != NULL) {
                  for (; i < argc; i++) {
                    // Do not add any "inline comments".
                    if(memcmp(current_config_array[i], "#", 1) == 0) {
                      break;
                    }
                    int new_size = add_to_array(&work_value_array, work_value_count, current_config_array[i]);
                    if (new_size != -1) {
                        work_value_count = new_size;
                    }
                  }
                }

                if (strstr(work_value_array[0], "-") != NULL) {
                  if (argc == 2 && \
                      strcmp(value[1], current_config_array[1]) == 0) {
                    printf("Last value for key removed. Key removed from file.\n");
                    for (int j = 0; current_config_array[j] != NULL; j++) {
                        free(current_config_array[j]);
                    }
                    free(current_config_array);
                    current_config_array = NULL;
                    
                    for (int j = 0; j < work_value_count; j++) free(work_value_array[j]);
                    free(work_value_array);
                    continue;
                  }

                  char **minus_config_array = calloc(2, sizeof(char *));
                  minus_config_array[0] = strdup(work_value_array[0]);
                  int minus_count = 1;

                  for (; i < argc; i++) {
                    if (count >= 1 && \
                        strcmp(value[1], current_config_array[i]) != 0) {

                      if(memcmp(current_config_array[i], "#", 1) == 0) {
                        break;
                      }
                      int new_minus_count = add_to_array(&minus_config_array, minus_count, current_config_array[i]);
                      if (new_minus_count != -1) {
                          minus_count = new_minus_count;
                      }
                    }
                  }
                  
                  for (int j = 0; j < work_value_count; j++) free(work_value_array[j]);
                  free(work_value_array);
                  work_value_array = minus_config_array;
                  work_value_count = minus_count;
                }

                // Assemble the new value string
                value_assembled = assemble_strings(work_value_array, work_value_count);
                if (value_assembled == NULL) {
                    for (int j = 0; j < work_value_count; j++) free(work_value_array[j]);
                    free(work_value_array);
                    if (current_config_array) {
                        for (int j = 0; current_config_array[j] != NULL; j++) free(current_config_array[j]);
                        free(current_config_array);
                    }
                    fclose(conf_file);
                    fclose(temp_file);
                    fprintf(stderr, "Unable to create final value string for config file writing.\n");
                    return 1;
                }

                // Calculate the length of the new line
                int new_line_length = strlen(key) + spaces_before + 1 + \
                                      strlen(value_assembled) + \
                                      strlen(quote_char) * 2 + \
                                      spaces_after + \
                                      strlen(quote_char) + 1; // last 1+ for terminator

                char *new_line = malloc(new_line_length + 1); // +1 for null terminator
                if (new_line == NULL) {
                    free(value_assembled);
                    for (int j = 0; j < work_value_count; j++) free(work_value_array[j]);
                    free(work_value_array);
                    if (current_config_array) {
                        for (int j = 0; current_config_array[j] != NULL; j++) free(current_config_array[j]);
                        free(current_config_array);
                    }
                    fclose(conf_file);
                    fclose(temp_file);
                    fprintf(stderr, "Unable to allocate memory for new replacement string.\n");
                    return 1;
                }

                // Construct the new line
                char *ptr = new_line;
                ptr += snprintf(ptr, new_line_length, "%s%*s%c%*s%s%s%s%c",
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
                if (comment != 0 && current_config_array != NULL) {
                    fputs("     ", temp_file);
                    for (int j = comment_pos; current_config_array[j] != NULL; j++) {
                        fputs(current_config_array[j], temp_file);
                        if (current_config_array[j+1] != NULL) fputs(" ", temp_file);
                    }
                }

                fputs("\n", temp_file);

                if (current_config_array != NULL) {
                    for (int j = 0; current_config_array[j] != NULL; j++) {
                        free(current_config_array[j]);
                    }
                    free(current_config_array);
                    current_config_array = NULL;
                }

                free(new_line);
                new_line = NULL;

                free(value_assembled);

                for (int j = 0; j < work_value_count; j++) free(work_value_array[j]);
                free(work_value_array);
                work_value_array = NULL;

                found = 1;
            }
        } else {
          // Write the original line to the temp file
          fputs(str, temp_file);
        }
    }
    fclose(conf_file);
    fclose(temp_file);
    if (rename(".sys.conf.file.tmp", filename) != 0) {
        perror("Error renaming temporary file");
        return 1;
    }
    return 0;
}

/**
 *: writevariable
 * @brief               Writes items value in the config file.
 *
 * @param key           The key in the key/value array.
 * @param value         The value (array) in the key/value array.
 * @param count         The value array count.
 * @param filename      The config file to change.
 */
void writevariable(const char *key, char **value, int count, const char *filename) {
  FILE* conf_file = fopen(filename, "a");
  int spaces_before = 0;
  int spaces_after = 0;
  char separator = '=';
  char terminator = ' ';

  char quote_char[2] = "";
  quote_char[0] = '"';

  // Strip the trailing '+' or '-' from key if present
  char *clean_key = strdup(key);
  size_t key_len = strlen(clean_key);
  if (key_len > 0 && (clean_key[key_len-1] == '+' || clean_key[key_len-1] == '-')) {
      clean_key[key_len-1] = '\0';
  }

  char *value_assembled = assemble_strings(value, count);

  // Construct the new line
  fprintf(conf_file, "%s%*s%c%*s%s%s%s%c\n", clean_key, spaces_before, "", separator, spaces_after, "", quote_char, value_assembled, quote_char, terminator);

  free(value_assembled);
  free(clean_key);
  fclose(conf_file);
}
