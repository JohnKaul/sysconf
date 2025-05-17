/**
 * This code will print a value string (array) to a config file.
 *
 * These functions accpet an array and locate the first value in the
 * array in the config file and replace the config value with the
 * given array contents.
 */

//: replacevariable
//      Replaces a items value in the config file.
int replacevariable(const char *key, char **value, int count, const char *filename);

//: writevariable
//      Write items in value in the config file.
int writevariable(const char *key, char **value, int count, const char *filename);

//: Printconfifile
//      Iterates the `config_array` and prints the items.
void printconfigfile(config_t *config_array,int array_count);

//: assemble_strings
//      This function assebles the array of char arrays into a string
//      (ommiting the first char array which should be the 'key' in a
//      key/value string).
char* assemble_strings(char **value, int count);

//: add_to_array
//      This function will append a string to given `argvp`.
int add_to_array(char ***argvp, int size, const char *string);
