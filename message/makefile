#----------------------------------------------------------------------------
# file:		Makefile for SMF files
#
# environment:	MPPDE, machine-independent 
# 	
# environment variables dependencies:
#   compiler:  smfcompile
#   includes:  PGSINC 
#   messages:  PGSMSG
#   other:     PGSBIN PGSSRC
# 
# author:  David P. Heroux
#          Mike Sucher
#          Guru Tej S. Khalsa
#
# history:
#	18-Apr-1994 DPH Created target 'all'
#	21-Apr-1994 MES Cleaned up output, split 'all' into sub-targets
# 	15-Sep-1994 MES Patched target smffiles to work on all platforms,
# 			including SunOS 4.x
#       13-Aug-1997 GSK Added individual targets for each SMF file
#			to allow 'make' dependencies to work
#	10-Oct-1997 GSK Expanded targets to include $(PGSMSG) and
#			defined the smfcompile command and flags as Makefile
#			variables.  These changes were made to allow the
#			ECS CM team to make use of this file.  This should
#			NOT be changed with out looking at the file
#			$(PGSHOME)/Makefile and seeing how any change will
#			affect the ability of CM to use this file.
#
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) To build the PGS SMF files, type 'make all'
#----------------------------------------------------------------------------
# Define sh shell
SHELL=/bin/sh

SC=$(PGSBIN)/smfcompile
SCFLAGS=-i
SF77FLAGS=-f77 -i

SMFFILES = 			\
	$(PGSMSG)/PGS_1		\
	$(PGSMSG)/PGS_3		\
	$(PGSMSG)/PGS_4		\
	$(PGSMSG)/PGS_5		\
	$(PGSMSG)/PGS_6		\
	$(PGSMSG)/PGS_7		\
	$(PGSMSG)/PGS_9		\
	$(PGSMSG)/PGS_10	\
	$(PGSMSG)/PGS_11	\
	$(PGSMSG)/PGS_12	\
	$(PGSMSG)/PGS_13	\
	$(PGSMSG)/PGS_14	\
	$(PGSMSG)/PGS_15

all: message smffiles

smffiles: $(SMFFILES)

message:
	@echo "Setting up SMF include files ..."

$(PGSMSG)/PGS_1: PGS_IO_1.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_2:
	@echo SMF seed value "2" is reserved for internal SMF tool usage

$(PGSMSG)/PGS_3: PGS_TD_3.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_4: PGS_CSC_4.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_5: PGS_EPH_5.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_6: PGS_CBP_6.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_7: PGS_MEM_7.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_9: PGS_PC_9.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_10: PGS_AA_10.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_11: PGS_CUC_11.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_12: PGS_GCT_12.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_13: PGS_MET_13.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_14: PGS_DEM_14.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

$(PGSMSG)/PGS_15: PGS_TSF_15.t
	$(SC) -f $? $(SCFLAGS)
	$(SC) -f $? $(SF77FLAGS)

buildsmf:
	@echo "Building the smfcompile utility ..."
	@cd $(PGSSRC)/SMF; $(MAKE) $(MFLAGS) smfcompile

checkout:
	@if [ "$(CLEARCASE_ROOT)" != "" ] ; then			    \
		cleartool co -nc PGS_[1-9] PGS_[1-9][0-9];		    \
		cleartool co -nc $(PGSINC)/PGS_[A-Z][A-Z]_[0-9]*.[hf];	    \
		cleartool co -nc $(PGSINC)/PGS_[A-Z][A-Z][A-Z]_[0-9]*.[hf]; \
	fi
