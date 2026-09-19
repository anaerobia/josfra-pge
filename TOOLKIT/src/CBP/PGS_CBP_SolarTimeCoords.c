/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CBP_SolarTimeCoords.c

DESCRIPTION:
   This file contains the function PGS_CBP_SolarTimeCoords()

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation
   Deborah A. Foch      / Applied Research Corporation
 
HISTORY:
   13-APR-1994  PDN  Designed			
   14-APR-1994  PDN  Coded
   29-AUG-1994  PDN  Fixed modular arithmetic on Local Apparent Solar Time
   22-SEPT-1994 DAF  Revised prolog, variable types and comments
   24-SEPT-1994 DAF  Made cosmetic changes
   26-SEPT-1994 DAF  Updated jdDiff equation to change order
   04-OCT-1994  DAF  Updated prolog to explain Min and Max input time values
   06-FEB-1995  DAF  Added GetMsg and GetMsgByCode for leap second ignored case
   06-JUN-1995  GTSK Spruced things up a bit, fixed up prolog and few lines of
                     code (minor improvements to efficiency/readability)
   07-JUL-1995  GTSK move FORTRAN binding to external file
   

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get Solar Time Coordinates

NAME:
   PGS_CBP_SolarTimeCoords()
 
SYNOPSIS:
C:
   #include <PGS_CBP.h>

   PGSt_SMF_status
   PGS_CBP_SolarTimeCoords(
      char          asciiUTC[28],
      PGSt_double   longitude,
      PGSt_double   *meanSolTimG,
      PGSt_double   *meanSolTimL,
      PGSt_double   *apparSolTimL,
      PGSt_double   *solRA,
      PGSt_double   *solDec)

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'

      integer function pgs_cbp_solartimecoords(asciiutc,longitude,
     >                                         meansoltimg,meansoltiml,
     >                                         apparsoltiml,solra,
     >                                         soldec)
      character*27       asciiutc
      double precision   longitude
      double precision   meansoltimg
      double precision   meansoltiml
      double precision   apparsoltiml
      double precision   solra
      double precision   soldec
      

DESCRIPTION:
   This tool performs a low accuracy rapid calculation of solar time
   and coordinates.  The accuracy of the equations here is expected
   to be about 0.5 minutes of time and 0.04 degrees for the coordinates 
   of the Sun.


INPUTS:
   Name           Description              Units       Min        Max
   -------        ------------             -----       ---        ---
   asciiUTC       coordinated Universal     N/A           See NOTES
                  Time in CCSDS ASCII               
                  Time Code A or B format
   longitude      longitude of observer     radians    -pi        pi
                  (positive is East)
                  Not required for solar
		  coordinates; should be 
		  set to 0 in that case.

   
OUTPUTS:
   Name           Description              Units       Min        Max
   ----           -----------              -----       ---        ---
   meanSolTimG    Greenwich Mean Solar     seconds      0         86400
                  Time as seconds
                  from midnight      
   meanSolTimL    Local Mean Solar Time    seconds      0         86400
                  as seconds from midnight
   apparSolTimL   Local Apparent Solar     seconds      0         86400
                  Time as seconds 
                  from midnight         
   solRA          Right Ascension of       radians      0         2*pi
                  the Mean Sun           
   solDec         Declination of the       radians     -pi        pi
                  Mean Sun
   
RETURNS:
   PGS_S_SUCCESS              successful execution
   PGSTD_M_LEAP_SEC_IGNORED   input leap second has been ignored
   PGSTD_E_TIME_FORMAT_ERROR  error in format of input ASCII UTC time
   PGSTD_E_TIME_VALUE_ERROR   error in value of input ASCII UTC time
   PGS_E_TOOLKIT              something unexpected happened, execution aborted
   
EXAMPLES:
C:
   PGSt_SMF_status    returnStatus;
   char               asciiUTC[28];
   PGSt_double        longitude;
   PGSt_double        meanSolTimG;
   PGSt_double        meanSolTimL;
   PGSt_double        solRA;
   PGSt_double        solDec;
   
   strcpy(asciiUTC,"1991-01-01T11:29:30");
   returnStatus = PGS_CBP_SolarTimeCoords(asciiUTC,longitude,&meanSolTimG,
                                          &meanSolTimL,&apparSolTimL,
					  &solRA,&solDec)
                               
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }
   printf("start time:%s",asciiUTC);
   printf("\n longitude: %lf",longitude);
   
   printf("Greenwich Mean Solar Time:%lf  Local Mean Solar Time:%lf",
          meanSolTimG,meanSolTimL);
   printf("\n Local Apparent Solar Time:%lf  Solar Right Asc/Dec:%lf/%lf",
          apparSolTimL,solRA,solDec);
 
       
FORTRAN:
      implicit none
      integer            pgs_cbp_solartimecoords
      character*27       asciiutc
      double precision   longitude      
      double precision   meansoltimg   
      double precision   meansoltiml     
      double precision   apparsoltiml 
      double precision   solra         
      double precision   soldec         
      integer            returnstatus

      asciiutc = '1991-01-01T11:29:30'
      longitude = 1.0
  
      returnstatus = pgs_cbp_solartimecoords(asciiutc,longitude,
     >                                       meansoltimg,meansoltiml,
     >                                       apparsoltiml,solra,soldec)
                                                    
      if(returnstatus .ne. pgs_s_success) go to 90
      write(6,*) asciiutc, longitude
      write(6,*) meansoltimg, meansoltiml, apparsoltiml, solra, soldec

  90  write(6,99) returnstatus
  99  format('ERROR:',I50)  

NOTES:
   The equations used in this function are referenced on page C24 of
   the 1994 Astronomical Almanac.  They are low precision formulas
   which give the apparent coordinates of the Sun to a precision of
   0.01 degrees and the equation of time to a precision of 0.1 minutes
   between the years 1950 and 2050.  Less accuracy is expected for
   dates before 1950 and after 2050.
  
   More accurate solar time determination requires improved solar
   coordinates and the value of UT1-UTC.  These items are accessible
   through other PGS tools.
 
   In particular, the Solar ephemeris yields accurate solar coordinates
   and the function PGS_TD_gmst() gives Greenwich Mean Sidereal Time.
   These can be combined to obtain more accurate Mean Solar Time. 
   The difference UT1 - UTC is determined within the CSC group of
   functions, in the transformations between ECR and ECI.  

REQUIREMENTS:
   PGSTK-0760, 0750

DETAILS:
   The Mean Solar Time derived here differs from UTC due to differences 
   in definition, and to the inaccuracy of the approximate equation
   for the right ascension of the Sun.  The apparent solar time is
   thus probably of greater interest, as it can be used to estimate
   the hour angle of the Sun, the lengthening of shadows, etc.

   References:
     - Explan. Suppl to Astron Almanac, pp. 49, 75, 77, 78, 80,484 and 485.  
     - 1994 Astron. Almanac, pp. B2, B6, C24.  

GLOBALS:
   None

FILES:
   None
     
FUNCTIONS_CALLED:
   PGS_TD_UTCtoUTCjd()
   PGS_TD_gmst()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CBP.h>           

/* constants */

#define SECONDSperDAY  86400.0             /* number of seconds in a day */
#define SECONDSperDEG  240.0               /* number of seconds of time per
					      degree of Earth rotation, right
					      ascension, or longitude */
#define TWELVEHOURS    43200.0             /* number of seconds in 12 hours */
#define TWOPI          6.283185307179586   /* twice pi (ratio of circumference
					      to diameter of a circle) */
#define DEGperRADIAN   57.295779513082     /* degrees per radian */
#define RADIANSperDEG  0.01745329251994    /* radians per degree */
#define JD2000         2451545.0           /* Julian Date of 01-01-2000 */

/* name of this function */

#define FUNCTION_NAME "PGS_CBP_SolarTimeCoords()"

PGSt_SMF_status 
PGS_CBP_SolarTimeCoords(          /* get solar time cooordinates */
    char           asciiUTC[28],  /* UTC start time in CCSDS ASCII Time
				     Code format (A or B). */
    PGSt_double    longitude,     /* longitude of observer */  
    PGSt_double    *meanSolTimG,  /* Greenwich Mean Solar Time in seconds since
				     midnight */ 
    PGSt_double    *meanSolTimL,  /* Local Mean Solar Time in seconds since
				     midnight */
    PGSt_double    *apparSolTimL, /* Local Apparent Solar Time in seconds since
				     midnight */
    PGSt_double    *solRA,        /* Right Ascension of Mean Sun (radians) */
    PGSt_double    *solDec)       /* Declination of Mean Sun (radians) */
{
    PGSt_double     jdUTC[2];     /* UTC Julian date in units of days;
				     jdUTC[0] is integer days and
				     jdUTC[1] is day fraction */
    PGSt_double     jdDiff;       /* Number of Julian days from jdUTC
				     to JD2000 */
    PGSt_double     sunG;         /* mean anomaly of the Sun */  
    PGSt_double     longSun;      /* mean longitude of the Sun */  
    PGSt_double     longitSec;    /* observer's East longitude in 
				     seconds of time */
    PGSt_double     solRAseconds; /* right ascension of the Mean Sun 
				     in seconds of time (not arcseconds) */
    PGSt_double     obliq;        /* obliquity of the ecliptic */
    PGSt_double     lambda;       /* ecliptic longitude of the Sun */
    PGSt_double     gmst;         /* Greenwich Mean Sidereal Time in radians */
    PGSt_double     gmstSeconds;  /* Greenwich Mean Sidereal Time in seconds */
    PGSt_double     apparSolTimG; /* apparent Solar time at Greenwich 
				     (same as apparSolTimL at longitude 0) */
    PGSt_double     EquT;         /* "equation of time" (Apparent Solar Time
				     minus Mean Solar Time) */
    PGSt_double     longitDiff;   /* True minus Mean Solar Longitude 
				     on the ecliptic */
       
    PGSt_SMF_status returnStatus; /* value returned by this function */

    
    /* Convert CCSDS ASCII UTC to Julian Date UTC */
    
    returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC,jdUTC);
    
    /* Check return value from PGS_TD_UTCtoUTCjd() */
    
    switch(returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_M_LEAP_SEC_IGNORED:
	break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    jdDiff = (jdUTC[0] - JD2000)  + jdUTC[1];    /* Julian days from J2000   */
    
    /* Get mean anomaly of Sun  */
    
    sunG =  RADIANSperDEG * (357.528 + 0.9856003 * jdDiff);

    /* Adjust the range of sunG to between 0 and 2*pi */

    sunG = fmod(sunG,TWOPI);
    
    /* Obtain the obliquity of the ecliptic (obliq) and the mean 
       longitude of the Sun (longSun) */
    
    obliq = RADIANSperDEG * (23.44 - 0.0000004 * jdDiff);
    
    longSun = RADIANSperDEG * (280.466 + 0.9856474 * jdDiff);
    
    /* This leads to lambda, the true ecliptic longitude of the Sun.
       First save the correction of the mean longitude to the apparent
       longitude for later use.  (The difference between mean and true
       is due to the eccentricity of the Earth's orbit, reflected in the
       apparent orbit of the Sun around the Earth)  */
    
    longitDiff =  1.915 * sin(sunG) + 0.020 * sin(2.0*sunG);   
    lambda = longSun + RADIANSperDEG * longitDiff;
    
    /* Adjust to range between 0 and 2*pi  */
    
    if ((lambda=fmod(lambda,TWOPI)) < 0)
	lambda += TWOPI;
    
    /* Get the Right Ascension of the Mean Sun using an equation modified
       from p. C24 of the Astronomical Almanac to take advantage of the 
       full range of the atan2 function */
    
    *solRA =  atan2(cos(obliq)*sin(lambda),cos(obliq)*cos(lambda));
    *solRA = *solRA > 0 ? *solRA : *solRA  + TWOPI;
    
    /*  Get the Declination of the Mean Sun */
    
    *solDec =  asin(sin(obliq)*sin(lambda));
    
    /* Call function PGS_TD_gmst() to get Greenwich Mean Sidereal Time (in
       radians), since the various Solar times are differences in Right
       Ascension of the Mean Sun and either Greenwich or the location of the
       observer, converted to seconds of time.  Note: leap seconds are ignored
       (also technically UT1 should be used here but this is a quick 'n dirty
       calculation so we'll just approximate UT1 with UTC). */
    
    gmst = PGS_TD_gmst(jdUTC);
    
    /* Convert GMST to Greenwich Mean Solar Time in seconds using the
       equation: 
       
       meanSolTimG = [GMST in secs] - [12 hrs in secs] - [solRA in secs] 
       
       First convert gmst and solRA to seconds */
    
    gmstSeconds  = gmst * SECONDSperDAY/TWOPI;
    
    solRAseconds = *solRA * SECONDSperDAY/TWOPI;
    
    *meanSolTimG  = gmstSeconds - TWELVEHOURS - solRAseconds;
    
    /* Adjust to range between 0 and 86400 seconds (start and end of day) */
    
    if ((*meanSolTimG=fmod(*meanSolTimG,SECONDSperDAY)) < 0)
	*meanSolTimG += SECONDSperDAY;
    
    /* Get the apparent solar time (in seconds) from mean solar time
       by adding  the "equation of time" (Explan. Suppl. p. 485).
       This part of the calculation is in degrees and time unit and does
       not involve conversion to or from radians */
    
    EquT = SECONDSperDEG * (-longitDiff + 2.466 * sin(2*lambda) 
			    - 0.053 * sin(4*lambda));
    
    apparSolTimG = *meanSolTimG + EquT;
    
    
    /* Get local mean and apparent solar time by adding the longitude in
       seconds of time to the corresponding Greenwich solar time */
    
    /* First convert longitude to seconds */
    
    longitSec = SECONDSperDAY * longitude/(TWOPI);
    
    /* Get local mean solar time */
    
    *meanSolTimL = *meanSolTimG + longitSec;
    
    /* Adjust to range between 0 and 86400 seconds (start and end of day) */  
    
    if ((*meanSolTimL=fmod(*meanSolTimL,SECONDSperDAY)) < 0)
	*meanSolTimL += SECONDSperDAY;
    
    /* Get apparent solar time */
    
    *apparSolTimL = apparSolTimG + longitSec;
    
    /* Adjust to range between 0 and 86400 seconds (start and end of day) */ 
    
    if ((*apparSolTimL=fmod(*apparSolTimL,SECONDSperDAY)) < 0)
	*apparSolTimL += SECONDSperDAY;
   
    /* Return to calling function */
    
    if(returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    
    return returnStatus;

}
