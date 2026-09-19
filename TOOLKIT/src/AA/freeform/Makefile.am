#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit AA freeform tool group
#
# author:
#	Matthew Yun / Raytheon 
#
# history:
#	5-Aug-2013 Initial version
#
# notes:
#
#----------------------------------------------------------------------------

# Set the name of this tool group
GRP = AA

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

CPPFLAGS += -I$(top_srcdir)/include/FF -I$(top_srcdir)/include

# Name of the remove utility and flags
RM= /bin/rm
RMFLAGS= -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libaafreeform.la

# define C preprocessor symbols
libaafreeform_la_CFLAGS = -DSUNCC -DPROTO

# List source files here
libaafreeform_la_SOURCES = \
	afm2bfm.c \
        byteswap.c \
        cv_units.c \
        dbevents.c \
        dbfree.c \
        dbhdlen.c \
        dl_lists.c \
        dupform.c \
        dvevents.c \
        eqn_util.c \
        error.c \
        eval_eqn.c \
        fflookup.c \
        file2buf.c \
        findfile.c \
        findvar.c \
        formlist.c \
        freeform.c \
        freeview.c \
        frm2vlst.c \
        geo44tim.c \
        get_doub.c \
        get_str.c \
        get_val.c \
        getcount.c \
        latlon.c \
        makedbin.c \
        makeform.c \
        makeindx.c \
        makeview.c \
	memtrack.c \
        mkformat.c \
        mkplist.c \
	mkstdbin.c \
        name_tab.c \
        os_utils.c \
        proclist.c \
        qstack.c \
	seismic.c \
        set_var.c \
        setdbin.c \
        setview.c \
        showdbin.c \
        showform.c \
        showvars.c \
        showview.c \
        skiphead.c \
	str2bin.c \
        strnstr.c \
        time.c \
        tytoty.c \
        viewsize.c \
        writform.c
