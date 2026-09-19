/*********************************************************
PGS_DEM_OrderIndicesSumPix.c--

This function take as array of indices and  array of PGS_DEM_PointRecords and 
orders the indices based on the subgrid value of the _PointRecord.  Essentially,
 it reorders a list of the indices of PGS_DEM_PointRecords into groupings by
subgrid number. This function assumes that the indices array is 0 based.

Presently using quick sort algorithm.  Eventually, should change to an algorithm
more adapt at dealing with binned data.  We expect that there will often be many
points which have the same subgrid value.  Because, no sorting occurs within a
subgrid value, quick sorting may not be idle. This quick sorting algorithm was
adapted from the quicksort in "Numerical Recipes in C".  I basically used the
format as is with slight modification for the comparison.

Will need to do a separate one for both PGSt_DEM_PointRecordPix and PGSt_DEM_PointRecordDeg.
This is PIX.

Sum up the number of subgrids and the number of points in each subgrid.  when
rewrite sorting function, probably more efficient to add to the actual function.


Author--
Alexis Zubrow
Abe Taaheri
Phuong T. Nguyen
Dates--
First Programming       2/11/1997
3/24/99        AT       Changed the function PGS_DEM_CompareIndicesBySubgrid2
                        to get rid of some redundent declerations and 
                        assignments.
7/8/99         SZ       Updated for the thread-safe functionality
7/15/2002      PTN      Updated for C++
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>
#include <PGS_TSF.h>

/*This isn't quite kosher, but come back and fix when implement new searching
  routine*/

#ifdef _PGS_THREADSAFE
    /* Create non-static variable for the thread-safe version */
    PGSt_DEM_PointRecordPix **pointArray;
#else
    static PGSt_DEM_PointRecordPix **pointArray;
#endif 

#ifdef __cplusplus
    static int PGS_DEM_CompareIndicesBySubgrid2(const void *, const void *);
#endif

PGSt_SMF_status
PGS_DEM_OrderIndicesSumPix(
    PGSt_integer indicesArray[], /*Array of indices of the dataArray*/
    PGSt_integer numPoints,      /*number of points to sort, begins from front*/
    PGSt_DEM_PointRecordPix **dataArray,  /*Array containing subgrid number*/
    PGSt_DEM_FileRecord **subset,         /*Subset, array of FileRecords*/
    PGSt_integer *numSubgrids)   /*number subgrids the pnts. ditributed over*/

{
    /*comparison function in qsort-- quick sort*/
    int PGS_DEM_CompareIndicesBySubgrid2(const void *, const void *);

    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE]; /*string appened to error return*/
    
    PGSt_integer subgridVal;         /*present points subgrid value*/
    PGSt_integer nextSubgridVal;     /*next points subgrid value*/
    PGSt_integer i;                  /*index*/
    PGSt_integer sumPoints;          /*number of points with sam subgrid value*/
    PGSt_integer countSubgrids;      /*keep track of number subgrids*/
    
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status  status;
    /* Set up thread-safe key keeper and TSD key */
    PGSt_TSF_MasterStruct *masterTSF;
    status = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(status))
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    pointArray = (PGSt_DEM_PointRecordPix **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMPOINTARRAY]);
#endif

    /*check if number of points is valid input*/
    if (numPoints <= 0)
    {
	/*ERROR no need to invoke function, can't sort less than one point*/
	sprintf(dynamicMsg, "Improper tag... (%d) is an improper number "
		"of points to sort", numPoints);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
                              "PGS_DEM_OrderIndicesSumPix()");
	return (PGSDEM_E_IMPROPER_TAG);
	
    }

    if (numPoints == 1)
    {
	/*don't need to order only a single point*/
	subgridVal = dataArray[indicesArray[0]] -> subgridValue;
	if (subset[subgridVal] == NULL)
	{
	    /*ERROR data not staged*/
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
				      "Cannot access subgrid... "
				      "data not staged", 
				      "PGS_DEM_OrderIndicesSumPix()");
	    return (PGSDEM_E_CANNOT_ACCESS_DATA);
	}
	subset[subgridVal] -> pntsRequested = numPoints;
	*numSubgrids = 1;
	return(PGS_S_SUCCESS);
    }
    
    /*assign the input arrray pointer to the static pointer, so that the
      comparison function can access the information*/

    pointArray = dataArray;
#ifdef _PGS_THREADSAFE
    /* Reset TSD key */
    pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMPOINTARRAY],
                            pointArray);
#endif
    
   /*Do comparison with qsort*/
    qsort((void *)indicesArray, numPoints, sizeof(PGSt_integer),
	  PGS_DEM_CompareIndicesBySubgrid2);

    /*AFter sorted, go through once more and get the number of subgrids, and the
      number of points in each subgrid. store this in the subset FileRecord, at
      some point this should be placed in the sort function, but this will help
      optimize PGS_DEM_RecursiveSearch() */
    /*REMEMBER - have to use indices array to reference the subgrid order of the
      datapoints*/

    /*initialize*/
    countSubgrids = 1;
    sumPoints = 1;
    
    for (i = 0; i < numPoints; i++)
    {
	/*check to see if last point, if so record the number of points
	  requested */
	if (i == numPoints -1)
	{
	    subgridVal = dataArray[indicesArray[i]] -> subgridValue;
	    if (subset[subgridVal] == NULL)
	    {
		/*ERROR data not staged*/
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
				      "Cannot access subgrid... "
				      "data not staged", 
				      "PGS_DEM_OrderIndicesSumPix()");
		return (PGSDEM_E_CANNOT_ACCESS_DATA);
	    }
	    subset[subgridVal] -> pntsRequested = sumPoints;
	    *numSubgrids = countSubgrids;
	    return(PGS_S_SUCCESS);
	    
	}

	subgridVal = dataArray[indicesArray[i]] -> subgridValue;
	nextSubgridVal = dataArray[indicesArray[i +1]] -> subgridValue;

	if (subgridVal == nextSubgridVal)
	{
	    sumPoints++;
	}
	else
	{
	    if(subset[subgridVal] == NULL)
	    {
		/*ERROR data not staged*/
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
				      "Cannot access subgrid... "
				      "data not staged", 
				      "PGS_DEM_OrderIndicesSumPix()");
		return (PGSDEM_E_CANNOT_ACCESS_DATA);
	    }
	    subset[subgridVal] -> pntsRequested = sumPoints;
	    sumPoints = 1;
	    countSubgrids ++;
	}
    }
    
    *numSubgrids = countSubgrids;
    
    /*Successfuly ordered array*/
    return (PGS_S_SUCCESS);
    
}
int PGS_DEM_CompareIndicesBySubgrid2(const void *qIndicesPointer, 
					   const void *yIndicesPointer)
{    
    return( (pointArray[*(int *)qIndicesPointer] -> subgridValue) - 
	    (pointArray[*(int *)yIndicesPointer] -> subgridValue));
}
/*comparison function for qsort*/
int PGS_DEM_CompareIndicesBySubgrid2_orig(const void *qIndicesPointer, 
					   const void *yIndicesPointer)
{
    
    PGSt_integer *aIndicesPointer, *bIndicesPointer;
    PGSt_integer subgridValue1;    /*subgrid value for particular indices*/
    PGSt_integer subgridValue2;
    
    /*Important to remeber that these are pointers to the indices of the array
      dataArray.  We are trying to reorder the array of these indices by the
      subgrid value in dataArray*/
    aIndicesPointer = (int *)qIndicesPointer;
    bIndicesPointer = (int *)yIndicesPointer;

    /*get subgrid value from particular indices*/
    subgridValue1 = pointArray[*aIndicesPointer] -> subgridValue;
    subgridValue2 = pointArray[*bIndicesPointer] -> subgridValue;
    
    if (subgridValue1 == subgridValue2)
    {
	return(0);
    }
    else if (subgridValue1 < subgridValue2)
    {
	return(-1);
    }
    else 
    {
	return(1);
    }
}
