#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit DEM tool group
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

GRP = DEM

# force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libdem.la

# List source files here
libdem_la_SOURCES = \
	PGS_DEM_Subset.c \
        PGS_DEM_Lookup.c \
	PGS_DEM_OrderSubset.c \
        PGS_DEM_Populate.c \
	PGS_DEM_WriteSubgridCalculator.c \
	PGS_DEM_RecursiveSearchPix.c \
	PGS_DEM_RecursiveSearchDeg.c \
	PGS_DEM_RecursiveSearchBil.c \
	PGS_DEM_OrderIndicesSumPix.c \
	PGS_DEM_OrderIndicesSumDeg.c \
	PGS_DEM_Open.c \
	PGS_DEM_Close.c \
	PGS_DEM_ExtentRegion.c \
	PGS_DEM_ExtractRegion.c \
	PGS_DEM_ReplaceFillPoints.c \
	PGS_DEM_ReplaceFillPointsInt8.c \
	PGS_DEM_ReplaceFillPointsInt16.c \
	PGS_DEM_ReplaceFillPointsFlt32.c \
	PGS_DEM_GetPoint.c \
	PGS_DEM_GetRegion.c \
	PGS_DEM_GetSize.c \
	PGS_DEM_GetMetadata.c \
	PGS_DEM_GetQualityData.c \
	PGS_DEM_GetBoundingPnts.c \
	PGS_DEM_DataPresent.c \
	PGS_DEM_SortModels.c \
	PGS_DEM_AccessFile.c \
	PGS_DEM_Interpolate.c \
        PGS_DEM_bindFORTRAN.c
