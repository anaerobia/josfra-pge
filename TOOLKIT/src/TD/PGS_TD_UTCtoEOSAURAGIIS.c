/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_UTCtoEOSAURAGIIS.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCtoEOSAURAGIIS().
   This function converts UTC in CCSDS ASCII time code A (or B) format
   to EOS AURA (GIIS) spacecraft (s/c) clock time in CCSDS day segmented Time
   Code (CDS) (with implicit P-field) format.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.
   Abe Taaheri / SM&A Corp.

HISTORY:
   08-Jun-1994  GTSK  Initial version
   27-Apr-1995  GTSK  Replaced "PGSt_uinteger" with "unsigned int" in an attempt
                      to guarantee that sscanf will always behave properly
  01-Dec-1999   AT    Used PGS_TD_UTCtoEOSAM to convert UTC time to 
                      GIIS formats of PM spacecraft

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert UTC Time to EOS AURA (GIIS) Clock Time

NAME:
   PGS_TD_UTCtoEOSAURAGIIS()

SYNOPSIS:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCtoEOSAURAGIIS(
      char         asciiUTC[28],
      PGSt_scTime  scTime[8])

DESCRIPTION:
   This function converts UTC in CCSDS ASCII time code A (or B) format
   to EOS AURA (GIIS) spacecraft (s/c) clock time in CCSDS Day Segmented (CDS) Time
   Code (with implicit P-field) format.

INPUTS:
   NAME       DESCRIPTION         UNITS   MIN         MAX
   ----       -----------         -----   ---         ---
   asciiUTC   UTC time in CCSDS   ASCII   1958-01-01  2047-09-17T23:59:59.999999
              ASCII Time Code A
	      or CCSDS ASCII
	      Time Code B format
   	    
OUTPUTS:    
   NAME       DESCRIPTION                         UNITS   MIN   MAX
   ----       -----------                         -----   ---   ---
   scTime     EOS AURA (GIIS) clock time in CCSDS day      *see NOTES below
              segmented time code format (CDS).  

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGS_E_DATE_OUT_OF_RANGE     input date is out of range of allowable
	                       times for the EOS AURA (GIIS) spacecraft clock
   PGSTD_E_TIME_FMT_ERROR      error in format of ascii UTC time
   PGSTD_E_TIME_VALUE_ERROR    error in value of the ascii UTC time
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
   N/A

NOTES:
   For EOS-AURA (GIIS), the output spacecraft (s/c) clock time scTime is a 63-bit value
   in CCSDS Day Segmented (CDS) format (Almost.  See Note below), which is
   comprised of three binary counter values:
                 
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
       
       This allows the s/c clock to represent times from 1958-01-01 to
       2047-09-17T23:59:59.999999.
 
   Note that this function outputs an array of 8 unsigned characters
   (PGSt_scTime).  This is 64 bits.  The first bit in the first byte of the
   return value is actually the secondary header ID flag which is always set
   to zero by EOS AURA (GIIS).  This makes the s/c time format used by EOS AURA (GIIS) a variation
   of the CDS format (not supported by CCSDS).

   The ASCII time input is in CCSDS ASCII Time Code A or CCSDS ASCII Time
   Code B format (see CCSDS 301.0-B-2 for details). 
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
   
REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   This algorithm checks the input UTC time to make sure it
   is in the correct format.  The input time string is then parsed
   into decimal values of its various fields (any missing fields after
   the day field are assumed to have a value of zero in accordance 
   with the PGS toolkit standard).  The year, month and day are then
   converted to the corresponding UTC julian day.  The UTC julian day
   of the of the epoch date (1-1-58) is then subtracted from the UTC
   julian day of the input time to determine the whole number of
   days since the epoch date (better be a whole number!).  The date
   is then checked to make sure it is in the allowable range of the
   EOS AURA (GIIS) s/c clock time.  The input time values are then converted
   to milliseconds of the day and microseconds of the millisecond.
   Finally if the above steps are all successful then the input
   time is decomposed into the appropriate octets of the EOS AURA (GIIS)
   spacecraft clock time (see NOTES above).
  
GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_julday()
   PGS_TD_timeCheck()
   PGS_TD_ASCIItime_BtoA()
   PGS_SMF_SetUnknownMsg()
   PGS_SMF_SetDynamicMsg()

END_PROLOG:
*******************************************************************************/

#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <PGS_TD.h>

/* constant */

#define EPOCH_JULIAN_DATE  2436205U   /* UTC julian date of the sc epoch
					  (1-1-58) */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoEOSAURAGIIS()"

PGSt_SMF_status
PGS_TD_UTCtoEOSAURAGIIS(           /* convert UTC time to EOS-AURA (GIIS) s/c clock time */
    char            *asciiUTC,   /* UTC time (in ASCII) (input) */
    unsigned char   scTime[8])   /* s/c clock time (in CDS) (output) */
{
    unsigned int    scJulianDate;    /* UTC julian date of the input time */
    unsigned int    daysSinceEpoch;  /* whole days since 1/1/58 */
    unsigned int    milliSecsOfDay;  /* milliseconds of the day (i.e. time) */
    unsigned int    year;            /* input year */
    unsigned int    month;           /* input month */
    unsigned int    day;             /* input day (of the month) */
    unsigned int    hour=0;          /* input hour */
    unsigned int    minute=0;        /* input minute */
    unsigned int    second=0;        /* input second */
    unsigned int    milliSecond=0;   /* input number of milliseconds */
    unsigned int    microSecond=0;   /* input number of microseconds of above */

    size_t          length;          /* length of input time string */

    char            *zPtr;           /* terminal 'Z' in input time */
    char            asciiUTC_A[28];  /* UTC in CCSDS ASCII time code A format */

    char            msg[PGS_SMF_MAX_MSG_SIZE]; /* temporary string
						  holder */

    PGSt_SMF_status returnStatus;    /* return status of PGS function calls */
    
    /*******************
     * BEGIN EXECUTION *
     *******************/

    /* Initialize returnStatus to indicate success, check input time for
       correct value and format (if input time is in CCSDS ASCII Time Code B
       format convert to Time Code A format). */

    returnStatus = PGS_S_SUCCESS;
    
    returnStatus = PGS_TD_timeCheck(asciiUTC);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	strcpy(asciiUTC_A,asciiUTC);
	break;
      case PGSTD_M_ASCII_TIME_FMT_B:
	returnStatus = PGS_TD_ASCIItime_BtoA(asciiUTC,asciiUTC_A);
	if (returnStatus != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
	break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* Fill out the fractional seconds field to 6 characters if one exists at
       all.  The PGS_TD_timeCheck will ensure that the fields of the time string
       are all correctly represented.  However each decimal place of the
       fractional second is considered an individual field, but this routine
       expects the fractional second part of the time string to consist of
       two 3-digit fields.  If no fractional seconds are passed in this is
       not a concern but if some part (but not all six allowed places) is
       specified in the input time string there may be a problem.  For example
       if the fractional part of the seconds is ".23" this will be read as
       "23" milliseconds, when what is really desired is "230" milliseconds.
       Therefore if the length of the time string indicates that at least one
       decimal place was specified (length > 20) but not all (length < 26)
       then trailing 0's are appended to the time string.  Note that asciiUTC_A
       is a copy of the input time so the input time string is not actually
       altered.  Also note:  The following code to parse the time string
       requires that the time is in CCSDS ASCII Time Code A format. */

    if ((zPtr=strchr(asciiUTC_A,'Z')) != NULL)
      *zPtr = '\0';
    length = strlen(asciiUTC_A);
    if (length > 20 && length < 26)
      strncat(asciiUTC_A,"00000",(26-length));

    /* Parse the input time string.  The year, month, day, hour, minute and
       whole second fields are each read directly into a variable representing
       their respective decimal values.  The fractional seconds fields (defined
       as one digit each) are parsed as follows:  the first three fields 
       representing the whole number of milliseconds of the second are read
       into one variable and the whole number of microseconds of the millisecond
       are read into another variable. */
       
    sscanf(asciiUTC_A,"%4u-%2u-%2uT%2u:%2u:%2u.%3u%3u",&year,&month,&day,&hour,
	   &minute,&second,&milliSecond,&microSecond);
    
    /* Call PGS_TD_julday() to determine the UTC Julian date of the input time
       which is then used with the UTC julian date of the epoch date to
       determine the whole number of days since the epoch date (daysSinceEpoch).
       The input time is then checked to ensure that it is within the allowable
       range of the EOS AURA (GIIS) s/c clock time.  Namely the input time should not
       be less than the epoch date since there is no way to represent a 
       negative time relative to the epoch date.  Also the s/c clock allows
       15 bits for the days since epoch so this number should not exceed
       2^15 - 1 (32767). */

    scJulianDate = PGS_TD_julday(year, month, day);
    daysSinceEpoch = scJulianDate - EPOCH_JULIAN_DATE;
    if (scJulianDate < EPOCH_JULIAN_DATE || daysSinceEpoch > 32767U)
    {
	sprintf(msg,"%s%s%s","The date portion of the input UTC time ",
		             "string is out of range for EOS AURA (GIIS) ",
	                     "(12-31-1957 < date < 9-18-2047)");
	PGS_SMF_SetDynamicMsg(PGSTD_E_DATE_OUT_OF_RANGE,msg,FUNCTION_NAME);
	return PGSTD_E_DATE_OUT_OF_RANGE;
    }
    
    /* calculate the total milliseconds of the day from the input
       hour, minute, second and millisecond */

    milliSecsOfDay = 3600000U*hour + 60000U*minute + 1000U*second
                     + milliSecond;

    /* Decompose the times to the s/c clock octets.  See NOTES above */
    
    scTime[0] = (PGSt_scTime) (daysSinceEpoch/256U);
    scTime[1] = (PGSt_scTime) (daysSinceEpoch%256U);
    	       
    scTime[2] = (PGSt_scTime) (milliSecsOfDay/16777216U);
    scTime[3] = (PGSt_scTime) ((milliSecsOfDay/65536U)%256U);
    scTime[4] = (PGSt_scTime) ((milliSecsOfDay/256U)%256U);
    scTime[5] = (PGSt_scTime) (milliSecsOfDay%256U);
	       
    scTime[6] = (PGSt_scTime) (microSecond/256U);
    scTime[7] = (PGSt_scTime) (microSecond%256U);
    
    return PGS_S_SUCCESS;
}
