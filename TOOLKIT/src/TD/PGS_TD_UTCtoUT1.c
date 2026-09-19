/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UTCtoUT1.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCtoUT1()

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation
   Anubha Singhal       / Applied Research Corporation
   Guru Tej S. Khalsa   / Applied Research Corp
	
HISTORY:
   19-Jul-1994       PDN   Designed and coded            
   26-Jul-1994       AS    Enhanced error reporting from call to
                           PGS_TD_timecheck and PGS_CSC_UTC_UT1Pole
                           and modified prolog
   01-Aug-1994       AS    Modified to take out the leap second while
                           accessing the utcpole table
   08-Aug-1994       AS    Modified error reporting from call to
                           PGS_CSC_UTC_UT1Pole
   06-Jun-1994      GTSK   Complete rewrite, replaced redundant code with calls
                           to appropriate lower level functions.
   18-Jun-1996       PDN   Deleted check for obsolete return
   22-May-1997       PDN   Fixed NOTES; removed code for PRED_LEAPS

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:   
   Convert ASCII UTC to UT1
 
NAME:    
    PGS_TD_UTCtoUT1() 

SYNOPSIS:
C:
    #include <PGS_CSC.h>
    #include <PGS_TD.h>
    
    PGSt_SMF_status
    PGS_TD_UTCtoUT1(
           char         asciiUTC[28],
           PGSt_double  *secUT1);

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'
      include 'PGS_CSC_4.f'

      integer function pgs_td_utctout1(asciiutc,secut1)
      char*27           asciiutc
      double precision  secut1

DESCRIPTION:
   This tool converts a time from CCSDS ASCII Time (Format A or  B) to UT1

INPUTS:
   Name       Description         Units    Min                  Max
   ----       -----------         -----    ---                  ---
   asciiUTC   UTC time in CCSDS   time     1979-06-30T00:00:00  date (see NOTES)
              ASCII Time Code A            also see notes
              or B format                  

OUTPUTS:
   Name       Description         Units    Min                  Max
   ----       -----------         -----    ---                  ---
   secUT1     UT1 in seconds      sec      0.0                  86400.999999
              from midnight

RETURNS:
   PGS_S_SUCCESS               
   PGSTD_E_TIME_FMT_ERROR      
   PGSTD_E_TIME_VALUE_ERROR    
   PGSCSC_W_PREDICTED_UT1
   PGSTD_E_NO_UT1_VALUE
   PGS_E_TOOLKIT

EXAMPLES:
C:
    PGSt_SMF_status      returnStatus
    char                 asciiUTC[28]="2002-07-27T11:04:57.987654Z"
    PGSt_double          secUT1
    char                 err[PGS_SMF_MAX_MNEMONIC_SIZE];
    char                 msg[PGS_SMF_MAX_MSG_SIZE];  
    
    returnStatus = PGS_TD_UTCtoUT1(asciiUTC,&secUT1)
    if(returnStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_GetMsg(&returnStatus,err,msg);
	printf("\nERROR: %s",msg);
    }

FORTRAN:
      implicit none
      integer           pgs_td_utctout1
      integer           returnstatus
      character*27      asciiutc
      double precision  secut1
      character*33      err
      character*241     msg

      asciiutc = '2002-07-27T11:04:57.987654Z'
      returnstatus = pgs_td_utctout1(asciiutc,secut1)
      if(returnstatus .ne. pgs_s_success) then
	      returnstatus = pgs_smf_getmsg(returnstatus,err,msg)
	      write(*,*) err, msg
      endif

NOTES:
   Although UT1 was used for civil timekeeping before Jan 1, 1972, today UT1 is
   a measure of Earth rotation only; it is a measure of the angle of the
   Greenwich Meridian from the equinox of date such that 24 hours of SI seconds
   (86400 seconds) of TAI or TDT constitute one full revolution.  As such, it
   can be directly reduced to GAST, Greenwich Apparent Sidereal Time.
   
   Prior to Jan 1, 1972, either UT1 or, for a brief period, a variant called UT2
   that accounts for some of the periodic nonuniformities of Earth rotation,
   were used for time keeping.
   
   TIME ACRONYMS:
     
     GAST is: Greenwich Apparent Sidereal Time
     GMST is: Greenwich Mean Sidereal Time
     MJD is:  Modified Julian Date
     TAI is:  International Atomic Time
     TJD is:  Truncated Julian Date
     TDT is:  Terrestrial Dynamical Time
     UT1 is:  Universal Time
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

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
   PGSTK - 1170, 1215

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   NONE

FILES:
   The function PGS_TD_UTCjdtoUT1jd requires the files:
     utcpole.dat
     leapsec.dat

FUNCTION CALLS:
   PGS_TD_UTCtoUTCjd     -   convert ASCII UTC time to UTC Julian date
   PGS_TD_UTCjdtoUT1jd   -   convert UTC Julian date to UT1 Julian date
   PGS_SMF_SetStaticMsg  -   set error/status message
   PGS_SMF_SetDynamicMsg -   set error/status message
   PGS_SMF_SetUnknownMsg -   set error/status message for unknown message

END_PROLOG
*******************************************************************************/
  
#include <PGS_TD.h>             /* Header file for Time and Date Tools */
#include <PGS_CSC.h>            /* Header file for Coord. Transform Tools */

/* constants */

#define SECONDSperDAY    86400.0 /* number of seconds in a day */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoUT1()"

PGSt_SMF_status
PGS_TD_UTCtoUT1(                      /* convert CCSDS ASCII UTC to UT1 */
    char            asciiUTC[28],     /* UTC in CCSDS ASCII format (A or B) */
    PGSt_double     *secUT1)          /* UT1  in seconds from midnight  */
{
    PGSt_double     jdUTC[2];         /* input time as UTC Julian date */
    PGSt_double     jdUT1[2];         /* input time as UT1 Julian date */
    
    PGSt_boolean    onLeap=PGS_FALSE; /* T if input time indicates a leap
					 second, otherwise F (default case) */
    PGSt_SMF_status returnStatus;     /* value returned by this function */

    /* convert input CCSDS ASCII UTC time to equivalent UTC Julian date (note
       that if the input Julian date indicates a leap second is occurring
       (seconds field >= 60.), "onLeap" will be set to T) */

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

    /* convert UTC Julian date to equivalent UT1 Julian date */

    returnStatus = PGS_TD_UTCjdtoUT1jd(jdUTC,onLeap,jdUT1);
    
    switch(returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSCSC_W_PREDICTED_UT1:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSTD_E_NO_LEAP_SECS:
	break;
      case PGSTD_E_NO_UT1_VALUE:
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "no UT1-UTC value available for input UTC time, "
			      "assuming UT1-UTC value of 0.0",
			      FUNCTION_NAME);
	break;
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* calculate UT1 value */
    
    *secUT1 = jdUT1[1]*SECONDSperDAY;
    
    /* return to calling function */

    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);

    return returnStatus;
}
