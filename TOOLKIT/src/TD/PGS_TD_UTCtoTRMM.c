/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_UTCtoTRMM.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCtoTRMM().
   This function converts UTC in CCSDS ASCII time code A (or B) format
   to TRMM spacecraft clock time in CCSDS Unsegmented Time Code (CUC) (with
   implicit P-field) format .
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
   09-Jun-1994  GTSK  Initial version
   10-Jun-1994  GTSK  Added WARNING message to NOTES section (below)
	              explaining why this algorithm doesn't actually
		      work with the real TRMM time.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert UTC Time to TRMM Clock Time

NAME:
   PGS_TD_UTCtoTRMM()
 
SYNOPSIS:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCtoTRMM(
      char         asciiUTC[28],
      PGSt_scTime  scTime[8])

DESCRIPTION:
   This function converts UTC in CCSDS ASCII time code A (or B) format to TRMM
   spacecraft (s/c) clock time in CCSDS Unsegmented Time Code (CUC) (with
   implicit P-field) format.
 
INPUTS:
   NAME         DESCRIPTION                UNITS     MIN         MAX
   ----         -----------                -----     ---         ---
   asciiUTC     UTC time in CCSDS ASCII     N/A      1993-001    ** see NOTES **
                Time Code (A or B format)

OUTPUTS:
   NAME         DESCRIPTION                UNITS     MIN         MAX
   ----         -----------                -----     ---         ---
   scTime       TRMM clock time in CCSDS     ** see NOTES below **
	        unsegmented time code 
		format (CUC).

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_NO_LEAP_SECS        leap seconds correction unavailable at 
                               requested time
   PGSTD_E_TIME_FMT_ERROR      error in format of ascii UTC time
   PGSTD_E_TIME_VALUE_ERROR    error in value of the ascii UTC time
   PGSTD_E_DATE_OUT_OF_RANGE   input UTC time is not within the possible values
                               of the s/c clock
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
   N/A

NOTES:
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

   TRMM TIME FORMAT:

     For TRMM, the output spacecraft clock time scTime is a 64-bit value in
     CCSDS unsegmented time code (CUC) format (Almost.  See Note below.) with an
     epoch defined (dynamically!) by TRMM (see Note below), which is comprised
     of two binary counter values:

     ("scTime" represents an array of 8 (unsigned) one byte elements)

     1)  A 32-bit value (the first four elements of array
         scTime each with 8 bits) containing the number of
         seconds since an epoch of 12AM UTC January 1, 1993. 
         The range of decimal values is 0-4294967295, computed as 
         256*256*256*element1 + 256*256*element2 + 256*element3 + element4. 
         The maximum decimal value of each element 1, 2, 3 and 4 is 255.
     2)  A 32-bit value (elements 5, 6, 7 and 8 of array scTime, each with 
         8 bits) containing the sub-seconds.
         The range of values is 0-4294967295 sub-seconds, computed as
         256*256*256*element3 + 256*256*element4 + 256*element5 + element6. 
         The maximum decimal value of each element 3,4,5 and 6 is 255.  The
         sub-seconds represent the fraction of the last second.  This fraction 
         is calculated as (sub-seconds)/(256^(# of sub-second elements)).

         This allows the s/c clock to represent times from 1993-01-01 to
         approximately 2129-02-07T06:26:24 (give or take a few seconds).

     Note: the CCSDS Blue Book on Time Code Formats explicitly states for CUC:
     "This time code IS NOT UTC-based and leap second corrections don no apply."
     The TRMM clock is actually a count of the number of seconds since the time
     card (hardware) was actually powered up (but only ostensibly).  This clock
     can and may indeed actually be "jammed" (reset at arbitrary times) from
     ground control.  TRMM keeps track of (separately of course) a Universal
     Time Correlation Factor (UTCF) which is an arbitrary number representing
     real seconds that must be added to the TRMM s/c clock in order to derive
     the time.  This number accounts for several things including time offset
     correction between power-up and the epoch time, drift in the s/c clock that
     has rendered it inaccurate (accuracy requirement for TRMM s/c clock is
     nearest millisecond) AND leap-second correction which means really that
     TRMM s/c clock is rendered directly into UTC (not TAI).  The formula to
     find UTC from TRMM s/c clock time is something like:

     daysSinceEpoch = (int) (scClockTime + UTCF)/86400;
     secondsOfDay = (double) (scClockTime + UTCF) - 86400*daysSinceEpoch;

     Then decipher daysSinceEpoch into the date and secondsOfDay into the time.
     That is, the above formula yields the UTC date and time.  This method of
     calculating the time will result in duplicate times around a leap second.

     The time as used by TRMM, therefore, basically IS a UTC based time.  This
     makes the s/c time format used by TRMM a variation of the CUC format (not
     supported by CCSDS).

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac
     TRMM-490-137, "Tropical Rainfall Measuring Mission (TRMM) Telemetry And
     Command Handbook", 1994-02-21, Goddard Space Flight Center, Appendix D

REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   The input time is passed to PGS_TD_UTCtoTAI() which converts
   the ASCII UTC time to real continuous seconds since 12AM UTC 1-1-93.
   The time is then checked to make sure it falls within the allowable
   range of the TRMM spacecraft clock. These seconds are then 
   separated into integer and fractional components and decomposed into
   the appropriate octets of the TRMM spacecraft clock time (see NOTES above).

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_UTCtoTAI()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_TD.h>

/* constants */

#define TRMM_EPOCH_DATE 2448988.50 /* UTC Julian date 12AM 1-1-1993 */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoTRMM()"

PGSt_SMF_status
PGS_TD_UTCtoTRMM(           /* convert UTC time to TRMM s/c clock time */
    char         *asciiUTC, /* UTC time (in ASCII) (input) */
    PGSt_scTime  scTime[8]) /* s/c clock time (in CUC) (output) */
{
    PGSt_uinteger  secondsSinceEpoch;         /* seconds since 12AM 1-1-58 */
    PGSt_uinteger  subSecondsSinceEpoch;      /* sub seconds of last second */
    PGSt_double    secTAI93;                  /* TAI time equivalent of 
						 input UTC time */
    PGSt_double   jdUTC[2];                   /* UTC Julian date equivalent of
						 UTCF */
    PGSt_double   jdTAI[2];                   /* TAI Julian date equivalent of
						 UTCF */
    PGSt_double   utcf;                       /* UTCF (Universal Time Correction
						 Factor), this is the time
						 offset added by TRMM to get
						 seconds since 12 AM 1-1-1993
						 from hardware time which is
						 what the spacecraft clock
						 actually is.  Hardware time is
						 the seconds since s/c (time
						 card) powerup. */

    PGSt_SMF_status   returnStatus;           /* return status of PGS function
						 calls */
    
    /*******************
     * BEGIN EXECUTION *
     *******************/

    /* initialize returnStatus to indicate success */

    returnStatus = PGS_S_SUCCESS;
    
    /* get the TRMM UTCF and convert it to an equivalent TAI time */

    returnStatus = PGS_TD_ManageUTCF(PGSd_GET, &utcf);
    if (returnStatus != PGS_S_SUCCESS)
    {
	return returnStatus;
    }
    
    jdUTC[0] = TRMM_EPOCH_DATE + (int)utcf/86400;
    jdUTC[1] = fmod(utcf, 86400.0)/86400.0;
    
    returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC, PGS_FALSE, jdTAI);
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
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* Call PGS_TD_UTCtoTAI to convert input UTC time to TAI time in
       seconds since 12AM UTC 1-1-93. */

    returnStatus = PGS_TD_UTCtoTAI(asciiUTC,&secTAI93);
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
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* decrement the input TAI time by the TAI value of the UTCF */

    secTAI93 -= PGS_TD_TAIjdtoTAI(jdTAI);
    
    /* Check to ensure that the input time is in the range of the s/c clock.
       The lowest value of the spacecraft clock is zero so secTAI93 should not
       be less than 0.0 (which condition would yield an overall 
       negative time since epoch which cannot be represented by the s/c
       clock).  There are four octets of course time (seconds) allowed by
       the s/c clock which yields a max possible value of 2^32-1 so the integer
       seconds since epoch had better be less than 2^32 (4294967296.0). */
    
    if (secTAI93 < 0.0 || secTAI93 >= 4294967296.0)
    {
	PGS_SMF_SetDynamicMsg(PGSTD_E_DATE_OUT_OF_RANGE,
			      "The date portion of the input UTC time "
			      "string is out of range for TRMM",
			      FUNCTION_NAME);
	return PGSTD_E_DATE_OUT_OF_RANGE;
    }

    /* Determine the integer seconds since epoch by casting the total (real)
       seconds since epoch as an unsigned integer (this will truncate the
       real seconds (not round) which is the desired result).  The sub-seconds
       field is four octets so the sub-seconds are the fraction of 2^32
       (4294967296) determined by multiplying the fraction part of the real
       second by 2^32 and casting to an unsigned integer (the .5 is added to
       achieve the effect of rounding which is desired in this case). */
    
    secondsSinceEpoch = (PGSt_uinteger) secTAI93;
    subSecondsSinceEpoch = (PGSt_uinteger) ((secTAI93 - secondsSinceEpoch)*
					    4294967296. + .5);

    /* Decompose the times to the s/c clock octets.  See NOTES above */

    scTime[0] = (PGSt_scTime) (secondsSinceEpoch/16777216U);
    scTime[1] = (PGSt_scTime) ((secondsSinceEpoch/65536U)%256U);
    scTime[2] = (PGSt_scTime) ((secondsSinceEpoch/256U)%256U);
    scTime[3] = (PGSt_scTime) (secondsSinceEpoch%256U);

    scTime[4] = (PGSt_scTime) (subSecondsSinceEpoch/16777216U);
    scTime[5] = (PGSt_scTime) ((subSecondsSinceEpoch/65536U)%256U);
    scTime[6] = (PGSt_scTime) ((subSecondsSinceEpoch/256U)%256U);
    scTime[7] = (PGSt_scTime) (subSecondsSinceEpoch%256U);
   
    return returnStatus; /* see WARNING in NOTES section above */
}

/* notes:

   critical before use:
   see WARNING in NOTES section above */
