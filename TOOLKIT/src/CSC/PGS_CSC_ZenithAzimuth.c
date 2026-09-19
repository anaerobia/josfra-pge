/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_ZenithAzimuth.c

DESCRIPTION:
   This file contains the tool PGS_CSC_ZenithAzimuth, which computes the 
   zenith and the azimuth of a vector defined in ECR at the look point.

AUTHOR:
   Peter D. Noerdlinger  / Applied Research Corporation
   Snehavadan Macwan     / Applied Research Corporation

HISTORY:
   02-FEB-1994  PDN    Designed
   03-MAR-1994  SM     created initial version
   27-MAR-1994  PDN    corrected units, logic and flag usage 
   28-AUG 1994  PDN    added parallax correction for Moon, sign reversal for
                       case of look vector as input 
   12-SEP 1994  PDN    added FORTRAN example; fixed line width
   14-SEP-1994  PDN    reworked logic to streamline cases where user does not
                       want azimuth or refraction added better warning reporting
   28-MAY-1996  PDN    added a check for out of range latitude input; 
                        fixed a comment
   21-OCT-1996  PDN    Fixed altitude test to work only with refraction "on"

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:
   Get Zenith and Azimuth of an ECR Vector at the look point

NAME:
   PGS_CSC_ZenithAzimuth()
 
SYNOPSIS:
 C:
    #include <PGS_CSC.h>

    PGSt_SMF_status
    PGS_CSC_ZenithAzimuth( 
        PGSt_double 	vectorECR[3],	
	PGSt_double 	latitude,
	PGSt_double 	longitude,
	PGSt_double 	altitude,	
	PGSt_tag    	vectorTag, 
	PGSt_boolean	zenithOnlyFlag,
	PGSt_boolean	refractFlag, 
	PGSt_double 	*zenith,
	PGSt_double	*azimuth,
	PGSt_double	*refraction) 

 FORTRAN:
      include <PGS_CSC.f>
      include <PGS_CSC_4.f>
      include <PGS_SMF.f>
      integer function pgs_csc_zenithazimuth(vectorecr,latitude,longitude,
     >                                       altitude,vectortag,
     >                                       zenithonlyflag,refractflag,
     >                                       zenith,azimuth,refraction) 
      double precision 	vectorecr[3]
      double precision 	latitude
      double precision 	longitude
      double precision 	altitude
      integer     	vectortag 
      integer           zenithonlyflag
      integer           refractflag 
      double precision  zenith
      double precision 	azimuth
      double precision  refraction

DESCRIPTION:
   Computes the zenith and the azimuth of a vector at the look point.  Allows
   for rerefraction if desired.
 
INPUTS:
   NAME            DESCRIPTION                   UNITS        MIN     MAX
   ----            -----------                   -----        ---     ---
   vectorECR       ECR vector whose              meters or
                   zenith & azimuth              unit vector  n/a     n/a
		   are desired
		   (in case of MOON do not use 
		   a unit vector! - see NOTES)

  latitude	   geodetic latitude	         radian      -pi/2    pi/2

  longitude        longitude	                 radian      -2*pi    2*pi

  vectorTag        CB, Moon, or Look             n/a          n/a     n/a
                   or a CB identifier   
                   (see NOTES)

  zenithOnlyFlag   omit azimuth calculation      n/a          n/a     n/a 

  refracFlag       turns on refraction           n/a          n/a     n/a 

  altitude         altitude                      meters      -2000   80000
                   (altitude is an input only 
		   when refracFlag is PGS_TRUE     
		   see NOTES).


OUTPUTS:
   NAME            DESCRIPTION                   UNITS        MIN     MAX
   ----            -----------                   -----        ---     ---
   zenith	   zenith angle                  radian        0      pi

   azimuth         azimuth E from N              radian       -pi     pi

   refraction      increase of zenith            radian        0      0.1
                   angle due to 
                   refraction
 
RETURNS:
   PGS_S_SUCCESS                successful execution
   PGSCSC_W_BELOW_HORIZON       warning indicating the object is below
                                horizon
   PGSCSC_W_UNDEFINED_AZIMUTH   the object is at the zenith.  In this case
                                azimuth is not calculated 
   PGSCSC_W_NO_REFRACTION       no refraction calculation done due to errors
   PGSCSC_E_INVALID_VECTAG      the input vector tag is not PGSd_CB, PGSd_MOON,
                                PGSd_LOOK or a celestial body identifier
   PGSCSC_E_LOOK_PT_ALTIT_RANGE look point altitude not reasonable
   PGSCSC_E_BAD_LAT             bad value for latitude
   PGSCSC_E_ZERO_INPUT_VECTOR   the input vector has zero length 
   PGS_E_TOOLKIT                unknown error occurred 

EXAMPLES:

 C:

    PGSt_SMF_status  	returnStatus;
    PGSt_double 	vectorECR[3];
    PGSt_double 	latitude  = 0.2;
    PGSt_double 	longitude = 0.1;
    PGSt_double 	altitude = 0.0;
    PGSt_tag    	vectorTag = PGSd_LOOK;
    PGSt_boolean	zenithOnlyFlag = PGS_FALSE;
    PGSt_boolean	refractFlag = PGS_FALSE;
    PGSt_double 	zenith;
    PGSt_double		azimuth;
    PGSt_double		refraction; 

    vectorECR[2] = 0.2;
    vectorECR[1] = 0.0;
    vectorECR[0] = -0.4;

    returnStatus = PGS_CSC_ZenithAzimuth(vectorECR,latitude,longitude,altitude,	
                                         vectorTag,zenithOnlyFlag,refractFlag, 
					 &zenith,&azimuth,&refraction) 

    ** do some error handling **
    ** if desired, convert zenith and azimuth to degrees **

    printf("zenith angle = %f,  azimuth = %f\n", zenith, azimuth);


 FORTRAN:
      implicit none
      integer           pgs_csc_zenithazimuth

      integer           zenithonlyflag
      integer           refractflag 
      integer     	vectortag
      integer           returnstatus 

      double precision 	look(3)
      double precision 	latitude
      double precision 	longitude
      double precision 	altitude
      double precision  zenith
      double precision 	azimuth
      double precision  refraction

      
      vectortag = pgsd_look
      latitude  = 0.2D0
      longitude = 0.1D0
      altitude  = 0.0D0
      zenithonlyflag = pgs_false
      refractflag    = pgs_false

      look(3) = 0.2;
      look(2) = 0.0;
      look(1) = -0.4;
 

      returnstatus = pgs_csc_zenithazimuth(look,latitude,longitude,
     >                                     altitude,vectortag,
     >                                     zenithonlyflag,refractflag,
     >                                     zenith,azimuth,refraction) 

 
!  do some error handling
!  if desired, convert zenith and azimuth to degrees

NOTES:
   The vectorECR vector must be in ECR coordinates.  If this function is used to
   obtain the zenith and azimuth of the look vector, the vectorTag must be set
   to PGSd_LOOK (this allows for the reversed sense of such a vector, which
   represents a line of sight above the horizon when pointing down!)  If the
   zenith and azimuth of a distant celestial body (such as the Sun or a planet)
   are desired, the user may supply PGSd_CB or any of the identifiers: PGSd_SUN,
   PGSd_MERCURY, PGSd_VENUS, PGSd_MARS, PGSd_JUPITER, PGSd_SATURN, PGSd_URANUS,
   PGSd_NEPTUNE, or PGSd_PLUTO.  This is purely a convenience for users doing
   other calculations with a CB identifier; the action of the function is in all
   cases the same - it finds the zenith and azimuth of the vector at the look
   point, without regard to parallax (i.e., the vector from Earth center to the
   Celestial body is regarded as unchanged due to the displacement of the look
   point from Earth center).

   In the case of the MOON, the geocentric parallax is appreciable,
   meaning that its apparent position is, in general, different as viewed 
   from Earth center or from the look point.  The difference can be 
   as large as a degree.  Therefore, in this case, a parallax correction 
   is made.  It is essential, in this case, of course, that the MOON 
   vector be supplied in meters.  In this case, the input vector should 
   be the Earth to MOON vector defined from Earth center (geocentric), 
   as obtained, for example, from the PGS_CBP_Earth_CB_Vector()  tool.  

   In all other cases, the input vector can be in any units, including
   normalized (unit vector).

   Users wishing to take into account the minuscule parallax correction
   for the Sun, or the correction for some other chosen body such as
   an asteroid, could simply label the vector as PGSd_MOON.  (For the Sun,
   the correction is only ~ 2.5 millidegrees.)

   Refraction by the atmosphere is calculated if the flag is set
   to PGS_TRUE.  This calculation approximately corrects, in the
   visual band, for the fact that any line of sight, such as the
   Sun, Moon, or look vector is bent by the atmosphere.  It should
   not be used for altitudes over 8 km (where the correction is
   entirely negligible).  

   If the vector is well below the horizon, a warning is returned and 
   no azimuth calculation is done. The present algorithm is fairly 
   forgiving for points slightly below the horizon (to 96 degrees), in 
   order that the user interested in the location of the glow before
   sunrise or after sunset can find its azimuth; it is user re-
   sponsibility to take special action between 90 degrees and 96
   degrees if these data are not wanted. 

   The altitude is required only if  refraction is to be calculated, 
   and its only effect is to change the mean density of the
   atmosphere in the refraction function.

   If the zenith only flag is defined by the user to be PGS_TRUE the function
   will run faster but will not calculate the azimuth.

   If desired, the unit look vector can be obtained and saved from 
   PGS_CSC_GetFOV_pixel(); this will achieve very good performance as
   the ECR look vector is calculated there.

   If the azimuth is requested but the zenith angle is < 0.026 deg, it is
   deemed that the azimuth calculation is unreliable, because variations
   in the local vertical as determined from the geoid, and variable
   refraction in the atmosphere dominate at that level.

   The calculation herein is entirely independent of the Earth model
   except for the parallax correction, where WGS84 is assumed, and any
   difference in other models introduces negligible error.  The use of
   geodetic latitude as input guarantees that the rest of the algorithm
   is independent of Earth model.

REQUIREMENTS:
   PGSTK-0860, 1091

DETAILS:
   See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
   ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
   for more information on the algorithm.

GLOBALS:
   None

FILES:
   None

FUNCTIONS CALLED:
   PGS_CSC_SpaceRefract()
   PGS_SMF_SetStaticMsg()

END_PROLOG
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

#define   EQUAT_RAD_A   6378137.0       /* Earth equatorial radius (WGS84) */
#define   POLAR_RAD_C   6356752.314245   /* Earth polar radius (WGS84) */

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_ZenithAzimuth()"
#define HALFPI          1.57079632679489662 /* half pi */

PGSt_SMF_status
PGS_CSC_ZenithAzimuth(            /* computes the zenith and the azimuth of a 
				     vector at the look point */
    PGSt_double  vectorECR[3],    /* ECR input vector */
    PGSt_double  latitude,        /* geodetic latitude at the look point */
    PGSt_double  longitude,	  /* longitude at the look point */
    PGSt_double  altitude,        /* altitude at the look point (advise using
				     0.0 if unknown)*/
    PGSt_tag     vectorTag,       /* CB for distant celestial body, PGSd_MOON
				     for Moon, or PGSd_LOOK for LOOK Vector  */
    PGSt_boolean zenithOnlyFlag,  /* zenith only flag to bypass azimuth
				     calculation */ 
    PGSt_boolean refractFlag,     /* flag to calculate refraction */
    PGSt_double  *zenith,	  /* zenith angle of the input vector at the
				     look point */
    PGSt_double	 *azimuth,        /* azimuth of the input vector at the look
				     point */
    PGSt_double	 *refraction)     /* Calculated refraction at the look point (to
				     be implemented) */
{
    PGSt_integer iXYZ;            /* index of coordinate in "for" loop */
    
    PGSt_double	 norm;		  /* norm of initial Input Vector */
    PGSt_double	 normVec[3];	  /* normalized Input Vector */
    PGSt_double	 scrVec[3];	  /* scratch vector */
    PGSt_double  xyzECR[3];       /* rectangular ECR coordinates of look point
				     in case vectorTag == PGSd_MOON  */
    PGSt_double	 slat;		  /* sin(latitude) */
    PGSt_double	 slon;		  /* sin(longitude) */
    PGSt_double	 clat;		  /* cos(latitude) */
    PGSt_double	 clon;		  /* cos(longitude) */
    PGSt_double  localVert[3];	  /* Normal to ellipsoid at look point */
    PGSt_double  dotProd;	  /* dot product of the normal at lookpoint
				     and normalized Input Vector */
    PGSt_double  projVec[3];	  /* projection of normVec on the ellipsoid */
    PGSt_double  distOfPoint;     /* distance of point from Earth's axis */
    PGSt_double  recip;           /* reciprocal of the curvature of the
				     ellipsoid in a vertical plane orthogonal to
				     the meridian */
    PGSt_double  scratchZenith;   /* temporary zenith angle in case of
				     refraction */
    PGSt_double  scratchDispl;    /* scratch storage for displacement returned
				     by PGS_CSC_SpaceRefract() */
    static  PGSt_double flatFac;   /* flattening factor - which is related to
				     eccentricity of the ellipsoid of
				     revolution */
    PGSt_SMF_status returnStatus; /* error status of function call */
    PGSt_SMF_status code;         /* status code returnedby PGS_SMF_GetMsg() */

    char            mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							    PGS_SMF_GetMsg()*/
    char            msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
							    PGS_SMF_GetMsg() */

    /* default status indicating success */

    returnStatus = PGS_S_SUCCESS;

    /*** First test altitude in case of garbage input or user's misunderstanding
         and entering Spacecraft altitude in place of that at the Look Point.  
         We allow wide limits for those working at the top of the stratosphere 
         all the way below the Dead Sea  ***/

    if ( (refractFlag == PGS_TRUE) && 
         ((altitude > 80000.0) || (altitude < -2000.0)) )
    {
	returnStatus = PGSCSC_E_LOOK_PT_ALTIT_RANGE;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	return returnStatus;
    }
    
   /* reject bad latitude data */
        if ((latitude < -HALFPI) || (latitude > HALFPI))
        {
           returnStatus =  PGSCSC_E_BAD_LAT;
 
           /* set error message */
           PGS_SMF_SetDynamicMsg(returnStatus,"out of range latitude input",
                                FUNCTION_NAME);
           return returnStatus;
        }

    /*** If the vectorTag is Moon, correct for parallax  ***/
    
    switch (vectorTag)   
    {
      case PGSd_CB:  /* any very distant celestial body - could be a star */
      case PGSd_SUN:
      case PGSd_MERCURY:
      case PGSd_VENUS:
      case PGSd_MARS:
      case PGSd_JUPITER:
      case PGSd_SATURN:
      case PGSd_URANUS:
      case PGSd_NEPTUNE:
      case PGSd_PLUTO:
	for (iXYZ=0; iXYZ<3; iXYZ++)
	{ 
	    scrVec[iXYZ] = vectorECR[iXYZ];
	}
	break;
      case PGSd_MOON: 
	/*  determine the flattening factor */
	
	flatFac = (EQUAT_RAD_A - POLAR_RAD_C) / EQUAT_RAD_A;
	
	/* compute distance of point from Earth's axis */
	
	recip = 1.0 / sqrt(1.0 - (2.0*flatFac - flatFac*flatFac) 
			   * sin(latitude) * sin(latitude));
	distOfPoint = ((EQUAT_RAD_A * recip) + altitude) * cos(latitude);
	
	/* calculate the ECR coordinates of the look point */
	
	xyzECR[0] = distOfPoint * cos(longitude);
	xyzECR[1] = distOfPoint * sin(longitude);
	xyzECR[2] = ((EQUAT_RAD_A * (1.0-flatFac) * (1.0-flatFac) * recip) 
		     + altitude) * sin(latitude);
	for (iXYZ=0; iXYZ<3; iXYZ++)
	{
	    scrVec[iXYZ] = vectorECR[iXYZ] - xyzECR[iXYZ];
	}
	break;
      case PGSd_LOOK:
	for (iXYZ=0; iXYZ<3; iXYZ++)
	{ 
	    scrVec[iXYZ] = - vectorECR[iXYZ];
	}
	break;
      default:
	returnStatus = PGSCSC_E_INVALID_VECTAG ;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	return returnStatus;
    }
    
    /*** First compute the norm of the input vector vectorECR and then compute
         the normalized Input Vector ***/

    norm = sqrt(scrVec[0]*scrVec[0] + scrVec[1]*scrVec[1] + 
		scrVec[2]*scrVec[2]);

    if (fabs(norm) < EPS_64)
    {
       returnStatus = PGSCSC_E_ZERO_INPUT_VECTOR;  
       PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
       return returnStatus;
    }

    normVec[0] = scrVec[0]/norm;
    normVec[1] = scrVec[1]/norm;
    normVec[2] = scrVec[2]/norm;

    /*** Define slat, slon, clat, clon ***/

    slat = sin(latitude);
    slon = sin(longitude);
    clat = cos(latitude);
    clon = cos(longitude);
    
    /*** Compute the local vertical vector  ***/

    localVert[0] = clat * clon;
    localVert[1] = clat * slon;
    localVert[2] = slat;
    
    /*** Compute the zenith angle of the vectorECR vector at the look point by
         first performing dot product of the norm of the look point and the
         normalized Input Vector. Then arc-cosine of the dot product gives
         required zenith angle of the vectorECR vector. ***/

    dotProd = localVert[0] * normVec[0] + localVert[1] * normVec[1] + 
		  localVert[2] * normVec[2];

    /*** If vectorECR is at zenith (dotProd > 0.9999999), zenith = 0.0 and the
         azimuth will be indeterminate. In this case set zenith = 0.0, 
	 azimuth = 0.0 (if requested), and return. ***/

    if (dotProd > 0.9999999)   /*  0.026 degree - object is at zenith */
    {
	*zenith = 0.0;
        if(zenithOnlyFlag != PGS_TRUE)
        {
	    returnStatus = PGSCSC_W_UNDEFINED_AZIMUTH;
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	    *azimuth = 0.0;
        }
        return	returnStatus;
    }
    else
        *zenith = acos(dotProd);
    
    /*** If the zenith > 96.0 degree (1.6755 radian), the vectorECR is below the
         horizon and the azimuth is meaningless. In this case return a warning
         message, set azimuth = 0.0 and return.  This limit is approximate,
         allowing user to get meaningful values in twilight. ***/

    if (*zenith > 1.6755)
    {
	returnStatus = PGSCSC_W_BELOW_HORIZON;
        PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
        if(zenithOnlyFlag != PGS_TRUE)
        {
	    *azimuth = 0.0;
        }
	return returnStatus;
    }
    
    /*** refraction calculation if requested ***/

    if (refractFlag == PGS_TRUE)
    {
	returnStatus = PGS_CSC_SpaceRefract(*zenith,altitude,latitude,
					    &scratchZenith,&scratchDispl);
	
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	    *refraction = *zenith - scratchZenith; /* refracted value (scratch)
						      is less! */
	    *zenith = scratchZenith;               /* report refracted value */
	    break;
	  case PGSCSC_W_INVALID_ALTITUDE:
	  case PGSCSC_W_BELOW_HORIZON:
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus)
		PGS_SMF_GetMsgByCode(returnStatus,msg);
	    returnStatus = PGSCSC_W_NO_REFRACTION;
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);	
	    *refraction = 0.0;
	    break;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	    returnStatus = PGS_E_TOOLKIT;
	    return returnStatus;
	}
	
    }  /* end if(refractFlag == PGS_TRUE) */

    /* if azimuth is not desired - return */

    if (zenithOnlyFlag == PGS_TRUE)
    {
	if(returnStatus == PGS_S_SUCCESS)
	{
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME); 
	}
	return returnStatus;
    }

    /*** Now compute Azimuth of the vectorECR. If flag for the zenith only
         indicator is PGS_TRUE , the azimuth is omitted. Skip rest of
         the computation of the azimuth and return.  Else compute 
         azimuth of the vectorECR at the look point. ***/

    /*** Define projection of the normVec on the Ellipsoid ***/
	
    for (iXYZ=0; iXYZ<3; iXYZ++)
	projVec[iXYZ] = normVec[iXYZ] - localVert[iXYZ] * dotProd;
	
    /*** Compute the azimuth of the vectorECR at the look point ***/

    *azimuth = atan2(-projVec[0]*slon + projVec[1]*clon, -slat*(projVec[0]*clon 
                     + projVec[1]*slon) + projVec[2]*clat);

    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME); 
    }
    return returnStatus;
}
