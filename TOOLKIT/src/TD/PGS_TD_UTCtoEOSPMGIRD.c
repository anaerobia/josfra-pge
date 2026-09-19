/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_UTCtoEOSPMGIRD.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCtoEOSPMGIRD().
   This function converts UTC in CCSDS ASCII Time Code A or CCSDS ASCII Time
   Code B  format to EOS PM (GIRD) spacecraft clock time in CCSDS Unsegmented Time Code
   (CUC) (with explicit P-field) format .
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.
   Abe Taaheri / SM&A Corp.

HISTORY:
   08-Jun-1994  GTSK  Initial version
   01-Dec-1999   AT   Used PGS_TD_UTCtoEOSPM to convert UTC time to 
                      GIRD formats of PM spacecraft
   29-Jun-2000   AT   Modified TAI time keeping only 6 significant digits
                      after the decimal point
   03-Jan-2001   AT   Moved back to previous version.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert UTC Time to EOS PM (GIRD) spacecraft clock time

NAME:
   PGS_TD_UTCtoEOSPMGIRD()

SYNOPSIS:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCtoEOSPMGIRD(
       char         asciiUTC[28],
       PGSt_scTime  scTime[8])

DESCRIPTION:
   This function converts UTC in CCSDS ASCII Time Code A or CCSDS ASCII Time
   Code B  format to EOS PM (GIRD) spacecraft clock time in CCSDS Unsegmented Time Code
   (CUC) (with explicit P-field) format .
 
INPUTS:
   NAME       DESCRIPTION          UNITS  MIN         MAX
   ----       -----------	   -----  ---         ---
   asciiUTC   UTC time in CCSDS	   ASCII  *see NOTES below
              ASCII Time Code A
	      or CCSDS ASCII Time
              Code B format
	    
OUTPUTS:    
   NAME       DESCRIPTION                           UNITS   MIN   MAX
   ----       -----------                           -----   ---   ---
   scTime     EOS PM (GIRD) s/c clock time in CCSDS        *see NOTES below
	      Unsegmented Time Code format (CUC).

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_DATE_OUT_OF_RANGE   input date is out of range of allowable
                               times for the EOS PM (GIRD) spacecraft clock
   PGSTD_E_NO_LEAP_SECS        leap seconds correction unavailable at 
	                       requested time
   PGSTD_E_TIME_FMT_ERROR      error in format of ascii UTC time
   PGSTD_E_TIME_VALUE_ERROR    error in value of the ascii UTC time
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

   EOS PM (GIRD) TIME FORMAT:

     For EOS-PM (GIRD), the output spacecraft (s/c) clock time scTime is a 64-bit value
     in CCSDS unsegmented (CUC) format, which is comprised of three binary
     counter values:
                 
     ("scTime" represents an array of 8 (unsigned) one byte elements)
          
     1)  A 16-bit value (the first two elements of array
         scTime each with 8 bits) containing the explicit 
         P-field.  The first element (octet) of the P-field 
         for EOS PM (GIRD) has a constant value of 10101110 (binary)
         which corresponds to a decimal value of 174.  The 
         second element of the P-field has a leading bit set
         to 0.  The other seven bits are a binary counter
         representing TAI-UTC with a maximum decimal value
         of 127 (1111111 binary).
     2)  A 32-bit value (the third through sixth elements of array
         scTime each with 8 bits) containing the number of
         seconds since an epoch of 12AM UTC January 1, 1958. 
         The range of decimal values is 0-4294967295, computed as 
         256*256*256*element3 + 256*256*element4 + 256*element5
         + element6. The maximum decimal value of each 
         element 1, 2, 3 and 4 is 255.
     3)  A 16-bit value (elements 7 and 8 of array
         scTime, each with 8 bits) containing the sub-seconds.
         The range of values is 0-65535 sub-seconds, computed as 
         256*element7 + element8. The maximum decimal value of
         each element 7 and 8 is 255.  The sub-seconds represent 
         the fraction of the last second.  This fraction is calculated
         as (sub-seconds)/(256^(# of sub-second elements)).
     
         This allows the s/c clock to represent times from 1958-01-01 to
         approximately 2094-02-06T06:26:25 (give or take a few seconds).

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

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   This algorithm checks the input UTC time to make sure it is in the correct
   format.  The input time is then converted to TAI time (as seconds since 12AM
   UTC 1-1-93).  If this conversion is successful the leap-second correction is
   between TAI and UTC is determined.  The time is then checked to make sure it
   falls within the allowable range of the EOS PM (GIRD) spacecraft clock.  If the
   above steps are all successful then the input time is decomposed into the
   appropriate octets of the EOS PM (GIRD) spacecraft clock time (see 1)-3) above).
   The explicit P-field and leap seconds count are also written to the
   appropriate fields.

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_UTCtoUTCjd()
   PGS_TD_UTCtoTAI()
   PGS_TD_LeapSec()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()
   PGS_SMF_SetUnknownMsg()
   PGS_SMF_SetDynamicMsg()

END_PROLOG:
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_TD.h>

#define EPOCH_DATE 1104537627.0 /* (julian date 12AM TAI 1-1-1993) less
				   (julian date 12AM UTC 1-1-1958) in TAI 
				   seconds */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoEOSPMGIRD()"

PGSt_SMF_status
PGS_TD_UTCtoEOSPMGIRD(              /* convert UTC time to EOS-PM (GIRD) s/c clock time */
    char            *asciiUTC,  /* UTC time (in ASCII) (input) */
    PGSt_scTime     scTime[8])  /* s/c clock time (in CUC) (output) */
{
    PGSt_uinteger   secondsSinceEpoch;    /* seconds since 12AM TAI 1-1-58 */
    PGSt_uinteger   subSecondsSinceEpoch; /* sub seconds of last second */

    PGSt_double     secTAI93; /* TAI time equivalent of input UTC time */
    PGSt_double     jdUTC[2]; /* UTC julian date of input time */
    PGSt_double     leapSecs; /* TAI-UTC at input time */
    PGSt_double     changeJD; /* last date of change of TAI-UTC at input time */
    
    char            leapStatus[10]; /* actual/predicted TAI-UTC */
    char mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* error mnemonic */
    char msg[PGS_SMF_MAX_MSG_SIZE];           /* error message */

    PGSt_SMF_status returnStatus;   /* return status of this function */
    PGSt_SMF_status returnStatus1;  /* return status of PGS function calls */

    /*******************
     * BEGIN EXECUTION *
     *******************/

    /* Call PGS_TD_UTCtoTAI to convert input UTC time to TAI time in
       seconds since 12AM 1-1-93.  If this call is successful call
       PGS_TD_UTCtoUTCjd to get the UTC julian day.  Call PGS_TD_LeapSec
       with date part of jdUTC to determine the value of TAI-UTC (leapSecs). 
       TAI-UTC is required in the second octet of the P-field of the EOS PM (GIRD)
       spacecraft clock. */

    returnStatus = PGS_TD_UTCtoTAI(asciiUTC,&secTAI93);
    switch (returnStatus)
    {
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_GetMsg(&returnStatus1,mnemonic,msg);
	if (returnStatus1 != returnStatus)
	  PGS_SMF_GetMsgByCode(returnStatus,msg);
      case PGS_S_SUCCESS:
	returnStatus1 = PGS_TD_UTCtoUTCjd(asciiUTC,jdUTC);
	if (returnStatus1 != PGS_S_SUCCESS &&
	    returnStatus1 != PGSTD_M_LEAP_SEC_IGNORED)
	{
	    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
	returnStatus1 = PGS_TD_LeapSec(jdUTC,&leapSecs,&changeJD,
				       &changeJD,leapStatus);
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGSTD_W_JD_OUT_OF_RANGE:
	  case PGSTD_W_DATA_FILE_MISSING:
	    if (returnStatus == PGSTD_E_NO_LEAP_SECS)
	      break;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
	if (returnStatus != PGS_S_SUCCESS)
	  PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* Check to ensure that the input time is in the range of the s/c clock.
       The seconds since epoch are equal to secTAI93 plus EPOCH_DATE.  The
       lowest value of the spacecraft clock is zero so secTAI93 should not
       be less than -EPOCH_DATE (which condition would yield an overall 
       negative time since epoch which cannot be represented by the s/c
       clock).  There are four octets of course time (seconds) allowed by
       the s/c clock which yields a max possible value of 2^32-1 so the
       integer seconds since epoch had better be less than 2^32. */
    
    if (secTAI93 < -EPOCH_DATE || (secTAI93+EPOCH_DATE) >= pow(2.0,32))
    {
	PGS_SMF_SetDynamicMsg(PGSTD_E_DATE_OUT_OF_RANGE,
			      "The input UTC time "
			      "string is out of range for EOS PM (GIRD) "
			      "(12-31-57 < date < 2-6-2094)",
			      FUNCTION_NAME);
	return PGSTD_E_DATE_OUT_OF_RANGE;
    }
    
    /* Determine the integer seconds since epoch by casting the total (real)
       seconds since epoch as an unsigned integer (this will truncate the
       real seconds (not round) which is the desired result).  The sub-seconds
       field is two octets so the sub-seconds are the fraction of 2^16 (65536)
       determined by multiplying the fraction part of the real second by
       2^16 and casting to an unsigned integer (the .5 is added to achieve
       the effect of rounding which is desired in this case). */
    
    secondsSinceEpoch = (PGSt_uinteger) (secTAI93+EPOCH_DATE);
    subSecondsSinceEpoch = (PGSt_uinteger) (((secTAI93+EPOCH_DATE) - 
                                             secondsSinceEpoch)*65536.0 + .5);

    /* if subSecondsSinceEpoch has been rounded to 65536, increase the seconds
       (secondsSinceEpoch) by one and set subSecondsSinceEpoch to zero */

    if (subSecondsSinceEpoch >= 65536U)
    {
	secondsSinceEpoch += 1U;
	subSecondsSinceEpoch = 0U;
    }
    
    /* The first octet of the P-field is the constant value 174 (10101110
       binary).  The second octet is the value of TAI-UTC (which must be less
       than 128 (10000000 binary) since the first bit of this octet must be
       zero.  That the value of TAI-UTC is less than 128 is assumed here
       since it is anticipated that TAI-UTC will not reach this value until
       long after the useful life of the s/c. */
       
    scTime[0] = 174U;
    scTime[1] = (PGSt_scTime) leapSecs;
    
    /* Decompose the times to the s/c clock octets.  See NOTES above */
    
    scTime[2] = (PGSt_scTime) (secondsSinceEpoch/16777216U);
    scTime[3] = (PGSt_scTime) ((secondsSinceEpoch/65536U)%256U);
    scTime[4] = (PGSt_scTime) ((secondsSinceEpoch/256U)%256U);
    scTime[5] = (PGSt_scTime) (secondsSinceEpoch%256U);
    scTime[6] = (PGSt_scTime) (subSecondsSinceEpoch/256U);
    scTime[7] = (PGSt_scTime) (subSecondsSinceEpoch%256U);
    
    return returnStatus;
}
