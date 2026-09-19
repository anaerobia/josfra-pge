/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
     
FILENAME:
   PGS_CBP_Earth_CB_Vector.c

DESCRIPTION:
   This file contains the function PGS_CBP_Earth_CB_Vector() which computes 
   Earth to Celestial Body ECI Vector.
   
AUTHOR:
   Marek Chmielowski / Applied Research Corporation
   Snehavadan Macwan / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   30-Mar-1994 	MC      Initial version
   05-Apr-1994 	MC  	included time format conversions
   07-Apr-1994 	MC	changed calling sequence
   25-Apr-1994 	MC	hard coded data file position
   07-Jul-1994 	SM	incorporated C version of JPL software
   27-Aug-1994	SM	redesigned to include all test cases before
                        calling any function
   01-Sep-1994	SM	incorporated changes suggested by code inspection group
   05-Jun-1995  GTSK    Reformatted prolog and code
   07-Jul-1995  GTSK    Moved FORTRAN binding to external file
   09-Apr-1997  PDN     Fixed comments to remove references to Earth axes
                        (irrelevant) and add time range information
   26-May-1997  PDN     Deleted branches for _W_PRED_LEAPS

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Compute Earth to Celestial Body ECI Vector
   
NAME:
   PGS_CBP_Earth_CB_Vector()

SYNOPSIS:
C:
    #include <PGS_CBP.h>                 
    
    PGSt_SMF_status
    
    PGS_CBP_Earth_CB_Vector(
    	PGSt_integer	numValues,
    	char		asciiUTC[28],
    	PGSt_double	offsets[],
    	PGSt_integer 	cbId,
    	PGSt_double	cbVectors[][3])

FORTRAN:
	
      include 'PGS_CBP.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'
      include 'PGS_CBP_6.f'
      include 'PGS_TD_3.f'

      integer function pgs_cbp_earth_cb_vector(numvalues,asciiutc,
     >                                         offsets,cbid,cbvectors)
      integer		 numvalues
      character*27	 asciiutc
      double precision	 offsets(*)
      integer		 cbid
      double precision	 cbvectors(3,*)                     

DESCRIPTION:
   This function computes the Earth-Centered Inertial (ECI J2000)
   frame vector from the Earth to the selected bodies of Solar System.

INPUTS:
   NAME       DESCRIPTION	  UNITS     MIN	        MAX
   ----       -----------	  -----	    ---	        ---
   asciiUTC   UTC time in CCSDC   time      1961-01-01  see NOTES
              ASCII Time Code A 
	      OR B format

   offsets    Array of offsets    seconds    (   see     NOTES	)   
              of each input 
              UTC time 

   cbId       identifier of        N/A	     1  	 13
              celestialbody 
	      ( see table below )		

   numValues  number of            N/A	     0		 ANY
	      required data	 
	      points (if 0 only
              asciiUTC is used)
 
             
                any - any time events are 
	            used
	

         THE DESIGNATION OF THE ASTRONOMICAL BODIES BY 
         CELESTIAL BODY IDENTIFIER ( cbId ) IS:

                             cbId =

                1 = MERCURY           8 = NEPTUNE
                2 = VENUS             9 = PLUTO
                3 = EARTH            10 = MOON
                4 = MARS             11 = SUN
                5 = JUPITER          12 = SOLAR-SYSTEM BARYCENTER
                6 = SATURN           13 = EARTH-MOON BARYCENTER
                7 = URANUS            

OUTPUTS:
    
   NAME		   DESCRIPTION		     UNITS	    MIN		MAX
   ----		   -----------		     -----	    ---		---
   cbVectors[][3]  ECI unit vectors          meter	    (  see   NOTES )
   		   from Earth to celestial  
		   body first subscript for
		   each time event specified   
		   second subscript gives
		   position vector

RETURNS:
   PGS_S_SUCCESS                 Successfull completion 
   PGSCBP_W_EARTH_CB_ID	         Earth cbId is specified
   PGSCBP_E_INVALID_CB_ID        Invalid celestial body identifier 
   PGSTD_E_BAD_INITIAL_TIME      Initial input time can not be deciphered
   PGSCBP_E_BAD_ARRAY_SIZE	 Incorrect array size
   PGSCBP_E_UNABLE_TO_OPEN_FILE  Ephemeris file can not be opened
   PGSCBP_E_TIME_OUT_OF_RANGE    Initial time is outside the ephemeris bounds
   PGSTD_E_NO_LEAP_SECS          No leap second correction available
   PGSCBP_W_BAD_CB_VECTOR        One or more errors in CB vectors
   PGS_E_TOOLKIT                 For unknown errors 


EXAMPLES:
C:
    #define ARRAY_SIZE	3
    
    PGSt_SMF_status returnStatus;
    PGSt_integer    cbId = 10;
    PGSt_integer    numValues;
    char            asciiUTC[28] = "2002-07-27T11:04:57.987654Z";
    PGSt_double     offsets[ARRAY_SIZE] = {3600.0, 7200.0, 10800.0};
    PGSt_double     cbVectors[ARRAY_SIZE][3];
    
    char 	    err[PGS_SMF_MAX_MNEMONIC_SIZE];
    char	    msg[PGS_SMF_MAX_MSG_SIZE];
    
    numValues = ARRAY_SIZE;
    
    returnStatus = PGS_CBP_Earth_CB_Vector(numValues, asciiUTC, offsets, cbId,
                                           cbVectors);
    
    if (returnStatus != PGS_S_SUCCESS)
    {
    	PGS_SMF_GetMsg(&returnStatus, err, msg);
    	printf ("ERROR: %s\n", msg);
    }

FORTRAN:
      integer		 pgs_cbp_earth_cb_vector
      integer		 returnstatus
      integer            cbid 
      integer		 numvalues
 
      double precision   offsets(3)
      double precision   cbvectors(3,3)

      character*27       asciiutc      
      character*33 	 err
      character*241 	 msg
      
      data offsets/3600.0, 7200.0, 10800.0/
      
      asciiutc = '2002-07-27T11:04:57.987654Z'
      cbid = 10
      numvalues = 3
      
      returnstatus = pgs_cbp_earth_cb_vector(numvalues, asciiutc, 
     >                                       offsets, cbid, cbvectors)
      
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
   target planet with respect to the center planet. 

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

   REFERENCES:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
   PGSTK-0800

DETAILS:
   This function first checks if the numValues is less than zero. If numValues
   is less than zero, it sets the output cbVectors to PGSd_GEO_ERROR_VALUE and
   returns with an error status. Next it checks if the cbId is greater than 13
   OR less than 1. If cbId is incorrect, it sets the output cbVectors to
   PGSd_GEO_ERROR_VALUE and returns with an error status. If cbId = 3, that is
   distance from Earth to Earth is desired. In this case, it sets the output
   cbVectors to 0.0 and returns with warning message. This function then passes
   the UTC time to PGS_TD_UTCtoTDBjed() which converts input UTC time to jedTDB
   as a Julian date (TDB = Barycentric Dynamical Time). The function then
   passes all the parameters to the PGS_CBP_EphemRead() function which reads
   and interpolates the JPL planetary ephemeris data file and then returns the
   corrected (correction for the parallax, aberration and light travel time is
   applied) position of the target planet with respect to the Earth.

    See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
    ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
    for more information on the algorithm.

GLOBALS:
   NONE

FILES:  
   de200.eos			A binary ephemeris data file

FUNCTIONS_CALLED:
   PGS_TD_UTCtoTDBjed()       Converts ASCII UTC to jedTDB in Julian days
   PGS_SMF_SetStaticMsg()     Register static message with SMF
   PGS_SMF_SetUnknownMsg()    Register unknown message with SMF
   PGS_CBP_EphemRead() 	      Reads and interpolates planetary ephemeris data

END_PROLOG:
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>          /* standard C PGS_math.header */
#include <PGS_CBP.h>	   /* function prototype */  

/* constants */

#define  MAXCBID   13			    /* Maximum value for cbId */
#define  AU        0.1495978706600000E+12   /* Astronomical units in meters */

/* name of this function */

#define FUNCTION_NAME "PGS_CBP_Earth_CB_Vector()"

PGSt_SMF_status
PGS_CBP_Earth_CB_Vector(	     /* computes Earth to Celestial body ECI 
				        vector */
    PGSt_integer    numValues,       /* number of time events to be used */
    char      	    asciiUTC[28],    /* time tag UTC as CCSDS ASCII 
				        (Format A or B) */
    PGSt_double     offsets[],       /* array with offsets of time events in
				        seconds */ 
    PGSt_integer    cbId,            /* planet identifier */
    PGSt_double     cbVectors[][3])  /* returned planet position for each time
				        event */
{
    PGSt_SMF_status  returnStatus;   /* holder for return status */
    PGSt_SMF_status  returnStatus1;  /* holder for return status */

    PGSt_double	     startTDB[2];    /* equivalent TDB Julian date (TDB) of
					input UTC time */
    PGSt_SMF_status  code;	     /* status code returned by
					PGS_SMF_GetMsg() */
    int              counter;        /* loop counter */

    char  mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by 
						  PGS_SMF_GetMsg() */
    char  msg[PGS_SMF_MAX_MSG_SIZE];	       /* message returned by
						  PGS_SMF_GetMsg() */
						   
    /* Test size of arrays */
    
    if ( numValues < 0 )
    {
	PGS_SMF_SetStaticMsg(PGSCBP_E_BAD_ARRAY_SIZE, FUNCTION_NAME);
	return  PGSCBP_E_BAD_ARRAY_SIZE;
    }
    
    /* check if planet identifier is correct. If it is incorrect return with
       an error status */
    
    if ((cbId > MAXCBID) || (cbId < 1))
    {
	PGS_SMF_SetStaticMsg(PGSCBP_E_INVALID_CB_ID, FUNCTION_NAME);
	return PGSCBP_E_INVALID_CB_ID;
    }
    
    /* check if cbId = 3; which means Earth to Earth ECI vector is needed. 
       Return with warning message, cbVectors is set to 0.0. */
    
    if (cbId == 3)
    {
	if (numValues == 0)
	    numValues = 1;  /* catch the single vector numValues == 0 implies */
	
	for (counter = 0; counter < numValues; counter++)
	{
	    cbVectors[counter][0] = 0.0;
	    cbVectors[counter][1] = 0.0;
	    cbVectors[counter][2] = 0.0;
	}
	
	PGS_SMF_SetStaticMsg(PGSCBP_W_EARTH_CB_ID, FUNCTION_NAME);
	return PGSCBP_W_EARTH_CB_ID;
    }
    
    /* convert ASCII UTC to equivalent TDB Julian date */
    
    returnStatus = PGS_TD_UTCtoTDBjed(asciiUTC, startTDB);
    
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if (code != returnStatus)
	    PGS_SMF_GetMsgByCode(returnStatus,msg);
	break;
	
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if (code != returnStatus)
	    PGS_SMF_GetMsgByCode(returnStatus,msg);
	returnStatus = PGSTD_E_BAD_INITIAL_TIME;
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	return returnStatus;
	
      case PGS_E_TOOLKIT:
	return returnStatus;
	
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Now call PGS_CBP_EphemRead() to read and interpolate JPL planetary 
       ephemeris data and to compute Earth to celestial body ECI vector. */
    
    returnStatus1 = PGS_CBP_EphemRead(startTDB, cbId, numValues, offsets,
				      cbVectors);
    switch (returnStatus1)
    {
      case PGS_S_SUCCESS:
      case PGSCBP_W_BAD_CB_VECTOR:
	if (returnStatus == PGS_S_SUCCESS)
	    returnStatus = returnStatus1;
	else
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	break;
	
      case PGSCBP_E_TIME_OUT_OF_RANGE:
      case PGSCBP_E_UNABLE_TO_OPEN_FILE:
      case PGS_E_TOOLKIT:
	returnStatus=returnStatus1;
	break;
	
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    
    return returnStatus;
}
