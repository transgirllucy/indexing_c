# configure.ac
AC_INIT([file_indexer], [1.0], [you@example.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
PKG_CHECK_MODULES(SQLITE3, [sqlite3])
AC_SUBST(SQLITE3_CFLAGS)
AC_SUBST(SQLITE3_LIBS)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
