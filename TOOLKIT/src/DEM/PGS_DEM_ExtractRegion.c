/*********************************************************
PGS_DEM_ExtractRegion.c--

This function actually extracts the data for the region.  It takes the array of
RegionRecords, one for each subregion, and opens each subgrid.  Then it uses the
HDF-EOS API's to extract the subregion into the buffer.  At this point in
memcpy's the region into the user's output buffer.  It continues to extract
subregions, and memcpy until all of the subregions have been extracted.  At this
point, the region has been copied into the user's data buffer.  The function
then returns to PGS_DEM_GetRegion.

Added wrapping capabilities.  If region requested overlaps the 180West/180East
boundary, can now access the data and stitch it into a continous region.

Author--
Alexis Zubrow/ARC
Abe Taaheri

Dates--
February 2, 1997     AZ   First Programming           
May 15, 1997         AZ   Added wrapping capability   
June 27, 1997        AZ   Efficiency Testing          
July 7, 1997         AZ   Added PGS_DEM_AccessFile
12/8/1998            AT   Fixed problem with extracting region when the
                          region wraps across the 180 degrees east and
			  both upperleft corner and upperright corner of
		          the region (or lowerleft corner and lowerright 
		          corner) fall inside the same subgrid.
3/24/1999            AT   Modified so that for data types of int16 and flt32
                          the data buffer is filled with fillvalues if data 
                          for layer 3arc reolution is not available. 
08-July-99           SZ   Updated for the thread-safe functionality
05-June-00           AT   Added functionality for 3km resolution
2010-10-06	  JK/AT   Changed to treat int8 the same way as int16 for
			  3arcsec layer ( the data buffer is filled with 
                          fillvalues if data for layer 3arc reolution is 
                          not available )

*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>
#include <PGS_MEM.h>
#include <mfhdf.h>
#include <HdfEosDef.h>
#include <PGS_TSF.h>

PGSt_SMF_status
PGS_DEM_ExtractRegion(
    PGSt_DEM_SubsetRecord *subsetInfo,        /*subset Information*/
    PGSt_DEM_FileRecord **subset,             /*subset*/
    PGSt_DEM_RegionRecord **extractedRegions, /*Array of subregions*/
    void *dataRegion,              /*User's output buffer for total region*/
    PGSt_integer subgridUpprLft,   /*subgrid value region's upper left corner*/
    PGSt_integer extentSubgridsVert,    /*number subgrids vertically spanned*/
    PGSt_integer extentSubgridsHoriz)  /*number subgrids horizontally spanned*/

{
    
      /*return status*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

    PGSt_integer i;                      /*looping index*/
    PGSt_integer j;                      /*looping index*/
    PGSt_integer k;                      /*looping index*/
    PGSt_integer num_temp_pix,fill_count;
    

    /*Offsets and dimensions of the subregions*/
    PGSt_integer presentSubgrid; /*subgrid actually analyzing*/

    PGSt_integer rowRegionOffset;   /*number columns spanning the
				      region. Offsets dataRegion by 1 row*/
    
    PGSt_integer subregionOffset;   /*Offset needed to find the beginning of the
				      subregion in the user's data buffer,
				      dataRegion*/ 
    PGSt_integer subregionBegin;    /*A second offset used for finding the
				      beginning of the subregion in
				      dataRegion. The difference between the two
				      is that subregionOffset is used for
				      finding the beginning based on vertical
				      offset, and subregionBegin, helps
				      calculate the beginning horizontal offset,
				      once one has already calculated the
				      verticual offset.*/

    PGSt_integer largestSubregion;  /*size of largest subregion, in pixels*/
    
    PGSt_integer numRowsSubregion;  /*Number rows spanning the subregion*/
    PGSt_integer numColsSubregion;  /*Number columns spanning the subregion*/
    PGSt_integer totalNumPixels; /*total number pixels in region*/

    /*information from subsetInfo*/
    PGSt_integer numSubgridsVert;  /*number subgrids vertically spanning world*/
				   

    /*Wrapping case, when the region wraps around from Eastern Hemisphere to
      Western Hemisphere. */
    PGSt_integer subgridWestBndry;     /*subgrid value of Western Boundary*/
    PGSt_integer subgridEastBndry;     /*subgrid value of Eastern Boundary,
					 these are the subgrids at the same
					 latitude as the upper left
					 subgrid. only important for wrapping
					 case */
   

    PGSt_integer maxHorizPixel;          /*maximum pixel value, horizontal (lon)
					   position*/

    /*HDF-EOS interface*/
    intn statusHdf;       /*status returns for HDF-EOS*/
    int32 gdID = PGSd_DEM_HDF_CLOSE;           /*GRID ID*/
    int32 hdfID = PGSd_DEM_HDF_CLOSE;          /*File Handle*/
    int32 start[2];       /*start of the subregion (internal pixels), first
			    element is vertical offset, second is horizontal*/
    int32 edge[2];        /*number pixels to extract*/
    int32 *stride;        /*number of values to skip along each dimension*/
    int32  current_resolution;    /*resolution tag from HDF-EOS attribute*/
    int32  layers_existvalue; /*layerDataExist tag from HDF-EOS attribute*/

    /*data buffers*/
    int8 *dataBuffInt8 = NULL;
    int16 *dataBuffInt16 = NULL;
    float32 *dataBuffFlt32 = NULL;
    /*pointer to switch dataRegionIn to int8, int16, float32 pntr. */
    int8 *dataRegion8;
    int16 *dataRegion16;
    float32 *dataRegion32;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif    
    
    /*Some initialization*/

    subregionOffset = 0;
    largestSubregion = 0;
    
    /*Initialize the subgrid diagnostics for this particular subset*/
    numSubgridsVert = subsetInfo -> subgridVert;


    /*if the horizontal coverage of the region is only over 1 subgrid, then we
      can use the user's data buffer as the buffer for GDreadfield.  This means
      we only have to do the simplest stitching of the subregions, which is only
      stitching a series of vertical subregions.  On the other hand, if there
      are multiple subregions across the horizontal extent of the region, the
      stitching procedure is more complicated. First, we'll do the simple case:
      extentSubgridsHoriz = 1.*/


    /*In this case, we do have to do some stitching, but it is still the
      simple case.  Here we use the user's own data buffer to store the
      different subregions.  The basic idea is that one fills the beginning
      of the data buffer with the first subregion.  Additional subregions 
      are appended at the end of the previous subregion. One keeps track of
      the subregion position within the data buffer with the variable
      "subregionOffset" */

    if (extentSubgridsHoriz == 1)
    {
        /*assign dataRegion to the address of dataRegionIn*/
	if (subsetInfo -> dataType == DFNT_INT8)
	{
	    dataRegion8 = (int8 *) dataRegion;
	}
	else if (subsetInfo -> dataType == DFNT_INT16)
	{
	    dataRegion16 = (int16 *) dataRegion;
	}
	else if (subsetInfo -> dataType == DFNT_FLOAT32)
	{ 
	    dataRegion32 = (float32 *) dataRegion;
	}
	/*calculate the dimensions of the dataRegion*/

	totalNumPixels = 0;
	
	for (j = 0; j < extentSubgridsVert; j++)
	{
	    presentSubgrid = subgridUpprLft + j;
	    /*Double check to see if RegionRecord has been calloced*/
	    if (extractedRegions[presentSubgrid] == NULL)
	    {
		/*ERROR was not calloced*/
		sprintf(dynamicMsg, "Cannot access data... "
			"subregion's data buffer for subgrid (%d) " 
			"not properly intitialized.", presentSubgrid);
		  PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					dynamicMsg, 
					"PGS_DEM_ExtractRegion()");
	    
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		    
		/*fatal error*/
		goto fatalError;
	    }
	    /*calculate the number of pixels horizontally and vertically
	      spanning the subregion*/
	    numColsSubregion = (extractedRegions[presentSubgrid] -> 
				colExtent[1] -
				extractedRegions[presentSubgrid] -> 
				colExtent[0] + 1);
		
	    numRowsSubregion = (extractedRegions[presentSubgrid] -> 
				rowExtent[1] -
				extractedRegions[presentSubgrid] -> 
				rowExtent[0] + 1);

	    totalNumPixels = totalNumPixels + numRowsSubregion *
	      numColsSubregion;
	}
	

	/*iterate through the vertical subgrids*/
	for (j = 0; j < extentSubgridsVert; j++)
	{
	    presentSubgrid = subgridUpprLft + j;
	
	    /*initialize hdfId and gdID to be closed or unattached*/
	    hdfID = PGSd_DEM_HDF_CLOSE;
	    gdID = PGSd_DEM_HDF_CLOSE;
	
	    /*Double check to see if RegionRecord has been calloced*/
	    if (extractedRegions[presentSubgrid] == NULL)
	    {
		/*ERROR was not calloced*/
		sprintf(dynamicMsg, "Cannot access data... "
			"subregion's data buffer for subgrid (%d) " 
			"not properly intitialized.", presentSubgrid);
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
                                      dynamicMsg,"PGS_DEM_ExtractRegion()");
	    
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		    
		/*fatal error*/
		goto fatalError;
	    }
	
	
	    /*Check to see that the file is properly staged*/
	
	    if (subset[presentSubgrid] == NULL)
	    {
		/*ERROR file not staged*/
		sprintf(dynamicMsg, "Cannot access data... "
			"Subgrid (%d), not properly staged", 
			presentSubgrid);
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
                                      dynamicMsg,"PGS_DEM_ExtractRegion()");
	    
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
		/*fatal error*/
		goto fatalError;
	    }
	    
	    /*calculate the number of pixels horizontally and vertically
	      spanning the subregion*/
	    numColsSubregion = (extractedRegions[presentSubgrid] -> 
				colExtent[1] -
				extractedRegions[presentSubgrid] -> 
				colExtent[0] + 1);
		
	    numRowsSubregion = (extractedRegions[presentSubgrid] -> 
				rowExtent[1] -
				extractedRegions[presentSubgrid] -> 
				rowExtent[0] + 1);
		
		
	    /*Initialize the parameters for extracting the subregion-- use
	      GDreadfield*/ 
		
	    start[0] = extractedRegions[presentSubgrid] -> rowExtent[0];
	    start[1] = extractedRegions[presentSubgrid] -> colExtent[0];
		
	    edge[0] = numRowsSubregion;	    
	    edge[1] = numColsSubregion;
	    stride = NULL;
		
				
		
	    /* Access the File and GRID tags, hdfID and gdID.  If the ID's 
	       are not available, open the particular file.  */

	    hdfID = subset[presentSubgrid] -> hdfID; 
	    gdID = subset[presentSubgrid] -> gdID;
	    if ((hdfID == PGSd_DEM_HDF_CLOSE) || (gdID == PGSd_DEM_HDF_CLOSE))
	    {
		/*HDF-EOS file needs to be opened and attached to */
		status = PGS_DEM_AccessFile(subsetInfo, subset, presentSubgrid,
					    PGSd_DEM_OPEN);
		if (status != PGS_S_SUCCESS)
		{
		    /*ERROR Opening and accessing data*/
		    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					    "Cannot access and attach to file"
					    "PGS_DEM_AccessFile failed.",
					    "PGS_DEM_ExtractRegion()");
		    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;	    
		    
		    /*fatal error*/
		    goto fatalError;
		}
	    }
	
	    /* get resolution and layers_existvalue from the file. 
	       layers_existvalue = 0 means that data for the layers elev, 
	       slope, aspect, land/water, std dev slope, and std dev elev
	       exist. layers_existvalue = 1 means that only data for 
	       land/water exist. */

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	    status = GDreadattr(gdID, "_resolution", &current_resolution);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	    if (status == FAIL)
	    {
		/*ERROR getting resolution tag attribute*/
		PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					  "Cannot access resolution attr.", 
					  "PGS_DEM_ExtractRegion()");
		statusError = PGSDEM_E_HDFEOS;
		
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

	    status = GDreadattr(gdID, "_layerDataExist", &layers_existvalue);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	    if (status == FAIL)
	    {
		/*ERROR getting layerDataExist tag attribute*/
		PGS_SMF_SetDynamicMsg(PGSDEM_W_HDFEOS,
		  "Cannot access layerDataExist attr. layers_existvalue will set to 0", 
						  "PGS_DEM_ExtractRegion()");

		/* old hdf files that had only elev and land/sea layers
		   do not have this flag. so if it fails 
		   to find this attribute, set the value to 0 */
		layers_existvalue = 0;
	    }

	    /*Read in the data, taking into account the particular 
	      offset for this subregion. This has to be data type specific*/

	    num_temp_pix = totalNumPixels;

	    if (subsetInfo -> dataType == DFNT_INT8)
	    {
		    
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

		statusHdf = GDreadfield(gdID, subsetInfo -> fieldName, 
					start, stride, edge, (void *) 
					(((int8 *)dataRegion8) + 
					 subregionOffset));
		/* only land/water layer is DFNT_INT8 type, for which 
		   data exist on all hdf files, 3ARC, 30Arc and 90ARC. So no 
		   need to check on data avaialability */

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	    }
	    else if (subsetInfo -> dataType == DFNT_INT16)
	    {
		if((layers_existvalue == 1 ) && (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataRegion16 with fillvalue */
		    
		    for (fill_count=0; fill_count<(edge[0]*edge[1]) ; fill_count++)
		    {    
			dataRegion16[fill_count+subregionOffset] = (subsetInfo -> fillvalue);
		    }
		    statusHdf = PGS_S_SUCCESS;
		}
		else
		{ 

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

		    statusHdf = GDreadfield(gdID, subsetInfo -> fieldName, 
					    start, stride, edge, (void *) 
					    (((int16 *)dataRegion16) + 
					     subregionOffset));

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		}
	    }
	    else if (subsetInfo -> dataType == DFNT_FLOAT32)
	    {
		if((layers_existvalue == 1 ) && (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataRegion32 with fillvalue */
		    for (fill_count=0; fill_count<(edge[0]*edge[1]) ; fill_count++)
		    {    
			dataRegion32[fill_count+subregionOffset] = (subsetInfo -> fillvalue);
		    }
		    statusHdf = PGS_S_SUCCESS;
		}
		else
		{ 

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

		    statusHdf = GDreadfield(gdID, subsetInfo -> fieldName, 
					    start, stride, edge, (void *) 
					    (((float32 *)dataRegion32) + 
					     subregionOffset));

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		}
	    }
		
	    if (statusHdf == FAIL)
	    {
		/*ERROR extracting data*/
		sprintf(dynamicMsg, "Cannot access the data... "
			"GDreadfield failed on subgrid (%d) " 
			"of subset (%d) (or PCF logical ID).",
			presentSubgrid, subsetInfo -> subset);
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
                                      dynamicMsg,"PGS_DEM_ExtractRegion()");
		    
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		    
		/*fatal error*/
		goto fatalError;
				
	    }
	    

	    /*iterate the subregion offset with the number of pixels in this
	      subregion*/ 
	    subregionOffset += extractedRegions[presentSubgrid] ->
	      sizeSubregion;
	}
	
	/*return success*/
	return(PGS_S_SUCCESS);
	
	
    }
    

    /*IF the horizontal span of the region covers more than 1 subgrid, more than
      1 subregion, then the problem is quite a bit more difficult.  First, we do
      a quick pass through each of the subregions to get some additional
      diagnostic information.  For example, we find out which subregion is the
      largest and the number of pixels horizontally spanning the region.  Second
      we malloc space for the largest subregion.  Third we loop through each
      subregion and extract the data for each subregion.  We iterate through
      in such a way as to read in a complete "row" of subregions.  In other
      words, a "row" of subregions is all the subregions which completly cover
      the West to East extent of the region. Fourth (and really simultaneous to
      third), when we  read in an individual subregion, we also memcpy it to the
      dataRegion. The memcpy is the tricky part of this whole procedure.  We
      want to copy the data into the buffer in such a way that geographic
      continuity is mantained. For example, the first n pixels of the
      dataRegion, where n equals the number of pixels horizontally spanning the
      region, should correspond to pixels which all have the same latitude. To
      accomplish this continuity, the subregions data buffer must be read one
      row at a time.  Then this row needs to written into the dataRegion to
      maintain its geographic positioning.  The result is a dataRegion
      consisting of interwoven rows of the subregions. After one has read a
      "row" of subregions, and written it to the user's data buffer, dataRegion,
      then the function iterates to the next "row" of subregions, until one has
      exhausted the extentSubgridsVert.  NOTE: For this functionality to work
      properly, one needs to access the subregions from West to East.  One needs
      to write one whole "row" of subregions before accessing the next "row" or
      the more Southerly subregions. To make matters slightly more complicated,
      accessing the horizontal subregions could possibly wrap around the
      180East/180West boundery.  If there is wrapping, then one must daeal with
      the fact that adjacent subregion's subgrid values will be vastly
      different.  I begin this section with a calculation of some diagnostics
      for the eventuality that the region wraps. */


     /*calculate the largest possible horizontal pixel value. */
    maxHorizPixel = ((subsetInfo -> horizPixSubgrid) * 
		     (subsetInfo -> subgridHoriz))  - 1;

    /*Wrapping condition.  Calculate the boundary subgrids at the same
      latitude of the Upper left subgrid. Double check if NW subgrid is
      staged. */ 

    if (subset[subgridUpprLft] == NULL)
    {
	/*ERROR file not staged*/
	sprintf(dynamicMsg, "Cannot access data... "
		"Subgrid (%d), not properly staged", subgridUpprLft);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, dynamicMsg, 
                              "PGS_DEM_ExtractRegion()");
	
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
    }

    subgridEastBndry = 
      PGSm_DEM_PixToSubgrid(subset[subgridUpprLft] ->  cornerRow[0], 
			    maxHorizPixel, subsetInfo);
    subgridWestBndry = 
      PGSm_DEM_PixToSubgrid(subset[subgridUpprLft] ->  cornerRow[0], 0,
			     subsetInfo);
   
    /*Quick pass for diagnostic information*/
    for (i = 0; i < extentSubgridsVert; i++)
    {
	rowRegionOffset = 0;

	/*Calculate the left most subgrid of one's region*/
	presentSubgrid = subgridUpprLft + i;
	
	for (j = 0; j < extentSubgridsHoriz; j++)
	{
	     
	    /*Double check to see if RegionRecord has been calloced*/
	    if(((j+1) == extentSubgridsHoriz) && 
	       (extentSubgridsHoriz > (subsetInfo -> subgridHoriz) ))
	    {
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		if (extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] == NULL)
		{
		    /*ERROR was not calloced*/
		    sprintf(dynamicMsg, "Cannot access data... "
			    "subregion's data buffer for subgrid (%d) " 
			    "not properly intitialized.", presentSubgrid);
		    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
                                          dynamicMsg,"PGS_DEM_ExtractRegion()");
		    
		    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		    
		    /*fatal error*/
		    goto fatalError;
		}
	    }
	    else
	    {
		
		if (extractedRegions[presentSubgrid] == NULL)
		{
		    /*ERROR was not calloced*/
		    sprintf(dynamicMsg, "Cannot access data... "
			    "subregion's data buffer for subgrid (%d) " 
			    "not properly intitialized.", presentSubgrid);
		    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
                                          dynamicMsg,"PGS_DEM_ExtractRegion()");
		    
		    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		    
		    /*fatal error*/
		    goto fatalError;
		}
	    }
	    
	    
	    /*Check to see that the file is properly staged*/
	    
	    if (subset[presentSubgrid] == NULL)
	    {
		/*ERROR file not staged*/
		sprintf(dynamicMsg, "Cannot access data... "
			"Subgrid (%d), not properly staged", presentSubgrid);
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
                                      dynamicMsg,"PGS_DEM_ExtractRegion()");
	    
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
				
		/*fatal error*/
		goto fatalError;
	    }
	    
	    /*iterate the the number of pixels spanning the regions horizontal
	      by the number of pixels horizontally spanning this subregion*/
	    if(((j+1) == extentSubgridsHoriz) && 
	       (extentSubgridsHoriz > (subsetInfo -> subgridHoriz) ))
	    {
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		rowRegionOffset += 
		  (extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] -> colExtent[1] -
		   extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] -> colExtent[0] + 1);
	    }
	    else
	    {
		
		rowRegionOffset += 
		  (extractedRegions[presentSubgrid] -> colExtent[1] -
		   extractedRegions[presentSubgrid] -> colExtent[0] + 1);
	    }
		
	    /*record the largest subregion*/
	    if(((j+1) == extentSubgridsHoriz) && 
	       (extentSubgridsHoriz > (subsetInfo -> subgridHoriz) ))
	    {
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		
		if ((extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] -> sizeSubregion) > 
		    largestSubregion)
		{
		    largestSubregion = extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] ->
		      sizeSubregion;
		}
	    }
	    else
	    {
		
		if ((extractedRegions[presentSubgrid] -> sizeSubregion) > 
		    largestSubregion)
		{
		    largestSubregion = extractedRegions[presentSubgrid] ->
		      sizeSubregion;
		}
	    }
	    
	    /*Iterate to next subgrid in same "row". Check if the present
	      subgrid is equal to Eastern Boundary subgrid.  If this is
	      satisfied, it indicates that this region potentially wraps,
	      dependent on whether or not we have already reached the horizontal
	      extent of the whole region. independently, we change the subgrid
	      to the appropriate western boundary subgrid.  If this is NOT equal
	      to the Eastern Boundary subgrid, simply increment to the
	      (horizontally) adjacent subgrid. */ 
	    if(presentSubgrid == subgridEastBndry + i)
	    {
		presentSubgrid = subgridWestBndry + i;
	    }
	    else
	    {
		presentSubgrid += numSubgridsVert;
	    }

	}
    }

    
    /*Malloc space for the largest of the subregions, needs to be data type
      specific*/ 
    if (subsetInfo -> dataType == DFNT_INT8)
    {
	dataBuffInt8 = (int8 *) malloc(largestSubregion*sizeof(int8));
	if (dataBuffInt8 == NULL)
	{
	    /*ERROR callocing*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "subregion data buffers");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
                                  "PGS_DEM_ExtractRegion()");
	    statusError = PGSMEM_E_NO_MEMORY;
	    
	    /*fatal error*/
	    goto fatalError;

	}
    }
    else if (subsetInfo -> dataType == DFNT_INT16)
    {
	dataBuffInt16 = (int16 *) malloc(largestSubregion*sizeof(int16));
	if (dataBuffInt16 == NULL)
	{
	    /*ERROR callocing*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "subregion data buffers");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
                                  "PGS_DEM_ExtractRegion()");
	    statusError = PGSMEM_E_NO_MEMORY;
	    
	    /*fatal error*/
	    goto fatalError;

	}
    }
    else if (subsetInfo -> dataType == DFNT_FLOAT32)
    {
	dataBuffFlt32 = (float32 *) malloc(largestSubregion*sizeof(float32));
	if (dataBuffFlt32 == NULL)
	{
	    /*ERROR callocing*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "subregion data buffers");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
                                  "PGS_DEM_ExtractRegion()");
	    statusError = PGSMEM_E_NO_MEMORY;
	    
	    /*fatal error*/
	    goto fatalError;

	}
    }


    /*Now we can actually extract the subregions.  Open up each HDF-EOS file for
      a particular subgrid.  Attach to the GRID, and read the data into the
      appropriate data buffer.  Because there are different data types,
      depending on the layer, one has to determine the appropriate data type.*/

    subregionOffset = 0;
    
    for(i = 0; i < extentSubgridsVert; i++)
    {
	/*offset for dataRegion to locate the beginning of this particular row
	  of subregions, i. the beginning of the first subregion in this "row"*/
	subregionBegin = subregionOffset;

	/*Calculate the left most subgrid of one's region*/
	presentSubgrid = subgridUpprLft + i;
	
	for(j = 0; j < extentSubgridsHoriz; j++)
	{

	    /*initialize hdfId and gdID to be closed or unattached*/
	    hdfID = PGSd_DEM_HDF_CLOSE;
	    gdID = PGSd_DEM_HDF_CLOSE;
	    
	    /* Access the File and GRID tags, hdfID and gdID.  If the ID's 
	       are not available, open the particular file.  */

	    hdfID = subset[presentSubgrid] -> hdfID; 
	    gdID = subset[presentSubgrid] -> gdID;
	    if ((hdfID == PGSd_DEM_HDF_CLOSE) || (gdID == PGSd_DEM_HDF_CLOSE))
	    {
		/*HDF-EOS file needs to be opened and attached to */
		status = PGS_DEM_AccessFile(subsetInfo, subset, presentSubgrid,
					    PGSd_DEM_OPEN);
		if (status != PGS_S_SUCCESS)
		{
		    /*ERROR Opening and accessing data*/
		    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					    "Cannot access and attach to file"
					    "PGS_DEM_AccessFile failed.",
					    "PGS_DEM_ExtractRegion()");
		    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;	    
		    
		    /*fatal error*/
		    goto fatalError;
		}
	    }

	    /* get resolution and layers_existvalue from the file. 
	       layers_existvalue = 0 means that data for the layers elev, 
	       slope, aspect, land/water, std dev slope, and std dev elev
	       exist. layers_existvalue = 1 means that only data for 
	       land/water exist. */
	    
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	    status = GDreadattr(gdID, "_resolution", &current_resolution);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	    if (status == FAIL)
	    {
		/*ERROR getting resolution tag attribute*/
		PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					  "Cannot access resolution attr.", 
					  "PGS_DEM_ExtractRegion()");
		statusError = PGSDEM_E_HDFEOS;
		
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

	    status = GDreadattr(gdID, "_layerDataExist", &layers_existvalue);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	    if (status == FAIL)
	    {
		/*ERROR getting layerDataExist tag attribute*/
		PGS_SMF_SetDynamicMsg(PGSDEM_W_HDFEOS,
		  "Cannot access layerDataExist attr. layers_existvalue will set to 0", 
						  "PGS_DEM_ExtractRegion()");
				    
		/* old hdf files that had only elev and land/sea layers
		   do not have this flag. so if it fails 
		   to find this attribute, set the value to 0 */
		layers_existvalue = 0;
	    }
	    
	    /*calculate the number of pixels horizontally and vertically
	      spanning the subregion*/
	    if(((j+1) == extentSubgridsHoriz) && 
	       (extentSubgridsHoriz > (subsetInfo -> subgridHoriz) ))
	    {
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		numColsSubregion = 
		  (extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] -> colExtent[1] -
		   extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] -> colExtent[0] + 1);
		
		numRowsSubregion = 
		  (extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] -> rowExtent[1] -
		   extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] -> rowExtent[0] + 1);
	    }
	    else
	    {
		
	    numColsSubregion = (extractedRegions[presentSubgrid] -> 
				  colExtent[1] -
				  extractedRegions[presentSubgrid] -> 
				  colExtent[0] + 1);

	    numRowsSubregion = (extractedRegions[presentSubgrid] -> 
				rowExtent[1] -
				extractedRegions[presentSubgrid] -> 
				rowExtent[0] + 1);
	    }
	    
	    /*Initialize the parameters for extracting the subregion-- useing
	      GDreadfield*/ 
	    if(((j+1) == extentSubgridsHoriz) && 
	       (extentSubgridsHoriz > (subsetInfo -> subgridHoriz) ))
	    {
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		start[0] = extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] -> rowExtent[0];
		start[1] = extractedRegions[presentSubgrid + subsetInfo -> numSubgrids] -> colExtent[0];
	    }
	    else
	    {
	    start[0] = extractedRegions[presentSubgrid] -> rowExtent[0];
	    start[1] = extractedRegions[presentSubgrid] -> colExtent[0];
	    }
	    
	    edge[0] = numRowsSubregion;	    
	    edge[1] = numColsSubregion;
	    stride = NULL;
	    
	    /*Read in data into internal data buffer, dataType specific buffer
	      for the subregion*/

	    num_temp_pix =largestSubregion;

	    if (subsetInfo -> dataType == DFNT_INT8)
	    {
		if((layers_existvalue == 1 ) && 
		   (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataBuffInt8 with fillvalue */
		    
		    for (fill_count=0; fill_count< num_temp_pix ; fill_count++)
		    {
			dataBuffInt8[fill_count] = (subsetInfo -> fillvalue);
		    }
		    statusHdf = PGS_S_SUCCESS;
		}
		else
		{ 

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	             statusHdf = GDreadfield(gdID, subsetInfo -> fieldName, start,
					     stride, edge, (void *)dataBuffInt8);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		}
	    }
	    else if(subsetInfo -> dataType == DFNT_INT16)
	    {
		if((layers_existvalue == 1 ) && 
		   (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataBuffInt16 with fillvalue */
		    
		    for (fill_count=0; fill_count< num_temp_pix ; fill_count++)
		    {
			dataBuffInt16[fill_count] = (subsetInfo -> fillvalue);
		    }
		    statusHdf = PGS_S_SUCCESS;
		}
		else
		{ 

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	            statusHdf = GDreadfield(gdID, subsetInfo -> fieldName, start,
					    stride, edge, (void *)dataBuffInt16);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		}
	    }
	    else if(subsetInfo -> dataType == DFNT_FLOAT32)
	    {
		if((layers_existvalue == 1 ) && 
		   (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataBuffFlt32 with fillvalue */
		    
		    for (fill_count=0; fill_count< num_temp_pix ; fill_count++)
		    {
			dataBuffFlt32[fill_count] = (subsetInfo -> fillvalue);
		    }
		    statusHdf = PGS_S_SUCCESS;
		}
		else
		{

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

		    statusHdf = GDreadfield(gdID, subsetInfo -> fieldName, start,
					    stride, edge, (void *)dataBuffFlt32);


#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		}
	    }
	    
	    /*Check if error reading data from HDF-EOS file*/
	    if (statusHdf == FAIL)
	    {
		/*ERROR extracting data*/
		sprintf(dynamicMsg, "Cannot access the data... "
			"GDreadfield failed on subgrid (%d) " 
			"of subset (%d) (or PCF logical ID).",
			presentSubgrid, subsetInfo -> subset);
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
                                      dynamicMsg,"PGS_DEM_ExtractRegion()");
		
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal error*/
		goto fatalError;
	    }


	    /*Now that we have read the data into the databuffer, we now have to
	      copy the data into the user's data buffer, dataRegion.  Here is
	      where all the pointer arithmetic is going to be utilized.
	      Remember, we copy the data in a row at a time (from the
	      subregion's data buffer) to ensure geographic continuity.*/

	    /*This is again data type specific*/
	    if (subsetInfo -> dataType == DFNT_INT8)
	    {
		
		/*iterate through each row of the subregion. copy the data to
		  the appropriate part of the dataRegion.  the first element of
		  memcpy takes the dataRegion and finds the beginning of this
		  particular subregion. It iterates this pointer by row number
		  times the horizontal extent of the region.  The second element
		  of memcpy is the particular row to be copied from the
		  subregion's data buffer. The third element says that we are
		  copying one row of the subregion to dataRegion.*/

		for (k = 0; k < numRowsSubregion; k++)
		{
		    memcpy((void *)(((int8 *)dataRegion) + subregionBegin + 
				    (k * rowRegionOffset)), 
			   (void *)(dataBuffInt8 + (k * numColsSubregion)),
			   numColsSubregion * sizeof(int8));
		}
	     
	    }
	    else if (subsetInfo -> dataType == DFNT_INT16)
	    {

		for (k = 0; k < numRowsSubregion; k++)
		{
		    memcpy((void *)(((int16 *)dataRegion) + subregionBegin + 
				    (k * rowRegionOffset)), 
			   (void *)(dataBuffInt16 + (k * numColsSubregion)),
			   numColsSubregion * sizeof(int16));
		}
	     
	    }
	    else if (subsetInfo -> dataType == DFNT_FLOAT32)
	    {
		
		for (k = 0; k < numRowsSubregion; k++)
		{
		    memcpy((void *)(((float32 *)dataRegion) + subregionBegin + 
				    (k * rowRegionOffset)), 
			   (void *)(dataBuffFlt32 + (k * numColsSubregion)),
			   numColsSubregion * sizeof(float32));
		}
	     
	    }


	    /*iterate subregionBegin by the number of pixels in a row of the
	      subregion.  this iteration is for next subregion in the same
	      subregion "row" to find it's beginning point*/ 	 
	    subregionBegin += numColsSubregion;
	    
	    /*iterate subregionOffset by the number of pixels in the subregion.
	     this only comes into effect when we iterate to the next "row" of
	     subregions, ie. iterate i. */
	    if(((j+1) == extentSubgridsHoriz) && 
	       (extentSubgridsHoriz > (subsetInfo -> subgridHoriz) ))
	    {
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		subregionOffset += 
		  extractedRegions[presentSubgrid + subsetInfo -> 
				  numSubgrids] -> sizeSubregion;
	    }
	    else
	    {
		subregionOffset += extractedRegions[presentSubgrid] ->
		  sizeSubregion;
	    }
	    
	    /*Iterate to next subgrid in same "row". Check if the present
	      subgrid is equal to Eastern Boundary subgrid.  If this is
	      satisfied, it indicates that this region potentially wraps,
	      dependent on whether or not we have already reached the horizontal
	      extent of the whole region. independently, we change the subgrid
	      to the appropriate western boundary subgrid.  If this is NOT equal
	      to the Eastern Boundary subgrid, simply increment to the
	      (horizontally) adjacent subgrid. */ 
	    if(presentSubgrid == subgridEastBndry + i)
	    {
		presentSubgrid = subgridWestBndry + i;
	    }
	    else
	    {
		presentSubgrid += numSubgridsVert;
	    }
	    
	}
    }
    
    /*if reached this point, succesfully completed the data transfer to
      dataRegion. free up the data buffers*/
    if(dataBuffInt8 != NULL)
    {
	free(dataBuffInt8);
	dataBuffInt8 = NULL;
    }
    else if (dataBuffInt16 != NULL)
    {
	free(dataBuffInt16);
	dataBuffInt16 = NULL;
    }
    else if (dataBuffFlt32 != NULL)
    {
	free(dataBuffFlt32);
	dataBuffFlt32 = NULL;
    }
	


    /*succesful return*/
    return(PGS_S_SUCCESS);
    
fatalError:
    {
	/*need to free up all the data buffers that were initialized.  Also
	  need to close any open HDF-EOS sessions*/

	for (i = 0; i < 2*(subsetInfo -> numSubgrids); i++)
	{
	    /*check if this particular subgrid's subregion has been
	      initialized*/ 
	    if (extractedRegions[i] != NULL)
	    {
		/*free subregion's RegionRecord*/
		free(extractedRegions[i]);
		extractedRegions[i] = NULL;
		
	    }
	}

	/*free data buffer*/
	if(dataBuffInt8 != NULL)
	{
	    free(dataBuffInt8);
	}
	else if (dataBuffInt16 != NULL)
	{
	    free(dataBuffInt16);
	}
	else if (dataBuffFlt32 != NULL)
	{
	    free(dataBuffFlt32);
	}
	
		
	/*Return appropriate error status*/
	return(statusError);
	
    }
    
}
