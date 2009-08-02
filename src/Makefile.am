AM_CFLAGS = -pedantic-errors -Werror -Wall -Wfatal-errors -Wwrite-strings

INCLUDES = $(MSSH_CFLAGS)

bin_PROGRAMS = mssh

mssh_SOURCES = mssh.c mssh-window.c
mssh_LDADD = $(MSSH_LIBS)

EXTRA_DIST = mssh-window.h
