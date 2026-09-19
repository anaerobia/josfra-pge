#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for SMF tools 
#
# environment:	MPPDE, machine-independent 
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS
#   includes:  PGSINC 
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC
# 
# author:  Kelvin K. Wan
#
# history:
#	01-Apr-1994 KKW Initial version
#	18-Apr-1994 DPH Created target 'all'
#	19-Apr-1994 MES Removed smfcompile from target 'all'
# 			Made target 'smfcompile' independent of libSMF.a
#	19-Oct-1994 DPH changed LIBS to -lPGSTK instead of -l$(GRP)
#	19-Oct-1994 MES Modified the .c.o target to compile without the
# 			-O (optimize) flag.
#	25-Oct-1994 DPH Added 'clean' target and removed target 'driver'
#	26-Sep-1995 MES - Added $(ADD_IFLAGS) to IFLAG definition
# 			- Added $(ADD_LFLAGS) to LFLAG definition
# 			These allow additional directories in the include 
# 			and lib paths by setting environment variable.
# 			- Added $(ADD_LIBS) to LIBS definition which allows
# 			additional libraries to be added by setting an 
# 			environment variable.
# 			- Removed redundancy from smfcompile target that caused
# 			PGS_SMF_Comp.o to be called twice in the link.
# 			- Added .c rule
#	28-May-1996 MES Modified the .c.o and .c targets to compile without 
# 			the -ansiposix flag, for SGI IRIX/IRIX64 version 6.2
# 			only.  Needed for signal handling support.
#
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) Target executable files are moved to $PGSBIN
# 	3) Target object files are moved to $(PGSOBJ)/SMF
#----------------------------------------------------------------------------

#
# set the name of this tool group
#
GRP = SMF
include ../ecs.options
# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM = /bin/rm
RMFLAGS = -f 

# define C preprocessor symbols 
DFLAGS = -D$(HDFSYS)

# path for #include directive
IFLAG = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) $(ADD_IFLAGS)

# path for libraries linked (compiler will search in the order listed)
LFLAG = -L$(PGSLIB) $(ADD_LFLAGS)

# libraries linked
LIBS   = -lPGSTK $(ADD_LIBS)

#extra libraries

#ELIBS_sun5.5 = -L/ecs/formal/COMMON/lib/sun5.5 -lstdio -lsfio 
ELIBS_sun5.5 = $(ECS_TOOLKIT_LIBS) -lpthread 
ELIBS_$(ARCH) = $(ECS_TOOLKIT_LIBS) -lpthread
ELIBS = $(ELIBS_$(ARCH))

LIBOFILES    = \
	$(PGSOBJ)/$(GRP)/PGS_SMF.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF1.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_CacheMsgDynm.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_Comm.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_GetToolkitVersion.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_InitializeLogging.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_LogPID.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_LoggingControl.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_ManageLogControlList.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_SendRuntimeData.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_SendStatusReport.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_TraceControl.o \
	$(PGSOBJ)/$(GRP)/PGS_SMF_bindFORTRAN.o

#
# targets
#

all:	message $(PGSLIB)/libPGSTK.a

$(PGSLIB)/libPGSTK.a: $(LIBOFILES) lib

lib:
	@$(PGSBIN)/mkpgslib $(GRP)

utilities:	smfcompile

smfcompile:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@

$(PGSBIN)/smfcompile: PGS_SMF_Comp.c
	$(CC) $(CFHFLAGS) $(DFLAGS) $(VFLAG) $(IFLAG) $? -o $@ $(ELIBS) 

PGS_SMF_Comp:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/smfcompile

$(PGSBIN)/PGS_SMF_Comp:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/smfcompile

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(PGSOBJ)/$(GRP)/*.o

#
# compilation rules
#

# compile Toolkit functions (source code files to object files)
$(PGSOBJ)/$(GRP)/PGS_SMF.o: PGS_SMF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(VFLAG) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF1.o: PGS_SMF1.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_CacheMsgDynm.o: PGS_SMF_CacheMsgDynm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_Comm.o: PGS_SMF_Comm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_GetToolkitVersion.o: PGS_SMF_GetToolkitVersion.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_InitializeLogging.o: PGS_SMF_InitializeLogging.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_LogPID.o: PGS_SMF_LogPID.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_LoggingControl.o: PGS_SMF_LoggingControl.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_ManageLogControlList.o: PGS_SMF_ManageLogControlList.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_SendRuntimeData.o: PGS_SMF_SendRuntimeData.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_SendStatusReport.o: PGS_SMF_SendStatusReport.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_TraceControl.o: PGS_SMF_TraceControl.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_SMF_bindFORTRAN.o: PGS_SMF_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rules to search for a specific make rule
# for a given input file (i.e. target)
.c:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@

.c.o:
	@$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
