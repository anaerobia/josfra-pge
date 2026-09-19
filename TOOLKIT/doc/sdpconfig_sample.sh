#!/bin/bash
############################################################
#   Shell script that can be used for auto configuration   #
#       after a minor modifcation for pre-installed        #
#       hdf4, hdf5, szip, zlib, and jpeg directories,      #
#       and for flags and/or compilers                     #
############################################################
HDFDIR=/tools/pgs/ataaheri/TK5219D/TOOLKIT_MTD/hdf/linux32/hdf-4.2.10
HDFDIR5=/tools/pgs/ataaheri/TK5219D/TOOLKIT_MTD/hdf5/linux32/hdf5-1.8.12
HDFEOS2=/tools/pgs/ataaheri/TK5219D/TOOLKIT_MTD/hdfeos
HDFEOS5=/tools/pgs/ataaheri/TK5219D/TOOLKIT_MTD/hdfeos5
HDF4=$HDFDIR
HDF5=$HDFDIR5
JPEG=$HDFDIR
SZIP=/tools/pgs/ataaheri/TK5219D/TOOLKIT_MTD/szip/linux32/szip-2.1
ZLIB=$HDFDIR
export CC="gcc -m32"
export FC="gfortran -m32 -ff2c -fPIC -fno-second-underscore"
export CPPFLAGS="-DNDEBUG -Df2cFortran -DH5_USE_16_API"
export HDFSYS="LINUX32"
############################################################
#               Do not change below this line              #
############################################################
./configure \
--with-hdf4=$HDF4 \
--with-hdf5=$HDF5 \
--with-szlib=$SZIP \
--with-zlib=$ZLIB \
--with-jpeg=$JPEG \
--with-hdfeos2=$HDFEOS2 \
--with-hdfeos5=$HDFEOS5 \
--enable-32-bit
#--enable-fortran \
#--with-hdf5=$HDF5/include,$HDF5/lib \
#--with-zlib=$ZLIB/include,$ZLIB/lib \
##--with-hdf4=$HDF4/include,$HDF4/lib \
#--with-jpeg=$JPEG/include,$JPEG/lib \
#--with-szlib=$SZIP/include,$SZIP/lib
