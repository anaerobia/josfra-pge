#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for MET tool group
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compilers:  CC F77
#   includes:  PGSINC
#   libraries: PGSLIB
#   compilation flags:
#	CFHFLAGS	common C flags + cfortran.h flags
#	F77FLAGS	common F77 flags     
#   other:     PGSBIN PGSSRC PGSOBJ
#
#
# author:  Mike Sucher 
#
# history:
#	13-Apr-1994 MES Initital version
#	18-Apr-1994 MES Pass all PGS env variables, and 
# 			call mkpgslib with $PGSHOME override
#	01-Sep-1994 MES Modified C and FORTRAN libs to use the 
# 			Release 3 library libPGSTK.a
# 	09-Sep-1994 MES Clean up target all - remove $(SUBGROUPS) from
# 			target list for recursive call to 'make'.
#       17-July-1995 ANS Fixed bug ECSed01008
#
# notes:
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
#----------------------------------------------------------------------------

#
# Set the name of this tool group
#

GRP= MET
include ../ecs.options
# Subgroups for this tool group
SUBGROUPS=  support tools

# Force make to use the 'sh' shell
SHELL = /bin/sh

# Default target
TARG=all

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f 

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
support: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(MFLAGS) $(TARG)

tools:	makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(MFLAGS) $(TARG)

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	@$(MAKE) $(MFLAGS) $(SUBGROUPS) TARG=$@
