# configure.am
#
# Process this file with autoconf to produce a configure script.
#
### AC_INIT
AC_CONFIG_SRCDIR([src/lightspeed.c])

m4_define([lightspeed_major_version], [1])
m4_define([lightspeed_minor_version], [2a])
m4_define([lightspeed_version], [lightspeed_major_version.lightspeed_minor_version])

AC_INIT([lightspeed],[lightspeed_version])

AC_CONFIG_SRCDIR([src/lightspeed.c])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE

# Check for programs
#
AM_SANITY_CHECK
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CPP

# Check for header files
#
AC_CHECK_HEADERS(getopt.h malloc.h sys/time.h)

# Check for typedefs, structures, and compiler characteristics.
#
AC_C_CONST
AC_TYPE_SIZE_T

# Check for library functions
#
AC_CHECK_FUNCS(getopt_long gettimeofday strcspn strdup strtod)

# Checks for GTK+ libraries.
#
AM_PATH_GTK_2_0(2.0.1, , AC_MSG_ERROR([Cannot find proper GTK+ version]))

#
# Check for OpenGL libraries
#
PKG_CHECK_MODULES([GL], [gl])
CFLAGS="$CFLAGS $gl_CFLAGS"
LDFLAGS="$LDFLAGS $gl_LIBS"

#
# Check for GtkGLArea widget
#
PKG_CHECK_MODULES([GTKGL], [gtkgl-2.0])
CFLAGS="$CFLAGS $gtkgl_CFLAGS"
LDFLAGS="$LDFLAGS $gtkgl_LIBS"

#
# Add maths
#

MATH_LIBS="-lm"

#
# That's a wrap!
#

CFLAGS="$CFLAGS $GTK_CFLAGS $GL_CFLAGS $GTKGL_CFLAGS"
LIBS="$LIBS $GTKGL_LIBS $GTK_LIBS $GL_LIBS $MATH_LIBS"

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

# end configure.ac
