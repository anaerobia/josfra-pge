#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit XML tool group
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

GRP = XML

# List subgroups for this tool group
SUBDIRS = xml xsl

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Default target
TARG=all

# name of remove utility
RM = /bin/rm
RMFLAGS = -f 

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/include/libxml -I$(top_srcdir)/include/xslt

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libxml.la

# List source files here
libxml_la_SOURCES = 

# List subgroups  needed to build the library
libxml_la_LIBADD = \
	xml/libxmlxml.la \
	xsl/libxmlxsl.la
