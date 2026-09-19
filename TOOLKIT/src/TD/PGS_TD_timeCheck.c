/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_timeCheck.c

DESCRIPTION:
  This file contains the function PGS_TD_timeCheck()

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation
  Peter D. Noerdlinger / SM&A Inc.

HISTORY:
  01-Mar-1994  GTSK  Initial version
  19-Jul-1994  GTSK  Modified to check for validity of CCSDS ASCII Time Code B
                     inputs (previously only checked for valid CCSDS ASCII Time
		     Code A inputs).  Increased efficiency slightly.  Added
		     prolog and comments (NONE previously).
  25-Jul-1994  GTSK  Further spiffing up of prologs to meet latest ECS/PGS 
                     standards.  Added code to allow leap seconds (i.e. seconds
		     equal to or greater than 60) only at 23:59 (i.e. just
		     before midnight).
  27-Oct-1999   PDN  Added a check for an input string pointer that is not
                     NULL and a minimum input string length (must have a
                     4 digit year) to prevent core dumps out of C library
                     string functions

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Verify or repudiate validity of input CCSDS ASCII time string

NAME:     
   PGS_TD_timeCheck()

SYNOPSIS:
   #include <PGS_TD.h>
   
   PGSt_SMF_status
   PGS_TD_timeCheck(
       char   *asciiUTC)
      
DESCRIPTION:
   This function accepts a character array (string) as an input and returns
   a value indicating if the string is in a valid CCSDS ASCII format.

INPUTS:
   Name         Description                     Units      Min     Max
   ----         -----------                     -----      ---     ---
   asciiUTC     CCSDS ASCII time string         ASCII      N/A     N/A
                in Time Code A or Time
		Code B format

OUTPUTS:
   None
          
RETURNS:
   PGS_S_SUCCESS               input string is in valid CCSDS 
                               ASCII Time Code A format
   PGSTD_M_ASCII_TIME_FMT_B    input string is in valid CCSDS
                               ASCII Time Code B format
   PGSTD_E_TIME_VALUE_ERROR    error in input time value
   PGSTD_E_TIME_FMT_ERROR      error in input time format

EXAMPLES:
   PGSt_SMF_status  returnStatus;
   char             asciiUTC[28];

   strcpy(asciiUTC,"1999-12-23T12:34:52.123456Z");
   returnStatus = PGS_TD_timeCheck(asciiUTC);
   if (returnStatus != PGS_S_SUCCESS)
   {
     ** handle errors **
              :
	      :
   }
	     

NOTES:
   The input is UTC time in CCSDS ASCII Time Code A or CCSDS ASCII Time 
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

   For the PGS Toolkit all fields up to the 'T' are mandatory (i.e. date must
   be specified).  After this minimum any number of the allowed field- 
   terminator/field combinations may be added.  For this purpose each place
   after the decimal point (terminator for the seconds field) is considered a
   field unto itself and has no terminator.

   The time string may not end on a field terminator and all fields that are
   included must be entirely included; padded with leading zeros if necessary
   (i.e. a two digit field MUST have two digits).

   All fields not included are assumed by PGS Toolkit software to be 0.

   The 'Z' terminator (not a field terminator) may optionally terminate any
   valid time string.

   Some examples:
      1994-123             <=== valid
      1995-126T            <=== invalid (may not end with field terminator)
      1996-235Z            <=== valid
      1992-12-24Z          <=== valid
      1999-01-12T23        <=== valid
      1986-1-14            <=== invalid (incomplete month field)
      1934-12-12T14: 4:12  <=== invalid (no leading zero in minute field)

REQUIREMENTS:
   PGSTK - 1180, 1190, 1210

DETAILS:
   This function accepts a character string as input and makes a local copy.
   The local copy is examined for the first occurrence of the character 'Z'.
   If found this 'Z' is replaced with the null character, terminating the
   string at this point an ignoring anything following the 'Z' in the input
   string.  A determination of the format (A or B) of the time string is made.
   The size of the string is then examined and an error returned if
   the size indicates that there are not enough characters to fully specify the
   date and/or to include another field (or multiple fields).  If the size of
   the string is OK the format of the string is checked.  If an unexpected 
   character is found (e.g. 'a') or a character is found in an unexpected place
   (e.g. a ':' as the second character in the year field) this function returns
   an error.  If the general format is OK the string is parsed into the
   numerical values of each field.  These are then checked to make sure that
   they are appropriate for the format, year, and time given (e.g. days < 28 for
   a month of 02 unless it is a leap year; hours less than 24 always).

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_timeCheck()"

PGSt_SMF_status
PGS_TD_timeCheck(              /* verify string is a valid CCSDS ASCII time */
    char         *asciiUTC)    /* time in CCSDS ASCII format (A or B) */
{
    char         tempTime[28]; /* temporary copy of input time */
    char         format='a';   /* A format or B format */
    char         *zPtr;        /* pointer to terminating Z char in time
				  string */
    
    char         specifics[PGS_SMF_MAX_MSG_SIZE]; /* string to hold a specific
						     message about an error */
    
    size_t       length;       /* length of time string */

    int cntr;                  /* loop counter */
    
    int          leapYear;     /* 1 if year is leap year, otherwise 0 */
    int          year;         /* calendar year */
    int          month;        /* calendar month */
    int          day;          /* day of month or day of year */
    int          hour=0;       /* hour of day */
    int          min=0;        /* minute of hour */
    int          adjust=0;     /* field offset in time string, 0 for format A
				  -2 for format B */

    double       sec=0.0;      /* second of minute (to nearest micro-second) */
   
    PGSt_SMF_status returnStatus=PGS_S_SUCCESS; /* return status of this 
						   function */
   
    /* copy asciiUTC into tempTime so the time string can be poked, prodded and
       otherwise manipulated without changing the input string.  Make sure the
       result is null terminated. First check that string is reasonable. */
    
    if(asciiUTC == NULL)
    {
          PGS_SMF_SetStaticMsg(PGSTD_E_TIME_FMT_ERROR,FUNCTION_NAME);
          return PGSTD_E_TIME_FMT_ERROR;
    }
 
    if(strlen(asciiUTC) > 5) /* require at least a year + terminator */
    {
          strncpy(tempTime,asciiUTC,28);
          tempTime[27] = '\0';
    }
    else
    {
          PGS_SMF_SetStaticMsg(PGSTD_E_TIME_FMT_ERROR,FUNCTION_NAME);
          return PGSTD_E_TIME_FMT_ERROR;
    }

    
    /* determine length of time string */

    length = strlen(tempTime);

    /* determine the useful length of input time string as determined by the
       terminating character 'Z'.  This is done by checking for the first
       occurrence of the character 'Z' in the time string and substituting a
       null character there by ignoring the string from the 'Z' on. */

    zPtr = strchr(tempTime,'Z');
    if (zPtr != NULL)
    {
	*zPtr = '\0';
	length = (size_t) (zPtr - tempTime);
    }

    /* if the eighth character of the time string is '-' then assume
       the input time is in CCSDS ASCII Time Code B format, otherwise
       assume default case of CCSDS ASCII Time Code A format */

    if (tempTime[7] != '-')
    {
	format = 'b';
	adjust = -2;
    }
	
    /* check for valid lengths.  The maximum length is 26 and this is
       guaranteed since tempTime was set up that way in the above code.
       The following code checks that the time string is long enough to
       contain the full date portion (left of the 'T') and that the
       string does not end in the middle of a field or on an element reserved
       for a field delimiter ('-', 'T', ':' or '.') */

    if (((int) length - adjust) < 21)
      switch ((int) length - adjust)
      {
	case 10:
	case 13:
	case 16:
	case 19:
	  break;
	default:
	  PGS_SMF_SetStaticMsg(PGSTD_E_TIME_FMT_ERROR,FUNCTION_NAME);
	  return PGSTD_E_TIME_FMT_ERROR;
      }
    
    /* now step through the string and check that each character in the
       string is in a place where it belongs (e.g. no field delimiter in
       the month field or number in a delimiter space) */
       
    for (cntr=0;cntr<(int)length;cntr++)
    {
	switch (tempTime[cntr])
	{
	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	    if (cntr == 4 || (cntr == 7 && format == 'a') || cntr == (10+adjust)
		|| cntr == (13+adjust) || cntr == (16+adjust) ||
		cntr ==	(19+adjust))
	    {
		returnStatus = PGSTD_E_TIME_FMT_ERROR;
		sprintf(specifics,"%s%c%s%d%s%s","A digit ('",tempTime[cntr],
			"') is in an inappropriate place (",cntr+1,") in the ",
			"time string");
	        cntr = (int) length;
	    }
	    break;
	  case '-':
	    if (!(cntr == 4 || (cntr == 7 && format == 'a')))
	    {
		returnStatus = PGSTD_E_TIME_FMT_ERROR;
		sprintf(specifics,"%s%s%d%s","A '-' is in an ",
			"inappropriate place (",cntr+1,") in the time string");
	        cntr = (int) length;
	    }
	    break;
	  case 'T':
	    if (cntr != (10+adjust))
	    {
		returnStatus = PGSTD_E_TIME_FMT_ERROR;
		sprintf(specifics,"%s%s%d%s","A 'T' is in an ",
			"inappropriate place (",cntr+1,") in the time string");
	        cntr = (int) length;
	    }
	    break;
	  case ':':
	    if (!(cntr == (13+adjust) || cntr ==(16+adjust)))
	    {
		returnStatus = PGSTD_E_TIME_FMT_ERROR;
		sprintf(specifics,"%s%s%d%s","A ':' is in an ",
			"inappropriate place (",cntr+1,") in the time string");
	        cntr = (int) length;
	    }
	    break;
	  case '.':
	    if (cntr != (19+adjust))
	    {
		returnStatus = PGSTD_E_TIME_FMT_ERROR;
		sprintf(specifics,"%s%s%d%s","A '.' is in an ",
			"inappropriate place (",cntr+1,") in the time string");
	        cntr = (int) length;
	    }
	    break;
	  default:
	    returnStatus = PGSTD_E_TIME_FMT_ERROR;
	    sprintf(specifics,"%s%c%s","An inappropriate character ('",
		    tempTime[cntr],"') is in the time string");
	    cntr = (int) length;
	}
    }

    /* if something was found to be in an inappropriate place, this is
       a format error, return to the calling function indicating this 
       error */

    if (returnStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	return returnStatus;
    }

    /* parse the time string.  This is obviously done differently for
       Time Code A (year/month/day) than for Time Code B (year/day of year) */

    if (format == 'a')
    {
	sscanf(tempTime,"%4d-%2d-%2dT%2d:%2d:%lfZ",&year,&month,&day,&hour,
	       &min,&sec);
    }
    else
    {
	sscanf(tempTime,"%4d-%3dT%2d:%2d:%lfZ",&year,&day,&hour,&min,&sec);
    }
    
    /* if the format is Time Code A, check to make sure the month is a valid
       number (i.e 0 < month < 13 ) */

    if (format == 'a')
    {
	if (month < 1 || month > 12)
	{
	    returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	    sprintf(specifics,"%s%s%d%s","error in ASCII time string value: ",
		    "bad month value (",month,")");
	    PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	    return returnStatus;
	}
    }
    
    /* check the hour, min, and sec variables for appropriate values.  There is
       no need to check for negative values since the above format check would
       catch a '-' in the wrong place.  The hours are of course on the 24 hour
       clock.  Note that the seconds are allowed to run up to just under 61
       seconds which may occur during a leap second.  This doesn't mean that
       this is appropriate at the given time but it should not be flagged as an
       error and it is left to the user not to put in an inappropriate value
       here. */

    if (hour > 23)
    {
	returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	sprintf(specifics,"%s%s%d%s","error in ASCII time string value: ",
		"bad hour value (",hour,")");
	PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	return returnStatus;
    }
    if (min > 59)
    {
	returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	sprintf(specifics,"%s%s%d%s","error in ASCII time string value: ",
		"bad minute value (",min,")");
	PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	return returnStatus;
    }
    if (sec > 60.99999999)
    {
	returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	sprintf(specifics,"%s%s%.6f%s","error in ASCII time string value: ",
		"bad second value (",sec,")");
	PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	return returnStatus;
    }
    else if (sec >= 60.0 && (min != 59 || hour != 23))
    {
	/* leap seconds always occur at midnight */

	returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	sprintf(specifics,"%s%s%.6f%s%s","error in ASCII time string value: ",
		"bad second value (",sec,") for input time (invalid time for ",
		"leap second)");
	PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	return returnStatus;
    }
	
    /* determine if input year is a leap year */

    leapYear = (year%4) ? 0 : 1;
    leapYear = (year%100) ? leapYear : 0;
    leapYear = (year%400) ? leapYear : 1;

    /* if input time string is in Time Code A format the day should be
       appropriate for the input month and year (i.e. 28, 29, 30 or 31).
       If input time string is in Time Code B format the day should be
       appropriate for the input year (i.e. 365 or 366). */

    if (format == 'a')
    {
	switch (month)
	{
	  case 1:
	  case 3:
	  case 5:
	  case 7:
	  case 8:
	  case 10:
	  case 12:
	    if (day > 31)
	      returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	    break;
	  case 4:
	  case 6:
	  case 9:
	  case 11:
	    if (day > 30)
	      returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	    break;
	  case 2:
	    if (day > (28 + leapYear))
	      returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	    break;
	  default:
	    returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	}
	if (returnStatus != PGS_S_SUCCESS)
	{
	    sprintf(specifics,"%s%s%d%s","error in ASCII time string value: ",
		    "bad day value (",day,") for given month");
	    PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	}
    }
    else
    {
	if (day > (365 + leapYear))
	{
	    returnStatus = PGSTD_E_TIME_VALUE_ERROR;
	    sprintf(specifics,"%s%s%d%s","error in ASCII time string value:",
		    " bad day value (",day,") for given year");
	    PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	}
    }
    
    /* if the time string is valid set return status to indicate if it is
       in Time Code B format and then set the appropriate message */
 
    if (returnStatus == PGS_S_SUCCESS)
    {
	if (format == 'b')
	  returnStatus = PGSTD_M_ASCII_TIME_FMT_B;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }

    return returnStatus;
}
