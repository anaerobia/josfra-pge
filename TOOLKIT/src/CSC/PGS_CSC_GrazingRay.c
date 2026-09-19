/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/****************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_GrazingRay.c

DESCRIPTION:
   This file contains the function PGS_CSC_GrazingRay(). 
   For rays that miss Earth limb, this function finds the nearest miss 
   point on the ray and corresponding surface point. For rays that strike 
   the Earth, it outputs instead the coordinates of the midpoint of the
   chord of the ray within the ellipsoid and surface coordinates of the 
   intersection nearest the observer.

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   22-Oct-1996  PDN  Designed			
   23-Oct-1996  PDN  Coded
   06-Mar-1997  PDN  Improved error reporting for case ray hits Earth
   07-Mar-1997  PDN  Altered to allow geolocation of representative
                     point on ray passing through the Earth, so that
                     user can model a ray that refracts around the
                     Earth
   11-Mar-1997  PDN  Altered to return the rectangular coordinates
                     of the surface point
   17-Mar-1997   PDN Fixed examples and details in PROLOG
                  
END_FILE_PROLOG:
****************************************************************************/
/****************************************************************************
BEGIN_PROLOG: 
 
TITLE:
   Find Point of Closest Miss and Surface Point

NAME:
   PGS_CSC_GrazingRay(). 

SYNOPSIS:
C:
   #include <PGS_CSC.h>
  
   PGSt_SMF_status
   PGS_CSC_GrazingRay(
       char              earthEllipsTag[20],
       PGSt_double       posECR[3], 
       PGSt_double       ray[3],
       PGSt_double	 *latitude,
       PGSt_double       *longitude,
       PGSt_double       *missAltitude,
       PGSt_double       *slantRange,
       PGSt_double       posNEAR[3],
       PGSt_double       posSURF[3])
     
FORTRAN:        
      include 'PGS_SMF.f'
      include 'PGS_CSC_4.f'
        integer function pgs_csc_grazingray(
        earthellipstag,pos,ray,latitude,longitude,
        missaltitude,slantrange,posnear,possurf)

      character*19            earthellipstag
      double precision        posecr(3)
      double precision        ray(3)
      double precision        latitude
      double precision        longitude
      double precision        missaltitude
      double precision        slantrange
      double precision        posnear(3)
      double precision        possurf(3)



DESCRIPTION:	For a line of sight ("ray") that misses Earth limb, this tool
   calculates the rectangular coordinates of the point Q of closest approach 
   to the Earth and the slant range to Q. It also obtains the latitude and 
   longitude of the surface point P nearest Q (and therefore nearest to the 
   ray) and the geodetic altitude of Q above P (Q and P have the same longitude
   and geodetic altitude). When the ray, instead, intersects the Earth 
   ellipsoid, the rectangular coordinates of Q are replaced by those of a point 
   halfway between the two "pierce points" where the ray intersects the 
   ellipsoid. The intent is to provide a point with the nearly the same latitude
   and longitude as the point closest to the Earth's surface on a ray from a 
   background object (such as the Sun) that is actually refracted round the 
   Earth. When the ray intersects the Earth, the latitude and longitude of P are 
   also replaced by those of the nearest pierce point, i.e. where the instrument
   is looking, and the slant range is replaced by the range to that point. 
   Furthermore, the return value PGSCSC_W_HIT_EARTH issues. If the ray, instead, 
   points away from the Earth ellipsoid, the altitude output variable is set to 
   PGSd_GEO_ERROR_VALUE and the return value to PGSCSC_W_LOOK_AWAY, in either 
   case - ray missing the ellipsoid or ray striking it.


   Get the rectangular coordinates of the surface point nearest the ray
   (for misses) or the visible surface point on the ray (for hits).
 
   If the ray points away from the Earth ellipsoid, missAltitude is set 
   to PGSd_GEO_ERROR_VALUE and PGSCSC_W_LOOK_AWAY  is returned.

INPUTS:
   Name             Description                  units       Min        Max
   ----             -----------                  ----       ---        ---

   earthEllipsTag   tag selecting Earth          N/A        N/A        N/A
 		    ellipsoidmodel (default 
		    is WGS84)	

   posECR[3]        ECR Spacecraft Position      meters     see NOTES

   ray[3]           unit vector in ECR           N/A
                    coordinates along the 
                    line of sight
 
OUTPUTS:
   Name             Description                  Units       Min        Max
   ----             -----------                  -----       ---        ---
   latitude         geodetic latitude of         radians    -pi/2       pi/2
                    posNEAR (see below)   
                    
   longitude        longitude of posNEAR         radians     -pi        pi
                    (see below)   

   slantRange       range to point nearest
                    Earth, when ray misses,      meters       0         n/a
                    or to closest ray inter-
                    section, when it hits

   missAltitude     missAltitudes                meters       0         n/a
                    
   posNEAR[3]       rectangular ECR coordinates  meters      n/a        n/a
                    of point on ray nearest   
                    Earth (misses) or sub-
                    terranean point halfway 
                    between the two pierce 
                    points (hits)

   posSURF[3]       rectangular ECR coordinates  meters      n/a       n/a
                    of the point on Earth closest
                    to the ray (when the ray misses
                    Earth limb) or where the ray 
                    strikes the Earth ellipsoid
                    

  
RETURNS:
   PGS_S_SUCCESS		   Successful return
   PGSCSC_W_SUBTERRANEAN           User provided a subterranean position
   PGSCSC_W_HIT_EARTH              Line of Sight struck the Ellipsoid
   PGSCSC_W_LOOK_AWAY              Line of sight points away from Earth
   PGSCSC_W_ERROR_IN_GRAZINGRAY    Generic surrogate for warning passed up from 
                                      lower level function
   PGSCSC_W_ZERO_PIXEL_VECTOR      Zero length ray vector was supplied
   PGSCSC_E_BAD_EARTH_MODEL        Bad earth model data (zero axis or prolate) 
   PGS_E_TOOLKIT                   Something unexpected happened, 
                                   - execution aborted

EXAMPLES:

C:
        char                    earthEllipsTag[20];
        PGSt_double             posECR[3];
        PGSt_double             ray[3];
        PGSt_double             latitude;
        PGSt_double             longitude;
        PGSt_double             missAltitude;
        PGSt_double             slantRange;
        PGSt_double             posNEAR[3];
        PGSt_double             posSURF[3];

        strcpy(earthEllipsTag,"GEM-10B");
        posECR[0] = 4077000.0;
        posECR[1] = 5000000.0;
        posECR[2] = -3200000.0;
        ray[0] = 0.0002;
        ray[1] = -1.0;
        ray[2] = -0.422;
        returnStatus = PGS_CSC_GrazingRay(earthEllipsTag,
                       posECR,ray,&latitude,&longitude,
                       &missAltitude,&slantRange, posNEAR,posSURF);
        printf("Longitude %f\n",longitude);
        printf("Latitude: %f\n",latitude);
        printf("Altitude: %f\n",missAltitude);
        printf("Slant Range: %f\n",slantRange);
        if(returnStatus == PGS_S_SUCCESS)
        {
                printf("Point on Ray Nearest Earth: %f, %f, %f\n",
                        posNEAR[0],posNEAR[1],posNEAR[2]);
                printf("Point on Surface Nearest Ray: %f, %f, %f\n",
                        posSURF[0],posSURF[1],posSURF[2]);
        }
        else if(returnStatus == PGSCSC_W_HIT_EARTH)
        {
                printf("Midpoint of Ray Chord in Earth: %f, %f, %f\n",
                        posNEAR[0],posNEAR[1],posNEAR[2]);
                printf("Line of Sight Strikes Earth at: %f, %f, %f\n",
                        posSURF[0],posSURF[1],posSURF[2]);
        }
        else
        {
          ** test errors,
              take appropriate
               action **
        }

       
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_CSC_4.f'
      implicit none
      integer            returnstatus
      integer            pgs_csc_grazingray 
      character*20       earthellipstag
      double precision   posecr(3)
      double precision   ray(3)
      double precision   latitude
      double precision   longitude
      double precision   missaltitude
      double precision   slantrange
      double precision   posnear(3) 
      double precision   possurf(3) 
 
      data earthellipstag/'GEM-10B'/

      posecr(1) = 4077000.0
      posecr(2) = 5000000.0
      posecr(3) = -3200000.0
      ray(1) = 0.0002
      ray(2) = -1.0
      ray(3) = -0.422
      returnstatus = pgs_csc_grazingray(earthellipstag,posecr,
     1ray,latitude,longitude,missaltitude,slantrange,posnear,
     2possurf)
      print*,'Longitude: ',longitude
      print*,'Latitude: ',latitude
      print*,'Slant Range: ',slantrange
      print*,'Altitude: ',missaltitude

         if(returnStatus .eq. PGS_S_SUCCESS) then

      print*,'Point on Ray Nearest Earth: X = ',
     1posnear(1),'  Y = ',posnear(2),' Z = ', posnear(3)
      print*,'Point on Surface Nearest Ray: X = ',
     1possurf(1),'  Y = ',possurf(2),' Z = ', possurf(3)

         else if (returnStatus .eq. PGSCSC_W_HIT_EARTH) then

      print*,'Midpoint of Ray Chord Within Earth: X = ',
     1posnear(1),'  Y = ',posnear(2),' Z = ', posnear(3)
      print*,'Line of Sight Strikes Earth at: X = ',
     1possurf(1),'  Y = ',possurf(2),' Z = ', possurf(3)

        else

C  ** test errors,
C      take appropriate
C       action **
      endif


NOTES:
   If an invalid earthEllipsTag is input, the program will use the WGS84 Earth
   model by default.

   A crude diagram of the two geometries (miss Earth limb or hit) follows:

   Key:  n = posNEAR; s = posSURF, v = viewer (spacecraft)
         b = point on back side of Earth from v where
             ray geometrically "exits" the ellipsoid
         x = point deemed representative of surface point 
             below that point on a circumrefracted ray 
             which is closest to the ellipsoid 
         v = viewer (instrument boresight)
         + = points on the ray other than n, b, v, or x
         . = Earth surface points

   I. Crude diagram of ray missing the Earth limb:

                +
                   +
                      +
                ..       n posNEAR  }
          .             s   +         }
       .               /   .   +       } slant range is from n to v
     .             posSURF   .    +     }
    .                         .       v   }
                                    viewer


   
   II. Crude diagram of ray through the Earth:

                .x
          .            .
       .                   . posSURF
     b++++++++++ n ++++++++++s+++++++++++++++++++++++++++v (viewer)
   .          posNEAR          .
                 |<-------------- slant range ---------->|

   Note that in this latter case, we do not compute the
   location of the surface point "x" above posNEAR in the
   diagram, but we invite the user to assume that its
   latitude and longitude are approximately the same as 
   those of posNEAR.


REQUIREMENTS:
   PGSTK - 

DETAILS: 

GLOBALS:
   None

FILES:
   earthfigure.dat 

FUNCTIONS CALLED:
   PGS_CSC_ECRtoGEO()
   PGS_CSC_LookTwice()
   PGS_CSC_GEOtoECR()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_SMF.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

/* name of this function */

#define  FUNCTION_NAME  "PGS_CSC_GrazingRay()"

/* convergence tolerance */

#define TOLERANCE 1.0e-10
#define ZERO 0.0

PGSt_SMF_status
PGS_CSC_GrazingRay(
    char          earthEllipsTag[20],
    PGSt_double   posECR[3],      /* spacecraft location */
    PGSt_double   ray[3],         /* ECR Unit Look Vector */
    PGSt_double   *latitude,      /* geodetic latitude of Near Point 
				     - see posNEAR, below */
    PGSt_double   *longitude,     /* longitude of Near Point 
				     - see posNEAR, below */
    PGSt_double   *missAltitude,  /* altitude of posNEAR off Earth 
				     limb (can be negative when
				     ray passes through ellipsoid) */
    PGSt_double   *slantRange,    /* range to intersection of ray 
				     with ellipsoid; or, when it 
				     misses, to the Near Point */
    PGSt_double   posNEAR[3],     /* point on ray nearest the 
				     ellipsoid; or, when ray hits
				     ellipsoid, midpoint between
				     the two intersections */
    PGSt_double   posSURF[3])     /* point on Earth nearest ray
				     (when it misses), or nearest
				     of the two pierce points 
				     (when it hits) */
{		     		    
    PGSt_integer counter=0;          /* iteration counter */
    PGSt_integer iXYZ;               /* coordinate index */
    PGSt_double  equatRad_A;         /* Earth equatorial radius A */
    PGSt_double  polarRad_C;         /* Earth polar radius C */
    PGSt_double  xLook[2][3];        /* intersections of ray and ellipsoid */
    PGSt_double  slantTwo[2];        /* slant ranges to TWO Earth 
                                        intersections */
    PGSt_double  normalV[3];         /* normal to ellipsoid */
    PGSt_double  flatFac;            /* Earth flattening factor (A-C)/A */
    PGSt_double  normVec;            /* normalized l.o.s. vector */
    PGSt_double  dotProd = -1.0;     /* dot product of Ray and normal
                                        to ellipsoid */

    PGSt_double  altitude;           /* altitude satat Near Point */
    PGSt_double  distance;           /* distance along ray from spacecraft to
                                        point of closest approach */
    PGSt_double  deltaDistance;      /* increment of slant range */

    PGSt_SMF_status  code;           /* status code returned from
					PGS_SMF_GetMsg() */
		     
    /* mnemonic returned by PGS_SMF_GetMsg() */
    char         mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];
		     
    /* message returned by PGS_SMF_GetMsg() */
    char         msg[PGS_SMF_MAX_MSG_SIZE]; 
    char         specifics[PGS_SMF_MAX_MSG_SIZE];     /* detailed error msg */
		     
    PGSt_SMF_status  returnStatus;     /* return value of this function */
    PGSt_SMF_status  retrn2;           /* additional return from lower level */
    PGSt_SMF_status  returnStatus1;    /* return value of PGS functions called
					  (except first one) */

    normVec = PGS_CSC_Norm(ray);
    if(fabs(normVec) < EPS_64)
    {
       returnStatus  = PGSCSC_W_ZERO_PIXEL_VECTOR;
       sprintf(specifics,"%s","ZERO Pixel Vector");
       PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
       return(returnStatus);
    }  /* end  if(fabs(abnormVec) < EPS_64) */
    for(iXYZ = 0; iXYZ  < 3; iXYZ++)
    {
       ray[iXYZ] =  ray[iXYZ]/normVec;
    }
    
    returnStatus = PGS_CSC_GetEarthFigure(earthEllipsTag,&equatRad_A,
					      &polarRad_C);
    switch (returnStatus)
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
        PGS_SMF_SetDynamicMsg(returnStatus,"the polar radius is negative "
			  "or zero",FUNCTION_NAME);
        return returnStatus;
    }

    /* determine the flattening factor */
	
    flatFac = (equatRad_A - polarRad_C) / equatRad_A;
	
    /* check for errors in the value of the flattening factor */
	
    if (flatFac < 0.0)
    {
        returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
        PGS_SMF_SetDynamicMsg(returnStatus, "the specified earth model "
                              "defines a prolate earth",FUNCTION_NAME);
        return returnStatus;
    } 
    else if (fabs(flatFac) < EPS_64)
    {
        returnStatus = PGSCSC_W_SPHERE_BODY;
        PGS_SMF_SetDynamicMsg(returnStatus,
		  "a spherical earth model is being used",
		  FUNCTION_NAME);
    }

/* HERE CHECK THAT x**2/A**2 + y**2/A**2 + z**2/C**2 > 1.0 ***/

    if((posECR[0] * posECR[0] /(equatRad_A * equatRad_A) +
    posECR[1] * posECR[1] /(equatRad_A * equatRad_A) +
    posECR[2] * posECR[2] /(polarRad_C * polarRad_C )) <= 1.00)
    {
       returnStatus = PGSCSC_W_SUBTERRANEAN;
       PGS_SMF_SetDynamicMsg(returnStatus,"the spacecraft is inside "
                             "the Earth",FUNCTION_NAME);
       return returnStatus;
    }

/* Check that instrument is not looking away from nadir */

    if( PGS_CSC_dotProduct(posECR,ray,3) > 0.0)
    {
       returnStatus = PGSCSC_W_LOOK_AWAY;
       PGS_SMF_SetDynamicMsg(returnStatus,
		  "the Ray looks away from Earth", FUNCTION_NAME);
       *missAltitude = PGSd_GEO_ERROR_VALUE;
       return returnStatus;
    }
	
    /* Begin  Iterations */
    
    returnStatus1 = PGS_CSC_LookTwice(posECR,ray,equatRad_A,equatRad_A,
                                      polarRad_C,slantTwo,xLook);
    switch (returnStatus1)
    {
       case PGS_S_SUCCESS:
       /* line of sight struck Earth; must average the two
          intersection points to find the middle one */

          for(iXYZ = 0; iXYZ < 3; iXYZ++)
          {
             posNEAR[iXYZ] = 0.5 *(xLook[0][iXYZ]+xLook[1][iXYZ]);
             posSURF[iXYZ] = xLook[0][iXYZ];
          }
          *slantRange = slantTwo[0];
          retrn2 = PGS_CSC_ECRtoGEO(posNEAR,earthEllipsTag,longitude,
                                    latitude,missAltitude);
          switch (retrn2)
          {        
             case PGS_S_SUCCESS:
        /* recent change to avoid dumping deep cases */
             case PGSCSC_W_INVALID_ALTITUDE:
                break;
             case PGSCSC_W_DEFAULT_EARTH_MODEL:
             case PGSCSC_W_SPHERE_BODY:
             case PGSCSC_W_LARGE_FLATTENING:
                returnStatus = retrn2;
                break;
             case PGSCSC_W_TOO_MANY_ITERS:
                if (returnStatus == PGS_S_SUCCESS)
                {
                   PGS_SMF_GetMsg(&code,mnemonic,msg);
                   if(code != retrn2)
                       PGS_SMF_GetMsgByCode(retrn2,msg);
                   returnStatus = PGS_E_TOOLKIT;
                   sprintf(specifics,"%s%s","Too many iterations ",
	                   "in PGS_CSC_ECRtoGEO");
                   retrn2 = PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                                         FUNCTION_NAME);
                   return returnStatus;
                }
             case PGSCSC_E_BAD_EARTH_MODEL:
                return retrn2;
             default:
                PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
                return PGS_E_TOOLKIT;
          } /* end switch on return status from ECRtoGEO */
          returnStatus = PGSCSC_W_HIT_EARTH;
          sprintf(specifics,"%s%s","The line of sight strikes",
	        " the Earth");
          retrn2 = PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                                         FUNCTION_NAME);
          return returnStatus;  /* end case PGS_S_SUCCESS */
      
       case PGSCSC_W_MISS_EARTH:
           break;
       case PGSCSC_W_ZERO_PIXEL_VECTOR:
           sprintf(specifics,"%s%s","PGS_CSC_LookPoint() detected a "
		," zero Look vector (impossible)");
           retrn2 = PGS_SMF_SetDynamicMsg(returnStatus,specifics,
				       FUNCTION_NAME);
       default:
	   return PGS_E_TOOLKIT;
    } /* end switch (returnStatus1) */
  
    distance = -  PGS_CSC_dotProduct(posECR,ray,3);
    while(fabs(dotProd) > TOLERANCE)
    { 
       counter ++;
       if(distance < 0.0)
       {
          /* l.o.s. is away from Earth */
          returnStatus = PGSCSC_W_LOOK_AWAY;
          PGS_SMF_SetDynamicMsg(returnStatus,
                "the Ray looks away from Earth", FUNCTION_NAME);
          *missAltitude = PGSd_GEO_ERROR_VALUE;
          return returnStatus;
       }
       for(iXYZ = 0; iXYZ <3; iXYZ++)
       {
          posNEAR[iXYZ] = posECR[iXYZ] + distance * ray[iXYZ];
       }

    /* Convert posECR from ECR coordinates to Geodetic (GEO) coordinates
       to get altitude, latitude, longitude of point Q  */

       returnStatus1 = PGS_CSC_ECRtoGEO(posNEAR,earthEllipsTag,
                                        longitude,latitude,&altitude);
       switch (returnStatus1)
       {        
	  case PGS_S_SUCCESS:
	    break;
	  case PGSCSC_W_DEFAULT_EARTH_MODEL:
	  case PGSCSC_W_SPHERE_BODY:
	  case PGSCSC_W_LARGE_FLATTENING:
       /* recent change to avoid dumping deep cases */
	  case PGSCSC_W_INVALID_ALTITUDE:
		returnStatus = returnStatus1;
	    break;
	  case PGSCSC_W_TOO_MANY_ITERS:
	    if (returnStatus == PGS_S_SUCCESS)
	    { 
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);   
		returnStatus = PGSCSC_W_ERROR_IN_GRAZINGRAY;
	    }
	  case PGSCSC_E_BAD_EARTH_MODEL:
	    return returnStatus;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
       } /* end switch on returnStatus1 */ 
       *missAltitude = altitude;
     
       /* Calculate Dot Product of Surface Normal and Ray */ 
       normalV[0] = cos(*latitude) * cos(*longitude);
       normalV[1] = cos(*latitude) * sin(*longitude);
       normalV[2] = sin(*latitude);

       dotProd = PGS_CSC_dotProduct(normalV,ray,3);
       deltaDistance = - dotProd * PGS_CSC_Norm(posNEAR);
       distance += deltaDistance;
       if(counter > 20)
       {
          returnStatus = PGSCSC_W_TOO_MANY_ITERS;
          return returnStatus;
       } 
       /* SUCCESS */
    }  /* end while */

    returnStatus1 = PGS_CSC_GEOtoECR(*longitude,*latitude,ZERO,
                                     earthEllipsTag,posSURF);
    switch (returnStatus1)
    {        
       case PGS_S_SUCCESS:
	    break;
       case PGSCSC_W_DEFAULT_EARTH_MODEL:
       case PGSCSC_W_SPHERE_BODY:
       case PGSCSC_W_LARGE_FLATTENING:
      /* recent change to avoid dumping deep cases */
       case PGSCSC_W_INVALID_ALTITUDE:
	     returnStatus = returnStatus1;
            break;
       case PGSCSC_W_TOO_MANY_ITERS:
            if (returnStatus == PGS_S_SUCCESS)
            { 
	        PGS_SMF_GetMsg(&code,mnemonic,msg);
	        if(code != returnStatus1)
                {
	            PGS_SMF_GetMsgByCode(returnStatus1,msg);   
                }
	        returnStatus = PGSCSC_W_ERROR_IN_GRAZINGRAY;
            }
       case PGSCSC_E_BAD_EARTH_MODEL:
         return returnStatus;
       default:
         PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
    }  /* end switch on returnStatus1 */
    
    *slantRange = distance;
    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);

    /*  Return to calling function. */
    
    return returnStatus; 
}


