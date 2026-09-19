#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit CBP tool group
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

GRP = CBP

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libcbp.la

libcbp_la_SOURCES = \
	PGS_CBP_EphemRead.c \
	PGS_CBP_Earth_CB_Vector.c \
	PGS_CBP_Sat_CB_Vector.c \
	PGS_CBP_SolarTimeCoords.c \
	PGS_CBP_bindFORTRAN.c \
	PGS_CBP_body_inFOV.c
