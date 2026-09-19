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
CPPFLAGS += -I$(top_srcdir)/include -I$(top_builddir)/src

# name of remove utility
RM = /bin/rm
RMFLAGS = -f 

# Path to SMF source directory
SMFSOURCE = $(top_srcdir)/src/SMF

# Create and compile SMF executables
archbindir = $(bindir)/$(BRAND)

archbin_PROGRAMS = smfcompile

smfcompile_SOURCES = $(SMFSOURCE)/PGS_SMF_Comp.c
smfcompile_LDADD = -lpthread
