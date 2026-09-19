/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_TRMMtoUTC.c

DESCRIPTION:
   This file contains the function PGS_TD_TRMMtoUTC().
   This function converts TRMM spacecraft clock time in CCSDS unsegmented Time
   Code (CUC) (with implicit P-field) format to UTC in CCSDS ASCII time code A
   format.
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
   31-May-1994  GTSK  Initial version
   10-Jun-1994  GTSK  Added WARNING message to NOTES section (below) explaining
                      why this algorithm doesn't actually work with the real 
		      TRMM time.
   21-May-1997  PDN   Fixed Comments on Leap Seconds and TAI
   22-May-1997  PDN   Removed references and code for PRED_LEAPS

END_FILE_PROLOG:
*******************************************************************************/

/******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert TRMM Clock Time to UTC Time

NAME:
   PGS_TD_TRMMtoUTC()
 
SYNOPSIS:
 C:
   #include PGS_TD.h

   PGSt_SMF_status
   PGS_TD_TRMMtoUTC(
       unsigned char *scTime,
       char          *asciiUTC)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_trmmtoutc(sctime,asciiutc)
      character*8      sctime
      character*27     asciiutc

DESCRIPTION:
   This function converts TRMM spacecraft clock time in CCSDS unsegmented Time
   Code (CUC) (with implicit P-field) format to UTC in CCSDS ASCII time code A
   format.
 
INPUTS:
   NAME        DESCRIPTION                UNITS        MIN            MAX
   ----        -----------                -----        ---            ---
   scTime      TRMM clock time in CCSDS         ** see NOTES below **
               unsegmented time code 
	       format (CUC).

OUTPUTS:
   NAME        DESCRIPTION                UNITS        MIN            MAX
   ----        -----------	          -----        ---            ---
   asciiUTC    UTC in CCSDS ASCII         ASCII        1993-01-01     see NOTES
               time code A format

RETURNS:
   PGS_S_SUCCESS               successful execution
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

   TRMM TIME FORMAT:

     For TRMM, the output spacecraft clock time scTime is a 64-bit value in
     CCSDS unsegmented time code (CUC) format (Almost.  See Note below.) with an
     epoch defined (dynamically!) by TRMM (see Note below), which is comprised
     of two binary counter values:

     ("scTime" represents an array of 8 (unsigned) one byte elements)

     1)  A 32-bit value (the first four elements of array
         scTime each with 8 bits) containing the number of
         seconds since an epoch of 12AM UTC January 1, 1993. 
         The range of decimal values is 0-4294967295, computed as 
         256*256*256*element1 + 256*256*element2 + 256*element3 + element4. 
         The maximum decimal value of each element 1, 2, 3 and 4 is 255.
     2)  A 32-bit value (elements 5, 6, 7 and 8 of array scTime, each with 
         8 bits) containing the sub-seconds.
         The range of values is 0-4294967295 sub-seconds, computed as
         256*256*256*element3 + 256*256*element4 + 256*element5 + element6. 
         The maximum decimal value of each element 3,4,5 and 6 is 255.  The
         sub-seconds represent the fraction of the last second.  This fraction 
         is calculated as (sub-seconds)/(256^(# of sub-second elements)).

         This allows the s/c clock to represent times from 1993-01-01 to
         approximately 2129-02-07T06:26:24 (give or take a few seconds).

     Note: the CCSDS Blue Book on Time Code Formats explicitly states for CUC:
     "This time code IS NOT UTC-based and leap second corrections don no apply."
     The TRMM clock is actually a count of the number of seconds since the time
     card (hardware) was actually powered up (but only ostensibly).  This clock
     can and may indeed actually be "jammed" (reset at arbitrary times) from
     ground control.  TRMM keeps track of (separately of course) a Universal
     Time Correlation Factor (UTCF) which is an arbitrary number representing
     real seconds that must be added to the TRMM s/c clock in order to derive
     the time.  This number accounts for several things including time offset
     correction between power-up and the epoch time, drift in the s/c clock that
     has rendered it inaccurate (accuracy requirement for TRMM s/c clock is
     nearest millisecond) AND leap-second correction which means really that
     TRMM s/c clock is rendered directly into UTC (not TAI).  The formula to
     find UTC from TRMM s/c clock time is something like:

     daysSinceEpoch = (int) (scClockTime + UTCF)/86400;
     secondsOfDay = (double) (scClockTime + UTCF) - 86400*daysSinceEpoch;

     Then decipher daysSinceEpoch into the date and secondsOfDay into the time.
     That is, the above formula yields the UTC date and time.  This method of
     calculating the time will result in duplicate times around a leap second.

     The time as used by TRMM, therefore, basically IS a UTC based time.  This
     makes the s/c time format used by TRMM a variation of the CUC format (not
     supported by CCSDS).

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
     TRMM-490-137, "Tropical Rainfall Measuring Mission (TRMM) Telemetry And
     Command Handbook", 1994-02-21, Goddard Space Flight Center, Appendix D

REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   conversion method:

   The algorithm computes the real continuous seconds since 1/1/93 from the
   input time.  This number is then passed to PGS_TD_TAItoUTC() which converts
   seconds since 12AM 1/1/93 to a UTC ASCII time.

GLOBALS:
   None

FILES:
   This function calls PGS_TD_TAItoUTC() which requires the file:
   leapsec.dat

FUNCTIONS_CALLED:
        PGS_TD_TAItoUTC()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_TRMMtoUTC()"

PGSt_SMF_status
PGS_TD_TRMMtoUTC(                 /* converts TRMM s/c time to ASCII UTC */
    unsigned char   scTime[8],    /* TRMM s/c time in CCSDS unseg. time code */
    char            *asciiUTC)    /* equivalent UTC time in CCSDS ASCII Time
				     Code A format */
{
    PGSt_double     secTAI93;     /* TAI seconds since 12AM 1-1-1993 */
    PGSt_SMF_status returnStatus; /* return value of function calls */
    
    returnStatus = PGS_TD_TRMMtoTAI(scTime, &secTAI93);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	break;
	
      case PGSTD_E_UTCF_UNINITIALIZED:
      case PGS_E_TOOLKIT:
	return returnStatus;
	
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    return PGS_TD_TAItoUTC(secTAI93, asciiUTC);
}

/* notes:   

   critical before use:
   see WARNING in NOTES section above */
