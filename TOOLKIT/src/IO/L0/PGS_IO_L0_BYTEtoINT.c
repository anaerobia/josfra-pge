/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include <PGS_IO.h>

PGSt_uinteger
PGS_IO_L0_BYTEtoINT(            /* convert "byte" stream to integer */
    void           *bitstream,  /* bit stream */
    PGSt_uinteger  length)      /* number of significant bits */
{		  
    unsigned char  *bitPtr;     /* pointer to bit stream members */
		  
    PGSt_uinteger  number=0;    /* whole nuber equivalent of bit stream */
    
    /* Set a pointer to the first useful BYTE in the input bit stream.  This is
       being done so that we can increment (not possible with the void* type
       passed in. */
    
    bitPtr = ((unsigned char*) bitstream);

    /* calculate integer (whole number) equivalent of input bit stream */

    while (length--)
	number = number*256 + *(bitPtr++);
    
    return number;
}

