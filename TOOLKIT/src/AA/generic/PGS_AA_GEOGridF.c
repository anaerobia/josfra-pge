/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:  

FILENAME:   
  	PGS_AA_GEOGridF.c 
 
DESCRIPTION:
  	The interface to the calling algorithm which extracts gridded 
        data by geographic location.


AUTHOR:
  	Graham Bland / EOSL 

HISTORY:
  	07-July-94 	GJB 	Initial version
  	11-Aug -94 	GJB     Update for coding standards
	31-August-94    GJB     Post review updates
	03-October-94   GJB     Make F version
	25-January-95	SGT	Added "undef _CFE" as long term fix to SGI IRIX 5.2 compile problem
	15-Jun-95       ANS     Added routines for interpolation operations
	18-July-95	ANS	Fixed bug ECSed01007
        06-July-99      SZ      Updated for the thread-safe functionality
  
END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <stdio.h>  
#include <math.h>
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

TITLE: Calculate grid position of requested data from geographic coordinates
	
  
NAME:  
	PGS_AA_GEOGrid

SYNOPSIS:
  N/A


DESCRIPTION:
 This module calculates the position of the requested data from the input 
 arguments in terms of the structure of the data and then calls xDGRIDRead to 
 extract data values.  Various operations and autoOperations are checked to 
 establish the type of transforms a) requested by the user and b) necessary for 
 the requested data set.

 [start]
 PERFORM	PGS_AA_GetSuppGeo with <support> to get maxLat, minLat, 
 maxLon, minLon,  autoOperation plus others as neceesary to support 
 autoOperation.
 LOOP	FOR 	nPoints
	IF	autoOperation includes convert to Greenwich base	THEN
		PERFORM	PGS_AA_AOP_GreenwichStart
	ELSEIF	autoOperation includes convert to IDL base	THEN
		PERFORM	PGS_AA_AOP_IDLStart
	ENDIF
	IF	autoOperation includes platte carree conversion	THEN
		PERFORM	PGS_AA_AOP_PLATTECARRE
	ELSEIF	autoOperation includes polar stereographic conversion	THEN
		PERFORM	PGS_AA_AOP_W3FB06
	ELSEIF	no autoOperation is set	THEN
		PERFORM	PGS_SMF_SetStaticMsg "PGS_AA_AUTO_OPERATION_UNSET"
		DO return to calling module with "PGS_AA_W_GEO_ERROR"	
	ENDIF
	DO	set xDim to 1
	DO	set yDim to 1
	DO	set zDim to 1
	IF	operation includes NEAREST_CELL	THEN
		PERFORM	PGS_AA_OP_NEAREST_CELL
	ELSEIF	operation includes NINT_CELL	THEN
		PERFORM	PGS_AA_OP_NINTCELL
	ELSEIF  operation includes INTERP2BY2 THEN
                PERFROM PGS_AA_PeV_interger to obtain function index
                PERFORM user defined function
        ELSEIF  operation includes INTERP3BY3 THEN
                PERFROM PGS_AA_PeV_interger to obtain function index
                PERFORM user defined function
	ELSE
		PERFORM	PGS_SMF_SetStaticMsg 				
			"PGS_AA_W_GEO_OPERATION_UNSET"
		DO return to calling module with "PGS_AA_W_GEO_ERROR"
	ENDIF
	IF	PGS_AA_2Dgeo toolUsed	THEN
		PERFORM	PGS_AA_2DReadGrid
	ELSEIF	PGS_AA_3Dgeo toolUsed	THEN
		DO	zStart = height input by user
		PERFORM	PGS_AA_3DReadGrid	
	ENDIF
	DO	increment results buffer by data type
ENDLOOP
[end]
 
	
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        parms           parameter names        	see notes
                        requested
        nParms          number of parms         none    1       #defined
        latitude        latitude(s) of the      degrees  -90.00  90.00
                        requested point
        longitude       longitude(s) of the     degrees  -180.00 180.00
                        requested point
        height          height of the point     none    variable
        nPoints         no. of points requested none    1       variable
        operation       defines user required   none    1       variable  
        logSuppFile     support file i.d.       none    variable
        parmBuffer       buffer containing     	see notes 
                        requested parameters
   totalParmMemoryCache bytes required for      none    1       variable
                        parameters
        toolUsed        assigned to distinguish none    2       3
                        gridRead call
   
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	results         results                 see notes 
	
RETURNS:
	PGSAA_E_GEOERROR          		(user)
 	PGSAA_E_GETSUPP				(user)
 	PGSAA_E_AUTOOPERATION			(log)
 	PGSAA_E_AUTOOPERATIONUNSET	        (log)
 	PGSAA_E_OPERATION			(log)
 	PGSAA_E_OPERATIONUNSET		        (log)
        PGSTSF_E_GENERAL_FAILURE                problem in the thread-safe code

EXAMPLES:
        N/A

FORTRAN:
	N/A	

NOTES:
	The upper limit of the range of input variables is data set specific.
	The parms input variable is a parameter and data set specific 
        set of strings.  The results buffer is a memory buffer holding 
	whatever data is extracted form the data set requested by the user.  
	It can hold data of 4 types (long, short, float,double).

	Simple bilinear interpolation routine is provided. Two operations are
        described to distinguish between grid sizesi(2x2 or 2x3) for interpolation.
        The routine used is taken from Numerical recipes in C by William H. Press et al.
        1988. The routines are called polint and polint2 and are on pages 90 and 106
        respectively. If the user wishes to add more routines then he must update
        the *interpFunc()[] array in PGS_AA_Tools.h. To use any available function
        the user must update the supportfile and set funcIndex for the said function.

REQUIREMENTS:
	N/A

DETAILS:
	N/A

GLOBALS:
	as in PGS_AA_Global.h
	
FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_AA_GetSuppGeo
        PGS_AA_AOP_GreenwichStart
        PGS_AA_AOP_IDLStart
        PGS_AA_AOP_PLATTECARRE
        PGS_SMF_SetStaticMsg
        PGS_AA_OP_NearestCell
        PGS_AA_2DReadGrid
        PGS_AA_3DReadGrid
        PGS_AA_OP_NINTCELL
        PGS_TSF_LockIt
        PGS_TSF_UnlockIt
        PGS_SMF_TestErrorLevel
	user defined interpolation functions

END_PROLOG:
***************************************************************************/

PGSt_SMF_status PGS_AA_GEOGridF( 
			  char *parms[], 
			  PGSt_integer nParms,
			  PGSt_integer logSuppFile[],
			  PGSt_integer operation,
			  PGSt_double latitude[], 
			  PGSt_double longitude[],
			  PGSt_integer height[],
			  PGSt_integer nPoints, 
			  char *parmBuffer, 
			  short toolUsed,
			  PGSt_integer totalParmMemoryCache,		  
			  void *results)  
{
    short nLoop; 		
    PGSt_double xRealStart;     /* number of cells in x dimension as a floating point value */
    PGSt_double yRealStart;     /* number of cells in y dimension as a floating point value */
    PGSt_integer xStart;        /* number of cells in x dimension for start of requested area */
    PGSt_integer yStart;        /* number of cells in y dimension for start of requested area */
    PGSt_integer zStart;        /* number of cells in z dimension for start of requested area */  
    PGSt_integer xDim = 1;      /* number of cells in x dimension to extract from start (set 1 for all GEO calls) */
    PGSt_integer yDim = 1;      /* number of cells in y dimension to extract from start (set 1 for all GEO calls) */
    PGSt_integer zDim = 1;      /* number of cells in z dimension to extract from start (set 1 for all GEO calls) */
    PGSt_double  mlatitude;     /* modeified latitude value */
    PGSt_double  mlongitude;    /* modeified longitude value */
    PGSt_double  y2d[3][3];     /* grid values actually used in the interpolation */
    PGSt_double  yBuffer[10];   /* grid data point values for interpolation */
    PGSt_double  x1a[3];        /* first dimention of interpolation */
    PGSt_double  x2a[3];        /* second dimention of interpolation */
    PGSt_double  *yp[3];        /* grid row vectors */
    PGSt_double  dResults;      /* interpolation difference */
    PGSt_double  locResults;    /* actual results calculated by the user routine */
    PGSt_integer* intPtr;      /* integer pointer to manage user result buffer if type is short */
    long *       longPtr;       /* long pointer to manage user result buffer if type is short */
    float *      floatPtr;      /* float pointer to manage user result buffer if type is short */
    PGSt_double* doublePtr;     /* double pointer to manage user result buffer if type is short */
    PGSt_double  roundOff;      /* To correctly roundoff the result */
    PGSt_SMF_status val=PGSAA_E_GEOERROR;
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retLock;    /* lock and unlock return */
#endif
    /* first check nPoints is sensible */

    if ( nPoints < 1 )
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_NPOINTSINVALID,"PGS_AA_GEOGrid");
	return val;
    }
    
    /*   first get support data necessary for GEO access */

    val = PGS_AA_GetSuppGeo(logSuppFile);
    
    if (val != PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_GETSUPP,"PGS_AA_GetSuppGeo");
	return val;	    
    }
    
    /* now set up a loop to get each lat/lon from the structure */
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    for ( nLoop = 0; nLoop < nPoints; nLoop++)  
    {   
	/* check autOperations from the support file */

	if ( (autoOperation &  PGSd_AA_AOP_GREENWICHSTART) == PGSd_AA_AOP_GREENWICHSTART)
	{
	    val = PGS_AA_AOP_GreenwichStart(latitude[nLoop], longitude[nLoop], &mlatitude, &mlongitude);
	    if (val != PGS_S_SUCCESS)
	    {
		PGS_SMF_SetStaticMsg (PGSAA_E_AUTOOPERATION,"PGSd_AA_AOP_GREENWICHSTART");
		PGS_SMF_SetStaticMsg (PGSAA_E_GEOERROR,"PGS_AA_GEOGrid");

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GEOERROR;	    
	    }
	} 
	else if ((autoOperation & PGSd_AA_AOP_IDLSTART ) == PGSd_AA_AOP_IDLSTART)
	{
	    val = PGS_AA_AOP_IDLStart(latitude[nLoop], longitude[nLoop], &mlatitude, &mlongitude);
	    if (val != PGS_S_SUCCESS)
	    {
		PGS_SMF_SetStaticMsg (PGSAA_E_AUTOOPERATION,"PGSd_AA_AOP_IDLSTART");
		PGS_SMF_SetStaticMsg (PGSAA_E_GEOERROR,"PGS_AA_GEOGrid");

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GEOERROR;	    
	    } 
	}
	else
	{
	    mlatitude = latitude[nLoop];
	    mlongitude = longitude[nLoop];
	}    
	      
	/* check the second set of autoOperations to apply additional transforms */

	if ( (autoOperation & PGSd_AA_AOP_PLATTECARRE) == PGSd_AA_AOP_PLATTECARRE )
	{
	    val = PGS_AA_AOP_PLATTECARRE(mlatitude, mlongitude, 
					 &xRealStart, &yRealStart);
	    if (val != PGS_S_SUCCESS)
	    {
		PGS_SMF_SetStaticMsg (PGSAA_E_AUTOOPERATION,"PGS_AA_AOP_PLATTECARRE");
		PGS_SMF_SetStaticMsg (PGSAA_E_GEOERROR,"PGS_AA_GEOGrid");

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GEOERROR;	    
	    }
	}
	else if ((autoOperation & PGSd_AA_AOP_POLARSTEREO) == PGSd_AA_AOP_POLARSTEREO)
	{
	    
	    /* finish with this autoOperation - needs to be in this loop because 
	       of the fact that this subroutine does lat/long - start cells 
	       and includes the nearest_cell operation with it */
	    val = PGS_AA_AOP_W3FB06(mlatitude, mlongitude,
				    &xRealStart, &yRealStart);   
	    if(val != PGS_S_SUCCESS)
	    {
		PGS_SMF_SetStaticMsg (PGSAA_E_AUTOOPERATION,"PGS_AA_OP_w3fb06");
		PGS_SMF_SetStaticMsg (PGSAA_E_GEOERROR,"PGS_AA_GEOGrid");

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GEOERROR;       	    
	    }	
	}
	else 
	{
	    PGS_SMF_SetStaticMsg (PGSAA_E_AUTOOPERATIONUNSET, 
					   "PGS_AA_GEOGrid");
	    PGS_SMF_SetStaticMsg (PGSAA_E_GEOERROR,"PGS_AA_GEOGrid");

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

	    return PGSAA_E_GEOERROR;
	}
	
	
	/* check out user operations for additional changes to start cells */
	
	if ((operation & PGSd_AA_OP_NEARESTCELL) == PGSd_AA_OP_NEARESTCELL)
	{
	    val = PGS_AA_OP_NearestCell(xRealStart, yRealStart, &xStart, &yStart);
	    if(val != PGS_S_SUCCESS)
	    {
		PGS_SMF_SetStaticMsg (PGSAA_E_OPERATION,"PGS_AA_NEARESTCELL");  
		PGS_SMF_SetStaticMsg (PGSAA_E_GEOERROR,"PGS_AA_GEOGrid");
	  
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GEOERROR;	
	    }      

	    /* now branch for 2 and 3 dimensional extraction) */
	    if (toolUsed == TOOL2D)
            {
#ifdef _PGS_THREADSAFE  
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
            	val = PGS_AA_2DReadGridF(parms,
                                    nParms, xStart, yStart,
                                    xDim, yDim, parmBuffer,
                                    totalParmMemoryCache, results);
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
            	if (val != PGS_S_SUCCESS)
            	{
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
                	return val;
            	}

            }   
            else if (toolUsed == TOOL3D)
            {
            	zStart = height[nLoop];
#ifdef _PGS_THREADSAFE  
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
            	val = PGS_AA_3DReadGridF(
                                    parms, nParms, xStart, yStart, zStart,
                                    xDim, yDim, zDim,
                                    parmBuffer, totalParmMemoryCache,
                                    results);
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
            	if (val != PGS_S_SUCCESS)
            	{
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
                	return val;
            	}
            }
	}
	else if ((operation & PGSd_AA_OP_NINTCELL) == PGSd_AA_OP_NINTCELL)
	{
	    val = PGS_AA_OP_NINTCELL(xRealStart, yRealStart, &xStart, &yStart);
	    if(val != PGS_S_SUCCESS)
	    {
		PGS_SMF_SetStaticMsg (PGSAA_E_OPERATION,"PGS_AA_NINTCELL");  
		PGS_SMF_SetStaticMsg (PGSAA_E_GEOERROR,"PGS_AA_GEOGrid");
	  
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GEOERROR;	
	    }      
	    
	    /* now branch for 2 and 3 dimensional extraction) */
            if (toolUsed == TOOL2D)
            {
#ifdef _PGS_THREADSAFE  
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
                val = PGS_AA_2DReadGridF(parms,
                                    nParms, xStart, yStart,
                                    xDim, yDim, parmBuffer,
                                    totalParmMemoryCache, results);
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
                if (val != PGS_S_SUCCESS)
                {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
                        return val;
                }

            }
            else if (toolUsed == TOOL3D)
            {
                zStart = height[nLoop];
#ifdef _PGS_THREADSAFE  
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
                val = PGS_AA_3DReadGridF(
                                    parms, nParms, xStart, yStart, zStart,
                                    xDim, yDim, zDim,
                                    parmBuffer, totalParmMemoryCache,
                                    results);
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
                if (val != PGS_S_SUCCESS)
                {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
                        return val;
                }
            }
	}
	
	/* perform interpolation on 2x2 grid */

	else if ((operation & PGSd_AA_OP_INTERP2BY2) == PGSd_AA_OP_INTERP2BY2 &&
		  toolUsed == TOOL2D)
        {
	    /* initialize dimension vectors */
	
	    x1a[0] = (PGSt_double) floor(xRealStart + (PGSt_double)0.5);
	    if(x1a[0] == (PGSt_double)0) /* compensate for the left margin */
            {
                x1a[0] = (PGSt_double)1.0;
            }
            else if(x1a[0] == xCells) /* compensate for the right margin */
            {
                x1a[0] = x1a[0] - (PGSt_double)1.0;
            }
            x1a[1] = x1a[0] + (PGSt_double)1.0;
	    x2a[0] = (PGSt_double) floor(yRealStart + (PGSt_double)0.5);
	    if(x2a[0] == (PGSt_double)0) /* compensate for the left margin */
            {
                x2a[0] = (PGSt_double)1.0;
            }
            else if(x2a[0] == yCells) /* compensate for the right margin */
            {
                x2a[0] = x2a[0] - (PGSt_double)1.0;
            }
            x2a[1] = x2a[0] + (PGSt_double)1.0;

	    /* read the grid values */
#ifdef _PGS_THREADSAFE  
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
            val = PGS_AA_2DReadGridF(parms,
                                    nParms, (PGSt_integer)x1a[0], (PGSt_integer)x2a[0],
                                    2, 2, parmBuffer,
                                    totalParmMemoryCache, yBuffer);
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
            if (val != PGS_S_SUCCESS)
            {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
                  return val;
            }
	    /* extract the info out */

	    /* interpolation routine uses doubles whereas the grid read routine above
	     * might return whatever is the type of data , therefor cast accordingly */

	   if(strcmp("short", dataType[0]) == 0)
           {
		intPtr = (PGSt_integer *) yBuffer;
		y2d[0][0] = (PGSt_double) *intPtr;
		y2d[0][1] = (PGSt_double) *(intPtr + 1);
		y2d[1][0] = (PGSt_double) *(intPtr + 2);
		y2d[1][1] = (PGSt_double) *(intPtr + 3);
           }
           else if(strcmp("long", dataType[0])==0)
           {
		longPtr  = (long *) yBuffer;
		y2d[0][0] = (PGSt_double) *longPtr;
                y2d[0][1] = (PGSt_double) *(longPtr + 1);
                y2d[1][0] = (PGSt_double) *(longPtr + 2);
                y2d[1][1] = (PGSt_double) *(longPtr + 3);
           }
           else if(strcmp("float", dataType[0])==0)
           {
		floatPtr = (float *)yBuffer;
                y2d[0][0] = (PGSt_double) *floatPtr;
                y2d[0][1] = (PGSt_double) *(floatPtr + 1);
                y2d[1][0] = (PGSt_double) *(floatPtr + 2);
                y2d[1][1] = (PGSt_double) *(floatPtr + 3);
           }
           else
           {
		y2d[0][0] = yBuffer[0];
		y2d[0][1] = yBuffer[1];
		y2d[1][0] = yBuffer[2];
		y2d[1][1] = yBuffer[4];
           }
	    yp[0] = &y2d[0][0];
 	    yp[1] = &y2d[1][0];

	   /* obtain the interpolation function index from the support file */

	   val = PGS_AA_PeV_integer(logSuppFile[0], "funcIndex",
                             &funcIndex);
           if (val != PGS_S_SUCCESS ) 
           {

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
 
             return PGSAA_E_GETSUPP;
           }	
	   val = interpFunc[funcIndex](x1a, x2a, yp, (int)2, (int)2, xRealStart + 0.5,
		 	 	yRealStart + 0.5, &locResults, &dResults);
	   if(val != PGS_S_SUCCESS) 
           {

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

             return val;
           }
	
	   /* results are in double , therefore cast apprpriately for the user buffer */

	   roundOff = 0.5;
           if(locResults < 0.0)
           {
                roundOff = (PGSt_double)(-1) * roundOff;
           }
	   if(strcmp("short", dataType[0]) == 0)
    	   {
        	intPtr = (PGSt_integer *) results;
                *intPtr= (PGSt_integer)(locResults + roundOff);
    	   }
    	   else if(strcmp("long", dataType[0])==0)
    	   {
        	longPtr = (long *) results;
                *longPtr= (long)(locResults + roundOff);
    	   }
    	   else if(strcmp("float", dataType[0])==0)
    	   {
        	floatPtr = (float *) results;
                *floatPtr= (float)locResults;
    	   }
    	   else
    	   {
        	doublePtr = (PGSt_double*) results;
                *doublePtr= (PGSt_double)locResults;
    	   }
	}
	/* perform interpolation on 3x3 grid */
	else if ((operation & PGSd_AA_OP_INTERP3BY3) == PGSd_AA_OP_INTERP3BY3 &&
		  toolUsed == TOOL2D)
        {
	    /* first find the nearest cell */
	    val = PGS_AA_OP_NearestCell(xRealStart, yRealStart, &xStart, &yStart);
            if(val != PGS_S_SUCCESS)
            {
                 PGS_SMF_SetStaticMsg (PGSAA_E_OPERATION,"PGS_AA_NEARESTCELL");
                 PGS_SMF_SetStaticMsg (PGSAA_E_GEOERROR,"PGS_AA_GEOGrid");
           
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

                 return PGSAA_E_GEOERROR;
            }
	    /* compensate for left Margin */
	    if(xStart == 1)
	    {
		xStart = xStart +1;
	    }
	    /* compensate for right margin */
	    if(xStart == xCells)
            {   
                xStart = xStart -1;
            }
	    /* compensate for top Margin */
            if(yStart == 1)
            {   
                yStart = yStart + 1;
            }
            /* compensate for bottom margin */
            if(yStart == yCells)
            {   
                yStart = yStart -1;
            }

	    /* initialize dimension vectors 
	     * the start point is the topleft corner cell
	     */

	    x1a[0] = (double) (xStart -1);
	    x1a[1] = (double) xStart;
	    x1a[2] = (double) (xStart + 1);
	    x2a[0] = (double) (yStart - 1);
	    x2a[1] = (double) (yStart);
	    x2a[2] = (double) (yStart + 1);


	    /* read grid values */
#ifdef _PGS_THREADSAFE  
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
            val = PGS_AA_2DReadGridF(parms,
                                    nParms, (PGSt_integer)x1a[0], (PGSt_integer)x2a[0],
                                    3, 3, parmBuffer,
                                    totalParmMemoryCache, yBuffer);
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
            if (val != PGS_S_SUCCESS)
            {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
                  return val;
            }
	    /* extract the info out */
	    /* interpolation routine uses doubles whereas the grid read routine above
             * might return whatever is the type of data , therefor cast accordingly */

	    if(strcmp("short", dataType[0]) == 0)
           {
		intPtr = (PGSt_integer *) yBuffer;
		y2d[0][0] = (PGSt_double) *intPtr;
		y2d[0][1] = (PGSt_double) *(intPtr + 1);
		y2d[0][2] = (PGSt_double) *(intPtr + 2);
		y2d[1][0] = (PGSt_double) *(intPtr + 3);
		y2d[1][1] = (PGSt_double) *(intPtr + 4);
		y2d[1][2] = (PGSt_double) *(intPtr + 5);
		y2d[2][0] = (PGSt_double) *(intPtr + 6);
		y2d[2][1] = (PGSt_double) *(intPtr + 7);
		y2d[2][2] = (PGSt_double) *(intPtr + 8);
           }
           else if(strcmp("long", dataType[0])==0)
           {
		longPtr  = (long *) yBuffer;
		y2d[0][0] = (PGSt_double) *longPtr;
		y2d[0][1] = (PGSt_double) *(longPtr + 1);
                y2d[0][2] = (PGSt_double) *(longPtr + 2);
                y2d[1][0] = (PGSt_double) *(longPtr + 3);
                y2d[1][1] = (PGSt_double) *(longPtr + 4);
                y2d[1][2] = (PGSt_double) *(longPtr + 5);
                y2d[2][0] = (PGSt_double) *(longPtr + 6);
                y2d[2][1] = (PGSt_double) *(longPtr + 7);
                y2d[2][2] = (PGSt_double) *(longPtr + 8);
           }
           else if(strcmp("float", dataType[0])==0)
           {
		floatPtr = (float *)yBuffer;
                y2d[0][0] = (PGSt_double) *floatPtr;
		y2d[0][1] = (PGSt_double) *(floatPtr + 1);
                y2d[0][2] = (PGSt_double) *(floatPtr + 2);
                y2d[1][0] = (PGSt_double) *(floatPtr + 3);
                y2d[1][1] = (PGSt_double) *(floatPtr + 4);
                y2d[1][2] = (PGSt_double) *(floatPtr + 5);
                y2d[2][0] = (PGSt_double) *(floatPtr + 6);
                y2d[2][1] = (PGSt_double) *(floatPtr + 7);
                y2d[2][2] = (PGSt_double) *(floatPtr + 8);
           }
           else
           {
		y2d[0][0] = yBuffer[0];
		y2d[0][1] = yBuffer[1];
		y2d[0][2] = yBuffer[2];
		y2d[1][0] = yBuffer[3];
		y2d[1][1] = yBuffer[4];
		y2d[1][2] = yBuffer[5];
		y2d[2][0] = yBuffer[6];
		y2d[2][1] = yBuffer[7];
		y2d[2][2] = yBuffer[8];
           }
	    yp[0] = &y2d[0][0];
 	    yp[1] = &y2d[1][0];
	    yp[2] = &y2d[2][0];
		 
	   /* obtain the interpolation function index from the support file */

           val = PGS_AA_PeV_integer(logSuppFile[0], "funcIndex",
                             &funcIndex);
           if (val != PGS_S_SUCCESS ) 
           {

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

             return PGSAA_E_GETSUPP;
	   }

	   /* interpolate */

	   val = interpFunc[funcIndex](x1a, x2a, yp, (int)3, (int)3, xRealStart + 0.5,
		 	 	yRealStart + 0.5, &locResults, &dResults);
	   if(val != PGS_S_SUCCESS)
	   {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
		return val;
	   }

	   /* cast the result value according to the type of user result buffer */
	  
	   roundOff = 0.5;
           if(locResults < 0.0)
           {
                roundOff = (PGSt_double)(-1) * roundOff;
           }

	   if(strcmp("short", dataType[0]) == 0)
    	   {
        	intPtr = (PGSt_integer *) results;
                *intPtr= (PGSt_integer)(locResults + roundOff);
    	   }
    	   else if(strcmp("long", dataType[0])==0)
    	   {
        	longPtr = (long *) results;
                *longPtr= (long)(locResults + roundOff);
    	   }
    	   else if(strcmp("float", dataType[0])==0)
    	   {
        	floatPtr = (float *) results;
                *floatPtr= (float)locResults;
    	   }
    	   else
    	   {
        	doublePtr = (PGSt_double*) results;
                *doublePtr= (PGSt_double)locResults;
    	   }
	}
	else
	{
	    PGS_SMF_SetStaticMsg (PGSAA_E_OPERATIONUNSET, 
					   "PGS_AA_GEOGrid");
	    PGS_SMF_SetStaticMsg (PGSAA_E_GEOERROR,"PGS_AA_GEOGrid");

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

	    return PGSAA_E_GEOERROR;
	}
	
	/* increment results to point to the next bit of memory */
	
	if ( strcmp ("float",dataType[0])==0)
	{
	    results = (float*)(results) + nParms; 
	}
	
	else if (strcmp ("double",dataType[0])==0)
	{
	    results = (double*)(results) + nParms;
	}
	
	else if (strcmp ("short",dataType[0])==0)
	{
	    results = (PGSt_integer*)(results) + nParms;
	}
	
	else if (strcmp ("long",dataType[0])==0)
	{
	    results = (long*)(results) + nParms;
	} 
	/* end increment of results */
    }
    /* end */

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

    return PGS_S_SUCCESS;
}
