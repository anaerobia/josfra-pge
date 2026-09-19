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
CPPFLAGS += -I$(top_srcdir)/include -I$(top_builddir)/src

LIBS = $(top_builddir)/src/libPGSTK.la -lpthread

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Path to PC source directory
PCSOURCE = $(top_srcdir)/src/PC

# Create and compile CSC executables
archbindir = $(bindir)/$(BRAND)

# Compile all PC utilities
archbin_PROGRAMS = \
	pctcheck \
	PGS_PC_InitCom \
	PGS_PC_TermCom \
	PGS_PC_GetConfigDataCom \
	PGS_PC_GetNumberOfFilesCom \
	PGS_PC_TempDeleteCom \
	PGS_PC_GetReferenceCom \
	PGS_PC_GetFileAttrCom \
	PGS_PC_GetTempReferenceCom \
	PGS_PC_GetFileSizeCom

# pctcheck
pctcheck_SOURCES = $(PCSOURCE)/PGS_PC_Check.c
pctcheck_LDADD = $(LIBS)

# PGS_PC_InitCom
PGS_PC_InitCom_SOURCES = $(PCSOURCE)/PGS_PC_InitCom.c
PGS_PC_InitCom_LDADD = $(LIBS)

# PGS_PC_TermCom
PGS_PC_TermCom_SOURCES = $(PCSOURCE)/PGS_PC_TermCom.c
PGS_PC_TermCom_LDADD = $(LIBS)

# PGS_PC_GetConfigDataCom
PGS_PC_GetConfigDataCom_SOURCES = $(PCSOURCE)/PGS_PC_GetConfigDataCom.c
PGS_PC_GetConfigDataCom_LDADD = $(LIBS)

# PGS_PC_GetNumberOfFilesCom
PGS_PC_GetNumberOfFilesCom_SOURCES = $(PCSOURCE)/PGS_PC_GetNumberOfFilesCom.c
PGS_PC_GetNumberOfFilesCom_LDADD = $(LIBS)

# PGS_PC_TempDeleteCom
PGS_PC_TempDeleteCom_SOURCES = $(PCSOURCE)/PGS_PC_TempDeleteCom.c
PGS_PC_TempDeleteCom_LDADD = $(LIBS)

# PGS_PC_GetReferenceCom
PGS_PC_GetReferenceCom_SOURCES = $(PCSOURCE)/PGS_PC_GetReferenceCom.c
PGS_PC_GetReferenceCom_LDADD = $(LIBS)

# PGS_PC_GetFileAttrCom
PGS_PC_GetFileAttrCom_SOURCES = $(PCSOURCE)/PGS_PC_GetFileAttrCom.c
PGS_PC_GetFileAttrCom_LDADD = $(LIBS)

# PGS_PC_GetTempReferenceCom
PGS_PC_GetTempReferenceCom_SOURCES = $(PCSOURCE)/PGS_PC_GetTempReferenceCom.c
PGS_PC_GetTempReferenceCom_LDADD = $(LIBS)

# PGS_PC_GetFileSizeCom
PGS_PC_GetFileSizeCom_SOURCES = $(PCSOURCE)/PGS_PC_GetFileSizeCom.c
PGS_PC_GetFileSizeCom_LDADD = $(LIBS)

# Install scripts
archbin_SCRIPTS = $(PCSOURCE)/pccheck.sh $(PCSOURCE)/PGS_PC_Shell.sh
