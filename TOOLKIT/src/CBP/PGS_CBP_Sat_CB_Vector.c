/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
     
FILENAME:
    PGS_CBP_Sat_CB_Vector.c

DESCRIPTION:
    This file contains the function PGS_CBP_Sat_CB_Vector() which computes 
    Spacecraft to Celestial Body ECI Vector.
   
AUTHOR:
    Peter D. Noerdlinger / Applied Research Corporation
    Marek Chmielowski    / Applied Research Corporation
    Anubha Singhal       / Applied Research Corporation
    Guru Tej S. Khalsa   / Applied Research Corporation
    Snehavadan Macwan    / Applied Research Corporation

HISTORY:
    01-Apr-1994		PDN 	designed
    04-Apr-1994		SM      coded initial version
    14-Apr-1994	        MC  	debugged and rewritten
    22-Aug-1994		SM	modified prolog
    27-Aug-1994		SM	incorporated PGSt data types
    02-Sep-1994		SM	rewritten
    07-Sep-1994		SM	incorporated changes suggested by code
    				inspection group
    05-Jan-1995         AS      updated error messages returned from call to
                                PGS_CSC_ECItoSC()
    20-Mar-1995        GTSK     replace calls to malloc() and free() with calls
                                to PGS_MEM_Calloc() and PGS_MEM_Free()
    29-Apr-1995        GTSK     prepend PGSd_ to #defines TRMM, EOS_AM,
                                EOS_PM
    07-Jul-1995        GTSK     moved FORTRAN binding to external file
    09-Apr-1997		PDN 	fixed comments on time, leap seconds,
                                and deleted references to Earth axis values
    27-May-1997		PDN 	deleted references/branches for predicted leap
                                seconds
                                
END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG

TITLE:	
   Compute satellite to Celestial Body Vector in spacecraft reference frame

NAME:
   PGS_CBP_Sat_CB_Vector()
 
SYNOPSIS:
C:	
    #include <PGS_CBP.h>	

    PGSt_SMF_status
    PGS_CBP_Sat_CB_Vector(
    	PGSt_tag	spacecraftTag,	
	PGSt_integer	numValues,
	char		asciiUTC[28],		
	PGSt_double	offsets[],			
	PGSt_integer	cbId,				
	PGSt_double	cbVectors[][3])	

FORTRAN:
      include 'PGS_CBP.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'
      include 'PGS_CBP_6.f'
      include 'PGS_EPH_5.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
      
      integer function pgs_cbp_sat_cbvectors(spacecrafttag,numvalues,
     >                                       asciiutc,offsets,cbid,
     >                                       cbvectors)
      integer		spacecrafttag
      integer		numvalues
      character*27    	asciiutc
      double precision	offsets
      integer		cbid
      double precision	cbvectors(3,*)

				

DESCRIPTION:
   This function computes the vector in the spacecraft reference frame from the
   spacecraft to the Sun, Moon, or planets at a given time or range of times. 

INPUTS:
   
   Name		   Description	          Units	   Min		Max
   ----		   -----------	          -----	   ---		---
   spacecraftTag   Unique spacecraft 
  		   identifier	          N/A	   N/A		N/A

   asciiUTC [28]   UTC time in CCSDS 
                   ASCII Time code A 
  		   or B format	          time     1961-01-01   see NOTES
                                                               
     
   offsets[]	   array of time offsets  seconds    (   see   NOTES   )
		   from asciiUTC in 
	           seconds

   numValues	   number of required	  N/A	    0	         any
		   data points: if 0
		   only asciiUTC is
		   used


   cbId 	   identifier of 
 		   celestial body
		   (see table below)      N/A	    N/A		 N/A

	     THE DESIGNATION OF THE ASTRONOMICAL BODIES BY 
	     CELESTIAL BODY IDENTIFIER ( cbId ) IS:

  			cbId = 
		
		1 = MERCURY		8 = NEPTUNE
		2 = VENUS		9 = PLUTO
		3 = EARTH 	       10 = MOON
		4 = MARS	       11 = SUN
		5 = JUPITER	       12 = SOLAR-SYSTEM BARYCENTER
		6 = SATURN	       13 = EARTH-MOON BARYCENTER
		7 = URANUS		

OUTPUTS:
   Name			Description		  Units		Min	Max
   ----			-----------		  -----		---	---
   cbVectors[][3]	vectors in spacecraft
  			reference frame from	  
  			satellite to the 	  meter         (see NOTES )
			celestial body for
			each time event	  
						  		 

RETURNS:
   PGS_S_SUCCESS		  Success
   PGSCSC_W_BELOW_SURFACE	  output vector from ECItoSC below surface
   PGSCBP_W_BAD_CB_VECTOR         one or more bad vectors for requested times
   PGSCBP_E_BAD_ARRAY_SIZE        numvalues is less than 0
   PGSCBP_E_INVALID_CB_ID	  invalid celestial body identifier
   PGSMEM_E_NO_MEMORY		  not enough memory for tmpVectors
   PGSCBP_E_UNABLE_TO_OPEN_FILE   unable to open planetary data file
   PGSTD_E_BAD_INITIAL_TIME       initial time is incorrect 
   PGSCBP_E_TIME_OUT_OF_RANGE     initial time is outside the ephemeris bounds
   PGSTD_E_SC_TAG_UNKNOWN	  invalid spacecraft tag
   PGSEPH_E_BAD_EPHEM_FILE_HDR    no s/c ephem files had readable headers
   PGSEPH_E_NO_SC_EPHEM_FILE      no s/c ephem files could be found for
                                  input times
   PGS_E_TOOLKIT		  toolkit error
  
EXAMPLES:
C:
    #define ARRAY_SIZE	3

    PGSt_SMF_status returnStatus;
    
    PGSt_integer    numValues = ARRAY_SIZE;
    
    PGSt_double     cbVectors[ARRAY_SIZE][3];
    PGSt_double     offsets[ARRAY_SIZE] = {3600.0, 7200.0, 10800.0};       
    
    char            asciiUTC[28] = "2002-07-27T11:04:57.987654Z";	
    char 	    err[PGS_SMF_MAX_MNEMONIC_SIZE];
    char	    msg[PGS_SMF_MAX_MSG_SIZE];
    
    returnStatus = PGS_CBP_Sat_CB_Vector(PGSd_EOS_AM, numValues, asciiUTC,
                                         offsets, PGSd_MOON, cbVectors);
    
    if (returnStatus != PGS_S_SUCCESS)
    {
    	PGS_SMF_GetMsg(&returnStatus, err, msg);
    	printf ("ERROR: %s\n", msg);
    }

FORTRAN:
      implicit none
      integer		numvalues
      character*27    	asciiutc
      double precision	offsets(3)
      integer		cbid
      double precision	cbvectors(3,3)
      
      character*33 	err
      character*241 	msg
      
      data offsets/3600.0, 7200.0, 10800.0/
      
      asciiutc = "2002-07-27T11:04:57.987654Z"
      cbid = 10
      numvalues = 3
      
      returnstatus = pgs_cbp_sat_cb_vector(pgsd_eos_am, numvalues,
     >                                     asciiutc, offsets, pgsd_moon,
     >                              	   cbvector)
      
      if (returnstatus .ne. pgs_s_success) then
      	  pgs_smf_getmsg(returnstatus, err, msg)
      	  write(*,*) err, msg
      endif
	
NOTES:
   Prior to 1984, there is no distinction between TDT and TDB; either one is 
   denoted "ephemeris time" (ET). Also, the values before  1972 are based on 
   U.S. Naval Observatory estimates, which are the same as adopted by the JPL 
   Ephemeris group that produces the DE series of solar system ephemerides, 
   such as DE200.
   
   The minimum and maximum values of the output cbVectors depends on the
   location of the spacecraft with respect to the target planet. If spacecraft
   is at or near the target planet then the minimum distance will be 0.0 OR
   few hundred meters. But if the spacecraft is far away from the target
   planet, then the maximum value of the cbvectors will be the distance
   between the spacecraft and the target planet. 

   TIME ACRONYMS:
     
     TAI is:  International Atomic Time
     TDB is:  Barycentric Dynamical Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file contains dates of leap 
     second events. The file is updated when a new leap second is announced. 
     If an input date is past the last date in the file (or if the file cannot 
     be read) but the date is after Jan 1, 1961, the function will use a 
     calculated value of TAI-UTC based on a linear fit of the data known to be 
     in the table.  This value of TAI-UTC is relatively crude estimate and 
     may be off by as much as 1.0 or more seconds.  Thus, when the function 
     is used for dates in the future of the date and time of invocation, the 
     user ought to carefully check the return status.  The status level will 
     be 'E' if the TAI-UTC value is calculated (although processing will 
     continue in this case, using the calculated value of TAI-UTC).

   JULIAN DATES:

     The Julian date is a day count originating at noon of Jan. 1st, 4713 B.C.

     Format:

       Toolkit Julian dates are kept as an array of two real (high precision)
       numbers (C: PGSt_double, FORTRAN: DOUBLE PRECISION).  The first element
       of the array should be the half integer Julian day (e.g. N.5 where N is a
       Julian day number).  The second element of the array should be a real
       number greater than or equal to zero AND less than one (1.0) representing
       the time of the current day (as a fraction of that (86400 second) day.
       This format allows relatively simple translation to calendar days (since
       the Julian days begin at noon of the corresponding calendar day).  Users
       of the Toolkit are encouraged to adhere to this format to maintain high
       accuracy (one number to track significant digits to the left of the
       decimal and one number to track significant digits to the right of the
       decimal).  Toolkit functions that do NOT require a Julian type date as an
       input and return a Julian date will return the Julian date in the above
       mentioned format.  Toolkit functions that require a Julian date as an
       input and do NOT return a Julian date will first convert (internally) the
       input date to the above format if necessary.  Toolkit functions that have
       a Julian date as both an input and an output will assume the input is in
       the above described format but will not check and the format of the
       output may not be what is expected if any other format is used for the
       input.

     Meaning:

       Toolkit "Julian dates" are all derived from UTC Julian Dates.  A Julian
       date in any other time stream (e.g. TAI, TDT, UT1, etc.) is the UTC
       Julian date plus the known difference of the other stream from UTC
       (differences range in magnitude from 0 seconds to about a minute).

       Examples:

         In the following examples, all Julian Dates are expressed in Toolkit
         standard form as two double precision numbers. For display here, the
         two members of the array are enclosed in braces {} and separated by a
         comma.

         A) UTC to TAI Julian dates conversion

         The Toolkit UTC Julian date for 1994-02-01T12:00:00 is: 
         {2449384.50, 0.5}.
         TAI-UTC at 1994-02-01T12:00:00 is 28 seconds (.00032407407407 days). 
         The Toolkit TAI Julian date for 1994-02-01T12:00:00 is:
         {2449384.50, 0.5 + .00032407407407} = {2449384.50, 0.50032407407407}

         Note that the Julian day numbers in UTC and the target time stream may
         be different by + or - 1 for times near midnight:

         B) UTC to UT1 Julian dates conversion

         The Toolkit UTC Julian date for 1994-04-10T00:00:00 is: 
         {2449452.50, 0.0}.
         UT1-UTC at 1994-04-10T00:00:00 is -.04296 seconds 
         (-0.00000049722221 days).  The Toolkit UT1 Julian date for
         1994-04-10T00:00:00 is:
         {2449452.50, 0.0 - 0.0000004972222} = 
         {2449452.50, -0.0000004972222} =
         {2449451.50, 0.9999995027778}

   MODIFIED JULIAN DATES:

     Modified Julian dates follow the same conventions as those for Julian
     dates (above) EXCEPT that the modified Julian day number is integral (NOT
     half-integral).  The modified Julian date in any time stream has a day 
     number that is 2400000.5 days less than the Julian date day number and an
     identical day fraction (i.e. the modified Julian date is a day count
     originating at 1858-11-17T00:00:00).

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

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac


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
      that did not prohibit processing.

      The reporting of general warnings takes precedence over the generic
      warning (see RETURNS above) that processing was not successful at some of
      the requested times.  Therefore in the case of any return status of level
      'W', the returned value of a high precision real variable generally should
      be examined for errors at each time offset, as specified above.


REQUIREMENTS:
   PGSTK-0810, 0680

DETAILS:
      This algorithm first checks the spacecraftTag. If spacecraftTag is invalid,
      it returns error message. Then it checks for the numValues and cbId, if any 
      of them invalid, it returns with an error message. It then calls 
      PGS_CBP_Earth_CB_Vector routine to get the ECI vector for each of
      the time events specified in the calling sequence. Now the algorithm calls 
      PGS_CSC_ECItoSC routine to transform ECI vector to spacecraft reference 
      frame. The algorithm uses time offset value to define the range of times.
      For each time, the algorithm computes the ECI vector from the spacecraft 
      to the celestial body.

      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   None

FILES:
   Spacecraft ephemeris file(s), Planetary ephemeris file (de200.eos) and
   Leapseconds file (leapsec.dat)


FUNCTIONS CALLED:
   PGS_CBP_Earth_CB_Vector()
   PGS_CSC_ECItoSC()

END_PROLOG:
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_MEM.h>
#include <PGS_EPH.h>
#include <PGS_CBP.h>

#define MAXCBID		13

/* name of this function */

#define FUNCTION_NAME "PGS_CBP_Sat_CB_Vector()"

PGSt_SMF_status
PGS_CBP_Sat_CB_Vector(                 	/* Computes the ECI vector from
					   the spacecraft to celestial 
					   body for a given time */
    PGSt_tag	    spacecraftTag,	/* Unique spacecraft 
					   identifier */
    PGSt_integer    numValues,		/* Number of time events to be 
					   calculated */
    char	    asciiUTC[28],	/* Time tag in CCSDS ASCII Time
					   code A format */
    PGSt_double	    offsets[],		/* array with offsets of time 
					   events in seconds */
    PGSt_integer    cbId,		/* celestial body 
					   identifier*/
    PGSt_double	    cbVectors[][3])	/* vector in spacecraft
					   reference frame from 
					   Spacecraft to celestial body
					   as a function of time */
{		   
    PGSt_integer    maxValues;          /* used for malloc call */
    PGSt_integer    counter;		/* integer counter value */

    PGSt_double	    (*tmpVectors)[3]=NULL; /* ECI vector from Earth 
					      to celestial body
					      as a function of time */
    
    PGSt_SMF_status returnStatus;	/* Error status for function */
    PGSt_SMF_status returnStatus1;	/* Error status for function calls */
    PGSt_SMF_status code;	    	/* status code returned by
					   PGS_SMF_GetMsg() */
    
    char	    mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							    PGS_SMF_GetMsg() */
    char	    msg[PGS_SMF_MAX_MSG_SIZE];	         /* message returned by
						            PGS_SMF_GetMsg() */
    
    returnStatus = PGS_S_SUCCESS;		/* default status indicating 
						   success */
    
    if ( numValues < 0 )
    {
	/* NO negative array size allowed */
	
	returnStatus = PGSCBP_E_BAD_ARRAY_SIZE;
	PGS_SMF_SetStaticMsg( returnStatus,FUNCTION_NAME );
	return  returnStatus;
    }
    
    /* check if planet identifier is correct. */
    
    if ((cbId > MAXCBID) || (cbId < 1))
    {
	returnStatus = PGSCBP_E_INVALID_CB_ID;
	PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
	return returnStatus;
    }
    
    /* allocate space for tmpVectors - ECI vector from Earth to celestial
       body */
    
    maxValues = (numValues == 0) ? 1 : numValues;
    returnStatus = PGS_MEM_Calloc((void**) &tmpVectors, maxValues,
				  sizeof(PGSt_double)*3);
    if (returnStatus != PGS_S_SUCCESS)
    {
	PGS_MEM_Free(tmpVectors);
	return returnStatus;
    }
	
    /*** Call PGS_CBP_Earth_CB_Vector() routine to get ECI vector for the 
         specified celestial body.  ***/
    
    returnStatus = PGS_CBP_Earth_CB_Vector(numValues, asciiUTC, offsets, cbId,
					   tmpVectors);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
      case PGSCBP_W_BAD_CB_VECTOR:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
        if(code != returnStatus)
	  PGS_SMF_GetMsgByCode(returnStatus,msg);  
	break;
      case PGSCBP_W_EARTH_CB_ID:
	returnStatus = PGS_S_SUCCESS;
	break;
      case PGSCBP_E_UNABLE_TO_OPEN_FILE:
      case PGSTD_E_BAD_INITIAL_TIME:
      case PGSCBP_E_TIME_OUT_OF_RANGE:
      case PGS_E_TOOLKIT:
	PGS_MEM_Free(tmpVectors);
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	returnStatus = PGS_E_TOOLKIT;
	PGS_MEM_Free(tmpVectors);
	return returnStatus;
    } /* end of switch */
    
    /* Now call PGS_CSC_ECItoSC() to get the ECI vector in spacecraft 
       reference frame. Check for the error messages. */
    
    returnStatus1 = PGS_CSC_ECItoSC(spacecraftTag,numValues,asciiUTC,offsets, 
				    tmpVectors, cbVectors);
    switch (returnStatus1)
    {
      case PGS_S_SUCCESS:
      case PGSCSC_W_BELOW_SURFACE:
	break;
      case PGSCSC_E_SC_TAG_UNKNOWN:
	PGS_MEM_Free(tmpVectors);
	return returnStatus1;
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if(code != returnStatus1)
	  PGS_SMF_GetMsgByCode(returnStatus1,msg);
	returnStatus = returnStatus1;
	break;
      case PGSCSC_W_BAD_TRANSFORM_VALUE:
	if (returnStatus == PGS_S_SUCCESS)
	{
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
	      PGS_SMF_GetMsgByCode(returnStatus1,msg);  
	    returnStatus = PGSCBP_W_BAD_CB_VECTOR;
	}
	break;
      case PGSMEM_E_NO_MEMORY:
      case PGSEPH_E_BAD_EPHEM_FILE_HDR:
      case PGSEPH_E_NO_SC_EPHEM_FILE:
      case PGSEPH_E_NO_DATA_REQUESTED:
	PGS_MEM_Free(tmpVectors);
	return returnStatus1;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	returnStatus = PGS_E_TOOLKIT;
	PGS_MEM_Free(tmpVectors);
	return returnStatus;
    }
    if (returnStatus != PGS_S_SUCCESS)
      for (counter = 0; counter < maxValues; counter++)
      {
	  if (tmpVectors[counter][0] == PGSd_GEO_ERROR_VALUE)
	  {
	      cbVectors[counter][0] = PGSd_GEO_ERROR_VALUE;
	      cbVectors[counter][1] = PGSd_GEO_ERROR_VALUE;
	      cbVectors[counter][2] = PGSd_GEO_ERROR_VALUE;
	  }
      }
    
    /* free allocated memory */

    PGS_MEM_Free(tmpVectors);
    
    PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);

    return returnStatus;
}
