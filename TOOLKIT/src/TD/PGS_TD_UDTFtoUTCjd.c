/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UDTFtoUTCjd.c

DESCRIPTION:
   This file contains the function PGS_TD_UDTFtoUTCjd().
   The function converts UTC as an integer array to UTC as a Julian date.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   19-Mar-1996  GTSK  Initial version.

END_FILE_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* constants */

#define MILLISECperDAY 86400000.0

/* name of this function */

#define FUNCTION_NAME  "PGS_TD_UDTFtoUTCjd()"

PGSt_SMF_status
PGS_TD_UDTFtoUTCjd(
    PGSt_integer     timeUDTF[2],
    PGSt_double      jdUTC[2])
{
    PGSt_integer     year;           /* integer representation of year value */
    PGSt_integer     month;          /* integer month of the year value */
    PGSt_integer     day;            /* integer day of the month value */
    PGSt_integer     leapYear;       /* 1 if leap year, otherwise 0 */

    PGSt_double      jdTAI[2];       /* TAI Julian date in Toolkit format */

    PGSt_SMF_status  returnStatus;   /* return status of this function */

    static PGSt_integer doy[13]={0,0,59,90,120,151,181,212,243,273,304,334,365};

    year = timeUDTF[1]/1000 + 1900;
    day = timeUDTF[1]%1000;
    
    if (day < 32)
    {
	month = 1;
    }
    else
    {
	leapYear = (year%4) ? 0 : 1;
	if (year%100 == 0)
	{
	    leapYear = (year%400) ? 0 : 1;
	}

	/* starting with February the total days of the year at the end of the
	   month are checked against the input day of the year, and the month is
	   incremented until the total days of the year at the end of the month
	   are >= the input day of the year */

	for(month=2;(doy[month]+leapYear)<day;month++);

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

	day = (month==2) ? day - 31 : day - doy[month] - leapYear;
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

    /* Calculate the day fraction.  If the input milliseconds of the day are 
       86400000, issue message that the indicated leap second is being ignored.
       If the input milliseconds of the day are greater 86400999, issue an
       error. */ 

    if (jdUTC[1] > 86399999)
    {
	if (jdUTC[1] > 86400999)
	{
	    returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	    PGS_SMF_SetDynamicMsg(returnStatus,
				  "input milliseconds field is too large "
				  "(i.e. > 86400999)",
				  FUNCTION_NAME);
	}
	else
	{	
	    jdUTC[1] = (timeUDTF[1]-1U)/MILLISECperDAY;
	    returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC, PGS_TRUE, jdTAI);
	    if (returnStatus != PGSTD_E_TIME_VALUE_ERROR)
	    {
		returnStatus = PGSTD_M_LEAP_SEC_IGNORED;
		PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
	    }
	}
    }
    else
    {
	    jdUTC[1] = timeUDTF[1]/MILLISECperDAY;	
    }

    if (returnStatus == PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
    }
    
    return returnStatus;
}

