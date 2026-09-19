/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_ASCIItime_BtoA.c

DESCRIPTION:
  This file contains the function PGS_TD_ASCIItime_BtoA()

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  06-Jun-1994  GTSK  Initial version
  25-Jul-1994  GTSK  Fixed up prologs - minor format changes to reflect latest
                     ECS/PGS standards
  27-Apr-1995  GTSK  Replaced "PGSt_uinteger" with "unsigned int" in an attempt
                     to guarantee that sscanf will always behave properly

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert CCSDS ASCII Time Code B to Time Code A

NAME:
   PGS_TD_ASCIItime_BtoA()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_ASCIItime_BtoA(
       char asciiUTC_B[26],
       char asciiUTC_A[28])

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'
    
      integer function pgs_td_asciitime_btoa(asciiutc_b,asciiutc_a)
      character*25 asciiutc_b
      character*27 asciiutc_a

DESCRIPTION:
   Convert UTC Time in CCSDS ASCII Time Code B to CCSDS ASCII Time Code A

INPUTS:
   Name         Description               Units       Min   Max
   ----         -----------               -----       ---   ---
   asciiUTC_B   UTC Time in CCSDS ASCII    N/A        N/A   N/A
                Time Code B


OUTPUTS:
   Name         Description               Units       Min   Max
   ----         -----------               -----       ---   ---
   asciiUTC_A   UTC Time in CCSDS ASCII    N/A        N/A   N/A
                Time Code A

RETURNS:
   PGS_S_SUCCESS               successful return
   PGSTD_E_TIME_VALUE_ERROR    error in input time value
   PGSTD_E_TIME_FMT_ERROR      error in input time format
   PGS_E_TOOLKIT               something unexpected happened, execution
                               of function terminated prematurely

EXAMPLES:
C:
   PGSt_SMF_status returnStatus;
   char            asciiUTC_B[26];
   char            asciiUTC_A[28];

   strcpy(asciiUTC_B,"1998-181T10:51:28.320000Z");
   returnStatus = PGS_TD_ASCIItime_BtoA(asciiUTC_B,asciiUTC_A);
   if (returnStatus != PGS_S_SUCCESS)
   {
   ** test errors, take appropriate action **
                    :
                    :
   }
   printf("%s\n",asciiUTC_A);

FORTRAN:
      implicit none

      include  'PGS_SMF.f'
      include  'PGS_TD_3.f'

      integer      pgs_td_asciitime_btoa

      integer      returnstatus

      character*25 asciiutc_b
      character*27 asciiutc_a

      asciiutc_b = '1998-181T10:51:28.320000'
      returnstatus = pgs_td_asciitime_btoa(asciiutc_b,asciiutc_a)
      if (returnstatus .ne. pgs_s_success) goto 999
      write(6,*) asciiutc_a

 999  ** test errors, take appropriate action **
                       :
                       :

NOTES:
   The input is UTC time in CCSDS ASCII Time Code B format.
   The output is UTC time in CCSDS ASCII Time Code A format.
   (see CCSDS 301.0-B-2 for details)
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


REQUIREMENTS:
   PGSTK - 1170, 1180, 1210

DETAILS:
   This routine checks the input time to ensure it is in CCSDS ASCII Time
   Code B, returning an error if it is not.  If it is, it converts the year
   and day of year fields from the input time string to decimal numbers and
   uses these numbers to determine if the input time occurs during a leap year
   (leap years are any year that has a 0 remainder when divided by four,
   excepting those years that have a 0 remainder when divided by one hundred
   AND a non-zero remainder when divided by 400) and finally what the month 
   and day are.  The year, month and day are then converted to the appropriate
   fields in the output time string and the TIME portion of the input string
   is appended to this (the TIME portion of Time Code A and Time Code B are
   identical).

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_timeCheck()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_ASCIItime_BtoA()"

PGSt_SMF_status
PGS_TD_ASCIItime_BtoA(           /* converts CCSDS ASCII Time B to 
				    Time A format */
    char         *asciiUTC_B,    /* UTC in CCSDS ASCII Time B format (input) */
    char         *asciiUTC_A)    /* UTC in CCSDS ASCII Time A format (output) */
{			                                                       
    unsigned int year;           /* calendar year */
    unsigned int month;          /* calendar month */
    unsigned int day;            /* calendar day (of month) */
    unsigned int doy;            /* day of year */
    unsigned int leapYear;       /* 1 if leap year, otherwise 0 */
    
    char         specifics[100]; /* detailed error message */
    
    /* number of days of the year by the end of each month */

    static unsigned int days[12]={31,59,90,120,151,181,212,243,273,304,334,365};
	
    PGSt_SMF_status returnStatus; /* checks status of PGS function calls */

    /* Check the input time string, if the return value indicates that the input
       time is in CCSDS ASCII Time B format continue.  If the return value is
       PGS_S_SUCCESS this indicates that the input time is in CCSDS ASCII Time A
       format, which is not what we want in this case.  Therefore set the
       message string to indicate that the wrong (but recognized) format was
       used and return a format error.  In the case of a general format or value
       error return these errors.  If something unexpected happened return
       PGS_E_TOOLKIT (this shouldn't occur unless the routine PGS_TD_timeCheck
       is changed and this code is not updated to reflect that change). */

    returnStatus = PGS_TD_timeCheck(asciiUTC_B);
    switch (returnStatus)
    {
      case PGSTD_M_ASCII_TIME_FMT_B:
	break;
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSTD_E_TIME_FMT_ERROR:
	return returnStatus;
      case PGS_S_SUCCESS:
	sprintf(specifics,"%s%s", "input time (1st parameter) must be in ",
		"CCSDS ASCII Time Code B (not A) format");
	PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_FMT_ERROR, specifics, FUNCTION_NAME);
	return PGSTD_E_TIME_FMT_ERROR;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* read the year and day of year from the input time (which should be
       guaranteed to be in the right format if this point is reached).  If the
       day of year is less than 32 it is still January and it really doesn't
       matter if it is a leap year so that determination is skipped.  Otherwise
       we determine if we are in a leap year and then figure out what the month
       and day are. */

    sscanf(asciiUTC_B, "%4u-%3u", &year, &doy);
    if (doy < 32U)
    {
	month = 1U;
	day = doy;
    }
    else
    {
	leapYear = (year%4U) ? 0U : 1U;
	if (year%100U == 0U)
	{
	    leapYear = (year%400U) ? 0U : 1U;
	}

	/* starting with February the total days of the year at the end of the
	   month are checked against the input day of the year, and the month is
	   incremented until the total days of the year at the end of the month
	   are >= the input day of the year */

	for(month=2U;(days[month-1U]+leapYear)<doy;month++);

	/* once the appropriate month is determined the total days of the year
	   up to the end of the preceding month are subtracted from the input
	   day of the year, the result being the day of the current month.  The
	   days of the year for a given month are stored in an array for the 365
	   day year and the extra day in the case of the leap year is accounted
	   for separately.  However the extra day should not be counted if it is
	   February since the leap year has not occurred yet, so only include
	   the extra day if the month != 2.  For speed's sake the number of days
	   in January (presumed to be stable as long as the Roman empire does
	   not regain world dominance) is hard coded in. */

	day = (month==2) ? doy - 31U : doy - days[month-2U] - leapYear;
    }

    /* Now write the time out in CCSDS ASCII Time A format by converting the
       decimal numbers for year, month and day and then appending whatever part
       of the input time string (in CCSDS ASCII Time B) that was specified after
       the day of year field.  There is no need to convert this to numbers first
       since PGS_TD_timeCheck has already assured us that the format is OK. (and
       is the same for both A and B formats). */

    sprintf(asciiUTC_A, "%04u-%02u-%02u%s", year, month, day, (asciiUTC_B+8));

    PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);
    return PGS_S_SUCCESS;
}
