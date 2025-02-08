#===---------------------------------------------*- makefile -*---===
#: sysconf
# This file was automatically generated
#===--------------------------------------------------------------===

sysconf : HEADERS	=	\
	src/include/parse-config.h	\
	src/include/print-config.h	\
	src/abort.h

sysconf : SOURCES	=	\
	src/include/print-config.c	\
	src/include/parse-config.c	\
	src/abort.c	\
	src/main.c

#--------------------------------------------------------------------
# Set the project directories and build parameters.
#--------------------------------------------------------------------
SRCDIR		=	src
DOCDIR		=	doc
INCDIR		:=	src/include

PREFIX		:=	/usr/local/bin

CC			:=	cc
CFLAGS		:=	-fno-exceptions -pipe -Wall -W -I $(SRCDIR) -I $(INCDIR)
REMOVE		:=	rm -f
CP			:=	cp
CTAGS       :=	ctags

#--------------------------------------------------------------------
# Define the target compile instructions.
#--------------------------------------------------------------------
sysconf: ctags cleanobjs
	SYSCONF_TARGET='sysconf'
		@$(CC) $(CFLAGS) -o sysconf $(SOURCES)
		@md2mdoc $(DOCDIR)/sysconf.md $(DOCDIR)/sysconf.7

.PHONY: clean
clean:
	@$(REMOVE) sysconf $(OBJECTS)

.PHONY: ctags
ctags:
	@$(CTAGS) $(SOURCES) $(INCDIR)/.

.PHONY: cleanobjs
cleanobjs:
	@$(REMOVE) $(OBJECTS)

.PHONY: install
install:
	@$(CP) sysconf $(PREFIX)/sysconf

.PHONY: all
all: sysconf

# vim: set noet
