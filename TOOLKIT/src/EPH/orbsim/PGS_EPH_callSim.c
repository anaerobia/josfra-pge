/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*HISTORY:                                                                 */
/*  09-Aug-1995  msucher        Initial version                            */
/*  30-Jun-2003  Abe Taaheri    added orbital elements                     */
/*-------------------------------------------------------------------------*/

#include <PGS_SIM.h>
#include <PGS_EPH.h>

PGSt_SMF_status
PGS_EPH_callSim(
    PGSt_double     aIn,      /* semi-major axis (m) */ 
    PGSt_double     eIn,      /* eccentricity */
    PGSt_double     incIn,    /* inclination (radians) */ 
    PGSt_double     anIn,     /* right ascension of ascending node (radians) */
    PGSt_double     apIn,     /* argument of perigee (radians) */ 
    PGSt_double     maIn,     /* mean anomaly at time (radians) */
    PGSt_double     secTAI93,
    PGSt_tag        spacecraftTag,
    scData          *record)
{
    PGSt_double     transform[3][3];

    PGSt_SMF_status returnStatus=PGS_S_SUCCESS;
    
    /**** begin program execution ****/

    /**** call the attitude and orbit simulator for time 'asciiUTC' ****/

    returnStatus = PGS_EPH_attOrbSim(aIn, eIn, incIn, anIn, apIn, maIn,
				     secTAI93,spacecraftTag,"N0010000100",
				     record->pos,record->vel,record->ypr,
				     record->yprRate,transform);
    if (returnStatus != PGS_S_SUCCESS)
    {
	returnStatus = PGSEPH_E_SIMULATOR_ERROR;
	PGS_SMF_SetStaticMsg(returnStatus,"PGS_EHP_callSim()");
	return returnStatus;
    }
    
    /**** convert the rotation matrix to a quaternion ****/

    returnStatus = PGS_EPH_getQuats(transform,record->quaternion);
    if (returnStatus != PGS_S_SUCCESS)
    {
	returnStatus = PGSEPH_E_SIMULATOR_ERROR;
	PGS_SMF_SetStaticMsg(returnStatus,"PGS_EHP_callSim()");
	return returnStatus;
    }

    return returnStatus;
}
