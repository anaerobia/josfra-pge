#-----------------------------------------------------------------------------
# file:         makefile for GCT tools
#
# environment:  MPPDE, machine-independent, PGS directory structure
#
# environment variables dependencies:
#   compiler:  CC CFHFLAGS F77 F77_CFH
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC HDFSYS
#
# author: Mike Sucher / Graham Bland
# author: Carol S. W. Tsai / Space Applications Corporation 
# history:
#       01-Apr-1994 MES Initial version
#	02-JAN-1995 ANS Adopted for GCT
#       11-Aug-1997 CSWT Added new object files sinusfor.o, isinusinv.o,
#                        utmfor.o, and utminv.o 
# notes:
#       1) This file is intended for use in the Multi-Platform PGS Development
#          Environment (MPPDE) .  It depends on the PGS-defined toolkit
#          directory structure, and on environmental variables defined
#          by MPPDE startup files.  This is how machine independence
#          is maintained.
#       2) Target executable files are moved to $PGSBIN
#       3) Target object files are moved to $(PGSOBJ)/GCT
#       4) To build the PGS AA tools library, type 'make libGCT.a'
#          (This runs the script $PGSBIN/mkpgslib to build the library).
#----------------------------------------------------------------------------

GRP = GCT
# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f 

# define C preprocessor symbols
DFLAGS  = -D$(HDFSYS) -DPGS_TOOLKIT_COMPILE

# path for #include directive
IFLAG   = $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(PGSINC)/FW

# object files from this sub-group needed to build library

LIBOFILES = \
	$(PGSOBJ)/$(GRP)/alberfor.o \
	$(PGSOBJ)/$(GRP)/alberinv.o \
	$(PGSOBJ)/$(GRP)/alconfor.o \
	$(PGSOBJ)/$(GRP)/alconinv.o \
	$(PGSOBJ)/$(GRP)/azimfor.o \
	$(PGSOBJ)/$(GRP)/aziminv.o \
	$(PGSOBJ)/$(GRP)/bceafor.o \
	$(PGSOBJ)/$(GRP)/bceainv.o \
	$(PGSOBJ)/$(GRP)/ceafor.o \
	$(PGSOBJ)/$(GRP)/ceainv.o \
	$(PGSOBJ)/$(GRP)/cproj.o \
	$(PGSOBJ)/$(GRP)/eqconfor.o \
	$(PGSOBJ)/$(GRP)/eqconinv.o \
	$(PGSOBJ)/$(GRP)/equifor.o \
	$(PGSOBJ)/$(GRP)/equiinv.o \
	$(PGSOBJ)/$(GRP)/gnomfor.o \
	$(PGSOBJ)/$(GRP)/gnominv.o \
	$(PGSOBJ)/$(GRP)/goodfor.o \
	$(PGSOBJ)/$(GRP)/goodinv.o \
	$(PGSOBJ)/$(GRP)/gvnspfor.o \
	$(PGSOBJ)/$(GRP)/gvnspinv.o \
	$(PGSOBJ)/$(GRP)/hamfor.o \
	$(PGSOBJ)/$(GRP)/haminv.o \
	$(PGSOBJ)/$(GRP)/imolwfor.o \
	$(PGSOBJ)/$(GRP)/imolwinv.o \
	$(PGSOBJ)/$(GRP)/lamazfor.o \
	$(PGSOBJ)/$(GRP)/lamazinv.o  \
	$(PGSOBJ)/$(GRP)/lamccfor.o \
	$(PGSOBJ)/$(GRP)/lamccinv.o \
	$(PGSOBJ)/$(GRP)/merfor.o \
	$(PGSOBJ)/$(GRP)/merinv.o \
	$(PGSOBJ)/$(GRP)/millfor.o \
	$(PGSOBJ)/$(GRP)/millinv.o \
	$(PGSOBJ)/$(GRP)/molwfor.o \
	$(PGSOBJ)/$(GRP)/molwinv.o \
	$(PGSOBJ)/$(GRP)/obleqfor.o \
	$(PGSOBJ)/$(GRP)/obleqinv.o \
	$(PGSOBJ)/$(GRP)/omerfor.o \
	$(PGSOBJ)/$(GRP)/omerinv.o \
	$(PGSOBJ)/$(GRP)/orthfor.o \
	$(PGSOBJ)/$(GRP)/orthinv.o \
	$(PGSOBJ)/$(GRP)/paksz.o \
	$(PGSOBJ)/$(GRP)/polyfor.o \
	$(PGSOBJ)/$(GRP)/polyinv.o \
	$(PGSOBJ)/$(GRP)/psfor.o \
	$(PGSOBJ)/$(GRP)/psinv.o \
	$(PGSOBJ)/$(GRP)/report.o \
	$(PGSOBJ)/$(GRP)/robfor.o \
	$(PGSOBJ)/$(GRP)/robinv.o \
	$(PGSOBJ)/$(GRP)/sinfor.o \
	$(PGSOBJ)/$(GRP)/sininv.o \
	$(PGSOBJ)/$(GRP)/somfor.o \
	$(PGSOBJ)/$(GRP)/sominv.o \
	$(PGSOBJ)/$(GRP)/sphdz.o \
	$(PGSOBJ)/$(GRP)/sterfor.o \
	$(PGSOBJ)/$(GRP)/sterinv.o \
	$(PGSOBJ)/$(GRP)/stplnfor.o \
	$(PGSOBJ)/$(GRP)/stplninv.o \
	$(PGSOBJ)/$(GRP)/tmfor.o \
	$(PGSOBJ)/$(GRP)/tminv.o \
	$(PGSOBJ)/$(GRP)/untfz.o \
	$(PGSOBJ)/$(GRP)/vandgfor.o \
	$(PGSOBJ)/$(GRP)/vandginv.o \
	$(PGSOBJ)/$(GRP)/wivfor.o \
	$(PGSOBJ)/$(GRP)/wivinv.o\
	$(PGSOBJ)/$(GRP)/wviifor.o \
	$(PGSOBJ)/$(GRP)/wviiinv.o \
	$(PGSOBJ)/$(GRP)/isinusfor.o \
	$(PGSOBJ)/$(GRP)/isinusinv.o \
	$(PGSOBJ)/$(GRP)/utmfor.o \
	$(PGSOBJ)/$(GRP)/utminv.o 

INIT_FUNCS = \
	$(PGSOBJ)/$(GRP)/for_init.o \
	$(PGSOBJ)/$(GRP)/inv_init.o

#
# targets
#

all:    $(LIBOFILES)

lib:
	@$(PGSBIN)/mkpgslib GCT

clean:
	/bin/rm -f *.o core
	$(RM) $(RMFLAGS) $(LIBOFILES)

# The following two targets are for use in building the HDF-EOS version of this
# code.  This is not part of the SDP Toolkit.

gctp:
	@$(MAKE) $(MFLAGS) PGSOBJ=$(HDFEOS_OBJ) \
	 GRP="" DFLAGS=-D$(HDFSYS) $(HDFEOS_LIB)/libGctp.a

gctp5:
	@$(MAKE) $(MFLAGS) PGSOBJ=$(HDFEOS5_OBJ) \
	 GRP="" DFLAGS=-D$(HDFSYS) $(HDFEOS5_LIB)/libGctp.a

$(HDFEOS_LIB)/libGctp.a: $(INIT_FUNCS) $(LIBOFILES)
	ar r $@ $?
$(HDFEOS5_LIB)/libGctp.a: $(INIT_FUNCS) $(LIBOFILES)
	ar r $@ $?

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/alberfor.o: alberfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/alberinv.o: alberinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/alconfor.o: alconfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/alconinv.o: alconinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/azimfor.o: azimfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/aziminv.o: aziminv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/bceafor.o: bceafor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/bceainv.o: bceainv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/ceafor.o: ceafor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/ceainv.o: ceainv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/cproj.o: cproj.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/eqconfor.o: eqconfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/eqconinv.o: eqconinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/equifor.o: equifor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/equiinv.o: equiinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/for_init.o: for_init.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/gnomfor.o: gnomfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/gnominv.o: gnominv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/goodfor.o: goodfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/goodinv.o: goodinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/gvnspfor.o: gvnspfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/gvnspinv.o: gvnspinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/hamfor.o: hamfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/haminv.o: haminv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/imolwfor.o: imolwfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/imolwinv.o: imolwinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/inv_init.o: inv_init.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/lamazfor.o: lamazfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/lamazinv.o: lamazinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/lamccfor.o: lamccfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/lamccinv.o: lamccinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/merfor.o: merfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/merinv.o: merinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/millfor.o: millfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/millinv.o: millinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/molwfor.o: molwfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/molwinv.o: molwinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/obleqfor.o: obleqfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/obleqinv.o: obleqinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/omerfor.o: omerfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/omerinv.o: omerinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/orthfor.o: orthfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/orthinv.o: orthinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/paksz.o: paksz.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/polyfor.o: polyfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/polyinv.o: polyinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/psfor.o: psfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/psinv.o: psinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/report.o: report.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/robfor.o: robfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/robinv.o: robinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/sinfor.o: sinfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/sininv.o: sininv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/somfor.o: somfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/sominv.o: sominv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/sphdz.o: sphdz.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/sterfor.o: sterfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/sterinv.o: sterinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/stplnfor.o: stplnfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/stplninv.o: stplninv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/tmfor.o: tmfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/tminv.o: tminv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/untfz.o: untfz.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vandgfor.o: vandgfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/vandginv.o: vandginv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/wivfor.o: wivfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/wivinv.o: wivinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/wviifor.o: wviifor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/wviiinv.o: wviiinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/isinusfor.o: isinusfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/isinusinv.o: isinusinv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/utmfor.o: utmfor.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/utminv.o: utminv.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# compile C to object
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
