/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_SIM.h>

PGSt_SMF_status
PGS_EPH_getQuats(            /* converts transformation matrix to quaternion */
    PGSt_double trans[3][3], /* transformation matrix (input) */
    PGSt_double quatern[4])  /* quaternion (output) */
{
    /* This subrountine converts a 3x3 transformation matrix */
    /* to an appropriate quaternian. */

    PGSt_double p12;
    PGSt_double p22;
    PGSt_double p32;
    PGSt_double p42;
    PGSt_double pmax2;
    PGSt_double p; 
    PGSt_double t;
    
    PGSt_SMF_status returnStatus;

    returnStatus = PGS_S_SUCCESS;
    pmax2 = -10;
    
    t = trans[0][0] + trans[1][1] + trans[2][2];
    p12 = 1.0 + t;
    p22 = 1.0 + trans[0][0] - trans[1][1] - trans[2][2];
    p32 = 1.0 + trans[1][1] - trans[0][0] - trans[2][2];
    p42 = 1.0 + trans[2][2] - trans[0][0] - trans[1][1];

    pmax2 = (p12 > p22) ? p12 : p22;
    pmax2 = (pmax2 > p32) ? pmax2 : p32;
    pmax2 = (pmax2 > p42) ? pmax2 : p42;

    p = sqrt(pmax2);
    
    if (pmax2 == p12)
    {
	quatern[0] = p/2.0;
	quatern[1] = (trans[2][1]-trans[1][2])/p/2.0;
	quatern[2] = (trans[0][2]-trans[2][0])/p/2.0;
	quatern[3] = (trans[1][0]-trans[0][1])/p/2.0;
    }
    else if (pmax2 == p22)
    {
	quatern[1] = p/2.0;
	quatern[0] = (trans[2][1]-trans[1][2])/p/2.0;
	quatern[3] = (trans[0][2]+trans[2][0])/p/2.0;
	quatern[2] = (trans[1][0]+trans[0][1])/p/2.0;
    }
    else if (pmax2 == p32)
    {
	quatern[2] = p/2.0;
	quatern[3] = (trans[2][1]+trans[1][2])/p/2.0;
	quatern[0] = (trans[0][2]-trans[2][0])/p/2.0;
	quatern[1] = (trans[1][0]+trans[0][1])/p/2.0;
    }
    else if (pmax2 == p42)
    {
	quatern[3] = p/2.0;
	quatern[2] = (trans[2][1]+trans[1][2])/p/2.0;
	quatern[1] = (trans[0][2]+trans[2][0])/p/2.0;
	quatern[0] = (trans[1][0]-trans[0][1])/p/2.0;
    }
    else
      returnStatus = PGS_E_TOOLKIT;
    
    return returnStatus;
}
