/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_GPStoUTC.c

DESCRIPTION:
   This file contains the function PGS_TD_GPStoUTC()

AUTHOR:
   Deborah A. Foch / Applied Research Corporation
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   12-Jul-1994  DAF  Initial version
   15-Jul-1994  DAF  Changed variable names, added error captures
   19-Jul-1994  DAF  Updated prolog
   20-Jul-1994  DAF  Made changes resulting from Code Inspection
   22-Jul-1994  DAF  Added call to new SMF tool PGS_SMF_SetUnknownMsg
                     and added SMF tools to functions called in prolog
   11-Aug-1994  DAF  Updated error returns
   19-Aug-1994  DAF  Corrected Minimum and Maximum values in prolog
   25-Oct-1994  DAF  Updated Notes in Prolog to reflect Users Guide
   25-Apr-1995  GTSK Rebased GPS time from 1993-01-01 to 1980-01-06
   22-May-1997  PDN  Fixed NOTES; removed references to _W_PRED_LEAPS

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG

TITLE:    
      Convert GPS to UTC Time

NAME:  
      PGS_TD_GPStoUTC()

SYNOPSIS:
C:    
      #include <PGS_TD.h>     
  
      PGSt_SMF_status
      PGS_TD_GPStoUTC(           
            PGSt_double secGPS,  
	    char        asciiUTC[28])     
   
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD_3.f'
      
      integer function  pgs_td_gpstoutc(secgps,asciiutc)
      double precision  secgps
      character*27      asciiutc
     
DESCRIPTION:
      This tool converts from GPS time to UTC time. 

INPUTS:
   Name      Description        Units      Min                 Max
   ----      -----------        -----      ---                 ---
   secGPS    continuous real    seconds    -599961636.577182   see NOTES
             seconds since
             0 hrs UTC on 
             Jan. 6, 1980
           
OUTPUTS:
   Name      Description        Units      Min                 Max
   ----      -----------        -----      ---                 ---
   asciiUTC  UTC time in CCSDS  time       1961-01-01          see NOTES
             ASCII Time Code A 

RETURNS:
   PGS_S_SUCCESS                     successful return 
   PGSTD_E_NO_LEAP_SECS              no leap seconds correction for input time
   PGS_E_TOOLKIT                     something unexpected happened, execution
                                     of function terminated prematurely


EXAMPLES:
C:
   char            asciiUTC[28];
   PGSt_double     secGPS;
   PGSt_SMF_status returnStatus;
   char            err[PGS_SMF_MAX_MNEMONIC_SIZE]
   char            msg[PGS_SMF_MAX_MSG_SIZE]
   
   returnStatus = PGS_TD_GPStoUTC(secGPS,asciiUTC);
   if(returnStatus != PGS_S_SUCCESS)
   {
       PGS_SMF_GetMsg(&returnStatus,err,msg);
       printf("\nERROR: %s",msg);
   }
	
FORTRAN:  
      integer           pgs_td_gpstoutc
      character*27      asciiutc
      double precision  secgps
      integer           returnstatus
      integer           anerror
      character*35      errname
      character*150     errmsg
      
      returnstatus = pgs_td_gpstoutc(secgps,asciiutc)
      if(returnstatus .ne. pgs_s_success) then
          returnstatus = pgs_smf_getmsg(anerr,errorname,errmsg)
          write(*,*) errname, errmsg
      endif

NOTES:
   UTC is output in CCSDS ASCII Time Code A format (see CCSDS 301.0-B-2 for
   details).  (CCSDS = Consultative Committee for Space Data Systems)
   
   The general format is:
   
      YYYY-MM-DDThh:mm:ss.ddddddZ
   
      where:

      -,T,: are field delimiters
      Z is the (optional) time code terminator
      other fields are numbers as implied:
           4 Year digits, 2 Month, 2 Day, 2 hour,
	   2 minute, 2 seconds, and up to 6 decimal
	   digits representing a fractional second


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

TIME ACRONYMS:
 
     GPS is:  Global Positioning System
     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

REQUIREMENTS:
   PGSTK-1170, 1210

DETAILS: 
   The input is GPS time in seconds.  409881608.0 seconds are added to obtain
   TAI time and TAI is converted to UTC using the tool PGS_TD_TAItoUTC().

GLOBALS:  
   NONE

FILES:
   The function PGS_TD_TAItoUTC() requires the presence of the file:
   leapsec.dat

FUNCTIONS_CALLED:
   PGS_TD_TAItoUTC()         converts TAI time to UTC time
   PGS_SMF_SetStaticMsg()    set a known static message
   PGS_SMF_SetUnknownMsg()   search for unexpected message

END_PROLOG
*******************************************************************************/

#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_GPStoUTC()"

PGSt_SMF_status
PGS_TD_GPStoUTC(                    /* converts GPS time to UTC time */
    PGSt_double     secGPS,         /* continuous seconds since UTC 01-06-80 */
    char            asciiUTC[28])   /* UTC time in CCSDS ASCII Time Code A */
{
    PGSt_double     secTAI93;       /* seconds from 0 hours UTC 01-01-93 */

    PGSt_SMF_status returnStatus;   /* error status returned by this function */

    
    /* initialize returnStatus and message buffer to indicate success */

    returnStatus = PGS_S_SUCCESS;


    /* Rebase GPS time from 1980-01-06 to 1993-01-01 by subtracting 409881608.0
       seconds.  This is toolkit internal time (also referred to as TAI). */

    secTAI93 = secGPS - 409881608.0;
    
    /* convert TAI time to UTC time */

    returnStatus = PGS_TD_TAItoUTC(secTAI93,asciiUTC);

    /* check errors from PGS_TD_TAItoUTC() */

    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
      case PGSTD_E_NO_LEAP_SECS:
	break;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    return returnStatus;
}
