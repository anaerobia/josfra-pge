/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_CSC_GEOtoECR.c

DESCRIPTION:
  This file contains the function PGS_CSC_GEOtoECR()

AUTHOR:
  Anubha Singhal    / Applied Research Corporation
  Peter Noerdlinger / Applied Research Corporation
  Curt Schafer      / Steven Myers & Associates

HISTORY:
  24-Jan-1994       PDN   Designed and created         
  16-Aug-1994       PDN   Modified design for change in calling sequence
  17-Aug-1994       AS    Original version
  31-Aug-1994       AS    Put in call to PGS_CSC_GetEarthFigure()
  15-Sep-1994       AS    Updated prologs
  21-Sep-1994       AS    Made changes from code inspection
  27-Jan-1995       AS    Fixed comment line in prolog - warning message 
                          PGSCSC_W_INVALID_ALTITUDE incorrectly specified
			  as PGSCSC_E_INVALID_ALTITUDE
  31-May-1995      GTSK   Altered algorithm to remember last earth tag passed
                          in. 
  29-Jan-1996       PDN   Put in check for bad incoming latitude
  07-Jul-1999        CS   Updated for Threadsafe functionality

ND_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:   
      This function converts from geodetic to ECR coordinates
 
NAME:    
      PGS_CSC_GEOtoECR() 

SYNOPSIS:
C:
      #include <PGS_CSC.h>

      PGSt_SMF_status
      PGS_CSC_GEOtoECR(
             PGSt_double       longitude,
             PGSt_double       latitude,
             PGSt_double       altitude,
             char              *earthEllipsTag,
             PGSt_double       posECR[3]);

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_CSC_4.f'

      integer function pgs_csc_geotoecr(longitude,latitude,altitude,
                                        earthellipstag,x)
      double precision  longitude
      double precision  latitude
      double precision  altitude
      character*20      earthellipstag
      double precision  posecr(3)

DESCRIPTION:
      This tool converts a geodetic latitude and longitude to ECR (Earth 
      Centered Rotating) coordinates 

INPUTS:
      Name               Description     Units    Min        Max
      ----               -----------     -----    ---        ---
      longitude          longitude       radians  -pi        pi
      latitude           geodetic        radians  -pi/2      pi/2
                         latitude
      altitude           altitude        meters   -.1*radius N/A 
                                                  of Earth  
      earthellipstag     earth model     N/A      N/A        N/A               
                         used

OUTPUTS:
      Name       Description         Units    Min           Max
      ----       -----------         -----    ---           ---
      posECR     ECR rectangular     meters   -100,000,000  100,000,000
                 coordinates                  (usually each component
                                              will be in range [-10,000,000,
                                              +10,000,000 m ] but function
                                              will work for Geosynchronous
                                              cases, e.g. )                 
RETURNS:
      PGS_S_SUCCESS                 success case
      PGSCSC_W_DEFAULT_EARTH_MODEL  the default earth model is used because
                                    a correct one was not specified   
      PGSCSC_W_SPHERICAL_BODY       using a spherical earth model
      PGSCSC_W_LARGE_FLATTENING     issued if flattening factor is greater
                                    then 0.01  
      PGSCSC_W_INVALID_ALTITUDE     an invalid altitude was specified
      PGSCSC_E_BAD_LAT              an invalid latitude was specified
      PGSCSC_E_BAD_EARTH_MODEL      The equatorial or polar radius is negative
                                    or zero OR the radii define a prolate Earth
      PGS_E_TOOLKIT                 something unexpected happened, execution
                                    of function ended prematurely
      PGSTSF_E_GENERAL_FAILURE      Bad return from PGS_TSF_GetTSFMaster or 
                                    PGS_TSF_GetMasterIndex()  

EXAMPLES:
C:
      PGSt_SMF_status      returnStatus
      PGSt_double          longitude
      PGSt_double          latitude
      PGSt_double          altitude
      char                 earthEllipsTag[5],
      PGSt_double          posECR[3] 
      char                 err[PGS_SMF_MAX_MNEMONIC_SIZE];  
      char                 msg[PGS_SMF_MAX_MSG_SIZE];      
     
      longitude = 0.45;
      latitude = 1.34;
      altitude = 5000.0;
      strcpy(earthEllipsTag,"WGS84"); 
       
      returnStatus = PGS_CSC_GEOtoECR(longitude,latitude,altitude,
                                      earthEllipsTag,posECR);
      if(returnStatus != PGS_S_SUCCESS)
      {
	  PGS_SMF_GetMsg(&returnStatus,err,msg);
	  printf("\nERROR: %s",msg);
      }

FORTRAN:
      implicit none
      integer              pgs_csc_geotoecr
      integer              returnstatus
      double precision     longitude
      double precision     latitude
      double precision     altitude
      character*20         earthellipstag,
      double precision     posecr(3)
      character*33 	   err
      character*241 	   msg

      longitude = 0.45
      latitude = 1.34
      altitude = 5000
      earthellipstag = 'WGS84'

      returnstatus = pgs_csc_geotoecr(longitude,latitude,altitude,
     >                                earthellipstag,posecr)

      if(returnstatus .ne. pgs_s_success) then
	  returnstatus = pgs_smf_getmsg(returnstatus,err,msg)
	  write(*,*) err, msg
      endif

NOTES:
      NONE
      
REQUIREMENTS:
      PGSTK - 1050, 0930

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
      PGSg_TSF_CSCGEOtoECRequatRad_A
      PGSg_TSF_CSCGEOtoECRpolarRad_C
      PGSg_TSF_CSCGEOtoECRflatFac
      PGSg_TSF_CSCGEOtoECRmodelStatus

FILES:
      earthfigure.dat

FUNCTION CALLS:
      PGS_SMF_SetStaticMsg      set error/status message
      PGS_SMF_SetDynamicMsg     set error/status message
      PGS_SMF_SetUnknownMsg     set error/status message for unknown message
      PGS_CSC_GetEarthFigure    get equatorial and polar radius
      PGS_TSF_GetTSFMaster      setup threads version
      PGS_TSF_GetMasterIndex    get the index for this thread
      
END_PROLOG
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <PGS_math.h>               /* math library  header file */
#include <PGS_CSC.h>            /* Header file for Coord. Transform Tools */
#include <PGS_TSF.h>            /* Header file for Coord. Transform Tools */

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_GEOtoECR()"
#define HALFPI          1.57079632679489662 /* half pi */

PGSt_SMF_status
PGS_CSC_GEOtoECR(                     /* converts from geodetic to ECR
                                         coordinates */
    PGSt_double       longitude,      /* longitude of point on earth*/
    PGSt_double       latitude,       /* latitude of point on earth */
    PGSt_double       altitude,       /* altitude of point on earth */
    char              *earthEllipsTag,/* used for determining earth radius */
    PGSt_double       posECR[3])      /* ECR rectangular coordinates */
{
    PGSt_double  distOfPoint;    /* distance of point from Earth's axis */
    PGSt_double  recip;          /* reciprocal of the radius of curvature in a
                                    plane orthogonal to the meridian */
    
    PGSt_SMF_status returnStatus;/* return value of function and default */
      
    /* returnStatus of call to PGS_CSC_GetEarthFigure() */

    static PGSt_SMF_status modelStatus=PGS_S_SUCCESS;
    
    /* earth tag from any previous call to this function, initialized to some
       nonsense value (hopefully) */

    static char oldEarthTag[50]="THIS IS THE DEFAULT (WGS84) EARTH MODEL";

    /* Earth radii (initialized to default (WGS84) values) */

    static PGSt_double equatRad_A=6378137.0;        /* equatorial radius */
    static PGSt_double polarRad_C=6356752.314245;    /* polar radius */

    /* flattening factor - which is related to eccentricity of the ellipsoid of
       revolution (initialized to default (WGS84) value) */

    static PGSt_double flatFac=3.35281068123811075E-3;

#ifdef _PGS_THREADSAFE

    /*  Declare variables used for THREADSAFE version to replace statics
        The local names are appended with TSF and globals are preceded with
        directory and function name   */
 
    PGSt_TSF_MasterStruct *masterTSF;
    PGSt_SMF_status modelStatusTSF;
    PGSt_double equatRad_ATSF;
    PGSt_double polarRad_CTSF;
    PGSt_double flatFacTSF;
    char *oldEarthTagTSF;     /* TSD key     does NOT use global */

       /*  Globals     originals in PGS_TSF_SetupCSC.c */

    extern PGSt_double PGSg_TSF_CSCGEOtoECRequatRad_A[];
    extern PGSt_double PGSg_TSF_CSCGEOtoECRpolarRad_C[];
    extern PGSt_double PGSg_TSF_CSCGEOtoECRflatFac[];
    extern PGSt_SMF_status PGSg_TSF_CSCGEOtoECRmodelStatus[];

    int masterTSFIndex;

    /* Get index  and initialize keys  Then test for bad returns */

    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(returnStatus) || masterTSFIndex ==
                                         PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* Initialize the variables used for the THREADSAFE version */

     oldEarthTagTSF = (char *) pthread_getspecific(
                masterTSF->keyArray[PGSd_TSF_KEYCSCGEOTOECROLDEARTHTAG]);
     modelStatusTSF = PGSg_TSF_CSCGEOtoECRmodelStatus[masterTSFIndex];
     equatRad_ATSF = PGSg_TSF_CSCGEOtoECRequatRad_A[masterTSFIndex];
     polarRad_C = PGSg_TSF_CSCGEOtoECRpolarRad_C[masterTSFIndex];
     flatFacTSF = PGSg_TSF_CSCGEOtoECRflatFac[masterTSFIndex];
 
    /*  Almost entire function is duplicated for the THREADSAFE version to
    protect statics.  When a value is reassigned the global or key is updated 
                        NO LOCKS  1KEY   5 GLOBALS   */

    /* Initialization */

    /* Threadsafe Protect: modelStatus */

    returnStatus = modelStatusTSF;

    /* if the earthEllipsTag passed in is not the same as the last value used,
       get equatorial and polar radius and calculate flattening and ellipsoid
       eccentricity */
    /* Threadsafe Protect: oldEarthTag */

    if (strcmp(earthEllipsTag,oldEarthTagTSF) != 0) /* BEGIN setup */
    {
	
        /* Threadsafe Protect: equatRad_A, polarRad_C 
                reassign their values for next use */

	returnStatus = PGS_CSC_GetEarthFigure(earthEllipsTag,&equatRad_ATSF,
					      &polarRad_CTSF);
        PGSg_TSF_CSCGEOtoECRequatRad_A[masterTSFIndex] = equatRad_ATSF;
        PGSg_TSF_CSCGEOtoECRpolarRad_C[masterTSFIndex] = polarRad_CTSF;

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
        /* Threadsafe Protect: equatRad_A  */
	
	if (equatRad_ATSF <= 0.0)
	{
	    returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
	    PGS_SMF_SetDynamicMsg(returnStatus,
				  "the equatorial radius is negative or zero",
				  FUNCTION_NAME);
	    return returnStatus;
	}
	
	/* issue an error if the polar radius is negative */
        /* Threadsafe Protect: polarRad_C  */
	
	if (polarRad_CTSF <= 0.0)
	{
	    returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
	    PGS_SMF_SetDynamicMsg(returnStatus,"the polar radius is negative"
				  "or zero",FUNCTION_NAME);
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

	/* determine the flattening factor */
        /* Threadsafe Protect: flatFac, equatRad_A, polarRad_C 
                reassign flatFac's value for next use */
	
	flatFacTSF = (equatRad_ATSF - polarRad_CTSF) / equatRad_ATSF;
          PGSg_TSF_CSCGEOtoECRflatFac[masterTSFIndex] = flatFacTSF;

	/* check for errors in the value of the flattening factor */
        /* Threadsafe Protect: flatFac */
	
	if (flatFacTSF < 0.0)
	{
	    returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
	    PGS_SMF_SetDynamicMsg(returnStatus,"the specified earth model "
				  "defines a prolate earth",FUNCTION_NAME);
	    return returnStatus;
	} 
        /* Threadsafe Protect: flatFac */

	else if (fabs(flatFacTSF) < EPS_64)
	{
	    returnStatus = PGSCSC_W_SPHERE_BODY;
	    PGS_SMF_SetDynamicMsg(returnStatus,
				  "a spherical earth model is being used",
				  FUNCTION_NAME);
	}

        /* Threadsafe Protect: flatFac */

	else if (flatFacTSF > 0.01)
	{
	    returnStatus = PGSCSC_W_LARGE_FLATTENING;
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	}
	
	/* set the oldEarthTag to the current value */
        /* Threadsafe Protect:  oldEarthTag
                reassign oldEarthTag's KEY value for next use */
	
	oldEarthTagTSF[49] = '\0';
	strncpy(oldEarthTagTSF,earthEllipsTag,49);
        pthread_setspecific(
          masterTSF->keyArray[PGSd_TSF_KEYCSCGEOTOECROLDEARTHTAG],
                                            (void *) oldEarthTagTSF);

	/* set modelStatus to current value of returnStatus */
        /* Threadsafe Protect:  modelStatus
                reassign modelStatus's  value for next use */

	modelStatusTSF = returnStatus;
        PGSg_TSF_CSCGEOtoECRmodelStatus[masterTSFIndex] = modelStatusTSF;

    } /* END setup */
    else if (returnStatus != PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    
    /* compute distance of point from Earth's surface */

    /* check for an invalid altitude */
    /* Threadsafe Protect:  equatRad_A */

    if (altitude < (-0.1 * equatRad_ATSF))
    {
	returnStatus = PGSCSC_W_INVALID_ALTITUDE;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }
    
    /* compute the reciprocal of the radius of curvature in a plane orthogonal
       to the meridian */
    /* Threadsafe Protect:  flatFac, equatRad_A, */

    recip = 1.0 / sqrt(1 - (2*flatFacTSF - flatFacTSF*flatFacTSF) * 
               sin(latitude) * sin(latitude));
    distOfPoint = ((equatRad_ATSF * recip) + altitude) * cos(latitude);
    
    /* calculate the ECR coordinates */

    /* calculate the x coordinate */
    posECR[0] = distOfPoint * cos(longitude);

    /* calculate the y coordinate */
    posECR[1] = distOfPoint * sin(longitude);

    /* calculate the z coordinate */
    posECR[2] = ((equatRad_ATSF * (1-flatFacTSF) * (1-flatFacTSF) * recip) + 
                                                altitude) * sin(latitude);
    
#else

    /* Initialization */

    returnStatus = modelStatus;

    /* if the earthEllipsTag passed in is not the same as the last value used,
       get equatorial and polar radius and calculate flattening and ellipsoid
       eccentricity */

    if (strcmp(earthEllipsTag,oldEarthTag) != 0) /* BEGIN setup */
    {
	
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
	    PGS_SMF_SetDynamicMsg(returnStatus,"the polar radius is negative"
				  "or zero",FUNCTION_NAME);
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

	/* determine the flattening factor */
	
	flatFac = (equatRad_A - polarRad_C) / equatRad_A;
	
	/* check for errors in the value of the flattening factor */
	
	if (flatFac < 0.0)
	{
	    returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
	    PGS_SMF_SetDynamicMsg(returnStatus,"the specified earth model "
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
	else if (flatFac > 0.01)
	{
	    returnStatus = PGSCSC_W_LARGE_FLATTENING;
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	}
	
	/* set the oldEarthTag to the current value */
	
	oldEarthTag[49] = '\0';
	strncpy(oldEarthTag,earthEllipsTag,49);

	/* set modelStatus to current value of returnStatus */

	modelStatus = returnStatus;

    } /* END setup */
    else if (returnStatus != PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    
    /* compute distance of point from Earth's surface */

    /* check for an invalid altitude */

    if (altitude < (-0.1 * equatRad_A))
    {
	returnStatus = PGSCSC_W_INVALID_ALTITUDE;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }
    
    /* compute the reciprocal of the radius of curvature in a plane orthogonal
       to the meridian */

    recip = 1.0 / sqrt(1 - (2*flatFac - flatFac*flatFac) * sin(latitude) * 
                       sin(latitude));
    distOfPoint = ((equatRad_A * recip) + altitude) * cos(latitude);
    
    /* calculate the ECR coordinates */

    /* calculate the x coordinate */
    posECR[0] = distOfPoint * cos(longitude);

    /* calculate the y coordinate */
    posECR[1] = distOfPoint * sin(longitude);

    /* calculate the z coordinate */
    posECR[2] = ((equatRad_A * (1-flatFac) * (1-flatFac) * recip) + altitude) *
                 sin(latitude);
#endif
    
    if (returnStatus == PGS_S_SUCCESS)  
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);

    return returnStatus;
}
