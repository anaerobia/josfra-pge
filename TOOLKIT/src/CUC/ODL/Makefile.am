#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit CUC ODL sub group
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

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Default target
TARG = all

# name of remove utility
RM = /bin/rm
RMFLAGS = -f

# Path for 'include' directive
CPPFLAGS += -I. -I$(top_srcdir)/include -I$(top_srcdir)/include/CUC

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libcucodl.la

# define C preprocessor symbols
libcucodl_la_CFLAGS = -DSUN_UNIX

# List source files here
libcucodl_la_SOURCES = \
	a_nodesa.c \
	ag_nodesag.c \
	ao_nodesao.c \
	comments.c \
	cvtvalue.c \
	fmtvalue.c \
	lexan.c \
	p_nodesp.c \
	parsact.c \
	parser.c \
	prtlabel.c \
	prtsrc.c \
	rdlabel.c \
	rdvalue.c \
	v_nodesv.c \
	wrtlabel.c \
	wrtsrc.c \
	output.c
