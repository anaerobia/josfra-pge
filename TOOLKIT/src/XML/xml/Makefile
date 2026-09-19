#-----------------------------------------------------------------------------
# file:		makefile for XML tools 
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
IFLAG   = -I. $(ECS_TOOLKIT_INCLUDES) -I$(PGSINC) -I$(PGSINC)/libxml

# object files from this sub-group needed to build library  

LIBOFILES = \
	$(PGSOBJ)/$(GRP)/SAX.o \
	$(PGSOBJ)/$(GRP)/entities.o \
	$(PGSOBJ)/$(GRP)/encoding.o \
	$(PGSOBJ)/$(GRP)/error.o \
	$(PGSOBJ)/$(GRP)/parserInternals.o \
	$(PGSOBJ)/$(GRP)/parser.o \
	$(PGSOBJ)/$(GRP)/tree.o \
	$(PGSOBJ)/$(GRP)/hash.o \
	$(PGSOBJ)/$(GRP)/list.o \
	$(PGSOBJ)/$(GRP)/xmlIO.o \
	$(PGSOBJ)/$(GRP)/xmlmemory.o \
	$(PGSOBJ)/$(GRP)/uri.o \
	$(PGSOBJ)/$(GRP)/valid.o \
	$(PGSOBJ)/$(GRP)/xlink.o \
	$(PGSOBJ)/$(GRP)/HTMLparser.o \
	$(PGSOBJ)/$(GRP)/HTMLtree.o \
	$(PGSOBJ)/$(GRP)/debugXML.o \
	$(PGSOBJ)/$(GRP)/xpath.o \
	$(PGSOBJ)/$(GRP)/xpointer.o \
	$(PGSOBJ)/$(GRP)/xinclude.o \
	$(PGSOBJ)/$(GRP)/nanohttp.o \
	$(PGSOBJ)/$(GRP)/nanoftp.o \
	$(PGSOBJ)/$(GRP)/DOCBparser.o \
	$(PGSOBJ)/$(GRP)/catalog.o \
	$(PGSOBJ)/$(GRP)/globals.o \
	$(PGSOBJ)/$(GRP)/threads.o \
	$(PGSOBJ)/$(GRP)/c14n.o \
	$(PGSOBJ)/$(GRP)/xmlstring.o \
	$(PGSOBJ)/$(GRP)/xmlregexp.o \
	$(PGSOBJ)/$(GRP)/xmlschemas.o \
	$(PGSOBJ)/$(GRP)/xmlschemastypes.o \
	$(PGSOBJ)/$(GRP)/xmlunicode.o \
	$(PGSOBJ)/$(GRP)/xmlreader.o \
	$(PGSOBJ)/$(GRP)/relaxng.o \
	$(PGSOBJ)/$(GRP)/dict.o \
	$(PGSOBJ)/$(GRP)/SAX2.o \
	$(PGSOBJ)/$(GRP)/xmlwriter.o \
	$(PGSOBJ)/$(GRP)/legacy.o \
	$(PGSOBJ)/$(GRP)/chvalid.o \
	$(PGSOBJ)/$(GRP)/pattern.o \
	$(PGSOBJ)/$(GRP)/xmlsave.o \
	$(PGSOBJ)/$(GRP)/xmlmodule.o \
	$(PGSOBJ)/$(GRP)/schematron.o

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

$(PGSOBJ)/$(GRP)/SAX.o: SAX.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/entities.o: entities.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/encoding.o: encoding.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/error.o: error.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/parserInternals.o: parserInternals.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/parser.o: parser.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/tree.o: tree.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/hash.o: hash.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/list.o: list.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlIO.o: xmlIO.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlmemory.o: xmlmemory.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/uri.o: uri.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/valid.o: valid.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xlink.o: xlink.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/HTMLparser.o: HTMLparser.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/HTMLtree.o: HTMLtree.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/debugXML.o: debugXML.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xpath.o: xpath.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xpointer.o: xpointer.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xinclude.o: xinclude.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/nanohttp.o: nanohttp.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/nanoftp.o: nanoftp.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/DOCBparser.o: DOCBparser.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/catalog.o: catalog.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/globals.o: globals.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/threads.o: threads.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/c14n.o: c14n.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlstring.o: xmlstring.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlregexp.o: xmlregexp.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlschemas.o: xmlschemas.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlschemastypes.o: xmlschemastypes.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlunicode.o: xmlunicode.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlreader.o: xmlreader.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/relaxng.o: relaxng.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/dict.o: dict.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/SAX2.o: SAX2.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlwriter.o: xmlwriter.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/legacy.o: legacy.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/chvalid.o: chvalid.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/pattern.o: pattern.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlsave.o: xmlsave.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/xmlmodule.o: xmlmodule.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@
$(PGSOBJ)/$(GRP)/schematron.o: schematron.c
	$(CC) -c $(CFLAG1) $(OPT_LEVEL) $(DFLAGS) $(IFLAG) $? -o $@

# force the default make rule to search for a specific make rule
# for a given input file (i.e. target)
.c.o:
	$(MAKE) $(MFLAGS) $(PGSOBJ)/$(GRP)/$@
