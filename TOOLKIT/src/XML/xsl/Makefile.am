#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit XML xsl sub group
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

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Default target
TARG=all

# name of remove utility
RM = /bin/rm
RMFLAGS = -f

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/include/libxslt -I$(top_srcdir)/include/libxml

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libxmlxsl.la

# define optimization level for compilation
libxmlxsl_la_CFLAGS = -DSUN_UNIX

# List source files here
libxmlxsl_la_SOURCES = \
	attrvt.c \
	xslt.c \
	xsltutils.c \
	xslpattern.c \
	templates.c \
	variables.c \
	keys.c \
	numbers.c \
	extensions.c \
	extra.c \
	functions.c \
	namespaces.c \
	imports.c \
	attributes.c \
	documents.c \
	preproc.c \
	transform.c \
	security.c
