/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UTCtoUT1jd.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCtoUT1jd()
   This function converts UTC time in CCSDS ASCII Time Code to UT1 time as a
   Julian date.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   04-Apr-1995  GTSK  Initial version
   22-May-1997  PDN   Deleted material about predicted leap seconds
                      Updated NOTES

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Convert UTC to UT1 Julian date

NAME:     
   PGS_TD_UTCtoUT1jd()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCtoUT1jd(
       char         asciiUTC[28],
       PGSt_double  jdUT1[2])

 FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
   
      integer function pgs_td_utctout1jd(asciiutc,jdut1)
      character*27      asciiutc
      double precision  jdut1(2)


DESCRIPTION:
   This tool converts UTC time in CCSDS ASCII Time Code to UT1 time as a
   Julian date.

INPUTS:
   Name         Description                Units    Min           Max
   ----         -----------                -----    ---           ---
   asciiUTC     UTC time in CCSDS ASCII    ASCII     ** see NOTES **
                Time Code (format A or B)

OUTPUTS:
   Name         Description                Units    Min           Max
   ----         -----------                -----    ---           ---
   jdUT1        UT1 Julian date as two     days      ** see NOTES **
                real numbers, the first
		a half integer number of
		days and the second the
		fraction of a day between
		this half integer number
		of days and the next half
		integer day number.

RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSCSC_W_PREDICTED_UT1      status of UT1-UTC correction is predicted
   PGSTD_E_TIME_FMT_ERROR      error in format of input ASCII UTC time
   PGSTD_E_TIME_VALUE_ERROR    error in value of input ASCII UTC time
   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for
                               input time
   PGSTD_E_NO_UT1_VALUE        no UT1-UTC correction available
   PGS_E_TOOLKIT               something radically wrong occurred

EXAMPLES:
   None

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
   PGSTK - 1170, 1210, 1215, 1220

DETAILS:
   This tool converts UTC time (in CCSDS ASCII Time Code) to UT1 Julian date.
   This function calls PGS_TD_UTCtoUTCjd() to convert the input ASCII UTC time
   to UTC Julian date.  If the input time is occurring during a leap second
   (i.e. 60. <= seconds < 61.)  then PGS_TD_UTCtoUTCjd will return the status
   PGSTD_M_LEAP_SEC_IGNORED.  This function will then set onLeap to PGS_TRUE to
   indicate that the input time is occurring during a leap second.
   PGS_TD_UTCjdtoUT1jd() is then called with the appropriate value of onLeap to
   get the UT1 Julian Date.

GLOBALS:  
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_UTCtoUTCjd()
   PGS_TD_UTCjdtoUT1jd()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoUT1jd()"

PGSt_SMF_status
PGS_TD_UTCtoUT1jd(                /* convert UTC (ASCII) to UT1 Julian Date */
    char            asciiUTC[28], /* UTC in CCSDS ASCII Time Code (A or B) */
    PGSt_double     jdUT1[2])     /* UT1 as a Julian Date */
{
    PGSt_SMF_status returnStatus; /* return status of this function */

    PGSt_double     jdUTC[2];     /* UTC time as a Julian Date */

    PGSt_boolean    onLeap;       /* T/F value indicating if input time is
				     occurring during a leap second */

    /* initialize onLeap to be false (the usual case) */

    onLeap = PGS_FALSE;
    
    /* Convert UTC time in CCSDS ASCII Time Code format (A or B) to UTC time as
       a Julian Date (in SDP Toolkit Julian Date format). */

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

    /* Convert UTC time as a Julian Date to UT1 time as a Julian Date (in SDP
       Toolkit Julian Date format). */

    returnStatus = PGS_TD_UTCjdtoUT1jd(jdUTC,onLeap,jdUT1);
    
    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,FUNCTION_NAME);
    
    return returnStatus;
}
