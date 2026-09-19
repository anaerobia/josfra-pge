#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#----------------------------------------------------------------------------
# file:		makefile for orbsim--orbit simulator (executable)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS F77 F77_CFH
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC
#  
# author:  Mike Sucher 
#          Guru Tej S. Khalsa
#          Xin Wang
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
#	03-Nov-1995 GTSK - Added $(ADD_IFLAGS) to IFLAG definition
# 			 - Added $(ADD_LFLAGS) to LFLAG definition
# 			 These allow additional directories in the include 
# 			 and lib paths by setting environment variable.
# 			 - Added $(ADD_LIBS) to LIBS definition which allows
# 			 additional libraries to be added by setting an 
# 			 environment variable.
#       10-Feb-2002  XW  Modified to add hdf path
#       15-May-2002  XW  do not need -lpthread when HP
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

GRP= EPH

# force make to use the 'sh' shell
SHELL = /bin/sh

# name of remove utility
RM= /bin/rm
RMFLAGS= -rf 

DFLAGS = -D$(HDFSYS)

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) $(ADD_IFLAGS) -I$(HDFINC) -I$(ZLIBLIB) -I$(JPEGINC) -I$(HDF5INC) -I$(SZIPINC)

# path for libraries linked (compiler will search in the order listed)
LFLAG	= -L$(PGSLIB) $(ADD_LFLAGS) -L$(HDFLIB) -L$(ZLIBLIB) -L$(JPEGLIB) -L$(HDF5LIB) -L$(SZIPLIB)

# libraries linked for C build
LIBS     = -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS) -lpthread -lmfhdf -ldf -ljpeg $(HDF5LIB)/libhdf5.a -lz -lsz -lm
LIBS_DEC = -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS) -lpthread -lmfhdf -ldf $(JPEGLIB)/libjpeg.a $(HDF5LIB)/libhdf5.a -lz -lsz -lm
LIBS_HP  = -lPGSTK $(ADD_LIBS) $(ECS_TOOLKIT_LIBS) -lmfhdf -ldf $(JPEGLIB)/libjpeg.a $(HDF5LIB)/libhdf5.a -lz -lsz -lm

#
# object files from this sub-group needed to build library
#

SRCFILES	=  			\
	gbadsim.c

OBJFILES	=  \
	$(BRAND)/gbadsim.o

#
# targets
#

all: $(PGSBIN)/gbadsim

utilities: all

obj_dir:
	@if [ ! -d $(BRAND) ] ; then	\
		mkdir $(BRAND);		\
	fi

obj_files: $(OBJFILES)

$(PGSBIN)/gbadsim: $(SRCFILES) $(PGSLIB)/libPGSTK.a
	@$(MAKE) $(MFLAGS) obj_dir obj_files
	@if [ $(BRAND) = "sun5" -o $(BRAND) = "sun4" -o $(BRAND) = "sun5.8" -o $(BRAND) = "sun5.9" -o $(BRAND) = "sun5.10" ] ; then \
		echo "$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS) -lnsl" ; \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS) -lnsl ; \
	elif [ $(BRAND) = "dec" ] ; then \
		echo "$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS_DEC)" ; \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS_DEC) ; \
	elif [ $(BRAND) = "hp" ] ; then \
		if [ `/bin/uname -r | cut -d. -f2` = "11" ]; then \
			echo "$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS) -lnsl" ; \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS) -lnsl ; \
		else \
			echo "$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS_HP)" ; \
			$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS_HP) ; \
		fi; \
	else \
		echo "$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS)" ; \
		$(CC) $(CFHFLAGS) $(DFLAGS) $(IFLAG) $(LFLAG) $(OBJFILES) -o $@ $(LIBS) ; \
	fi ;


silent_clean:
	@$(RM) $(RMFLAGS) $(BRAND) core *.o

clean:
	$(RM) $(RMFLAGS) $(BRAND) core *.o

#
# compilation rules
#

$(BRAND)/gbadsim.o: gbadsim.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@


# compile Toolkit binaries
.c:
	@$(MAKE) $(MFLAGS) $(PGSBIN)/$@

# compile C to object
.c.o:
	@$(MAKE) $(MFLAGS) obj_dir $(BRAND)/$@
