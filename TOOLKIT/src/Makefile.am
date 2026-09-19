#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit source files
#
# author:
#	Matthew Yun / Raytheon 
#
# history:
#	5-Aug-2013 Initial version
#
# notes:
#
#----------------------------------------------------------------------------

# List subgroups for this tool group
if BUILD_AA
SUBDIRS = TSF AA CBP CSC CUC DEM EPH GCT IO MEM MET PC SMF TD XML .
else
SUBDIRS = TSF CBP CSC CUC DEM EPH GCT IO MEM MET PC SMF TD XML .
endif

# force make to use the 'sh' shell
SHELL = /bin/sh

# Add 'include' directory
CPPFLAGS += -I$(top_srcdir)/include $(RANLIB)

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Specify the bin directory
archlibdir = $(libdir)/$(BRAND)

# Source files from the subgroups needed to build the library
archlib_LTLIBRARIES = libPGSTK.la

libPGSTK_la_SOURCES = 

# List subgroups  needed to build the library
libPGSTK_la_LIBADD = \
	CBP/libcbp.la \
	CSC/libcsc.la \
	CUC/libcuc.la \
	DEM/libdem.la \
	EPH/libeph.la \
	GCT/libgct.la \
	IO/libio.la \
	MEM/libmem.la \
	MET/libmet.la \
	PC/libpc.la \
	SMF/libsmf.la \
	TD/libtd.la \
	TSF/libtsf.la \
	XML/libxml.la

# Include AA libraries if requested
if BUILD_AA
libPGSTK_la_LIBADD += AA/libaa.la
endif

