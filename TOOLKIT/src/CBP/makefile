
#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:  makefile for PGS Toolkit Celestial Body Positioning tools (CBP)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFLAGS C_CFH
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSOBJ PGSBIN
#  
# author:  Mike Sucher / Applied Research Corporation
#          Guru Tej S. Khalsa / Applied Research Corporation
#
# history:
#	13-Apr-1994 MES  Template version
#	18-Apr-1994 MES  Pass all PGS env variables, and 
# 			 call mkpgslib with $PGSHOME override
#       04-Aug-1994 GTSK Specialized for building libPGSTK.a
#                        single library.  No longer builds executables.
#       02-Aug-1994 MES  Renamed target 'asctobin' to 'ephtobin'.   
# 			 Modified to build executable of same name as target.
#	03-Nov-1995 GTSK - Added $(ADD_IFLAGS) to IFLAG definition
# 			 - Added $(ADD_LFLAGS) to LFLAG definition
# 			 These allow additional directories in the include 
# 			 and lib paths by setting environment variable.
# 			 - Added $(ADD_LIBS) to LIBS definition which allows
# 			 additional libraries to be added by setting an 
# 			 environment variable.
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

GRP=CBP

include ../ecs.options

# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# define C preprocessor symbols 
DFLAGS  = -D$(HDFSYS)

# path for #include directive
#IFLAG   = -I$(PGSINC) $(ADD_IFLAGS) 
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) $(ADD_IFLAGS)

# path for libraries linked
LFLAG   = -L$(PGSLIB) $(ADD_LFLAGS)

# libraries linked for C build
LIBS   = -lPGSTK $(ADD_LIBS) -lm

# object files from this sub-group needed to build library
#
# !!! LIST ALL OBJECT FILES HERE NEEDED FOR THE GROUP LIBRARY !!!
#
LIBOFILES	=  \
     $(PGSOBJ)/$(GRP)/PGS_CBP_EphemRead.o \
     $(PGSOBJ)/$(GRP)/PGS_CBP_Earth_CB_Vector.o \
     $(PGSOBJ)/$(GRP)/PGS_CBP_Sat_CB_Vector.o \
     $(PGSOBJ)/$(GRP)/PGS_CBP_SolarTimeCoords.o \
     $(PGSOBJ)/$(GRP)/PGS_CBP_bindFORTRAN.o \
     $(PGSOBJ)/$(GRP)/PGS_CBP_body_inFOV.o

#
# targets
#


all: 	message $(PGSLIB)/libPGSTK.a

$(PGSLIB)/libPGSTK.a: $(LIBOFILES) lib

lib:
	@$(PGSBIN)/mkpgslib $(GRP)

# build the ephtobin utility
# note: we don't use the .c rule because we're not linking to libPGSTK

utilities: ephtobin

ephtobin:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/ephtobin

$(PGSBIN)/ephtobin: PGS_CBP_ASCtoBIN.c
	$(CC) $(CFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(PGSOBJ)/$(GRP)/*.o

#
# compilation rules
#

# compile C to executable
.c:
	$(MAKE) $(MFLAGS) $(PGSBIN)/$@

# compile Toolkit functions (source code files to object files)
$(PGSOBJ)/$(GRP)/PGS_CBP_EphemRead.o: PGS_CBP_EphemRead.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CBP_Earth_CB_Vector.o: PGS_CBP_Earth_CB_Vector.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CBP_Sat_CB_Vector.o: PGS_CBP_Sat_CB_Vector.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CBP_SolarTimeCoords.o: PGS_CBP_SolarTimeCoords.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CBP_bindFORTRAN.o: PGS_CBP_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CBP_body_inFOV.o: PGS_CBP_body_inFOV.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
