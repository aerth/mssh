AC_INIT([MultiSSH], [0.1], [brad@brad-smith.co.uk], [mssh])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

AC_PROG_CC

PKG_CHECK_MODULES(MSSH, [gtk+-2.0 vte])
AC_SUBST(MSSH_CFLAGS)
AC_SUBST(MSSH_LIBS)

AC_OUTPUT(
    Makefile
    src/Makefile
)
