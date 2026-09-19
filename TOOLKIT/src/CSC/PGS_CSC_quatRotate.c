/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_quatRotate.c

DESCRIPTION:
   This file contains the function PGS_CSC_quatRotate().
   This function transforms a vector from one coordinate system to 
   to a rotated coordinate system with a common origin, where the
   rotation is defined by a quaternion.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   01-Feb-1994 GTSK Initial version
   01-Sep-1994 GTSK Modified prologs and variable types to comply with latest
                    ECS/PGS standards.  Added code to check for valid input
		    quaternion.  Added status message return values.

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************

BEGIN_PROLOG:

TITLE:
   Rotate vector
 
NAME:
   PGS_CSC_quatRotate()

DESCRIPTION:
   This function transforms a vector from one coordinate system to 
   to a rotated coordinate system with a common origin, where the
   rotation is defined by a quaternion.

SYNOPSIS:
   #include <PGS_CSC.h>

   PGSt_SMF_status
   PGS_CSC_quatRotate(
       PGSt_double  quat[4],
       PGSt_double  inVector[3],
       PGSt_double  outVector[3])

INPUTS:
   Name       Description               Units     Min     Max
   ----       -----------               -----     ---     ---
   quat       quaternion defining the   N/A       N/A     N/A
              the desired rotation
   
   inVector   vector in original        any       N/A     N/A
              coordinate system

OUTPUTS:
   Name       Description               Units     Min     Max
   ----       -----------               -----     ---     ---
   outVector  vector in rotated         any       N/A     N/A
              coordinate system

RETURNS:
   PGS_S_SUCCESS
   PGSCSC_E_BAD_QUATERNION

EXAMPLE:
   None

NOTES:
   The algorithm used by this function requires that the first component of
   the input quaternion is the scalar component of the quaternion.

REQUIREMENTS:
   Derived

DETAILS:
   The algorithm used here is as follows:

     R = Q*R0*Q'

     where:
        R  = 0 + r
	R0 = 0 + r0
	Q  = quaternion describing desired rotation
	Q' = the inverse (or conjugate) of Q
	r0 = vector in original coordinate system
	r  = vector in rotated coordinate system
	*  = denotes quaternion multiplication

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
*******************************************************************************/

#include <PGS_CSC.h>

PGSt_SMF_status
PGS_CSC_quatRotate(            /* use a quaternion to rotate a vector */
    PGSt_double  quat[4],      /* quaternion */
    PGSt_double  inVector[3],  /* vector in original coordinate system */
    PGSt_double  outVector[3]) /* vector in rotated coordinate system */
{
    PGSt_double  sumSquares;   /* sum of squares of quat components */

    PGSt_double  quat0;        /* quat[0] -- used to increase speed */
    PGSt_double  quat1;        /* quat[1] -- used to increase speed */
    PGSt_double  quat2;        /* quat[2] -- used to increase speed */
    PGSt_double  quat3;        /* quat[3] -- used to increase speed */

    PGSt_double  inVec0;       /* inVector[0] -- used to increase speed */
    PGSt_double  inVec1;       /* inVector[1] -- used to increase speed */
    PGSt_double  inVec2;       /* inVector[2] -- used to increase speed */

    PGSt_double  temp0;        /* 1st component of intermediate quaternion */
    PGSt_double  temp1;        /* 2nd component of intermediate quaternion */
    PGSt_double  temp2;        /* 3rd component of intermediate quaternion */
    PGSt_double  temp3;        /* 4th component of intermediate quaternion */

    /* Assign incoming quaternion and vector components to individual
       variables.  This is done in an attempt (brilliant? misguided? vain?) to
       make things speedy */

    quat0 = quat[0];
    quat1 = quat[1];
    quat2 = quat[2];
    quat3 = quat[3];

    inVec0 = inVector[0];
    inVec1 = inVector[1];
    inVec2 = inVector[2];
    
    /* Make sure the input quaternion is indeed a quaternion.  The sum of the
       squares of the components of a quaternion should equal one (1). */

    sumSquares = quat0*quat0 + quat1*quat1 + quat2*quat2 + quat3*quat3;
    if (sumSquares < PGSd_LOWER_LIMIT || sumSquares > PGSd_UPPER_LIMIT)
      return PGSCSC_E_BAD_QUATERNION;
    
    /* The first step is to multiply the incoming vector (as a quaternion) by
       the incoming quaternion (discarding terms that evaluate to zero) */

    temp0 = -quat1*inVec0 - quat2*inVec1 - quat3*inVec2;
    temp1 =  quat0*inVec0 + quat2*inVec2 - inVec1*quat3;
    temp2 =  quat0*inVec1 + inVec0*quat3 - quat1*inVec2;
    temp3 =  quat0*inVec2 + quat1*inVec1 - inVec0*quat2;

    /* Now multiply the inverse of the incoming quaternion by the quaternion
       resulting from the previous operation (discarding the "scalar" component
       of the result) */

    outVector[0] = quat0*temp1 - temp0*quat1 - temp2*quat3 + quat2*temp3;
    outVector[1] = quat0*temp2 - temp0*quat2 - quat1*temp3 + temp1*quat3;
    outVector[2] = quat0*temp3 - temp0*quat3 - temp1*quat2 + quat1*temp2;

    return PGS_S_SUCCESS;
}
