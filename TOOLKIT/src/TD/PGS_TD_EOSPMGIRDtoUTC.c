/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_EOSPMGIRDtoUTC.c

DESCRIPTION:
   This file contains the function PGS_TD_EOSPMGIRDtoUTC().
   This function converts EOS PM (GIRD) spacecraft clock time in CCSDS unsegmented Time
   Code (CUC) (with explicit P-field) format to UTC in CCSDS ASCII time code A
   format.
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.
   Abe Taaheri / SM&A Corp.

HISTORY:
   31-May-1994  GTSK  Initial version.
   01-Dec-1999   AT   Used PGS_TD_EOSPMtoUTC to convert EOSPM (GIRD) time to 
                      UTC time
   19-Jun-2000   AT   Modified so that TAI time has 6 significant digits after
                      the decimal point.
   03-Jan-2001   AT   Moved back to previous version
   05-Jun-2002   AT   Modified time conversion scheme for microsec part to 
                      correct 1 microsec discrapancy seen in some L0 files
                      between EDOS and TOOLKIT.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert EOS PM (GIRD) Clock Time to UTC Time

NAME:
   PGS_TD_EOSPMGIRDtoUTC()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGS_TD_EOSPMGIRDtoUTC(
        unsigned char *scTime,
        char          *asciiUTC)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_eospmtoutc(sctime,asciiutc)
      character*8  sctime
      character*27 asciiutc

DESCRIPTION:
   This function converts EOS PM (GIRD) spacecraft clock time in CCSDS unsegmented Time
   Code (CUC) (with explicit P-field) format to UTC in CCSDS ASCII time code A
   format.

INPUTS:
   NAME      DESCRIPTION                  UNITS       MIN           MAX
   ----      -----------                  -----       ---           ---
   scTime    EOS PM (GIRD) clock time in CCSDS        ** see NOTES below **
             unsegmented time code
	     format (CUC).

OUTPUTS:
   NAME      DESCRIPTION                  UNITS       MIN           MAX
   ----      -----------                  -----       ---           ---
   asciiUTC  UTC to equivalent CCSDS      ASCII       1961-01-01    see NOTES
             ASCII Time Code (format A)

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_BAD_P_FIELD         s/c clock P-field value is incorrect
   PGSTD_E_NO_LEAP_SECS        leap seconds correction unavailable at 
	                       requested time

EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES for TRANSFORMATIONS BETWEEN UTC and TAI:

     The minimum and maximum times that can successfully be processed by 
     this function depend on the file "leapsec.dat" which relates leap 
     second (TAI-UTC) values to UTC Julian dates. The file "leapsec.dat" 
     contains dates of new leap seconds and the total leap seconds for 
     times on and after Jan 1, 1972.  For times between Jan 1, 1961 and 
     Jan 1, 1972 it contains coefficients for an approximation supplied 
     by the International Earth Rotation Service (IERS) and the United 
     States Naval Observatory (USNO).  The Toolkit then uses these 
     coefficients in an algorithm consistent with IERS/USNO usage. For 
     times after Jan 1, 1961, but before the last date in the file, the
     Toolkit sets TAI-UTC equal to the total number of leap seconds to 
     date, (or to the USNO/IERS approximation, for dates before Jan 1,
     1972). If an input date is before Jan 1, 1961 the Toolkit sets the
     leap seconds value to 0.0.  This is consistent with the fact that,
     for civil timekeeping since 1972, UTC replaces Greenwich Mean Solar 
     Time (GMT), which had no leap seconds. Thus for times before Jan 1, 
     1961, the user can, for most Toolkit-related purposes, encode 
     Greenwich Mean Solar Time as if it were UTC.  If an input date
     is after the last date in the file, or after Jan 1, 1961, but the 
     file cannot be read, the function will use a calculated value of 
     TAI-UTC based on a linear fit of the data known to be in the table
     as of early 1997.  This value is a crude estimate and may be off by 
     as much as 1.0 or more seconds.  If the data file, "leapsec.dat", 
     cannot be opened, or the time is outside the range from Jan 1, 1961 
     to the last date in the file, the return status level will be 'E'.  
     Even when the status level is 'E', processing will continue, using 
     the default value of TAI-UTC (0.0 for times before Jan 1, 1961, or 
     the linear fit for later times). Thus, the user should always 
     carefully check the return status.  

     The file "leapsec.dat" is updated when a new leap second is 
     announced by the IERS, which has been, historically, about once 
     every year or two. 

   EOS PM (GIRD) TIME FORMAT:

     For EOS-PM (GIRD), the output spacecraft (s/c) clock time scTime is a 64-bit value
     in CCSDS unsegmented (CUC) format, which is comprised of three binary
     counter values:
                 
     ("scTime" represents an array of 8 (unsigned) one byte elements)
          
     1)  A 16-bit value (the first two elements of array
         scTime each with 8 bits) containing the explicit 
         P-field.  The first element (octet) of the P-field 
         for EOS PM (GIRD) has a constant value of 10101110 (binary)
         which corresponds to a decimal value of 174.  The 
         second element of the P-field has a leading bit set
         to 0.  The other seven bits are a binary counter
         representing TAI-UTC with a maximum decimal value
         of 127 (1111111 binary).
     2)  A 32-bit value (the third through sixth elements of array
         scTime each with 8 bits) containing the number of
         seconds since an epoch of 12AM UTC January 1, 1958. 
         The range of decimal values is 0-4294967295, computed as 
         256*256*256*element3 + 256*256*element4 + 256*element5
         + element6. The maximum decimal value of each 
         element 1, 2, 3 and 4 is 255.
     3)  A 16-bit value (elements 7 and 8 of array
         scTime, each with 8 bits) containing the sub-seconds.
         The range of values is 0-65535 sub-seconds, computed as 
         256*element7 + element8. The maximum decimal value of
         each element 7 and 8 is 255.  The sub-seconds represent 
         the fraction of the last second.  This fraction is calculated
         as (sub-seconds)/(256^(# of sub-second elements)).
     
         This allows the s/c clock to represent times from 1958-01-01 to
         approximately 2094-02-06T06:26:25 (give or take a few seconds).

   ASCII UTC:

     Toolkit functions that accept an ASCII time as input require the time to
     be UTC time in CCSDS ASCII Time Code A or CCSDS ASCII Time Code B format.
     Toolkit functions that return an ASCII time return the UTC time in CCSDS
     ASCII Time Code A format (see CCSDS 301.0-B-2 for details).
     (CCSDS => Consultative Committee for Space Data Systems)

      The general format is:

          YYYY-MM-DDThh:mm:ss.ddddddZ (Time Code A)
          YYYY-DDDThh:mm:ss.ddddddZ   (Time Code B)
 
          where:
              -,T,: are field delimiters
              Z is the (optional) time code terminator
              other fields are numbers as implied:
                Time Code A:
                   4 Year digits, 2 Month, 2 Day, 2 hour,
                   2 minute, 2 second, and up to 6 decimal
                   digits representing a fractional second
                Time Code B:
                   4 Year digits, 3 Day of year, 2 hour,
                   2 minute, 2 second, and up to 6 decimal
                   digits representing a fractional second

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   conversion method:

   This algorithm checks the P-field and returns an error if the first 9 bits do
   not match their expected values.  The last 7 bits are currently ignored (A
   look up table is used to compute the values of TAI-UTC).  The algorithm then
   computes the real decimal number of seconds since the epoch 1/1/58 from the
   input time and then converts this with a constant offset to real continuous
   seconds (+/-) since 1/1/93.  This number is then passed to PGS_TD_TAItoUTC()
   which converts seconds since 12AM 1/1/93 to a UTC ascii time.

GLOBALS:
   None

FILES:
   This function calls PGS_TD_TAItoUTC() which requires the file:
   leapsec.dat

FUNCTIONS_CALLED:
   PGS_TD_TAItoUTC()
   PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* constants */

#define  P_FIELD_MASK       65408U        /* binary: 1111111110000000 */
#define  P_FIELD_TEMPLATE   44544U        /* binary: 1010111000000000 */
#define  EPOCH_TIME         1104537627.0  /* (julian date 12AM 1/1/1993) -
					     (julian date 12AM 1/1/1958) 
					     in seconds */
/* name of this function */

#define FUNCTION_NAME "PGS_TD_EOSPMGIRDtoUTC()"

PGSt_SMF_status
PGS_TD_EOSPMGIRDtoUTC(              /* converts EOS PM (GIRD) s/c time to ASCII UTC */
    unsigned char *scTime,      /* EOS PM (GIRD) s/c time in CCSDS unseg. time code */
    char          *asciiUTC)    /* equivalent UTC time in CCSDS ASCII Time Code
				   A format */
{
    PGSt_uinteger seconds;      /* decimal number of s/c clock seconds */
    PGSt_uinteger subSeconds;   /* decimal number of s/c clock sub-seconds */
    PGSt_uinteger pField;       /* holder for p-field */

    PGSt_double   totalSeconds; /* */
    PGSt_uinteger micsec;

    pField = scTime[0]*256U + scTime[1];
    if ( (pField & P_FIELD_MASK) != P_FIELD_TEMPLATE)
    {
	PGS_SMF_SetStaticMsg(PGSTD_E_BAD_P_FIELD, FUNCTION_NAME);
	return PGSTD_E_BAD_P_FIELD;
    }

    seconds = ((scTime[2]*256U + scTime[3])*256U + scTime[4])*256U + scTime[5];
    subSeconds = scTime[6]*256U + scTime[7];
    /*
      totalSeconds = seconds + ( (double) subSeconds)/0x10000;
    */
    /* sub_sec = 65536 corresponds to micsecond = 1000000. The conversion is
     * therefore micsecond =(1000000/65536)*sub_sec. Due to integer arithemetic
     * and 4-byte int limitation, and in order to round to nearest integer, the
     * conversion is done as follows [1000000/65536 = 15625/1024]:
     */

    micsec=(15625 * (unsigned int)subSeconds) / 1024;
    totalSeconds = seconds + ((PGSt_double)micsec)/1000000.0;

    return PGS_TD_TAItoUTC((totalSeconds-EPOCH_TIME), asciiUTC);
}

