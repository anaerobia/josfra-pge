/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_UTCtoFGDC.c

DESCRIPTION:
  This file contains the function PGS_TD_UTCtoFGDC()
   This function converts UTC Time in CCSDS ASCII Time Code (format A or B) to
   FGDC ASCII date string and time string.

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  29-Feb-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert CCSDS ASCII Time Code to FGDC ASCII format

NAME:
   PGS_TD_UTCtoFGDC()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCtoFGDC(
       char asciiUTC[28],
       char tdf[6],
       char date[9],
       char time[18])

FORTRAN:
   include 'PGS_SMF.f'
   include 'PGS_TD_3.f'
    
   integer function pgs_td_utctofgdc(asciiutc, tdf, date, time)
   character*27 asciiutc
   character*5  tdf
   character*8  date
   character*17 time

      
DESCRIPTION:
   This function converts UTC Time in CCSDS ASCII Time Code (format A or B) to
   the equivalent FGDC ASCII date string and time string.  The time string will
   be in "Universal Time" or "local time" format depending on the value of the
   input variable tdf.

INPUTS:
   Name         Description                   Units       Min   Max
   ----         -----------                   -----       ---   ---
   asciiUTC     UTC Time in CCSDS ASCII          ** see NOTES **
                Time Code (format A or B)

   tdf          time differential factor,        ** see NOTES **
                if the value of tdf is "Z"
		or if tdf is a NULL pointer,
		the input ASCII UTC time
		string will be converted to
		FGDC date and "Universal
		Time" time strings, if tdf
		is a valid time differential
		factor, the input ASCII UTC
		will be converted to FGDC
		date and "local time" time
		strings.

OUTPUTS:
   Name         Description                   Units       Min   Max
   ----         -----------                   -----       ---   ---
   date         FGDC date string                 ** see NOTES **

   time         FGDC time string in              ** see NOTES **
                "Universal Time" format or
		"local time" format as
		determined by the input
		variable tdf (see INPUTS)
          
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_E_TIME_VALUE_ERROR    error in input time value
   PGSTD_E_TIME_FMT_ERROR      error in input time format
   PGS_E_TOOLKIT               something unexpected happened, execution
                               of function terminated prematurely

EXAMPLES:
C:
   PGSt_SMF_status returnStatus;
   char            asciiUTC[28];
   char            tdf[6];
   char            date[9];
   char            time[18];

   strcpy(asciiUTC,"1998-06-30T10:51:28.320000Z");
   strcpy(tdf,"-0500");

   returnStatus = PGS_TD_UTCtoFGDC(asciiUTC, tdf, date, time);
   if (returnStatus != PGS_S_SUCCESS)
   {
   ** test errors, take appropriate action **
                    :
                    :
   }

   ** the variable "date" is now: "19980630" **
   ** the variable "time" is now: "055128320000-0500" **

   printf("%s %s\n", date, time);

   returnStatus = PGS_TD_UTCtoFGDC(asciiUTC, NULL, date, time);
   if (returnStatus != PGS_S_SUCCESS)
   {
   ** test errors, take appropriate action **
                    :
                    :
   }

   ** the variable "date" is now: "19980630" **
   ** the variable "time" is now: "105128320000Z" **

   printf("%s %s\n", date, time);

FORTRAN:
      implicit none

      include  'PGS_SMF.f'
      include  'PGS_TD_3.f'

      integer      pgs_td_utctofgdc
      integer      returnstatus
      character*27 asciiutc_a
      character*5  tdf
      character*8  date
      character*17 time
      
      asciiutc = '1998-06-30T10:51:28.320000Z'
      tdf = '-0500'

      returnstatus = pgs_td_utctofgdc(asciiutc, tdf, date, time)
      if (returnstatus .ne. pgs_s_success) goto 999

!     the variable "date" is now: '19980630'
!     the variable "time" is now: '055128320000-0500'

      write(6,*) date, ' ', time

      tdf = 'Z'

      returnstatus = pgs_td_utctofgdc(asciiutc, tdf, date, time)
      if (returnstatus .ne. pgs_s_success) goto 999

!     the variable "date" is now: '19980630'
!     the variable "time" is now: '105128320000Z'

      write(6,*) date, ' ', time

 999  ** test errors, take appropriate action **
                       :
                       :

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
   PGS_TD_ASCIItime_BtoA()
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

#define   MINUTESperDAY  1440.0   /* number of minutes in a day */
#define   HOURSperDAY    24.0     /* number of hours in a day */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoFGDC()"

PGSt_SMF_status
PGS_TD_UTCtoFGDC(         /* convert date/time from CCSDS to FGDC format */
    char     asciiUTC[],  /* date (incl. time) in CCSDS ASCII format A or B */
    char     tdf[],       /* time differential factor */
    char     date[],      /* date in FGDC ASCII calendar date format */
    char     time[])      /* time in FGDC ASCII format */
{
    char              asciiUTC_A[28];  /* CCSDS ASCII Time format A */

    char*             asciiUTC_time="T00:00:00.000000Z"; /* default value of the
							    time portion of a
							    CCSDS ASCII Time
							    string */
    char*             z_ptr;           /* pointer to the Z terminator in a time
					  string */

    size_t            length;          /* length of a string */
    size_t            offset;          /* offset into asciiUTC_time */
    
    PGSt_integer      tdf_hours;       /* hours portion of the time differential
					  factor */
    PGSt_integer      tdf_minutes;     /* minutes portion of the time
					  differential factor */
    
    PGSt_double       tdf_total;       /* the total time differential factor
					  expressed as a fraction of a day */
    PGSt_double       jdUTC[2];        /* UTC Julian date */
    
    PGSt_SMF_status   returnStatus;    /* return value of function calls */
    
    /* check the format of the input time string */

    returnStatus = PGS_TD_timeCheck(asciiUTC);
    
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:

	/* copy the input time string into the scratch variable asciiUTC_A */

	strncpy(asciiUTC_A, asciiUTC, 28);
	break;
	
      case PGSTD_M_ASCII_TIME_FMT_B:

	/* the input time string was in CCSDS ASCII Time format B, convert it to
	   format A */

	PGS_TD_ASCIItime_BtoA(asciiUTC, asciiUTC_A);
	break;
	
      default:
	
	/* invalid input time string */

	return returnStatus;
    }

    /* if the length of the input time string is not the full 27 characters
       allowed, fill it out with the default values of any missing fields */

    length = strlen(asciiUTC_A);
    if (length != 27U)
    {
	z_ptr = strchr(asciiUTC_A, 'Z');
	if (z_ptr != NULL)
	{
	    *z_ptr = '\0';
	    length--;
	}
	offset = strlen(asciiUTC_time) - (27-length);
	
	strcat(asciiUTC_A, asciiUTC_time+offset);
    }

    /* If the seconds field of the input time is "60", verify that the input
       time is a valid time for the occurrence of a leap second.  Not the most
       efficient thing to do perhaps, but this should only happen VERY rarely and
       the use of the UTCjd routines generates appropriate error messages (as
       opposed to just running PGS_TD_UTCtoTAI() which will give inappropriate
       error messages). */

    if (*(asciiUTC_A+17) == '6')
    {
	PGS_TD_UTCtoUTCjd(asciiUTC_A, jdUTC);
	returnStatus = PGS_TD_UTCjdtoUTC(jdUTC, PGS_TRUE, asciiUTC_A);
	
	if (returnStatus != PGS_S_SUCCESS)
	{
	    return returnStatus;
	}
    }
    
    /* Construct the FGDC date and time strings, the format of the time string
       will depend on the input quantity "tdf" (time differential factor).  If
       tdf is the NULL string or if it is a "Z" then return the UTC version of
       FGDC time and date.  If tdf is in valid time differential factor format
       (shhmm) then adjust the input UTC to local time and return the local time
       format of the FGDC time and date. */

    if (tdf == NULL || tdf[0] == 'Z')
    {
	/* UTC format requested.  In this simple case the FGDC time format is
	   very similar to the CCSDS ASCII Time format A,  the date and time are
	   split into separate variables and the field separators are
	   removed. */

	sprintf(date, "%.4s%.2s%.2s", asciiUTC_A, asciiUTC_A+5, asciiUTC_A+8);
	sprintf(time, "%.2s%.2s%.2s%.7s", asciiUTC_A+11, asciiUTC_A+14,
		asciiUTC_A+17, asciiUTC_A+20);
    }
    else if (tdf[0] == '+' || tdf[0] == '-')
    {
	/* verify that "tdf" is in a valid format (or close enough) */

	if (strspn(tdf+1, "0123456789") != 4U)
	{
	    /* tdf is in an invalid format */

	    PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_FMT_ERROR,
				  "error in FGDC time differential factor "
				  "format (generic format: shhmm or Z)",
				  FUNCTION_NAME);

	    return PGSTD_E_TIME_FMT_ERROR;
	}
	else
	{
	    /* extract the numerical values of the hours and minutes of the time
	       differential factor */

	    sscanf(tdf, "%3d%2d", &tdf_hours, &tdf_minutes);

	    /* the sign of the time was extracted with the hours field but
	       really applies to the minutes field as well, here the minutes are
	       properly adjusted */

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
	    
	}
	
	/* Convert the ASCII UTC time we have constructed to the equivalent UTC
	   Julian date and then adjust by the time differential factor to
	   determine the actual UTC Julian date (as opposed to the "local"
	   Julian date which is what is really determined in the following
	   function call.  Adjust the UTC Julian date (i.e. the format) as
	   appropriate if the time differential factor causes the day portion to
	   be greater than or equal to 1 or less than zero (see NOTES above). */

	returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC_A, jdUTC);
	jdUTC[1] = jdUTC[1] + tdf_total;

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
	
	/* retrieve new "local" value of time */

	PGS_TD_UTCjdtoUTC(jdUTC, PGS_FALSE, asciiUTC_A);
	
	/* if a leap second was occurring during the original UTC time, add it
	   back in to the just retrieved "local" time */

	if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
	{
	    memcpy(asciiUTC_A+17, "60", 2U);
	}

	/* convert CCSDS ASCII Time string and input tdf to FGDC date and time
	   strings (local time format) */

	sprintf(date, "%.4s%.2s%.2s", asciiUTC_A, asciiUTC_A+5, asciiUTC_A+8);
	sprintf(time, "%.2s%.2s%.2s%.6s%.5s", asciiUTC_A+11, asciiUTC_A+14,
		asciiUTC_A+17, asciiUTC_A+20, tdf);
    }
    else
    {
	/* the input variable "tdf" is in an invalid/unsupported format */

	PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_FMT_ERROR,
			      "error in FGDC time differential factor "
			      "format (generic format: shhmm or Z)",
			      FUNCTION_NAME);

	returnStatus = PGSTD_E_TIME_FMT_ERROR;
    }
    
    PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);

    return PGS_S_SUCCESS;
}

