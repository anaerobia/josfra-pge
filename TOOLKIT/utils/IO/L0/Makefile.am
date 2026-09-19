#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit IO L0 sub group
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

SUBDIRS = L0sim

SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# Default target
TARG=all

# name of remove utility
RM = /bin/rm
RMFLAGS = -f
