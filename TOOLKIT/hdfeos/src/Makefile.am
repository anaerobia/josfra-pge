# $(top_srcdir)/src Makefile.am

# Boilerplate definitions
include $(top_srcdir)/config/include.am

INCLUDES=-I$(top_srcdir)/include/

# Set LDFLAGS to alow the HDF-EOS library to use extern variables from HDF4
LDFLAGS=-Wl,-single_module

# Build HDF-EOS2
lib_LTLIBRARIES=libhdfeos.la

# Source files for library
libhdfeos_la_SOURCES=EHapi.c GDapi.c PTapi.c SWapi.c

