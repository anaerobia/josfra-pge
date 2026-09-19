/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

NAME:
   PGS_TD_gmst.c

DESCRIPTION
   This file contains the function PGS_TD_gmst().  The function converts UT1 to
   Greenwich Mean Sidereal Time expressed as the hour angle of the mean vernal
   equinox at the Greenwich meridian (in radians).

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation
   Guru Tej S. Khalsa   / Applied Research Corporation

HISTORY:
      Dec-1993   PDN  Acquired from E. Myles Standish at JPL (this is personal
                      and not JPL certified code).  Code was checked against the
		      1994 Astronomical Almanac for five different dates in 1994
		      and agrees in all tabulated digits (Test was and must be
		      in combination with the codes that transform UTC to UTC
		      as a Julian Day and then to UT1).
   21-Dec-1993  GTSK  Converted from FORTRAN to C.
   02-Aug-1994  GTSK  Updated prologs, variable names and variable types to
                      conform to latest ECS/PGS standards.  Converted to handle
                      the input as two double precision numbers.
   20-Aug-1994   PDN  Moved constants to "#defines", rechecked performance,
                      updated prolog
   31-May-1995  GTSK  Altered function to have return value of "gmst" instead of
                      returning "gmst" in calling sequence.  Changed names of
		      constants being "#defined" to C? from a? (where ? is 0-3).
   05-Jun-1995  GTSK  Altered/added comments in code.

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Converts UT1 to Greenwich Mean Sidereal Time

NAME:     
   PGS_TD_gmst()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_double
   PGS_TD_gmst(
       PGSt_double  jdUT1[2])
 
 FORTRAN:
   double precision function pgs_td_gmst(jdut1)
   double precision jdut1(2)

DESCRIPTION:
   The function converts UT1 expressed as a Julian day to Greenwich Mean 
   Sidereal Time, i.e. the hour angle of the vernal equinox at the
   Greenwich meridian (in radians).

INPUTS:
   Name     Description                 Units         Min        Max
   ----     -----------                 -----         ---        ---
   jdUT1    UT1 Julian date             days            See  Notes
	    
OUTPUTS:    
   None

RETURNS:
   Description                 Units         Min        Max
   -----------                 -----         ---        ---
   hour angle of the mean      radians       0.0        2 * pi
   vernal equinox at the 
   Greenwich meridian
          
EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     GMST is: Greenwich Mean Sidereal Time
     UT1 is:  Universal Time

   Historically, UT1 was used as a measure of time, but since 1958 it has served
   only as a measure of Earth rotation. The only real difference between UT1 and
   GMST is that UT1 measures Earth rotation in regards to the vector from Earth
   center to the mean Sun (a fictitious point that traverses the celestial
   equator at the same mean rate that the Sun apparently traverses the
   ecliptic), while GMST measures Earth rotation relative to the vernal equinox.
   Essentially, value of GMST in radians is larger than that of UT1 in radians
   by the ratio of the mean solar day to the sidereal day; however, there is a
   small correction due to precession.  The equation used here is valid for the
   period 1950 to well past 2000, so long as the definition of UT1 and the
   reference equinox (J2000) are not changed.  The basic limitation is the
   accuracy of UT1.  Users obtaining UT1 from the PGS Toolkit should observe
   time limitations in the function PGS_TD_UTCtoUT1().

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
       input and do NOT return a Julian date will first convert the input date
       (internal) to the above format.  Toolkit functions that have a Julian
       date as both an input and an output will assume the input is in the above
       described format but will not check and the format of the output may not
       be what is expected if any other format is used for the input.

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

   Users carrying jdUT1 as only one double precision number should supply it in
   the first member of the array, i.e. as jdUT1[0], and should set the second
   member, jdUT1[1], equal to 0.0.

REQUIREMENTS:
   PGSTK - 0770, 1050

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   None

FUNCTIONS_CALLED:
   None

FILES:
   None

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_TD.h>

/* constants */

#define  J2000           2451545.0       /* UT1 Julian day number of 1-1-2000 */
#define  DAYSperJCENTURY 36525.0         /* days per Julian century */
#define  SECONDSperDAY   86400.0         /* seconds per day */
#define  TWO_PI          6.2831853071796 /* radians per revolution */
#define  C0              67310.54841     /* units: seconds */
#define  C1              8640184.812866  /* units: seconds */
#define  C2              0.093104        /* units: seconds */
#define  C3              -6.2e-6         /* units: seconds */

PGSt_double
PGS_TD_gmst(                /* convert UT1 to GMST */
    PGSt_double   jdUT1[2]) /* UT1 Julian date */
{
    PGSt_double   t;        /* Julian centuries since J2000 (UT1) */
    PGSt_double   temp;     /* temporary variable for intermediate math step */
    PGSt_double   gmst;     /* Greenwich Mean Sidereal Time (radians) */

    /* The following equation is identical with that on p. B6 of the 1994
       Astronomical Almanac, except that the constant C0 differs from that in
       the Almanac by 43200, which is one half day in seconds.  The reason for
       this difference is that the A.A. is converting UT1 to GMST while the JPL
       suite of programs works in Julian Days.  Julian Days start at noon, not
       midnight.  Furthermore, the equation in the A. A. is for GMST as 0 hours
       UT1.  The equation below uses continuous UT1 for any given day.  This
       means that the final number calculated below (temp) is actually GMST at 0
       hours UT1 PLUS the amount that the sidereal time has advanced on the
       UT1 time between UT1 at 0 hours and the instantaneous value of UT1. */
    
    t = ((jdUT1[0] - J2000) + jdUT1[1])/DAYSperJCENTURY;
    temp = (((C3*t + C2)*t + C1)*t + C0)/SECONDSperDAY;

    /* The final GMST is GMST at 0 hours UT1 plus the elapsed sidereal time
       between 0 hours UT1 and the instantaneous value of UT1.  The second term
       in the following equation is the elapsed UT1 time which has a slower rate
       than sidereal time.  This means that the second term is less than the
       elapsed sidereal time ought to be by the amount that the sidereal time
       has advance on the UT1 time over the interval between 0 hours UT1 and
       the instantaneous UT1 time.  But this is the amount by which the first
       term exceeds the actual value of GMST at 0 hours UT1 (see comment above),
       so no need to adjust the second term by the ratio of sidereal to UT1
       time (as would be necessary if the first term were actually GMST at
       0 hours UT1). */

    gmst = fmod(temp,1.0) + fmod((jdUT1[1]+fmod(jdUT1[0],1.0)),1.0);

    /* We are looking for GMST as a fraction of a sidereal day (or revolution of
       the Earth).  The modular arithmetic above takes the input UT1 Julian day
       and reduces it to the fraction of the current day which is then added
       to the value of GMST at 0 hours of that day (plus the difference describe
       in the above comments).  This may result in a value of GMST between 0
       and 2 days (since GMST at 0 hours UT1 will be increasing from 0 to 1 (in
       days) as the year goes by).  Since GMST basically describes revolutions
       of the Earth values outside the range 0 to 1 are redundant. */

    if (gmst > 1.0)
	gmst -= 1.0;

    return TWO_PI*gmst;  /* conversion from revolutions to radians */
}
