/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_GreenwichHour.c

DESCRIPTION:
   This file contains the function PGS_CSC_GreenwichHour().  The function
   accepts an input start time and array of offsets from that start time,
   and, for each time, calls the function PGS_TD_gmst() to compute Greenwich
   Mean Sidereal Time expressed as the hour angle of the Vernal equinox at 
   the Greenwich meridian in radians. This function then converts from
   radians to hours for output.

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation
   Deborah A. Foch      / Applied Research Corporation
   Guru Tej S. Khalsa   / Applied Research Corporation

HISTORY:
   25-AUG-1994  PDN  Designed
   31-AUG-1994  DAF  Completed coding
   1-SEPT-1994  DAF  Fixed errors in formats and added check of
                     jdUTC[2]
   4-SEPT-1994  DAF  Modified to get hour angle for input UTC time
   7-SEPT-1994  DAF  Made changes resulting from code inspection
   8-SEPT-1994  DAF  Fixed reporting of warning messages
   9-SEPT-1994  DAF  Changed/added calls to set messages
   25-OCT-1994  DAF  Changed Min/Max info for asciiUTC in prolog
   06-JUN-1995  GTSK Substantial rewrite.  Incorporated lower level functions in
                     place of otherwise redundant code.  Touched up prologs.
   15-JUN-1995  GTSK Changed prolog (added stuff to NOTES section).
   25-AUG-1996  PDN  Removed test for obsolete warning message 
                     PGSCSC_W_INTERIM_UT1

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get Greenwich Hour Angles

NAME:
   PGS_CSC_GreenwichHour()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_GreenwichHour(
       PGSt_integer      numValues,
       char              asciiUTC[28],
       PGSt_double       offsets[],
       PGSt_double	 hourAngleGreenw[])

FORTRAN:
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'

      integer function pgs_csc_greenwichhour(numvalues,asciiutc,offsets,
     >                                       houranglegreenw)
      integer          numvalues
      character*27     asciiutc
      double precision offsets(*)
      double precision houranglegreenw(*)

DESCRIPTION:   This function computes hour angle of the Vernal Equinox at
   the Greenwich meridian, accepting an input start time plus an array of
   time offsets.

INPUTS:
   Name             Description         Units       Min             Max
   ----             -----------         -----       ---             ---
   numValues        number of input      N/A         0              any
                    time offsets

   asciiUTC         UTC start time in    N/A      See NOTES      See NOTES
                    CCSDS ASCII Time                   
		    Code A or B format

   offsets          array of time       seconds     Max and Min such that
                    offsets                         asciiUTC + offset is
		                                    between asciiUTC Min
						    and Max values
OUTPUTS:
   Name             Description            Units       Min       Max
   ----             -----------            -----       ---       ---
   hourAngleGreenw  array of values of     hours        0         24
                    the hour angle of the
                    Vernal Equinox at 
		    Greenwich; a value
		    of 999999.0 is
		    returned for invalid
		    offset times


RETURNS:
   PGS_S_SUCCESS                  successful return
   PGSTD_W_PRED_LEAPS             TAI-UTC value in leapsec.dat file
                                  is predicted (not actual) for at 
				  least one of the input times
   PGSCSC_W_ERRORS_IN_GHA         an error occurred in computing at least
                                  one Greenwich hour angle			
   PGSCSC_W_PREDICTED_UT1         data in utcpole.dat file is predicted (not
                                  final) value for at least one input time
   PGSTD_E_TIME_VALUE_ERROR       error in input time value
   PGSTD_E_TIME_FMT_ERROR         error in input time format
   PGSTD_E_NO_LEAP_SECS           no leap seconds correction is available
                                  in leapsec.dat file for at least one of the
				  input times/offsets
   PGS_E_TOOLKIT                  something unexpected happened, execution 
                                  of function ended prematurely
 
EXAMPLES:
C:
   #define   ARRAY_SIZE   3

   PGSt_SMF_status    returnStatus;
   PGSt_integer       numValues;
   PGSt_integer       counter;
   char               asciiUTC[28];
   PGSt_double        offsets[ARRAY_SIZE] = {3600.0,7200.0,10800.0};
   PGSt_double        hourAngleGreenw[ARRAY_SIZE];
   
   numValues = ARRAY_SIZE;
   strcpy(asciiUTC,"1991-01-01T11:29:30");
   returnStatus = PGS_CSC_GreenwichHour(numValues,asciiUTC,offsets,
                                       hourAngleGreenw);
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }
   printf("start time:%s",asciiUTC);
   counter = 0;
   while(counter < numValues)
   {
       printf("Offset: %f   Hour Angle:%f",offset[counter],
              hourAngleGreenw[counter]);
       counter++;	      
   }
       
FORTRAN:
      implicit none
      integer            pgs_csc_greenwichhour
      integer            array_size
      integer            returnstatus
      integer            counter     
      integer            numvalues
      character*27       asciiutc
      double precision   offsets(array_size)
      double precision   houranglegreenw(array_size)
      
      data offsets/3600.0,7200.0,10800.0/
      array_size = 3
      numvalues = array_size
      asciiutc = '1991-01-01T11:29:30'
  
      returnstatus = pgs_csc_greenwichhour(numvalues,asciiutc,offsets,
     >                                     houranglegreenw)
      
      if(returnstatus .ne. pgs_s_success) go to 90
      
      write(6,*) asciiutc
      if(numvalues.eq.0) numvalues = 1
      do 40 counter = 1,numvalues,1
      write(6,*)offsets(counter),houranglegreenw(counter)
   40 continue

   90 write(6,99)returnstatus
   99 format('ERROR:',A50)  

NOTES: 
   Historically, UT1 was used as a measure of time, but since 1958 it
   has served only as a measure of Earth rotation. The only real 
   difference between UT1 and Greenwich Mean Sidereal Time (GMST) is
   that UT1 measures Earth rotation in regards to the vector from 
   Earth center to the mean Sun (a fictitious point that traverses
   the celestial equator at the same mean rate that the Sun apparently
   traverses the ecliptic), while GMST measures Earth rotation relative
   to the vernal equinox.  Essentially, the value of GMST in radians is
   larger than that of UT1 in radians by the ratio of the mean solar day
   to the sidereal day; however, there are small correction terms due to
   precession.  The equation used in function PGS_TD_gmst() is valid for
   the period 1950 to well past 2000, as long as the definition of UT1
   and the reference equinox (J2000) are not changed.  The basic limitation
   is the accuracy of UT1.  Users obtaining UT1 from the PGS Toolkit
   should observe time limitations in the function PGS_TD_UTCtoUT1().

   TIME ACRONYMS:
     
     GMST is: Greenwich Mean Sidereal Time
     TAI is:  International Atomic Time
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
   PGSTK - 0770

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   None

FUNCTIONS CALLED:
   PGS_TD_UTCtoTAI()
   PGS_TD_TAItoUTC()
   PGS_TD_UTCtoUTCjd()
   PGS_CSC_UTCtoUT1Pole()
   PGS_TD_gmst()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

FILES:
   This function requires the files utcpole.dat and leapsec.dat for
   successful execution.

END_PROLOG:
*******************************************************************************/

#include <PGS_CSC.h>

/* conversion constants */

#define SECONDSperDAY  86400.0          /* number of seconds in a day */
#define HOURSperDAY    24.0             /* number of hours in a day */
#define TWOPI          6.2831853071796  /* 2 times PI */

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_GreenwichHour()"

PGSt_SMF_status
PGS_CSC_GreenwichHour(                  /* Get array of Greenwich Hour Angles */
    PGSt_integer      numValues,        /* number of time offsets */
    char              asciiUTC[28],     /* UTC start time in CCSDS ASCII Time
					   Code (A or B format) */
    PGSt_double       offsets[],        /* array of time offsets from start
					   time (seconds) */
    PGSt_double       hourAngleGreenw[])/* array of Greenwich Hour Angles */
{					
    PGSt_SMF_status   returnStatus;     /* value returned by this function */
    PGSt_SMF_status   returnStatus1;    /* value returned by calls to PGS
					   functions */
    PGSt_SMF_status   code;             /* status code returned by 
					   PGS_SMF_GetMsg() */
    PGSt_integer      counter;          /* loop counter */
    PGSt_integer      maxValues;        /* number of hour angles returned */
    PGSt_integer      numBad=0;         /* number of unusable input times */

    PGSt_double       secTAI93;         /* continuous seconds since 0 hours
					   UTC on 01-01-93 (of each offset) */
    PGSt_double       startTAI93;       /* continuous seconds  since 0 hours
					   UTC on 01-01-93 (of asciiUTC) */
    PGSt_double       jdUT1[2];         /* UT1 Julian date */
    PGSt_double       gmst;             /* Greenwich Mean Sidereal Time (hour
					   angle of the Vernal equinox at the
					   Greenwich meridian) */

    /* mnemonic returned by PGS_SMF_GetMsg()*/

    char              mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];

    /* message returned by PGS_SMF_GetMsg() */

    char              msg[PGS_SMF_MAX_MSG_SIZE];

    /*********************
     *  BEGIN EXECUTION  *
     *********************/

    /* convert input start time to TAI */
    
    returnStatus = PGS_TD_UTCtoTAI(asciiUTC,&startTAI93);
    
    /* check errors returned from PGS_TD_UTCtoTAI() */
    
    switch(returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSTD_W_PRED_LEAPS:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if(code != returnStatus)
	    PGS_SMF_GetMsgByCode(returnStatus,msg);
	break;
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSTD_E_TIME_FMT_ERROR:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Add each offset to TAI start time.  Then, convert to jdUT1, call
       PGS_TD_gmst() to calculate hour angle, and convert to hours.  Save output
       Greenwich hour angles in array hourAngleGreenw[]. */
    
    /* set maximum value of counter for looping to include start UTC time */
    
    maxValues = (numValues == 0) ? 1 : numValues;
    
    for(counter=0;counter<maxValues;counter++)
    {
	
	/* calculate time of current offset */
	
	if (numValues == 0)
	    secTAI93 = startTAI93;
	else
	    secTAI93 = startTAI93 + offsets[counter];
	
	/* convert TAI (toolkit internal time) to UT1 Julian date */

	returnStatus1 = PGS_TD_TAItoUT1jd(secTAI93,jdUT1);

	switch(returnStatus1)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGSTD_W_PRED_LEAPS:
	    if (returnStatus == PGSTD_W_PRED_LEAPS)
		break;
	  case PGSCSC_W_PREDICTED_UT1:
	    if (returnStatus != PGSCSC_W_PREDICTED_UT1)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);
		returnStatus = returnStatus1;
	    }
	    break;
	  case PGS_E_TOOLKIT:
	    return returnStatus1;
	  default:
	    if(returnStatus == PGS_S_SUCCESS)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);
		returnStatus = PGSCSC_W_ERRORS_IN_GHA;
	    }

	    /* Set Greenwich Hour angle to error value and continue looping
	       through to next offset value */
	    
	    hourAngleGreenw[counter] = PGSd_GEO_ERROR_VALUE;
	    if (++numBad == maxValues)
	    {
		PGS_SMF_SetStaticMsg(returnStatus1,FUNCTION_NAME);
		return returnStatus1;
	    }
	    continue;
	}
	
	/* Call function PGS_TD_gmst() to compute the GMST (hour angle of the
	   Vernal equinox at the Greenwich meridian in radians) */
	
	gmst = PGS_TD_gmst(jdUT1);
	
	hourAngleGreenw[counter] = gmst * HOURSperDAY/TWOPI;
	
	/* adjust hour angle to be:  0 <= hour angle < 24.0 */
	
	if (hourAngleGreenw[counter] < 0.0)
	    hourAngleGreenw[counter] = hourAngleGreenw[counter] + 24.0;
	else if(hourAngleGreenw[counter] >= 24.0)
	    hourAngleGreenw[counter] = hourAngleGreenw[counter] - 24.0;
    }
    
    
    /* return to calling function */
    
    if(returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    else
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
    
  
    return returnStatus;
    
}
