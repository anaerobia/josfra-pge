#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#-----------------------------------------------------------------------------
# file:		makefile for AA tools 
#		(for VPF only)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS F77 F77_CFH
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC HDFSYS
#  
# author: Mike Sucher / Richard Morris / Guru Tej Khalsa
#
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) Target object files are built to $(PGSOBJ)/AA
# 	4) To build the PGS AA tools library, type 'make lib'
#	   (This runs the script $PGSBIN/mkpgslib to build the library).
#----------------------------------------------------------------------------

#
# Set the name of this tool group
#

GRP= AA

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# define C preprocessor symbols 
DFLAGS  = -D$(HDFSYS) -DSUNCC  -DFPROTO -D_INCLUDE_POSIX_SOURCE

# path for #include directive
#IFLAG   = -I$(PGSINC) -I$(PGSINC)/DCW 
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(PGSINC)/DCW

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f 

# object files from this sub-group needed to build library  

LIBOFILES = \
	$(PGSOBJ)/$(GRP)/coorgeom.o \
	$(PGSOBJ)/$(GRP)/linklist.o \
	$(PGSOBJ)/$(GRP)/set.o \
	$(PGSOBJ)/$(GRP)/strfunc.o \
	$(PGSOBJ)/$(GRP)/vpfprim2.o \
	$(PGSOBJ)/$(GRP)/vpfquery.o \
	$(PGSOBJ)/$(GRP)/vpfrelat.o \
	$(PGSOBJ)/$(GRP)/vpftable.o \
	$(PGSOBJ)/$(GRP)/vpftidx.o \
	$(PGSOBJ)/$(GRP)/vpfwrite.o \
	$(PGSOBJ)/$(GRP)/vpfspx.o \
	$(PGSOBJ)/$(GRP)/vpfsprel.o \
	$(PGSOBJ)/$(GRP)/vpfread.o \
	$(PGSOBJ)/$(GRP)/vvmisc.o 

#
# targets
#

all: message $(LIBOFILES)

lib:
	@${PGSBIN}/mkpgslib $(GRP)

message:
	@echo "    Makefile for AA/VPF; Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(LIBOFILES)

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/coorgeom.o: coorgeom.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/linklist.o: linklist.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/set.o: set.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/strfunc.o: strfunc.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vpfprim2.o: vpfprim2.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vpfquery.o: vpfquery.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vpfrelat.o: vpfrelat.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vpftable.o: vpftable.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vpftidx.o: vpftidx.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vpfwrite.o: vpfwrite.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vpfspx.o: vpfspx.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vpfsprel.o: vpfsprel.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vpfread.o: vpfread.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vvmisc.o: vvmisc.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# compile C to object
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
