#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit utilities files
#
# author:
#	Matthew Yun / Raytheon 
#
# history:
#	5-Aug-2013 Initial version
#
# notes:
#	Here is where all the SDP Toolkit utilities will be built. Since 
#	none of the source files are contained in this folder, all the
#	Makefiles will link to the files they need in the src directory and
#	compile the utilities in this folder. 
#	These were kept seperate from the library because the utilities depend 
#	on the complete toolkit library. The utilites are built after the 
#	library has been built.
#----------------------------------------------------------------------------

# List subgroups for this tool group
SUBDIRS = CBP CSC EPH PC SMF TD

# force make to use the 'sh' shell
SHELL = /bin/sh

CPPFLAGS += -I$(top_srcdir)/include

# name of remove utility
RM= /bin/rm
RMFLAGS= -f
