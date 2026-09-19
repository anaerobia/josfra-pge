/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UTCjdtoPB5C.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCjdtoPB5C().
   This function converts a UTC time in toolkit Julian date format to PB5C time
   format.
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
   18-Mar-1996 GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert Julian Date to PB5C Time Format

NAME:
   PGS_TD_UTCjdtoPB5C()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCjdtoPB5C(
      PGSt_double  jdUTC[2],
      PGSt_boolean onLeap,
      PGSt_scTime  timePB5C[7])

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_utcjdtopb5c(jdutc,onleap,timepb5)
      double precision jdutc(2)
      integer          onleap
      character*7      timepb5


DESCRIPTION:
   This function converts a UTC time in toolkit Julian date format to PB5C time
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
   timePB5C   time in PB5C time format     *see NOTES below

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_NO_LEAP_SECS        leap seconds file not available
   PGSTD_E_DATE_OUT_OF_RANGE   input date is out of range of allowable PB5C
                               times
   PGSTD_E_TIME_VALUE_ERROR    error in value of the ascii UTC time
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     UTC is:  Coordinated Universal Time

   PB5C TIME FORMAT:
    
     A 7 byte (56 bit) time format comprised of six fields:

     ("timePB5C" represents an array of 9 (unsigned) one byte elements)
          
     1)  "Flag bit" - A 1-bit value, always set to 1.
     2)  "Truncated Julian Day" - A 14-bit This is a binary counter with
         allowable values of 0-9999.  This value represents days since the
	 latest Julian day epoch (as of 6-13-96 this epoch was 10-10-95).  The
	 range is a approximately 27.379 years.
     3)  "Seconds of Day" - A 17-bit binary counter value representing seconds
         of the latest day.  The range is 0-86400 (the toolkit allows the
	 seconds field to run up to 86400 during a leap-second occurrence).
     4)  "Milliseconds of a Second" - A 10-bit binary counter value representing
         milliseconds of the latest second.  The range is 0-999.
     5)  "Microseconds of a Millisecond" - A 10-bit binary counter value
         representing microseconds of the latest millisecond.  The range is
	 0-999.
     6)  "Fill/Spare" - A 4-bit value, always 0 (this field reserved for future
         use).

     This allows the PB5C format to represent times over a 27.379 year range.
     The actual dates depend on the reference Julian day epoch.
     Warning: the use of the term Truncated Julian Day here is different from
     the general definition of Truncated Julian Date used elsewhere in the
     toolkit.

     Reference for this time format is:
     510-ICD-EDOS/EGS 1-19-1996 Table 8.1.2.4-1
     (Interface control document Between The Earth Observing System (EOS) Data
     and Operations System (EDOS) and the EOS Ground System (EGS) Elements CDRL
     B301)

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

/* constants */

#define EPOCH_DATE     2450000.50

/* name of this function */

#define FUNCTION_NAME  "PGS_TD_UTCjdtoPB5C()"

PGSt_SMF_status
PGS_TD_UTCjdtoPB5C(               /* convert Julian date to PB5C time format */
    PGSt_double     jdUTC[2],     /* UTC Julian date */
    PGSt_boolean    onLeap,       /* indicates if leap second is occurring */
    PGSt_scTime     timePB5C[7])  /* UTC time in PB5C format */
{
    PGSt_integer    leapSecond=0; /* value of leap second (0 or 1) */

    PGSt_uinteger   intSeconds;   /* whole number of seconds of day */
    PGSt_uinteger   intMillisecs; /* whole number of millisecs of second */
    PGSt_uinteger   intMicrosecs; /* whole number or microsecs of millisec */
    
    PGSt_double     tjdUTC[2];    /* UTC "truncated" Julian date */
    PGSt_double     jdTAI[2];     /* TAI Julian date */
    PGSt_double     realSeconds;  /* real number of seconds of day */
    
    PGSt_SMF_status returnStatus; /* return value of this function */
    
    returnStatus = PGS_S_SUCCESS;
    
    /* convert Julian date to a "truncated" Julian date */

    /* PGS_TD_JulianDateSplit() will ensure return tjdUTC in Toolkit Julian date
       format (see NOTES above).  The day value of tjdUTC is then adjusted so
       that this truncated value of the Julian date is referenced to the
       current epoch for this format (October 10, 1995 -- valid for 27.379
       years -- see comment below for reference). */

    PGS_TD_JulianDateSplit(jdUTC, tjdUTC);

    tjdUTC[0] -= EPOCH_DATE;
    
    /* this "truncated" Julian date may have values ranging from 0.0 to 9999.0 
       (reference: 510-ICD-EDOS/EGS 1-19-1996 Table 8.1.2.4-1 item 2) */

    if (tjdUTC[0] > 9999.0 || tjdUTC[0] < 0.0)
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

    timePB5C[0] = (PGSt_scTime) ((PGSt_uinteger)tjdUTC[0]/128U);
    timePB5C[0] = timePB5C[0] | 128;
    timePB5C[1] = (PGSt_scTime) ((PGSt_uinteger)tjdUTC[0]%128U);
    timePB5C[1] = timePB5C[1] << 1;
    
    if (intSeconds >= 0x10000)
    {
	timePB5C[1] = timePB5C[1] | 1;
	intSeconds -= 0x10000;
    }
    timePB5C[2] = (PGSt_scTime) (intSeconds/256U);
    timePB5C[3] = (PGSt_scTime) (intSeconds%256U);

    timePB5C[4] = (PGSt_scTime) (intMillisecs/4U);
    timePB5C[5] = (PGSt_scTime) (intMillisecs%4);
    timePB5C[5] = timePB5C[5] << 6;
    
    timePB5C[6] = (PGSt_scTime) (intMicrosecs/16U);
    timePB5C[5] = timePB5C[5] | timePB5C[6];
    timePB5C[6] = (PGSt_scTime) (intMicrosecs%16U);
    timePB5C[6] = timePB5C[6] << 4;

    return returnStatus;
}
