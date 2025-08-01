#===---------------------------------------------*- makefile -*---===
#: sysconf
# This file was automatically generated
#===--------------------------------------------------------------===

sysconf : HEADERS	=	\
	src/parse-config.h	\
	src/print-config.h	\
	src/abort.h

sysconf : SOURCES	=	\
	src/print-config.c	\
	src/parse-config.c	\
	src/abort.c	\
	src/sysconf.c

TEST_HEADERS	=	\
	test/minunit.h

TEST_SOURCES	=	\
	src/print-config.c	\
	src/parse-config.c	\
	src/abort.c	\
	test/test_sysconf.c

#--------------------------------------------------------------------
# Set the project directories and build parameters.
#--------------------------------------------------------------------
SRCDIR 		= 	src
DOCDIR		=	doc

PREFIX		:=	/usr/local/bin
MANPATH		:=	/usr/local/share/man/man7

CC			:=	cc
#-X- CFLAGS		:=	-fno-exceptions -pipe -Wall -W -g -fsanitize=address
CFLAGS		:=	-fno-exceptions -pipe -Wall -W
INCPATH		=	-I $(SRCDIR) -I $(SRCDIR)
REMOVE		:=	rm -f
CP			:=	cp
CTAGS       :=	ctags

OBJECTS = $(SOURCES:.c=.o)
#--------------------------------------------------------------------
# Define the target compile instructions.
#--------------------------------------------------------------------

# By listing the header files as a dependency as well, it ensures that
# if a change was made to a header file then the entire program will
# be rebuilt.

sysconf: $(HEADERS) ctags cleanobjs
	SYSCONF_TARGET='sysconf'
		@$(CC) $(CFLAGS) $(INCPATH) -o sysconf $(SOURCES)

.PHONY: test
test: $(HEADERS) $(TEST_HEADERS)
	TEST='test_sysconf'
		@$(CC) $(CFLAGS) -I test $(INCPATH) -o test_sysconf $(TEST_SOURCES)

.PHONY: clean
clean:
	@$(REMOVE) sysconf $(OBJECTS)

.PHONY: ctags
ctags:
	@$(CTAGS) $(SOURCES) .

.PHONY: cleanobjs
cleanobjs:
	@$(REMOVE) $(OBJECTS)

.PHONY: install
install:
	@$(CP) sysconf $(PREFIX)/sysconf
	@$(CP) ./doc/sysconf.7 $(MANPATH)

.PHONY: uninstall
uninstall:
	$(RM) $(PREFIX)/sysconf
	$(RM) $(MANPATH)/sysconf.7

.PHONY: all
all: sysconf

# vim: set noet
