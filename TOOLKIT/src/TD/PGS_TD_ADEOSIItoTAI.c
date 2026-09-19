/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_ADEOSIItoTAI.c

DESCRIPTION:
   This file contains the function PGS_TD_ADEOSIItoTAI().
   This function converts ADEOS II spacecraft clock time to TAI (as real
   continuous seconds since 12AM UTC 1-1-1993).
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
   26-Mar-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert ADEOS II Clock Time to TAI Time


NAME:
   PGS_TD_ADEOSIItoTAI()
 

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_ADEOSIItoTAI(
       PGSt_scTime  scTime[5],
       PGSt_double  *secTAI93)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_adeosiitotai(sctime,sectai93)
      character*5      sctime
      double precision sectai93

DESCRIPTION:
   This function converts ADEOS II spacecraft clock time to TAI (as real
   continuous seconds since 12AM UTC 1-1-1993).
 
INPUTS:
   NAME      DESCRIPTION
   ----      -----------
   scTime    ADEOS II clock time.  See NOTES below for detailed description as
             well as discussion of min and max values.

OUTPUTS:
   NAME      DESCRIPTION                UNITS      MIN              MAX
   ----      -----------                -----      ---              ---
   secTAI93  real continuous seconds    seconds       ** see NOTES **
             since 12AM UTC 1-1-93

RETURNS:
   PGS_S_SUCCESS                successful execution
   PGSTD_E_TMDF_UNINITIALIZED   attempted access (get) of uninitialized TMDF
                                values
   PGS_E_TOOLKIT                an unexpected error occurred

EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

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
       
     Note: this quantity is s/c clock time.  To derive an actual UTC time
     several other variables must be known.  These are the s/c clock reference
     value, the s/c clock period and the ground reference value.  These values
     are collectively referred to as the TMDF (Time Difference) values.

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

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
   PGS_TD_ManageTMDF()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>


/* name of this function */

#define FUNCTION_NAME "PGS_TD_ADEOSIItoTAI()"

PGSt_SMF_status
PGS_TD_ADEOSIItoTAI(              /* converts ADEOS-II s/c time to TAI */
    PGSt_scTime   scTime[5],      /* ADEOS-II s/c time */
    PGSt_double   *secTAI93)      /* seconds since 12AM UTC 1-1-1993 */
{
    PGSt_double   sc_clock;       /* s/c clock (to nearest 1/512 second) */

    PGSt_double   period;         /* ADEOS-II s/c clock period */
    PGSt_double   sc_ref;         /* ADEOS-II s/c clock reference time */
    PGSt_double   grnd_ref;       /* ADEOS-II ground reference time */

    PGSt_SMF_status returnStatus; /* return value of this function */
    
    /* calculate s/c clock time as instrument time + pulse time (see NOTES
       above) */

    sc_clock = (((scTime[0]*256U +scTime[1])*256U + scTime[2])*256U +scTime[3]);
    sc_clock = sc_clock/32.0;
    sc_clock = sc_clock + (PGSt_double) scTime[4]/512.0;

    /* get the TMDF values */

    returnStatus = PGS_TD_ManageTMDF(PGSd_GET, &period, &sc_ref, &grnd_ref);
    if (returnStatus != PGS_S_SUCCESS)
    {
	return returnStatus;
    }
    else if (scTime[4] > 15U)
    {
	PGS_SMF_SetDynamicMsg(PGSTD_E_TIME_VALUE_ERROR,
			      "the first 4 bits of the Pulse Time field are "
			      "not null (they should be)",
			      FUNCTION_NAME);
	returnStatus = PGSTD_E_TIME_VALUE_ERROR;
    }

    /* convert s/c clock time to TAI */

    *secTAI93 = period*(sc_clock - sc_ref) + grnd_ref;
    
    return returnStatus;
}
