/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_GCT_Init.c
 
DESCRIPTION:
	Performs Geo-coordinate transformations initialization for the given
        projection with the given parameters.
	
AUTHOR: 
        Alward N. Siyyid/ EOSL
        Carol S. W. Tsai / Space Applications Corporation 
	Abe Taaheri / Emergent Information Tecnologies, Inc.

HISTORY:
        12-Dec-94      ANS     Initial version
	11-JAN-95      ANS     Code review comments updated  
	08-Feb-95      ANS     Fixed Bug ECSed00624 file27->file83
	08-Feb-95      ANS     Fixed Bug ECSed00621 explaination in prolog
	08-Feb_95      ANS     Fixed Bug ECSed00622
	15-Feb-95      ANS     Updated to check minor radius correctly
	17-Feb-95      ANS     Initialized numfiles to 1
	16-Jun-95      ANS     changed user available definitions of projections
	28-Jun-95      ANS     Fixed bug ECS00967 about an error in fortran 
			       binding
	26-July-95     ANS     changed PGSt_double to double for the local
			       variables and used casting to assign values 
			       of projParam to local variables. This was to
			       solve the problems on cray
        11-Aug-97      CSWT    Added a new projection type called Integerized
                               Sinusoidal Grid to support MODIS level 3 datasets
        10-Sep-97      CSWT    FIXED Bug ECSed06175 about GCT is having minor
                               differences on the 64bit version
        20-Sep-97      CSWT    Created a new function PGS_GCT_SetGetrMajorrMinor
                               to save the semi-major axis and semi-minor axis values
                               for projection UTM(UNIVERSAL TRANSVERSE MERCATOR) in 
                               order those values can be retrieved by the function
                               PGS_GCT_Proj() 
                               Added status message checking for all functions existing 
                               on the freeware directory that were replaced to use the   
                               gctp package being maintained by the HDF-EOS developer.   
                               the return status message in this package is using the   
                               UNIX standard I/O function that is different from the  
                               original package that calls the Toolkit functions to   
                               handle all return status messages (This changing is for 
                               ECSed08976 about GCT tools require update)   

        09-July-99     SZ      Updated for the thread-safe functionality
	21-June-00     AT      Added a new projection type called Cylinderical
	                       Equal Area to suuport EASE grid.
        23-Oct-00      AT      Updated for ISINUS projection, so that both 
                               codes 31 and 99 can be used for this projection.
        21-Feb-03      AT      Modified sections related to Cylinderical
	                       Equal Area because of generalization of BCEA in
                               GCTP.
        24-Jan-14      AT      Modified sections related to Lambert Azimuthal
	                       Equal Area because of generalization of LAMAZ 
                               to supprt ellipsoid (WGS84) for EASE GRID 2.0

END_FILE_PROLOG
*******************************************************************************/

/* include files */

#include "PGS_GCT.h"
#include <cproj.h>
#include <proj.h>
#include <PGS_TSF.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Initialize given projection parameters
  
NAME:  
        PGS_GCT_Init()

SYNOPSIS:
C:
        #include "PGS_GCT.h"

	PGSt_SMF_status
	PGS_GCT_Init(
        	PGSt_integer projId,     
        	PGSt_double projParam[], 
        	PGSt_integer directFlag) 

FORTRAN:
         include "PGS_GCT_12.f"
	 include "PGS_GCT.f" 
         integer function pgs_gct_init( projId, projParam, directFlag) 

         integer      projId
         double precision(30)   projParam
         integer      directFlag
   
DESCRIPTION:
	Nearly all geographical map projections require setting of certain
        parameters which form part of the projection equations. Some parameters
        such as semi-major axis of an ellipsoidal earth are common to all
        projections while some are specific to certain projections only such as
        inclination angle of a satellite (Space Oblique Mercator). This tool
        initializes such parameters for a given projection.

INPUTS:
        Name            Description            Units    	Min          Max
        ----            -----------             -----   	---          ---
        projId		projection code	        none 		1	     #defined
        projParam       projection parms         rad, m	
						if latitude	-90(PI/180)  90(PI/180)
						if longitude 	-PI 	     PI
	directFlag	forward/inverse		none        PGSd_GCT_FORWARD PGSd_GCT_INVERSE
        

OUTPUTS:
	none

RETURNS:   
   	PGS_S_SUCCESS
   	PGSGCT_E_NO_DATA_FILES		datafiles for state plane could not be found
  	PGSGCT_E_GCTP_ERROR		Error has occurred in the GCTP lib
   	PGSGCT_E_BAD_INC_ANGLE		Invalid inclination angle in the SOM projection
   	PGSGCT_E_BAD_RADIUS		Invalid radius
	PGSGCT_E_BAD_MINOR_AXIS		Invalid Minor radius
   	PGSGCT_E_MINOT_GT_MAJOR		Minor radius is greater than major radius
   	PGSGCT_E_BAD_LONGITUDE		Invalid longitude
  	PGSGCT_E_BAD_LATITUDE		Invalid latitude
   	PGSGCT_E_BAD_DIRECTION		Invalid direction
	PGSGCT_E_INVD_SPCS_SPHEROID	Invalid SPCS Spheroid
	PGSGCT_E_INVD_PROJECTION	Invalid Projection
        PGSGCT_E_INVALID_SWITCH         Switch for setting or getting Major and Minor should
                                        be defined
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
	NONE (see example for PGS_GCT_Proj())
             
NOTES:
        This routine simply initializes the parameters required by a particular 
	projection. The user is refferd to the following appendices for further
	details
	
	Projection List 	- 	Appendix[TBD]
	Parameter List and Use	- 	Appendix[TBD]
	Spheroid List		-	Appendix[TBD] (State Plane Projection only)

        Following steps should be taken if a new projection is to be added
	to the projection library:

        Step 1 - archive new code to the projection library
	Step 2 - define projection code for the new projection in proj.h
	Step 3 - increment the value of MAXPROJ by one in proj.h
	step 4 - add calls to the forward and inverse initialization routines at the end
		 of this file.

        Parameters 0 and 1 are reserved for major axis and flatenning respectively
	Parameter 4 is reserved for longitude values only.
	Parameter 5 is reserved for latitude values only.
	Parameter 6 and 7 are reserved for false easting and northing values only.

	IMPORTANT - All blank array elements are set to zero by the user

	Latitude and longitude ranges are as defined in the input section above. The 
	routine checks the longitude value as -PI <= longitude <= PI and latitude as 
	-PI/2 <= latitude <= PI/2. The value of PI is defined as 3.141592653589793238
	which is available to the userPI is defined as 3.141592653589793238
	which is available to the user

REQUIREMENTS:
        PGSTK-1500, 1502 

DETAILS:
        This routine need only be performed once prior to coordinate transformations.
 	However, the routine must be called whenever a new projection is required or
 	if there is a change in the parameters of the in-use projection, for e.g.
 	different central meridian. 

GLOBALS:
        NONE

FILES:
	nad27sp, nad83sp for the state plane projection

FUNCTIONS_CALLED:
                PGS_GCT_CheckRadii
		PGS_GCT_CheckLongitude
		PGS_GCT_CheckLatitude
                PGS_GCT_SetGetrMajorrMinor
		xxxforint() 		the xxx refers to the abbreviated 
					projection name for eg. merfor()
					for Mercator Projection
		xxxinvint()
                PGS_SMF_SetStaticMsg 
                PGS_TSF_LockIt
                PGS_TSF_UnlockIt
                PGS_SMF_TestErrorLevel

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_GCT_Init(			 /* Initializes required projection 		*/
	PGSt_integer projId,	 /* Input  Projection  Id 			*/
	PGSt_double projParam[], /* Input  Input array of projection parameters	*/
	PGSt_integer directFlag) /* Input  Flag to indicate forward or inverse 
			  	  * operation. Must be equal to PGSd_GCT_FORWARD 
			  	  * or PGSd_GCT_INVERSE				
				  */
{
   PGSt_SMF_status   retVal;	 /* function status value			 */
   PGSt_integer spheroid = 0;	 /* code for certain spheroid (State plae only)  */
   PGSt_integer numFiles = 1;    /* num of files remaining in the PC table	 */

   char file27[PGSd_GCT_FILENAME_LENGTH] = ""; /* data file for state plane
					        * projection 1927 standard
					        */

   char file83[PGSd_GCT_FILENAME_LENGTH] = ""; /* data file for state plane
                                                * projection 1983 standard
                                                */
#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;            /* lock and unlock return */
#endif

   long spcs_zone = 0;
   double azimuth = 0;     /* azimuth					*/
   double alf = 0;		/* SOM angle					*/
   double angle = 0;	/* rotation angle				*/
   double lon1 = 0;	/* longitude point in utm scene			*/
   double lon2 = 0;	/* 2nd longitude 				*/
   double lat1 = 0;	/* 1st standard parallel			*/
   double lat2 = 0;	/* 2nd standard parallel			*/
   double centerLong = 0;	/* center longitude				*/
   double centerLat = 0;	/* center latitude				*/
   double h = 0;		/* height above sphere				*/
   double lonOrigin = 0;	/* longitude at origin				*/
   double latOrigin = 0;	/* latitude at origin				*/
   double rMajor = 0;	/* major axis in meters				*/
   double rMinor = 0;	/* minor axis in meters				*/
   double scaleFactor = 0;	/* scale factor					*/
   double falseEasting = 0;/* false easting in meters			*/
   double falseNorthing = 0;/* false northing in meters			*/
   double shapeM = 0;	/* constant used for Oblated Equal Area		*/
   double shapeN = 0;	/* constant used for Oblated Equal Area		*/
   PGSt_integer start = 0;	/* where SOM starts beginning or end		*/
   double time = 0;	/* SOM time					*/
   double radius = 0;	/* radius of sphere				*/
   PGSt_integer path = 0;	/* SOM path number				*/
   PGSt_integer satnum = 0;	/* SOM satellite number				*/
   long mode = 0;	/* which initialization method  to use A or B	*/
   PGSt_boolean setrMajorrMinor = PGSd_SET;
   double sat_ratio = 0;	/* satellite ratio for SOM projection */
   double dzone;                /* number of longitudinal zones in ISG          */
   double djustify;             /* justify flag in ISG projection               */


   /* Initialize forward transformations
   -----------------------------------*/
   /* check for the input radii values of semi major axis and the eccentricity
    */

   if(projId != PGSd_SPCS) /* State plane projection uses its own projection radii */
   {
      retVal = PGS_GCT_CheckRadii(projParam[0], projParam[1]);
      if(retVal != PGS_S_SUCCESS)
      {
         return retVal;
      }
   }
   /* check for the direction flag. Must be forward or inverse */
   
   if(directFlag != PGSd_GCT_FORWARD && directFlag != PGSd_GCT_INVERSE)
   {
      return (PGSGCT_E_BAD_DIRECTION);
   } 

   /* check for the latitude and longitude values for reserved parametes 4 and 5 */

   retVal = PGS_GCT_CheckLongitude(projParam[4]);
   if(retVal != PGS_S_SUCCESS)
   {
      return retVal;
   }

   retVal = PGS_GCT_CheckLatitude(projParam[5]);
   if(retVal != PGS_S_SUCCESS)
   {
      return retVal;
   }

   /* Set common parameters */

   rMajor = (double) projParam[0];
   radius  = (double) projParam[0];
   rMinor = (double) projParam[1];
   /* for Behrmann Cylindrical Equal Area projection 
      radius is 6371228.0 meters and Ltruescale is
      30 degrees */
   /* Because of generalization of BCEA the following 
      few lines are obsolete */
   /*
   if(projId == PGSd_BCEA)
   {
       projParam[5] = PGSd_DEFAULT_BCEA_LTRUESCALE * 3600 * S2R;
       radius  = PGSd_DEFAULT_BCEA_RADIUS;
       rMajor = radius;
       rMinor = radius;
   }
   */  
   falseEasting  = (double) projParam[6];
   falseNorthing = (double) projParam[7];

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEWARE) */   
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEWARE);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
   if (projId == PGSd_UTM)
   {
       /* this is the call to initialize U T M */
	  
       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }

       scaleFactor = PGSd_GCT_UTM_SCALE_FACTOR;

       retVal = PGS_GCT_SetGetrMajorrMinor(setrMajorrMinor, &rMajor, &rMinor);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif

       return PGS_S_SUCCESS;
   }
   else if (projId == PGSd_SPCS)
   {
       /* this is the call to initialize STATE PLANE
	  -------------------------------------------*/
       spheroid = (PGSt_integer) projParam[2];
       spcs_zone = (long) projParam[3];

       if(spheroid == PGSd_CLARK66)
       {
	   retVal = PGS_PC_GetPCSData(PGSd_PC_INPUT_FILE_NAME,PGSd_GCT_NAD27,
				      file27,&numFiles);
       }
       else if (spheroid == PGSd_GRS80_WGS84)
       {
	   retVal = PGS_PC_GetPCSData(PGSd_PC_INPUT_FILE_NAME,PGSd_GCT_NAD83,
				      file83,&numFiles);
       }
       else
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INVD_SPCS_SPHEROID,
                                        "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_INVD_SPCS_SPHEROID;
       } 
 
       if(retVal != PGS_S_SUCCESS)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_NO_DATA_FILES,
                                        "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_NO_DATA_FILES;
       }

       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = stplnforint(spcs_zone,spheroid,file27,file83);
	   if(retVal != PGS_S_SUCCESS)
	   {
	       switch (retVal)
	       {
		 case 21:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_SPCS_ZONE,
						"stpl-forward");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_SPCS_ZONE;
		 case 22:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_SPCS_ZONE,
						"stpl-forward");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_SPCS_ZONE;
		 default:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
						"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_GCTP_ERROR;
	       }
	   }
       }
       else
       {
	   retVal = stplninvint(spcs_zone,spheroid,file27,file83);
	   if(retVal != PGS_S_SUCCESS)
	   {
	       switch (retVal)
	       {
		 case 21:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_SPCS_ZONE,
						"stpl-inverse");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_SPCS_ZONE;
		 case 22:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_SPCS_ZONE,
						"stpl-inverse");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_SPCS_ZONE;
		 default:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
						"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_GCTP_ERROR;
	       }
	   }
       }	
   }
   else if (projId == PGSd_ALBERS)
   {
       /* this is the call to initialize ALBERS CONICAL EQUAL AREA
	  ----------------------------------------------------------*/

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* check standard parallel values */
       retVal = PGS_GCT_CheckLatitude(projParam[2]);
       if(retVal != PGS_S_SUCCESS)
       {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return retVal;
       }

       retVal = PGS_GCT_CheckLatitude(projParam[3]);
       if(retVal != PGS_S_SUCCESS)
       {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return retVal;
       }

       lat1 = (double) projParam[2];
       lat2 = (double) projParam[3];
       latOrigin = (double) projParam[5];
       centerLong = (double) projParam[4];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal =  alberforint(rMajor,rMinor,lat1,lat2,centerLong,latOrigin,
				 falseEasting, falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_STD_PARALLEL, 
					    "alber-forinit");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_STD_PARALLEL;
	   }
       }
       else
       {
	   retVal = alberinvint(rMajor,rMinor,lat1,lat2,centerLong,latOrigin,
				falseEasting, falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_STD_PARALLEL, 
					    "alber-invinit");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_STD_PARALLEL;
	   }
       }
   }
   else if (projId == PGSd_LAMCC)
   {
       /* this is the call to initialize LAMBERT CONFORMAL CONIC
	  --------------------------------------------------------*/

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* check standard parallel values */
       retVal = PGS_GCT_CheckLatitude(projParam[2]);
       if(retVal != PGS_S_SUCCESS)
       {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return retVal;
       }

       retVal = PGS_GCT_CheckLatitude(projParam[3]);
       if(retVal != PGS_S_SUCCESS)
       {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return retVal;
       }

       lat1 = (double) projParam[2];
       lat2 = (double) projParam[3];
       centerLong = (double) projParam[4];
       latOrigin  = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = lamccforint(rMajor,rMinor,lat1,lat2,centerLong,latOrigin,
				falseEasting, falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_PROJECT_FAILED,
					    "lamcc-for");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_PROJECT_FAILED;
	   }
       }
       else
       {
	   retVal = lamccinvint(rMajor,rMinor,lat1,lat2,centerLong,latOrigin,
				falseEasting, falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_STD_PARALLEL, 
					    "lamcc-inv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_STD_PARALLEL;
	   }
       }
   }
   else if (projId == PGSd_MERCAT)
   {

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* this is the call to initialize MERCATOR
	  ----------------------------------------*/
       centerLong  = (double) projParam[4];
       lat1   = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = merforint(rMajor,rMinor,centerLong,lat1,falseEasting,
			      falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_POINTS_ON_POLES,
					    "mer-forward");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_POINTS_ON_POLES;
	   }
       }
       else
       {
	   retVal =  merinvint(rMajor,rMinor,centerLong,lat1,falseEasting,
			       falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_PS)
   {
       /* this is the call to initialize POLAR STEREOGRAPHIC
	  ----------------------------------------------------*/

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       centerLong = (double) projParam[4];
       lat1  = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = psforint(rMajor,rMinor,centerLong,lat1,falseEasting,
			     falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = psinvint(rMajor,rMinor,centerLong,lat1,falseEasting,
			     falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_POLYC)
   {

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* this is the call to initialize POLYCONIC
	  -----------------------------------------*/
       centerLong  = (double) projParam[4];
       latOrigin   = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = polyforint(rMajor,rMinor,centerLong,latOrigin,falseEasting,
			       falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = polyinvint(rMajor,rMinor,centerLong,latOrigin,falseEasting,
			       falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INVERSE_FAILED,
					    "poly-forinit");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_INVERSE_FAILED;
	   }
       }
   }
   else if (projId == PGSd_EQUIDC)
   {
       /* this is the call to initialize EQUIDISTANT CONIC
	  -------------------------------------------------*/

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* check standard parallel values */
       retVal = PGS_GCT_CheckLatitude(projParam[2]);
       if(retVal != PGS_S_SUCCESS)
       {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return retVal;
       }

       lat1 = (double) projParam[2];
       lat2 = (double) projParam[3];
       centerLong  = (double) projParam[4];
       latOrigin   = (double) projParam[5];
       if (projParam[8] == 0)
       {
	   mode = 0;
	   /* check second standard parallel */

	   retVal = PGS_GCT_CheckLatitude(projParam[3]);
	   if(retVal != PGS_S_SUCCESS)
	   {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return retVal;
	   }
       }
       else
       {
	   mode = 1;
       }
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = eqconforint(rMajor,rMinor,lat1,lat2,centerLong,latOrigin,
				falseEasting,falseNorthing,mode);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_STD_PARALLEL_OPP, 
					    "eqcon_for");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return(PGSGCT_E_STD_PARALLEL_OPP);
	   }
       }
       else
       {
	   retVal = eqconinvint(rMajor,rMinor,lat1,lat2,centerLong,latOrigin,
				falseEasting,falseNorthing,mode);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_STD_PARALLEL_OPP, 
					    "eqcon_inv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return(PGSGCT_E_STD_PARALLEL_OPP);
	   }
       }	
   }
   else if (projId == PGSd_TM)
   {

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* this is the call to initialize TRANSVERSE MECTAR
	  -------------------------------------------------*/
       scaleFactor = (double) projParam[2];
       centerLong  = (double) projParam[4];
       latOrigin   = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = tmforint(rMajor,rMinor,scaleFactor,centerLong,latOrigin,
			     falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INFINITE,
					    "tm-for");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_INFINITE;
	   }
       }
       else
       {
	   retVal = tminvint(rMajor,rMinor,scaleFactor,centerLong,latOrigin,
			     falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_LAT_CONVERGE,
					    "TM-INVERSE");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_LAT_CONVERGE;
	   }
       }
   }
   else if (projId == PGSd_STEREO)
   {
       /* this is the call to initialize STEREOGRAPHIC
	  ---------------------------------------------*/
       centerLong  = (double) projParam[4];
       centerLat   = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = sterforint(radius,centerLong,centerLat,falseEasting,
			       falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INFINITE,
					    "ster-for");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_INFINITE;
	   }
       }
       else
       {
	   retVal = sterinvint(radius,centerLong,centerLat,falseEasting,
			       falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_LAMAZ)
   {
       /* this is the call to initialize LAMBERT AZIMUTHAL
	  -------------------------------------------------*/
       centerLong = (double) projParam[4];
       centerLat  = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = lamazforint(rMajor,rMinor,centerLong, centerLat,falseEasting,
				falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_POINT_PROJECT,
					    "lamaz-forward");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_POINT_PROJECT;
	   }
       }
       else
       {
	   retVal = lamazinvint(rMajor,rMinor,centerLong, centerLat,falseEasting,
				falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR, 
					    "lamaz-inverse");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_INPUT_DATA_ERROR;
	   }
       }
   }
   else if (projId == PGSd_AZMEQD)
   {
       /* this is the call to initialize AZIMUTHAL EQUIDISTANT
	  -----------------------------------------------------*/
       centerLong  = (double) projParam[4];
       centerLat   = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = azimforint(radius,centerLong,centerLat,falseEasting,
			       falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_POINT_PROJECT, 
					    "azim-for");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_POINT_PROJECT;
	   }
       }
       else
       {
	   retVal = aziminvint(radius,centerLong,centerLat,falseEasting,
			       falseNorthing);
	   if(retVal != PGSd_GCT_OK) 
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR, 
					    "azim-inv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return(PGSGCT_E_INPUT_DATA_ERROR);
	   }
       }
   }
   else if (projId == PGSd_GNOMON)
   {
       /* this is the call to initialize GNOMONIC
	  ----------------------------------------*/
       centerLong  = (double) projParam[4];
       centerLat   = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = gnomforint(radius,centerLong,centerLat,falseEasting,
			       falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INFINITE, 
					    "gnomfor-conv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_INFINITE;
	   }
       }
       else
       {
	   (void) gnominvint(radius,centerLong,centerLat,falseEasting,
			     falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_ORTHO)
   {
       /* this is the call to initalize ORTHOGRAPHIC
	  -------------------------------------------*/
       centerLong  = (double) projParam[4];
       centerLat   = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = orthforint(radius,centerLong,centerLat,falseEasting,
			       falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_PROJECT_FAILED,"orth-for");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_PROJECT_FAILED;
	   }
       }
       else
       {
	   retVal = orthinvint(radius,centerLong,centerLat,falseEasting,
			       falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR,
					    "orth-inv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_INPUT_DATA_ERROR;
	   }
       }
   }
   else if (projId == PGSd_GVNSP)
   {
       /* this is the call to initalize GENERAL VERTICAL NEAR-SIDE PERSPECTIVE
	  ----------------------------------------------------------------------*/
       centerLong  = (double) projParam[4];
       centerLat   = (double) projParam[5];
       h = (double) projParam[2];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = gvnspforint(radius,h,centerLong,centerLat,falseEasting,
				falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_PROJECT_FAILED,
					    "gvnsp-for");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_PROJECT_FAILED;
	   }
       }
       else
       {
	   retVal = gvnspinvint(radius,h,centerLong,centerLat,falseEasting,
				falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR,
					    "gvnsp_inv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_INPUT_DATA_ERROR;
	   }
       }
   }
   else if (projId == PGSd_SNSOID)
   {
       /* this is the call to initialize SINUSOIDAL
	  -------------------------------------------*/
       centerLong = (double) projParam[4];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = sinforint(rMajor,rMinor, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = sininvint(rMajor,rMinor, centerLong,falseEasting,falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR,
					    "sinusoidal-inverse");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_INPUT_DATA_ERROR;
	   }
       }
   }
   else if (projId == PGSd_EQRECT)
   {
       /* this is the call to initialize EQUIRECTANGULAR
	  -----------------------------------------------*/
       centerLong  = (double) projParam[4];
       lat1   = (double) projParam[5];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   (void) equiforint(radius,centerLong,lat1,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }

       }
       else
       {
	   retVal = equiinvint(radius,centerLong,lat1,falseEasting,falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR,
					    "equi-inv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return(PGSGCT_E_INPUT_DATA_ERROR);
	   }
       }
   }
   else if (projId == PGSd_MILLER)
   {
       /* this is the call to initialize MILLER CYLINDRICAL
	  --------------------------------------------------*/
       centerLong  = (double) projParam[4];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = millforint(radius, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = millinvint(radius, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_VGRINT)
   {
       /* this is the call to initialize VAN DER GRINTEN
	  -----------------------------------------------*/
       centerLong  = (double) projParam[4];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = vandgforint(radius, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = vandginvint(radius, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_HOM)
   {

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* this is the call to initialize HOTINE OBLIQUE MERCATOR
	*/
       scaleFactor = (double) projParam[2];
       latOrigin = (double) projParam[5];
       if (projParam[12] != 0)
       {
	   mode = 1;
	   azimuth = (double) projParam[3];
	   lonOrigin = (double) projParam[4];
       }
       else
       {
	   mode = 0;
	   /* check latitude and longitude of these two points */
	   retVal = PGS_GCT_CheckLongitude(projParam[8]);
	   if(retVal != PGS_S_SUCCESS)
	   {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return retVal;
	   }
	   retVal = PGS_GCT_CheckLongitude(projParam[10]);
	   if(retVal != PGS_S_SUCCESS)
	   {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return retVal;
	   }
	   retVal = PGS_GCT_CheckLatitude(projParam[9]);
	   if(retVal != PGS_S_SUCCESS)
	   {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return retVal;
	   }
	   retVal = PGS_GCT_CheckLatitude(projParam[11]);
	   if(retVal != PGS_S_SUCCESS)
	   {
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return retVal;
	   }
	   lon1 = (double) projParam[8];
	   lat1 = (double) projParam[9];
	   lon2 = (double) projParam[10];
	   lat2 = (double) projParam[11];
       }
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = omerforint(rMajor,rMinor,scaleFactor,azimuth,lonOrigin,
			       latOrigin,falseEasting, falseNorthing,lon1,lat1,
			       lon2,lat2,mode);
	   if (retVal != PGSd_GCT_OK)
	   {
	       switch (retVal)
	       {
		 case 201:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR,
						"omer-init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_INPUT_DATA_ERROR;
		 case 202:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR,
						"omer-init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_INPUT_DATA_ERROR;
		 case 205:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INFINITE,
						"omer-for");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_INFINITE;
		 default:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
						"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_GCTP_ERROR;
	       }
	   }
       }
       else
       {
	   retVal = omerinvint(rMajor,rMinor,scaleFactor,azimuth,lonOrigin,
			       latOrigin,falseEasting, falseNorthing,lon1,lat1,
			       lon2,lat2,mode);
	   if (retVal != PGSd_GCT_OK)
	   {
	       switch (retVal)
	       {
		 case 201:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR,
						"omer-init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_INPUT_DATA_ERROR;
		 case 202:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR,
						"omer-init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_INPUT_DATA_ERROR;
		 default:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
						"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_GCTP_ERROR;
	       }
	   }
       }
   }
   else if (projId == PGSd_SOM)
   {

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* this is the call to initialize SOM
	  -----------------------------------*/
       path = (PGSt_integer) projParam[3];
       satnum = (PGSt_integer) projParam[2];
       if (projParam[12] == (PGSt_double) 0)
       {
	   mode = (PGSt_integer)1;
	   /* check that the the inclination angle is between 0 and PI radians */

	   if(projParam[3] < 0 || projParam[3] > PI)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_INC_ANGLE,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_BAD_INC_ANGLE;
	   }
	  	  
	   alf = (double) projParam[3];
	   lon1 = (double) projParam[4];
	   time = (double) projParam[8];
	   start = (PGSt_integer) projParam[10];
	   sat_ratio = (double) projParam[11];
       }
       else
       {
	   mode = (long) 0;
	   sat_ratio = (double) 0.5201613;
       }
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = somforint(rMajor,rMinor,satnum,path,alf,lon1,falseEasting,
			      falseNorthing,time,start,mode, sat_ratio);
	   if(retVal != PGSd_GCT_OK) 
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_ITER_SOM,
					    "som-forward");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_ITER_SOM;
	   }
       }
       else
       {
	   retVal = sominvint(rMajor,rMinor,satnum,path,alf,lon1,falseEasting,
			      falseNorthing,time,mode, sat_ratio);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_ITER_SOM,
					    "som-inverse");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_ITER_SOM;
	   }
       }
   }
   else if (projId == PGSd_HAMMER)
   {
       /* this is the call to initialize HAMMER
	  --------------------------------------*/
       centerLong  = (double) projParam[4];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = hamforint(radius,centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = haminvint(radius,centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_ROBIN)
   {
       /* this is the call to initialize ROBINSON
	  ----------------------------------------*/
       centerLong  = (double) projParam[4];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = robforint(radius,centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = robinvint(radius,centerLong,falseEasting,falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_ITER_EXCEEDED,
					    "robinv-conv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_ITER_EXCEEDED;
	   }
       }
   }
   else if (projId == PGSd_GOOD)
   {
       /* this is the call to initialize GOODE'S HOMOLOSINE
	  ---------------------------------------------------*/
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = goodforint(radius);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_ITER_FAILED, 
					    "goode-forward");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_ITER_FAILED;
	   }
       }
       else
       {
	   retVal = goodinvint(radius);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INPUT_DATA_ERROR, 
					    "good-inv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_INPUT_DATA_ERROR;
	   }
       }
   }
   else if (projId == PGSd_MOLL)
   {
       /* this is the call to initialize MOLLWEIDE
	  ------------------------------------------*/
       centerLong = (double) projParam[4];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = molwforint(radius, centerLong,falseEasting,falseNorthing);
	   if(retVal != PGSd_GCT_OK) 
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_ITER_FAILED,
					    "Mollweide-forward");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_ITER_FAILED;
	   }
       }
       else
       {
	   retVal = molwinvint(radius, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_IMOLL)
   {
       /* this is the call to initialize INTERRUPTED MOLLWEIDE
	  -----------------------------------------------------*/
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = imolwforint(radius);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_ITER_FAILED, 
					    "IntMoll-forward");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_ITER_FAILED;
	   }
       }
       else
       {
	   retVal = imolwinvint(radius);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_ALASKA)
   {

       if(rMinor == 0)
       {
	   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,
					"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	   return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* this is the call to initialize ALASKA CONFORMAL
	  ------------------------------------------------*/
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   (void) alconforint(rMajor,rMinor,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = alconinvint(rMajor,rMinor,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       switch (retVal)
	       {
		 case 235:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_ITER_EXCEEDED,
						"alcon-inv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_ITER_EXCEEDED;
		 case 236:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_ITER_EXCEEDED,
						"alcon-inv");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_ITER_EXCEEDED;
		 default:
		   (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
						"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
		   return PGSGCT_E_GCTP_ERROR;
	       }
	   }
       }
   }
   else if (projId == PGSd_WAGIV)
   {
       /* this is the call to initialize WAGNER IV
	  -----------------------------------------*/
       centerLong = (double) projParam[4];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = wivforint(radius, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_ITER_FAILED,
					    "wagneriv-forward");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_ITER_FAILED;
	   }
       }
       else
       {
	   retVal = wivinvint(radius, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_WAGVII)
   {
       /* this is the call to initialize WAGNER VII
	  ------------------------------------------*/
       centerLong = (double) projParam[4];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = wviiforint(radius, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = wviiinvint(radius, centerLong,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_OBEQA)
   {
       /* this is the call to initialize OBLATED EQUAL AREA
	  ---------------------------------------------------*/
       centerLong = (double) projParam[4];
       centerLat  = (double) projParam[5];
       shapeM = (double) projParam[2];
       shapeN = (double) projParam[3];
       angle = (double) projParam[8];
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = obleqforint(radius,centerLong,centerLat,shapeM, shapeN,
				angle,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = obleqinvint(radius,centerLong,centerLat,shapeM, shapeN,
				angle,falseEasting,falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if (projId == PGSd_BCEA)
   {
     if(rMinor == 0)
       {
	 (void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
	 PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	 return PGSGCT_E_BAD_MINOR_AXIS;
       }
       /* this is the call to initialize Behemann Cylindrical Equal Area
	  --------------------------------------------------------------*/
       centerLong  = (double) projParam[4];
       centerLat   = (double) projParam[5]; /* LTrueScale is 30 degrees 
					       for BCEA But user can use any
					       value for this since 
					       Cylinderical Equal Area 
					       projection has been 
					       generalized */
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = bceaforint(rMajor,rMinor,centerLong,centerLat,falseEasting,
			       falseNorthing);
	   if(retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = bceainvint(rMajor,rMinor,centerLong,centerLat,falseEasting,
			       falseNorthing);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else if ((projId == PGSd_ISINUS) || (projId == PGSd_ISINUS1))
   {
       /* this is the call to initialize INTEGERIZED SINUSOIDAL GRID
	  ------------------------------------------------------------*/
 
       centerLong = projParam[4];
       dzone = projParam[8];
       djustify = projParam[10];
 
       if(directFlag == PGSd_GCT_FORWARD)
       {
	   retVal = isinusforinit(radius, centerLong, falseEasting, falseNorthing,
				  dzone, djustify);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
       else
       {
	   retVal = isinusinvinit(radius, centerLong, falseEasting, falseNorthing,
				  dzone, djustify);
	   if (retVal != PGSd_GCT_OK)
	   {
	       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_GCTP_ERROR,
					    "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
	       return PGSGCT_E_GCTP_ERROR;
	   }
       }
   }
   else
   {
       (void) PGS_SMF_SetStaticMsg (PGSGCT_E_INVD_PROJECTION, "PGS_GCT_Init");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
       return PGSGCT_E_INVD_PROJECTION;
   }

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEWARE);
#endif
   return PGS_S_SUCCESS;
}

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Checks the values of given semi-major axis and semi-minor axis
  
NAME:  
        PGS_GCT_CheckRadii()

DESCRIPTION:
 	Earth's semi-major and semi-minor axes radii should be greater than zero
   	to be meaningful. Also minor-axis is not allowed to be greater than major
	axis. This functions checks the given values in the parameters array to
 	ensure that the values fall within legal range.

INPUTS:
        Name            Description            Units    	Min          Max
        ----            -----------             -----   	---          ---
        semiMajor	semi major radius	meters 		0   	     N/A
						(see notes)	(see notes)
	semiMinor	semi-minor radius	meters		0	     N/A

OUTPUTS:
	none

RETURNS:   
   PGS_S_SUCCESS
   PGSGCT_E_BAD_RADIUS		bad semi major radius
   PGSGCT_E_BAD_MINOR_AXIS	bad semi minor radius
   PGSGCT_E_MINOR_GT_MAJOR      semi minor axis greater then semi major axis
             
NOTES:
	The semi major axis value can be used to "scale" the output. For e.g.
	the output for Goode projection using radius = 6370997.000000 meters is

	LONG	LAT	 X		  Y
	-PI  -87(PI/180) -18086470.93146  -8575012.47001

	The output using a unit radius value is 

	LONG    LAT      X               Y
	-PI  -87(PI/180) -2.83888        -1.34595

	Note that -2.83888 * 6370997 = 18086496(Some precision is inevitably lost
        by scaling down)

GLOBALS:
        NONE

FILES:
	NONE

FUNCTIONS_CALLED:

	PGS_SMF_SetStaticMsg()
END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_GCT_CheckRadii(                /* Checks major axis and flattening values */
    PGSt_double majorAxis,     /* Input  Major axis                       */
    PGSt_double minorAxis)	   /* Input  flattening 		      */
{
    if(majorAxis <= 0)
    {
	(void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_RADIUS,"PGS_GCT_CheckRadii");
	return PGSGCT_E_BAD_RADIUS;
    }

/* some projections does not need semi minor axis therefor 0 is allowed */

    if(minorAxis < 0)
    {
	(void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_MINOR_AXIS,"PGS_GCT_CheckRadii");
	return PGSGCT_E_BAD_MINOR_AXIS;
    }
	
    if(minorAxis > majorAxis)
    {
	(void) PGS_SMF_SetStaticMsg (PGSGCT_E_MINOR_GT_MAJOR,"PGS_GCT_CheckRadii");
	return PGSGCT_E_MINOR_GT_MAJOR;
    }

    return PGS_S_SUCCESS;
}
/***************************************************************************
BEGIN_PROLOG:

TITLE: Saves or retrieves the semi-major axis and semi-minor axis values based 
       on the input flag is PGSd_SET or PGSd_GET   
  
NAME:  
        PGS_GCT_SetGetrMajorrMinor()

DESCRIPTION: Saves the given semi-major axis and semi-minor axis values or retrieves
             the semi-major axis and semi-minor axis values. 

INPUTS:
        Name            Description            Units    	Min          Max
        ----            -----------             -----   	---          ---
        SWITCH          flag of PGSd_SET or
                        PGSd_GET
        semiMajor	semi major radius	meters 		0   	     N/A
						(see notes)	(see notes)
	semiMinor	semi-minor radius	meters		0	     N/A

OUTPUTS:
        Name            Description            Units            Min          Max
        ----            -----------             -----           ---          ---
        semiMajor       semi major radius       meters          0            N/A
                                                (see notes)     (see notes)
        semiMinor       semi-minor radius       meters          0            N/A

RETURNS:   
   PGS_S_SUCCESS
   PGSGCT_E_NO_SWITCH Switch for setting or getting Major and Minor should be defined
   PGSTSF_E_GENERAL_FAILURE      problem in the thread-safe code
             
NOTES:
        NONE

GLOBALS:
        PGSg_TSF_GCTrMajor
        PGSg_TSF_GCTrMinor
        PGSg_TSF_GCTset

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()
        PGS_SMF_TestErrorLevel()
        PGS_TSF_GetTSFMaster()
        PGS_TSF_GetMasterIndex()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_GCT_SetGetrMajorrMinor(   /* Saves or retrieves the semi-major axis and semi-minor 
                                 axis values               
                               */
    PGSt_boolean SWITCH,      /* Input flag */
    double *majorAxis,   /* Input  Major axis */  
    double *minorAxis)   /* Input Minor axis */	  
{
    
#ifdef _PGS_THREADSAFE
    /* Create non-static variables and get globals for the thread-safe version */
    double rMajor;
    double rMinor;
    int set;
    int masterTSFIndex;
    extern double PGSg_TSF_GCTrMajor[];
    extern double PGSg_TSF_GCTrMinor[];
    extern int PGSg_TSF_GCTset[];

    /* Set up global index for the thread-safe */
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    /* Get data from globals counterpart */
    rMajor = PGSg_TSF_GCTrMajor[masterTSFIndex];
    rMinor = PGSg_TSF_GCTrMinor[masterTSFIndex];
    set = PGSg_TSF_GCTset[masterTSFIndex];
#else
    static double rMajor;
    static double rMinor;
    static int set = PGS_FALSE; 
#endif

    if(SWITCH == PGSd_SET) 
    {
	rMajor = *majorAxis;
	rMinor = *minorAxis;
        set = PGS_TRUE;

#ifdef _PGS_THREADSAFE
        /* Reset globals */
        PGSg_TSF_GCTrMajor[masterTSFIndex] = rMajor;
        PGSg_TSF_GCTrMinor[masterTSFIndex] = rMinor;
        PGSg_TSF_GCTset[masterTSFIndex] = set;
#endif
    }
    else if(SWITCH == PGSd_GET && set == PGS_TRUE)
    {
	*majorAxis = rMajor;
	*minorAxis = rMinor;
    }
    else
    {    
	PGS_SMF_SetUnknownMsg(PGSGCT_E_INVALID_SWITCH, "PGS_GCT_SetGetrMajorrMinor");
	return PGSGCT_E_INVALID_SWITCH;
    }

    return PGS_S_SUCCESS;

}
/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Checks the value of a given longitude
  
NAME:  
        PGS_GCT_CheckLongitude

DESCRIPTION:
	Unlike latitudes, longitudes have no natural reference from which to count. 
	The reference used here uses the meridian passing through the centre of the 
	transit instrument at the observatory of Greenwich as the initial meridian 
	for longitude. East of Greenwich meridain is considered positive while west
	of Greenwich is negative. Therefore a value of longitude within -180 to +180 
	degrees and a given latitude should be able to define any point on the earth's
	surface.

INPUTS:
        Name            Description            Units    	Min          Max
        ----            -----------             -----   	---          ---
	longitude	geographical coordinate radians		-PI	     +PI

OUTPUTS:
	none

RETURNS:   
   	PGS_S_SUCCESS
   	PGSGCT_E_BAD_LONGITUDE  bad longitude value
             

FUNCTIONS_CALLED:
	
	PGS_SMF_SetMessage

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_GCT_CheckLongitude(                    /* Checks longitude values 		*/
    PGSt_double longitude)        /* Input  longitude                       */
{
    if(longitude < -PI || longitude > PI)
    {
	(void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_LONGITUDE,"PGS_GCT_CheckLongitude");
	return PGSGCT_E_BAD_LONGITUDE;
    }
	
    return PGS_S_SUCCESS;
}
/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Checks the value of a given latitude
  
NAME:  
        PGS_GCT_CheckLatitude

SYNOPSIS:
C:
        #include "PGS_GCT.h"

	PGSt_SMF_status
	PGS_GCT_CheckLatitude(		 
		PGSt_double latitude) 
   
DESCRIPTION:
	For the purpose of this tool equator is taken as the refernece point 
	to define latitude starting point. According to this reference, 
	equator is latutude of zero degrees, Northpole is 90 degrees latitude
	and the south pole is -90 degrees latitude.

INPUTS:
        Name            Description            Units    	Min          Max
        ----            -----------             -----   	---          ---
	latitude	geographical coordinate radians		-PI/2	     +PI/2

OUTPUTS:
	none

RETURNS:   
   	PGS_S_SUCCESS
   	PGSGCT_E_BAD_LATITUDE  bad latitude value
             

FUNCTIONS_CALLED:
	
	PGS_SMF_SetMessage

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_GCT_CheckLatitude(                    /* Checks latitude values 			*/
    PGSt_double latitude)        /* Input  latitude                         */
{
    if(latitude < -HALF_PI || latitude > HALF_PI)
    {
	(void) PGS_SMF_SetStaticMsg (PGSGCT_E_BAD_LATITUDE,"PGS_GCT_CheckLatitude");
	return PGSGCT_E_BAD_LATITUDE;
    }
	
    return PGS_S_SUCCESS;
}
