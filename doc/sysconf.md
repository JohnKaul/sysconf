author: John Kaul
date: Feb 07 2025
title: sysconf.7

# NAME
sysconf

# SYNOPSIS
sysconf
[-f configfile]
[key=value]

# OPTIONS
-f
Configuration file

-

# DESCRIPTION
This utility will print/change a configuration value stored in a configuration file formatted in the typical key/value syntax.

By design this utility does not have any flags to specify actions to preform different actions, instead this utility will determine what to do based on the arguments given. See the examples below. This utility will also try to determine the syntax of the key/value pair by reading the configuration file (-i.e. it will try to determine if the value needs quotes or a temination character and either spaces or an equal sign or colon seperation character).

Configuration file syntaxes this utility can parse:
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
Some keys or values in configuration files can contain special characters like dollarsigns ($). To pass these types of vaiables to this utility, the dollar sign needs to be escaped with a slash (\\) or surrounded with single quates (').


# EXAMPLES
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

To use a dollar sign in a key, escape it.
```
    sysconf -f /path/config.file \\$key
```
