#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for PGS Toolkit Time and Date tools (TD)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS
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
#       15-May-2002 XW   do not need -lpthread when HP
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

GRP=TD

include ../ecs.options

# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# define C preprocessor symbols 
DFLAGS = -D$(HDFSYS)

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) $(ADD_IFLAGS) 

# path for libraries linked
LFLAG   = -L$(PGSLIB) $(ADD_LFLAGS)

# libraries linked for C build
LIBS   = -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS) -lpthread -lm
LIBS_HP= -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS) -lm

# object files from this sub-group needed to build library
#
# !!! LIST ALL OBJECT FILES HERE NEEDED FOR THE GROUP LIBRARY !!!
#
LIBOFILES	=  \
     $(PGSOBJ)/$(GRP)/PGS_TD_ADEOSIItoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_ADEOSIItoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_ASCIItime_AtoB.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_ASCIItime_BtoA.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSAMtoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSAMtoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSPMtoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSPMGIIStoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSPMGIRDtoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSPMtoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSPMGIIStoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSPMGIRDtoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSAURAGIIStoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSAURAGIRDtoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_EOSAURAtoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_FGDCtoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_GPStoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_ISOinttoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_ISOinttoUTCjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_JDtoMJD.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_JDtoTJD.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_JulianDateSplit.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_LeapSec.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_MJDtoJD.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_ManageTMDF.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_ManageUTCF.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_PB5CtoUTCjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_PB5toTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_PB5toUTCjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_SCtime_to_UTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAIjdtoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAIjdtoTDTjed.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAIjdtoUTCjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAItoGAST.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAItoISOint.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAItoTAIjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAItoUDTF.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAItoUT1jd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAItoUT1pole.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAItoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TAItoUTCjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TDBjedtoTDTjed.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TDTjedtoTAIjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TDTjedtoTDBjed.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TJDtoJD.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TRMMtoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TRMMtoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_TimeInterval.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UDTFtoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UDTFtoUTCjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UT1jdtoUTCjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTC_to_SCtime.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoISOint.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoPB5.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoPB5C.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoTAIjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoUDTF.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoUT1jd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoUTC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoADEOSII.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSAM.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSPM.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSPMGIIS.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSPMGIRD.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSAURAGIIS.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSAURAGIRD.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoFGDC.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoGPS.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTAI.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTAIjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTDBjed.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTDTjed.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTRMM.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoUT1.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoUT1jd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_UTCtoUTCjd.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_bindFORTRAN.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_calday.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_gast.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_gmst.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_julday.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_sortArrayIndices.o \
     $(PGSOBJ)/$(GRP)/PGS_TD_timeCheck.o

#
# targets
#


all: 	message lib

lib: $(LIBOFILES)
	@$(PGSBIN)/mkpgslib $(GRP)

utilities: $(PGSBIN)/PGS_TD_NewLeap update_leapsec update_leapsec_CC

update_leapsec:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@.sh

update_leapsec_CC:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@.sh

$(PGSBIN)/update_leapsec.sh: update_leapsec.sh
	cp $? $@; chmod 755 $@

$(PGSBIN)/update_leapsec_CC.sh: update_leapsec_CC.sh
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
$(PGSBIN)/PGS_TD_NewLeap: PGS_TD_NewLeap.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_TD_NewLeap.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_TD_NewLeap.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_TD_NewLeap.c $(LIBS) ; \
	fi ;

# compile Toolkit functions (source code files to object files)
$(PGSOBJ)/$(GRP)/PGS_TD_ADEOSIItoTAI.o: PGS_TD_ADEOSIItoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_ADEOSIItoUTC.o: PGS_TD_ADEOSIItoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_ASCIItime_AtoB.o: PGS_TD_ASCIItime_AtoB.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_ASCIItime_BtoA.o: PGS_TD_ASCIItime_BtoA.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_EOSAMtoTAI.o: PGS_TD_EOSAMtoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_EOSAMtoUTC.o: PGS_TD_EOSAMtoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_EOSPMtoTAI.o: PGS_TD_EOSPMtoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_EOSPMGIIStoTAI.o: PGS_TD_EOSPMGIIStoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_EOSPMGIRDtoTAI.o: PGS_TD_EOSPMGIRDtoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_EOSPMtoUTC.o: PGS_TD_EOSPMtoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_EOSPMGIIStoUTC.o: PGS_TD_EOSPMGIIStoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_EOSPMGIRDtoUTC.o: PGS_TD_EOSPMGIRDtoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_EOSAURAGIIStoTAI.o: PGS_TD_EOSAURAGIIStoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@
 
$(PGSOBJ)/$(GRP)/PGS_TD_EOSAURAGIRDtoTAI.o: PGS_TD_EOSAURAGIRDtoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@
 
$(PGSOBJ)/$(GRP)/PGS_TD_EOSAURAtoUTC.o: PGS_TD_EOSAURAtoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_FGDCtoUTC.o: PGS_TD_FGDCtoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_GPStoUTC.o: PGS_TD_GPStoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_ISOinttoTAI.o: PGS_TD_ISOinttoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_ISOinttoUTCjd.o: PGS_TD_ISOinttoUTCjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_JDtoMJD.o: PGS_TD_JDtoMJD.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_JDtoTJD.o: PGS_TD_JDtoTJD.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_JulianDateSplit.o: PGS_TD_JulianDateSplit.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_LeapSec.o: PGS_TD_LeapSec.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_MJDtoJD.o: PGS_TD_MJDtoJD.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_ManageTMDF.o: PGS_TD_ManageTMDF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_ManageUTCF.o: PGS_TD_ManageUTCF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_PB5CtoUTCjd.o: PGS_TD_PB5CtoUTCjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_PB5toTAI.o: PGS_TD_PB5toTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_PB5toUTCjd.o: PGS_TD_PB5toUTCjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_SCtime_to_UTC.o: PGS_TD_SCtime_to_UTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAIjdtoTAI.o: PGS_TD_TAIjdtoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAIjdtoTDTjed.o: PGS_TD_TAIjdtoTDTjed.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAIjdtoUTCjd.o: PGS_TD_TAIjdtoUTCjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAItoGAST.o: PGS_TD_TAItoGAST.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAItoISOint.o: PGS_TD_TAItoISOint.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAItoTAIjd.o: PGS_TD_TAItoTAIjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAItoUDTF.o: PGS_TD_TAItoUDTF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAItoUT1jd.o: PGS_TD_TAItoUT1jd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAItoUT1pole.o: PGS_TD_TAItoUT1pole.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAItoUTC.o: PGS_TD_TAItoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TAItoUTCjd.o: PGS_TD_TAItoUTCjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TDBjedtoTDTjed.o: PGS_TD_TDBjedtoTDTjed.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TDTjedtoTAIjd.o: PGS_TD_TDTjedtoTAIjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TDTjedtoTDBjed.o: PGS_TD_TDTjedtoTDBjed.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TJDtoJD.o: PGS_TD_TJDtoJD.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TRMMtoTAI.o: PGS_TD_TRMMtoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TRMMtoUTC.o: PGS_TD_TRMMtoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_TimeInterval.o: PGS_TD_TimeInterval.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UDTFtoTAI.o: PGS_TD_UDTFtoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UDTFtoUTCjd.o: PGS_TD_UDTFtoUTCjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UT1jdtoUTCjd.o: PGS_TD_UT1jdtoUTCjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTC_to_SCtime.o: PGS_TD_UTC_to_SCtime.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoISOint.o: PGS_TD_UTCjdtoISOint.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoPB5.o: PGS_TD_UTCjdtoPB5.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoPB5C.o: PGS_TD_UTCjdtoPB5C.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoTAIjd.o: PGS_TD_UTCjdtoTAIjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoUDTF.o: PGS_TD_UTCjdtoUDTF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoUT1jd.o: PGS_TD_UTCjdtoUT1jd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCjdtoUTC.o: PGS_TD_UTCjdtoUTC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoADEOSII.o: PGS_TD_UTCtoADEOSII.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSAM.o: PGS_TD_UTCtoEOSAM.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSPM.o: PGS_TD_UTCtoEOSPM.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSPMGIIS.o: PGS_TD_UTCtoEOSPMGIIS.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSPMGIRD.o: PGS_TD_UTCtoEOSPMGIRD.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSAURAGIIS.o: PGS_TD_UTCtoEOSAURAGIIS.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@
 
$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoEOSAURAGIRD.o: PGS_TD_UTCtoEOSAURAGIRD.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@
 
$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoFGDC.o: PGS_TD_UTCtoFGDC.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoGPS.o: PGS_TD_UTCtoGPS.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTAI.o: PGS_TD_UTCtoTAI.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTAIjd.o: PGS_TD_UTCtoTAIjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTDBjed.o: PGS_TD_UTCtoTDBjed.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTDTjed.o: PGS_TD_UTCtoTDTjed.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoTRMM.o: PGS_TD_UTCtoTRMM.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoUT1.o: PGS_TD_UTCtoUT1.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoUT1jd.o: PGS_TD_UTCtoUT1jd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_UTCtoUTCjd.o: PGS_TD_UTCtoUTCjd.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_bindFORTRAN.o: PGS_TD_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_calday.o: PGS_TD_calday.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_gast.o: PGS_TD_gast.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_gmst.o: PGS_TD_gmst.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_julday.o: PGS_TD_julday.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_sortArrayIndices.o: PGS_TD_sortArrayIndices.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_TD_timeCheck.o: PGS_TD_timeCheck.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rules to search for a specific make rule
# for a given input file (i.e. target)
.c:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@

.c.o:
	@$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
