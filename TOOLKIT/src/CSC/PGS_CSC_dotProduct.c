/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include <PGS_CSC.h>

PGSt_double
PGS_CSC_dotProduct(                /* calculate dot product of two vectors */
    PGSt_double  vectorA[],        /* vector A (input) */
    PGSt_double  vectorB[],        /* vector B (input) */
    PGSt_integer dimension)        /* dimension of vectors A and B */
{
    PGSt_double  dotProductAB=0.0; /* dot product of vectors A and B */
    PGSt_integer cnt;              /* loop counter */
    
    for (cnt=0;cnt<dimension;cnt++)
      dotProductAB += vectorA[cnt]*vectorB[cnt];
    
    return dotProductAB;
}
