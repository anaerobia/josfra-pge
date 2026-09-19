/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_SCtime_to_UTC.c

DESCRIPTION:
   This file contains the function PGS_TD_SCtime_to_UTC().
   This function converts spacecraft clock time in platform-dependent 
   format to UTC in CCSDS ASCII Time Code A format.
 
AUTHOR:
   Urmila Prasad / Applied Research Corp.
   Guru Tej S. Khalsa / Applied Research Corp.
   Peter D. Noerdlinger / Applied Research Corp.
   Abe Taaheri / SM&A Corp.
   Xin Wang / EIT Inc.

HISTORY:
   18-Mar-1994  UP  Initial version
   23-May-1994 GTSK Added ability to handle EOS-PM and TRMM conversions.
                    Moved EOS-AM conversion to separate routine, using
		    new algorithm.
   11-Aug-1994 GTSK Change calling sequence to accept an array of spacecraft
                    times and return an ASCII start time and an array of offsets
		    (in seconds) relative to the start time.
   06-Jun-1995 GTSK Complete rewrite.  Using function pointers instead of
                    multiple branching to handle different s/c time conversions.
		    Slightly altered error handling scheme (see DETAILS below).
   23-May-1997  PDN Removed all branches & references for predicted leap seconds
   01-Dec-1999   AT   Modified EOS PM to support GIIS and GIRD formats
   01-Nov-2001   XW   Modified to support AURA spacecraft
                      
END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert Spacecraft Clock Time to UTC Time

NAME:
   PGS_TD_SCtime_to_UTC()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_SCtime_to_UTC(
       PGSt_tag     spacecraftTag,
       PGSt_scTime  scTime[][8],
       PGSt_integer numValues,
       char         asciiUTC[28],
       PGSt_double  offsets[])

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD.f'
      include 'PGS_TD_3.f'

      integer function pgs_td_sctime_to_utc(spacecrafttag,sctime,
     >                                      numvalues,asciiutc,offsets)
      integer          spacecrafttag
      character*8      sctime(*)
      integer          numvalues
      character*27     asciiutc
      double precision offsets(*)

DESCRIPTION:
   This function converts spacecraft clock time in platform-dependent 
   format to UTC in CCSDS ASCII Time Code A format.
 
INPUTS:
   NAME            DESCRIPTION
   ----            -----------
   spacecraftTag   Spacecraft identifier; must be one of:
                   PGSd_TRMM, PGSd_EOS_AM, PGSd_EOS_PM_GIIS, PGSd_EOS_PM_GIRD
   
   scTime          Array of spacecraft clock times in platform dependent CCSDS
                   format.  UNITS, MAX, and MIN depend on the spacecraft,
		   see the files containing the specific conversions
		   for more information.

   numValues       number of elements in the input scTime array (and therefore
                   the output offsets array)

OUTPUTS:
   NAME            DESCRIPTION                                  UNITS
   ----            -----------                                  -----
   asciiUTC        UTC time of first s/c clock time in input    ASCII
                   array (in CCSDS ASCII Time Code A format).
		   The values of MAX, and MIN depend on the 
		   spacecraft, see the files containing the 
		   specific conversions for more information.
   
   offsets         Array of offsets of each input s/c clock     seconds
                   time in input array scTime relative to the
		   first time in the array.  This includes
		   the first time as well (i.e. the first
		   offset will be 0.0). The values of MAX, 
		   and MIN depend on the first time as well
		   the spacecraft, see the files containing
		   the specific conversions for more
		   information.

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_W_BAD_SC_TIME         one or more input s/c times could not be
                               deciphered
   PGSTD_E_BAD_INITIAL_TIME    the initial input s/c time (first time in input
                               array) could not be deciphered
   PGSTD_E_SC_TAG_UNKNOWN      unknown/unsupported spacecraft ID tag
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
C:
   #define ARRAY_SIZE 1000

   PGSt_scTime      scTime[ARRAY_SIZE][8];
   char             asciiUTC[28];
   PGSt_double      offsets[ARRAY_SIZE];
   PGSt_SMF_status  returnStatus;

   *** Initialize scTime array***
             :
             :

   returnStatus = PGS_TD_SCtime_to_UTC(PGSd_EOS_AM,scTime,ARRAY_SIZE,asciiUTC,
                                       offsets);
   if (returnStatus != PGS_S_SUCCESS)
   {
   *** do some error handling ***
             :
	     :
   }
   
FORTRAN:
       integer           pgs_td_sctime_to_utc
       integer           array_size/1000/
       character*8       sctime(array_size)
       character*27      asciiUTC
       double precision  offsets(array_size)
       integer           returnstatus

!      *** Initialize sctime array ***
                 :
                 :
       returnstatus = pgs_td_sctime_to_utc(pgsd_eos_am,sctime,
      >                                    array_size,asciiutc,offsets)

       if (returnstatus .ne. pgs_s_success) then
                 :
!      *** do some error handling ***
	         :
       endif

NOTES:
   This function converts an array of input s/c times to an initial time and an
   array of offsets relative to this initial time.  If the first time in the
   input array cannot be deciphered this function returns an error.  If any
   other time in the input array cannot be deciphered the corresponding offset
   is set to PGSd_GEO_ERROR_VALUE and this function continues after setting the
   return value to a warning (see DETAILS below).

   The ASCII time output is in CCSDS ASCII Time Code A format (see CCSDS 
   301.0-B-2 for details). 
   (CCSDS => Consultative Committee for Space Data Systems)
      The general format is:

          YYYY-MM-DDThh:mm:ss.ddddddZ (Time Code A)
	
	  where:
              -,T,: are field delimiters
	      Z is the (optional) time code terminator
	      other fields are numbers as implied:
                   4 Year digits, 2 Month, 2 Day, 2 hour,
                   2 minute, 2 second, and up to 6 decimal
                   digits representing a fractional second

   The input spacecraft times vary in format.  The supported spacecraft times
   are in the following formats:
      TRMM         CUC (platform specific variant of CUC used)
      EOS AM       CDS (platform specific variant of CDS used)
      EOS PM       CUC

   UTC: Coordinated Universal Time
   TAI: International Atomic Time
   CUC: CCSDS Unsegmented Time Code
   CDS: CCSDS Day Segmented Time Code
      
REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   This function checks for a correct spacecraft (s/c) tag and returns an error
   if an incorrect tag is encountered.  If the tag is valid the first s/c time
   is converted to UTC (in CCSDS ASCII Time Code A format) and the first offset 
   is set to 0.0.  Each additional s/c time in the input array is converted to 
   an offset in seconds relative to the first time and stored in the output 
   offsets array.
   
   Error reporting is handled as follows: 

   The first spacecraft time in the input array is converted to both UTC and TAI
   times.  If any error (i.e. SMF mask level 'E') is encountered converting the
   first s/c time in the input array to either UTC or TAI time this function
   calls PGS_SMF_GetMsg() to get the relevant message text.
   PGS_SMF_SetDynamicMsg() is then called with status code
   PGSTD_E_BAD_INITIAL_TIME and the message retrieved with the call to
   PGS_SMF_GetMsg().  This function then returns to the calling function with
   the status value PGSTD_E_BAD_INITIAL_TIME. If an error (i.e. SMF mask level 
   'E') is encountered in converting any s/c time to TAI time, offset 
   corresponding to that s/c time is set PGSd_GEO_ERROR_VALUE.  In addition, the 
   first time an error is encountered in a s/c time other than the initial time,
   return value of this function is set to PGSTD_W_BAD_SC_TIME to indicate to the 
   user that one or more s/c times from the input array could not be deciphered.

   In summary:

   A status level of 'S' indicates successful execution.

   A status level of 'W' MAY indicate that SOME input times could not be
   deciphered, the user should check EACH time offset returned against the
   value PGSd_GEO_ERROR_VALUE which indicates that the corresponding s/c time
   could not be deciphered.

   A status level of 'E' indicates that NO s/c times were successfully
   deciphered.

GLOBALS:
   None

FILES:
   The function PGS_TD_TAItoUTC() requires the file leapsec.dat

FUNCTIONS_CALLED:
   PGS_TD_TRMMtoTAI()
   PGS_TD_EOSAMtoTAI()
   PGS_TD_EOSPMGIIStoTAI()
   PGS_TD_EOSPMGIRDtoTAI()
   PGS_TD_TAItoUTC()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetUnknownMsg()
   PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/

#include <string.h>
#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_SCtime_to_UTC()"

PGSt_SMF_status
PGS_TD_SCtime_to_UTC(           /* converts s/c time to UTC time in ASCII */
    PGSt_tag     spacecraftTag, /* spacecraft identifier */
    PGSt_scTime  scTime[][8],   /* spacecraft Time */
    PGSt_integer numValues,     /* number of input values */
    char         asciiUTC[28],  /* UTC time in CCSDS ASCII Time Code A format */
    PGSt_double  offsets[])     /* time offsets array */
{
    PGSt_double     startTAI93; /* TAI time of initial s/c time */
    PGSt_double     secTAI93;   /* TAI time of s/c time */
    
    PGSt_integer    cntr;       /* loop counter */

    char            mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
                                                            PGS_SMF_GetMsg() */
    char            msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
                                                            PGS_SMF_GetMsg() */
    
    PGSt_SMF_status code;         /* status code returned by PGS_SMF_GetMsg() */
    PGSt_SMF_status returnStatus; /* value returned by this function */
    PGSt_SMF_status returnStatus1;/* value returned by calls to PGS functions */

    /* pointer to appropriate s/c time to TAI conversion function - one of:
           PGS_TD_TRMMtoTAI()
	   PGS_TD_EOSAMtoTAI()
	   PGS_TD_EOSPMGIIStoTAI()
	   PGS_TD_EOSPMGIRDtoTAI() */

    PGSt_SMF_status (*scTime_to_TAI)(PGSt_scTime [8], PGSt_double*);
    
    
    /* Must have at LEAST one time to process */

    if (numValues < 1)
    {
        PGS_SMF_SetStaticMsg(PGSTD_E_BAD_ARRAY_SIZE, FUNCTION_NAME);
        return PGSTD_E_BAD_ARRAY_SIZE;
    }

    /* Check Spacecraft tag to insure that it is valid.  If it
       is not a recognized value, the switch statement returns the
       indicated error status, otherwise the appropriate conversion
       routine is called */
    
    switch (spacecraftTag)
    {
      case PGSd_TRMM:
	scTime_to_TAI = PGS_TD_TRMMtoTAI;
	break;
      case PGSd_EOS_AM:
	scTime_to_TAI = PGS_TD_EOSAMtoTAI;
	break;
      case PGSd_EOS_PM_GIIS:
	scTime_to_TAI = PGS_TD_EOSPMGIIStoTAI;
	break;
      case PGSd_EOS_PM_GIRD:
	scTime_to_TAI = PGS_TD_EOSPMGIRDtoTAI;
	break;
      case PGSd_EOS_AURA:
        scTime_to_TAI = PGS_TD_EOSAURAGIRDtoTAI;
        break;
      default:  
	PGS_SMF_SetStaticMsg(PGSTD_E_SC_TAG_UNKNOWN,FUNCTION_NAME);
	return PGSTD_E_SC_TAG_UNKNOWN;
    }
    
    /* convert the first spacecraft time to UTC (CCSDS ASCII Time Code A) */

    returnStatus = scTime_to_TAI(scTime[0], &startTAI93);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "the value of TAI-UTC (leap seconds) could not "
			      "be determined for the initial spacecraft time "
			      "in the input array", FUNCTION_NAME);
      case PGSTD_E_BAD_2ND_HDR_FLAG:
      case PGSTD_E_MILSEC_TOO_BIG:
      case PGSTD_E_MICSEC_TOO_BIG:
      case PGSTD_E_BAD_P_FIELD:
      case PGSTD_E_UTCF_UNINITIALIZED:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if (code == returnStatus)
	    PGS_SMF_SetDynamicMsg(PGSTD_E_BAD_INITIAL_TIME,msg,FUNCTION_NAME);
	else
	    PGS_SMF_SetStaticMsg(PGSTD_E_BAD_INITIAL_TIME,FUNCTION_NAME);
	return PGSTD_E_BAD_INITIAL_TIME;
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* convert this first time to UTC (in CCSDS ASCII Time Code format A) */

    returnStatus = PGS_TD_TAItoUTC(startTAI93, asciiUTC);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_SetDynamicMsg(PGSTD_E_BAD_INITIAL_TIME,
			      "the value of TAI-UTC (leap seconds) could not "
			      "be determined for the initial spacecraft time "
			      "in the input array", FUNCTION_NAME);
	return PGSTD_E_BAD_INITIAL_TIME;
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	returnStatus = PGS_E_TOOLKIT;
    }

    /* set the first time offset to 0.0 (i.e. the ASCII time found above IS the
       first spacecraft time) */

    offsets[0] = 0.0;

    /* for each spacecraft time call the appropriate conversion routine to
       convert the spacecraft time to TAI then use this TAI to calculate the
       offset of each spacecraft time relative to the first spacecraft time */

    for(cntr=1;cntr<numValues;cntr++)
    {
	returnStatus1 = scTime_to_TAI(scTime[cntr], &secTAI93);
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGS_E_TOOLKIT:
	    return returnStatus1;
	  default:
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if (code != returnStatus1)
		    PGS_SMF_GetMsgByCode(PGSTD_W_BAD_SC_TIME,msg);
		returnStatus = PGSTD_W_BAD_SC_TIME;
	    }
	    offsets[cntr] = PGSd_GEO_ERROR_VALUE;
	    continue;
	}
	offsets[cntr] = secTAI93 - startTAI93;
    }
    
    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
    else
	PGS_SMF_SetDynamicMsg(returnStatus, msg, FUNCTION_NAME);
    
    return returnStatus;
}
