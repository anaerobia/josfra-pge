#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit MET tools sub group
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

GRP = MET

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/include/CUC

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libmettools.la

libmettools_la_CFLAGS = -DPGS_MET_COMPILE

# List source files here
libmettools_la_SOURCES = \
	PGS_MET_GetSetAttr.c \
	PGS_MET_GetSetAttrF.c \
	PGS_MET_Init.c \
	PGS_MET_SetAttr.c \
	PGS_MET_SetAttrF.c \
	PGS_MET_SetMultiAttr.c \
	PGS_MET_SetMultiAttrF.c \
	PGS_MET_GetPCAttr.c \
	PGS_MET_GetPCAttrF.c \
	PGS_MET_GetConfigData.c \
	PGS_MET_GetConfigDataF.c \
	PGS_MET_Write.c \
	PGS_MET_Remove.c \
	PGS_MET_HDFFileType.c \
	PGS_MET_HDFSDstart.c \
	PGS_MET_bindFORTRAN.c
