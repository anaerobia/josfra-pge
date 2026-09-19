/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_TAItoUTC.c

DESCRIPTION:
   This file contains the function PGS_TD_TAItoUTC()
   This tool converts Toolkit internal time (real continuous seconds since
   12AM UTC 1-1-93) to UTC time in CCSDS ASCII Time Code A format.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter Noerdlinger  / Applied Research Corporation

HISTORY:
   05-Apr-1994  GTSK  Initial version
   22-Jul-1994  GTSK  Altered algorithm to increase accuracy, modified prologs
                      to conform to latest ECS/PGS standards
   28-Jul-1994  GTSK  Further altered algorithm to increase accuracy.  Now
                      lugging around all Julian dates as two real numbers.
		      Modified comments to explain this new more accurate and
		      simpler scheme.
   03-Mar-1994  GTSK  Extracted all real functionality to lower level functions.
                      This tool now just strings together the appropriate lower
                      level functions.
   21-May-1997  PDN   Fixed NOTES
   22-May-1997  PDN   Deleted return value and branch on predicted leap seconds

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Convert TAI to UTC Time

NAME:     
   PGS_TD_TAItoUTC()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_TAItoUTC(
       PGSt_double  secTAI93,
       char         asciiUTC[28])
     
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'
   
      integer function pgs_td_taitoutc(sectai93,asciiutc)
      double precision  sectai93
      character*27      asciiutc

DESCRIPTION:
   This tool converts Toolkit internal time (real continuous seconds since
   12AM UTC 1-1-93) to UTC time in CCSDS ASCII Time Code A format.

INPUTS:
   Name       Description                Units    Min                  Max
   ----       -----------                -----    ---                  ---
   secTAI93   continuous seconds since   seconds  -1009843225.577182   see NOTES
              12AM UTC Jan 1, 1993

OUTPUTS:
   Name       Description                Units    Min                  Max
   ----       -----------                -----    ---                  ---
   asciiUTC   UTC time in CCSDS ASCII    time     1961-01-01T00:00:00  see NOTES
              Time Code A format
          
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for
                               input time
   PGS_E_TOOLKIT               something radically wrong occurred

EXAMPLES:
C:
   PGSt_SMF_status  returnStatus;
   PGSt_double      secTAI93;
   char             asciiUTC[28];

   secTAI93 = 86400.;
   returnStatus = PGS_TD_TAItoUTC(secTAI93,asciiUTC);
   if (returnStatus != PGS_S_SUCCESS)
   {
    *** do some error handling ***
             :
	     :
   }
	   
   printf("UTC: %s\n",asciiUTC);

FORTRAN:
      integer          pgs_td_taitoutc
      integer          returnstatus
      double precision sectai93
      character*27     asciiutc

      sectai93 = 86400.D0
      returnstatus = pgs_td_taitoutc(sectai93,asciiutc)
      if (returnstatus .ne. pgs_s_success) goto 999
      write(6,*) 'UTC: ', asciiutc

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

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
   PGSTK - 1170, 1210, 1220

DETAILS:
   This tool converts TAI time to UTC time.  Input is in seconds since 12AM UTC
   January 1st, 1993.  The function PGS_TD_TAItoTAIjd() then converts that input
   to the equivalent TAI Julian date.  The TAI Julian date is then passed into
   the function PGS_TD_TAIjdtoUTCjd() which converts the TAI Julian date to the
   equivalent UTC Julian date.  Note that there is no way to uniquely indicate
   the occurrence of a leap leap second in a UTC Julian date and the function
   PGS_TD_TAIjdtoUTCjd() will return a status message indicating that the leap
   second has been ignored if the input TAI Julian date occurs during a leap
   second.  If this is the case then the variable onLeap is set to PGS_TRUE to
   indicate that the input time is occurring during a leap second.  This
   function then calls PGS_TD_UTCjdtoUTC() which converts the UTC Julian date to
   a string indicating the date (CCSDS ASCII Time Code - format A).
   PGS_TD_UTCjdtoUTC() is passed the value of the variable onLeap and will
   properly indicate if the input time is occurring during a leap second (the
   ASCII UTC time seconds field is allowed to run up to 60.999999).

GLOBALS:  
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_TAItoTAIjd()
   PGS_TD_TAIjdtoUTCjd()
   PGS_TD_UTCjdtoUTC()
   PGS_SMF_SetUnknownMsg()
   PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_TAItoUTC()"

PGSt_SMF_status
PGS_TD_TAItoUTC(                  /* convert internal time to ASCII UTC time */ 
    PGSt_double     secTAI93,	  /* toolkit internal time */ 
    char            asciiUTC[28]) /* UTC time in CCSDS ASCII Format (A or B) */
{        
    PGSt_double     jdTAI[2];	  /* TAI Julian Date equivalent of input time */
    PGSt_double     jdUTC[2];	  /* UTC Julian Date equivalent of input time */

    PGSt_boolean    onLeap;	  /* indicates if input time occurs during a
    				     leap second (TRUE) or not (FALSE) */    

    char            mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
                                                            PGS_SMF_GetMsg() */
    char            msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
                                                            PGS_SMF_GetMsg() */
    PGSt_SMF_status returnStatus1;/* return status of functions called */
    PGSt_SMF_status returnStatus; /* return status of this function */       

    /* Convert toolkit internal time (real continuous seconds since 12 AM UTC
       1-1-1993) to TAI Julian date (in toolkit julian date format). */

    PGS_TD_TAItoTAIjd(secTAI93,jdTAI);

    /* Initialize onLeap to be false (usual case).  Convert TAI Julian date to
       UTC Julian date.  For times after 1-1-1972 this is just an integer
       number of seconds difference. */

    onLeap = PGS_FALSE;
    returnStatus = PGS_TD_TAIjdtoUTCjd(jdTAI,jdUTC);
    switch (returnStatus)
    {
      case PGSTD_M_LEAP_SEC_IGNORED:
	onLeap = PGS_TRUE;
	returnStatus = PGS_S_SUCCESS;
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_GetMsg(&returnStatus1,mnemonic,msg);
	if (returnStatus1 != returnStatus)
	    PGS_SMF_GetMsgByCode(returnStatus,msg);
	break;
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Call PGS_TD_UTCjdtoUTC() to convert input time from Julian date expressed
       as two real numbers (toolkit Julian date format) to the equivalent CCSDS
       ASCII Format (Format A). */

    returnStatus1 = PGS_TD_UTCjdtoUTC(jdUTC,onLeap,asciiUTC);
    switch (returnStatus1)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGS_E_TOOLKIT:
	return PGS_E_TOOLKIT;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    else
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
    
    return returnStatus;
}
