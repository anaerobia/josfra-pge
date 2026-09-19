#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for PC tool group
#
# environment:	MPPDE, machine-independent 
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS
#   includes:  PGSINC 
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC
# 
# author:  David P. Heroux / Applied Research Corporation
#	   Michael E. Sucher / Applied Research Corporation
#	   Ray Milburn / Applied Research Corporation
#          Guru Tej S. Khalsa / Applied Research Corporation
#
# history:
#	01-Apr-1994 MES Initial version
#	18-Apr-1994 DPH created target 'all'
#	30-Aug-1994 RM  added new files
#	01-Sep-1994 MES Modified C and FORTRAN libs to use the 
# 			Release 3 library libPGSTK.a
#	25-Oct-1994 DPH Added target 'clean' & removed targets for 'Put' & 
#			'Get' drivers. Also removed rules for C/Fortran 
#			executables.
#	18-Dec-1994 RM  Added new files for TK4 and added targets for 
#			PGS_PC_InitCom and PGS_PC_TermCom.
#	02-Jan-1995 DPH Added target 'commands' to build all command utilities.
#	06-Feb-1995 MES Remove 'commands' from dependency  list for 'all'.
#	05-Apr-1995 RM  Added new filesc for TK5.  Functionality to move the
#			default file locations from environment variables to
#			the PCF.
#	11-Apr-1995 RM  Added new file PGS_PC_GetReferenceType.o
#	20-Apr-1995 RM  Added new files for new marking RUNTIME functionality.
#			New files are PGS_PC_CheckFlags.o, 
#			PGS_PC_MarkRuntimeAscii.o, PGS_PC_MarkRuntimeShm.o,
#			and PGS_PC_MultiRuntimes.o.
#	30-Jun-1995 MES Revised to use a single .c rule to compile all of the 
# 			utilities.  Also check the platform type and disable 
#			the -DSHMMEM (shared memory) flag, if compilation is 
# 			on the Cray. 
#	03-Jul-1995 MES Removed the move to object dir command from .c.o rule. 
# 			Revised target 'pc' to use $LIBOFILES, renamed to 'pc_obj'.
# 			Updated target 'pctcheck' to use the new build rules.
#	07-Jul-1995 RM  Added new file PGS_PC_GetFileByAttrF.c for DR ECSed00932.
#	03-Nov-1995 GSK - Added $(ADD_IFLAGS) to IFLAG definition
# 			- Added $(ADD_LFLAGS) to LFLAG definition
# 			These allow additional directories in the include 
# 			and lib paths by setting environment variable.
# 			- Added $(ADD_LIBS) to LIBS definition which allows
# 			additional libraries to be added by setting an 
# 			environment variable.
#	15-Dec-1995 RM  Updated for TK6.  Added new file PGS_PC_GetPCFTemp.c.
#			This allows the Process Control Tools to works across
#			filesystems.
#	05-Jan-1996 RM 	Updated for TK6.  Added Universal References to
#			file entries for files.  Added new file 
#			PGS_PC_GetUniversalRef.c.
#	17-Jan-1996 GSK	Added PGS_PC_bindFORTRAN.o to list of object files
#			defined by LIBOFILES.
#       05-Mar-1997 CST Added new object file PGS_PC_PGS_PC_GetFileSize.o and 
#                       new command file PGS_PC_PGS_PC_GetFileSizeCom.
#       15-May-2002 XW  do not need -lpthread when HP
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) Target executable files are moved to $PGSBIN
# 	3) Target object files are moved to $(PGSOBJ)/PC
# 	4) To build the PGS PC tools library, type 'make libPC.a'
#	   (This runs the script $PGSBIN/mkpgslib to build the library).
#----------------------------------------------------------------------------

#
# set the name of this tool group
#

GRP= PC

include ../ecs.options
# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# define C preprocessor symbols 
DFLAGS  = -D$(HDFSYS)

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) $(ADD_IFLAGS)

# path for libraries linked (compiler will search in the order listed)
LFLAG	= -L$(PGSLIB) $(ADD_LFLAGS)

# libraries linked for C build
LIBS   = -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS) -lpthread -lm
LIBS_HP = -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS) -lm

LIBOFILES	=  			\
	$(PGSOBJ)/$(GRP)/PGS_PC_PutPCSData.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_GetPCSData.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_OpenPCSFile.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_LocateEntry.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_AdvanceArea.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_GetIndex.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_RetrieveData.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_GetRequest.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_GetFileName.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_OpenFiles.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_AdvanceToLoc.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_PutInArea.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_InsertCheck.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_SkipCheck.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_FixBuffer.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_GetReference.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_GetConfigData.o		\
        $(PGSOBJ)/$(GRP)/PGS_PC_GetNumberOfFiles.o 	\
        $(PGSOBJ)/$(GRP)/PGS_PC_GenUniqueID.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_GetFileAttr.o 		\
        $(PGSOBJ)/$(GRP)/PGS_PC_BuildAttribute.o 	\
        $(PGSOBJ)/$(GRP)/PGS_PC_GetFileByAttr.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_BuildFileShm.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_GetDataFromShm.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_GetFileFromShm.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_SearchShm.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_PutDataInShm.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_DeleteFileShm.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_WriteNewToShm.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_MarkAtTerm.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_BuildNumericInput.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_CalcArrayIndex.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_FindDefaultLocLine.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_GetPCEnv.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_GetReferenceType.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_CheckFlags.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_OneMarkRuntime.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_MarkRuntimeAscii.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_MarkRuntimeShm.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_MultiRuntimes.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_GetFileByAttrF.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_GetPCFTemp.o		\
	$(PGSOBJ)/$(GRP)/PGS_PC_GetUniversalRef.o	\
	$(PGSOBJ)/$(GRP)/PGS_PC_bindFORTRAN.o           \
	$(PGSOBJ)/$(GRP)/PGS_PC_GetFileSize.o

CMDFILES	= \
	PGS_PC_Shell					\
	$(PGSBIN)/PGS_PC_InitCom			\
	$(PGSBIN)/PGS_PC_TermCom			\
	$(PGSBIN)/PGS_PC_GetConfigDataCom		\
	$(PGSBIN)/PGS_PC_GetNumberOfFilesCom		\
	$(PGSBIN)/PGS_PC_TempDeleteCom			\
	$(PGSBIN)/PGS_PC_GetReferenceCom		\
	$(PGSBIN)/PGS_PC_GetFileAttrCom			\
	$(PGSBIN)/PGS_PC_GetTempReferenceCom		\
	$(PGSBIN)/PGS_PC_GetFileSizeCom

#
# targets
#

all: message lib

lib: $(LIBOFILES)
	@$(PGSBIN)/mkpgslib $(GRP)

utilities: PGS_PC_Shell pctcheck commands

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) $(PGSOBJ)/$(GRP)/*.o

PGS_PC_Shell:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@.sh

commands: $(CMDFILES)

pctcheck: pccheck
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@ 

pccheck:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@.sh

PGS_PC_Check:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/pctcheck

#move Toolkit shell-script utilities to $PGSBIN
$(PGSBIN)/pccheck.sh: pccheck.sh
	cp $? $@; chmod 755 $@

$(PGSBIN)/PGS_PC_Shell.sh: PGS_PC_Shell.sh
	cp $? $@; chmod 755 $@

#compile Toolkit utilities
$(PGSBIN)/pctcheck: PGS_PC_Check.c
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_Check.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_Check.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_Check.c $(LIBS) ; \
	fi ;

$(PGSBIN)/PGS_PC_InitCom: PGS_PC_InitCom.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_InitCom.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_InitCom.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_InitCom.c $(LIBS)  ; \
	fi ;

$(PGSBIN)/PGS_PC_TermCom: PGS_PC_TermCom.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_TermCom.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_TermCom.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_TermCom.c $(LIBS) ; \
	fi ;

$(PGSBIN)/PGS_PC_GetConfigDataCom: PGS_PC_GetConfigDataCom.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetConfigDataCom.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetConfigDataCom.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetConfigDataCom.c $(LIBS) ; \
	fi ;

$(PGSBIN)/PGS_PC_GetNumberOfFilesCom: PGS_PC_GetNumberOfFilesCom.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetNumberOfFilesCom.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetNumberOfFilesCom.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetNumberOfFilesCom.c $(LIBS) ; \
	fi ;

$(PGSBIN)/PGS_PC_TempDeleteCom: PGS_PC_TempDeleteCom.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_TempDeleteCom.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_TempDeleteCom.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_TempDeleteCom.c $(LIBS) ; \
	fi ;

$(PGSBIN)/PGS_PC_GetReferenceCom: PGS_PC_GetReferenceCom.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetReferenceCom.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetReferenceCom.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetReferenceCom.c $(LIBS) ; \
	fi ;

$(PGSBIN)/PGS_PC_GetFileAttrCom: PGS_PC_GetFileAttrCom.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetFileAttrCom.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetFileAttrCom.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetFileAttrCom.c $(LIBS) ; \
	fi ;

$(PGSBIN)/PGS_PC_GetTempReferenceCom: PGS_PC_GetTempReferenceCom.c
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetTempReferenceCom.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetTempReferenceCom.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetTempReferenceCom.c $(LIBS) ; \
	fi ;

$(PGSBIN)/PGS_PC_GetFileSizeCom: PGS_PC_GetFileSizeCom.c $(PGSLIB)/libPGSTK.a
	@if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetFileSizeCom.c $(LIBS) ; \
		else \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetFileSizeCom.c $(LIBS_HP) ; \
		fi; \
	else \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $@ PGS_PC_GetFileSizeCom.c $(LIBS) ; \
	fi ;

# compile Toolkit functions (source code files to object files)
$(PGSOBJ)/$(GRP)/PGS_PC_PutPCSData.o: PGS_PC_PutPCSData.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetPCSData.o: PGS_PC_GetPCSData.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_OpenPCSFile.o: PGS_PC_OpenPCSFile.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_LocateEntry.o: PGS_PC_LocateEntry.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_AdvanceArea.o: PGS_PC_AdvanceArea.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetIndex.o: PGS_PC_GetIndex.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_RetrieveData.o: PGS_PC_RetrieveData.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetRequest.o: PGS_PC_GetRequest.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetFileName.o: PGS_PC_GetFileName.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_OpenFiles.o: PGS_PC_OpenFiles.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_AdvanceToLoc.o: PGS_PC_AdvanceToLoc.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_PutInArea.o: PGS_PC_PutInArea.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_InsertCheck.o: PGS_PC_InsertCheck.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_SkipCheck.o: PGS_PC_SkipCheck.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_FixBuffer.o: PGS_PC_FixBuffer.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetReference.o: PGS_PC_GetReference.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetConfigData.o: PGS_PC_GetConfigData.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetNumberOfFiles.o: PGS_PC_GetNumberOfFiles.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GenUniqueID.o: PGS_PC_GenUniqueID.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetFileAttr.o: PGS_PC_GetFileAttr.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_BuildAttribute.o: PGS_PC_BuildAttribute.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetFileByAttr.o: PGS_PC_GetFileByAttr.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_BuildFileShm.o: PGS_PC_BuildFileShm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetDataFromShm.o: PGS_PC_GetDataFromShm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetFileFromShm.o: PGS_PC_GetFileFromShm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_SearchShm.o: PGS_PC_SearchShm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_PutDataInShm.o: PGS_PC_PutDataInShm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_DeleteFileShm.o: PGS_PC_DeleteFileShm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_WriteNewToShm.o: PGS_PC_WriteNewToShm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_MarkAtTerm.o: PGS_PC_MarkAtTerm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_BuildNumericInput.o: PGS_PC_BuildNumericInput.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_CalcArrayIndex.o: PGS_PC_CalcArrayIndex.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_FindDefaultLocLine.o: PGS_PC_FindDefaultLocLine.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetPCEnv.o: PGS_PC_GetPCEnv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetReferenceType.o: PGS_PC_GetReferenceType.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_CheckFlags.o: PGS_PC_CheckFlags.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_OneMarkRuntime.o: PGS_PC_OneMarkRuntime.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_MarkRuntimeAscii.o: PGS_PC_MarkRuntimeAscii.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_MarkRuntimeShm.o: PGS_PC_MarkRuntimeShm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_MultiRuntimes.o: PGS_PC_MultiRuntimes.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetFileByAttrF.o: PGS_PC_GetFileByAttrF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetPCFTemp.o: PGS_PC_GetPCFTemp.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetUniversalRef.o: PGS_PC_GetUniversalRef.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_bindFORTRAN.o: PGS_PC_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_PC_GetFileSize.o: PGS_PC_GetFileSize.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rules to search for a specific make rule
# for a given input file (i.e. target)
.c:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@

.c.o:
	@$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
