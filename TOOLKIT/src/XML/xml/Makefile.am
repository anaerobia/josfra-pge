#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit XML xml sub group
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
CPPFLAGS += -I. -I$(top_srcdir)/include -I$(top_srcdir)/include/libxml

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libxmlxml.la

# define C preprocessor symbols 
libxmlxml_la_CFLAGS = -DSUN_UNIX

# List source files here
libxmlxml_la_SOURCES = \
	SAX.c \
	entities.c \
	encoding.c \
	error.c \
	parserInternals.c \
	parser.c \
	tree.c \
	hash.c \
	list.c \
	xmlIO.c \
	xmlmemory.c \
	uri.c \
	valid.c \
	xlink.c \
	HTMLparser.c \
	HTMLtree.c \
	debugXML.c \
	xpath.c \
	xpointer.c \
	xinclude.c \
	nanohttp.c \
	nanoftp.c \
	DOCBparser.c \
	catalog.c \
	globals.c \
	threads.c \
	c14n.c \
	xmlstring.c \
	xmlregexp.c \
	xmlschemas.c \
	xmlschemastypes.c \
	xmlunicode.c \
	xmlreader.c \
	relaxng.c \
	dict.c \
	SAX2.c \
	xmlwriter.c \
	legacy.c \
	chvalid.c \
	pattern.c \
	xmlsave.c \
	xmlmodule.c \
	schematron.c
