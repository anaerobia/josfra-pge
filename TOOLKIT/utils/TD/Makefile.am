#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit TD tool group
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

GRP = TD

# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_builddir)/src

# Path to TD source directory
TDSOURCE = $(top_srcdir)/src/TD

# Create and compile TD executables
archbindir = $(bindir)/$(BRAND)

archbin_PROGRAMS = PGS_TD_NewLeap

PGS_TD_NewLeap_SOURCES = $(TDSOURCE)/PGS_TD_NewLeap.c
PGS_TD_NewLeap_LDADD = $(top_builddir)/src/libPGSTK.la -lpthread

# Install scripts
archbin_SCRIPTS = $(TDSOURCE)/update_leapsec.sh $(TDSOURCE)/update_leapsec_CC.sh
