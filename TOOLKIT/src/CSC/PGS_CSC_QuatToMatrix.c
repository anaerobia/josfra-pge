/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_QuatToMatrix.c

DESCRIPTION:
   This file contains the function PGS_CSC_QuatToMatrix()
   This functionconverts a quaternion to a rotation matrix.
   
AUTHOR:
   Peter D. Noerdlinger / Space Applications Corporation

HISTORY:
   1-Sep-1997  PDN  Initial Version

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE: 
   Obtains Rotation Matrix from a quaternion

NAME:   
   PGS_CSC_QuatToMatrix()

SYNOPSIS:
 C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_QuatToMatrix( 
       PGSt_double  quaternion[4],            
       PGSt_double  rotation[3][3])   
	      
 FORTRAN:
      include 'PGS_CSC_4.f'
      include 'PGS_SMF.f'

      integer function pgs_csc_quattomatrix(quaternion,rotation)
      double precision quaternion(4)
      double precision rotation(3)(3)
    
DESCRIPTION:
   This function gets the rotation matrix from a quaternion.

INPUTS:
   Name              Description            Units    Min        Max
   ----              -----------            -----    ---        --- 
   quaternion        quaternion defining    N/A      N/A        N/A
                     the desired rotation                
                                                   
OUTPUTS:
   Name              Description            Units    Min        Max
   ----              -----------            ----     ---        ---
   rotation          3 x 3 matrix           N/A      -1          1 
                                                  
RETURNS:
   PGS_S_SUCCESS		 Success
   PGSCSC_E_BAD_QUATERNION       Input quaternion is invalid

NOTES:
   The rotation matrix  will map a column vector to a new one in the same way
   that a quaternion rotation would, e.g. in PGS_CEC_quatRotate()


   [1] "A Survey of Attitude Representations", by Malcolm D. Schuster, The
       Journal of the Astronautical Sciences vol 41, #4, pp. 439-517 (AIAA,
       Oct 1993)

REQUIREMENTS:
   PGSTK - 1050 

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   NONE

FILES:
   NONE

FUNCTIONS CALLED:
   PGS_SMF_SetStaticMsg()
      
END_PROLOG
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_QuatToMatrix()"

PGSt_SMF_status
PGS_CSC_QuatToMatrix(                       /* obtains the equivalent rotation
					      matrix from a quaternion */  
    PGSt_double         quat[],              /* quaternion for rotation */
    PGSt_double	        rotation[3][3])      /* rotation matrix */ 
{  
    short               count;             /* loop counter */
    short               next;              /* loop counter */

    PGSt_double         sumSquares;        /* sum of the squares of the
					      components of the quat */
    PGSt_SMF_status     returnStatus;      /* return status of function call */

    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS;
    
    /* Make sure the input quat is indeed a quat.  The sum of the
       squares of the components of a quat should equal one (1). */

    sumSquares = quat[0]*quat[0] + quat[1]*quat[1] +
                 quat[2]*quat[2] + quat[3]*quat[3];
    if (sumSquares < PGSd_LOWER_LIMIT || sumSquares > PGSd_UPPER_LIMIT)
    {
	returnStatus =  PGSCSC_E_BAD_QUATERNION;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	return returnStatus;
    }
    
    for(count = 0; count < 3; count++)
    {
       rotation[count][count] = quat[0] * quat[0] + quat[count+1] * quat[count+1];
       next = ( count + 1) % 3;
       rotation[count][count] -= quat[next+1] * quat[next+1];
       next = ( count + 2) % 3;
       rotation[count][count] -= quat[next+1] * quat[next+1];
    }

    rotation[0][1] = 2.0 * (quat[1] * quat[2] - quat[0] * quat[3]);
    rotation[0][2] = 2.0 * (quat[0] * quat[2] + quat[1] * quat[3]);
    rotation[1][0] = 2.0 * (quat[1] * quat[2] + quat[0] * quat[3]);
    rotation[1][2] = 2.0 * (quat[3] * quat[2] - quat[0] * quat[1]);
    rotation[2][0] = 2.0 * (quat[1] * quat[3] - quat[0] * quat[2]);
    rotation[2][1] = 2.0 * (quat[3] * quat[2] + quat[0] * quat[1]);
    
    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
       
    return returnStatus;
}
