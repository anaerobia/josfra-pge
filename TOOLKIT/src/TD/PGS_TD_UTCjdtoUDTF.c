/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UTCjdtoISOint.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCjdtoUDTF().
   The function converts UTC as a Julian date to UTC as an integer array.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   19-Mar-1996  GTSK  Initial version.

END_FILE_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_TD.h>

/* constants */

#define MILLISECperDAY 86400000.0

/* name of this function */

#define FUNCTION_NAME  "PGS_TD_UTCjdtoUDTF()"

PGSt_SMF_status
PGS_TD_UTCjdtoUDTF(
    PGSt_double      jdUTC_in[2],
    PGSt_boolean     onLeap,
    PGSt_integer     timeUDTF[2])
{
    PGSt_integer     year;           /* integer representation of year value */
    PGSt_integer     month;          /* integer month of the year value */
    PGSt_integer     day;            /* integer day of the month value */
    PGSt_integer     leapYear;       /* 1 if leap year, otherwise 0 */

    PGSt_double      jdUTC[2];       /* UTC Julian date in Toolkit format */
    PGSt_double      jdTAI[2];       /* TAI Julian date in Toolkit format */

    PGSt_SMF_status  returnStatus;   /* return status of this function */

    static PGSt_integer doy[13]={0,0,31,59,90,120,151,181,212,243,273,304,334};

    /* make sure the input date is in Toolkit Julian date format */

    PGS_TD_JulianDateSplit(jdUTC_in, jdUTC);
    
    /* Get calendar day components of Julian day.  The function PGS_TD_calday() 
       returns the year, month, and day on which the current Julian date 
       started, however since the Julian day starts at noon half a day should
       be added to the Julian day first and then this whole number Julian day
       will give us the proper calendar date.  Since the Julian day has
       been kept as integer.5 the day fraction portion already represents the
       fraction of the calendar day (as opposed to the actual fraction of the
       integer Julian day number) so there is no need to modify this value. */

    PGS_TD_calday((PGSt_integer) (jdUTC[0] + .5),&year,&month,&day);
    
    /* the first element of UDTF time is defined as:
       ((YEAR - 1900) * 1000) + DAY OF YEAR */

    timeUDTF[0] = ((unsigned)year - 1900)*1000 + doy[month] + (unsigned)day;
    if (month > 2)
    {
	leapYear = (year%4) ? 0 : 1;
	if (year%100 == 0)
	{
	    leapYear = (year%400) ? 0 : 1;
	}
	timeUDTF[0] += leapYear;
    }
    
    /* the second element of UDTF is the time in milliseconds of the day */

    timeUDTF[1] = (PGSt_uinteger) (jdUTC[1]*MILLISECperDAY);
    if (onLeap == PGS_TRUE)
    {
	returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC, PGS_TRUE, jdTAI);
	if (returnStatus == PGSTD_E_TIME_VALUE_ERROR)
	{
	    return returnStatus;
	}
	timeUDTF[1] += 1000;
    }
    
    if (fabs(timeUDTF[1]/MILLISECperDAY-jdUTC[1]) >= 1.0E6)
    {
	returnStatus = PGSTD_M_TIME_TRUNCATED;
    }
    else
    {
	returnStatus = PGS_S_SUCCESS;
    }
    
    PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);

    return returnStatus;
}
