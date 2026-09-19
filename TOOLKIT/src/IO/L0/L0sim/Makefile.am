#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit IO L0 L0sim sub group
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

SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libiol0l0sim.la

# List source files here
libiol0l0sim_la_SOURCES = \
	PGS_IO_L0_File_Sim.c \
	PGS_IO_L0_EDOS_hdr_Sim.c \
	PGS_IO_L0_sortArrayIndices.c \
	PGS_IO_L0_SFDU_Sim.c \
	PGS_IO_L0_L0Sim_bindFORTRAN.c
