#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit AA generic sub group
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

# Set the name of this sub group
GRP = AA

# define sh shell (needed for embedded shell scripts)
SHELL = /bin/sh

CPPFLAGS += -I. -I$(top_srcdir)/include -I$(top_srcdir)/include/FF

# Name of the remove utility and flags
RM = /bin/rm
RMFLAGS = -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libaageneric.la

libaageneric_la_CFLAGS = -DSUNCC -DPROTO

# List source files here
libaageneric_la_SOURCES = \
	PGS_AA_2DRead.c \
	PGS_AA_2DReadF.c \
	PGS_AA_3DRead.c \
	PGS_AA_3DReadF.c \
	PGS_AA_2Dgeo.c \
        PGS_AA_2DgeoF.c \
	PGS_AA_3Dgeo.c \
	PGS_AA_3DgeoF.c \
	PGS_AA_2DReadGrid.c \
	PGS_AA_2DReadGridF.c \
	PGS_AA_3DReadGrid.c \
	PGS_AA_3DReadGridF.c \
	PGS_AA_GEOGrid.c \
	PGS_AA_GEOGridF.c \
	PGS_AA_PeV.c \
	PGS_AA_PeVA.c \
	PGS_AA_Map.c \
	PGS_AA_Operation.c \
	PGS_AA_FF_Setup.c  \
	PGS_AA_FF.c \
	PGS_AA_dem.c \
	PGS_AA_demF.c \
	PGS_AA_bindFORTRAN.c
