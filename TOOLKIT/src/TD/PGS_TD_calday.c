/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_calday.c

DESCRIPTION:
   This file contains the function PGS_TD_calday().
   PGS_TD_calday() converts Julian day to calendar day.

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
      Dec-1993   PDN  Acquired from Gernot M. R. Winkler of USNO
   04-Dec-1993  GTSK  Added comments and prologs
   27-Jul-1994  GTSK  Modified prolog to conform to latest ECS/PGS standards,
                      added comments
   27-Jul-1994   PDN  Added comments

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Converts Julian day to calendar day

NAME:
   PGS_TD_calday()

SYNOPSIS:
   #include <PGS_TD.h>

   void
   PGS_TD_calday(
       PGSt_integer  julianDayNum,
       PGSt_integer  *year,
       PGSt_integer  *month,
       PGSt_integer  *day)

DESCRIPTION:
   Converts Julian day to calendar day
 
INPUTS:
   NAME           DESCRIPTION            UNITS         MIN        MAX
   ----           -----------            -----         ---        ---
   julianDayNum   Julian day number      days            see NOTES
					
OUTPUTS:				
   NAME           DESCRIPTION            UNITS         MIN        MAX
   ----           -----------            -----         ---        ---
   year           calendar year          years           see NOTES
   month          calendar month         months          see NOTES
   day            calendar day           days            see NOTES

RETURNS:
   None

EXAMPLES:
   N/A

NOTES:
   This function accepts an integer Julian day number and returns integer values
   of year, month and day.  The values of the year, month and day returned
   are the values corresponding to the start of the Julian day input.  i.e. the
   Julian day input begins at NOON on the day represented by the return values.
   

   Based on the algorithms of H. F. Fliegel and T.C. Van Flandern,
   Communications of the Association for Computing Machines 11,675 (1968)

   Julian day ==> number of mean Solar days since GMT noon Jan 1, 4713 BC

   The true Julian Day time system commenced on Jan 1, 4713 BC (year -4712),
   Greenwich noon on the Julian proleptic calendar (which is reported in the
   1991 Supplement to the Astronomical Almanac, p. 604 to be equivalent to
   -4714 Nov 23, Gregorian system).  Because many PGS functions access the 
   Modified Julian Date (MJD) (Julian Date less 2400000.5), and that system 
   commenced on Nov 17, 1858, with MJD = 0, it is recommended that this
   function not be used before MJD 0 (Nov 17, 1858), JD 2400000.5.

   This program is adapted from Program  MJDDOY 
                  :19870814winkler
   
   Gernot M. R. Winkler
   Time Service Department
   U.S. NAVAL OBSERVATORY
   3450 MASSACHUSETTS AVENUE NW
   WASHINGTON DC  20392-5420
   
   Neither Dr. Winkler nor the U.S. Naval Observatory is responsible for
   any errors in this program nor due to its use.
 
REQUIREMENTS:
   PGSTK- 1050, 0930

DETAILS:
   This program takes a whole Julian day number as an input
   and returns the corresponding calendar year, month and day.

GLOBALS:
   None
       
FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
*******************************************************************************/

#include <PGS_SMF.h>           /* defines PGSt_integer */
#include <PGS_TD.h>

void
PGS_TD_calday(                 /* converts Julian day to calendar components */
    PGSt_integer julianDayNum, /* Julian day */
    PGSt_integer *year,        /* calendar year */
    PGSt_integer *month,       /* calendar month */
    PGSt_integer *day)         /* calendar day (of month) */
{ 
    long         l;            /* intermediate variable */
    long         n;            /* intermediate variable */

    l = julianDayNum + 68569; 
    n = 4*l/146097; 
    l = l - (146097*n + 3)/4;  
    *year = (PGSt_integer) (4000*(l + 1)/1461001); 
    l = l - 1461*(*year)/4 + 31; 
    *month = (PGSt_integer) (80*l/2447L); 
    *day = (PGSt_integer) (l - 2447*(*month)/80);  
    l = *month/11;  
    *month = (PGSt_integer) (*month + 2 -12*l);  
    *year = (PGSt_integer) (100*(n - 49) + *year + l); 
    return;  
}
