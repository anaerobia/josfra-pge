/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_PB5CtoUTCjd.c

DESCRIPTION:
   This file contains the function PGS_TD_PB5CtoUTCjd().
   This function converts a time in PB5C time format to UTC time in toolkit 
   Julian date format.
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
   18-Mar-1996 GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert PB5C Time Format to Julian Date

NAME:
   PGS_TD_PB5CtoUTCjd()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_PB5CtoUTCjd(
      PGSt_scTime  timePB5C[9],
      PGSt_double  jdUTC[2])

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_pb5ctoutcjd(timepb5,jdutc)
      character*7      timepb5
      double precision jdutc(2)

DESCRIPTION:
   This function converts a time in PB5C time format to UTC time in toolkit 
   Julian date format.

INPUTS:
   NAME       DESCRIPTION                 UNITS       MIN      MAX
   ----       -----------                 -----       ---      ---
   timePB5C   time in PB5C time format         *see NOTES below

OUTPUTS:    
   NAME      DESCRIPTION                  UNITS       MIN      MAX
   ----      -----------                  -----       ---      ---
   jdUTC     UTC as a Julian date         days        *see NOTES below

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_M_LEAP_SEC_IGNORED    leap second portion of input time discarded
   PGSTD_E_SECONDS_TOO_BIG     seconds field too large (>=86401)
   PGSTD_E_MILSEC_TOO_BIG      millisecond field too large (>=1000)
   PGSTD_E_MICSEC_TOO_BIG      microsecond field too large (>=1000)
   PGSTD_E_NO_LEAP_SECS        leap seconds file not available
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
   PGS_TD_TJDtoJD()
   PGS_TD_UTCjdtoTAIjd()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>
#include <math.h>

extern PGSt_integer PGS_SMF_FEQ(PGSt_double, PGSt_double);

/* constants */

#define EPOCH_DATE     2450000.50

/* name of this function */

#define FUNCTION_NAME  "PGS_TD_PB5CtoUTCjd()"

PGSt_SMF_status
PGS_TD_PB5CtoUTCjd(               /* convert PB5C time format to Julian date */
    PGSt_scTime     timePB5C[9],  /* UTC time in PB5C format */
    PGSt_double     jdUTC[2])     /* UTC Julian date */
{
    PGSt_boolean    onLeap;       /* indicates if leap second is occurring */

    PGSt_double     jdTAI[2];     /* TAI Julian date */
    PGSt_double     seconds;      /* seconds of day */
    PGSt_double     millisecs;    /* millisecond of second */
    PGSt_double     microsecs;    /* microseconds of millisecond */
    
    PGSt_SMF_status returnStatus; /* return value of this function */
    
    returnStatus = PGS_S_SUCCESS;
    onLeap = PGS_FALSE;
    
    jdUTC[0] = ((timePB5C[0]%128)*128U) + timePB5C[1]/2U;
    jdUTC[0] = jdUTC[0] + EPOCH_DATE;
    
    seconds = ((timePB5C[1]%2)*256U + timePB5C[2])*256U + timePB5C[3];
    millisecs = (timePB5C[4]*4U) + timePB5C[5]/64U;
    microsecs = ((timePB5C[5]%64U)*16U) + timePB5C[6]/16U;

    if (seconds >= 86400.0)
    {
	/* Size of seconds should be less than (a day + 1 second) since this
	   value represents seconds of the day in the day field its resolution
	   should not be as large as a day (plus a second in case of leap
	   seconds) */

	if (PGS_SMF_FEQ(seconds, 86400.0))
	{
	    seconds -= 1.0;
	    onLeap = PGS_TRUE;
	}
	else
	{
	    PGS_SMF_SetStaticMsg(PGSTD_E_SECONDS_TOO_BIG,FUNCTION_NAME);
	    return PGSTD_E_SECONDS_TOO_BIG;
	}
    }
    
    if (millisecs >= 1000.0)
    {
	/* Size of millisecs is equal to or larger than a second, milliseconds
	   field should be less than 1000 (which is an entire second and
	   should be accounted for in the seconds field) */ 

	PGS_SMF_SetDynamicMsg(PGSTD_E_MILSEC_TOO_BIG,
			      "millisecond field too large, should be < 1000",
			      FUNCTION_NAME);
	return PGSTD_E_MILSEC_TOO_BIG;
    }
    if (microsecs >= 1000.0)
    {
	/* Size of microsecs is equal to or larger than a millisec, microseconds
	   field should be less than 1000 (which is an entire millisec and
	   should be accounted for in the milliseconds field) */ 

	PGS_SMF_SetStaticMsg(PGSTD_E_MICSEC_TOO_BIG,FUNCTION_NAME);
	return PGSTD_E_MICSEC_TOO_BIG;
    }

    /* jdUTC[1] represents the day fraction.  This is the total real number of
       seconds of the day, therefore it is the sum of the various fields
       calculated below (each piece is converted to seconds). */

    jdUTC[1] = seconds + millisecs/1.0E3 + microsecs/1.0E6;

    /* The units of jdUTC is days so convert seconds to days (a day consists of
       86400 seconds). */

    jdUTC[1] = jdUTC[1]/86400.0;
    
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
	    PGS_SMF_SetStaticMsg(PGSTD_M_LEAP_SEC_IGNORED,FUNCTION_NAME);
	    return PGSTD_M_LEAP_SEC_IGNORED;
	  case PGSTD_E_TIME_VALUE_ERROR:
	    PGS_SMF_SetStaticMsg(PGSTD_E_SECONDS_TOO_BIG,FUNCTION_NAME);
	    return PGSTD_E_SECONDS_TOO_BIG;
	  case PGS_E_TOOLKIT:
	    return returnStatus;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
    }

    return PGS_S_SUCCESS;
}
