#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit AA DCW sub group
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

# Set the tool group name
GRP = AA

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# path for #include directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/include/DCW

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f 

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libaadcw.la

libaadcw_la_SOURCES = \
	PGS_AA_dcw.c \
	PGS_AA_bindFORTRAN.c
