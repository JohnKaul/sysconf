//===---------------------------------------------------*- C -*---===
// File Last Updated: 02.07.25 15:27:30
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
// So far this utility can preform actions based on the arguments given.
// For example:
//      % sysconf -f <config_file>
// Will display the cofig_file key=value pair.
//      % sysconf -f <config_file> key
// Will display the config_file key's value.
//      % sysconf -f <config_file> key=value
// Will change the config_file value to the value specified as an argument.
//      % sysconf -f <config_file> -d <defaults_config_file>
// Will check for duplicate value entries for each key in the config_file against the
// defaults_config_file.
//
// SYNOPSYS
//      sysconf -f configfile
//
//      sysconf -f configfile -d configfile.defaults
//
//      sysconf -f configfile [-n] [key]
//
//      sysconf -f configfile [key=value]
//
//      sysconf -f configfile [key+=value]
//
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
  int exit_code = 0;

  config_t* config_array = NULL;
  int config_count = 0;
  char **arg_array = NULL;
  int arg_count = 0;

  // -Check the command line arguments.
  //  if there are not enough arguments, exit.
  if (argc < 3) {
    fprintf(stderr, "Usage: %s -f <configuration file> <value to get>\n", argv[0]);
    fprintf(stderr, "**** %s version: %s\n", argv[0], program_version);
    AbortTranslation(abortInvalidCommandLineArgs);
  }

  // -Parse the command line options.
  for (int i = 1; i < argc; i++) {
    if (argv[i] && strlen(argv[i]) > 1 && argv[i][0] == '-') {
      if (strcmp(argv[i], "-f") == 0) {
        if (i + 1 < argc) {
          file_string = argv[++i];
        } else {
          fprintf(stderr, "Error: -f requires an argument\n");
          AbortTranslation(abortInvalidCommandLineArgs);
        }
      } else if (strcmp(argv[i], "-d") == 0) {
        if (i + 1 < argc) {
          default_string = argv[++i];
        } else {
          fprintf(stderr, "Error: -d requires an argument\n");
          AbortTranslation(abortInvalidCommandLineArgs);
        }
      } else if (strcmp(argv[i], "-n") == 0) {
        keyvalue_output = 1;
      } else {
        fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
        AbortTranslation(abortInvalidCommandLineArgs);
      }
    } else {
      arg_string = argv[i];
    }
  }

  // -If there is not a `file_string` variable, quit.
  if (! file_string) {
    fprintf(stderr, "Usage: %s -f <configuration file> <value to get>\n", argv[0]);
    AbortTranslation(abortInvalidCommandLineArgs);
  }

  // -Parse the config file.
  config_array = parse_config(file_string, &config_count, delimiters);

  // -If we couldn't allocate memory, quit.
  if (!config_array) {
    printf("Failed to allocate memory for configuration\n");
    return 1;
  }

  if (default_string != NULL) {         /* We are going to check the config file for
                                         * duplicates against a defaults config file.
                                         */
    int default_count = 0;
    config_t* default_array = parse_config(default_string, &default_count, delimiters);

    if (!default_array) {
      printf("Failed to parse the defaults file: %s\n", default_string);
      exit_code = 1;
      goto cleanup;
    }

    for (int i = 0; i < config_count; i++) {
      if (config_array[i].values != NULL && config_array[i].values[0] != NULL) {
        char* key = config_array[i].values[0];
        config_t* temp_default = find_config_item(default_array, key, default_count);
        if (temp_default != NULL) {
          for (int x = 1; x < config_array[i].value_count; x++) {
            char* value = config_array[i].values[x];
            if (value[0] == '#')
              break;
            if (contains(temp_default->values, temp_default->value_count, value) == 1)
              printf("*DUPLICATE* %s: '%s'\n", key, value);
          }
        }
      }
    }
    free_config(default_array, default_count);
    free(default_array);
    exit_code = 0;
    goto cleanup;
  }

  // -No argument given (just -f file)
  if (arg_string == NULL) {
    printconfigfile(config_array, config_count);
    exit_code = 0;
    goto cleanup;
  }

  // -Parse the argument string passed to this program.
  arg_count = make_argv(arg_string, delimiters, &arg_array);

  if(arg_count >= 1) {
    char *lookup_key = strdup(arg_array[0]);
    size_t lk_len = strlen(lookup_key);
    if (lk_len > 0 && (lookup_key[lk_len-1] == '+' || lookup_key[lk_len-1] == '-')) {
        lookup_key[lk_len-1] = '\0';
    }

    char **config_line_array = get_value(config_array, config_count, lookup_key);
    free(lookup_key);

    if (config_line_array == NULL) {
      if(arg_count == 1) {
        fprintf(stderr, "Error: key not found\n");
        exit_code = 1;
        goto cleanup;
      }
      if(arg_count > 1) {
        printf("%-5s: %s = %s\n", file_string, arg_array[0], arg_array[1]);
        writevariable(arg_array[0], arg_array, arg_count, file_string);
        exit_code = 0;
        goto cleanup;
      }
    }

    if(arg_count == 1) {
      if (keyvalue_output != 0) {
        printf("%s: ", config_line_array[0]);
      }
      for (int i = 1; config_line_array[i] != NULL; i++) {
        if (config_line_array[i][0] == '#')
          break;
        printf("%s ", config_line_array[i]);
      }
      printf("\n");
      exit_code = 0;
      goto cleanup;
    }

    if(arg_count > 1) {
      int val_count = 0;
      while (config_line_array[val_count] != NULL) val_count++;

      if (contains(config_line_array, val_count, arg_array[1]) == 0) {
        if (strchr(arg_array[0], '-') != NULL) {
          printf("Value not found in value string. No change made.\n");
          exit_code = 0;
          goto cleanup;
        }

        if (strchr(arg_array[0], '+') != NULL) {
          printf("%s: ", config_line_array[0]);
          for(int i = 1; config_line_array[i] != NULL; i++) {
            if (config_line_array[i][0] == '#') break;
            printf("%s ", config_line_array[i]);
          }
          printf("-> %s ", arg_array[1]);
          for(int i = 1; config_line_array[i] != NULL; i++) {
            if (config_line_array[i][0] == '#') break;
            printf("%s ", config_line_array[i]);
          }
          printf("\n");
        } else {
          printf("%s: %s -> %-5s\n", config_line_array[0], config_line_array[1] ? config_line_array[1] : "(null)", arg_array[1]);
        }

        replacevariable(config_line_array[0], arg_array, arg_count, file_string);
        exit_code = 0;
        goto cleanup;

      } else { // Value found
        if (strchr(arg_array[0], '-') != NULL) {
          printf("%s: ", config_line_array[0]);
          for(int i = 1; config_line_array[i] != NULL; i++) {
            if (config_line_array[i][0] == '#') break;
            printf("%s ", config_line_array[i]);
          }
          printf("-> ");
          for(int i = 1; config_line_array[i] != NULL; i++) {
            if (config_line_array[i][0] == '#') break;
            if(strcmp(config_line_array[i], arg_array[1]) != 0) {
              printf("%s ", config_line_array[i]);
            }
          }
          printf("\n");
          replacevariable(config_line_array[0], arg_array, arg_count, file_string);
          exit_code = 0;
          goto cleanup;
        } else {
          printf("Value found. No change made.\n");
          exit_code = 0;
          goto cleanup;
        }
      }
    }
  }

cleanup:
  if (arg_array) {
    for (int i = 0; i < arg_count; i++) free(arg_array[i]);
    free(arg_array);
  }
  if (config_array) {
    free_config(config_array, config_count);
    free(config_array);
  }

  return exit_code;
}
