#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit MEM tool group
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

GRP = MEM

# force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libmem.la

libmem_la_CFLAGS = -DSHMMEM

# List source files here
libmem_la_SOURCES = \
	PGS_MEM.c \
	PGS_MEM1.c \
	PGS_MEM_ShmReadF.c\
	PGS_MEM_ShmWriteF.c \
	PGS_MEM_bindFORTRAN.c
