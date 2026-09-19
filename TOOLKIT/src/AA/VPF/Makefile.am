#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit AA VPF sub group
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

# Set the name of tool group
GRP = AA

# path for include directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/include/DCW 

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libaavpf.la

libaavpf_la_CFLAGS = -DSUNCC -DFPROTO -D_INCLUDE_POSIX_SOURCE

libaavpf_la_SOURCES = \
	coorgeom.c \
	linklist.c \
	set.c \
	strfunc.c \
	vpfprim2.c \
	vpfquery.c \
	vpfrelat.c \
	vpftable.c \
	vpftidx.c \
	vpfwrite.c \
	vpfspx.c \
	vpfsprel.c \
	vpfread.c \
	vvmisc.c
