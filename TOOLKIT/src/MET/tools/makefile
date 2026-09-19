#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#-----------------------------------------------------------------------------
# file:         makefile for MET tools
#
# environment:  MPPDE, machine-independent, PGS directory structure
#
# environment variables dependencies:
#   compiler:  CC CFHFLAGS F77 F77_CFH
#   includes:  PGSINC
#   other:     PGSOBJ HDFSYS HDFINC HDF5INC
#
# author: Mike Sucher / Graham Bland / Abe Taaheri
# history:
#       01-Apr-1994 MES Initial version
#	02-JAN-1995 ANS Adopted for GCT
#	04-Apr-1995 ANS Adopted for MET
#       23-Jan-2001 AT  Added PGS_MET_SetMultiAttr
#       30-Mar-2001 AT  Modified for HDF5 support
# notes:
#       1) This file is intended for use in the Multi-Platform PGS Development
#          Environment (MPPDE) .  It depends on the PGS-defined toolkit
#          directory structure, and on environmental variables defined
#          by MPPDE startup files.  This is how machine independence
#          is maintained.
#       2) Target object files are moved to $(PGSOBJ)/MET
#
#----------------------------------------------------------------------------

#
# Set the name of this tool group
#

GRP= MET

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f 

# define C preprocessor symbols
DFLAGS  = -D$(HDFSYS) -DPGS_MET_COMPILE

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(PGSINC)/CUC -I$(HDFINC) -I$(HDF5INC)

# object files from this sub-group needed to build library

LIBOFILES = \
	$(PGSOBJ)/$(GRP)/PGS_MET_GetSetAttr.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_GetSetAttrF.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_Init.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_SetAttr.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_SetAttrF.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_SetMultiAttr.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_SetMultiAttrF.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_GetPCAttr.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_GetPCAttrF.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_GetConfigData.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_GetConfigDataF.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_Write.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_Remove.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_HDFFileType.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_HDFSDstart.o \
	$(PGSOBJ)/$(GRP)/PGS_MET_bindFORTRAN.o
#
# targets
#

all:    $(LIBOFILES)

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(LIBOFILES)

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/PGS_MET_GetSetAttr.o: PGS_MET_GetSetAttr.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_GetSetAttrF.o: PGS_MET_GetSetAttrF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_Init.o: PGS_MET_Init.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_SetAttr.o: PGS_MET_SetAttr.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_SetAttrF.o: PGS_MET_SetAttrF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_SetMultiAttr.o: PGS_MET_SetMultiAttr.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_SetMultiAttrF.o: PGS_MET_SetMultiAttrF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_GetPCAttr.o: PGS_MET_GetPCAttr.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_GetPCAttrF.o: PGS_MET_GetPCAttrF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_GetConfigData.o: PGS_MET_GetConfigData.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_GetConfigDataF.o: PGS_MET_GetConfigDataF.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_Write.o: PGS_MET_Write.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_Remove.o: PGS_MET_Remove.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_bindFORTRAN.o: PGS_MET_bindFORTRAN.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_HDFFileType.o: PGS_MET_HDFFileType.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/PGS_MET_HDFSDstart.o: PGS_MET_HDFSDstart.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# compile C to object
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
