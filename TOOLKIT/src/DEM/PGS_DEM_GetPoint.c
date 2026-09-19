/*********************************************************
PGS_DEM_GetPoint.c--

This API accesses the data sets initialized by PGS_DEM_Open(). The function is
passed an array of resolution tags.  It takes each point and retrieves the data
for that point from the first resolution. If that point is a fill value, it goes
to the next resolution in the list and attempts to get that point.  It continues
to "perculate" through lower resolutions, depending on the order of the
resolutions in the resolutionList, until a real data point is found or we
run out of resolutions.  This function behaves at a much more efficient level if
it is accessed with many position inputs (large value for numPoints) rather
than if it is placed in a loop. 

Author--
Alexis Zubrow

Dates--
February 19, 1997    AZ       First Programming
July 28, 1997        AZ       Added Bilinear Interpolation
Feb 26, 1998     Abe Taaheri  Correctd ammount of memory allocated for
                              each point, i.e. pixDataPoints and degDataPoints

*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>

PGSt_SMF_status 
PGS_DEM_GetPoint(
    PGSt_DEM_Tag resolutionList[],  /*list of resolutions to access*/ 
    PGSt_integer numResolutions,    /*number of resolutions in list*/
    PGSt_integer layer,             /*layer to be accessed*/
    PGSt_integer positionCode,      /*position format, pixels or degrees*/
    PGSt_double pntLatitude[],      /*latitude values of the points*/
    PGSt_double pntLongitude[],     /*longitude values of the points*/
    PGSt_integer numPoints,         /*number of points to access*/
    PGSt_integer interpolation,     /*interpolation flag*/
    void *interpValues)             /*buffer to return data for points*/
  
{
    
    /*return status*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    char errorBuf[PGS_SMF_MAX_MSG_SIZE]; /*Strings appended to error returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    char errorMsg[PGS_SMF_MAX_MSGBUF_SIZE];

    PGSt_integer i;                      /*looping index*/
    
    /*first resolution used for ordering the data by subgrid*/
    PGSt_DEM_Tag resolution;
    PGSt_DEM_SubsetRecord *subsetInfo;   /*first subset info for first
					   resolution and layer*/
    PGSt_DEM_FileRecord **subset;          /*first subset*/

    PGSt_DEM_SubsetRecord *subsetInfoNext;   /*subset info for second
					       resolution and layer*/
    PGSt_DEM_FileRecord **subsetNext;          /*second subset*/
    
    PGSt_DEM_PointRecordPix **pixDataPoints = NULL;  /*array of structs holding
						       data for each point,
						       pixel form*/
    PGSt_DEM_PointRecordDeg **degDataPoints = NULL;  /*array of structs holding
						       data for each point,
						       decimal degree form*/
    PGSt_integer *indicesArray = NULL;        /*array of indices for either
						piDataPoints or degDataPoints*/

    PGSt_integer numSubgridsCovered;          /*number of subgrids covered by
						the data points*/
    PGSt_integer maxVertPixel;         /*maximum pixel value, vertical (lat)
					 position*/ 
    PGSt_integer maxHorizPixel;        /*maximum pixel value, horizontal (lon)
					 position*/

    PGSt_double dataTemp;             /* temporarily holds data value.  Used for
					 bilinear interpolation */

    PGSt_integer numFillPoints = 0;   /*number of fill value points discovered,
					ofr bilinear interpolation */

    

    /**************
      ERROR TRAPPING
      **************/
    
    /*error trapping for improper number of resolutions or of points and for
      improper interpolation flags*/
    if (numResolutions <= 0)
    {
	/*ERROR improper number of resolutions*/
	sprintf(dynamicMsg, "(%d) is an improper number of resolutions.", 
		numResolutions);
	PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	sprintf(errorMsg, errorBuf, dynamicMsg);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
				       "PGS_DEM_GetPoint()");
	return(PGSDEM_E_IMPROPER_TAG);
    }

    if (numPoints <= 0)
    {
	/*ERROR improper number of resolutions*/
	sprintf(dynamicMsg, "(%d) is an improper number of points.", 
		numPoints);
	PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	sprintf(errorMsg, errorBuf, dynamicMsg);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
				       "PGS_DEM_GetPoint()");
	return(PGSDEM_E_IMPROPER_TAG);
    }

    if ((positionCode != PGSd_DEM_PIXEL) && (positionCode != PGSd_DEM_DEGREE))
    {
	/*ERROR improper positionCode*/
	sprintf(dynamicMsg, "(%d) is an improper positionCode.", 
		positionCode);
	PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	sprintf(errorMsg, errorBuf, dynamicMsg);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
				       "PGS_DEM_GetPoint()");
	return(PGSDEM_E_IMPROPER_TAG);
    }
  
    if ((interpolation != PGSd_DEM_NEAREST_NEIGHBOR) && (interpolation !=
						     PGSd_DEM_BILINEAR))
    {
	/*ERROR improper interpolation flag*/
	sprintf(dynamicMsg, "Improper Tag... (%d) is an improper "
		"interpolation flag.", interpolation);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					  "PGS_DEM_GetPoint()");

	return(PGSDEM_E_IMPROPER_TAG);
    }

    /*trapping for NULL pointers*/
    if (resolutionList == NULL)
    {
	/*ERROR  resolutionList is a NULL pointer*/
	sprintf(dynamicMsg, "Improper Tag... "
		"resolutionList equal to NULL");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
				       "PGS_DEM_GetPoint()");
	return(PGSDEM_E_IMPROPER_TAG);
    }
    

    /***********************
      ACTUALLY ACCESS DATA
      *********************/

    /*Get the subset and subsetInfo for the first resolution.  we need this for
      multiple reasons.  we need to calculate the subgrid value for each point.
      The subgrid value is a unique value for each file in a particular
      resolution. Secondly, this is part of the inputs to the functions which
      actually retrieve the data, PGS_DEM_RecursiveSearch...*/

    resolution = resolutionList[0];
    
    status = PGS_DEM_Subset(resolution, layer, PGSd_DEM_INFO, &subset,
			    &subsetInfo);
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR layer or resolution is not recoginized or not initialized*/
	sprintf(dynamicMsg, "resolution (%d), layer (%d) not recognized " 
		"or initialized", resolution, layer);
	PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	sprintf(errorMsg, errorBuf, dynamicMsg);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
				       "PGS_DEM_GetPoint()");
	return(PGSDEM_E_IMPROPER_TAG);
    }

    
    /*calloc space for the indicesArray*/
    indicesArray = (PGSt_integer *) calloc(numPoints, sizeof(PGSt_integer));
    if(indicesArray == NULL)
    {
	/*ERROR callocing space*/
	sprintf(dynamicMsg, "Error allocating memory for "
		"indicesArray");
	PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					  dynamicMsg, 
					  "PGS_DEM_GetPoint()");
	statusError = PGSMEM_E_NO_MEMORY;
	
	/*fatal ERROR*/
	goto fatalError;
    }
    

    /*calloc space for the array of PointRecords.  This array of structs will be
      passed to the various functions to keep track of additional information
      needed for accessing the data. Separate array for pixel inputs and decimal
      degree inputs*/ 
    switch(positionCode)
    {
	/*global pixel input*/
      case PGSd_DEM_PIXEL:
	/*calloc space for the data points array*/
	pixDataPoints = (PGSt_DEM_PointRecordPix **) calloc(numPoints, 
                                   sizeof(PGSt_DEM_PointRecordPix *));
	if (pixDataPoints == NULL)
	{
	    /*ERROR callocing space*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "pixDataPoints");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					      dynamicMsg, 
					      "PGS_DEM_GetPoint()");
	    statusError = PGSMEM_E_NO_MEMORY;
	    
	    /*fatal ERROR*/
	    goto fatalError;
	}
	for (i = 0; i < numPoints; i++)
	{
	    pixDataPoints[i] = (PGSt_DEM_PointRecordPix *) calloc(1, 
				      sizeof(PGSt_DEM_PointRecordPix));
	    if (pixDataPoints[i] == NULL)
	    {
		/*ERROR callocing space*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "pixDataPoints[%d]", i);
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					      dynamicMsg, 
					      "PGS_DEM_GetPoint()");
		statusError = PGSMEM_E_NO_MEMORY;
		
		/*fatal ERROR*/
		goto fatalError;
	    }
	}
	

 	/*we have to move the point locations into the array of PointRecords,
	  calculate each points respective subgrid, and record the original
	  resolution, ie. the first resolution in the resolutionList. */
	
	for(i = 0; i < numPoints; i++)
	{
	    pixDataPoints[i] -> positionOrig[0] = (PGSt_integer)pntLatitude[i];
	    pixDataPoints[i] -> positionOrig[1] = (PGSt_integer)pntLongitude[i];
	    pixDataPoints[i] -> resolutionOrig = resolutionList[0];
	    
	    /*calculate the subgrid*/
	    pixDataPoints[i] -> subgridValue =
	      PGSm_DEM_PixToSubgrid((pixDataPoints[i] -> positionOrig[0]),
				    (pixDataPoints[i] -> positionOrig[1]),
				    subsetInfo);

	    /*initialize indicesArray to the element number of this point*/
	    indicesArray[i] = i;

	    
	    /*calculate the largest possible pixel value for vertical and
	      horizontal. see if points are outside the extent of global pixel
	      coordinates system*/ 
	    maxVertPixel = ((subsetInfo -> vertPixSubgrid) * 
			    (subsetInfo -> subgridVert))  - 1;
	    maxHorizPixel = ((subsetInfo -> horizPixSubgrid) * 
			     (subsetInfo -> subgridHoriz))  - 1;
    
	    /*Check if pixel values are reasonable, ie. it is within the
	      geographic bounds of the world*/
	    if ((pixDataPoints[i] -> positionOrig[0] < 0) || 
		(pixDataPoints[i] -> positionOrig[0] > maxVertPixel) ||
		(pixDataPoints[i] -> positionOrig[1] < 0) ||
		(pixDataPoints[i] -> positionOrig[1] > maxHorizPixel))
	    {
		sprintf(dynamicMsg, "Cannot access data... " 
			"pntLatitude (%d) and pntLongitude (%d) "
			"are not valid geographic positions " 
			"for this resolution.",
			pixDataPoints[i] -> positionOrig[0],
			pixDataPoints[i] -> positionOrig[1]);
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					       dynamicMsg,
					       "PGS_DEM_GetPoint()");
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal ERROR*/
		goto fatalError;
		
	    }
	    
	}
	
	/*Order the indices of the array pixDataPoints based on the each points
	  respective subgrid number.  This function also determines the number
	  of subgrids the points cover.  In other words, it gives information
	  from how many files we will be accessing data.*/
	status = PGS_DEM_OrderIndicesSumPix(indicesArray, numPoints,
					    pixDataPoints, subset,
					    &numSubgridsCovered);
	if(status != PGS_S_SUCCESS)
	{
	    /*ERROR sorting indices by subgrid number*/
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					      "Cannot reorganize points based "
					      "on subgrid value", 
					      "PGS_DEM_GetPoint()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
	    /*fatal ERROR*/
	    goto fatalError;
		
	}
	

	/* At this point we need to check if the interpolation method is
	   bilinear.  In the case of bilinear interpolation, only search the
	   first resolution with nearest neighbor It does not make sense to
	   interpolate in the first resolution because the user already inputs
	   the points of interest in terms of the nearest neighbor.  If any of
	   these points were not able to be interpolated from this first
	   resolution, then one performs bilinear interpolation on the remaining
	   points.  First one locates all of the fill values.  Rerecord these
	   fill values, although this time using the array degDataPoints.
	   Re-order the fill value indices array.  Finally, pass this to the
	   bilinear interpolating function. */

	if ((interpolation == PGSd_DEM_NEAREST_NEIGHBOR) || 
	    (numResolutions == 1)) 
	{ 
	    /*Call the search function to retrieve the data points.  If the data
	      is not found at a particular resolution, the next resolution in
	      the resolutionList will be searched for that data point.*/

	    status = PGS_DEM_RecursiveSearchPix(resolutionList, numResolutions,
						subsetInfo, subset, 
						pixDataPoints, 
						numSubgridsCovered, 
						indicesArray, numPoints);
	}
	
	else if (interpolation == PGSd_DEM_BILINEAR)
	{

	    /* Bilinearr interpolation and more than 1 resolution*/

	    /*Call the search function to retrieve the data points.  If the data
	      is not found at a particular resolution, the next resolution in
	      the resolutionList will be searched by bilinear interpolation. */

	    status = PGS_DEM_RecursiveSearchPix(resolutionList, 1,
						subsetInfo, subset, 
						pixDataPoints, 
						numSubgridsCovered, 
						indicesArray, numPoints);
	    
	    /*Check for error return */
	    if ((status != PGS_S_SUCCESS) && 
		(status != PGSDEM_M_FILLVALUE_INCLUDED) &&
		(status != PGSDEM_M_MULTIPLE_RESOLUTIONS))
	    {
		/* Error extracting data from first resolution */
		sprintf(dynamicMsg, "Cannot access data..."
			"Problem extracting data from first resolution.");

		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						  dynamicMsg,
						  "PGS_DEM_GetPoint()");
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal ERROR*/
		goto fatalError;
	    }

	    /*If all values points were extracted from first resolution, then no
	      interpolation is necessary. Otherwise, need to populate a
	      degDataPoints with the fill values from the first search.
	      Reorganize this based on the subgrid values of the next
	      resolution. Take the number of fill values and pass the above
	      arrays to the bilinear search function. */

	    if (status != PGS_S_SUCCESS)
	    {
		/* Perform bilinear interpolation */


		/* calloc space for degDataPoints */
		degDataPoints = (PGSt_DEM_PointRecordDeg **) calloc(numPoints, 
				       sizeof(PGSt_DEM_PointRecordDeg *));
		if (degDataPoints == NULL)
		{
		    /*ERROR callocing space*/
		    sprintf(dynamicMsg, "Error allocating memory for "
			    "degDataPoints");
		    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						      dynamicMsg, 
						      "PGS_DEM_GetPoint()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal ERROR*/
		    goto fatalError;
		}
		for (i = 0; i < numPoints; i++)
		{
		    degDataPoints[i] = (PGSt_DEM_PointRecordDeg *) calloc(1, 
					      sizeof(PGSt_DEM_PointRecordDeg));
		    if (degDataPoints[i] == NULL)
		    {
			/*ERROR callocing space*/
			sprintf(dynamicMsg, "Error allocating memory for "
				"degDataPoints[%d]", i);
			PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
							  dynamicMsg, 
							  "PGS_DEM_GetPoint()");
			statusError = PGSMEM_E_NO_MEMORY;
			
			/*fatal ERROR*/
			goto fatalError;
		    }
		}
	
		/* Get subset Info and subset for next resolution */
		status = PGS_DEM_Subset(resolutionList[1], 
					   subsetInfo -> layer,
					   PGSd_DEM_INFO, &subsetNext,
					   &subsetInfoNext);
		if (status != PGS_S_SUCCESS)
		{
		    /*ERROR getting info and subset from next resolution*/
		    
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


		/* Locate fill points. If one finds a fill value, store its
		   position in degDataPoints (in decimal degree format). In
		   addition, one neads to update the indices array so that only
		   the indices of fill points are in the first n elements of
		   indices array. Finally, increment the number of fill values
		   to be interpolated. */

		for (i = 0; i < numPoints; i++)
		{
		    if (subsetInfo -> dataType == DFNT_INT8)
		    {
			dataTemp = pixDataPoints[i] -> dataValue.OneByteInt;
		    }
		    else if (subsetInfo -> dataType == DFNT_INT16)
		    {
		    	dataTemp = pixDataPoints[i] -> dataValue.TwoByteInt;
		    }
		    else if (subsetInfo -> dataType == DFNT_FLOAT32)
		    {
		    	dataTemp = pixDataPoints[i] -> dataValue.FourByteFlt;
		    }


		    if (dataTemp == subsetInfo -> fillvalue)
		    {
			
			/*found fill value. Record point info for next
			  resolution. */
		    
			degDataPoints[i] -> positionOrig[0] = 
			  PGSm_DEM_PixelToLat(pixDataPoints[i] -> 
					      positionOrig[0],
					      subsetInfo);
			
			degDataPoints[i] -> positionOrig[1] = 
			  PGSm_DEM_PixelToLon(pixDataPoints[i] -> 
					      positionOrig[1],
					      subsetInfo);

			
			degDataPoints[i] -> subgridValue =
			  PGSm_DEM_LatLonToSubgrid(degDataPoints[i] ->
						   positionOrig[0],
						   degDataPoints[i] ->
						   positionOrig[1],
						   subsetInfoNext);

			/* repopulate the indices array with indices of fill
			   values. Increment the number of fill values */
			indicesArray[numFillPoints] = i;
			
			numFillPoints ++;
		    }
		}
		
		/* Reorganize the indices array based on each points respective
		   subgrid number in the next resolution. Only reorganize the
		   fill point indices.*/
		status = PGS_DEM_OrderIndicesSumDeg(indicesArray, numFillPoints,
						    degDataPoints, subsetNext,
						    &numSubgridsCovered);
		if(status != PGS_S_SUCCESS)
		{
		    /*ERROR sorting indices by subgrid number*/
		    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					    "Cannot reorganize points based "
					    "on subgrid value", 
					    "PGS_DEM_GetPoint()");
		    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		    
		    /*fatal ERROR*/
		    goto fatalError;
		    
		}
	
		/*Perform bilinear interpolation on the fill points in the next
		  resolution. Increment resolution list, decriment number of
		  resolutions, pass in next resolution subsetInfo, pass in
		  reorganized indicesArray, and pass in the number of fill value
		  points If it does not find the data in the next resolution, it
		  will continue to increment through the resolutionList until it
		  interpolates every point or runs out of resolutions. */

		status = PGS_DEM_RecursiveSearchDeg(resolutionList + 1,
						    numResolutions - 1,
						    subsetInfoNext, 
						    subsetNext,
						    degDataPoints,
						    numSubgridsCovered,
						    indicesArray,
						    numFillPoints);
		if (status == PGS_S_SUCCESS)
		{
		    /* Points have actually been interpolated from multiple
		       resolutions, update status return. */

		    status = PGSDEM_M_MULTIPLE_RESOLUTIONS;
		}
	    }
	}
	
	break;

	/* Decimal degree input*/
      case PGSd_DEM_DEGREE:
	degDataPoints = (PGSt_DEM_PointRecordDeg **) calloc(numPoints, 
                                      sizeof(PGSt_DEM_PointRecordDeg *));
	if (degDataPoints == NULL)
	{
	    /*ERROR callocing space*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "degDataPoints");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					      dynamicMsg, 
					      "PGS_DEM_GetPoint()");
	    statusError = PGSMEM_E_NO_MEMORY;
	    
	    /*fatal ERROR*/
	    goto fatalError;
	}
	for (i = 0; i < numPoints; i++)
	{
	    degDataPoints[i] = (PGSt_DEM_PointRecordDeg *) calloc(1, 
				      sizeof(PGSt_DEM_PointRecordDeg));
	    if (degDataPoints[i] == NULL)
	    {
		/*ERROR callocing space*/
		sprintf(dynamicMsg, "Error allocating memory for "
			"degDataPoints[%d]", i);
		PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						  dynamicMsg, 
						  "PGS_DEM_GetPoint()");
		statusError = PGSMEM_E_NO_MEMORY;
		
		/*fatal ERROR*/
		goto fatalError;
	    }
	}
	

 	/*we have to move the point locations into the array of PointRecords,
	  calculate each points respective subgrid, and record the original
	  resolution, ie. the first resolution in the resolutionList. */

  	for(i = 0; i < numPoints; i++)
	{
	    degDataPoints[i] -> positionOrig[0] = pntLatitude[i];
	    degDataPoints[i] -> positionOrig[1] = pntLongitude[i];
	    degDataPoints[i] -> resolutionOrig = resolutionList[0];
	    
	    /*calculate the subgrid*/
	    degDataPoints[i] -> subgridValue =
	      PGSm_DEM_LatLonToSubgrid(pntLatitude[i], pntLongitude[i],
				    subsetInfo);

	    /*initialize indicesArray to the element number of this point*/
	    indicesArray[i] = i;
	    

	    /*Check if position values are reasonable, ie. it is within the
	      geographic bounds of the world*/
	    if ((degDataPoints[i] -> positionOrig[0] <= -90.0) || 
		(degDataPoints[i] -> positionOrig[0] > 90.0) ||
		(degDataPoints[i] -> positionOrig[1] < -180.0) ||
		(degDataPoints[i] -> positionOrig[1] >= 180.0))
	    {
		sprintf(dynamicMsg, "Cannot access data... " 
			"pntLatitude (%f) and pntLongitude (%f) "
			"are not valid geographic positions " 
			"for this resolution.",
			degDataPoints[i] -> positionOrig[0],
			degDataPoints[i] -> positionOrig[1]);
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					       dynamicMsg,
					       "PGS_DEM_GetPoint()");
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal ERROR*/
		goto fatalError;
		
	    }
	}

	/*Order the indices of the array degDataPoints based on the each points
	  respective subgrid number.  This function also determines the number
	  of subgrids the points cover.  In other words, it gives information
	  from how many files we will be accessing data.*/
	status = PGS_DEM_OrderIndicesSumDeg(indicesArray, numPoints,
						 degDataPoints, subset,
						 &numSubgridsCovered);
	if(status != PGS_S_SUCCESS)
	{
	    /*ERROR sorting indices by subgrid number*/
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					      "Cannot reorganize points based "
					      "on subgrid value", 
					      "PGS_DEM_GetPoint()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
	    /*fatal ERROR*/
	    goto fatalError;

	}
	

	/*Call the search function to retrieve the data points.  If the data is
	  not found at a particular resolution, the next resolution in the
	  resolutionList will be searched for that data point.  This process is
	  repeated until all of the data points have real data or until one runs
	  out of resolutions to search. Two methods of extracting data: nearest
	  neighbor and bilinear interpolation. */

	if (interpolation == PGSd_DEM_NEAREST_NEIGHBOR) 
	{ 
	    /* Nearest Neighbor interpolation*/
	    status = PGS_DEM_RecursiveSearchDeg(resolutionList, numResolutions,
						subsetInfo, subset,
						degDataPoints,
						numSubgridsCovered,
						indicesArray, numPoints); 
	}
	else if (interpolation == PGSd_DEM_BILINEAR)
	{
	    /* bilinear interpolation */
	    status = PGS_DEM_RecursiveSearchBil(resolutionList, numResolutions,
						subsetInfo, subset,
						degDataPoints,
						numSubgridsCovered,
						indicesArray, numPoints); 
	}

	break;
	
      default:
	/*ERROR improper position code*/
	sprintf(dynamicMsg, "position code (%d) is not recognized",
		positionCode);
	PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	sprintf(errorMsg, errorBuf, dynamicMsg);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
				       "PGS_DEM_GetPoint()");
	statusError = PGSDEM_E_IMPROPER_TAG;
		
	/*fatal ERROR*/
	goto fatalError;
    }

    /*Check the status return from _RecursiveSearch.  depending on the return,
      either write data to the output buffer, or return the appropriate error.
      The switch statement is necessary to access the appropriate data type for
      this particular layer*/

    if ((status == PGS_S_SUCCESS) || (status == PGSDEM_M_FILLVALUE_INCLUDED) ||
	(status == PGSDEM_M_MULTIPLE_RESOLUTIONS))
    {
	/*memcopy the appropriate data to the output buffer*/
	if (positionCode == PGSd_DEM_PIXEL)
	{
	    /*check if interpValue is NULL. if it is, just return the
	      appropriate status.  if it is not, memcpy the data into the
	      buffer.*/

	    if (interpValues != NULL)
	    {
		
		switch(subsetInfo -> dataType)
		{
		    /*one byte integer*/
		  case DFNT_INT8:

		    /*write the data to the buffer.  Need to do pointer
		      arithemetic on interpValues to increment for each
		      dataPoint.  The second paramater, the beast, is necessary
		      to get the address of the  particular data points' data
		      value*/ 
		
		    for (i = 0; i < numPoints; i++)
		    {
			memcpy(((int8 *)interpValues + i), 
			       (void *)&pixDataPoints[i] ->
			       dataValue.OneByteInt, sizeof(int8));
		    }
		
		    break;
		  case DFNT_INT16:

		    /*write the data to the buffer.  Need to do pointer
		      arithemetic on interpValues to increment for each
		      dataPoint.  The second paramater, the beast, is necessary
		      to get the address of the particular data points' data
		      value*/ 
		
		    for (i = 0; i < numPoints; i++)
		    {
			memcpy(((int16 *)interpValues + i), 
			       (void *)&pixDataPoints[i] -> 
			       dataValue.TwoByteInt, 
			       sizeof(int16));
		    }
		
		    break;
		  case DFNT_FLOAT32:

		    /*write the data to the buffer.  Need to do pointer
		      arithemetic on interpValues to increment for each
		      dataPoint.  The second paramater, the beast, is necessary
		      to get the address of the particular data points' data
		      value*/ 
		
		    for (i = 0; i < numPoints; i++)
		    {
			memcpy(((float32 *)interpValues + i), 
			       (void *)&pixDataPoints[i] -> 
			       dataValue.FourByteFlt, 
			       sizeof(float32));
		    }
		
		    break;
		
		  default:
		    /*ERROR data type not yet acceptable, has to be added to
		      code*/ 
		    sprintf(dynamicMsg, "Improper Tag... "
			    "Data type (%d) is not presently "
			    "accounted for in code",
			    subsetInfo -> dataType);
		    PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG,
						      dynamicMsg,
						      "PGS_DEM_GetPoint()");
		    statusError = PGSDEM_E_IMPROPER_TAG;
		
		    /*fatal error*/
		    goto fatalError;
		
		}
	    }
	    
	    /*free the data point array and return success*/
	    for (i = 0; i < numPoints; i++)
	    {
		free(pixDataPoints[i]);
	    }
	    free(pixDataPoints);
	    pixDataPoints = NULL;
	    
	    if (degDataPoints != NULL)
	    {
		/*also need to free degDataPoints */
		for (i = 0; i < numPoints; i++)
		{
		    free(degDataPoints[i]);
		}
		free(degDataPoints);
		degDataPoints = NULL;
		
	    }
	    
		    
	    /*free up the appropriate data points*/
	    free(indicesArray);
	    indicesArray = NULL;
	    
	    
	}
   
	
	
	/*memcopy the appropriate data to the output buffer*/
	else if (positionCode == PGSd_DEM_DEGREE)
	{
	    /*check if interpValue is NULL. if it is, just return the
	      appropriate status.  if it is not, memcpy the data into the
	      buffer.*/

	    if (interpValues != NULL)
	    {
		
		switch(subsetInfo -> dataType)
		{
		    /*one byte integer*/
		  case DFNT_INT8:
		    
		    /*write the data to the buffer.  Need to do pointer
		      arithemetic on interpValues to increment for each
		      dataPoint.  The second paramater, the beast, is necessary
		      to get the address of the particular data points' data
		      value*/ 
		    
		    for (i = 0; i < numPoints; i++)
		    {
			memcpy(((int8 *)interpValues + i), 
			       (void *)&degDataPoints[i] ->
			       dataValue.OneByteInt, sizeof(int8));
		    }
		
		    break;
		  case DFNT_INT16:
 
		    for (i = 0; i < numPoints; i++)
		    {
			memcpy(((int16 *)interpValues + i), 
			       (void *)&degDataPoints[i] -> 
			       dataValue.TwoByteInt, 
			       sizeof(int16));
		    }
		
		    break;
		  case DFNT_FLOAT32:
		
		    for (i = 0; i < numPoints; i++)
		    {
			memcpy(((float32 *)interpValues + i), 
			       (void *)&degDataPoints[i] ->
			       dataValue.FourByteFlt, 
			       sizeof(float32));
		    }
		
		    break;
		
		  default:
		    /*ERROR data type not acceptable*/
		    sprintf(dynamicMsg, "Improper Tag... "
			    "Data type (%d) is not presently "
			    "accounted for in code",
			    subsetInfo -> dataType);
		    PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG,
						      dynamicMsg,
						      "PGS_DEM_GetPoint()");
		    statusError = PGSDEM_E_IMPROPER_TAG;
		
		    /*fatal error*/
		    goto fatalError;
		}
	    }
	    
	    /*free the data point array */
	    for (i = 0; i < numPoints; i++)
	    {
		free(degDataPoints[i]);
	    }
	    free(degDataPoints);
	    degDataPoints = NULL;
	    	      
		 
	    /*free up the appropriate data points*/
	    free(indicesArray);
	    indicesArray = NULL;
	    
	}
    
   
	/*Now, Set and return the appropriate message return for successful
	  acquisition of data*/

	if (status == PGSDEM_M_FILLVALUE_INCLUDED)
	{
	    
	    /*Set message that fill values are in data*/
	    PGS_SMF_SetStaticMsg(PGSDEM_M_FILLVALUE_INCLUDED, 
					 "PGS_DEM_GetPoint()");

	}
	else if (status == PGSDEM_M_MULTIPLE_RESOLUTIONS)
	{
	    /*Set message that the data from multiple resolutions*/
	    PGS_SMF_SetStaticMsg(PGSDEM_M_MULTIPLE_RESOLUTIONS, 
					 "PGS_DEM_GetPoint()");

	}

	/*return the appropriate status*/
	return (status);
    }
    
	
    
    
    /*PROBLEM WITH Accessing the data. free up data arrays*/
    else if (status != PGS_S_SUCCESS)
    {
	if (status == PGSDEM_E_IMPROPER_TAG)
	{
	    /*Set dynamic message*/
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, 
					   "Improper resolution or layer", 
					   "PGS_DEM_GetPoint()");
	    statusError = PGSDEM_E_IMPROPER_TAG;
		
	    /*fatal error*/
	    goto fatalError;
	}
	else if (status == PGSDEM_E_CANNOT_ACCESS_DATA)
	{
	    /*ERROR can't access data*/
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					   "Cannot access data files", 
					   "PGS_DEM_GetPoint()");
	    statusError = PGSDEM_E_IMPROPER_TAG;
		
	    /*fatal error*/
	    goto fatalError;
	}
    }

    /*fatal error clean up all loose memory, return appropriate status*/
    
fatalError:
    {
	
	/*free up the appropriate data points*/
	if (indicesArray != NULL)
	{
	    free(indicesArray);
	}
	
	/*check if data array has been allocated*/
	if (pixDataPoints != NULL)
	{
	    for (i = 0; i < numPoints; i++)
	    {
		if(pixDataPoints[i] != NULL)
		{
		    free(pixDataPoints[i]);
		}
	    }
	    
	    free(pixDataPoints);
	    pixDataPoints = NULL;
	    
	}
    
	/*check if data array has been allocated*/
	if(degDataPoints != NULL)
	{
	    for (i = 0; i < numPoints; i++)
	    {
		
		if (degDataPoints[i] != NULL)
		{ 
		    free(degDataPoints[i]);
		}
	    }
	    free(degDataPoints);
	    degDataPoints = NULL;
	    
	}
	
	
	/*return the appropriate status return*/
	return(statusError);
    }
}

		   
		    
		 
		 


