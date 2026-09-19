#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit MET tool group
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

GRP = MET

# List subgroups for this tool group
SUBDIRS = support tools

# Force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# Default target
TARG=all

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libmet.la

# List source files here
libmet_la_SOURCES = 

# List subgroups  needed to build the library
libmet_la_LIBADD = \
	support/libmetsupport.la \
	tools/libmettools.la
