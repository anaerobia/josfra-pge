/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_getQuats.c

DESCRIPTION:
   This file contains the function PGS_CSC_getQuats().
   This function converts transformation matrix to quaternion

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Anubha Singhal     / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
   14-Oct-1994 AS/GTSK Initial version
   12-Jul-1995 PDN used temporary variable "fac" to streamline code

END_FILE_PROLOG:
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

extern PGSt_integer PGS_SMF_FEQ(PGSt_double, PGSt_double);

PGSt_SMF_status
PGS_CSC_getQuats(            /* converts transformation matrix to quaternion */
    PGSt_double trans[3][3], /* transformation matrix (input) */
    PGSt_double quatern[4])  /* quaternion (output) */
{
    /* This subroutine converts a 3x3 transformation matrix */
    /* to an appropriate quaternion. */

    PGSt_double p12;
    PGSt_double p22;
    PGSt_double p32;
    PGSt_double p42;
    PGSt_double pmax2;
    PGSt_double p; 
    PGSt_double t;
    PGSt_double fac;
    
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
    fac = 1.0/(p*2.0);
    
    if (PGS_SMF_FEQ(pmax2, p12))    
    {
	quatern[0] = p/2.0;
	quatern[1] = (trans[2][1]-trans[1][2])*fac;
	quatern[2] = (trans[0][2]-trans[2][0])*fac;
	quatern[3] = (trans[1][0]-trans[0][1])*fac;
    }
    else if (PGS_SMF_FEQ(pmax2, p22))
    {
	quatern[1] = p/2.0;
	quatern[0] = (trans[2][1]-trans[1][2])*fac;
	quatern[3] = (trans[0][2]+trans[2][0])*fac;
	quatern[2] = (trans[1][0]+trans[0][1])*fac;
    }
    else if (PGS_SMF_FEQ(pmax2, p32))
    {
	quatern[2] = p/2.0;
	quatern[3] = (trans[2][1]+trans[1][2])*fac;
	quatern[0] = (trans[0][2]-trans[2][0])*fac;
	quatern[1] = (trans[1][0]+trans[0][1])*fac;
    }
    else if (PGS_SMF_FEQ(pmax2, p42))
    {
	quatern[3] = p/2.0;
	quatern[2] = (trans[2][1]+trans[1][2])*fac;
	quatern[1] = (trans[0][2]+trans[2][0])*fac;
	quatern[0] = (trans[1][0]-trans[0][1])*fac;
    }
    else
      returnStatus = PGS_E_TOOLKIT;
    
    return returnStatus;
}
