#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit CBP tool group
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

GRP = CBP

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_builddir)/src

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f

# Path to CBP src directory
CBPSOURCE = $(top_srcdir)/src/CBP

# Create and compile CSC executables
archbindir = $(bindir)/$(BRAND)

archbin_PROGRAMS = ephtobin

ephtobin_SOURCES = $(CBPSOURCE)/PGS_CBP_ASCtoBIN.c

ephtobin_LDADD = $(top_builddir)/src/libPGSTK.la
