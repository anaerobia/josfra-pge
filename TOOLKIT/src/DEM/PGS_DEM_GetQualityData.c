/*********************************************************
PGS_DEM_GeQualityData.c--

This API extracts quality data from the DEM data files.  All of the quality data
will be included in every file of the data set.  The quality data consists of
quality data, source information, and geoid data presently.  The program first
must open up an apropriate subset.  This process is a bit adhoc. One needs to go
through the subset for each of the possible data layers to find an initialized
subset.  This is necessary to actually gain access to the HJDF-EOS files.  In
addition, we need "artificially" construct a subsetInfo for the source Quality
data.  This is used in the general data access, added to function
PGS_DEM_Subset. Once one has located an initialized subset, one opens the first
subgrid and accesses the quality data.


Author--
Alexis Zubrow

Dates--
May 8, 1997     AZ  First Programming  
July 7, 1997    AZ  Added PGS_DEM_AccessFile
July 8, 1999    SZ  Updated for the thread-safe functionality

*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>
#include <mfhdf.h>
#include <HdfEosDef.h>
#include <PGS_TSF.h>

PGSt_SMF_status 
PGS_DEM_GetQualityData(
    PGSt_DEM_Tag resolution,       /*resolution to access*/ 
    PGSt_integer qualityField,     /*field  to be accessed*/
    PGSt_integer positionCode,     /*position format, pixels or degrees*/
    PGSt_double latitude[2],  /*latitude of upper left, and lower right pnts.*/
    PGSt_double longitude[2], /*longitude of upper left, and lower right pnts.*/
    void *qualityData)             /*data buffer for output*/
  
{
    
    /*return status*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

    PGSt_integer i;                      /*looping index*/

    /*first resolution used for ordering the data by subgrid*/
    PGSt_integer possibleLayers[PGSd_DEM_MAX_LAYERS];  /* all the possible
							  layers of the data set
							  */ 

    PGSt_DEM_SubsetRecord *subsetInfo;   /*subset info for first
					   resolution and layer that is
					   initialized*/ 
    PGSt_DEM_FileRecord **subset;        /*first subset*/

    PGSt_DEM_SubsetRecord *subsetQualInfo;   /*subset info for quality/source/
					      geoid data */

    PGSt_integer crnRowPixel[2];         /* global pixel coordinates, upper and
					    lower pixels, respectively*/ 
    PGSt_integer crnColPixel[2];         /*farthest West and East,
					   respectively. position of the corners
					   of the data region*/
    
    PGSt_integer maxVertPixel;           /*maximum pixel value, vertical (lat)
					   position*/ 
    PGSt_integer maxHorizPixel;          /*maximum pixel value, horizontal (lon)
					   position*/
    PGSt_integer numRowsRegion;       /*Number rows spanning the region*/
    PGSt_integer numColsRegion;       /*Number columns spanning the region*/ 
 
    /*HDF-EOS interface*/
    intn statusHdf;       /*status returns for HDF-EOS*/
    int32 gdID;           /*GRID ID*/
    int32 hdfID;          /*File Handle*/
    int32 start[2];       /*start of the subregion (internal pixels), first
			    element is vertical offset, second is horizontal*/
    int32 edge[2];        /*number pixels to extract*/
    int32 *stride;        /*number of values to skip along each dimension*/
    
#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif

    /*************
      initializing
      *************/

    subsetInfo = NULL;
    subset  = NULL;
    subsetQualInfo = NULL;

    /*initialize each possible layer*/    
    possibleLayers[0] = PGSd_DEM_ELEV;
    possibleLayers[1] = PGSd_DEM_STDEV_ELEV;
    possibleLayers[2] = PGSd_DEM_SLOPE;
    possibleLayers[3] = PGSd_DEM_STDEV_SLOPE;
    possibleLayers[4] = PGSd_DEM_ASPECT;
    possibleLayers[5] = PGSd_DEM_TOP_OBSC;
    possibleLayers[6] = PGSd_DEM_TOP_SHAD;
    possibleLayers[7] = PGSd_DEM_WATER_LAND;

    /*initialize hdfId and gdID to be closed or unattached*/
    hdfID = PGSd_DEM_HDF_CLOSE;
    gdID = PGSd_DEM_HDF_CLOSE;
 

    /**************
      ERROR TRAPPING
      **************/

    if ((positionCode != PGSd_DEM_PIXEL) && (positionCode != PGSd_DEM_DEGREE))
    {
	/*ERROR improper positionCode*/
	sprintf(dynamicMsg, "Improper Tag... (%d) is an improper "
		"positionCode.", positionCode);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					  "PGS_DEM_GetQualityData()");

	return(PGSDEM_E_IMPROPER_TAG);
    }
  


    /***********************
      ACTUALLY ACCESS DATA
      *********************/


    /*first we need to get the first subset which has been initialized for
      diagnostic info on the subset.  What we do is loop through each of the
      possible layers until we find an initialized layer.  Once we find an
      initialized layer, we jump out of the loop.  If we are not able to find
      and initialized layer in this resolution, then return and error*/

    for (i = 0; i < PGSd_DEM_MAX_LAYERS; i++)
    {
	
	status = PGS_DEM_Subset(resolution, possibleLayers[i], PGSd_DEM_INFO,
				&subset, &subsetInfo);
	/*check to see if found initialized layer, if so leave loop */
	if (status == PGS_S_SUCCESS)
	{
	    i = PGSd_DEM_MAX_LAYERS;
	}
    }
    
    /*Check to see if successfuly located an initialized layer*/
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR accessing subset and subset info*/
	sprintf(dynamicMsg, "Cannot access data.. "
		"error attempting to get subset information "
		"for resolution (%d), no initiialized layers located",
		resolution); 
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg,
					  "PGS_DEM_GetQualityData()"); 
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
	     
    }
	
    /* Now initialize the information needed for accessing quality/source/geoid
       data. REMEMBER to CLOSE the Quality info when finshed using*/
    status = PGS_DEM_Subset(resolution, qualityField, PGSd_DEM_QUALITYINFO,
			    NULL, &subsetQualInfo);
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR accessing quality/source/geoidt info*/
	sprintf(dynamicMsg, "Cannot access data.. "
		"error attempting to get quality information "
		"for resolution (%d), qualityField (%d),", resolution,
		qualityField);  
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg,
					  "PGS_DEM_GetQualityData()"); 
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
	     
    }

    /*convert corner points into latitude and longitude in pixel format*/

    if (positionCode == PGSd_DEM_PIXEL)
    {
	
	crnRowPixel[0] = (PGSt_integer)latitude[0];
	crnRowPixel[1] = (PGSt_integer)latitude[1];
	crnColPixel[0] = (PGSt_integer)longitude[0];
	crnColPixel[1] = (PGSt_integer)longitude[1];
	    
    }

    else if (positionCode == PGSd_DEM_DEGREE)
    {
	
	crnRowPixel[0] = PGSm_DEM_LatToPixel(latitude[0], subsetQualInfo);
	crnRowPixel[1] = PGSm_DEM_LatToPixel(latitude[1], subsetQualInfo);
	crnColPixel[0] = PGSm_DEM_LonToPixel(longitude[0], subsetQualInfo);
	crnColPixel[1] = PGSm_DEM_LonToPixel(longitude[1], subsetQualInfo);
	    	    
    }

    /*HERE, we do some more error checking based on the coordinates of the
      upperleft and lower right points*/
 
    /*Check if the input coordinates are reasonable (ie. within the extent
      of the world coordinate system), and that the order makes sense*/
	
    if (((crnRowPixel[1] - crnRowPixel[0]) < 0) || ((crnColPixel[1] -
						     crnColPixel[0]) < 0))  
    {
	/*ERROR wrong ordering of input coordinates*/
	sprintf(dynamicMsg, "Cannot access data... "
		"improper order of elements in " 
		"latitude of longitude inputs");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg,
					  "PGS_DEM_GetQualityData()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	/*fatal error*/
	goto fatalError;
	
    }
    

    /*calculate the largest possible pixel value for vertical and
      horizontal. see if points are outside the extent of global pixel
      coordinates system*/ 
    maxVertPixel = ((subsetQualInfo -> vertPixSubgrid) * 
		    (subsetQualInfo -> subgridVert))  - 1;
    maxHorizPixel = ((subsetQualInfo -> horizPixSubgrid) * 
		     (subsetQualInfo -> subgridHoriz))  - 1;
    
    
    if ((crnRowPixel[0] < 0) || (crnRowPixel[0] > maxVertPixel)
	|| (crnColPixel[0] < 0) || (crnColPixel[0] > maxHorizPixel))
    {
	/*ERROR upper left point is beyond extent of the world coordinates*/
	sprintf(dynamicMsg, "Cannot access data... "
		"Coordinates of upper left point are beyond scope "
		"of the coordinates system.");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg,
					  "PGS_DEM_GetQualityData()");
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
					  "PGS_DEM_GetQualityData()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	/*fatal error*/
	goto fatalError;

    }
	
 	
    /*If we have reached this point without any errors, we are now able to
      extract the quality, source or geoid data.  First we access the first
      subgrid of the initialized subset.  This file contains two HDF-EOS grids.
      The first is the DEM data, the second is the Grid for all the
      source/quality/geoid data.  We access the relevant data from this second
      grid. Unlike extracting data from the layers, the pixel values
      already calculated (for the quality/source/geoid data) ARE VALID within
      the internal coordinate system of the HDF-EOS grid.*/

   
    /*Check to see that the first subgrid file is properly staged*/

    if (subset[subsetInfo -> firstSubgrid] == NULL)
    {
	/*ERROR file not staged*/
	sprintf(dynamicMsg, "Cannot access data... "
		"Subgrid (%d), not properly staged. Cannot access"
		"quality/source/geoid data", subsetInfo -> firstSubgrid); 
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg, 
						 "PGS_DEM_GetQualityData()");
	
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
				
	/*fatal error*/
	goto fatalError;
    }
	    
    /*calculate the number of pixels horizontally and vertically
      spanning the region*/
    numColsRegion = crnColPixel[1] - crnColPixel[0] + 1;
    
    numRowsRegion = crnRowPixel[1] - crnRowPixel[0] + 1;
    

    /*Initialize the parameters for extracting the subregion-- use
      GDreadfield*/ 
    
    start[0] = crnRowPixel[0];
    start[1] = crnColPixel[0];
    
    edge[0] = numRowsRegion;	    
    edge[1] = numColsRegion;
    stride = NULL;
	     

    /* Access the File tag, hdfID.  If the ID are not available, 
       open the particular file.  Look at first subgrid. */

    hdfID = subset[subsetInfo -> firstSubgrid] -> hdfID; 
    if (hdfID == PGSd_DEM_HDF_CLOSE)
    {
	/*HDF-EOS file needs to be opened and attached to */
	status = PGS_DEM_AccessFile(subsetInfo, subset, 
				    subsetInfo -> firstSubgrid,
				    PGSd_DEM_OPEN);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR Opening and accessing data*/
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					      "Cannot access and attach to file"
					      "PGS_DEM_AccessFile failed.",
					      "PGS_DEM_GetQualityData()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;	    
	    
	    /*fatal error*/
	    goto fatalError;
	}
    }


    /*notice, grid name and field name are taken from subsetQualInfo, NOT
      subsetInfo. Therefore, need a separate GDattach call.  Not the same 
      GRID which is accessed through PGS_DEM_AccessFile. */

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
    
    gdID = GDattach(hdfID, (subsetQualInfo -> gridName));

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

    if (gdID == FAIL)
    {
	/*ERROR attaching to the GRID*/
	statusError = PGSDEM_E_HDFEOS;
	
	/*fatal Error*/
	goto fatalError;
	
    }

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    /*actually read data*/
    statusHdf = GDreadfield(gdID, subsetQualInfo -> fieldName, start, stride,
			    edge, (void *)qualityData);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

    if (statusHdf == FAIL)
    {
	/*ERROR extracting data*/
	sprintf(dynamicMsg, "Cannot access the data... "
		"GDreadfield failed on subgrid (%d) of subset (%d) "
		"(or PCF logical ID). Problem reading " 
		"source/quality/geoid data.",
	        subsetInfo -> firstSubgrid, subsetInfo -> subset);
	
	  PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
				dynamicMsg,
				"PGS_DEM_GetQualityData()");
	
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
    }
    
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    /*close and detach from HDF-EOS file*/
    statusHdf = GDdetach(gdID);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

    if (statusHdf == FAIL)
    {
	/*ERROR detaching from grid*/
	statusError = PGSDEM_E_HDFEOS;
	
	/*fatal error*/
	goto fatalError;
	
    }
    gdID = PGSd_DEM_HDF_CLOSE;


    /*Close the quality fields' subsetInfo.  DO NOT use for normal data layers*/
    status = PGS_DEM_Subset(resolution, qualityField, PGSd_DEM_QUALITYCLOSE,
			    NULL, &subsetQualInfo);
			    
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR accessing quality/source/geoidt info*/
	sprintf(dynamicMsg, "Cannot access data.. "
		"error attempting to close quality information "
		"for resolution (%d), qualityField (%d),", resolution,
		qualityField);  
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg,
					  "PGS_DEM_GetQualityData()"); 
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
	     
    }
    subsetQualInfo = NULL;
    

    /*succesful return*/
    return(PGS_S_SUCCESS);
    

    /*fatal errors need to close down HDF-EOS session and close the
      subsetQualInfo */

fatalError:
    {
	
	/*close quality subsetInfo*/
	if(subsetQualInfo != NULL)
	{
	    status = PGS_DEM_Subset(resolution, qualityField,
				    PGSd_DEM_QUALITYCLOSE, NULL,
				    &subsetQualInfo); 
	}
	
	    
	/*close any open HDF-EOS sessions-- Close only the Quality GRIDS */
	if (gdID != PGSd_DEM_HDF_CLOSE)
	{

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	    GDdetach(gdID);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	}
	
	/*Return appropriate error status*/
	return(statusError);
	
    }
    
    
}
