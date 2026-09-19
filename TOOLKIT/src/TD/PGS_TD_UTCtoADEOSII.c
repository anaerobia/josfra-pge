/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_UTCtoADEOSII.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCtoADEOSII().
   This function converts UTC (in CCSDS ASCII Time Code-format A or B) to
   ADEOS II spacecraft clock time.
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
   26-Mar-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert UTC Time to ADEOS II Clock Time


NAME:
   PGS_TD_UTCtoADEOSII()
 

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTCtoADEOSII(
       char         asciiUTC[28],
       PGSt_scTime  scTime[5])

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_utctoadeosii(asciiutc,sctime)
      character*27     asciiutc
      character*5      sctime

DESCRIPTION:
   This function converts UTC (in CCSDS ASCII Time Code-format A or B) to
   ADEOS II spacecraft clock time.
 
INPUTS:
   NAME      DESCRIPTION                UNITS    MIN                  MAX
   ----      -----------                -----    ---                  ---
   asciiUTC  UTC time in CCSDS ASCII    time     1961-01-01T00:00:00  see NOTES
             Time Code (A or B format)

OUTPUTS:
   NAME      DESCRIPTION
   ----      -----------
   scTime    ADEOS II clock time.  See NOTES below for detailed description as
             well as discussion of min and max values.

RETURNS:
   PGS_S_SUCCESS                successful execution
   PGSTD_E_NO_LEAP_SECS         no leap seconds correction available for
                                input time
   PGSTD_E_TIME_FMT_ERROR       error in format of input ASCII UTC time
   PGSTD_E_TIME_VALUE_ERROR     error in value of input ASCII UTC time
   PGSTD_E_TMDF_UNINITIALIZED   attempted access (get) of uninitialized TMDF
                                values
   PGS_E_TOOLKIT                an unexpected error occurred

EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file, which is updated when a new
     leap second event is announced, contains actual (definitive) and predicted
     (long term; very approximate) dates of leap second events.  The latter can
     be used only in simulations.   If an input date is outside of the range of
     dates in the file (or if the file cannot be read) the function will use
     a calculated value of TAI-UTC based on a linear fit of the data known to be
     in the table.  This value of TAI-UTC is relatively crude estimate and may
     be off by as much as 1.0 or more seconds.  Thus, when the function is used
     for dates in the future of the date and time of invocation, the user ought
     to carefully check the return status.  The status level of the return 
     status of this function will be 'W' (at least) if the TAI-UTC value used
     is a predicted value.  The status level will be 'E' if the TAI-UTC value
     is calculated (although processing will continue in this case, using the
     calculated value of TAI-UTC).

   ADEOS-II TIME FORMAT:

     For EOS-AM, the input spacecraft (s/c) clock time scTime is a 40-bit value
     which is comprised of two binary counter values:
                 
     ("scTime" represents an array of 5 (unsigned) one byte elements)
          
     1)  Instrument Time - A 32-bit value (the first four elements of array
         scTime each with 8 bits containing a count of 1/32 seconds since an
	 arbitrary epoch.  The range of decimal values is 0-134217727.969,
	 computed as (256*256*256*element1+256*256*element2+256*element3+
	 element4)/32.0. The maximum decimal value of each of elements 1,2,3
	 and 4 is 255.

     2)  Pulse Time - A 4-bit value (the second 4 bits of element 5 of array
         scTime--the first 4 bits are ALWAYS set to 0) containing a count of
	 1/512 seconds.  The range of decimal values is 0.0-0.030 seconds
         computed as element5/512. The maximum decimal value of element 5 is 15.
       
     Note: this quantity is s/c clock time.  To derive an actuall UTC time
     several other variables must be known.  These are the s/c clock reference
     value, the s/c clock period and the ground reference value.  These values
     are collectively refered to as the TMDF (Time Difference) values.

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
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_UTCtoTAI()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoADEOSII()"

PGSt_SMF_status
PGS_TD_UTCtoADEOSII(           /* convert UTC time to ADEOSII s/c clock time */
    char         asciiUTC[28], /* UTC time (in ASCII) (input) */
    PGSt_scTime  scTime[5])    /* s/c clock time (output) */
{
    PGSt_double   sc_clock;     /* s/c clock (to nearest 1/512 second) */
    PGSt_double   period;       /* ADEOS-II s/c clock period */
    PGSt_double   sc_ref;       /* ADEOS-II s/c clock reference time */
    PGSt_double   grnd_ref;     /* ADEOS-II ground reference time */
    PGSt_double   secTAI93;
    
    unsigned int  course_time;  /* s/c clock (to nearest 1/32 second) */
    
    PGSt_SMF_status   returnStatus;  /* return status of PGS function calls */
    
    /*******************
     * BEGIN EXECUTION *
     *******************/

    /* initialize returnStatus to indicate success */

    returnStatus = PGS_S_SUCCESS;
    
    /* get the ADEOS-II TMDF values */

    returnStatus = PGS_TD_ManageTMDF(PGSd_GET, &period, &sc_ref, &grnd_ref);
    if (returnStatus != PGS_S_SUCCESS)
    {
	return returnStatus;
    }

    /* Call PGS_TD_UTCtoTAI() to convert input UTC time to TAI time in
       seconds since 12AM UTC 1-1-93. */

    returnStatus = PGS_TD_UTCtoTAI(asciiUTC, &secTAI93);
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
    
    if (fabs(period) < EPS_64)
    {
	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT, "attempt to divide by zero",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    sc_clock = (secTAI93 - grnd_ref)/period + sc_ref;

    if (sc_clock >= 134217728.0 || sc_clock < 0.0)
    {
	PGS_SMF_SetStaticMsg(PGSTD_E_DATE_OUT_OF_RANGE, FUNCTION_NAME);
	return PGSTD_E_DATE_OUT_OF_RANGE;
    }
    
    sc_clock = sc_clock*32.0;
    course_time = (unsigned int) sc_clock;

    scTime[0] = (PGSt_scTime) (course_time/16777216U);
    scTime[1] = (PGSt_scTime) ((course_time/65536U)%256U);
    scTime[2] = (PGSt_scTime) ((course_time/256U)%256U);
    scTime[3] = (PGSt_scTime) (course_time%256U);

    scTime[4] = (PGSt_scTime) (fmod(sc_clock,1.0)*16.0);

    return returnStatus;
}
