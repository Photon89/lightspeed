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

AC_ARG_WITH(GL-prefix, [  --with-GL-prefix=PFX    Prefix where OpenGL is installed (optional)])

GL_CFLAGS=""
GL_LDOPTS=""
if test -n "$with_GL_prefix" ; then
	GL_CFLAGS="-I$with_GL_prefix/include"
	GL_LDOPTS="-L$with_GL_prefix/lib"
fi

AC_CHECK_LIB(GL, glBegin, , AC_MSG_ERROR([OpenGL library not found.]), $GL_LDOPTS)

GL_LIBS="$GL_LDOPTS -lGL"

#
# Check for GtkGLArea widget
#

AC_ARG_WITH(gtkgl-prefix, [  --with-gtkgl-prefix=PFX Prefix where GtkGLArea is installed (optional)])

GTKGL_CFLAGS=""
GTKGL_LDOPTS="" 
if test -n "$with_gtkgl_prefix" ; then
	GTKGL_CFLAGS="-I$with_gtkgl_prefix/include"
	GTKGL_LDOPTS="-L$with_gtkgl_prefix/lib"
fi

AC_CHECK_LIB(gtkgl-2.0, gtk_gl_area_new, ,
	AC_MSG_ERROR([Cannot find GtkGLArea library.]),
	$GTKGL_LDOPTS $GTK_LIBS $GL_LIBS)

GTKGL_LIBS="$GTKGL_LDOPTS -lgtkgl-2.0"

#
# Test for libpng
#

  if test -z "$LIBPNG_LIB"; then
    AC_CHECK_LIB(png, png_read_info,
      AC_CHECK_HEADER(png.h,
	png_ok=yes,
	png_ok=no),
      AC_MSG_WARN(*** PNG exporter cannot be built (PNG library not found) ***),
      -lz -lm)
    if test "$png_ok" = yes; then
      AC_MSG_CHECKING([for png_structp in png.h])
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <png.h>]], [[png_structp pp; png_infop info; png_colorp cmap; png_create_read_struct;]])],[png_ok=yes],[png_ok=no])
      AC_MSG_RESULT($png_ok)
      if test "$png_ok" = yes; then
        PNG='png'; LIBPNG_LIB='-lpng -lz -lm'
      else
        AC_MSG_WARN(*** PNG exporter cannot be built (PNG library is too old) ***)
      fi
    else
      AC_MSG_WARN(*** PNG exporter cannot be built (PNG header file not found) ***)
    fi
  fi

#
# Test for libtiff
#

  if test -z "$LIBTIFF_LIB"; then
    AC_CHECK_LIB(tiff, TIFFReadScanline,
      AC_CHECK_HEADER(tiffio.h,
	TIFF='tiff'; LIBTIFF_LIB='-ltiff',
	AC_MSG_WARN(*** TIFF exporter cannot be built (TIFF header files not found) ***)),
      AC_CHECK_LIB(tiff, TIFFWriteScanline,
	AC_CHECK_HEADER(tiffio.h,
          TIFF='tiff'; LIBTIFF_LIB='-ltiff -ljpeg -lz',
	  AC_MSG_WARN(*** TIFF exporter cannot be built (TIFF header files not found) ***)),
	AC_CHECK_LIB(tiff34, TIFFFlushData,
	  AC_CHECK_HEADER(tiffio.h,
            TIFF='tiff'; LIBTIFF_LIB='-ltiff34 -ljpeg -lz',
	    AC_MSG_WARN(*** TIFF exporter cannot be built (TIFF header files not found) ***)),
        AC_MSG_WARN(*** TIFF exporter cannot be built (TIFF library not found) ***), -ljpeg -lz -lm), -ljpeg -lz -lm), -lm)
  fi

if test "$PNG" = png ; then
	AC_DEFINE(HAVE_LIBPNG)
fi
if test "$TIFF" = tiff ; then
	AC_DEFINE(HAVE_LIBTIFF)
fi

IMG_LIBS="$LIBPNG_LIB $LIBTIFF_LIB"

#
# That's a wrap!
#

CFLAGS="$CFLAGS $GTK_CFLAGS $GL_CFLAGS $GTKGL_CFLAGS"
LIBS="$LIBS $GTKGL_LIBS $GTK_LIBS $GL_LIBS $IMG_LIBS"

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

AH_TEMPLATE([HAVE_LIBPNG])
AH_TEMPLATE([HAVE_LIBTIFF])

# end configure.in
