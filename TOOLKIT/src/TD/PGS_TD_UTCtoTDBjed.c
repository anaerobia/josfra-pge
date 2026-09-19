/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_UTCtoTDBjed.c

DESCRIPTION:
  This file contains the function PGS_TD_UTCtoTDBjed()

AUTHOR:
  Peter D. Noerdlinger / Applied Research Corporation
  Anubha Singhal / Applied Research Corporation
	
HISTORY:
  02-Apr-1994  PDN Designed and coded            
  15-Jul-1994  AS  Code replicating function PGS_TD_UTCtoTAIjd
                   replaced with call to PGS_TD_UTCtoTAIjd
  29-Jul-1994  AS  Modified to use two doubles for the Julian day number
  12-Apr-1995  PDN Fixed prolog

END_FILE_PROLOG:
******************************************************************************/

/******************************************************************************
BEGIN_PROLOG:

TITLE:  
      Convert UTC in CCSDS ASCII Time (Format A or B) to TDB as a Julian Date

NAME:
      PGS_TD_UTCtoTDBjed

SYNOPSIS:
C:    
      include <PGS_TD.h>

      PGSt_SMF_status
      PGS_TD_UTCtoTDBjed(
             char        asciiUTC[28],
             PGSt_double jedTDB[2]);
   
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'

      integer function pgs_td_utctotdbjed(asciiutc,jedtdb)
      char*27           asciiutc
      double precision  jedtdb[2]
      
DESCRIPTION:
      This tool converts UTC in CCSDS ASCII time format A or B to TDB as a 
      Julian date (TDB = Barycentric Dynamical Time)

INPUTS:
      Name       Description            Units    Min         Max
      ----       -----------            -----    ---         ---
      asciiUTC   UTC time in CCSDS      time     1961-01-01  ** see NOTES **
                 ASCII time Code A 
                 or B format
OUTPUTS:
      Name       Description            Units    Min         Max
      ----       -----------            -----    ---         ---
      jedTDB     TDB as a Julian date   days     ** see NOTES **

RETURNS:
      PGS_S_SUCCESS              successful return 
      PGSTD_E_TIME_FMT_ERROR     error in format of input ASCII UTC time
      PGSTD_E_TIME_VALUE_ERROR   error in value of input ASCII UTC time
      PGSTD_E_NO_LEAP_SECS       leap second errors			 
      PGS_E_TOOLKIT              something unexpected happened, execution of
                                 function terminated prematurely
                                             
EXAMPLES:
C:
      PGSt_SMF_status   returnStatus;
      char              asciiUTC[28]="2002-06-30T11:04:57.987654Z";
      PGSt_double       jedTDB[2];
      char err[PGS_SMF_MAX_MNEMONIC_SIZE];
      char msg[PGS_SMF_MAX_MSG_SIZE];     

      returnStatus = PGS_TD_UTCtoTDBjed(asciiUTC,jedTDB);
      if(returnStatus != PGS_S_SUCCESS)
	  {
	      PGS_SMF_GetMsg(&returnStatus,err,msg);
	      printf("\nERROR: %s",msg);
	  }
      
FORTRAN:
      implicit none
      integer           pgs_td_utctotdbjed
      integer           returnstatus
      character*27      asciiutc
      double precision  jedtdb[2]
      character*33      err
      character*241     msg


      asciiutc = '1998-06-30T10:51:28.320000Z'
      returnstatus = pgs_td_utctotdbjed(asciiutc,jedtdb)
      if(returnstatus .ne. pgs_s_success) then
	      returnstatus = pgs_smf_getmsg(returnstatus,err,msg)
	      write(*,*) err, msg
      endif

NOTES:
   TIME ACRONYMS:
     
     TDB is:  Barycentric Dynamical Time
     UTC is:  Coordinated Universal Time

   Prior to 1984, there is no distinction between TDT and TDB; either one is
   denoted "ephemeris time" (ET). Also, the values before 1972 are based on
   U.S. Naval Observatory estimates, which are the same as adopted by the JPL
   Ephemeris group that produces the DE series of solar system ephemerides, such
   as DE200.

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
      PGSTK-1190, 1215

DETAILS:
      This function passes the UTC time to PGS_TD_UTCtoTAIjd() which
      parses the time string and converts it to the equivalent TAI
      Julian date (two double precision numbers are used). It then adds 
      32.184 seconds (converted to days) to obtain TDT Terrestrial
      Dynamical Time as a Julian date. Using TDT the mean anomaly (g)
      of the earth is calculated using the following formula:

      g = 6.24008 + 0.017202*(TDT-2451545.0)

      TDB is then  calculated from g and TDT as follows:

      TDB = TDT + (0.001658 * sin(g) + 0.000014 * sin(2*g))/86400.0

      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
      NONE

FILES:
      NONE

FUNCTIONS CALLED:
      PGS_TD_UTCtoTAIjd()
      PGS_SMF_SetStaticMsg()
      PGS_SMF_SetUnknownMsg()

END_PROLOG:
******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>             /*  Standard header file for math libraries */
#include <PGS_TD.h>           /*  Header file for Time and Date Tools */

PGSt_SMF_status
PGS_TD_UTCtoTDBjed(           /* converts UTC in CCSDS ASCII time format A or
                                 B to TDB as a Julian date */
    char        *asciiUTC,    /* UTC as a CCSDS ASCII time code A or B string */
    PGSt_double jedTDB[2])    /* TDB as a Julian date */
{
    PGSt_double     jdTAI[2];  /* TAI as a Julian Day */
    PGSt_double     jedTDT[2]; /* TDT (Terrestrial Dynamical Time) as a
                                  Julian Day */
    PGSt_double     g;        /* Mean Anomaly of the Earth (angle in radians be
                                 tween a vector from the Sun to the Earth and a
                                 vector from the Sun to the perihelion of the
                                 Earth's orbit); however for the calculation of
                                 the mean (as opposed to the true) anomaly, the
                                 Earth's annual motion about the Sun is averaged 
                                 to constant angular speed.  The vectors may
                                 actually be measured from the Solar System
                                 barycenter (TBD) (the difference is minor in
                                 EOSDIS context)  */

    PGSt_SMF_status returnStatus; /* value returned by function indicating
                                     success or the nature of any errors */
        
    /* default answer in case of failure */

    jedTDB[0] = 0.0;
    jedTDB[1] = 0.0;

    /* initialize return value and message to indicate success */

    returnStatus = PGS_S_SUCCESS;
        
    /* Call function PGS_TD_UTCtoTAIjd to convert UTC to TAI as a Julian date */

    returnStatus = PGS_TD_UTCtoTAIjd(asciiUTC,jdTAI);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
        break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGS_E_TOOLKIT:
        return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,"PGS_TD_UTCtoTDBjed()");
	return PGS_E_TOOLKIT;
    }
    
    /* get TDT from TAI by adding 32.184 seconds converted to days */
    
    jedTDT[1] =  jdTAI[1] + 32.184/86400.0; 
    jedTDT[0] =  jdTAI[0];
    
    /* Get mean anomaly of the Earth - see notes above */
	
    g = 6.24008 + 0.017202 * ((jedTDT[0] - 2451545.0) + jedTDT[1]);
	
    /* compute jedTDB converting seconds to days - see notes above*/

    jedTDB[1]  = jedTDT[1] + 
                (0.001658 * sin(g) + 0.000014 * sin(2*g)) / 86400.0;
    
    /* make sure that the fractional part of jedTDB is between 0.0 and 1.0  */
    
    
    if (jedTDB[1] >= 1.0)
    {
	jedTDB[1] = jedTDB[1] - 1.0;
	jedTDB[0] = jedTDT[0] + 1.0;
    }
    else
    	jedTDB[0] = jedTDT[0];
    
    /* return to calling function */
    if (returnStatus == PGS_S_SUCCESS)
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_TD_UTCtoTDBjed"); 
    return returnStatus;
}
