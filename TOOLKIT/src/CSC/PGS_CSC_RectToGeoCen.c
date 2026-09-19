/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_RectToGeoCen.c

DESCRIPTION:
   This file contains the function PGS_CSC_RectToGeoCen().
   This function converts rectangular coordinates to geocentric latitudinal
   coordinates.

AUTHOR:
   Peter D. Noerdlinger/ Applied Research Corporation

HISTORY:
   10-Dec-1996 PDN Initial version
   02-Jan-1997 PDN Tidied up prologue; added FORTRAN binding
   03-Oct-2000 PDN Fixed a bug which returned |latitude| for latitude

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:
   Convert From Rectangular To Geocentric Latitudinal Coordinates

NAME:
   PGS_CSC_RectToGeoCen()

SYNOPSIS:
C:
   #include <PGS_CSC.h>

   void 
   PGS_CSC_RectToGeoCen( 
       PGSt_double  X[3],
       PGSt_double  *R,
       PGSt_double  *latitude,
       PGSt_double  *longitude)

DESCRIPTION:
   This function converts rectangular coordinates to Geocentric latitudinal
   coordinates.

INPUTS:
   Name         Description                    Units       Min       Max
   ----         -----------                    -----       ---       ---
   X[3]         rectangular coordinates          m         n/a       n/a

OUTPUTS:
   Name         Description                    Units       Min       Max
   ----         -----------                    -----       ---       ---
     R          distance from origin             m          0        n/a
   latitude     angle to equatorial plane      radians    -pi/2      pi/2
   longitude    azimuthal angle                radians    -pi        pi

*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

void
PGS_CSC_RectToGeoCen(
    PGSt_double  X[],
    PGSt_double  *R,
    PGSt_double  *latitude,
    PGSt_double  *longitude)
{
    PGSt_double rho;
    
    rho = X[0]*X[0] + X[1]*X[1];
    *R =  sqrt(rho + X[2]*X[2]);
    if(fabs(*R) > EPS_64 )
    {
	rho = sqrt(rho);
	*latitude = asin(X[2]/(*R));
    }
    else
    {
	*latitude = 0.0;
    }
    
    *longitude = atan2(X[1],X[0]);

    return;
}
