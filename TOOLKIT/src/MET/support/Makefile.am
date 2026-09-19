#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit MET support sub group
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
noinst_LTLIBRARIES = libmetsupport.la

# define C preprocessor symbols
libmetsupport_la_CFLAGS = -DPGS_MET_COMPILE

# List source files here
libmetsupport_la_SOURCES = \
	PGS_MET_LoadAggregate.c \
	PGS_MET_ErrorMsg.c \
	PGS_MET_CheckAttr.c \
	PGS_MET_GetDDAttr.c \
	PGS_MET_CheckAgainstDD.c \
	PGS_MET_ConvertToOdl.c \
	PGS_MET_GetConfigByLabel.c \
	PGS_MET_RetrieveConfigData.c \
	PGS_MET_HDFToODL.c \
	PGS_MET_NameAndClass.c \
	PGS_MET_SearchAttr.c \
	PGS_MET_SearchAttrF.c \
	PGS_MET_ConvertToMCF.c \
	PGS_MET_GetSetAttrTD.c \
	PGS_MET_ODLToXML.c \
	PGS_MET_WriteXML.c \
	PGS_MET_XslProcessor.c
