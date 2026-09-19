/*********************************************************
PGS_DEM_GetSize.c--

This function queries a particular resolution and layer for pixel information.
It retrieves the size of one pixel.  In addition, it will allow the user to
calculate the number of pixels spanning a region.  This allows the user to
ignore all consideration of global pixel coordinates and differing size of
pixels, depending on resolution.  It allows the user to work completly in signed
decimal degree format.

Added wrapping capabilities.  If region requested overlaps the 180West/180East
boundary, can now access the data and stitch it into a continous region.


Author--
Alexis Zubrow

Dates--
3/3/1997   AZ  First Programming 
5/19/1997  AZ  Added wrapping capability
7/14/1997  AZ  Added new quality flags 

*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>

PGSt_SMF_status 
PGS_DEM_GetSize(
    PGSt_DEM_Tag resolution,        /*resolution to access*/ 
    PGSt_integer layer,             /*layer to be accessed*/
    PGSt_integer positionCode,      /*position format, pixels or degrees*/
    PGSt_double latitude[],   /*latitude of upper left, and lower right pnts.*/
    PGSt_double longitude[],  /*longitude of upper left, and lower right pnts.*/
    PGSt_integer *numPixVertical,   /*number pixels vertically spanning region*/
    PGSt_integer *numPixHorizontal, /*number horizontally spanning region*/
    PGSt_integer *sizeDataType)     /*size of a data pixel, number of bytes*/
  
{
    /*return status*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    
    PGSt_DEM_SubsetRecord *subsetInfo;   /*subset info for resolution and 
					   layer*/
    PGSt_DEM_FileRecord **subset;        /*subset*/

    PGSt_integer crnRowPixel[2];         /*global pixel coordinates, upper and
					   lower pixels, respectively*/ 
    PGSt_integer crnColPixel[2];         /*farthest West and East,
					   respectively. position of the corners
					   of the data region*/

    PGSt_integer maxVertPixel;           /*maximum pixel value, vertical (lat)
					   position*/ 
    PGSt_integer maxHorizPixel;          /*maximum pixel value, horizontal (lon)
					   position*/
    
    /**************
      ERROR TRAPPING
      **************/
    if ((positionCode != PGSd_DEM_PIXEL) && (positionCode != PGSd_DEM_DEGREE))
    {
	/*ERROR improper positionCode*/
	sprintf(dynamicMsg, "Improper Tag... (%d) is an improper "
		"positionCode.", positionCode);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					  "PGS_DEM_GetSize()");

	return(PGSDEM_E_IMPROPER_TAG);
    }
  

    /***********************
      ACTUALLY ACCESS INFO
      *********************/



    /*we need to get the subset and get diagnostic info on the
      subset*/

    /*if layer is one of the quality fields, special call to PGS_DEM_Subset() */
    if((layer == PGSd_DEM_GEOID) || (layer == PGSd_DEM_METHOD) || 
       (layer == PGSd_DEM_SOURCE) || (layer == PGSd_DEM_VERTICAL_ACCURACY) ||
       (layer == PGSd_DEM_HORIZONTAL_ACCURACY))
    {
	status = PGS_DEM_Subset(resolution, layer, PGSd_DEM_QUALITYINFO, 
				NULL, &subsetInfo);
    }
    else
    {
	status = PGS_DEM_Subset(resolution, layer, PGSd_DEM_INFO, &subset,
				&subsetInfo);
    }
    
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR accessing subset and subset info*/
	sprintf(dynamicMsg, "Cannot access data.. "
		"error attempting to get diagnostic information "
		"for resolution (%d), layer (%d)",
		resolution, layer);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg, "PGS_DEM_GetSize()");
	return(PGSDEM_E_CANNOT_ACCESS_DATA);
	     
    }

    /*return the data type of the layer. this should be the number of bytes in
      one pixel at this particular layer*/

    if (sizeDataType != NULL)
    {
	if (subsetInfo -> dataType == DFNT_INT8)
	{
	    *sizeDataType = sizeof(int8);
	}
	else if (subsetInfo -> dataType == DFNT_INT16)
	{
	    *sizeDataType = sizeof(int16);
	}
	if (subsetInfo -> dataType == DFNT_FLOAT32)
	{
	    *sizeDataType = sizeof(float32);
	}
    }
    
    /*if the latitude and longitude are valid, return the size of the region, in
      pixels*/

    if ((latitude != NULL) && (longitude != NULL))
    {
	
	if (positionCode == PGSd_DEM_PIXEL)
	{
	    
	    
	    crnRowPixel[0] = (PGSt_integer)latitude[0];
	    crnRowPixel[1] = (PGSt_integer)latitude[1];
	    crnColPixel[0] = (PGSt_integer)longitude[0];
	    crnColPixel[1] = (PGSt_integer)longitude[1];
	    
	}
	
	else if (positionCode == PGSd_DEM_DEGREE)
	{
	    /*convert corner points from latitude and longitude in decimal
	      degree format into global pixels*/
 
	    crnRowPixel[0] = PGSm_DEM_LatToPixel(latitude[0], subsetInfo);
	    crnRowPixel[1] = PGSm_DEM_LatToPixel(latitude[1], subsetInfo);
	    crnColPixel[0] = PGSm_DEM_LonToPixel(longitude[0], subsetInfo);
	    crnColPixel[1] = PGSm_DEM_LonToPixel(longitude[1], subsetInfo);
	    	    
	}

	/*Check if the input coordinates are reasonable (ie. within the extent
	  of the world coordinate system), and that the order makes sense*/
	
	if ((crnRowPixel[1] - crnRowPixel[0]) < 0)  
	{
	    /*ERROR wrong ordering of input coordinates*/
	    sprintf(dynamicMsg, "Cannot access data... "
		    "improper order of elements in " 
		    "latitude of longitude inputs");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg,
					      "PGS_DEM_GetSize()");
	    return(PGSDEM_E_CANNOT_ACCESS_DATA);
	    
	}
	
	/*calculate the largest possible pixel value for vertical and
	  horizontal. see if points are outside the extent of global pixel
	  coordinates system*/ 
	maxVertPixel = ((subsetInfo -> vertPixSubgrid) * 
			(subsetInfo -> subgridVert))  - 1;
	maxHorizPixel = ((subsetInfo -> horizPixSubgrid) * 
			 (subsetInfo -> subgridHoriz))  - 1;
	
    
	if ((crnRowPixel[0] < 0) || (crnRowPixel[0] > maxVertPixel)
	    || (crnColPixel[0] < 0) || (crnColPixel[0] > maxHorizPixel))
	{
	    /*ERROR upper left point is beyond extent of the world coordinates*/
	    sprintf(dynamicMsg, "Cannot access data... "
		    "Coordinates of upper left point are beyond scope "
		    "of the coordinates system.");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg,
					      "PGS_DEM_GetSize()");
	    return(PGSDEM_E_CANNOT_ACCESS_DATA);

	}
	
	if ((crnRowPixel[1] < 0) || (crnRowPixel[1] > maxVertPixel)
	    || (crnColPixel[1] < 0) || (crnColPixel[1] > maxHorizPixel))
	{
	    /*ERROR lower right point is beyond extent of world coordinates*/
	    sprintf(dynamicMsg, "Cannot access data... "
		    "Coordinates of the lower right point are beyond scope "
		    "of the coordinates system.");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg,
					      "PGS_DEM_GetSize()");
	    return(PGSDEM_E_CANNOT_ACCESS_DATA);
	    
	}
	
	/*if there are no problems with the the input latitudes and longitudes,
	  return size of region, in terms of pixels*/

	/*Check if region wraps across 180E/180W */
	if ((crnColPixel[1] - crnColPixel[0]) < 0)
	{
	    /*calculate number of pixels from upper left corner to Eastern
	      boundary (180E/W) plus the number of pixels from the Western
	      boundary (180E/W) to lower right corner*/    
	    *numPixHorizontal = (maxHorizPixel - crnColPixel[0] + 1) +
	      (crnColPixel[1] - 0 + 1);
	}
	else
	{
	    *numPixHorizontal = crnColPixel[1] - crnColPixel[0] + 1;
	}
	
	*numPixVertical = crnRowPixel[1] - crnRowPixel[0] + 1;
	
    }
    
    /*return success*/
    return(PGS_S_SUCCESS);
}






