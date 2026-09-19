/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_FGDCtoUTC.c

DESCRIPTION:
  This file contains the function PGS_TD_FGDCtoUTC()
   This function converts an FGDC ASCII date string and time string to CCSDS
   ASCII Time Code (format A).

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  29-Feb-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert FGDC ASCII Time and Date to CCSDS ASCII Time Code (Format A)

NAME:
   PGS_TD_FGDCtoUTC()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_FGDCtoUTC(
       char date[9],
       char time[18],
       char asciiUTC[28])

FORTRAN:
   include 'PGS_SMF.f'
   include 'PGS_TD_3.f'
    
   integer function pgs_td_fgdctoutc(date, time, asciiutc)
   character*8  date
   character*17 time
   character*27 asciiutc

      
DESCRIPTION:
   This function converts an FGDC ASCII date string and time string to CCSDS
   ASCII Time Code (format A).  The input FGDC time string may be in "Universal
   Time" or "local time" format.

INPUTS:
   Name         Description                   Units       Min   Max
   ----         -----------                   -----       ---   ---
   date         FGDC date string                 ** see NOTES **

   time         FGDC time string in              ** see NOTES **
                "Universal Time" format or
		"local time" format

OUTPUTS:
   Name         Description                   Units       Min   Max
   ----         -----------                   -----       ---   ---
   asciiUTC     UTC Time in CCSDS ASCII          ** see NOTES **
                Time Code (format A)

RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_E_TIME_VALUE_ERROR    error in input time value
   PGSTD_E_TIME_FMT_ERROR      error in input time format
   PGS_E_TOOLKIT               something unexpected happened, execution
                               of function terminated prematurely

EXAMPLES:
C:
   PGSt_SMF_status returnStatus;
   char            date[9];
   char            time[18];
   char            asciiUTC[28];

   strcpy(date,"19980630");
   strcpy(time,"055128320000-0500");  ** FGDC local time format **

   returnStatus = PGS_TD_FGDCtoUTC(date, time, asciiUTC);
   if (returnStatus != PGS_S_SUCCESS)
   {
   ** test errors, take appropriate action **
                    :
                    :
   }

   ** the variable "asciiUTC" is now: "1998-06-30T10:51:28.320000Z" **

   printf("%s\n", asciiUTC);

   strcpy(time,"105128320000Z");  ** FGDC UTC format **

   returnStatus = PGS_TD_FGDCtoUTC(date, time, asciiUTC);
   if (returnStatus != PGS_S_SUCCESS)
   {
   ** test errors, take appropriate action **
                    :
                    :
   }

   ** the variable "asciiUTC" is now: "1998-06-30T10:51:28.320000Z" **

   printf("%s\n", asciiUTC);

FORTRAN:
      implicit none

      include  'PGS_SMF.f'
      include  'PGS_TD_3.f'

      integer      pgs_td_fgdctoutc
      integer      returnstatus
      character*8  date
      character*17 time
      character*27 asciiutc_a
      
      date = '19980630'
      time = '055128320000-0500'  ! FGDC local time format

      returnstatus = pgs_td_fgdctoutc(date, time, asciiutc)
      if (returnstatus .ne. pgs_s_success) goto 999

!     the variable "asciiutc" is now: '1998-06-30T10:51:28.320000Z'

      write(6,*) asciiutc

      time = '105128320000Z'  ! FGDC UTC format

      returnstatus = pgs_td_fgdctoutc(date, time, asciiutc)
      if (returnstatus .ne. pgs_s_success) goto 999

!     the variable "asciiutc" is now: '1998-06-30T10:51:28.320000Z'

      write(6,*) asciiutc


NOTES:
   TIME ACRONYMS:
     UTC is:  Coordinated Universal Time
     
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


   FGDC TIME FORMATS:

     FGDC is:  Federal Geographic Data Committee.

     Date:

       The FGDC ASCII date format is:

           YYYYMMDD

           where: 
  	       YYYY is the four digit year specification
	       MM is the two digit month specification
	       DD is the two digit day specification

     Time:

       The FGDC time string may be in either "local time" format or 
       "Universal Time"  format (the toolkit will assume that this
       "Universal Time" is UTC).

       The general FGDC ASCII time format is:
 
            
            HHMMSSSSSSSS

	    where:
	        HH is the two digit hour specification
		MM is the two digit minute specification
		SSSSSSS is the multi-digit second and decimal fraction of second
		        specification, the first two digits represent the whole
			number of seconds any additional digits may be used to
			express the decimal fraction of the nearest second to
			the desired precision

       This sequence of characters MUST be followed by one of either:

       The letter 'Z', denoting that the string represents Universal Time (UTC).

       The time differential factor:

           shhmm

       The time differential factor expresses the difference in hours and
       minutes between local time and UTC.  It is represented by a four-digit
       number preceded by a plus sign (+) or minus sign (-), indicating the
       hours and minutes the local time is ahead of or behind UTC, respectively.

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Content Standards for Digital Geospatial Metadata (Federal Geographic Data
     Committee)

REQUIREMENTS:
   PGSTK - ????, ????

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_timeCheck()
   PGS_TD_UTCtoUTCjd()
   PGS_TD_UTCjdtoUTC()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <PGS_math.h>
#include <PGS_TD.h>

/* constants */

#define   SECONDSperDAY  86400.0  /* number of seconds in a day */
#define   MINUTESperDAY  1440.0   /* number of minutes in a day */
#define   HOURSperDAY    24.0     /* number of hours in a day */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_FGDCtoUTC()"

PGSt_SMF_status
PGS_TD_FGDCtoUTC(         /* convert date/time from FGDC to CCSDS format */
    char     date[],      /* date in FGDC ASCII calendar date format */
    char     time[],      /* time in FGDC ASCII format */
    char     asciiUTC[])  /* date (incl. time) in CCSDS ASCII format A */
{
    size_t   length;      /* string length */
    
    PGSt_integer      tdf_hours;   /* time differential factor - hour portion */
    PGSt_integer      tdf_minutes; /* time differential factor - minute portion */
    
    PGSt_double   tdf_total;   /* time differential factor as day fraction */
    PGSt_double   fraction;    /* fraction of second part of input time */
    PGSt_double   jdUTC[2];    /* UTC Julian date */
    
    PGSt_boolean onLeap;  /* T if on leap second otherwise F */
    
    PGSt_SMF_status returnStatus; /* return value of function calls */
    
    /* The format of the input date string is: YYYYMMDD.  All 8 characters are
       required to be present. */

    length = strlen(date);
    
    /* The date string should consist only of digits. */

    if (length == 8U)
    {
	length = strspn(date, "0123456789");
    }

    if (length != 8U)
    {
	PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_FMT_ERROR,
			      "error in FGDC date string format "
			      "(generic format: YYYYMMDD)",
			      FUNCTION_NAME);
	
	return PGSTD_E_TIME_FMT_ERROR;
    }

    /* Rewrite the date string into the date portion of the CCSDS ASCII Time
       string, inserting the appropriate field separators. */

    sprintf(asciiUTC, "%.4s-%.2s-%.2sT", date, date+4, date+6);

    /* The general format of the FGDC time string is either:
       HHMMSSSSSSshhmm or HHMMSSSSSSSSZ.
       Determine the length of the portion of the string up to the first
       non-numeric character. */

    length = strspn(time, "0123456789");
    
    /* The following lengths are invalid since the time string must fields must
       be complete and they are two characters each (of course the fractional
       second field may vary in size (0+ characters)). */

    if (length == 1U || length == 3U || length == 5U)
    {
	PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_FMT_ERROR,
			      "error in FGDC time string format "
			      "(generic format: HHMMSSSSSSSS",
			      FUNCTION_NAME);

	return PGSTD_E_TIME_FMT_ERROR;
    }

    switch (length)
    {
      default:

	/* This is the default case, i.e. the initial numeric field in the FGDC
	   time string is greater than 6 characters long, this means that it
	   contains some fractional second portion. */

	sscanf(time+6, "%lf", &fraction);
	while (fraction >= 1.0E6)
	{
	    fraction = fraction/10.0;
	}
	fraction = fraction + 0.5;
	
	sprintf(asciiUTC+11,"%.2s:%.2s:%.2s.%06.0fZ", time, time+2, time+4,
		fraction);
	break;
	
      case 6:

	/* The numeric field in the FGDC is exactly 6 characters long.  This
	   means the hours, minutes and seconds have been explicitly
	   included. */ 

	sprintf(asciiUTC+11, "%.2s:%.2s:%.2s.000000Z", time, time+2, time+4);
	break;

      case 4:

	/* The numeric field in the FGDC is exactly 4 characters long.  This
	   means the hours and minutes have been explicitly included, the
	   seconds are implied to be "00" in this case. */ 

	sprintf(asciiUTC+11, "%.2s:%.2s:00.000000Z", time, time+2);
	break;

      case 2:

	/* The numeric field in the FGDC is exactly 2 characters long.  This
	   means the hours have been explicitly included, the minutes and
	   seconds are implied to be "00" in this case. */ 

	sprintf(asciiUTC+11, "%.2s:00:00.000000Z", time);
	break;

      case 0:

	/* The numeric field in the FGDC is exactly 0 characters long.  The
	   hours ,minutes and seconds are all implied to be "00" in this
	   case. */

	sprintf(asciiUTC+11, "00:00:00.000000Z");
	break;
    }

    /* Check the character immediately following the initial numeric field in
       the FGDC time string. */

    switch (*(time+length))
    {
      case 'Z':

	/* The FGDC time represents UTC, no further correction is necessary,
	   check that the time (now in CCSDS ASCII Time format A) is a valid
	   time and return the results of the check as the results of this
	   function. */

	returnStatus = PGS_TD_timeCheck(asciiUTC);
	
	if (returnStatus == PGS_S_SUCCESS)
	{
	    PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);
	}
	
	return returnStatus;
	
      case '+':
      case '-':

	/* The FGDC time represents a local time.  This time must be corrected
	   by the time differential factor (immediately following the + /-
	   field). */

	if (strspn(time+length+1, "0123456789") != 4U)
	{
	    PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_FMT_ERROR,
				  "error in FGDC time differential factor "
				  "format (generic format: shhmm or Z)",
				  FUNCTION_NAME);

	    return PGSTD_E_TIME_FMT_ERROR;
	}

	/* extract the numerical values of the hours and minutes of the time
	   differential factor */

	sscanf(time+length, "%3d%2d", &tdf_hours, &tdf_minutes);

	/* the sign of the time was extracted with the hours field but really
	   applies to the minutes field as well, here the minutes are properly
	   adjusted */

	tdf_minutes = (tdf_hours < 0) ? -tdf_minutes : tdf_minutes;

	/* minutes should be 59 or less */
	
	if (fabs((PGSt_double)tdf_minutes) >= 60.0)
	{
	    PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_VALUE_ERROR,
				  "error in FGDC time differential factor "
				  "value (minutes >= 60)",
				  FUNCTION_NAME);
	    
	    return PGSTD_E_TIME_VALUE_ERROR;
	}

	/* convert the time differential factor from hours and minutes to a
	   fraction of a day */

	tdf_total = tdf_hours/HOURSperDAY + tdf_minutes/MINUTESperDAY;

	/* the maximum time zone differences from UTC are +13 hours and
	   -12 hours */
	
	if (tdf_total > 13.0/HOURSperDAY || tdf_total < -12.0/HOURSperDAY)
	{
	    PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_VALUE_ERROR,
				  "error in FGDC time differential factor "
				  "value (TDF > 13 hours or < -12 hours)",
				  FUNCTION_NAME);
	    
	    return PGSTD_E_TIME_VALUE_ERROR;		
	}
    
	/* If a leap second is occurring set "onLeap" to PGS_TRUE and adjust the
	   seconds field to be 59.  This is necessary since PGS_TD_UTCtoUTCjd()
	   will probably complain if a time with a leap second occurring at other
	   than midnight (such as is the case with a local time not coincident
	   with UTC and is never the case with UTC) is passed to it. */

	if (strncmp(asciiUTC+17, "60", 2) == 0)
	{
	    memcpy(asciiUTC+17, "59", 2);
	    onLeap = PGS_TRUE;
	}
	else
	{
	    onLeap = PGS_FALSE;	
	}
	
	/* Convert the ASCII UTC time we have constructed to the equivalent UTC
	   Julian date and then adjust by the time differential factor to
	   determine the actual UTC Julian date (as opposed to the "local"
	   Julian date which is what is really determined in the following
	   function call.  Adjust the UTC Julian date (i.e. the format) as
	   appropriate if the time differential factor causes the day portion to
	   be greater than or equal to 1 or less than zero (see NOTES above). */

	returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, jdUTC);
	if (returnStatus != PGS_S_SUCCESS)
	{
	    return returnStatus;
	}
	
	jdUTC[1] = jdUTC[1] - tdf_total;

	if (jdUTC[1] < 0.0)
	{
	    jdUTC[1] += 1.0;
	    jdUTC[0] -= 1.0;
	}
	
	if (jdUTC[1] >= 1.0)
	{
	    jdUTC[1] -= 1.0;
	    jdUTC[0] += 1.0;
	}
	
	/* convert the UTC Julian date back to CCSDS ASCII Time format A */

	returnStatus =  PGS_TD_UTCjdtoUTC(jdUTC, onLeap, asciiUTC);
	if (returnStatus == PGS_S_SUCCESS)
	{
	    PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);
	}
	return returnStatus;
	
      default:

	/* invalid character follows the initial time field(s) of the FGDC time
	   string */

	PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_FMT_ERROR,
			      "error in FGDC time differential factor "
			      "format (generic format: shhmm or Z)",
			      FUNCTION_NAME);

	return PGSTD_E_TIME_FMT_ERROR;
    }    
}
