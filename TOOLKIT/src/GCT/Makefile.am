#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit GCT tool group
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

GRP = GCT

# List subgroups for this tool group
SUBDIRS = freeware

# force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG=all

# Path for 'include' directive
CPPFLAGS = -Df2cFortran -I$(top_srcdir)/include -I$(top_srcdir)/include/FW

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libgct.la

# List source files here
libgct_la_SOURCES = \
	PGS_GCT_Init.c \
	PGS_GCT_Proj.c \
	PGS_GCT_bindFORTRAN.c

# List subgroups  needed to build the library
libgct_la_LIBADD = \
	freeware/libgctfreeware.la
