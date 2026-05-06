# Changelog

- process CLI args with getopt

v1.3.1 - 2026-05-07
- Added macros to call for cleanup and errors. This cleans up the code
  a bit and makes it more standarized (less code duplication and
  errors, now consitatly print to stderr).
- Added support to supply arguments in any order. This allows the
  ability to call the program in a number of different ways such as:
```
     % sysconf -n -f <file.conf> key=value
     % sysconf -f <file.conf> -n key=value
     % sysconf key=value -n -f <file.conf>
     ...
```
- Changed argument count index to 1 (instead of 0) to skip program
  name (index 0).
- Added a github workflow test.yml file from: @msimerson to build and
  run tests.

v1.3.0 - 2026-05-05
- Added ability to handle indented key/value strings.

v1.2.0 - 2026-05-04
- SoC: Moved printing (to STDOUT) from `sysconf.c` to the `print-config.c`.
- Cleaned up memory managment.
- Removed reference to "Semantic Versioning". This should allow me to
  version any code related changes that do not add features or are for
  bug/issue fixes.

v1.1.2 - 2026-05-04
- Corrected config file creating portion whereas file was immediately
  closed.
- Corrected issue where strings were not being matched exactly in
  either key or value.

v1.1.1 - 2026-04-29
- Corrected read-after-free bug.

v1.1.0 - 2026-04-28
- Updated documentation.

v1.0.0 - 2026-04-28
- Introduced static versioning via src/version.h
- Added CHANGELOG.md for project tracking
- Removed Git-based version generation from the Makefile
