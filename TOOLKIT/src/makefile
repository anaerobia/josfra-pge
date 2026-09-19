#----------------------------------------------------------------------------
# file:		makefile for PGS Toolkit source directory
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
# author:  Mike Sucher 
#
# history:
#	13-Apr-1994 MES Initital version
#	18-Apr-1994 MES Pass all PGS env variables, and 
# 			call mkpgslib with $PGSHOME override
#
# 	01-Sep-1994 MES Revisions for Release 3:
# 			- Renamed target 'message' to 'success'
# 			- Revised target 'all'.
# 			- Reformatted target '_groups'.
# 	02-Sep-1994 MES Added target 'cleano'
# 			Added target 'pgstk'
# 			Revised target 'all' to call 'pgstk' and 'cleano'
# 	13-Sep-1994 MES Revised target 'cleano' to use rm -f instead of rm.
# 	30-Dec-1994 MES Added the tool group: GCT
#       27-June-00  Yangling  ECSed26997 Exclude AA tools
#
# notes:
# 	1) This file is dependent on the PGS-defined toolkit directory 
# 	   structure, and on environment variables defined by PGS 
# 	   environment startup files.  This is how machine independence
# 	   is maintained.
#----------------------------------------------------------------------------

#include ./ecs.options

# Define sh shell
SHELL=/bin/sh

# Default target
TARG=all

# Name of remove utility
RM= /bin/rm
RMFLAGS= -f 
#NOWINKINOPT= -V

#
# PGS Toolkit Groups
#

#PGS_GROUPS=  AA CBP CSC CUC DEM EPH GCT IO MEM MET PC SMF TD
#PGS_GROUP_r= TSF AA CBP CSC CUC DEM EPH GCT IO MEM MET PC SMF TD
#
#
# General rules
#

all:    announce
	@if [ "${AA_install}" = "0" ] ; then \
	PGS_GROUPS='CBP CSC CUC DEM EPH GCT IO MEM MET PC SMF TD XML';	\
	else	\
	PGS_GROUPS='AA CBP CSC CUC DEM EPH GCT IO MEM MET PC SMF TD XML';	\
	fi;	\
	for i in $$PGS_GROUPS; do	\
	$(MAKE) $$i;	\
	done

thread: announce2 
	@if [ "${AA_install}" = "0" ] ; then	\
	PGS_GROUPS_r='TSF CBP CSC CUC DEM EPH GCT IO MEM MET PC SMF TD XML'; \
	else	\
	PGS_GROUPS_r='TSF AA CBP CSC CUC DEM EPH GCT IO MEM MET PC SMF TD XML'; \
	fi;	\
	for j in $$PGS_GROUPS_r; do	\
	$(MAKE) $$j;	\
	done

announce:
	@echo "SDP Toolkit top level makefile..."

announce2:
	@echo "SDP Toolkit top level makefile for threadsafe C library..."

utilities:
	@$(MAKE) $(NOWINKINOPT) IO CSC EPH PC TD TARG=utilities

smfcompile:
	@$(MAKE) $(NOWINKINOPT) SMF TARG=smfcompile

ephtobin:
	@$(MAKE) $(NOWINKINOPT) CBP TARG=ephtobin

# dummy target to force make
makeme:

# Target to make AA tools
AA: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make CBP tools
CBP: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make CSC tools
CSC: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make CUC tools
CUC: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make DEM tools
DEM: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make EPH tools
EPH: force_EPH

force_EPH:
	@echo ""; cd EPH; echo Making \`$(TARG)\' in ./EPH; \
              $(MAKE) $(NOWINKINOPT) GRP=EPH $(TARG)

# Target to make GCT tools
GCT: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE)  $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make IO tools
IO: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make MEM tools
MEM: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make MET tools
MET: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE)  $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make PC tools
PC: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT)  GRP=$@ $(TARG)

# Target to make SMF tools
SMF: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make TD tools
TD: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target below are for threadsafe version of the C library
TSF: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)

# Target to make XML tools
XML: makeme
	@echo ""; cd $@; echo Making \`$(TARG)\' in ./$@; \
              $(MAKE) $(NOWINKINOPT) GRP=$@ $(TARG)


# Search for and remove all .o files from source and object directories
cleano:
	@echo "Removing all object files in TOOLKIT source directories..." ; \
	find $(PGSSRC) -name \*.o -print -exec rm -f {} \; 
	@echo "Removing all object files in TOOLKIT object directories..." ; \
	find $(PGSOBJ) -name \*.o -print -exec rm -f {} \;
