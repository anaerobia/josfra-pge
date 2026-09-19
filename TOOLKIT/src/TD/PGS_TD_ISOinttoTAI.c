/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_ISOinttoTAI.c

DESCRIPTION:
   This file contains the function PGS_TD_ISOinttoTAI().
   The function converts UTC as an integer to TAI seconds since
   12 AM UTC 1-1-1993.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   06-Mar-1996  GTSK  Initial version.
   21-May-1997  PDN   Fixed Comments
   22-May-1997  PDN   Deleted code & References to Predicted Leap Seconds 

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert Integer UTC To TAI

NAME:
   PGS_TD_ISOinttoTAI()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_ISOinttoTAI(
       PGSt_integer  intISO,
       PGSt_double   *secTAI93)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_isointtotai(intiso,sectai93)
      integer          intiso
      double precision sectai93

DESCRIPTION:
   The function converts UTC as an integer to TAI seconds since
   12 AM UTC 1-1-1993.

INPUTS:
   NAME      DESCRIPTION                UNITS    MIN                  MAX
   ----      -----------		-----    ---                  ---
   intISO    UTC as an integer           N/A     N/A                  N/A
             (see NOTES)

OUTPUTS:	  
   NAME      DESCRIPTION                UNITS    MIN                  MAX
   ----      -----------		-----    ---                  ---
   secTAI93  continuous seconds since   seconds  -1009843225.577182   see NOTES
             12AM UTC Jan 1, 1993

RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for
                               input time
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
   N/A

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

   INTEGER UTC:

     Integer UTC is an integer representing UTC time to "hour" accuracy.

      The general format is:
 
          YYMMDDHH
 
          where:
              YY  represents the last two digits of the year (if YY is less than
	          50 the implied year is 20YY otherwise the implied year is
		  19YY).  Range is 00-99.

              MM  represents the month of the year.  Range is 01-12.

              DD  represents the day of the month.  Range is 01-31.

              HH  represents the hour of the day.  Range is 00-23.

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:  
   None

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_ISOinttoUTCjd()
   PGS_TD_UTCjdtoTAIjd()
   PGS_TD_TAIjdtoTAI()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_ISOinttoTAI()"

PGSt_SMF_status
PGS_TD_ISOinttoTAI(                  /* convert integer UTC time to TAI */
    PGSt_integer    intISO,          /* UTC integer in "ISO" format: YYMMDDHH */
    PGSt_double     *secTAI93)       /* TAI seconds since 12 AM UTC 1-1-1993 */
{
    PGSt_SMF_status returnStatus;    /* return status of function calls */
    
    PGSt_double     jdUTC[2];        /* UTC Julian date (toolkit format) */
    PGSt_double     jdTAI[2];        /* TAI Julian date (toolkit format) */
    
    /* convert integer UTC time to UTC as a Julian date */

    returnStatus = PGS_TD_ISOinttoUTCjd(intISO, jdUTC);
    if (returnStatus != PGS_S_SUCCESS)
    {
	return returnStatus;
    }
    
    /* convert UTC as a Julian date to TAI as a Julian date */

    returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC, PGS_FALSE, jdTAI);
    
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	break;
	
      case PGS_E_TOOLKIT:
	return returnStatus;
	
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* convert TAI as a Julian date to seconds since 12 AM UTC 1-1-1993 */

    *secTAI93 = PGS_TD_TAIjdtoTAI(jdTAI);
    
    /* if returnStatus is success set the message buffer to indicate this, if it
       is NOT success then the message buffer has already been set by the lower
       level function (PGS_TD_UTCjdtoTAIjd()) where the anomaly occurred */

    if (returnStatus == PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);
    }

    return returnStatus;
}
