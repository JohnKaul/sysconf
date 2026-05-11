<!--------------------------------------------*- MARKDOWN -*------
File Last Updated: October 21 2020 16:01

File:    readme.md
Author:  John Kaul <john.kaul@outlook.com>
Brief:   This section should contain information about this project.
------------------------------------------------------------------>

# sysconf

## BRIEF

A utility to make changes to key/value type system configuration
files.

## SYNOPSIS
sysconf -f file.conf

sysconf -f file.conf -d file.conf.defaults

sysconf -f file.conf [-n] [key]

sysconf -f file.conf [key=value]

sysconf -f file.conf [key+=value]

sysconf -f file.conf [key-=value]

## OPTIONS
-f      Configuration file

-d      Check the `configfile`'s key values against `configfile.defaults`'s key values for duplicate entries.

-n      Display "key" as well when retrieving a variable. The default method is to only display a key's value but this option makes the return show both the key and the value.

## DESCRIPTION
This utility will print/change a configuration value stored in a configuration file formatted in a simple key/value syntax.

Operating systems and user tools have many configuration files which can be kept in many different syntax languages. On FreeBSD there are a few tools to parse system type configuration files (`sysrc` and `sysctl` for example) to parse/change a few of the different configuration syntaxes and each tool has it's own command flags and options. Managing system configuration files is typically done with tools like the ones listed or with an editor.

The current tool(s) in FreeBSD (`sysrc`) can be made to work on other files other than /etc/rc.conf but the tool does not handle configuration files written in UCL -i.e., with a dot in the name -e.g., jail.conf files. Other configuration settings (like: `sysctl` for kernel settings) are also kept using a different style than UCL. This utility is meant to offer simple key/value changes, for the different configuration styles/languages, in one tool (in lieu of several for example).  Although, this tool--while not being terribly robust and exhaustive--should offer the ability to make changes to many different style configuration key/value entries like the `sysrc` and `sysctl` type of tools with no external library dependencies. Meaning, this tool does not rely on third party libraries to parse UCL, JSON, etc. it uses a simple tokeniser to parse the entries and syntax to make changes/additions. Because this tool does not use external libraries, this ultimately limits it's use--it will not parse XML for example--but should offer a more consistent user interface for making changes to key/value type configuration files.

By design this utility does not have many flags/options to specify actions to perform different actions, instead this utility will try to determine what to do (-i.e., add/change) based on the arguments given. See the examples below. This utility will also try to determine the syntax of the key/value pair by reading the configuration file (-i.e., it will try to determine if the value needs quotes or a termination character and either spaces or an equal sign or colon separation character).

Example configuration file syntaxes this utility can parse/change:
```conf
    key=value
    key = value;
    key.subkey = value;
    key="value"
    key value
    key value;
    key:"value"
    key = "value1 value2";
    etc.
```

*NOTE*:
- Keys or values can contain special characters like dollarsigns ($). To pass these characters to this utility, the dollar sign must be escaped with a slash (\\) or surrounded with single quotes (').
- This utility cannot read multi-line configuration values.
- This utility cannot locate configuration keys in "sections". For example, if your configuration file (like an .htaccess file for Apache) contains multiple entries with the same key in separate sections.
- This utility was not meant to replace a text editor; it is meant to offer simple(er) changes via scripting/automation.

It is also possible to check a configuration file's key/values against a default configuration file which will search for duplicate values in keys listed in the configuration file. For example, in FreeBSD the /etc/rc.conf file is included from the file /etc/defaults/rc.conf, which specifies the default settings for all the available options.  Options need only be specified in /etc/rc.conf when the system administrator wishes to override these defaults. See the example section below for 'checking for duplicates'.


## FEATURES
* Simple syntax - no command flags to remember and consistent syntax no matter the configuration language used.
* Will not write a key twice - if a key/value already exists, this utility will not write a second entry.
* Zero dependencies - will not break or fall behind if library is (not) updated.
* Add or Remove values with either '+=' or '-='.
* Check configuration file's key values against a default configuration file's default key values.

## EXAMPLES
To list off some key/values in a configuration file.
```sh
    syconf -f /path/file.conf
```

To retrieve the value from a key in a file.
```sh
    sysconf -f /path/file.conf key
```

If there is a need to produce the key as well as the value in the output of a 'get a value' operation, the `-n` switch can be used.
```sh
    sysconf -f /path/file.conf -n key
```

To change a value associated with a key.
```sh
    sysconf -f /path/file.conf key=value
```

To add to a value associated with a key.
```sh
    sysconf -f /path/file.conf key+=value
```

To remove a value associated with a key.
```sh
    sysconf -f /path/file.conf key-=value
```

To use a dollar sign in a key, escape it.
```sh
    sysconf -f /path/file.conf \\$key
```

To search for duplicate values in a configuration file against a default configuration file key values, use the -f and -d flags.
```sh
    % sysconf -f /path/file.conf -d /path/file.conf.defaults
```
_Real World Example_:
```sh
    % sysconf -f /etc/rc.conf -d /etc/defaults/rc.conf
```

## WHY?
* Why did I create this? I created this utility because I was attempting
  to use `sysrc` on a jail.conf file when I ran into a roadblock about
  the <dot> used in the key/variable syntax. This lead me down a bit of
  a deeper rabbit hole before I just convinced myself "It shouldn't be
  complicated to edit a 'key/value' file".

* This utility certainly isn't perfect but it works very well for the
  cases I need. So, if you're reading this and you think it sucks, well,
  this utility probably isn't for you then. Sorry, this utility does not
  exist to serve all of your configuration file needs. -i.e., it does not do
  YMAL, XML, JSON, etc., this is for simple key/value type files and can
  perform a lot of same operations as `sysrc` for keys or values with
  non standard characters. For the most part--currently--I do not need
  all these configuration file languages because this utility is helping me on
  my servers (no GUI, no desktop, no docker, nothing fancy, just dead
  simple server stuff).

## DOWNLOAD INSTRUCTIONS

To clone a copy:

```sh
    $ cd ~/<place>/<you>/<keep>/<your>/<code>
    $ git clone git@git.local:server/sysconf.git
```

## BUILD INSTRUCTIONS

```sh
    $ cd sysconf
    $ make
    $ doas make install
```

This project also has some unit testing which can be compiled and run.

```sh
    $ cd sysconf
    $ make test
```
This will compile a file called `test_sysconf`.

This project also has a shell script to perform some syntax type
tests.

```sh
    $ chmod u+x ./test_syntax.sh
    $ ./test_syntax.sh
```

## CONTRIBUTION GUIDELINES
Contributions should be single-subject only (single-line bug fixes,
small refactors, spelling/typo fixes, etc.). Changes that touch
multiple files or address several unrelated topics are hard to review
and may have unforeseen side effects; contributions spanning multiple
subjects may be requested to be split into smaller, focused commits or
closed.

This project includes tests you can run against possible changes:
- Syntax tests verify sysconf can perform basic functionality like
  get/set a key using =, +=, and -=. See `test_synatx.sh`.
- Unit tests verify internal library functions and behavior in
  specific scenarios. See `test/test_synsconf.c`.

Run tests:
```sh
    % make test
    % ./test_sysconf    # unit tests
    % ./test_syntax     # syntax tests
```

### Versioning
Use MAJOR.MINOR.PATCH:
- **MAJOR** — incompatible API changes
- **MINOR** — added or improved backward-compatible functionality
- **PATCH** — backward-compatible bug fixes

### Committing
1. List file(s) in the subject line. If committing several files,
   prefix the subject line with "(MF)" (multiple files) and separate
   filenames with commas.
2. Separate subject from body with a blank line.
3. Limit the subject line to 50 characters.
4. Do not end the subject line with a period.
5. Use the imperative mood in the subject line.
6. Wrap the body at 72 characters.
7. Use the body to explain what and why (not how).
8. Update `changelog` and `src/version.h` for each contribution.

Example commit:
```
    (MF) changelog, version.h, sysconf.7

    1. Update documentation.
```

### Use of AI
Code produced with AI assistance may be accepted only if it is:
- Trivial and not copyrightable (e.g., single-line fixes, basic refactors), or
- Accompanied by a public statement from the AI provider that they do
  not assert copyright over the generated work.

AI-assisted code is often harder to maintain and for that reason,
human-authored patches for larger changes is preferred. Large-scope
refactors or additions that appear to be AI-generated can be closed
without explanation.

## HISTORY
* Created for my personal use.

## AUTHOR
* John Kaul
