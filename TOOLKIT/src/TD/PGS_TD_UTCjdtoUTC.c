/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UTCjdtoUTC.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCjdtoUTC()
   The function converts UTC as a Julian date to UTC in CCSDS ASCII Time
   Code A format.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   01-Dec-1994  GTSK  Initial version.
   15-Dec-1994  GTSK  Added check to ensure that the input time is not an
                      invalid candidate time for a leap second if the user has
		      indicated that a leap second is occurring.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Convert UTC Julian date to CCSDS ASCII Time Code A format

NAME:     
   PGS_TD_UTCjdtoUTC()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCjdtoUTC(
       PGSt_double  jdUTC,
       PGSt_boolean onLeap,
       char         asciiUTC[28])
     
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'
   
      integer function pgs_td_utcjdtoutc(jdutc,onleap,asciiutc)
      double precision  jdutc(2)
      integer           onleap
      character*27      asciiutc

DESCRIPTION: 
   This tool converts UTC as a Julian date to UTC in CCSDS ASCII Time
   Code A format.

INPUTS:
   Name       Description                  Units
   ----       -----------                  -----

   jdUTC      UTC time as a Julian date    days	

   onLeap     Indicates if input time is   T/F
              occurring during a leap
	      second

OUTPUTS:					
   Name       Description                  Units
   ----       -----------                  -----
   asciiUTC   UTC time in CCSDS ASCII      time 
              Time Code A format
          
RETURNS:
   PGS_S_SUCCESS               successful return
   PGSTD_E_TIME_FMT_ERROR      a leap second was indicated at an inappropriate
                               time
   PGS_E_TOOLKIT               something unexpected happened

EXAMPLES:
C:
   PGSt_SMF_status  returnStatus;
   PGSt_double      jdUTC[2]={2449534.5,0.5};
   char             asciiUTC[28];

   returnStatus = PGS_TD_UTCjdtoUTC(jdUTC,PGS_FALSE,asciiUTC);
   if (returnStatus != PGS_S_SUCCESS)
   {
    *** do some error handling ***
             :
	     :
   }
   
   *** asciiUTC now contains the value: "1994-07-01T12:00:00.000000Z" ***
   printf("UTC: %s\n",asciiUTC);

FORTRAN:
      integer          pgs_td_utcjdtoutc
      integer          returnstatus
      double precision jdutc(2)
      character*27     asciiutc

      jdutc(1) = 2449534.5D0
      jdutc(2) = 0.5D0
      returnstatus = pgs_td_utcjdtoutc(jdutc,pgs_false,asciiutc)
      if (returnstatus .ne. pgs_s_success) goto 999

!  asciiutc now contains the value: '1994-07-01T12:00:00.000000Z'

      write(6,*) 'UTC: ', asciiutc

 999  ! *** do some error handling ***

NOTES:
   UTC is:  Coordinated Universal Time

   JULIAN DATES:

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
   PGSTK - 1210, 1220, 1160, 1170

DETAILS:
   None

GLOBALS:  
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_JulianDateSplit()
   PGS_TD_UTCjdtoTAIjd()
   PGS_TD_calday()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME   "PGS_TD_UTCjdtoUTC()"

/* constants */

#define SECONDSperDAY   86400.0   /* number of seconds in a day */

PGSt_SMF_status
PGS_TD_UTCjdtoUTC(                /* converts UTC Julian date to CCSDS ASCII
				     Time Code A format */
    PGSt_double  jdUTCin[2],      /* UTC as a Julian date */
    PGSt_boolean onLeap,          /* set to true if UTC time is occurring during
				     a positive leap second */
    char         *asciiUTC)       /* UTC time in CCSDS ASCII time code A
				     format */
{
    PGSt_integer year;            /* year portion of date */
    PGSt_integer month;           /* month portion of date */
    PGSt_integer day;             /* day of month */

    int          hours;           /* hour of day */
    int          minutes;         /* minute of hour */
    
    PGSt_double  seconds;         /* seconds of minute */
    PGSt_double  jdUTC[2];        /* UTC as a Julian date */
    PGSt_double  dayFractionSecs; /* seconds of UTC day */
    PGSt_double  jdTAI[2];        /* TAI as a Julian date, this is only needed
				     if the input parameter onLeap has been set
				     to PGS_TRUE--and it is not itself used but
				     rather the return values from a call to 
				     PGS_TD_UTCjdtoTAIjd is checked to verify
				     that the input UTC time is an appropriate
				     time for a leap second to occur. */

    PGSt_SMF_status returnStatus1;/* return value of call to toolkit function */
    PGSt_SMF_status returnStatus; /* return value of function */

    /* due to various bugs/inconsistencies in various operating systems (or at
       least their C compilers) it seems necessary to render the seconds into
       an integer portion and a fractional portion which will represent integer
       microseconds.  Specifically every once in a while the DECalpha sprintf()
       will decide to put four leading zeros in a field where it has been told
       to put two leading zeros, and the Sun Solaris operating will put no 
       leading zeros under the same condition padding with blanks instead.  Both
       these conditions occurred when trying to convert seconds as a floating
       point number to an ASCII string. */

    int          intSecs;   /* integer number of UTC seconds of latest minute */
    int          fracSecs;  /* integer number of microseconds of latest second */

    /* initialize return value and message to indicate success */

    returnStatus = PGS_S_SUCCESS;

    /* make sure the Julian date is properly split */

    PGS_TD_JulianDateSplit(jdUTCin,jdUTC);
    
    /* Get calendar day components of Julian day.  The function PGS_TD_calday() 
       returns the year, month, and day on which the current Julian date 
       started, however since the Julian day starts at noon half a day should
       be added to the Julian day first and then this whole number Julian day
       will give us the proper calendar date.  Since the Julian day has
       been kept as integer.5 the day fraction portion already represents the
       fraction of the calendar day (as opposed to the actual fraction of the
       integer Julian day number) so there is no need to modify this value. */

    PGS_TD_calday((PGSt_integer) (jdUTC[0] + .5),&year,&month,&day);
   
    /* convert dayFractionUTC from days to seconds, this step is taken
       to persuade the computer to render better accuracies when parsing
       the dayFraction into the various time units below.  Note that after
       converting to seconds 5.e-7 is added to ensure roundoff to the nearest
       microsecond. The numbers 3600. and 60. below are of course seconds
       per hour and seconds per minute respectively. */

    dayFractionSecs = jdUTC[1]*SECONDSperDAY + 5.e-7;

    hours = (int) (dayFractionSecs/3600.);
    minutes = (int) ((dayFractionSecs - hours*3600.)/60.);
    seconds = (dayFractionSecs - hours*3600. - minutes*60.);
    intSecs = (int) seconds;
    fracSecs = (int) (fmod(dayFractionSecs,1.0)*1.e6);
    
    /* sometimes jdUTC[1] is right on the edge and is just shy of 1 when it
       should be 1 and "tip over" into the next day */

    if (hours == 24)
    {
	hours = 0;
	PGS_TD_calday((PGSt_integer) (jdUTC[0] + 1.5),&year,&month,&day);
    }
    
    /* If onLeap is set to PGS_TRUE then make sure that the given time is
       an appropriate time for a leap second.  The easy way to do this is to
       call the function PGS_TD_UTCjdtoTAIjd() and check for the return value
       PGSTD_E_TIME_VALUE_ERROR.  As an extra check make sure that the time
       is one second before midnight which is the only time a leap second may
       occur. */

    if (onLeap == PGS_TRUE)
    {
	returnStatus1 = PGS_TD_UTCjdtoTAIjd(jdUTC,PGS_TRUE,jdTAI);
	
	switch(returnStatus1)
	{
	  case PGS_S_SUCCESS:
	  case PGSTD_E_NO_LEAP_SECS:
	    if (hours == 23 && minutes == 59 && intSecs == 59)
	    {
		intSecs = 60;
		break;
	    }
	    returnStatus1 = PGSTD_E_TIME_VALUE_ERROR;
	    PGS_SMF_SetStaticMsg(returnStatus1,FUNCTION_NAME);
	  case PGSTD_E_TIME_VALUE_ERROR:
	    returnStatus = returnStatus1;
	    break;
	  case PGS_E_TOOLKIT:
	    return returnStatus1;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
    }
    
    /* create the ASCII time string in CCSDS ASCII Time Code A format from the
       various components of the calendar day and time */

    sprintf(asciiUTC,"%04d-%02d-%02dT%02d:%02d:%02d.%06dZ", (int) year, 
	    (int) month, (int) day, hours, minutes, intSecs, fracSecs);
    
    /* return to calling function */
    
    if (returnStatus == PGS_S_SUCCESS)
      PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    
    return returnStatus;
}

