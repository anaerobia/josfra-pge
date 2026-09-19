/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_CSC_LookPoint()

DESCRIPTION:
  This file contains the function PGS_CSC_LookPoint()

AUTHOR:
  Peter Noerdlinger    / Applied Research Corporation
  Guru Tej S. Khalsa   / Applied Research Corporation

HISTORY:
  12-20-1993        PDN   Designed in consultation with James Storey
  12-21-1993        PDN   Coded
  12/22/93          GTSK  translated to C
  03-03-94          PDN   edited for clarity
  10/13/94          PDN   error checking modified
  10/13/94          PDN   added the second Earth intersection

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE: Get ECR Rectangular Coordinates of Pixel Center and mirror point

NAME:  PGS_CSC_LookTwice()

SYNOPSIS:
    #include <PGS_CSC.h>
                          

DESCRIPTION:

 Solves the look point equation.
  method:                                                         
        solve the quadratic equation
                x = p + d*u                                       
        where x must lie on an ellipsoid, for the slant range, 
        d, corresponding to the intersection of the extended 
        look vector, u, with the surface of the   earth. Then 
        compute x directly.   This variant also computes and
        outputs the "mirror image" point where the ray again
        exits the Earth far from the observer; this is needed
        for certain calculations by limb sounders observing
        rays refracted around the Earth, rays which would
        otherwise (without refraction) hit the ellipsoid.
 
INPUTS:

 Name             Description                  Units       Min        Max
 ----------------------------------------------------------------------------
 pos[3]         ECR position vector of SC         m     -10000000    +10000000
 pixUv[3]       look unit vector in ECR          N/A       -1           1
  					       
 axis_A         Earth semi-Major Axis             m      6,370,000    6,400,000
 axis_B         Earth semi-Major Axis             m      6,370,000    6,400,000
 axis_C         Earth semi-Minor Axis             m      6,350,000    6,380,000
 
  (In the PGS Toolkit axis_A and axis_B are assumed to be equal.)
  (large upper limits accommodate experiments like CERES which
   may inflate Earth axes to allow for height of atmosphere)
 
OUTPUTS:

 Name             Description                     Units       Min        Max
 ----------------------------------------------------------------------------
  slantRange[2]   distance from SC to lookpoint
                   or mirror point                      m      5000       100000000

  xLook[2][3]     ECR rectangular coords of lookpoint   m    -10000000    10000000
 

GLOBALS:
    none

FILES:
    none

FUNCTIONS_CALLED:

    none

AUTHOR:

   Peter D. Noerdlinger
   Applied Research Corp
   Original function PGS_CCS_LookPoint() done April 10, 1994
   Modified for two points March 7, 1997

RETURNS:

  Errors
  Returned                                          Description
  -----------------------------------------------------------------------------
 PGS_S_SUCCESS                                      knock on wood
 PGSCSC_W_MISS_EARTH                                boresight misses Earth
 PGSCSC_W_ZERO_PIXEL_VECTOR

 CHANGE HISTORY:
 Problem ID              Date                        Responsible Person
 ------------------------------------------------------------------------
 Designed                12/20/93                    P. Noerdlinger
                                      in consultation with James Storey
 Created                 12/21/93                    P. Noerdlinger
 translated to C         12/22/93                    G. Tej Khalsa  
 edited for clarity      03/03/94                    P. Noerdlinger
 error checking modified 10/13/94                    P. Noerdlinger
 second point added      03/07/97                    P. Noerdlinger


 END_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_SMF.h>

PGSt_SMF_status
PGS_CSC_LookTwice(               /* solves the look point equation */
    PGSt_double  pos[3],         /* SC   position vector */  
    PGSt_double  pixUv[3],       /* look unit vector */    
    PGSt_double  axis_A,         /* ellipsoid x axis dimension */  
    PGSt_double  axis_B,         /* ellipsoid y axis dimension */  
    PGSt_double  axis_C,         /* ellipsoid z axis dimension */  
    PGSt_double slantRange[2],   /* spacecraft to lookpoint */
    PGSt_double xLook[2][3])     /* solution vectors */       
{
    PGSt_double aCoef;           /* coefficient of square of slant range in quadratic eq */
    PGSt_double bCoef;           /* coefficient of slant range in quadratic eq */
    PGSt_double cCoef;           /* constant term in quadratic eq for slant range */
    PGSt_double radical;         /* sqrt(b*b-4*a*c) in usual quadratic equation notation */
    PGSt_double axis_SqA;        /* square of axis_A */
    PGSt_double axis_SqB;        /* square of axis_B */
    PGSt_double axis_SqC;        /* square of axis_C */
    PGSt_integer       i;        /* i=0 near, i=1 far */

    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;
    
    /**  Initializations  **/

    for(i=0; i < 2;i++)
    {
       xLook[i][0] = PGSd_GEO_ERROR_VALUE;
       xLook[i][1] = PGSd_GEO_ERROR_VALUE;
       xLook[i][2] = PGSd_GEO_ERROR_VALUE;
       slantRange[i] = PGSd_GEO_ERROR_VALUE;
    }

    /*========== define squares of axis lengths ==========*/
    
    axis_SqA = axis_A*axis_A;
    axis_SqB = axis_B*axis_B;
    axis_SqC = axis_C*axis_C;

    /*====== Check for Zero Pixel Vector   =========*/

    if ( fabs(pixUv[0]) + fabs(pixUv[1]) + fabs(pixUv[2])  < EPS_64)
    {
	returnStatus = PGSCSC_W_ZERO_PIXEL_VECTOR;
	return (returnStatus);
    }

    /*====== solve for distance = distance from s/c to look point:  =========*/
    
    aCoef = pixUv[0]*pixUv[0]/axis_SqA + pixUv[1]*pixUv[1]/axis_SqB + pixUv[2]*pixUv[2]/axis_SqC;
    bCoef = 2.0*(pos[0]*pixUv[0]/axis_SqA + pos[1]*pixUv[1]/axis_SqB + pos[2]*pixUv[2]/axis_SqC);
    cCoef = -1.0 + pos[0]*pos[0]/axis_SqA + pos[1]*pos[1]/axis_SqB+pos[2]*pos[2]/axis_SqC;
    radical =  bCoef*bCoef - 4.0*aCoef*cCoef;

    if ( (aCoef <= 0.0) || (radical < 0.0) )
    {
	returnStatus = PGSCSC_W_MISS_EARTH;
	return (returnStatus);
    }
    else
    {
	slantRange[0] = (-bCoef - sqrt(radical))/(2.0*aCoef);
	slantRange[1] = (-bCoef + sqrt(radical))/(2.0*aCoef);
    }

    /*=============== compute look point vector and mirror vector (ecr): */

    for(i=0; i < 2;i++)
    {
       xLook[i][0] = pos[0] + slantRange[i] * pixUv[0];
       xLook[i][1] = pos[1] + slantRange[i] * pixUv[1];
       xLook[i][2] = pos[2] + slantRange[i] * pixUv[2];
    }
    
    return (returnStatus);
}


