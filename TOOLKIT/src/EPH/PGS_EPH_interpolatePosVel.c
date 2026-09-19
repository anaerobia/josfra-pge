/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_EPH_interpolatePosVel.c

DESCRIPTION:
   This file contains the function PGS_EPH_interpolatePosVel()

AUTHOR:
  Guru Tej S. Khalsa

HISTORY:
  17-Oct-1994  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   PGS_EPH_interpolatePosVel

NAME:
   PGS_EPH_interpolatePosVel()

SYNOPSIS:
C:

#include <PGS_EPH.h>

PGSt_SMF_status
PGS_EPH_interpolatePosVel(
    PGSt_double     beginTAI93,
    PGSt_double     beginPosition[3],
    PGSt_double     beginVelocity[3],
    PGSt_double     endTAI93,
    PGSt_double     endPosition[3],
    PGSt_double     endVelocity[3],
    PGSt_double     secTAI93,
    PGSt_double     position[3],
    PGSt_double     velocity[3])


DESCRIPTION:
   

INPUTS:
   Name           Description                 Units       Min   Max
   ----           -----------                 -----       ---   ---
   beginTAI93     lower time boundary         time
   beginPosition  position at lower time      length
   beginVelocity  velocity at lower time      length/time
   endTAI93	  upper time boundary         time
   endPosition	  position at upper time      length
   endVelocity	  velocity at upper time      length/time

OUTPUTS:
   Name           Description                 Units       Min   Max
   ----           -----------                 -----       ---   ---
   secTAI93       requested time
   position       position at requested time
   velocity       velocity at requested time

          
RETURNS:
   PGS_S_SUCCESS
   PGSEPH_E_BAD_TIME_ORDER

EXAMPLES:
   None
  
NOTES:
   None

REQUIREMENTS:
   PGSTK - 0720

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_EPH.h>

/* name of this function */

#define FUNCTION_NAME "PGS_EPH_interpolatePosVel()"

PGSt_SMF_status
PGS_EPH_interpolatePosVel(            /* interpolate position and velocity */
    PGSt_double     beginTAI93,       /* lower time boundary */    
    PGSt_double     beginPosition[3], /* position at lower time */ 
    PGSt_double     beginVelocity[3], /* velocity at lower time */ 
    PGSt_double     endTAI93,         /* upper time boundary */    
    PGSt_double     endPosition[3],   /* position at upper time */ 
    PGSt_double     endVelocity[3],   /* velocity at upper time */ 
    PGSt_double     secTAI93,         /* requested time */         
    PGSt_double     position[3],      /* position at requested time */
    PGSt_double     velocity[3])      /* velocity at requested time */
{
    PGSt_double     cp[3][4];         /* interpolation coefficients */
    PGSt_double     deltaTAI;         /* upper and lower time difference */
    PGSt_double     dt;               /* requested and lower time difference */
    
    PGSt_integer    j;                /* loop counter */

    /* Make sure that beginTAI93 is not greater than endTAI93 and that secTAI93
       is contained in the set [beginTAI93,endTAI93].  If these conditions are
       not met, return an error. */

    if (beginTAI93 > endTAI93 || secTAI93 < beginTAI93 || secTAI93 > endTAI93)
    {
	PGS_SMF_SetStaticMsg(PGSEPH_E_BAD_TIME_ORDER,FUNCTION_NAME);
	return PGSEPH_E_BAD_TIME_ORDER;
    }
    
    /* Calculate the time interval between the end points. */

    deltaTAI = endTAI93 - beginTAI93;
    
    /* If all times are equal (they should be if the above test was false and
       the following one is true) set the position and velocity to the beginning
       values and return.  This must be done to avoid dividing by 0.0 later. */

    if (fabs(deltaTAI) < 1.e-10)
    {
	for (j=0;j<3;j++)
	{
	    position[j] = beginPosition[j];
	    velocity[j] = beginVelocity[j];
	}
	return PGS_S_SUCCESS;
    }
    
    /* Determine the interpolation coefficients. */

    for (j=0;j<3;j++)
    {
	cp[j][0] = beginPosition[j];
	cp[j][1] = deltaTAI*beginVelocity[j];
	cp[j][2] = 3.0*( endPosition[j] - beginPosition[j]) -
	           deltaTAI*(endVelocity[j] + 2.0*beginVelocity[j]);
	cp[j][3] = 2.0*(-endPosition[j] + beginPosition[j]) +
	           deltaTAI*(endVelocity[j] +  beginVelocity[j]);
    }

    /* Interpolate to the desired time (secTAI93). */

    dt = (secTAI93 - beginTAI93)/deltaTAI;
    for (j=0;j<3;j++)
    {
        position[j]=dt*(dt*(dt*cp[j][3] + cp[j][2]) + cp[j][1]) + cp[j][0];
        velocity[j]=(dt*(dt*3.0*cp[j][3] + 2.0*cp[j][2]) + cp[j][1])/deltaTAI;
    }
    return PGS_S_SUCCESS;
}









