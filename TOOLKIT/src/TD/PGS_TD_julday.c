/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_TD_julday.c

DESCRIPTION:
   This file contains the function PGS_TD_julday().
   PGS_TD_julday() converts calendar day to Julian day.

AUTHOR:
   Peter D. Noerdlinger / Applied Research Corporation
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
      Dec-1993   PDN  Acquired from Gernot M. R. Winkler of USNO
   26-Jul-1994  GTSK  Added comments and prologs, changed file name from
                      PGS_TD_Julday.c to PGS_TD_julday.c, changed function
		      name from julday() to PGS_TD_julday()
   26-Jul-1994   PDN  Added comments


END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Converts calendar day to Julian day

NAME:
   PGS_TD_julday()

SYNOPSIS:
   #include <PGS_TD.h>

   PGSt_integer
   PGS_TD_julday(
       PGSt_integer  year,
       PGSt_integer  month,
       PGSt_integer  day)

DESCRIPTION:
   Converts calendar day to Julian day
 
INPUTS:
   NAME         DESCRIPTION            UNITS         MIN        MAX
   ----         -----------            -----         ---        ---
   year         calendar year          years           see NOTES
   month        calendar month         months          see NOTES
   day          calendar day           days            see NOTES
					
OUTPUTS:				
   None		
		
RETURNS:	
   NAME         DESCRIPTION            UNITS         MIN        MAX
   ----         -----------            -----         ---        ---
   N/A          Julian day number      days            see NOTES

EXAMPLES:
   N/A

NOTES:
   This function accepts integer values of year, month and day and returns an
   integer Julian day number.  The integer Julian day number returned is the
   the Julian day that begins at NOON on the input year, month and day.

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

   This program is adapted from Program  DOYMJD 
                  :19870819winkler
   
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
   This program takes a calendar year, month and day as an input
   and returns the corresponding whole Julian day number.
   
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

PGSt_integer 
PGS_TD_julday(
    PGSt_integer year, 
    PGSt_integer month,
    PGSt_integer day)
{ 
    long         j1;    /*  Scratch Variable */
    long         j2;    /*  Scratch Variable */
    long         j3;    /*  Scratch Variable */
    
    j1 = 1461L*(year + 4800L + (month - 14L)/12L)/4L;  
    j2 = 367L*(month - 2L - (month - 14L)/12L*12L)/12L;
    j3 = 3L*((year + 4900L + (month - 14L)/12L)/100L)/4L;  
    return (PGSt_integer) (day - 32075L + j1 + j2 - j3);  
} 
