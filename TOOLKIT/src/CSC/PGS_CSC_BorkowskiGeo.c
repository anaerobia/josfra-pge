/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
   PGS_CSC_BorkowskiGeo.c
 
DESCRIPTION:
   This file contains the function PGS_CSC_BorkowskiGeo()
 
AUTHOR:
   Peter Noerdlinger / Space Applications Corporation

HISTORY:
   02-Jan-1997 PDN Adapted from IERS Version FORTRAN code
   06-Jan-1997 PDN Finished and Tested 
   24-Aug-1997 PDN Deleted an unused variable

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG
 
TITLE:
   This function transforms Geocentric cylindrical coordinates to geodetic
 
NAME:
   PGS_CSC_BorkowskiGeo()
 
SYNOPSIS:
C:
   #include <PGS_CSC.h>
 
   PGSt_SMF_status
   PGS_CSC_BorkowskiGeo(
    PGSt_double equatRad_A,
    PGSt_double polarRad_C,
    PGSt_double axialDist,
    PGSt_double z_ECR,
    PGSt_double *latitude,
    PGSt_double *altitude)


INPUTS:
   Name           Description                    Units    Min      Max
   ----           -----------                    -----    ---      ---
   axialDist      distance of the point from
                    the Earth's axis               m       0       see NOTES
   z_ECR          distance of the point N (+) or
                    S(-) of the equator            m    see NOTES  see NOTES
OUTPUTS:
   Name           Description                    Units       Min       Max
   ----           -----------                    ----        ---       ---
   latitude       geodetic latitude             radians     -pi/2      pi/2

   altitude       altitude off the spheroid     radians   see NOTES  see NOTES

RETURNS:
   PGS_S_SUCCESS               successful return
 
NOTES:

   The function is believed to be quite reliable from Earth center to many
   Earth diameters away, but in the Toolkit it is used only for points less
   than 90% of the mean Earth radius from Earth center, because an existing
   iterative algorithm works excellently at larger radii.

   This program is based on the exact solution in: 

   K.M. Borkowski, Bull. Geod. 63 pp. 50-56, (1989)]

   This adaptation is taken from from FORTRAN code in "IERS Technical Note 21: 
   The IERS Conventions (1996)" by  Dennis D. McCarthy, USNO, pp. 12,13

   The U.S. Naval Observatory is not responsible for any mistakes herein.

*******************************************************************************/

#define HALFPI          1.57079632679489662 /* half pi */
 #include <stdio.h> 
 #include <math.h> 
#include <PGS_CSC.h>
PGSt_SMF_status
    PGS_CSC_BorkowskiGeo(
    PGSt_double equatRad_A,  /* Earth spheroid equatorial radius (m) */
    PGSt_double polarRad_C,  /* Earth spheroid polar radius (m) */
    PGSt_double axialDist,   /* distance of a point from Earth axis */
    PGSt_double z_ECR,       /* Z coordinate from equatorial plane */
    PGSt_double *latitude,   /* geodetic latitude */
    PGSt_double *altitude)   /* altitude off the spheroid */
{
   PGSt_double  signedRad_C;   /* polar radius with sign of Z coordinate*/
   PGSt_double  E,F,P,Q,D,s,v,G,t,signZ; /* scratch variables - see Borkowski */

   if(z_ECR >= 0.0)
   {
      signZ = 1.0;
   }
   else
   {
      signZ =  -1.0;
   }
   signedRad_C = signZ * polarRad_C;
   if(fabs(axialDist) < EPS_64)
   {
      *latitude = signZ * HALFPI; 
      *altitude = fabs(z_ECR) - polarRad_C;
      return PGS_S_SUCCESS;
   }
   E = ((z_ECR + signedRad_C)*signedRad_C/equatRad_A - equatRad_A)/axialDist;
   F = ((z_ECR - signedRad_C)*signedRad_C/equatRad_A + equatRad_A)/axialDist;

/*  Find solution to:	t**4 + 2*E*t**3 + 2*F*t - 1 = 0  */

   P = (E*F + 1.0)*4.0/3.0;
   Q = (E*E - F*F)*2.0;
   D = P*P*P + Q*Q;
   if(D >= 0.0)
   {
      s = sqrt(D) + Q;
      if(s > 0)
      {
          s = exp(log(s)/3.0);
          v = P/s - s;
          /*  Improve the accuracy of numeric values of v */
          v = -(2.0*Q + v*v*v)/(3.0*P);
      }
      else if(s < 0)
      {
          s = -exp(log(-s)/3.0);
          v = P/s - s;
          /*  Improve the accuracy of numeric values of v */
          v = -(2.0*Q + v*v*v)/(3.0*P);
      }
      else
      {
         v = 0.0;
      }
   }
   else
   {
      v = 2.0*sqrt(-P)*cos(acos(Q/(P*sqrt(-P)))/3.0);
   }
   G = 0.5 * (E + sqrt(E*E + v));
   t = sqrt(G*G + (F - v * G)/(2.0*G - E)) - G;
   *latitude = atan2((1.0 - t*t)*equatRad_A,(2.0*signedRad_C*t));
   *altitude = (axialDist - equatRad_A*t)*cos(*latitude)
                + (z_ECR - signedRad_C)*sin(*latitude);
   return PGS_S_SUCCESS;
}
