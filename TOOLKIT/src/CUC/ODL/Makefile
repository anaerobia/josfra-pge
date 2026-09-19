#-----------------------------------------------------------------------------
# file:		makefile for CUC tools 
#		(for ODL only)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC CFHFLAGS F77 F77_CFH
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC HDFSYS
#  
# author: Mike Sucher / Richard Morris /Megan E. D. Spencer / Abe Taaheri
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
#	09-Aug-1994 TWA Add PG_IO_Gen_Temp_Ref to obj file list
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
#	07-Sep-1994 RSM Change to use with AA tools
#	24-Oct-1994 MES Enhance target finc: Don't attempt chmod on existing
# 			files if they are already writable.
# 			Remove -g switch, obsolete targets
#	21-Jul-1999 MEDS Updated with flag needed for HP
#       05-Mar-2001 AT  Modified for linux
#
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) Target executable files are moved to $PGSBIN
# 	3) Target object files are moved to $(PGSLIB)/obj/CUC
# 	4) To build the PGS CUC tools library, type 'make lib'
#	   (This runs the script $PGSBIN/mkpgslib to build the library).
#----------------------------------------------------------------------------

#
# set the name of this tool group
#
GRP = CUC

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh


# Default target
TARG=all

# name of remove utility
RM = /bin/rm
RMFLAGS = -f 

CFLAG1 =

# define optimization level for comilation
OPT_LEVEL = -O

# define C preprocessor symbols 
DFLAGS  = -DSUN_UNIX

# path for #include directive
IFLAG   = -I. $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(PGSINC)/CUC 

# object files from this sub-group needed to build library  

LIBOFILES = \
	$(PGSOBJ)/$(GRP)/a_nodesa.o \
	$(PGSOBJ)/$(GRP)/ag_nodesag.o \
	$(PGSOBJ)/$(GRP)/ao_nodesao.o \
	$(PGSOBJ)/$(GRP)/comments.o \
	$(PGSOBJ)/$(GRP)/cvtvalue.o \
	$(PGSOBJ)/$(GRP)/fmtvalue.o \
	$(PGSOBJ)/$(GRP)/lexan.o \
	$(PGSOBJ)/$(GRP)/p_nodesp.o \
	$(PGSOBJ)/$(GRP)/parsact.o \
	$(PGSOBJ)/$(GRP)/parser.o \
	$(PGSOBJ)/$(GRP)/prtlabel.o \
	$(PGSOBJ)/$(GRP)/prtsrc.o \
	$(PGSOBJ)/$(GRP)/rdlabel.o \
	$(PGSOBJ)/$(GRP)/rdvalue.o \
	$(PGSOBJ)/$(GRP)/v_nodesv.o \
	$(PGSOBJ)/$(GRP)/wrtlabel.o \
	$(PGSOBJ)/$(GRP)/wrtsrc.o \
	$(PGSOBJ)/$(GRP)/output.o

#
# targets
#

all:	os_type

os_type:
	@cflag=$(CFLAG1);					\
	if [ $(BRAND) = "hp" ] ; then 				\
	    cflag="-Ae" ;					\
	    echo "Compiling on HP.  NOTE: FLAG added." ;	\
	fi ;							\
	if [ $(BRAND) = "linux" ] ; then 			\
	    cflag="-DLINUX" ;					\
	    echo "Compiling on LINUX.  NOTE: FLAG added." ;	\
	fi ;							\
	if [ $(BRAND) = "linux64" ] ; then 			\
	    cflag="-DLINUX64" ;					\
	    echo "Compiling on LINUX64.  NOTE: FLAG added." ;	\
	fi ;							\
	if [ $(BRAND) = "linux32" ] ; then 			\
	    cflag="-DLINUX" ;					\
	    echo "Compiling on LINUX32.  NOTE: FLAG added." ;	\
	fi ;							\
        if [ $(BRAND) = "macintosh" ] ; then                        	\
            cflag="-DMACINTOSH" ;                                   	\
            echo "Compiling on Macintosh.  NOTE: FLAG added." ;     	\
        fi ;                        	                 	    	\
        if [ $(BRAND) = "macintel" ] ; then            	    		\
            cflag="-DMACINTEL" ;                                   	\
            echo "Compiling on Macintosh (Intel).  NOTE: FLAG added." ; \
        fi ;                                             		\
        if [ $(BRAND) = "macintel32" ] ; then            	    		\
            cflag="-DMACINTEL" ;                                   	\
            echo "Compiling on Macintosh (Intel).  NOTE: FLAG added." ; \
        fi ;                                             		\
        if [ $(BRAND) = "macintel64" ] ; then            	    		\
            cflag="-DMACINTEL" ;                                   	\
            echo "Compiling on Macintosh (Intel).  NOTE: FLAG added." ; \
        fi ;                                             		\
        if [ $(BRAND) = "cygwin" ] ; then            	    		\
            cflag="-DCYGWIN" ;                                   	\
            echo "Compiling on Cygwin.  NOTE: FLAG added." ; 		\
        fi ;                        	                 	    	\
	$(MAKE) $(MFLAGS) all2 CFLAG1="$$cflag $(CFHFLAGS)"

all2: $(PGSLIB)/libPGSTK.a

$(PGSLIB)/libPGSTK.a: $(LIBOFILES)
	@$(PGSBIN)/mkpgslib $(GRP)

dbug:
	@$(MAKE) OPT_LEVEL=-g $(TARG)

lib:
	@${PGSBIN}/mkpgslib $(GRP)

clean:
	$(RM) $(RMFLAGS) core *.o $(LIBOFILES)

#
# compilation rules
#

$(PGSOBJ)/$(GRP)/a_nodesa.o: a_nodesa.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/ag_nodesag.o: ag_nodesag.c
	$(CC) -c $(CFLAG1)  $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/ao_nodesao.o: ao_nodesao.c
	$(CC) -c $(CFLAG1)  $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/comments.o: comments.c
	$(CC) -c $(CFLAG1)  $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/cvtvalue.o: cvtvalue.c
	$(CC) -c $(CFLAG1)  $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/fmtvalue.o: fmtvalue.c
	$(CC) -c  $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/lexan.o: lexan.c
	$(CC) -c  $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/p_nodesp.o: p_nodesp.c
	$(CC) -c  $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/parsact.o: parsact.c
	$(CC) -c  $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/parser.o: parser.c
	$(CC) -c  $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/prtlabel.o: prtlabel.c
	$(CC) -c  $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/prtsrc.o: prtsrc.c
	$(CC) -c $(CFLAG1)  $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/rdlabel.o: rdlabel.c
	$(CC) -c  $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/rdvalue.o: rdvalue.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/v_nodesv.o: v_nodesv.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/wrtlabel.o: wrtlabel.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/wrtsrc.o: wrtsrc.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

$(PGSOBJ)/$(GRP)/output.o: output.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
