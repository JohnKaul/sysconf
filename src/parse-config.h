/**
 * This code defines a `config_t` structure to hold the configuration
 * data, which consists of an array of char arrays and a count value
 * (of the number of item=value entries in the config file).
 *
 *           // Structure to hold the configuration data
 *           typedef struct {
 *               int value_count;
 *               char** values;
 *           } config_t;
 *
 * The `parse_config` function reads the configuration file, counts
 * the number of lines, allocates memory for the configuration data
 * struture, and then passes the configuration line--constructed in
 * the typical `name = value;` syntax to the `make_argv()` function to
 * tokenize the string, and  allocate the memory for an array of
 * pointers to the char arrays. The `get_value` function retrieves a
 * value from the data structure and returns an array of char arrays
 * of the value associated with that name.  The `free_config` function
 * frees the allocated memory for the configuration data.
 *
 * The configuation file can contain comment lines in either c-style
 * or shell-style. Values should be terminated with a semi-colon (;).
 * This configuration style resembles the UCI style FreeBSD uses,
 * although it does not support the `+=` or `-=` directives although,
 * plans are to add that feature set in the future.
 *
 * If the config line item contains in-line comments, the comment is
 * also returned in the char array (it is not ommited in the parsing
 * step). However, using a simple check durring iteration can ommit
 * the remainder of the array.
 *
 * For example, if the configuration file looks like:
 *      item1 = value2;                 # comment string returned.
 * The array will contain the comment string. Iteration of the array
 * can be stopped upon reaching the comment string.
 *
 *      // Get the values.
 *      char **args = get_value(config, count, argv[2]);
 *      args += 1;      // skip name. -i.e. args[0].
 *
 *      // Iterate the returned array of char arrays
 *      // and print the values.
 *      while(*args) {
 *        if(memcmp(*args, "#", 1) == 0)
 *          break;
 *        printf("%s ", *args++);
 *      }
 *
 * EXAMPLE CONFIG FILE:
 * ---
 *      / **
 *       * This is a test header
 *       * with multiple lines.
 *       * /
 *      item1 = value2;                 # comment string returned.
 *      item2  = value2 subvalue2;
 *
 *      / **
 *       * comment = noprint
 *       * /
 *       config {
 *              item3=value3;
 *              item4=value4;
 *          item5.subitem5 = /bin/sh /etc/rc;
 *       }
 * ---
 *
 * Example usage:
 *
 *      int main(int argc, char *argv[]) {
 *
 *        // --Check the command line arguments.
 *        //   if there are not enough arguments, exit.
 *        if (argc != 3) {
 *          fprintf(stderr, "Usage: %s <configuration file> <value to get>\n", argv[0]);
 *          return 1;
 *        }
 *
 *        int count;
 *        char delimiters[] = " \n\"=:;";
 *
 *        config_t* config = parse_config(argv[1], &count, delimiters);
 *
 *        if (!config) {
 *          printf("Failed to parse the configuration file.\n");
 *          free(config);
 *          return 1;
 *        }
 *
 *        // Get the values associated with the argument passed to this function.
 *        char **args = get_value(config, count, argv[2]);
 *        args += 1; // skip name.
 *
 *        // Iterate the returned array of char arrays and print the values.
 *        while(*args) {
 *          printf("%s ", *args++);
 *        }
 *        printf("\n");
 *
 *        free_config(config, count);
 *        return 0;
 *      } ///:~
 *
 */

// Configuration data structure
typedef struct {
    int value_count;
    char** values;
} config_t;

//: free_config
//      Free the allocated memory for the configuration data.
void free_config(config_t *config,int count);

//: get_value
//      Function to find a configuration item by name.
char **get_value(config_t *config,int count,const char *name);

//: parse_config
//      Parse the configuration file and store the data in a config_t
//      array.
config_t *parse_config(const char *filename,int *count, char *delimiters);

//: print_config_item
//      Function to print a configuration item's value to STDOUT.
//      NOTE: only the first value is printed.
void print_config_item(config_t *config, int count, const char *name);

//: find_config_item
//      A function to find a configuration value given a name.
config_t* find_config_item(config_t* config, const char* name, int count);

//: contains
//      A function to find a value in a char array.
int contains(char **array, int size, const char *value);

//: count_tokes
//      Counts the number of tokens in the input string based on the delimiters.
//
int count_tokens(const char *input_string, const char *delimiters);

//: populate_array
//      Populates the argv array with tokens from the input string.
int populate_array(const char *input_string, const char *delimiters, char ***argvp, int tokens);

//: make_argv
//      This function will take a string and tokenize it based on a
//      delimeter and store the pointers to the strings into an array
//      for easier access.
//
//  NOTE:
//      The concept for this function came from the wonderful book
//      "Unix(TM) Systems Programming: Communication, Concurrency, and
//      Threads" By Kay A. Robbins, Steven Robbins
//      However the one listed in the book had a memmory leak, I have
//      made this version to correct that issue and be a bit more robust.
int make_argv(const char *buf, const char *delimiters, char ***argvp);
