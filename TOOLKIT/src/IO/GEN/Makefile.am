#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit IO GEN sub group
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

GRP = IO

# define sh shell (needed for embedded shell scripts)
SHELL=/bin/sh

# Path for 'include' directive
CPPFLAGS += $(ENS_TOOLKIT_INCLUDES) -I$(top_srcdir)/include

# name of remove utility
RM = /bin/rm
RMFLAGS = -f

# Source files from this subgroup needed to build the library
noinst_LTLIBRARIES = libiogen.la

# List C source files here
CSOURCES = \
	PGS_IO_Gen_Close.c\
	PGS_IO_Gen_Open.c \
	PGS_IO_Gen_Temp_Delete.c \
	PGS_IO_Gen_Temp_Open.c \
	PGS_IO_Gen_Temp_Reference.c

# FORTRAN subset of object files in sub-group
if BUILD_FORTRAN
FFLAGS += \
	-I$(top_srcdir)/include

# Some architectures only support fortran f77. These files are seperated from
# the fortran 90 files in case only the f77 files need to be compiled. The
# FORTRAN_LANG_F77 will be set in configure if so.
if FORTRAN_LANG_F77
FSOURCES = \
	PGS_IO_Gen_CloseF.f \
	PGS_IO_Gen_OpenF.f \
	PGS_IO_Gen_Temp_OpenF.f \
	PGS_IO_Gen_Track_LUN.f
else
# Fortran 90 source files.
FSOURCES = \
	PGS_IO_Gen_CloseF.f \
	PGS_IO_Gen_OpenF90.f \
	PGS_IO_Gen_Temp_OpenF90.f \
	PGS_IO_Gen_Track_LUN.f
endif
endif

if BUILD_FORTRAN
libiogen_la_SOURCES = $(CSOURCES) $(FSOURCES)
else
libiogen_la_SOURCES = $(CSOURCES)
endif
