# ===--------------------------------------------*- makefile -*---===
#: sysconf
# This makefile is used by my project-root makefile and is
# located in the source directory.
#
# _target       =       program name
# _source       =       source file list
# ===-------------------------------------------------------------===

#-X- #: sysconf
#-X- _target		:= sysconf.7
#-X- _docsource	:= \
#-X-  		doc/sysconf.md
 
#-X- $(eval $(call make-mdoc,$(_target),$(_docsource)))
#-X- #:~

## mdoc:
## 	@md2mdoc doc/sysconf.md doc/sysconf.7

$(eval $(call make-mdoc,sysconf.7,doc/sysconf.md))

# vim: set noet
