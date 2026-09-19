#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit IO L0 sub group
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

SUBDIRS = L0sim

SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# Default target
TARG=all

# name of remove utility
RM = /bin/rm
RMFLAGS = -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libiol0.la

# List source files here
libiol0_la_SOURCES = \
	PGS_IO_L0_BYTEtoINT.c \
        PGS_IO_L0_Close.c \
        PGS_IO_L0_FileVersionInfo.c \
        PGS_IO_L0_GetEOSAMfileTimes.c \
        PGS_IO_L0_GetEOSPMGIISfileTimes.c \
        PGS_IO_L0_GetEOSPMGIRDfileTimes.c \
        PGS_IO_L0_GetEOSAURAfileTimes.c \
        PGS_IO_L0_GetHeader.c \
        PGS_IO_L0_GetPacket.c \
        PGS_IO_L0_ManageTable.c \
        PGS_IO_L0_MapVersions.c \
        PGS_IO_L0_NextPhysical.c \
        PGS_IO_L0_Open.c \
        PGS_IO_L0_SeekPacket.c \
        PGS_IO_L0_SetStart.c \
        PGS_IO_L0_SetStartCntPkts.c \
        PGS_IO_L0_TRMM_HdrInfo.c \
	PGS_IO_L0_VersionInfoCheck.c \
	PGS_IO_L0_bindFORTRAN_L0.c

libiol0_la_LIBADD = \
	L0sim/libiol0l0sim.la
