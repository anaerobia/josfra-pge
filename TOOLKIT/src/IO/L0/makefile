#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for Level 0 IO tools 
#		(subgroup of the PGS Toolkit IO group)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS F77 F77_CFH
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC HDFSYS
#  
# author:  Mike Sucher / Applied Research Corporation
#	   Tom W. Atwater / Applied Research Corporation 
#          Guru Tej S. Khalsa / Applied Research Corporation
#          Xin Wang / EIT Inc.
#
# history:
#	18-Nov-1994 MES  Initial version (from Generic IO makefile).
#	12-Dec-1994 MES  Updated list of object files.
#	30-Jan-1995 TWA  Added function PGS_IO_L0_TRMM_HeaderInfo
#	03-Nov-1995 GTSK - Added $(ADD_IFLAGS) to IFLAG definition
# 			 - Added $(ADD_LFLAGS) to LFLAG definition
# 			 These allow additional directories in the include 
# 			 and lib paths by setting environment variable.
# 			 - Added $(ADD_LIBS) to LIBS definition which allows
# 			 additional libraries to be added by setting an 
# 			 environment variable.
#       01-Nov-2001 XW   Modified to support AURA spacecraft
#
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) Target executable files are moved to $PGSBIN
# 	3) Target object files are moved to $(PGSOBJ)/IO
# 	4) To build the PGS IO tools library, type 'make libIO.a'
#	   (This runs the script $PGSBIN/mkpgslib to build the library).
#----------------------------------------------------------------------------

#
# set the name of this tool group
#
GRP = IO

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Default target
TARG=all

# name of remove utility
RM = /bin/rm
RMFLAGS = -f

# define C preprocessor symbols 
DFLAGS  = -D$(HDFSYS)

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC)  $(ADD_IFLAGS)

# path for libraries linked (compiler will search in the order listed)
LFLAG	= -L$(PGSLIB) $(ADD_LFLAGS)

# libraries linked for C build
LIBS   = -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS) -lpthread


# object files from this sub-group needed to build library

LIBOFILES	=  					\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_BYTEtoINT.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_Close.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_FileVersionInfo.o 	\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_GetEOSAMfileTimes.o 	\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_GetEOSPMGIISfileTimes.o \
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_GetEOSPMGIRDfileTimes.o \
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_GetEOSAURAfileTimes.o  \
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_GetHeader.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_GetPacket.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_ManageTable.o 	\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_MapVersions.o 	\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_NextPhysical.o 	\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_Open.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_SeekPacket.o 	\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_SetStart.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_SetStartCntPkts.o	\
        $(PGSOBJ)/$(GRP)/PGS_IO_L0_TRMM_HdrInfo.o 	\
	$(PGSOBJ)/$(GRP)/PGS_IO_L0_VersionInfoCheck.o 	\
	$(PGSOBJ)/$(GRP)/PGS_IO_L0_bindFORTRAN_L0.o

#
# targets
#

all: 	message $(LIBOFILES)

lib:
	${PGSBIN}/mkpgslib $(GRP)

# dummy target to force make
makeme:

utilities: L0sim

L0sim:	makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(MFLAGS) $(TARG)

message:
	@echo "    Makefile for subgroup: L0; Target: all"

clean:
	$(RM) $(RMFLAGS) *.o;
	$(RM) $(RMFLAGS) $(LIBOFILES)
	@$(MAKE) $(MFLAGS) L0sim TARG=$@

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/PGS_IO_L0_BYTEtoINT.o: PGS_IO_L0_BYTEtoINT.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_Close.o: PGS_IO_L0_Close.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_FileVersionInfo.o: PGS_IO_L0_FileVersionInfo.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_GetEOSAMfileTimes.o: PGS_IO_L0_GetEOSAMfileTimes.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_GetEOSPMGIISfileTimes.o: PGS_IO_L0_GetEOSPMGIISfileTimes.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_GetEOSPMGIRDfileTimes.o: PGS_IO_L0_GetEOSPMGIRDfileTimes.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_GetEOSAURAfileTimes.o: PGS_IO_L0_GetEOSAURAfileTimes.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_GetHeader.o: PGS_IO_L0_GetHeader.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_GetPacket.o: PGS_IO_L0_GetPacket.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_ManageTable.o: PGS_IO_L0_ManageTable.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_MapVersions.o: PGS_IO_L0_MapVersions.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_NextPhysical.o: PGS_IO_L0_NextPhysical.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_Open.o: PGS_IO_L0_Open.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_SeekPacket.o: PGS_IO_L0_SeekPacket.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_SetStart.o: PGS_IO_L0_SetStart.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_SetStartCntPkts.o: PGS_IO_L0_SetStartCntPkts.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_TRMM_HdrInfo.o: PGS_IO_L0_TRMM_HdrInfo.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_VersionInfoCheck.o: PGS_IO_L0_VersionInfoCheck.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_bindFORTRAN_L0.o: PGS_IO_L0_bindFORTRAN_L0.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
