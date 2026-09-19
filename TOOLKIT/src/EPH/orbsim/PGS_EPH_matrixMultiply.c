/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*
C***  SUBR: MATRIX_MULTIPLY VERSION:      PROJECT:  UARS   SUBSYS: OASIM
C*
C*    LANGUAGE:  FORTRAN77
C*
C*    PURPOSE:  This routine multiplies a (3 x 3) matrix by 
C*              a (3 x 3) matrix to form a (3 x 3) matrix.
C*
C*    CALLING SEQUENCE:  CALL MATRIX_MULTIPLY(AMAT, BMAT, CMAT )
C*
C*        NAME       I/O         TYPE               DESCRIPTION
C*        ----       ---         ----               -----------
C*        AMAT        I           R       Input matrix 1
C*        BMAT        I           R       Input matrix 1
C*        CMAT        O           R       Output product matrix
C*
C*    GLOBAL AREAS:  None
C*
C*    EXTERNAL FILES USED:  None
C*
C*    ERROR HANDLING:  None
C*
C*    ROUTINES CALLED:  None
C*
C*    CALLED BY:  Any
C*
C*    PDL:
C*----------------------------------------------------------------------
C*    BEGIN matrix_multiply
C*
C*    DO FOR each row component in matrix_1
C*    DO FOR each column component in matrix_2
C*           temp = 0.0
C*           DO FOR each column component in matrix_1
C*              temp = temp + 
C*                     matrix_1(loop-1-index, loop-3-index)*
C*                     matrix_2(loop-3-index, loop-2-index)
C*           END DO
C*    output_matrix(loop-1-index,loop-2-index) = temp
C*    END DO  {loops 1 and 2}
C*
C*    return
C*
C*    END matrix_multiply
C*----------------------------------------------------------------------
C*
C*    PROGRAMMER:  D. Marinelli, (pirated from SLP_MTXMPY) 563.1
*/

#include <PGS_SIM.h>

void      
PGS_EPH_matrixMultiply(
    PGSt_double matrixA[3][3],
    PGSt_double matrixB[3][3],
    PGSt_double matrixC[3][3])
{
    short ira;
    short icb;
    short ica;

    PGSt_double temp;
          
    /* Multiply matrix A by matrix B to get matrix C
       i.e. [C]=[A][B] */

    for(ira=0;ira<3;ira++)
      for(icb=0;icb<3;icb++)
      {
          temp = 0.0;
            
          for(ica=0;ica<3;ica++)
            temp=temp+matrixA[ira][ica]*matrixB[ica][icb];
          matrixC[ira][icb]=temp;
      }
}
