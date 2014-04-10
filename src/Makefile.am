AM_CFLAGS = -pedantic-errors -Werror -Wall -Wfatal-errors -Wwrite-strings

AM_CPPFLAGS = $(MSSH_CFLAGS)

bin_PROGRAMS = mssh

mssh_SOURCES = mssh.c mssh-terminal.c mssh-window.c mssh-pref.c \
	mssh-gconf.c
mssh_LDADD = $(MSSH_LIBS) $(LTLIBINTL)

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

EXTRA_DIST = mssh-window.h mssh-terminal.h mssh-pref.h mssh-gconf.h
