/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_QuatToEuler.c

DESCRIPTION:
   This file contains the function PGS_CSC_QuatToEuler().
   This function gets Euler angles from a quaternion.
   
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Anubha Singhal     / Applied Research Corporation
   Peter D. Noerdlinger/ Emergent Information Technologies, Inc.
   Curt Schafer      / Steven Myers & Associates

HISTORY:
   14-Oct-1994 AS/GTSK Initial version
   25-Aug-1999 PDN fixed calculation of "alpha" (renamed from "psi"
               to avoid confusion with an Euler angle). Renamed
               intermediate angle from alpha to chi for similar reason.
   11-Aug-2000 PDN fixed calculation so that the total rotation is
               in the range -Pi to Pi radians.  This keeps Euler
               angles in tighter ranges of absolute value without
               impairing the ability to model any rotation
   23-Nov-2000 PDN again fixed calculation to avoid out-of-range
               angles. Output angles will be adjusted by + or -
               2*Pi if out of the range -Pi,Pi.  Furthermore, for
               asymmetric cases (such as 3,2,1 or 2,3,1) the middle
               angle eulerAngle[1] must lie in the range (-Pi/2,Pi/2),
               while in the symmetric or repeating case it must lie
               in the range (0,Pi).  Added comments on trig identities.
               Ranges are in Schuster, cited below, p. 457, but 
               we adjust the first and last Euler angle to lie in
               (-Pi,Pi) instead of (0,2*Pi).
   6-Dec-2000  CS fixed nomenclature in thread-safe portion

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE: 
   Obtains Euler angles from a quaternion

NAME:   
   PGS_CSC_QuatToEuler()

SYNOPSIS:
 C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_QuatToEuler( 
       PGSt_double  quaternion[4],            
       PGSt_integer eulerAngleOrder[3],
       PGSt_double  eulerAngle[3])   
	      
 FORTRAN:
      include 'PGS_CSC_4.f'
      include 'PGS_SMF.f'

      integer function pgs_csc_quattoeuler(quaternion,eulerangleorder,
     >                                     eulerangle)
      double precision quaternion(4)
      integer          eulerangleorder(3)
      double precision eulerangle(3)
    
DESCRIPTION:
   This function gets euler angles from a quaternion.

INPUTS:
   Name              Description            Units    Min        Max
   ----              -----------            -----    ---        --- 
   quaternion        quaternion defining    N/A      N/A        N/A
                     the desired rotation                
   eulerAngleOrder   order of rotations     N/A      N/A        N/A  
                                                   
OUTPUTS:
   Name              Description            Units    Min        Max
   ----              -----------            ----     ---        ---
   eulerAngle        Euler angles           radians  -Pi        Pi
                                                  
RETURNS:
   PGS_S_SUCCESS		 Success
   PGSCSC_E_EULER_REP_INVALID    Euler angle order is invalid
   PGSCSC_E_EULER_INDEX_ERROR    Error in value of euler index
   PGSCSC_E_BAD_QUATERNION       Input quaternion is invalid

NOTES:
   The indices 1, 2, 3 are used for the Euler angle order. They refer to the
   rotations about the body axes labeled 1, 2 and 3 which are assumed to be x,
   y and z.  The angles of rotation are phi, theta and psi in the order
   performed.  Thus, if the Euler angle order is, for example, 2, 1, 3, it means
   we rotate phi about axis 2, then by theta about new axis 1, then by psi about
   new axis 3.

   If the 3 Euler rotation axes are all different(e.g 3,1,2), the quaternion 
   is obtained from the following equations (ref: Schuster[1] Eq(191))

   xi(i,j) = 1.0 if {i,j} = 1,2 or 2,3 or 3,1
           = -1.0 if {i,j} = 2,1 or 3,2 or 1,3
	   = 0 otherwise

   [1] "A Survey of Attitude Representations", by Malcolm D. Schuster, The
       Journal of the Astronautical Sciences vol 41, #4, pp. 439-517 (AIAA,
       Oct 1993)

REQUIREMENTS:
   PGSTK - 1050 

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
	PGSg_TSF_CSCQuatpsi
        PGSg_TSF_CSCQuatiIndex
        PGSg_TSF_CSCQuatjIndex
        PGSg_TSF_CSCQuatkIndex
        PGSg_TSF_CSCQuatoldEAOrder
        PGSg_TSF_CSCQuatindicesDiff

FILES:
   NONE

FUNCTIONS CALLED:
   PGS_SMF_SetStaticMsg()
      
END_PROLOG
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_TSF.h>

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_QuatToEuler()"
 

PGSt_SMF_status
PGS_CSC_QuatToEuler(                       /* obtains the equivalent Euler
					      angles from a quaternion */  
    PGSt_double         quaternion[],      /* orbital */
    PGSt_integer        eulerAngleOrder[], /* order of the rotations */
    PGSt_double	        eulerAngle[])      /* Euler angles */ 
{  
    short               cnt;               /* loop counter */

    static PGSt_integer iIndex;            /* eulerAngleOrder[0] */
    static PGSt_integer jIndex;            /* eulerAngleOrder[1] */
    static PGSt_integer kIndex;            /* eulerAngleOrder[2] */
    static PGSt_integer oldEAOrder[3];     /* saved Euler angle order */ 
    static PGSt_integer alpha;            /* right hand (1) or left hand (-1)
					      order else 0 */
    PGSt_double         chi;             /* intermediate angle */
    PGSt_double         beta;              /* intermediate angle */
    PGSt_double         Pi;              /* 3.141592653589793... */
    PGSt_double         quatTemp[4];     /* quaternion modified for rotation < 180 deg
                                            in magnitude */
    PGSt_double         sumSquares;        /* sum of the squares of the
					      components of the quaternion */
    static PGSt_boolean indicesDiff;       /* whether a symmetric or asymmetric
					      Euler angle order */
    PGSt_integer        i4;                /* index for reversing quat if needed */
    PGSt_SMF_status     returnStatus;      /* return status of function call */

#ifdef _PGS_THREADSAFE

   /* Declare variables used for THREADSAFE version to replace statics
      The local names are appended with TSF and globals are preceded with
      directory and function name */
 
    PGSt_TSF_MasterStruct *masterTSF;
    PGSt_integer iIndexTSF;
    PGSt_integer jIndexTSF;
    PGSt_integer kIndexTSF;
    PGSt_integer alphaTSF;
    PGSt_integer oldEAOrderTSF[3];
    PGSt_boolean indicesDiffTSF;
    PGSt_integer x;                   /* loop counter for initialization */
 
    /* Globals     originals in PGS_TSF_SetupCSC.c */

    extern PGSt_integer PGSg_TSF_CSCQuatpsi[];
    extern PGSt_integer PGSg_TSF_CSCQuatiIndex[];
    extern PGSt_integer PGSg_TSF_CSCQuatjIndex[];
    extern PGSt_integer PGSg_TSF_CSCQuatkIndex[];
    extern PGSt_integer PGSg_TSF_CSCQuatoldEAOrder[][3];
    extern PGSt_boolean PGSg_TSF_CSCQuatindicesDiff[];
    int masterTSFIndex;

    /* Get index  and initialize keys then test for bad returns */
 
    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(returnStatus) || masterTSFIndex ==
                                         PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
 
    /*   Initialize the variables used for the THREADSAFE version */

    alphaTSF = PGSg_TSF_CSCQuatpsi[masterTSFIndex];
    iIndexTSF = PGSg_TSF_CSCQuatiIndex[masterTSFIndex];
    jIndexTSF = PGSg_TSF_CSCQuatjIndex[masterTSFIndex];
    kIndexTSF = PGSg_TSF_CSCQuatkIndex[masterTSFIndex];
    indicesDiffTSF = PGSg_TSF_CSCQuatindicesDiff[masterTSFIndex];
    for (x=0; x<3; x++)
    {
        oldEAOrderTSF[x] =
             PGSg_TSF_CSCQuatoldEAOrder[masterTSFIndex][x];
    }
 
#endif


    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS;
    
    /* Make sure the input quaternion is indeed a quaternion.  The sum of the
       squares of the components of a quaternion should equal one (1). */

    sumSquares = quaternion[0]*quaternion[0] + quaternion[1]*quaternion[1] +
                 quaternion[2]*quaternion[2] + quaternion[3]*quaternion[3];
    if (sumSquares < PGSd_LOWER_LIMIT || sumSquares > PGSd_UPPER_LIMIT)
    {
	returnStatus =  PGSCSC_E_BAD_QUATERNION;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	return returnStatus;
    }

    Pi = 2.0000 * asin(1.000000);
    
    /* if quaternion has 1st term < 0, reverse all terms; this keeps
        total rotation < 180 degrees in magnitude. For example, minus
        270 degrees or minus 3*Pi/2 becomes plus 90 degrees or Pi/2 */

    if(quaternion[0] < 0.0)
    {
       for(i4=0; i4 < 4; i4++)
       {
          quatTemp[i4] = -quaternion[i4];
       }
    }
    else
    {
       for(i4=0; i4 < 4; i4++)
       {
          quatTemp[i4] = quaternion[i4];
       }
    }


    /* check if we have used this function  with the same euler angle order-- 
       if so, omit setup and proceed to obtain the Euler angles */ 

#ifdef _PGS_THREADSAFE
 
    /* Threadsafe Protect oldEAOrder  */

    if ((eulerAngleOrder[0] != oldEAOrderTSF[0]) ||
	(eulerAngleOrder[1] != oldEAOrderTSF[1]) ||
	(eulerAngleOrder[2] != oldEAOrderTSF[2]) )
    {
	
	/* test that none of the values in the euler angle order is less than 
           1 or greater than 3 */

	for (cnt = 0 ;cnt < 3; cnt++)
	  switch (eulerAngleOrder[cnt])
	  {
	    case 1:
	    case 2:
	    case 3:
	      break;
	    default:
	      returnStatus = PGSCSC_E_EULER_INDEX_ERROR;
	      PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	      return returnStatus;
	  }

        /* Threadsafe Protect: iIndex,jIndex, kIndex    Reassign values  */

	iIndexTSF = eulerAngleOrder[0];
        PGSg_TSF_CSCQuatiIndex[masterTSFIndex] = iIndexTSF;
	jIndexTSF = eulerAngleOrder[1];
        PGSg_TSF_CSCQuatjIndex[masterTSFIndex] = jIndexTSF;
	kIndexTSF = eulerAngleOrder[2];
        PGSg_TSF_CSCQuatkIndex[masterTSFIndex] = kIndexTSF;

	/* test to see if all the values in the euler angle order are 
           different */

	if (((iIndexTSF - jIndexTSF) * (iIndexTSF - kIndexTSF) *
                                           (jIndexTSF - kIndexTSF)) != 0)
        {
            /* Threadsafe Protect: indicesDiff    Reassign values  */ 

	    indicesDiffTSF = PGS_TRUE;
            PGSg_TSF_CSCQuatindicesDiff[masterTSFIndex] = indicesDiffTSF;
        }

        /* test to see if the last and first indices are the same and the 
	   middle one is different */
        /* Threadsafe Protect: iIndex,jIndex, kIndex  Reassign values  */

	else if ((iIndexTSF == kIndexTSF) && (jIndexTSF != kIndexTSF))
        {
	    kIndexTSF = 6 - iIndexTSF - jIndexTSF;
            PGSg_TSF_CSCQuatkIndex[masterTSFIndex] = kIndexTSF;
	    indicesDiffTSF = PGS_FALSE;
            PGSg_TSF_CSCQuatindicesDiff[masterTSFIndex] = indicesDiffTSF;
	}
     
        /* euler angle order is invalid - issue error and return */

	else 
	{
	    returnStatus = PGSCSC_E_EULER_REP_INVALID;
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	    return returnStatus; 
	}

	/* determine psi */
        /* Threadsafe Protect: psi, i,j,k,Index    Reassign psi value  */ 

        alphaTSF = (jIndexTSF  == (iIndexTSF + 1) % 3) -
                                (jIndexTSF == (iIndexTSF + 2) % 3);
        PGSg_TSF_CSCQuatpsi[masterTSFIndex]  = alphaTSF;

	/* save the Euler angle order so that the setup can be omitted if the
	   order is the same the next time the function is called */

	for (cnt = 0; cnt < 3; cnt++)
        {
	   oldEAOrderTSF[cnt] = eulerAngleOrder[cnt];
           PGSg_TSF_CSCQuatoldEAOrder[masterTSFIndex][cnt] = oldEAOrderTSF[cnt];
        }
    } 
    
    /* determine Euler angles for a non-repeating sequence of Euler angles */
    /* Threadsafe Protect psi, iIndex, jIndex, kIndex, indicesDiff */

    if (indicesDiffTSF == PGS_TRUE)
    {
	eulerAngle[1] = asin(2.0*(alphaTSF*quatTemp[iIndexTSF]*quatTemp[kIndexTSF] +
				  quatTemp[jIndexTSF]*quatTemp[0]));
	chi = atan2((quatTemp[iIndexTSF] + quatTemp[kIndexTSF]),
		       (alphaTSF*quatTemp[jIndexTSF] + quatTemp[0]));
	beta = atan2((quatTemp[iIndexTSF] - quatTemp[kIndexTSF]),
		    (-alphaTSF*quatTemp[jIndexTSF] + quatTemp[0]));
	eulerAngle[0] = chi + beta;
	eulerAngle[2] = chi - beta;
        for (cnt = 0; cnt < 3; cnt++)
        {
           if(eulerAngle[cnt] < -Pi) eulerAngle[cnt] += 2*Pi;
           if(eulerAngle[cnt] >  Pi) eulerAngle[cnt] -= 2*Pi;
        }
    }

    /* determine Euler angles for a repeating sequence of Euler angles */
    /* Threadsafe Protect iIndex, psi, kIndex, jIndex  */
    else 
    {
	chi = atan2(quatTemp[iIndexTSF],quatTemp[0]);
	beta = atan2(alphaTSF*quatTemp[kIndexTSF],quatTemp[jIndexTSF]);
/*  the following uses the identity 2*asin(x)=acos(1-2*x*x) and Schuster's Eq. 193,
     as well as the sum rule on quat. components squared.  PDN  */
	eulerAngle[1] = 2.0*asin(sqrt(quatTemp[jIndexTSF]*quatTemp[jIndexTSF] 
                                    + quatTemp[kIndexTSF]*quatTemp[kIndexTSF]));
        eulerAngle[0] = chi + beta;
	eulerAngle[2] = chi - beta;
        for (cnt = 0; cnt < 3; cnt++,cnt++)
        {
           if(eulerAngle[cnt] < -Pi) eulerAngle[cnt] += 2*Pi;
           if(eulerAngle[cnt] >  Pi) eulerAngle[cnt] -= 2*Pi;
        }
           if(eulerAngle[1] < 0.0 ) eulerAngle[1] += 2*Pi;
           if(eulerAngle[1] >  2.0*Pi) eulerAngle[1] -= 2*Pi;
    } 
    
#else
    
    if ((eulerAngleOrder[0] != oldEAOrder[0]) ||
	(eulerAngleOrder[1] != oldEAOrder[1]) ||
	(eulerAngleOrder[2] != oldEAOrder[2]) )
    {
	
	/* test that none of the values in the euler angle order is less than 
           0 or greater than 2 */

	for (cnt = 0 ;cnt < 3; cnt++)
	  switch (eulerAngleOrder[cnt])
	  {
	    case 1:
	    case 2:
	    case 3:
	      break;
	    default:
	      returnStatus = PGSCSC_E_EULER_INDEX_ERROR;
	      PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	      return returnStatus;
	  }

	iIndex = eulerAngleOrder[0];
	jIndex = eulerAngleOrder[1];
	kIndex = eulerAngleOrder[2];

	/* test to see if all the values in the euler angle order are 
           different */

	if (((iIndex - jIndex) * (iIndex - kIndex) * (jIndex - kIndex)) != 0)
	  indicesDiff = PGS_TRUE;

        /* test to see if the last and first indices are the same and the 
	   middle one is different */

	else if ((iIndex == kIndex) && (jIndex != kIndex))
        {
	    kIndex = 6 - iIndex - jIndex;
	    indicesDiff = PGS_FALSE;
	}
     
        /* euler angle order is invalid - issue error and return */

	else 
	{
	    returnStatus = PGSCSC_E_EULER_REP_INVALID;
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	    return returnStatus; 
	}

	/* determine psi */
        alpha = (jIndex - 1 == (iIndex) % 3) - (jIndex -1 == (iIndex + 1) % 3);

	/* save the Euler angle order so that the setup can be omitted if the
	   order is the same the next time the function is called */

	for (cnt = 0; cnt < 3; cnt++)
	  oldEAOrder[cnt] = eulerAngleOrder[cnt];
    } 
    
    /* determine Euler angles for a non-repeating sequence of Euler angles */

    if (indicesDiff == PGS_TRUE)
    {
	eulerAngle[1] = asin(2.0*(alpha*quatTemp[iIndex]*quatTemp[kIndex] +
				  quatTemp[jIndex]*quatTemp[0]));
	chi = atan2((quatTemp[iIndex] + quatTemp[kIndex]),
		       (alpha*quatTemp[jIndex] + quatTemp[0]));
	beta = atan2((quatTemp[iIndex] - quatTemp[kIndex]),
		    (-alpha*quatTemp[jIndex] + quatTemp[0]));
	eulerAngle[0] = chi + beta;
	eulerAngle[2] = chi - beta;
        for (cnt = 0; cnt < 3; cnt++)
        {
           if(eulerAngle[cnt] < -Pi) eulerAngle[cnt] += 2*Pi;
           if(eulerAngle[cnt] >  Pi) eulerAngle[cnt] -= 2*Pi;
        }
    }

    /* determine Euler angles for a repeating sequence of Euler angles */

    else 
    {
	chi = atan2(quatTemp[iIndex],quatTemp[0]);
	beta = atan2(alpha*quatTemp[kIndex],quatTemp[jIndex]);
/*  the following uses the identity 2*asin(x)=acos(1-2*x*x) and Schuster's Eq. 193,
     as well as the sum rule on quat. components squared.  PDN  */
	eulerAngle[1] = 2.0*asin(sqrt(quatTemp[jIndex]*quatTemp[jIndex] 
                                    + quatTemp[kIndex]*quatTemp[kIndex]));
	eulerAngle[0] = chi + beta;
	eulerAngle[2] = chi - beta;
        for (cnt = 0; cnt < 3; cnt++,cnt++)
        {
           if(eulerAngle[cnt] < -Pi) eulerAngle[cnt] += 2*Pi;
           if(eulerAngle[cnt] >  Pi) eulerAngle[cnt] -= 2*Pi;
        }
           if(eulerAngle[1] < 0.0 ) eulerAngle[1] += 2*Pi;
           if(eulerAngle[1] >  2.0*Pi) eulerAngle[1] -= 2*Pi;
    } 
    
#endif

    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
       
    return returnStatus;
}
