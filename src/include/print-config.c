#include "parse-config.h"
#include "print-config.h"
#include "abort.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

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

/*
 * replacevariable
 *      Replaces a items value in the config file.
 *
 * ARGS
 * key              :   the key in the key/value array.
 * value            :   the value in the key/value array.
 * filename         :   the config file to change.
 */
int replacevariable(const char *key, const char *value, const char *filename) {/*{{{*/
  FILE* conf_file = fopen(filename, "r");
  FILE* temp_file = fopen(".sys.conf.file.tmp", "w");
  int found = 0;
  if (!conf_file || !temp_file) {
    fprintf(stderr, "Unable to create temp file or read config file\n");
    AbortTranslation(abortRuntimeError);
  }

  int lines = 0;                                      /* Count the number of lines */
  char buffer[1024];                                  /* buffer stores the line */
//:~    char *comment;
//:~    comment = "# my comment.";

  while (fgets(buffer, sizeof(buffer), conf_file)) {
    char *str = buffer;
    if (memcmp(key, str, strlen(key)) == 0) {

      // If we've found this key before, skip it and move on.
      if (found == 1)
        continue;

      // If we havent found the key yet.
      if (found != 1) {

        // Find the position of the separator
        char *sep_pos = str;
        int spaces_before = 0;
        int spaces_after = 0;
        char separator;
//:~          char separator = ' ';
        char terminator = ' ';

        if (strnstr(str, "=", strlen(str))) separator = '=';
        if (strnstr(str, ":", strlen(str))) separator = ':';

        if (strnstr(str, ";", strlen(str))) terminator = ';';

        // Skip non-space characters until a space is reached
//:~          while (*sep_pos != '\0' && !isspace(*sep_pos)) {
        while (*sep_pos != '\0' && \
              !isspace(*sep_pos) && \
               *sep_pos != separator) {
          sep_pos++;
        }

        // Count the spaces before the separator
//:~          while (*sep_pos != '\0' && isspace(*sep_pos)) {
        while (isspace(*sep_pos) && \
               *sep_pos != '\0' && \
               *sep_pos != separator) {
          spaces_before++;
          sep_pos++;
        }

//:~         // Check if the separator is an equal sign or a colon
//:~         if (*sep_pos == '=') {
//:~           separator = '=';
//:~           sep_pos++;
//:~         } else if (*sep_pos == ':') {
//:~           separator = ':';
//:~           sep_pos++;
//:~         }

        if (*sep_pos == separator) sep_pos++;

        // Count the spaces after the separator
        while (*sep_pos != '\0' && isspace(*sep_pos)) {
//:~          while (*sep_pos != '\0' && \
//:~                (isspace(*sep_pos) || *sep_pos != separator)) {
          spaces_after++;
          sep_pos++;
        }

        // Check if the value is enclosed in quotes
        int quotes = 0;
        for (char *p = sep_pos; *p != '\0'; p++) {
          if (*p == '"') quotes++;
        }

        // Reconstruct the string
        char quote_char[2] = "";
        if (quotes == 2) {
          quote_char[0] = '"';
        }
        if (separator == '=' || separator == ':') {
          sprintf(str, "%s%*s%c%*s%s%s%s%c\n", key, spaces_before, "", separator, spaces_after, "", quote_char, value, quote_char, terminator);
//:~            sprintf(str, "%s%*s%c%*s%s%s%s%c%-8s%s\n", key, spaces_before, "", separator, spaces_after, "", quote_char, value, quote_char, terminator, "", comment);
        } else {
          sprintf(str, "%s%*s%s%s%s%c\n", key, spaces_before, "", quote_char, value, quote_char, terminator);
        }
        found = 1;
      }
    }
    fputs(str, temp_file);
    lines += 1;
  }
  fclose(conf_file);
  fclose(temp_file);
  remove(filename);
  rename(".sys.conf.file.tmp", filename);
  return lines;
}
/*}}}*/
