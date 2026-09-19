/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_SubSatPoint.c

DESCRIPTION:
   This file contains the function PGS_CSC_SubSatPoint(). 

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation
   Deborah A. Foch      / Applied Research Corporation
   Guru Tej S. Khalsa   / Applied Research Corporation

HISTORY:
   01-Feb-1994  PDN  Designed			
   19-Mar-1994  PDN  Coded
   20-Sep-1994  DAF  Revised prolog
   24-Sep-1994  DAF  Revised function calls and program logic/comments
   30-Sep-1994  DAF  Completed revisions in preparation for unit tests
   03-Oct-1994  DAF  Updated EphemAttit error returns
   18-Oct-1994  DAF  Updated EphemAttit error returns
   20-Mar-1994  GTSK Added code to free memory allocated internally before
                     returning to calling function.  Replaced multiple calls to
		     malloc() with single call to PGS_MEM_Calloc().
   20-May-1995  GTSK Major rewrite.
   11-Jul-1995  PDN  Fixed description of the function; examples
   25-Jun-1996  PDN  Deleted check for obsolete return value
   05-Jul-1996  GTSK Fixed logic which uses qualityFlags array to hold bad
                     values encountered in this routine.  This routine was
		     setting the qualityFlags value for a given offset to zero
		     if an error was encountered in processing the given offset.
		     This was because after the call to PGS_EPH_EphemAttit()
		     bad offsets were already set to zero.  NOW bad offsets are
		     set to one so the code in this routine had to be similarly
		     changed to use one instead of zero to indicate bad offsets.
		     Thanks to the MISR instrument team for pointing this out.
                  
END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG: 
 
TITLE:
   Get Sub Satellite Point Position and Velocity and SC altitude rate

NAME:
   PGS_CSC_SubSatPoint() 

SYNOPSIS:
C:
   #include <PGS_CSC.h>
  
   PGSt_SMF_status
   PGS_CSC_SubSatPoint(
       PGSt_tag          spacecraftTag,
       PGSt_integer      numValues,
       char              asciiUTC[28],
       PGSt_double       offsets[],
       char              earthEllipsTag[20],
       PGSt_boolean      velFlag,
       PGSt_double	 latitude[],
       PGSt_double       longitude[],
       PGSt_double       altitude[],
       PGSt_double       velSub[][3])
     
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_EPH_5.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
      include 'PGS_TD.f'
      include 'PGS_MEM_7.f'
   
      integer function pgs_csc_subsatpoint(spacecrafttag,numvalues,
     >                                     asciiutc,offsets,
     >                                     earthellipstag,velflag,
     >          			   latitude,longitude,altitude,
     >                                     velsub)

      integer            spacecrafttag
      integer            numvalues
      character*27       asciiutc
      double precision   offsets(*)
      character*20       earthellipstag
      integer            velflag
      double precision   latitude(*)
      double precision   longitude(*)
      double precision   altitude(*)
      integer            velsub(3,*)

DESCRIPTION:
   This tool finds the latitude, and longitude, of the subsatellite points 
   and the spacecraft altitude at the input times/offsets.  Optionally, it 
   returns the North and East vector components of the velocity of the
   subsatellite point along the Earth ellipsoid and the rate of change of
   the altitude of the spacecraft at each time/offset. 

INPUTS:
   Name             Description                 nits       Min        Max
   ----             -----------                 ----       ---        ---
   spacecraftTag    spacecraft identifier       N/A        N/A        N/A

   numValues        number of input offset      N/A         0         any

   asciiUTC         timesstart UTC time in      N/A        1979-06-30 see NOTES
                    CCSDS ASCII Time Code 	
		    (A or B format)		

   offsets          array of time offsets       seconds    Max and Min such
                                                           that asciiUTC +
							   offset is between
						           Min and Max values

   earthEllipsTag   tag selecting Earth         N/A        N/A        N/A
 		    ellipsoidmodel (default 
		    is WGS84)	

   velFlag          flag indicating whether to  N/A        PGS_FALSE  PGS_TRUE
                    return the velocity of the
                    subsatellite point and
                    rate of change of altitude
 
OUTPUTS:
   Name             Description                  Units       Min        Max
   ----             -----------                  -----       ---        ---
   latitude         array of subsatellite        radians    -pi/2       pi/2
                    point geodetic latitudes

   longitude        array of subsatellite        radians     -pi        pi
                    point longitudes

   altitude         array of spacecraft          m           250000     10000000
                    altitudes

   velSub[0]        North component of the       m/s        -7000       7000
                    subsatellite point velocity 
                    on the ellipsoid                

   velSub[1]        East component of the        m/s        -7000       7000
                    subsatellite point velocity 
                    on the ellipsoid                

   velSub[2]        rate of change of            m/s        -200        200
                    spacecraft altitude relative
                    to nadir on the ellipsoid                        

  
RETURNS:
   PGS_S_SUCCESS		   successful return
   PGSTD_W_PRED_LEAPS              leap second value was predicted, for at
                                   least one of the input times/offsets
   PGSCSC_W_ERROR_IN_SUBSATPT      an error occurred in computing at least
                                   one subsatellite point
   PGSCSC_W_PREDICTED_UT1          at least one of the values obtained
                                   from the utcpole.dat file is 
				   'predicted' 
   PGSCSC_W_PROLATE_BODY           using a prolate earth model
   PGSCSC_W_SPHERE_BODY            using a spherical earth model
   PGSCSC_W_LARGE_FLATTENING       issued if flattening factor is greater
                                   than 0.01  
   PGSCSC_W_DEFAULT_EARTH_MODEL    default earth model was used
   PGSCSC_W_ZERO_JACOBIAN_DET      Jacobian determinant is close to zero
   PGSCSC_E_BAD_ARRAY_SIZE         numValues (and array size) is less than
                                   zero 
   PGSMEM_E_NO_MEMORY              no memory available to allocate vectors
   PGSTD_E_SC_TAG_UNKNOWN          invalid spacecraft tag
   PGSEPH_E_BAD_EPHEM_FILE_HDR     no spacecraft ephemeris files had
                                   reasonable headers
   PGSEPH_E_NO_SC_EPHEM_FILE       no spacecraft ephemeris files could 
                                   be found for input
   PGSTD_E_TIME_FMT_ERROR          format error in input asciiUTC 
   PGSTD_E_TIME_VALUE_ERROR        error in one of time values in asciiUTC
   PGSTD_E_NO_LEAP_SECS            no leap seconds correction available
                                   for at least one of the input times/
				   offsets - a linear approximation was
				   used to obtain the leapsec value
   PGSTD_E_NO_UT1_VALUE            no UT1-UTC correction available
   PGSCSC_E_BAD_EARTH_MODEL        The equatorial or polar radius is negative
                                   or zero OR the radii define a prolate Earth
   PGS_E_TOOLKIT                   something unexpected happened, 
                                   - execution aborted

EXAMPLES:
C:
   #define   ARRAY_SIZE   3

   PGSt_SMF_status    returnStatus;
   PGSt_tag           spacecraftTag = PGSd_EOS_AM;
   PGSt_integer       numValues;
   char               asciiUTC[28];
   PGSt_double        offsets[ARRAY_SIZE] = {3600.0,7200.0,10800.0};
   char               earthEllipsTag[20];
   PGSt_boolean       velFlag = PGS_TRUE;
   PGSt_double        latitude[ARRAY_SIZE];
   PGSt_double        longitude[ARRAY_SIZE];
   PGSt_double        altitude[ARRAY_SIZE];
   PGSt_double        velSub[ARRAY_SIZE][3];
   PGSt_integer       counter;

   numValues = ARRAY_SIZE;
   strcpy(asciiUTC,"1991-01-01T11:29:30");
   strcpy(earthEllipsTag,"WGS84");

   returnStatus = PGS_CSC_SubSatPoint(spacecraftTag,numValues,asciiUTC,offsets,
                                      earthEllipsTag,velFlag,latitude,longitude,
				      altitude,velSub);
                                                                        
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }
   printf("start time:%s",asciiUTC);
   counter = 0;
   while(counter <= numValues)
   {
       printf("Offset: %f   Latitude: %f   Longitude: %f  Altitude: %f\n",
              offset[counter], latitude[counter], longitude[counter],
	      altitude[counter]);
       printf("Velocity of subsatellite point (North,East): %f, %f, m/s\n", 
               velSub[counter][0], velSub[counter][1]);
       printf("Rate of altitude change: %f m/s\n", velSub[counter][2]);
	      
	      counter++;	      
   }
       
FORTRAN:
      implicit none
      integer            pgs_csc_subsatpoint
      integer            array_size
      integer            spacecrafttag
      integer            numvalues
      character*27       asciiutc
      double precision   offsets(array_size)
      character*20       earthellipstag
      integer            velflag
      double precision   latitude(array_size)
      double precision   longitude(array_size)
      double precision   altitude(array_size)
      double precision   velsub(3,array_size)
      integer            returnstatus
      integer            counter  
   
      data offsets/3600.0,7200.0,10800.0/
      data earthellipstag/'WGS84'/,velflag/pgs_true/
      array_size = 3
      numvalues = array_size
      spacecrafttag = pgsd_eos_am
      asciiutc = '1991-01-01T11:29:30'
  
      returnstatus = pgs_csc_subsatpoint(spacecrafttag,numvalues,
     >                                   asciiutc,offsets,
     >                                   velflag,earthellipstag,
     >                                   latitude,longitude,altitude,
     >  				 velsub)
      if (returnstatus .ne. pgs_s_success) go to 90
      write(6,*) asciiutc
      do 40 counter = 0, numvalues, 1
          write(6,*) offsets(counter),latitude(counter),
     >               longitude(counter),altitude(counter),
     >               velsub(1,counter),velsub(2,counter),
     >               velsub(3,counter)
 40   continue

 90   write(6,99) returnstatus
 99   format('ERROR:',I50)  


 
NOTES:
   If an invalid earthEllipsTag is input, the program will use the WGS84 Earth
   model by default.

   The option to obtain velocity is controlled by setting the velocity flag
   velFlag to either PGS_TRUE or PGS_FALSE.  If velFlag is PGS_FALSE, all
   components of velSub will be set to zero.  If the velocity is not needed it
   is strongly recommended to use PGS_FALSE to speed the execution of the code.
   
   The horizontal velocity calculated in function PGS_CSC_SubSatPointVel() is
   that of a mathematical point on the Earth at (nominal) spacecraft nadir, and
   not that of any material object.  It is orthogonal to nadir, so is suitable
   as a descriptor of ground track but not for Doppler work.

   The third (vertical) component of velocity is useful for Doppler work at
   nadir, but Doppler velocity along ANY look vector (not just nadir) is
   provided in the lookpoint algorithm in the function PGS_CSC_GetFOVPixel().
   
   See "Theoretical Basis of the SDP Toolkit Geolocation Package", Document 
   445-TP-002-002, May 1995 for more information on the algorithm.

   TIME ACRONYMS:
     
     UT1 is:  Universal Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file utcpole.dat which relates UT1 - UTC values to
     UTC dates.  As of July 1995 utcpole.dat starts at June 30, 1979; therefore,
     the correction UT1 - UTC, which can be as large as (+-) 0.9 s, is not
     available until that date, and an error status message will be returned if
     this function is called with an initial time before that date.  The file
     utcpole.dat, which is maintained periodically, contains final (definitive),
     interim (partially reduced data and short term predictions of good
     accuracy) and predicted (long term; very approximate) values.  The latter
     can be used only in simulations.  Thus, when the present function is used,
     the user ought to carefully check the return status.  A success status 
     message will be returned if all input times correspond to final values.
     A warning status message will be returned if any input times correspond to
     interim or predicted values.

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
   PGSTK - 1060, 0930

DETAILS: 
   The warning message "PGSCSC_W_ZERO_JACOBIAN_DET" and default velocities of
   0.0 m/s are returned when a singularity is encountered at the North or South
   Pole.  The singularity will occur only if the velocity is required (velFlag=
   PGS_TRUE) and the spacecraft passes directly over the pole, which is not 
   contemplated for the present and next generation of EOS spacecraft.  
   There is no "workaround" for the  singularity, because the North and East 
   components of the velocity are undefined at the poles.

GLOBALS:
   None

FILES:
   utcpole.dat
   leapsec.dat
   earthfigure.dat 

FUNCTIONS CALLED:
   PGS_EPH_EphemAttit()
   PGS_EPH_ManageMasks()
   PGS_CSC_ECItoECR()
   PGS_CSC_SubSatPointVel() 
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_EPH.h>
#include <PGS_MEM.h>

/* name of this function */

#define  FUNCTION_NAME  "PGS_CSC_SubSatPoint()"

PGSt_SMF_status
PGS_CSC_SubSatPoint(                   /* finds satellite nadir point and
					  (optionally) its velocity */
    PGSt_tag        spacecraftTag,     /* unique spacecraft identifier */
    PGSt_integer    numValues,         /* number of time offsets */
    char            asciiUTC[28],      /* start time in CCSDS UTC Time Code
					  A or B format */
    PGSt_double     offsets[],         /* array of time offsets from start time
					  (seconds) */
    char            *earthEllipsTag,   /* Earth model tag -- eg. WGS84, MERIT */
    PGSt_boolean    velFlag,           /* flag indicating whether to return
					  the velocity of the subsatellite
					  point */
    PGSt_double     latitude[],        /* array of geodetic latitudes of the 
					  subsatellite points */
    PGSt_double     longitude[],       /* array of longitudes of the 
					  subsatellite points */
    PGSt_double     altitude[],        /* array of spacecraft altitudes at the
					  subsatellite points */
    PGSt_double     velSub[][3])       /* array of values of velocity
					  components (north, east, and 
					  altitude) of each of the 
					  subsatellite points (only if velFlag
					  is PGS_TRUE). */ 
{		     		    
    void             *memPtr=NULL;      /* pointer to memory space allocated
					   dynamically for scratch arrays used
					   internally by this function */
    PGSt_integer     counter;           /* loop counter */
    PGSt_integer     maxValues;         /* counter cannot equal or exceed this
					   value */	
    PGSt_integer     (*qualityFlags)[2];/* quality flags (pos/attitude) for each
					   set of ephemeris values returned */
    PGSt_integer     qualityFlagsMasks[2];/* user definable quality flag masks
					     used to check validity of data
					     returned by PGS_EPH_EphemAttit() */
    PGSt_double      (*posECI)[3];      /* array of spacecraft position vectors
					   in ECI coordinates for each input
					   time/offset */
    PGSt_double      (*velECI)[3];      /* array of spacecraft velocity vectors
					   in ECI coordinates for each input
					   time/offset */
    PGSt_double      (*posVel)[6];      /* spacecraft position (m) and velocity
					   (m/sec) for each input time/offset */
    PGSt_SMF_status  code;              /* status code returned from
					   PGS_SMF_GetMsg() */
		     
    /* mnemonic returned by PGS_SMF_GetMsg() */
    char             mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];
		     
    /* message returned by PGS_SMF_GetMsg() */
    char             msg[PGS_SMF_MAX_MSG_SIZE]; 
		     
    PGSt_SMF_status  returnStatus;     /* return value of this function */
    PGSt_SMF_status  returnStatus1;    /* return value of PGS functions called
					  (except first one) */
    
    /* Set maximum value of counter for looping to include case
       of no offset times, when computing just for start UTC time */
    
    if (numValues < 0 )
    {
        /* NO negative array size is allowed, return with an error status. */

        returnStatus = PGSCSC_E_BAD_ARRAY_SIZE;
        PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME );
        return returnStatus;
    }
    else if (numValues == 0)
      maxValues = 1;
    else
      maxValues = numValues;
    
    /* Allocate space for scratch arrays.  This includes qualityFlags, posECI,
       velECI and posVel. */
    
    returnStatus = PGS_MEM_Calloc(&memPtr, 1,
				  maxValues*(sizeof(PGSt_integer)*2 +
					     sizeof(PGSt_double)*(3+3+6)));
    
    if (returnStatus != PGS_S_SUCCESS)
	return returnStatus;
    
    /* Asign space for qualityFlags - quality flags for the position data
       returned by PGS_EPH_EphemAttit() (called below). */

    qualityFlags = (PGSt_integer (*)[2]) memPtr;
    
    /* Asign space for posECI - Spacecraft ECI position vector and
       velECI - Spacecraft ECI velocity vector */

    posECI = (PGSt_double (*)[3]) (qualityFlags + maxValues);
    velECI = posECI + maxValues;

    /* Asign space for the 6-vector Spacecraft ephemeris values */
    
    posVel = (PGSt_double (*)[6]) (velECI + maxValues);
    
   
    /* Get Spacecraft Ephemeris (position and velocity vectors) in ECI
       coordinates */
    
    returnStatus = PGS_EPH_EphemAttit(spacecraftTag,numValues,asciiUTC,offsets,
				      PGS_TRUE,PGS_FALSE,qualityFlags,posECI,
				      velECI,NULL,NULL,NULL);
    switch(returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSEPH_W_BAD_EPHEM_VALUE:
      case PGSTD_W_PRED_LEAPS:
      case PGSTD_E_NO_LEAP_SECS:
      	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if(code != returnStatus)
	    PGS_SMF_GetMsgByCode(returnStatus,msg);
	if (returnStatus == PGSEPH_W_BAD_EPHEM_VALUE)
	    returnStatus = PGSCSC_W_ERROR_IN_SUBSATPT;
	break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSEPH_E_BAD_EPHEM_FILE_HDR:
      case PGSEPH_E_NO_SC_EPHEM_FILE:
      case PGSTD_E_SC_TAG_UNKNOWN:
	PGS_MEM_Free(memPtr);
      	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	PGS_MEM_Free(memPtr);
	return PGS_E_TOOLKIT;
    }
    
    /* Place position and velocity 3-vectors in 6-vectors that can be
       passed to the ECI to ECR function */
    
    for(counter=0;counter<maxValues;counter++)
    { 
	posVel[counter][0] = posECI[counter][0];
	posVel[counter][1] = posECI[counter][1];
	posVel[counter][2] = posECI[counter][2];
	posVel[counter][3] = velECI[counter][0];
	posVel[counter][4] = velECI[counter][1];
	posVel[counter][5] = velECI[counter][2];
    }
    
    /* get the users qualityFlags masks (returns system defaults if the user has
       not set any mask values) */

    returnStatus1 = PGS_EPH_ManageMasks(PGSd_GET, qualityFlagsMasks);
    switch (returnStatus1)
    {
      case PGS_S_SUCCESS:
	break;
	
      case PGSPC_E_DATA_ACCESS_ERROR:
	returnStatus1 = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus1, "unexpected error accessing "
			      "Process Control File", FUNCTION_NAME);
	
      case PGS_E_TOOLKIT:
	return returnStatus1;
	
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* make sure the qualityFlags mask for ephemeris data contains the "no data"
       bit set by the toolkit */

    qualityFlagsMasks[0] = qualityFlagsMasks[0] | PGSd_NO_DATA;
    
    /* Convert Spacecraft Ephemeris Vectors from ECI to ECR coordinates (posVel
       in ECI is input, posVel will be overwritten with ECR values). */
    
    returnStatus1 = PGS_CSC_ECItoECR(numValues,asciiUTC,offsets,posVel,posVel);
    
    /* check errors returned from PGS_CSC_ECItoECR */
    
    switch(returnStatus1)
    {		    
      case PGS_S_SUCCESS:
	break;
      case PGSTD_W_PRED_LEAPS:
      case PGSTD_E_NO_LEAP_SECS:
    	if(returnStatus != returnStatus1)
	{
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
	      PGS_SMF_GetMsgByCode(returnStatus1,msg);
	    returnStatus = returnStatus1;
	}
      case PGSCSC_W_BAD_TRANSFORM_VALUE: 
      case PGSCSC_W_PREDICTED_UT1: 
	if (returnStatus == PGS_S_SUCCESS)
        { 
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
	      PGS_SMF_GetMsgByCode(returnStatus1,msg);   
	    returnStatus = returnStatus1;
	    if (returnStatus == PGSCSC_W_BAD_TRANSFORM_VALUE)
		returnStatus = PGSCSC_W_ERROR_IN_SUBSATPT;
	}

	/* Keep track of bad data points using the qualityFlags array.  A value
	   of one indicates bad data.  Set the appropriate flag equal to 1 if
	   it isn't already one and the corresponding value returned from
	   PGS_CSC_ECItoECR() is no good (components have been set to 
	   PGSd_GEO_ERROR_VALUE). */

	for (counter=0;counter<maxValues;counter++)
	{
	    if (posVel[counter][0] == PGSd_GEO_ERROR_VALUE)
	    {
		qualityFlags[counter][0] = qualityFlagsMasks[0];
	    }
	}

      	break;
      case PGS_E_TOOLKIT:
      case PGSTD_E_NO_UT1_VALUE:
        PGS_MEM_Free(memPtr);
	return returnStatus1;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
        PGS_MEM_Free(memPtr);
	return PGS_E_TOOLKIT;
    }

    /* Convert posVel from ECR coordinates to Geodetic (GEO) coordinates
       to get spacecraft altitude and latitude, longitude  */

    for(counter=0;counter<maxValues;counter++)
    {
	if ((qualityFlagsMasks[0] & qualityFlags[counter][0]) != 0)
	{
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;
		sprintf(msg,"one or more points was not processed due to "
			"quality of ephemeris data");
	    }
	    continue;
	}
	
	returnStatus1 = PGS_CSC_ECRtoGEO(posVel[counter],earthEllipsTag,
					 longitude+counter,latitude+counter,
					 altitude+counter);
	
	switch (returnStatus1)
	{        
	  case PGS_S_SUCCESS:
	    break;
	  case PGSCSC_W_DEFAULT_EARTH_MODEL:
	  case PGSCSC_W_SPHERE_BODY:
	  case PGSCSC_W_LARGE_FLATTENING:
	    if (returnStatus == PGS_S_SUCCESS || 
		returnStatus == PGSCSC_W_ERROR_IN_SUBSATPT)
	    { 
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);   
		returnStatus = returnStatus1;
	    }
	    break;
	  case PGSCSC_W_TOO_MANY_ITERS:
	  case PGSCSC_W_INVALID_ALTITUDE:
	    if (returnStatus == PGS_S_SUCCESS)
	    { 
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);   
		returnStatus = PGSCSC_W_ERROR_IN_SUBSATPT;
	    }
	    
	    /* Keep track of bad data points using the qualityFlags array.  A
	       value of one indicates bad data.  Set the appropriate flag equal
	       to 1 and continue looping through to next offset time */
	    
	    qualityFlags[counter][0] = qualityFlagsMasks[0];

	    /* temporarily set lat., lon. and alt. to zero so
	       PGS_CSC_SubSatPointVel() won't choke on them (they will be set to
	       PGSd_GEO_ERROR_VALUE before being returned to the calling 
	       function) */

	    latitude[counter] = 0.0;
	    longitude[counter] = 0.0;
	    altitude[counter] = 0.0;
	    
	    continue; 
	  case PGSCSC_E_BAD_EARTH_MODEL:
	    PGS_MEM_Free(memPtr);
	    return returnStatus;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	    PGS_MEM_Free(memPtr);
	    return PGS_E_TOOLKIT;
	    
	}
    }
	
    /* Get velocity of
       subsatellite point by calling function PGS_CSC_SubSatPointVel().  The
       function calls PGS_CSC_ECRtoGEO(), which gets the Earth radii based
       on the input Earth model -- if the model is not in the file
       earthfigure.dat or if the file is not found, then the default model
       (WGS84) is assumed.  Set velFlag to PGS_TRUE if the velocity of the
       subsatellite point is desired */
    
    if (velFlag == PGS_TRUE)
    {
	returnStatus1 = PGS_CSC_SubSatPointVel(maxValues,posVel,earthEllipsTag,
					       longitude,latitude,altitude,
					       velSub);

	/* Check error returns */

	switch (returnStatus1)
	{        
	  case PGS_S_SUCCESS:
	  case PGSCSC_W_DEFAULT_EARTH_MODEL:
	  case PGSCSC_W_SPHERE_BODY:
	  case PGSCSC_W_LARGE_FLATTENING:
	    break;
	  case PGSCSC_W_ZERO_JACOBIAN_DET:
	    if (returnStatus == PGS_S_SUCCESS)
	    { 
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);   
		returnStatus = returnStatus1; 
	    }
	    for (counter=0;counter<maxValues;counter++)
		if (velSub[counter][0] == PGSd_GEO_ERROR_VALUE)
		{
		    latitude[counter] = PGSd_GEO_ERROR_VALUE;
		    longitude[counter] = PGSd_GEO_ERROR_VALUE;
		    altitude[counter] = PGSd_GEO_ERROR_VALUE;
		}
	    break;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	    PGS_MEM_Free(memPtr);
	    return PGS_E_TOOLKIT;
	} 
    }

    /* if the return status is NOT PGS_S_SUCCESS at this point, there may be
       SOME bad data points, check the qualityFlags array and set the bad values
       to PGSd_GEO_ERROR_VALUE so that the users can distinguish good values
       from bad */
    
    if (returnStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	for (counter=0;counter<maxValues;counter++)
	{
	    if ((qualityFlagsMasks[0] & qualityFlags[counter][0]) != 0)
	    {
		/* Set output latitude, longitude, and altitude, and
		   velocities to error values */
		
		latitude[counter] = PGSd_GEO_ERROR_VALUE;
		longitude[counter] = PGSd_GEO_ERROR_VALUE;
		altitude[counter] = PGSd_GEO_ERROR_VALUE;
		if (velFlag == PGS_TRUE)
		{
		    velSub[counter][0] = PGSd_GEO_ERROR_VALUE;
		    velSub[counter][1] = PGSd_GEO_ERROR_VALUE;
		    velSub[counter][2] = PGSd_GEO_ERROR_VALUE;
		}
	    }
	}
    }
    else
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	
    /* Free memory. */

    PGS_MEM_Free(memPtr);
    
    /*  Return to calling function. */
    
    return returnStatus; 
}




