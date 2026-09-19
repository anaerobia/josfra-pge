/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_SubSatPointVel.c

DESCRIPTION:
   This file contains the function PGS_CSC_SubSatPointVel(). 
   This tool finds the North and East components of the velocity
   of the subsatellite point and the rate of change of spacecraft
   altitude. 

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation
   Deborah A. Foch      / Applied Research Corporation
   Guru Tej S. Khalsa   / Applied Research Corporation

HISTORY:
   22-Mar-1994  PDN   Designed			
   23-Mar-1994  PDN   Coded
   30-Sep-1994  DAF   Revised to call function PGS_CSC_ECRtoGEO()
   30-May-1996  GTSK  Total rewrite, new calling sequence, no longer calculates
                      sub-satellite point, just sub-sat point velocity
   11-Jul-1995  PDN   Fixed prolog, examples, description, list of I/O
  
END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG: 
 
TITLE:
   Get Sub Satellite Point Velocity

NAME:
   PGS_CSC_SubSatPointVel() 

SYNOPSIS:
C:
   #include <PGS_CSC.h>

   PGSt_SMF_status
   PGS_CSC_SubSatPointVel(
      PGSt_integer      numValues,
      PGSt_double       posVelECR[][6],
      char              *earthEllipsTag,
      PGSt_double       longitude[],
      PGSt_double	latitude[],
      PGSt_double       altitude[],
      PGSt_double       velSub[][3])
     
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_CSC_4.f'
   
      integer function pgs_csc_subsatpointvel(numvalues,posvelecr,
     >                                        earthellipstag,latitude,
     >                                        longitude,altitude,velsub)
      integer            numvalues
      double precision   posvelecr(6,*)
      character*(*)      earthellipstag
      double precision   longitude(*)
      double precision   latitude(*)
      double precision   altitude(*)
      integer            velsub(3,*)

DESCRIPTION:
   This tool finds the North and East components of the velocity
   of the subsatellite point and the rate of change of spacecraft
   altitude. 

INPUTS:
   Name             Description                     Units       Min        Max
   ----             -----------                     -----       ---        ---
   numValues        Number of vectors to process    N/A          1         ANY

   posVelECR[][6]   Geocentric position/            m           ANY        ANY
                    velocity state vector in ECR                    

   earthEllipsTag   tag selecting Earth ellipsoid   N/A         N/A        N/A
 		    model (default is WGS84)	

   longitude[]      subsatellite point              radians     -pi        pi
                    longitude

   latitude[]       subsatellite point              radians    -pi/2       pi/2
                    geodetic latitude                

   altitude[]       spacecraft altitude             m           ANY        ANY
 
OUTPUTS:
   Name             Description                     Units       Min        Max
   ----             -----------                     -----       ---        ---
   velSub[][0]      North component of the          m/s        -7000       7000
                    subsatellite point velocity 
                    on the ellipsoid                

   velSub[][1]      East component of the           m/s        -7000       7000
                    subsatellite point velocity 
                    on the ellipsoid                

   velSub[][2]      rate of change of               m/s        -200        200
                    spacecraft altitude relative
                    to nadir on the ellipsoid                        

  
RETURNS:
   PGS_S_SUCCESS		   successful return
   PGSCSC_W_TOO_MANY_ITERS         normal iteration count exceeded in
                                   function PGS_CSC_ECRtoGEO() - could
                                   indicate inconsistent units for Spacecraft
                                   and Earth data, or corrupted Earth Axis
				   values
   PGSCSC_W_INVALID_ALTITUDE       spacecraft altitude is below the surface
                                   of the Earth - probably indicates bad
                                   input data
   PGSCSC_W_PROLATE_BODY           using a prolate earth model
   PGSCSC_W_SPHERE_BODY            using a spherical earth model
   PGSCSC_W_LARGE_FLATTENING       issued if flattening factor is greater
                                   than 0.01  
   PGSCSC_E_NEG_OR_ZERO_RAD        the equatorial or polar radius is 
                                   negative or zero
   PGSCSC_W_DEFAULT_EARTH_MODEL    default earth model was used
   PGSCSC_W_DATA_FILE_MISSING      data file is missing
   PGSCSC_W_ZERO_JACOBIAN_DET      Jacobian determinant is close to zero
   PGS_E_TOOLKIT                   something unexpected happened, 
                                   - execution aborted

  
     
EXAMPLES:
C:
   PGSt_SMF_status    returnStatus;
   PGSt_integer       numValues=1;
   PGSt_double        posVelECR[6];
   char               earthEllipsTag[20],
   PGSt_double        longitude;
   PGSt_double        latitude;
   PGSt_double        altitude;
   PGSt_double        velSub[3];
   
   posVelECR[0] = 7000000.0;
   posVelECR[1] = 0.0;
   posVelECR[2] = 0.0;
   posVelECR[3] = 7000.0;
   posVelECR[4] = 1000.0;
   posVelECR[5] = 1000.0;

   strcpy(earthEllipsTag,"WGS84");

   returnStatus = PGS_CSC_SubSatPointVel(numValues,posVelECR,earthEllipseTag,
                                         &longitude,&latitude,&altitude,velSub);
                                                                        
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, take appropriate action **
   }
   printf("input state vector: %f %f %f %f %f %f",posVelECR[0],
          posVelECR[1],posVelECR[2],posVelECR[3],posVelECR[4],posVelECR[5]);
 
   printf("Longitude:%f Latitude:%f  Altitude:%f",
           longitude, latitude, altitude);
   printf("Velocity of subsatellite point (North,East): %f, %f m/s\n", 
           velSub[0], velSub[1]);
   printf("Rate of change of altitude %f\n," velSub[2]); 
	       
FORTRAN:
      implicit none
      integer            pgs_csc_subsatpointvel
      integer            numvalues
      double precision   posvelecr(6)
      character*20       earthellipstag
      double precision   longitude
      double precision   latitude  
      double precision   altitude
      double precision   velsub(3)
      integer            returnstatus
 
      data posvelecr/7000000,0,0,7000,1000,1000/   
      data earthellipstag/'WGS84'/

      numvalues = 1

      returnstatus = pgs_csc_subsatpointvel(numvalues,posvelecr,
     >                                      earthellipsetag,pgsd_true,
     >                                      longitude,latitude,
     >                                      altitude,velsub)

      if(returnstatus .ne. pgs_s_success) go to 90
      write(6,*) posvelecr,earthellipstag,velflag
      write(6,*) longitude,latitude,altitude,velsub(1),velsub(2),
     >           velsub(3)

 90   write(6,99)returnstatus
 99   format('returnstatus: ',I10)  


 
NOTES:
   The tool is called by PGS_CSC_SubSatPoint() which provides the input
   spacecraft ephemeris, latitude and longitude, and manages the ECI to 
   ECR coordinate transformation.

   If an invalid earthEllipsTag is input, the program will use the WGS84 Earth
   model by default. The earthEllipsTag MUST be the same as used to obtain
   the latitude, or the results will be meaningless.
 
   This function is called only if the velocity flag velFlag was set to PGS_TRUE
   in PGS_CSC_SubSatPoint().
   
   The horizontal velocity calculated here is that of a mathematical point on
   the Earth at (nominal) spacecraft nadir, and not that of any material object.
   It is orthogonal to nadir, so is suitable as a descriptor of ground track but
   not for Doppler work.  In general, its magnitude will be less than that of
   the projection of the spacecraft velocity on the ellipsoid, because a point 
   on the  ellipsoid is closer to Earth center than the spacecraft.

   The third (vertical) component of velocity is useful for Doppler work at
   nadir, but Doppler velocity along ANY look vector (not just nadir) is
   provided in the lookpoint algorithm in the function PGS_CSC_GetFOVPixel().
   
REQUIREMENTS:
   PGSTK - 1060, 0930

DETAILS:
   This function returns a warning message PGSCSC_W_ZERO_JACOBIAN_DET and
   default velocities of 0.0 m/s when it encounters a singularity at the North
   or South Pole.  The singularity will occur only if the spacecraft passes
   directly over the pole, which is not contemplated for the present and next
   generation of EOS spacecraft.  There is no "workaround" for the singularity,
   because the North and East components of the velocity are undefined at the
   poles.

   The quantity called "den" in this code is the quantity 

      sqrt(1 - eccentricity*eccentricity*sin(latitude)*sin(latitude)

   and is related to the radius of curvature in a vertical plane orthogonal
   to the meridian.

   
GLOBALS:
   None

FILES:
   earthfigure.dat 

FUNCTIONS CALLED:
   PGS_CSC_GetEarthFigure()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_SubSatPointVel()"

PGSt_SMF_status
PGS_CSC_SubSatPointVel(            /* finds velocity of the subsatellite
				      point and rate of altitude change */
    PGSt_integer   numValues,      /* number of vectors to process */
    PGSt_double    posVelECR[][6], /* input position and velocity state vector
				      in ECR Coords. */
    char           *earthEllipsTag,/* Earth model tag -- eg. WGS84, MERIT,
				      new_intl */
    PGSt_double    longitude[],    /* longitude of the subsatellite point */
    PGSt_double    latitude[],     /* geodetic latitude of the subsatellite 
				      point */
    PGSt_double    altitude[],     /* spacecraft geodetic altitude */
    PGSt_double    velSub[][3])    /* value of velocity components (north, 
				      east, of the subsatellite and rate of
				      change of altitude */
{
    PGSt_integer    cnt;            /* loop counter */
    
    PGSt_double     equatRad_A;     /* Earth semi-major axis (m) */
    PGSt_double     polarRad_C;     /* Earth semi-minor axis (m) */
    PGSt_double     ecc2;           /* Earth ellipsoid eccentricity squared */
    PGSt_double     ecc2c;          /* 1.0 - ecc2 */
    PGSt_double     den;            /* denominator - See "DETAILS" above */
    PGSt_double     den2;           /* denominator squared */
    PGSt_double     sinlat;         /* sine of the geodetic latitude */
    PGSt_double     coslat;         /* cosine of the geodetic latitude */
    PGSt_double     sinlon;         /* sine of the longitude */
    PGSt_double     coslon;         /* cosine of the longitude */
    PGSt_double     determ;         /* determinant of the Jacobian from (x,y,z)
				       to (theta, phi, h) (latitudinal)
				       coordinates */
    PGSt_double     ideterm;        /* 1.0/determ */
    PGSt_double     xi;             /* radius of curvature in the prime
				       vertical = 1/den */
    PGSt_double     axih;           /* factor equatRad_A*xi + altitude */
    PGSt_double     ds_dlat;        /* arc length per radian along spheroid in
				       itude direction */
    PGSt_double     ds_dlon;        /* arc length per radian along spheroid in
				       longitude direction */
    PGSt_double     axi3hec;        /* factor equatRad_A*xi^3*(1-ecc^2) + 
				       altitude */
    PGSt_double     dlat_dx;        /* partial of geodetic latitude on 
				       x coordinate */
    PGSt_double     dlat_dy;        /* partial of geodetic latitude on
				       y coordinate */
    PGSt_double     dlat_dz;        /* partial of geodetic latitude on
				       z coordinate */
    PGSt_double     dlon_dx;        /* partial of longitude on x coordinate */
    PGSt_double     dlon_dy;        /* partial of longitude on y coordinate */
    PGSt_double     dh_dx;          /* partial of altitude on x coordinate */
    PGSt_double     dh_dy;          /* partial of altitude on y coordinate */
    PGSt_double     dh_dz;          /* partial of altitude on z coordinate */
    PGSt_double     dlat_dt;        /* rate of change of geodetic latitude */
    PGSt_double     dlon_dt;        /* rate of change of longitude */
		    
    PGSt_SMF_status returnStatus;   /* return value of this function */
    
    /* Initialize return value to success */
   
    returnStatus = PGS_S_SUCCESS; 
  
	
    if (numValues < 1)
    {
        /* Must have at LEAST one vector to process */

        returnStatus = PGSCSC_E_BAD_ARRAY_SIZE;
        PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
        return returnStatus;
    }

    /* Compute the velocity of the subsatellite point.  See document by
       "Theoretical Basis of the SDP Toolkit Geolocation Package", Document
        445-TP-002-002, May 1995 by P. Noerdlinger for more information on 
        the algorithm. */
    
    /* Get equatorial and polar radii of the Earth */
    
    returnStatus = PGS_CSC_GetEarthFigure(earthEllipsTag,&equatRad_A,
					  &polarRad_C);
    
    switch(returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSCSC_W_DEFAULT_EARTH_MODEL:
	break;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* issue an error if the equatorial radius is negative */
    
    if (equatRad_A <= 0.0)
    {
	returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "the equatorial radius is negative or zero",
			      FUNCTION_NAME);
	return returnStatus;
    }
    
    /* issue an error if the polar radius is negative */
    
    if (polarRad_C <= 0.0)
    {
	returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
	PGS_SMF_SetDynamicMsg(returnStatus,"the polar radius is negative"
			      "or zero",FUNCTION_NAME);
	return returnStatus;
    }
    
    /* check for errors in the value of the flattening factor */
    
    if (equatRad_A < polarRad_C)
    {
	returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
	PGS_SMF_SetDynamicMsg(returnStatus,"the specified earth model "
			      "defines a prolate earth",FUNCTION_NAME);
	return returnStatus;
    } 
    else if (equatRad_A == polarRad_C)
    {
	returnStatus = PGSCSC_W_SPHERE_BODY;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "a spherical earth model is being used",
			      FUNCTION_NAME);
    }
    else if ((polarRad_C/equatRad_A) <  0.9)
    {
	returnStatus = PGSCSC_W_LARGE_FLATTENING;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }
    
    /* Compute the square of the ellipsoid eccentricity */
    
    ecc2 = 1.0 - polarRad_C * polarRad_C/(equatRad_A * equatRad_A);
    ecc2c   = 1.0 - ecc2;
    
    for (cnt=0;cnt<numValues;cnt++)
    {
	/* Setup of factors in Eqs. (4 - 28) of reference document (ecc2c is set
	   above) */
	
	sinlat = sin(latitude[cnt]);
	coslat = cos(latitude[cnt]);
	den2 = 1.0 - ecc2 * sinlat * sinlat;
	den = sqrt(den2);
	xi      = 1.0/den;
	
	axih    = equatRad_A*xi + altitude[cnt];
	ds_dlat = equatRad_A*xi*xi*xi*ecc2c;
	ds_dlon = equatRad_A*coslat*xi;
	axi3hec = ds_dlat + altitude[cnt];
	coslon = cos(longitude[cnt]);
	sinlon = sin(longitude[cnt]);
	
	/* Get determinant of the Jacobian */
	
	determ = -coslat*(equatRad_A*(equatRad_A*xi*xi*xi*xi*ecc2c + 
				      altitude[cnt]*xi*(1.0 + xi*xi*ecc2c)) +
			  altitude[cnt]*altitude[cnt]);
    
	/* Invert if possible; else signal error (may be at singular point).  A
	   'typical value' for the Jacobian determinant will be of the order of
	   the square of the Earth radius, ~4.0e13 meters squared; thus a value
	   less than 0.1 must be regarded as suspicious on typical platforms of
	   accuracy 15 significant digits */
	
	if(fabs(determ) >= 0.1)
	    ideterm = 1.0/determ;
	else
	{
	    velSub[cnt][0] = PGSd_GEO_ERROR_VALUE;
	    velSub[cnt][1] = PGSd_GEO_ERROR_VALUE;
	    velSub[cnt][2] = PGSd_GEO_ERROR_VALUE;
	    if (returnStatus != PGSCSC_W_ZERO_JACOBIAN_DET)
	    {
		returnStatus = PGSCSC_W_ZERO_JACOBIAN_DET;
		PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	    }
	    continue;
	}
	
	/* Get basic partials of latitude, longitude with respect to 
	   x,y,z from inverting Jacobian */
	
	dlat_dx =  ideterm * axih * coslat * sinlat * coslon;
	dlat_dy =  ideterm * axih * coslat * sinlat * sinlon;
	dlat_dz = -ideterm * axih * coslat * coslat;
	dlon_dx =  ideterm * axi3hec * sinlon;
	dlon_dy = -ideterm * axi3hec * coslon;
	dh_dx  =  dlat_dz  * axi3hec * coslon;
	dh_dy  =  dlat_dz  * axi3hec * sinlon;
	dh_dz  = -ideterm * axi3hec * axih * coslat * sinlat;
	
	/* Get the total derivative of latitude with respect to time using the
	   chain rule */
	
	dlat_dt = dlat_dx * posVelECR[cnt][3] + dlat_dy * posVelECR[cnt][4] + 
	          dlat_dz * posVelECR[cnt][5];
	
	/* Get the total derivative of longitude with respect to time using the
	   chain rule */
	
	/*  No term from zdot */
	
	dlon_dt = dlon_dx * posVelECR[cnt][3] + dlon_dy * posVelECR[cnt][4];  
	
	/* Get the total derivative of altitude with respect to time using the
	   chain rule */
	
	velSub[cnt][2] = dh_dx * posVelECR[cnt][3] + dh_dy * posVelECR[cnt][4] +
	                 dh_dz * posVelECR[cnt][5];
	
	/* Compute the North and East components of the subsatellite point
	   velocity*/
	
	velSub[cnt][0] = ds_dlat * dlat_dt;
	velSub[cnt][1] = ds_dlon * dlon_dt;
    }
    
    /*  Return to calling function */
    
    return returnStatus; 
}
