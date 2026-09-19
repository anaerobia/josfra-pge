/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UTCtoTAI.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCtoTAI()
   This function converts UTC time in CCSDS ASCII Time Code (A or B format) to
   Toolkit internal time (real continuous seconds since 12AM UTC 1-1-93).

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   30-Mar-1994  GTSK  Initial version
   18-Jul-1994  GTSK  Increased accuracy by splitting Julian date into
                      two doubles: jdTAI and dayFraction.  No longer calling
		      PGS_TD_UTCtoTAIjd, cribbed much of the code from that
		      function (by Peter D. Noerdlinger) for direct use here.
		      Corrected algorithm to allow for the unlikely event of
		      a negative leap second.
   28-Jul-1994  GTSK  Added call to PGS_TD_UTCtoTAIjd() which now keeps track
                      of TAI Julian date in two (count 'em, 2) "double"
		      variables so the accuracy is fine now.  Removed much
		      of the code alluded to in above message.
   03-Mar-1994  GTSK  Extracted all real functionality to lower level functions.
                      This tool now just strings together the appropriate lower
		      level functions.
   22-May-1997  PDN   Removed code/references to PGSTD_W_PRED_LEAPS
   28-Oct-1999  PDN   Added a call to PGS_TD_timeCheck to prevent core dumps down
                      the road when leap seconds file is extremely corrupt

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert UTC to TAI Time

NAME:
   PGS_TD_UTCtoTAI()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCtoTAI(
       char         asciiUTC[28],
       PGSt_double  *secTAI93)
     
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'
      
      integer function pgs_td_utctotai(asciiutc,sectai93)
      character*27      asciiutc
      double precision  sectai93
      
DESCRIPTION:
   This function converts UTC time in CCSDS ASCII Time Code (A or B format) to
   Toolkit internal time (real continuous seconds since 12AM UTC 1-1-93).

INPUTS:
   Name       Description                Units    Min                  Max
   ----       -----------                -----    ---                  ---
   asciiUTC   UTC time in CCSDS ASCII    time     1961-01-01T00:00:00  see NOTES
              Time Code A format or
	      ASCII Time Code B format

OUTPUTS:
   Name       Description                Units    Min                  Max
   ----       -----------                -----    ---                  ---
   secTAI93   continuous seconds since   seconds  -1009843225.5        see NOTES
              12AM UTC Jan 1, 1993
             
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for
                               input time
   PGSTD_E_TIME_FMT_ERROR      error in format of input ASCII UTC time
   PGSTD_E_TIME_VALUE_ERROR    error in value of input ASCII UTC time
   PGS_E_TOOKIT                something unexpected happened, execution aborted
	
EXAMPLES:
C:
   PGSt_SMF_status  returnStatus;
   char             asciiUTC[28];
   PGSt_double      secTAI93;

   strcpy(asciiUTC,"1993-01-02T00:00:00.000000Z");
   returnStatus = PGS_TD_UTCtoTAI(asciiUTC,&secTAI93);
   if (returnStatus != PGS_S_SUCCESS)
   {
    *** do some error handling ***
                  :
	          :
   }
	   
   printf("TAI: %f\n", (double) secTAI93);

FORTRAN:
      implicit none

      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'

      integer          pgs_td_utctotai

      integer          returnstatus

      character*27     asciiutc

      double precision sectai93

      asciiutc = '1993-01-02T00:00:00.000000Z'
      returnstatus = pgs_td_utctotai(asciiutc,sectai93)
      if (returnstatus .ne. pgs_s_success) goto 999
      write(6,*) 'TAI: ', sectai93

 999  *** do some error handling ***
                    :
                    :

NOTES:
   TIME ACRONYMS:
     
     TAI is: International Atomic Time
     UTC is: Universal Coordinated Time
			  
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

REQUIREMENTS:
   PGSTK - 1170, 1210

DETAILS:
   This tool converts UTC time to TAI time.  Input is a string in CCSDS ASCII
   Time Code (Format A or B).  The function PGS_TD_UTCtoUTCjd() then converts
   that input to the equivalent UTC Julian date.  Note that there is no way to
   uniquely indicate the occurrence of a leap leap second in a UTC Julian date
   and the function PGS_TD_UTCtoUTCjd() will return a status message indicating
   that the leap second has been ignored if the input ASCII UTC date occurs
   during a leap second.  If this is the case then the variable onLeap is set to
   PGS_TRUE to indicate that the input time is occurring during a leap second.
   This function then calls PGS_TD_UTCjdtoTAIjd which converts the UTC Julian
   date to the equivalent TAI Julian date.  PGS_TD_UTCjdtoTAIjd is passed the
   value of the variable onLeap and will properly adjust the TAI Julian date if
   the input time is occurring during a leap second.  Finally this function
   calls PGS_TD_TAIjdtoTAI() to convert the TAI Julian date to Toolkit Internal
   time (real continuous seconds since 12AM UTC January 1st, 1993).

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_timeCheck()      - verify format of ASCII time
   PGS_TD_UTCtoUTCjd()     - convert ASCII UTC to UTC Julian date
   PGS_TD_UTCjdtoTAIjd()   - convert UTC Julian date to TAI Julian date
   PGS_TD_TAIjdtoTAI()     - convert TAI Julian date to Toolkit Internal time
   PGS_SMF_SetUnknownMsg() - set error message
   PGS_SMF_SetStaticMsg()  - set error message

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoTAI()"

PGSt_SMF_status
PGS_TD_UTCtoTAI(                  /* convert ASCII UTC time to internal time */
    char            asciiUTC[28], /* UTC time in CCSDS ASCII Format (A or B) */
    PGSt_double     *secTAI93)    /* equivalent toolkit internal time */
{
    PGSt_double     jdTAI[2];     /* TAI Julian Date equivalent of input time */
    PGSt_double     jdUTC[2];     /* UTC Julian Date equivalent of input time */

    PGSt_boolean    onLeap;       /* indicates if input time occurs during a
				     leap second (TRUE) or not (FALSE) */

    PGSt_SMF_status returnStatus; /* return status of this function */

    /* Initialize onLeap to be false (usual case).  Call PGS_TD_UTCtoUTCjd() to
       convert input time from CCSDS ASCII Format to the equivalent Julian date
       expressed as two real numbers (toolkit Julian date format). */

    onLeap = PGS_FALSE;

    /* prevent feeding a bad ASCII string to PGS_TD_UTCtoUTCjd() */

    /* call PGS_TD_timeCheck() which checks the time string for format
       or value errors.  PGS_TD_timeCheck also indicates if the input time
       string is in CCSDS ASCII Time Code B format.  If the input string
       is in ASCII B format PGS_TD_ASCIItime_BtoA is called which returns
       ASCII A format.  This function assumes CCSDS ASCII Time Code A format
       when parsing the time string so this step ensures that the appropriate
       format is used. */
 
    returnStatus = PGS_TD_timeCheck(asciiUTC);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
        break;
      case PGSTD_M_ASCII_TIME_FMT_B:
        returnStatus = PGS_S_SUCCESS;
        break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
        return returnStatus;
      default:
        PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
        return PGS_E_TOOLKIT;
    }

    returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC,jdUTC);
    switch (returnStatus)
    {
      case PGSTD_M_LEAP_SEC_IGNORED:
	onLeap = PGS_TRUE;
      case PGS_S_SUCCESS:
	break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Convert UTC Julian date to TAI Julian date.  For times after 1-1-1972
       this is just an integer number of seconds difference. */

    returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC,onLeap,jdTAI);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	break;
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Convert TAI Julian date (in toolkit julian date format) to toolkit
       internal time (real continuous seconds since 12 AM UTC 1-1-1993). */

    *secTAI93 = PGS_TD_TAIjdtoTAI(jdTAI);
    
    if (returnStatus == PGS_S_SUCCESS)
      PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);

    return returnStatus;
}
