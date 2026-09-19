/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
  	PGS_AA_2DReadGrid.c
 
DESCRIPTION:
         Contains PGS_AA_2DReadGrid.c

AUTHOR: 
  	Graham J Bland / EOSL

HISTORY:   
  	07-July-94 	GJB 	Initial version
  	17-August-95    GJB     Update for coding standards
	31-Aug-94       GJB     Post reivew updates to code
	30-Sep-94       GJB     Change to long output to cope with DEC
	11-Oct-94       GJB     Add checks for Start + Dim for DR correction
	28-Jan-95       GJB     Change interblock check to account for 
	                        extreme condition
        06-July-99      SZ      Updated for the thread-safe functionality
  
END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <stdio.h>  
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h> 

#undef DEFINE_DATA
#include <freeform.h>
#include <databin.h>
#include <PGS_AA_10.h>
#include <PGS_AA_Tools.h>
#include <PGS_TSF.h>

/*----   mnemonics -----*/

/*----   global variables  ----*/

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: 
Read data from grid
  	
 
NAME:
  	PGS_AA_2DReadGrid

SYNOPSIS:
  	N/A

DESCRIPTION:
 Accepts file structure locations as a means of accessing single points or 
 areas of 2 dimensional data sets. This module calculates the position of 
 the data requested in <file_structure_location> in terms of byte position 
 in the parmBuffer. In the case of multiple parameters in one parmBuffer 
 holds all parameters interleaved so the correct position is found by using 
 the aggregate record length. The order of extraction is the same as that 
 built up in outputFormat.  The y and x dimensions are used assuming that 
 the user has input in the order understood from data set documentation.

 [start]
 DO	check start points are not out of range, report any error and exit
 CALCULATE	sum noOfBytes per block for all parameters from 		
 			cacheFormatBytes for each
 CALCULATE	noOfBlocks from (xDim * yDim)
 CALCULATE	startPoint in parmBuffer from
 			((xCells * (yStart -1)) + (xStart - 1 )
 CALCULATE 	interBlock from (xCells - xDim) * noOfBytes 	
 IF	startPoint > totalParmMemoryCache OR
 	 interBlock  < 1		OR
 	(startPoint * noOfBytes) +  (noOfBlocks * noOfBytes) + ((noOfBlocks -1) * 
         interBlock ) ) >  totalParmMemoryCache 	THEN
 	PERFORM	PGS_SMF_SetStaticMsg "PGS_AA_W_POSITION_CALC_FAILURE"
 	DO return to calling module with "PGS_AA_W_2D_GRID_READ_ERROR"
 ENDIF
 DO	examine data Type and use appropriate buffer casting
 LOOP	yDim  as slower changing dimension
 	LOOP	xDim as fastest changing dimension
 		DO	extract noOfBlocks from parmBuffer into results 
 	ENDLOOP
 ENDLOOP
 [end]
  

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        parms           parameter names        	see notes
                        requested
        nParms          number of parms         none    1       #defined
        xStart          the x start point               1       variable   
        yStart          the y start point       none    1       variable   
        xDim            the x dimension         none    1       variable   
        yDim            the y dimension         none    1       variable      
    totalParmMemoryCache bytes required for     none    1       variable
                        parameters
        parmBuffer       buffer containing     	see notes   
                        requested parameters
           
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        results         results                 variable see notes
	 

RETURNS:
	PGSAA_E_OUTOFRANGE		(user + log)
 	PGSAA_E_EXTRACTORESULTS		(log)
 	PGSAA_E_TWODGRID_READ_ERROR	(user )
 	PGSAA_E_POSITION_CALC_FAILURE	(user + log)
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
	N/A

NOTES:
	The upper limit of the range of input variables is data set specific.
	The parms input variable is a parameter and data set specific 
        set of strings. The parmBuffer input is a memory buffer holding 
	whatever data is extracted form the data set requested by the user.  
	The results buffer is similar although holds the final output sent 
	back to the user.  It can hold data of 4 types (long, short, float,double).

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	from PGS_AA_Globals.h

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt() 	
        PGS_SMF_TestErrorLevel()        

END_PROLOG:
***************************************************************************/

PGSt_SMF_status PGS_AA_2DReadGrid(  
			     char *parms[], 
			     PGSt_integer nParms,
			     PGSt_integer xStart,
			     PGSt_integer yStart,
			     PGSt_integer xDim,
			     PGSt_integer yDim, 
			     char *parmBuffer,
			     PGSt_integer totalParmMemoryCache,
			     void *results)
			   
{
    PGSt_integer noOfBytes = 0;       /* number of bytes in the block of parameters being extracted */
    PGSt_integer noOfBlocks = 0;      /* number of blocks to be extracted */
    PGSt_integer interBlock;          /* number of bytes between blocks */
    PGSt_integer startPoint;          /* offset from start of parmBuffer in parameters */
    PGSt_integer parmCount;           /* number of parameters being extracted */
    PGSt_integer yCount;              /* count for y dimension */
    PGSt_integer xCount;              /* count for x dimension */
    PGSt_integer count;               /* count for the output results */
    PGSt_real *floatPointer;         /* temporary pointer to start of requested data in parmBuffer */
    PGSt_double *doublePointer;       /* temporary pointer to start of requested data in parmBuffer */
    long *longPointer;        /* temporary pointer to start of requested data in parmBuffer */
    short *shortPointer;              /* temporary pointer to start of requested data in parmBuffer */
    PGSt_real *tempFloatResult;      /* temporary type specific pointer/array used to fill results memory */
    PGSt_double *tempDoubleResult;    /* temporary type specific pointer/array used to fill results memory */
    short *tempShortResult;           /* temporary type specific pointer/array used to fill results memory */
    long *tempLongResult;     /* temporary type specific pointer/array used to fill results memory */

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retLock;      /* lock and unlock return */
#endif

/* first check start points are not out of range */
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);  
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    if ( xStart > xCells || yStart > yCells 
	|| xStart < 1 || yStart < 1 
	|| xDim > xCells || yDim > yCells 
	|| xDim < 1 || yDim < 1 
	|| (xDim + xStart - 1) > xCells || (yDim + yStart - 1) > yCells )
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_OUTOFRANGE, 
				       "PGSAA_TWODReadGrid");

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
	return PGSAA_E_OUTOFRANGE;
    }
    
    /* calculate total no of bytes in each block being extracted */

    for (parmCount = 0; parmCount < nParms; parmCount++)    
    {
	noOfBytes = noOfBytes + cacheFormatBytes[parmCount];
    }
    
    /* no of blocks to be extracted - assumes rectilinear gridding */
    noOfBlocks = xDim * yDim;
    
    /* calculate start point in buffer blocks */
    startPoint = ((xCells * (yStart - 1)) + (xStart - 1));
		  
    /* calculate interblock set in bytes */
    interBlock = (xCells - xDim) * noOfBytes;
    
    /* check these calcs */
    if ( interBlock < 0 || startPoint < 0 ||
	((startPoint * noOfBytes) + (noOfBlocks * noOfBytes) 
	 + ((yDim - 1) * interBlock)) > totalParmMemoryCache)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_POSITION_CALC_FAILURE, 
				       "PGSAA_TWODReadGrid");

#ifdef _PGS_THREADSAFE
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
		return PGSAA_E_POSITION_CALC_FAILURE;
    }
    
    
    /* first check out the type of returned values, then fill results */
    
    if ( strcmp ("float",dataType[0]) == 0)
    {
	tempFloatResult = (float*)results;
 
	/* find start float number */
	floatPointer = (float *)parmBuffer + (startPoint * nParms);
	
	/* loop for x blocks */
	for ( yCount = 0; yCount < yDim; yCount++)
	{
	    /* loop for contiguous cells */
	    for(xCount = 0; xCount < (xDim * nParms); xCount++)
	    {
		/* counter for results buffer */
		count = (yCount * xDim * nParms) + (xCount);
		
		tempFloatResult[count] = *(floatPointer + xCount + 
					   (yCount * xCells * nParms));
	    }
	}
    } 
    else if (strcmp ("double",dataType[0]) == 0)
    {
	tempDoubleResult = (PGSt_double*)results;
	
	/* find start  number */
	doublePointer = (PGSt_double *)parmBuffer + (startPoint * nParms);
	
	/* loop for x blocks */
	for ( yCount = 0; yCount < yDim; yCount++)
	{
	    tempDoubleResult = (PGSt_double*)results;
 	    /* loop for contiguous cells */
	    for(xCount = 0; xCount < (xDim * nParms); xCount++)
	    {
		/* counter for results buffer */
		count = (yCount * xDim * nParms) + (xCount);
		
		tempDoubleResult[count] = *(doublePointer + xCount + 
					    (yCount * xCells * nParms));
		
	    }   
	}
    }
    else if (strcmp ("short",dataType[0]) == 0)
    {
	tempShortResult = (short*)results;

	/* find start number */
	shortPointer = (short *)parmBuffer + (startPoint * nParms);
		
	/* loop for x blocks */
	for ( yCount = 0; yCount < yDim; yCount++)
	{
	    /* loop for contiguous cells */
	    for(xCount = 0; xCount < (xDim * nParms); xCount++)
	    {
		/* counter for results buffer */
		count = (yCount * xDim * nParms) + (xCount);
		
		tempShortResult[count] = *(shortPointer + xCount + 
					   (yCount * xCells * nParms));

		
	    }    
	}
    }	  
    else if (strcmp ("long",dataType[0]) == 0) 
    {
	tempLongResult = (long*)results;
	
	/* find start number */
	longPointer = (long*)parmBuffer + (startPoint * nParms);
	
	/* loop for x blocks */
	for ( yCount = 0; yCount < yDim; yCount++)
	{
	    /* loop for contiguous cells */
	    for(xCount = 0; xCount < (xDim * nParms); xCount++)
	    {
		/* counter for results buffer */
		count = (yCount * xDim * nParms) + (xCount);	  
		
		tempLongResult[count] = *(longPointer + xCount + (yCount * xCells * nParms));
		
	    }   
	}
    }
    /* handle error possibilities */

    else
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_EXTRACTORESULTSERROR, 
				       "PGSAA_TWODReadGrid");	

#ifdef _PGS_THREADSAFE
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
	return PGSAA_E_TWOD_READ_ERROR;
    }
  
    /* end */

#ifdef _PGS_THREADSAFE
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
    return PGS_S_SUCCESS;
}
         
