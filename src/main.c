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
//
// SYNOPSYS
//      sysconf -f <config_file> [key [ = value]]
//===-------------------------------------------------------------===

#include "parse-config.h"
#include "print-config.h"
#include "abort.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

//-------------------------------------------------------------------
// Constants Declarations
//-------------------------------------------------------------------
const char program_version[] = "0.0.1";

//------------------------------------------------------*- C -*------
// Main
//
// ARGS
//  argc            :   number of args
//  argv[]          :   array of arguments
//
// RETURN
//  int
//
// TODO
// 1. Add a "default value check" where, when just listing out the
//    values a default file is also parsed and the 'configfile' and
//    the 'default_configfile' is checked. Any duplicates in the
//    'configfile' promps a notice to remove duplicate.
//      a. This can be done with the use of a `sysconf.config` file
//         (possibly located at: ~/.config/sysconf/defaults.conf) where
//         configutation files and their defaults are listed.
//          EX:
//              # USER                    DEFAULT
//              # --------------------------------------------------
//              /etc/rc.conf            = /etc/defaults/rc.conf
//              /etc/sysctl.conf        =
//              /etc/jail.conf.d/*.conf = /etc/jail.conf
//      b. This will be a problem for the jail.conf file entry.
//-------------------------------------------------------------------
int main(int argc, char *argv[]) {

  char *file_string = NULL;                             /* Used to store config file name */
  char *arg_string = NULL;                              /* Used to store the argument string. */
  char delimiters[] = " \t\n\"\':=;";
  int keyvalue_output = 0;

  /*
  // The following works to parse a config file to find the default configuration file to read.
  int my_config_count = 0;
  config_t* my_config_array = parse_config("./sysconf.conf", &my_config_count, delimiters);
  char **defaultconfig_line_array = get_value(my_config_array, my_config_count, "defaultconfig");

  if (defaultconfig_line_array) {
    file_string = defaultconfig_line_array[1];

    int config_count = 0;
    config_t* config_array = parse_config(file_string, &config_count, delimiters);

    printconfigfile(config_array, config_count);
  }
  */

  // -Check the command line arguments.
  //  if there are not enough arguments, exit.
  if (argc < 3) {
    fprintf(stderr, "Usage: %s -f <configuration file> <value to get>\n", argv[0]);
    AbortTranslation(abortInvalidCommandLineArgs);
  }

  // -Parse the command line options.
  for (int i = 0; i < argc; i++) {
    if (argv[i] && strlen(argv[i]) > 1) {
      if (argv[i][0] == '-' && argv[i][1] == 'f') { file_string = argv[++i]; }
      if (argv[i][0] == '-' && argv[i][1] == 'n') { keyvalue_output = 1; }
      if (argv[i][0] != '-') { arg_string = argv[i]; }
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

  // -No argument (key = value or key) given so just
  //  print the config values.
  if( argc == 3) {
    printconfigfile(config_array, config_count);
    free_config(config_array, config_count);
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
        free(arg_array);                                /* cleanup */
        free_config(config_array, config_count);
        return 1;
      }
      if(arg_count > 1) {                               /* Seems to be a condition where the key/value
                                                           needs to be appended to the config file.*/
        printf("%-5s: %s = %s\n", file_string, arg_array[0], arg_array[1]);
        writevariable(arg_array[0], arg_array, arg_count, file_string);

        free(arg_array);                                /* cleanup */
        free_config(config_array, config_count);
        return 1;
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
      if (find_config_item(config_line_array, i, arg_array[1]) == 0) {          /* 0 = value not fount... */

        if (strnstr(arg_array[0], "-", strlen(arg_array[0])) != NULL) { /* if the user wants to subtract a value
                                                                           but the value was not found so exit. */
          printf("Value not found in value string. No change made.\n");
          return 0;
        }

        if (strnstr(arg_array[0], "+", strlen(arg_array[0])) != NULL) { /* if the user wants to set an additional value... */
          printf("%s: ", config_line_array[0]);

          for(int i = 1; i < config_count && config_line_array[i] != NULL; i++) {
            if (memcmp(config_line_array[i], "#", 1) == 0)
                break;
            printf("%s ", config_line_array[i]);
          }

          printf("-> %s ", arg_array[1]);

          for(int i = 1; i < config_count && config_line_array[i] != NULL; i++) {
            if (memcmp(config_line_array[i], "#", 1) == 0)
                break;
            printf("%s ", config_line_array[i]);
          }
          printf("\n");

        } else {                                                        /* just a simple = statement */
          // replacing a variable...
          printf("%s: %s -> %-5s\n", config_line_array[0], config_line_array[1], arg_array[1]);
        }

        replacevariable(config_line_array[0], arg_array, arg_count, file_string);

      } else if(find_config_item(config_line_array, i, arg_array[1]) == 1) {   /* 1 = Value found... */
        if (strnstr(arg_array[0], "-", strlen(arg_array[0])) != NULL) {/* Check if the user wants a subtraction... */
          printf("%s: ", config_line_array[0]);

          for(int i = 1; i < config_count && config_line_array[i] != NULL; i++) {
            if (memcmp(config_line_array[i], "#", 1) == 0)
                break;
            printf("%s ", config_line_array[i]);
          }

          printf("-> ");

          for(int i = 1; i < config_count && config_line_array[i] != NULL; i++) {
            if(strncmp(config_line_array[i], arg_array[1], strlen(config_line_array[i])) != 0) {
              if (memcmp(config_line_array[i], "#", 1) == 0)
                  break;
              printf("%s ", config_line_array[i]);
            }
          }
          printf("\n");

          replacevariable(config_line_array[0], arg_array, arg_count, file_string);
        } else {                                                    /* Assume the user wants to set a value that already exits. */
        printf("Value found. No change made.\n");
        }

        // free stuff here
        free(arg_array);
        free_config(config_array, config_count);
        return 0;
      }

    }   /* end_ if(arg_count > 1)  */
  }     /* end_ if(arg_count >= 1) */

  // cleanup
  free_config(config_array, config_count);

  // The following line "free(arg_array)" causes a
  // malloc error durring a key=value operation.
//:~    free(arg_array);

  return 0;
} ///:~
