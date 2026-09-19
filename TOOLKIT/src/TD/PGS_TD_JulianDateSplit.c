/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_JulianDateSplit.c

DESCRIPTION:
   This file contains the function PGS_TD_JulianDateSplit().
   This function converts a Julian date to Toolkit Julian date format.

AUTHOR:
   Guru Tej S. Khalsa

HISTORY:
   14-Dec-1994  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Convert Julian Date to Toolkit Julian Date Format

NAME:
   PGS_TD_JulianDateSplit()

SYNOPSIS:
C:
   #include <PGS_TD.h>

   PGSt_double *
   PGS_TD_JulianDateSplit(
       PGSt_double  inputJD[2],
       PGSt_double  outputJD[2])

DESCRIPTION:
   This function converts a Julian date to Toolkit Julian date format (see NOTES
   section below).

INPUTS:
   Name         Description                    Units       Min   Max
   ----         -----------                    -----       ---   ---
   inputJD      a Julian date                  days        N/A   N/A

OUTPUTS:
   Name         Description                    Units       Min   Max
   ----         -----------                    -----       ---   ---
   outputJD     the input Julian date in       days        N/A   N/A
                Toolkit Julian date format

RETURNS:
   Julian date in Toolkit Julian date format (pointer to outputJD).

EXAMPLES:
C:
   What?  Are you serious?
  
NOTES:
   This function takes as input an array of two real numbers and returns an
   array of two real numbers in the Toolkit Julian day format (as described
   below).  The same array can be passed in for input and output.

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

REQUIREMENTS:
   PGSTK - 1170, 1220

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_TD.h>

PGSt_double *
PGS_TD_JulianDateSplit(      /* divide a Julian date into:
				1. a half-integer Julian day
				2. the fraction of that Julian day */
    PGSt_double inputJD[2],  /* two PGSt_doubles representing a Julian date */
    PGSt_double outputJD[2]) /* same date reformatted as necessary */
{
    PGSt_double remainder;   /* fractional part of Julian date component */
    PGSt_double temp;        /* temporary variable used in exchanging the 
				values of outputJD[0] and outputJD[1] */
    outputJD[0] = inputJD[0];
    outputJD[1] = inputJD[1];

    /* the Julian date must be of the form:
       outputJD[0] = (Julian Day #) + 0.5
       outputJD[1] = Julian Day fraction (0 <= outputJD[1] < 1) */

    /* Make sure outputJD[0] is greater in magnitude than outputJD[1] */

    if (fabs(outputJD[1]) > fabs(outputJD[0]))
    {
	temp = outputJD[0];
	outputJD[0] = outputJD[1];
	outputJD[1] = temp;
    }

    /* Make sure outputJD[0] is half integral */

    if ((remainder=fmod(outputJD[0],1.0)) != 0.5)
    {
	outputJD[0] = outputJD[0] - remainder + 0.5;
	outputJD[1] = outputJD[1] + remainder - 0.5;
    }

    /* Make sure magnitude of outputJD[1] is less than 1.0 */

    if (fabs(outputJD[1]) >= 1.0)
    {
	remainder=fmod(outputJD[1],1.0);
	outputJD[0] += outputJD[1] - remainder;
	outputJD[1] = remainder;
    }

    /* Make sure outputJD[1] is greater than or equal to 0.0 */

    if (outputJD[1] < 0)
    {
	/* Very small negative values of outputJD[1] (e.g. -1.0E-20) will cause
	   us to end up here (i.e. the above test will evaluate as true), but
	   if 1.0 is added to these small values the result will be 1.0 (to
	   computer precision).  Therefore make sure that the sum of outputJD[1]
	   and 1.0 can be recognized as different from 1.0 before we increment
	   outputJD[0].  If outputJD[1] is too small, just set it equal to zero
	   (on a typical machine with 8 byte double, about 14 decimal places can
	   be unambiguously maintained, since outputJD has units of days this is
	   equivalent to about a nano second). */

	if ((outputJD[1] + 1.0) < 1.0)
	{
	    outputJD[0] -= 1.0;
	    outputJD[1] += 1.0;
	}
	else
	    outputJD[1] = 0.0;
    }

    return outputJD;
}
