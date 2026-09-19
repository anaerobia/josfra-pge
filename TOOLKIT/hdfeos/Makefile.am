# Top-level Makefile for HDF-EOS2

# Include boilerplate
include $(top_srcdir)/config/include.am

# List of subdirectories.
# Only build the testdrivers directory is configure detected that it's present. 
if TESTDRIVERS_CONDITIONAL
TESTDRIVERS=testdrivers
else
TESTDRIVERS=
endif

if INSTALL_INCLUDE_CONDITIONAL
INCLUDE=include
else
INCLUDE=
endif

SUBDIRS=gctp src $(INCLUDE) samples $(TESTDRIVERS)

