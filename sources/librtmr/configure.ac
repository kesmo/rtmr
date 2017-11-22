#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([rtmr-lib], [1.8.0], [contact@rtmr.net])
AM_INIT_AUTOMAKE([foreign -Wall])

AC_DEFINE([RTMR_LIBRARY_EXPORTS])
AC_DEFINE([_RTMR])
AC_DEFINE([_POSTGRES])
AC_DEFINE([AUTOMATION_LIB])

AC_CONFIG_SRCDIR([src/client-lib/client.c])
AC_CONFIG_FILES([Makefile])

AC_CONFIG_MACRO_DIR([m4])

AC_PREFIX_DEFAULT([/usr])

AC_DEFUN([_RRA_LIB_ARCH_NAME],
[rra_lib_arch_name=lib
 AC_CHECK_SIZEOF([long])
 AS_IF([test "$ac_cv_sizeof_long" -eq 4 && test -d /usr/lib32],
     [rra_lib_arch_name=lib32],
     [AS_IF([test "$ac_cv_sizeof_long" -eq 8 && test -d /usr/lib64],
         [rra_lib_arch_name=lib64])])])

dnl The public interface.  Set VARIABLE to PREFIX/lib{,32,64} as appropriate.
AC_DEFUN([RRA_SET_LDFLAGS],
[AC_REQUIRE([_RRA_LIB_ARCH_NAME])
 AS_IF([test -d "$2/$rra_lib_arch_name"],
    [$1="-L$2/$rra_lib_arch_name"],
    [$1="-L$2/lib"])])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
LT_INIT([disable-static])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h malloc.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyname inet_ntoa memset strrchr strtoul])

AC_OUTPUT
