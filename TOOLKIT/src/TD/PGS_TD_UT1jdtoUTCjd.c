/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UT1jdtoUTCjd.c

DESCRIPTION:
   This file contains the function PGS_TD_UT1jdtoUTCjd()

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   14-Dec-1995  GTSK  Initial version
   21-May-1997  PDN   Fixed NOTES

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Convert UT1 Julian date to UTC Julian date

NAME:     
   PGS_TD_UT1jdtoUTCjd()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UT1jdtoUTCjd(
       PGSt_double  jdUT1[2],
       PGSt_double  jdUTC[2])

 FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
   
      integer function pgs_td_ut1jdtoutcjd(jdut1,jdutc)
      double precision  jdut1(2)
      double precision  jdutc(2)


DESCRIPTION:
   This tool converts UT1 time as a Julian date to UTC time as a Julian date.

INPUTS:
   Name         Description                Units    Min           Max
   ----         -----------                -----    ---           ---
   jdUT1        UT1 Julian date as two     days      ** see NOTES **
                real numbers, the first
		a half integer number of
		days and the second the
		fraction of a day between
		this half integer number
		of days and the next half
		integer day number.

OUTPUTS:
   Name         Description                Units    Min           Max
   ----         -----------                -----    ---           ---
   jdUTC        UTC Julian date as two     days      ** see NOTES **
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
   PGSCSC_W_PREDICTED_UT1      status of UT1-UTC correction is predicted
   PGSTD_E_NO_UT1_VALUE        no UT1-UTC correction available
   PGS_E_TOOLKIT               something radically wrong occurred

EXAMPLES:
   None

NOTES:

   TIME ACRONYMS:
     
     MJD is:  Modified Julian Date
     TJD is:  Truncated Julian Date
     TAI is:  International Atomic Time
     UT1 is:  Universal Time
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


   TIME BOUNDARIES for TRANSFORMATIONS BETWEEN UT1 and OTHER TIMES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file utcpole.dat which relates UT1 - UTC values to
     UTC dates.  The file "utcpole.dat" starts at Jan 1, 1972; therefore,
     the correction UT1 - UTC, which can be as large as (+-) 0.9 s, is not
     available until that date, and an error status message will be returned if
     this function is called with a time before that date.  The file
     utcpole.dat, which is maintained periodically, contains final (definitive),
     and predicted values (partially reduced data and short term predictions of 
     good accuracy).  The predictions run about a year ahead of real time. By
     that time, the error in UT1 is generally equivalent to 10 to 12 meters of
     equivalent Earth surface motion. Thus, when the present function is used,
     users should carefully check the return status.  A success status 
     message will be returned if all input times correspond to final values.
     A warning status message will be returned if predicted values are
     encountered.  An error message will be returned if the time requested is
     beyond the end of the predictions, or the file cannot be read.

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

   TRUNCATED JULIAN DATES:

     Truncated Julian dates follow the same conventions as those for Julian
     dates (above) EXCEPT that the truncated Julian day number is integral (NOT
     half-integral).  The truncated Julian date in any time stream has a day 
     number that is 2440000.5 days less than the Julian date day number and an
     identical day fraction (i.e. the truncated Julian date is a day count
     originating at 1968-05-24T00:00:00).

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

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

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

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac


REQUIREMENTS:
   PGSTK - 1170, 1210

DETAILS:
   This tool converts UT1 time to UTC time.  Input is UT1 Julian date.  Then
   PGS_CSC_UTC_UT1Pole() is called which reads the file utcpole.dat and gets the
   value in seconds of UT1 - UTC for the input time. The function
   PGS_CSC_UTC_UT1Pole() assumes an input of UTC Julian date, and is being
   called with UT1 Julian date so this is really just a 1st guess (UT1 and UTC
   are so close in time difference that to our accuracy (1.0E-6 seconds) the
   value of UT1 - UTC calculated will be the same for a given UT1 time or the
   corresponding UTC time--UNLESS a leap second has occurred).  The UTC Julian
   date is then assumed to be the UT1 julian date less the value of UT1-UTC.
   This new new value must then be checked to see if the adjustment yields a UTC
   value which is occurring in the last second of the day.  If this is the case
   then the UTC time is a candidate time for a leap second occurrence and
   PGS_CSC_UTC_UT1Pole must again be called with the value of UTC just
   determined (UT1 Julian date less the approximated value of UT1 - UTC).  If
   the value of UT1 - UTC thus determined varies significantly from the value
   determined with the first guess then a leap second must have occurred (here
   significant means above the Toolkit tolerance of 1.0E-6 seconds, although the
   difference will likely always be several tenths of a second).  Just before a
   leap second the value of UT1 - UTC will be relatively large, just after the
   leap second the value of UT1 - UTC will be relatively small (since by
   definition leap seconds are introduced to keep UTC in sync with UT1).
   Therefore IF the value of UT1 - UTC determined from our first guess at it
   (using UT1) is significantly different (i.e. within our tolerance of 1.0E-6
   seconds) from the value determined by using UTC, a leap second must have
   occurred.

GLOBALS: None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_CSC_UTC_UT1Pole()
   PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME   "PGS_TD_UT1jdtoUTCjd()"

/* constants */

#define SECONDSperDAY 86400.0           /* seconds per day */
#define CRITICAL_TIME .9999884259259259 /* last second of the day, during which
					   a leap second may be occurring
					   (i.e. this is the time 23:59:59
					   expressed as a fraction of a day) */

PGSt_SMF_status
PGS_TD_UT1jdtoUTCjd(              /* convert UT1 Julian day to UTC Julian day */
    PGSt_double     jdUT1[2],     /* UT1 in SDP Toolkit Julian day format */
    PGSt_double     jdUTC[2])     /* UTC in SDP Toolkit Julian day format */
{
    PGSt_double     xpole;        /* dummy variable (not used) */
    PGSt_double     ypole;        /* dummy variable (not used) */
    PGSt_double     diffUT1UTC_A; /* UT1 - UTC value (first guess value) */
    PGSt_double     diffUT1UTC_B; /* UT1 - UTC value (verification value) */
    PGSt_double     jdtable;      /* dummy variable (not used) */ 
    
    PGSt_SMF_status returnStatus; /* value returned by this function */
    
    /* initialize diffUT1UTC_A and diffUT1UTC_B to avoid any out of bounds
       errors in case the calls to PGS_CSC_UTC_UT1Pole() are not successful
       (since the return values from those calls are not checked herein) */

    diffUT1UTC_A = 0.0;
    diffUT1UTC_B = 0.0;
    
    /* Call PGS_CSC_UTC_UT1Pole() with UT1 as input.  This is a first guess
       where UT1 is used to approximate the value of UTC.  The value of
       UT1 - UTC calculated in PGS_CSC_UTC_UT1Pole() depends on the input time.
       UT1 and UTC are so close in time difference that to our accuracy
       (1.0E-6 seconds) the value of UT1 - UTC calculated will be the same for
       a given UT1 time or the corresponding UTC time.  UNLESS a leap second
       has occurred, an event which is checked for below. */

    returnStatus = PGS_CSC_UTC_UT1Pole(jdUT1, &xpole, &ypole, &diffUT1UTC_A,
				       &jdtable);

    /* Clearly: UTC = UT1 - (UT1 - UTC) */

    jdUTC[0] = jdUT1[0];
    jdUTC[1] = jdUT1[1] - diffUT1UTC_A/SECONDSperDAY;
    
    /* Make sure that jdUTC[1] is always less than 1.0 and not less than 0.0
       (see JULIAN DATES in NOTES section of function prolog above). */

    if (jdUTC[1] < 0.0)
    {
	jdUTC[1] += 1.0;
	jdUTC[0] -= 1.0;
    }

    /* IMPORTANT: this test must NOT be in an else clause dependent on the
       failure of the above test.  Computer roundoff error may cause the above
       test to succeed and then set jdUTC to 1.0 which is undesirable (this can
       happen if the value of jdUTC[1] is a VERY small negative number). */
 
    if (jdUTC[1] >= 1.0)
    {
	jdUTC[1] -= 1.0;
	jdUTC[0] += 1.0;
    }
    
    /* If the UTC time is occurring in the last second of the day, then it is a
       candidate for a leap second time. */

    if (jdUTC[1] >= CRITICAL_TIME)
    {
	/* Get the value of UT1 - UTC with the value of UTC (just determined)
	   as input. */

	returnStatus = PGS_CSC_UTC_UT1Pole(jdUTC, &xpole, &ypole, &diffUT1UTC_B,
					   &jdtable);

	/* Just before a leap second the value of UT1 - UTC will be relatively
	   large, just after the leap second the value of UT1 - UTC will be
	   relatively small (since by definition leap seconds are introduced to
	   keep UTC in sync with UT1).  Therefore IF the value of UT1 - UTC
	   determined from our first guess at it (using UT1) is significantly
	   different (i.e. within our tolerance of 1.0E-6 seconds) from the
	   value determined by using UTC, a leap second must have occurred. */

	if (fabs(diffUT1UTC_B-diffUT1UTC_A) >= 1.0E-6)
	{
	    PGS_SMF_SetStaticMsg(PGSTD_M_LEAP_SEC_IGNORED, FUNCTION_NAME);
	    return PGSTD_M_LEAP_SEC_IGNORED;
	}
    }
    
    return returnStatus;
}
