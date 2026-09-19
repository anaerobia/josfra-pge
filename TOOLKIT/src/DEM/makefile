#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:  makefile for PGS Toolkit Digital Elevation Model tools (DEM)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS
#   includes:  PGSINC HDFINC HDFEOS_INC
#   other:     PGSOBJ PGSLIB PGSBIN
#  
# author:  Mike Sucher / Applied Research Corporation
#          Guru Tej S. Khalsa / Applied Research Corporation
#          Alexis Zubrow / Applied Research Corporation
#
# history:
#	20-Mar-1997 MES/GTSK/AZ  Initial version
#       15-Apr-1997 AZ           Added PGS_DEM_GetMetadata
#	24-Jun-1997 AZ		 Added PGS_DEM_GetQualityData
#	17-Jul-1997 AZ           Added PGS_DEM_RecursiveSearchBil, 
#				       PGS_DEM_AccessFile,  
#				       PGS_DEM_Interpolate, and
#				       PGS_DEM_GetBoundingPnts	 
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
# 	3) This file builds the library for this group.
#
#----------------------------------------------------------------------------

#
# set the name of this tool group
#

GRP=DEM

include ../ecs.options
# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 
TOUCH=`which touch`

# define C preprocessor symbols 
DFLAGS  = -D$(HDFSYS)

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(HDFINC) -I$(HDF5INC) -I$(HDFEOS_INC) $(ADD_IFLAGS) 

# object files from this sub-group needed to build library
#
# !!! LIST ALL OBJECT FILES HERE NEEDED FOR THE GROUP LIBRARY !!!
#
LIBOFILES = \
        $(PGSOBJ)/$(GRP)/PGS_DEM_Subset.o \
        $(PGSOBJ)/$(GRP)/PGS_DEM_Lookup.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_OrderSubset.o \
        $(PGSOBJ)/$(GRP)/PGS_DEM_Populate.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_WriteSubgridCalculator.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_RecursiveSearchPix.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_RecursiveSearchDeg.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_RecursiveSearchBil.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_OrderIndicesSumPix.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_OrderIndicesSumDeg.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_Open.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_Close.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_ExtentRegion.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_ExtractRegion.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_ReplaceFillPoints.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_ReplaceFillPointsInt8.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_ReplaceFillPointsInt16.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_ReplaceFillPointsFlt32.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_GetPoint.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_GetRegion.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_GetSize.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_GetMetadata.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_GetQualityData.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_GetBoundingPnts.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_DataPresent.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_SortModels.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_AccessFile.o \
	$(PGSOBJ)/$(GRP)/PGS_DEM_Interpolate.o \
        $(PGSOBJ)/$(GRP)/PGS_DEM_bindFORTRAN.o 



all: message $(PGSLIB)/libPGSTK.a

$(PGSLIB)/libPGSTK.a: $(LIBOFILES) lib

lib:
	@$(PGSBIN)/mkpgslib $(GRP)

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(PGSOBJ)/$(GRP)/*.o

message:
	@echo "    Makefile for group: $(GRP); Target: all"

#
# compilation rules
#

# compile Toolkit functions (source code files to object files)
$(PGSOBJ)/$(GRP)/PGS_DEM_Subset.o: PGS_DEM_Subset.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_Lookup.o: PGS_DEM_Lookup.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_OrderSubset.o: PGS_DEM_OrderSubset.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_Populate.o: PGS_DEM_Populate.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_WriteSubgridCalculator.o: PGS_DEM_WriteSubgridCalculator.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_RecursiveSearchPix.o: PGS_DEM_RecursiveSearchPix.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_RecursiveSearchDeg.o: PGS_DEM_RecursiveSearchDeg.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_RecursiveSearchBil.o: PGS_DEM_RecursiveSearchBil.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_OrderIndicesSumPix.o: PGS_DEM_OrderIndicesSumPix.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_OrderIndicesSumDeg.o: PGS_DEM_OrderIndicesSumDeg.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_Open.o: PGS_DEM_Open.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_Close.o: PGS_DEM_Close.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_ExtentRegion.o: PGS_DEM_ExtentRegion.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_ExtractRegion.o: PGS_DEM_ExtractRegion.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_ReplaceFillPoints.o: PGS_DEM_ReplaceFillPoints.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_ReplaceFillPointsInt8.o: PGS_DEM_ReplaceFillPointsInt8.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_ReplaceFillPointsInt16.o: PGS_DEM_ReplaceFillPointsInt16.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_ReplaceFillPointsFlt32.o: PGS_DEM_ReplaceFillPointsFlt32.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_GetPoint.o: PGS_DEM_GetPoint.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_GetRegion.o: PGS_DEM_GetRegion.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_GetSize.o: PGS_DEM_GetSize.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_GetMetadata.o: PGS_DEM_GetMetadata.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_GetQualityData.o: PGS_DEM_GetQualityData.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_GetBoundingPnts.o: PGS_DEM_GetBoundingPnts.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_DataPresent.o: PGS_DEM_DataPresent.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_SortModels.o: PGS_DEM_SortModels.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_AccessFile.o: PGS_DEM_AccessFile.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_Interpolate.o: PGS_DEM_Interpolate.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_DEM_bindFORTRAN.o: PGS_DEM_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

