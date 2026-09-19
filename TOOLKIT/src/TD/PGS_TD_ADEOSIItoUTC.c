/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_ADEOSIItoUTC.c

DESCRIPTION:
   This file contains the function PGS_TD_ADEOSIItoUTC().
   This function converts ADEOS II spacecraft clock time to UTC (in CCSDS ASCII
   Time Code A Format).
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
   26-Mar-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert ADEOS II Clock Time to UTC Time


NAME:
   PGS_TD_ADEOSIItoUTC()
 

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_ADEOSIItoUTC(
       PGSt_scTime  scTime[5],
       char         asciiUTC[28])

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_adeosiitoutc(sctime,asciiutc)
      character*5      sctime
      character*27     asciiutc

DESCRIPTION:
   This function converts ADEOS II spacecraft clock time to UTC (in CCSDS ASCII
   Time Code A Format).
 
INPUTS:
   NAME      DESCRIPTION
   ----      -----------
   scTime    ADEOS II clock time.  See NOTES below for detailed description as
             well as discussion of min and max values.

OUTPUTS:
   NAME      DESCRIPTION                UNITS    MIN                  MAX
   ----      -----------                -----    ---                  ---
   asciiUTC  UTC time in CCSDS ASCII    time     1961-01-01T00:00:00  see NOTES
             Time Code A format

RETURNS:
   PGS_S_SUCCESS                successful execution
   PGSTD_E_NO_LEAP_SECS         no leap seconds correction available for
                                input time
   PGSTD_E_TMDF_UNINITIALIZED   attempted access (get) of uninitialized TMDF
                                values
   PGS_E_TOOLKIT                an unexpected error occurred

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

   ADEOS-II TIME FORMAT:

     For EOS-AM, the input spacecraft (s/c) clock time scTime is a 40-bit value
     which is comprised of two binary counter values:
                 
     ("scTime" represents an array of 5 (unsigned) one byte elements)
          
     1)  Instrument Time - A 32-bit value (the first four elements of array
         scTime each with 8 bits containing a count of 1/32 seconds since an
	 arbitrary epoch.  The range of decimal values is 0-134217727.969,
	 computed as (256*256*256*element1+256*256*element2+256*element3+
	 element4)/32.0. The maximum decimal value of each of elements 1,2,3
	 and 4 is 255.

     2)  Pulse Time - A 4-bit value (the second 4 bits of element 5 of array
         scTime--the first 4 bits are ALWAYS set to 0) containing a count of
	 1/512 seconds.  The range of decimal values is 0.0-0.030 seconds
         computed as element5/512. The maximum decimal value of element 5 is 15.
       
     Note: this quantity is s/c clock time.  To derive an actual UTC time
     several other variables must be known.  These are the s/c clock reference
     value, the s/c clock period and the ground reference value.  These values
     are collectively referred to as the TMDF (Time Difference) values.

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
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_ADEOSIItoTAI()
   PGS_TD_UTCtoTAI()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_ADEOSIItoUTC()"

PGSt_SMF_status
PGS_TD_ADEOSIItoUTC(              /* converts ADEOSII s/c time to ASCII UTC */
    unsigned char   scTime[5],    /* ADEOS-II s/c time */
    char            *asciiUTC)    /* equivalent UTC time in CCSDS ASCII Time
				     Code A format */
{
    PGSt_double     secTAI93;     /* TAI seconds since 12AM 1-1-1993 */
    PGSt_SMF_status returnStatus; /* return value of function calls */
    
    returnStatus = PGS_TD_ADEOSIItoTAI(scTime, &secTAI93);
    if (returnStatus != PGS_S_SUCCESS)
    {
	return returnStatus;
    }
    
    return PGS_TD_TAItoUTC(secTAI93, asciiUTC);
}
