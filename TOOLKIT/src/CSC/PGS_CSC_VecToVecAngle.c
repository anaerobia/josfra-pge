/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_VecToVecAngle.c

DESCRIPTION:
   This file contains the function PGS_CSC_VecToVecAngle()
   This function finds the angle between 2 vectors of known zenith and azimuth.

AUTHOR:
   Peter Noerdlinger / Applied Research Corporation

HISTORY:
   10 Nov-1996 PDN Designed and wrote

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:
   Calculate Angle Between Two Vectors

NAME:
   PGS_CSC_VecToVecAngle()

SYNOPSIS:
C:
   #include <PGS_CSC.h>

   PGSt_double
   PGS_CSC_VecToVecAngle(
       PGSt_double  zenith1,
       PGSt_double  zenith2,
       PGSt_double  azimuth1,
       PGSt_double  azimuth2)

DESCRIPTION:
   This function finds the angle between 2 vectors of known zenith and azimuth.
   For example, this angle is 0.0 if the vectors are parallel, pi/2 if 
       orthogonal, pi if opposite.

INPUTS:
   Name         Description              Units           Min           Max
   ----         -----------              -----           ---           -_-
   zenith1      zenith angle of one      radians         -pi/2         pi/2
                vector				               	

   zenith2      zenith angle of the      radians         -pi/2         pi/2
                other vector

   azimuth1     azimuthal angle of one	 radians         -pi           pi
                vector

   azimuth1     azimuthal angle of the	 radians         -pi           pi
                other vector

OUTPUTS:
   NONE

RETURNS:
   Description                      Units       Min       Max
   -----------                      ----_       ---       ---
   angle between the 2 vectors      radians      0        pi 


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

PGSt_double
PGS_CSC_VecToVecAngle(
    PGSt_double  zenith1,
    PGSt_double  zenith2,
    PGSt_double  azimuth1,
    PGSt_double  azimuth2)
{
    return (acos(cos(zenith1)*cos(zenith2) +
		 sin(zenith1)*sin(zenith2)*cos(azimuth1 - azimuth2)));
}
