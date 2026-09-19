#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for EPH tool group
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS F77 F77FLAGS
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC
#  
# author:  Mike Sucher / Applied Research Corporation 
#          Guru Tej S. Khalsa / Applied Research Corporation
#          Xin Wang / EIT Inc.
#
# history:
#	13-Apr-1994 MES  Template version
#	18-Apr-1994 MES  Pass all PGS env variables, and 
# 		 	 call mkpgslib with $PGSHOME override
#	01-Sep-1994 MES  Modified C and FORTRAN libs to use the 
# 			 Release 3 library libPGSTK.a
# 	08-Sep-1994 MES  Clean up target all - remove $(SUBGROUPS) from
# 			 target list for recursive call to 'make'.
#	03-Nov-1995 GTSK - Added $(ADD_IFLAGS) to IFLAG definition
# 			 - Added $(ADD_LFLAGS) to LFLAG definition
# 			 These allow additional directories in the include 
# 			 and lib paths by setting environment variable.
# 			 - Added $(ADD_LIBS) to LIBS definition which allows
# 			 additional libraries to be added by setting an 
# 			 environment variable.
#       10-Dec-2001  XW  Provided the actual attitude data upon request.
#       10-Feb-2002  XW  Got ephemeris data from HDF files.
#	08-Jan-2003  AA  Modified to include gbadsim
#
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE).  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) Target executable files are moved to $PGSBIN.
# 	3) Target object files are moved to $(PGSOBJ)/$(GRP).
# 	4) This file does NOT build the library for this group.
#	   That is handled by the group-level makefile.
#
#----------------------------------------------------------------------------

#
# set the name of this tool group
#

GRP = EPH
include ../ecs.options
# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM = /bin/rm
RMFLAGS = -f 

# define C preprocessor symbols 
DFLAGS = -D$(HDFSYS)

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) $(ADD_IFLAGS) -I$(HDFINC) -I$(ZLIBINC) -I$(JPEGINC) -I$(HDF5INC)
#IFLAG = -I$(PGSINC) $(ADD_IFLAGS) 

# path for libraries linked (compiler will search in the order listed)
LFLAG = -L$(PGSLIB) $(ADD_LFLAGS) -L$(HDFLIB) -L$(ZLIBLIB) -L$(JPEGLIB) -L$(HDF5LIB)

# libraries linked for C build
LIBS = -lPGSTK $(ADD_LIBS) -lmfhdf -ldf -ljpeg $(HDF5LIB)/libhdf5.a -lz -lm

# for building orbsim
TARG = all

#
# object files from this sub-group needed to build library
#

C_LIBOFILES = \
	$(PGSOBJ)/$(GRP)/PGS_EPH_EphemAttit.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_EphAtt_unInterpolate.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_UnInterpEphAtt.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_GetEphMet.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_GetSpacecraftData.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_ManageMasks.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_bindFORTRAN.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_getEphemHeaders.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_getAttitHeaders.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_getEphemRecords.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_getAttitRecords.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_getToken.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_fileHandlingStuff.o \
        $(PGSOBJ)/$(GRP)/PGS_EPH_interpolateAttitude.o \
        $(PGSOBJ)/$(GRP)/PGS_EPH_interpolatePosVel.o

F_LIBOFILES = \
	$(PGSOBJ)/$(GRP)/PGS_EPH_GetEphMetF.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_UnInterpEphAttF.o \
	$(PGSOBJ)/$(GRP)/PGS_EPH_EphAtt_unInterpolateF.o

FINCFILES = PGS_SMF.f PGS_TD.f PGS_TD_3.f PGS_EPH_5.f PGS_MEM_7.f
SUBGROUPS = orbsim gbadsim

#
# targets
#

all:	message
	@case "$(F77)" in		\
	    "none"|"")			\
	        echo "No FORTRAN compiler - the FORTRAN version of PGS_EPH_GetEphMet() will NOT be built." ; \
	        ftn_targ='' ;		\
	        ;; 			\
	    *)				\
	        ftn_targ=f_obj ;	\
	        ;;			\
	esac ;				\
	$(MAKE) $(MFLAGS) c_obj $$ftn_targ lib

lib:
	@$(PGSBIN)/mkpgslib $(GRP)

c_obj:	$(C_LIBOFILES)

f_obj:	finc $(F_LIBOFILES) dfinc 

finc: $(FINCFILES)

utilities: orbsim gbadsim

PGS_SMF.f: $(PGSINC)/PGS_SMF.f
	cp $? .

PGS_TD.f: $(PGSINC)/PGS_TD.f
	cp $? .

PGS_TD_3.f: $(PGSINC)/PGS_TD_3.f
	cp $? .

PGS_EPH_5.f: $(PGSINC)/PGS_EPH_5.f
	cp $? .

PGS_MEM_7.f: $(PGSINC)/PGS_MEM_7.f
	cp $? .

dfinc:
	$(RM) $(RMFLAGS) $(FINCFILES)

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(FINCFILES)
	$(RM) $(RMFLAGS) $(PGSOBJ)/$(GRP)/*.o
	@$(MAKE) $(MFLAGS) orbsim TARG=clean
	@$(MAKE) $(MFLAGS) gbadsim TARG=clean

orbsim: force_orbsim

# for backward compatibility
orb: force_orbsim

force_orbsim:
	@echo ""; cd orbsim; echo Making \`$(TARG)\' in ./orbsim; \
              $(MAKE) $(MFLAGS) $(TARG)

gbadsim: force_gbadsim

gbad: force_gbadsim

force_gbadsim:
	@echo ""; cd gbadsim; echo Making \`$(TARG)\' in ./gbadsim; \
              $(MAKE) $(MFLAGS) $(TARG)

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/PGS_EPH_EphemAttit.o: PGS_EPH_EphemAttit.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_EphAtt_unInterpolate.o: PGS_EPH_EphAtt_unInterpolate.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_UnInterpEphAtt.o: PGS_EPH_UnInterpEphAtt.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_GetEphMet.o: PGS_EPH_GetEphMet.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_GetSpacecraftData.o: PGS_EPH_GetSpacecraftData.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_ManageMasks.o: PGS_EPH_ManageMasks.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_bindFORTRAN.o: PGS_EPH_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_getEphemHeaders.o: PGS_EPH_getEphemHeaders.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_getAttitHeaders.o: PGS_EPH_getAttitHeaders.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_getEphemRecords.o: PGS_EPH_getEphemRecords.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_getAttitRecords.o: PGS_EPH_getAttitRecords.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_getToken.o: PGS_EPH_getToken.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_fileHandlingStuff.o: PGS_EPH_fileHandlingStuff.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_interpolateAttitude.o: PGS_EPH_interpolateAttitude.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_interpolatePosVel.o: PGS_EPH_interpolatePosVel.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_GetEphMetF.o: PGS_EPH_GetEphMetF.f
	$(F77) -c $(F77FLAGS) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_UnInterpEphAttF.o: PGS_EPH_UnInterpEphAttF.f
	$(F77) -c $(F77FLAGS) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_EPH_EphAtt_unInterpolateF.o: PGS_EPH_EphAtt_unInterpolateF.f
	$(F77) -c $(F77FLAGS) $? -o $@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.c.o:
	@$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.f.o:
	@$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@

