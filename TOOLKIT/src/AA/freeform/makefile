#-------------------------------------------------------------------------#
#                                                                         #
#  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  #
#  and suppliers.  ALL RIGHTS RESERVED.                                   #
#                                                                         #
#-------------------------------------------------------------------------#
#-----------------------------------------------------------------------------
# file:		makefile for AA tools 
#		(for freeform only)
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
OPSYS = SUNCC
DFLAGS = -D$(OPSYS) -DPROTO

# path for #include directive
#IFLAG   = -I$(HDFINC) -I$(PGSINC) -I$(PGSINC)/FF
IFLAG   = -I$(HDFINC) -I$(PGSINC) -I$(PGSINC)/FF $(ECS_TOOLKIT_INCLUDES)

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f 

# object files from this sub-group needed to build library  

LIBOFILES = \
	$(PGSOBJ)/$(GRP)/afm2bfm.o \
        $(PGSOBJ)/$(GRP)/byteswap.o \
        $(PGSOBJ)/$(GRP)/cv_units.o \
        $(PGSOBJ)/$(GRP)/dbevents.o \
        $(PGSOBJ)/$(GRP)/dbfree.o \
        $(PGSOBJ)/$(GRP)/dbhdlen.o \
        $(PGSOBJ)/$(GRP)/dl_lists.o \
        $(PGSOBJ)/$(GRP)/dupform.o \
        $(PGSOBJ)/$(GRP)/dvevents.o \
        $(PGSOBJ)/$(GRP)/eqn_util.o \
        $(PGSOBJ)/$(GRP)/error.o\
        $(PGSOBJ)/$(GRP)/eval_eqn.o \
        $(PGSOBJ)/$(GRP)/fflookup.o \
        $(PGSOBJ)/$(GRP)/file2buf.o \
        $(PGSOBJ)/$(GRP)/findfile.o \
        $(PGSOBJ)/$(GRP)/findvar.o \
        $(PGSOBJ)/$(GRP)/formlist.o \
        $(PGSOBJ)/$(GRP)/freeform.o \
        $(PGSOBJ)/$(GRP)/freeview.o \
        $(PGSOBJ)/$(GRP)/frm2vlst.o \
        $(PGSOBJ)/$(GRP)/geo44tim.o \
        $(PGSOBJ)/$(GRP)/get_doub.o \
        $(PGSOBJ)/$(GRP)/get_str.o \
        $(PGSOBJ)/$(GRP)/get_val.o \
        $(PGSOBJ)/$(GRP)/getcount.o \
        $(PGSOBJ)/$(GRP)/latlon.o \
        $(PGSOBJ)/$(GRP)/makedbin.o \
        $(PGSOBJ)/$(GRP)/makeform.o \
        $(PGSOBJ)/$(GRP)/makeindx.o \
        $(PGSOBJ)/$(GRP)/makeview.o \
	$(PGSOBJ)/$(GRP)/memtrack.o \
        $(PGSOBJ)/$(GRP)/mkformat.o \
        $(PGSOBJ)/$(GRP)/mkplist.o \
	$(PGSOBJ)/$(GRP)/mkstdbin.o \
        $(PGSOBJ)/$(GRP)/name_tab.o \
        $(PGSOBJ)/$(GRP)/os_utils.o \
        $(PGSOBJ)/$(GRP)/proclist.o \
        $(PGSOBJ)/$(GRP)/qstack.o \
	$(PGSOBJ)/$(GRP)/seismic.o \
        $(PGSOBJ)/$(GRP)/set_var.o \
        $(PGSOBJ)/$(GRP)/setdbin.o \
        $(PGSOBJ)/$(GRP)/setview.o \
        $(PGSOBJ)/$(GRP)/showdbin.o \
        $(PGSOBJ)/$(GRP)/showform.o \
        $(PGSOBJ)/$(GRP)/showvars.o \
        $(PGSOBJ)/$(GRP)/showview.o \
        $(PGSOBJ)/$(GRP)/skiphead.o \
	$(PGSOBJ)/$(GRP)/str2bin.o \
        $(PGSOBJ)/$(GRP)/strnstr.o \
        $(PGSOBJ)/$(GRP)/time.o \
        $(PGSOBJ)/$(GRP)/tytoty.o \
        $(PGSOBJ)/$(GRP)/viewsize.o \
        $(PGSOBJ)/$(GRP)/writform.o

#
# targets
#
all: message
	@case "$(HDFINC)" in	 \
	    "") \
	        echo "No HDF include directory specified - AA/freeform will NOT be built" ; \
	        echo "" ; \
	        ;; \
	    *) \
		case "$(BRAND)" in \
		   dec)  dflags="-DDEC_ALPHA $(DFLAGS)" ;; \
		   hp) dflags="-DHP9000 $(DFLAGS)" ;; \
		   ibm) dflags="-DIBM6000 $(DFLAGS)" ;; \
		   linux) dflags="-DLINUX $(DFLAGS)" ;; \
		   linux32) dflags="-DLINUX32 $(DFLAGS)" ;; \
		   linux64) dflags="-DLINUX64 $(DFLAGS)" ;; \
		   sco) dflags="-DLINUX $(DFLAGS)" ;; \
		   sgi) dflags="-DIRIS4 $(DFLAGS)" ;; \
		   sgi32) dflags="-DIRIS4 $(DFLAGS)" ;; \
                   irix65) dflags="-DIRIS4 $(DFLAGS)" ;; \
		   sgi64) dflags="-DIRIX $(DFLAGS)" ;; \
		   sun4) dflags="-DSUN $(DFLAGS)" ;; \
		   sun5) dflags="-DSUN $(DFLAGS)" ;; \
		   sun5.8) dflags="-DSUN $(DFLAGS)" ;; \
		   sun5.9) dflags="-DSUN $(DFLAGS)" ;; \
		   sun5.10) dflags="-DSUN $(DFLAGS)" ;; \
		   *) dflags="$(DFLAGS)" ;; \
		esac ; \
		$(MAKE) $(MFLAGS) $(PGS_FLAGS) DFLAGS="$$dflags" ff_all; \
	        ;; \
	esac                                

ff_all:	$(LIBOFILES)

lib:
	@${PGSBIN}/mkpgslib $(GRP)

message:
	@echo "    Makefile for AA/freeform; Target: all"

clean:
	$(RM) $(RMFLAGS) core *.o
	$(RM) $(RMFLAGS) $(LIBOFILES)

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/afm2bfm.o: afm2bfm.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/byteswap.o: byteswap.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/cv_units.o: cv_units.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/dbevents.o: dbevents.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/dbfree.o: dbfree.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/dbhdlen.o: dbhdlen.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/dl_lists.o: dl_lists.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/dupform.o: dupform.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/dvevents.o: dvevents.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/eqn_util.o: eqn_util.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/error.o: error.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/eval_eqn.o: eval_eqn.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/fflookup.o: fflookup.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/file2buf.o: file2buf.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/findfile.o: findfile.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/findvar.o: findvar.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/formlist.o: formlist.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/freeform.o: freeform.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/freeview.o: freeview.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/frm2vlst.o: frm2vlst.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/geo44tim.o: geo44tim.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/get_doub.o: get_doub.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/get_str.o: get_str.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/get_val.o: get_val.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/getcount.o: getcount.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/latlon.o: latlon.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/makedbin.o: makedbin.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/makeform.o: makeform.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/makeindx.o: makeindx.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/makeview.o: makeview.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/memtrack.o: memtrack.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/mkformat.o: mkformat.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/mkplist.o: mkplist.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/mkstdbin.o: mkstdbin.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/name_tab.o: name_tab.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/os_utils.o: os_utils.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/proclist.o: proclist.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/qstack.o: qstack.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/seismic.o: seismic.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/set_var.o: set_var.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/setdbin.o: setdbin.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/setview.o: setview.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/showdbin.o: showdbin.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/showform.o: showform.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/showvars.o: showvars.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/showview.o: showview.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/skiphead.o: skiphead.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/str2bin.o: str2bin.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/strnstr.o: strnstr.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/time.o: time.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/tytoty.o: tytoty.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/viewsize.o: viewsize.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/writform.o: writform.c
	$(CC) -c $(CFHFLAGS) $(DFLAGS) $(IFLAG) $? -o $@

# compile C to object
.c.o:
	@case "$(BRAND)" in \
	    dec)  dflags="-DDEC_ALPHA $(DFLAGS)" ;; \
	    hp) dflags="-DHP9000 $(DFLAGS)" ;; \
	    ibm) dflags="-DIBM6000 $(DFLAGS)" ;; \
	    linux) dflags="-DLINUX $(DFLAGS)" ;; \
	    sco) dflags="-DLINUX $(DFLAGS)" ;; \
	    sgi) dflags="-DIRIS4 $(DFLAGS)" ;; \
	    sgi32) dflags="-DIRIS4 $(DFLAGS)" ;; \
            irix65) dflags="-DIRIS4 $(DFLAGS)" ;; \
	    sgi64) dflags="-DIRIX $(DFLAGS)" ;; \
	    sun4) dflags="-DSUN $(DFLAGS)" ;; \
	    sun5) dflags="-DSUN $(DFLAGS)" ;; \
	    sun5.8) dflags="-DSUN $(DFLAGS)" ;; \
	    sun5.9) dflags="-DSUN $(DFLAGS)" ;; \
	    sun5.10) dflags="-DSUN $(DFLAGS)" ;; \
	    *) dflags="$(DFLAGS)" ;; \
	esac ; \
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@ DFLAGS="$$dflags"
