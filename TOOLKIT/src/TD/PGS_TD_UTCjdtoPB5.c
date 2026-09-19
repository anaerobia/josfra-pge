/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UTCjdtoPB5.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCjdtoPB5().
   This function converts a UTC time in toolkit Julian date format to PB5 time
   format.
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.
   Peter D. Noerdlinger  / Applied Research Corp.

HISTORY:
   18-Jan-1995 GTSK  Initial version
   07-Oct-1995 GTSK  Replaced (erroneous) call to PGS_TD_JDtoMJD() with call
                     to PGS_TD_JDtoTJD()
   22-May-1997 PDN   Removed branches/references/return to PRED_LEAPS

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert Julian Date to PB5 Time Format

NAME:
   PGS_TD_UTCjdtoPB5()

SYNOPSIS:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCjdtoPB5(
      PGSt_double  jdUTC[2],
      PGSt_boolean onLeap,
      PGSt_scTime  timePB5[9])

DESCRIPTION:
   This function converts a UTC time in toolkit Julian date format to PB5 time
   format.

INPUTS:
   NAME      DESCRIPTION                  UNITS       MIN        MAX
   ----      -----------                  -----       ---        ---
   jdUTC     UTC as a Julian date         days        *see NOTES below

   onLeap    set to PGS_TRUE to indicate  T/F
             that the current time is
	     occurring during a leap
	     second, otherwise set to
	     PGS_FALSE
   	    
OUTPUTS:    
   NAME       DESCRIPTION                 UNITS   MIN   MAX
   ----       -----------                 -----   ---   ---
   timePB5    time in PB5 time format     *see NOTES below

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_NO_LEAP_SECS        leap seconds file not available
   PGSTD_E_DATE_OUT_OF_RANGE   input date is out of range of allowable PB5 times
   PGSTD_E_TIME_VALUE_ERROR    error in value of the ascii UTC time
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     TJD is:  Truncated Julian Date
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

   TRUNCATED JULIAN DATES:

     Truncated Julian dates follow the same conventions as those for Julian
     dates (above) EXCEPT that the truncated Julian day number is integral (NOT
     half-integral).  The truncated Julian date in any time stream has a day 
     number that is 2440000.5 days less than the Julian date day number and an
     identical day fraction (i.e. the truncated Julian date is a day count
     originating at 1968-05-24T00:00:00).

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:  

DETAILS:
   None
  
GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_JulianDateSplit()
   PGS_TD_JDtoTJD()
   PGS_TD_UTCjdtoTAIjd()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME  "PGS_TD_UTCjdtoPB5()"

PGSt_SMF_status
PGS_TD_UTCjdtoPB5(                /* convert Julian date to PB5 time format */
    PGSt_double     jdUTC[2],     /* UTC Julian date */
    PGSt_boolean    onLeap,       /* indicates if leap second is occurring */
    PGSt_scTime     timePB5[9])   /* UTC time in PB5 format */
{
    PGSt_integer    leapSecond=0; /* value of leap second (0 or 1) */

    PGSt_uinteger   intSeconds;   /* whole number of seconds of day */
    PGSt_uinteger   intMillisecs; /* whole number of millisecs of second */
    PGSt_uinteger   intMicrosecs; /* whole number or microsecs of millisec */
    
    PGSt_double     tjdUTC[2];    /* UTC truncated Julian date */
    PGSt_double     jdTAI[2];     /* TAI Julian date */
    PGSt_double     realSeconds;  /* real number of seconds of day */
    
    PGSt_SMF_status returnStatus; /* return value of this function */
    
    returnStatus = PGS_S_SUCCESS;
    
    /* convert Julian date to truncated Julian date */

    /* PGS_TD_JulianDateSplit() will ensure return tjdUTC in Toolkit Julian date
       format (see NOTES above).  The array tjdUTC is then given as both input
       AND output to PGS_TD_JDtoTJD() so its contents on input (Julian date)
       will be overwritten with the truncated Julian date which will be its
       contents upon returning from PGS_TD_JDtoTJD(). */

    PGS_TD_JulianDateSplit(jdUTC,tjdUTC);
    PGS_TD_JDtoTJD(tjdUTC,tjdUTC);
    
    /* the truncated Julian day must fit in 2 bytes so it cannot be larger than
       0xffff, it also cannot be less than 0.0 */

    if (tjdUTC[0] > (PGSt_double) 0xffff || tjdUTC[0] < 0.0)
    {
	PGS_SMF_SetStaticMsg(PGSTD_E_DATE_OUT_OF_RANGE,FUNCTION_NAME);
	return PGSTD_E_DATE_OUT_OF_RANGE;
    }
    
    /* If onLeap is set to PGS_TRUE then make sure that the given time is
       an appropriate time for a leap second.  The easy way to do this is to
       call the function PGS_TD_UTCjdtoTAIjd() and check for the return value
       PGSTD_E_TIME_VALUE_ERROR.  Continue processing if the error does not pose
       a problem to future calculations (that is the error will not cause a
       divide by zero or some other mathematical error to occur). */

    if (onLeap == PGS_TRUE)
    {
	returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC,PGS_TRUE,jdTAI);
	
	switch(returnStatus)
	{
	  case PGS_S_SUCCESS:
	  case PGSTD_E_NO_LEAP_SECS:
	    leapSecond = 1;
	  case PGSTD_E_TIME_VALUE_ERROR:
	    break;
	  case PGS_E_TOOLKIT:
	    return returnStatus;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
    }
    
    /* convert day fraction (tjdUTC[1]--see NOTES above) to seconds (a day
       consists of 86400 seconds) */

    realSeconds = tjdUTC[1]*86400.0;

    /* break the real seconds up into integer components of whole seconds in
       day, whole milliseconds of latest second and whole microseconds of latest
       millisecond */

    intSeconds = (PGSt_uinteger) realSeconds + leapSecond;
    intMillisecs = (PGSt_uinteger) (fmod(realSeconds,1.0)*1000.0);

    /* Note: fmod() proved unreliable for the following calculation */

    intMicrosecs = (PGSt_uinteger) 
                   ((realSeconds-intSeconds+leapSecond)*1.0E6)%1000;
    
    /* convert whole number values above to binary counter equivalents */

    timePB5[0] = (PGSt_scTime) ((PGSt_uinteger)tjdUTC[0]/256U);
    timePB5[1] = (PGSt_scTime) ((PGSt_uinteger)tjdUTC[0]%256U);
    
    timePB5[2] = (PGSt_scTime) ((intSeconds/65536U)%256U);
    timePB5[3] = (PGSt_scTime) ((intSeconds/256U)%256U);
    timePB5[4] = (PGSt_scTime) (intSeconds%256U);

    timePB5[5] = (PGSt_scTime) (intMillisecs/256U);
    timePB5[6] = (PGSt_scTime) (intMillisecs%256U);

    timePB5[7] = (PGSt_scTime) (intMicrosecs/256U);
    timePB5[8] = (PGSt_scTime) (intMicrosecs%256U);

    return returnStatus;
}

