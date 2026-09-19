#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit AA tool group
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
# Set the name of tool group
GRP = AA

# List subgroups for this tool group
SUBDIRS = DCW VPF freeform generic

# force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG=all

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Path for #include directive
CPPFLAGS += -I$(top_srcdir)/include

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libaa.la

libaa_la_SOURCES = 

# List subgroups  needed to build the library
libaa_la_LIBADD = \
	DCW/libaadcw.la \
	VPF/libaavpf.la \
	freeform/libaafreeform.la \
	generic/libaageneric.la
