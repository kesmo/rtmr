#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([netserver], [1.8.0], [contact@rtmr.net])
AM_INIT_AUTOMAKE([foreign -Wall])

AC_DEFINE([_POSTGRES])
AC_DEFINE([_RTMR])
AC_DEFINE([AUTOMATION_LIB])

AC_CONFIG_SRCDIR([src/netserver/main.c])
AC_CONFIG_FILES([Makefile])

AC_PROG_LN_S

#AC_CONFIG_MACRO_DIR([m4])

#AC_PREFIX_DEFAULT([/usr])

# Checks for programs.

AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyname inet_ntoa memset strrchr strtoul])

AC_OUTPUT
