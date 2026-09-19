/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_EulerToQuat.c

DESCRIPTION:
   This file contains the function PGS_CSC_EulerToQuat().
   
AUTHOR:
   Peter Noerdlinger / Applied Research Corporation
   Anubha Singhal    / Applied Research Corporation
   Curt Schafer      / Steven Myers & Associates

HISTORY:
   27-Sep-1994 PDN Designed				
   03-Oct-1994 AS  Coded
   12-Jul-1995 PDN fixed prolog, examples
   12-Feb-1999 PDN fixed some notation
   09-Jul-1999 CS  Updated for Threadsafe functionality
   25-Aug-1999 PDN fixed calculation of alpha

END_FILE_PROLOG:
******************************************************************************/
/******************************************************************************
BEGIN_PROLOG

TITLE: 
   This function gets quaternions from euler angles.

NAME:   
   PGS_CSC_EulerToQuat()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_EulerToQuat( 
              PGSt_double  eulerAngle[3],            
              PGSt_integer eulerAngleOrder[3],
              PGSt_double  quat[4])   
	      
FORTRAN:
   include 'PGS_CSC_4.f'
   include 'PGS_SMF.f'

   integer function
     pgs_csc_eulertoquat(eulerangle,eulerangleorder,quat)
     double precision eulerangle[3]
     integer          eulerangleorder[3]
     double precision quat[4]
    
DESCRIPTION:
   Transforms Euler angles to Quaternions.

INPUTS:
   Name              Description         Units    Min        Max
   ----              -----------         -----    ---        --- 
   eulerAngle        Euler angles in     radians  -Pi        Pi
                     the specified order
   eulerAngleOrder   order of rotations  N/A      N/A        N/A  
                                                   
OUTPUTS:
   Name          Description                 Units       Min        Max
   ----          -----------                 ----        ---        ---
   quat          quaternion defining the     N/A         N/A        N/A
                 desired rotation                 
                                                  
RETURNS:
   PGS_S_SUCCESS		 Success
   PGSCSC_E_EULER_REP_INVALID    Euler angle order is invalid
   PGSCSC_E_EULER_INDEX_ERROR    Error in value of euler index
   PGSTSF_E_GENERAL_FAILURE      Bad return from PGS_TSF_GetTSFMaster or 
                                 PGS_TSF_GetMasterIndex()

EXAMPLES:
C:

   PGSt_SMF_status    returnStatus;
   PGSt_double        eulerAngle[3] = {0.5, 1.2, 2.1);
   PGSt_integer       eulerAngleOrder[3] = {3, 1, 2};
   PGSt_double        quat[4];   
  
   returnStatus = PGS_CSC_EulerToQuat(eulerAngle,eulerAngleOrder,quat)
                                                                      
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }

FORTRAN:
   integer                pgs_csc_eulertoquat 
   integer		  returnstatus
   double precision    	  eulerangle(3)
   integer                eulerangleorder(3)
   double precision       quat(4)
   character*33 	  err
   character*241 	  msg
 	
   data eulerAngle/0.5, 1.2, 2.1/
   data eulerangleorder/3, 1, 2/
   
   returnstatus = 
		  pgs_csc_eulertoquat(eulerangle,eulerangleorder,quat)

   if (returnstatus .ne. pgs_s_success) then
	 pgs_smf_getmsg(returnstatus, err, msg)
	 write(*,*) err, msg
   endif

NOTES:
   The indices 1,2,3 are used for the Euler angle order. They refer to the 
   rotations about the body axes labeled 1,2 and 3 which are assumed to be
   x,y and z. The angles of rotation are phi, theta and psi in the order 
   performed. Thus, if the Euler angle order is, for example, 2, 1, 3, it 
   means we rotate phi about axis 2, then by theta about new axis 1, then by 
   psi about new axis 3.

   If the 3 Euler rotation axes are all different(e.g 3,1,2), the quaternion 
   is obtained from the following equations (ref: Schuster[1] Eq(191))

   Quaternion    Quaternion Value
   index
   i             cos(psi) * cos(theta) * sin(phi) + 
                 alpha(i,j) * sin(psi) * sin(theta) * cos(phi)    
   
   j             cos(psi) * sin(theta) * cos(phi) -
	         alpha(i,j) * sin(psi) * cos(theta) * sin(phi)

   k             sin(psi) * cos(theta) * cos(phi) +
	         alpha(i,j) * cos(psi) * sin(theta) * sin(phi)

   0             cos(psi) * cos(theta) * cos(phi) - 
	         alpha(i,j) * sin(psi) * sin(theta) * sin(phi)

   If 2 axes duplicate (eg. 3,1,3), the quaternion is obtained from the
   following equations. In this case, k is defined as k = 6-i-j. (ref 
   Schuster[1] Eq(192))

   i             cos(theta) * sin(phi + psi)

   j             sin(theta) * cos(phi - psi)

   k             alpha(i,j) * sin(theta) * sin(phi - psi)

   0             cos(theta) * cos(phi + psi) 

   alpha(i,j) = 1.0 if {i,j} = 1,2 or 2,3 or 3,1
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
     PGSg_TSF_CSCEuleriIndex
     PGSg_TSF_CSCEulerjIndex
     PGSg_TSF_CSCEulerkIndex
     PGSg_TSF_CSCEuleralpha
     PGSg_TSF_CSCEuleroldEulerAngleOrder
     PGSg_TSF_CSCEulerindicesDiff

FILES:
   NONE

FUNCTIONS CALLED:
   PGS_SMF_SetStaticMsg()
   PGS_TSF_GetTSFMaster()
   PGS_TSF_GetMasterIndex()
      
END_PROLOG
******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_TSF.h>

PGSt_SMF_status
PGS_CSC_EulerToQuat(                /* obtains the  quaternion from the
                                       Euler angle */  
    PGSt_double	 eulerAngle[],      /* euler angles */ 
    PGSt_integer eulerAngleOrder[], /* order of the rotations */
    PGSt_double  quat[])            /* output quaternion */
{  
    short               cnt;        /* loop counter */
    PGSt_double         phi;        /* 1st Euler angle divided by two */
    PGSt_double         theta;      /* 2nd Euler angle divided by two */
    PGSt_double         psi;        /* 3rd Euler angle divided by two */
    static PGSt_integer iIndex;     /* index i */
    static PGSt_integer jIndex;     /* index j */
    static PGSt_integer kIndex;     /* index k */
    static PGSt_boolean indicesDiff;/* whether a symmetric or asymmetric euler 
				       angle order */
    static PGSt_integer oldEulerAngleOrder[3]; 
                                    /* saved euler angle order */
    static PGSt_integer alpha;      /* right hand or left hand order */
    static PGSt_boolean first = PGS_TRUE; /* set to true if no setup has
                                             occurred */ 
    PGSt_SMF_status returnStatus;   /* error returnStatus of function call */

#ifdef _PGS_THREADSAFE

   /* Declare variables used for THREADSAFE version to replace statics
      The local names are appended with TSF and globals are preceded with
      directory and function name */

    PGSt_TSF_MasterStruct *masterTSF;
    PGSt_integer iIndexTSF;
    PGSt_integer jIndexTSF;
    PGSt_integer kIndexTSF;
    PGSt_integer alphaTSF;
    PGSt_integer oldEulerAngleOrderTSF[3];
    PGSt_boolean indicesDiffTSF;
    PGSt_integer x;                   /* loop counter for initialization */

    /* Globals     originals in PGS_TSF_SetupCSC.c */

    extern PGSt_integer PGSg_TSF_CSCEuleriIndex[];
    extern PGSt_integer PGSg_TSF_CSCEulerjIndex[];
    extern PGSt_integer PGSg_TSF_CSCEulerkIndex[];
    extern PGSt_integer PGSg_TSF_CSCEuleralpha[];
    extern PGSt_integer PGSg_TSF_CSCEuleroldEulerAngleOrder[][3];
    extern PGSt_boolean PGSg_TSF_CSCEulerindicesDiff[];
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

    iIndexTSF = PGSg_TSF_CSCEuleriIndex[masterTSFIndex];
    jIndexTSF = PGSg_TSF_CSCEulerjIndex[masterTSFIndex];
    kIndexTSF = PGSg_TSF_CSCEulerkIndex[masterTSFIndex];
    alphaTSF = PGSg_TSF_CSCEuleralpha[masterTSFIndex];
    indicesDiffTSF = PGSg_TSF_CSCEulerindicesDiff[masterTSFIndex];
    for (x=0; x<3; x++)
    {
        oldEulerAngleOrderTSF[x] = 
             PGSg_TSF_CSCEuleroldEulerAngleOrder[masterTSFIndex][x];
    }

    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS; 
    
    /* check if we have used this function  with the same euler angle order - 
       If so, omit setup and proceed to obtain the quaternions */ 

    /* Almost entire function is duplicated for the THREADSAFE version to
    protect statics.  When a value is reassigned the global or key is updated                      NO LOCKS  NO KEYS   6 GLOBALS                          */

    /* Threadsafe Protect: oldEulerAngleOrder  */
 
    if ((eulerAngleOrder[0] != oldEulerAngleOrderTSF[0]) ||
	(eulerAngleOrder[1] != oldEulerAngleOrderTSF[1]) ||
	(eulerAngleOrder[2] != oldEulerAngleOrderTSF[2]) ||
        (first == PGS_TRUE))
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
	      PGS_SMF_SetStaticMsg(returnStatus,"PGS_CSC_EulerToQuat()");
	      return returnStatus;
	  }

        /* first rotation, second rotation and third rotation axes. Note that
           in the case of a symmetric sequence the k index will change */

        /* Threadsafe Protect: iIndex,jIndex, kIndex
                 Reassign values  */ 

	iIndexTSF = eulerAngleOrder[0];
        PGSg_TSF_CSCEuleriIndex[masterTSFIndex] = iIndexTSF;
	jIndexTSF = eulerAngleOrder[1];
        PGSg_TSF_CSCEulerjIndex[masterTSFIndex] = jIndexTSF;
	kIndexTSF = eulerAngleOrder[2];
        PGSg_TSF_CSCEulerkIndex[masterTSFIndex] = kIndexTSF;

	/* test to see if all the values in the euler angle order are 
           different */

	if (((iIndexTSF - jIndexTSF) * (iIndexTSF - kIndexTSF) * 
                                    (jIndexTSF - kIndexTSF)) != 0)
	{
            /* Threadsafe Protect: indicesDiff    Reassign values  */

	    indicesDiffTSF = PGS_TRUE;
            PGSg_TSF_CSCEulerindicesDiff[masterTSFIndex] = indicesDiffTSF;
	}

        /* test to see if the last and first indices are the same and the 
	   middle one is different */
        /* Threadsafe Protect: iIndex,jIndex, kIndex
                 Reassign values  */ 

	else if ((iIndexTSF == kIndexTSF) && (jIndexTSF != kIndexTSF))
        {
	    kIndexTSF = 6 - iIndexTSF - jIndexTSF;
            PGSg_TSF_CSCEulerkIndex[masterTSFIndex] = kIndexTSF;
	    indicesDiffTSF = PGS_FALSE;
            PGSg_TSF_CSCEulerindicesDiff[masterTSFIndex] = indicesDiffTSF;
	}
     
        /* euler angle order is invalid - issue error and return */

	else 
	{
	    returnStatus = PGSCSC_E_EULER_REP_INVALID;
	    PGS_SMF_SetStaticMsg(returnStatus,"PGS_CSC_EulerToQuat()");
	    return returnStatus; 
	}

	/* determine alpha */
        /* Threadsafe Protect: alpha     Reassign values  */

        alphaTSF = (jIndexTSF - 1  == (iIndexTSF) % 3) - 
                            (jIndexTSF - 1 == (iIndexTSF + 1) % 3);
        PGSg_TSF_CSCEuleralpha[masterTSFIndex]  = alphaTSF;

	/* save the Euler angle order so that the setup can be omitted if the
	   order is the same the next time the function is called */
        /* Threadsafe Protect: oldEulerAngleOrder     Reassign values  */

	for (cnt = 0; cnt < 3; cnt++)
	{
	    oldEulerAngleOrderTSF[cnt] = eulerAngleOrder[cnt];
            PGSg_TSF_CSCEuleroldEulerAngleOrder[masterTSFIndex][cnt] = 
                                           oldEulerAngleOrderTSF[cnt];
        }

        if (first == PGS_TRUE)
	  first = PGS_FALSE;
    } 

    phi   = eulerAngle[0] / 2;
    theta = eulerAngle[1] / 2;
    psi   = eulerAngle[2] / 2;
    
    /* determine quaternion for a asymmetric sequence of Euler angles */
    /* Threadsafe Protect: alpha */

    if (indicesDiffTSF == PGS_TRUE)
    {
	quat[0]      = cos(psi) * cos(theta) * cos(phi) - 
	                      alphaTSF * sin(psi) * sin(theta) * sin(phi);
	quat[iIndexTSF] = cos(psi) * cos(theta) * sin(phi) +
	                      alphaTSF * sin(psi) * sin(theta) * cos(phi);
        quat[jIndexTSF] = cos(psi) * sin(theta) * cos(phi) -
	                      alphaTSF * sin(psi) * cos(theta) * sin(phi);
        quat[kIndexTSF] = sin(psi) * cos(theta) * cos(phi) +
	                      alphaTSF * cos(psi) * sin(theta) * sin(phi);
    }
    /* determine quaternion for a symmetric sequence of Euler angles */

    else 
    {
	quat[0]      = cos(theta) * cos(phi + psi);
	quat[iIndexTSF] = cos(theta) * sin(phi + psi);
	quat[jIndexTSF] = sin(theta) * cos(phi - psi);
	quat[kIndexTSF] = alphaTSF * sin(theta) * sin(phi - psi);
    } 
#else    

    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS; 
    
    /* check if we have used this function  with the same euler angle order - 
       If so, omit setup and proceed to obtain the quaternions */ 
       
    
    if ((eulerAngleOrder[0] != oldEulerAngleOrder[0]) ||
	(eulerAngleOrder[1] != oldEulerAngleOrder[1]) ||
	(eulerAngleOrder[2] != oldEulerAngleOrder[2]) ||
        (first == PGS_TRUE))
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
	      PGS_SMF_SetStaticMsg(returnStatus,"PGS_CSC_EulerToQuat()");
	      return returnStatus;
	  }

        /* first rotation, second rotation and third rotation axes. Note that
           in the case of a symmetric sequence the k index will change */

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
	    PGS_SMF_SetStaticMsg(returnStatus,"PGS_CSC_EulerToQuat()");
	    return returnStatus; 
	}

	/* determine alpha */
        alpha = (jIndex - 1  == (iIndex) % 3) - (jIndex - 1 == (iIndex + 1) % 3);

	/* save the Euler angle order so that the setup can be omitted if the
	   order is the same the next time the function is called */

	for (cnt = 0; cnt < 3; cnt++)
	  oldEulerAngleOrder[cnt] = eulerAngleOrder[cnt];

        if (first == PGS_TRUE)
	  first = PGS_FALSE;
    } 

    phi   = eulerAngle[0] / 2;
    theta = eulerAngle[1] / 2;
    psi   = eulerAngle[2] / 2;
    
    /* determine quaternion for a asymmetric sequence of Euler angles */

    if (indicesDiff == PGS_TRUE)
    {
	quat[0]      = cos(psi) * cos(theta) * cos(phi) - 
	                      alpha * sin(psi) * sin(theta) * sin(phi);
	quat[iIndex] = cos(psi) * cos(theta) * sin(phi) +
	                      alpha * sin(psi) * sin(theta) * cos(phi);
        quat[jIndex] = cos(psi) * sin(theta) * cos(phi) -
	                      alpha * sin(psi) * cos(theta) * sin(phi);
        quat[kIndex] = sin(psi) * cos(theta) * cos(phi) +
	                      alpha * cos(psi) * sin(theta) * sin(phi);
    }
    /* determine quaternion for a symmetric sequence of Euler angles */

    else 
    {
	quat[0]      = cos(theta) * cos(phi + psi);
	quat[iIndex] = cos(theta) * sin(phi + psi);
	quat[jIndex] = sin(theta) * cos(phi - psi);
	quat[kIndex] = alpha * sin(theta) * sin(phi - psi);
    } 
#endif    
    PGS_SMF_SetStaticMsg(returnStatus,"PGS_CSC_EulerToQuat()");
       
    return returnStatus;
}
