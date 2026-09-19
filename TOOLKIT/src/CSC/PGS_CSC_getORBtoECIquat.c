/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
   PGS_CSC_getORBtoECIquat.c
 
DESCRIPTION:
   This file contains the function PGS_CSC_getORBtoECIquat().
   This function returns a quaternion describing the rotation from the Earth
   Centered Inertial (ECI) reference frame to the Orbital (ORB) reference frame.
   That is the quaternion returned will transform a vector in the ORB frame to
   the equivalent vector in the ECI frame.
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
 
HISTORY:
   12-Apr-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/
 
/*******************************************************************************
BEGIN_PROLOG:
 
TITLE:
   Get ORB to ECI rotation quaternion
 
NAME:
   PGS_CSC_getORBtoECIquat()
 
SYNOPSIS:
   #include <PGS_CSC.h>
      
   PGSt_SMF_status
   PGS_CSC_getORBtoECIquat(
       PGSt_double   positionECI[3],
       PGSt_double   velocityECI[3],
       PGSt_double   quatORBtoECI[4])

DESCRIPTION:
   This function returns a quaternion describing the rotation from the Orbital
   (ORB) reference frame to the Earth Centered Inertial (ECI) reference frame.
   That is the quaternion returned will transform a vector in the ORB frame to
   the equivalent vector in the ECI frame.

INPUTS:
   Name           Description                 Units       Min   Max
   ----           -----------                 -----       ---   ---
   positionECI    ECI position vector of      meters      ANY   ANY
                  spacecraft

   velocityECI    ECI velocity vector or      m/s         ANY   ANY
                  spacecraft
OUTPUTS:
   Name           Description                 Units       Min    Max
   ----           -----------                 -----       ---    ---
   quatORBtoECI   quaternion describing       N/A         N/A    N/A
                  rotation from ORB to ECI
		  reference frames
          
RETURNS:
   PGS_S_SUCCESS               successful return
   PGSCSC_E_QUAT_NOT_FOUND     ORB to ECI quaternion could not be determined
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
   None

NOTES:
   None
 
REQUIREMENTS:
   None
 
DETAILS:
   This routine calculates the orbital reference frame to ECI frame coordinate
   transformation matrix from the input spacecraft (s/c) ECI position and
   velocity vectors.  The transformation matrix is the 3X3 matrix whose columns
   consist of the X, Y, and Z unit vectors of the orbital reference frame
   rendered in ECI coordinates.  Positive Z is defined as the anti-radius of the
   spacecraft, that is the unit vector along the radius vector pointing from the
   spacecraft to the center of the earth.  The X axis is defined to be in the
   plane defined by the spacecraft position and velocity vectors, perpendicular
   to the Z axis and in the same general direction as the velocity vector.  The
   Y axis is the cross-product of the Z and X axis unit vectors.

GLOBALS:
   None
 
FILES:
   None
 
FUNCTIONS_CALLED:
   PGS_SMF_SetDynamicMsg()
   PGS_CSC_getQuats()
   PGS_SMF_SetUnknownMsg()
 
END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_getORBtoECIquat()"

PGSt_SMF_status
PGS_CSC_getORBtoECIquat(
    PGSt_double positionECI[3],   /* s/c position in ECI coordinates */
    PGSt_double velocityECI[3],   /* s/c velocity in ECI coordinates */
    PGSt_double quatORBtoECI[4])  /* quaternion defining rotation (ONLY) from 
				     orbital reference to ECI reference frame 
				     frame */
{
    PGSt_double  rdotr;           /* dot product of position vector with
				     itself */
    PGSt_double  r;               /* magnitude of position vector */
    PGSt_double  vdotr;           /* dot product of velocity vector and position
				     vector */
    PGSt_double  vcg;             /* normalizing factor for 1st column of
				     transformation  matrix */
    PGSt_double  transform[3][3]; /* orbital to ECI transformation matrix */

    PGSt_integer j;               /* loop counter */
    
    PGSt_SMF_status returnStatus; /* return value of call to PGS function */

    /* to determine the transformation matrix the unit vectors along
       the X-Y-Z axes of the orbital reference frame are rendered in
       their ECI components. */

    rdotr = PGS_CSC_dotProduct(positionECI,positionECI,3);
    vdotr = PGS_CSC_dotProduct(velocityECI,positionECI,3);
    r = sqrt(rdotr);

    /* if position vector is zero the orbital coordinate system cannot be
       usefully determined */

    if (rdotr < 1.e-10)
    {
	PGS_SMF_SetDynamicMsg(PGSCSC_E_QUAT_NOT_FOUND,
			      "encountered zero length position vector, "
			      "orbital coordinate system cannot be defined",
			      FUNCTION_NAME);
	return PGSCSC_E_QUAT_NOT_FOUND;
    }
    
    
    /* Z-axis is unit vector toward nadir (anti-radius) */

    for (j=0;j<3;j++)
    {
        transform[j][2] = -positionECI[j]/r;                        /* Z-axis */
        transform[j][0] = velocityECI[j]-vdotr*positionECI[j]/rdotr;/* X-axis */
    }

    /* X-axis is close to velocity, but normal to R in R-V plane.  The X-axis
       components were calculated in the above loop, here they are being
       normalized. */

    vcg=sqrt(transform[0][0]*transform[0][0] + transform[1][0]*transform[1][0] +
	     transform[2][0]*transform[2][0]);

    /* If vcg is 0.0 then the velocity vector is either zero or coincident with
       the position vector, either way the orbital coordinate system cannot be
       usefully determined.  Otherwise... */

    if (fabs(vcg) < 1.e-10)
    {
	PGS_SMF_SetDynamicMsg(PGSCSC_E_QUAT_NOT_FOUND,
			      "velocity vector has zero length or is coincident"
			      " with position vector, orbital coordinate system"
			      " cannot be defined",
			      FUNCTION_NAME);
	return PGSCSC_E_QUAT_NOT_FOUND;
    }

    /* ...normalize. */

    for (j=0;j<3;j++)
      transform[j][0]=transform[j][0]/vcg;

    /* Y-axis is Z cross X (already unitized)
       i.e. Z-axis is column 3 of transformation matrix, and X-axis is column 1
            of transformation matrix, therefore Y-axis is the cross-product of
            column 3 and column 1.  Note of course that the cross-product of two
            orthogonal unit vectors is another unit vector so no normalization
            is necessary */

    transform[0][1] = transform[1][2]*transform[2][0] -
                      transform[2][2]*transform[1][0];
    transform[1][1] = transform[2][2]*transform[0][0] -
                      transform[0][2]*transform[2][0];
    transform[2][1] = transform[0][2]*transform[1][0] -
                      transform[1][2]*transform[0][0];

    returnStatus = PGS_CSC_getQuats(transform,quatORBtoECI);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
}

