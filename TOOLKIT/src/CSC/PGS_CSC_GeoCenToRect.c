/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_GeoCenToRect.c

DESCRIPTION:
   This file contains the function PGS_CSC_GeoCenToRect()
   This function converts Geocentric latitudinal coordinates to rectangular
   coordinates.

AUTHOR:
   Peter Noerdlinger / Applied Research Corporation

HISTORY:
   10 Nov-1996 PDN Designed and wrote

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:
   Converts From Geocentric Latitudinal To Rectangular Coordinates

NAME:
   PGS_CSC_GeoCenToRect()

SYNOPSIS:
C:
   #include <PGS_CSC.h>

   PGSt_double*
   PGS_CSC_GeoCenToRect(
       PGSt_double  R,
       PGSt_double  latitude,
       PGSt_double  longitude,
       PGSt_double  X[])

DESCRIPTION:
   This function converts Geocentric latitudinal coordinates to rectangular
   coordinates.

INPUTS:
   Name         Description                     Units       Min       Max
   ----         -----------                     -----       ---       ---
   R            distance from origin              m          0        n/a
   latitude     angle to equatorial plane       radians    -pi/2      pi/2
   longitude    azimuthal angle                 radians    -pi        pi  

OUTPUTS:
   Name         Description                     Units       Min       Max
   ----         -----------                     ----        ---       ---
   X[3]         rectangular coordinates           m         n/a       n/a

RETURNS:
   Description
   -----------
   pointer to X (see OUTPUTS above)

GLOBALS:
   NONE

FILES:
   NONE

FUNCTIONS CALLED:
   NONE

END_PROLOG
*******************************************************************************/

 
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

PGSt_double*
PGS_CSC_GeoCenToRect(
        PGSt_double  R,
        PGSt_double  latitude,
        PGSt_double  longitude,
        PGSt_double  X[])
 
{
    X[0] = R * cos(latitude)*cos(longitude);
    X[1] = R * cos(latitude)*sin(longitude);
    X[2] = R * sin(latitude);

    return X;
}
