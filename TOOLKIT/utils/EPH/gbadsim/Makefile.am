#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit EPH gbadsim sub group
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

GRP = EPH

# force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_builddir)/src

# name of remove utility
RM= /bin/rm
RMFLAGS= -rf

# Path to gbadsim source directory
gbadsimSOURCE = $(top_srcdir)/src/EPH/gbadsim

# Create and compile CSC executables
archbindir = $(bindir)/$(BRAND)

archbin_PROGRAMS = gbadsim

# List source files here
gbadsim_SOURCES = $(gbadsimSOURCE)/gbadsim.c

gbadsim_LDADD = \
	$(top_builddir)/src/libPGSTK.la \
	-lpthread \
	-lmfhdf \
	-ldf
