/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_PB5toTAI.c

DESCRIPTION:
   This file contains the function PGS_TD_PB5toTAI().
   This function converts a time in PB5 time format to TAI (Toolkit internal
   time).
 
AUTHOR:
   Tom W. Atwater / Applied Research Corp.
   Guru Tej S. Khalsa / Applied Research Corp.
   Peter D. Noerdlinger / Applied Research Corp.

HISTORY:
   18-Jan-1995 TWA  Initial version
   06-Jun-1995 GTSK Added prolog and comments
   21-May-1997 PDN  Fixed Comments
   22-May-1997 PDN  Removed branch for _W_PRED_LEAPS


END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert PB5 Time Format to TAI

NAME:
   PGS_TD_PB5toTAI()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_PB5toTAI(
      PGSt_scTime  timePB5[9],
      PGSt_double  &secTAI93)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_pb5totai(timepb5,sectai93)
      character*9      timepb5
      double precision sectai93

DESCRIPTION:
   This function converts a time in PB5 time format to TAI (Toolkit internal
   time).

INPUTS:
   NAME      DESCRIPTION                     UNITS       MIN      MAX
   ----      -----------                     -----       ---      ---
   timePB5   time in PB5 time format            *see NOTES below

OUTPUTS:    
   NAME      DESCRIPTION                     UNITS       MIN      MAX
   ----      -----------                     -----       ---      ---
   secTAI93  TAI (Toolkit internal time)     sec        *see NOTES below

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_SECONDS_TOO_BIG     seconds field too large (>=86401)
   PGSTD_E_MILSEC_TOO_BIG      millisecond field too large (>=1000)
   PGSTD_E_MICSEC_TOO_BIG      microsecond field too large (>=1000)
   PGSTD_E_NO_LEAP_SECS        leap seconds file not available
   PGSTD_E_TIME_VALUE_ERROR    error in value of the ascii UTC time
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

   PB5 TIME FORMAT:
    
     A 9 byte (72 bit) time format comprised of four binary counter values:

     ("timePB5" represents an array of 9 (unsigned) one byte elements)
          
     1)  A 16-bit value (the first two elements of array timePB5 each with 8 
	 bits) containing the truncated Julian date (integral day number).  The
         range of decimal values is 0-65535, computed as 256*element1+element2.
         The maximum decimal values of elements 1 and 2 are 255 and 255
         respectively.
     2)  A 24-bit value (elements 3, 4 and 5 of array timePB5, each with 8 bits)
         containing the seconds of the latest day.  The range of values is
         0-86401 seconds, computed as 256*256*element3+256*element4+element5.
     3)  A 16-bit value (elements 6 and 7 of array timePB5, each with 8 bits)
         containing the number of milliseconds of the latest second.  The range
	 of values is 0-999 milliseconds, computed as 256*element6+element7.
     4)  A 16-bit value (elements 8 and 9 of array timePB5, each with 8 bits)
         containing the number of microseconds of latest millisecond.  The range
         of values is 0-999 microseconds, computed as 256*element8+element9.
       
     This allows the PB5 format to represent times from 1968-05-24T00:00:00 to
     2147-10-28T23:59:59.999999.

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).


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


   JULIAN DATES:

     The Julian date is a day count originating at noon of Jan. 1st, 4713 B.C.

     Format:

       Toolkit Julian dates are kept as an array of two real (high precision)
       numbers (C: PGSt_double, FORTRAN: DOUBLE PRECISION).  The first element
       of the array should be the half integer Julian day (e.g. N.5 where N is a
       Julian day number).  The second element of the array should be a real
       number greater than or equal to zero AND less than one (1.0) representing
       the time of the current day (as a fraction of that (86400 second) day.
       This format allows relatively simple translation to calendar days (since
       the Julian days begin at noon of the corresponding calendar day).  Users
       of the Toolkit are encouraged to adhere to this format to maintain high
       accuracy (one number to track significant digits to the left of the
       decimal and one number to track significant digits to the right of the
       decimal).  Toolkit functions that do NOT require a Julian type date as an
       input and return a Julian date will return the Julian date in the above
       mentioned format.  Toolkit functions that require a Julian date as an
       input and do NOT return a Julian date will first convert (internally) the
       input date to the above format if necessary.  Toolkit functions that have
       a Julian date as both an input and an output will assume the input is in
       the above described format but will not check and the format of the
       output may not be what is expected if any other format is used for the
       input.

     Meaning:

       Toolkit "Julian dates" are all derived from UTC Julian Dates.  A Julian
       date in any other time stream (e.g. TAI, TDT, UT1, etc.) is the UTC
       Julian date plus the known difference of the other stream from UTC
       (differences range in magnitude from 0 seconds to about a minute).

       Examples:

         In the following examples, all Julian Dates are expressed in Toolkit
         standard form as two double precision numbers. For display here, the
         two members of the array are enclosed in braces {} and separated by a
         comma.

         A) UTC to TAI Julian dates conversion

         The Toolkit UTC Julian date for 1994-02-01T12:00:00 is: 
         {2449384.50, 0.5}.
         TAI-UTC at 1994-02-01T12:00:00 is 28 seconds (.00032407407407 days). 
         The Toolkit TAI Julian date for 1994-02-01T12:00:00 is:
         {2449384.50, 0.5 + .00032407407407} = {2449384.50, 0.50032407407407}

         Note that the Julian day numbers in UTC and the target time stream may
         be different by + or - 1 for times near midnight:

         B) UTC to UT1 Julian dates conversion

         The Toolkit UTC Julian date for 1994-04-10T00:00:00 is: 
         {2449452.50, 0.0}.
         UT1-UTC at 1994-04-10T00:00:00 is -.04296 seconds 
         (-0.00000049722221 days).  The Toolkit UT1 Julian date for
         1994-04-10T00:00:00 is:
         {2449452.50, 0.0 - 0.0000004972222} = 
         {2449452.50, -0.0000004972222} =
         {2449451.50, 0.9999995027778}

   MODIFIED JULIAN DATES:

     Modified Julian dates follow the same conventions as those for Julian
     dates (above) EXCEPT that the modified Julian day number is integral (NOT
     half-integral).  The modified Julian date in any time stream has a day 
     number that is 2400000.5 days less than the Julian date day number and an
     identical day fraction (i.e. the modified Julian date is a day count
     originating at 1858-11-17T00:00:00).


   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:  

DETAILS:
   None
  
GLOBALS:
   None

FILES:
   The file "leapsec.dat" is accessed by  PGS_TD_UTCjdtoTAIjd()

FUNCTIONS_CALLED:
   PGS_TD_PB5toUTCjd()
   PGS_TD_UTCjdtoTAIjd()
   PGS_TD_TAIjdtoTAI()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_PB5toTAI()"

PGSt_SMF_status
PGS_TD_PB5toTAI(                  /* Converts PB5 format to TAI */
    PGSt_scTime     timePB5[9],   /* UTC time in PB5 format */
    PGSt_double     *secTAI93)    /* TAI time in seconds */
{
    PGSt_double  jdUTC[2];        /* UTC Julian date */
    PGSt_double  jdTAI[2];        /* TAI Julian date */
    PGSt_boolean onLeap;          /* T/F value - true if input time indicates a
				     leap second is occurring */

    PGSt_SMF_status returnStatus; /* return value of this function */

    /* convert input PB5 time to equivalent UTC Julian date */

    returnStatus = PGS_TD_PB5toUTCjd( timePB5, jdUTC );

    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	onLeap = PGS_FALSE;
	break;
      case PGSTD_M_LEAP_SEC_IGNORED:
	onLeap = PGS_TRUE;
	break;
      default:
	return returnStatus;
    }
    
    /* convert UTC Julian date to equivalent TAI Julian date */

    returnStatus = PGS_TD_UTCjdtoTAIjd( jdUTC, onLeap, jdTAI );

    /* convert TAI Julian date to TAI */

    *secTAI93 = PGS_TD_TAIjdtoTAI( jdTAI );
    
    return returnStatus;
}
