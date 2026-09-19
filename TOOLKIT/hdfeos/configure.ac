#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Prototype configure.ac for HDF-EOS2
# Created 7/18/07 by Mike McGreevy (mcgreevy@hdfgroup.org) 

AC_PREREQ([2.61])
AC_INIT([hdf-eos2], [4.2], [null@bogus.email])
# AC_CONFIG_SRCDIR([src/EHapi.c])
AC_CONFIG_HEADER([include/HE2_config.h])

AC_CONFIG_AUX_DIR([config])

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

# Disable shared libraries (for now)
AC_DISABLE_SHARED

AM_PROG_LIBTOOL


dnl --------------------------------------------------
dnl Set prefix default (install directory) to a directory
dnl in the build area.
AC_PREFIX_DEFAULT([`pwd`/hdfeos2])

# Check for the math library that HDF4 needs
AC_CHECK_LIB([m], [ceil])

# Check whether jpeg is enabled.
# Although HDF-EOS2 doesn't use jpeg, HDF4 needs it.
AC_ARG_WITH([jpeg],
	    [AC_HELP_STRING([--with-jpeg=DIR],
			    [Specify path to external jpeg library.
			     Linker must be able to find jpeg if HDF4 was built with jpeg.
			     [default=yes]])],,
	    withval=yes)
case "$withval" in
  yes)
    HAVE_JPEG="yes"
    AC_CHECK_LIB([jpeg],[jpeg_set_defaults],, [unset HAVE_JPEG])
    if test -z "$HAVE_JPEG"; then
      AC_MSG_ERROR([couldn't find jpeg library])
    fi
    ;;
  no)
    HAVE_JPEG="no"
    AC_MSG_CHECKING([for jpeg library])
    AC_MSG_RESULT([suppressed])
    ;;
  *)
    HAVE_JPEG="yes"
    case "$withval" in
      *,*)
	jpeg_lib="`echo $withval |cut -f2 -d, -s`"
	;;
      *)
	if test -n "$withval"; then
	  jpeg_lib="$withval/lib"
	fi
	;;
    esac

    dnl Trying to include -L/usr/lib is redundant and
    dnl can mess some compilers up.
    if test "X$jpeg_lib" = "X/usr/lib"; then
      jpeg_lib=""
    fi

    saved_LDFLAGS="$LDFLAGS"

    if test -n "$jpeg_lib"; then
      LDFLAGS="$LDFLAGS -L$jpeg_lib"
    fi

    AC_CHECK_LIB([jpeg],[jpeg_set_defaults],,
		 [LDFLAGS="$saved_LDFLAGS"; unset HAVE_JPEG])
    if test -z "$HAVE_JPEG"; then
      AC_MSG_ERROR([couldn't find jpeg library])
    fi
    ;;
esac

# Check whether zlib is enabled.
# HDF-EOS2 doesn't use zlib, but HDF4 might. If HDF4 does, it needs to be given
# the linker of there will be undefined symbols in HDF4.
AC_ARG_WITH([zlib],
            [AC_HELP_STRING([--with-zlib=DIR],
                            [Specify path to external zlib library.
                             Linker must be able to find zlib if HDF4
                             was built with zlib.
                             [default=yes]])],,
            withval=yes)

case $withval in
  yes)
    HAVE_ZLIB="yes"
    AC_CHECK_HEADERS([zlib.h],)
    AC_CHECK_LIB([z], [compress2],, [unset HAVE_ZLIB])

    if test -z "$HAVE_ZLIB"; then
      AC_MSG_ERROR([couldn't find zlib library])
    fi
    ;;
  no)
    HAVE_ZLIB="no"
    AC_MSG_CHECKING([for GNU zlib])
    AC_MSG_RESULT([suppressed])
    ;;
  *)
    HAVE_ZLIB="yes"
    case "$withval" in
      *,*)
        zlib_inc="`echo $withval |cut -f1 -d,`"
        zlib_lib="`echo $withval |cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          zlib_inc="$withval/include"
          zlib_lib="$withval/lib"
        fi
        ;;
    esac

    dnl Trying to include -I/usr/include and -L/usr/lib is redundant and
    dnl can mess some compilers up.
    if test "X$zlib_inc" = "X/usr/include"; then
      zlib_inc=""
    fi
    if test "X$zlib_lib" = "X/usr/lib"; then
      zlib_lib=""
    fi

    saved_CPPFLAGS="$CPPFLAGS"
    saved_LDFLAGS="$LDFLAGS"

    if test -n "$zlib_inc"; then
      CPPFLAGS="$CPPFLAGS -I$zlib_inc"
    fi

    AC_CHECK_HEADERS([zlib.h], ,
                     [CPPFLAGS="$saved_CPPFLAGS"])

    if test -n "$zlib_lib"; then
      LDFLAGS="$LDFLAGS -L$zlib_lib"
    fi

    AC_CHECK_LIB([z], [compress2],,
                 [LDFLAGS="$saved_LDFLAGS"; unset HAVE_ZLIB])

    if test -z "$HAVE_ZLIB"; then
      AC_MSG_ERROR([couldn't find zlib library])
    fi
    ;;
esac

dnl --------------------------------------------------
dnl Check if -Df2cFortran is specified
dnl Running only the preprocessor may not be a good idea
dnl because this can be hard-coded.
AC_MSG_CHECKING([for fc2Fortran macro])
AC_CACHE_VAL([he2_cv_f2cFortran_defined],
    [AC_TRY_RUN([
    int main(void)
    {
    #ifdef f2cFortran
      return 0;
    #else
      return 1;
    #endif
    }
    ], [he2_cv_f2cFortran_defined=yes], [he2_cv_f2cFortran_defined=no],)])
if test ${he2_cv_f2cFortran_defined} = "yes"; then
  F2CFORTRAN_MACRO="yes"
  AC_MSG_RESULT([defined])
else
  F2CFORTRAN_MACRO="no"
  AC_MSG_RESULT([not defined])
fi

if test ${he2_cv_f2cFortran_defined} = "yes"; then

  AC_CHECK_SIZEOF([int*])
  AC_CACHE_VAL([he2_cv_32ptr],
      [AC_TRY_RUN([
      int main(void)
      {
#ifdef SIZEOF_INTP
	return SIZEOF_INTP == 4 ? 0 : 1;
#else
#error SIZEOF_INTP is not defined
#endif
      }
      ], [he2_cv_32ptr=yes], [he2_cv_32ptr=no],)])
  if test ${he2_cv_32ptr} = "yes"; then
    AC_MSG_NOTICE([possibly 32 bit system])
    F2CFORTRAN_32PTR="yes"
  else
    AC_MSG_NOTICE([possibly 64 bit system; some fortran tests will be skipped.])
  fi

  AC_PROG_FC
  if test -n "$FC"; then
    AC_MSG_CHECKING([for fortran compiler $FC])
    AC_LANG_PUSH([Fortran])
    AC_TRY_LINK([], [
	INTEGER :: I
	REAL :: R
	I=TRANSFER(R,I)
	], [he2_cv_fortran90_compiler=yes], [he2_cv_fortran90_compiler=no])
    AC_LANG_POP([Fortran])
    if test ${he2_cv_fortran90_compiler} = "yes"; then
      AC_MSG_RESULT([Fortran 90 code was successfully compiled; assume f90 or later])
      F2CFORTRAN_90="yes"
    else
      AC_MSG_RESULT([Fortran 90 code format was not compiled; assume f77])
    fi
    if test -n "$F77" -o -n "$FFLAGS"; then
      AC_MSG_NOTICE([$FC, $FCFLAGS will override $F77, $FFLAGS.])
    fi
    F77=$FC
    FFLAGS=$FCFLAGS
  else
    AC_MSG_NOTICE([f90 or compatible was not detected.])
    AC_PROG_F77
    if test -n "$F77"; then
      AC_MSG_NOTICE([f77 or compatible was detected. $F77 will be used to check Fortran testdrivers.])
    else
      AC_MSG_ERROR([couldn't find fortran compiler. You may want to FC environment variable])
    fi
  fi
fi


dnl --------------------------------------------------
dnl Check whether szlib is enabled.
dnl If HDF4 is built with szlib, user should use --with-szlib=DIR option.
dnl If this option is given, whether SZIP is decoder only or decoder plus encoder
dnl should be checked.
AC_ARG_WITH([szlib],
            [AC_HELP_STRING([--with-szlib=DIR],
                            [Use szlib library for external szlib I/O
                             filter.  Linker must be able to find szlib
                             if HDF4 was build with szip. [default=no]])],,
            withval=no)

case $withval in
  yes)
    HAVE_SZLIB="yes"
    AC_CHECK_HEADERS([szlib.h], [HAVE_SZLIB_H="yes"])
    AC_CHECK_LIB([sz], [SZ_BufftoBuffCompress],, [unset HAVE_SZLIB])

    if test -z "$HAVE_SZLIB"; then
      AC_MSG_ERROR([couldn't find szlib library, you may need to specify szip library path])
    fi
    ;;
  no)
    HAVE_SZLIB="no"
    AC_MSG_CHECKING([for szlib])
    AC_MSG_RESULT([suppressed])
    ;;
  *)
    HAVE_SZLIB="yes"
    case "$withval" in
      *,*)
        szlib_inc="`echo $withval |cut -f1 -d,`"
        szlib_lib="`echo $withval |cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          szlib_inc="$withval/include"
          szlib_lib="$withval/lib"
        fi
        ;;
    esac

    dnl Check if szlib.h exists. We cannot use AC_CHECK_HEADERS here
    dnl because it will use preprocessor. Preprocessor always succeeds in
    dnl finding szlib.h because h4cc provides the path to szlib.h.
    AC_CHECK_FILE($szlib_inc/szlib.h, , [unset HAVE_SZLIB])
    if test -z "$HAVE_SZLIB"; then
      AC_MSG_ERROR([couldn't find szlib.h, check szip library path])
    fi
    
    dnl Trying to include -I/usr/include and -L/usr/lib is redundant and
    dnl can mess some compilers up.
    if test "X$szlib_inc" = "X/usr/include"; then
      szlib_inc=""
    fi 
    if test "X$szlib_lib" = "X/usr/lib"; then
      szlib_lib=""
    fi
    
    saved_CPPFLAGS="$CPPFLAGS"
    saved_LDFLAGS="$LDFLAGS"
    
    if test -n "$szlib_inc"; then
      CPPFLAGS="$CPPFLAGS -I$szlib_inc"
    fi
    
    AC_CHECK_HEADERS([szlib.h],
                     [HAVE_SZLIB_H="yes"],
                     [CPPFLAGS="$saved_CPPFLAGS"])
    
    if test -n "$szlib_lib"; then
      LDFLAGS="$LDFLAGS -L$szlib_lib"
    fi
    
    AC_CHECK_LIB([sz], [SZ_BufftoBuffCompress],,
                 [LDFLAGS="$saved_LDFLAGS"; unset HAVE_SZLIB])
    
    if test -z "$HAVE_SZLIB" -a -n "$HDF5_CONFIG_ABORT"; then
      AC_MSG_ERROR([couldn't find szlib library, check if szip library is valid])
    fi
    ;;
esac


if test "X$HAVE_SZLIB" = "Xyes"; then
    dnl SZLIB library is available.  Check if it can encode

    dnl Given that user specifies h4cc, LDFLAGS in h4cc should be overriden; otherwise,
    dnl the encoding test will be performed on the SZIP library linked with HDF4, not
    dnl the SZIP library user specifies. To suppress the SZIP library linked with HDF4,
    dnl we prepend the user-specified SZIP library path to LDFLAGS in h4cc by using awk.

    AC_PROG_AWK

    AC_MSG_CHECKING([for h4cc])
    PURE_CC=""
    if ($CC -show &> /dev/null); then
	dnl Since CC can be "h4cc -Df2cFortran", arguments should be removed
	PURE_CC=`echo "$CC" | $AWK '{ split($1, array, " ") ; printf array[[1]] }'`
	AC_MSG_RESULT([$PURE_CC])
    else
	AC_MSG_RESULT([$CC])
	AC_MSG_ERROR([CC is not h4cc])
    fi

    SZIP_CC=szip_cc
    cat $PURE_CC | $AWK '{ if ( $0 ~ /^LDFLAGS=\"(.*)/ ) { print substr($0, 1, 9) " -L'$szlib_lib' " substr($0, 10) } else  { print } } ' > $SZIP_CC
    chmod 755 $SZIP_CC
    saved_CC="$CC"
    CC=./$SZIP_CC

    AC_MSG_CHECKING([for szlib encoder])

    AC_CACHE_VAL([he2_cv_szlib_functional],
        [AC_TRY_RUN([
	#include <stddef.h>
        #include <szlib.h>

        int main(void)
        {
            SZ_encoder_enabled();
            exit(0);
        }
        ], [he2_cv_szlib_functional=yes], [he2_cv_szlib_functional=no],)])

    AC_CACHE_VAL([he2_cv_szlib_can_encode],
        [AC_TRY_RUN([
	#include <stddef.h>
        #include <szlib.h>

        int main(void)
        {
            /* SZ_encoder_enabled returns 1 if encoder is present */
            if(SZ_encoder_enabled() == 1)
                exit(0);
            else
                exit(1);
        }
        ], [he2_cv_szlib_can_encode=yes], [he2_cv_szlib_can_encode=no],)])

    CC="$saved_CC"
    rm -f $SZIP_CC

    if test ${he2_cv_szlib_functional} = "no"; then
        he2_cv_szlib_can_encode=broken
    else
        AC_DEFINE(HAVE_FILTER_SZIP, 1,
                [Define if support for szip filter is enabled])
        USE_FILTER_SZIP="yes"
    fi

    if test ${he2_cv_szlib_can_encode} = "yes"; then
    	AC_DEFINE(HAVE_FILTER_SZIP_ENCODER, 1,
		[Define if support for szip encoder filter is enabled])
	USE_SZIP_ENCODER="yes"
        AC_MSG_RESULT([yes])
    fi
    if test ${he2_cv_szlib_can_encode} = "no"; then
        AC_MSG_RESULT([no])
    fi
    if test ${he2_cv_szlib_can_encode} = "broken"; then
        AC_MSG_RESULT([shared szlib doesn't work. disabling szip.])
    fi

    dnl Add "szip" to external filter list
    if test ${he2_cv_szlib_can_encode} = "yes"; then
        if test "X$EXTERNAL_FILTERS" != "X"; then
            EXTERNAL_FILTERS="${EXTERNAL_FILTERS},"
        fi
        EXTERNAL_FILTERS="${EXTERNAL_FILTERS}szip(encoder)"
    fi
    if test ${he2_cv_szlib_can_encode} = "no"; then
        if test "X$EXTERNAL_FILTERS" != "X"; then
            EXTERNAL_FILTERS="${EXTERNAL_FILTERS},"
        fi
        EXTERNAL_FILTERS="${EXTERNAL_FILTERS}szip(no encoder)"
    fi
fi

dnl ----------------------------------------------------------------
dnl Check for the HDF4 library using the --with argument
HAVE_HDF4="yes"
AC_ARG_WITH([hdf4],
            [AC_HELP_STRING([--with-hdf4=DIR],
                            [Specify path to HDF4 library to use while building
                            [default=yes]])],,
            withval=yes)

case $withval in
  yes)
    ;;
  no)
    AC_MSG_ERROR([HDF4 disabled in configure, but is required to build HDF-EOS2.])
    ;;
  *)
    case "$withval" in
      *,*)
        hdf4_inc="`echo $withval |cut -f1 -d,`"
        hdf4_lib="`echo $withval |cut -f2 -d, -s`"
        ;;
      *)
        if test -n "$withval"; then
          hdf4_inc="$withval/include"
          hdf4_lib="$withval/lib"
        fi
        ;;
    esac

    dnl Trying to include -I/usr/include and -L/usr/ib is redundant and
    dnl can mess some compilers up.
    if test "X$hdf4_inc" = "X/usr/include"; then
      hdf4_inc=
    fi
    if test "X$hdf4_lib" = "X/usr/lib"; then
      hdf4_lib=
    fi

    if test -n "$hdf4_inc"; then
      CPPFLAGS="$CFLAGS -I$hdf4_inc"
      CFLAGS="$CFLAGS -I$hdf4_inc"
    fi

    if test -n "$hdf4_lib"; then
      LDFLAGS="$LDFLAGS -L$hdf4_lib"
    fi
    ;;
esac

dnl See if we have a valid linking path to the HDF4 library, either because
dnl it's in the user's path or because they provided the correct path using
dnl --with-hdf4
AC_CHECK_HEADERS([hdf.h], , [HAVE_HDF4="no"])
AC_CHECK_HEADERS([mfhdf.h], , [HAVE_HDF4="no"])
AC_CHECK_LIB(df,main,, [HAVE_HDF4="no"])
AC_CHECK_LIB(mfhdf,main,, [HAVE_HDF4="no"])

if test "X$HAVE_HDF4" = "Xno"; then
  AC_MSG_ERROR([can't link against HDF4 library])
fi

if test "x$HAVE_HDF4" = "xyes"; then
  dnl Check if HDF4 is linked with SZIP encoder

  AC_MSG_CHECKING([for hdf4 szip decoding filter])
  AC_CACHE_VAL([he2_cv_hdf4_szip_can_decode],
      [AC_TRY_RUN([
      #include <hdf.h>

      int main(void)
      {
	comp_coder_t codertype = COMP_CODE_SZIP;
	uint32 configinfo;
	int decoder = 0;

	HCget_config_info(codertype, &configinfo);
	decoder = configinfo & COMP_DECODER_ENABLED;
	if (decoder) exit(0);
	else exit(1);
      }
      ], [he2_cv_hdf4_szip_can_decode=yes], [he2_cv_hdf4_szip_can_decode=no],)])
  if test ${he2_cv_hdf4_szip_can_decode} = "yes"; then
    AC_DEFINE(HAVE_HDF4_SZIP_DECODER, 1,
	    [Define if HDF4 has szip decoder filter])
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi

  AC_MSG_CHECKING([for hdf4 szip encoding filter])
  AC_CACHE_VAL([he2_cv_hdf4_szip_can_encode],
      [AC_TRY_RUN([
      #include <hdf.h>

      int main(void)
      {
	comp_coder_t codertype = COMP_CODE_SZIP;
	uint32 configinfo;
	int encoder = 0;

	HCget_config_info(codertype, &configinfo);
	encoder = configinfo & COMP_ENCODER_ENABLED;
	if (encoder) exit(0);
	else exit(1);
      }
      ], [he2_cv_hdf4_szip_can_encode=yes], [he2_cv_hdf4_szip_can_encode=no],)])

  if test ${he2_cv_hdf4_szip_can_encode} = "yes"; then
    AC_DEFINE(HAVE_HDF4_SZIP_ENCODER, 1,
	    [Define if HDF4 has szip encoder filter])
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi

  dnl Check if SZIP linked with HDF4 has the same configuration as SZIP given by --with-szlib option has
  if test ${he2_cv_hdf4_szip_can_decode} = "no"; then
    if test "X$HAVE_SZLIB" = "Xyes"; then
      AC_MSG_ERROR([HDF4 was linked without SZIP, but --with-szlib was given])
    fi
  else
    if test "X$HAVE_SZLIB" != "Xyes"; then
      AC_MSG_ERROR([HDF4 was linked with SZIP, but --with-szlib was not given])
    fi
    if test ${he2_cv_szlib_functional} = "no"; then
      AC_MSG_ERROR([SZIP specified in --with-szlib is not functional])
    fi
    if test ${he2_cv_hdf4_szip_can_encode} = "no"; then
      if test ${he2_cv_szlib_can_encode} = "yes"; then
	AC_MSG_ERROR([HDF4 was linked without SZIP encoder, but yours has SZIP encoder])
      fi
    else
      if test ${he2_cv_szlib_can_encode} = "no"; then
	AC_MSG_ERROR([HDF4 was linked with SZIP encoder, but yours does not have SZIP encoder])
      fi
    fi
  fi
fi

CFLAGS=$CFLAGS

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h malloc.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memchr memmove memset pow sqrt strchr strstr])

# Add a macro to distinguish automatic configuration from 
# the original configuration; this will affect the handling
# of HDFEOSVersion.h file.
AC_DEFINE([EOS2_AUTO_CONFIG], [1],
            [Define the automatic configuration macro.])


# Check whether testdrivers directory is present. The user might have
# downloaded HDF-EOS2 but not the tests.
AC_MSG_CHECKING([for testdrivers directory])
if test -d "${srcdir}/testdrivers"; then
  TESTDRIVERS_DIR="yes"
  AC_MSG_RESULT([present])

  # If the testdrivers directory is present, see if it contains a file
  # named 'makefile'. If so, rename the file so it doesn't cause
  # confusion later on. 
  if test -e "testdrivers/makefile"; then
    mv testdrivers/makefile testdrivers/makefile_orig
  fi

else
  TESTDRIVERS_DIR="no"
  AC_MSG_RESULT([not found])
  AC_MSG_WARN([testdrivers directory is not present. Tests will not be run])
fi

AC_ARG_ENABLE([install-include],
              [AC_HELP_STRING([--enable-install-include],
                              [HDF-EOS2 normally only installs libraries,
                               assuming that users will use the headers in
                               the include directories. Users who want the
                               standard automake behavoior of installing
                               both libraries and header files should
                               enable this option.])],
              [INSTALL_INCLUDE=$enableval])

AM_CONDITIONAL([TESTDRIVERS_CONDITIONAL], [test "X$TESTDRIVERS_DIR" = "Xyes"])
AM_CONDITIONAL([INSTALL_INCLUDE_CONDITIONAL], [test "X$INSTALL_INCLUDE" = "Xyes"])
AM_CONDITIONAL([F2CFORTRAN_CONDITIONAL], [test "X$F2CFORTRAN_MACRO" = "Xyes"])
AM_CONDITIONAL([F2CFORTRAN_90_CONDITIONAL], [test "X$F2CFORTRAN_90" = "Xyes"])
AM_CONDITIONAL([F2CFORTRAN_32PTR_CONDITIONAL], [test "X$F2CFORTRAN_32PTR" = "Xyes"])
AM_CONDITIONAL([SZIP_ENCODER_CONDITIONAL], [test "X$USE_SZIP_ENCODER" = "Xyes"])

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 src/Makefile
                 gctp/Makefile
                 gctp/include/Makefile
                 gctp/src/Makefile
                 samples/Makefile])

if test "X$TESTDRIVERS_DIR" = "Xyes"; then
  AC_CONFIG_FILES([ testdrivers/Makefile
                 testdrivers/grid/Makefile
                 testdrivers/point/Makefile
                 testdrivers/swath/Makefile])
fi

AC_OUTPUT
