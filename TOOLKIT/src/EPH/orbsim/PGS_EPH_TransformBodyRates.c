/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_EPH_TransformBodyRates.c

DESCRIPTION:
   This file contains the function PGS_EPH_TransformBodyRates().
   This function transforms spacecraft body rotation rates between the orbital
   (ORB) and inertial (ECI) reference frames.

AUTHOR:
   Peter D. Noerdlinger / Space Applications Corporation
   Guru Tej S. Khalsa / Space Applications Corporation

HISTORY:
   02-Oct-1997  GTSK  Initial version.  Algorithm provided by PDN.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Transform Body Rotation Rates

NAME:
   PGS_EPH_TransformBodyRates()

SYNOPSIS:
C:
   #include <PGS_EPH.h>

   PGSt_SMF_status
   PGS_EPH_TransformBodyRates(
        PGSt_dirTag     direction,
	PGSt_double     positionECI[3], 
	PGSt_double     velocityECI[3], 
	PGSt_double     eulerAngles[3],         
        PGSt_integer    eulerAngleOrder[3],
	PGSt_double     inputBodyRates[3],
	PGSt_double     outputBodyRates[3])

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD.f'
      include 'PGS_TD_3.f'
      include 'PGS_EPH_5.f'
      include 'PGS_MEM_7.f'

      integer function pgs_csc_transformbodyrates(direction,
     >                                            positioneci,
     >                                            velocityeci,
     >                                            eulerangles,
     >                                            eulerangleorder,
     >                                            inputbodyrates,
     > 				                  outputbodyrates)
      integer           direction
      double precision  positioneci(3)
      double precision  velocityeci(3)
      double precision  eulerangles(3)
      integer           eulerangleorder(3)
      double precision  inputbodyrates(3)
      double precision  outputbodyrates(3)

DESCRIPTION:
   This function transforms spacecraft body rotation rates between the orbital
   (ORB) and intertial (ECI) reference frames.

INPUTS:
   Name              Description                              Units
   ----              -----------                              -----
   direction         transform direction flag (should be      N/A
                     one of: PGSe_ECItoORB or PGSe_ORBtoECI)

   positionECI       ECI position vector                      meters

   velocityECI       ECI velocity vector                      meters/sec

   eulerAngles       s/c attitude as a set of Euler angles    radians

   eulerAngleOrder   array of integer values specifying
                     Euler angle order (e.g. 3,1,2)

   inputBodyRates    angular rates about body x, y and z     radians/sec
                     axes (in input reference frame)

OUTPUTS:
   Name              Description                             Units
   ----              -----------                             -----
   outputBodyRates   angular rates about body x, y and z     radians/sec
                     axes (in output reference frame)
          
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
C:
        #define ARRAY_SIZE 10

	PGSt_double     positionECI[3];
	PGSt_double     velocityECI[3];
	PGSt_double     eulerAngles[3];
	PGSt_double     bodyRatesECI[3];
	PGSt_double     bodyRatesORB[3];

	PGSt_integer    eulerAngleRates[3]={3,1,2};
	
	PGSt_SMF_status returnStatus;


	** initialize variables **

	positionECI[0] = 195225.557;
	positionECI[1] = 5177116.421;
	positionECI[2] = -4840814.041;
	velocityECI[0] = 1649.022;
	velocityECI[1] = 4951.011;
	velocityECI[2] = 5374.462;
	eulerAngles[0] = 0.0;
	eulerAngles[1] = 0.0;
	eulerAngles[2] = 0.0;
	bodyRatesECI[0] = 0.0;
	bodyRatesECI[1] = 0.0;
	bodyRatesECI[2] = 0.0;

        returnStatus = PGS_EPH_TransformBodyRates(
                                          PGSe_ECItoORB,positionECI,velocityECI,
					  eulerAngles,eulerAngleOrder,
					  bodyRatesECI, bodyRatesORB);

	if (returnStatus != PGS_S_SUCCESS)
	{
	            :
	 ** do some error handling ***
		    :
	}
   
FORTRAN:
      implicit none

      include  'PGS_SMF.f'
      include  'PGS_TD.f'
      include  'PGS_TD_3.f'
      include  'PGS_EPH_5.f'
      include  'PGS_MEM_7.f'

      integer           pgs_eph_transformbodyrates

      integer           numvalues/10/

      double precision  positioneci(3)
      double precision  velocityeci(3)
      double precision  eulerangles(3)
      double precision  bodyrateseci(3)
      double precision  bodyratesorb(3)
			
      integer           euleranglerates(3)/3,1,2/
			
      integer           returnstatus;


!  initialize variables

      positioneci(1) = 195225.557
      positioneci(2) = 5177116.421
      positioneci(3) = -4840814.041
      velocityeci(1) = 1649.022
      velocityeci(2) = 4951.011
      velocityeci(3) = 5374.462
      eulerangles(1) = 0.0
      eulerangles(2) = 0.0
      eulerangles(3) = 0.0
      bodyrateseci(1) = 0.0
      bodyrateseci(2) = 0.0
      bodyrateseci(3) = 0.0
      
      returnstatus = pgs_eph_transformbodyrates(
     >                                    pgse_ecitoorb, positioneci,
     >				          velocityeci, eulerangles,
     >					  eulerangleorder, bodyrateseci,
     >					  bodyratesorb)

      if (returnstatus .ne. pgs_s_success) then
                :
      *** do some error handling ***
                :
      endif

NOTES:
   The transformation sense is as follows:

   PGSe_ECItoORB:
   The rates are initially those of the spacecraft as measured in inertial
   space, projected on the body axes; the final rates are the angular velocity
   of the spacecraft in the orbital frame, projected on body reference axes.

   PGSe_ORBtoECI:
   The rates are initially those of the spacecraft as measured in the orbital
   reference frame, projected on body axes.  The final rates are the spacecraft
   rates in inertial space, projected on the body axes.

REQUIREMENTS:
   PGSTK - 0141, 0720, 0740

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_CSC_dotProduct()          returns dot product of two vectors
   PGS_CSC_crossProduct()        calculates cross product of two vectors
   PGS_EPH_matrixMultiply()      multiplies two 3x3 matrices
   PGS_CSC_Norm()                returns norm of 3-vector
   PGS_CSC_quatRotate()          rotates a 3-vector
   PGS_CSC_EulerToQuat()         converts Euler angles to a quaternion
   PGS_SMF_SetStaticMsg()        sets the message buffer
   PGS_SMF_SetDynamicMsg()       sets the message buffer

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <PGS_CSC.h>
#include <PGS_SIM.h>

#define GMe 3.986004415E14  /* (gravitational const.)*(mass of Earth) m^3/s^2 */
#define J2  0.0010826269    /* dimensionless Earth quadrupole moment */
#define Re  6.37E6          /* Radius of Earth in meters */

/* name of this function */

#define FUNCTION_NAME "PGS_EPH_TransformBodyRates()"

PGSt_SMF_status
PGS_EPH_TransformBodyRates(
    PGSt_integer direction,              /* transformation direction flag */
    PGSt_double  positionECI[3],         /* ECI position vector */
    PGSt_double  velocityECI[3],         /* ECI velocity vector */
    PGSt_double  eulerAngles[3],         /* Euler angles array */
    PGSt_integer eulerAngleOrder[3],     /* Euler angle order array */
    PGSt_double  inputBodyRate[3],       /* array of body rotation rates (in) */
    PGSt_double  outputBodyRate[3])      /* array of body rot. rates (out) */
{
    PGSt_integer i;                      /* looping variable */

    PGSt_double* bodyRateECI;            /* body rotation rates in ECI frame */
    PGSt_double* bodyRateORB;            /* body rotation rates in ORB frame */

    PGSt_double  coeff1;                 /* temporary coefficient variable */
    PGSt_double  coeff2;                 /* temporary coefficient variable */
    PGSt_double  r;                      /* magnitude of ECI position */
    PGSt_double  r2;                     /* mag. of ECI position, squared */
    PGSt_double  r3;                     /* mag. of ECI position, cubed */
    PGSt_double  v2;                     /* mag. of ECI velocity, squared */
    PGSt_double  r_dot_v;                /* dot product of pos. and vel. */
    PGSt_double  v_dot_a;                /* dot product of vel. and accel. */
    PGSt_double  r_dot_a;                /* dot product of pos. and accel. */

    PGSt_double  x[3];                   /* ORB X unit vector in ECI coords. */
    PGSt_double  y[3];                   /* ORB Y unit vector in ECI coords. */
    PGSt_double  z[3];                   /* ORB Z unit vector in ECI coords. */
    PGSt_double  dx_dt[3];               /* dX/dt (see above) */
    PGSt_double  dy_dt[3];               /* dY/dt (see above) */
    PGSt_double  dz_dt[3];               /* dZ/dt (see above) */
    PGSt_double  v_cross_r[3];           /* cross product of vel. and pos. */
    PGSt_double  accelerationECI[3];     /* ECI acceleration vector */
    PGSt_double  orbitalFrameRateECI[3]; /* accel. of ORB frame in ECI coords */
    PGSt_double  result[3];              /* scratch vector */

    PGSt_double  Aoi_trans[3][3];        /* transpose of ECI to ORB attitude
					    matrix */
    PGSt_double  Aoi_dot[3][3];          /* time derivative of ECI to ORB
					    attitude matrix */
    PGSt_double  Omega_oi[3][3];         /* angular velocity of ORB frame in ECI
					    coords (expressed as matrix) */

    PGSt_double  quatORBtoSC[4];         /* ORB to spacecraft (SC) quaternion */

    PGSt_SMF_status returnStatus;        /* return status of Toolkit calls */

    /* check the value of the input direction flag to determine what the input
       and output body rates represent (return an error if the flag is not one
       of the two supported values) */

    switch (direction)
    {
      case PGSe_ORBtoECI:
	bodyRateORB = inputBodyRate;
	bodyRateECI = outputBodyRate;
	break;

      case PGSe_ECItoORB:
	bodyRateECI = inputBodyRate;
	bodyRateORB = outputBodyRate;
	break;

      default:
	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
			      "Unexpected value of the input 'direction' "
			      "switch.  Value must be either PGSe_ORBtoECI "
			      "or PGSe_ECItoORB.",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* define some scalar quantities */

    r = PGS_CSC_Norm(positionECI);    /* magnitude of position */
    r2 = r*r;                         /* magnitude of position, squared */
    r3 = r2*r;                        /* magnitude of position, cubed */
    v2 = PGS_CSC_Norm(velocityECI);   /* magnitude of velocity */
    v2 = v2*v2;                       /* magnitude of velocity, squared */


    r_dot_v = PGS_CSC_dotProduct(positionECI, velocityECI, 3); /* dot product of
								  position and
								  velocity
								  (V o R) */
    PGS_CSC_crossProduct(velocityECI, positionECI, v_cross_r); /* cross product
								  of velocity
								  and position
								  (V x R) */

    /* calculate the Z unit vector (R/r) */

    z[0] = -positionECI[0]/r;
    z[1] = -positionECI[1]/r;
    z[2] = -positionECI[2]/r;

    /* calculate the Y unit vector (V x R)/|(V X R)| */

    coeff1 = PGS_CSC_Norm(v_cross_r);
    y[0] = v_cross_r[0]/coeff1;
    y[1] = v_cross_r[1]/coeff1;
    y[2] = v_cross_r[2]/coeff1;

    /* calculate the X unit vector (Y x Z) */

    PGS_CSC_crossProduct(y, z, x);
    
    /* create the acceleration vector */

    coeff1 = -(GMe/r3);

    /* A = [-(GMe)/r^3]*R */

    accelerationECI[0] = coeff1*positionECI[0];
    accelerationECI[1] = coeff1*positionECI[1];
    accelerationECI[2] = coeff1*positionECI[2];

    coeff1 = 3.0/2.0*J2*(Re*Re/r2)*(GMe/r3);
    coeff2 = coeff1*(5.0*pow(positionECI[2],2.0)/r2 - 1);

    accelerationECI[0] += coeff2*positionECI[0];
    accelerationECI[1] += coeff2*positionECI[1];
    
    coeff2 = coeff1*(3 - 5.0*pow(positionECI[2],2.0)/r2);
    
    accelerationECI[2] += coeff2*positionECI[2];

    /* calculate dot products of velocity and acceleration as well as position
       and acceleration */

    v_dot_a = PGS_CSC_dotProduct(velocityECI, accelerationECI, 3);
    r_dot_a = PGS_CSC_dotProduct(positionECI, accelerationECI, 3);

    /* determine dZ/dt (time derivative of the Z unit vector) */

    coeff1 = r_dot_v/r3;   /* coefficient of the second terms */

    /* dZ/dt = -V/r + coeff1*R */

    dz_dt[0] = -velocityECI[0]/r + coeff1*positionECI[0];
    dz_dt[1] = -velocityECI[1]/r + coeff1*positionECI[1];
    dz_dt[2] = -velocityECI[2]/r + coeff1*positionECI[2];

    /* determine dY/dt (time derivative of the Y unit vector) */

    PGS_CSC_crossProduct(accelerationECI, positionECI, dy_dt);
    PGS_CSC_crossProduct(velocityECI, positionECI, v_cross_r);

    coeff1 = PGS_CSC_Norm(v_cross_r);               /* coefficient of the first
						       terms */
    coeff2 = pow(coeff1, 3.0);                      /* intermediate step */
    coeff2 = (v_dot_a*r2 - r_dot_a*r_dot_v)/coeff2; /* coefficient of the
						       second terms */
    /* dY/dt = (A x R)/(coeff1) - (coeff2)*(V x R) */

    dy_dt[0] = dy_dt[0]/coeff1 - coeff2*v_cross_r[0];
    dy_dt[1] = dy_dt[1]/coeff1 - coeff2*v_cross_r[1];
    dy_dt[2] = dy_dt[2]/coeff1 - coeff2*v_cross_r[2];
    
    /* determine dX/dt (time derivative of the X unit vector */

    PGS_CSC_crossProduct(dy_dt, z, dx_dt); /* dY/dt x Z (intermediate step) */

    coeff1 = coeff1/r3;   /* |V x R|/r^3 (coefficient of second term) */

    /* dX/dt = (dY/dt x Z) - coeff1*R */

    dx_dt[0] = dx_dt[0] - coeff1*positionECI[0];
    dx_dt[1] = dx_dt[1] - coeff1*positionECI[1];
    dx_dt[2] = dx_dt[2] - coeff1*positionECI[2];

    for (i=0;i<3;i++)
    {
	/* Aoi_trans is the transpose of the Matrix that will rotate a vector
	   from the inertial to the orbital reference frame */

	Aoi_trans[i][0] = x[i];
	Aoi_trans[i][1] = y[i];
	Aoi_trans[i][2] = z[i];

	/* Aoi_dot is the time derivative of the Matrix that will rotate a
	   vector from the inertial to the orbital reference frame */

	Aoi_dot[0][i] = dx_dt[i];
	Aoi_dot[1][i] = dy_dt[i];
	Aoi_dot[2][i] = dz_dt[i];
    }

    PGS_EPH_matrixMultiply(Aoi_dot, Aoi_trans, Omega_oi);

    orbitalFrameRateECI[0] = Omega_oi[1][2];
    orbitalFrameRateECI[1] = Omega_oi[2][0];
    orbitalFrameRateECI[2] = Omega_oi[0][1];

    returnStatus = PGS_CSC_EulerToQuat(eulerAngles, eulerAngleOrder,
				       quatORBtoSC);
    if (returnStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
			      "Error attempting to convert input Euler angles "
			      "to quaternion.",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    for (i=1;i<4;i++)
    {
	quatORBtoSC[i] = -quatORBtoSC[i];
    }

    /* call PGS_CSC_quatRotate to transform the vector from the Orbital
       coordinate system to the rotated SC coordinate system where the
       rotation is defined by a quaternion */

    returnStatus = PGS_CSC_quatRotate(quatORBtoSC, orbitalFrameRateECI,
				      result);
    if (returnStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
			      "Invalid quaternion generated from input Euler "
			      "angles.",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    switch (direction)
    {
      case PGSe_ORBtoECI:

	/* transform body rates in the orbital reference frame (ORB) to the
	   equivalent rates expressed in the inertial frame (ECI) */

	bodyRateECI[0] = bodyRateORB[0] + result[0];
	bodyRateECI[1] = bodyRateORB[1] + result[1];
	bodyRateECI[2] = bodyRateORB[2] + result[2];
	break;

      case PGSe_ECItoORB:

	/* transform body rates in the inertial reference frame (ECI) to the
	   equivalent rates expressed in the orbital frame (ORB) */

	bodyRateORB[0] = bodyRateECI[0] - result[0];
	bodyRateORB[1] = bodyRateECI[1] - result[1];
	bodyRateORB[2] = bodyRateECI[2] - result[2];
	break;

      default:
	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
			      "Unexpected value of the input 'direction' "
			      "switch.  Value must be either PGSe_ORBtoECI "
			      "or PGSe_ECItoORB.",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    return PGS_S_SUCCESS;
}
