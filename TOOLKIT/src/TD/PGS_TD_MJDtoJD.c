/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_MJDtoJD.c

DESCRIPTION:
   This file contains the function PGS_TD_MJDtoJD().
   This function converts a modified Julian date to a Julian date.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   19-Jan-1994  GTSK  Initial version
   24-Apr-1995  GTSK  Oops!  JD-MJD had been entered as 2440000.5.  The actual
                      value of JD-MJD is 2400000.5.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert Modified Julian Date To Julian Date

NAME:
   PGS_TD_MJDtoJD()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_double *
   PGS_TD_MJDtoJD(
       PGSt_double  modifiedJulianDate[2],
       PGSt_double  julianDate[2])

DESCRIPTION:
   This function converts a modified Julian date to a Julian date.

INPUTS:
   Name                  Description               Units       Min   Max
   ----                  -----------               -----       ---   ---
   modifiedJulianDate    modified Julian date      days        N/A   N/A

OUTPUTS:
   Name                  Description               Units       Min   Max
   ----                  -----------               -----       ---   ---
   julianDate            Julian date               days        N/A   N/A

RETURNS:
   Julian date (address of julianDate).

EXAMPLES:
C:
   What?  Are you serious?
  
NOTES:
   TIME ACRONYMS:
     TJD is:  Truncated Julian Date
     UTC is:  Coordinated Universal Time
     
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

PGSt_double *
PGS_TD_MJDtoJD(
    PGSt_double    modifiedJulianDate[2],
    PGSt_double    julianDate[2])
{
    /* The Julian date is just the modified Julian date plus 2400000.5. */

    /* Since the toolkit Julian date format stores the Julian date as a half
       integral number in the first component of the Julian date array, this
       is the only component that needs to be modified.  The modified Julian
       date input should have an integer number for the first component (that
       is the first number will be an integer "number" NOT an integer "type").
       The second component will be the same as for the Julian date. */

    julianDate[0] = modifiedJulianDate[0] + 2400000.5;
    julianDate[1] = modifiedJulianDate[1];
    
    return julianDate;
}
