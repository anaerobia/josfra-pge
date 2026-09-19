/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_Norm.c

DESCRIPTION:
   This file contains the function PGS_CSC_Norm()
   This tool computes the norm of a 3-vector.

AUTHOR:
   Anubha Singhal / Applied Research Corporation

HISTORY:
   22-Sep-1994  AS  Initial version

END_FILE_PROLOG:
******************************************************************************/

/******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Compute The Norm Of A 3-Vector

NAME:     
   PGS_CSC_Norm()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_double
   PGS_CSC_Norm(
       PGSt_double  inVec[3])

DESCRIPTION:
   This tool computes the norm of a 3-vector.

INPUTS:
   Name       Description                Units    Min           Max
   ----       -----------                -----    ---           ---
   inVec      input vector               N/A      N/A           N/A

OUTPUTS:
   None

RETURNS:
   norm of the input vector (inVec)
  
EXAMPLES:
   None

NOTES:
   None

REQUIREMENTS:
   Derived

DETAILS:
   None

GLOBALS:  
   None

FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

PGSt_double
PGS_CSC_Norm(                        /* compute the norm of a 3-vector */
    PGSt_double  *inVec)             /* input vector */
    
{
    PGSt_double  norm;               /* norm of the vector */
    
    /* compute the norm */

    norm = sqrt(inVec[0] * inVec[0] + inVec[1] * inVec[1] + 
                inVec[2] * inVec[2]);
    
    return norm;                   				       
}
