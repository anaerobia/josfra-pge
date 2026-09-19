/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_TRMMtoTAI.c

DESCRIPTION:
   This file contains the function PGS_TD_TRMMtoTAI().
   This function converts TRMM spacecraft clock time in CCSDS Unsegmented Time
   Code (CUC) (with implicit P-field) format to TAI (Toolkit internal time).
 
AUTHOR:
   Guru Tej S. Khalsa /	Applied Research Corp.

HISTORY:
   09-Aug-1994  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert TRMM Clock Time to TAI Time

NAME:
   PGS_TD_TRMMtoTAI()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_TRMMtoTAI(
       PGSt_scTime  scTime[8],
       PGSt_double  *secTAI93)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_trmmtotai(sctime,sectai93)
      character*8      sctime
      double precision sectai93

DESCRIPTION:
   This function converts TRMM spacecraft clock time in CCSDS Unsegmented Time
   Code (CUC) (with implicit P-field) format to TAI (Toolkit internal time).

INPUTS:
   NAME      DESCRIPTION
   ----      -----------
   scTime    TRMM clock time in CCSDS unsegmented time code format (CUC).
	     Almost. See NOTES below for detailed description as well as
	     discussion of min and max values.

OUTPUTS:	  
   NAME      DESCRIPTION                       UNITS    MIN   MAX
   ----      -----------		       -----    ---   ---
   secTAI93  real continuous seconds since     seconds  0.0   4294967296.999999
             12AM UTC 1-1-93

RETURNS:
   PGS_S_SUCCESS               successful execution

EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:

     TAI is:  International Atomic Time

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

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac
     TRMM-490-137, "Tropical Rainfall Measuring Mission (TRMM) Telemetry And
     Command Handbook", 1994-02-21, Goddard Space Flight Center, Appendix D

REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   The algorithm computes the real continuous seconds since 12AM UTC 1-1-1993 
   from the input time.  To do this a different method of calculating the time
   is used than that recommended by TRMM.  First the UTCF value is retrieved.
   This value is then converted--as is (i.e. without adding the MET)--to a UTC
   time.  This UTC time is then converted to TAI (seconds since 1-1-1993).  The
   s/c clock time (MET) is assumed to be a count of SI seconds since the epoch
   of the TAI time determined from the UTCF.  The MET values (as real seconds)
   is added to the UTCF (as TAI) value.  This final value is considered to be
   the actual TAI time.

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_TD.h>

/* constants */

#define TRMM_EPOCH_DATE 2448988.50 /* UTC Julian date 12AM 1-1-1993 */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_TRMMtoTAI()"

PGSt_SMF_status
PGS_TD_TRMMtoTAI(               /* converts TRMM s/c time to TAI */
    PGSt_scTime   scTime[8],    /* TRMM s/c time in CCSDS unseg. time code */
    PGSt_double   *secTAI93)    /* seconds since 12AM UTC 1-1-1993 */
{
    PGSt_uinteger seconds;      /* decimal number of s/c clock seconds */
    PGSt_uinteger subSeconds;   /* decimal number of s/c clock sub-seconds */

    PGSt_double   jdUTC[2];     /* UTC Julian date equivalent of UTCF */
    PGSt_double   jdTAI[2];     /* TAI Julian date equivalent of UTCF */

    PGSt_double   utcf;         /* UTCF (Universal Time Correction Factor), this
				   is the time offset added by TRMM to get
				   seconds since 12 AM 1-1-1993 from hardware
				   time which is what the spacecraft clock
				   actually is.  Hardware time is the seconds
				   since s/c (time card) powerup. */

    PGSt_SMF_status returnStatus; /* return value of this function */
    
    seconds = ((scTime[0]*256U + scTime[1])*256U + scTime[2])*256U + 
              scTime[3];
    subSeconds = ((scTime[4]*256U + scTime[5])*256U + scTime[6])*256U + 
	         scTime[7];

    returnStatus = PGS_TD_ManageUTCF(PGSd_GET, &utcf);
    if (returnStatus != PGS_S_SUCCESS)
    {
	return returnStatus;
    }
    
    jdUTC[0] = TRMM_EPOCH_DATE + (int)utcf/86400;
    jdUTC[1] = fmod(utcf, 86400.0)/86400.0;
    
    returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC, PGS_FALSE, jdTAI);
    *secTAI93 = PGS_TD_TAIjdtoTAI(jdTAI);
#ifdef IRIX
    /* IRIX 6.2 likes to convert large unsigned integers to negative doubles,
       therefore beat it into submission (note that we are recycling the jdUTC
       variable here, none of this has anything to do with UTC Julian Date). */

    jdUTC[0] = seconds;
    jdUTC[1] = subSeconds;
    if (jdUTC[0] < 0.0)
    {
	jdUTC[0] = jdUTC[0] + 4294967296.0;
    }
    if (jdUTC[1] < 0.0)
    {
	jdUTC[1] = jdUTC[1] + 4294967296.0;
    }
    
    jdUTC[1] = jdUTC[1]/4294967296.0;

    *secTAI93 += jdUTC[0] + jdUTC[1];
#else
    *secTAI93 += (PGSt_double) seconds + ((PGSt_double)subSeconds)/4294967296.0;
#endif

    return returnStatus;          /* see WARNING in NOTES section above */
}

/* notes:   

   critical before use:
   see WARNING in NOTES section above */
