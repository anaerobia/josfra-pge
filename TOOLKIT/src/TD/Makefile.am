#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit TD tool group
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

GRP = TD

# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libtd.la

# List source files here
libtd_la_SOURCES = \
	 PGS_TD_ADEOSIItoTAI.c \
     PGS_TD_ADEOSIItoUTC.c \
     PGS_TD_ASCIItime_AtoB.c \
     PGS_TD_ASCIItime_BtoA.c \
     PGS_TD_EOSAMtoTAI.c \
     PGS_TD_EOSAMtoUTC.c \
     PGS_TD_EOSPMtoTAI.c \
     PGS_TD_EOSPMGIIStoTAI.c \
     PGS_TD_EOSPMGIRDtoTAI.c \
     PGS_TD_EOSPMtoUTC.c \
     PGS_TD_EOSPMGIIStoUTC.c \
     PGS_TD_EOSPMGIRDtoUTC.c \
     PGS_TD_EOSAURAGIIStoTAI.c \
     PGS_TD_EOSAURAGIRDtoTAI.c \
     PGS_TD_EOSAURAtoUTC.c \
     PGS_TD_FGDCtoUTC.c \
     PGS_TD_GPStoUTC.c \
     PGS_TD_ISOinttoTAI.c \
     PGS_TD_ISOinttoUTCjd.c \
     PGS_TD_JDtoMJD.c \
     PGS_TD_JDtoTJD.c \
     PGS_TD_JulianDateSplit.c \
     PGS_TD_LeapSec.c \
     PGS_TD_MJDtoJD.c \
     PGS_TD_ManageTMDF.c \
     PGS_TD_ManageUTCF.c \
     PGS_TD_PB5CtoUTCjd.c \
     PGS_TD_PB5toTAI.c \
     PGS_TD_PB5toUTCjd.c \
     PGS_TD_SCtime_to_UTC.c \
     PGS_TD_TAIjdtoTAI.c \
     PGS_TD_TAIjdtoTDTjed.c \
     PGS_TD_TAIjdtoUTCjd.c \
     PGS_TD_TAItoGAST.c \
     PGS_TD_TAItoISOint.c \
     PGS_TD_TAItoTAIjd.c \
     PGS_TD_TAItoUDTF.c \
     PGS_TD_TAItoUT1jd.c \
     PGS_TD_TAItoUT1pole.c \
     PGS_TD_TAItoUTC.c \
     PGS_TD_TAItoUTCjd.c \
     PGS_TD_TDBjedtoTDTjed.c \
     PGS_TD_TDTjedtoTAIjd.c \
     PGS_TD_TDTjedtoTDBjed.c \
     PGS_TD_TJDtoJD.c \
     PGS_TD_TRMMtoTAI.c \
     PGS_TD_TRMMtoUTC.c \
     PGS_TD_TimeInterval.c \
     PGS_TD_UDTFtoTAI.c \
     PGS_TD_UDTFtoUTCjd.c \
     PGS_TD_UT1jdtoUTCjd.c \
     PGS_TD_UTC_to_SCtime.c \
     PGS_TD_UTCjdtoISOint.c \
     PGS_TD_UTCjdtoPB5.c \
     PGS_TD_UTCjdtoPB5C.c \
     PGS_TD_UTCjdtoTAIjd.c \
     PGS_TD_UTCjdtoUDTF.c \
     PGS_TD_UTCjdtoUT1jd.c \
     PGS_TD_UTCjdtoUTC.c \
     PGS_TD_UTCtoADEOSII.c \
     PGS_TD_UTCtoEOSAM.c \
     PGS_TD_UTCtoEOSPM.c \
     PGS_TD_UTCtoEOSPMGIIS.c \
     PGS_TD_UTCtoEOSPMGIRD.c \
     PGS_TD_UTCtoEOSAURAGIIS.c \
     PGS_TD_UTCtoEOSAURAGIRD.c \
     PGS_TD_UTCtoFGDC.c \
     PGS_TD_UTCtoGPS.c \
     PGS_TD_UTCtoTAI.c \
     PGS_TD_UTCtoTAIjd.c \
     PGS_TD_UTCtoTDBjed.c \
     PGS_TD_UTCtoTDTjed.c \
     PGS_TD_UTCtoTRMM.c \
     PGS_TD_UTCtoUT1.c \
     PGS_TD_UTCtoUT1jd.c \
     PGS_TD_UTCtoUTCjd.c \
     PGS_TD_bindFORTRAN.c \
     PGS_TD_calday.c \
     PGS_TD_gast.c \
     PGS_TD_gmst.c \
     PGS_TD_julday.c \
     PGS_TD_sortArrayIndices.c \
     PGS_TD_timeCheck.c
