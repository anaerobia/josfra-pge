/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_TAItoISOint.c

DESCRIPTION:
   This file contains the function PGS_TD_TAItoISOint().
   The function converts TAI seconds since 12 AM UTC 1-1-1993 to UTC as an
   integer.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   06-Mar-1996  GTSK  Initial version.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert TAI To Integer UTC

NAME:
   PGS_TD_TAItoISOint()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_TAItoISOint(
       PGSt_double   secTAI93,
       PGSt_integer  *intISO)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_taitoisoint(sectai93, intiso)
      double precision sectai93
      integer          intiso

DESCRIPTION:
   The function converts TAI seconds since 12 AM UTC 1-1-1993 to UTC as an
   integer.

INPUTS:
   NAME      DESCRIPTION                UNITS    MIN                  MAX
   ----      -----------		-----    ---                  ---
   secTAI93  continuous seconds since   seconds  -1009843225.577182   see NOTES
             12AM UTC Jan 1, 1993

OUTPUTS:	  
   NAME      DESCRIPTION                UNITS    MIN                  MAX
   ----      -----------		-----    ---                  ---
   intISO    UTC as an integer           N/A     N/A                  N/A
             (see NOTES)

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
			  
   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file, which is updated when a new
     leap second event is announced, contains actual (definitive) and predicted
     (long term; very approximate) dates of leap second events.  The latter can
     be used only in simulations.   If an input date is outside of the range of
     dates in the file (or if the file cannot be read) the function will use
     a calculated value of TAI-UTC based on a linear fit of the data known to be
     in the table.  This value of TAI-UTC is relatively crude estimate and may
     be off by as much as 1.0 or more seconds.  Thus, when the function is used
     for dates in the future of the date and time of invocation, the user ought
     to carefully check the return status.  The status level of the return 
     status of this function will be 'W' (at least) if the TAI-UTC value used
     is a predicted value.  The status level will be 'E' if the TAI-UTC value
     is calculated (although processing will continue in this case, using the
     calculated value of TAI-UTC).

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
   PGS_TD_ISOinttoUTC()
   PGS_TD_TAIjdtoUTCjd()
   PGS_TD_TAItoTAIjd()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_TAItoISOint()"

PGSt_SMF_status
PGS_TD_TAItoISOint(                 /* convert TAI to UTC as an integer */
    PGSt_double     secTAI93,       /* TAI seconds since 12 AM UTC 1-1-1993 */
    PGSt_integer    *intISO)        /* UTC integer in "ISO" format: YYMMDDHH */
{
    PGSt_SMF_status returnStatus;   /* return status of function calls */
    PGSt_SMF_status returnStatus1;  /* return status of function calls */
    
    PGSt_double     jdUTC[2];       /* UTC Julian date (toolkit format) */
    PGSt_double     jdTAI[2];       /* TAI Julian date (toolkit format) */
    
    /* convert TAI as seconds since 12 AM UTC 1-1-1993 to TAI Julian date */

    PGS_TD_TAItoTAIjd(secTAI93, jdTAI);
    
    /* convert TAI Julian date to UTC Julian date */

    returnStatus = PGS_TD_TAIjdtoUTCjd(jdTAI, jdUTC);
    
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_M_LEAP_SEC_IGNORED:
      case PGSTD_E_NO_LEAP_SECS:
	break;
	
      case PGS_E_TOOLKIT:
	return returnStatus;
	
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* convert UTC as a Julian date to integer UTC time */

    returnStatus1 = PGS_TD_UTCjdtoISOint(jdUTC, intISO);
    
    if (returnStatus1 != PGS_S_SUCCESS)
    {
	/* if returnStatus1 is NOT PGS_S_SUCCESS then it should take precedence
	   over the value of returnStatus from the call to PGS_TD_TAIjdtoUTCjd()
	   (above) */

	returnStatus = returnStatus1;
    }
    
    /* if returnStatus is success set the message buffer to indicate this, if it
       is NOT success then the message buffer has already been set by the lower
       level function where the anomaly occurred (i.e. either
       PGS_TD_TAIjdtoUTCjd() or PGS_TD_UTCjdtoISOint()) */

    if (returnStatus == PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
    }

    return returnStatus;
}
