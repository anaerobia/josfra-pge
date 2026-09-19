/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_EOSPMGIRDtoTAI.c

DESCRIPTION:
   This file contains the function PGS_TD_EOSPMGIRDtoTAI().
   This function converts EOS PM (GIRD) spacecraft clock time in CCSDS 
   Unsegmented
   Time Code (CUC) (with explicit P-field) format to TAI (as real continuous
   seconds since 12AM UTC 1-1-1993).
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.
   Abe Taaheri / SM&A Corp.

HISTORY:
   09-Aug-1994  GTSK  Initial version
   01-Dec-1999   AT   Used PGS_TD_EOSPMtoTAI to convert EOS PM (GIRD format) 
                      to TAI time
   29-Jun-2000   AT   Modified TAI time keeping only 6 significant digits
                      after the decimal point
   03-Jan-2001   AT   Moved back to previous version
   05-Jun-2002   AT   Modified time conversion scheme for microsec part to 
                      correct 1 microsec discrapancy seen in some L0 files
                      between EDOS and TOOLKIT.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert EOS PM (GIRD) Clock Time to TAI Time

NAME:
   PGS_TD_EOSPMGIRDtoTAI()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_EOSPMGIRDtoTAI(
       PGSt_scTime  scTime[8],
       PGSt_double  *secTAI93)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_eospmtotai(sctime,sectai93)
      character*8      sctime
      double precision sectai93

DESCRIPTION:
   This function converts EOS PM (GIRD) spacecraft clock time in CCSDS 
   Unsegmented
   Time Code (CUC) (with explicit P-field) format to TAI (as real continuous
   seconds since 12AM UTC 1-1-1993).

INPUTS:
   NAME      DESCRIPTION
   ----      -----------
   scTime    EOS PM (GIRD) clock time in CCSDS unsegmented time code format 
             (CUC).
             See NOTES below for detailed description as well as discussion of
	     min and max values.

OUTPUTS:
   NAME      DESCRIPTION         UNITS     MIN             MAX
   ----      -----------	 -----     ---             ---
   secTAI93  real continuous     seconds   -1104537627.0   3190429668.999985
             seconds since
	     12AM UTC 1-1-93

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_BAD_P_FIELD         s/c clock P-field value is incorrect

EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     TAI is:  International Atomic Time

   EOS PM (GIRD) TIME FORMAT:

     For EOS-PM (GIRD), the output spacecraft (s/c) clock time scTime is a 
     64-bit value in CCSDS unsegmented (CUC) format, which is comprised of 
     three binary counter values:

     ("scTime" represents an array of 8 (unsigned) one byte elements)

     1)  A 16-bit value (the first two elements of array
         scTime each with 8 bits) containing the explicit 
         P-field.  The first element (octet) of the P-field 
         for EOS PM (GIRD) has a constant value of 10101110 (binary)
         which corresponds to a decimal value of 174.  The 
         second element of the P-field has a leading bit set
         to 0.  The other seven bits are a binary counter
         representing TAI-UTC with a maximum decimal value
         of 127 (1111111 binary).
     2)  A 32-bit value (the third through sixth elements of array
         scTime each with 8 bits) containing the number of
         seconds since an epoch of 12AM UTC January 1, 1958. 
         The range of decimal values is 0-4294967295, computed as 
         256*256*256*element3 + 256*256*element4 + 256*element5
         + element6. The maximum decimal value of each 
         element 1, 2, 3 and 4 is 255.
     3)  A 16-bit value (elements 7 and 8 of array
         scTime, each with 8 bits) containing the sub-seconds.
         The range of values is 0-65535 sub-seconds, computed as 
         256*element7 + element8. The maximum decimal value of
         each element 7 and 8 is 255.  The sub-seconds represent 
         the fraction of the last second.  This fraction is calculated
         as (sub-seconds)/(256^(# of sub-second elements)).

         This allows the s/c clock to represent times from 1958-01-01 to
         approximately 2094-02-06T06:26:25 (give or take a few seconds).

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
   This algorithm checks the P-field and returns an error if the first 9 bits do
   not match their expected values.  The last 7 bits are currently ignored (A
   look up table is used to compute the values of TAI-UTC).  The algorithm then
   computes the real decimal number of seconds since the epoch 1-1-58 from the
   input time and then converts this with a constant offset to real continuous
   seconds (+/-) since 1-1-93.

GLOBALS:
   None

FILES:
   None


FUNCTIONS_CALLED:
   PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

/* constants */

#define P_FIELD_MASK       65408UL       /* binary: 1111111110000000 */
#define P_FIELD_TEMPLATE   44544UL       /* binary: 1010111000000000 */
#define EPOCH_TIME         1104537627.0  /* (julian date 12AM TAI 1-1-1993) less
					    (julian date 12AM UTC 1-1-1958)
					    in TAI seconds */
    
/* name of this function */

#define FUNCTION_NAME "PGS_TD_EOSPMGIRDtoTAI()"

PGSt_SMF_status
PGS_TD_EOSPMGIRDtoTAI(                /* converts EOS PM (GIRD) s/c time to TAI */
    PGSt_scTime   scTime[8],      /* EOS PM (GIRD) s/c time in CUC */
    PGSt_double   *secTAI93)      /* seconds since 12AM UTC 1-1-1993 */
{
    PGSt_uinteger   seconds;      /* decimal number of s/c clock seconds */
    PGSt_uinteger   subSeconds;   /* decimal number of s/c clock sub-seconds */
    PGSt_uinteger   pField;       /* holder for p-field */
    PGSt_uinteger   micsec;

    pField = scTime[0]*256U + scTime[1];
    if ( (pField & P_FIELD_MASK) != P_FIELD_TEMPLATE)
    {
	PGS_SMF_SetStaticMsg(PGSTD_E_BAD_P_FIELD, FUNCTION_NAME);
	return PGSTD_E_BAD_P_FIELD;
    }
    
    seconds = ((scTime[2]*256U + scTime[3])*256U + scTime[4])*256U + scTime[5];
    subSeconds = scTime[6]*256U + scTime[7];
    /*
      *secTAI93 = (seconds - EPOCH_TIME) + ((PGSt_double) subSeconds)/0x10000;
    */

    /* sub_sec = 65536 corresponds to micsecond = 1000000. The conversion is
     * therefore micsecond =(1000000/65536)*sub_sec. Due to integer arithemetic
     * and 4-byte int limitation, and in order to round to nearest integer, the
     * conversion is done as follows [1000000/65536 = 15625/1024]:
     */

    micsec=(15625 * (unsigned int)subSeconds) / 1024;
    *secTAI93 = (seconds - EPOCH_TIME) + ((PGSt_double)micsec)/1000000.0;

    return PGS_S_SUCCESS;
}
