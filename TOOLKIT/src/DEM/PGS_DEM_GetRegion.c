/*********************************************************
PGS_DEM_GetRegion.c--

This APi access the data sets intialized by PGS_DEM_Open().  The functionality
is based on array of resolution tags.  It takes the two points inputed, and uses
them to define a rectangular region in coordinates space.  It then attempts to
extract this region from the first resolution in the resolutionList.  In
actuality, it divides this region into subregions based on resolution specific
subgrids  (see PGS_DEM.h for explaination of subgrids).  Once the geographic
extent fo the subregions are determined, it extracts these areas from the
subgrid HDF-EOS GRIDS.  At this point, we have multiple data buffers, which when
assembled will cover the same geographic region as the inputed region. Before,
returning this re-assembled region to the user, we check for fill values.  Each
fill value is recorded in a array of PointRecords.  After all of the fill values
have been recorded, we do a RecursiveSearch on the next series of resolutions in
the resolution list. The interpolated values which are returned from the
RecursiveSearch are then added to the appropriate position in the particular
subregion.  If there is only one resolution in the resolutionList, then there
will not be necessary to do the RecursiveSearch step. After all the data that
can possible be extracted has been extracted, the regions are assembled and
memcpy'ed to the output buffer.


Author--
Alexis Zubrow

Dates--
First Programming              AZ 2/23/1997
Added wrapping capabilities    AZ 5/16/1997
12/8/1998      Abe Taaheri  Fixed problem with extracting region when the
                            region wraps across the 180 degrees east and
			    both upperleft corner and upperright corner of
			    the region (or lowerleft corner and lowerright 
			    corner) fall inside the same subgrid.
			    extractedRegions are now allocated twice the
			    memory allocated originally to avoid conflict when
			    two subregions fall on the same subgrid after and 
			    before wrapping
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>

PGSt_SMF_status 
PGS_DEM_GetRegion(
    PGSt_DEM_Tag resolutionList[],  /*list of resolutions to access*/ 
    PGSt_integer numResolutions,    /*number of resolutions in list*/
    PGSt_integer layer,             /*layer to be accessed*/
    PGSt_integer positionCode,      /*position format, pixels or degrees*/
    PGSt_integer interpolation,     /*interpolation flag*/
    PGSt_double latitude[],   /*latitude of upper left, and lower right pnts.*/
    PGSt_double longitude[],  /*longitude of upper left, and lower right pnts.*/
    void *dataRegion,               /*data buffer for output*/
    PGSt_double regionSize[2], /*size region (degrees) latitude and longitude*/
    PGSt_double firstElement[2],    /*coordinates of first pixel element*/
    PGSt_double pixelSize[2])       /*dimensions of a pixel, decimal degrees*/
  
{
    
    /*return status*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

    PGSt_integer i;                      /*looping index*/

    /*first resolution used for ordering the data by subgrid*/
    PGSt_DEM_SubsetRecord *subsetInfo;   /*first subset info for first
					   resolution and layer*/
    PGSt_DEM_FileRecord **subset;          /*first subset*/
    
    PGSt_DEM_RegionRecord **extractedRegions; /*array holding the data and
						diagnostics of each of
						subregions*/

    PGSt_integer crnRowPixel[2];    /* global pixel coordinates, upper and lower
				       pixels, respectively*/ 
    PGSt_integer crnColPixel[2];    /*farthest West and East,
				      respectively. position of the corners of
				      the data region*/
    
    PGSt_integer maxVertPixel;         /*maximum pixel value, vertical (lat)
					 position*/ 
    PGSt_integer maxHorizPixel;        /*maximum pixel value, horizontal (lon)
					 position*/
    
    PGSt_integer subgridUpprLft;       /*subgrid value of upper left pixel*/
    
    PGSt_integer extentSubgridsVert;   /*Number of subgrids vertically spanned
					 by the region*/
    PGSt_integer extentSubgridsHoriz;   /*Number of subgrids Horizontally 
					  spanned by the region*/
    

    /*records the corner points*/
    PGSt_double pntLatitude[2];       /*the latitude values of the corner pixels
					interms in  decimal degree format*/
    PGSt_double pntLongitude[2];
    

    /*some initializing*/
    extractedRegions = NULL;

    /**************
      ERROR TRAPPING
      **************/

    if (numResolutions <= 0)
    {
	/*ERROR improper number of resolutions*/
	sprintf(dynamicMsg, "Improper Tag... (%d) is an improper "
		"number of resolutions.", numResolutions);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					  "PGS_DEM_GetRegion()");
	
	return(PGSDEM_E_IMPROPER_TAG);
    }

    if ((positionCode != PGSd_DEM_PIXEL) && (positionCode != PGSd_DEM_DEGREE))
    {
	/*ERROR improper positionCode*/
	sprintf(dynamicMsg, "Improper Tag... (%d) is an improper "
		"positionCode.", positionCode);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					  "PGS_DEM_GetRegion()");

	return(PGSDEM_E_IMPROPER_TAG);
    }
  
    if ((interpolation != PGSd_DEM_NEAREST_NEIGHBOR) && (interpolation !=
						     PGSd_DEM_BILINEAR))
    {
	/*ERROR improper interpolation flag*/
	sprintf(dynamicMsg, "Improper Tag... (%d) is an improper "
		"interpolation flag.", interpolation);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					  "PGS_DEM_GetRegion()");

	return(PGSDEM_E_IMPROPER_TAG);
    }

    if (latitude == NULL)
    {
	/*ERROR insufficient information to access region*/

	return (PGSDEM_E_CANNOT_ACCESS_DATA);
    }
    
    if (longitude == NULL)
    {
	/*ERROR insufficient information to access region*/

	return (PGSDEM_E_CANNOT_ACCESS_DATA);
    }

    /*trapping for NULL pointers*/
    if (resolutionList == NULL)
    {
	/*ERROR  resolutionList is a NULL pointer*/
	sprintf(dynamicMsg, "Improper Tag... "
		"resolutionList equal to NULL");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
				       "PGS_DEM_GetRegion()");
	return(PGSDEM_E_IMPROPER_TAG);
    }
	


    /***********************
      ACTUALLY ACCESS DATA
      *********************/



    /*first we need to get the first subset and get diagnostic info on the
      subset*/

    status = PGS_DEM_Subset(resolutionList[0], layer, PGSd_DEM_INFO, &subset,
			    &subsetInfo);
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR accessing subset and subset info*/
	sprintf(dynamicMsg, "Cannot access data.. "
		"error attempting to get subset information "
		"for resolution (%d), layer (%d)",
		resolutionList[0], layer);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg, "PGS_DEM_GetRegion()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
	     
    }


    if (positionCode == PGSd_DEM_PIXEL)
    {
	/*convert corner points into latitude and longitude in decimal
	  degree format abnd calculate the regionSize interms of decimal
	  degrees*/ 
	
	crnRowPixel[0] = (PGSt_integer)latitude[0];
	crnRowPixel[1] = (PGSt_integer)latitude[1];
	crnColPixel[0] = (PGSt_integer)longitude[0];
	crnColPixel[1] = (PGSt_integer)longitude[1];
	    
    }

    else if (positionCode == PGSd_DEM_DEGREE)
    {
	/*convert corner points into latitude and longitude in decimal
	  degree format abnd calculate the regionSize interms of decimal
	  degrees*/ 
 
	crnRowPixel[0] = PGSm_DEM_LatToPixel(latitude[0], subsetInfo);
	crnRowPixel[1] = PGSm_DEM_LatToPixel(latitude[1], subsetInfo);
	crnColPixel[0] = PGSm_DEM_LonToPixel(longitude[0], subsetInfo);
	crnColPixel[1] = PGSm_DEM_LonToPixel(longitude[1], subsetInfo);
	    	    
    }

    /*calculate the corners of the regionSize interms of decimal degrees*/
    pntLatitude[0] = PGSm_DEM_PixelToLat(crnRowPixel[0], subsetInfo);
    pntLatitude[1] = PGSm_DEM_PixelToLat(crnRowPixel[1], subsetInfo);
    pntLongitude[0] = PGSm_DEM_PixelToLon(crnColPixel[0], subsetInfo);
    pntLongitude[1] = PGSm_DEM_PixelToLon(crnColPixel[1], subsetInfo);
	    
	
    /*calculate the size of the region in decimal degree format */   
    if (regionSize != NULL)
    {
	regionSize[0] = pntLatitude[0] - pntLatitude[1];

	/*Check to see if cross the 180E/W boundary*/
	if (crnColPixel[0] > crnColPixel[1])
	{
	    /*wrapping condition*/
	    regionSize[1] = (180.0 - pntLongitude[0]) + (pntLongitude[1] -
							 -180.0);
	}
	else
	{
	    
	    regionSize[1] = pntLongitude[1] - pntLongitude[0];
	}
    }
    

    /*the latitude and longitude, in signed decimal degree format, of the upper
      left pixel*/
    if (firstElement != NULL)
    {
	firstElement[0] = pntLatitude[0];
	firstElement[1] = pntLongitude[0];
    }
    

    /*the size of a pixel interms of decimal degrees.  first element is
      latitude, second is longitude*/
    if (pixelSize != NULL)
    {
	pixelSize[0] = 1.0/ (subsetInfo -> pixPerDegree);
	pixelSize[1] = 1.0/ (subsetInfo -> pixPerDegree);
    }
	    
    
   
    /*HERE, we do some more error checking based on the coordinates of the
      upperleft and lower right points*/
 
    /*Check if the input coordinates are reasonable (ie. within the extent
      of the world coordinate system), and that the order makes sense.
      Previously checked if either the northern/south boundaries or the
      east/west boundaries were unreasonable.  now, only check if the
      north/south boundaries are swwitched.  switching of the west/east extent
      of the region indicates wrapping across the 180East/180West boundary.
      Wrapping is now supported.*/
	
    if ((crnRowPixel[1] - crnRowPixel[0]) < 0)
    {
	/*ERROR wrong ordering of input coordinates*/
	sprintf(dynamicMsg, "Cannot access data... "
		"improper order of elements in " 
		"latitude of longitude inputs");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg,
					  "PGS_DEM_GetRegion()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	/*fatal error*/
	goto fatalError;
	
    }
    
    subgridUpprLft = PGSm_DEM_PixToSubgrid(crnRowPixel[0], crnColPixel[0],
					   subsetInfo);


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
					  "PGS_DEM_GetRegion()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	/*fatal error*/
	goto fatalError;

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
					  "PGS_DEM_GetRegion()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	/*fatal error*/
	goto fatalError;

    }
	
   
    /*If the dataRegion pointer is equal to NULL, this program assumes that all
      that should be returned is the diagnostic information, i.e. regionSize,
      firstElement, and pixelSize*/

    if (dataRegion == NULL)
    {
	
	/* everything has been completed successfully*/
	return (PGS_S_SUCCESS);
    }
    
    /*If one actually needs to extract data from the data sets, then there are a
      couple of steps we need to undertake.  First, we have to determine the
      requested region's span over which particular subgrids. Then we have to
      determine the particular span of the subregion over its corresponding
      subgrid.  At this point we can actually demarcate space for the subregions
      data buffer.  After we have extracted the data, we now can look for fill
      values*/ 


    /*calloc space for the RegionRecord array*/
    extractedRegions = (PGSt_DEM_RegionRecord **) calloc (
                               2*(subsetInfo -> numSubgrids), 
			       sizeof(PGSt_DEM_RegionRecord *));
    if (extractedRegions == NULL)
    {
	/*ERROR callocing*/
	sprintf(dynamicMsg,"Error allocating space... "
		"for the RegionRecord");
	PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
					  "PGS_DEM_GetRegion()");
	statusError = PGSMEM_E_NO_MEMORY;
	
	/*fatal error*/
	goto fatalError;
	
    }
    

    /*Determine the subgrids covered, and the coordinates of each subregion*/
    status = PGS_DEM_ExtentRegion(crnRowPixel, crnColPixel, subsetInfo,
				  extractedRegions, &extentSubgridsVert,
				  &extentSubgridsHoriz);
    
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR defining the subregions */
	sprintf(dynamicMsg, "Cannot access data... "
		"error defining the extent of the subregions");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg,
					  "PGS_DEM_GetRegion()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	/*fatal error*/
	goto fatalError;
	
    }

    /*Extract the subregions from their respective subgrids and writes the
      subregions to the output buffer*/
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
					  dynamicMsg,
					  "PGS_DEM_GetRegion()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	/*fatal error*/
	goto fatalError;
   
    }

    /*Replace any fill value points that were in the extracted region. If the
      number of resolutions in the resolutionList is one, this only determines
      if there are any fill values in the region extracted. If the number of
      resolutions is greater than 1, then this looks at the next resolution in
      the resolutionList and attempts to interpolate each fill value point.*/
    status = PGS_DEM_ReplaceFillPoints(resolutionList, numResolutions, layer,
				       interpolation, dataRegion, subsetInfo,
				       crnRowPixel, crnColPixel);

    /*Check for a fatal error*/
    if ((status == PGSDEM_E_CANNOT_ACCESS_DATA) || 
	(status == PGSDEM_E_IMPROPER_TAG) ||
	(status == PGSDEM_E_HDFEOS) ||
	(status == PGSMEM_E_NO_MEMORY))
    {
	/*ERROR in trying to replace fill values*/
	sprintf(dynamicMsg, "Cannot access data... "
		"error checking for fill value points "
		"and/or attempting to interpolate fill points");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg,
					  "PGS_DEM_GetRegion()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
	
    }
    
    else if (status == PGSDEM_M_MULTIPLE_RESOLUTIONS)
    {
	/*data in data region from multiple resolutions*/
	PGS_SMF_SetStaticMsg(PGSDEM_M_MULTIPLE_RESOLUTIONS, 
					 "PGS_DEM_GetRegion()");
    }
    
    else if (status == PGSDEM_M_FILLVALUE_INCLUDED)
    {
	/*data in data region still contains some fill points*/
	PGS_SMF_SetStaticMsg(PGSDEM_M_FILLVALUE_INCLUDED, 
					 "PGS_DEM_GetRegion()");
    }
    
    /*At this point, succesfully extracted the data into the user's data buffer,
      dataRegion*/

    /*free allocated space and return appropriate status*/
    for (i = 0; i < 2*(subsetInfo -> numSubgrids); i++)
    {
	if (extractedRegions[i] != NULL)
	{
	    free(extractedRegions[i]);
	}
    }
    free(extractedRegions);
      

    /*return status*/
    return(status);
    

    /*fatal errors.  Need to deallocate space and return appropriate error
      message*/ 

fatalError:
    {
	if (extractedRegions != NULL)
	{
	    for (i = 0; i < 2*(subsetInfo -> numSubgrids); i++)
	    {
		if (extractedRegions[i] != NULL)
		{
		    free(extractedRegions[i]);
		}
	    }
	    free(extractedRegions);
	}
	
	/*return error status*/
	return(statusError);
    }
}
