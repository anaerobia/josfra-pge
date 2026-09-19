#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2009, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for PGS Toolkit source group directories
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
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
# author:  Abe Taaheri
#
# history:
#	01-May-2009 AT Initital version
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
# 	3) Variable SUBGROUPS must be set for each tool group to be built.
#
#----------------------------------------------------------------------------

#
# Set the name of this tool group
#

GRP= XML

include ../ecs.options
# Subgroups for this tool group
SUBGROUPS = xml xsl

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
# IFLAG   = -I$(PGSINC) -I$(PGSINC)/libxml -I$(PGSINC)/libxslt
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(PGSINC)/libxml -I$(PGSINC)/libxslt


# object files from this sub-group needed to build library
LIBOFILES = 

#
# General rules
#

all:	message $(LIBOFILES) $(SUBGROUPS) lib

lib:
	@$(PGSBIN)/mkpgslib $(GRP)

# dummy target to force make
makeme:

#targets to make subgroups
xml: makeme
	@if [ "`echo $(CFLAGS) | cut -d' ' -f1`" = "-g" ] ; then \
	    targ="TARG=$(TARG) dbug"; \
	else \
	    targ="$(TARG)"; \
	fi; \
	echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(MFLAGS) $$targ

xsl: makeme
	@echo ""; cd xsl; echo Making \`$(TARG)\' in ./xsl; \
              $(MAKE) $(MFLAGS) $(TARG)

message:
	@echo "    Makefile for group: $(GRP); Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o $(LIBOFILES)
	@$(MAKE) $(MFLAGS) $(SUBGROUPS) TARG=$@

#
# compilation rules
#

# compile C to executable
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
