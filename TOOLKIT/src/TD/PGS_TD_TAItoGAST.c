/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
   PGS_TD_TAItoGAST.c
 
DESCRIPTION:
   This file contains the function PGS_TD_TAItoGAST().
   This function converts TAI (toolkit internal time) to Greenwich Apparent
   Sidereal Time (GAST) expressed as the hour angle of the true vernal equinox
   of date at the Greenwich meridian (in radians).
 
AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation
 
HISTORY:
   03-Jun-1995  GTSK  Initial version
   21-May-1997  PDN   Fixed Comments
 
END_FILE_PROLOG:
*******************************************************************************/
 
/*******************************************************************************
BEGIN_PROLOG:
 
TITLE:    
   Convert TAI to GAST
 
NAME:     
   PGS_TD_TAItoGAST()
 
SYNOPSIS:
C:
   #include <PGS_TD.h>
 
   PGSt_SMF_status
   PGS_TD_TAItoGAST(
       PGSt_double  secTAI93,
       PGSt_double  *gast)
 
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
   
      integer function pgs_td_taitogast(sectai93,gast)
      double precision  sectai93
      double precision  gast
 
 
DESCRIPTION:
   This function converts TAI (toolkit internal time) to Greenwich Apparent
   Sidereal Time (GAST) expressed as the hour angle of the true vernal equinox
   of date at the Greenwich meridian (in radians).
 
INPUTS:
   Name       Description                Units    Min           Max
   ----       -----------                -----    ---           ---
   secTAI93   continuous seconds since   seconds  -426297609.0  see NOTES
              12AM UTC Jan 1, 1993

OUTPUTS:
   Name       Description                Units    Min           Max
   ----       -----------                -----    ---           ---
   gast       Greenwich Apparent         radians   0            2PI
              Sidereal Time

RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSCSC_W_PREDICTED_UT1      status of UT1-UTC correction is predicted
   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for
                               input time
   PGSTD_E_NO_UT1_VALUE        no UT1-UTC correction available
   PGS_E_TOOLKIT               something radically wrong occurred
 
EXAMPLES:
   None
 
NOTES:
   TIME ACRONYMS:
     
     GAST is: Greenwich Apparent Sidereal Time
     GMST is: Greenwich Mean Sidereal Time
     MJD is:  Modified Julian Date
     TAI is:  International Atomic Time
     UT1 is:  Universal Time
     UTC is:  Coordinated Universal Time

   TOOLKIT INTERNAL TIME (TAI):
 
     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).
 

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


   TIME BOUNDARIES for TRANSFORMATIONS BETWEEN UT1 and OTHER TIMES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file utcpole.dat which relates UT1 - UTC values to
     UTC dates.  The file "utcpole.dat" starts at Jan 1, 1972; therefore,
     the correction UT1 - UTC, which can be as large as (+-) 0.9 s, is not
     available until that date, and an error status message will be returned if
     this function is called with a time before that date.  The file
     utcpole.dat, which is maintained periodically, contains final (definitive),
     and predicted values (partially reduced data and short term predictions of 
     good accuracy).  The predictions run about a year ahead of real time. By
     that time, the error in UT1 is generally equivalent to 10 to 12 meters of
     equivalent Earth surface motion. Thus, when the present function is used,
     users should carefully check the return status.  A success status 
     message will be returned if all input times correspond to final values.
     A warning status message will be returned if predicted values are
     encountered.  An error message will be returned if the time requested is
     beyond the end of the predictions, or the file cannot be read.

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
   PGSTK - 1170?, 1210?
 
DETAILS:
   None
 
GLOBALS:  
   None
 
FILES:
   The function PGS_TD_TAItoUT1jd() requires the files:
       leapsec.dat
       utcpole.dat
 
FUNCTIONS_CALLED:
   PGS_TD_TAItoUT1jd()
   PGS_CSC_UTC_quickWahr()
   PGS_TD_gmst()
   PGS_TD_gast()
   PGS_SMF_SetStaticMsg()
 
END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>
#include <PGS_CSC.h>

/* name of this function */

#define FUNCTION_NAME "PGS_TD_TAItoGAST()"

PGSt_SMF_status
PGS_TD_TAItoGAST(          /* convert TAI to GAST */
    PGSt_double  secTAI93, /* TAI (toolkit internal time) */
    PGSt_double  *gast)    /* GAST (radians) */
{
    PGSt_double jdUT1[2];  /* UT1 Julian date equivalent of input TAI */
    PGSt_double jedTDB[2]; /* TDB Julian date equivalent of input TAI */
    PGSt_double gmst;      /* GMST equivalent of input TAI */
    PGSt_double dvnut[4];  /* nutation angles and rates
                                dvnut[0] - nutation in longitude (radians)
                                dvnut[1] - nutation in obliquity (radians)
                                dvnut[2] - nut. rate in longitude (radians/sec)
                                dvnut[3] - nut. rate in obliquity (rad/sec) */

    PGSt_SMF_status returnStatus; /* error status of this function */
    
    /* Get the nutation angle(s) (really only need nutation in longitude). */

    PGS_CSC_quickWahr(secTAI93,jedTDB,dvnut);

    /* convert TAI to UT1 Julian date */

    returnStatus = PGS_TD_TAItoUT1jd(secTAI93,jdUT1);

    /* return if PGS_E_TOOLKIT encountered, in all other cases continue (if 
       returnStatus is of status level 'E' the GAST found may be a gross
       approximation, but what the heck, lets do our best anyway, still
       returning an error) */

    if (returnStatus == PGS_E_TOOLKIT)
	return returnStatus;
    
    /* convert UT1 Julian date to GMST (in radians) */

    gmst = PGS_TD_gmst(jdUT1);

    /* get GAST from GMST and the nutation */

    *gast = PGS_TD_gast(gmst,dvnut[0],jedTDB);

    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    
    return returnStatus;
}
