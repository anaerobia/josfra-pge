/*********************************************************
PGS_DEM_ReplaceFillPointsInt16.c--

This function reads the region of data which has been extracted and records any
fillvalues. This function has two different branches.  If the number of
resolutions in the resolutionList is equal to 1, it only searches until it finds
the first fill value point.  This branch is solely for error reportoing. In
other words, if the number of resolutions equals one, the function only needs to
determine if there are any fill values in the region.  The second branch, only
is invoked if the number of resolutions is greater than 1.  In this case, every
fill value point is recorded in a PointRecord array. This PointRecord array
becomes the source of a second search on these points.  In order to
interpolate their values from the next resolution in the resolutionList, the
points are passed to PGS_DEM_RecursiveSearch. This function assumes that the
corner pixels of the region were checked to see if they were reasonable.


 This Function only replaces fill points for layers with data types of INT16 


Author--
Alexis Zubrow/SAC  
Ebraahim Moghaddam-Taaheri/SAC  

Dates--
2/25/1997    AZ       First Programming       
7/10/1997    AZ/EMT   Add wrapping condition, Nearest Neighbor
7/30/1997    AZ       Added bilinear interpolation

*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>
#include <PGS_MEM.h>

PGSt_SMF_status
PGS_DEM_ReplaceFillPointsInt16(
    PGSt_DEM_Tag resolutionList[],  /*list of resolutions to access*/ 
    PGSt_integer numResolutions,    /*number of resolutions in list*/
    PGSt_integer layer,             /*layer to be accessed*/
    PGSt_integer interpolation,     /*interpolation flag*/
    void *dataRegionIn,               /*data buffer for output*/
    PGSt_DEM_SubsetRecord *subsetInfo,  /*subset Information*/
    PGSt_integer cornerRowPixels[2],    /*corners of region, row pixels*/
    PGSt_integer cornerColPixels[2])    /*corners of region, column pixels*/

  
{
    /*pointer to switch dataRegionIn to int16 pntr. */
    int16 *dataRegion;
    
      /*return status*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    PGSt_SMF_status  statusReturn;       /*status returned to _GetRegion*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

    PGSt_integer i;                      /*looping index*/
    

    PGSt_integer numFillPoints = 0; /*number of fill value points discovered*/

    /*arrays to hold fill points and an indicesArray for RecursiveSearch*/
    PGSt_integer *indicesArray;               
    PGSt_DEM_PointRecordDeg **dataPointsDeg; /*fill points array bilinear
					       interpolation*/
    PGSt_integer arrayLength;                /*number elements in arrays*/
     
    


    /*dimensions and positions*/
    PGSt_integer numRowsRegion;  /*Number rows spanning the region*/
    PGSt_integer numColsRegion;  /*Number columns spanning the region*/
    PGSt_integer totalNumPixels; /*total number pixels in region*/

    PGSt_integer pixLat;         /*global pixel position- present resolution*/
    PGSt_integer pixLon;         /*global pixel position- longitude*/

    PGSt_double pntLatitude;     /*latitude in signed decimal degrees*/
    PGSt_double pntLongitude;    /*longitude in signed decimal degrees*/

    PGSt_integer pixLatOffset;   /*Number of row pixels from corner dataRegion*/
    PGSt_integer pixLonOffset;   /*Number of column pixels from the upper left
				   corner of the dataRegion*/
    
    PGSt_integer buffPosition;   /*points position within buffer, number from
				   the first point in the dataRegion*/
    
    PGSt_integer subgridNext;    /*subgrid value of point, next resolution*/

    PGSt_integer maxHorizPixel;  /*Maximum number of horizontal pixels, spanning
				   world.  Used for wrapping condition.*/
    
    
    /*SubsetInfo, subset, and number of points covering-- for the next
      resolution in the resolution list*/
    PGSt_DEM_SubsetRecord *subsetInfoNext;
    PGSt_DEM_FileRecord **subsetNext;
    PGSt_integer numSubgridCoverNext;

    /*Calculate maximum horizontal pixel value */
    maxHorizPixel = (subsetInfo -> horizPixSubgrid) * 
      (subsetInfo -> subgridHoriz) - 1;

    /*first calculate the dimensions of the dataRegion*/
    numRowsRegion = cornerRowPixels[1] - cornerRowPixels[0] + 1;

    /*Check for wrapping condition, ie. the region of interest crossing the
      intenational date line */
    if (cornerColPixels[0] > cornerColPixels[1])
    {
	/*Wrapping condition*/
	numColsRegion = (maxHorizPixel - cornerColPixels[0] + 1) +
	  (cornerColPixels[1] + 1);
    }
    else
    {
	/*non wrapping condition*/
	numColsRegion = cornerColPixels[1] - cornerColPixels[0] + 1;
    }
    
    totalNumPixels = numRowsRegion * numColsRegion;

    /*assign dataRegion to the address of dataRegionIn*/
    dataRegion = (int16 *) dataRegionIn;
    
    /**********************************************************/    

    /*Here is the first possible branch of this function. if the number of
      resolution in the resolutionList equals one, then the function does not
      interpolate values for the fill points.  in this case, all we need to do
      is search the data region until a fill value is found. error checking.*/

    /**********************************************************/    

    if (numResolutions == 1)
    {
	for (i = 0; i < totalNumPixels; i++)
	{
	    if (dataRegion[i] == (subsetInfo -> fillvalue))
	    {
		/*Message to user, fill values included in final data region*/
		PGS_SMF_SetStaticMsg(PGSDEM_M_FILLVALUE_INCLUDED, 
					 "PGS_DEM_ReplaceFillPoints()");
		return (PGSDEM_M_FILLVALUE_INCLUDED);
	    }
	}
	
	/*return succes, if none of the data points in dataRegion are fill
	  values*/
	return(PGS_S_SUCCESS);
    }
    
    /**********************************************************/    

    /*If there are more than one resolution in the list, then have to check
      every data element retrieved for fill values. As one finds fill
      values, the numFillPoints is incremented. These fill points must be
      recorded in an array of PointRecords. Presently, I've allocated a fourth
      the total number of pixels in the region for fill value points.  If there
      are more fill values than this initial (hopefully over) estimate, than I
      will realloc a space twice the original. As fill points are found, their
      position must be recorded in the PointRecord. If the interpolation method
      is nearest neighbor, the positions will be recorded interms of the pixel
      coordinates of the next resolution in the resolutionList. If the
      interpolation method is bilinear, the position will be recorded in signed
      decimal degree format. This allows for more exact interpolation. Once all
      the fill points have been recorded, the array of PointRecords and an
      indices array are reordered by the subgrid values.  The subgrid values are
      the subgrid values of these points in the next resolution in the
      resolutionList. These reordered points are then passed on to the
      RecursiveSearch function for interpolation*/ 	 

    /**********************************************************/    

    /*With the assumptions that we will need to interpolate fill points,  let us
      get the subset and subsetInfo for the next resolution*/
    status = PGS_DEM_Subset(resolutionList[1], layer, PGSd_DEM_INFO,
			    &subsetNext, &subsetInfoNext);
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR trying to access next resolution*/
	sprintf(dynamicMsg, "Cannot access data... "
		"error getting subset info for next resolution");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
				  dynamicMsg,
				  "PGS_DEM_ReplaceFillPoints.c");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
	
    }
    
    /*************************************/    
    /* we search for fill values.  we then record the fill points location
       interms of signed decimal degrees*/
    /*************************************/    


    /*calloc space for the data points and the corresponding indicesArray*/
    arrayLength = (totalNumPixels/4) + 1;
    
    dataPointsDeg = (PGSt_DEM_PointRecordDeg **) calloc(arrayLength,
			   sizeof(PGSt_DEM_PointRecordDeg *));
    if (dataPointsDeg == NULL)
    {
	/*ERROR callocing*/
	sprintf(dynamicMsg,"Error allocating space... "
		"for the dataPointsDeg");
	PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
				  "PGS_DEM_ReplaceFillPointsInt16()");
	statusError = PGSMEM_E_NO_MEMORY;
	
	/*fatal error*/
	goto fatalError;

    }
    indicesArray = (PGSt_integer *) calloc(arrayLength, sizeof(PGSt_integer));
    if (indicesArray == NULL)
    {
	/*error callocing*/
	sprintf(dynamicMsg,"Error allocating space... "
		"for the indicesArray");
	PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
					  "PGS_DEM_ReplaceFillPointsInt16()");
	statusError = PGSMEM_E_NO_MEMORY;
	
	/*fatal error*/
	goto fatalError;


    }
    
	/*read every pixel and check for fill value*/
	for (i = 0; i < totalNumPixels; i++)
	{
	    if (dataRegion[i] == (int16) (subsetInfo -> fillvalue))
	    {
		numFillPoints ++;
		
		/*convert the points position in the dataRegion to coordinates
		  in the present resolution. Then convert these coordinates to
		  signed decimal degrees.*/

		pixLat = (PGSt_integer)(i / numColsRegion) + cornerRowPixels[0];
		pixLon = (i % numColsRegion) + cornerColPixels[0];
		
		/*Check if fill point has wrapped across the international date
		  line */
		if (pixLon > maxHorizPixel)
		{
		    /*Wrapping condition. find the position east of 180 degrees
		      west (i.e. 0 in global pixels) */
		
		    pixLon = pixLon - maxHorizPixel - 1;
		}
		
		pntLatitude = PGSm_DEM_PixelToLat(pixLat, subsetInfo);
		pntLongitude = PGSm_DEM_PixelToLon(pixLon, subsetInfo);
		
		subgridNext = PGSm_DEM_LatLonToSubgrid(pntLatitude,
						       pntLongitude, 
						       subsetInfoNext);
		
		/*check if one has more fill points than is available in
		  the PointRecord array. If there isn't enough space in the
		  array, realloc the dataPointsPix and realloc the
		  indicesArray*/ 
		if (numFillPoints == arrayLength)
		{
		    arrayLength += (totalNumPixels/4);
		    
		    dataPointsDeg = (PGSt_DEM_PointRecordDeg **) realloc(
                                           (void *)dataPointsDeg, arrayLength *
					   sizeof(PGSt_DEM_PointRecordDeg *));
		    if (dataPointsDeg == NULL)
		    {
			/*ERROR callocing*/
			sprintf(dynamicMsg,"Error reallocating space... "
				"for the dataPointsDeg");
			PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
						  dynamicMsg,
						  "PGS_DEM_ReplaceFillPointsInt16()");
			statusError = PGSMEM_E_NO_MEMORY;
			
			/*fatal error*/
			goto fatalError;

		    }
		    indicesArray = (PGSt_integer *) realloc(
                                         (void *)indicesArray, arrayLength *
					 sizeof(PGSt_integer));
		    if (indicesArray == NULL)
		    {
			/*error callocing*/
			sprintf(dynamicMsg,"Error reallocating space... "
				"for the indicesArray");
			PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
						  dynamicMsg,
						  "PGS_DEM_ReplaceFillPointsInt16()");
			statusError = PGSMEM_E_NO_MEMORY;
			
			/*fatal error*/
			goto fatalError;
		    
		    }
		}
		
		/*calloc space for this PointRecord*/
		dataPointsDeg[numFillPoints - 1] = (PGSt_DEM_PointRecordDeg *) 
		  calloc(1, sizeof(PGSt_DEM_PointRecordDeg));
		if (dataPointsDeg[numFillPoints - 1] == NULL)
		{
		    /*ERROR callocing*/
		    sprintf(dynamicMsg,"Error allocating space... "
			    "for the dataPointsDeg[%d]", numFillPoints - 1);
		    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
						      dynamicMsg,
						      "PGS_DEM_ReplaceFillPointsInt16()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal error*/
		    goto fatalError;
		}
		

		/*add this fill point to the series of fill points in the
		  PointRecord array and iterate the indicesArray. Place the
		  origninal resolution in the data point record as well*/
		dataPointsDeg[numFillPoints -1] -> subgridValue = subgridNext;
		dataPointsDeg[numFillPoints -1] -> resolutionOrig = 
		  resolutionList[1];
		dataPointsDeg[numFillPoints -1] -> positionOrig[0] = 
		  pntLatitude;
		dataPointsDeg[numFillPoints -1] -> positionOrig[1] = 
		  pntLongitude;

		indicesArray[numFillPoints - 1] = numFillPoints - 1;
				  
	    }
	}

	/********************************/
	/*One case is that there are no fill values in the data, this is
	  simplest situation for this branch. At this point one can simply free
	  the data buffers and return SUCCESS*/
	/********************************/

	if (numFillPoints == 0)
	{
	    /*free data point array indices array*/
	    free(dataPointsDeg);
	    free(indicesArray);
	
	    /*no fill points in data, return success*/    
	    return(PGS_S_SUCCESS);
	}
	

	/********************************/
	/*On the other hand, if there are fill values detected, the function
	  attempts to interpolate non fill values for them. It reorders the fill
	  points by subgrid value (subgrids based on the next resolution in the
	  resolution list). Then it passes the reorganized indicesArray and the
	  PointRecord array to the Recursive Search function.  At this point,
	  the values are interpolated, based on the corresponding interpolation
	  method. */
	/********************************/
	   
	/*reorder these fill points by their respective subgrid value, recorded
	  in the PointRecord array.*/

	status = PGS_DEM_OrderIndicesSumDeg(indicesArray, numFillPoints,
					    dataPointsDeg, subsetNext,
					    &numSubgridCoverNext);
	if (status != PGS_S_SUCCESS)
	{
	    /*error in reorganizing the points by subset value*/
	}
	
	/*take the reordered indicesArray and attempt to interpolate the
	  fillpoints from the next resolution's data set*/

	/*NEAREST NEIGHBOR Interpolation*/
	if (interpolation == PGSd_DEM_NEAREST_NEIGHBOR)
	{
	    status = PGS_DEM_RecursiveSearchDeg((resolutionList + 1),
						numResolutions - 1,
						subsetInfoNext,
						subsetNext,
						dataPointsDeg,
						numSubgridCoverNext,
						indicesArray,
						numFillPoints);

	}

	/*BILINEAR interpolation*/
	else if (interpolation == PGSd_DEM_BILINEAR)
	{
	    	      
	    status = PGS_DEM_RecursiveSearchBil((resolutionList + 1),
						numResolutions - 1,
						subsetInfoNext,
						subsetNext,
						dataPointsDeg,
						numSubgridCoverNext,
						indicesArray,
						numFillPoints);
	    
	}

	/*look for fatal error*/
	if ((status == PGSDEM_E_CANNOT_ACCESS_DATA) || 
	    (status == PGSDEM_E_IMPROPER_TAG) ||
	    (status == PGSDEM_E_HDFEOS) ||
	    (status == PGSMEM_E_NO_MEMORY))
	{
	    statusError = status;
	    
	    /*fatal error*/
	    goto fatalError;
	}
	
	/*check if return is PGS_S_SUCCESS. If this is the return, need to
	  upgrade the return to PGSDEM_M_MULTIPLE_RESOLUTIONS_INCLUDED. A
	  successful return from RecursiveSearch only means that all of the fill
	  points were interpolated in the next resolution in the
	  resolutionList. On the other hand, the data in dataRegion now consists
	  of data constructed from multiple resolutions*/
	if (status == PGS_S_SUCCESS)
	{
	    status = PGSDEM_M_MULTIPLE_RESOLUTIONS;
	}
	
	/*record the status return for returning to PGS_DEM_GetRegion*/
	statusReturn = status;

    /*********************************************/    
    /*At this point we have interpolated as many fill points as was
      possible. Now all we have to do is rewrite the interpolated fill points to
      the dataRegion. */
    /*********************************************/    


	/*first we have to recalculate the original position interms of pixel
	  coordinates of the first resolution in the resolution list.  Then we
	  convert the point in the present coordinate system to position in the
	  data buffer dataRegion. Finally, we write the data value of that
	  particular point to the dataRegion.*/

	for (i = 0; i < numFillPoints; i++)
	{
	    
	    /*calculate position interms of the present resolution
	      (ie. resolutionList[0])*/ 

	    pixLat = PGSm_DEM_LatToPixel(dataPointsDeg[i] -> positionOrig[0],
					 subsetInfo);

	    pixLon = PGSm_DEM_LonToPixel(dataPointsDeg[i] -> positionOrig[1],
					 subsetInfo);

	    /*calculate the pixel offset from the origin of the dataRegion*/
	    pixLatOffset = pixLat - cornerRowPixels[0];
	    pixLonOffset = pixLon - cornerColPixels[0];

	    /*Check if point is wrapped across the international date line */
	    if (pixLonOffset < 0)
	    {
		/*wrapped condition*/
		pixLonOffset = pixLon + (maxHorizPixel - cornerColPixels[0] +
					 1);
	    }
	    
	    /*calculate the buffer position of this point*/
	    buffPosition = (pixLatOffset * numColsRegion) + pixLonOffset;
	    
	    /*transfer data to the data region*/
	    dataRegion[buffPosition] = dataPointsDeg[i] -> 
	      dataValue.TwoByteInt;
	}
	
	/*free the data point and the index arrays*/
	free(indicesArray);
	for (i = 0; i < numFillPoints; i++)
	{
	    free(dataPointsDeg[i]);
	}

	free(dataPointsDeg);
	
	/*return status from PGS_DEM_RecursiveSearch*/
	return(statusReturn);

fatalError:
    {
	/*need to free up all the data buffers that were intialized*/

	/*free the data point and the index arrays*/
	if (indicesArray != NULL)
	{
	    free(indicesArray);
	}
	
	if(dataPointsDeg != NULL)
	{
	    for (i = 0; i < numFillPoints; i++)
	    {
		if(dataPointsDeg[i] != NULL)
		{
		    free(dataPointsDeg[i]);
		}
		
	    }
	    free(dataPointsDeg);
	}
	
	/*return appropriate error status*/
	return(statusError);
	

    }
}
