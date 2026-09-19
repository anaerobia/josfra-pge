/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_SpaceRefract()

DESCRIPTION:
   This file contains the function PGS_CSC_SpaceRefract.c

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   20-Sept-1994 PDN  Initial Version
   20-Dec -1994 PDN  Corrected Adiabatic index, humidity, returned angle
   02-Feb-1995  PDN  Improved error messaging
   29-Jun-1995  PDN  Added a check for negative zenith angle input 
   29-Mar-1996  PDN  made tropause, surface temperature latitude dependent
   01-May-1996  PDN  fixed comments
   28-Oct-1999  PDN  fixed returned data in some failure cases, changing
                     from "uninitialized" to PGSd_GEO_ERROR_VALUE

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Estimate the refraction for a ray incident from space or a line of sight
   from space to the Earth's surface, based on the unrefracted zenith angle.

NAME:  PGS_CSC_SpaceRefract()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
 
   PGSt_SMF_status
   PGS_CSC_SpaceRefract(
       PGSt_double spaceZenith,
       PGSt_double altitude,  
       PGSt_double latitude, 
       PGSt_double *surfaceZenith, 
       PGSt_double *displacement) 

FORTRAN:
      include 'PGS_CSC_4.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'

      integer function pgs_csc_spacerefract(spacezenith,altitude,
     >                                      latitude,surfacezenith,
     >                                      displacement)
      double precision spacezenith
      double precision altitude
      double precision latitude
      double precision surfacezenith
      double precision displacement

DESCRIPTION:
   This function estimates the refraction of a  ray incident from space 
   or a line of sight from space to the Earth's surface based on the 
   unrefracted zenith angle (most common algorithms, intended for
   ground based observation, require knowledge of the refracted, not
   the unrefracted zenith angle.)  The algorithm is suitable for:
     (a) approximate determination of the apparent Solar zenith angle
         from the true (geometrical; unrefracted) Solar zenith angle.
         (Obviously, also applicable to Lunar zenith angle, etc.)
     (b) approximate correction of the viewing angle from space, to
         approximately remove the effects of refraction.
   
    The method is briefly indicated in the NOTES, q.v. for various caveats.
    

INPUTS:
   Name              Description              Units      Min         Max
   --------          -----------              -----      ---         ---
   spaceZenith       unrefracted zenith       radians     0          pi/2
                     angle                                         (90 deg) 

   altitude          altitude                 meters     -1000      50000

   latitude          latitude                 radians    -pi/2       pi/2

OUTPUTS:
   Name              Description              Units       Min       Max
   ----              -----------              -----       ---       ---
   surfaceZenith     refracted zenith angle   radians      0        n/a

   displacement      displacement of the 
                     footpoint of ray         radians      0        ~0.01


RETURNS:
   PGS_S_SUCCESS                 successful return
   PGSCSC_W_INVALID_ALTITUDE     attempt to calculate refraction at point too
                                 far below Earth's surface
   PGSCSC_E_BAD_LAT              latitude out of the range (+-) pi/2 was input
   PGSCSC_E_INVALID_ZENITH       negative zenith angle
   PGSCSC_W_BELOW_HORIZON        attempt to calculate refraction of ray below
                                 horizon
 
EXAMPLES:
C:

   PGSt_SMF_status  returnStatus;

   PGSt_double      spaceZenith=0.4;
   PGSt_double      altitude=5000.0;
   PGSt_double      latitude=-0.2;
   PGSt_double      surfaceZenith;
   PGSt_double      displacement; 
   
   returnStatus = PGS_CSC_SpaceRefract(spaceZenith,altitude,latitude,
                  &surfaceZenith,&displacement) 
                                      
   {
     ** test errors, take appropriate action **
   }
       
FORTRAN:
      implicit none
      integer          pgs_csc_spacerefract
      integer          returnstatus

      double precision spacezenith
      double precision altitude
      double precision latitude
      double precision surfacezenith
      double precision displacement

      data spacezenith /0.4/
      data altitude /5000.0/
      data latitude /-0.2/

      returnstatus = pgs_csc_spacerefract(spacezenith,altitude,latitude,
     >                                    surfacezenith,displacement)
                                    
      if (returnstatus .ne. pgs_s_success) go to 90
      write(6,*) surfacezenith,displacement

  90  write(6,99)returnstatus
  99  format('ERROR:',I15)  

NOTES: 
   This algorithm is intended as a mean-atmosphere approximation, valid for
   white light (for example, sunlight).  Refraction is quite wavelength
   dependent, and in the atmosphere it will also depend strongly on local
   conditions (the weather, e.g.).  The present algorithm is intended to be a
   reasonable approximation such that to do better one would need local and, for
   large zenith angles, regional weather.

   Caveat: The altitude is used ONLY to obtain the air pressure, which is then
   used to obtain the surface index of refraction.  Users who employ an inflated
   Earth radius in geolocation should be especially careful to replace any
   derived altitude with the height in meters above the geoid before calling
   this function.

   The (horizontal) displacement of the ray is in a vertical plane containing
   the ray and is in the sense that the actual (refracted) ray will meet the
   Earth d = (displacement)*Re meters from the geometrical (unrefracted)
   position, on the side towards the horizon.

      Outer  Space  Here 

           .
             . unrefracted ray 
               .
                 .
   refracted ray  .*
                   . * unrefracted ray
   _________________.__*________________ Earth surface
                      d

  the angle "displacement" is the angle that the displacement in meters "d"
  subtends at Earth center.


  The following table exemplifies results at sea level, using a conversion of
  6371000 m per radian on the displacement.

  Altitude - sea level

 Zenith angle   Zenith angle                        linear
   in space      at surface         refraction   displacement   
    (deg)          (deg)             (deg)         (meters)

  10.000000        9.997066         0.002934        0.549064
  20.000000       19.993944         0.006056        1.222937
  30.000000       29.990394         0.009606        2.221314
  40.000000       39.986039         0.013961        3.982978
  45.000000       44.983363         0.016637        5.464087
  50.000000       49.980174         0.019826        7.725334
  55.000000       54.976243         0.023757       11.398788
  60.000000       59.971192         0.028808       17.845724
  61.000000       60.969996         0.030004       19.696711
  62.000000       61.968722         0.031278       21.816620
  63.000000       62.967361         0.032639       24.256691
  64.000000       63.965905         0.034095       27.080360
  65.000000       64.964340         0.035660       30.366779
  70.000000       69.954333         0.045667       58.380584
  75.000000       74.938025         0.061975      136.072953
  76.000000       75.933417         0.066583      166.728721
  77.000000       76.928121         0.071879      207.384912
  78.000000       77.921967         0.078033      262.469333
  79.000000       78.914723         0.085277      338.977167
  80.000000       79.906069         0.093931      448.379942
  81.000000       80.895543         0.104457      610.332976
  82.000000       81.882461         0.117539      860.316290
  83.000000       82.865762         0.134238     1266.536004
  84.000000       83.843713         0.156287     1970.638000
  85.000000       84.813286         0.186714     2974.066487
  86.000000       85.768718         0.231282     4858.394025
  87.000000       86.697712         0.302288     8677.416632
  88.000000       87.569758         0.430242    17538.457911
  89.000000       88.295108         0.704892    41818.325388
  90.000000       88.619113         1.380887   113429.256196

  Note that the linear displacement at 88 degrees zenith angle is about 17.5 km
  - very substantial.  Because of the very approximate atmosphere model, this
  number could vary by perhaps 25% depending on weather in temperate and
  tropical regions; in the arctic it would be considerably smaller. The
  displacement at 90 degrees incidence, over 113 km, is only suggestive and
  could easily vary by 50%.

   The composition of the atmosphere was obtained from Allen's "Astrophysical
   Quantities, 2nd ed." (London, the Athlone Press, 1976) p. 119.

   The atmosphere model is used only to get the index of refraction at sea
   level.  Latitude dependence is based on the following fits to the sea 
   level temperature and mean scale height as functions of latitude, from
   Allen's table on p. 121.
   
   Fits to the  tropopause height vs latitude and surface temperature:

	latitude (deg)  Tropopause (m)	T(Fitted)(K)
	 0		19873.3		298.25
	10		17963.6		297.57
	20		16194.8		295.57
	30		14566.9		292.29
	40		13080.0		287.99
	50		11733.9		282.35
	60		10528.8		275.99
	70		9464.5		268.96
	80		8541.2		261.46
	90		7758.8		253.73

   The only effect of the atmosphere model is to affect the dropoff of refraction
   with altitude; the larger scale height at the equator and smaller at the
   poles implies that the fall off with altitude is steeper at high latitudes.

   The calculations are based on the geometry of a spherical Earth. User may
   employ her/his favorite Earth radius to transform radians of displacement to
   meters.
   
REQUIREMENTS:
   PGSTK - 0860, 1080, 1092

DETAILS:
   See "Theoretical Basis of the SDP Toolkit Geolocation Package", Document
   445-TP-002-002, May 1995, by P. Noerdlinger, for more information on the 
   algorithm.

GLOBALS:
   None

FUNCTIONS CALLED:
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()

FILES:
   none

END_PROLOG:
*******************************************************************************/

#include <PGS_CSC.h>
#include <stdio.h>
#include <PGS_math.h>

/* constants */

#define DEGperRADIAN  57.295779513082  /* degrees per radian */
#define RADIANperDEG  0.01745329251994 /* radians per degree */
#define pi            3.1415926535897932
#define HAPI          1.57079632679489662 /* half pi */
#define MEAN_EARTH_A  6371000.0        /* mean Earth radius */
#define TROPORATE     0.0065           /* degrees temperature decrease 
                                          per meter altitude increase */
#define DINDEX0       0.0002905        /* Allen's approximate sea level 
                                          index of refraction minus 1.00 */
#define GAS_CONST     8314.3           /* universal gas constant */
#define GRAV_ACCEL    9.805            /* mean sea level acceleration of 
                                          gravity */
                                       /* OK to use near sea level to our 
                                          level of accuracy */
#define MEAN_MOLEC    28.825           /* mean molecular wt in troposphere based
					  on 80 parts dry air (wt 28.96) plus 1
					  part water vapor (wt 18.01)  */
#define ALLENFAC      0.00117          /* ratio of term in tan(z) cubed to that
                                          in tan(z)--adjusted from 0.00115 for
                                          smoother fit to equation from A.A.
                                          supplement  */
#define CHANGE_Z      1.465            /* changeover angle from Allen style 
					  approx. to one from Explanatory 
					  Supplement (about 83.9 degrees)  */

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_SpaceRefract()"

PGSt_SMF_status 
PGS_CSC_SpaceRefract(
    PGSt_double spaceZenith,          /* unrefracted zenith angle (radians) */
    PGSt_double altitude,             /* altitude in meters */
    PGSt_double latitude,             /* latitude in radians*/
    PGSt_double *surfaceZenith,       /* refracted zenith angle (radians) */
    PGSt_double *displacement)        /* displacement of the footpoint of ray 
					 in radians about Earth center, in the 
					 vertical plane of the ray, and directed
					 away from the geometric look point */
{
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;

    PGSt_double spaceZen1;
    PGSt_double indexSurf;
    PGSt_double dindex;                /* index of refraction - 1.0  */
    PGSt_double elevation;             /* (pi/2) - zenith angle */
    PGSt_double tempFac;               /*  ratio of temperature at altitude to 
                                           sea level */
    PGSt_double densExponent;          /*  exponent for density vs altitude */
    PGSt_double densFac;               /*  ratio of density at altitude to 
                                           sea level */
    PGSt_double densScale;             /*  density scale height - used only to
                                           estimate displacement at small z */
    PGSt_double tropopause;             /* tropopause altitude (m) */
    PGSt_double surfTemp;               /* mean suurface temperature (K) */
    PGSt_double xlat;                   /* absolute value of the latitude*/

    PGSt_double refrac;                /*  refraction in radians = z0 - z' */
    char specifics[PGS_SMF_MAX_MSG_SIZE];     /* detailed error msg */

    /*  This function contains a static which is not THREAD SAFE: countMsg
        This is used to stop warning messages at 50.
        The messages from all threads will go to the same file, this will make
        the order unpredictable.  Therefore there is no reason to protect each
        individual thread's number of messages.  After any 50 messages the
        file will issue No more messages. */
 
    static PGSt_integer countMsg = 0;      /* counter to avoid deluge of warnings */

    /*  if the altitude is < -1000 m there is almost certainly an error -
        return an error message.  Note that this altitude is off the Geoid, 
        NOT the ellipsoid.   */

    if (altitude < -1000.0)
    {
       returnStatus =  PGSCSC_W_INVALID_ALTITUDE;

       /* set error message */
       if ( countMsg < 50 )
       {
          countMsg++;
          sprintf(specifics,"%s%9.6g%s",
		  "altitude off geoid = ", altitude,
		  " m, unreasonably deep in Earth)");
          PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
       }
       *surfaceZenith = PGSd_GEO_ERROR_VALUE;
       *displacement = PGSd_GEO_ERROR_VALUE;
       return returnStatus;
    }

    if ((latitude < -HAPI) || (latitude > HAPI))
    {
       returnStatus =  PGSCSC_E_BAD_LAT;

       /* set error message */
       if ( countMsg < 50 )
       {
          countMsg++;
          sprintf(specifics,"%s%s",
		  "Refraction set = PGSd_GEO_ERROR_VALUE, ",
		  "latitude must be between -pi/2 and pi/2");
          PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
       }
       *surfaceZenith = PGSd_GEO_ERROR_VALUE;
       *displacement = PGSd_GEO_ERROR_VALUE;
       return returnStatus;
    }

    /* check space zenith angle > 0.0 - less than 0 is meaningless */

    if (spaceZenith < 0.0)
    {
       returnStatus =  PGSCSC_E_INVALID_ZENITH;
       *surfaceZenith = PGSd_GEO_ERROR_VALUE;
       *displacement = PGSd_GEO_ERROR_VALUE;

       /* set error message */

       if ( countMsg < 50 )
       {
          countMsg++;
          sprintf(specifics,"%s%9.6g",
                  "Negative Zenith Angle Input = ", (double) spaceZenith);
          PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
       }
       return returnStatus;
    }

    /*  if the altitude is > 50000 m there is almost certainly an error -
        return 0.0 and an error message  (value would be zero)  */

    if(altitude > 50000.0)
    {
       returnStatus =  PGSCSC_W_INVALID_ALTITUDE;

       /* return the result no refraction so as not to burden user with
          PGSd_GEO_ERROR_VALUE */

       *surfaceZenith = spaceZenith;
       *displacement = 0.000;

       /* set error message */

       if ( countMsg < 50 )
       {
          countMsg ++;
          sprintf(specifics,"%s%10.6g%s",
                  "Refraction set = 0, altitude off geoid = ",
                   (double) altitude," m ( >  model atmosphere)");
          PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
       }
       return returnStatus;
    }

    /*  if the zenith angle in space exceeds 90 degrees, algorithm 
        fails - return with error condition  */
    
    if(spaceZenith >= HAPI ) 

    {
       /* set message; set return status  */

       returnStatus = PGSCSC_W_BELOW_HORIZON;
       PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
       *surfaceZenith = spaceZenith;
       *displacement = PGSd_GEO_ERROR_VALUE;
       return returnStatus;
    }

   /* calculate height of tropopause, scale height, and surface 
      temperature at the given latitude.  Based on fits to pp.
      120-121 of Allen's astrophysical Quantities.  */

       xlat = fabs(latitude);
       tropopause = 19873.3 - 11345.4 * xlat + 2312.9 * xlat * xlat;
	surfTemp = 253.73 + 44.52 * cos(latitude);

      /* calculate exponent for adiabatic law */
  
      densExponent = GRAV_ACCEL * MEAN_MOLEC/(GAS_CONST * TROPORATE) -1.0;

   if (altitude <= tropopause)   /* adiabatic density law  */
   {
      tempFac = 1.0 - TROPORATE * altitude / surfTemp;
      densFac = exp(densExponent * log(tempFac)); 
   }
   else              /* isothermal atmos starting at tropopause */
   {
      tempFac = 1.0 - TROPORATE * tropopause / surfTemp;
      densFac = exp(densExponent * log(tempFac) - (altitude - tropopause) *
                GRAV_ACCEL * MEAN_MOLEC/(GAS_CONST * surfTemp * tempFac)); 
   }

   dindex  = DINDEX0 * densFac;
   indexSurf = 1.000 + dindex;

   /* true surface zenith angle  z'  */

   *surfaceZenith = asin(sin(spaceZenith)/indexSurf);


   /* Get empirically estimated value of z = z' . The A.A. supplement eq'n is
      not consistent with what we know about refraction at small zenith angles,
      so we use Allen there and the Supplement only for large angles */

   if(*surfaceZenith < CHANGE_Z )
   {

   /* use formula from Allen, p. 124 for z(z') for angles < 1.465 rad (z').
      Allen's eq. is refraction = 58.3"arc * tan(z') - 0.067 "arc * (tan(z'))^3
      where ^ = exponentiation operator.  The formula is adapted here by noting
      that the number 58.3 arc seconds is just the index of refraction, less
      1.0, and converted from radians to arc seconds.  ALLENFAC is the ratio of
      0.067 to 58.3 - relating the 2 terms - adjusted slightly upward from
      0.00115 to 0.00117 for a smoother transition to the A.A. Supplement
      Reversing the transformation and scaling with air density, we get: */

      densScale = tempFac * surfTemp / (TROPORATE * densExponent);
      refrac = dindex * (tan(*surfaceZenith) - ALLENFAC * 
             tan(*surfaceZenith) * tan(*surfaceZenith) * tan(*surfaceZenith))/
             (1.0 + densScale/MEAN_EARTH_A);
   }
   else
   {
      
   /* refraction according to Eq. 3.283-1, p. 144, Astron. Almanac Supplement
      with elevation = H */

   /* peculiar combination of units due to A. A. Suppl. !  */

   elevation = HAPI - *surfaceZenith;

   refrac = densFac * (0.01667*RADIANperDEG)/
             tan(elevation + 7.31*RADIANperDEG/ 
                (elevation*DEGperRADIAN + 4.4));
   }

   if (refrac < 0.0) refrac = 0.0;

   /* z = angle of original ray to normal at true impact */

   spaceZen1 = *surfaceZenith + refrac;  

   /* displacement from z0 - z */

   *displacement =  (spaceZenith - spaceZen1 > 0.0) ? 
                      spaceZenith - spaceZen1 : 0.0;

    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    return returnStatus;
}





