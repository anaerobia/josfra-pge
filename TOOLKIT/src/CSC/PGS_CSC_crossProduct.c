/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include <PGS_CSC.h>

PGSt_double *
PGS_CSC_crossProduct(             /* calculate cross product of two 3-vectors */
    PGSt_double  vectorA[],       /* 3-vector A (input) */
    PGSt_double  vectorB[],       /* 3-vector B (input) */
    PGSt_double  crossProductAB[])/* cross product of vectors A and B (A X B) */
{
    /* in lieu of a keen algorithm... */

    crossProductAB[0] = vectorA[1]*vectorB[2] - vectorA[2]*vectorB[1];
    crossProductAB[1] = vectorA[2]*vectorB[0] - vectorA[0]*vectorB[2];
    crossProductAB[2] = vectorA[0]*vectorB[1] - vectorA[1]*vectorB[0];
    
    return crossProductAB;
}
