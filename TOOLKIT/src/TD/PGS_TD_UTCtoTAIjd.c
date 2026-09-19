/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_UTCtoTAIjd.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCtoTAIjd()
                                         
AUTHOR:
   Peter D. Noerdlinger / Applied Research Corp
   Anubha Singhal       / Applied Research Corp
   Guru Tej S. Khalsa   / Applied Research Corp

HISTORY:
      Feb-1994 PDN   Initial Version created
   01-Apr-1994 UP    Modified prolog
   20-Jul-1994 PDN   Enhanced error reporting due to new Time Check code 
                     which accepts A or B format and returns more informative
                     error messages
   25-Jul-1994 AS    Enhanced error reporting from call to PGS_TD_LeapSec
                     and modified prolog
   28-Jul-1994 AS    Modified to use two doubles to store Julian Days
   19-Aug-1994 AS    Modified to ensure that only real leap seconds are
                     allowed in time strings
   06-Jun-1994 GTSK  Complete rewrite, replaced redundant code with calls to
                     appropriate lower level functions.
   12-Jul-1995 PDN   Fixed prolog, "details" section
   22-May-1997 PDN   Fixed NOTES; deleted material about PRED_LEAPS

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:  
   Convert ASCII UTC to TAI Julian Date

NAME:    
   PGS_TD_UTCtoTAIjd

SYNOPSIS
C:
    #include PGS_TD.h
    
    PGSt_SMF_status
    PGS_TD_UTCtoTAIjd(
           char         asciiUTC[28],
           PGSt_double  jdTAI[2]);

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'

      integer function pgs_td_utctotaijd(asciiutc,jdtai)
      char*27           asciiutc
      double precision  jdtai[2]

DESCRIPTION:
   This tool converts UTC in CCSDS ASCII time format A or B to TAI as a 
   Julian date.

INPUTS:
   NAME      DESCRIPTION                  UNITS       MIN        MAX
   ----      -----------                  -----       ---        ---
   asciiUTC  UTC as a CCSDS ASCII time    time        Jan 1 1961 see NOTES
             code A or B string

OUTPUTS:
   NAME      DESCRIPTION                  UNITS       MIN        MAX
   ----      -----------                  -----       ---        ---
   jdTAI     TAI as a Julian date         days        2441317.5  invocation
                                                                 (see NOTES)

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_TIME_FMT_ERROR      error in format of input ASCII UTC time
   PGSTD_E_TIME_VALUE_ERROR    error in value of input ASCII UTC time
   PGSTD_E_NO_LEAP_SECS        assigned to errors coming back from
                               PGS_TD_Leapsec
   PGS_E_TOOLKIT               something unexpected happened, execution of
                               function terminated prematurely

EXAMPLES:
C:
     PGSt_SMF_status      returnStatus
     char                 asciiUTC[28]="1994-07-27T11:04:57.987654Z"
     PGSt_double          jdTAI[2]
     char                 err[PGS_SMF_MAX_MNEMONIC_SIZE];
     char                 msg[PGS_SMF_MAX_MSG_SIZE];  

     returnStatus = PGS_TD_UTCtoTAIjd(asciiUTC,jdTAI)
     if(returnStatus != PGS_S_SUCCESS)
     {
          PGS_SMF_GetMsg(&returnStatus,err,msg);
          printf("\nERROR: %s",msg);
      }

FORTRAN:
      implicit none
      integer           pgs_td_utctotaijd
      integer           returnstatus
      character*27      asciiutc
      double precision  jdtai[2]
      character*33      err
      character*241     msg

      asciiutc = '1998-06-30T10:51:28.320000Z'
      returnstatus = pgs_td_utctotaijd(asciiutc,jdtai)
      if(returnstatus .ne. pgs_s_success) then
	      returnstatus = pgs_smf_getmsg(returnstatus,err,msg)
	      write(*,*) err, msg
      endif

NOTES:
   TIME ACRONYMS:
     
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
  
   JULIAN DATES:

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
   PGS_TD_UTCtoTAI requires   leapsec.dat
         	  
FUNCTIONS_CALLED:
   PGS_TD_UTCtoTAI         -    covert UTC to TAI as seconds since 12 AM UTC
                                1-1-1993 (toolkit internal time)
   PGS_TD_TAItoTAIjd       -    covert TAI (toolkit internal time) to the 
                                equivalent TAI Julian date

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>          /*  Header file for Time and Date Tools */
       
/* constants */

#define SECONDSperMINUTE  60.0       /* number of seconds in a minute */
#define SECONDSperHOUR    3600.0     /* number of seconds in an hour */
#define SECONDSperDAY     86400.0    /* number of seconds in a day */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoTAIjd()"

PGSt_SMF_status
PGS_TD_UTCtoTAIjd(                /* converts ASCII UTC to TAI Julian date */
    char           asciiUTC[28],  /* UTC in CCSDS ASCII Time Code (A or B) */
    PGSt_double    jdTAI[2])      /* TAI as a Julian date */
{
    PGSt_double     secTAI93;     /* toolkit internal time */
    PGSt_SMF_status returnStatus; /* return status of this function */

    returnStatus = PGS_TD_UTCtoTAI(asciiUTC,&secTAI93);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	PGS_TD_TAItoTAIjd(secTAI93,jdTAI);
      default:
	return returnStatus;
    }
}
