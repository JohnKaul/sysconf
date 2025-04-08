# ===--------------------------------------------*- makefile -*---===
#: sysconf
# This makefile is used by my project-root makefile and is
# located in the source directory.
#
# _target       =       program name
# _depend       =       required libaries
# _source       =       source file list
# ===-------------------------------------------------------------===

#-X- #: sysconf
_target	:= sysconf
#-X- _depend :=
_source	:= \
	src/main.c \
	src/abort.c \
	src/include/parse-config.c \
	src/include/print-config.c

#-X- $(eval $(call make-program,$(_target),$(_source),$(_depend)))
#-X- #:~

# Add your files here
#-X- SOURCES = \
#-X- 	src/main.c \
#-X- 	src/abort.c \
#-X- 	src/include/parse-config.c \
#-X- 	src/include/print-config.c

EXECUTABLE=     $(_target)

# vim: set noet
