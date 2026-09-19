#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:         makefile for UDUNITS CUC tools 
#               (subgroup of the PGS Toolkit CUC group)
#
# environment:  MPPDE, machine-independent, PGS directory structure
# 
# environment variables dependencies:
#   compiler:  CC CFHFLAGS F77
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSOBJ HDFSYS
#  
# author:  Mike Sucher / Applied Research Corporation
#          Tom W. Atwater / Applied Research Corporation 
#          Guru Tej S. Khalsa / Applied Research Corporation
#
# history:
#       15-Aug-1997 GTSK Initial version.
#
# notes:
#       1) This file is intended for use in the Multi-Platform PGS Development
#          Environment (MPPDE) .  It depends on the PGS-defined toolkit 
#          directory structure, and on environmental variables defined
#          by MPPDE startup files.  This is how machine independence
#          is maintained.
#       2) Target object files are moved to $(PGSOBJ)/CUC
#       3) To build add the UDUNITS files to the Toolkit library
#          type: make lib
#----------------------------------------------------------------------------
#
# Set the name of this tool group
#

GRP = CUC

# force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG = all

# name of remove utility
RM = /bin/rm
RMFLAGS = -f 

# define C preprocessor symbols 
DFLAGS = -D$(HDFSYS) -DUT_DEFAULT_PATH='"$(PGSHOME)/database/common/CUC/udunits.dat"'

# path for #include directive
IFLAG =  -I. -I/. $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC)/CUC -I$(PGSINC)

# object files from this sub-group needed to build library
LIBOFILES      =  \
	$(PGSOBJ)/$(GRP)/utlib.o \
	$(PGSOBJ)/$(GRP)/utparse.o \
	$(PGSOBJ)/$(GRP)/utscan.o \
	$(PGSOBJ)/$(GRP)/udalloc.o

#
# General rules
#

all: $(LIBOFILES)

lib: $(LIBOFILES)
	@$(PGSBIN)/mkpgslib $(GRP)

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) $(LIBOFILES)

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/utlib.o: utlib.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/utparse.o: utparse.c
	$(CC) -c $(CFHFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/utscan.o: utscan.c
	$(CC) -c $(CFHFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/udalloc.o: udalloc.c
	$(CC) -c $(CFHFLAGS) $(IFLAG) $? -o $@

.c.o:
	@$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
