/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_ORBtoSC.c

DESCRIPTION:
   This file contains the function PGS_CSC_ORBtoSC().
   Transforms vector from Orbital reference frame vector to a vector in 
   Spacecraft reference frame.
  
AUTHOR:
   Peter Noerdlinger  / Applied Research Corporation
   Anubha Singhal     / Applied Research Corporation
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   27-Sep-1994 PDN  Designed				
   30-Sep-1994 AS   Coded
   20-Mar-1994 GTSK Replaced multiple calls to malloc() with single call to
                    PGS_MEM_Calloc().
   15-Jun-1995 GTSK Reworked algorithm which was calculating SC to ORB!
   21-Jul-1995 GTSK Fixed up rotation order (was wrong for non-TRMM cases).
   20-Jun-1996 PDN  Removed special transformation from tipped to orbital
                    coordinates for TRMM as this work is now in pre-processing.
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
   Transform from Orbital Frame to Spacecraft Frame

NAME:   
   PGS_CSC_ORBtoSC()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_ORBtoSC(  
              PGSt_tag     spacecraftTag,
              PGSt_integer numValues,
              char         asciiUTC[28], 
              PGSt_double  offsets[],  
              PGSt_double  posORB[][3],
              PGSt_double  posSC[][3])
	      
FORTRAN:
      include 'PGS_MEM_7.f'
      include 'PGS_EPH_5.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'

      integer function pgs_csc_orbtosc(spacecrafttag,numvalues,asciiutc,
     >                                 offsets,posorb,possc)
      integer          spacecrafttag
      integer          numvalues
      character*27     asciiutc
      double precision offsets(*)
      double precision posorb(3,*)
      double precision possc(3,*)  
    
DESCRIPTION:
   Transforms vector from Orbital reference frame vector to a vector in 
   Spacecraft reference frame.

INPUTS:
   Name          Description         Units    Min                 Max
   ----          -----------         -----    ---                 --- 
   spacecraftTag Unique spacecraft   N/A      N/A                 N/A
                 identifier       

   numValues     number of input     N/A      0                   any
                 time offsets

   asciiUTC      UTC start time in   N/A      1960-01-01          see NOTES

                 CCSDS ASCII Time             
		 Code A or B format

   offsets       array of time       seconds  Max and Min such that
                 offsets                      asciiUTC+offset is 
		                              between asciiUTC Min
					      and Max values

   posORB        Coordinates or unit meters    N/A        N/A  
                 vector components in 
                 Orbital reference frame                            
                                                   
OUTPUTS:
   Name          Description                     Units       Min        Max
   ----          -----------                     ----        ---        ---
   posSC  	 Coordinates or unit vector      meters      N/A        N/A
                 components in spacecraft
                 reference frame                   
                                                  
RETURNS:
   PGS_S_SUCCESS		    Success
   PGSCSC_E_SC_TAG_UNKNOWN	    Invalid Spacecraft tag
   PGSCSC_W_BELOW_SURFACE 	    vector magnitude indicates subsurface 
                                    location specified
   PGSCSC_W_BAD_TRANSFORM_VALUE     one or more values in transformation
                                    could not be determined
   PGSTD_E_TIME_FMT_ERROR           format error in asciiUTC
   PGSTD_E_TIME_VALUE_ERROR         value error in asciiUTC
   PGSTD_E_NO_LEAP_SECS             no leap seconds correction available for
                                    input time
   PGSMEM_E_NO_MEMORY               no memory is available to allocate vectors
   PGSEPH_E_BAD_EPHEM_FILE_HDR      no s/c ephem files had readable headers
   PGSEPH_E_NO_SC_EPHEM_FILE        no s/c ephem files could be found for input
                                    times

EXAMPLES:
C:
   #define   ARRAY_SIZE   3

   PGSt_SMF_status    returnStatus;
   PGSt_integer       numValues;
   char               asciiUTC[28];
   PGSt_double        offsets[ARRAY_SIZE] = {3600.0,7200.0,10800.0};
   PGSt_double        posORB[ARRAY_SIZE][3] = {
                                                   {0.5,0.75,0.90,0.3,0.2,0.8},
                                                   {0.65,1.2,3.65,0.1,3.2,1,7},
						   {0.98,2.6,4,78,0.2,1.5,0.9}
					      };
   PGSt_double        posSC[ARRAY_SIZE][3];				     
  
   numValues = ARRAY_SIZE;
   strcpy(asciiUTC,"1991-01-01T11:29:30.123211Z");
   returnStatus = PGS_CSC_ORBtoSC(PGSd_TRMM,numValues,asciiUTC,offsets,posORB,
                                  posSC)
                                                                      
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }

FORTRAN:
      implicit none
      integer		  pgs_csc_orbtosc
      integer		  returnstatus
      integer		  numvalues
      character*27        asciiutc 
      double precision    offsets(3)
      double precision    posorb(3,3)
      double precision    possc(3,3)
      integer             cnt1
      integer             cnt2
      character*33 	  err
      character*241 	  msg
	
      data offsets/3600.0, 7200.0, 10800.0/

      do 10 cnt1 = 1,3
          do 10 cnt2 = 1,3
              posorb(cnt1,cnt2) = 100 * cnt1 * cnt2
   10 continue

      asciiutc = '1991-07-27T11:04:57.987654Z'
      numvalues = 3
      
      returnstatus = pgs_csc_orbtosc(pgsd_trmm,numvalues, asciiutc, 
     >                               offsets, posorb, possc)

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
   PGSTK - 0680, 1050 

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   NONE

FILES:
   the spacecraft ephemeris is required by the ephemeris access tool 
   earthfigure.dat

FUNCTIONS CALLED:
   PGS_EPH_Ephem_Attit()
   PGS_EPH_ManageMasks()
   PGS_CSC_quatRotate()
   PGS_CSC_EulerToQuat()
   PGS_CSC_TiltYaw()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()    
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()

END_PROLOG
*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <PGS_CSC.h>
#include <PGS_EPH.h>
#include <PGS_MEM.h>

/* name of this function */

#define  FUNCTION_NAME "PGS_CSC_ORBtoSC()"

PGSt_SMF_status
PGS_CSC_ORBtoSC(                    /* converts from Orbital coordinate system 
				       to Spacecraft coordinate system */ 
    PGSt_tag	 spacecraftTag,     /* unique spacecraft identifier */ 
    PGSt_integer numValues,         /* number of values requested */
    char         asciiUTC[28],      /* UTC start time in CCSDS ASCII Time Code A
				       format */ 
    PGSt_double  offsets[],         /* time offsets relative to asciiUTC */
     
    PGSt_double	 posORB[][3],       /* coordinates in Orbital reference frame */
    PGSt_double	 posSC[][3])        /* coordinates in SC reference frame */ 
{    
    PGSt_double  (*attitQuat)[4];   /* spacecraft to ECI rotation quaternion */
    PGSt_integer (*qualityFlags)[2];/* quality flags (pos/attitude) for each set
				       of ephemeris values returned */
    PGSt_double  (*eulerAngle)[3];  /* Euler Angles in the given order */
    PGSt_double  (*angularVelocity)[3];
                                    /* J2000 anguler velocity proj. on the 
                                       body axes */
    PGSt_double  orbitalQuat[4];    /* orbital quaternion */

    PGSt_integer maxValues;         /* counter cannot equal or exceed this 
                                       value */	
    PGSt_integer cnt1;              /* loop counter */
    PGSt_integer qualityFlagsMasks[2]; /* user definable quality flag masks
					  used to check validity of data
					  returned by PGS_EPH_EphemAttit() */

    int          cnt2;              /* loop counter */

    void         *memPtr=NULL;      /*pointer to memory space allocated
				      dynamically for scratch arrays used 
				      internally by this function */

    char         mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							 PGS_SMF_GetMsg() */
    char         msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
     					                 PGS_SMF_GetMsg() */
    PGSt_scTagInfo  scData;         /* s/c data structure */

    PGSt_SMF_status returnStatus;   /* error returnStatus of function call */
    PGSt_SMF_status returnStatus1;  /* error status of call to second or later
                                       functions */
    PGSt_SMF_status code;           /* status code returned by 
				       PGS_SMF_GetMsg()*/
       
    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS;

    /* initialize pointers to NULL */

    attitQuat = NULL;
    qualityFlags = NULL;
    eulerAngle = NULL;
    angularVelocity = NULL;
    
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

    /*** Check for valid Spacecraft tag ***/

    returnStatus = PGS_EPH_GetSpacecraftData(spacecraftTag, NULL,
					     PGSe_TAG_SEARCH, &scData);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSTD_E_SC_TAG_UNKNOWN:
	PGS_SMF_SetStaticMsg(PGSCSC_E_SC_TAG_UNKNOWN, FUNCTION_NAME);
	return PGSCSC_E_SC_TAG_UNKNOWN;
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* Allocate space for scratch arrays.  This includes qualityFlags, eulerAngle,
       angularVelocity, attitQuat. */
    
    returnStatus = PGS_MEM_Calloc(&memPtr, 1,
				  maxValues*(sizeof(PGSt_integer)*2 +
					     sizeof(PGSt_double)*(4+3+3)));
    
    if (returnStatus != PGS_S_SUCCESS)
	return returnStatus;
	
    /* Asign space for qualityFlags - quality flags for the position data
       returned by PGS_EPH_EphemAttit() (called below). */

    qualityFlags = (PGSt_integer (*)[2]) memPtr;
    
    /* Asign space for attitQuat - Spacecraft coordinate system to ECI rotation
       quaternion. */

    attitQuat = (PGSt_double (*)[4]) (qualityFlags + maxValues);

    /* Asign space for eulerAngle - Spacecraft Attitude Euler Angles, angularVelocity -
       Spacecraft Attitude Euler Angle Rates. */

    eulerAngle = (PGSt_double (*)[3]) (attitQuat + maxValues);
    angularVelocity = eulerAngle + maxValues;

    /*** Get attitude quaternions and position and velocity vectors 
         If an error occurs return ***/  
 
    returnStatus = PGS_EPH_EphemAttit(spacecraftTag,numValues,asciiUTC,offsets,
				      PGS_FALSE,PGS_TRUE,qualityFlags,
				      NULL,NULL,eulerAngle,angularVelocity,attitQuat);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
        break;
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
        PGS_MEM_Free(memPtr);
        PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
        return returnStatus;
      case PGS_E_TOOLKIT:
        PGS_MEM_Free(memPtr);
        return PGS_E_TOOLKIT;
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
    
    for(cnt1=0;cnt1 < maxValues;cnt1++)
    {
	if (((qualityFlagsMasks[0] & qualityFlags[cnt1][0]) != 0) &&
	    ((qualityFlagsMasks[1] & qualityFlags[cnt1][1]) != 0))
	{
            for (cnt2 = 0 ; cnt2 < 3; cnt2++)
	    {
		posSC[cnt1][cnt2] = PGSd_GEO_ERROR_VALUE;
	    }
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;
		sprintf(msg,"one or more points was not processed due to "
			"quality of ephemeris/attitude data");
	    }
	    continue;
        }

	/* call PGS_CSC_EulertoOrbit_Quat to get the quaternion defining the
	   rotation from the spacecraft to the orbital (or tip (TRMM))
	   reference frame */
	
	returnStatus1 = PGS_CSC_EulerToQuat(eulerAngle[cnt1], scData.eulerAngleOrder,
					    orbitalQuat);
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:

	    /* The call above returns a quaternion defining the rotation from
	       the spacecraft to orbital (or tip) reference frame.  The rotation
	       from orbital (or tip) to spacecraft is required, this is just
	       the inverse of the quaternion that was returned. */

	    for (cnt2 = 1 ; cnt2 < 4 ; cnt2++)
		orbitalQuat[cnt2] = -orbitalQuat[cnt2];
	
	    break;
	  default: 
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);   
		returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;    
	    }
	    for (cnt2=0; cnt2 < 3; cnt2++)
		posSC[cnt1][cnt2] = PGSd_GEO_ERROR_VALUE;
	    continue;
	}

	/* call PGS_CSC_quatRotate to transform the vector from the Orbital
	   coordinate system to the rotated SC coordinate system where the
	   rotation is defined by a quaternion */
	
	returnStatus1 = PGS_CSC_quatRotate(orbitalQuat,posORB[cnt1],
					   posSC[cnt1]);

	switch (returnStatus1)  
	{ 
	  case PGS_S_SUCCESS: 
	    break;
	  default: 
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);   
		returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;    
	    }
	    for (cnt2=0; cnt2 < 3; cnt2++)
		posSC[cnt1][cnt2] = PGSd_GEO_ERROR_VALUE;
	    continue;
	}  
    }

    PGS_MEM_Free(memPtr);
    
    if (returnStatus == PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }
    else
    {
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
    }
    
    return returnStatus;
}


























































































































