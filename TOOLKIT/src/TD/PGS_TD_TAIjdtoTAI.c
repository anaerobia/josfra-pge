/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_TAIjdtoTAI.c

DESCRIPTION:
   This file contains the function PGS_TD_TAIjdtoTAI().
   This function converts TAI Julian date to time in TAI seconds since
   12 AM UTC 1-1-1993.

AUTHOR:
   Guru Tej S. Khalsa

HISTORY:
   14-Dec-1994  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert TAI Julian Date to Toolkit Internal Time

NAME:
   PGS_TD_TAIjdtoTAI()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_double
   PGS_TD_TAIjdtoTAI(
       PGSt_double  jdTAI[2])

 FORTRAN:
      double precision function pgs_td_taijdtotai(jdtai)
      double precision jdtai(2)

DESCRIPTION:
   This function converts TAI Julian date to time in TAI seconds since
   12 AM UTC 1-1-1993.

INPUTS:
   Name         Description               Units       Min   Max
   ----         -----------               -----       ---   ---
   jdTAI        TAI Julian date           days        ANY   ANY

OUTPUTS:
   None
          
RETURNS:
   Toolkit internal time (seconds since 12 AM UTC 1-1-1993).

EXAMPLES:
C:
   PGSt_double      secTAI93;
   PGSt_double      jdTAI[2];

   jdTAI[0] = 2448989.5;
   jdTAI[1] = 0.0003125;

   secTAI93 = PGS_TD_TAItoTAIjd(secTAI93);
    
   ** secTAI93 should now have the value: 86400.**

FORTRAN:
      double precision pgs_td_taitotaijd

      double precision sectai93
      double precision jdtai
 
      jdtai(1) = 2448989.5d0;
      jdtai(2) = 0.0003125d0;

      sectai93 = pgs_td_taijdtotai(jdtai)

      ! sectai93 should now have the value: 86400.

NOTES:
   TAI is: International Atomic Time

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

   JULIAN DATES:

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
   PGSTK - 1220, 1160, 1170

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* constants */

#define EPOCH_DAY            2448988.5  /* TAI Julian day of 0 hrs UTC 1-1-93 */
#define EPOCH_DAY_FRACTION   0.0003125  /* TAI Julian day fraction 0 hrs UTC
					   1-1-1993 */
#define SECONDSperDAY        86400.0    /* number of seconds in a day */
    
PGSt_double
PGS_TD_TAIjdtoTAI(          /* converts TAI as a Julian date to TAI as seconds
			       since 12AM UTC 1-1-93 */
    PGSt_double jdTAI[2])   /* TAI time as a Julian date */
{
    /* calculate the seconds since epoch.  The Julian day portions and the
       fractional day portions are calculated separately.  This is because the
       Julian day numbers can be so big that there is a loss of accuracy in the
       day fraction if the two numbers are added together.  The desired accuracy
       is microseconds which is aprx. 10^-12 day.  This is within a few decimal
       places of the limit on the accuracy of a "double" variable.  Maximum 
       Julian days of only 10^3 or so could be allowed if the day and day
       fraction were added together first and this wouldn't even get the dates
       out of the B.C. range!  By taking the differences of the big numbers
       first and multiplying each of the differences by the appropriate scaling
       factor before adding them, much better accuracy is achieved. */

    /* basically this is the difference between the TAI Julian Date of the input
       time (julianDayNum+dayFraction+leapSec) and the TAI Julian Date of the
       epoch time (EPOCH_DAY + EPOCH_DAY_FRACTION).  The epoch time is
       1993-01-01T00:00:00.000000 UTC */

    return (jdTAI[0] - EPOCH_DAY)*SECONDSperDAY +
           (jdTAI[1] - EPOCH_DAY_FRACTION)*SECONDSperDAY;
}
