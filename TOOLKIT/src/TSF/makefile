#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for PGS Toolkit Thread-Safe Functions (TSF)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSOBJ PGSBIN
#  
# author:  Ray Milburn / Steven Myers & Associates
#
# history:
#	29-Arp-1999 RM   Template version
#
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE).  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.  Before using this makefile do the following:
#            %source $PGSBIN/pgs-dev-env.csh
#          (csh users only--others may have to manually set the variables
#          mentioned above to the values appropriate for their platform
#          as is done automatically in pgs-dev-env.csh)
# 	2) Target object files are moved to $(PGSOBJ)/$(GRP).
# 	3) This file does builds the library for this group.
#
#----------------------------------------------------------------------------

#
# set the name of this tool group
#

GRP=TSF

include ../ecs.options

# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# define C preprocessor symbols 
DFLAGS = -D$(HDFSYS) -DSUNCC

# path for #include directive
#IFLAG   = -I$(PGSINC) -I$(PGSINC)/FF -I$(HDFINC) -I$(HDFEOS_INC) $(ADD_IFLAGS) 
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(PGSINC)/FF -I$(HDFINC) -I$(HDF5INC) -I$(HDFEOS_INC) $(ADD_IFLAGS) 

# path for libraries linked
LFLAG   = -L$(PGSLIB) $(ADD_LFLAGS)

# libraries linked for C build
LIBS   = -lPGSTK $(ADD_LIBS) -lm

# object files from this sub-group needed to build library
#
# !!! LIST ALL OBJECT FILES HERE NEEDED FOR THE GROUP LIBRARY !!!
#
LIBOFILES	=  \
     $(PGSOBJ)/$(GRP)/PGS_TSF_GetTSFMaster.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_HandleLock.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupAA.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupCBP.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupCSC.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupDEM.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupSMF.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupIO.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupTD.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupEPH.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupMET.o \
     $(PGSOBJ)/$(GRP)/PGS_TSF_SetupGCT.o

#
# targets
#


all: 	message lib

lib: $(LIBOFILES)
	@$(PGSBIN)/mkpgslib $(GRP)

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(PGSOBJ)/$(GRP)/*.o

#
# compilation rules
#

# compile Toolkit functions (source code files to object files)
$(PGSOBJ)/$(GRP)/PGS_TSF_GetTSFMaster.o: PGS_TSF_GetTSFMaster.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_HandleLock.o: PGS_TSF_HandleLock.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupAA.o: PGS_TSF_SetupAA.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupCBP.o: PGS_TSF_SetupCBP.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupCSC.o: PGS_TSF_SetupCSC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupDEM.o: PGS_TSF_SetupDEM.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupSMF.o: PGS_TSF_SetupSMF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupIO.o: PGS_TSF_SetupIO.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupTD.o: PGS_TSF_SetupTD.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupEPH.o: PGS_TSF_SetupEPH.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupMET.o: PGS_TSF_SetupMET.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TSF_SetupGCT.o: PGS_TSF_SetupGCT.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rules to search for a specific make rule
# for a given input file (i.e. target)
.c:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@

.c.o:
	@$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
