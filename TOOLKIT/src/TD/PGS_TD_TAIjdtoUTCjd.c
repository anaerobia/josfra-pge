/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_TAIjdtoUTCjd.c

DESCRIPTION:
   This file contains the function PGS_TD_TAIjdtoUTCjd().
   This function converts TAI Julian date to UTC Julian date.

AUTHOR:
   Guru Tej S. Khalsa
   Peter D. Noerdlinger

HISTORY:
   14-Dec-1994  GTSK  Initial version
   21-May-1997  PDN   Fixed comments
   22-May-1997  PDN   Deleted code for PGSTD_W_PRED_LEAPS

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert TAI Julian Date to UTC Julian Date

NAME:
   PGS_TD_TAIjdtoUTCjd()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_TAIjdtoUTCjd(
       PGSt_double  jdTAI[2],
       PGSt_double  jdUTC[2])

DESCRIPTION:
   This function converts TAI Julian date to UTC Julian date.

INPUTS:
   Name         Description                 Units       Min   Max
   ----         -----------                 -----       ---   ---
   jdTAI        TAI Julian date             days        N/A   N/A

OUTPUTS:
   Name         Description                 Units       Min   Max
   ----         -----------                 -----       ---   ---
   jdUTC        UTC Julian date             days        N/A   N/A
          
RETURNS:
   PGS_S_SUCCESS               successful return
   PGSTD_M_LEAP_SEC_IGNORED    leap second portion of input time discarded
   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for
                               input time
   PGS_E_TOOLKIT               something radically wrong occurred

EXAMPLES:
C:
   What?  Are you serious?

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
   PGSTK - 1160, 1170, 1210

DETAILS:
   This tool converts TAI time to UTC time.  Input is TAI Julian date.  Then
   PGS_TD_LeapSec() is called which reads the file leapsec.dat and gets the
   value in seconds of TAI-UTC for the input time as well as the date that
   TAI-UTC last changed and whether this date is actual or predicted. The
   function PGS_TD_LeapSec() assumes an input of UTC Julian date, and is being
   called with TAI Julian date so this is really just a 1st guess.  The UTC
   Julian date is then assumed to be the TAI julian date less the value of
   TAI-UTC.  This new new value must then be checked to see if the adjustment
   brings the value below the last date of change for TAI-UTC.  If this is the
   case then PGS_TD_LeapSec() is again called with the new guess (TAI Julian
   date less TAI-UTC).  The new value obtained for the leap seconds (TAI-UTC) is
   compared with the value from the previous call.  If the new value is greater
   than the old value, the new value represents a negative leap second situation
   (highly unusual--this has never occurred but is theoretically possible).
   This means that in subtracting the value TAI-UTC from the original input TAI
   Julian date one less leap second was subtracted than should have been.  So
   one more second is subtracted to arrive at the correct UTC Julian date.  If
   the the new leap seconds value is greater than the old value (normally the
   case) TAI-UTC MAY have been over estimated by one second and the UTC Julian
   date is increased by one second. This value is then rechecked against the
   last change date, if the new UTC Julian date is once again greater than the
   last change date then it is occurring during a leap second and a notice is
   issued to indicate that the leap second is being ignored (since it cannot be
   represented in UTC time expressed as a Julian date).

GLOBALS:
   None

FILES:
   The function  PGS_TD_LeapSec() uses "leapsec.dat"

FUNCTIONS_CALLED:
   PGS_TD_LeapSec()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>
#include <PGS_math.h>
#include <string.h>

/* name of this function */

#define FUNCTION_NAME   "PGS_TD_TAIjdtoUTCjd()"

/* constants */

#define SECONDSperDAY   86400.0   /* number of seconds in a day */

PGSt_SMF_status
PGS_TD_TAIjdtoUTCjd(              /* converts TAI time to UTC time */
    PGSt_double  jdTAI[2],        /* TAI Julian date */
    PGSt_double  jdUTC[2])        /* UTC Julian date equivalent of input time */
{
    PGSt_boolean onLeap=PGS_FALSE;/* in a leap second (T) or not (F) */
    PGSt_double  leapSecs;        /* leap second correction factor */
    PGSt_double  lastChangeJD;    /* last date leap sec. correction was made */
    PGSt_double  nextChangeJD;    /* next date leap sec. correction was made */
    PGSt_double  newLeapSecs;     /* leap second correction factor from second
				     call to PGS_TD_LeapSecs() */
    PGSt_double  leapDiff;        /* leapSecs - newLeapSecs */
    PGSt_double  newChangeJD;     /* leap sec. change date from 2nd call to 
				     PGS_TD_LeapSec() */

    char         leapStatus[10];  /* indicates if actual or predicted leap
				     seconds used */
    char        newLeapStatus[10];/* indicates if actual or predicted leap
				     seconds used in 2nd call to 
				     PGS_TD_LeapSecs() */

    PGSt_SMF_status returnStatus; /* return value of function */


    returnStatus = PGS_S_SUCCESS;

    /* Get the value of TAI-UTC (leapSecs) and check for errors.  For the sake
       of accuracy the Julian day (both TAI and UTC) are kept as two separate
       numbers: a half integer (integer.5) day and a day fraction. */
      
    /* Note: TAI Julian date is used here as a first guess at the UTC Julian
       date (required by the function PGS_TD_LeapSec(). */

    returnStatus = PGS_TD_LeapSec(jdTAI, &leapSecs, &lastChangeJD,
				  &nextChangeJD, leapStatus);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSTD_W_JD_OUT_OF_RANGE:
	returnStatus = PGSTD_E_NO_LEAP_SECS;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "input julian day out of range for tabulated"
			      " corrections - approximate value used",
			      FUNCTION_NAME);
	break;
      case PGSTD_W_DATA_FILE_MISSING:
	returnStatus = PGSTD_E_NO_LEAP_SECS;
	PGS_SMF_SetDynamicMsg(returnStatus,"unable to find or open leap second "
			      "correction file: leapsec.dat - approximate"
			      " value used",FUNCTION_NAME);
	break;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* convert TAI Julian date to UTC Julian date.   Basically the two are the
       same except that they differ by TAI-UTC (leapSecs), so the UTC Julian
       date is the TAI Julian date less the value of leapSecs (converted to
       days).  The leap second adjustment is accomplished by adjusting the day
       fraction.  Note that this may cause the UTC day fraction to become less
       than zero.  An appropriate correction is made so that:
       0.0 <= jdUTC[1]. */
    
    jdUTC[0] = jdTAI[0];
    jdUTC[1] = jdTAI[1] - leapSecs/SECONDSperDAY;

    if (jdUTC[1] < 0.0)
    {
	jdUTC[0] -= 1.0;
	jdUTC[1] += 1.0;
    }

    /* computer roundoff error may cause the above test to succeed and set
       jdUTC[1] to 1.0 which is undesirable, test for that possibility and fix
       it */

    if (jdUTC[1] >= 1.0)
    {
	jdUTC[0] += 1.0;
	jdUTC[1] -= 1.0;
    }

    /* If no leap seconds were found from the leap seconds table (leapsec.dat)
       just go with the approximate value returned by PGS_TD_LeapSec() skipping
       all the fancy footwork done below to get the actual leap second. This
       may be several seconds off.  This function will return an error (E) but
       may as well give an approximate value too in case that suits the purpose
       of the user. */

    if (returnStatus != PGSTD_E_NO_LEAP_SECS)
    {
	
	/* Since the value of leapSecs was obtained by using TAI time
	   to approximate UTC time, the new value of UTC Julian date needs to be
	   checked and perhaps corrected, see "DETAILS" section in "PROLOG"
	   above.*/

	if (jdUTC[0] < lastChangeJD)
	{
	    returnStatus = PGS_TD_LeapSec(jdUTC, &newLeapSecs, &newChangeJD,
					  &nextChangeJD, newLeapStatus);
	    if (returnStatus != PGS_S_SUCCESS)
	    {
		PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
		return PGS_E_TOOLKIT;
	    }
	
	    if ((leapDiff = (leapSecs - newLeapSecs)) > 0.0)
	    {
		jdUTC[1] += leapDiff/SECONDSperDAY;
		if (jdUTC[1] >= 1.0)
		{
		    jdUTC[1] -= 1.0/SECONDSperDAY;
		    onLeap = PGS_TRUE;
		}
		else
		{
		    leapSecs = newLeapSecs;
		    strcpy(leapStatus,newLeapStatus);
		}		
	    }
	    else
	    {
		jdUTC[1] += leapDiff/SECONDSperDAY;
		leapSecs = newLeapSecs;
		strcpy(leapStatus,newLeapStatus);
	    }
	}
    
	/* issue error if predicted value of TAI-UTC used */
 
	if (strcmp(leapStatus,"PREDICTED") == 0)
	{
            returnStatus = PGS_E_TOOLKIT;
            PGS_SMF_SetDynamicMsg(returnStatus,
                  "Obsolete leap seconds file format encountered ",
                                         FUNCTION_NAME);
	}
    }
    
    /* issue notice if leap second discarded */

    if (onLeap == PGS_TRUE)
    {
	returnStatus = PGSTD_M_LEAP_SEC_IGNORED;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }
    
    /* return to calling function */
    
    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    
    return returnStatus;
}
