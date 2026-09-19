#-----------------------------------------------------------------------------
# file:		makefile for XSL tools 
#		(for MET only)
#
# environment:	MPPDE, machine-independent, PGS directory structure
# 	
# environment variables dependencies:
#   compiler:  CC
#   includes:  PGSINC
#   libraries: PGSLIB
#   other:     PGSBIN PGSSRC HDFSYS
#  
# author: Abe Taaheri / Raytheon
#
# history:
#	01-May-2009 AT  Initial version
#
# notes:
# 	1) This file is intended for use in the Multi-Platform PGS Development
# 	   Environment (MPPDE) .  It depends on the PGS-defined toolkit 
# 	   directory structure, and on environmental variables defined
# 	   by MPPDE startup files.  This is how machine independence
# 	   is maintained.
# 	2) Target executable files are moved to $PGSBIN
# 	3) Target object files are moved to $(PGSLIB)/obj/XML
# 	4) To build the PGS CUC tools library, type 'make lib'
#	   (This runs the script $PGSBIN/mkpgslib to build the library).
#----------------------------------------------------------------------------

#
# set the name of this tool group
#
GRP = XML

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
IFLAG   = -I. $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(PGSINC)/libxslt -I$(PGSINC)/libxml

# object files from this sub-group needed to build library  

LIBOFILES = \
	$(PGSOBJ)/$(GRP)/attrvt.o \
	$(PGSOBJ)/$(GRP)/xslt.o \
	$(PGSOBJ)/$(GRP)/xsltutils.o \
	$(PGSOBJ)/$(GRP)/xslpattern.o \
	$(PGSOBJ)/$(GRP)/templates.o \
	$(PGSOBJ)/$(GRP)/variables.o \
	$(PGSOBJ)/$(GRP)/keys.o \
	$(PGSOBJ)/$(GRP)/numbers.o \
	$(PGSOBJ)/$(GRP)/extensions.o \
	$(PGSOBJ)/$(GRP)/extra.o \
	$(PGSOBJ)/$(GRP)/functions.o \
	$(PGSOBJ)/$(GRP)/namespaces.o \
	$(PGSOBJ)/$(GRP)/imports.o \
	$(PGSOBJ)/$(GRP)/attributes.o \
	$(PGSOBJ)/$(GRP)/documents.o \
	$(PGSOBJ)/$(GRP)/preproc.o \
	$(PGSOBJ)/$(GRP)/transform.o \
	$(PGSOBJ)/$(GRP)/security.o 


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


$(PGSOBJ)/$(GRP)/attrvt.o: attrvt.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xslt.o: xslt.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xsltutils.o: xsltutils.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xslpattern.o: xslpattern.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/templates.o: templates.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/variables.o: variables.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/keys.o: keys.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/numbers.o: numbers.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/extensions.o: extensions.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/extra.o: extra.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/functions.o: functions.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/namespaces.o: namespaces.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/imports.o: imports.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/attributes.o: attributes.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/documents.o: documents.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/preproc.o: preproc.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/transform.o: transform.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/security.o: security.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
