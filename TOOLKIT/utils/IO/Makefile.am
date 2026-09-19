#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit IO tool group
#
# author:
#	Matthew Yun / Raytheon 
#
# history:
#	5-Aug-2013 Initial version
#
# notes:
#	This Makefile was left here in case the user wanted to build the
#	contents of the L0sim directory. The toolkit does not build this 
#	utility, but the user can build them if they want by creating a
#	Makefile in the L0sim directory. The libraries for this utility is
#	built with the rest of the toolkit library.
#----------------------------------------------------------------------------

GRP = IO

# List subgroups for this tool group
SUBDIRS = L0

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# Force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG=all

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f
