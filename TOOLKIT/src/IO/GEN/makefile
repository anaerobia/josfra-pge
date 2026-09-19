#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for Generic IO tools 
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
# author:  Mike Sucher 
#
# history:
#	01-Apr-1994 MES Initial version
#	05-Apr-1994 MES Remove references to private directories
#	07-Apr-1994 MES Add FORTRAN compilation rule, target for test drivers
#	04-Aug-1994 TWA Delete old files from LIBOFILES; change "Tmp" to 
# 			"Temp" in filename
#	04-Aug-1994 MES Add FORTRAN to object  (.f.o) compilation rule
# 			Add FORTRAN toolkit files to object list
# 			Add FORTRAN header files dependencies
#	09-Aug-1994 TWA Add PGS_IO_Gen_Temp_Ref to obj file list
#	10-Aug-1994 MES Revise rules to leave executables in local dir
#	12-Aug-1994 TWA Change PGS_IO_Gen_Temp_Ref.o to 
# 	                PGS_IO_Gen_Temp_Reference.o
#	17-Aug-1994 MES Enhance target finc: (copy in FORTRAN include files)
# 			  It now checks if files are there, sets write
# 			  permission and overwrites with versions in $PGSINC.
# 			Add finc to the target list for target 'all'.
#	26-Aug-1994 MES Remove obsolete FORTRAN SMF header files.
# 			Add target 'fobj' to make FORTRAN lib files.
#	30-Aug-1994 DPH Adjusted to refect new name of file PGS_IO_Gen_Temp_Del.c
#	30-Aug-1994 MES Modified C and FORTRAN libs to use the 
# 			Release 3 library libPGSTK.a
#	12-Sep-1994 MES Enhanced target finc: Don't attempt chmod on existing
# 			files if they are already writable.
#	25-Oct-1994 DPH Enhanced target 'clean' and removed target 'test', as
#			well as rules for building C/Fortran executables
#	27-Jan-1995 MES Modified object file lists, added new targets, 
# 			rewrote target 'all'.  This was done to support 
# 			selective compilation, depending on whether f77 
# 			or f90 is used to build the FORTRAN tools, because 
# 			different files are compiled.
#	03-Mar-1995 MES Modified to skip FORTRAN build if no compiler present
#	01-Nov-1995 MES Modified to allow f90 build if $F77 has flags appended.
#	03-Nov-1995 MES Modified to allow f90 build if $F77 has path prepended.
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
LIBS   = -lPGSTK $(ADD_LIBS)

# C language subset of object files from this sub-group

C_LIBOFILES	=					\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_Close.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_Open.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_Delete.o 	\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_Open.o 	\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_Reference.o

# FORTRAN 77 subset of object files from this sub-group

F77_LIBOFILES	=  					\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_CloseF.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_OpenF.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_OpenF.o 	\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_Track_LUN.o 


# FORTRAN 90 subset of object files from this sub-group

F90_LIBOFILES	=  					\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_CloseF.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_OpenF90.o 		\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_OpenF90.o	\
        $(PGSOBJ)/$(GRP)/PGS_IO_Gen_Track_LUN.o 


# FORTRAN include files needed to compile tools

FINCFILES =		\
	PGS_IO.f	\
	PGS_IO_1.f	\
	PGS_PC.f	\
	PGS_PC_9.f	\
	PGS_SMF.f 

#
# targets
#

# note: if F77 is blank or set to "none", the FORTRAN IO tools will not be built

all: message ftn_type

all_f77: cc_obj f77_obj dfinc

all_f90: cc_obj f90_obj dfinc

ftn_type:
	@case "$(F77)" in	 \
	    /*/f90* | f90*) \
	        echo "The F90 version of FORTRAN IO tools will be built." ; \
	        targ=all_f90 ; \
	        ;; \
	    /*/pgf90* | pgf90*) \
	        echo "The F90 version of FORTRAN IO tools will be built." ; \
	        targ=all_f90 ; \
	        ;; \
	    "none"|"") \
	        echo "No FORTRAN compiler - the FORTRAN IO tools will NOT be built." ; \
	        targ=cc_obj ; \
	        ;; \
	    *) \
	        echo "The F77 version of FORTRAN IO tools will be built." ; \
	        targ=all_f77 ; \
	        ;; \
	esac ; \
	$(MAKE) $(MFLAGS) $$targ


cc_obj: 	$(C_LIBOFILES)

f77_obj: 	finc $(F77_LIBOFILES)

f90_obj: 	finc $(F90_LIBOFILES)

lib:
	${PGSBIN}/mkpgslib $(GRP)

message:
	@echo "    Makefile for subgroup: GEN; Target: all"

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) $(FINCFILES)
	$(RM) $(RMFLAGS) $(C_LIBOFILES) $(F77_LIBOFILES) $(F90_LIBOFILES)

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_Close.o: PGS_IO_Gen_Close.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_Open.o: PGS_IO_Gen_Open.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_Delete.o: PGS_IO_Gen_Temp_Delete.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_Open.o: PGS_IO_Gen_Temp_Open.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_Reference.o: PGS_IO_Gen_Temp_Reference.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_CloseF.o: PGS_IO_Gen_CloseF.f
	$(F77) -c $(F77FLAGS) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_OpenF.o: PGS_IO_Gen_OpenF.f
	$(F77) -c $(F77FLAGS) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_OpenF90.o: PGS_IO_Gen_OpenF90.f
	$(F77) -c $(F77FLAGS) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_OpenF.o: PGS_IO_Gen_Temp_OpenF.f
	$(F77) -c $(F77FLAGS) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_Temp_OpenF90.o: PGS_IO_Gen_Temp_OpenF90.f
	$(F77) -c $(F77FLAGS) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_IO_Gen_Track_LUN.o: PGS_IO_Gen_Track_LUN.f
	$(F77) -c $(F77FLAGS) $? -o $@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.f.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@

#
# rule to copy FORTRAN include files from $PGSINC to working directory
#

finc: $(FINCFILES)

PGS_IO.f: $(PGSINC)/PGS_IO.f
	cp $? $@

PGS_IO_1.f: $(PGSINC)/PGS_IO_1.f
	cp $? $@

PGS_PC.f: $(PGSINC)/PGS_PC.f
	cp $? $@

PGS_PC_9.f: $(PGSINC)/PGS_PC_9.f
	cp $? $@

PGS_SMF.f: $(PGSINC)/PGS_SMF.f
	cp $? $@


#
# rule to delete FORTRAN include files from working directory
#

dfinc:
	@$(RM) $(RMFLAGS) $(FINCFILES)


