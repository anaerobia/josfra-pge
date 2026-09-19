/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
   PGS_CSC_ORBtoECI.c
 
DESCRIPTION:
   This file contains the function PGS_CSC_ORBtoECI().
   This function transforms vector in Orbital coordinate system to vector in ECI
   coordinate system.
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
 
HISTORY:
   18-Apr-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE: 
   Transform from ORB Frame to ECI Frame

NAME:   
   PGS_CSC_ORBtoECI()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_ORBtoECI(
       PGSt_tag     spacecraftTag,
       PGSt_integer numValues,
       char         asciiUTC[28], 
       PGSt_double  offsets[],  
       PGSt_double  positionORB[][3],
       PGSt_double  positionECI[][3])
	      
FORTRAN:
      include 'PGS_CSC_4.f'
      include 'PGS_SMF.f'

      integer function
      pgs_csc_orbtoeci(spacecrafttag,numvalues,asciiutc,offsets,positionorb,
     >                 positioneci)
      integer          spacecrafttag
      integer          numvalues
      character*27     asciiutc
      double precision offsets(*)
      double precision positionorb(3,*)  
      double precision positioneci(3,*)
    
DESCRIPTION:
   Transforms vector in Orbital coordinate system to vector in ECI coordinate
   system.

INPUTS:
   Name          Description            Units      Min               Max
   ----          -----------            -----      ---               --- 
   spacecraftTag Unique spacecraft      N/A        N/A               N/A
                 identifier       

   numValues     number of input        N/A        0                 any
                 time offsets

   asciiUTC      UTC start time in      N/A        1961-01-01        see NOTES
                 CCSDS ASCII Time
		 Code A or B format

   offsets       array of time          seconds    Max and Min such that
                 offsets                           asciiUTC+offset is between
		                                   asciiUTC Min and Max values

   positionORB 	 Coordinates or unit    meters      N/A        N/A
                 vector components in 
                 orbital reference
		 frame 

OUTPUTS:
   Name          Description            Units       Min        Max
   ----          -----------            ----        ---        ---
   positionECI   Coordinates or unit    meters     N/A               N/A  
                 vector components in 
                 ECI reference frame                            
                                                   

RETURNS:
   PGS_S_SUCCESS		 Success
   PGSCSC_E_SC_TAG_UNKNOWN	 Invalid Spacecraft tag
   PGSCSC_W_BELOW_SURFACE 	 vector magnitude indicates subsurface 
                                 location specified
   PGSCSC_W_BAD_TRANSFORM_VALUE  one or more values in transformation
                                 could not be determined
   PGSTD_E_TIME_FMT_ERROR        format error in asciiUTC
   PGSTD_E_TIME_VALUE_ERROR      value error in asciiUTC
   PGSTD_E_NO_LEAP_SECS          no leap seconds correction available for
                                 input time
   PGSTD_W_PRED_LEAPS            a predicted leap second value was used
                                 for at least one of the input offset times
   PGSMEM_E_NO_MEMORY            no memory is available to allocate vectors
   PGSEPH_E_BAD_EPHEM_FILE_HDR   no s/c ephem files had readable headers
   PGSEPH_E_NO_SC_EPHEM_FILE     no s/c ephem files could be found for input
                                 times
                  
EXAMPLES:
C:
   #define   ARRAY_SIZE   3

   PGSt_SMF_status    returnStatus;
   PGSt_integer       numValues;
   char               asciiUTC[28];
   PGSt_double        offsets[ARRAY_SIZE] = {3600.0,7200.0,10800.0};
   PGSt_double        positionORB[ARRAY_SIZE][3] = {
                                                    {0.5,0.75,0.90,0.3,0.2,0.8},
                                                    {0.65,1.2,3.65,0.1,3.2,1,7},
						    {0.98,2.6,4,78,0.2,1.5,0.9}
						   };
   PGSt_double        positionECI[ARRAY_SIZE][3];
  
   numValues = ARRAY_SIZE;
   strcpy(asciiUTC,"1991-01-01T11:29:30.123211Z");
   returnStatus = PGS_CSC_ORBtoECI (TRMM,numValues,asciiUTC,offsets,positionORB,
                                    positionECI)
                                                                      
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }

FORTRAN:
      implicit none
      integer             pgs_csc_orbtoeci
      integer             returnstatus
      integer             numvalues
      character*27        asciiutc 
      double precision    offsets(3)
      double precision    positionorb(3,3)
      double precision    positioneci(3,3)
      integer             cnt1
      integer             cnt2
      character*33        err
      character*241       msg
	
      data offsets/3600.0, 7200.0, 10800.0/

      do 10 cnt1 = 1,3
          do 10 cnt2 = 1,3
	      positionorb(cnt1,cnt2) = 100 * cnt1 * cnt2
   10 continue

      asciiutc = '1991-07-27T11:04:57.987654Z'
      numvalues = 3
	
      returnstatus = pgs_csc_orbtoeci(trmm, numvalues, asciiutc, offsets, 
     >                                positionorb, positioneci)

      if (returnstatus .ne. pgs_s_success) then
	  pgs_smf_getmsg(returnstatus, err, msg)
	  write(*,*) err, msg
      endif
                                                  
NOTES:
   TIME ACRONYMS:
     
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file, which is updated when a new
     leap second event is announced, contains actual (definitive) and predicted
     (long term; very approximate) dates of leap second events.  The latter can
     be used only in simulations.   If an input date is outside of the range of
     dates in the file (or if the file cannot be read) the function will use
     a calculated value of TAI-UTC based on a linear fit of the data known to be
     in the table.  This value of TAI-UTC is relatively crude estimate and may
     be off by as much as 1.0 or more seconds.  Thus, when the function is used
     for dates in the future of the date and time of invocation, the user ought
     to carefully check the return status.  The status level of the return 
     status of this function will be 'W' (at least) if the TAI-UTC value used
     is a predicted value.  The status level will be 'E' if the TAI-UTC value
     is calculated (although processing will continue in this case, using the
     calculated value of TAI-UTC).

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

REQUIREMENTS:
   PGSTK - 1050 

DETAILS:
   The function first gets the position and velocity vectors for the specified
   spacecraft at the specified times.

   Next the function checks the input vector to see if it is a unit vector.  If
   so, it is assumed that the user wishes only to transform its direction.  If
   not, it is assumed that the vector locates some point of interest (for
   example, a TDRSS satellite, or a lookpoint).  Thus, for that case a
   translation to the ECI coordinate system origin is performed following the
   rotation.  Aberration correction is also performed in both cases, except
   that in the second case, points within 120 m of spacecraft center are
   deemed to be on the spacecraft and are not aberrated.

   Points are checked to make sure they are not subterranean but no other 
   visibility check is performed (such as line-of-sight).

   See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
   ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
   for more information on the algorithm.

GLOBALS:
   None

FILES:
   None

FUNCTIONS CALLED:
   PGS_EPH_EphemAttit()
   PGS_EPH_ManageMasks()
   PGS_CSC_getORBtoECIquat()
   PGS_CSC_quatRotate()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()    
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()

END_PROLOG
*******************************************************************************/

#include <string.h>
#include <PGS_EPH.h>
#include <PGS_CSC.h>
#include <PGS_MEM.h>

/* constants */

#define SPEED_OF_LIGHT  299792458.0     /* speed of light (m/s) */
#define EARTH_RAD  6378137.0            /* WGS84 Earth equatorial radius (m) */

/* name of this function */

#define  FUNCTION_NAME  "PGS_CSC_ORBtoECI()"

PGSt_SMF_status
PGS_CSC_ORBtoECI(                       /* transform vector from ECI to ORB */
    PGSt_tag        spacecraftTag,      /* unique spacecraft ID */
    PGSt_integer    numValues,          /* number of time offsets */
    char            asciiUTC[28],       /* start time in CCSDS ASCII format */
    PGSt_double     offsets[],          /* array of time offsets */
    PGSt_double     positionORB[][3],   /* array of ORB vectors input */
    PGSt_double     positionECI[][3])   /* array of equiv. ECI vectors output */
{
    PGSt_double     (*scPosECI)[3];     /* spacecraft ECI position (meters) */
    PGSt_double     (*scVelECI)[3];       /* spacecraft ECI velocity (m/s) */
    PGSt_double     quatORBtoECI[4];    /* quat. defining ECI to ORB rotation */
    PGSt_double     norm;               /* norm of position vectors */
    PGSt_double     vDivc[3];           /* velocity divided by the speed of
					   light*/
    PGSt_integer    (*qualityFlags)[2]; /* spacecraft ephemeris quality flags */
    PGSt_integer    qualityFlagsMasks[2]; /* user definable quality flag masks
					     used to check validity of data
					     returned by PGS_EPH_EphemAttit() */
    PGSt_integer    maxValues;          /* number of vectors to process */

    void            *memPtr=NULL;       /* pointer to dynamically allocated
					   memory */
    PGSt_integer    cnt;                /* loop counter */

    short           iXYZ;               /* loop counter (vector indices) */

    PGSt_SMF_status returnStatus;       /* return status of this function */
    PGSt_SMF_status returnStatus1;      /* return status of function calls */

    char         mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							 PGS_SMF_GetMsg() */
    char         msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
     					                 PGS_SMF_GetMsg() */
    
    /* Allocate space for scratch arrays.  This includes qualityFlags, scPosECI,
       scVelECI. */
    
    maxValues = (numValues < 2) ? 1 : numValues;
    
    returnStatus1 = PGS_MEM_Calloc(&memPtr, maxValues, (3+3)*sizeof(PGSt_double)
				                       +2*sizeof(PGSt_integer));

    if (returnStatus1 != PGS_S_SUCCESS)
	return returnStatus1;
    
    /* Asign space for qualityFlags - quality flags for the position data
       returned by PGS_EPH_EphemAttit() (called below). */

    qualityFlags = (PGSt_integer (*)[2]) memPtr;
    
    /* Asign space for scPosECI - Spacecraft ECI position vector and
       scVelECI - Spacecraft ECI velocity vector */

    scPosECI = (PGSt_double (*)[3]) (qualityFlags + maxValues);
    scVelECI = scPosECI + maxValues;
    
    /* get the spacecraft position and velocity at each of the input times so
       that the ORB coordinate system can be determined */

    returnStatus = PGS_EPH_EphemAttit(spacecraftTag,numValues,asciiUTC,offsets,
				      PGS_TRUE,PGS_FALSE,qualityFlags,scPosECI,
				      scVelECI,NULL,NULL,NULL);
    switch (returnStatus)
    {
      case PGSEPH_W_BAD_EPHEM_VALUE:
	returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;
      case PGS_S_SUCCESS:
      case PGSTD_W_PRED_LEAPS:
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_GetMsg(&returnStatus1, mnemonic, msg);
	if (returnStatus1 != returnStatus)
	  PGS_SMF_GetMsgByCode(returnStatus,msg);   
	break;
      case PGSEPH_E_BAD_EPHEM_FILE_HDR:
      case PGSEPH_E_NO_SC_EPHEM_FILE:
      case PGSTD_E_SC_TAG_UNKNOWN:
      case PGSEPH_E_BAD_ARRAY_SIZE:
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
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
    
    /* for each time requested transform the corresponding input vector from the
       ECI to the ORB coordinate system */

    for (cnt=0;cnt<maxValues;cnt++)
    {
	/* don't process bad values from call to PGS_EPH_EphemAttit(), set the
	   corresponding positionECI vector components to PGSd_GEO_ERROR_VALUE.
	   */

	if ((qualityFlagsMasks[0] & qualityFlags[cnt][0]) != 0)
	{
	    for (iXYZ=0;iXYZ<3;iXYZ++)
	    {
		positionECI[cnt][iXYZ] = PGSd_GEO_ERROR_VALUE;
	    }
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;
		sprintf(msg,"one or more points was not processed due to "
			"quality of ephemeris data");
	    }
	    continue;
	}
	
	/* Get the quaternion that defines the rotation from the ECI to the ORB
	   coordinate system.  If unsuccessful set all components of return
	   vector to PGS_GEO_ERROR_VALUE and process next vector. */

	returnStatus1 = PGS_CSC_getORBtoECIquat(scPosECI[cnt],scVelECI[cnt],
						quatORBtoECI);
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGSCSC_E_QUAT_NOT_FOUND:
	  case PGS_E_TOOLKIT:
	    if (returnStatus != PGSCSC_W_BAD_TRANSFORM_VALUE)
	    {
		returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;
		PGS_SMF_GetMsgByCode(returnStatus,msg);
	    }
	    for (iXYZ=0;iXYZ<3;iXYZ++)
		positionECI[cnt][iXYZ] = PGS_GEO_ERROR_VALUE;
	    continue;
	  default:
	    PGS_MEM_Free(memPtr);
	    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
	
	/* rotate the vector from its ORB coords. to its ECI coordinates */

	returnStatus1 = PGS_CSC_quatRotate(quatORBtoECI,positionORB[cnt],
					   positionECI[cnt]);
	
	if (returnStatus1 != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	    PGS_MEM_Free(memPtr);
	    return PGS_E_TOOLKIT;
	}
	
	/* Compute the norm of the input vector positionORB. */
	
	norm = PGS_CSC_Norm(positionORB[cnt]);
	
	/* Check if the norm of the input vector is between 0.99999 and 1.00001 .
	   If it is, then don't do a translation. Either way, correct for
	   aberration. */
        
	for(iXYZ=0; iXYZ<3; iXYZ++)
	    vDivc[iXYZ] = scVelECI[cnt][iXYZ]/SPEED_OF_LIGHT;
	
	if (norm < 0.99999 || norm > 1.00001)
	{
	    /* if length of input vector is greater than 120 m then do 
	       aberration correction */
	    
	    if (norm > 120.0)
		for(iXYZ=0; iXYZ<3; iXYZ++) 
		    positionECI[cnt][iXYZ] = positionECI[cnt][iXYZ] -
                                             norm*vDivc[iXYZ];  

	    /* do the translation */
	    
	    for (iXYZ=0;iXYZ<3;iXYZ++)
		positionECI[cnt][iXYZ] = positionECI[cnt][iXYZ] +
                                         scPosECI[cnt][iXYZ];

	    /* Compute the norm of the output vector positionECI, if it is
	       indicates a location below the Earth's surface return a warning
	       to that effect.   This is a low priority message so only return
	       this warning if the return status is PGS_S_SUCCESS. */
	    
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		norm = PGS_CSC_Norm(positionECI[cnt]);
		if ( norm < (0.98*EARTH_RAD) )
		{
		    returnStatus = PGSCSC_W_BELOW_SURFACE;
		    PGS_SMF_GetMsgByCode(returnStatus,msg);
		}
	    }
	}     /* END: if (norm < 0.99999 || norm > 1.00001) */
	else
	{
	    for(iXYZ=0; iXYZ<3; iXYZ++)
		positionECI[cnt][iXYZ] = positionECI[cnt][iXYZ] - vDivc[iXYZ];
	    
	    norm = PGS_CSC_Norm(positionECI[cnt]);
            
	    for(iXYZ=0; iXYZ<3; iXYZ++)
		positionECI[cnt][iXYZ] = positionECI[cnt][iXYZ]/norm;
	}
    }     /* END: for (cnt=0;cnt<maxValues;cnt++) */
    
    if (returnStatus == PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,FUNCTION_NAME);
    }
    else
    {
	PGS_SMF_SetDynamicMsg(returnStatus, msg, FUNCTION_NAME);
    }
    
    PGS_MEM_Free(memPtr);
    return returnStatus;
}
