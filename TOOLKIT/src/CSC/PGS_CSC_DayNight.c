/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_DayNight.c

DESCRIPTION:
   This file contains the function PGS_CSC_DayNight(). 

AUTHOR:
   Peter D. Noerdlinger / Space Applications Corporation
   Deborah A. Foch      / Applied Research Corporation
   Anubha Singhal       / Applied Research Corporation
   Guru Tej S. Khalsa   / Applied Research Corporation

HISTORY:
   25-AUG-1994  PDN  Designed
   05-SEPT-1994 DAF  Completed coding
   08-SEPT-1994 DAF  Changed variable names and added call to ECItoECR
   11-SEPT-1994 DAF  Added error reporting and fixed compilation errors
   12-SEPT-1994 DAF  Made changes resulting from code inspection
   13-SEPT-1994 AS   Added function to free memory allocated
   16-SEPT-1994 AS   Added case to check if number of offsets is less than zero
   19-SEPT-1994 AS   Changed Astronomical Twilight from 106 degrees to 108
                     degrees
   20-SEPT-1994 PDN  Improved FORTRAN Synopsis and Examples
   24-SEPT-1994 DAF  Fixed prolog
   27-SEPT-1994 DAF  Updated FORTRAN example, functions called, files, and
                     error returns listed in prolog and a few odds and ends
   25-OCT-1994  DAF  updated afterDark min/max in prolog
   20-MAR-1994  GTSK Replaced multiple calls to malloc() with single call to
                     PGS_MEM_Calloc()
   29-Jun-1995  GTSK Removed return value PGSCSC_BELOW_HORIZON.  This is being
                     by the call to PGS_CSC_ZenithAzimuth, but should not be
                     returned to the user.  This function now returns
		     PGS_S_SUCCESS when PGS_CSC_ZenithAzimuth returns
		     PGS_CSC_BELOW_HORIZON.
   18-Jun-1996  PDN  Removed check for obsolete return value: 
                     PGSCSC_W_INTERIM_UT1; added check for invalid latitude
                     and setting Dynamic Message identifying offending offset
   25-Aug-1997  PDN  Removed return value from PGS_SMF_SetDynamicMsg

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Determine if Location on Earth is in Day or Night 

NAME:
   PGS_CSC_DayNight()

SYNOPSIS:
C:
#include <PGS_CBP.h>
 
   PGSt_SMF_status
   PGS_CSC_DayNight(
       PGSt_integer      numValues,
       char              asciiUTC[28],
       PGSt_double       offsets[],
       PGSt_double	 latitude[],
       PGSt_double       longitude[],
       PGSt_tag          sunZenithLimitTag,
       PGSt_boolean      afterDark[])

FORTRAN:
      include 'PGS_MEM_7.f'
      include 'PGS_CBP_6.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'

      integer function 
      pgs_csc_daynight(numvalues,asciiutc,offsets,latitude,longitude,
     >                 sunzenithlimittag,afterdark)
   
      integer            numvalues
      character*27       asciiutc
      double precision   offsets(*)
      double precision   latitude(*)
      double precision   longitude(*)
      integer            sunzenithlimittag
      integer            afterdark(*)

DESCRIPTION:
   This function determines whether each point in a set of 
   input Earth locations is in day or night at the corresponding
   input times. The function accepts an input start time, array of 
   offsets from that start time, and an array of corresponding geodetic
   latitudes and longitudes. It then determines whether each time and 
   point on the surface of the Earth (altitude = 0 km) is night, based
   on definitions of either civil twilight or night, nautical night, or
   astronomical night.


INPUTS:
   Name              Description          Units      Min           Max
   --------          -----------          -----      ---           ---
   numValues         number of input       N/A        0            any
                     time offsets,
		     longitudes, and
		     latitudes

   asciiUTC          UTC start time in     N/A   1979-06-30        see NOTES
                     CCSDS ASCII Time            (1961-001)
		     Code A or B format

   offsets           array of time        seconds   Max and Min such that
                     offsets                        asciiUTC+offset is 
		                                    between asciiUTC Min
						    and Max values

   latitude          array of Geodetic    radians   -pi/2         +pi/2
                     latitudes for array
		     of time offsets

   longitude         array of longitudes  radians   -2*pi          2*pi
                     corresponding to
		     time offsets

   sunZenithLimitTag tag specifying basis   N/A      N/A           N/A
                     of day/night 
		     determination
		       Allowed values:  
		          PGSd_CivilTwilight - (end of day) Sun deemed to set
			                       within 90 degrees 50 arc minutes
					       from zenith
			  PGSd_CivilNight    - (end of civil twilight) Sun more
			                       than 96 degrees from zenith.
					       (same as start of Nautic
					       twilight)
                          PGSd_NauticalNight - (end of Nautical twilight) Sun
			                       more than 102 degrees from
					       zenith.
                          PGSd_AstronNight   - (end of Astronomical Twilight)
			                       Sun more than 108 degrees from
					       zenith.

		     		      
OUTPUTS:
   Name               Description            Units       Min       Max
   ----               -----------            -----       ---       ---
   afterDark          array of answers:      boolean     see Description
                      array values will
                      be either PGS_TRUE
		      or PGS_FALSE,
		      according to the
		      tag definition.
		      PGS_TRUE means 
		      point is in night,
		      PGS_FALSE means		      
		      point is in daylight
		      or twilight.

RETURNS:
   PGS_S_SUCCESS                 successful return
   PGSTD_E_NO_LEAP_SECS          no leap second value available in table
                                 for at least one of the input offset times;
				 a linear approximation was used to get value
   PGSTD_W_PRED_LEAPS            a predicted leap second value
                                 was used for at least one of
				 the input offset times
   PGSCSC_E_INVALID_LIMITTAG     invalid sunZenithLimitTag  
   PGSCSC_E_BAD_ARRAY_SIZE       numValues (and array size) is less than zero
   PGSCSC_W_ERROR_IN_DAYNIGHT    an error occurred in computing at least
                                 one afterDark value         		
   PGSCSC_W_BAD_TRANSFORM_VALUE  invalid ECItoECR transformation
   PGSCSC_W_PREDICTED_UT1        at least one of the values obtained
                                 from the utcpole.dat file is 'predicted'
   PGSTD_E_NO_UT1_VALUE          no UT1-UTC correction available
   PGSTD_E_BAD_INITIAL_TIME      initial input time cannot be deciphered
   PGSCBP_E_TIME_OUT_OF_RANGE    start UTC time is not in the range of the
                                 planetary ephemeris file (de200.eos)
   PGSCBP_E_UNABLE_TO_OPEN_FILE  ephemeris file cannot be opened
   PGSMEM_E_NO_MEMORY            no memory available to allocate vectors
   PGS_E_TOOLKIT                 something unexpected happened, execution 
                                 of function ended prematurely
 
EXAMPLES:
C:
   #define   ARRAY_SIZE   3

   PGSt_SMF_status    returnStatus;
   PGSt_integer       numValues;
   PGSt_integer       counter;
   char               asciiUTC[28];
   PGSt_double        offsets[ARRAY_SIZE] = {3600.0,7200.0,10800.0};
   PGSt_double        latitude[ARRAY_SIZE]= {0.5,0.75,0.90};
   PGSt_double        longitude[ARRAY_SIZE] = {1.0,2.0,3.0};
   PGSt_tag           sunZenithLimitTag = PGSd_CivilTwilight;
   PGSt_boolean       afterDark[ARRAY_SIZE];
   
   numValues = ARRAY_SIZE;
   strcpy(asciiUTC,"1991-01-01T11:29:30");
   returnStatus = PGS_CSC_DayNight(numValues,asciiUTC,offsets,latitude,
                                   longitude,sunZenithLimitTag,afterDark);
                                      
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
       printf("Offset: %f   Latitude:%f   Longitude:%f  Day/Night:%u",
              offset[counter], latitude[counter], longitude[counter],
	      afterDark[counter]);
	      counter++;	      
   }
       
FORTRAN:
      integer            pgs_csc_daynight
      parameter          (array_size=3)
      integer            returnstatus
      integer            counter     
      integer            numvalues
      character*27       asciiutc
      double precision   offsets(array_size)
      double precision   latitude(array_size)
      double precision   longitude(array_size)
      integer            sunzenithlimittag
      integer            afterdark(array_size)

      data offsets/3600.0,7200.0,10800.0/
      data latitude/0.5,0.75,0.90/
      data longitude/1.0,2.0,3.0/
      numvalues = array_size
      asciiutc = '1991-01-01T11:29:30'
      sunzenithlimittag = pgsd_civiltwilight

      returnstatus = pgs_csc_daynight(numvalues,asciiutc,offsets,
     >                                latitude,longitude,
     >                                sunzenithlimittag,afterdark)
                                    
      if(returnstatus .ne. pgs_s_success) go to 90

      write(6,*) asciiutc
      if(numvalues.eq.0) numvalues = 1
      do 40 counter = 1,numvalues,1
          write(6,*) offsets(counter),latitude(counter),
                     longitude(counter),afterdark(counter)
  40  continue

  90  write(6,99)returnstatus
  99  format('ERROR:',I50)  

NOTES: 
   If there is an error in computing one or more of the afterDark values,
   which does not affect the computation of the other values for the
   input offset times, it is set to the returnStatus value.

   An Earth model tag is not needed because the latitude is
   geodetic.  Input latitude values should be based on an Earth model
   (flattening) consistent with that used for other data analysis and
   processing for the same spacecraft.

   User supplies one of the four sunZenithLimitTags as part of the 
   input information.  Users wishing to know if a point is in Nautical
   Twilight or darker should use the Civil Night tag;  those wishing to
   determine whether the point is after the start of Astronomical 
   Twilight should use the Nautical Night tag.  An example of tag usage
   is the following: if the tag is set to Nautical night and the Sun 
   zenith angle is less than 102 degrees, afterDark will be false; 
   if 102 degrees or more it will be true.
							      	
   TIME ACRONYMS:
     
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file utcpole.dat which relates UT1 - UTC values to
     UTC dates.  As of July 1995 utcpole.dat starts at June 30, 1979; therefore,
     the correction UT1 - UTC, which can be as large as (+-) 0.9 s, is not
     available until that date, and an error status message will be returned if
     this function is called with a time before that date.  The file
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

REQUIREMENTS:
   PGSTK - 0860, 0750, 0930

DETAILS:
   None

GLOBALS:
   None

FILES:
   leapsec.dat
   utcpole.dat
   de200.eos

FUNCTIONS CALLED:
   PGS_MEM_Calloc()
   PGS_CBP_Earth_CB_Vector()
   PGS_CSC_ECItoECR()
   PGS_CSC_ZenithAzimuth()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()  

END_PROLOG:
*******************************************************************************/

#include <PGS_CBP.h>
#include <PGS_CSC.h>
#include <PGS_MEM.h>

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_DayNight()"
		
PGSt_SMF_status
PGS_CSC_DayNight(                       /* Get array of Day/Night indicators */
    PGSt_integer    numValues,          /* number of time offsets */
    char            asciiUTC[28],       /* UTC start time input in either ASCII
					   Time Code A or B format.  If B, then
					   it is converted to A format */
    PGSt_double     offsets[],          /* array of time offsets from start time
					   (seconds) */
    PGSt_double     latitude[],         /* array of Geodetic latitudes */
    PGSt_double     longitude[],        /* array of longitudes */
    PGSt_tag        sunZenithLimitTag,  /* tag specifying method of Day/Night 
		                           determination */
    PGSt_boolean    afterDark[])        /* array of Day/Night indicators */
{		    			
    PGSt_SMF_status returnStatus;       /* error return value for this 
					   function */
    PGSt_SMF_status returnStatus1;      /* error return value for 2nd or later
					   functions called */
    PGSt_SMF_status code;               /* status code returned by
					   PGS_SMF_GetMsg() */

    char            mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							    PGS_SMF_GetMsg()*/
    char            msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
							    PGS_SMF_GetMsg() */
    char            specifics[PGS_SMF_MAX_MSG_SIZE];     /* detailed error msg */

    void            *memPtr=NULL;       /* pointer to memory space allocated
					   dynamically for scratch arrays used
					   internally by this function */
    PGSt_integer    counter;            /* loop counter */
    PGSt_integer    maxValues;          /* counter cannot equal or exceed this
					   value */
    PGSt_double     (*cbVectors)[3];    /* ECI unit position vectors from Earth
					   to Celestial body (Sun) */
    PGSt_double     (*cbVectorsECI)[6]; /* ECI unit pos/vel vectors from Earth
					   to Celestial body (Sun) */
    PGSt_double     (*cbVectorsECR)[6]; /* ECR unit pos/vel vectors from Earth
					   to Celestial body (Sun) */
    PGSt_double     zenith;             /* zenith angle in radians */
    
    returnStatus = PGS_S_SUCCESS;       /* Default return value set to 
					   success */

 /* If sunZenithLimitTag is not valid, then return */
    
    switch(sunZenithLimitTag)
	{
	  case PGSd_CivilTwilight:
	  case PGSd_CivilNight:
	  case PGSd_NauticalNight:
	  case PGSd_AstronNight:
	    break;
	  default:
	    PGS_SMF_SetStaticMsg(PGSCSC_E_INVALID_LIMITTAG,FUNCTION_NAME);
	    return PGSCSC_E_INVALID_LIMITTAG;
	}
    
    /* Set maximum value of counter for looping to include case
       of no offset times, when computing just for start UTC time */
    if (numValues < 0 )
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
    
    /* Allocate space for cbVectors - ECI vector from Earth to celestial body,
       cbVectorsECI - ECI vector from Earth to celestial body and cbVectorsECR -
       ECR vector from Earth to celestial body */

    returnStatus = PGS_MEM_Calloc(&memPtr, 1, maxValues*(sizeof(PGSt_double)*
							 (3 + 6 +6)));
    
    if (returnStatus != PGS_S_SUCCESS)
	return returnStatus;
    
    cbVectors = (PGSt_double (*)[3]) memPtr;
    cbVectorsECI = (PGSt_double (*)[6]) (cbVectors + maxValues);
    cbVectorsECR = cbVectorsECI + maxValues;
    
    /* Get Earth to Sun vectors */
    
    returnStatus = PGS_CBP_Earth_CB_Vector(numValues,asciiUTC,offsets,PGSd_SUN,
                                           cbVectors);
      
    /* Check errors returned from PGS_CBP_Earth_CB_Vector */
    
    switch(returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSTD_W_PRED_LEAPS:
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if(code != returnStatus)
	  PGS_SMF_GetMsgByCode(returnStatus,msg);
	break;
      case PGSTD_E_BAD_INITIAL_TIME:
      case PGSCBP_E_UNABLE_TO_OPEN_FILE:
      case PGSCBP_E_TIME_OUT_OF_RANGE: 
	PGS_MEM_Free(memPtr);
	return returnStatus;
      default:
        PGS_MEM_Free(memPtr);
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Copy cbVectors into cbVectorsECI to add 0 velocities for
       call to ECItoECR */
    
    for(counter=0;counter<maxValues;counter++)
    {
	cbVectorsECI[counter][0] = cbVectors[counter][0];
	cbVectorsECI[counter][1] = cbVectors[counter][1];
	cbVectorsECI[counter][2] = cbVectors[counter][2];
	cbVectorsECI[counter][3] = 0.0;
	cbVectorsECI[counter][4] = 0.0;
	cbVectorsECI[counter][5] = 0.0;
    }
    
    /* Convert cbVectors from ECI to ECR coordinates */
    
    returnStatus1 = PGS_CSC_ECItoECR(numValues,asciiUTC,offsets,
				     cbVectorsECI,cbVectorsECR);
    
    /* Check errors returned from PGS_CSC_ECItoECR */
    
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
	break;
      case PGSTD_E_NO_UT1_VALUE:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if(code != returnStatus1)
	  PGS_SMF_GetMsgByCode(returnStatus1,msg);   
	returnStatus = returnStatus1;
      case PGSCSC_W_BAD_TRANSFORM_VALUE:
      case PGSCSC_W_PREDICTED_UT1: 
	if (returnStatus == PGS_S_SUCCESS)
        { 
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
	      PGS_SMF_GetMsgByCode(returnStatus1,msg);   
	    returnStatus = returnStatus1; 
	} 
      	break;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
        PGS_MEM_Free(memPtr);
	return PGS_E_TOOLKIT;
    }
    
       
    for(counter=0;counter<maxValues;counter++)
    {
	/* Check if input vector is valid -- if not, set afterDark to 
	   PGSCSC_W_BAD_TRANSFORM_VALUE and loop to next counter value */

	if(cbVectorsECR[counter][0] == PGSd_GEO_ERROR_VALUE)
	{
	    afterDark[counter] = PGSCSC_W_BAD_TRANSFORM_VALUE;
	    continue;
	}
	
	/* Get zenith angle */
	
       	returnStatus1 = PGS_CSC_ZenithAzimuth(cbVectorsECR[counter],
					      latitude[counter],
					      longitude[counter],0.0,
					      PGSd_CB,PGS_TRUE,PGS_FALSE,
					      &zenith,NULL,NULL);
	
	/* Check errors returned from PGS_CSC_ZenithAzimuth */
	/* Note: PGSCSC_E_BAD_LAT return leads to "default" case 
           plus a message to the logfile */
	
	switch(returnStatus1)
	{
	  case PGS_S_SUCCESS:
	  case PGSCSC_W_BELOW_HORIZON:
	    break;
          case PGSCSC_E_BAD_LAT:
            sprintf(specifics,"Invalid latitude supplied, offset %d",
                               (int) counter);
            PGS_SMF_SetDynamicMsg(returnStatus1,specifics,
                              FUNCTION_NAME);
	  default:
	    if(returnStatus != PGSCSC_W_ERROR_IN_DAYNIGHT)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		  PGS_SMF_GetMsgByCode(returnStatus1,msg);
		returnStatus = PGSCSC_W_ERROR_IN_DAYNIGHT;
	    }
	    /* set afterDark to error message and loop to next
	       counter value */

	    afterDark[counter] = returnStatus1;
	    continue;
	}
	

	/* Determine whether in day or night by checking the
	   sunZenithLimitTag. */
	
	switch(sunZenithLimitTag)
	{
	  case PGSd_CivilTwilight:
	    /* (end of day) Sun deemed to set within 90 degrees 50 arc
	       minutes (1.5853407 radians) from zenith */
	    if(zenith > 1.5853407)
	      afterDark[counter] = PGS_TRUE;
	    else
	      afterDark[counter] = PGS_FALSE;
	    break;
	  case PGSd_CivilNight:
	    /* (end of civil twilight) Sun more than 96 degrees
	       (1.6755161 radians) from zenith. */
	    if(zenith > 1.6755161)
	      afterDark[counter] = PGS_TRUE;
	    else
	      afterDark[counter] = PGS_FALSE;
	    break;
	  case PGSd_NauticalNight:
	    /* (end of Nautical twilight) Sun more than 102 degrees
	       (1.7802358 radians) from zenith. */
	    if(zenith > 1.7802358)
	      afterDark[counter] = PGS_TRUE;
	    else
	      afterDark[counter] = PGS_FALSE;
	    break;
	  case PGSd_AstronNight:
	    /* (end of Astronomical Twilight) Sun more than 108 degrees
	       (1.8849556 radians) from zenith. */
	    if(zenith > 1.8849556)
	      afterDark[counter] = PGS_TRUE;
	    else
	      afterDark[counter] = PGS_FALSE;
	    
	}			    
    }

    PGS_MEM_Free(memPtr);

    /* Return to calling function */
   
    if(returnStatus == PGS_S_SUCCESS)  
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    else
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
    return returnStatus;     
}
