/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1996, Raytheon Inc., its vendors,            */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_EPH_interpolateAttitude.c

DESCRIPTION:
   This file contains the function PGS_EPH_interpolateAttitude().
   Given a pair of spacecraft attitudes (as Euler angles), attitude rates and
   their corresponding times this function interpolates the spacecraft
   attitude and attitude rates to a requested time between the the two input
   times.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter D. Noerdlinger / SM&A Corporation

HISTORY:
   17-Oct-1994  GTSK  Initial version
   13-Nov-1998  PDN   Fixed several places where arithmetic exception could
                      occur to remedy output of NaNs when near data points.
   12-Feb-1999  PDN   Deleted diagnostic print statements. Added Notes

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Interpolate Spacecraft Attitude

NAME:
   PGS_EPH_interpolateAttitude()

SYNOPSIS:
C:

   PGSt_SMF_status
   PGS_EPH_interpolateAttitude(            
       PGSt_double  beginTAI93,           
       PGSt_double  beginEulerAngle[3],    
       PGSt_double  beginEulerAngleRate[3],
       PGSt_double  endTAI93,              
       PGSt_double  endEulerAngle[3],     
       PGSt_double  endEulerAngleRate[3],  
       PGSt_integer eulerAngleOrder[3],    
       PGSt_double  secTAI93,             
       PGSt_double  eulerAngle[3],         
       PGSt_double  eulerAngleRate[3])     

DESCRIPTION:
   Given a pair of spacecraft attitudes (as Euler angles), attitude rates and
   their corresponding times this function interpolates the spacecraft
   attititude and attitude rates to a requested time between the the two input
   times.

INPUTS:
   Name                 Description                      Units       Min   Max
   ----                 -----------                      -----       ---   ---
   beginTAI93           lower time boundary              sec
   beginEulerAngle	Euler angles at lower time       radians
   beginEulerAngleRate	Angular rates at lower time      rad/sec
   endTAI93    		upper time boundary              sec
   endEulerAngle	Euler angles at upper time       radians
   endEulerAngleRate	Angular rates at upper time      rad/sec
   eulerAngleOrder	Euler angle rotation order       N/A
   secTAI93		requested time                   sec

OUTPUTS:
   Name                 Description                      Units       Min   Max
   ----                 -----------                      -----       ---   ---
   eulerAngle           Euler angles at requested time   radians
   eulerAngleRate       Euler angle rates at req. time   rad/sec 
 
          
RETURNS:
   PGS_S_SUCCESS
   PGSEPH_E_BAD_TIME_ORDER
   PGSCSC_E_EULER_REP_INVALID:
   PGSCSC_E_EULER_INDEX_ERROR:
   PGSCSC_E_BAD_QUATERNION
   PGS_E_TOOLKIT

EXAMPLES:
   None
  
NOTES:
   The "Euler Angle Rates" herein mean (for all spacecraft after TRMM)
   the components of the angular velocity in J2000, projected on the
   body axes.  This makes no difference to the algorithm. 

REQUIREMENTS:
   PGSTK - 0720

DETAILS:
   This function converts the Euler angles at the lower and upper boundaries
   to a pair of quaternions describing the attitude of the spacecraft at the
   lower and upper boundary times respectively.  These are used to find the
   quaternion defining the rotation from the attitude at the lower boundary time
   to the attitude at the upper boundary time.  This quaternion by definition
   defines an angle and an axis of rotation that describes the rotation from the
   attitude at the lower boundary time to the attitude at the upper boundary
   time.  This information is used to construct a new quaternion with the same
   axis of rotation but with an angle linearly interpolated to the requested
   time.  This quaternion is then used, along with the quaternion defining the
   attitude at the lower boundary time, to find the quaternion that defines the
   attitude at the requested time.  This final quaternion is then converted back
   to the equivalent Euler angles.

   The Euler angle rates at the requested time are determined by linear
   interpolations of the Euler angle rates at the upper and lower boundary
   times.

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_CSC_EulerToQuat()
   PGS_CSC_quatMultiply()
   PGS_CSC_QuatToEuler()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_EPH.h>

/* name of this function */

#define FUNCTION_NAME "PGS_EPH_interpolateAttitude()"

PGSt_SMF_status
PGS_EPH_interpolateAttitude(            /* interpolate attitude data */
    PGSt_double  beginTAI93,            /* lower time boundary */           
    PGSt_double  beginEulerAngle[3],    /* Euler angles at lower time */    
    PGSt_double  beginEulerAngleRate[3],/* Euler angle rates at lower time */
    PGSt_double  endTAI93,              /* upper time boundary */           
    PGSt_double  endEulerAngle[3],      /* Euler angles at upper time */    
    PGSt_double  endEulerAngleRate[3],  /* certain angle rates at upper time */
    PGSt_integer eulerAngleOrder[3],    /* Euler angle rotation order */    
    PGSt_double  secTAI93,              /* requested time */                
    PGSt_double  eulerAngle[3],         /* Euler angles at requested time */
    PGSt_double  eulerAngleRate[3])     /* certain angle rates at req. time */
{
    PGSt_double  deltaTAI;              /* endTAI93 - beginTAI93 */
    PGSt_double  dt;                    /* secTAI93 - beginTAI93 */
    PGSt_double  beginQuat[4];          /* quaternion equivalent of rotation
					   defined by Euler angles at lower
					   time */
    PGSt_double  endQuat[4];            /* quaternion equivalent of rotation
					   defined by Euler angles at upper
					   time */
    PGSt_double  finalQuat[4];          /* quaternion equivalent of rotation
					   defined by Euler angles at requested
					   time */
    PGSt_double  deltaQuat[4];          /* quaternion defining rotation from
					   attitude at lower time to attitude at
					   upper time */
    PGSt_double  dQuat[4];              /* quaternion defining rotation from
					   attitude at lower time to attitude at
					   requested time */
    PGSt_double  beginQuatCong[4];      /* conjugate (inverse) of beginQuat */
    PGSt_double  deltaTheta;            /* angular rotation from attitude at
					   lower time to attitude at upper time
					   (about axis defined by deltaQuat) */
    PGSt_double  dTheta;                /* angular rotation from attitude at
					   lower time to attitude at requested
					   time */
    PGSt_double  sineRatio;             /* sine of half dTheta divided by sine
					   of half deltaTheta */
    PGSt_double  timeRatio;             /* dt/deltaTAI */
    
    PGSt_integer cnt;                   /* loop counter */
 
    PGSt_SMF_status returnStatus;       /* return value of PGS function calls */
    
    /* initialize returnStatus to PGS_E_TOOLKIT, returnStatus should never be
       returned unless something goes wrong */

    returnStatus = PGS_E_TOOLKIT;
    
    /* Make sure that beginTAI93 is not greater than endTAI93 and that secTAI93
       is contained in the set [beginTAI93,endTAI93].  If these conditions are
       not met, return an error. */

    if (beginTAI93 > endTAI93 || secTAI93 < beginTAI93 || secTAI93 > endTAI93)
    {
        PGS_SMF_SetStaticMsg(PGSEPH_E_BAD_TIME_ORDER,
			     FUNCTION_NAME);
	return PGSEPH_E_BAD_TIME_ORDER;
    }
    
    /* Calculate the time interval between the end points. */

    deltaTAI = endTAI93 - beginTAI93;

    /* Calculate the time interval between the desired time and the beginning
       time. */

    dt = secTAI93 - beginTAI93;
    
    /* If all times are equal (they should be if the above test was false and
       the following one is true) set the attitude and rates to the beginning
       values and return.  This must be done to avoid dividing by 0.0 later.
       Also if dt is zero then the requested time is the same as the first time
       anyway so no need to interpolate. */

    if (fabs(deltaTAI) < 1.e-10 || fabs(dt) < 1.e-10)
    {
        for (cnt=0;cnt<3;cnt++)
        {
	    eulerAngle[cnt] = beginEulerAngle[cnt];
	    eulerAngleRate[cnt]= beginEulerAngleRate[cnt];
        }
        return PGS_S_SUCCESS;
    }
    
    /* this line moved here, from above last "if" by PDN, Nov 13, 1998 */

    timeRatio = dt/deltaTAI;

    /* Convert the yaw, pitch and roll angles input to the equivalent
       quaternions. */

    returnStatus = PGS_CSC_EulerToQuat(beginEulerAngle,eulerAngleOrder,
				       beginQuat);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSCSC_E_EULER_REP_INVALID:
      case PGSCSC_E_EULER_INDEX_ERROR:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
	
    returnStatus = PGS_CSC_EulerToQuat(endEulerAngle,eulerAngleOrder,endQuat);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSCSC_E_EULER_REP_INVALID:
      case PGSCSC_E_EULER_INDEX_ERROR:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Get the inverse of the quaternion representing the attitude at the
       initial time. */

    beginQuatCong[0] = beginQuat[0];
    for (cnt=1;cnt<4;cnt++)
      beginQuatCong[cnt] = -beginQuat[cnt];

    /* Determine the quaternion that represents the rotation from the beginning
       attitude to the ending attitude. */

    returnStatus = PGS_CSC_quatMultiply(endQuat,beginQuatCong,deltaQuat);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSCSC_E_BAD_QUATERNION:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* Determine the angular rotation (detaTheta) between the beginning attitude
       and the ending attitude quaternions. */
   
    /* next 2 lines added by PDN  Nov 13, 1998 to prevent 
       possible FP exception in acos */

    if(deltaQuat[0] > 1.0) 
    {
       deltaQuat[0] = 1.0;
    }
    if(deltaQuat[0] < -1.0) 
    {
       deltaQuat[0] = -1.0;
    }

    deltaTheta = 2.0*acos(deltaQuat[0]);
    
    /* If deltaTheta is zero no rotation has occured between the beginning and
       ending times.  Set the attitude to the beginning values then interpolate
       the rates and return.  This must be done to avoid dividing by 0.0
       later. */

    /* the following line was changed to use 1.e-18 not 1.e-25 by PDN
       Nov 13, 1998. This prevents possible FP exception at the line about
       21 lines below where there is a division by sin(deltaTheta/2.0  */

    if (fabs(deltaTheta) < 1.e-18)
    {
        for (cnt=0;cnt<3;cnt++)
	    eulerAngle[cnt] = beginEulerAngle[cnt];
	for (cnt=0;cnt<3;cnt++)
	  eulerAngleRate[cnt] = beginEulerAngleRate[cnt] + 
	                       (endEulerAngleRate[cnt] -
				beginEulerAngleRate[cnt])*timeRatio;
        return PGS_S_SUCCESS;
    }
    
    /* Calculate the angular rotation (dTheta) between the beginning attitude
       and the attitude at the desired time.  This is done by assuming the
       rotation to the requested attitude is the proportion of the total
       rotation to the ending attitude accomplished at a steady rate in the time
       from the beginning point to the requested point. */

    dTheta = deltaTheta*timeRatio;
    
    sineRatio = sin(dTheta/2.0)/sin(deltaTheta/2.0);

    dQuat[0] = cos(dTheta/2.0);
    for (cnt=1;cnt<4;cnt++)
      dQuat[cnt] = deltaQuat[cnt]*sineRatio;
    
    returnStatus = PGS_CSC_quatMultiply(dQuat,beginQuat,finalQuat);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSCSC_E_BAD_QUATERNION:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    returnStatus = PGS_CSC_QuatToEuler(finalQuat,eulerAngleOrder,eulerAngle);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSCSC_E_EULER_REP_INVALID:
      case PGSCSC_E_EULER_INDEX_ERROR:
      case PGSCSC_E_BAD_QUATERNION:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    for (cnt=0;cnt<3;cnt++)
      eulerAngleRate[cnt] = beginEulerAngleRate[cnt] + 
	                   (endEulerAngleRate[cnt] - beginEulerAngleRate[cnt])*
			   timeRatio;
    
    return PGS_S_SUCCESS;
}

    
