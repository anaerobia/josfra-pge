#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for L0sim utility (simulated L0 file generator)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC PGSOBJ
#  
# author:  Mike Sucher 
#          Guru Tej S. Khalsa
#
# history:
#	13-Apr-1994 MES  Template version
#	18-Apr-1994 MES  Pass all PGS env variables, and 
# 			 call mkpgslib with $PGSHOME override
#	01-Sep-1994 MES  Modified C and FORTRAN libs to use the 
# 			 Release 3 library libPGSTK.a
#       18-Sep-1994 GTSK Modified to build single executable "orbsim".
#                        No longer builds liborbsim.a although that can
#                        be done using the liborbsim target.  Stripped out
#                        irrelavant FORTRAN stuff.
#	21-Feb-1995 MES  Add special rule for compilation on DECs.  The size
#			 size of a block of code that will be optimized was
#			 increased.
#	03-Nov-1995 GTSK - Added $(ADD_IFLAGS) to IFLAG definition
# 			 - Added $(ADD_LFLAGS) to LFLAG definition
# 			 These allow additional directories in the include 
# 			 and lib paths by setting environment variable.
# 			 - Added $(ADD_LIBS) to LIBS definition which allows
# 			 additional libraries to be added by setting an 
# 			 environment variable.
#      05-May-2000 Yangling  Huang ECSed24649 Remove Executable L0sim file
#      15-May-2002  XW   do not need -lpthread when HP
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE).  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) Target executable files are moved to $PGSBIN.
# 	3) Target object files are moved to $(PGSOBJ)/$(GRP).
# 	4) This file does NOT build the library for this group.
#	   That is handled by the group-level makefile.
#
#----------------------------------------------------------------------------

#
# set the name of this tool group
#

GRP= IO
## this sets ECS FLAGS.
#include ../../../ecs.options

# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -f
# define C preprocessor symbols 
DFLAGS = -D$(HDFSYS)

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) $(ADD_IFLAGS)
# path for libraries linked (compiler will search in the order listed)
LFLAG	= -L$(PGSLIB) $(ADD_LFLAGS)

# libraries linked for C build

LIBS   = -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS)  -lpthread  -lm
LIBS_HP= -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS) -lm
#LIBS   = -lPGSTK $(ADD_LIBS) -lm

LIBOFILES =				\
	$(PGSOBJ)/$(GRP)/PGS_IO_L0_File_Sim.o		\
	$(PGSOBJ)/$(GRP)/PGS_IO_L0_EDOS_hdr_Sim.o	\
	$(PGSOBJ)/$(GRP)/PGS_IO_L0_sortArrayIndices.o	\
	$(PGSOBJ)/$(GRP)/PGS_IO_L0_SFDU_Sim.o		\
	$(PGSOBJ)/$(GRP)/PGS_IO_L0_L0Sim_bindFORTRAN.o

#
# targets
#

all:	message L0sim

$(PGSLIB)/libPGSTK.a: $(LIBOFILES)
	$(PGSBIN)/mkpgslib $(GRP)

lib: $(LIBOFILES)
	$(PGSBIN)/mkpgslib $(GRP)

# Libraries

utilities: L0sim

# special rule is required to compile L0sim with optimization on DECs
L0sim: L0sim.c $(PGSLIB)/libPGSTK.a
	@cfhflags="$(CFHFLAGS)" ; \
	tab="" ; \
	case "$(BRAND)" in \
	dec) echo "Compiling on DEC.." ; \
	     cfhflags=`echo $(CFHFLAGS) | sed 's/-O/-Olimit 700/'` ;; \
	aix|sgi*) tab="	" ;; \
	esac ; \
	if [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			echo "$$tab"$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF  $(LIBS) ; \
			$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF $(LIBS) ; \
			echo "$$tab"NEWSTDLIBS = $(NEWSTDLIBS); \
			echo "$$tab"$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF $(LIBS) $(NEWSTDLIBS) ; \
			$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF $(LIBS) $(NEWSTDLIBS) ; \
		else \
			echo "$$tab"$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF  $(LIBS_HP) ; \
			$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF $(LIBS_HP) ; \
			echo "$$tab"NEWSTDLIBS = $(NEWSTDLIBS); \
			echo "$$tab"$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF $(LIBS_HP) $(NEWSTDLIBS) ; \
			$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF $(LIBS_HP) $(NEWSTDLIBS) ; \
		fi; \
	else \
		echo "$$tab"$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF  $(LIBS) ; \
		$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF $(LIBS) ; \
		echo "$$tab"NEWSTDLIBS = $(NEWSTDLIBS); \
		echo "$$tab"$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF $(LIBS) $(NEWSTDLIBS) ; \
		$(CC) $$cfhflags $(DFLAGS) $(IFLAG) $(LFLAG) L0sim.c -o $@ -DDDDDJDF $(LIBS) $(NEWSTDLIBS) ; \
	fi ;

#for backward compatibility
simlib: $(PGSLIB)/libPGSTK.a

message:
	@echo "    Makefile for utility: L0sim; Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(LIBOFILES)
	$(RM) $(RMFLAGS) L0sim
#
# compilation rules
#

$(PGSOBJ)/$(GRP)/PGS_IO_L0_File_Sim.o: PGS_IO_L0_File_Sim.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_EDOS_hdr_Sim.o: PGS_IO_L0_EDOS_hdr_Sim.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_sortArrayIndices.o: PGS_IO_L0_sortArrayIndices.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_SFDU_Sim.o: PGS_IO_L0_SFDU_Sim.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_L0_L0Sim_bindFORTRAN.o: PGS_IO_L0_L0Sim_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.c:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@

.c.o:
	@$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
