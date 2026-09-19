/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_EOSPMGIIStoTAI.c

DESCRIPTION:
   This file contains the function PGS_TD_EOSPMGIIStoTAI().
   This function converts EOS PM (GIIS) spacecraft clock time in CCSDS day 
   segmented
   Time Code (CDS) (with implicit P-field) format to TAI (as real continuous 
   seconds since 12AM UTC 1-1-1993).
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.
   Abe Taaheri / SM&A Corp.

HISTORY:
   09-Aug-1994  GTSK  Initial version
   05-Jun-1995  GTSK  Removed code duplicated elsewhere to calculate TAI.  This
                      has been replaced with calls to the appropriate lower
		      level functions.
   01-Dec-1999   AT   Used PGS_TD_EOSAMtoTAI to convert EOS PM (GIIS format) 
                      to TAI time

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert EOS PM (GIIS) Clock Time to TAI Time


NAME:
   PGS_TD_EOSPMGIIStoTAI()
 

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_EOSPMGIIStoTAI(
       PGSt_scTime  scTime[8],
       PGSt_double  *secTAI93)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_eosamtotai(sctime,sectai93)
      character*8      sctime
      double precision sectai93

DESCRIPTION:
   This function converts EOS PM (GIIS) spacecraft clock time in CCSDS day 
   segmented
   Time Code (CDS) (with implicit P-field) format to TAI (as real continuous
   seconds since 12AM UTC 1-1-1993).
 
INPUTS:
   NAME      DESCRIPTION
   ----      -----------
   scTime    EOS PM (GIIS) clock time in CCSDS day segmented time code format 
             (CDS).  
             See NOTES below for detailed description as well as discussion of
	     min and max values.

OUTPUTS:
   NAME      DESCRIPTION                UNITS      MIN              MAX
   ----      -----------                -----      ---              ---
   secTAI93  real continuous seconds    seconds       ** see NOTES **
             since 12AM UTC 1-1-93

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_BAD_2ND_HDR_FLAG    bad value of secondary header ID flag
   PGSTD_E_MILSEC_TOO_BIG      millisecond field too large (>=86401000)
   PGSTD_E_MICSEC_TOO_BIG      microsecond field too large (>=1000)
   PGSTD_E_NO_LEAP_SECS        leap seconds correction unavailable at 
                               requested time
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
   N/A

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

   EOS PM (GIIS) TIME FORMAT:

     For EOS-PM (GIIS), the input spacecraft (s/c) clock time scTime is a 
     63-bit value
     in CCSDS Day Segmented (CDS) format (Almost.  See Note below.), which is
     comprised of three binary counter values:
                 
     ("scTime" represents an array of 8 (unsigned) one byte elements)
          
     1)  A 15-bit value (the first two elements of array
         scTime each with 8 bits excluding the first bit of element 1
         which is not used for time, see Note below) containing the
         number of days since an epoch of January 1, 1958.  The range of 
         decimal values is 0-32767, computed as 256*element1
         +element2. The maximum decimal values of elements 1
         and 2 are 127 and 255 respectively.
     2)  A 32-bit value (elements 3,4,5 and 6 of array
         scTime,each with 8 bits) containing the milliseconds
         of the day. The range of values is 0-86400999 
         milliseconds, computed as 256*256*256*element3+
         256*256*element4+256*element5+element6. The maximum
         decimal values of elements 3,4,5 and 6 are 5,38,95
         and 231 respectively.
     3)  A 16-bit value (elements 7 and 8 of array
         scTime, each with 8 bits) containing the number of 
         microseconds of milliseconds.  The range of values
         is 0-999 microseconds, computed as 256*element6
         +element7. The maximum values of elements 6 and 7
          are 3 and 231.
       
     Note: this function expects an input array of 8 unsigned characters
     (PGSt_scTime).  This is 64 bits.  The first bit in the first byte of the
     return value is actually the secondary header ID flag which is always set
     to zero by EOS PM (GIIS).  This makes the s/c time format used 
     by EOS PM (GIIS) a variation of the CDS format (not supported by CCSDS).

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   None

GLOBALS:
   None

FILES:
   This function calls PGS_TD_UTCjdtoTAIjd() which requires the file:
   leapsec.dat

FUNCTIONS_CALLED:
   PGS_TD_UTCjdtoTAIjd()
   PGS_TD_TAIjdtoTAI()
   PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* constants */

#define EPOCH_DATE          2436204.5  /* UTC Julian date 12AM Jan 1, 1958 */
#define MILLISECperDAY      86400000.0 /* milliseconds in one day  */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_EOSPMGIIStoTAI()"

PGSt_SMF_status
PGS_TD_EOSPMGIIStoTAI(                  /* converts EOS PM (GIIS) s/c time to TAI */
    PGSt_scTime     scTime[8],      /* EOS PM (GIIS) s/c time in CDC (almost) */
    PGSt_double     *secTAI93)      /* seconds since 12AM UTC 1-1-1993 */
{
    PGSt_uinteger   days;           /* days since epoch */
    PGSt_uinteger   milliSec;       /* millisecond of day */
    PGSt_uinteger   microSec;       /* microsecond of millisecond */

    PGSt_double     jdUTC[2];       /* UTC Julian date */
    PGSt_boolean    onLeap;         /* true if in leap second else false */

    PGSt_SMF_status returnStatus;   /* return value of this function */

    /* The first bit of the s/c time should be set to 0 for EOS PM (GIIS).  
       Technically
       this is not the first bit of the time field it is the secondary header
       ID flag. */

    if (scTime[0] & 128)
    {
	PGS_SMF_SetStaticMsg(PGSTD_E_BAD_2ND_HDR_FLAG, FUNCTION_NAME);
	return PGSTD_E_BAD_2ND_HDR_FLAG;
    }

    days = (scTime[0]*256U) + scTime[1];
    milliSec = ((scTime[2]*256U + scTime[3])*256U + scTime[4])*256U
             + scTime[5];
    microSec = (scTime[6]*256U) + scTime[7];
	
    /* Value of milliSec should be < (a day + 1 second).  Since this value
       represents milliseconds of the day in the day field, its resolution
       should not be as large as a day (plus a second in case of leap seconds).
       Similarly the value of microSec should not be equal to or greater than
       1000. (which is an entire millisec and should be accounted for in the
       milliseconds field). */

    if (milliSec >= 86401000)
    {
	PGS_SMF_SetStaticMsg(PGSTD_E_MILSEC_TOO_BIG,FUNCTION_NAME);
	return PGSTD_E_MILSEC_TOO_BIG;
    }
    if (microSec >= 1000)
    {
	PGS_SMF_SetStaticMsg(PGSTD_E_MICSEC_TOO_BIG,FUNCTION_NAME);
	return PGSTD_E_MICSEC_TOO_BIG;
    }

    /* Note if input time is occurring during a leap second.  The leap second
       would cause times to duplicate so it is removed for the calculation of 
       jdUTC, however if a leap second IS occurring, that is noted in the value
       of onLeap. */

    if(milliSec >= (PGSt_uinteger) MILLISECperDAY)
    {
	onLeap = PGS_TRUE;
	milliSec -= (PGSt_uinteger)1000.0;
    }
    else
      onLeap = PGS_FALSE;

    /* UTC Julian day is the s/c clock epoch day + whole number of days since
       then.  UTC Julian day fraction is the fraction of the UTC Julian day
       (which is actually half integral here) less the leap second if one is
       occurring (see adjustment of milliSec above).   */

    jdUTC[0] = EPOCH_DATE + days;
    jdUTC[1] = (milliSec + microSec/1000.)/MILLISECperDAY;

    /* Convert the UTC Julian date to the equivalent TAI Julian date (the TAI
       Julian date is the UTC Julian date + (TAI-UTC) ).  Note: the variable 
       jdUTC is the UTC Julian date on input and is overwritten with the TAI
       Julian date on output. */

    returnStatus = PGS_TD_UTCjdtoTAIjd(jdUTC,onLeap,jdUTC);

    /* jdUTC is actually the TAI Julian date at this point (see above) */

    *secTAI93 = PGS_TD_TAIjdtoTAI(jdUTC);
    
    return returnStatus;
}
