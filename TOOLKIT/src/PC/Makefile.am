#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit PC tool group
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

GRP = PC

# force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libpc.la

# List source files here
libpc_la_SOURCES = \
	PGS_PC_PutPCSData.c \
	PGS_PC_GetPCSData.c \
	PGS_PC_OpenPCSFile.c \
	PGS_PC_LocateEntry.c \
	PGS_PC_AdvanceArea.c \
	PGS_PC_GetIndex.c \
	PGS_PC_RetrieveData.c \
	PGS_PC_GetRequest.c \
	PGS_PC_GetFileName.c \
	PGS_PC_OpenFiles.c \
	PGS_PC_AdvanceToLoc.c \
	PGS_PC_PutInArea.c \
	PGS_PC_InsertCheck.c \
	PGS_PC_SkipCheck.c \
	PGS_PC_FixBuffer.c \
	PGS_PC_GetReference.c \
	PGS_PC_GetConfigData.c \
	PGS_PC_GetNumberOfFiles.c \
	PGS_PC_GenUniqueID.c \
	PGS_PC_GetFileAttr.c \
	PGS_PC_BuildAttribute.c \
	PGS_PC_GetFileByAttr.c \
	PGS_PC_BuildFileShm.c \
	PGS_PC_GetDataFromShm.c	\
	PGS_PC_GetFileFromShm.c	\
	PGS_PC_SearchShm.c \
	PGS_PC_PutDataInShm.c \
	PGS_PC_DeleteFileShm.c \
	PGS_PC_WriteNewToShm.c \
	PGS_PC_MarkAtTerm.c \
	PGS_PC_BuildNumericInput.c \
	PGS_PC_CalcArrayIndex.c	\
	PGS_PC_FindDefaultLocLine.c \
	PGS_PC_GetPCEnv.c \
	PGS_PC_GetReferenceType.c \
	PGS_PC_CheckFlags.c \
	PGS_PC_OneMarkRuntime.c	\
	PGS_PC_MarkRuntimeAscii.c \
	PGS_PC_MarkRuntimeShm.c	\
	PGS_PC_MultiRuntimes.c \
	PGS_PC_GetFileByAttrF.c	\
	PGS_PC_GetPCFTemp.c \
	PGS_PC_GetUniversalRef.c \
	PGS_PC_bindFORTRAN.c \
	PGS_PC_GetFileSize.c
