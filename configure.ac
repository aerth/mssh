AC_INIT([MultiSSH], [2.2], [hector@debian.org], [mssh])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

GETTEXT_PACKAGE=mssh
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",
                    [The prefix for our gettext translation domains.])
IT_PROG_INTLTOOL(0.26)

#AM_GNU_GETTEXT([external])


#AC_SUBST([GETTEXT_PACKAGE], [mssh])
#AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Define to the gettext package name.])

AC_PROG_CC

PKG_CHECK_MODULES(MSSH, [gtk+-3.0 vte-2.91 gconf-2.0])
AC_SUBST(MSSH_CFLAGS)
AC_SUBST(MSSH_LIBS)

AC_PATH_PROG(GCONFTOOL, gconftool-2, no)

if test x"$GCONFTOOL" = xno; then
  AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
fi

AM_GCONF_SOURCE_2

AC_OUTPUT( po/Makefile.in
    Makefile
    src/Makefile
)
