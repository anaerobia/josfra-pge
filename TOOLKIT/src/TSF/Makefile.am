#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit TSF tool group
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

GRP = TSF

# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/include/FF

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libtsf.la

libtsf_la_CFLAGS = -DSUNCC

libtsf_la_SOURCES = \
	PGS_TSF_GetTSFMaster.c \
	PGS_TSF_HandleLock.c \
	PGS_TSF_SetupAA.c \
	PGS_TSF_SetupCBP.c \
	PGS_TSF_SetupCSC.c \
	PGS_TSF_SetupDEM.c \
	PGS_TSF_SetupSMF.c \
	PGS_TSF_SetupIO.c \
	PGS_TSF_SetupTD.c \
	PGS_TSF_SetupEPH.c \
	PGS_TSF_SetupMET.c \
	PGS_TSF_SetupGCT.c
