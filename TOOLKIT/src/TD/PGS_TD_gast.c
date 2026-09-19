/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

NAME:
   PGS_TD_gast.c

DESCRIPTION
   This file contains the function PGS_TD_gast().
   This function converts GMST, nutation in longitude and TDB Julian date to
   Greenwich Apparent Sidereal Time expressed as the hour angle of the true
   vernal equinox of date at the Greenwich meridian (in radians).

AUTHOR:
   Guru Tej S. Khalsa   / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   31-May-1995  GTSK  Initial version
   22-Apr-1996  GTSK  Accounted for Lunar effect on GAST by adding Lunar terms
                      to the value of GAST for times after 02-26-1997
   16-May-1996  PDN   Fixed comments

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Calculate GAST

NAME:     
   PGS_TD_gast()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_double
   PGS_TD_gast(
       PGSt_double  gmst,
       PGSt_double  dvnut0,
       PGSt_double  jedTDB[2])

 FORTRAN:
      double precision function pgs_td_gast(gmst, dvnut0, jedtdb)
      double precision gmst
      double precision dvnut0
      double precision jedtdb(2)

DESCRIPTION:
   This function converts GMST, nutation in longitude and TDB Julian date to
   Greenwich Apparent Sidereal Time expressed as the hour angle of the true
   vernal equinox of date at the Greenwich meridian (in radians).

INPUTS:
   Name     Description                 Units         Min        Max
   ----     -----------                 -----         ---        ---
   gmst     Greenwich Mean Sidereal     radians       ANY        ANY
            Time

   dvnut0   Nutation in longitude       radians       N/A        N/A
	
   jedTDB   TDB Julian Date             days          ANY        ANY
    
OUTPUTS:    
   None

RETURNS:
   Description                 Units         Min        Max
   -----------                 -----         ---        ---
   hour angle of the true      radians       0.0        2 * pi
   vernal equinox of date
   at the Greenwich meridian
          
EXAMPLES:
   N/A

NOTES:
   TIME ACRONYMS:
     
     GAST is: Greenwich Apparent Sidereal Time
     GMST is: Greenwich Mean Sidereal Time
     TDB is:  Barycentric Dynamical Time
     UTC is:  Coordinated Universal Time

   GAST = GMST + (equation of equinoxes)
   equation of equinoxes is:
   
       (nutation in longitude)*cos(MEAN obliquity of the ecliptic)
   
   the equation of the equinoxes is sometimes also seen as:
   
       (nutation in longitude)*cos(TRUE obliquity of the ecliptic)
   
   Where the true obliquity of the ecliptic is the mean obliquity of the
   ecliptic + the nutation in obliquity.  IERS recommends use of the 
   former.

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

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac
     and "IERS Technical Note 13: The IERS Standards, Ed. D.D. McCarthy, 
     U.S. Naval Observatory,  1992, pp. 30-31".


REQUIREMENTS:
   PGSTK - 0900, 0770

DETAILS:
   See "Theoretical Basis of the SDP Toolkit Geolocation Package for the ECS
   Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, for more
   information on the algorithm.

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

#define  J2000           2451545.0      /* TDB Julian day number of 1-1-2000 */
#define  DAYSperJCENTURY 36525.0        /* days per Julian century */
#define  ARCSECperRAD    206264.8062471 /* seconds or arc per radian */
#define  C0              84381.448      /* units: seconds of arc */
#define  C1              46.815         /* units: seconds of arc */
#define  C2              0.00059        /* units: seconds of arc */
#define  C3              0.001813       /* units: seconds of arc */

#define  A0  2.18243862436099     /* 125deg 2arc-min 40.28arc-sec (radians) */
#define  A1 33.7570459337535      /* 1934deg 8arc-min 10.539arc-sec (radians) */
#define  A2  3.61186192426604E-5  /* 7.455arc-sec (radians) */
#define  A3  3.87850944887629E-8  /* 0.008arc-sec (radians) */

#define  B1  1.27990811812918E-8  /* 0.00264arc-sec (radians) */
#define  B2  3.05432619099008E-10 /* 0.000063arc-sec (radians) */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_gast()"

PGSt_double
PGS_TD_gast(                  /* return Greenwich Apparent Sidereal Time */
    PGSt_double   gmst,       /* Greenwich Mean Sidereal Time */
    PGSt_double   dvnut0,     /* nutation in longitude (radians) */ 
    PGSt_double   jedTDB[2])  /* TDB Julian ephemeris date */
{
    PGSt_double   t;          /* Julian centuries since J2000 (TDB) */
    PGSt_double   epsilonA;   /* mean obliquity of date (radians) */
    PGSt_double   omega;      /* mean longitude of ascending node of the moon */
    PGSt_double   gast;       /* Greenwich Apparent Sidereal Time */
      
    t = ((jedTDB[0] - J2000) + jedTDB[1])/DAYSperJCENTURY;

    epsilonA = (C0 + t*(C1 + t*(C2 + C3*t)))/ARCSECperRAD;

    gast = gmst + dvnut0*cos(epsilonA);

    /* if the date is >= TDB 1997-02-26, add lunar effects (see reference:
       "IERS Technical Note 13", page 30) */

    if (jedTDB[0] >= 2450505.50)
    {
	/* reference: "IERS Technical Note 13", page 32 */

	omega = A0 + t*(A1 + t*(A2 +t*A3));
	
	/* reference: "IERS Technical Note 13", page 30 */

	gast += B1*sin(omega) + B2*sin(omega*omega);
    }

    return gast;
}
