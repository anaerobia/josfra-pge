#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit CSC tool group
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

GRP = CSC

# force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_builddir)/src

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# Path to CSC src directory
CSCSOURCE = $(top_srcdir)/src/CSC

# Create and compile CSC executables
archbindir = $(bindir)/$(BRAND)

archbin_PROGRAMS = PGS_CSC_UT1_update

PGS_CSC_UT1_update_SOURCES = $(CSCSOURCE)/PGS_CSC_UT1_update.c
PGS_CSC_UT1_update_LDADD = $(top_builddir)/src/libPGSTK.la -lpthread

# Install scripts
archbin_SCRIPTS = $(CSCSOURCE)/update_utcpole.sh $(CSCSOURCE)/update_utcpole_CC.sh
