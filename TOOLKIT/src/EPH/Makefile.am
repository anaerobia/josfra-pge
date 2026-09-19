#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit EPH tool group
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

GRP = EPH

# force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# name of remove utility
RM = /bin/rm
RMFLAGS = -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libeph.la

# List source files here
CSOURCES = \
	PGS_EPH_EphemAttit.c \
	PGS_EPH_EphAtt_unInterpolate.c \
	PGS_EPH_UnInterpEphAtt.c \
	PGS_EPH_GetEphMet.c \
	PGS_EPH_GetSpacecraftData.c \
	PGS_EPH_ManageMasks.c \
	PGS_EPH_bindFORTRAN.c \
	PGS_EPH_getEphemHeaders.c \
	PGS_EPH_getAttitHeaders.c \
	PGS_EPH_getEphemRecords.c \
	PGS_EPH_getAttitRecords.c \
	PGS_EPH_getToken.c \
	PGS_EPH_fileHandlingStuff.c \
        PGS_EPH_interpolateAttitude.c \
        PGS_EPH_interpolatePosVel.c

if BUILD_FORTRAN
FFLAGS += \
	-I$(top_srcdir)/include

FSOURCES = \
	PGS_EPH_GetEphMetF.f \
	PGS_EPH_UnInterpEphAttF.f \
	PGS_EPH_EphAtt_unInterpolateF.f
endif

if BUILD_FORTRAN
libeph_la_SOURCES = $(CSOURCES) $(FSOURCES)
else
libeph_la_SOURCES = $(CSOURCES)
endif
