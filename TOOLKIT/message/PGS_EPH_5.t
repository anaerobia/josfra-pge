# Purpose: define error messages for PGS_EPH_ tools
# File:    PGS_EPH_5.t
# Author:  various

%INSTR = PGSTK
%LABEL = PGSEPH
%SEED  = 5



PGSEPH_E_SIMULATOR_ERROR    orbit/attitude simulator error
PGSEPH_W_BAD_EPHEM_VALUE    one or more of the requested values could not be determined, undetermined values set to 1.0E50
PGSEPH_E_BAD_ARRAY_SIZE     incorrect array size (i.e. < 0) specified
PGSEPH_E_NO_SC_EPHEM_FILE   error in accessing spacecraft ephemeris and/or attitude file
PGSEPH_E_BAD_EPHEM_FILE_HDR error in file header of spacecraft ephemeris and/or attitude file
PGSEPH_E_BAD_TIME_ORDER     bad ordering of input times (start time <= desired time <= stop time)
PGSEPH_E_NO_DATA_REQUESTED  no data have been requested (i.e. all data flags set to PGS_FALSE)
PGSEPH_M_SHORT_ARRAY        not all elements of input array have been initialized with data
PGSEPH_W_CORRUPT_METADATA   one or more ephemeris files have out of order or corrupt metadata
PGSEPH_W_EPHATT_NUMRECS_DIFFER in the given time period number of records for eph is different from number of records for att
PGSEPH_W_DBLVALUE_METADATA  for the same orbit from two adjacent eph files the metadata values are slightly different
