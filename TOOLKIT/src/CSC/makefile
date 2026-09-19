#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:  makefile for PGS Toolkit Coordinate System Conversion (CSC) tools
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

GRP=CSC

include ../ecs.options
# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# define C preprocessor symbols 
DFLAGS = -D$(HDFSYS)

# path for #include directive
#IFLAG   = -I$(PGSINC) $(ADD_IFLAGS) 
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) $(ADD_IFLAGS)
# path for libraries linked
LFLAG   = -L$(PGSLIB) $(ADD_LFLAGS)

# libraries linked for C build
LIBS   = -lPGSTK  $(ADD_LIBS)  $(ECS_TOOLKIT_LIBS) -lpthread -lm
LIBS_HP= -lPGSTK  $(ADD_LIBS)  $(ECS_TOOLKIT_LIBS) -lm
# libraries linked for C build

# object files from this sub-group needed to build library
#
# !!! LIST ALL OBJECT FILES HERE NEEDED FOR THE GROUP LIBRARY !!!
#
LIBOFILES	=  \
     $(PGSOBJ)/$(GRP)/PGS_CSC_BorkowskiGeo.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_DayNight.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_ECItoECR.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_ECItoORB.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_ECItoSC.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_ECRtoECI.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_ECRtoGEO.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_EarthOccult.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_Earthpt_FixedFOV.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_Earthpt_FOV.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_EulerToQuat.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_FOVconicalHull.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_GEOtoECR.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_GeoCenToRect.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_GetEarthFigure.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_GetFOV_Pixel.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_GrazingRay.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_GreenwichHour.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_J2000toTOD.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_LookPoint.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_LookTwice.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_Norm.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_ORBtoECI.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_ORBtoSC.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_PointInFOVgeom.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_QuatToEuler.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_QuatToMatrix.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_RectToGeoCen.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_Rotate3or6.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_SCtoECI.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_SCtoORB.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_SpaceRefract.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_SubSatPoint.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_SubSatPointVel.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_TODtoJ2000.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_TiltYaw.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_UTC_UT1Pole.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_VecToVecAngle.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_ZenithAzimuth.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_bindFORTRAN.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_crossProduct.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_dotProduct.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_getECItoORBquat.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_getORBtoECIquat.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_getQuats.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_nutate2000.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_precs2000.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_quatMultiply.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_quatRotate.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_quickWahr.o \
     $(PGSOBJ)/$(GRP)/PGS_CSC_wahr2.o

#
# targets
#


all: 	message lib

lib: $(LIBOFILES)
	@$(PGSBIN)/mkpgslib $(GRP)

utilities: PGS_CSC_UT1_update update_utcpole update_utcpole_CC

PGS_CSC_UT1_update:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/PGS_CSC_UT1_update

update_utcpole:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@.sh

update_utcpole_CC:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@.sh

$(PGSBIN)/update_utcpole.sh: update_utcpole.sh
	cp $? $@; chmod 755 $@

$(PGSBIN)/update_utcpole_CC.sh: update_utcpole_CC.sh
	cp $? $@; chmod 755 $@

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(PGSOBJ)/$(GRP)/*.o


#
# compilation rules
#

# compile Toolkit binary utilities
$(PGSBIN)/PGS_CSC_UT1_update: PGS_CSC_UT1_update.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_CSC_UT1_update.c $(LIBS) $(NEWSTDLIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_CSC_UT1_update.c $(LIBS_HP) $(NEWSTDLIBS) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_CSC_UT1_update.c $(LIBS) $(NEWSTDLIBS) ; \
	fi ;

# compile Toolkit functions (source code files to object files)
$(PGSOBJ)/$(GRP)/PGS_CSC_BorkowskiGeo.o: PGS_CSC_BorkowskiGeo.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_DayNight.o: PGS_CSC_DayNight.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_ECItoECR.o: PGS_CSC_ECItoECR.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_ECItoORB.o: PGS_CSC_ECItoORB.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_ECItoSC.o: PGS_CSC_ECItoSC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_ECRtoECI.o: PGS_CSC_ECRtoECI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_ECRtoGEO.o: PGS_CSC_ECRtoGEO.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_EarthOccult.o: PGS_CSC_EarthOccult.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_Earthpt_FixedFOV.o: PGS_CSC_Earthpt_FixedFOV.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_Earthpt_FOV.o: PGS_CSC_Earthpt_FOV.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_EulerToQuat.o: PGS_CSC_EulerToQuat.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_FOVconicalHull.o: PGS_CSC_FOVconicalHull.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_GEOtoECR.o: PGS_CSC_GEOtoECR.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_GeoCenToRect.o: PGS_CSC_GeoCenToRect.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_GetEarthFigure.o: PGS_CSC_GetEarthFigure.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_GetFOV_Pixel.o: PGS_CSC_GetFOV_Pixel.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_GrazingRay.o: PGS_CSC_GrazingRay.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_GreenwichHour.o: PGS_CSC_GreenwichHour.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_J2000toTOD.o: PGS_CSC_J2000toTOD.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_LookTwice.o: PGS_CSC_LookTwice.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_LookPoint.o: PGS_CSC_LookPoint.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_Norm.o: PGS_CSC_Norm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_ORBtoECI.o: PGS_CSC_ORBtoECI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_ORBtoSC.o: PGS_CSC_ORBtoSC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_PointInFOVgeom.o: PGS_CSC_PointInFOVgeom.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_QuatToEuler.o: PGS_CSC_QuatToEuler.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_QuatToMatrix.o: PGS_CSC_QuatToMatrix.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_RectToGeoCen.o: PGS_CSC_RectToGeoCen.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_Rotate3or6.o: PGS_CSC_Rotate3or6.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_SCtoECI.o: PGS_CSC_SCtoECI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_SCtoORB.o: PGS_CSC_SCtoORB.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_SpaceRefract.o: PGS_CSC_SpaceRefract.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_SubSatPoint.o: PGS_CSC_SubSatPoint.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_SubSatPointVel.o: PGS_CSC_SubSatPointVel.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_TODtoJ2000.o: PGS_CSC_TODtoJ2000.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_TiltYaw.o: PGS_CSC_TiltYaw.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_UTC_UT1Pole.o: PGS_CSC_UTC_UT1Pole.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_VecToVecAngle.o: PGS_CSC_VecToVecAngle.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_ZenithAzimuth.o: PGS_CSC_ZenithAzimuth.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_bindFORTRAN.o: PGS_CSC_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_crossProduct.o: PGS_CSC_crossProduct.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_dotProduct.o: PGS_CSC_dotProduct.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_getECItoORBquat.o: PGS_CSC_getECItoORBquat.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_getORBtoECIquat.o: PGS_CSC_getORBtoECIquat.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_getQuats.o: PGS_CSC_getQuats.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_nutate2000.o: PGS_CSC_nutate2000.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_precs2000.o: PGS_CSC_precs2000.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_quatMultiply.o: PGS_CSC_quatMultiply.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_quatRotate.o: PGS_CSC_quatRotate.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_quickWahr.o: PGS_CSC_quickWahr.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_CSC_wahr2.o: PGS_CSC_wahr2.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rules to search for a specific make rule
# for a given input file (i.e. target)
.c:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@

.c.o:
	@$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
