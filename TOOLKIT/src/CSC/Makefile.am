#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit CSC tool group
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

GRP = CSC

# force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# CSC library files are created here
# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libcsc.la

# List source files here
libcsc_la_SOURCES = \
	PGS_CSC_BorkowskiGeo.c \
	PGS_CSC_DayNight.c \
	PGS_CSC_ECItoECR.c \
	PGS_CSC_ECItoORB.c \
	PGS_CSC_ECItoSC.c \
	PGS_CSC_ECRtoECI.c \
	PGS_CSC_ECRtoGEO.c \
	PGS_CSC_EarthOccult.c \
	PGS_CSC_Earthpt_FixedFOV.c \
	PGS_CSC_Earthpt_FOV.c \
	PGS_CSC_EulerToQuat.c \
	PGS_CSC_FOVconicalHull.c \
	PGS_CSC_GEOtoECR.c \
	PGS_CSC_GeoCenToRect.c \
	PGS_CSC_GetEarthFigure.c \
	PGS_CSC_GetFOV_Pixel.c \
	PGS_CSC_GrazingRay.c \
	PGS_CSC_GreenwichHour.c \
	PGS_CSC_J2000toTOD.c \
	PGS_CSC_LookPoint.c \
	PGS_CSC_LookTwice.c \
	PGS_CSC_Norm.c \
	PGS_CSC_ORBtoECI.c \
	PGS_CSC_ORBtoSC.c \
	PGS_CSC_PointInFOVgeom.c \
	PGS_CSC_QuatToEuler.c \
	PGS_CSC_QuatToMatrix.c \
	PGS_CSC_RectToGeoCen.c \
	PGS_CSC_Rotate3or6.c \
	PGS_CSC_SCtoECI.c \
	PGS_CSC_SCtoORB.c \
	PGS_CSC_SpaceRefract.c \
	PGS_CSC_SubSatPoint.c \
	PGS_CSC_SubSatPointVel.c \
	PGS_CSC_TODtoJ2000.c \
	PGS_CSC_TiltYaw.c \
	PGS_CSC_UTC_UT1Pole.c \
	PGS_CSC_VecToVecAngle.c \
	PGS_CSC_ZenithAzimuth.c \
	PGS_CSC_bindFORTRAN.c \
	PGS_CSC_crossProduct.c \
	PGS_CSC_dotProduct.c \
	PGS_CSC_getECItoORBquat.c \
	PGS_CSC_getORBtoECIquat.c \
	PGS_CSC_getQuats.c \
	PGS_CSC_nutate2000.c \
	PGS_CSC_precs2000.c \
	PGS_CSC_quatMultiply.c \
	PGS_CSC_quatRotate.c \
	PGS_CSC_quickWahr.c \
	PGS_CSC_wahr2.c

