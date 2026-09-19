/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
        PGS_GCT_Proj.c

DESCRIPTION:
	Performs Geo-coordinate transformations for the given projection
	in the forward and inverse directions.
AUTHOR:
        Alward N. Siyyid/ EOSL
        Carol S. W. Tsai / Applied Reseach Corporation
	Abe Taaheri / Emergent Information Tecnologies, Inc.

HISTORY:
        12-Dec-94      	ANS     Initial version
	01-Jan-95      	ANS     Added inverse functions declarations for 
		               	pointer use
			       	Added fortran interface	
	06-Jan-95	ANS	Corected logic for the expression checking input
				UTM zone value for inverse routines
	12-Jan_95	ANS	Implemented code review comments
	08-Feb-95	ANS	Changed prolog to include fortran example
				changed 'CLRK80_flatening' CLRK80_MINOR_AXIS
				changed 'CLRK80_axis' CLRK80_MAJOR_AXIS
				Some other minor changes in the prolog
	15-Feb-95       ANS	put SetStaticMsg for the warning PGSGCT_W_INTP_REGION
	16-Jun-95	ANS	changed user available definitions
	28-Jun-95	ANS	Fixed bug ECSed00968 about the error in fortran
				binding
	11-July-95	ANS	Improved fortran example
				Sorted the allignment problem caused on dec
	21-July-95	ANS	Fixed DR ECSed00700 
        11-July-97      CSWT    Added a new projection type called Integerized
                                Sinusoidal Grid to support MODIS level 3 datasets 
        23-Sep-97       CSWT    Dued to the calling sequence is different between 
                                the gctp package being maintained by the HDF-EOS 
                                developer and gctp package being maintained by the
                                TOOLKIT for projection UNIVERSAL TRANSVERSE MERCATOR
                                to transforms input longitude and latitude to
                                Easting and Northing or vice versa
                                Added code to calculate the ZONE number from the 
                                given input Longitude value and obtain the semi-major
                                axis and semi-minor axis values from by calling the 
                                PGS_GCT_SetGetrMajorrMinor, a function to set or retreive
                                major and mijnor axis values, in order ZONE, semi-major 
                                and semi-minor axis values can be passed into the lower 
                                functions, utmforint() and utminvint() that will be called
                                here instead of being called in the function PGS_GCT_Init()
                                (This changing is for ECSed08976 about GCT tools require 
                                update)
        23-Oct-97       CSWT    Added the include file proj.h and modified the code
                                to match the changing of ISINUS projection from the integer 
                                code 31 to 99 that defined in the header file PGS_GCT.h
        09-July-99      SZ      Updated for the thread-safe functionality
	21-June-00      AT      Added a new projection type called Cylindrical
	                        Equal Area to suuport EASE grid.
        23-Oct-00       AT      Updated for ISINUS projection, so that both 
                                codes 31 and 99 can be used for this projection.

END_FILE_PROLOG
*******************************************************************************/

/* include files */
#include "PGS_GCT.h"
#include <cproj.h>
#include <proj.h>
#include <math.h>
#include <PGS_TSF.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
        Transforms geographical coordinates into cartesian coordinates and
	vice versa for the given projection

NAME:
        PGS_GCT_Proj()

SYNOPSIS:
C:

        #include "PGS_GCT.h"

	PGSt_SMF_status PGS_GCT_Proj(
                        PGSt_integer projId,
                        PGSt_integer directFlag,
                        PGSt_integer nPoints,
                        PGSt_double  longitude[],
                        PGSt_double  latitude[],
                        PGSt_double  mapX[],
                        PGSt_double  mapY[],
                        PGSt_integer zone[]);
FORTRAN:

	include "PGS_GCT.f"
	include "PGS_GCT_12.f"

	integer function pgs_gct_init( projId, directFlag, nPoints, longitude,
				       latitude, mapX, mapY, zone)

        integer			projId
	integer			directFlag
	integer			nPoints
	double precision	longitude(*)
	double precision        latitude(*)
        double precision        mapX(*)
        double precision        mapY(*)
        double precision        zone(*)

DESCRIPTION:
	This tool provides a general inteface to perform geo-coordinate
        transformations in the forward/inverse directions. In general
        the tool requires a projection id, location of input date vectors
        input data vectors and the location of result vectors for the output
        vectors. UTM projection is a special case for which zone value is
        also needed to define a point.	
INPUTS:
        Name            Description            Units            Min          Max
        ----            -----------             -----           ---          ---
        projId          projection code         none            1            #defined
        directFlag      forward/inverse         none        PGSd_GCT_FORWARD PGSd_GCT_INVERSE
	nPoints		num. of points		none		1	     variable
	longitude[]	longitude values	radians		-PI	     +PI
	latitude[]	latitude values		radians 	-PI	     +PI
	mapX[]		x catesian		meters		variable     variable
			coordinate		(see notes)
	mapY[]          y catesian              meters          variable     variable   
                        coordinate              (see notes)
	zone[]		UTM zones (-ve for	none		1	     60
			southern hemisphere)

OUTPUTS:
	see description 

RETURNS:
   	PGS_S_SUCCESS
   	PGSGCT_E_BAD_ZONE		invalid UTM zone
   	PGSGCT_E_BAD_DIRECTION		invalid direction
  	PGSGCT_E_INVD_PROJECTION	projection doesn't exists
  	PGSGCT_E_NO_POINTS		number of points less than one
   	PGSGCT_E_GCTP_ERROR         	error in the GCTP library
   	PGSGCT_E_BAD_LONGITUDE      	bad longitude value (out of range)
   	PGSGCT_E_BAD_LATITUDE		bad latitude value (out of range)
  	PGSGCT_W_INTP_REGION		interrupted region encountered
        PGSGCT_E_INFINITE               Point projects into infinity
        PGSGCT_E_LAT_CONVERGE           Lattitude failed to converge
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
C:
	#include "PGS_GCT.h"

	PGS_SMF_status		retValue;
   	PGSt_double             projParam[15] ;
        PGSt_double      	latitude[4];
        PGSt_double      	longitude[4];
	PGSt_double             mapX[4], mapY[4];
        PGSt_integer       	ProjId = PGSd_UTM;
	PGSt_integer            nPoints = 4;
	PGSt_integer            directFlag, i;
	PGSt_integer            zone[4] = {0, 0, 0, 0};
	PGSt_integer		cucFileId;

	for (i = 0; i<5) i++) 
	{
		longitude[i] = PI/i;
		latitude[i] = PI/4;
	}
	cucFileId = 10999;
        retValue =      PGS_CUC_cons(cucFileId,"CLRK80_MAJOR_AXIS", &ProjParam[0]);
        retValue        PGS_CUC_cons(cucFileId,"CLRK80_MINOR_AXIS", &ProjParam[1]);
	ProjParam[5] = PI/2;
	ProjParam[6] = 3000000 (false easting in meters)
	ProjParam[7] = 75000000 (false northing in meters)

	directFlag = PGSd_GCT_FORWARD;
        retValue =      PGS_GCT_Init (projId, projParam, directFlag);
        retValue =      PGS_GCT_Proj(projId, directflag, nPoints,
				     latitude, longitude, mapX, mapY, zone);

	directflag = PGSd_GCT_INVERSE; (cartesian to geographical)
        retValue =      PGS_GCT_Init (projId, projParam, directFlag);
	retValue =      PGS_GCT_Proj(projId, directflag, nPoints,
                                     latitude, longitude, mapX, mapY, zone);

F:
	IMPLICIT 		NONE
	integer			pgs_gct_proj
	integer			retValue
   	double precision        projParam(4)
        double precision        latitude(4)
        double precision        longitude(4)
	double precision	mapX[4], mapY(4)
        integer			ProjId
	integer			nPoints
	nteger			directFlag, i
	integer			zone(4)

	nPoints = 4
	ProjId = PGSd_UTM
	do 20 i = 1, 5
		longitude[i] = PI/i
		latitude[i] = PI/4
	cucFileId = 10999
        pgs_cuc_cons(cucFileId,"CLRK80_MAJOR_AXIS", ProjParam(1))
        pgs_cuccons(cucFileId,"CLRK80_MINOR_AXIS", ProjParam(2))
	ProjParam(5) = PI/2
C	false easting in meters
	ProjParam(6) = 3000000 
C	false northing in meters
	ProjParam(7) = 75000000 

	directFlag = PGSd_GCT_FORWARD
        pgs_gct_init (projId, projParam, directFlag)
        pgs_gct_proj(projId, directflag, nPoints,
				     latitude, longitude, mapX, mapY, zone)

C	cartesian to geographical
	directflag = PGSd_GCT_INVERSE 
        PGS_GCT_Init (projId, projParam, directFlag)
	PGS_GCT_Proj(projId, directflag, nPoints,
                                     latitude, longitude, mapX, mapY, zone)
NOTES:
	The units of output cartesian coordinates essentially depends on the units
	used for the earth's radii, false easting and northing etc in the parameters
	list. The only requirement is that the units used should be consistent.

	The zones[] parameter is at present only used for UTM transformations.

	All points are processed regardless if there is an error condition for some points.
	If bad point(s) are encountered the routine returns PGSd_GCT_IN_ERROR
	in the output vector. The user can find out the offending input values by
        searching for the PGSd_GCT_IN_BREAK in the output vector. For eg. if the
        third point is in error then:

        Input Vector
        Longitude              1, 2, 3, 4, 5
        latitude               1, 1, 1, 1, 1

        Output Vector
        X		.01, .02, PGSd_GCT_IN_ERROR, .04, .05
        Y		.1, .2. PGSd_GCT_IN_ERROR, .4, .5

	For the inverse equations couple of projections namely Interrupted Goode
	and Interrupted Mollweide sometimes sometimes encounter a point that is
	in an interrupted region. In such cases the tool does not abandon 
	processing but puts a value PGSd_GCT_IN_BREAK in the output vector.
	At theend of processing the tool returns a warning taht a Interrupted region
	was encountered. The user can find out the offending input values by
	searching for the PGSd_GCT_IN_BREAK in the output vector. For eg. if the 
	third point is in the interrupted region:

	Input Vector
	X		1, 2, 3, 4, 5
	Y		1, 1, 1, 1, 1

	Output Vector
	Longitude	.01, .02, PGSd_GCT_IN_BREAK, .04, .05
	latitude	.1, .2. PGSd_GCT_IN_BREAK, .4, .5
	

REQUIREMENTS:
        PGSTK-1500, 1502

GLOBALS:
        NONE

FILES:
        NONE

FUNCTIONS_CALLED:
		PGS_GCT_ValidateProjArgs
                PGS_GCT_CheckLongitude
                PGS_GCT_CheckLatitude
                xxxfor()
                xxxinv()
                PGS_SMF_SetStaticMsg
                PGS_TSF_LockIt
                PGS_TSF_UnlockIt
                PGS_SMF_TestErrorLevel

END_PROLOG:
***************************************************************************/

PGSt_SMF_status PGS_GCT_Proj(	          /* performs projection transforms */
                PGSt_integer projId,	  /* projection id */
                PGSt_integer directFlag,  /* Forward or inverse transforms */
                PGSt_integer nPoints,     /* number of points */
                PGSt_double  longitude[], /* longitude */
                PGSt_double  latitude[],  /* latitude */
                PGSt_double  mapX[],      /* horizontal cartesian coordinate */
                PGSt_double  mapY[],	  /* vertical cartesian coordinate */
                PGSt_integer zone[])      /* UTM zone */

{
   PGSt_SMF_status	retValue = PGS_S_SUCCESS;
   PGSt_SMF_status	intpRetVal = PGS_S_SUCCESS;  /* return value if interrupted 
						      * region encountered
						      */
   PGSt_SMF_status      errorRetVal = PGS_S_SUCCESS;  /* return value if error */

   PGSt_integer loopCount = 0;	/* loop counter	*/

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retLock;            /* lock and unlock return */
        PGSt_SMF_status retTSF;
#endif
   /* The following variables are to be used as intermediate varaible
    * These are required to avoid allignment problems on Cray (uses long double)
    * and dec (uses 64 bit long)
    */
   long		longZone; 
   double	doubleLongi;
   double	doubleLat;
   double	doubleMapX =0.0;
   double	doubleMapY =0.0;
   double	rMajor;
   double	rMinor;
   PGSt_integer Zone = 0;
   double scaleFactor = 0.0;      /* scale factor */
   static PGSt_integer   last_zone = -9999;
   PGSt_boolean getrMajorrMinor = PGSd_GET;

   /* function declaration for function pointer use */
   static PGSt_integer (*forTrans[MAXPROJ + 1])(double lon, double lat, 
                                                   double *x, double *y) = 
		{0, utmfor, stplnfor, alberfor, lamccfor, merfor,
		 psfor, polyfor, eqconfor, tmfor, sterfor,
		 lamazfor, azimfor, gnomfor, orthfor, gvnspfor,
		 sinfor, equifor, millfor, vandgfor, omerfor,
		 robfor, somfor, alconfor, goodfor, molwfor,
		 imolwfor, hamfor, wivfor, wviifor, obleqfor, 
                 isinusfor, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, bceafor, isinusfor}; 

   /* Inverse functions */
   static PGSt_integer (*invTrans[MAXPROJ + 1])(double x, double y, 
                                              double *lon, double *lat) = 
		{0,utminv, stplninv, alberinv, lamccinv, merinv,
		 psinv, polyinv, eqconinv, tminv, sterinv,
		 lamazinv, aziminv, gnominv, orthinv, gvnspinv,
		 sininv, equiinv, millinv, vandginv, omerinv,
		 robinv, sominv, alconinv, goodinv, molwinv,
		 imolwinv, haminv, wivinv, wviiinv, obleqinv, 
                 isinusinv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, bceainv, isinusinv};

   /* validate arguments */

   retValue = PGS_GCT_ValidateProjArgs(projId, directFlag, nPoints);
   if(retValue != PGS_S_SUCCESS)
   {
       return retValue;
   } 

   /* Forward transformations */

   for(loopCount=0; loopCount < nPoints; loopCount++)
   {
      /* transfer the variables to intermediate variables */
 
      if(directFlag == PGSd_GCT_FORWARD)
      {
         doubleLongi = (double) longitude[loopCount];
         doubleLat = (double) latitude[loopCount];
         retValue = PGS_GCT_CheckLongitude(longitude[loopCount]);
         if(retValue != PGS_S_SUCCESS)
         {
            return retValue;
         }

         retValue = PGS_GCT_CheckLatitude(latitude[loopCount]);
         if(retValue != PGS_S_SUCCESS)
         {
            return retValue;
         }

         if(projId == PGSd_UTM)         /* UTM is a special case requiring zone info */
         {
            scaleFactor = PGSd_GCT_UTM_SCALE_FACTOR;
            retValue = PGS_GCT_SetGetrMajorrMinor(getrMajorrMinor, &rMajor, &rMinor); 

            if(retValue != PGS_S_SUCCESS)
            {
               return retValue;
            }
 
            Zone =  (int) calc_utm_zone(doubleLongi * R2D);
 
            if (doubleLat < 0.0)
            {
               Zone = -Zone; 
            }

            if (Zone != last_zone)
            {
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEWARE) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEWARE);
        if (PGS_SMF_TestErrorLevel(retTSF))       
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
               retValue = utmforint(rMajor,rMinor,scaleFactor,Zone);
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
               switch(retValue)
               {
                 case 11:
                   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_ZONE,"utm-forint");
                   return PGSGCT_E_BAD_ZONE;
                 case 93:
                   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INFINITE,"utm-for");
                   return PGSGCT_E_INFINITE;
               }
               last_zone = Zone;
            }
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_GCTLOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
            retValue =
                forTrans[projId](doubleLongi,
                                 doubleLat,
                                 &doubleMapX,
                                 &doubleMapY);
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_GCTLOCK);
#endif
         }
         else                      /* other projections with a general interface */
         {
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_GCTLOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
            retValue =
                forTrans[projId](doubleLongi,
                                 doubleLat,
                                 &doubleMapX,
                                 &doubleMapY);
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_GCTLOCK);
#endif
         } 
         if(retValue != PGSd_GCT_OK)
         {
               doubleMapY = PGSd_GCT_IN_ERROR;
               doubleMapX = PGSd_GCT_IN_ERROR;
               errorRetVal = PGSGCT_E_GCTP_ERROR;
         }
      }
      else if(directFlag == PGSd_GCT_INVERSE)
      {
         doubleMapX = (double) mapX[loopCount];
         doubleMapY = (double) mapY[loopCount];
         if(projId == PGSd_UTM)         /* UTM is a special case requiring zone info */
         {
            longZone = (long) zone[loopCount];
            scaleFactor = PGSd_GCT_UTM_SCALE_FACTOR;
            retValue = PGS_GCT_SetGetrMajorrMinor(getrMajorrMinor, &rMajor, &rMinor);

            if(retValue != PGS_S_SUCCESS)
            {
               return retValue;
            }

            if(zone[loopCount] < PGSd_GCT_UTM_MINIMUM_ZONE ||
               zone[loopCount] == 0 ||
               zone[loopCount] > PGSd_GCT_UTM_MAXIMUM_ZONE)
            {
               (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_ZONE,"PGS_GCT_Proj");
                return PGSGCT_E_BAD_ZONE;
            }

            if(loopCount == 0)
            {
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEWARE) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEWARE);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE; 
        }
#endif
              retValue = utminvint(rMajor,rMinor,scaleFactor,longZone);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
              switch (retValue)
              {
                case 11:
                  (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_ZONE,"utm-invint");
                  return PGSGCT_E_BAD_ZONE;
                case 95:
                  (void) PGS_SMF_SetStaticMsg (PGSGCT_E_LAT_CONVERGE,"UTM-INVERSE");
                  return PGSGCT_E_LAT_CONVERGE;
              }
            }
            else if(loopCount > 0 && zone[loopCount] != zone[loopCount-1])
            {
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEWARE) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEWARE);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE; 
        }
#endif        
              (void) utminvint(rMajor,rMinor,scaleFactor,longZone);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
            }

#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_GCTLOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
            retValue =
                invTrans[projId](doubleMapX,
                                 doubleMapY,
                                 &doubleLongi,
                                 &doubleLat);
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_GCTLOCK);
#endif
         }
         else              /* general transformations */
         {
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_GCTLOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
            retValue =
                invTrans[projId](doubleMapX,
                                 doubleMapY,
                                 &doubleLongi,
                                 &doubleLat);
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_GCTLOCK);
#endif
         } 
         if(retValue != PGSd_GCT_OK)
         {
            if(retValue == IN_BREAK) /* Interrupted region */
            {
               doubleLongi = PGSd_GCT_IN_BREAK;
               doubleLat = PGSd_GCT_IN_BREAK;
	       intpRetVal = PGSGCT_W_INTP_REGION;
            }
            else
            {
               doubleLongi = PGSd_GCT_IN_ERROR;
               doubleLat = PGSd_GCT_IN_ERROR;
               errorRetVal = PGSGCT_E_GCTP_ERROR;
            }
         }
      }
      longitude[loopCount] = (PGSt_double) doubleLongi;
      latitude[loopCount] = (PGSt_double) doubleLat;
      mapX[loopCount] = (PGSt_double) doubleMapX;
      mapY[loopCount] = (PGSt_double) doubleMapY;
      if(directFlag == PGSd_GCT_FORWARD) zone[loopCount] = last_zone;
      else if(directFlag == PGSd_GCT_INVERSE) zone[loopCount] = longZone;
   }

   if(intpRetVal != PGS_S_SUCCESS)
   {
      (void) PGS_SMF_SetStaticMsg (PGSGCT_W_INTP_REGION, "PGS_GCT_Proj");
      return (intpRetVal);
   }
   else if(errorRetVal != PGS_S_SUCCESS)
   {
      (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR, "PGS_GCT_Proj");
      return (errorRetVal);
   }	
   else
   {
      return PGS_S_SUCCESS;
   }
}

/***************************************************************************
BEGIN_PROLOG:

TITLE:
        Checks projection arguments

NAME:
        PGS_GCT_ValidateProjArgs

DESCRIPTION:
        Ensures that the projection arguments are within limits

INPUTS:
        Name            Description            Units            Min          Max
        ----            -----------             -----           ---          ---
        projId          projection code         none            1            #defined
        directFlag      forward/inverse         none            #defined     #defined
        nPoints         num. of points          none            1            variable

OUTPUTS:
        none

RETURNS:
   	PGS_S_SUCCESS
	PGSGCT_E_INVD_PROJECTION	projection doesn't exist
	PGSGCT_E_NO_POINTS		number of points ess than 1
	PGSGCT_E_BAD_DIRECTION		only forward or inverse is allowed


FUNCTIONS_CALLED:

        PGS_SMF_SetMessage

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_GCT_ValidateProjArgs(               /* Validate projection arguments */
                        PGSt_integer projId,    /* projection code */
                        PGSt_integer directFlag, /* flag for forward or inverse */
                        PGSt_integer nPoints)    /* number of points */
{

   if((directFlag != PGSd_GCT_FORWARD) && (directFlag != PGSd_GCT_INVERSE))
   {
      (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_DIRECTION,"PGS_GCT_ValidateProjArgs");
      return PGSGCT_E_BAD_DIRECTION;
   }
   else if(projId < 1 || projId > MAXPROJ)
   {
      (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INVD_PROJECTION,
                                        "PGS_GCT_ValidateProjArgs");
      return PGSGCT_E_INVD_PROJECTION;
   }
   else if(nPoints < 1)
   {
      (void) PGS_SMF_SetStaticMsg (PGSGCT_E_NO_POINTS,
                                        "PGS_GCT_ValidateProjArgs");
      return PGSGCT_E_NO_POINTS;
   }
   return PGS_S_SUCCESS;
}
