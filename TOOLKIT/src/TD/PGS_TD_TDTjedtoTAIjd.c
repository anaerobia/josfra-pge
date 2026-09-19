/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_TDTjedtoTAIjd.c

DESCRIPTION:
   This file contains the function PGS_TD_TDTjedtoTAIjd().
   This function converts TDT Julian ephemeris date to TAI Julian date.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   06-Jun-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert TDT Julian Ephemeris Date to TAI Julian Date

NAME:
   PGS_TD_TDTjedtoTAIjd()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_double *
   PGS_TD_TDTjedtoTAIjd(
       PGSt_double  jedTDT[2],
       PGSt_double  jdTAI[2])

DESCRIPTION:
   This function converts TDT Julian ephemeris date to TAI Julian date.

INPUTS:
   Name         Description                 Units       Min   Max
   ----         -----------                 -----       ---   ---
   jedTDT       TDT Julian ephemeris date   days        N/A   N/A

OUTPUTS:
   Name         Description                 Units       Min   Max
   ----         -----------                 -----       ---   ---
   jdTAI        TAI Julian date             days        N/A   N/A
          
RETURNS:
   TAI Julian ephemeris date (address of jdTAI).

EXAMPLES:
C:
   What?  Are you serious?
  
NOTES:
   TIME ACRONYMS:

     TDT is: Terrestrial Dynamical Time
     TAI is: International Atomic Time

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
   PGSTK - ????, ????

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

#define SECONDSperDAY        86400.0    /* number of seconds in a day */

PGSt_double *
PGS_TD_TDTjedtoTAIjd(       /* converts TDT as a Julian date to TAI as a Julian
			       date */
    PGSt_double jedTDT[2],  /* TDT time as a Julian ephemeris date */
    PGSt_double jdTAI[2])   /* TAI time as a Julian date */
{
    /* get TAI from TDT by subtracting 32.184 seconds converted to days */

    jdTAI[0] = jedTDT[0];
    jdTAI[1] = jedTDT[1] - 32.184/SECONDSperDAY;
    
    /* make sure the fractional part of jdTAI is between 0.0 and 1.0
       i.e. [0,1) */

    if (jdTAI[1] < 0.0)
    {
	jdTAI[0] -= 1.0;
	jdTAI[1] += 1.0;
    }

    /* IMPORTANT: this test must not be in an else clause dependent on the
       failure of the above test.  Computer roundoff error may cause the above
       test to succeed and then set jdTAI to 1.0 which is undesireable. */

    if (jdTAI[1] >= 1.0)
    {
	jdTAI[0] += 1.0;
	jdTAI[1] -= 1.0;
    }

    return jdTAI;
}
