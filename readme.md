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
sysconf -f configfile [key=value]

## OPTIONS
-f      Configuration file

## DESCRIPTION
This utility will print/change a configuration value stored in a configuration file formatted in a simple key/value syntax.

Operating systems and user tools have many configuration files which can be kept in many different syntax languages. On FreeBSD there are a few tools to parse system type configuration files (`sysrc` and `sysctl` for example) to parse/change a few of the different configuration syntaxes and each tool has it's own command flags and options. Managing system configuration files is typically done with tools like the ones listed or with an editor.

The current tool(s) in FreeBSD (`sysrc`) can be made to work on other files other than /etc/rc.conf but the tool does not handle configuration files written in UCL -i.e., with a dot in the name -e.g., jail.conf files. Other configuration settings (like: `sysctl` for kernel settings) are also kept using a different style than UCL. This utility is meant to offer simple key/value changes, for the different configuration styles/languages, in one tool (in lieu of several for example).  Although, this tool--while not being terribly robust and exhaustive--should offer the ability to make changes to many different style configuration key/value entries like the `sysrc` and `sysctl` type of tools with no external library dependencies. Meaning, this tool does not rely on third party libraries to parse UCL, JSON, etc. it uses a simple tokeniser to parse the entries and syntax to make changes/additions. Because this tool does not use external libraries, this ultimately limits it's use--it will not parse XML for example--but should offer a more consistent user interface for making changes to key/value type configuration files.

By design this utility does not have many flags/options to specify actions to preform different actions, instead this utility will try to determine what to do (-i.e., add/change) based on the arguments given. See the examples below. This utility will also try to determine the syntax of the key/value pair by reading the configuration file (-i.e., it will try to determine if the value needs quotes or a termination character and either spaces or an equal sign or colon separation character).

Example configuration file syntaxes this utility can parse/change:
```
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
- Some keys or values in configuration files can contain special characters like dollarsigns ($). To pass these types of variables to this utility, the dollar sign needs to be escaped with a slash (\\) or surrounded with single quotes (').
- This utility will not be able to read multi-line configuration values.
- This utility will not be able to locate configuration keys in "sections". For example, if your configuration file (like an .htaccess file for Apache) contains multiple entries with the same key in separate sections.
- This utility was not meant to replace the use of a text editor; it is only meant to offer simple(er) changes via scripting/automation.

## FEATURES
* Simple syntax - no command flags to remember and consistent syntax no matter the configuration language used.
* Will not write a key twice - if a key/value already exists, this utility will not write a second entry.
* Zero dependencies - will not break or fall behind if library is (not) updated.
* Add or Remove vlues with either '+=' or '-='.

## EXAMPLES
To list off some key/values in a config file.
```
    syconf -f /path/config.file
```

To retrieve the value from a key in a file.
```
    sysconf -f /path/config.file key
```

To change a value associated with a key.
```
    sysconf -f /path/config.file key=value
```

To add to a value associated with a key.
```
    sysconf -f /path/config.file key+=value
```

To remove a value associated with a key.
```
    sysconf -f /path/config.file key-=value
```

To use a dollar sign in a key, escape it.
```
    sysconf -f /path/config.file \\$key
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
  exist to serve all of your config file needs. -i.e., it does not do
  YMAL, XML, JSON, etc., this is for simple key/value type files and can
  preform a lot of same operations as `sysrc` for keys or values with
  non standard characters. For the most part--currently--I do not need
  all these config file languages because this utility is helping me on
  my servers (no GUI, no desktop, no docker, nothing fancy, just dead
  simple server stuff).

## DOWNLOAD INSTRUCTIONS

To clone a copy:

```bash
$ cd ~/<place>/<you>/<keep>/<your>/<code>
$ git clone git@git.local:server/sysconf.git
```

## BUILD INSTRUCTIONS

```bash
$ cd sysconf
$ make
```

## CONTRIBUTION GUIDELINES

### Git Standards

#### Commiting

1.  Commit each file as changes are made.
2.  Do not commit files in batch.
3.  Please prefix all commits with the file you are commiting.
4.  Separate subject from body with a blank line
5.  Limit the subject line to 50 characters
6.  Capitalize the subject line
7.  Do not end the subject line with a period
8.  Use the imperative mood in the subject line
9.  Wrap the body at 72 characters
10. Use the body to explain what and why vs. how

## HISTORY
* Created for my personal use.

## AUTHOR
* John Kaul
