#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		Makefile for MEM tools 
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
#	25-Oct-1994 DPH Added target 'clean' 
# 	13-Feb-1995 MES Remove flags used in development
# 			Don't include shared memory on Cray platform
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) Target executable files are moved to $PGSBIN
# 	3) Target object files are moved to $(PGSOBJ)/SMF
# 	4) To build the PGS IO tools library, type 'make libSMF.a'
#	   (This runs the script $PGSBIN/mkpgslib to build the library).
#       5) Cannot compile under POSIX as it need shared memory
#----------------------------------------------------------------------------

#
# set the name of this tool group
#
GRP= MEM

include ../ecs.options
# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# define C preprocessor symbols 
DFLAGS  = -D$(HDFSYS) -DSHMMEM 

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) 

# path for libraries linked (compiler will search in the order listed)
LFLAG	= -L$(PGSLIB)

# libraries linked
LIBS   = -l$(GRP) -lDBG

LIBOFILES	=  				\
	$(PGSOBJ)/$(GRP)/PGS_MEM.o		\
	$(PGSOBJ)/$(GRP)/PGS_MEM1.o		\
	$(PGSOBJ)/$(GRP)/PGS_MEM_ShmReadF.o	\
	$(PGSOBJ)/$(GRP)/PGS_MEM_ShmWriteF.o	\
	$(PGSOBJ)/$(GRP)/PGS_MEM_bindFORTRAN.o

#
# targets
#

all:	message os_type

all2: $(PGSLIB)/libPGSTK.a

os_type:
	@dflags="$(DFLAGS)" ;						\
	if [ $(BRAND) = "cray" ] ; then 				\
	    dflags="-D$(HDFSYS)" ;					\
	    echo "Compiling on Cray.  NOTE: Shared memory disabled." ;	\
	fi ;								\
	$(MAKE) $(MFLAGS) all2 DFLAGS="$$dflags"

$(PGSLIB)/libPGSTK.a: $(LIBOFILES) lib

lib:
	@$(PGSBIN)/mkpgslib $(GRP)

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) *.o;
	$(RM) $(RMFLAGS) $(PGSOBJ)/$(GRP)/*.o

# compile Toolkit functions (source code files to object files)
$(PGSOBJ)/$(GRP)/PGS_MEM.o: PGS_MEM.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MEM1.o: PGS_MEM1.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MEM_ShmReadF.o: PGS_MEM_ShmReadF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MEM_ShmWriteF.o: PGS_MEM_ShmWriteF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MEM_bindFORTRAN.o: PGS_MEM_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# compile C to executable
.c:
	$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) -o $(PGSSRC)/$@ $< $(LIBS)

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
