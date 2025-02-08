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
 	doc/sysconf.md
 
 $(eval $(call make-mdoc,$(_target),$(_source)))
#:~

## mdoc:
## 	@md2mdoc doc/sysconf.md doc/sysconf.7

# vim: set noet
