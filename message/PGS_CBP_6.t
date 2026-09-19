############################################################################## 
# BEGIN_FILE_PROLOG:
# 
# FILENAME:
#   PGS_CBP_6.t 	
#   Return code definitions for SMF seed value 1 (PGS_CBP)
#   
# 
# DESCRIPTION:
# 
#   This file contains PGS_SMF standard return code definitions for the
#   PGS_CBP group of tools.
#
#   The file is intended to be used as input by the smfcompile utility,
#   which generates the PGS_6 message file, 
#   the PGS_CBP_6.h header file, and PGS_CBP_6.f include file.
#   
# 
# END_FILE_PROLOG:
##############################################################################

%INSTR	= PGSTK

%LABEL	= PGSCBP

%SEED 	= 6

#        1234567890123456789  
       
PGSCBP_E_NO_EPHEMERIS_DATA      Ephemeris data (file) is unavailable.

PGSCBP_E_BAD_ARRAY_SIZE         Invalid arrays size

PGSCBP_E_BAD_TIME_FORMAT        Invalid time format

PGSCBP_E_NO_MEMORY              Memory allocation failed, array may be to large

PGSCBP_E_BAD_CB_ID              Invalid celestian body identifier

PGSCBP_E_TIME_OUT_OF_RANGE      There is NO data for time specified.
 
PGSCBP_W_EPHEMERIS_ERROR        Error occured in JPL Ephemeris software function PLEPH

PGSCBP_E_UNABLE_TO_READ_FILE	Input file can't be open

PGSCBP_E_UNABLE_TO_OPEN_FILE	Output file can't be open

PGSCBP_W_NOT_HEADER		Header not found

PGSCBP_E_INVALID_NCOEFF		NCOEFF is invalid

PGSCBP_E_NO_OVERLAP		Records do not overlap or abut

PGSCBP_W_NO_RECORD_WRITTEN	Records not written

PGSCBP_E_INVALID_CB_ID          Invalid cb_id

PGSCBP_E_NO_LIBRATIONS          No librations

PGSCBP_E_TARG_EQUAL_CENTER      Target equal to center

PGSCBP_E_INVALID_FILE_POS       File not positioned

PGSCBP_E_READ_RECORD_ERROR      Read error record

PGSCBP_E_INVALID_INT_FLAG       Invalid flag

PGSCBP_E_EPOCH_OUT_OF_SPAN      Ephemeris out of span

PGSCBP_E_NO_NUTATIONS           No nutations

PGSCBP_E_INVALID_BODY           Invalid requested body

PGSCBP_E_BAD_INITIAL_TIME       initial time incorrect	

PGSCBP_W_EARTH_CB_ID            cbId of Earth

PGSCBP_W_BAD_CB_VECTOR          one or more bad vectors for requested times

PGSCBP_W_ERROR_IN_BODYINFOV     an error occurred in determining if the CB was in the FOV for at least one case
