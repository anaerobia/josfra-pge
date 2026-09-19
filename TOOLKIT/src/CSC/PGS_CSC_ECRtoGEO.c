/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_CSC_ECRtoGEO.c

DESCRIPTION:
  This file contains the function PGS_CSC_ECRtoGEO()

AUTHOR:
  Peter Noerdlinger  / Applied Research Corporation
  Anubha Singhal     / Applied Research Corporation
  Guru Tej S. Khalsa / Applied Research Corporation
  Curt Schafer       / Steven Myers & Associates
  Xinmin Hua         / Steven Myers & Associates

HISTORY:
  01-Feb-1994       PDN   Designed       
  19-Mar-1994       PDN   Coded
  31-Aug-1994       AS    Included call to PGS_CSC_GetEarthFigure and changed
                          calling sequence
  15-Sep-1994       AS    Modified prolog to conform to latest ECS/PGS 
                          standards
  21-Sep-1994       AS    Made changes from code inspection
  22-Sep-1994       PDN   Designed new algorithm for determining the latitude
                          and altitude (more robust at large geocentric 
                          distances; converges faster; no trig functions in
                          iterative loop)
  23-Sep-1994       AS    Coded new algorithm
  31-May-1995      GTSK   Altered algorithm to remember last earth tag passed
                          in. 
  12-Jul-1995       PDN   Rewrote prolog; changed comments
  29-Oct-1996       PDN   Put in early check point is not too
                          near Earth center - convergence is poor there
                          Output approximate answer in that case
  02-Jan-1997       PDN   Replaced the above approximation with the Borkowski
                          method
  07-Jul-1999        CS   Updated for Threadsafe functionality
  12-Apr-2000        XH   Designed a new algorithm for determining the latitude
                          and altitude based on iteration for exact solution
                          of eqaution for normRhoFoot. Totally eliminated sin()
                          and cos() computations. Save ~25% CPU time.
  15-May-2000       PDN   Added a macro SIGN

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:  
      This function converts from ECR to geodetic coordinates

NAME:   
      PGS_CSC_ECRtoGEO()                                    

SYNOPSIS:
C:
      #include <PGS_CSC.h>

      PGSt_SMF_status
      PGS_CSC_ECRtoGEO(
             PGSt_double       posECR[3],
             char              *earthEllipsTag,
             PGSt_double       *longitude,
             PGSt_double       *latitide,
             PGSt_double       *altitude);

FORTRAN:     
      include 'PGS_SMF.f'
      include 'PGS_CSC_4.f'
 
      integer function  pgs_csc_ecrtogeo(posecr,earthellipstag,longitude,
                                         latitude,height,
         
      double precision  posecr(3)
      character*20      earthellipstag
      double precision  longitude
      double precision  latitude
      double precision  altitude
              
DESCRIPTION:
      This function converts from ECR to geodetic coordinates.

INPUTS:
      Name            Description           Units   Min           Max
      ----            -----------           -----   ---           ---          
      posECR[3]       Geocentric position   meters  N/A           N/A

      earthEllipsTag  earth model           N/A     N/A           N/A          
                      used                                           
						 
OUTPUTS:
      Name          Description           Units       Min       Max
      ----          -----------           -----       ---       ---     
      latitude      geodetic latitude     radians     -pi/2     pi/2

      longitude     longitude             radians     -pi       pi
 
      altitude      altitude               meters    -.1*Earth   sky's the limit
 
RETURNS:
      PGS_S_SUCCESS		    Successful return
      PGSCSC_W_TOO_MANY_ITERS       Normal Iteration Count exceeded - could
                                    indicate inconsistent units for Spacecraft
                                    and Earth data, or corrupted Earth Axis
                                    values
      PGSCSC_W_INVALID_ALTITUDE     Spacecraft underground - probably 
                                    indicates bad input data
      PGSCSC_W_SPHERE_BODY          using a spherical earth model
      PGSCSC_W_LARGE_FLATTENING     issued if flattening factor is greater
                                    then 0.01  
      PGSCSC_W_DEFAULT_EARTH_MODEL  Uses default earth model
      PGSCSC_E_BAD_EARTH_MODEL      The equatorial or polar radius is negative
                                    or zero OR the radii define a prolate Earth
      PGS_E_TOOLKIT                 Something unexpected happened, execution
                                    of function ended prematurely
      PGSTSF_E_GENERAL_FAILURE      bad return from PGS_TSF_GetTSFMaster or 
                                    PGS_TSF_GetMasterIndex()  

EXAMPLES:
C:
      PGSt_SMF_status      returnStatus
      PGSt_double          longitude
      PGSt_double          latitude
      PGSt_double          altitude
      char                 earthEllipsTag[5],
      PGSt_double          posECR[3] = {1000.5,64343.56,34343.92} 
      char                 err[PGS_SMF_MAX_MNEMONIC_SIZE];  
      char                 msg[PGS_SMF_MAX_MSG_SIZE];      
      
      strcpy(earthEllipsTag,"WGS84"); 
       
      returnStatus = PGS_CSC_ECRtoGEO(posECR[3],earthEllipsTag,longitude,
                                      latitude,altitude);
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

      data posecr/1000.5,64343.56,34343.92/
      earthellipstag = 'WGS84'

      returnstatus = pgs_csc_ecrtogeo(posecr,earthellipstag,longitude,
     >                                latitude,altitude)

      if(returnstatus .ne. pgs_s_success) then
	  returnstatus = pgs_smf_getmsg(returnstatus,err,msg)
	  write(*,*) err, msg
      endif

NOTES:  
      The Earth axes will accessed from the earthfigure.dat file.
      The input must always be in meters and should never be a unit vector.

REQUIREMENTS: 
      PGSTK-1050, 930

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

      The new method, which is different from the one above, is:
      To find the latitude and altitude, denote the point (X,Y,Z) as B and
      normalize all lengths by dividing by equatRad_A, the Earth's 
      equatorial radius.  The purpose is to find the coordinates normRhoFoot
      and normZFoot of the point on the elliptical surface of the Earth
      vertically below (or above) point B. The normRhoFoot can be
      found by iteration based on an equation derived from geometrical 
      relationships by Xinmin Hua.


GLOBALS:
     PGSg_TSF_CSCECRtoGEOmodelStatus
     PGSg_TSF_CSCECRtoGEOequatRad_A
     PGSg_TSF_CSCECRtoGEOpolarRad_C
     PGSg_TSF_CSCECRtoGEOellipsEccSq
     PGSg_TSF_CSCECRtoGEOflatFac
     

FILES:
      earthfigure.dat
 
FUNCTIONS_CALLED:
      PGS_SMF_SetStaticMsg      set error/status message
      PGS_SMF_SetDynamicMsg     set error/status message
      PGS_SMF_SetUnknownMsg     set error/status message for unknown message
      PGS_CSC_GetEarthFigure    get equatorial and polar radius 
      PGS_TSF_GetTSFMaster      setup threads version
      PGS_TSF_GetMasterIndex    get the index for this thread

END_PROLOG:   
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <PGS_math.h>            /*  Standard header file for math libraries */
#include <PGS_CSC.h>         /*  General header file for Geolocation Tools */
#include <PGS_TSF.h>         /*  General header file for Geolocation Tools */

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_ECRtoGEO()"
#define HALFPI          1.57079632679489662 /* half pi */
/* define a macro "SIGN" = 1 for positive argument, -1 for negative 
   it will be -1 for zero argument but will not encounter that case) */
#define SIGN(x) ((x>0)? 1.00 : -1.00)

PGSt_SMF_status  
PGS_CSC_ECRtoGEO(                 /* converts from ECR to geodetic
				     coordinates */
    PGSt_double  posECR[3],       /* Geocentric position vector in ECR */
    char         *earthEllipsTag, /* tag for selecting Earth Ellipsoid model */
    PGSt_double  *longitude,      /* latitude in radians */
    PGSt_double  *latitude,       /* longitude in radians */
    PGSt_double  *altitude)       /* altitude in meters */
{
    int          ncount = 0;      /* number of iterations */
    
    PGSt_double  rho;             /* Component of ECR vector in equatorial 
                                     plane */
    PGSt_double  normRho;         /* Component of ECR vector in equatorial 
                                     plane normalized */ 
    PGSt_double  normZ;           /* Z component of ECR vector normalized */
    PGSt_double  lengthOQ;        /* distance of Q from O */
    PGSt_double  signn;           /* used to check if the point is in or 
                                     outside the ellipsoid */
    PGSt_double  normRhoFootStore;   /* stored value of normRhoFoot -xh */
    PGSt_double  normRhoFoot;     /* normalized radial cylindrical coordinate of
                                     point U at foot of normal from B to 
                                     ellipsoid */
    PGSt_double  normZFoot;       /* location of point at foot of normal */
    PGSt_double  correcterm;      /* intermediate results for iteration -xh */
    
    PGSt_SMF_status returnStatus; /* return status */

    /* returnStatus of call to PGS_CSC_GetEarthFigure() */

    static PGSt_SMF_status modelStatus=PGS_S_SUCCESS;
    
    /* earth tag from any previous call to this function, initialized to some
       nonsense value (hopefully) */

    static char oldEarthTag[50]="THIS IS THE DEFAULT (WGS84) EARTH MODEL";

    /* Earth radii (initialized to default (WGS84) values) */

    static PGSt_double equatRad_A=6378137.0;        /* equatorial radius */
    static PGSt_double polarRad_C=6356752.314245;    /* polar radius */

    /* ellipsoid eccentricity (initialized to default (WGS84) value) */
    
    static PGSt_double ellipsEccSq=6.69438002301199714E-3;

    /* flattening factor - which is related to eccentricity of the ellipsoid of
       revolution (initialized to default (WGS84) value) */

    static PGSt_double flatFac=3.35281068123811075E-3;

#ifdef _PGS_THREADSAFE

    /* Declare variables used for THREADSAFE version to replace statics
        The local names are appended with TSF and globals are preceded with
        directory and function name  */

    PGSt_TSF_MasterStruct *masterTSF;
    PGSt_double equatRad_ATSF;
    PGSt_double polarRad_CTSF;
    PGSt_double ellipsEccSqTSF;
    PGSt_double flatFacTSF;
    PGSt_SMF_status modelStatusTSF;
    char *oldEarthTagTSF;     /* TSD key     does Not use global */

       /*  Globals     originals in PGS_TSF_SetupCSC.c */

    extern PGSt_SMF_status PGSg_TSF_CSCECRtoGEOmodelStatus[];
    extern PGSt_double PGSg_TSF_CSCECRtoGEOequatRad_A[];
    extern PGSt_double PGSg_TSF_CSCECRtoGEOpolarRad_C[];
    extern PGSt_double PGSg_TSF_CSCECRtoGEOellipsEccSq[];
    extern PGSt_double PGSg_TSF_CSCECRtoGEOflatFac[];
   int masterTSFIndex;
 
   /* Get index  and initialize keys then test for bad returns  */

   returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
   masterTSFIndex = PGS_TSF_GetMasterIndex();
   if (PGS_SMF_TestErrorLevel(returnStatus) || masterTSFIndex ==
                                          PGSd_TSF_BAD_MASTER_INDEX)
   {
       return PGSTSF_E_GENERAL_FAILURE;
   }
 
   /*   Initialize the variables used for the THREADSAFE version */

    modelStatusTSF = PGSg_TSF_CSCECRtoGEOmodelStatus[masterTSFIndex];
    equatRad_ATSF = PGSg_TSF_CSCECRtoGEOequatRad_A[masterTSFIndex];
    polarRad_CTSF = PGSg_TSF_CSCECRtoGEOpolarRad_C[masterTSFIndex];
    ellipsEccSqTSF = PGSg_TSF_CSCECRtoGEOellipsEccSq[masterTSFIndex];
    flatFacTSF = PGSg_TSF_CSCECRtoGEOflatFac[masterTSFIndex];
    oldEarthTagTSF = (char *) pthread_getspecific(
                masterTSF->keyArray[PGSd_TSF_KEYCSCECRTOGEOOLDEARTHTAG]);
 
   /*  Almost entire function is duplicated for the THREADSAFE version
       to protect statics  NO LOCKS  1KEY   5 GLOBALS        
       When a value is reassigned the global or key is updated       */

    /* Initialization */
    /* Threadsafe Protect modelStatus */

    returnStatus = modelStatusTSF;

    /* if the earthEllipsTag passed in is not the same as the last value used,
       get equatorial and polar radius and calculate flattening and ellipsoid
       eccentricity */

    if (strcmp(earthEllipsTag,oldEarthTagTSF) != 0) /* BEGIN setup */
    {
	/* Threadsafe Protect: equatRad_A, polarRad_C
                reassign their values for next use   */

	returnStatus = PGS_CSC_GetEarthFigure(earthEllipsTag,&equatRad_ATSF,
					      &polarRad_CTSF);
        PGSg_TSF_CSCECRtoGEOequatRad_A[masterTSFIndex] = equatRad_ATSF;
        PGSg_TSF_CSCECRtoGEOpolarRad_C[masterTSFIndex] = polarRad_CTSF;

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
        /* Threadsafe Protect:  polarRad_C   */
	
	if (polarRad_CTSF <= 0.0)
	{
	    returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
	    PGS_SMF_SetDynamicMsg(returnStatus,"the polar radius is negative "
				  "or zero",FUNCTION_NAME);
	    return returnStatus;
	}
	
	/* determine the flattening factor */
	/* Threadsafe Protect: equatRad_A, polarRad_C, flatFac
                reassign flatFac's value for next use   */
	
	flatFacTSF = (equatRad_ATSF - polarRad_CTSF) / equatRad_ATSF;
        PGSg_TSF_CSCECRtoGEOflatFac[masterTSFIndex] = flatFacTSF;
	
	/* check for errors in the value of the flattening factor */
	
	if (flatFacTSF < 0.0)
	{
	    returnStatus = PGSCSC_E_BAD_EARTH_MODEL;
	    PGS_SMF_SetDynamicMsg(returnStatus,"the specified earth model "
				  "defines a prolate earth",FUNCTION_NAME);
	    return returnStatus;
	} 
	else if (fabs(flatFacTSF) < EPS_64)
	{
	    returnStatus = PGSCSC_W_SPHERE_BODY;
	    PGS_SMF_SetDynamicMsg(returnStatus,
				  "a spherical earth model is being used",
				  FUNCTION_NAME);
	}
	else if (flatFacTSF > 0.01)
	{
	    returnStatus = PGSCSC_W_LARGE_FLATTENING;
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	}
	
	/*  determine the square of eccentricity of the ellipsoid */
	/* Threadsafe Protect: ellipsEccSq, flatFac
                reassign ellipsEccSq's value for next use   */
	
	ellipsEccSqTSF = (2.0 * flatFacTSF) - (flatFacTSF * flatFacTSF);
         PGSg_TSF_CSCECRtoGEOellipsEccSq[masterTSFIndex] = ellipsEccSqTSF;
	
	/* set the oldEarthTag to the current value */
	/* Threadsafe Protect: oldEarthTag,
                reassign oldEarthTag's key value for next use   */
	
	oldEarthTagTSF[49] = '\0';
	strncpy(oldEarthTagTSF,earthEllipsTag,49);
        pthread_setspecific(masterTSF->keyArray
        [PGSd_TSF_KEYCSCECRTOGEOOLDEARTHTAG],(void *) oldEarthTagTSF);

	/* set modelStatus to current value of returnStatus */
	/* Threadsafe Protect: modelStatus
                reassign modelStatus's value for next use   */

	modelStatusTSF = returnStatus;
        PGSg_TSF_CSCECRtoGEOmodelStatus[masterTSFIndex] = modelStatusTSF;

    } /* END setup */
    else if (returnStatus != PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    
    /* Compute radius projected onto X-Y plane */

    rho = sqrt(posECR[0]*posECR[0] + posECR[1]*posECR[1]);  
                                                               
    /*  Compute Longitude */

    if ( rho > 0.0 )
	*longitude = atan2(posECR[1], posECR[0]);     /* Arctangent of Y/X */
    else
	*longitude = 0.0;

    /* normalize the coordinates */
    /* Threadsafe Protect: equatRad_A */

    normRho  = rho / equatRad_ATSF;
    normZ    = posECR[2]/ equatRad_ATSF;
        
    /* if point is within 0.5 Earth radii of the center convergence
       may be poor.  Use Borkowski method (actually quite good all
       over - but perhaps slower */ 

    if ( normRho*normRho + normZ*normZ < 0.80 )
    { 
    /* Threadsafe Protect: equatRad_A, polarRad_C
       Their values don't change in Borkowski.  NO reassignment */

       returnStatus = PGS_CSC_BorkowskiGeo(equatRad_ATSF,polarRad_CTSF,rho,
                       posECR[2],latitude,altitude);
                     
       returnStatus = PGSCSC_W_INVALID_ALTITUDE;
       PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
       return returnStatus;
 
    } 
    
    /* test if the point is inside or outside the ellipsoid */
    /* Threadsafe Protect: ellipsEccSq */

    signn = (normRho*normRho + normZ*normZ / 
             (1.0 - ellipsEccSqTSF)) > 1.0 ? 1.0 : -1.0;

    /* initialize the distance of Q from O and the stored value */

    normRhoFoot  = normRho/sqrt(normRho*normRho + normZ*normZ*(1.0 - ellipsEccSqTSF));
    normRhoFootStore = 0.0;
 
    /* iterate to about 9 significant digits accuracy (~1 mm) */
 
    while (((fabs(normRhoFootStore  - normRhoFoot)) > 1.5e-09) && (ncount < 10))
    {
        normRhoFootStore = normRhoFoot;
        correcterm = normRho - ellipsEccSqTSF * normRhoFoot; 
 
        /* calculate the new length */
        normRhoFoot = correcterm / sqrt(correcterm*correcterm + normZ*normZ*(1.0 - ellipsEccSqTSF));
 
        /* increment ncount */
        ++ncount;
    }
    lengthOQ = ellipsEccSqTSF * normRhoFoot;

    /* issue a warning if the number of iterations ran too high */

    if (ncount == 10)
    {
	returnStatus = PGSCSC_W_TOO_MANY_ITERS;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }
    
    if(normRho < 5.0e-17) normZFoot = SIGN(normZ) * polarRad_CTSF/equatRad_ATSF;
    else   normZFoot = normZ *(normRhoFoot - lengthOQ)/(normRho - lengthOQ);
    /* calculate the latitude */

    if (normRho < 1.0e-17) *latitude = SIGN(normZ) * HALFPI;
    else *latitude = atan2(normZ, normRho - lengthOQ);

    /* calculate the true altitude = normalized altitude * equatorial radius */
    /* Threadsafe Protect: equatRad_A */

    *altitude  = signn * sqrt((normRhoFoot-normRho) * (normRhoFoot-normRho) + 
                 (normZFoot-normZ) * (normZFoot-normZ)) * equatRad_ATSF;

    /* check for an invalid altitude */
    
    if (*altitude < (-0.1 * equatRad_ATSF))
        if (returnStatus != PGSCSC_W_TOO_MANY_ITERS)
	{
	    returnStatus = PGSCSC_W_INVALID_ALTITUDE;
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	}

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
	
	/*  determine the square of eccentricity of the ellipsoid */
	
	ellipsEccSq = (2.0 * flatFac) - (flatFac * flatFac);
	
	/* set the oldEarthTag to the current value */
	
	oldEarthTag[49] = '\0';
	strncpy(oldEarthTag,earthEllipsTag,49);

	/* set modelStatus to current value of returnStatus */

	modelStatus = returnStatus;

    } /* END setup */
    else if (returnStatus != PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    
    /* Compute radius projected onto X-Y plane */

    rho = sqrt(posECR[0]*posECR[0] + posECR[1]*posECR[1]);  
                                                               
    /*  Compute Longitude */

    if ( rho > 0.0 )
	*longitude = atan2(posECR[1], posECR[0]);     /* Arctangent of Y/X */
    else
	*longitude = 0.0;

    /* normalize the coordinates */

    normRho  = rho / equatRad_A;
    normZ    = posECR[2]/ equatRad_A;
        
    /* if point is within 0.5 Earth radii of the center convergence
       may be poor.  Use Borkowski method (actually quite good all
       over - but perhaps slower */ 

    if ( normRho*normRho + normZ*normZ < 0.80 )
    { 
       returnStatus = PGS_CSC_BorkowskiGeo(equatRad_A,polarRad_C,rho,
                       posECR[2],latitude,altitude);
                     
       returnStatus = PGSCSC_W_INVALID_ALTITUDE;
       PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
       return returnStatus;
 
    } 
    
    /* test if the point is inside or outside the ellipsoid */

    signn = (normRho*normRho + normZ*normZ / 
             (1.0 - ellipsEccSq)) > 1.0 ? 1.0 : -1.0;

    /* initialize the distance of Q from O and the stored value */

    normRhoFoot  = normRho/sqrt(normRho*normRho + normZ*normZ*(1.0 - ellipsEccSq));
    normRhoFootStore = 0.0;
 
    /* iterate to about 9 significant digits accuracy (~1 mm) */
 
    while (((fabs(normRhoFootStore  - normRhoFoot)) > 1.5e-09) && (ncount < 10))
    {
        normRhoFootStore = normRhoFoot;
        correcterm = normRho - ellipsEccSq * normRhoFoot; 
 
        /* calculate the new length */
        normRhoFoot = correcterm / sqrt(correcterm*correcterm + normZ*normZ*(1.0 - ellipsEccSq));
 
        /* increment ncount */
        ++ncount;
    }
    lengthOQ = ellipsEccSq * normRhoFoot;

    /* issue a warning if the number of iterations ran too high */

    if (ncount == 10)
    {
	returnStatus = PGSCSC_W_TOO_MANY_ITERS;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }
    
    if (normRho < 5.0e-17) normZFoot = SIGN(normZ) * polarRad_C/equatRad_A;
    else   normZFoot = normZ *(normRhoFoot - lengthOQ)/(normRho - lengthOQ);
    /* calculate the latitude */

    if (normRho < 5.0e-17) *latitude = SIGN(normZ) * HALFPI;
    else *latitude = atan2(normZ, normRho - lengthOQ);

    /* calculate the true altitude = normalized altitude * equatorial radius */

    *altitude  = signn * sqrt((normRhoFoot-normRho) * (normRhoFoot-normRho) + 
                 (normZFoot-normZ) * (normZFoot-normZ)) * equatRad_A;

    /* check for an invalid altitude */
    
    if (*altitude < (-0.1 * equatRad_A))
        if (returnStatus != PGSCSC_W_TOO_MANY_ITERS)
	{
	    returnStatus = PGSCSC_W_INVALID_ALTITUDE;
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	}

#endif
    if (returnStatus == PGS_S_SUCCESS)  
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);

    return returnStatus;
}
