/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UDTFtoTAI.c

DESCRIPTION:
   This file contains the function PGS_TD_UDTFtoTAI().
   The function converts UTC as an integer array to TAI seconds since
   12 AM UTC 1-1-1993.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   20-Mar-1996  GTSK  Initial version.

END_FILE_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UDTFtoTAI()"

PGSt_SMF_status
PGS_TD_UDTFtoTAI(                    /* convert integer UTC time to TAI */
    PGSt_integer    timeUDTF[2],     /* UTC integer in "ISO" format: YYMMDDHH */
    PGSt_double     *secTAI93)       /* TAI seconds since 12 AM UTC 1-1-1993 */
{
    PGSt_SMF_status returnStatus;    /* return status of function calls */
    
    PGSt_boolean    onLeap;

    PGSt_double     jdUTC[2];        /* UTC Julian date (toolkit format) */
    PGSt_double     jdTAI[2];        /* TAI Julian date (toolkit format) */
    
    /* convert integer UTC time to UTC as a Julian date */

    returnStatus = PGS_TD_UDTFtoUTCjd(timeUDTF, jdUTC);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	onLeap = PGS_FALSE;
	break;
	
      case PGSTD_M_LEAP_SEC_IGNORED:
	onLeap = PGS_TRUE;
	break;
	
      default:
	return returnStatus;
    }

    /* convert UTC as a Julian date to TAI as a Julian date */

    returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC, onLeap, jdTAI);
    
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	break;
	
      case PGS_E_TOOLKIT:
	return returnStatus;
	
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* convert TAI as a Julian date to seconds since 12 AM UTC 1-1-1993 */

    *secTAI93 = PGS_TD_TAIjdtoTAI(jdTAI);
    
    /* if returnStatus is success set the message buffer to indicate this, if it
       is NOT success then the message buffer has already been set by the lower
       level function (PGS_TD_UTCjdtoTAIjd()) where the anomaly occurred */

    if (returnStatus == PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);
    }

    return returnStatus;
}
