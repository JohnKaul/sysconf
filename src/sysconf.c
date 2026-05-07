//===---------------------------------------------------*- C -*---===
// File Last Updated: 05.04.26 21:35:15
//
//: main.c
//
// BY  : John Kaul [john.kaul@outlook.com]
//
// DESCRIPTION
// This is a utility to list and change key/value based config file values.
//
// Configuration files typically have the `key = value` syntax but
// some key/value items are seperated with equal (=), spaces ( ),
// colons (:), etc. and there does't seem to be a utility that allows
// for quick changing of these values.
//
// The `sysrc` utility in FreeBSD allows for easy changing of
// variables/values but the utility does not work with doted names
// -i.e. `key.subkey = value;` entries. This utility should help.
//
// This utility can also be called from a script; this utility when
// reporting the value for a key only reports the value for the given
// variable (like using the -n switch with sysrc).
//
// This utility can preform actions based on the arguments given.
// For example:
//      % sysconf -f <config_file>
//    Will display the cofig_file key=value pair.
//
//      % sysconf -f <config_file> key
//    Will display the config_file key's value.
//
//      % sysconf -f <config_file> key=value
//      % sysconf -f <config_file> key+=value
//      % sysconf -f <config_file> key-=value
//    Will change the config_file value to the value specified as an argument.
//
//      % sysconf -f <config_file> -d <defaults_config_file>
//    Will check for duplicate value entries for each key in the config_file against the
//    defaults_config_file.
//
// If this utlity is called to set a key/value and the configuration
// file doesn't exist, it will be created.
//
// SYNOPSYS
//      sysconf -f configfile
//      sysconf -f configfile -d configfile.defaults
//      sysconf -f configfile [-n] [key]
//      sysconf -f configfile [key=value]
//      sysconf -f configfile [key+=value]
//      sysconf -f configfile [key-=value]
//===-------------------------------------------------------------===

#include "parse-config.h"
#include "print-config.h"
#include "abort.h"
#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

//------------------------------------------------------*- C -*------
// Main
//
// ARGS
//  argc            :   number of args
//  argv[]          :   array of arguments
//
// RETURN
//  int
//-------------------------------------------------------------------
int main(int argc, char *argv[]) {

  char *file_string = NULL;                             /* Used to store config file name */
  char *default_string = NULL;                          /* Used to store default config file name. */
  char *arg_string = NULL;                              /* Used to store the argument string. */
  char delimiters[] = " \t\n\"\':=;";
  int keyvalue_output = 0;

  // -Check the command line arguments.
  //  if there are not enough arguments, exit.
  if (argc < 3) {
    fprintf(stderr, "Usage: %s -f <configuration file> <value to get>\n", argv[0]);
    fprintf(stderr, "**** %s version: %s\n", argv[0], program_version);
    AbortTranslation(abortInvalidCommandLineArgs);
  }

  // -Parse the command line options.
  for (int i = 1; i < argc; i++) {
    if (argv[i] && strlen(argv[i]) > 1) {
      if (argv[i][0] != '-') { arg_string = argv[i]; }
      if (argv[i][0] == '-' && argv[i][1] == 'f') { file_string = argv[++i]; }
      if (argv[i][0] == '-' && argv[i][1] == 'd') { default_string = argv[++i]; }
      if (argv[i][0] == '-' && argv[i][1] == 'n') { keyvalue_output = 1; }
    }
  }

  // -If there is not a `file_string` variable, quit.
  if (! file_string) {
    fprintf(stderr, "Usage: %s -f <configuration file> <value to get>\n", argv[0]);
    AbortTranslation(abortInvalidCommandLineArgs);
  }

  // -Keep a record of how many items in the config file.
  int config_count = 0;
  int arg_count = 0;

  // -Parse the config file.
  config_t* config_array = parse_config(file_string, &config_count, delimiters);

  // -If we couldn't parse the file, quit.
  if (!config_array) {
    printf("Failed to parse the configuration file.\n");
    free(config_array);
    return 1;
  }

  if (default_string != NULL) {         /* We are going to check the config file for
                                         * duplicates against a defaults config file.
                                         */
    int default_count = 0;
    config_t* default_array = parse_config(default_string, &default_count, delimiters);

    if (!default_array) {
      printf("Failed to parse the configuration file.\n");
      free(config_array);
      free(default_array);
      return 1;
    }

    for (int i = 0; i < config_count; i++) {
      if (config_array[i].values != NULL && config_array[i].values[0] != NULL) {
        char* key = config_array[i].values[0];
        config_t* temp_conf = find_config_item(config_array, key, config_count);
        config_t* temp_default = find_config_item(default_array, key, default_count);
        if (temp_default != NULL) {
          for (int x = 1; x < config_array[i].value_count; x++) {
            char* value = temp_conf->values[x];
            if (memcmp(value, "#", 1) == 0)
              break;
            if (contains(temp_default->values, temp_default->value_count, value) == 1)
              printf("*DUPLICATE* %s: '%s'\n", key, value);
          }
        }
      }
    }
    return 0;
  }     /* end_ if(default_string) */

  // -No argument (key = value or key) given so just
  //  print the config values.
  if( argc == 3) {
    printconfigfile(config_array, config_count);
    free_config(config_array, config_count);
    free(config_array);
    return 0;
  }

  // -Parse the argument string passed to this program.
  //  Based on the size of this array, we are going to determine if we
  //  need to preform replacement operations or just list the value.
  char **arg_array;                                     /* Used to store the argument
                                                           string passed to this program. */
  arg_count = make_argv(arg_string, delimiters, &arg_array);

  // -Do things differently based on the number of arguments given.
  //  no argument;
  //    disply the config file.
  //  key;
  //    look up the key in the config_file and disply it's value.
  //  key=value;
  //    Make a addition/replacement/update in the config_file.
  if(arg_count >= 1) {

    // Get the values associated with the argument passed to this function.
    char **config_line_array = get_value(config_array, config_count, arg_array[0]);

    // If the key cannot be found in the config file, we need to check
    // to see if the value is:
    // 1. being searched for; if so, exit.
    // 2. wanting to be added; if so, add it.
    if (config_line_array == NULL) {

      if(arg_count == 1) {                              /* Seems to be a simple search situation,
                                                           since, we do not have a key in the config
                                                           file, we exit.*/
        fprintf(stderr, "Error: key not found\n");
        for (int i = 0; i < arg_count; i++) free(arg_array[i]);
        free(arg_array);                                /* cleanup */
        arg_array = NULL;
        free_config(config_array, config_count);
        free(config_array);
        config_array = NULL;
        return 1;
      }
      if(arg_count > 1) {                               /* Seems to be a condition where the key/value
                                                           needs to be appended to the config file.*/
        //:~          printf("%-5s: %s = %s\n", file_string, arg_array[0], arg_array[1]);

        /* In the condition where the key is not found we need to
         * check to see if the string is not a += or -= operation
         * before we append the config file.  */
        if ((strnstr(arg_array[0], "-", strlen(arg_array[0])) != NULL) ||
            (strnstr(arg_array[0], "+", strlen(arg_array[0])) != NULL)) {
          printf("Incorrect syntax. Key is not found in config file.\n");
          return 1;
        }

        writevariable(arg_array[0], arg_array, arg_count, file_string);

        for (int i = 0; i < arg_count; i++) free(arg_array[i]);
        free(arg_array);
        arg_array = NULL;

        free_config(config_array, config_count);
        free(config_array);
        config_array = NULL;

        return 0;
      }
    }

    // -If we only have a 'key' as an argument; just find in the
    //  config and display the value set.
    if(config_line_array != NULL && arg_count == 1) {
      // Itterate the array and print chars.
      if (keyvalue_output != 0) {
        printf("%s: ", config_line_array[0]);
      }
      config_line_array += 1;                           /* strip the 'key' from the array */
      while(*config_line_array) {
        if (memcmp(*config_line_array, "#", 1) == 0)
          break;
        printf("%s ", *config_line_array++);
      }
      printf("\n");
      for (int i = 0; i < arg_count; i++) free(arg_array[i]);
      free(arg_array);                                /* cleanup */
      arg_array = NULL;
      free_config(config_array, config_count);
      free(config_array);
      config_array = NULL;
      return 0;
    }

    // -If the argument is equal to 'key=value' make a replacement.
    // -However, if the argument is 'key+=value' or 'key-=value' make an update.
    if(arg_count > 1) {

      // Count the config_line_array number of elements.
      int i = 0;
      for (; config_line_array[i] != NULL; i++)
        ;

      // -Determine if we have a change to make; compare the `arg_array`
      //  and the `config_array` values.
      if (contains(config_line_array, i, arg_array[1]) == 0) {          /* 0 = A value was not found in the key's string... */

        if (strnstr(arg_array[0], "-", strlen(arg_array[0])) != NULL) { /* However, if the user wants to subtract a value
                                                                           but the value was not found, we need to exit. */
          printf("Value not found in value string. No change made.\n");

          for (int i = 0; i < arg_count; i++) free(arg_array[i]);
          free(arg_array);
          arg_array = NULL;

          free_config(config_array, config_count);
          free(config_array);
          config_array = NULL;

          return 0;
        }

        replacevariable(config_line_array[0], arg_array, arg_count, file_string);

        /* LOGIC:
         * - The concept is to free `arg_array` up to the `arg_count`, and allow the
         *   `print-config.c` to free the additional value(s) added to the `arg_array`.
         */
        arg_array = NULL;

        free_config(config_array, config_count);
        free(config_array);
        config_array = NULL;

        return 0;

      } else if(contains(config_line_array, i, arg_array[1]) == 1) {   /* 1 = Value found... */

        replacevariable(config_line_array[0], arg_array, arg_count, file_string);

        /* LOGIC:
         * - The concept is to free `arg_array` up to the `arg_count`,
         *   and allow the `print-config.c` to subtract from it's
         *   counter to free any remaining value(s) from the
         *   `arg_array`.
         */
        for (int i = 0; i < arg_count; i++) free(arg_array[i]);
        free(arg_array);
        arg_array = NULL;

        free_config(config_array, config_count);
        free(config_array);
        config_array = NULL;

        return 0;
      } else {                                                    /* Assume the user wants to set a value that already exits. */
        printf("Value found. No change made.\n");
      }
    }   /* end_ if(arg_count > 1)  */
  }     /* end_ if(arg_count >= 1) */

  // cleanup
  for (int i = 0; i < arg_count; i++) free(arg_array[i]);
  free(arg_array);
  arg_array = NULL;

  free_config(config_array, config_count);
  free(config_array);
  config_array = NULL;

  return 0;
} ///:~
