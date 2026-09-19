#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit IO tool group
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

GRP = IO

# List subgroups for this tool group
SUBDIRS = GEN L0

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# Force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG=all

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libio.la

libio_la_SOURCES =

# List subgroups  needed to build the library
libio_la_LIBADD = \
	GEN/libiogen.la \
	L0/libiol0.la
