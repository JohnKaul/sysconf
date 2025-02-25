//===---------------------------------------------------*- C -*---===
// File Last Updated: 02.07.25 15:27:30
//
//: main.c
//
// DATE: October 21 2020
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

#include "include/parse-config.h"
#include "include/print-config.h"
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
// 1. Support the "+=" and "-=" options.
//      a. This will require a signifigant refactor; checking of each
//         elements in the `arg_array` against the `config_array` will
//         need to take place.
//-------------------------------------------------------------------
int main(int argc, char *argv[]) {

  int commentflag = 0;
  char *file_string = NULL;
  char *arg_string = NULL;

  // --Check the command line arguments.
  //   if there are not enough arguments, exit.
  if (argc < 3) {
    fprintf(stderr, "Usage: %s -f <configuration file> <value to get>\n", argv[0]);
    AbortTranslation(abortInvalidCommandLineArgs);
  }

  for (int i = 0; i < argc; i++) {
    if (argv[i] && strlen(argv[i]) > 1) {
      if (argv[i][0] == '-' && argv[i][1] == 'c') { commentflag = 1; }
      if (argv[i][0] == '-' && argv[i][1] == 'f') { file_string = argv[++i]; }
      if (argv[i][0] != '-') { arg_string = argv[i]; }
    }
  }

  // If ther isnt a file_string variable, quit.
  if (! file_string) {
    fprintf(stderr, "Usage: %s -f <configuration file> <value to get>\n", argv[0]);
    AbortTranslation(abortInvalidCommandLineArgs);
  }

  // keep a record of how many items in the config file.
  int config_count = 0;
  int arg_count = 0;
  char delimiters[] = " \t\n\":=;";

  // Parse the config file.
  config_t* config_array = parse_config(file_string, &config_count, delimiters);

  // If we couldn't parse the file, quit.
  if (!config_array) {
    printf("Failed to parse the configuration file.\n");
    free(config_array);
    return 1;
  }

  // No argument (key = value or key) given so just print the config values.
  if( argc == 3) {
    printconfigfile(config_array, config_count);
    return 0;
  }

  // Parse the argument string passed to this program.
  // Based on the size of this array, we are going to determine if we
  // need to preform replacement operations or just list the value.
  char **arg_array;                                     // Used to store the argument
                                                        // string passed to this program.
  arg_count = make_argv(arg_string, delimiters, &arg_array);

  // Do things differently based on the number of arguments given.
  // no argument;
  //    disply the config file.
  // key;
  //    look up the key in the config_file and disply it's value.
  // key=value;
  //    Make a replacement in the config_file.
  if(arg_count >= 1) {

    // Get the values associated with the argument passed to this function.
    char **config_line_array = get_value(config_array, config_count, arg_array[0]);

    // If the value cannot be found, just exit.
    if (config_line_array == NULL) return 1;

     // if we only have a 'key' as an argument; just find in the
     // config and display the value set.
    if(arg_count == 1) {

      // We could use the syntax: "config_line_array[0], config_line_array[1]" but we can also
      // just itterate the array. Fut first, we have to pop the first value (the key).
      config_line_array += 1;                        // strip the 'key' from the array
      while(*config_line_array) {
        if(commentflag == 0) {
          if(memcmp(*config_line_array, "#", 1) == 0) {
            break;
          }
        }
        printf("%s ", *config_line_array++);
      }
      printf("\n");
    }

    // If the argument is equal to 'key=value' make a replacement.
    if(arg_count > 1) {
      // Determine if we have a change to make; compare the arg_array and the config_array value.
      //
      // TODO: Assemble the array of char arrays into a temporary string and compiare those strings
      //       instead of compairing a single element from each array.
      if (strncmp(arg_array[1], config_line_array[1], strlen(arg_array[1])) == 0) {
        printf("You have an exact match! %s & %s\n", arg_array[1], config_line_array[1]);
      } else {
        // TODO: Clean up output to match `sysrc` output.
        printf("OLD:  %-5s\t=\t%s;\n", config_line_array[0], config_line_array[1]);
        printf("NEW:  %-5s\t=\t%s;\n", config_line_array[0], arg_array[1]);
        replacevariable(config_line_array[0], arg_array, arg_count, file_string);
      }
    }
  }

  // cleanup
  free_config(config_array, config_count);
  // The following line "free(arg_array)" causes a malloc error.
//:~    free(arg_array);

  return 0;
} ///:~
