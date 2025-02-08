# ===--------------------------------------------*- makefile -*---===
#: sysconf
# This makefile is used by my project-root makefile and is
# located in the source directory.
#
# _target       =       program name
# _depend       =       required libaries
# _source       =       source file list
# ===-------------------------------------------------------------===

#: sysconf
_target	:= sysconf
_depend :=
_source	:= \
	src/main.c \
	src/abort.c \
	src/include/parse-config.c \
	src/include/print-config.c

$(eval $(call make-program,$(_target),$(_source),$(_depend)))
#:~

# vim: set noet
