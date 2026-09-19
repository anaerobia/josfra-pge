/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_TiltYaw.c

DESCRIPTION:
   This file contains the function PGS_CSC_TiltYaw().
   
AUTHORS:
   Anubha Singhal / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   06-Oct-1994  AS  Initial Coding
   21-Mar-1995  PN  Rewrote, correcting the sense (direction) of the
                    transformation and streamlining the code.
   22-Mar-1995  PN  Added special cases for spacecraft instantaneously
                    at the equator, or instantaneous velocity North-South

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE: 
   This function obtains the tipped orbital to orbital quaternion.

NAME:   
   PGS_CSC_TiltYaw()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_TiltYaw( 
              PGSt_double  positionTCI[3],            
              PGSt_double  velocityTOD[3],
	      PGSt_double  equatRad_A,
              PGSt-double  polarRad_C,
              PGSt_double  quatTIPtoORB[4])   
	      
 FORTRAN:
      include 'PGS_CSC_4.f'
      include 'PGS_SMF.f'

      integer function pgs_csc_tiltyaw(positioneci,velocityeci,
     >                                 equatrad_a,polarrad_c,
     >                                 quattiptoorb)
      double precision positiontod(3)
      double precision velocitytod(3)
      double precision equatrad_a
      double precision polarrad_c
      double precision quattiptoorb(4)
    
DESCRIPTION:
   Obtains the tipped orbital to orbital transformation quaternion.

INPUTS:
   Name              Description            Units       Min        Max
   ----              -----------            -----       ---        --- 
   positionTOD       spacecraft position    meters      N/A        N/A
                     in TOD coordinates

   velocityTOD       spacecraft velocity    m/s         N/A        N/A
                     in TOD coordinates

   equatRad_A        equatorial radius      meters      N/A        N/A

   polarRad_C        polar radius           meters      N/A        N/A
                                                   
OUTPUTS:
   Name              Description            Units       Min        Max
   ----              -----------            ----        ---        ---
   quatTIPtoORB      tipped orbital to      N/A         N/A        N/A
                     orbital quaternion
                                                  
RETURNS:
   PGS_S_SUCCESS		 Success
   PGSCSC_E_ZERO_INPUT_VECTOR    The input position or velocity was 0 length
   PGS_E_TOOLKIT                 Toolkit error
 

EXAMPLES:
 C:
   PGSt_SMF_status    returnStatus;
   PGSt_double        positionTOD[3] = {343049304.0, 4343434.0, 434343434.0};
   PGSt_double        velocityTOD[3] = {400000.0,300023.0,21111.0};    
   PGSt_double        equatRad_A = 6378137.0;
   PGSt_double        polarRad_C = 6356752.314245;
   PGSt_double        quatTIPtoORB[4];   
  
   returnStatus = PGS_CSC_TiltYaw(positionTOD,velocityTOD,equatRad_A,
                                  polarRad_C,quatTIPtoORB)
                                                                      
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }

 FORTRAN:
      implicit none 
      integer             pgs_csc_tiltyaw
      integer		  returnstatus
      double precision    positioneci(3)
      double precision    velocityeci(3)
      double precision    equatrad_a
      double precision    polarrad_c
      double precision    quattiptoorb(4)
      character*33 	  err
      character*241 	  msg
      
      data positioneci/343049304.0, 4343434.0, 434343434.0/
      data velocityeci/400000.0, 300023.0, 21111.0/

      equatrad_a = 6378137.0
      polarrad_c = 6356752.314245
   
      returnstatus = pgs_csc_tiltyaw(positioneci,velocityeci,equatrad_a,
     >                               polarrad_c,quattiptoorb)

      if (returnstatus .ne. pgs_s_success) then
	  pgs_smf_getmsg(returnstatus, err, msg)
	  write(*,*) err, msg
      endif

NOTES:
   The algorithm is as follows:

   1) the tip angle is determined
   2) the x and z components of the North TOD vector in Orbital 
      Coordinates are found from position and velocity vectors
      (The TOD frame should be True of date for the case TRMM) 
   3) the y component of the North TOD vector in Orbital Coordinates
      is found from normalization and the condition that its sign
      agree with the sense (East or West) of spacecraft motion
   4) the tip axis vector rotAxisVector[3] is determined in Orbital
      coordinates from the fact that it is orthogonal to the radius
      and also orthogonal to the North vector, north[].
   4) using the tip angle and the axis of rotation in orbital coordinates,
      the quaternion is found

REQUIREMENTS:
   PGSTK - 1050 

DETAILS:
   See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
   ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
   for more information on the algorithm.

GLOBALS:
   NONE

FILES:
   NONE

FUNCTIONS CALLED:
   PGS_SMF_SetStaticMsg()
      
END_PROLOG
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_TiltYaw()"

PGSt_SMF_status
PGS_CSC_TiltYaw(                      /* obtains the tipped orbital to orbital
				         transformation matrix*/  
    PGSt_double     positionTOD[3],   /* TOD position of spacecraft */
    PGSt_double     velocityTOD[3],   /* TOD velocity of spacecraft */
    PGSt_double     equatRad_A,       /* equatorial earth radius */
    PGSt_double     polarRad_C,       /* polar radius */
    PGSt_double     quatTIPtoORB[4])  /* tip to orbital quaternion */
{
    PGSt_double     tipAngle;         /* tip angle */
    PGSt_double     norm;             /* norm of spacecraft position */
    PGSt_double     rotAxisVector[3]; /* the axis of rotation involved in 
				         tipping the z-axis to the "apparent 
				         nadir" in Orbital coordinates */
    PGSt_double     vPerpMag;         /* the length of the part of the velocity 
				         orthogonal to the radius */
    PGSt_double     vPerpZ;           /* the TOD Z component of the part of the 
				         velocity orthogonal to the radius  */
    PGSt_double     testEW;           /* test threshold for polar orbits */
    PGSt_double     signEW;           /* +1.0 for prograde, -1.0 for retrograde
				         (West traveling) spacecraft */
    PGSt_double     northTOD[3];      /* the unit North TOD vector in  
				         Orbital coordinates */
    PGSt_double     vdotrOverR2;      /* dot product of velocityTOD and position
				         vectors, divided by the square of the
				         distance to Earth center */
    short           iXYZ;             /* loop counter for space coordinates */
    PGSt_double     sinLat;           /* sine of spacecraft latitude */
    PGSt_double     cosLat;           /* cosine of spacecraft latitude */
    PGSt_double     sinTip;           /* sine of tip angle */    
    PGSt_SMF_status returnStatus;     /* error returnStatus of function call */
    
    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS; 

     /* initialize return quaternion for the case the norm of the
       positionTOD vector being zero  */

    quatTIPtoORB[0] = 1.0;
    quatTIPtoORB[1] = 0.0;
    quatTIPtoORB[2] = 0.0;
    quatTIPtoORB[3] = 0.0;
    
    norm = PGS_CSC_Norm(positionTOD);

    /* check for norm = 0  - meaning bad input position vector */

    if (fabs(norm) < EPS_64 || fabs(PGS_CSC_Norm(velocityTOD)) < EPS_64)
    {
       returnStatus = PGSCSC_E_ZERO_INPUT_VECTOR;
       PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
       return returnStatus;
    }

    /* check for spacecraft at equator or pole - no tip at all then */
    
    sinLat = positionTOD[2] / norm;
    cosLat = sqrt(1.0 - sinLat*sinLat);

    if ( (fabs(cosLat) < EPS_64) || (fabs(sinLat) < EPS_64) )
    {
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	return returnStatus;
    }
    
    /* compute the tip angle */

/* 8888 */
    tipAngle =  2.0 * (equatRad_A - polarRad_C) * (equatRad_A)
                     * sinLat * cosLat/(norm * norm);
    
    /* compute orbital x component of the TOD North vector.  
       To determine this, we need  vPerpZ, which can be found
       only by orthogonalizing the velocity to the position
       vector; the precise equation is vPerp[i] = v[i] - r[i]*
       vdotr/r*r, where r is positionTOD and v is velocityTOD */
       
    vdotrOverR2 = (velocityTOD[0]*positionTOD[0] + velocityTOD[1]*positionTOD[1]+
                   velocityTOD[2]*positionTOD[2])/(norm*norm); 

    vPerpZ      =       (velocityTOD[2] - positionTOD[2] * vdotrOverR2);
    vPerpMag    = sqrt( (velocityTOD[0] - positionTOD[0] * vdotrOverR2) *
                        (velocityTOD[0] - positionTOD[0] * vdotrOverR2) +
                        (velocityTOD[1] - positionTOD[1] * vdotrOverR2) *
                        (velocityTOD[1] - positionTOD[1] * vdotrOverR2) +
                         vPerpZ * vPerpZ);

    /* if the velocity and radius are parallel or the velocity is zero
       there is a serious error */

    if(fabs(vPerpMag) < EPS_64)
    {
       returnStatus = PGS_E_TOOLKIT;
       PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
       return returnStatus;
    }

    /* compute orbital x and z components of the TOD North vector */
    
    northTOD[0] =  vPerpZ/vPerpMag; 
    northTOD[2] = - sinLat;
    

    /* find if the orbit is prograde (Eastward) or retrograde */

    /* first find the angular momentum about the Z axis; if less
       than 0.5 (out of roughly 7000 m/s * 6378000 m) it is noise */

    testEW = (velocityTOD[1] * positionTOD[0] - velocityTOD[0] *
                   positionTOD[1]);

    /* determine sign of motion - Eastward or Westward - from angular
       mom. about Z axis, but if it is zero (within noise), spacecraft 
       is polar */

    if (fabs(testEW) > 0.5) 
    {
       signEW = (velocityTOD[1] * positionTOD[0] 
                - velocityTOD[0] * positionTOD[1]) > 0.0 ?  1.0 : -1.0;
    }
    else
    {
       signEW = 0.0; 
    }

    /* check for polar orbit (spacecraft at pole already checked) */
    if( 
        (fabs(signEW -(-1)) < EPS_64 ) && 
        ( 
          (fabs(velocityTOD[0]) + fabs(velocityTOD[1])) < EPS_64 
        ) 
       ) signEW = 0.0;

    /* compute orbital y component of the TOD North vector */

    northTOD[1] =  - signEW * sqrt(1.0 - northTOD[0] 
                   * northTOD[0] - northTOD[2] * northTOD[2]);
       
 if (fabs(signEW) > EPS_64)
    {
       rotAxisVector[0] =   northTOD[1];
       rotAxisVector[1] = - northTOD[0];
       rotAxisVector[2] = 0.0;
    
       norm = PGS_CSC_Norm(rotAxisVector);
    
       if (fabs(norm) > EPS_64 )
       {
          for (iXYZ = 0; iXYZ < 2; iXYZ++)
          rotAxisVector[iXYZ] = rotAxisVector[iXYZ] / norm;
       }
       else  /* unless spacecraft is at pole, you can always
                find an E-W vector at its location */
       {
          returnStatus = PGS_E_TOOLKIT;
          PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
          return returnStatus;
       }
     }
     else  /* polar orbit */
     /* the tilt vector points along the y axis if velocity is South,
        opposite if North */
     {
       rotAxisVector[0] = 0.0;
       rotAxisVector[1] =  (velocityTOD[2] > 0.0) ? -1.0 : 1.0;
       rotAxisVector[2] = 0.0;
     }
    /* determine the tip to orbital quaternion */
    
    sinTip = sin(tipAngle / 2.0);

    quatTIPtoORB[0] = cos(tipAngle / 2.0);
    for (iXYZ = 0; iXYZ < 3; iXYZ++)
    {
       quatTIPtoORB[iXYZ+1] = rotAxisVector[iXYZ] * sinTip;
    }
        
    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    return returnStatus;
}
