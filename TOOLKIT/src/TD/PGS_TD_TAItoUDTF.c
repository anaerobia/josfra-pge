/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_TAItoUDTF.c

DESCRIPTION:
   This file contains the function PGS_TD_TAItoUDTF().
   The function converts TAI seconds since 12 AM UTC 1-1-1993 to UTC as an
   integer array.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   20-Mar-1996  GTSK  Initial version.

END_FILE_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_TAItoUDTF()"

PGSt_SMF_status
PGS_TD_TAItoUDTF(                   /* convert TAI to UTC as an integer array */
    PGSt_double     secTAI93,       /* TAI seconds since 12 AM UTC 1-1-1993 */
    PGSt_integer    timeUDTF[2])    /* UTC integer in "ISO" format: YYMMDDHH */
{
    PGSt_SMF_status returnStatus;   /* return status of function calls */
    PGSt_SMF_status returnStatus1;  /* return status of function calls */
    
    PGSt_double     jdUTC[2];       /* UTC Julian date (toolkit format) */
    PGSt_double     jdTAI[2];       /* TAI Julian date (toolkit format) */
    
    PGSt_boolean     onLeap;
    
    /* convert TAI as seconds since 12 AM UTC 1-1-1993 to TAI Julian date */

    PGS_TD_TAItoTAIjd(secTAI93, jdTAI);
    
    /* convert TAI Julian date to UTC Julian date */

    returnStatus = PGS_TD_TAIjdtoUTCjd(jdTAI, jdUTC);
    
    switch (returnStatus)
    {
      case PGSTD_M_LEAP_SEC_IGNORED:
	onLeap = PGS_TRUE;
	break;
	
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	onLeap = PGS_FALSE;
	break;
	
      case PGS_E_TOOLKIT:
	return returnStatus;
	
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* convert UTC as a Julian date to integer UTC time */

    returnStatus1 = PGS_TD_UTCjdtoUDTF(jdUTC, onLeap, timeUDTF);
    
    if (returnStatus1 != PGS_S_SUCCESS ||
	returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
    {
	/* if returnStatus1 is NOT PGS_S_SUCCESS then it should take precedence
	   over the value of returnStatus from the call to PGS_TD_TAIjdtoUTCjd()
	   (above), if returnStatus is PGSTD_M_LEAP_SEC_IGNORED it should be
	   overwritten anyway since it is no longer relevant (or true) */

	returnStatus = returnStatus1;
    }
    
    /* if returnStatus is success set the message buffer to indicate this, if it
       is NOT success then the message buffer has already been set by the lower
       level function where the anomaly occurred (i.e. either
       PGS_TD_TAIjdtoUTCjd() or PGS_TD_UTCjdtoUDTF()) */

    if (returnStatus == PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
    }

    return returnStatus;
}
