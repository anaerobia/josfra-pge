/**********************************************************
PGS_DEM_RecursiveSearchBil.c--

This function searches through various subgrids for data points.  Each recursion
is a different resolution search.  The function takes an array of position
elements and extracts the four nearest data points to each position.  Once it
has successfuly extracted the four data points, it performs a bilinear
interpolation on the points.  Before doing this interpolation, it checks the
four points for fill values.  

If a point is a fill value, then it's indices is stored at the beginning of the
ordered array.  When the function has gotten values for all the entered points,
it calls itself with the new ordered array of fill value points. This time, the
array has been ordered by the subgrid values of the next (lower) resolution
dataset.  The base case for the function is when no more resolutions are
available or if there are no fill values left in the points of interest. 

Author--
Alexis Zubrow/SAC

Dates--
July 15, 1997       AZ      First Created
12/8/1998      Abe Taaheri  Fixed problem with extracting region when the
                            region wraps across the 180 degrees east and
			    both upperleft corner and upperright corner of
			    the region (or lowerleft corner and lowerright 
			    corner) fall inside the same subgrid.
			    extractedRegions are now allocated twice the
			    memory allocated originally to avoid conflict when
			    two subregions fall on the same subgrid after and 
			    before wrapping
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>
#include <mfhdf.h>
#include <HdfEosDef.h>

PGSt_SMF_status
PGS_DEM_RecursiveSearchBil(
    PGSt_DEM_Tag resolutionList[],       /*list of resolutions to search*/
    PGSt_integer numResolutions,         /*number of resolutions in list*/
    PGSt_DEM_SubsetRecord *subsetInfo,   /*subset Information*/
    PGSt_DEM_FileRecord  **subset, 	 /*pointer to complete subset*/
    PGSt_DEM_PointRecordDeg **dataPoints, /*Array of points to search*/
    PGSt_integer numSubgridCover,        /*number subgrids, covered by points*/
    PGSt_integer orderedArray[],         /*order of indices for dataPoints*/
    PGSt_integer numPoints)   		 /*number points in orderedArray*/
{
    

    /*status and status messages*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

    /*SubsetInfo, subset, and number of points covering-- for the next
      resolution in the resolution list*/

    PGSt_DEM_SubsetRecord *subsetInfoNext;
    PGSt_DEM_FileRecord **subsetNext;
    PGSt_integer numSubgridCoverNext;
    
    /*Fill point info and looping variables */
    PGSt_integer numFillPoints = 0;   /*number of fill value points discovered*/
    PGSt_integer i;		      /*counter, for looping*/
    PGSt_integer j;		      /*counter, for looping*/
    PGSt_integer fillIncluded;        /*number of fill values in bounding 
					region */

    
    PGSt_integer crnRowPixel[2];  /*Row position, global pixels, 0th element is
				    North point*/
    PGSt_integer crnColPixel[2];  /*Column position, global pixels, 0th element
				    is West point*/
    
    PGSt_DEM_PointRecordBil *interpPnts; /*Records all position info for
					   bilinear interpolation*/
    PGSt_integer subgridUpprLft;       /*Subgrid value of Northwest corner*/
    
    PGSt_byte *dataRegion = NULL;      /*data buffer variable type */

    PGSt_DEM_RegionRecord **extractedRegions; /*Holds information for each
						subregion. Used to extract the
						region*/ 
    
    PGSt_integer extentSubgridsVert;   /*Number of subgrids vertically spanned
					 by the region*/
    PGSt_integer extentSubgridsHoriz;  /*Number of subgrids Horizontally 
					 spanned by the region*/
    PGSt_double bndValue[4];           /*data value, element 0-- Southwest
					 corner of region, element 1-- SE point,
					 element-- 2 NE point, element 3-- NW
					 point */
    PGSt_integer fillPosition[4];      /*elements of the bounding region
					 (bndValue) which are fill values*/
    
    PGSt_double interpValue;           /*this is the value interpolated for the
					 point of interest (store in double
					 format until moved to dataPoints)*/
 
    /*Facilitates access to buffer with multiple type data*/
    int8 *pntBufInt8;           /*int8 data*/
    int16 *pntBufInt16;         /*int16 data*/
    float32 *pntBufFlt32;       /*float32 data*/
    


    /***************************/


    /*Calloc space for the data buffer. This is type dependent, look at layer to
      determine this*/

    /*!!!THIS should be removed and added to _Subset or _Lookup */
    if (subsetInfo -> dataType == DFNT_INT8)
    {
	subsetInfo -> numBytes = sizeof(int8);
    }
    else if (subsetInfo -> dataType == DFNT_INT16)
    {
	subsetInfo -> numBytes = sizeof(int16);
    }
    else if (subsetInfo -> dataType == DFNT_FLOAT32)
    {
	subsetInfo -> numBytes = sizeof(float32);
    }

    dataRegion = (PGSt_byte *) calloc(4, subsetInfo -> numBytes);
    if (dataRegion == NULL)
    {
	/*ERROR callocing space*/
	sprintf(dynamicMsg,"Error allocating space... "
		"for dataRegion");
	PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
				  "PGS_DEM_RecursiveSearchBil()");
	statusError = PGSMEM_E_NO_MEMORY;
	
	/*fatal error*/
	goto fatalError;
	
    }
	
    /*calloc space for the interpolationPnts*/
    interpPnts = (PGSt_DEM_PointRecordBil *) calloc(1, 
                             sizeof(PGSt_DEM_PointRecordBil));
    if (interpPnts == NULL)
    {
	/*ERROR callocing space*/
	sprintf(dynamicMsg,"Error allocating space... "
		"for interpPnts");
	PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
				  "PGS_DEM_RecursiveSearchBil()");
	statusError = PGSMEM_E_NO_MEMORY;
	
	/*fatal error*/
	goto fatalError;
	
    }


    /*calloc space for the RegionRecord array*/
    extractedRegions = (PGSt_DEM_RegionRecord **) calloc (
          2*(subsetInfo -> numSubgrids), sizeof(PGSt_DEM_RegionRecord *));
    if (extractedRegions == NULL)
    {
	/*ERROR callocing*/
	sprintf(dynamicMsg,"Error allocating space... "
		"for the RegionRecord");
	PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
				  "PGS_DEM_RecursiveSearchBil()");
	statusError = PGSMEM_E_NO_MEMORY;
	
	/*fatal error*/
	goto fatalError;
	
    }
	
    /*Loop through every point of interest.  For each point record the position
      to be interpolated. */
    for (i = 0; i < numPoints; i++)
    {
	/*Remeber use the indices array to access elements of the array
	  dataPoints.  By using the indices array, one can reorder the accessing
	  pattern of dataPoints without changing the order of the elements. */

	/*record the point of interest, the position which one is
	  interpolating.  Element 0 is latitude, element 1 is longitude, in
	  decimal degrees.*/
	
	interpPnts -> positionInterp[0] = 
	  dataPoints[orderedArray[i]] -> positionOrig[0];
	
	interpPnts -> positionInterp[1] = 
	  dataPoints[orderedArray[i]] -> positionOrig[1];
	
	/*Find the bounding points. need these to succesfully perform the
	  bilinear interpolation. These are the four points which surround the
	  point of interest*/
	
	status = PGS_DEM_GetBoundingPnts(subsetInfo, interpPnts);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR calculating bounding points*/
	    sprintf(dynamicMsg,"Cannot access data... "
		    "error calculating bounding points for "
		    "bilinear interpolation");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
                                  dynamicMsg,"PGS_DEM_RecursiveSearchBil()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    	    
	    /*fatal error*/
	    goto fatalError;
	    
	}
	
	/*convert the North East and SouthWest bounding points into global
	  pixels.  Used in PGS_DEM_ExtentRegion. */
	crnRowPixel[0] = PGSm_DEM_LatToPixel(interpPnts -> boundingPnts[2][0],
					     subsetInfo);
	crnRowPixel[1] = PGSm_DEM_LatToPixel(interpPnts -> boundingPnts[1][0],
					     subsetInfo);
	crnColPixel[0] = PGSm_DEM_LonToPixel(interpPnts -> boundingPnts[3][1],
					     subsetInfo);
	crnColPixel[1] = PGSm_DEM_LonToPixel(interpPnts -> boundingPnts[2][1],
					     subsetInfo);
	subgridUpprLft = PGSm_DEM_PixToSubgrid(crnRowPixel[0], crnColPixel[0],
					       subsetInfo);
	

	/*At this point we have position of the 4 points which bound the point
	  to be interpolated.  We now need to extract the data. Use the "guts" 
	  of PGS_DEM_GetRegion. Don't use the internal function
	  PGS_DEM_ReplaceFillPoints */  
  
	/*Determine the number of subgrids covered by the 4 pnts., and the
	  coordinates of each subregion. There is always the possibility that
	  the 4 point region crosses a file boundary*/
	
	status = PGS_DEM_ExtentRegion(crnRowPixel, crnColPixel, subsetInfo,
				      extractedRegions, &extentSubgridsVert,
				      &extentSubgridsHoriz);
	
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR defining the subregions */
	    sprintf(dynamicMsg, "Cannot access data... "
		    "error defining the extent of the subregions");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
                                  dynamicMsg,"PGS_DEM_RecursiveSearchBil()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	    
	}
	
	/*Extract the subregions from their respective subgrids and writes the
	  subregions to the buffer.  This buffer is the values of our 4 bounding
	  points*/

	status = PGS_DEM_ExtractRegion(subsetInfo, subset, extractedRegions,
				       dataRegion, subgridUpprLft, 
				       extentSubgridsVert, extentSubgridsHoriz);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR extracting the subregions*/
	    sprintf(dynamicMsg, "Cannot access data... "
		    "error extracting the subregions or composing them " 
		    "into a single region");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
                                  dynamicMsg,"PGS_DEM_RecursiveSearch()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	    
	}
	
	/*store the values for each of the bounding points.  this is type
	  specific.  The elements in dataRegion are 0th = NW, 1st = NE, 2nd =
	  SW, 3rd = SE.  the elements for bndValue are defined above */
	if (subsetInfo -> dataType == DFNT_INT8)
	{
	    /*8 bit integer*/
	    pntBufInt8 = (int8 *)dataRegion;
	    
	    bndValue[3] = pntBufInt8[0];	    
	    bndValue[2] = pntBufInt8[1];
	    bndValue[0] = pntBufInt8[2];
	    bndValue[1] = pntBufInt8[3];
	}
	else if (subsetInfo -> dataType == DFNT_INT16)
	{
	    /*16 bit integer*/
	    pntBufInt16 = (int16 *)dataRegion;
	    
	    bndValue[3] = pntBufInt16[0];
	    bndValue[2] = pntBufInt16[1];
	    bndValue[0] = pntBufInt16[2];
	    bndValue[1] = pntBufInt16[3];
	}
	else if (subsetInfo -> dataType == DFNT_FLOAT32)
	{
	    /*32 bit float*/
	    pntBufFlt32 = (float32 *)dataRegion;
	    
	    bndValue[3] = pntBufFlt32[0];
	    bndValue[2] = pntBufFlt32[1];
	    bndValue[0] = pntBufFlt32[2];
	    bndValue[1] = pntBufFlt32[3];
	}
	

	/*check if any of the elements are fill data.  If there are fill values,
	 record the element of the position array and the increment the number
	 of fills in the bounding region (used for interpolation).*/
	fillIncluded = 0;
	for (j = 0; j < 4; j++)
	{
	    if(bndValue[j] == (subsetInfo -> fillvalue))
	    {
		fillPosition[fillIncluded] = j;		
		fillIncluded++;
	    }
	}
	
	/*Interpolate the point of interest */
	status = PGS_DEM_Interpolate(subsetInfo, interpPnts, bndValue,
				     fillPosition, fillIncluded, &interpValue);
	if ((status != PGS_S_SUCCESS) && 
	    (status != PGSDEM_M_FILLVALUE_INCLUDED))
	{
	    /*Error interpolating point of interest*/
	    sprintf(dynamicMsg,"Cannot access data... "
		    "problem interpolating interpValue");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
                                  dynamicMsg,"PGS_DEM_RecursiveSearchBil()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	    
	}
	
	/*Assign interpValue to the appropriate type of dataValue.  Remeber, use
	  the orderedArray to access the appropriate record of dataPoints.*/
	if (subsetInfo -> dataType == DFNT_INT8)
	{
	    /*int8 data type*/
	    dataPoints[orderedArray[i]] -> dataValue.OneByteInt = (int8)interpValue;
	}
	else if (subsetInfo -> dataType == DFNT_INT16)
	{
	    /*int16 data type*/
	    dataPoints[orderedArray[i]] -> dataValue.TwoByteInt = (int16)interpValue;
	}
	if (subsetInfo -> dataType == DFNT_FLOAT32)
	{
	    /*float32 data type*/
	    dataPoints[orderedArray[i]] -> dataValue.FourByteFlt = 
	      interpValue;
	}

	
	/*Here we determine which points need to be interpolated from the next
	  (lower) resolution.  If there are no more resolutions, then we simply
	  increment the number of fill values. If there are more resolutions in
	  the resolutionList, we set up the interpolation for that next
	  resolution. To perform the interpolation on the next resolution, we
	  need to record the next subgrid value in the appropriate PointRecord
	  of the array dataPoints.  In addition, we need to re-populate the
	  orderedArray.  We begin the orderedArray (element 0) with the index of
	  the first point which could not be interpolated in this resolution,
	  ie. a point which is fill. with each new fill point, we also increment
	  the attribute numFillPoints. */

	if (numResolutions == 1)
	{
	    
	    if(status == PGSDEM_M_FILLVALUE_INCLUDED)
	    {
		/*interpValue is a fill value. Increment numFillPoints*/
		numFillPoints++;
	    }
	}

	/*More than 1 resolution, need to setup fill points for interpolation in
	  the next resolution*/
	else
	{
	    if(status == PGSDEM_M_FILLVALUE_INCLUDED)
	    {
		/*interpValue is a fill value. */

		/*Check if this is the first fill value, if so, need to access
		  subsetInfo one the next resolution*/

		if (numFillPoints == 0)
		{
		    /*Get new subsetInfo and subset of the next
		      resolution in the resolutionList*/ 
		    status = PGS_DEM_Subset(resolutionList[1], 
					    subsetInfo -> layer,
					    PGSd_DEM_INFO,
					    &subsetNext,
					    &subsetInfoNext);
		    if (status != PGS_S_SUCCESS)
		    {
			/*ERROR getting info and subset from next
			  resolution*/
			
			sprintf(dynamicMsg,"Cannot access data... "
				"problem accessing subsetInfoNext and "
				"subsetNext for resolution (%d)", 
				resolutionList[1]);
			PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
                                              dynamicMsg,
                                              "PGS_DEM_RecursiveSearchBil()");
			statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
			
			/*fatal error*/
			goto fatalError;
			
		    }
		}

		/*Change subgrid value for the interpolation point of interest.
		  Subgrid value needs to now correspond to the subgrids of the
		  next resolution, the resolution in which the recursive call
		  will attempt to interpolate this value from. */

		dataPoints[orderedArray[i]] -> subgridValue =
		  PGSm_DEM_LatLonToSubgrid(interpPnts -> positionInterp[0],
					   interpPnts -> positionInterp[1],
					   subsetInfoNext);
		
		/*reassign the fill value index to orderedArray.  This allows
		  for the next recursive pass through this function to only
		  attempt to interpolate the fill values returned from this
		  resolution. increment the number of fill points*/

		orderedArray[numFillPoints] = orderedArray[i];
		numFillPoints++;
	    }
	}
    }
    
		  
    /*free up allocated space inside extractedRegion, each element of the
      array */
    for (i = 0; i < 2*(subsetInfo -> numSubgrids); i++)
    {
	/*check if particular subgrid was allocated*/
	if (extractedRegions[i] != NULL)
	{
	    /*free subregion's RegionRecord*/
	    
	    free(extractedRegions[i]);
	    extractedRegions[i] = NULL;
	    
	}
    }
    
    
    
    /*Free up calloced space*/
    free(extractedRegions);
    free(interpPnts);
    free(dataRegion);
    

    /*Check for base cases. We have two possible base cases for this recursive
      function: if there are no fill values and if there are no more
      resolutions available to interpolate from.  IF neither of these are
      satisfied, reorder the indices array by subgrid and call
      PGS_DEM_RecursiveSearchBil on the fill points using the next
      resolution. */

    if (numFillPoints == 0)
    {
	/*no fill values, able to successfuly interpolate all points*/
	return(PGS_S_SUCCESS);
    }
    
    if (numResolutions == 1)
    {
	/*Fill values are still in data, but there are no more resolutions in
	  the resolutionList to interpolate from*/
	sprintf(dynamicMsg, "Fill Values included..."
		"possibilitly some of data interpolated from " 
		"multiple resolutions");
	PGS_SMF_SetDynamicMsg(PGSDEM_M_FILLVALUE_INCLUDED,
                              dynamicMsg,"PGS_DEM_RecursiveSearchBil()");
	return(PGSDEM_M_FILLVALUE_INCLUDED);
    }
    
    /*otherwise must reorder the ordered array of fill values and call
      the function PGS_DEM_RecursiveSearchBil... again*/

    status = PGS_DEM_OrderIndicesSumDeg(orderedArray, numFillPoints,
					dataPoints, subsetNext,
					&numSubgridCoverNext);
	
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR reordering array*/
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
				  "Cannot reorganize points based "
				  "on subgrid value", 
				  "PGS_DEM_RecursiveSearchBil()");
	return (PGSDEM_E_CANNOT_ACCESS_DATA);
	    
    }
    
    
    /*Calls itself, increment resolution list, decriment number of
      resolutions, pass in next resolution subsetInfo, pass in
      reorganized orderedArray, and pass in the number of fill value
      points*/

    status = PGS_DEM_RecursiveSearchBil((resolutionList + 1),
					numResolutions - 1,
					subsetInfoNext,
					subsetNext,
					dataPoints,
					numSubgridCoverNext,
					orderedArray,
					numFillPoints);
    
    /*If status is success, one has actually interpolated all the fill values
      from this resolution in the next resolution.  Therefore, the appropriate
      return is PGSDEM_M_MULTIPLE_RESOLUTIONS */
    if (status == PGS_S_SUCCESS)
    {
	sprintf(dynamicMsg, 
		"Fill Values interpolated from multiple resolutions");
	PGS_SMF_SetDynamicMsg(PGSDEM_M_MULTIPLE_RESOLUTIONS,
                              dynamicMsg,"PGS_DEM_RecursiveSearchBil()");
	return(PGSDEM_M_MULTIPLE_RESOLUTIONS);
	
    }
    else
    {
	/*simply return the status return of the recursive call*/
	return(status);
    }
    


    /*fatal error-- this cleans up all the calloced space, closes all the
      opened files, and returns the error code*/

fatalError:
    {

	if (dataRegion != NULL)
	{
	    free(dataRegion);
	}
	
	if(interpPnts != NULL)
	{
	    free(interpPnts);
	}
	
	if (extractedRegions != NULL)
	{
	    for (i = 0; i < 2*(subsetInfo -> numSubgrids); i++)
	    {
		if (extractedRegions[i] != NULL)
		{
		    free(extractedRegions[i]);
		    extractedRegions[i] = NULL;
		    
		}
	    }
	    free(extractedRegions);
	    extractedRegions = NULL;
	    
	}
	

	return (statusError);
    }
    

}
