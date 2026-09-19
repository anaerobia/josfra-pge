/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_quatMultiply.c

DESCRIPTION:
   This file contains the function PGS_CSC_quatMultiply().
   This function multiplies two quaternions, using a short
   algorithm with 11 multiplications and 19 additions.

AUTHOR:
   Peter D. Noerdlinger/ Applied Research Corporation

HISTORY:
   14-Oct-1994 PDN Initial version
   11-Apr-1995 PDN Fixed comments to better identify arguments

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************

BEGIN_PROLOG:

TITLE:
   Multiply Quaternions
 
NAME:
   PGS_CSC_quatMultiply()

DESCRIPTION:
   This file contains the function PGS_CSC_quatMultiply().
   This function multiplies two quaternions, using a short
   algorithm with 11 multiplications and 19 additions. This


SYNOPSIS:
   #include <PGS_CSC.h>

   PGSt_SMF_status
   PGS_CSC_quatMultiply(  
      PGSt_double  quatP[4],
      PGSt_double  quat[4], 
      PGSt_double  quatout[4])  

INPUTS:
   Name       Description               Units     Min     Max
   ----       -----------               -----     ---     ---
   quatP      quaternion defining       N/A       N/A     N/A
              the second rotation

   quat       quaternion defining       N/A       N/A     N/A
              the first rotation
   
OUTPUTS:
   Name       Description               Units     Min     Max
   ----       -----------               -----     ---     ---

   quatout    quaternion defining       N/A       N/A     N/A
              the resulting rotation

RETURNS:
   PGS_S_SUCCESS
   PGSCSC_E_BAD_QUATERNION

EXAMPLE:
   None

NOTES:
   The algorithm used by this function requires that the first component of
   the input quaternion is the scalar component of the quaternion.
   The method is presented by V. N. Dvornychenko, in "Journal of Guidance
   Control and Dynamics, Vol. 8, pp. 157-159 (Jan-Feb 1985).  Additional
   grouping of terms within  Dvornychenko's expressions is required to implement
   the code with the stated number of operations (11 multiplications and 19 
   additions.)  For comparison, direct quaternion multiplication takes 16 
   multiplications and 12 additions and matrix multiplication takes 27 
   multiplications and 18 additions.

REQUIREMENTS:
   Derived

DETAILS:
   The algorithm does the following multiplication:

     q" = q' * q  

     where:
	q   = quaternion for first rotation (quat)
	q'  = quaternion for second rotation (quatP) 
	q"  = product quaternion 
	*   = denotes quaternion multiplication

     The order of the arguments is (q',q) because of the way that quaternions
     are applied to a vector,  symbolically as a left-operator;  thus
     (q' * q) acting on a vector V has the effect of q first, then
     q'; also under multiplication with  ixj = k, jxk=i, kxi=j, this
     agrees, when the unit skew field elements are  (1,i,j,k).

      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
*******************************************************************************/

#include <PGS_CSC.h>

PGSt_SMF_status
PGS_CSC_quatMultiply(            /* Multiply 2 quaternions  */
    PGSt_double  quatP[4],       /* first input quaternion */
    PGSt_double  quat[4],        /* second input quaternion */
    PGSt_double  quatout[4])     /* output quaternion */
{
    PGSt_double  sumSquares;   /* sum of squares of quat components */

    PGSt_double  q13;          /* q1 + q3 */
    PGSt_double  qp13;         /* q'1 - q'3 */
    PGSt_double  q24;          /* q2 + q4 */
    PGSt_double  qp24;         /* q'2 + q'4 */
    PGSt_double  u1;           /* q13(qp13+qp24) */
    PGSt_double  u2;           /* (q13+q24)(qp24) */
    PGSt_double  u3;           /* (q24-q13)(qp13) */
    PGSt_double  v1;           /* the v's are simple products-
                                  see Dvornychenko */
    PGSt_double  v2;   
    PGSt_double  v3;  
    PGSt_double  v4; 
    PGSt_double  v5;
    PGSt_double  v6; 
    PGSt_double  v7; 
    PGSt_double  v8; 
    PGSt_double  v23;          /* v2 - v3 */
    PGSt_double  v14;          /* v1 + v4 */
    PGSt_double  v58;          /* v5 + v8 */
    PGSt_double  v67;          /* v6 - v7 */


    /* Make sure the input quaternions are indeed quaternions.  The sum of the
       squares of the components of a quaternion should equal one (1). */

    sumSquares = quatP[0]*quatP[0] + quatP[1]*quatP[1] + 
                 quatP[2]*quatP[2] + quatP[3]*quatP[3] ; 

    if (sumSquares < PGSd_LOWER_LIMIT || sumSquares > PGSd_UPPER_LIMIT)
    {
        PGS_SMF_SetDynamicMsg(PGSCSC_E_BAD_QUATERNION,"first argument",
                              "PGS_CSC_quatMultiply()");
        return PGSCSC_E_BAD_QUATERNION;
    }
    
    sumSquares = quat[0]*quat[0] + quat[1]*quat[1] + 
                 quat[2]*quat[2] + quat[3]*quat[3] ; 

    if (sumSquares < PGSd_LOWER_LIMIT || sumSquares > PGSd_UPPER_LIMIT)
    {
        PGS_SMF_SetDynamicMsg(PGSCSC_E_BAD_QUATERNION,"second argument",
                              "PGS_CSC_quatMultiply()");
        return PGSCSC_E_BAD_QUATERNION;
    }
    
    v1 = quatP[0] * quat[3];
    v2 = quatP[1] * quat[3];
    v3 = quatP[2] * quat[0];
    v4 = quatP[3] * quat[0];
    v5 = quatP[0] * quat[2];
    v6 = quatP[1] * quat[2];
    v7 = quatP[2] * quat[1];
    v8 = quatP[3] * quat[1];

    q13  = quatP[0] + quatP[2];
    qp13 = quat[0]  - quat[2];
    qp24 = quat[1]  + quat[3];
    q24  = quatP[1] + quatP[3];

    u1 = q13 * (qp13 + qp24);
    u2 = (q13 + q24) * qp24;   
    u3 = (q24 - q13) * qp13; 

    v23 = v2 - v3;
    v14 = v1 + v4;
    v58 = v5 + v8;
    v67 = v6 - v7;

    quatout[3] = v14 + v67;
    quatout[0] = u1 - u2 + v23 + v58;
    quatout[1] = u1 + u3 - v14 + v67;
    quatout[2] = v58 - v23;

    PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_CSC_quatMultiply()");
    return PGS_S_SUCCESS;
}
