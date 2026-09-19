/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_ASCIItime_AtoB.c

DESCRIPTION:
  This file contains the function PGS_TD_ASCIItime_AtoB()

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
   Convert CCSDS ASCII Time Code A to Time Code B

NAME:
   PGS_TD_ASCIItime_AtoB()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_ASCIItime_AtoB(
       char asciiUTC_A[28],
       char asciiUTC_B[26])

FORTRAN:
   include 'PGS_SMF.f'
   include 'PGS_TD_3.f'
    
   integer function pgs_td_asciitime_atob(asciiutc_a,asciiutc_b)
   character*27 asciiutc_a
   character*25 asciiutc_b

      
DESCRIPTION:
   Convert UTC Time in CCSDS ASCII Time Code A to CCSDS ASCII Time Code B

INPUTS:
   Name         Description               Units       Min   Max
   ----         -----------               -----       ---   ---
   asciiUTC_A   UTC Time in CCSDS ASCII    N/A        N/A   N/A
                Time Code A

OUTPUTS:
   Name         Description               Units       Min   Max
   ----         -----------               -----       ---   ---
   asciiUTC_B   UTC Time in CCSDS ASCII    N/A        N/A   N/A
                Time Code B
          
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_E_TIME_VALUE_ERROR    error in input time value
   PGSTD_E_TIME_FMT_ERROR      error in input time format
   PGS_E_TOOLKIT               something unexpected happened, execution
                               of function terminated prematurely

EXAMPLES:
C:
   PGSt_SMF_status returnStatus;
   char            asciiUTC_A[28];
   char            asciiUTC_B[26];

   strcpy(asciiUTC_A,"1998-06-30T10:51:28.320000Z");
   returnStatus = PGS_TD_ASCIItime_AtoB(asciiUTC_A,asciiUTC_B);
   if (returnStatus != PGS_S_SUCCESS)
   {
   ** test errors, take appropriate action **
                    :
                    :
   }
   printf("%s\n",asciiUTC_B);

FORTRAN:
      implicit none

      include  'PGS_SMF.f'
      include  'PGS_TD_3.f'

      integer      pgs_td_asciitime_atob
      integer      returnstatus
      character*27 asciiutc_a
      character*25 asciiutc_b
      
      asciiutc_a = '1998-06-30T10:51:28.320000'
      returnstatus = pgs_td_asciitime_atob(asciiutc_a,asciiutc_b)
      if (returnstatus .ne. pgs_s_success) goto 999
      write(6,*) asciiutc_b

 999  ** test errors, take appropriate action **
                       :
                       :

NOTES:
   The input is UTC time in CCSDS ASCII Time Code A format.
   The output is UTC time in CCSDS ASCII Time Code B format.
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
   This routine checks the input time to ensure it is in CCSDS ASCII Time Code
   A, returning an error if it is not.  If it is, it converts the year month and
   day fields from the input time string to decimal numbers and uses these
   numbers to determine if the input time occurs during a leap year (leap years
   are any year that has a 0 remainder when divided by four, excepting those
   years that have a 0 remainder when divided by one hundred AND a non-zero
   remainder when divided by 400) and finally what the day of year is.  The year
   and day of year are then converted to the appropriate fields in the output
   time string and the TIME portion of the input string is appended to this (the
   TIME portion of Time Code A and Time Code B are identical).

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

PGSt_SMF_status
PGS_TD_ASCIItime_AtoB(           /* converts CCSDS ASCII Time A to 
				    Time B format */
    char         *asciiUTC_A,	 /* UTC in CCSDS ASCII Time A format (input) */
    char         *asciiUTC_B)	 /* UTC in CCSDS ASCII Time B format (output) */
{			                                                       
    unsigned int year;	         /* calendar year */
    unsigned int month;          /* calendar month */
    unsigned int day;	         /* calendar day (of month) */
    unsigned int doy;	         /* day of year */
    unsigned int leapYear;       /* 1 if leap year, otherwise 0 */

    char         specifics[100]; /* detailed error message */
    
    /* number of days of the year by the end of each month */

    static unsigned int days[12]={31,59,90,120,151,181,212,243,273,304,334,365};

    PGSt_SMF_status returnStatus; /* checks status of PGS function calls */

    /* Check the input time string, if the return value indicates that the input
       time is in CCSDS ASCII Time A format continue.  If the return value is
       PGSTD_M_ASCII_TIME_FMT_B this indicates that the input time is in CCSDS
       ASCII Time B format, which is not what we want in this case.  Therefore
       set the message string to indicate that the wrong (but recognized) format
       was used and return a format error.  In the case of a general format or
       value error return these errors.  If something unexpected happened return
       PGS_E_TOOLKIT (this shouldn't occur unless the routine PGS_TD_timeCheck
       is changed and this code is not updated to reflect that change). */

    returnStatus = PGS_TD_timeCheck(asciiUTC_A);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSTD_E_TIME_FMT_ERROR:
	return returnStatus;
      case PGSTD_M_ASCII_TIME_FMT_B:
	sprintf(specifics,"%s%s","input time (1st parameter) must be in ",
		"CCSDS ASCII Time Code A (not B) format");
	PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_FMT_ERROR,specifics,
			      "PGS_TD_ASCIItime_AtoB()");
	return PGSTD_E_TIME_FMT_ERROR;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,"PGS_TD_ASCIItime_AtoB()");
	return PGS_E_TOOLKIT;
    }

    /* Read the year, month and day from the input time (which should be
       guaranteed to be in the right format if this point is reached).  Then
       determine if we are in a leap year and calculate the day of year from the
       days array and the leap year info. */

    sscanf(asciiUTC_A,"%4u-%2u-%2u",&year,&month,&day);
    
    /* If the month is January or February there is no need to calculate whether
       we are in a leap year, so why bother? */

    if (month < 3U)
    {
	/* If it is January the day of the month is also the day of the year, if
	   it is February the day of the year is the day of the month + 31 (the
	   days in January). */
	
	doy = (month==1) ? day : 31 + day;
    }
    else
    {
	leapYear = (year%4U) ? 0U : 1U;
	if (year%100U == 0U)
	{
	    leapYear = (year%400U) ? 0U : 1U;
	}

	/* day of year is the number of days of the year up to the preceding
	   month (days[month-2] + leapYear) plus the days of the current month
	   (day) */

	doy = days[month-2U] + leapYear + day;
    }
    
    /* Now write the time out in CCSDS ASCII Time B format by converting the
       decimal numbers for year, day of year and then appending whatever part of
       the input time string (in CCSDS ASCII Time A) that was specified after
       the day of year field.  There is no need to convert this to numbers first
       since PGS_TD_timeCheck has already assured us that the format is OK (and
       is the same for both A and B formats). */

    sprintf(asciiUTC_B,"%04u-%03u%s",year,doy,(asciiUTC_A+10));

    PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_TD_ASCIItime_AtoB()");
    return PGS_S_SUCCESS;
}
