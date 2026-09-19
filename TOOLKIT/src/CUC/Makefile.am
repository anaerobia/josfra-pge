#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit CUC tool group
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

# List subgroups for this tool group
SUBDIRS = ODL UDUNITS

# force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG=all

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/include/CUC

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libcuc.la

# List source files here
libcuc_la_SOURCES = \
	PGS_CUC_Cons.c \
	PGS_CUC_Conv.c \
	PGS_CUC_bindFORTRAN.c

# List subgroups  needed to build the library
libcuc_la_LIBADD = \
	ODL/libcucodl.la \
	UDUNITS/libcucudunits.la
