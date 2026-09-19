/*******************************************************
PGS_DEM_GetMetadata.c--

This tool accesses the general, collection level metadata, which pertains to a
specific resolution and layer.  Presently, all of this metadata is stored in the
Archive level of the granule, but it will eventually be moved to collection
level metadata.  When the metadata's location and "consistency" changes, this
function may need to be updated.


Author -- 
Alexis Zubrow/ARC
Abe Taaheri

history --
April 10, 1997    AZ    first created    
July 7, 1997      AZ    Added PGS_DEM_AccessFile
March 24 1999     AT    Modified so that one can get metadata for
                        elevaton, slope, aspect, std dev elev, and 
                        std dev slope, and land/water.
08-July-99        SZ    Updated for the thread-safe functionality

*******************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <PGS_DEM.h>
#include <PGS_PC.h>
#include <PGS_MET.h>
#include <mfhdf.h>
#include <HdfEosDef.h>
#include <PGS_TSF.h>

PGSt_SMF_status
PGS_DEM_GetMetadata(
    PGSt_DEM_Tag resolution,        /*resolution to access*/ 
    PGSt_integer layer,             /*layer to be accessed*/
    PGSt_double pixLatInfo[2],      /*Pixel diagnostics, latitudinal*/
    PGSt_double pixLonInfo[2],      /*Pixel diagnostics, longitudinal*/
    char positionUnits[],             /*Units of possition coordinates*/
    PGSt_double *scaling,           /*Scaling factor for data*/
    PGSt_double *offset,            /*Offset of scaled data*/
    PGSt_double *fillValue,         /*fill value, no data*/
    char units[],                    /*Units of field data*/
    PGSt_integer *mapProjection,    /*Projection of data*/
    PGSt_boolean *qualityAssurLayer) /*Existance or absence, quality layer*/
  
{
    /*return status*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    PGSt_integer temp_layer = -1;

    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    
    PGSt_DEM_SubsetRecord *subsetInfo;   /*subset info for resolution and 
					   layer*/
    PGSt_DEM_FileRecord **subset;        /*subset*/

    PGSt_integer subgridFirst;           /*first subgrid in subset*/
    
    /*HDF-EOS interface*/
    int32 hdfID = PGSd_DEM_HDF_CLOSE;
    int32 gdID = PGSd_DEM_HDF_CLOSE;
    int32 pixRegionCode;
    int32 numGrids;
    intn statusHDF;
    char YSizePixel_A[80];
    char XSizePixel_A[80];
    char Units_A[80];
    char Scaling_A[80];
    char Offset_A[80];
    char FillValue_A[80];
    char ZUnits_A[80];
    char Projection_A[80];

    /* Strings to hold metadata */
    char *positionUnitsString = NULL;
    char *dataUnitsString = NULL;
    char *mapProjectionString = NULL;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;          /* lock and unlock return */
#endif
    
    /**************************
      ACTUALLY ACCESS METADATA
      *************************/

    /*first we need to set attributes string for the layer */
    if (layer == PGSd_DEM_ELEV)
    {
	strcpy(YSizePixel_A,"YSizePixel.1");
	strcpy(XSizePixel_A,"XSizePixel.1");
	strcpy(Units_A,"Units.1");
	strcpy(Scaling_A,"Scaling.1");
	strcpy(Offset_A,"Offset.1");
	strcpy(FillValue_A,"FillValue.1");
	strcpy(ZUnits_A,"ZUnits.1");
	strcpy(Projection_A,"Projection.1");
	temp_layer = layer;
    }
    else if (layer == PGSd_DEM_SLOPE)
    {
	strcpy(YSizePixel_A,"YSizePixel.2");
	strcpy(XSizePixel_A,"XSizePixel.2");
	strcpy(Units_A,"Units.2");
	strcpy(Scaling_A,"Scaling.2");
	strcpy(Offset_A,"Offset.2");
	strcpy(FillValue_A,"FillValue.2");
	strcpy(ZUnits_A,"ZUnits.2");
	strcpy(Projection_A,"Projection.2");	
	temp_layer = layer;
    }
    else if (layer == PGSd_DEM_ASPECT)
    {
	strcpy(YSizePixel_A,"YSizePixel.3");
	strcpy(XSizePixel_A,"XSizePixel.3");
	strcpy(Units_A,"Units.3");
	strcpy(Scaling_A,"Scaling.3");
	strcpy(Offset_A,"Offset.3");
	strcpy(FillValue_A,"FillValue.3");
	strcpy(ZUnits_A,"ZUnits.3");
	strcpy(Projection_A,"Projection.3");	
	temp_layer = layer;
    }
    else if (layer == PGSd_DEM_STDEV_ELEV)
    {
	strcpy(YSizePixel_A,"YSizePixel.4");
	strcpy(XSizePixel_A,"XSizePixel.4");
	strcpy(Units_A,"Units.4");
	strcpy(Scaling_A,"Scaling.4");
	strcpy(Offset_A,"Offset.4");
	strcpy(FillValue_A,"FillValue.4");
	strcpy(ZUnits_A,"ZUnits.4");
	strcpy(Projection_A,"Projection.4");	
	temp_layer = layer;
    }
    else if (layer == PGSd_DEM_STDEV_SLOPE)
    {
	strcpy(YSizePixel_A,"YSizePixel.5");
	strcpy(XSizePixel_A,"XSizePixel.5");
	strcpy(Units_A,"Units.5");
	strcpy(Scaling_A,"Scaling.5");
	strcpy(Offset_A,"Offset.5");
	strcpy(FillValue_A,"FillValue.5");
	strcpy(ZUnits_A,"ZUnits.5");
	strcpy(Projection_A,"Projection.5");	
	temp_layer = layer;
    }
    else if (layer == PGSd_DEM_WATER_LAND)
    {
	/* for land/water there is no archive metadata in the hdf files. but
	   metadata for this layer is the same as those for elevation, 
	   except that ZUnit is CODE */
	strcpy(YSizePixel_A,"YSizePixel.1");
	strcpy(XSizePixel_A,"XSizePixel.1");
	strcpy(Units_A,"Units.1");
	strcpy(Scaling_A,"Scaling.1");
	strcpy(Offset_A,"Offset.1");
	strcpy(FillValue_A,"FillValue.1");
	strcpy(ZUnits_A,"ZUnits.1");
	strcpy(Projection_A,"Projection.1");
	temp_layer = PGSd_DEM_ELEV;
    }

    /*Next we need to get the first subset and get diagnostic info on the
      subset*/

    status = PGS_DEM_Subset(resolution, temp_layer, PGSd_DEM_INFO, &subset,
			    &subsetInfo);
    if (status != PGS_S_SUCCESS)
    {
	/*ERROR accessing subset and subset info*/
	sprintf(dynamicMsg, "Cannot access data..."
		"error attempting to get subset information "
		"for resolution (%d), layer (%d)",
		resolution, temp_layer);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg, "PGS_DEM_GetMetadata()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
	     
    }

    subgridFirst = subsetInfo -> firstSubgrid;
    
    /*Check to see that first subgrid is actually staged*/
    if (subset[subgridFirst] == NULL)
    {
	/*ERROR, subgrid not staged */
	sprintf(dynamicMsg, "Cannot access data..."
		"error attempting to access subgrid (%d),"
		"file not staged", subgridFirst);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					  dynamicMsg, "PGS_DEM_GetMetadata()");
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
	     
    }

    /* Access the File and GRID tags, hdfID and gdID.  If the ID's 
       are not available, open the particular file.  */
    
    hdfID = subset[subgridFirst] -> hdfID; 
    gdID = subset[subgridFirst] -> gdID;
    if ((hdfID == PGSd_DEM_HDF_CLOSE) || (gdID == PGSd_DEM_HDF_CLOSE))
    {
	/*HDF-EOS file needs to be opened and attached to */
	status = PGS_DEM_AccessFile(subsetInfo, subset, subgridFirst,
				    PGSd_DEM_OPEN);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR Opening and accessing data*/
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					      "Cannot access and attach to file"
					      "PGS_DEM_AccessFile failed.",
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;	    
	    
	    /*fatal error*/
	    goto fatalError;
	}
    }
   
    /*Access each of the metadata attributes. NOTE: This is the section which is
      very fluid. This section MUST correspond with the latest state of the
      metadata for the DEM data files.  One should also maintain consistency
      across the resolutions. */

    /*Check for each attribute, whether user passed in a NULL pointer.  Passing
      a NULL pointer will not result in an error, rather, this attribute will
      not be returned.*/

    /*Get PIXEL LEVEL metadata*/

    if (pixLatInfo != NULL)
    {
	status = PGS_MET_GetPCAttr(subsetInfo -> subset, 
				   subset[subgridFirst] -> version, 
				   "ArchiveMetadata", YSizePixel_A,
				   &pixLatInfo[0]);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR, accessing pixel metadata */
	    sprintf(dynamicMsg, "Cannot access data...."
		    "error accessing pixel metadata");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
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

	statusHDF = GDpixreginfo(gdID, &pixRegionCode);

#ifdef _PGS_THREADSAFE
       /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (statusHDF == FAIL)
	{
	    /*ERROR attaching to first HDFEOS GRID*/
	    sprintf(dynamicMsg, "HDFEOS error... "
		    "error accessing pixel location");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_HDFEOS;
	    
	    /*fatal error*/
	    goto fatalError;
	     
	}

	/*I'm assuming that there are only two possible location of the data in
	  the location: center or upper left corner.  If in future there are
	  more than these two locations, one will need to add a call to
	  GDorigininfo to fully determine the location*/
	if (pixRegionCode == HDFE_CORNER)
	{
	    pixLatInfo[1] = 0;
	}
	else if (pixRegionCode == HDFE_CENTER)
	{
	    pixLatInfo[1] = pixLatInfo[0]/2.0;
	}
	
    }
    
    if (pixLonInfo != NULL)
    {
	status = PGS_MET_GetPCAttr(subsetInfo -> subset, 
				   subset[subgridFirst] -> version, 
				   "ArchiveMetadata", XSizePixel_A,
				   &pixLonInfo[0]);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR, accessing pixel metadata */
	    sprintf(dynamicMsg, "Cannot access data..."
		    "error accessing pixel metadata");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
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

	statusHDF = GDpixreginfo(gdID, &pixRegionCode);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	if (statusHDF == FAIL)
	{
	    /*ERROR attaching to first HDFEOS GRID*/
	    sprintf(dynamicMsg, "HDFEOS error... "
		    "error accessing pixel location");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_HDFEOS;
	    
	    /*fatal error*/
	    goto fatalError;
	     
	}

	/*I'm assuming that there are only two possible location of the data in
	  the location: center or upper left corner.  If in future there are
	  more than these two locations, one will need to add a call to
	  GDorigininfo to fully determine the location*/
	if (pixRegionCode == HDFE_CORNER)
	{
	    pixLonInfo[1] = 0;
	}
	else if (pixRegionCode == HDFE_CENTER)
	{
	    pixLonInfo[1] = pixLonInfo[0]/2.0;
	}
	
    }
    

    /*Get POSITION UNITS metadata */

    if (positionUnits != NULL)
    {
	/*allocate space for string*/
	positionUnitsString = (char *) calloc(PGSd_DEM_STRING_MET, sizeof(char));
	if (positionUnitsString == NULL)
	{
	    /*ERROR callocing*/
	    PGS_SMF_SetStaticMsg(PGSMEM_E_NO_MEMORY, 
					 "PGS_DEM_GetMetadata()");
	    statusError = PGSMEM_E_NO_MEMORY;
	   	    
	    /*fatal error*/
	    goto fatalError;
	}
	
	status = PGS_MET_GetPCAttr(subsetInfo -> subset, 
				   subset[subgridFirst] -> version, 
				   "ArchiveMetadata", Units_A,
				   &positionUnitsString);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR, accessing pixel metadata */
	    sprintf(dynamicMsg, "Cannot access data..."
		    "error accessing position units metadata");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	     
	}

	strcpy(positionUnits, positionUnitsString);
	
    }

    /* Get SCALING metadata */
    if (scaling != NULL)
    {
	status = PGS_MET_GetPCAttr(subsetInfo -> subset, 
				   subset[subgridFirst] -> version, 
				   "ArchiveMetadata", Scaling_A,
				   scaling);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR, accessing pixel metadata */
	    sprintf(dynamicMsg, "Cannot access data..."
		    "error accessing scaling metadata");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	     
	}
    }
    
 
    /* Get Offset*/
    if (offset != NULL)
    {
	status = PGS_MET_GetPCAttr(subsetInfo -> subset, 
				   subset[subgridFirst] -> version, 
				   "ArchiveMetadata", Offset_A,
				   offset);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR, accessing pixel metadata */
	    sprintf(dynamicMsg, "Cannot access data..."
		    "error accessing offset metadata");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	     
	}
    }
    
    /* FILL VALUE */

    if (fillValue != NULL)
    {
	status = PGS_MET_GetPCAttr(subsetInfo -> subset, 
				   subset[subgridFirst] -> version, 
				   "ArchiveMetadata", FillValue_A,
				   fillValue);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR, accessing pixel metadata */
	    sprintf(dynamicMsg, "Cannot access data..."
		    "error accessing fill value metadata");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	     
	}
    }
	    
    /* UNITS */
    if (units != NULL)
    {
	dataUnitsString = (char *) calloc(PGSd_DEM_STRING_MET, sizeof(char));
	if (dataUnitsString == NULL)
	{
	    /*ERROR callocing*/
	    PGS_SMF_SetStaticMsg(PGSMEM_E_NO_MEMORY, 
					 "PGS_DEM_GetMetadata()");
	    statusError = PGSMEM_E_NO_MEMORY;
	   	    
	    /*fatal error*/
	    goto fatalError;
	}
	 
	status = PGS_MET_GetPCAttr(subsetInfo -> subset, 
				   subset[subgridFirst] -> version, 
				   "ArchiveMetadata", ZUnits_A,
				   &dataUnitsString);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR, accessing pixel metadata */
	    sprintf(dynamicMsg, "Cannot access data..."
		    "error accessing data units metadata");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	     
	}
	strcpy(units, dataUnitsString);
	
    }

    /* Type of PROJECTION */
    if (mapProjection != NULL)
    {
	
	/*allocate space for string*/
	mapProjectionString = (char *) calloc(PGSd_DEM_STRING_MET, sizeof(char));
	if (mapProjectionString == NULL)
	{
	    /*ERROR callocing*/
	    PGS_SMF_SetStaticMsg(PGSMEM_E_NO_MEMORY, 
					 "PGS_DEM_GetMetadata()");
	    statusError = PGSMEM_E_NO_MEMORY;
	   	    
	    /*fatal error*/
	    goto fatalError;
	     
	}
	status = PGS_MET_GetPCAttr(subsetInfo -> subset, 
				   subset[subgridFirst] -> version, 
				   "ArchiveMetadata", Projection_A,
				   &mapProjectionString);

	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR, accessing pixel metadata */
	    sprintf(dynamicMsg, "Cannot access data..."
		    "error accessing projection metadata");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	     
	}

	/*determine type of porjection and assign corresponding HDF-EOS code to
	  the output parameter mapProjection */
	/*presently, the only projection which is supported is GEOGRAPHIC*/

	if (strcmp(mapProjectionString, "GEOGRAPHIC") == 0)
	{
	    /* Geographic projection */
	    *mapProjection = GCTP_GEO;
	}
	else
	{

	    /*ERROR retrieved unexpected projection*/
	    sprintf(dynamicMsg, "Cannot access data..."
		    "able to retrieve projection metadata, "
		    "but unrecognized attribute (%s)", mapProjectionString);
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	    /*fatal error*/
	    goto fatalError;
	    
	}
    }
    
    /*Presence of QUALITY ASSURANCE LAYER */
    
    if (qualityAssurLayer != NULL)
    {
	/*Check to see if multiple grids in the file.  If there are, I assume
	  that these other grids are used for storiny Quality Assurance, Source,
	  or Geoid layers*/	

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	numGrids = GDinqgrid(subset[subgridFirst] -> filePath, NULL, NULL);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	if (numGrids == FAIL)
	{
	    /*ERROR accessing number of subgrids */
	    sprintf(dynamicMsg, "HDFEOS ERROR... "
		    "error accessing number of GRIDS in file");
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      dynamicMsg, 
					      "PGS_DEM_GetMetadata()");
	    statusError = PGSDEM_E_HDFEOS;
	    
	    /*fatal error*/
	    goto fatalError;
	     
	}
	    
	if (numGrids == 1)
	{
	    /* No quality assurance layer */
	    *qualityAssurLayer = PGS_FALSE;
	}
	else
	{
	    /*there is Quality assurance, source, and/or geoid layer*/
	    *qualityAssurLayer = PGS_TRUE;
	}
    }
    
                             
    /*At this point we successfuly extracted all metadata, clean up any open
      sessions and free allocated memory*/
    PGS_MET_Remove();

    if (positionUnitsString != NULL)
    {
	free(positionUnitsString);
    }
    
    if (dataUnitsString != NULL)
    {
	free(dataUnitsString);
    }
    
    if (mapProjectionString != NULL)
    {
	free(mapProjectionString);
    }


    /* if layer is water/land then units id code */
    if (layer == PGSd_DEM_WATER_LAND)
    {
	strcpy(units, "CODE");
    }
    
    /* Return SUCCESS */
    return(PGS_S_SUCCESS);
    

    /*Fatal Errors: clean up all allocated memory, close any sessions, return
      appropriate status*/

fatalError:
    {
	PGS_MET_Remove();
	
	if (positionUnitsString != NULL)
	{
	    free(positionUnitsString);
	}
	
	if (dataUnitsString != NULL)
	{
	    free(dataUnitsString);
	}
	
	if (mapProjectionString != NULL)
	{
	    free(mapProjectionString);
	}
	
	return(statusError);
    }
}
