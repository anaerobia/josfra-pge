/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_UTCtoTDTjed.c

DESCRIPTION:
  This file contains the function PGS_TD_UTCtoTDTjed()

AUTHOR:
  Anubha Singhal / Applied Research Corp. 
  Peter Noerdlinger / Applied Research Corp.
	
HISTORY:
  10-Jul-1994 PDN Designed Version
  12-Jul-1994 AS Initial version
  29-Jul-1994 AS Modified to use two doubles for the Julian day number
  22-May-1997 PDN Removed cases/comments on PRED_LEAPS

END_FILE_PROLOG:
******************************************************************************/

/******************************************************************************
BEGIN_PROLOG:

TITLE:   
      Convert UTC in CCSDS ASCII Time (Format A or B) to TDT as a Julian Date

NAME: 
      PGS_TD_UTCtoTDTjed()

SYNOPSIS:
C:   
      #include <PGS_TD.h>

      PGSt_SMF_status
      PGS_TD_UTCtoTDTjed(
             char        asciiUTC[28],
             PGSt_double jedTDT[2]);
   
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'

      integer function pgs_td_utctotdtjed(asciiutc,jedtdt)
      char*27           asciiutc
      double precision  jedtdt[2]

DESCRIPTION:
      This tool converts UTC in CCSDS ASCII time format A or B to TDT as a 
      Julian date (TDT = Terrestrial Dynamical Time)
      
INPUTS:
      Name      Description              Units    Min         Max
      ----      -----------              -----    ---         ---
      asciiUTC  UTC time in CCSCS        time     1961-01-01  ** see NOTES **
                ASCII time Code A  
                or B format

OUTPUTS:
      Name       Description             Units     Min        Max
      ----       -----------             -----     ---        ---
      jedTDT     TDT as a Julian date    days      ** see NOTES **

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
      PGSt_double       jedTDT[2];
      char err[PGS_SMF_MAX_MNEMONIC_SIZE];
      char msg[PGS_SMF_MAX_MSG_SIZE];     

      returnStatus = PGS_TD_UTCtoTDTjed(asciiUTC,jedTDT);
      if(returnStatus != PGS_S_SUCCESS)
	  {
	       PGS_SMF_GetMsg(&returnStatus,err,msg);
               printf("\nERROR: %s",msg);  
	  }
     
FORTRAN:
      implicit none
      integer           pgs_td_utctotdtjed
      integer           returnstatus
      character*27      asciiutc
      double precision  jedtdt[2]
      character*33      err
      character*241     msg

      asciiutc = '1998-06-30T10:51:28.320000Z'
      returnstatus = pgs_td_utctotdtjed(asciiutc,jedtdt)
      if(returnstatus .ne. pgs_s_success) then
	      returnstatus = pgs_smf_getmsg(returnstatus,err,msg)
	      write(*,*) err, msg
      endif
      
NOTES:

   TIME ACRONYMS:
     
     MJD is:  Modified Julian Date
     TAI is:  International Atomic Time
     TJD is:  Truncated Julian Date
     TDT is:  Terrestrial Dynamical Time
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
      PGSTK-1190, 1215

DETAILS:
      This function passes the UTC time to PGS_TD_UTCtoTAIjd() which
      parses the time string and converts it to the equivalent TAI
      Julian date (two double precision numbers are used). It then adds 
      32.184 seconds (converted to days) to obtain TDT as a Julian date.
      
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
      None

FILES:
      None

FUNCTIONS CALLED:
      PGS_TD_UTCtoTAIjd()
      PGS_SMF_SetStaticMsg()
      PGS_SMF_SetUnknownMsg()
           
END_PROLOG:
******************************************************************************/

#include <PGS_TD.h>  
       
PGSt_SMF_status
PGS_TD_UTCtoTDTjed(                /* converts UTC in CCSDS ASCII time format
                                      A or B to TDT as a Julian date */
        
    char        *asciiUTC,         /* UTC as a CCSDS ASCII time code A or B
                                      string */
    PGSt_double jedTDT[2])         /* TDT as a Julian date */
{
   
    PGSt_double     jdTAI[2];      /* TAI as a Julian date */
    
    PGSt_SMF_status returnStatus;  /* value returned by function indicating
                                      success or the nature of any errors */

    /* default answer in case of failure */
 
    jedTDT[0] = 0.0;
    jedTDT[1] = 0.0;
    
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
	PGS_SMF_SetUnknownMsg(returnStatus,"PGS_TD_UTCtoTDTjed()");
	return PGS_E_TOOLKIT;
    }
            
    /* get TDT from TAI by adding 32.184 seconds converted to days */
    
    jedTDT[0] = jdTAI[0];
    jedTDT[1] = jdTAI[1] + 32.184 / 86400.0; 

    /* make sure the fractional part of jedTDT is between 0.0 and 1.0 */

    if (jedTDT[1] >= 1.0)
    {
	jedTDT[1] = jedTDT[1] - 1.0;
	jedTDT[0] = jedTDT[0] + 1.0;
    }
    /* return to calling function */
    if (returnStatus == PGS_S_SUCCESS)
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_TD_UTCtoTDTjed()");
    return returnStatus;
}
