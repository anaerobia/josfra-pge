/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_Earthpt_FOV.c

DESCRIPTION:
   This file contains the function PGS_CSC_Earthpt_FOV().
   This function, for each time value, using the FOV description returns a flag
   or flags indicating if the Earth point of given latitude, longitude and
   altitude is in the FOV and also returns a unit vector to that point from 
   the SC in SC coordinates 
   
AUTHOR:
   Anubha Singhal     / Applied Research Corporation
   Peter Noerdlinger  / Space Applications Corporation
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   25-Oct-1994 PDN  Designed				
   26-Oct-1994 AS   Coded
   02-Nov-1994 AS   Added check to test if the point is on the other side of the
                    earth and therefore not in the field of view
   08-Nov-1994 AS   Added call to PGS_CSC_FOVconicalHull
   19-Dec-1994 AS   Updated to allow moving FOV's and fixed up prolog
   21-Dec-1994 AS   Made changes suggested in code inspection
   27-Jan-1995 AS   Changing error handling and prolog for min/max values for 
                    altitude
   25-May-1995 PDN  Fixed error handling  for case of bad inFOVvector
   06-Jul-1995 GTSK Fix code to normalize returned spacecraft to Earth point 
                    vectors even when the specified Earth point is on the "far
		    side" of the Earth.  Change PGSCSC_W_BELOW_SURFACE to
		    PGSCSC_E_BELOW_SURFACE (i.e. changed status level from W to
		    E).
   12-Jul-1995 PDN  Fixed examples in prolog
   19-Jul-1995 PDN  Fixed final setting of message for case 
                    of some bad offsets
   19-Jun-1996 PDN  Trapped new error return "PGSCSC_E_BAD_LAT" from GEOtoECR
   22-Oct-1996 PDN  Deleted search for "PGSCSC_W_DATA_FILE_MISSING" from GEOtoECR
   25-Aug-1997 PDN  Deleted unneeded return status

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:
   Determine if Earth Point is in Field of View 

NAME:   
   PGS_CSC_Earthpt_FOV()

SYNOPSIS:
C:
   #include <PGS_TD.h>
   #include <PGS_CSC.h>
   #include <PGS_EPH.h>
   #include <PGS_MEM.h>

   PGSt_SMF_status
   PGS_CSC_Earthpt_FOV(  
              PGSt_integer numValues,
              char         asciiUTC[28],
              PGSt_double  offsets[],
              PGSt_tag     spacecraftTag,
              char         *earthEllipsTag, 
              PGSt_double  latitude,
              PGSt_double  longitude,
              PGSt_double  altitude,
              PGSt_integer numFOVperimVec,
              PGSt_double  inFOVvector[][3],
              void         *perimFOV_vectors,
              PGSt_boolean inFOVflag[],
              PGSt_double  sctoEarthptVec[][3])
	      
FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_CSC_4.f'
      include 'PGS_EPH_5.f'
      include 'PGS_MEM_7.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'

      integer function
     >pgs_csc_earthpt_fov(numvalues,asciiutc,offsets,spacecrafttag,
     >                    earthellipstag,latitude, longitude,altitude,
     >                    numfovperimvec,infovvector,perimfov_vectors,
     >   		  infovflag,sctoearthptvec)
                         
      integer          numvalues
      character*27     asciiutc
      double precision offsets(*)
      integer          spacecrafttag
      character*20     earthellipstag
      double precision latitude
      double precision longitude
      double precision altitude
      integer          numfovperimvec 
      double precision infovvector(3,*)
      double precision perimfov_vectors(3,*,*)
      integer          infovflag(*)
      double precision sctoearthptvec(3,*,*)
    
DESCRIPTION:
   For each time value, the tool, using the FOV description, returns a flag or
   flags indicating if the Earth point of given latitude, longitude and 
   altitude is in the FOV, and the vector to that point from the SC in SC 
   coordinates.

INPUTS:
   Name             Description        Units    Min                 Max
   ----             -----------        -----    ---                 --- 
   numValues        number of time     N/A      0                   any
                    gridpoints

   asciiUTC         UTC start time     N/A         ** see NOTES **

   offsets          array of time      seconds  Max and Min such that
                    offsets                     asciiUTC+offset is
                                                between asciiUTC Min
                                                and Max values

   spacecraftTag    unique spacecraft  N/A      N/A                 N/A
                    identifier       

   earthEllipsTag   earth model used   N/A      N/A                 N/A

   latitude         latitude of Earth  radians  -PI/2               +PI/2
                    point

   longitude        longitude of Earth radians  -2*PI               +2*PI
                    point

   altitude         altitude of Earth  meters   -50000              100000
                    point

   numFOVperimVec   number of vectors  N/A      3                   any
                    defining FOV
                    perimeter

   inFOVvector      vector in FOV -    N/A      N/A                 N/A
                    preferably near the 
		    center in SC 
		    coordinates

   perimFOV_vectors vectors in SC      N/A      N/A                 N/A
                    coords defining
                    FOV's; MUST be
                    sequential around
                    FOV; the middle
		    dimension must be
		    exactly the same
		    as numFOVperimVec
		    because of the way
		    the array dimensioning
		    works in the function
                                                   
OUTPUTS:
   Name           Description                     Units       Min       Max
   ----           -----------                     ----        ---       ---
   inFOVflag      PGS_TRUE if earth point is in   N/A         N/A       N/A
                  FOV - see notes

   sctoEarthptVec vector to earth point in SC     meters      -1        1
                  coords - returned normalized

RETURNS:
   PGS_S_SUCCESS		 success
   PGSCSC_E_SC_TAG_UNKNOWN	 invalid Spacecraft tag
   PGSCSC_W_ERROR_IN_EARTHPTFOV  an error occurred in determining if point was 
                                 in the FOV for at least one case
   PGSCSC_E_BELOW_SURFACE 	 location is below surface
   PGSCSC_W_BAD_TRANSFORM_VALUE  one or more values in transformation
                                 could not be determined
   PGSTD_E_BAD_INITIAL_TIME      initial time is incorrect
   PGSTD_E_NO_LEAP_SECS          no leap seconds correction available for
                                 input time
   PGSTD_W_PRED_LEAPS            a predicted leap second value was used
                                 for at least one of the input offset times
   PGSCSC_W_DEFAULT_EARTH_MODEL  the default earth model is used because
                                 a correct one was not specified   
   PGSCSC_W_SPHERICAL_BODY       using a spherical earth model
   PGSCSC_W_PROLATE_BODY         using a prolate earth model
   PGSCSC_W_LARGE_FLATTENING     issued if flattening factor is greater
                                 then 0.01  
   PGSCSC_W_INVALID_ALTITUDE     an invalid altitude was specified
   PGSCSC_E_BAD_LAT              an invalid latitude was specified
   PGSCSC_E_NEG_OR_ZERO_RAD      the equatorial or polar radius is 
                                 negative or zero
   PGSMEM_E_NO_MEMORY            no memory is available to allocate vectors
   PGSCSC_E_BAD_ARRAY_SIZE       incorrect array size
   PGSCSC_W_PREDICTED_UT1        status of UT1-UTC correction is predicted
   PGSTD_E_NO_UT1_VALUE          no UT1-UTC correction available
   PGSEPH_E_BAD_EPHEM_FILE_HDR   no s/c ephem files had readable headers
   PGSEPH_E_NO_SC_EPHEM_FILE     no s/c ephem files could be found for input
                                 times
   PGSCSC_E_INVALID_FOV_DATA     FOV perimeter vectors or inFOVvector invalid
   PGSCSC_E_FOV_TOO_LARGE        FOV specification outside algorithmic limits
   PGSCSC_E_INVALID_EARTH_PT     one of the earth point vectors was zero
   PGSCSC_W_ZERO_PIXEL_VECTOR    instrument pixel vector of zero length  
   PGSCSC_W_BAD_EPH_FOR_PIXEL    ephemeris Data missing for some pixels  
            
EXAMPLES:
C:
   #define   ARRAY_SIZE   3
   #define   PERIMVEC_SIZE 4
   PGSt_SMF_status    returnStatus;
   char               asciiUTC[28];
   PGSt_double        offsets[ARRAY_SIZE] = {3600.0,7200.0,10800.0};
   PGSt_integer       numValues;
   PGSt_double        latitude;
   PGSt_double        longitude;
   PGSt_double        altitude;
   PGSt_integer       numFOVperimVec;
   PGSt_double        inFOVvector[ARRAY_SIZE][3] = { {0.0,0.0,100.0},
                                                     {0.0,0.0,200.0},
						     {0.0,0.0,300.0}
						     };
   PGSt_double        perimFOV_vectors[ARRAY_SIZE][PERIMVEC_SIZE][3]=
                                  { {100.0,100.0,100.0},
				    {-100.0,100.0,100.0},
				    {-100.0,-100.0,100.0},
				    {100.0,-100.0,100.0},
				    {200.0,200.0,200.0},
				    {-200.0,200.0,200.0},
				    {-200.0,-200.0,200.0},
				    {200.0,-200.0,200.0},
				    {300.0,200.0,200.0},
				    {-200.0,300.0,200.0},
				    {-200.0,-300.0,300.0},
				    {300.0,-200.0,200.0},
				  };  	 		      
   PGSt_boolean       inFOVflag[ARRAY_SIZE];
   PGSt_double        sctoEarthptVec[ARRAY_SIZE][3];
  
   numValues = ARRAY_SIZE;
   numFOVperimVec = PERIMVEC_SIZE;
   strcpy(asciiUTC,"1995-06-21T11:29:30.123211Z");
   altitude = 10000.0;
   latitude = 0.32;
   longitude = 2.333;
   returnStatus = PGS_CSC_Earthpt_FOV(numValues,asciiUTC,offsets,PGSd_TRMM,
                                      "WGS84",latitude,longitude,
				      altitude,numFOVperimVec,inFOVvector,
				      perimFOV_vectors,inFOVflag,sctoEarthptVec)
                                                                      
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }

FORTRAN:

      implicit none
      integer		  pgs_csc_earthpt_fov
      integer		  returnstatus
      integer		  numvalues
      character*27        startutc 
      double precision    offsets(3)
      double precision    latitude
      double precision    longitude
      double precision    altitude
      integer             numfovperimvec
      double precision    infovvector(3,4)
      double precision    perimfov_vectors(3,4,3)
      integer             infovflag(3)
      double precision    sctoearthptvec(3,3)
      integer             cnt1
      integer             cnt2
      character*33 	  err
      character*241 	  msg
      
      data offsets/3600.0, 7200.0, 10800.0/
      
      perimfov_vectors(1,1,1) = 100.0
      perimfov_vectors(2,1,1) = 100.0
      perimfov_vectors(3,1,1) = 100.0
      
      perimfov_vectors(1,2,1) = -100.0
      perimfov_vectors(2,2,1) = 100.0
      perimfov_vectors(3,2,1) = 100.0
      
      perimfov_vectors(1,3,1) = -100.0
      perimfov_vectors(2,3,1) = -100.0
      perimfov_vectors(3,3,1) = 100.0
      
      perimfov_vectors(1,4,1) = 100.0
      perimfov_vectors(2,4,1) = -100.0
      perimfov_vectors(3,4,1) = 100.0
      
      perimfov_vectors(1,1,2) = 200.0
      perimfov_vectors(2,1,2) = 200.0
      perimfov_vectors(3,1,2) = 200.0
      
      perimfov_vectors(1,2,2) = -200.0
      perimfov_vectors(2,2,2) = 200.0
      perimfov_vectors(3,2,2) = 200.0
      
      perimfov_vectors(1,3,2) = -200.0
      perimfov_vectors(2,3,2) = -200.0
      perimfov_vectors(3,3,2) = 200.0
      
      perimfov_vectors(1,4,2) = 200.0
      perimfov_vectors(2,4,2) = -200.0
      perimfov_vectors(3,4,2) = 200.0
      
      perimfov_vectors(1,1,3) = 300.0
      perimfov_vectors(2,1,3) = 300.0
      perimfov_vectors(3,1,3) = 300.0
      
      perimfov_vectors(1,2,3) = -300.0
      perimfov_vectors(2,2,3) = 300.0
      perimfov_vectors(3,2,3) = 300.0
      
      perimfov_vectors(1,3,3) = -300.0
      perimfov_vectors(2,3,3) = -300.0
      perimfov_vectors(3,3,3) = 300.0
      
      perimfov_vectors(1,4,3) = 300.0
      perimfov_vectors(2,4,3) = -300.0
      perimfov_vectors(3,4,3) = 300.0
      
      infovvector(1,1) = 0.0
      infovvector(1,2) = 0.0
      infovvector(1,3) = 100.0
      
      infovvector(2,1) = 0.0
      infovvector(2,2) = 0.0
      infovvector(2,3) = 200.0
      
      infovvector(3,1) = 0.0
      infovvector(3,2) = 0.0
      infovvector(3,3) = 300.0
      
      asciiutc = '1995-06-21T11:04:57.987654Z'
      numvalues = 3
      numfovperimvec = 4
      altitude = 10000.0
      latitude = 0.32
      longitude = 2.333
      
      returnstatus = pgs_csc_earthpt_fov(numvalues,startutc,offsets,
     >                                   pgsd_trmm,'WGS84',latitude,
     >  	                         longitude,altitude,
     >                                   numfovperimvec,infovvector,
     >  			         perimfov_vectors,infovflag,
     >  				 sctoearthptvec)

      if (returnstatus .ne. pgs_s_success) then
	  pgs_smf_getmsg(returnstatus, err, msg)
	  write(*,*) err, msg
      endif
                                                  
NOTES:
   At each time, the tool determines if the earth point at (latitude, longitude,
   altitude) is in the FOV, setting inFOVflag = PGS_TRUE if so, else PGS_FALSE.
   The vector from SC to Earth point is also returned, whether or not the Earth
   point is in the FOV, and even if it is on the far side of the Earth. Test for
   the spacecraft to Earth point being equal to PGSd_GEO_ERROR_VALUE to avoid
   processing Earth points which could not be determined because of one or more
   errors in the transformation.

   The FOV is always specified in SC coordinates. For an instrument fixed to 
   the SC, use the same FOV description always. For scanning instruments, user
   should provide the description appropriate to the scan instrument. 
   numFOVperimVec should be at least 3. The tool determines if the earth point
   lies within the perimeter defined by the vectors perim_FOVvectors[][][3]. 
   The first index in C (last in Fortran) is the time offset index and the
   second must be sequential around the FOV perimeter. If the altitude is 
   unknown, use zero.

   The vector inFOVvector[][3] must be defined in SC coordinates and must lie 
   within the FOV. The last index in C (first in Fortran) on these vectors is 
   for x,y and z components in SC coordinates. It is necessary for the user to
   supply a vector within the FOV because on the surface of a sphere, a closed
   curve or "perimeter" does not have an inside nor outside, except by 
   arbitrary definition; i.e. this vector tells the algorithm which part of 
   sky is inside the FOV, which outside. If the vector is well centered in the
   FOV, the algorithm will be faster.

   The vectors "perimFOV_vectors[][][3]" defining the FOV perimeter can be in
   clock or counter-clockwise sequence. If the FOV perimeter vectors are
   supplied out of order, the algorithm will run but the results are 
   unpredictable. The input vectors need not be normalized but must not be 
   zero.

   If the FOV definition is bad (two collinear vertices, a zero length
   perimeter vector, or larger than 2*pi steradians), a warning status
   is returned ONLY if the candidate Earth point is on the near side of
   the Earth; for points hidden by the solid Earth, the FOV shape does not
   matter, and no tests are performed on it.  The tool always calls
   PGS_CSC_ECItoSC() with the input position (0,0,0) because it needs the
   distance to Earth center. This results in a message to the logfile
   PGSCSC_W_BELOW_SURFACE, which should be ignored.

   TIME ACRONYMS:
     
     UT1 is:  Universal Time
     UTC is:  Coordinated Universal Time

  TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file $PGSDAT/CSC/utcpole.dat which relates UT1 - 
     UTC values to UTC dates.  This file starts at June 30, 1979; therefore, 
     the correction UT1 - UTC, which can be as large as (+-) 0.9 s, is not 
     available until that date, and an error status message will be returned if
     this function is called with a time before that date.  (A data file good
     for the period  1972 Jan 1 - 1979 June 29, in a slightly different format,
     is present in the Toolkit delivery as $PGSDAT/CSC/utcpole.1972to1979, and
     could easily be reformatted by users needing data that early.) The file
     utcpole.dat, which is maintained periodically, contains final (definitive),
     and predicted (approximate) values.  Thus, when the present function is used,
     the user ought to carefully check the return status.  A success status
     message will be returned if all input times correspond to final values.
     A warning status message will be returned if any input times correspond to
     predicted values.  The predicted values are generally quite usable for
     most geolocation purposes provided that the data file was updated from
     the U.S. Naval Observatory source within the last few months.  Even
     "final" values may change very slightly (equivalent of < 1 cm Earth position)
     if the U.S. Naval Observatory later improves its solution). 
     Use of "PREDICTED UT1" within six months of the last update to utcpole.dat
     will generally result in an error of under 7 meters. The time of the
     most recent update can be found by examining the first line of the
     file $PGSDAT/CSC/utcpole.dat.
    
   ASCII UTC:

     Toolkit functions that accept an ASCII time as input require the time to
     be UTC time in CCSDS ASCII Time Code A or CCSDS ASCII Time Code B format.
     Toolkit functions that return an ASCII time return the UTC time in CCSDS
     ASCII Time Code A format (see CCSDS 301.0-B-2 for details).
     (CCSDS => Consultative Committee for Space Data Systems)

      The general format is:
 
          YYYY-MM-DDThh:mm:ss.ddddddZ (Time Code A)
          YYYY-DDDThh:mm:ss.ddddddZ   (Time Code B)
 
          where:
              -,T,: are field delimiters
              Z is the (optional) time code terminator
              other fields are numbers as implied:
                Time Code A:
                   4 Year digits, 2 Month, 2 Day, 2 hour,
                   2 minute, 2 second, and up to 6 decimal
                   digits representing a fractional second
                Time Code B:
                   4 Year digits, 3 Day of year, 2 hour,
                   2 minute, 2 second, and up to 6 decimal
                   digits representing a fractional second

   TIME OFFSETS:

     This function accepts an ASCII UTC time, an array of time offsets and the
     number of offsets as input.  Each element in the offset array is an offset
     in seconds relative to the initial input ASCII UTC time.
     
     An error will be returned if the number of offsets specified is less than
     zero.  If the number of offsets specified is actually zero, the offsets
     array will be ignored.  In this case the input ASCII UTC time will be
     converted to Toolkit internal time (TAI) and this time will be used to
     process the data.  If the number of offsets specified is one (1) or
     greater, the input ASCII UTC time will converted to TAI and each element
     'i' of the input data will be processed at the time: (initial time) +
     (offset[i]).
     
     Examples:
        
        if numValues is 0 and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
        then input[0] will be processed at time 432000.0 and return output[0]
     
        if numValues is 1 and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
        then input[0] will be processed at time 432000.0 + offsets[0] and
        return output[0]
     
        if numValues is N and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
        then each input[i] will be processed at time 43000.0 + offsets[i] and
        the result will be output[i], where i is on the interval [0,N)

   ERROR HANDLING:

     This function processes data over an array of times (specified by an input
     ASCII UTC time and an array of time offsets relative to that time). 
     
     If processing at each input time is successful the return status of this
     function will be PGS_S_SUCCESS (status level of 'S').
     
     If processing at ALL input times was unsuccessful the status level of the
     return status of this function will be 'E'.
     
     If processing at some (but not all) input times was unsuccessful the
     status level (see SMF) of the return status of this function will be 'W'
     AND all high precision real number (C: PGSt_double, FORTRAN: DOUBLE
     PRECISION) output variables that correspond to the times for which
     processing was NOT successful will be set to the value:
     PGSd_GEO_ERROR_VALUE.  In this case users may (should) loop through the
     output testing any one of the aforementioned output variables against
     the value PGSd_GEO_ERROR_VALUE.  This indicates that there was an error in
     processing at the corresponding input time and no useful output data was
     produced for that time.  
     
     Note: a return status with a status of level of 'W' does not necessarily
     mean that some of the data could not be processed.  The 'W' level may
     indicate a general condition that the user may need to be aware of but
     that did not prohibit processing.  For example, if an Earth ellipsoid
     model is required, but the user supplied value is undefined, the WGS84
     model will be used, and processing will continue normally, except that the
     return status will be have a status level of 'W' to alert the user that
     the default earth model was used and not the one specified by the user.
     The reporting of such general warnings takes precedence over the generic
     warning (see RETURNS above) that processing was not successful at some of
     the requested times.  Therefore in the case of any return status of level
     'W', the returned value of a high precision real variable generally should
     be examined for errors at each time offset, as specified above.

  REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
   PGSTK - 1090 

DETAILS:
   See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
   ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
   for more information on the algorithm.

GLOBALS:
   NONE

FILES:
   $PGSDAT/CSC/earthfigure.dat
   $PGSDAT/TD/leapsec.dat
   $PGSDAT/CSC/utcpole.dat
   also, the spacecraft ephemeris is required by the ephemeris access tool

FUNCTIONS CALLED:
   PGS_CSC_GEOtoECR()
   PGS_CSC_ECRtoECI()
   PGS_CSC_ECItoSC()
   PGS_CSC_PointInFOVgeom()
   PGS_CSC_FOVconicalHull()
   PGS_CSC_GetFOV_Pixel()
   PGS_CSC_GetEarthFigure()
   PGS_MEM_Malloc()
   PGS_MEM_Free()
   PGS_CSC_Norm()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()    
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()

END_PROLOG
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <stdlib.h>
#include <PGS_TD.h>
#include <PGS_CSC.h>
#include <PGS_EPH.h>
#include <PGS_MEM.h>

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_Earthpt_FOV()"

PGSt_SMF_status
PGS_CSC_Earthpt_FOV(                   /* returns a flag or flags indicating 
                                          if the Earth point of given 
                                          latitude, longitude and altitude is 
                                          in the FOV */ 
    PGSt_integer numValues,            /* number of values requested */
    char         asciiUTC[28],         /* UTC start time in CCSDS ASCII Time 
                                          Code A or B format */
    PGSt_double  offsets[],            /* array of time offsets */
    PGSt_tag	 spacecraftTag,        /* unique spacecraft identifier */ 
    char         *earthEllipsTag,      /* earth model id */
    PGSt_double  latitude,             /* latitude of Earth point */
    PGSt_double  longitude,            /* longitude of Earth point */
    PGSt_double  altitude,             /* altitude of Earth point */
    PGSt_integer numFOVperimVec,       /* number of vectors defining FOV 
                                          perimeter */
    PGSt_double  inFOVvector[][3],     /* vector in FOV, in SC coordinates */
    void         *tempPerimFOV_vectors,/* vectors in SC coords defining FOV */
    PGSt_boolean inFOVflag[],          /* set to PGS_TRUE if earth point is 
                                          in FOV */
    PGSt_double  sctoEarthptVec[][3])  /* vector to Earth point in SC coords */
{
    PGSt_double  *perimFOV_vectors; /* vectors in SC coords defining FOV */
    PGSt_double	 norm;	            /* norm of input vector */
    PGSt_double  rMdotProduct;      /* smallest dot product between the inFOV
				       vector and any perimeter vector */
    PGSt_double  (*perimFOV_vecCopy)[3]; /* copy of FOV perimeter vectors */
    PGSt_double  (*perimFOV_vecNorm)[3]; /* FOV perimeter vectors normalized 
					    after call to 
					    PGS_CSC_FOVconicalHull()*/
    PGSt_integer counter;           /* loop counter */
    PGSt_integer counter1;          /* loop counter */
    short        cnt2;              /* loop counter */
    PGSt_double  scToEarthCenterDist; /* distance from spacecraft to the center
				         of the earth */
    PGSt_double  sctoEarthptDist;   /* distance from spacecraft to Earth point*/
    PGSt_double  posECR[3];         /* position ECR vector */
    PGSt_double  inFOVvecNorm[3];   /* in FOV vector normalized */
    PGSt_double  equatRad_A;        /* equatorial radius */
    PGSt_double  polarRad_C;        /* polar radius */
    PGSt_double  flatFac;           /* flattening factor - which is related to
                                       eccentricity of the ellipsoid of 
                                       revolution */
    PGSt_double  meanEarthRadius;   /* radius of earth */
    PGSt_double  scToEarthLimb;     /* distance from spacecraft to earth limb */
    PGSt_double  (*posvelECI)[6];   /* position velocity ECI vectors */
    PGSt_double  (*posvelECR)[6];   /* position velocity ECR vectors */
    PGSt_double  (*posECI)[3];      /* position ECI vectors */
    PGSt_double  (*posSC)[3];       /* position SC vectors */
    PGSt_double  latitudeFOV[1];    /* latitude of the look point */
    PGSt_double  longitudeFOV[1];   /* longitude of the look point */
    PGSt_double  pixelUnitECR[1][3];/* ECR unit pixel vector */
    PGSt_double  slantRange[1];     /* slant range; SC to lookpoint */
    PGSt_double  velocDoppl[1];     /* Doppler velocity of the look point */
    PGSt_integer maxValues;         /* counter cannot equal or exceed this 
				       value */
    PGSt_integer fovValues;         /* number of offsets passed to 
                                       PGS_CSC_GetFOV_Pixel */
    PGSt_double  timeOffset[1];     /* time offset parameter used in call to
				       GetFOV_Pixel() */
    char         *startMem = NULL;  /* pointer to start of memory block 
				       allocated */
    char         *ptrMem;           /* pointer to a location in the memory
				       block allocated */
    char         mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							 PGS_SMF_GetMsg() */
    char         msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
     					                 PGS_SMF_GetMsg() */
    char         specifics[PGS_SMF_MAX_MSG_SIZE];     /* detailed error msg */


    PGSt_SMF_status returnStatus;   /* error returnStatus of function call */
    PGSt_SMF_status holdStatus;     /* saves status for single offsets */     
    PGSt_SMF_status returnStatus1;  /* error status of call to second or later
                                       functions */
    PGSt_boolean saveStatus;        /* becomes TRUE if any cases can be decided */
 
    PGSt_SMF_status code;           /* status code returned by 
				       PGS_SMF_GetMsg()*/

    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS; 
    holdStatus   = PGS_S_SUCCESS;
    saveStatus   = PGS_FALSE;

    /* assign tempPerimFOV_vectors to perimFOV_vectors */

    perimFOV_vectors  = (PGSt_double *) tempPerimFOV_vectors;
    
    /* Test size of arrays */

    if ( numValues < 0 )
    {
	/* NO negative array size is allowed, return with an error status. */

	returnStatus = PGSCSC_E_BAD_ARRAY_SIZE;
	PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME);
	return returnStatus;
    }
    else if (numValues == 0)
	maxValues = 1;
    else
	maxValues = numValues;
    
    /* check that the altitude of the point is not below the surface */

    if (altitude < -50000.0)
    {
	returnStatus = PGSCSC_E_BELOW_SURFACE;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	return returnStatus;
    }

    /* check that the altitude of the point is not greater than 100000 meters 
       - if it is then issue a warning message */

    if (altitude > 100000.0)
    {
	returnStatus = PGSCSC_W_INVALID_ALTITUDE;
	PGS_SMF_GetMsgByCode(returnStatus,msg);  
    }
    
    /* allocate memory */

    returnStatus1 = PGS_MEM_Malloc((void**)&startMem,
				   (sizeof(PGSt_double)*6*maxValues)+
                                   (sizeof(PGSt_double)*6*maxValues)+
                                   (sizeof(PGSt_double)*3*maxValues)+
                                   (sizeof(PGSt_double)*3*numFOVperimVec)+
				   (sizeof(PGSt_double)*3*maxValues)+
				   (sizeof(PGSt_double)*3*numFOVperimVec));
  
    switch (returnStatus1)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSMEM_E_NO_MEMORY:
	return returnStatus1;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    ptrMem = startMem;

    /* assign ECI position/velocity vectors */

    posvelECI = (PGSt_double (*)[6]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*6*maxValues);

     /* assign ECR position/velocity vectors */

    posvelECR = (PGSt_double (*)[6]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*6*maxValues);

    /* assign position vectors */

    posECI = (PGSt_double (*)[3]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*3*maxValues);

    /* assign perimFOV_vecCopy vectors */

    perimFOV_vecCopy = (PGSt_double (*)[3]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*3*numFOVperimVec);

    /* assign posSC vectors */

    posSC = (PGSt_double (*)[3]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*3*maxValues);

    /* assign perimFOV_vecNorm vectors */

    perimFOV_vecNorm = (PGSt_double (*)[3]) (ptrMem);

    /* transform from GEO to ECR */

    returnStatus1 = PGS_CSC_GEOtoECR(longitude,latitude,altitude,
                                    earthEllipsTag,posECR);
        
    switch(returnStatus1)
    {
      case PGS_S_SUCCESS:
        break;
      case PGSCSC_W_DEFAULT_EARTH_MODEL:
      case PGSCSC_W_PROLATE_BODY:
      case PGSCSC_W_SPHERE_BODY:
      case PGSCSC_W_LARGE_FLATTENING:
	if (returnStatus == PGS_S_SUCCESS)
        { 
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
	      PGS_SMF_GetMsgByCode(returnStatus1,msg);
	    returnStatus = returnStatus1;
            holdStatus = returnStatus1;
	}
	break;
      case PGSCSC_E_BAD_LAT:
      case PGSCSC_E_NEG_OR_ZERO_RAD:
	PGS_MEM_Free(startMem);
	return returnStatus1;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	PGS_MEM_Free(startMem);
        return PGS_E_TOOLKIT;
    }
        
    /* assign position/velocity ECR vectors */
  
    for (counter = 0; counter < maxValues; counter++)
    {
	for (cnt2 = 0; cnt2 < 3; cnt2++)
	{
	    posvelECR[counter][cnt2] = posECR[cnt2];
	    posvelECR[counter][cnt2+3]= 0.0;
	}
    }
    
    /* transform from ECR to ECI */

    returnStatus1 = PGS_CSC_ECRtoECI(numValues,asciiUTC,offsets,posvelECR,
				     posvelECI);
 
    switch(returnStatus1)
    {		    
      case PGS_S_SUCCESS:
	break;
      case PGSTD_W_PRED_LEAPS:
      case PGSTD_E_NO_LEAP_SECS:
      case PGSTD_E_NO_UT1_VALUE:	
      case PGSCSC_W_BAD_TRANSFORM_VALUE:
      case PGSCSC_W_PREDICTED_UT1:
	if (returnStatus == PGS_S_SUCCESS)
        { 
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
	      PGS_SMF_GetMsgByCode(returnStatus1,msg);   
	    returnStatus = returnStatus1; 
            holdStatus = returnStatus1;
	} 
      	break;
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSTD_E_TIME_FMT_ERROR:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if(code != returnStatus1)
	  PGS_SMF_GetMsgByCode(returnStatus1,msg); 
	returnStatus = PGSTD_E_BAD_INITIAL_TIME;
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	PGS_MEM_Free(startMem);
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
        PGS_MEM_Free(startMem);
	return PGS_E_TOOLKIT;
    }
        
    /* assign position/velocity ECI to position vector */

    for (counter =0; counter < maxValues; counter++)
      for (cnt2 =0; cnt2 < 3;cnt2++)
	posECI[counter][cnt2] = posvelECI[counter][cnt2];
    
    /* transforms all the candidate points to SC coordinates */

    returnStatus1 = PGS_CSC_ECItoSC(spacecraftTag,numValues,asciiUTC,offsets,
                                    posECI,sctoEarthptVec);

    switch(returnStatus1)
    {
      case PGS_S_SUCCESS:
      case PGSTD_W_PRED_LEAPS:
      case PGSTD_E_NO_LEAP_SECS:
	break;
      case PGSCSC_E_SC_TAG_UNKNOWN:
        PGS_MEM_Free(startMem);
	return returnStatus1;
      case PGSCSC_W_BAD_TRANSFORM_VALUE:
        if (returnStatus == PGS_S_SUCCESS)
        { 
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
	      PGS_SMF_GetMsgByCode(returnStatus1,msg);   
	    returnStatus = returnStatus1; 
            holdStatus = returnStatus1;
	} 
      	break;
      case PGSMEM_E_NO_MEMORY:
      case PGSEPH_E_BAD_EPHEM_FILE_HDR:
      case PGSEPH_E_NO_SC_EPHEM_FILE:
      case PGSEPH_E_NO_DATA_REQUESTED:
        PGS_MEM_Free(startMem);
        return returnStatus1;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
        PGS_MEM_Free(startMem);
	return PGS_E_TOOLKIT;
    }
        
    /* if a posvelECI vector was equal to PGSd_GEO_ERROR_VALUE assign the
       corresponding sctoEarthpt vector to PGSd_GEO_ERROR_VALUE */

    if (returnStatus != PGS_S_SUCCESS)
    {
	for (counter = 0; counter < maxValues; counter++)
	  if (posvelECI[counter][0] == PGSd_GEO_ERROR_VALUE)
	    for (cnt2 = 0; cnt2 < 3; cnt2++)
	      sctoEarthptVec[counter][cnt2] = PGSd_GEO_ERROR_VALUE;
    }
    
    /* determine the position of the earth center in spacecraft coordinates for
       the specified times */

    for (counter = 0; counter < maxValues; counter++)
      for (cnt2 = 0; cnt2 < 3; cnt2++)
        posECI[counter][cnt2] = 0.0;

    returnStatus1 = PGS_CSC_ECItoSC(spacecraftTag,numValues,asciiUTC,offsets,
                                    posECI,posSC);
    
    switch(returnStatus1)
    {
      case PGS_S_SUCCESS:
      case PGSTD_W_PRED_LEAPS:
      case PGSTD_E_NO_LEAP_SECS:
      case PGSCSC_W_BELOW_SURFACE:
	break;
      case PGSCSC_W_BAD_TRANSFORM_VALUE:
        if (returnStatus == PGS_S_SUCCESS)
        { 
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
	      PGS_SMF_GetMsgByCode(returnStatus1,msg);   
	    returnStatus = returnStatus1; 
            holdStatus = returnStatus1;
	} 
      	break;
      case PGSMEM_E_NO_MEMORY:
        PGS_MEM_Free(startMem);
        return returnStatus1;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
        PGS_MEM_Free(startMem);
	return PGS_E_TOOLKIT;
    }

    /* get equatorial and polar radii */
    
    returnStatus1 = PGS_CSC_GetEarthFigure(earthEllipsTag,&equatRad_A,
					   &polarRad_C);
    switch (returnStatus1)
    {
      case PGS_S_SUCCESS:
      case PGSCSC_W_DEFAULT_EARTH_MODEL:
        if (returnStatus == PGS_S_SUCCESS)
        { 
	    holdStatus = returnStatus1;
        }
	break;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	PGS_MEM_Free(startMem);
	return PGS_E_TOOLKIT;
    }	

    /* find the radius of the Earth */

    meanEarthRadius = (equatRad_A + equatRad_A + polarRad_C) / 3.0;

    /*  determine the flattening factor of the Earth */

    flatFac = (equatRad_A - polarRad_C) / equatRad_A;

    /* for each time, determine if the Earth point is in the field of view */

    for (counter = 0; counter < maxValues; counter++)
    {
        /* initialize the inFOVflag to PGS_FALSE */

	inFOVflag[counter] = PGS_FALSE;

	/* if the sctoEarthpt vector equals PGSd_GEO_ERROR_VALUE process the
	   next time value*/

	if (sctoEarthptVec[counter][0] == PGSd_GEO_ERROR_VALUE)
	  continue;
	
	/* check to see that the point is not on the other side of the Earth */

	/* find the distance of the spacecraft to the center of the earth */
	
	scToEarthCenterDist = PGS_CSC_Norm(posSC[counter]);
	
	/* find the distance of the Earth point from the spacecraft */
	
	sctoEarthptDist = PGS_CSC_Norm(sctoEarthptVec[counter]);
	
	/* find the distance from the spacecraft to the Earth limb */
	
	scToEarthLimb = sqrt((scToEarthCenterDist * scToEarthCenterDist) 
			     - (meanEarthRadius * meanEarthRadius));

	/* normalize spacecraft to Earth point vectors */

	if (fabs(sctoEarthptDist) > EPS_64)
	{
	    for (cnt2 = 0; cnt2 < 3; cnt2++)
		sctoEarthptVec[counter][cnt2] = sctoEarthptVec[counter][cnt2] /
		                                sctoEarthptDist;
	}
	else
	{
	    if (returnStatus == PGS_S_SUCCESS)
            {
		holdStatus = PGSCSC_E_INVALID_EARTH_PT;
		returnStatus = PGSCSC_W_ERROR_IN_EARTHPTFOV; 
		PGS_SMF_GetMsgByCode(returnStatus,msg);
		for (cnt2 = 0; cnt2 < 3; cnt2++)
		  sctoEarthptVec[counter][cnt2] = PGSd_GEO_ERROR_VALUE;
	    }
	    continue;
	}  
		
	/* if the spacecraft to earth point distance is greater than 
	   1 + 2 times the flattening factor of that to the limb, 
           then the point is on the other side of the earth and not visible */
	
	if (sctoEarthptDist > ((1.0 + 2.0 * flatFac) * scToEarthLimb))
	{
            /* a decision has been made that point is not in FOV */
            saveStatus = PGS_TRUE;
	    inFOVflag[counter] = PGS_FALSE;
	    continue;
	}

	/* copy the FOV perimeter vectors for the time offset into a two 
	   dimensional array */

	for (counter1 = 0; counter1 < numFOVperimVec; counter1++)
	{    
	    for (cnt2 = 0; cnt2 < 3; cnt2++)
	    {
		perimFOV_vecCopy[counter1][cnt2] = 
		  perimFOV_vectors[counter * numFOVperimVec * 3 + counter1 * 3
                                   + cnt2];
	    }
	}

	/* normalize the in field of view vector */

	norm = PGS_CSC_Norm(inFOVvector[counter]);
		
	if (fabs(norm) > EPS_64 )
	  for (cnt2 = 0; cnt2 < 3; cnt2++)
	    inFOVvecNorm[cnt2] = inFOVvector[counter][cnt2] / norm;
	else
	{	
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		
		returnStatus = PGSCSC_E_INVALID_FOV_DATA;
		PGS_SMF_GetMsgByCode(returnStatus,msg);
		for (cnt2 = 0; cnt2 < 3; cnt2++)
		  sctoEarthptVec[counter][cnt2] = PGSd_GEO_ERROR_VALUE;
	    } 
	    continue;
	} 
        
	/* call function PGS_CSC_FOVconicalHull() which draws a circular cone 
	   around the FOV and first checks if the earth point is inside it 
	   before going any further */
        		
  	returnStatus1 = PGS_CSC_FOVconicalHull(numFOVperimVec,
					       inFOVvecNorm,
					       perimFOV_vecCopy,0.0,
					       sctoEarthptVec[counter],
					       perimFOV_vecNorm,
					       (&inFOVflag[counter]),
					       &rMdotProduct);
    
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGSCSC_E_INVALID_FOV_DATA:
	  case PGSCSC_E_FOV_TOO_LARGE:
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		  PGS_SMF_GetMsgByCode(returnStatus1,msg);   
                  sprintf(specifics,"%s%s %d","FOV too large, bad perimeter or", 
                    " inFOVvector, offset",(int) counter);
                PGS_SMF_SetDynamicMsg(returnStatus1,specifics,
                              FUNCTION_NAME);
		returnStatus = PGSCSC_W_ERROR_IN_EARTHPTFOV; 
                holdStatus = returnStatus1;
		for (cnt2 = 0; cnt2 < 3; cnt2++)
		  sctoEarthptVec[counter][cnt2] = PGSd_GEO_ERROR_VALUE;
	    }
	    continue;
	  default:
	      PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	      for (cnt2 = 0; cnt2 < 3; cnt2++)
	         sctoEarthptVec[counter][cnt2] = PGSd_GEO_ERROR_VALUE;
              PGS_MEM_Free(startMem);
	      return PGS_E_TOOLKIT;
	}

	if (inFOVflag[counter] == PGS_FALSE)
        {
            /* a decision has been made that point is not in FOV */
            saveStatus = PGS_TRUE;
	    continue;
        }
  
	/* call function PGS_CSC_PointInFOVgeom() to determine if the point is
	   in the FOV at the specified time */
	
	returnStatus1 = PGS_CSC_PointInFOVgeom(numFOVperimVec,
					       inFOVvecNorm,perimFOV_vecNorm,
					       sctoEarthptVec[counter],
					       (&inFOVflag[counter]));
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:

	    break;
	  case PGSMEM_E_NO_MEMORY:
	    PGS_MEM_Free(startMem);
	    return returnStatus1;
	  case  PGSCSC_E_INVALID_FOV_DATA:
             returnStatus = PGSCSC_W_ERROR_IN_EARTHPTFOV;
             holdStatus = returnStatus1;
	     for (cnt2 = 0; cnt2 < 3; cnt2++)
	      sctoEarthptVec[counter][cnt2] = PGSd_GEO_ERROR_VALUE;
              sprintf(specifics,"%s%s %d","Invalid perimeter or inFOVvector",
                    "offset",(int) counter);
              PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                              FUNCTION_NAME);
	    continue;  
	  default:
	      PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	      for (cnt2 = 0; cnt2 < 3; cnt2++)
	         sctoEarthptVec[counter][cnt2] = PGSd_GEO_ERROR_VALUE;
              PGS_MEM_Free(startMem);
	      return PGS_E_TOOLKIT;
	}

        /* PGS_CSC_PointInFOVgeom has decided point in/not in FOV */
        saveStatus = PGS_TRUE;
	if (inFOVflag[counter] == PGS_FALSE)
        {
	    continue;
        }

	/* check if the spacecraft to earth point distance is less than 
           1 - 2 * flatFac of that to the limb - if so, then it is in the 
           field of view */
	
	if (sctoEarthptDist < ((1.0 - 2.0 * flatFac) * scToEarthLimb))
	    continue;
	
	/* otherwise ellipticity is important, compare the longitudes and 
	   latitudes to see if the point is in the field of view. (Recall
           that we already tested very large distances well beyond Earth 
           limb and reported PGS_FALSE in that case */
	
	else
	{
	    if (numValues == 0 )
	      fovValues = 0;
	    else
	    {
		timeOffset[0] = offsets[counter];
		fovValues = 1;
	    }
	    
	    /* call GetFOV_Pixel to get the longitude and latitude of the
	       point */
	    
	    returnStatus1 = PGS_CSC_GetFOV_Pixel(spacecraftTag,fovValues,
				      asciiUTC,timeOffset,earthEllipsTag,
				      PGS_FALSE,(sctoEarthptVec+counter),NULL,
				      latitudeFOV,longitudeFOV,pixelUnitECR,
				      slantRange,velocDoppl);

	    switch (returnStatus1)
	    {
            /* if all is A-OK, check if lat and lon agree within a tolerance
               that was found by experimentation.  If they disagree, then the
               candidate point is almost certainly beyond Earth limb, while the
               line of sight towards it intersects the ellipsoid nearby on the 
               near side of the limb.  The tolerance is somewhat ad hoc, but
               errors in terrain and refraction will dominate the determination
               in case of any smaller difference */

	      case PGS_S_SUCCESS:
	      case PGSCSC_W_DEFAULT_EARTH_MODEL:
		if ((fabs(latitudeFOV[0] - latitude) >= 0.0001) ||
		    (fabs(longitudeFOV[0] - longitude) >= 0.0001))
		  inFOVflag[counter] = PGS_FALSE;
		break;
	    }             
	} /* else - end checking for ellipticity */
    } /* end for(counter =0;counter < ...... */
        
    /* free memory allocated */

    PGS_MEM_Free(startMem);

    /* cleanup error messages and status */
    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    else if (saveStatus == PGS_FALSE)
	/* No good offsets processed - return error and message */
    {
	switch (PGS_SMF_TestStatusLevel(returnStatus))
	{
	  case PGS_SMF_MASK_LEV_E:
	    break;
	  default: 
	    if(holdStatus != PGS_S_SUCCESS)
	    {
		returnStatus = holdStatus;
	    }
	}
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
    }
    else
	switch (PGS_SMF_TestStatusLevel(returnStatus))
    	{
	    /* error for only some offsets - in message Logfile - return
	       warning */
	  case PGS_SMF_MASK_LEV_E:
	    returnStatus = PGSCSC_W_ERROR_IN_EARTHPTFOV; 
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	    break;
	  default: /* cases W, M ... * all ok */ 
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
    	}
    return returnStatus;
}
