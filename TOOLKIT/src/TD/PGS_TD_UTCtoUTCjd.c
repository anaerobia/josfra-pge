/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_UTCtoUTCjd.c

DESCRIPTION:
  This file contains the function PGS_TD_UTCtoUTCjd()

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation
  Deborah Foch / Applied Research Corporation
  Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
      Mar-1994  GTSK    Initial version
      Mar-1994  DF/PDN  Modifications in notation and comments
   20-Jul-1994  GTSK    Changed name from PGS_TD_UtcAsciiToDouble
                        to PGS_TD_UTCtoUTCjd.  Added ability to
			accept CCSDS ASCII Time Code B as input.
			Fixed a leap second situation to repeat
			the second before midnight, not the 
			second after.  Updated prologs to current ECS/PGS
			standards, changed appropriate variable names
			to standard names.
   29-Jul-1994  GTSK    Altered function to return UTC Julian day as an array
                        of two real numbers instead of a single real number.
			Modified comments and prolog to reflect this change.
   04-Aug-1994  GTSK    Added message PGSTD_M_LEAP_SEC_IGNORED to indicate that
                        an input time with a seconds field of 60 has been
			treated as if the seconds field were 59.
  27-Apr-1995  GTSK     Replaced "PGSt_integer" with "int" and "PGSt_double"
                        with "double" (in one case) in an attempt to guarantee
                        that sscanf will always behave properly

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG

TITLE:   
   Convert UTC in CCSDS ASCII format to Julian date format

NAME:    
   PGS_TD_UTCtoUTCjd

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCtoUTCjd(
       char        asciiUTC[28],
       PGSt_double jdUTC[2])

DESCRIPTION:
   This tool converts a time from CCSDS ASCII Time (Format A or B) to Julian
   date.

INPUTS:
   Name       Description                 Units      Min     Max
   ----       -----------                 -----      ---     ---
   asciiUTC   UTC time in CCSDS ASCII     ASCII      N/A     N/A     
              Time Code A format or
	      ASCII Time Code B format

OUTPUTS:
   Name       Description                 Units      Min     Max
   ----       -----------                 -----      ---     ---
   jdUTC      UTC Julian date as two      days       N/A     N/A
              real numbers, the first
	      a half integer number of
	      days and the second the
	      fraction of a day between
              this half integer number
	      of days and the next half
	      integer day number.
RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_M_LEAP_SEC_IGNORED    leap second portion of input time discarded
   PGSTD_E_TIME_FMT_ERROR      error in format of input ASCII UTC time
   PGSTD_E_TIME_VALUE_ERROR    error in value of input ASCII UTC time
   PGS_E_TOOLKIT               something unexpected happened, execution aborted

EXAMPLES:
C:
   char             asciiUTC[28];

   PGSt_double      jdUTC[2];

   strcpy(asciiUTC, "1993-01-02T00:00:00.000000Z");

   returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC,jdUTC);

   ** jdUTC[0] should now have the value: 2448989.5 **
   ** jdUTC[1] should now have the value: 0.0 **
 
FORTRAN:
      integer pgs_td_utctoutcjd

      double precision sectai93
      double precision jdtai
 
      integer returnstatus

      asciiutc = '1993-01-02T00:00:00.000000Z'

      returnstatus = pgs_td_utctoutcjd(asciiutc, utcjd)

! jdutc[0] should now have the value: 2448989.5
! jdutc[1] should now have the value: 0.0
  
NOTES:
   Caution should be used because UTC time jumps backwards each time a leap
   second is introduced.  Therefore, in a leap second interval the output
   times will repeat those in the previous second (provided that the UTC 
   seconds ran from 60.0 to 60.9999999  etc. as they should during that
   one second).  Therefore, the only known uses for this function are:

       (a) to get UT1, by accessing an appropriate table of differences
       (b) to determine the correct Julian Day at which to access any table
           based on UTC and listed in Julian date, such as leap seconds,
           UT1, and polar motion tables.

   The input is UTC time in CCSDS ASCII Time Code A or CCSDS ASCII Time 
   Code B format (see CCSDS 301.0-B-2 for details). 
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

   The output is UTC as a Julian Date.
   Toolkit Julian dates are kept as an array of two real (high precision)
   numbers (C: PGSt_double, FORTRAN: DOUBLE PRECISION).  The first element of
   the array should be the half integer Julian day (e.g. N.5 where N is a Julian
   day number).  The second element of the array should be a real number greater
   than or equal to zero AND less than one (1.0) representing the time of the
   current day (as a fraction of that (86400 second) day.  This format allows
   relatively simple translation to calendar days (since the Julian days begin
   at noon of the corresponding calendar day).  Users of the Toolkit are
   encouraged to adhere to this format to maintain high accuracy (one number to
   track significant digits to the left of the decimal and one number to track
   significant digits to the right of the decimal).  Toolkit functions that do
   NOT require a Julian type date as an input and return a Julian date will
   return the Julian date in the above mentioned format.  Toolkit functions that
   require a Julian date as an input and do NOT return a Julian date will first
   convert the input date (internal) to the above format.  Toolkit functions
   that have a Julian date as both an input and an output will assume the input
   is in the above described format but will not check and the format of the
   output may not be what is expected if any other format is used for the input.

   UTC is: Coordinated Universal Time

REQUIREMENTS:
   PGSTK - 1170, 1220

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_timeCheck()      - check time string for format or value errors
   PGS_TD_ASCIItime_BtoA() - convert CCSDS ASCII Time Code A to Time Code B
   PGS_TD_julday()         - compute Julian Day number
   PGS_SMF_SetStaticMsg()  - set error/status message
   PGS_SMF_SetDynamicMsg() - set error/status message
   PGS_SMF_SetUnknownMsg() - set error/status message for unknown status

END_PROLOG
*******************************************************************************/

#include <string.h>
#include <sys/types.h>  
#include <stdio.h>
#include <PGS_TD.h>

/* conversion constants */

#ifdef UNICOS

#define SECONDSperDAY     86400.0L  /* number of seconds in a day */  
#define	SECONDSperHOUR    3600.0L   /* number of seconds in an hour */
#define	SECONDSperMINUTE  60.0L     /* number of seconds in a min. */ 

#else

#define SECONDSperDAY     86400.0   /* number of seconds in a day */  
#define	SECONDSperHOUR    3600.0    /* number of seconds in an hour */
#define	SECONDSperMINUTE  60.0      /* number of seconds in a min. */ 

#endif /* END: #ifdef UNICOS */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoUTCjd()"

PGSt_SMF_status
PGS_TD_UTCtoUTCjd(         /* converts CCSDS ASCII format to Julian date */
    char        *asciiUTC, /* UTC as a CCSDS ASCII Time Code A (or B) string */
    PGSt_double jdUTC[2])  /* UTC Julian date */
{
    char         asciiUTC_A[28];   /* UTC as a CCSDS ASCII Time Code A string */

    int          scanCheck;        /* checks the return value of sscanf call */
    int          year;             /* year portion of date */
    int          month;            /* month portion of date */
    int          day;              /* day portion of date */
    int          hours;            /* hours of the given date */
    int          minutes;          /* minutes of the given date */

    double       seconds;          /* seconds portion of the given date */

    PGSt_SMF_status returnStatus1; /* return value of PGS functions called */
    PGSt_SMF_status returnStatus;  /* return value of function */

    /* defaults for values not specified in time string */

    hours = 0;
    minutes = 0;
    seconds = 0.0;

    /* initialize return status to indicate success */

    returnStatus = PGS_S_SUCCESS;

    /* default value in case of failure */

    jdUTC[0] = 0.0;
    jdUTC[1] = 0.0;

    /* call PGS_TD_timeCheck() which checks the time string for format 
       or value errors.  PGS_TD_timeCheck also indicates if the input time
       string is in CCSDS ASCII Time Code B format.  If the input string
       is in ASCII B format PGS_TD_ASCIItime_BtoA is called which returns
       ASCII A format.  This function assumes CCSDS ASCII Time Code A format
       when parsing the time string so this step ensures that the appropriate
       format is used. */

    returnStatus = PGS_TD_timeCheck(asciiUTC);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	strncpy(asciiUTC_A,asciiUTC,28);
	asciiUTC_A[27] = '\0';
	break;
      case PGSTD_M_ASCII_TIME_FMT_B:
	returnStatus1 = PGS_TD_ASCIItime_BtoA(asciiUTC,asciiUTC_A);
	if (returnStatus1 != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
	returnStatus = PGS_S_SUCCESS;
	break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* separate the time string into its various numerical components. 
       Indicate error if an inappropriate number of fields is read.
       Note that the minimum number of fields (for CCSDS ASCII Time Code A
       format) accepted by PGS Toolkit functions is 3 (year, month, day) and the
       maximum is 6 (year, month, day, hours, minutes, seconds). */

    scanCheck = sscanf(asciiUTC_A,"%4d-%2d-%2dT%2d:%2d:%lfZ",
		       &year, &month, &day, &hours, &minutes, &seconds);
    if (scanCheck < 3 || scanCheck > 6)
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus,"error parsing time string",
			      FUNCTION_NAME);
	return returnStatus;
    }

    /* calculate the time of the date as fraction of 86400 sec day.
       UTC Julian date can't represent leap seconds uniquely, so just
       duplicate time during leap second, however duplicate the second
       before midnight, not the second after.  Return a message indicating
       that this has been done.  */

    if (seconds >= 60.0)
    {
	seconds -= 1.0;
	returnStatus = PGSTD_M_LEAP_SEC_IGNORED;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }

    /* calculate the fraction of the day:
       the input hours and minutes are converted to seconds and added to
       the input seconds to get the total seconds of the day.  This number
       is then divided by the total number of seconds in a day to get the
       time of day as the fraction of a whole day. */

    jdUTC[1] = (hours*SECONDSperHOUR + minutes*SECONDSperMINUTE + seconds)/
               SECONDSperDAY;


    /* calculate the Julian day number:
       PGS_TD_julday() returns the integer number of the Julian day that begins
       on the input calendar year-month-day.  Since the Julian day begins at
       noon, this function returns the Julian day that begins at noon on the
       input day.  What is actually desired here is the Julian DATE of midnight
       of the input year-month-day which occurs half a day earlier than the
       Julian day returned by the function.  Therefore 0.5 is subtracted from
       the return value of PGS_TD_julday to get the appropriate answer. */

    jdUTC[0] = PGS_TD_julday(year, month, day) - 0.5;

    /* return to calling function */

    if (returnStatus == PGS_S_SUCCESS)
      PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);

    return returnStatus;
}




