/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_TAItoUTCjd.c

DESCRIPTION:
  This file contains the function PGS_TD_TAItoUTCjd()

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  15-Sep-1994  GTSK  Initial version
  06-Jun-1994  GTSK  Complete rewrite, replaced redundant code with calls to
                     appropriate lower level functions.
  21-May-1997  PDN   Fixed Comments 

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Convert TAI to UTC Julian date

NAME:     
   PGS_TD_TAItoUTCjd()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_TAItoUTCjd(
       PGSt_double  secTAI93,
       PGSt_double  jdUTC[2])

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'
   
      integer function pgs_td_taitoutc(sectai93,jdutc)
      double precision  sectai93
      double precision  jdutc(2)


DESCRIPTION:
   This tool converts continuous seconds since 12AM UTC 1-1-93 to UTC time as
   a Julian date.

INPUTS:
   Name       Description                Units    Min                Max
   ----       -----------                -----    ---                ---
   secTAI93   continuous seconds since   seconds  -1009843225.577182 see NOTES
              12AM UTC Jan 1, 1993

OUTPUTS:
   Name       Description                Units    Min           Max
   ----       -----------                -----    ---           ---
   jdUTC      UTC Julian date as two     days     N/A           N/A
              real numbers, the first
              a half integer number of
              days and the second the
              fraction of a day between
              this half integer number
              of days and the next half
              integer day number.

RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_M_LEAP_SEC_IGNORED    leap second portion of input time discarded
   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for
                               input time
   PGS_E_TOOLKIT               something radically wrong occurred

EXAMPLES:
   None

NOTES:
   TIME ACRONYMS:
     
     TAI is: International Atomic Time
     UTC is: Universal Coordinated Time
			  
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

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

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

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
   PGSTK - 1170, 1210

DETAILS:
   See files PGS_TD_TAItoTAIjd.c and PGS_TD_TAIjdtoUTCjd.c

GLOBALS:  
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_TAItoTAIjd()
   PGS_TD_TAIjdtoUTCjd()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

PGSt_SMF_status
PGS_TD_TAItoUTCjd(
    PGSt_double     secTAI93,
    PGSt_double     jdUTC[2])
{
    /* convert secTAI93 (real seconds since UTC 12 AM 1-1-1993) to the equivalent
       TAI Julian date, then convert this TAI Julian date to UTC julian date */

    PGS_TD_TAItoTAIjd(secTAI93,jdUTC);
    return PGS_TD_TAIjdtoUTCjd(jdUTC,jdUTC);
}
