# Changelog

- Add test cases, CI testing, fix typos.

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
