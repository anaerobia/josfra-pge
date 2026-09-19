#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit CUC UDUNITS sub group
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

GRP = CUC

# force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG = all

# name of remove utility
RM = /bin/rm
RMFLAGS = -f 

# Path for 'include' directive
CPPFLAGS += -I. -I/. -I$(top_srcdir)/include/CUC -I$(top_srcdir)/include

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libcucudunits.la

libcucudunits_la_CFLAGS = -DUT_DEFAULT_PATH='"$(PGSHOME)/database/common/CUC/udunits.dat"'

# List source files here
libcucudunits_la_SOURCES = \
	utlib.c \
	utparse.c \
	utscan.c \
	udalloc.c
