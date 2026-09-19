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

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE: Get ECR Rectangular Coordinates of Pixel Center

NAME:  PGS_CSC_LookPoint()

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
        compute x directly.                           
 
INPUTS:

 Name             Description                  Units       Min        Max
 ----------------------------------------------------------------------------
 pos[3]         ECR position vector of SC         m     -10000000    +10000000
 pixUv[3]       look unit vector in ECR          N/A       -1           1
  					       
 axis_A         Earth semi-Major Axis             m      6,376,000    6,390,000
 axis_B         Earth semi-Major Axis             m      6,376,000    6,390,000
 axis_C         Earth semi-Minor Axis             m      6,355,000    6,380,000
 
  (In the PGS Toolkit axis_A and axis_B are assumed to be equal.)
  (large upper limits accommodate experiments like CERES which
   may inflate Earth axes to allow for height of atmosphere)
 
OUTPUTS:

 Name             Description                     Units       Min        Max
 ----------------------------------------------------------------------------
  slantRange   distance from SC to lookpoint         m      5000       100000000

  xLook[3]     ECR rectangular coords of lookpoint   m        0        10000000
 

GLOBALS:
    none

FILES:
    none

FUNCTIONS_CALLED:

    none

AUTHOR:

   Peter D. Noerdlinger
   Applied Research Corp
   April 10, 1994

RETURNS:

  Errors
  Returned                                          Description
  -----------------------------------------------------------------------------
 PGS_S_SUCCESS                                      knock on wood
 PGSCSC_W_MISS_EARTH                                boresight misses Earth
 PGSCSC_W_ZERO_PIXEL_VECTOR


 END_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_SMF.h>

PGSt_SMF_status
PGS_CSC_LookPoint(          /* solves the look point equation */
    PGSt_double  pos[3],         /* SC   position vector */  
    PGSt_double  pixUv[3],       /* look unit vector */    
    PGSt_double  axis_A,         /* ellipsoid x axis dimension */  
    PGSt_double  axis_B,         /* ellipsoid y axis dimension */  
    PGSt_double  axis_C,         /* ellipsoid z axis dimension */  
    PGSt_double* slantRange,     /* spacecraft to lookpoint */
    PGSt_double xLook[3])        /* solution vector */       
{
    PGSt_double aCoef;           /* coefficient of square of slant range in quadratic eq */
    PGSt_double bCoef;           /* coefficient of slant range in quadratic eq */
    PGSt_double cCoef;           /* constant term in quadratic eq for slant range */
    PGSt_double radical;         /* sqrt(b*b-4*a*c) in usual quadratic equation notation */
    PGSt_double axis_SqA;        /* square of axis_A */
    PGSt_double axis_SqB;        /* square of axis_B */
    PGSt_double axis_SqC;        /* square of axis_C */

    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;
    
    /**  Initializations  **/

    xLook[0] = PGSd_GEO_ERROR_VALUE;
    xLook[1] = PGSd_GEO_ERROR_VALUE;
    xLook[2] = PGSd_GEO_ERROR_VALUE;
    *slantRange = PGSd_GEO_ERROR_VALUE;

    /*========== define squares of axis lengths ==========*/
    
    axis_SqA = axis_A*axis_A;
    axis_SqB = axis_B*axis_B;
    axis_SqC = axis_C*axis_C;

    /*====== Check for Zero Pixel Vector   =========*/

    if ( fabs(pixUv[0]) + fabs(pixUv[1]) + fabs(pixUv[2])  < EPS_64 )
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
	*slantRange = (-bCoef - sqrt(radical))/(2.0*aCoef);
    }

    /*=============== compute look point vector (eci): */

    xLook[0] = pos[0] + *slantRange * pixUv[0];
    xLook[1] = pos[1] + *slantRange * pixUv[1];
    xLook[2] = pos[2] + *slantRange * pixUv[2];
    
    return (returnStatus);
}


