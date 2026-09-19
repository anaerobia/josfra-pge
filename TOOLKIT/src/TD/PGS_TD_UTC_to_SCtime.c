/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_UTC_to_SCtime.c

DESCRIPTION:
   This file contains the function PGS_TD_UTC_to_SCtime().
   This function converts UTC in CCSDS ASCII Time Code A or CCSDS ASCII Time
   Code B format to spacecraft clock time in platform-dependent format.
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.
   Abe Taaheri / SM&A Corp.
   Xin Wang / EIT Inc.

HISTORY:
   23-May-1994 GTSK Initial version
   01-Dec-1999   AT   Modified EOS PM to support GIIS and GIRD formats 
   01-Nov-2001 XW   Modified to support AURA spacecraft
   04-Feb-2005 MP   Changed logging message to reflect correct file name
                    (NCR 41911)

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert UTC Time to Spacecraft Clock Time

NAME:
   PGS_TD_UTC_to_SCtime()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_UTC_to_SCtime(
       PGSt_tag     spacecraftTag,
       char         asciiUTC[28],
       PGSt_scTime  scTime[8])

FORTRAN:
   include 'PGS_SMF.f'
   include 'PGS_TD.f'
   include 'PGS_TD_3.f'
   
   integer function pgs_td_sctime_to_utc(spacecrafttag,asciiutc,sctime)
   integer          spacecrafttag
   character*27     asciiutc
   character*8      sctime

DESCRIPTION:
   This function converts UTC in CCSDS ASCII Time Code A or CCSDS ASCII Time
   Code B format to spacecraft clock time in platform-dependent format.
 
INPUTS:
   NAME            DESCRIPTION
   ----            -----------
   spacecraftTag   Spacecraft identifier; must be one of:
                   PGSd_TRMM, PGSd_EOS_AM, PGSd_EOS_PM_GIIS, PGSd_EOS_PM_GIRD
   
   asciiUTC        UTC time of in CCSDS ASCII Time Code A or CCSDS ASCII Time
                   Code B format.  The values of MAX, and MIN depend on the 
		   spacecraft, see the files containing the specific conversions
		   for more information.

OUTPUTS:
   NAME            DESCRIPTION
   ----            -----------
   scTime          Spacecraft clock time in platform dependent CCSDS
                   format.  UNITS, MAX, and MIN depend on the spacecraft,
		   see the files containing the specific conversions
		   for more information.

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_SC_TAG_UNKNOWN      unknown spacecraft tag
   PGSTD_E_TIME_FMT_ERROR      error in input time format
   PGSTD_E_TIME_VALUE_ERROR    error in input time value
   PGSTD_E_DATE_OUT_OF_RANGE   input date is out of range of s/c clock
   PGSTD_E_NO_LEAP_SECS        leap seconds correction unavailable at 
                               requested time
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
C:
   char             asciiUTC[28];
   PGSt_scTime      scTime[8];
   PGSt_SMF_status  returnStatus;

   strcpy(asciiUTC,"1995-02-04T12:23:44.125438Z");

   returnStatus = PGS_TD_UTC_to_SCtime(PGSd_EOS_AM,asciiUTC,scTime);
   if (returnStatus != PGS_S_SUCCESS)
   {
   *** do some error handling ***
             :
	     :
   }

FORTRAN:
      integer       pgs_td_utc_to_sctime
      character*27  asciiutc;
      character*8   sctime;
      integer       returnstatus;

      asciiutc = '1995-02-04t12:23:44.125438Z'

      returnstatus = pgs_td_utc_to_sctime(pgsd_eos_am,asciiutc,sctime)
      if (returnstatus .ne. pgs_s_success) then
                :
c     *** do some error handling ***
	        :
      endif



NOTES:
   UTC is:  Coordinated Universal Time

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

   The output spacecraft times vary in format.  The supported spacecraft times
   are in the following formats:
      TRMM         CUC (platform specific variant of CUC used)
      EOS AM       CDS (platform specific variant of CDS used)
      EOS PM       CUC

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   This function checks the input spacecraft ID tag returning an error if its
   value is unrecognized or unsupported.  Otherwise this function calls the
   appropriate function to translate the input ASCII time to the requested
   spacecraft time format.

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_UTCtoTRMM()
   PGS_TD_UTCtoEOSAM()
   PGS_TD_UTCtoEOSPMGIIS()
   PGS_TD_UTCtoEOSPMGIRD()
   PGS_SMF_SetUnknownMsg()
   PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

PGSt_SMF_status
PGS_TD_UTC_to_SCtime(          /* converts UTC time string to s/c clock time */
    PGSt_tag  spacecraftTag, /* Spacecraft identifier */
    char        *asciiUTC,     /* UTC time in CCSDS ASCII format (A or B) */
    PGSt_scTime scTime[8])     /* Spacecraft Time */
{
   
    PGSt_SMF_status returnStatus=PGS_S_SUCCESS; /* Value returned by function
						   initialized to indicate
						   success */

    /* Check Spacecraft tag to ensure that it is valid.  If it
       is not a recognized value, the switch statement returns the
       appropriate error status, otherwise the appropriate conversion
       routine is called */
    
    switch (spacecraftTag)
    {
    case PGSd_EOS_AM:
	returnStatus = PGS_TD_UTCtoEOSAM(asciiUTC, scTime);
	break;
    case PGSd_EOS_PM_GIIS:
	returnStatus = PGS_TD_UTCtoEOSPMGIIS(asciiUTC, scTime);
	break;
    case PGSd_EOS_PM_GIRD:
	returnStatus = PGS_TD_UTCtoEOSPMGIRD(asciiUTC, scTime);
	break;

    case PGSd_TRMM:
	returnStatus = PGS_TD_UTCtoTRMM(asciiUTC, scTime);
	break;
    case PGSd_EOS_AURA:
        returnStatus = PGS_TD_UTCtoEOSAURAGIRD(asciiUTC, scTime);
        break;
    default:  

	/* Unknown/unsupported or incorrect spacecraft identifier */

	returnStatus = PGSTD_E_SC_TAG_UNKNOWN;
	PGS_SMF_SetStaticMsg(PGSTD_E_SC_TAG_UNKNOWN,"PGS_TD_UTC_to_SCtime()");
    }

    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_TD_UTC_to_SCtime()");
	break;
      case PGSTD_E_NO_LEAP_SECS:
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSTD_E_DATE_OUT_OF_RANGE:
      case PGSTD_E_SC_TAG_UNKNOWN:
      case PGS_E_TOOLKIT:
	break;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,"PGS_TD_UTC_TO_SCtime()");
	returnStatus = PGS_E_TOOLKIT;
    }
    return returnStatus;
}
