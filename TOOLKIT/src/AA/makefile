#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
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
# author:  Mike Sucher / Graham Bland
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
#	20-Jul-1994 MES Add $(HDFINC) to IFLAGS
# 			Update target 'all' so that if HDFINC is not set,
# 			the tools will not be built.
# 
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
#
# 	2) This makefile calls all subgroup makefile to build the object
# 	   files needed for the group library.  Then it calls the utility
# 	   mkpgslib to build the library for this tool group.
#
# 	3) Variable SUBGROUPS must be set for each tool group to be built.
#
#----------------------------------------------------------------------------

#
# Set the name of this tool group
#

GRP = AA

include ../ecs.options

# Subgroups for this tool group
SUBGROUPS = DCW VPF freeform generic 

# force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG=all

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# path for #include directive
# IFLAG   = -I$(HDFINC) -I$(PGSINC)
IFLAG   = -I$(HDFINC) $(ECS_TOOLKIT_INCLUDES)  -I$(PGSINC)
#
#
# General rules
#

all: message
	@case "$(HDFINC)" in	 \
	    "") \
	        echo "No HDF include directory specified - the $(GRP) tools will NOT be built" ; \
	        echo "" ; \
	        ;; \
	    *) \
	        $(MAKE) $(MFLAGS) $(SUBGROUPS) lib; \
	        ;; \
	esac                                

lib:
	@$(PGSBIN)/mkpgslib $(GRP)

# dummy target to force make
makeme:

#targets to make subgroups
DCW:	makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(MFLAGS) $(TARG)

VPF:	makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(MFLAGS) $(TARG)

freeform: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(MFLAGS) $(TARG)

generic: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(MFLAGS) $(TARG)

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	@$(MAKE) $(MFLAGS) $(SUBGROUPS) TARG=$@
