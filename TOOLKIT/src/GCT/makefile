#----------------------------------------------------------------------------
# file:		makefile for PGS Toolkit source group directories
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependenci es:
#   compilers:  CC F77
#   includes:  PGSINC
#   libraries: PGSLIB
#   compilation flags:
#	CFLAGS		common C flags
#	C_CFH		C source + cfortran.h, FORTRAN callable (old CFHFLAGS)
#  	C_F77_CFH	C source + cfortran.h calls FORTRAN obj
#  	C_F77_LIB	libs needed when compiling C, linking to FORTRAN obj
#	F77FLAGS	common F77 flags     
#	F77_C_CFH	FORTRAN source, calls C_CFH C code (old CFH_F77)
#	F77_C_LIB	libs needed when compiling FORTRAN, linking to C obj
#   other:     PGSBIN PGSSRC
#
#
# author:  Mike Sucher / G Bland
#
# history:
#	13-Apr-1994 MES Initital version
#	18-Apr-1994 MES Pass all PGS env variables, and 
# 			call mkpgslib with $PGSHOME override
#	01-Sep-1994 MES Modified C and FORTRAN libs to use the
# 			Release 3 library libPGSTK.a
# 	08-Sep-1994 MES Clean up target 'all' - remove $(SUBGROUPS) from
# 			target list for recursive call to 'make'.
#	24-Oct-1994 MES Remove -g switch, update target 'clean'.
# 			Update status messages.
#	02-Feb-1995 ANS Adopted for GCT
#
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
#
# 	2) This makefile calls all subgroup makefiles to build the object
# 	   files needed for the group library.  Then it calls the utility
# 	   mkpgslib to build the library for this tool group.
#
# 	3) Variable SUBGROUPS must be set for each subgroup to be built.
#
#----------------------------------------------------------------------------

#
# Set the name of this tool group
#

GRP= GCT

include ../ecs.options

# Subgroups for this tool group
SUBGROUPS=  	freeware

# force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG=all

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# define C preprocessor symbols 
DFLAGS  = -D$(HDFSYS)

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(PGSINC)/FW 


# object files from this sub-group needed to build library
#
LIBOFILES      =  \
	$(PGSOBJ)/$(GRP)/PGS_GCT_Init.o \
	$(PGSOBJ)/$(GRP)/PGS_GCT_Proj.o \
	$(PGSOBJ)/$(GRP)/PGS_GCT_bindFORTRAN.o

#
# General rules
#

all:	message $(LIBOFILES) $(SUBGROUPS) lib

lib:
	@$(PGSBIN)/mkpgslib $(GRP)

# dummy target to force make
makeme:

#targets to make subgroups
freeware: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(MFLAGS) $(TARG)

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o $(LIBOFILES)
	@$(MAKE) $(MFLAGS) $(SUBGROUPS) TARG=$@

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/PGS_GCT_Init.o: PGS_GCT_Init.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_GCT_Proj.o: PGS_GCT_Proj.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_GCT_bindFORTRAN.o: PGS_GCT_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# compile C to executable
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
