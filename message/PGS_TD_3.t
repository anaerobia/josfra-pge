# Purpose: define error messages for PGS Toolkit Time and Date (TD) tools
# File:    PGS_TD_3.t
# Author:  various

%INSTR = PGSTK
%LABEL = PGSTD
%SEED  = 3

# Messages

PGSTD_M_ASCII_TIME_FMT_B     the time string passed in is in proper CCSDS ASCII Time Format B
PGSTD_M_LEAP_SEC_IGNORED     the input time string seconds field has been reduced from 60 to 59
PGSTD_M_TIME_TRUNCATED       the input time has been truncated to match the resolution of the output time format
 

# Warnings

PGSTD_W_DATA_FILE_MISSING    an open file call failed
PGSTD_W_JD_OUT_OF_RANGE      input julian day out of range for tabulated corrections
PGSTD_W_PRED_LEAPS           predicted value of TAI-UTC used (actual value unavailable)
PGSTD_W_BAD_SC_TIME          one or more spacecraft times in the input array (other than the 1st) could not be deciphered


# Errors

PGSTD_E_BAD_P_FIELD	     the first 9 bits of the P-field (which are constant for EOS PM) differ from the expected state
PGSTD_E_DATE_OUT_OF_RANGE    the input time is outside the range of allowable values for the spacecraft clock
PGSTD_E_MICSEC_TOO_BIG       microsecond field too large, should be < 1000
PGSTD_E_MILSEC_TOO_BIG       millisecond field too large, should be < 86401000
PGSTD_E_NO_LEAP_SECS         no leap second correction available
PGSTD_E_SC_TAG_UNKNOWN       spacecraft tag is unknown or not currently supported
PGSTD_E_TIME_FMT_ERROR       error in ascii time string format (generic format: YYYY-MM-DDThh:mm:ss.ddddddZ)
PGSTD_E_TIME_VALUE_ERROR     error in ascii time string value (e.g. hours > 23)
PGSTD_E_UNIX_ERROR           some sort of unix error occured
PGSTD_E_NO_UT1_VALUE         no UT1 value available                
PGSTD_E_BAD_2ND_HDR_FLAG     the secondary header ID flag (one bit value) is set to 1 (should be set to 0)
PGSTD_E_BAD_INITIAL_TIME     the initial spacecraft time in the input array cannot be deciphered
PGSTD_E_BAD_ARRAY_SIZE       incorrect array size (e.g. negative) specified
PGSTD_E_SECONDS_TOO_BIG      seconds field to large, should be < 86401
PGSTD_E_UTCF_UNINITIALIZED   the TRMM UTCF value has not been initialized
PGSTD_E_TMDF_UNINITIALIZED   the ADEOS-II TMDF values (period, s/c reference and ground reference) have not been initialized
PGSTD_M_HEADER_UPDATED       the leap seconds file header was updated to show USNO listed no new leap second

