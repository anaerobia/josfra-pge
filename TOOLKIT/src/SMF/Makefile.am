#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit SMF tool group
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

GRP = SMF

# force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# name of remove utility
RM = /bin/rm
RMFLAGS = -f 

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libsmf.la

# List source files here
libsmf_la_SOURCES = \
	PGS_SMF.c \
	PGS_SMF1.c \
	PGS_SMF_CacheMsgDynm.c \
	PGS_SMF_Comm.c \
	PGS_SMF_GetToolkitVersion.c \
	PGS_SMF_InitializeLogging.c \
	PGS_SMF_LogPID.c \
	PGS_SMF_LoggingControl.c \
	PGS_SMF_ManageLogControlList.c \
	PGS_SMF_SendRuntimeData.c \
	PGS_SMF_SendStatusReport.c \
	PGS_SMF_TraceControl.c \
	PGS_SMF_bindFORTRAN.c
