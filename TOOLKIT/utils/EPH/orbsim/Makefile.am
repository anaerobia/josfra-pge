#----------------------------------------------------------------------------
# file:
#	Makefile.am for SDP Toolkit EPH orbsim sub group
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

GRP = EPH

# force make to use the 'sh' shell
SHELL = /bin/sh

# Path for 'include' directive
CPPFLAGS += -I$(top_srcdir)/include -I$(top_builddir)/src -I$(top_srcdir)/include/libxml -I$(top_srcdir)/include/libxslt

# name of remove utility
RM= /bin/rm
RMFLAGS= -f 

# Standard libraries for most architectures
ORBSIM_LIBS = \
	$(top_builddir)/src/libPGSTK.la \
	-lpthread \
	-lmfhdf \
	-ldf

# Specific libraries for cygwin
if BUILD_CYG_LIBICONV
ORBSIM_LIBS += \
	/usr/lib/librpc.a \
	-liconv.dll
endif
# Specific libraries for mac architectures
if BUILD_MAC_LIBICONV
ORBSIM_LIBS += \
	/usr/lib/libiconv.dylib \
	-liconv
endif

# Path to orbsim source directory
orbsimSOURCE = $(top_srcdir)/src/EPH/orbsim

# Create and compile CSC executables
archbindir = $(bindir)/$(BRAND)

archbin_PROGRAMS = orbsim chkeph

#orbsim
orbsim_SOURCES = \
	$(orbsimSOURCE)/PGS_EPH_TransformBodyRates.c \
	$(orbsimSOURCE)/PGS_EPH_attOrbSim.c \
	$(orbsimSOURCE)/PGS_EPH_attitudeNoise.c \
	$(orbsimSOURCE)/PGS_EPH_getQuats.c \
	$(orbsimSOURCE)/PGS_EPH_matrixMultiply.c \
	$(orbsimSOURCE)/PGS_EPH_orbSim.c \
	$(orbsimSOURCE)/PGS_EPH_orbitalElements.c \
	$(orbsimSOURCE)/orbsim.c

orbsim_LDADD = $(ORBSIM_LIBS)

# chkeph
chkeph_SOURCES = $(orbsimSOURCE)/chkeph.c
chkeph_LDADD = \
	$(top_builddir)/src/libPGSTK.la
