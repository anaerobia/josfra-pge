# AUTOCONF_INSTALL.sh
#
# The shell script to install TOOLKIT using Autoconf.
#
# Specify all the paths containing the required programs below. This
# script will then run ./configure with all the file paths for you!
# The TOOLKIT will be constructed and made in the source directory.
# Once that is completed, run the shell script and the TOOLKIT should 
# be installed!
#
# INSTALL INSTRUCTIONS:
#
#	1. Specify directory paths for HDF4, HDF5, jpeg, szlib, zlib,
#		HDFEOS, and HDFEOS5 below.
#
#	2. Run `bash AUTHOCONF_INSTALL.sh`.
#
#	3. Run `make`, then `make install`.
#
#	4. After configuration, run `make distclean` to clean out source
#		directory.
#

# HDF4 path
HDF4=/home/myun/HDFLibraries/linux32/hdf-4.2.10
# HDF5 path
HDF5=/home/myun/HDFLibraries/linux32/hdf5-1.8.12
# jpeg path
JPEG=/home/myun/HDFLibraries/linux32/jpeg-6b
# szip path
SZIP=/home/myun/HDFLibraries/linux32/szip-2.1
# zlib path
ZLIB=/home/myun/HDFLibraries/linux32/zlib-1.2.8
# HDFEOS path
HDFEOS2=/home/myun/HDFLibraries/linux32/hdfeos
# HDFEOS5 path
HDFEOS5=/home/myun/HDFLibraries/linux32/hdfeos5


# Do not change anything below this line
#	|	|	|	|
#	V	V	V	V
#--------------------------------------------------

cd ..

./configure \
--with-hdf4=$HDF4 \
--with-hdf5=$HDF5 \
--with-szlib=$SZIP \
--with-zlib=$ZLIB \
--with-jpeg=$JPEG \
--with-hdfeos2=$HDFEOS2 \
--with-hdfeos5=$HDFEOS5
# --enable-32-bit \
# --enable-fortran \
# --with-aa

# **Remember, if you are compiling under Solaris, you must edit the 
#   $(srcdir)/message/Makefile and find and delete this line:
#     
#     export PGSINC
#
#   Then after ./configure is complete, you must set the PGSINC
#   environment variable by typing in this line:
#
#     setenv PGSINC ../include
#
#   Only after this can you run make without error.
#


