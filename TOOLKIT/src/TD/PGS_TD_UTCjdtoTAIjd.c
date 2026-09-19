/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_UTCjdtoTAIjd.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCjdtoTAIjd().
   The function converts UTC as a Julian date to TAI as a Julian date.
                                         
AUTHOR:
   Peter D. Noerdlinger / Applied Research Corp.
   Anubha Singhal / Applied Research Corp.
   Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
   15-Dec-1994  GTSK   Initial version--based on code by PDN and AS.
   07-Jun-1995  GTSK   Altered algorithm to check for occurrence of a negative
                       leap second event (highly unlikely, but possible).
   04-Jul-1995  GTSK   Fixed minor bug that was causing an erroneous time to be
                       returned for times very near a leap second  event IF the
		       same array was passed in for both input and output.
   12-Jul-1995  PDN    Fixed prolog.
   25-Jul-1995  GTSK   Improved efficacy and efficiency of leap second event
                       testing schemes.
   22-May-1997  PDN    Fixed NOTES and code on PRED_LEAPS no longer supported

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:  
   Convert UTC Julian date to TAI Julian date

NAME:    
   PGS_TD_UTCjdtoTAIjd()

SYNOPSIS
 C:
   #include PGS_TD.h

   PGSt_SMF_status
   PGS_TD_UTCjdtoTAIjd(
          PGSt_double  jdUTC[2],
	  PGSt_boolean onLeap,
	  PGSt_double  jdTAI[2]);

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'

      integer function pgs_td_utcjdtotaijd(jdutc,onleap,jdtai)
      double precision  jdutc[2]
      integer           onleap
      double precision  jdtai[2]

DESCRIPTION:
   This tool converts UTC as a Julian date to TAI as a Julian date.

INPUTS:
   NAME      DESCRIPTION                  UNITS       MIN        MAX
   ----      -----------                  -----       ---        ---
   jdUTC     UTC as a Julian date         days        2441317.5  invocation
                                                                 (see notes)
   onLeap    set to PGS_TRUE to indicate  T/F
             that the current time is
	     occurring during a leap
	     second, otherwise set to
	     PGS_FALSE
 
OUTPUTS:
   NAME      DESCRIPTION                  UNITS       MIN        MAX
   ----      -----------                  -----       ---        ---
   jdTAI     TAI as a Julian date         days        2441317.5  invocation
                                                                 (see notes)

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_TIME_VALUE_ERROR    error in value of input UTC time
   PGSTD_E_NO_LEAP_SECS        assigned to errors coming back from
                               PGS_TD_Leapsec
   PGS_E_TOOLKIT               something unexpected happened, execution of
                               function terminated prematurely
EXAMPLES:
 C:
   PGSt_SMF_status      returnStatus
   PGSt_double          jdUTC[2]={2449305.5,0.0}
   PGSt_double          jdTAI[2]
   char                 err[PGS_SMF_MAX_MNEMONIC_SIZE];
   char                 msg[PGS_SMF_MAX_MSG_SIZE];  

   returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC,PGS_FALSE,jdTAI)
   if (returnStatus != PGS_S_SUCCESS)
   {
       PGS_SMF_GetMsg(&returnStatus,err,msg);
       printf("\nERROR: %s",msg);
    }

 FORTRAN:
      implicit none
      integer           pgs_td_utcjdtotaijd
      integer           returnstatus
      double precision  jdutc(2)
      double precision  jdtai(2)
      character*33      err
      character*241     msg

      jdutc(1) = 2449305.5D0
      jdutc(2) = 0.0D0
      returnstatus = pgs_td_utcjdtotaijd(jdutc,onleap,jdtai)
      if (returnstatus .ne. pgs_s_success) then
	  returnstatus = pgs_smf_getmsg(returnstatus,err,msg)
	  write(*,*) err, msg
      endif

 NOTES:

   TIME ACRONYMS:
     
     MJD is:  Modified Julian Date
     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES for TRANSFORMATIONS BETWEEN UTC and TAI:

     The minimum and maximum times that can successfully be processed by 
     this function depend on the file "leapsec.dat" which relates leap 
     second (TAI-UTC) values to UTC Julian dates. The file "leapsec.dat" 
     contains dates of new leap seconds and the total leap seconds for 
     times on and after Jan 1, 1972.  For times between Jan 1, 1961 and 
     Jan 1, 1972 it contains coefficients for an approximation supplied 
     by the International Earth Rotation Service (IERS) and the United 
     States Naval Observatory (USNO).  The Toolkit then uses these 
     coefficients in an algorithm consistent with IERS/USNO usage. For 
     times after Jan 1, 1961, but before the last date in the file, the
     Toolkit sets TAI-UTC equal to the total number of leap seconds to 
     date, (or to the USNO/IERS approximation, for dates before Jan 1,
     1972). If an input date is before Jan 1, 1961 the Toolkit sets the
     leap seconds value to 0.0.  This is consistent with the fact that,
     for civil timekeeping since 1972, UTC replaces Greenwich Mean Solar 
     Time (GMT), which had no leap seconds. Thus for times before Jan 1, 
     1961, the user can, for most Toolkit-related purposes, encode 
     Greenwich Mean Solar Time as if it were UTC.  If an input date
     is after the last date in the file, or after Jan 1, 1961, but the 
     file cannot be read, the function will use a calculated value of 
     TAI-UTC based on a linear fit of the data known to be in the table
     as of early 1997.  This value is a crude estimate and may be off by 
     as much as 1.0 or more seconds.  If the data file, "leapsec.dat", 
     cannot be opened, or the time is outside the range from Jan 1, 1961 
     to the last date in the file, the return status level will be 'E'.  
     Even when the status level is 'E', processing will continue, using 
     the default value of TAI-UTC (0.0 for times before Jan 1, 1961, or 
     the linear fit for later times). Thus, the user should always 
     carefully check the return status.  

     The file "leapsec.dat" is updated when a new leap second is 
     announced by the IERS, which has been, historically, about once 
     every year or two. 


   TIME BOUNDARIES for TRANSFORMATIONS BETWEEN UT1 and OTHER TIMES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file utcpole.dat which relates UT1 - UTC values to
     UTC dates.  The file "utcpole.dat" starts at Jan 1, 1972; therefore,
     the correction UT1 - UTC, which can be as large as (+-) 0.9 s, is not
     available until that date, and an error status message will be returned if
     this function is called with a time before that date.  The file
     utcpole.dat, which is maintained periodically, contains final (definitive),
     and predicted values (partially reduced data and short term predictions of 
     good accuracy).  The predictions run about a year ahead of real time. By
     that time, the error in UT1 is generally equivalent to 10 to 12 meters of
     equivalent Earth surface motion. Thus, when the present function is used,
     users should carefully check the return status.  A success status 
     message will be returned if all input times correspond to final values.
     A warning status message will be returned if predicted values are
     encountered.  An error message will be returned if the time requested is
     beyond the end of the predictions, or the file cannot be read.

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

   TRUNCATED JULIAN DATES:

     Truncated Julian dates follow the same conventions as those for Julian
     dates (above) EXCEPT that the truncated Julian day number is integral (NOT
     half-integral).  The truncated Julian date in any time stream has a day 
     number that is 2440000.5 days less than the Julian date day number and an
     identical day fraction (i.e. the truncated Julian date is a day count
     originating at 1968-05-24T00:00:00).

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

   PB5 TIME FORMAT:
    
     A 9 byte (72 bit) time format comprised of four binary counter values:

     ("timePB5" represents an array of 9 (unsigned) one byte elements)
          
     1)  A 16-bit value (the first two elements of array timePB5 each with 8 
	 bits) containing the truncated Julian date (integral day number).  The
         range of decimal values is 0-65535, computed as 256*element1+element2.
         The maximum decimal values of elements 1 and 2 are 255 and 255
         respectively.
     2)  A 24-bit value (elements 3, 4 and 5 of array timePB5, each with 8 bits)
         containing the seconds of the latest day.  The range of values is
         0-86401 seconds, computed as 256*256*element3+256*element4+element5.
     3)  A 16-bit value (elements 6 and 7 of array timePB5, each with 8 bits)
         containing the number of milliseconds of the latest second.  The range
	 of values is 0-999 milliseconds, computed as 256*element6+element7.
     4)  A 16-bit value (elements 8 and 9 of array timePB5, each with 8 bits)
         containing the number of microseconds of latest millisecond.  The range
         of values is 0-999 microseconds, computed as 256*element8+element9.
       
     This allows the PB5 format to represent times from 1968-05-24T00:00:00 to
     2147-10-28T23:59:59.999999.

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
   PGSTK - 1160, 1170, 1210

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   None

FILES:
   PGS_TD_LeapSec requires leapsec.dat
         	  
FUNCTIONS_CALLED:
   PGS_TD_LeapSec          -    get leap second value
   PGS_SMF_SetStaticMsg    -    set error/status message
   PGS_SMF_SetDynamicMsg   -    set error/status message
   PGS_SMF_SetUnknownMsg   -    set unknown error/status message

END_PROLOG:
*******************************************************************************/

#include <string.h>
#include <PGS_TD.h>               /*  Header file for Time and Date Tools */
       
/* name of this function */

#define FUNCTION_NAME   "PGS_TD_UTCjdtoTAIjd()"

/* constants */

#define SECONDSperDAY     86400.0 /* number of seconds in a day */

PGSt_SMF_status
PGS_TD_UTCjdtoTAIjd(              /* converts UTC as a Julian date to TAI as a
				     Julian date  */
    PGSt_double  jdUTC[2],        /* UTC as a Julian date */
    PGSt_boolean onLeap,          /* set to true if UTC time is occurring during
				     a positive leap second */
    PGSt_double  jdTAI[2])        /* TAI as a Julian date */
{
    PGSt_double  jdTemp[2];       /* temporary Julian date array */
    PGSt_double  leapSecs;        /* leap seconds value from tables read 
                                     by PGS_TD_LeapSec()  */
    PGSt_double  newleapSecs;     /* leap seconds value from tables read
                                     by second call to PGS_TD_LeapSec()  */
    PGSt_double  lastChangeJD;    /* Julian day number on which leap second
				     returned by PGS_TD_LeapSec() became (or is
				     predicted to become) effective */
    PGSt_double  newLastChangeJD; /* Julian day number on which leap second
				     returned by PGS_TD_LeapSec() became (or is
				     predicted to become) effective (used in
				     second call to PGS_TD_LeapSec()) */
    PGSt_double  nextChangeJD;    /* Julian day number on which next leap second
				     value became (or is predicted to become)
				     effective */
    PGSt_integer currentLeap=0;   /* current leap second value (0 or 1) as
				     determined by onLeap */
    char         leapStatus[10];  /* flag indicating actual or predicted leap
                                     seconds - for user-defined diagnostics
                                     and latency checks */
    PGSt_SMF_status returnStatus; /* default return value of function */
    PGSt_SMF_status returnStatus1;/* status/error return of SetStaticMsg */
    
    /* Start Executable Code */
    
    /* initialize returnStatus to indicate success */

    returnStatus = PGS_S_SUCCESS;
    
    /* Access Leap Seconds file; obtain current leap second value */

    returnStatus = PGS_TD_LeapSec(jdUTC, &leapSecs, &lastChangeJD, 
				  &nextChangeJD, leapStatus);
        
    /* check for error, reassign some of the errors coming back from 
       PGS_TD_LeapSec() since they are really more specific than
       required, set dynamic message to pass on specifics */
    
      switch (returnStatus)
      {
	case PGS_S_SUCCESS:


	  /* issue an error if the difference of TAI and UTC is predicted */
	  
	  if (strcmp(leapStatus,"PREDICTED") == 0)
	  {
	      returnStatus = PGS_E_TOOLKIT;
	      PGS_SMF_SetDynamicMsg(returnStatus,
                      "Obsolete leap seconds file format encountered ",
                                         FUNCTION_NAME);
	  }

	  /* If onLeap is set to true, then call PGS_TD_Leapsec with
	     the next julian date value. If the last change day is the same as
	     the last change day from the previous call, then a leap second
	     boundary has not been crossed and this is an invalid date for a
	     leap second. */

          if (onLeap == PGS_TRUE)
          {
	      /* set currentLeap to 1 */

	      currentLeap = 1;
	      
	      jdTemp[0] = jdUTC[0];
	      jdTemp[1] = jdUTC[1] + 1.0/SECONDSperDAY;
	  
	      returnStatus1 = PGS_TD_LeapSec(jdTemp,&newleapSecs,
					     &newLastChangeJD,&nextChangeJD,
					     leapStatus);

	      /* if the return status is not PGS_S_SUCCESS, we have most likely
		 fallen off the end of the table, don't harass users about leap
		 seconds if this is the case since the values are unknown at
		 that point anyway */

	      if (returnStatus1 != PGS_S_SUCCESS)
		  break;

              if ((jdTemp[1] > (1.0 + (newleapSecs-leapSecs)/SECONDSperDAY)) ||
		  (newLastChangeJD == lastChangeJD))
	      {
	           returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	           PGS_SMF_SetDynamicMsg(returnStatus,"error in UTC Julian "
					 "date: invalid date for leap second",
                                         FUNCTION_NAME);
              }

          }

	  /* If the input date is very near to the next date of change of leap
	     second value then check for the occurrence of a negative leap
	     second (very unlikely, but possible). */

	  else if ((jdUTC[0] + 1.0) == nextChangeJD)
	  {
	      /* if the day fraction is not very near midnight then there is no
		 need to check for a negative leap second occurrence, so break
		 here in that case (actually there is no need to check for the
		 occurrence of a leap second unless the day fraction indicates a
		 time of 23:59:59 or greater, but let's not push machine
		 accuracy by testing right at the boundary only) */

	      if (jdUTC[1] < .999)
		  break;
	      
	      jdTemp[0] = jdUTC[0];
	      jdTemp[1] = jdUTC[1] + 1.0/SECONDSperDAY;
	  
	      returnStatus1 = PGS_TD_LeapSec(jdTemp,&newleapSecs,&lastChangeJD,
					     &nextChangeJD,leapStatus);

	      /* if the return status is not PGS_S_SUCCESS, we have most likely
		 fallen off the end of the table, don't harass users about leap
		 seconds if this is the case since the values are unknown at
		 that point anyway */

	      if (returnStatus1 != PGS_S_SUCCESS)
		  break;

	      /* If (newleapSecs < leapSecs) then the next leap second event
		 is a negative leap second and the quantity 
		 (1.0 - (leapSecs - newleapSecs)/SECONDSperDAY) will be the
		 largest value that the current day fraction can be.  If the
		 current day fraction is greater than this value then it is
		 occurring during a UTC time which has been precluded by the
		 occurrence of the negative leap second event. */

	      if (jdUTC[1] >= (1.0 - (leapSecs - newleapSecs)/SECONDSperDAY))
	      {
	           returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	           PGS_SMF_SetDynamicMsg(returnStatus,"error in UTC Julian "
					 "date: a negative leap second event "
					 "precludes input date",FUNCTION_NAME);
              }
	  }
	  break;
	case PGSTD_W_JD_OUT_OF_RANGE:
	  returnStatus = PGSTD_E_NO_LEAP_SECS;
	  PGS_SMF_SetDynamicMsg(
		      returnStatus,
		      "input Julian day out of range for tabulated corrections"
		      " - approximate value used",
		      FUNCTION_NAME);
	  break;
	case PGSTD_W_DATA_FILE_MISSING:
	  returnStatus = PGSTD_E_NO_LEAP_SECS;
	  PGS_SMF_SetDynamicMsg(
		      returnStatus,
		     "unable to find or open leap second correction file:"
                     " leapsec.dat - an approximate value was used",
		     FUNCTION_NAME);
	  break;
	default:
	  PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	  return PGS_E_TOOLKIT;
      }

    /* calculate the TAI julian date:
       the Julian date is the Julian day and the fraction of the day */
    
    jdTAI[0] = jdUTC[0];
    jdTAI[1] = jdUTC[1] + ((leapSecs + currentLeap) / SECONDSperDAY);
    
    /* check to see if the day fraction is greater than or equal one - if so, 
       then add a day to the Julian day number and subtract one from the day 
       fraction so that the day fraction is between 0.0 and 1.0 */ 

    if (jdTAI[1] >= 1.0)
    {
	jdTAI[0] +=  1.0;
	jdTAI[1] -=  1.0;
    }
    
    /* return to calling function */

    if (returnStatus == PGS_S_SUCCESS)
        PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    return returnStatus;
}
