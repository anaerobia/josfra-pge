/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_UTCtoGPS.c

DESCRIPTION:
   This file contains the function PGS_TD_UTCtoGPS()

AUTHOR:
   Deborah A. Foch / Applied Research Corporation
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   11-Jul-1994  DAF  Initial Version
   15-Jul-1994  DAF  Changed variable names and types, added error captures
   19-Jul-1994  DAF  Updated prolog
   20-Jul-1994  DAF  Made changes resulting from Code Inspection
   22-Jul-1994  DAF  Added call to new SMF tool PSG_SMF_SetUnknownMsg
                      and added SMF tools to functions called in prolog
   11-Aug-1994  DAF  Updated error returns
   19-Aug-1994  DAF  Changed Max value and corrected typo in prolog
   25-Oct-1994  DAF  Updated calling sequence and date formats in prolog 
   25-Apr-1995  GTSK Rebased GPS time from 1993-01-01 to 1980-01-06
   22-May-1997  PDN  Deleted material on PRED_LEAPS; fixed NOTES

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG

TITLE:   
   Convert UTC Time to GPS Time

NAME: 
   PGS_TD_UTCtoGPS()

SYNOPSIS:
C:
   #include <PGS_TD.h>
   
   PGSt_SMF_status
   PGS_TD_UTCtoGPS(
       char        asciiUTC[28],
       PGSt_double *secGPS)

FORTRAN:
   include 'PGS_SMF.f'
   include 'PGS_TD_3.f'
     
   integer function   pgs_td_utctogps(asciiutc,secgps)
   character*27       asciiutc
   double precision   secgps
   
   
DESCRIPTION:
   This tool converts from UTC time to GPS time.  

INPUTS:
   Name      Description        Units     Min                 Max
   ----      -----------        -----     ---                 ---
   asciiUTC  UTC time in CCSDS  time      1961-01-01          2008-03-30
             ASCII Time Code A            T00:00:00           T23:59:59.999999 
             or  B format

OUTPUTS:
   Name      Description        Units      Min                 Max
   ----      -----------        -----      ---                 ---
   secGPS    continuous real    seconds    -599961636.577182   890956802.999999
             seconds since
             0 hrs UTC on 
             Jan. 6, 1980
           
RETURNS:
   PGS_S_SUCCESS                  successful return 
   PGSTD_E_NO_LEAP_SECS           no leap seconds correction available for
                                  input time
   PGSTD_E_TIME_FMT_ERROR         error in format of ascii UTC time
   PGSTD_E_TIME_VALUE_ERROR       error in value of the ascii UTC time
   PGS_E_TOOLKIT                  something unexpected happened, execution
                                  of function terminated prematurely
  
EXAMPLES:
C:
   char            asciiUTC[28];
   PGSt_double     secGPS;
   PGSt_SMF_status returnStatus;
   char            err[PGS_SMF_MAX_MNEMONIC_SIZE]
   char            msg[PGS_SMF_MAX_MSG_SIZE]

   returnStatus = PGS_TD_UTCtoGPS(asciiUTC,&secGPS)
   if(returnStatus != PGS_S_SUCCESS)
   {
       PGS_SMF_GetMsg(&returnStatus,err,msg);
       printf("\nERROR: %s",msg);
   }
	
FORTRAN:  
      integer           pgs_td_utctogps
      character*27      asciiutc
      double precision  secgps
      integer           returnstatus
      integer           anerror
      character*35      errname
      character*150     errmsg

      returnstatus = pgs_td_utctogps(asciiutc,secgps)
      if (returnstatus .ne. pgs_s_success) then
          returnstatus = pgs_smf_getmsg(anerr,errorname,errmsg)
          write(*,*) errname, errmsg
      endif

NOTES:
   TIME ACRONYMS:
     
     GPS is:  Global Positioning System
     MJD is:  Modified Julian Date
     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

   The input is UTC time in CCSDS ASCII Time Code A or B format (see document
   CCSDS 301.0-B-2 for details). (CCSDS = Consultative Committee for Space Data
   Systems.) The output is GPS time, which is continuous real seconds from
   01-06-80.  UTC time is converted to TAI time using tool PGS_TD_TAItoUTC.
   409881608.0 seconds are then added to TAI to obtain GPS time.
   
   The general CCSDS ASCII Time Code A Format is:
   
      YYYY-MM-DDThh:mm:ss.ddddddZ
      
      where:
      
      -,T,: are field delimiters
      Z is the (optional) time code terminator
      other fields are numbers as implied:
           4 Year digits, 2 Month, 2 Day, 2 hour,
           2 minute, 2 seconds, and up to 6 decimal
           digits representing a fractional second
   
   The general Time Code B Format is:
   
      YYYY-DDDThh:mm:ss.ddddddZ
      
      which is identical to A Format, except that the month, day combination
      MM-DD is replaced by day of year.
      
      DDD = Day of Year as a 3 character subfield with values 001-365 in non
      leap years and 001-366 in leap years.
   
      The CCSDS Formats require all leading zeros to be present.
      
      The B format will be processed similarly to the A; for example, a fatal
      message will issue if DDD = 366 in a non leap year, just as a fatal
      message would have been issued in A format for month = 02 and day = 29 in
      a non leap year.
     
      The output strings will be 27 characters in A format, including the "Z",
      and 25 in B format, including the "Z" (but not including the extra null
      character required in C strings for which an extra character must be
      allocated).
	  
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

REQUIREMENTS:
   PGSTK-1170, 1210

DETAILS: 
   NONE

GLOBALS:  
  NONE

FILES:
   The function requires the presence of the file
   leapsec.dat

FUNCTIONS_CALLED:
   PGS_TD_UTCtoTAI()        converts UTC time to TAI time
   PGS_SMF_SetStaticMsg()   set a known static message
   PGS_SMF_SetUnknownMsg()  search for unexpected message

END_PROLOG
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_UTCtoGPS()"

PGSt_SMF_status
    PGS_TD_UTCtoGPS(              /* converts GPS time to UTC time */
    char            asciiUTC[28], /* UTC time in CCSDS ASCII Time Code A or B
				     format */
    PGSt_double     *secGPS)      /* continuous seconds since UTC 1980-01-06 */
{
    PGSt_double     secTAI93;     /* continuous secs since UTC 01-01-93 */
    PGSt_SMF_status returnStatus; /* error status returned by this function */


    /* convert UTC time in CCSDS ASCII Time Code A or B Format to TAI time */
    /* Format is recognized by function called by PGS_TD_UTCtoTAI tool */
    
    returnStatus = PGS_TD_UTCtoTAI(asciiUTC,&secTAI93); 
    

    /* check errors from PGS_TD_UTCtoTAI() */
    
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
      case PGSTD_E_NO_LEAP_SECS:
	break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Rebase toolkit internal time (TAI) from 1993-01-01 to 1980-01-06 by
       adding 409881608.0 seconds.  This is GPS time. */
    
    *secGPS = secTAI93 + 409881608.0;
    
    return returnStatus;
	
}
