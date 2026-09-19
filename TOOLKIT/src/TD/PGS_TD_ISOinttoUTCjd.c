/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_ISOinttoUTCjd.c

DESCRIPTION:
   This file contains the function PGS_TD_ISOinttoUTCjd().
   The function converts UTC as an integer to UTC as a Julian date.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   06-Mar-1996  GTSK  Initial version.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert Integer UTC To UTC Julian Date

NAME:
   PGS_TD_ISOinttoUTCjd()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_ISOinttoUTCjd(
       PGSt_integer  intISO,
       PGSt_double   jdUTC[2])

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_isointtoutcjd(intiso,jdutc)
      integer          intiso
      double precision jdutc(2)

DESCRIPTION:
   The function converts UTC as an integer to UTC as a Julian date.

INPUTS:
   NAME      DESCRIPTION                UNITS        MIN        MAX
   ----      -----------		-----        ---        ---
   intISO    UTC as an integer           N/A         N/A        N/A
             (see NOTES)

OUTPUTS:	  
   NAME      DESCRIPTION                UNITS        MIN        MAX
   ----      -----------		-----        ---        ---
   jdUTC     UTC Julian date as two     days         ** see NOTES **
             real numbers, the first
	     a half integer number of
	     days and the second the
	     fraction of a day between
	     this half integer number
	     of days and the next half
	     integer day number.

RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_E_TIME_VALUE_ERROR    error in value of input integer UTC time

EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     UTC is: Universal Coordinated Time
			  
   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function are limited by the format of the input time.  The integer input
     time can represent times from 00:00 1-1-1950 to 23:00 12-31-2049.  The
     output range of the UTC Julian dates therefore corresponds to these times
     (JD 2433282.50 to JD 2469807.45833333).

   INTEGER UTC:

     Integer UTC is an integer representing UTC time to "hour" accuracy.

      The general format is:
 
          YYMMDDHH
 
          where:
              YY  represents the last two digits of the year (if YY is less than
	          50 the implied year is 20YY otherwise the implied year is
		  19YY).  Range is 00-99.

              MM  represents the month of the year.  Range is 01-12.

              DD  represents the day of the month.  Range is 01-31.

              HH  represents the hour of the day.  Range is 00-23.

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
   None

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* constants */

#define HOURSperDAY 24.0   /* number of hours in a day */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_ISOinttoUTCjd()"

PGSt_SMF_status
PGS_TD_ISOinttoUTCjd(            /* convert integer UTC to UTC Julian date */
    PGSt_integer  intISO,        /* UTC integer in "ISO" format: YYMMDDHH */
    PGSt_double   jdUTC[2])      /* UTC Julian date (toolkit format) */
{
    PGSt_integer  year;          /* integer representation of year value */
    PGSt_integer  month;         /* integer month of the year value */
    PGSt_integer  day;           /* integer day of the month value */
    PGSt_integer  hour;          /* integer hour of the day value */
    PGSt_integer  leapday;       /* 1 if leap year AND month=2, otherwise 0 */
    
    static PGSt_integer dim[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};

    PGSt_SMF_status returnStatus;
    
    returnStatus = PGS_S_SUCCESS;
    
    /* the input integer UTC time is in the format YYMMDDHH, decode this to the
       individual year, month, day and hour values (note that for year values
       greater than 49, 20th century is assumed, otherwise 21st century is
       assumed) */

    year = intISO/1000000;
    year = (year > 49) ? 1900 + year : 2000 + year;
    month = (intISO - year)/10000;
    day = (intISO - year - month)/100;
    hour = intISO%100;

    if (month > 12 || month < 1)
    {
	returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "invalid value in month field (i.e. 0 or > 12)",
			      FUNCTION_NAME);
    }

    if (day < 1 || day > 28)
    {
	leapday = (year%4 == 0) ? 1 : 0;
        leapday = (month == 2) ? leapday : 0;

	if (day < 1 || day > (dim[month]+leapday))
	{
	    returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	    PGS_SMF_SetDynamicMsg(returnStatus,
				  "invalid value in day field (i.e. 0 or > "
				  "maximum days for specified month)",
				  FUNCTION_NAME);
	}
    }
    
    if (hour > 23)
    {
	returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "invalid value in hour field (i.e. > 23)",
			      FUNCTION_NAME);
    }

    /* calculate the Julian day number:
       PGS_TD_julday() returns the integer number of the Julian day that begins
       on the input calendar year-month-day.  Since the Julian day begins at
       noon, this function returns the Julian day that begins at noon on the
       input day.  What is actually desired here is the Julian DATE of midnight
       of the input year-month-day which occurs half a day earlier than the
       Julian day returned by the function.  Therefore 0.5 is subtracted from
       the return value of PGS_TD_julday to get the appropriate answer. */

    jdUTC[0] = PGS_TD_julday(year, month, day) - 0.5;

    /* calculate the fraction of the day, this is the hour value (determined
       above) converted to units of "day" (i.e. this is the fraction of the
       current day) */

    jdUTC[1] = hour/HOURSperDAY;
    
    if (returnStatus == PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);
    }
    
    return returnStatus;
}
