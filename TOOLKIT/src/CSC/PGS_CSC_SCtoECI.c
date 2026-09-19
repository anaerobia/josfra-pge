/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_SCtoECI.c

DESCRIPTION:
   This file contains the function PGS_CSC_SCtoECI().
   
AUTHOR:
   Peter Noerdlinger  / Applied Research Corporation
   Snehevadan Macwan  / Applied Research Corporation
   Anubha Singhal     / Applied Research Corporation
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   01-Feb-1994 PDN  Designed				
   04-Feb-1994 SM   Coded
   29-Mar-1994 PDN  Embellished comments		        
   30-Sep-1994 AS   Updated to accept time offsets,calling sequences updated, 
                    types updated, variable names updated,error messages 
                    updated. Modified prologs and comply with latest ECS/PGS 
                    standards. 
   20-Mar-1994 GTSK Replaced multiple calls to malloc() with single call to
                    PGS_MEM_Calloc().
   06-Jun-1995 GTSK Spiffed up formatting, simplified code
   15-Jun-1995 GTSK Changed prolog
   10-Oct-1996 PDN  Fixed prolog, title
   12-Feb-1999 PDN  Fixed nomenclature consistent with AM1 and later spacecraft
                    note that the function is also OK for TRMM, but in that case
                    please observe that angularVelocity is really (yaw rate, 
                    pitch rate, and roll rate in that order). Also deleted 
                    references to predicted leap seconds, which were used in
                    early I&T work only.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG

TITLE:  
   This function transforms from Spacecraft to Earth Centered Inertial 
   Reference Frames

NAME:   
   PGS_CSC_SCtoECI()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_SCtoECI(  
              PGSt_tag     spacecraftTag,
              PGSt_integer numValues,
              char         asciiUTC[28], 
              PGSt_double  offsets[],  
              PGSt_double  posSC[][3],
              PGSt_double  posECI[][3])
	      
FORTRAN:
      include 'PGS_MEM_7.f'
      include 'PGS_EPH_5.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'

      integer function pgs_csc_sctoeci(spacecraftTag,numvalues,asciiutc,
     >                                 offsets,possc,poseci)
      integer          spacecrafttag
      integer          numvalues
      character*27     asciiutc
      double precision offsets(*)
      double precision possc(3,*)
      double precision poseci(3,*)  
    
DESCRIPTION:
   Transforms vector in Spacecraft coordinate system to vector in ECI 
   coordinate system.

INPUTS: 
   Name          Description        Units Min                 Max
   ----          -----------        ----- ---                 --- 
   spacecraftTag Unique spacecraft  N/A   N/A                 N/A
                 identifier       

   numValues     number of input    N/A   0                   any
                 time offsets

   asciiUTC      UTC start time in  N/A   1961-01-01          see NOTES

                 CCSDS ASCII Time             
		 Code A or B format

   offsets       array of time      seconds  Max and Min such that
                 offsets                      asciiUTC+offset is 
		                              between asciiUTC Min
					      and Max values

   posSC         Coordinates or unit meters   N/A        N/A  
                 vector components in 
                 SC reference frame                            
                                                   
OUTPUTS:
   Name          Description                     Units       Min        Max
   ----          -----------                     ----        ---        ---
   posECI 	 Coordinates or unit vector      meters      N/A        N/A
                 components ECI
                 reference frame     
  
RETURNS:
   PGS_S_SUCCESS	         Success
   PGSCSC_E_SC_TAG_UNKNOWN       Invalid Spacecraft tag
   PGSCSC_W_BELOW_SURFACE        vector magnitude indicates subsurface 
                                 location specified
   PGSCSC_W_BAD_TRANSFORM_VALUE  one or more values in transformation
                                 could not be determined
   PGSTD_E_TIME_FMT_ERROR        format error in asciiUTC
   PGSTD_E_TIME_VALUE_ERROR      value error in asciiUTC
   PGSTD_E_NO_LEAP_SECS          no leap seconds correction available for
                                 input time
   PGSMEM_E_NO_MEMORY            no memory is available to allocate vectors
   PGSEPH_E_BAD_EPHEM_FILE_HDR   no s/c ephem files had readable headers
   PGSEPH_E_NO_SC_EPHEM_FILE     no s/c ephem files could be found for input
                                 times
   PGSEPH_E_NO_DATA_REQUESTED    both orb and att flags are set to false

EXAMPLES:
C:
   #define   ARRAY_SIZE   3

   PGSt_SMF_status    returnStatus;
   PGSt_integer       numValues;
   char               asciiUTC[28];
   PGSt_double        offsets[ARRAY_SIZE] = {3600.0,7200.0,10800.0};
   PGSt_double        posSC[ARRAY_SIZE][3] = {
                                                   {0.5,0.75,0.90,0.3,0.2,0.8},
                                                   {0.65,1.2,3.65,0.1,3.2,1,7},
						   {0.98,2.6,4,78,0.2,1.5,0.9}
						  };
   PGSt_double        posECI[ARRAY_SIZE][3];				     
  
   numValues = ARRAY_SIZE;
   strcpy(asciiUTC,"1991-01-01T11:29:30.123211Z");
   returnStatus = PGS_CSC_SCtoECI(PGSd_TRMM,numValues,asciiUTC,offsets,posSC,
                                   posECI)
                                                                      
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }

FORTRAN:
      integer		  pgs_csc_sctoeci
      integer		  returnstatus
      integer		  numvalues
      character*27        asciiutc 
      double precision    offsets(3)
      double precision    possc(3,3)
      double precision    poseci(3,3)
      integer             cnt1
      integer             cnt2
      character*33 	  err
      character*241 	  msg
      
      data offsets/3600.0, 7200.0, 10800.0/
      
      do 10 cnt1 = 1,3
          do 10 cnt2 = 1,3
               posveleci(cnt1,cnt2) = 100 * cnt1 * cnt2
  10  continue

      asciiutc = '1991-07-27T11:04:57.987654Z'
      numvalues = 3
	
      returnstatus = pgs_csc_sctoeci(pgsd_trmm,numvalues, asciiutc,
     >                               offsets, possc, poseci)

      if (returnstatus .ne. pgs_s_success) then
	   pgs_smf_getmsg(returnstatus, err, msg)
	   write(*,*) err, msg
      endif

NOTES:
   Certain checks are performed in the case of translation to ensure that the
   transformed point is not below the Earth's surface; other visibility checks
   (such as line-of-sight) are not performed.

   TIME ACRONYMS:
     
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file, which is updated when a 
     new leap second event is announced, contains dates of leap second events.
     If an input date is outside of the range of dates in the file (or if the 
     file cannot be read) the function PGS_TD_LeapSec() which reads the file
     will return a calculated value of TAI-UTC based on a linear fit of the data
     in the table as of late 1997.  This value of TAI-UTC is relatively crude 
     estimate and may be off by as much as 1.0 or more seconds. The warning 
     return status from PGS_TD_LeapSec() is promoted to an error level by any 
     Toolkit function receiving it, which will terminate the present function.

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
   PGSTK - 0680, 1050 

DETAILS:
      Basis of algorithm:

      The function first checks the input vector to see if it is a unit vector.
      If so, it is assumed that the user wishes only to transform its direction.
      If not, it is assumed that the vector locates some point of interest (for
      example, a TDRSS satellite, or a lookpoint). For that case a rotation to 
      ECI axes is performed first, and then a translation to the ECI coordinate 
      center. The function obtains the position vector and attitude quaternions
      from the ephemeris access tool. It proceeds to do the rotation, and then,
      if called for the translation. Aberration correction is also performed if 
      the input is a unit vector or is in meters and represents a point more 
      than 120 m from spacecraft center.

      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   NONE

FILES:
   The function PGS_EPH_Ephem_Attit() requires:
      spacecraft ephemeris file(s)
      leapsec.dat

FUNCTIONS CALLED:
   PGS_EPH_Ephem_Attit()
   PGS_EPH_ManageMasks()
   PGS_CSC_quatRotate()
   PGS_CSC_Norm()
   PGS_MEM_Calloc()
   PGS_MEM_Free()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()    
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()
        
END_PROLOG
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_EPH.h>
#include <PGS_MEM.h>

/* name of this function */

#define FUNCTION_NAME  "PGS_CSC_SCtoECI()"

/* constants */

#define PGS_RE          6378130.0
#define SPEED_OF_LIGHT  299792458.0

PGSt_SMF_status
PGS_CSC_SCtoECI(                    /* converts from Spacecraft coordinate
				       system to ECI coordinate system */ 
    PGSt_tag	 spacecraftTag,     /* unique spacecraft identifier */ 
    PGSt_integer numValues,         /* number of values requested */
    char         asciiUTC[28],      /* UTC start time in CCSDS ASCII Time
				       Code A format */ 
    PGSt_double  offsets[],         /* time offsets relative to asciiUTC */
    
    PGSt_double	 posSC[][3],        /* coordinates in SC reference frame */
    PGSt_double	 posECI[][3])       /* coordinates in ECI reference frame */ 
{
    PGSt_double	 norm;	            /* norm of input vector (posECI) */
        
    PGSt_double  (*attitQuat)[4];   /* spacecraft to ECI rotation quaternion */
    PGSt_double  (*positionECI)[3]; /* ECI position */
    PGSt_double  (*velocityECI)[3]; /* ECI velocity */
    PGSt_integer (*qualityFlags)[2];/* quality flags (pos/attitude) for each set
				       of ephemeris values returned */

    PGSt_double  (*eulerAngle)[3];  /* Euler Angles in the given order */
    PGSt_double  (*angularVelocity)[3];
                                    /* J2000 angular velocity proj. on the 
                                       body axes */
    PGSt_integer cnt1;              /* loop counter */
    short        cnt2;              /* loop counter */
    PGSt_integer maxValues;         /* counter cannot equal or exceed this 
                                       value */	
    PGSt_integer qualityFlagsMasks[2]; /* user definable quality flag masks
					  used to check validity of data
					  returned by PGS_EPH_EphemAttit() */
    PGSt_double  vDivc[3];          /* velocity divided by the speed of light*/
  
    void         *memPtr=NULL;      /* pointer to memory space allocated
				       dynamically for scratch arrays used 
				       internally by this function */
    char         mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							 PGS_SMF_GetMsg() */
    char         msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
     					                 PGS_SMF_GetMsg() */
    PGSt_SMF_status returnStatus;   /* error returnStatus of function call */
    PGSt_SMF_status returnStatus1;  /* error status of call to second or later
                                       functions */
    PGSt_SMF_status code;           /* status code returned by 
				       PGS_SMF_GetMsg()*/
    
    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS;  
    
    /* Test size of arrays */

    if ( numValues < 0 )
    {
	/* NO negative array size is allowed, return with an error status. */

	returnStatus = PGSCSC_E_BAD_ARRAY_SIZE;
	PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME );
	return  returnStatus;
    }
    else if (numValues == 0)
      maxValues = 1;
    else 
      maxValues = numValues; 

    /* Allocate space for scratch arrays.  This includes qualityFlags, 
       positionECI, velocityECI, eulerAngle, angularVelocity, attitQuat. */
    
    returnStatus = PGS_MEM_Calloc(&memPtr, 1,
				  maxValues*(sizeof(PGSt_integer)*2 +
					     sizeof(PGSt_double)*(3+3+3+3+4)));
    
    if (returnStatus != PGS_S_SUCCESS)
	return returnStatus;
    
    /* Assign space for qualityFlags - quality flags for the position data
       returned by PGS_EPH_EphemAttit() (called below). */

    qualityFlags = (PGSt_integer (*)[2]) memPtr;
    
    /* Assign space for positionECI - Spacecraft ECI position vector, velocityECI
       - Spacecraft ECI velocity vector, eulerAngle - Spacecraft Attitude Euler 
       Angles, angularVelocity - Spacecraft angular vel. in J2000, projected 
       on the body axes in the order roll, pitch, yaw. (for TRMM - the yaw, pitch,
       and roll rates) */

    positionECI = (PGSt_double (*)[3]) (qualityFlags + maxValues);
    velocityECI = positionECI + maxValues;
    eulerAngle = velocityECI + maxValues;
    angularVelocity = eulerAngle + maxValues;
    
    /* Assign space for attitQuat - Spacecraft coordinate system to ECI rotation
       quaternion. */

    attitQuat = (PGSt_double (*)[4]) (angularVelocity + maxValues);
        
    /*** Get attitude quaternions and position and velocity vectors 
         If an error occurs return ***/  
 
    returnStatus = PGS_EPH_EphemAttit(spacecraftTag,
				      numValues,
				      asciiUTC,
				      offsets,
				      PGS_TRUE,
				      PGS_TRUE,
				      qualityFlags,
				      positionECI,
				      velocityECI,
				      eulerAngle,
				      angularVelocity,
				      attitQuat);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
        break;
      case PGSTD_E_SC_TAG_UNKNOWN:
        PGS_MEM_Free(memPtr);
	PGS_SMF_SetStaticMsg(PGSCSC_E_SC_TAG_UNKNOWN, FUNCTION_NAME);
	return PGSCSC_E_SC_TAG_UNKNOWN;
      case PGSEPH_W_BAD_EPHEM_VALUE:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
        if(code != returnStatus)
	  PGS_SMF_GetMsgByCode(returnStatus,msg); 
        returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;
	break;
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
        if(code != returnStatus)
	  PGS_SMF_GetMsgByCode(returnStatus,msg);  
        break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSEPH_E_BAD_EPHEM_FILE_HDR:
      case PGSEPH_E_NO_SC_EPHEM_FILE:
      case PGSEPH_E_NO_DATA_REQUESTED:
        PGS_MEM_Free(memPtr);
        PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
        return returnStatus;
      default:
        PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
        PGS_MEM_Free(memPtr);
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

    /* make sure the qualityFlags masks for ephemeris data and attitude data
       contain the "no data" bit set by the toolkit */

    qualityFlagsMasks[0] = qualityFlagsMasks[0] | PGSd_NO_DATA;
    qualityFlagsMasks[1] = qualityFlagsMasks[1] | PGSd_NO_DATA;
    
    for(cnt1=0; cnt1<maxValues; cnt1++)
    {
	if (((qualityFlagsMasks[0] & qualityFlags[cnt1][0]) != 0) &&
	    ((qualityFlagsMasks[1] & qualityFlags[cnt1][1]) != 0))
	{
            for (cnt2 = 0 ; cnt2 < 3; cnt2++)
	    {
		posECI[cnt1][cnt2] = PGSd_GEO_ERROR_VALUE;
	    }
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;
		sprintf(msg,"one or more points was not processed due to "
			"quality of ephemeris/attitude data");
	    }
	    continue;
        }

	/*** Compute the norm of the input vector posSC ***/
	
	norm = PGS_CSC_Norm(posSC[cnt1]);
	
	/* call PGS_CSC_quatRotate to transform the vector from SC to the
	   rotated ECI coordinate system where the rotation is defined by 
	   a quaternion */
	
	returnStatus1 = PGS_CSC_quatRotate(attitQuat[cnt1], posSC[cnt1],
					   posECI[cnt1]);
	
	switch (returnStatus1)  
	{ 
	  case PGS_S_SUCCESS: 
	    break;
	  case PGS_E_TOOLKIT:
	    return returnStatus1;
	  default: 
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);   
		returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;    
	    }
	    for(cnt2=0;cnt2<3;cnt2++)
		posECI[cnt1][cnt2] = PGSd_GEO_ERROR_VALUE;
	    continue;
	}  
	
	for(cnt2=0; cnt2<3; cnt2++)
	    vDivc[cnt2] = velocityECI[cnt1][cnt2]/SPEED_OF_LIGHT;
	
	/* Check if the norm of the input vector is between 0.99999 and 1.00001,
	   if it is, then just do aberration correction and continue on to
	   process the next vector... */
        
	if (norm >= 0.99999 && norm <= 1.00001)
	{
	    for(cnt2=0;cnt2 < 3;cnt2++)
		posECI[cnt1][cnt2] = posECI[cnt1][cnt2] - vDivc[cnt2];
	    
	    norm = PGS_CSC_Norm(posECI[cnt1]);
            
	    for(cnt2=0;cnt2 < 3;cnt2++)
		posECI[cnt1][cnt2] = posECI[cnt1][cnt2] / norm;
	    
	    continue;
	}    
	    
	/* ...otherwise do a translation and aberration correction */
	    
	/* if length of input vector is greater than 120 m then do aberration
	   correction */
	    
	if (norm > 120.0)
	{
	    for(cnt2=0;cnt2 < 3;cnt2++) 
		posECI[cnt1][cnt2] = posECI[cnt1][cnt2] - 
		    norm * vDivc[cnt2];  
	}
	
	/* do the translation */
	
	for(cnt2=0;cnt2 < 3;cnt2++)
	    posECI[cnt1][cnt2] += positionECI[cnt1][cnt2]; 
	
	/* Check if the norm is <= 0.98 * (radius of earth) If it is, return
	   warning message indicating point is below the surface of the earth,
	   possible but unlikely to be what was intended */
	
	norm = PGS_CSC_Norm(posECI[cnt1]);
	
	if (norm < 0.98 * PGS_RE)
	{   
	    switch (returnStatus)
	    {
	      case PGSTD_E_NO_LEAP_SECS:
	      case PGSCSC_W_BELOW_SURFACE: 
		break;
	      default:
		returnStatus = PGSCSC_W_BELOW_SURFACE;
		PGS_SMF_GetMsgByCode(returnStatus,msg);        
	    }
	}
    } 	  /* END: for(cnt1=0; cnt1<maxValues; cnt1++) */

    PGS_MEM_Free(memPtr); 
    
    if (returnStatus == PGS_S_SUCCESS)
      PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    else
      PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
    
    return returnStatus;
}
