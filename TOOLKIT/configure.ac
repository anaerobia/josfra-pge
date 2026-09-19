#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# configure.ac file for SDP Toolkit. Created 6/12/13


# Prerequisite of Autoconf 2.59 or above needed to run configure.ac
AC_PREREQ(2.59)
AC_INIT([SDP Toolkit], [5.2.20], [na@no.com])
AC_CONFIG_HEADER([config.h])

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])

# Takes a list of options that should be applied to every Makefile.am 
# when automake is run.
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_MAINTAINER_MODE

# It's possible to configure for a host other than the one on which
# configure is currently running by using the --host=foo flag.
# For machines on which HDF5 is often configured, it can be convenient
# to specify the name of the machine rather than its canonical type.
case $host_alias in
  redstorm)
    host_alias=x86_64-redstorm-linux-gnu
    ;;
esac

AC_CANONICAL_HOST
AC_SUBST([CPPFLAGS])

# H5_CFLAGS (and company) are for CFLAGS that should be used on HDF5, but
# not exported to h5cc (or h5fc, etc.)
AC_SUBST([H5_CFLAGS])
AC_SUBST([H5_CPPFLAGS])
AC_SUBST([H5_FCFLAGS])
AC_SUBST([H5_CXXFLAGS])
AC_SUBST([H5_LDFLAGS])

AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_FCFLAGS])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_LDFLAGS])

# Ensures that all flags are set to something
AM_CFLAGS="${AM_CFLAGS}"
AM_CXXFLAGS="${AM_CXXFLAGS}"
AM_FCFLAGS="${AM_FCFLAGS}"
AM_CPPFLAGS="${AM_CPPFLAGS}"
AM_LDFLAGS="${AM_LDFLAGS}"
CFLAGS="${CFLAGS}"
CXXFLAGS="${CXXFLAGS}"
FCFLAGS="${FCFLAGS}"
CPPFLAGS="${CPPFLAGS}"
LDFLAGS="${LDFLAGS}"

# Different compilers may need different libraries. These files are saved
# under the /config directory. Statement will be adjusted to define which file
DEFAULT_LIBS=""

# Check if Fortran 90 compiler exists
if test "x" = "x$FC"; then
  FC=${F9X}
fi

# Set prefix default (install directory) to a directory in the build area.
AC_PREFIX_DEFAULT([`pwd`])

# -------------------------------------------------------------------------
# Information on the package
# ------------------------------------------------------------------------

# Dump all shell variables values.
AC_MSG_CHECKING([shell variables initial values])
set >&AS_MESSAGE_LOG_FD
AC_MSG_RESULT([done])

# Define all symbol variables used for configure summary
AC_SUBST(FC) BUILD_FORTRAN=no
AC_SUBST(STATIC_SHARED)
AC_SUBST(enable_shared)
AC_SUBST(enable_static)
AC_SUBST(UNAME_INFO) UNAME_INFO=`uname -a`
AC_SUBST(BRAND) BRAND=`uname | awk -F_ '{print $1}'`
AC_SUBST(HDFSYS) HDFSYS=unknown
AC_SUBST(BUILD_MAC_LIBICONV) BUILD_MAC_LIBICONV=no
AC_SUBST(BUILD_CYG_LIBICONV) BUILD_CYG_LIBICONV=no

# Check for 32-bit flag. If enabled, toolkit will be compiled in 32-bit
AC_MSG_CHECKING([if compiling in 32-bit])
AC_ARG_ENABLE([32-bit],
              [AC_HELP_STRING([--enable-32-bit],
                              [Typically 32-bit systems will automatically
				compile in 32-bit and 64-bit systems will 
				compile in 64-bit. However, the flag will
				allow 64-bit systems to compile 32-bit 
				libraries if they want.])],
              [ENABLE_32_BIT=$enableval])
if test "X$ENABLE_32_BIT" = "Xyes"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

# Check if user would like AA files to be built
AC_MSG_CHECKING([if building AA libraries])
AC_ARG_WITH([aa],
              [AC_HELP_STRING([--with-aa],
                              [Flag to build AA libraries. AA files need their
				own seperate packages and some users may not
				want to include the AA tools with the SDP
				Toolkit. Otherwise, this flag can be set.])],
              [WITH_AA=$withval])

if test "X$WITH_AA" = "Xyes"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([BUILD_AA], [test "X$WITH_AA" = "Xyes"])


# Check for debug flag. If enabled, toolkit will add -g flag to CPPFLAGS
AC_MSG_CHECKING([if debug is enabled])
AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug],
                              [Users have the option whether or not they would
				like to enable the debug feature of the toolkit.
				This simply adds the -g flag to the CPPFLAGS
				listed below.])],
              [ENABLE_DEBUG=$enableval])
if test "X$ENABLE_DEBUG" = "Xyes"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi


# Some platforms have broken basename, and/or xargs programs. Check that it
# actually does what it's supposed to do.
AC_MSG_CHECKING([if basename works])
BASENAME_TEST="`basename /foo/bar/baz/qux/basename_works`"
if test $BASENAME_TEST != "basename_works"; then
  AC_MSG_ERROR([basename program doesn't work])
else
  AC_MSG_RESULT([yes])
fi

AC_MSG_CHECKING([if xargs works])
XARGS_TEST="`echo /foo/bar/baz/qux/xargs_works | xargs basename`"
if test $XARGS_TEST != "xargs_works"; then
  AC_MSG_ERROR([xargs program doesn't work])
else
  AC_MSG_RESULT([yes])
fi

# Check that the cache file was built on the same current host. Else, returns
# an error
AC_CACHE_CHECK([for cached host], [hdf5_cv_host], [hdf5_cv_host="none"]);
if test $hdf5_cv_host = "none"; then
  hdf5_cv_host=$host
elif test $hdf5_cv_host != $host; then
  echo "The config.cache file was generated on $hdf5_cv_host but"
  echo "this is $host.  Please remove that file and try again."
  AC_MSG_ERROR([config.cache file is invalid])
fi

# ----------------------------------------------------------------------
# Source any special files that we need.  These files normally aren't
# present but can be used by the maintainers to fine tune things like
# turning on debug or profiling flags for the compiler.  The search order
# is:
#
#      CPU-VENDOR-OS
#      VENDOR-OS
#      CPU-OS
#      CPU-VENDOR
#      OS
#      VENDOR
#      CPU
#
# If the `OS' ends with a version number then remove it. For instance,
# `freebsd3.1' would become `freebsd'

case $host_os in
  aix*)
    host_os_novers=aix
    ;;
  freebsd*)
    host_os_novers=freebsd
    ;;
  irix5.*)
    host_os_novers=irix5.x
    ;;
  irix6.*)
    host_os_novers=irix6.x
    ;;
  osf4.*)
    host_os_novers=osf4.x
    ;;
  osf5.*)
    host_os_novers=osf5.x
    ;;
  solaris2.*)
    host_os_novers=solaris2.x
    ;;
  *)
    host_os_novers=$host_os
    ;;
esac

host_config="none"
for f in $host_cpu-$host_vendor-$host_os \
         $host_cpu-$host_vendor-$host_os_novers \
         $host_vendor-$host_os \
         $host_vendor-$host_os_novers \
         $host_cpu-$host_os \
         $host_cpu-$host_os_novers \
         $host_cpu-$host_vendor \
         $host_os \
         $host_os_novers \
         $host_vendor \
         $host_cpu ; do
  AC_MSG_CHECKING([for config $f])
  if test -f "$srcdir/config/$f"; then
    host_config=$srcdir/config/$f
    AC_MSG_RESULT([found])
    break
  fi
  AC_MSG_RESULT([no])
done
if test "X$host_config" != "Xnone"; then
  CC_BASENAME="`echo $CC | cut -f1 -d' ' | xargs basename 2>/dev/null`"
  . $host_config
fi

# Source any special site-specific file
hname="`hostname`"
while test -n "$hname"; do
  file=$srcdir/config/site-specific/host-$hname
  AC_MSG_CHECKING([for config $file])
  if test -f "$file"; then
    . $file
    AC_MSG_RESULT([found])
    break
  fi
  AC_MSG_RESULT([no])
  hname_tmp=$hname
  hname="`echo $hname | cut -d. -f2-99`"
  test "$hname_tmp" = "$hname" && break
done

# Specify the brand of operating system and label them accordingly. This
# will be done for every compatiable OS for the SDP Toolkit. This is where
# the flags for each architecture are set and the names of the folders are
# established. If the architecture is not listed, then the default name of
# of the architecture will be used. If there are any missing flags, here is
# where you can set them.
#
# Use INSTALL-Toolkit file in the ./bin/linux directory as reference to find
# any missing flags for a particular architecture.
# ibm
if test "X$BRAND" = "XAIX"; then
  HDFSYS="IBM6000"
  BRAND="ibm"
fi

# hp
if test "X$BRAND" = "XHP-UX"; then
  HDFSYS="HP9000"
  BRAND="hp"
fi

# sgi
if test "X$BRAND" = "XIRIX"; then
  HDFSYS="IRIX"
  BRAND="sgi"
fi

# Linux
if test "X$BRAND" = "XLinux"; then
  HDFSYS="LINUX"
  BRAND="linux64"
  if test "X$ENABLE_32_BIT" = "Xyes"; then
    BRAND="linux32"
    CFLAGS="$CFLAGS -m32"
    CXXFLAGS="$CXXFLAGS -m32"
    LDFLAGS="$LDFLAGS -m32"
    CPPFLAGS="$CPPFLAGS -m32"
  fi
  CPPFLAGS="$CPPFLAGS -O2 -Df2cFortran -DH5_USE_16_API"
fi

# macintel
if test "X$BRAND" = "XDarwin"; then
  HDFSYS="MACINTOSH"
  if test "X$ENABLE_32_BIT" = "Xyes"; then
    BRAND="macintel32"
    CFLAGS="$CFLAGS -m32"
    CXXFLAGS="$CXXFLAGS -m32"
    LDFLAGS="$LDFLAGS -m32"
    CPPFLAGS="$CPPFLAGS -m32"

    # extract libiconv library for 32-bit Mac
    rm -rf $srcdir/lib/$BRAND
    mkdir $srcdir/lib/$BRAND
    tar -zxvf $srcdir/lib/common/tmp/$BRAND/libiconv-1.13_MacIntel32.tar.Z
    mv $srcdir/libiconv/ $srcdir/lib/$BRAND/libiconv
  else
    BRAND="macintel64"
    # For macintel, you must specify 64-bits for Fortran code. Otherwise,
    # the compiler will complain about a type mismatch.
    FFLAGS="$FFLAGS -m64"
    # extract libiconv library for 64-bit Mac
    rm -rf $srcdir/lib/$BRAND
    mkdir $srcdir/lib/$BRAND
    tar -zxvf $srcdir/lib/common/tmp/$BRAND/libiconv-1.13_MacIntel64.tar.Z
    mv $srcdir/libiconv/ $srcdir/lib/$BRAND/libiconv
  fi
  CPPFLAGS="$CPPFLAGS -O2 -Df2cFortran -DH5_USE_16_API"
fi

# cygwin
if test "X$BRAND" = "XCYGWIN"; then
  HDFSYS="CYGWIN"
  BRAND="cygwin"
  CPPFLAGS="$CPPFLAGS -O2 -Df2cFortran -DH5_USE_16_API"

  # Fortran part of cygwin must be manually set to g77. This was done to follow
  # previous fortran setup. This also means that the fortran code of the 
  # toolkit library must be built with f77 fortran files.
  FC="g77"
  BUILD_WITH_F77="yes"

  # create the cygwin directory since it may not exist
  rm -rf $srcdir/bin/$BRAND
  mkdir $srcdir/bin/$BRAND
  rm -rf $srcdir/lib/$BRAND
  mkdir $srcdir/lib/$BRAND
  # extract libiconv library for Cygwin host
  tar -zxvf $srcdir/lib/common/tmp/cygwin/libiconv-1.13_CYGWIN.tar.Z
  mv $srcdir/libiconv/ $srcdir/lib/$BRAND/libiconv
fi

# dec
if test "X$BRAND" = "XOSF1"; then
  HDFSYS="DEC_ALPHA"
  BRAND="dec"
fi

# sco
if test "X$BRAND" = "Xsco386"; then
  HDFSYS="SCO"
  BRAND="sco"
fi

# SunOS
if test "X$BRAND" = "XSunOS"; then
  HDFSYS="SUN"
  CPPFLAGS="$CPPFLAGS -DH5_USE_16_API"
  AC_SUBST(CBRAND) CBRAND=`uname -r | awk -F. '{print $1, $2}'`
  sun_Version1=`uname -r | awk -F. '{print $1}'`
  sun_Version2=`uname -r | awk -F. '{print $2}'`
  BRAND="sun$sun_Version1.$sun_Version2"
  if test "X$CBRAND" = "X5 10"; then
    CFLAGS="$CFLAGS -DSUNOS510X"
  fi
  if test "X$CBRAND" = "X5 9"; then
    CFLAGS="$CFLAGS -DSUNOS59X"
  fi
  if test "X$CBRAND" = "X5 8"; then
    CFLAGS="$CFLAGS -DSUNOS58X"
  fi
  if test "X$CBRAND" = "X5 5"; then
    BRAND="sun5"
  fi
  if test "X$sun_Version1" = "X4"; then
    BRAND="sun4"
  fi
fi

# UNICOS
if test "X$BRAND" = "XUNICOS"; then
  HDFSYS="UNICOS"
  BRAND="cray"
fi

CFLAGS="$CFLAGS -D$HDFSYS"
# Debug flag feature
if test "X$ENABLE_DEBUG" = "Xyes"; then
  CPPFLAGS="$CPPFLAGS -g"
fi

AM_CONDITIONAL([BUILD_MAC_LIBICONV], [test "X$BRAND" = "Xmacintel32" -o "X$BRAND" = "Xmacintel64"])
AM_CONDITIONAL([BUILD_CYG_LIBICONV], [test "X$BRAND" = "Xcygwin"])

AC_MSG_RESULT(Compiling under $BRAND)


# Some built-in configure checks can only see CFLAGS (not AM_CFLAGS), so
# we need to add this in so configure works as intended. We will need to
# reset this value at the end of configure, to preserve the user's settings.
CFLAGS="${AM_CFLAGS} ${CFLAGS}"
FCFLAGS="${AM_FCFLAGS} ${FCFLAGS}"
CXXFLAGS="${AM_CXXFLAGS} ${CXXFLAGS}"
CPPFLAGS="${AM_CPPFLAGS} ${CPPFLAGS}"
LDFLAGS="${AM_LDFLAGS} ${LDFLAGS}"

# Enable dependency tracking unless the configure options or a
# site-specific file told us not to.  This prevents configure from
# silently disabling dependencies for some compilers.
#
if test -z "${enable_dependency_tracking}"; then
  enable_dependency_tracking="yes"
fi

# Check which archiving tool to use. This should be dont before libtool
# macros
if test -z "$AR"; then
  AC_CHECK_PROGS([AR], [ar xar], [:], [$PATH])
fi


# -------------------------------------------------------------------------
# Checks for programs
# -------------------------------------------------------------------------

# Basic programs
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
CC_BASENAME="`echo $CC | cut -f1 -d' ' | xargs basename 2>/dev/null`"
AC_PROG_RANLIB

# Fortran start------------------------------------------------------------
# Checks if the user would like Fortran interface compiled
HAVE_SIZEOF="no"
FORTRAN_DEFAULT_REALisDBLE="no"

AC_MSG_CHECKING([if Fortran interface enabled])
AC_ARG_ENABLE([fortran],
              [AC_HELP_STRING([--enable-fortran],
                              [Build Fortran into library [default=no]])],,
              [enableval="no"])
if test "X$enableval" = "Xyes"; then
  echo "yes"
else
  echo "no"
fi

if test "X$enableval" = "Xyes"; then

  # General Fortran flags
  AM_FCFLAGS="${AM_FCFLAGS} ${FFLAGS}"
  FCFLAGS="${FCFLAGS} ${FFLAGS}"

  # Check for a Fortran 9X compiler and how to include modules.
  AC_PROG_FC([f90 pgf90 slf90 f95 g95 xlf95 efc ifort ftn],)
  AC_F9X_MODS

  # Tell it that our F77 compiler is $FC (actually a F9X compiler)
  F77=$FC

  # Change to the Fortran 90 language
  AC_LANG_PUSH(Fortran)
  AC_FC_LIBRARY_LDFLAGS

  # Define wrappers for the C compiler to use Fortran function names
  AC_FC_WRAPPERS

  # See if the fortran compiler supports the intrinsic function "SIZEOF"
  AC_MSG_CHECKING([if Fortran compiler supports intrinsic SIZEOF])
  AC_TRY_RUN([ 
   PROGRAM main
     i = sizeof(x)
   END PROGRAM
  ], [AC_MSG_RESULT(yes)
     	HAVE_SIZEOF="yes"],
     AC_MSG_RESULT(no))

  # Check to see if -r8 was specified to determine if we need to
  # compile the DOUBLE PRECISION interfaces.
  AC_MSG_CHECKING([if Fortran default REAL is DOUBLE PRECISION])
  
  AC_TRY_RUN([
     MODULE type_mod
       INTERFACE mtd    
         MODULE PROCEDURE mtd_real
         MODULE PROCEDURE mtd_dble
       END INTERFACE
     CONTAINS
       SUBROUTINE mtd_real(r)
         REAL :: r
       END SUBROUTINE mtd_real
       SUBROUTINE mtd_dble(d)
         DOUBLE PRECISION :: d
       END SUBROUTINE mtd_dble
     END MODULE type_mod
     PROGRAM main
       USE type_mod
       REAL :: r
       DOUBLE PRECISION :: d
       CALL mtd(r)
       CALL mtd(d)
     END PROGRAM main
                  ],
    AC_MSG_RESULT(no), 
    [AC_MSG_RESULT(yes)
        FORTRAN_DEFAULT_REALisDBLE="yes"])

  # Change back to the C language
  AC_LANG_POP(Fortran)

else
  FC="no"
fi


AC_MSG_CHECKING([what FC is set to])
AC_MSG_RESULT([$FC])

# Return some fortran variables here
AM_CONDITIONAL([BUILD_FORTRAN], [test "X$enableval" = "Xyes"])
AM_CONDITIONAL([FORTRAN_LANG_F77], [test "X$BUILD_WITH_F77" = "Xyes"])
AM_CONDITIONAL([FORTRAN_HAVE_SIZEOF], [test "X$HAVE_SIZEOF" = "Xyes"])
AM_CONDITIONAL([FORTRAN_DEFAULT_REALisDBLE_F], [test "X$FORTRAN_DEFAULT_REALisDBLE" = "Xyes"])

# fortran end---------------------------------------------------------------

# Check if they have Perl installed on their system. We only need Perl
# if they're using a GNU compiler.
#
AC_SUBST([PERL]) PERL=""
if test "X$GCC" = "Xyes"; then
  AC_CHECK_PROGS([PERL], [perl],, [$PATH])
fi

# Check which archiving tool to use. This needs to be done before
# the AM_PROG_LIBTOOL macro.
#
if test -z "$AR"; then
  AC_CHECK_PROGS([AR], [ar xar], [:], [$PATH])
fi
AC_SUBST([AR])

# Export the AR macro so that it will be placed in the libtool file
# correctly.
export AR

# Libtool
# Windows won't create DLLs without the following macro.
AC_LIBTOOL_WIN32_DLL

# Create libtool. If shared/static libraries are going to be enabled or 
# disabled, it should happen before these macros.
AC_DISABLE_SHARED
AC_PROG_LIBTOOL

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Check that the tr utility is working properly.
AC_PATH_PROG([TR], [tr])

TR_TEST=`echo Test | ${TR} ${as_cr_letters}"," ${as_cr_LETTERS}" "`
if test "X${TR_TEST}" != "XTEST"; then
  AC_MSG_ERROR([tr program doesn't work])
fi

# Check that time can be used with srcdir. This is okay on most systems, but
# seems to cause problems on Cygwin.
AC_MSG_CHECKING([if srcdir= and time commands work together])

AC_SUBST([TIME])
TIME=time
TIME_TEST=`foo="bar" ${TIME} echo 'baz' 2> /dev/null | grep baz`

if test "X${TIME_TEST}" = "Xbaz"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
  TIME=
fi


# -----------------------------------------------------------------------
# Checks for libraries
# -------------------------------------------------------------------------

# ----------------------------------------------------------------------
# Fake --with-xxx option to allow us to create a help message for the
# following --with-xxx options which can take either a =DIR or =INC,LIB
# specifier.
#
AC_ARG_WITH([fnord],
  [
 For the following --with-xxx options, you can specify where the header
 files and libraries are in two different ways:

    --with-xxx=INC,LIB - Specify individually the include directory and
                         library directory separated by a comma
    --with-xxx=DIR     - Specify only the directory which contains the
                         include/ and lib/ subdirectories
  ])

# ----------------------------------------------------------------------
# Is the GNU zlib present? It has a header file `zlib.h' and a library
# `-lz' and their locations might be specified with the `--with-zlib'
# command-line switch. The value is an include path and/or a library path.
# If the library path is specified then it must be preceded by a comma.
AC_ARG_WITH([zlib],
            [AC_HELP_STRING([--with-zlib=DIR],
                            [Use zlib library [default=yes]])],,
            [withval=yes])

case "$withval" in
  yes)
    HAVE_ZLIB="yes"
    AC_CHECK_HEADERS([zlib.h],[HAVE_ZLIB_H = "yes"], [unset HAVE_ZLIB])
    if test "x$HAVE_ZLIB" = "xyes" -a "x$HAVE_ZLIB_H" = "xyes"; then
      AC_CHECK_LIB([z], [compress2],, [unset HAVE_ZLIB])
    fi

    if test -z "$HAVE_ZLIB"; then
      AC_MSG_ERROR([couldn't find zlib library])
    else
      AC_CHECK_FUNC([compress2], [HAVE_COMPRESS2="yes"])
    fi
    ;;
  no)
    HAVE_ZLIB="no"
    AC_MSG_ERROR([zlib library required to build SDP Toolkit])
    ;;
  *)
    HAVE_ZLIB="yes"
    case "$withval" in
      *,*)
        zlib_inc="`echo $withval | cut -f1 -d,`"
        zlib_lib="`echo $withval | cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          zlib_inc="$withval/include"
          zlib_lib="$withval/lib"
        fi
        ;;
    esac

    # Trying to include -I/usr/include and -L/usr/lib is redundant and
    # can mess some compilers up.
    if test "X$zlib_inc" = "X/usr/include"; then
      zlib_inc=""
    fi
    if test "X$zlib_lib" = "X/usr/lib"; then
      zlib_lib=""
    fi

    if test -n "$zlib_inc"; then
      CPPFLAGS="$CPPFLAGS -I$zlib_inc"
    fi

    # Checks again
    AC_CHECK_HEADERS([zlib.h],[HAVE_ZLIB_H="yes"], [unset HAVE_ZLIB])

    if test -n "$zlib_lib"; then
      LDFLAGS="$LDFLAGS -L$zlib_lib"
    fi

    if test "x$HAVE_ZLIB" = "xyes" -a "x$HAVE_ZLIB_H" = "xyes"; then
      AC_CHECK_LIB([z], [compress2],, [unset HAVE_ZLIB])
    fi

    if test -z "$HAVE_ZLIB"; then
      AC_MSG_ERROR([couldn't find zlib library])
    else
      AC_CHECK_FUNC([compress2], [HAVE_COMPRESS2="yes"])
    fi
    ;;
esac

# ----------------------------------------------------------------------
# Is the JPEG library present?
AC_ARG_WITH([jpeg],
            [AC_HELP_STRING([--with-jpeg=DIR],
                            [Use jpeg library [default=yes]])],,
            [withval=yes])

case "$withval" in
  yes)
    HAVE_JPEG="yes"
    AC_CHECK_HEADERS([jpeglib.h],[HAVE_JPEG_H = "yes"], [unset HAVE_JPEG])
    if test "x$HAVE_JPEG" = "xyes" -a "x$HAVE_JPEG_H" = "xyes"; then
      AC_CHECK_LIB([jpeg], [jpeg_start_decompress],, [unset HAVE_JPEG])
    fi

    if test -z "$HAVE_JPEG"; then
      AC_MSG_ERROR([couldn't find jpeg library])
    fi
    ;;
  no)
    HAVE_JPEG="no"
    AC_MSG_ERROR([jpeg library required to build SDP Toolkit])
    ;;
  *)
    HAVE_JPEG="yes"
    case "$withval" in
      *,*)
        jpeg_inc="`echo $withval | cut -f1 -d,`"
        jpeg_lib="`echo $withval | cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          jpeg_inc="$withval/include"
          jpeg_lib="$withval/lib"
        fi
        ;;
    esac

    # Trying to include -I/usr/include and -L/usr/lib is redundant and
    # can mess some compilers up.
    if test "X$jpeg_inc" = "X/usr/include"; then
      jpeg_inc=""
    fi
    if test "X$jpeg_lib" = "X/usr/lib"; then
      jpeg_lib=""
    fi

    if test -n "$jpeg_inc"; then
      CPPFLAGS="$CPPFLAGS -I$jpeg_inc"
    fi

    AC_CHECK_HEADERS([jpeglib.h],[HAVE_JPEG_H="yes"], [unset HAVE_JPEG])

    if test -n "$jpeg_lib"; then
      LDFLAGS="$LDFLAGS -L$jpeg_lib"
    fi

    if test "x$HAVE_JPEG" = "xyes" -a "x$HAVE_JPEG_H" = "xyes"; then
      AC_CHECK_LIB([jpeg], [jpeg_start_decompress],, [unset HAVE_JPEG])
    fi

    if test -z "$HAVE_JPEG"; then
      AC_MSG_ERROR([couldn't find jpeg library])
    fi
    ;;
esac

# ----------------------------------------------------------------------
# Is the szip library present?
AC_SUBST(USE_COMP_SZIP) USE_COMP_SZIP="no"
AC_ARG_WITH([szlib],
            [AC_HELP_STRING([--with-szlib=DIR],
                            [Use szlib library [default=no]])],,
            [withval=no])

case "$withval" in
  yes)
    HAVE_SZIP="yes"
    AC_CHECK_HEADERS([szlib.h],[HAVE_SZLIB_H="yes"], [unset HAVE_SZIP])
    if test "x$HAVE_SZLIB" = "xyes" -a "x$HAVE_SZLIB_H" = "xyes"; then
      AC_CHECK_LIB([szip], [SZ_BufftoBuffCompress],, [unset HAVE_SZLIB])
    fi

    if test -z "$HAVE_SZIP"; then
      AC_MSG_ERROR([couldn't find szlib library])
    fi
    ;;
  no)
    HAVE_SLIB="no"
    AC_MSG_CHECKING([for szlib])
    AC_MSG_RESULT([suppressed])
    SZIP_INFO="${SZIP_INFO}disabled"
    ;;
  *)
    HAVE_SZIP="yes"
    case "$withval" in
      *,*)
        szip_inc="`echo $withval | cut -f1 -d,`"
        szip_lib="`echo $withval | cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          szip_inc="$withval/include"
          szip_lib="$withval/lib"
        fi
        ;;
    esac

    # Trying to include -I/usr/include and -L/usr/lib is redundant and
    # can mess some compilers up.
    if test "X$szip_inc" = "X/usr/include"; then
      szip_inc=""
    fi
    if test "X$szip_lib" = "X/usr/lib"; then
      szip_lib=""
    fi

    if test -n "$szip_inc"; then
      CPPFLAGS="$CPPFLAGS -I$szip_inc"
    fi

    AC_CHECK_HEADERS([szlib.h],[HAVE_SZLIB_H="yes"], [unset HAVE_SZIP])

    if test -n "$szip_lib"; then
      LDFLAGS="$LDFLAGS -L$szip_lib"
    fi

    if test "x$HAVE_SZLIB" = "xyes" -a "x$HAVE_SZLIB_H" = "xyes"; then
      AC_CHECK_LIB([szip], [SZ_BufftoBuffCompress],, [unset HAVE_SZLIB])
    fi

    if test -z "$HAVE_SZIP"; then
      AC_MSG_ERROR([couldn't find szlib library])
    else
      USE_COMP_SZIP="yes"
    fi
    ;;
esac

# Check to see if SZIP has encoder 
if test "X$HAVE_SZIP" = "Xyes"; then
    # SZLIB library is available. Check if it can encode.
    AC_MSG_CHECKING([for szlib encoder])
    
    AC_TRY_RUN([
    #include <stdlib.h>
    #include <szlib.h>

    int main(void)
    {
        /* SZ_encoder_enabled returns 1 if encoder is present */
        if (SZ_encoder_enabled() == 1)
            exit(0);
        else
            exit(1);
    }
    ], [CAN_ENCODE="yes"], [CAN_ENCODE="no"],)

    # Report szip encoder test results
    if test "X$CAN_ENCODE" = "Xyes"; then
        AC_MSG_RESULT([yes])
    fi
    if test "X$CAN_ENCODE" = "Xno"; then
        AC_MSG_RESULT([no])
    fi

    # Add "szip" to external filter list
    if test "X$CAN_ENCODE" = "Xyes"; then
        if test "X$SZIP_INFO" != "X"; then
            SZIP_INFO="${SZIP_INFO}"
        fi
        SZIP_INFO="${SZIP_INFO}enabled with encoder"
    fi

    if test "X$CAN_ENCODE" = "Xno"; then
        if test "X$SZIP_INFO" != "X"; then
            SZIP_INFO="${SZIP_INFO}"
        fi
        SZIP_INFO="${SZIP_INFO}enabled with decoder only"
    fi

    # Create macro to specify if encoder is present
    if test "X$CAN_ENCODE" = "Xyes"; then
        AC_DEFINE([HAVE_SZIP_ENCODER], [1],
                [Define if szip has encoder])
        SZIP_HAS_ENCODER="yes"
    fi
fi
AC_SUBST([SZIP_INFO])

# --------------------------------------------------------------------------
# Check for system libraries
# --------------------------------------------------------------------------

AC_CHECK_LIB([m], [ceil])
# "dl" stands for dynamically loaded library
AC_CHECK_LIB([dl], [dlopen])

if test "`uname`" = "SunOS" -o "`uname -sr`" = "HP-UX B.11.00"; then
  # ...for Solaris
  AC_CHECK_LIB([socket], [socket])
  AC_CHECK_LIB([nsl], [xdr_int])
fi

# ----------------------------------------------------------------------
# Where's HDF4? We need to find out where the HDF4 library
# and header files are if they aren't in system library/include places.

AC_SUBST(LIBHDF4) LIBHDF4=""
AC_SUBST(HDF4PATH) HDF4PATH=""
AC_ARG_WITH([hdf4],
            [AC_HELP_STRING([--with-hdf4=DIR],
                            [Where to find the HDF4 library])],,)

case "$withval" in
  yes)
    AC_CHECK_HEADERS(hdf.h,,
                     AC_MSG_ERROR(cannot find HDF4 header files.))
    ;;
  no)
    AC_MSG_ERROR(HDF4 library needed for compilation.)
    ;;
  *)
    case "$withval" in
      *,*)
        hdf4_inc="`echo $withval | cut -f1 -d,`"
        hdf4_lib="`echo $withval | cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          hdf4_inc="$withval/include"
          hdf4_lib="$withval/lib"
        fi
        ;;
    esac

    if test -n "$hdf4_inc"; then
      CPPFLAGS="$CPPFLAGS -I$hdf4_inc"
    fi

    AC_CHECK_HEADERS(hdf.h,,
                     AC_MSG_ERROR(cannot find HDF4 header files!))

    HDF4PATH="$hdf4_lib,"
    HDF4PATH="`echo $HDF4PATH | sed -e "s/lib,/bin/"`"

    if test -n "$hdf4_lib"; then
      LD_LIBRARY_PATH="$hdf4_lib:$LD_LIBRARY_PATH"
      LDFLAGS="$LDFLAGS -L$hdf4_lib"

      # We want to use the ``.la'' version of the HDF4 library which
      # is the thing libtool creates and has in it all of the neat
      # stuff like where to find the libraries libdf depends upon.
      if test -f "$hdf4_lib/libhdf.la"; then
        libdir_saved=$libdir
        . $hdf_lib/libhdf.la
        libdir=$libdir_saved
        for lib in X $dependency_libs; do
          case "$lib" in
            *libdf.la*)
              ;;
            *)
              if test "$lib" != "X"; then
                LIBHDF4="$LIBHDF4 $lib"
              fi
              ;;
          esac
        done
      fi
    fi

    # LDFLAGS="$LDFLAGS $LIBHDF4"
    # LIBHDF4="$LIBHDF4" 
    AC_CHECK_LIB(df, main,,
                 AC_MSG_ERROR(cannot find HDF4 library.))
    AC_CHECK_LIB(mfhdf, main,,
                 AC_MSG_ERROR(cannot find HDF4 library.))
    ;;
esac

# ----------------------------------------------------------------------
# Test for szip in hdf4

AC_SUBST(H4_USE_FILTER_SZIP) H4_USE_FILTER_SZIP="no"

AC_MSG_CHECKING([for szlib compression in hdf4 library])

AC_CACHE_VAL([h4h5_cv_hdf4_szlib],
    [AC_TRY_RUN([
    #include <hdf.h>

    int main(void)
    {
        #ifdef H4_HAVE_LIBSZ
           exit(0);
        #else
           exit(1);
        #endif
    }
    ], [h4h5_cv_hdf4_szlib=yes], [h4h5_cv_hdf4_szlib=no],)])

if test ${h4h5_cv_hdf4_szlib} = "yes"; then
    AC_MSG_RESULT([yes])
    H4_USE_FILTER_SZIP="yes"
else
    AC_MSG_RESULT([no])
fi

# Find HDF5-----------------------------------------------------------------
# Where's HDF5? We need to find out where the HDF5 library
# and header files are if they aren't in system library/include places.

AC_SUBST(LIBHDF5) LIBHDF5=""
AC_SUBST(HDF5PATH) HDF5PATH=""
AC_ARG_WITH([hdf5],
            [AC_HELP_STRING([--with-hdf5=DIR],
                            [Where to find the HDF5 library])],,)

case "$withval" in
  yes)
    AC_CHECK_HEADERS(hdf5.h,,
                     AC_MSG_ERROR(cannot find HDF5 header files.))
    ;;
  no)
    AC_MSG_ERROR(HDF5 library needed for compilation.)
    ;;
  *)
    case "$withval" in
      *,*)
        hdf5_inc="`echo $withval | cut -f1 -d,`"
        hdf5_lib="`echo $withval | cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          hdf5_inc="$withval/include"
          hdf5_lib="$withval/lib"
        fi
        ;;
    esac

    if test -n "$hdf5_inc"; then
      CPPFLAGS="$CPPFLAGS -I$hdf5_inc"
    fi

    AC_CHECK_HEADERS(hdf5.h,,
                     AC_MSG_ERROR(cannot find HDF5 header files!))

    HDF5PATH="$hdf5_lib,"
    HDF5PATH="`echo $HDF5PATH | sed -e "s/lib,/bin/"`"

    if test -n "$hdf5_lib"; then
      LD_LIBRARY_PATH="$hdf5_lib:$LD_LIBRARY_PATH"
      LDFLAGS="$LDFLAGS -L$hdf5_lib"

      # We want to use the ``.la'' version of the HDF5 library which
      # is the thing libtool creates and has in it all of the neat
      # stuff like where to find the libraries libhdf5 depends upon.
      if test -f "$hdf5_lib/libhdf5.la"; then
        libdir_saved=$libdir
        . $hdf5_lib/libhdf5.la
        libdir=$libdir_saved
        for lib in X $dependency_libs; do
          case "$lib" in
            *libhdf5.la*)
              ;;
            *)
              if test "$lib" != "X"; then
                LIBHDF5="$LIBHDF5 $lib"
              fi
              ;;
          esac
        done
      fi
    fi

    # LDFLAGS="$LDFLAGS $LIBHDF5"
    # LIBHDF5="$LIBHDF5" 
    AC_CHECK_LIB(hdf5, H5open,,
                 AC_MSG_ERROR(cannot find HDF5 library.))
    ;;
esac

# ---------------------------------------------------------------------
# What filters do we have?
# Depending on the compression filters that HDF5 and HDF4 are using,
# certain tests may not succeed (i.e., the ones that test converting
# compressed objects).

AC_SUBST(H5_USE_FILTER_ZLIB) H5_USE_FILTER_ZLIB="no"

AC_MSG_CHECKING([for deflate filter in hdf5 library])

AC_CACHE_VAL([h4h5_cv_hdf5_has_zlib],
   [AC_TRY_RUN([
   #include <hdf5.h>

   int main(void)
   {
         /* H5Zfilter_avail() returns 1 if ZLIB is available */
      if(H5Zfilter_avail(H5Z_FILTER_DEFLATE) == 1)
         exit(0);
      else
         exit(1);
   }
   ], [h4h5_cv_hdf5_has_zlib=yes], [h4h5_cv_hdf5_has_zlib=no],)])
       
if test ${h4h5_cv_hdf5_has_zlib} = "yes"; then
   AC_MSG_RESULT([yes])
   H5_USE_FILTER_ZLIB="yes"
else
   AC_MSG_RESULT([no])
fi

# ----------------------------------------------------------------------
# Test for szip and for szip encoder in hdf5

AC_SUBST(H5_USE_FILTER_SZIP) H5_USE_FILTER_SZIP="no"
AC_SUBST(H5_USE_SZIP_ENCODER) H5_USE_SZIP_ENCODER="no"

AC_MSG_CHECKING([for szlib filter in hdf5 library])

AC_CACHE_VAL([h4h5_cv_hdf5_has_szlib],
   [AC_TRY_RUN([
   #include <hdf5.h>

   int main(void)
   {
         /* H5Zfilter_avail() returns 1 if SZIP is available */
      if(H5Zfilter_avail(H5Z_FILTER_SZIP) == 1)
         exit(0);
      else
         exit(1);
   }
   ], [h4h5_cv_hdf5_has_szlib=yes], [h4h5_cv_hdf5_has_szlib=no],)])

if test ${h4h5_cv_hdf5_has_szlib} = "yes"; then
   AC_MSG_RESULT([yes])
   H5_USE_FILTER_SZIP="yes"
else
   AC_MSG_RESULT([no])
fi

if test ${h4h5_cv_hdf5_has_szlib} = "yes"; then
      # Check if szip has encoding enabled
   AC_MSG_CHECKING([for szlib encoder in hdf5 library])

   AC_CACHE_VAL([h4h5_cv_hdf5_szlib_encode],
       [AC_TRY_RUN([
       #include <hdf5.h>

       int main(void)
       {
           unsigned int config_flags;

           if(H5Zget_filter_info(H5Z_FILTER_SZIP, &config_flags) == 0)
              if(config_flags & H5Z_FILTER_CONFIG_ENCODE_ENABLED)
                 exit(0);
           exit(1);
       }
       ], [h4h5_cv_hdf5_szlib_encode=yes], [h4h5_cv_hdf5_szlib_encode=no],)])

   if test ${h4h5_cv_hdf5_szlib_encode} = "yes"; then
       AC_MSG_RESULT([yes])
       H5_USE_SZIP_ENCODER="yes"
   else
       AC_MSG_RESULT([no])
   fi
fi

# ----------------------------------------------------------------------
# Where's HDFEOS2? We need to find out where the HDFEOS2 library
# and header files are if they aren't in system library/include places.
#
AC_SUBST(LIBHDFEOS2) LIBHDFEOS2=""
AC_SUBST(HDFEOS2PATH) HDFEOS2PATH=""
AC_SUBST(H5_USE_HDFEOS2) H5_USE_HDFEOS2="no"
AC_ARG_WITH([hdfeos2],
            [AC_HELP_STRING([--with-hdfeos2=DIR],
                            [Where to find the HDFEOS2 library])],,)

case "$withval" in
  yes)
    AC_CHECK_HEADERS(ease.h,,
                     AC_MSG_ERROR(cannot find HDFEOS2 header files.))
    ;;
  no)
    AC_MSG_ERROR(HDFEOS2 library needed for compilation.)
    ;;
  *)
    case "$withval" in
      *,*)
        hdfeos2_inc="`echo $withval | cut -f1 -d,`"
        hdfeos2_lib="`echo $withval | cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          hdfeos2_inc="$withval/include"
          hdfeos2_lib="$withval/lib"
        fi
        ;;
    esac

    if test -n "$hdfeos2_inc"; then
      CPPFLAGS="$CPPFLAGS -I$hdfeos2_inc"
    fi

    AC_CHECK_HEADERS(ease.h,,
                     AC_MSG_ERROR(cannot find HDFEOS2 header files.))

    HDFEOS2PATH="$hdfeos2_lib,"
    HDFEOS2PATH="`echo $HDFEOS2PATH | sed -e "s/lib,/bin/"`"

    if test -n "$hdfeos2_lib"; then
      LD_LIBRARY_PATH="$hdfeos2_lib:$LD_LIBRARY_PATH"
      LDFLAGS="$LDFLAGS -L$hdfeos2_lib"
    fi

    AC_CHECK_HEADERS(ease.h,,
                     AC_MSG_ERROR(cannot find HDFEOS2 header files.))

    H5_USE_HDFEOS2="yes"
    ;;
esac

# ----------------------------------------------------------------------
# Where's HDFEOS5? We need to find out where the HDFEOS5 library
# and header files are if they aren't in system library/include places.

AC_SUBST(LIBHDFEOS5) LIBHDFEOS5=""
AC_SUBST(HDFEOS5PATH) HDFEOS5PATH=""
AC_SUBST(H5_USE_HDFEOS5) H5_USE_HDFEOS5="no"
AC_ARG_WITH([hdfeos5],
            [AC_HELP_STRING([--with-hdfeos5=DIR],
                            [Where to find the HDFEOS5 library])],,)

case "$withval" in
  yes)
    AC_CHECK_HEADERS(HE5_GctpFunc.h,,
                     AC_MSG_ERROR(cannot find HDFEOS5 header files.))
    ;;
  no)
    AC_MSG_ERROR(HDFEOS5 library needed for compilation.)
    ;;
  *)
    case "$withval" in
      *,*)
        hdfeos5_inc="`echo $withval | cut -f1 -d,`"
        hdfeos5_lib="`echo $withval | cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          hdfeos5_inc="$withval/include"
          hdfeos5_lib="$withval/lib"
        fi
        ;;
    esac

    if test -n "$hdfeos5_inc"; then
      CPPFLAGS="$CPPFLAGS -I$hdfeos5_inc"
    fi

    AC_CHECK_HEADERS(HE5_GctpFunc.h,,
                     AC_MSG_ERROR(cannot find HDFEOS5 header files.))

    HDFEOS5PATH="$hdfeos5_lib,"
    HDFEOS5PATH="`echo $HDFEOS5PATH | sed -e "s/lib,/bin/"`"

    if test -n "$hdfeos5_lib"; then
      LD_LIBRARY_PATH="$hdfeos5_lib:$LD_LIBRARY_PATH"
      LDFLAGS="$LDFLAGS -L$hdfeos5_lib"
    fi

    AC_CHECK_HEADERS(HE5_GctpFunc.h,,
                     AC_MSG_ERROR(cannot find HDFEOS5 header files.))

    H5_USE_HDFEOS5="yes"
    ;;
esac

# --------------------------------------------------------------------------
# Checks for header files.
# --------------------------------------------------------------------------

AC_PATH_X
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([OS.h arpa/inet.h fcntl.h float.h inttypes.h limits.h locale.h malloc.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/param.h sys/socket.h sys/time.h sys/timeb.h unistd.h values.h wchar.h wctype.h])

# --------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# --------------------------------------------------------------------------

AC_HEADER_STAT
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

AC_CHECK_TYPES([ptrdiff_t])

# --------------------------------------------------------------------------
# Checks for library functions.
# --------------------------------------------------------------------------

AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRCOLL
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor ftime ftruncate getcwd gethostbyname gettimeofday isascii localeconv localtime_r mblen memchr memmove memset mkdir modf munmap pow  putenv regcomp select socket sqrt strcasecmp strchr strcspn strdup strerror strncasecmp strpbrk strrchr strspn strstr strtol strtoul])

# Moves old makefiles to prevent them from being run during make. These
# makefiles are used by the INSTALL scripts in the bin directory. You can
# restore these files by simply renaming them back to their original name
# in the source directory. If these files do not exist or have already
# been moved, this can be ignored.
mv Makefile Makefile_orig
mv src/AA/makefile src/AA/makfile_org
mv src/AA/DCW/makefile src/AA/DCW/makefile_orig
mv src/AA/VPF/makefile src/AA/VPF/makefile_orig
mv src/AA/freeform/makefile src/AA/freeform/makefile_orig
mv src/AA/generic/makefile src/AA/generic/makefile_orig
mv src/CBP/makefile src/CBP/makefile_orig
mv src/CSC/makefile src/CSC/makefile_orig
mv src/CUC/makefile src/CUC/makefile_orig
mv src/CUC/ODL/makefile src/CUC/ODL/makefile_orig
mv src/CUC/UDUNITS/makefile src/CUC/UDUNITS/makefile_orig
mv src/DEM/makefile src/DEM/makefile_orig
mv src/EPH/makefile src/EPH/makefile_orig
mv src/GCT/makefile src/GCT/makefile_orig
mv src/GCT/freeware/makefile src/GCT/freeware/makefile_orig
mv src/IO/GEN/makefile src/IO/GEN/makefile_orig
mv src/IO/L0/L0sim/makefile src/IO/L0/L0sim/makefile_orig
mv src/IO/L0/makefile src/IO/L0/makefile_orig
mv src/IO/makefile src/IO/makefile_orig
mv src/MEM/makefile src/MEM/makefile_orig
mv src/MET/makefile src/MET/makefile_orig
mv src/MET/support/makefile src/MET/support/makefile_orig
mv src/MET/tools/makefile src/MET/tools/makefile_orig
mv src/makefile src/makefile_orig
mv src/PC/makefile src/PC/makefile_orig
mv src/SMF/makefile src/SMF/makefile_orig
mv src/TD/makefile src/TD/makefile_orig
mv src/TSF/makefile src/TSF/makefile_orig
mv src/XML/makefile src/XML/makefile_orig
mv src/XML/xml/makefile src/XML/xml/makefile_orig
mv src/XML/xsl/makefile src/XML/xsl/makefile_orig
mv message/makefile message/makefile_orig

# copy linklist header file for VPF
cp include/DCW/linklist.h src/AA/VPF

AC_CONFIG_FILES([Makefile
		 message/Makefile
                 src/AA/Makefile
		 src/AA/DCW/Makefile
                 src/AA/VPF/Makefile
                 src/AA/freeform/Makefile
                 src/AA/generic/Makefile
                 src/CBP/Makefile
                 src/CSC/Makefile
                 src/CUC/Makefile
                 src/CUC/ODL/Makefile
                 src/CUC/UDUNITS/Makefile
                 src/DEM/Makefile
                 src/EPH/Makefile
                 src/GCT/Makefile
                 src/GCT/freeware/Makefile
                 src/IO/GEN/Makefile
                 src/IO/L0/L0sim/Makefile
                 src/IO/L0/Makefile
                 src/IO/Makefile
                 src/MEM/Makefile
                 src/MET/Makefile
                 src/MET/support/Makefile
                 src/MET/tools/Makefile
                 src/Makefile
                 src/PC/Makefile
                 src/SMF/Makefile
                 src/TD/Makefile
                 src/TSF/Makefile
                 src/XML/Makefile
                 src/XML/xml/Makefile
                 src/XML/xsl/Makefile
		 utils/Makefile
		 utils/CBP/Makefile
		 utils/CSC/Makefile
		 utils/EPH/Makefile
		 utils/EPH/orbsim/Makefile
		 utils/EPH/gbadsim/Makefile
		 utils/PC/Makefile
		 utils/SMF/Makefile
		 utils/TD/Makefile])

AC_OUTPUT
