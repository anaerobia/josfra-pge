/**********************************************************
PGS_DEM_RecursiveSearchPix.c--

This function searches through various subgrids for data points.  Each recursion
is a different resolution search.  The function takes an array of position
elements and extracts each data point.  To optimize the extraction of the
points, this function takes a second ordered array.  In this array is the
indices of the first (data) array grouped by subgrid number.  This grouping
allows one to access all the data from one file before opening a second file. 
If a point is a fill value, then it's indices is stored at the beginning of the
ordered array.  When the function has gotten values for all the entered points,
it calls itself with the new ordered array of fill value points. This time, the
array has been ordered by the subgrid values of the next (lower) resolution
dataset.  The base case for the function is when no more resolutions are
available. 

PIXEL VERSION OF THE SEARCHING FUNCTION
Author--
Alexis Zubrow/ARC
Abe Taaheri

Dates--
February 9, 1997   AZ  first created
July 7, 1997       AZ  Added PGS_DEM_AccessFile
3/24/1999          AT  Modified so that for data types of int16 and flt32
                       the data buffer is filled with fillvalues if data 
                       for layer 3arc reolution is not available. 
7/8/1999           SZ  Updated for the thread-safe functionality
6/5/2000           AT  Added functionality for 3km resolution
9/14/2011       JK/AT  Changed to treat int8 the same as int16 with respect
		       to 3arc layer.


**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>
#include <mfhdf.h>
#include <HdfEosDef.h>
#include <PGS_TSF.h>

PGSt_SMF_status
PGS_DEM_RecursiveSearchPix(
    PGSt_DEM_Tag resolutionList[],       /*list of resolutions to search*/
    PGSt_integer numResolutions,         /*number of resolutions in list*/
    PGSt_DEM_SubsetRecord *subsetInfo,   /*subset Information*/
    PGSt_DEM_FileRecord  **subset, 	 /*pointer to complete subset*/
    PGSt_DEM_PointRecordPix **dataPoints, /*Array of points to search*/
    PGSt_integer numSubgridCover,        /*number subgrids, covered by points*/
    PGSt_integer orderedArray[],         /*order of indices for dataPoints*/
    PGSt_integer numPoints)   		 /*number points in orderedArray*/
{
    
    PGSt_integer previousSubgrid = PGSd_DEM_NOT_ACCESSED; /*subgridValue of 
							    previous point*/

    PGSt_integer previousNumPoints = 0; /*number of points in previous subrid*/
    PGSt_integer presentNumPoints;    /*total number points in present subgrid*/
    
    PGSt_integer presentSubgrid;      /*Subgrid presently accessing*/
    PGSt_integer numFillPoints = 0;   /*number of fill value points discovered*/
    PGSt_integer i;		      /*counter, for looping*/
    PGSt_integer j;		      /*counter, for looping*/
    
    /*SubsetInfo, subset, and number of points covering-- for the next
      resolution in the resolution list*/
    PGSt_DEM_SubsetRecord *subsetInfoNext;
    PGSt_DEM_FileRecord **subsetNext;
    PGSt_integer numSubgridCoverNext;
    
    PGSt_SMF_status statusSmf;  /*status return for PGS toolkit calls*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retVal;
#endif
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    
    /*HDF-EOS interface*/
    int32 gridPixRow;   /*position within a particular GRID's coordinates*/
    int32 gridPixCol;	/*position wihin particular GRID's coordinates*/
    
    int32 latNearest;       /*pixel position used for either decimal degree
			      position or for converting pixel position between
			      resolutions, ONLY for nearest neighbor
			      interpolation*/ 
    int32 lonNearest;

    int32 *rowArray = NULL;      /*array to hold gridPixRow*/
    int32 *colArray = NULL;      /*array to hole gridPixCol*/
    int8 *dataBuffInt8 = NULL;     /*data buffer for the int8 data layers*/
    int16 *dataBuffInt16 = NULL;   /*for the int16 layers*/
    float32 *dataBuffFlt32 = NULL; /*for the float32 layers, need one for each
				     possible data type available in the
				     datasets*/ 

    
    int32 statusGetPix;   /*status for GDgetPixValues*/
    int32 gdID = PGSd_DEM_HDF_CLOSE;           /*GRID ID*/
    int32 hdfID = PGSd_DEM_HDF_CLOSE;          /*File Handle*/
    int32  current_resolution;    /*resolution tag from HDF-EOS attribute*/
    int32  layers_existvalue; /*layerDataExist tag from HDF-EOS attribute*/

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif

    /*information pertaining to original resolution, resolution when the 
      recursive search is first invoked.  needed for converting pixel values 
      between resolutions.  Need to retain information between resolutions,
      i.e. between recursion calls*/
    
#ifdef _PGS_THREADSAFE
    /* Create non-static variable for the thread-safe version */
    PGSt_DEM_SubsetRecord *subsetInfoOrig;

    /* Set up thread-safe key keeper and TSD key */
    PGSt_TSF_MasterStruct *masterTSF;
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(retVal))
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }

    subsetInfoOrig = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                 masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSETINFOORIG]);
#else
    static PGSt_DEM_SubsetRecord *subsetInfoOrig;
#endif

    /*****************/
    /*FUNCTION ITSELF*/
    /*****************/

    /*Setup for converting pixel values between resolutions-- the position
      pixels within a dataPoint struct, always remain in the coordinate system
      of the highest resolution, i.e. the values that were originaly entered in
      by the user, use this variable to convert the original pixel position to
      that of the */
    if ((dataPoints[0] -> resolutionOrig) == resolutionList[0])
    {
	/*record the "original" subsetInfo*/
	subsetInfoOrig = subsetInfo;
#ifdef _PGS_THREADSAFE
            /* Reset TSD key */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSETINFOORIG],
                     subsetInfoOrig);
#endif      

    }


    /*************************************************/

    /*Base case of recursion function-- if only one resolution left in
      resolution list, no need to call the function again. In addition,
      there is no longer a need for checking and reordering the fillvalue
      points. Therefore, I only check for  fillvalues until I find the first
      one in any of the subgrids. I do thisinitial check  for error reporting.
      This way to the user know that there are still some fill values in the
      final data returned.  After I find the first fill value, I no longer check
      for them in the base case*/ 

    /*************************************************/


    if (numResolutions == 1)
    {
	/*use the ordered array to call actual values from the array structs
	  concerning the data points.  A call will look like following:
	  "dataPoints[orderedArray[i]]"-- this translates to the value at the 
	  ith element of the orderedArray becomes the indices of the array
	  dataPoints. I chose to do it this way so could order and reorder
	  sections of the array dataPoints without having to constantly
	  reallocate space and without having to keep track of the original
	  order of the data*/

	/* Don't increment through the total number of points entered in
	   dataArray.  Rather, we use the number of subgrids covered by the
	   points and the number of points in each subgrid (information in the
	   File Record-- subset) to loop through only those points which are in
	   a singular subgrid or HDF-EOS file.  This gives us the additional
	   advantage of using the optimized form of GDgetpixvalue.  Now we can
	   pass it an array of points, rather than simply a point at a time.*/ 

	for (j = 0; j < numSubgridCover; j++)
	{

	    /*get the number of data points to be extracted from the present
	      subgrid.  In the subgrid's individual File Record.  To find the
	      present subgrid, have to count off from the beginning of the
	      orderedArray, tot he present point you will be analyzing.  That
	      should be the new subgrid value*/
	    presentSubgrid = dataPoints[orderedArray[previousNumPoints]] ->
	      subgridValue;
	    
	    /*Check if the sorting and summing routine worked properly.  the
	      last and the present subgrid should be different*/
	    if (presentSubgrid == previousSubgrid)
	    {
		/*ERROR, with sorting and summing routine*/
		statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					      "Sorting routine error... " 
					      "cannot access data",
					      "PGS_DEM_RecursiveSearchPix()");

		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
				
		/*fatal error*/
		goto fatalError;
		
	    }
	    
	    /*Check if the file has been staged from which the point is
	      being extracted*/
	    if (subset[presentSubgrid] == NULL)
	    {
		/*ERROR file not staged*/
		sprintf(dynamicMsg, "Cannot access data... "
			"Subgrid (%d), not properly staged", presentSubgrid);
		statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						 dynamicMsg, 
						 "PGS_DEM_RecursiveSearchPix()");
	    
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
				
		/*fatal error*/
		goto fatalError;
		
	    }

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
		    statusSmf = 
		      PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					    "Cannot access and attach to file"
					    "PGS_DEM_AccessFile failed.",
					    "PGS_DEM_RecursiveSearchPix()");
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
		statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
						  "Cannot access resolution attr.", 
						  "PGS_DEM_PGS_DEM_RecursiveSearchPix()");
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
		statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_W_HDFEOS,
						  "Cannot access layerDataExist attr. layers_existvalue will set to 0", 
						  "PGS_DEM_PGS_DEM_RecursiveSearchPix()");

		/* old hdf files that had only elev and land/sea layers
		   do not have this flag. so if it fails 
		   to find this attribute, set the value to 0 */
		layers_existvalue = 0;
	    }
	
	    /* Get the number of points requested from this subgrid. thsi is the
	       number of points which will be extracted from one HDF file*/

	    presentNumPoints = subset[presentSubgrid] -> pntsRequested;

	    /*update previousSubgrid*/
	    previousSubgrid = presentSubgrid;


	    /*Calloc space for the rowArray and colArray*/
	    rowArray = (int32 *) calloc(presentNumPoints, sizeof(int32));
	    colArray = (int32 *) calloc(presentNumPoints, sizeof(int32));
	    if ((rowArray == NULL) || (colArray == NULL))
	    {
		/*ERROR callocing*/
		sprintf(dynamicMsg, "Error allocating memory for "
			"rowArray or colArray");
		statusSmf = PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						  dynamicMsg, 
						  "PGS_DEM_RecursiveSearchPix()");
		statusError = PGSMEM_E_NO_MEMORY;
		
		/*fatal ERROR*/
		goto fatalError;
	    }
	    

	    /*now loop through the points in an individual subgrid*/
	    for (i = previousNumPoints; i < (presentNumPoints +
					     previousNumPoints); i++) 
	    {
			    	    
		/*Calculate global pixels in the new resolution--
		  these pixels are NOT valid for the internal GRID*/
		latNearest = PGSm_DEM_PixToPix((dataPoints[orderedArray[i]]
					       -> positionOrig[0]),
					       subsetInfoOrig, subsetInfo);
		lonNearest = PGSm_DEM_PixToPix((dataPoints[orderedArray[i]]
					       -> positionOrig[1]),
					       subsetInfoOrig, subsetInfo);

#ifdef _PGS_THREADSAFE
            /* Reset TSD key */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSETINFOORIG],
                               subsetInfoOrig);
#endif
		
		
		/*calculate internal pixels for the particular GRID in the
		  particular file (subgrid)*/
		gridPixRow = latNearest % (subsetInfo -> vertPixSubgrid);
		gridPixCol = lonNearest % (subsetInfo -> horizPixSubgrid);
	    
		/*record the internal pixel points in arrays*/
		rowArray[i - previousNumPoints] = gridPixRow;
		colArray[i - previousNumPoints] = gridPixCol;
	    }

	    
	    /*get actual data by pixels*/
	    /*NOTE-- to speed up, could co-opt code from
	      GDgetpixelvalues, don't neeed some of the checks for
	      merged fields, etc..*/
	    /*Have to get read into the proper type buffer, size and type fo
	      data dependent on layer what I am actually passing to
	      GDgetpixvalues is: pixel coordinates of the point (not in global
	      pixels, but in the internal coordinate system of the GRID) and the
	      fieldname In
	      addition, the datavalue is a union of different size variables. I
	      used this because the data will be in various types. therefore,
	      one must dereference the appropriate type from the datavalue 
	      union before filling it with data*/
 
	    if (subsetInfo -> dataType == DFNT_FLOAT32)
  	    {
		/*first calloc space for output buffer*/
		dataBuffFlt32 = (float32 *) calloc(presentNumPoints, 
                                                       sizeof(float32));
		if (dataBuffFlt32 == NULL)
		{
		    /*ERROR callocing*/
		    sprintf(dynamicMsg, "Error allocating memory for "
			    "dataBuffFlt32");
		    statusSmf = PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						      dynamicMsg, 
						      "PGS_DEM_RecursiveSearchPix()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal ERROR*/
		    goto fatalError;
		}
		
		/*get all the points from this particular subgrid*/
       
		if((layers_existvalue == 1 ) && (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataBuffFlt32 with fillvalue */
		    
		    for (i= 0; i <presentNumPoints; i++)
		    {
			dataBuffFlt32[i] = (subsetInfo -> fillvalue);
		    }
		    statusGetPix = PGS_S_SUCCESS;
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

		    statusGetPix = GDgetpixvalues(gdID, presentNumPoints, 
						  rowArray, colArray, 
						  subsetInfo -> fieldName, 
						  (void *)
						  dataBuffFlt32);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		}

		if (statusGetPix == FAIL)
		{
		    /*ERROR retrieving data*/
		    sprintf(dynamicMsg, "Could not access data... "
			    "GDgetpixvalues failed on subgrid (%d) " 
			    "of subset (%d) (or PCF logical ID)",
			    presentSubgrid, subsetInfo -> subset);

		    statusSmf =  
		      PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					    dynamicMsg, 
					    "PGS_DEM_RecursiveSearchPix()");
		    
		    /*fatal error*/
		    goto fatalError;
		    
		}

		/*for base case, only need to check for fill values until one
		  has found one data point which is fill.  Only checking for
		  error handling */
		if (numFillPoints == 0)
		{
		    for(i = 0; i <presentNumPoints; i++)
		    {
			if (subsetInfo -> fillvalue == dataBuffFlt32[i])
			{
			    /*WARNING fill values included*/
			    
			    numFillPoints ++;
			    i = presentNumPoints; /*kick out of loop*/
			}
		    }
		}
		
		/*write data to the dataPoints*/
		for(i = previousNumPoints; i < (presentNumPoints +
					     previousNumPoints); i++) 
		{
		    /*remember, have to use orderedArray to get appropriate
		      indices for points in this subgrid. The beast on the left
		      side is necessary to get the appropriate point record and
		      the appropriate data type*/
		    (dataPoints[orderedArray[i]] -> dataValue.FourByteFlt) =
		      dataBuffFlt32[i - previousNumPoints];
		}
		
		/*Free data buffer*/
		free(dataBuffFlt32);
		
	    }
	    else if (subsetInfo -> dataType == DFNT_INT16)
  	    {
		/*first calloc space for output buffer*/
		dataBuffInt16 = (int16 *) calloc(presentNumPoints, sizeof(int16));
		if (dataBuffInt16 == NULL)
		{
		    /*ERROR callocing*/
		    sprintf(dynamicMsg, "Error allocating memory for "
			    "dataBuffFlt16");
		    statusSmf = PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						      dynamicMsg, 
						      "PGS_DEM_RecursiveSearchPix()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal ERROR*/
		    goto fatalError;
		}
		
		
		/*get all the points from this particular subgrid*/

		if((layers_existvalue == 1 ) && (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataBuffInt16 with fillvalue */
		    
		    for (i= 0; i <presentNumPoints; i++)
		    {
			dataBuffInt16[i] = (subsetInfo -> fillvalue);
		    }
		    statusGetPix = PGS_S_SUCCESS;
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

		    statusGetPix = GDgetpixvalues(gdID, presentNumPoints, 
						  rowArray, colArray, 
						  subsetInfo -> fieldName, 
						  (void *)
						  dataBuffInt16);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		}
		
		if (statusGetPix == FAIL)
		{
		    /*ERROR retrieving data*/
		    sprintf(dynamicMsg, "Could not access data... "
			    "GDgetpixvalues failed on subgrid (%d) " 
			    "of subset (%d) (or PCF logical ID)",
			    presentSubgrid, subsetInfo -> subset);

		    statusSmf =  
		      PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					    dynamicMsg, 
					    "PGS_DEM_RecursiveSearchPix()");
		    
		    /*fatal error*/
		    goto fatalError;
		    
		}

		/*for base case, only need to check for fill values until one
		  has found one data point which is fill.  Only checking for
		  error handling */
		if (numFillPoints == 0)
		{
		    for(i = 0; i <presentNumPoints; i++)
		    {
			if (subsetInfo -> fillvalue == dataBuffInt16[i])
			{
			    /*WARNING fill values included*/
			    
			    numFillPoints ++;
			    i = presentNumPoints; /*kick out of loop*/
			}
		    }
		}
		
		/*write data to the dataPoints*/
		for(i = previousNumPoints; i < (presentNumPoints +
					     previousNumPoints); i++) 
		{
		    /*remember, have to use orderedArray to get appropriate
		      indices for points in this subgrid. The beast on the left
		      side is necessary to get the appropriate point record and
		      the appropriate data type*/
		    (dataPoints[orderedArray[i]] -> dataValue.TwoByteInt) =
		      dataBuffInt16[i - previousNumPoints];
		}
		
		/*Free data buffer*/
		free(dataBuffInt16);
		
	    }
	    else if (subsetInfo -> dataType == DFNT_INT8)
  	    {
		/*first calloc space for output buffer*/
		dataBuffInt8 = (int8 *) calloc(presentNumPoints, sizeof(int8));
		if (dataBuffInt8 == NULL)
		{
		    /*ERROR callocing*/
		    sprintf(dynamicMsg, "Error allocating memory for "
			    "dataBuffFlt8");
		    statusSmf = PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						      dynamicMsg, 
						      "PGS_DEM_RecursiveSearchPix()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal ERROR*/
		    goto fatalError;
		}
		
		/*get all the points from this particular subgrid*/

		if((layers_existvalue == 1 ) && (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataBuffInt8 with fillvalue */
		    
		    for (i= 0; i <presentNumPoints; i++)
		    {
			dataBuffInt8[i] = (subsetInfo -> fillvalue);
		    }
		    statusGetPix = PGS_S_SUCCESS;
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

		statusGetPix = GDgetpixvalues(gdID, presentNumPoints, 
					      rowArray, colArray, 
					      subsetInfo -> fieldName, 
					      (void *)
					      dataBuffInt8);
			
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

		}
			
		if (statusGetPix == FAIL)
		{
		    /*ERROR retrieving data*/
		    sprintf(dynamicMsg, "Could not access data... "
			    "GDgetpixvalues failed on subgrid (%d) " 
			    "of subset (%d) (or PCF logical ID)",
			    presentSubgrid, subsetInfo -> subset);

		    statusSmf =  
		      PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					    dynamicMsg, 
					    "PGS_DEM_RecursiveSearchPix()");
		    
		    /*fatal error*/
		    goto fatalError;

		}

		/*for base case, only need to check for fill values until one
		  has found one data point which is fill.  Only checking for
		  error handling */
		if (numFillPoints == 0)
		{
		    for(i = 0; i <presentNumPoints; i++)
		    {
			if (subsetInfo -> fillvalue == dataBuffInt8[i])
			{
			    /*WARNING fill values included*/
			    
			    numFillPoints ++;
			    i = presentNumPoints; /*kick out of loop*/
			}
		    }
		}
		
		/*write data to the dataPoints*/
		for(i = previousNumPoints; i < (presentNumPoints +
					     previousNumPoints); i++) 
		{
		    /*remember, have to use orderedArray to get appropriate
		      indices for points in this subgrid. The beast on the left
		      side is necessary to get the appropriate point record and
		      the appropriate data type*/
		    (dataPoints[orderedArray[i]] -> dataValue.OneByteInt) =
		      dataBuffInt8[i - previousNumPoints];
		}

		/*Free data buffer*/
		free(dataBuffInt8);
		
	    }

	
	    /*free the rowArray and the colArray*/
	    free(rowArray);
	    free(colArray);
	    
	    /*iterate the previous number of points so that at the beginning of
	      next subgrid.  Record this subgrid as the previous one, for
	      testing*/
	    previousNumPoints += presentNumPoints;
	    previousSubgrid = presentSubgrid;
	    
	}

	/*Before returning, check if all the fill values have been filled,a nd
	  if there may be data from multiple resolutions*/
	
	if ((numFillPoints != 0) && ((dataPoints[orderedArray[0]] ->
				     resolutionOrig)  != resolutionList[0]))
	{
	    /*Fill values included in final data and there possibly is data from
	     multiple resolutions*/
	    statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_M_FILLVALUE_INCLUDED,
					      "Fillvalues included, possibility"
					      " that there is data interpolated"
					      " from multiple resolutions.",
					      "PGS_DEM_RecrusiveSearchPix()");
	    return(PGSDEM_M_FILLVALUE_INCLUDED);
	}
	else if (numFillPoints != 0)
	{
	    /*fill values included in data*/
	    statusSmf = PGS_SMF_SetStaticMsg(PGSDEM_M_FILLVALUE_INCLUDED, 
					     "PGS_DEM_RecursiveSearchPix()");
	    return (PGSDEM_M_FILLVALUE_INCLUDED);
	}
	else if ((dataPoints[orderedArray[0]] -> resolutionOrig)  != 
		 resolutionList[0])
	{
	    /*data interpolated from multiple resolutions*/
	    statusSmf = PGS_SMF_SetStaticMsg(PGSDEM_M_MULTIPLE_RESOLUTIONS, 
					  "PGS_DEM_RecursiveSearchPix()");
	    return (PGSDEM_M_MULTIPLE_RESOLUTIONS);
	}
	else
	{
	    /*otherwise all data was retrieved from first resolution*/
	    return(PGS_S_SUCCESS);
	}
    }
    
    /*************************************************/

    /*If there are more than one resolution in the list, then have to check
      every data element retrieved for fill values. As one finds fill
      values, the numFillPoints is incremented.  In addition, the indices of
      these fill values element of the dataPoints array are rerecorded in
      the top elements of the orderedArray. The call will look like:

      orderedArray[numFillPoints - 1] = orderedArray[i];
      --which will reassign the indices of the dataPoint array, now only
      those values which have fill values, to the top of the array
      "orderedArray" 
  
      Once the function has completed
      the loop, the new orderedArray is again organized by subgrid value,
      this time the subgrid value of the next resolution.  But, only the
      first elements of the orderedArray are reordered (only the fill
      values).  The re-ordered, new orderedArray is then passed to
      RecursiveSearch again.  The number of points now being the number of
      fill values and the number of resolutions being decrimented by 1 */

    /*************************************************/
 
    else if (numResolutions > 1)
    { 

	/* Don't increment through the total number of points entered in
	   dataArray.  Rather, we use the number of subgrids covered by the
	   points and the number of points in each subgrid (information in the
	   File Record-- subset) to loop through only those points which are in
	   a singular subgrid or HDF-EOS file.  This gives us the additional
	   advantage of using the optimized form of GDgetpixvalue.  Now we can
	   pass it an array of points, rather than simply a point at a time.*/ 

	for (j = 0; j < numSubgridCover; j++)
	{

	    /*get the number of data points to be extracted from the present
	      subgrid.  In the subgrid's individual File Record.  To find the
	      present subgrid, have to count off from the beginning of the
	      orderedArray, tot he present point you will be analyzing.  That
	      should be the new subgrid value*/
	    presentSubgrid = dataPoints[orderedArray[previousNumPoints]] ->
	      subgridValue;
	    
	    /*Check if the sorting and summing routine worked properly.  the
	      last and the present subgrid should be different*/
	    if (presentSubgrid == previousSubgrid)
	    {
		/*ERROR, with sorting and summing routine*/
		statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					      "Sorting routine error... " 
					      "cannot access data",
					      "PGS_DEM_RecursiveSearchPix()");

		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
				
		/*fatal error*/
		goto fatalError;
		
	    }
	    
	    /*Check if the file has been staged from which the point is
	      being extracted*/
	    if (subset[presentSubgrid] == NULL)
	    {
		/*ERROR file not staged*/
		sprintf(dynamicMsg, "Cannot access data... "
			"Subgrid (%d), not properly staged", presentSubgrid);
		statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						 dynamicMsg, 
						 "PGS_DEM_RecursiveSearchPix()");
	    
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
				
		/*fatal error*/
		goto fatalError;
		

	    }
	    
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
		    statusSmf = 
		      PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					    "Cannot access and attach to file"
					    "PGS_DEM_AccessFile failed.",
					    "PGS_DEM_RecursiveSearchPix()");
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
		statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
						  "Cannot access resolution attr.", 
						  "PGS_DEM_PGS_DEM_RecursiveSearchPix()");
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
		statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_W_HDFEOS,
						  "Cannot access layerDataExist attr. layers_existvalue will set to 0", 
						  "PGS_DEM_PGS_DEM_RecursiveSearchPix()");

		/* old hdf files that had only elev and land/sea layers
		   do not have this flag. so if it fails 
		   to find this attribute, set the value to 0 */
		layers_existvalue = 0;
	    }

	    /* Get the number of points requested from this subgrid. thsi is the
	       number of points which will be extracted from one HDF file*/

	    presentNumPoints = subset[presentSubgrid] -> pntsRequested;

	    /*update previousSubgrid*/
	    previousSubgrid = presentSubgrid;


	    /*Calloc space for the rowArray and colArray*/
	    rowArray = (int32 *) calloc(presentNumPoints, sizeof(int32));
	    colArray = (int32 *) calloc(presentNumPoints, sizeof(int32));
	    if ((rowArray == NULL) || (colArray == NULL))
	    {
		/*ERROR callocing*/
		sprintf(dynamicMsg, "Error allocating memory for "
			"rowArray or colArray");
		statusSmf = PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						  dynamicMsg, 
						  "PGS_DEM_RecursiveSearchPix()");
		statusError = PGSMEM_E_NO_MEMORY;
		
		/*fatal ERROR*/
		goto fatalError;
	    }
	    

	    /*now loop through the points in an individual subgrid*/
	    for (i = previousNumPoints; i < (presentNumPoints +
					     previousNumPoints); i++) 
	    {
			    	    
		/*Calculate global pixels in the new resolution--
		  these pixels are NOT valid for the internal GRID*/
		latNearest = PGSm_DEM_PixToPix((dataPoints[orderedArray[i]]
					       -> positionOrig[0]),
					       subsetInfoOrig, subsetInfo);
		lonNearest = PGSm_DEM_PixToPix((dataPoints[orderedArray[i]]
					       -> positionOrig[1]),
					       subsetInfoOrig, subsetInfo);

#ifdef _PGS_THREADSAFE
            /* Reset TSD key */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSETINFOORIG],
                            subsetInfoOrig);
#endif
		
		/*calculate internal pixels for the particular GRID in the
		  particular file (subgrid)*/
		gridPixRow = latNearest % (subsetInfo -> vertPixSubgrid);
		gridPixCol = lonNearest % (subsetInfo -> horizPixSubgrid);
	    
		/*record the internal pixel points in arrays*/
		rowArray[i - previousNumPoints] = gridPixRow;
		colArray[i - previousNumPoints] = gridPixCol;
	    }

	    
	    /*get actual data by pixels*/
	    /*NOTE-- to speed up, could co-opt code from
	      GDgetpixelvalues, don't neeed some of the checks for
	      merged fields, etc..*/
	    /*Have to get read into the proper type buffer, size and type fo
	      data dependent on layer what I am actually passing to
	      GDgetpixvalues is: pixel coordinates of the point (not in global
	      pixels, but in the internal coordinate system of the GRID) and the
	      fieldname In
	      addition, the datavalue is a union of different size variables. I
	      used this because the data will be in various types. therefore,
	      one must dereference the appropriate type from the datavalue 
	      union before filling it with data*/
 
	    if (subsetInfo -> dataType == DFNT_FLOAT32)
  	    {
		/*first calloc space for output buffer*/
		dataBuffFlt32 = (float32 *) calloc(presentNumPoints, 
                                                      sizeof(float32));
		if (dataBuffFlt32 == NULL)
		{
		    /*ERROR callocing*/
		    sprintf(dynamicMsg, "Error allocating memory for "
			    "dataBuffFlt32");
		    statusSmf = PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						      dynamicMsg, 
						      "PGS_DEM_RecursiveSearchPix()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal ERROR*/
		    goto fatalError;
		}
		
		/*get all the points from this particular subgrid*/
       
		if((layers_existvalue == 1 ) && (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataBuffFlt32 with fillvalue */
		    
		    for (i= 0; i <presentNumPoints; i++)
		    {
			dataBuffFlt32[i] = (subsetInfo -> fillvalue);
		    }
		    statusGetPix = PGS_S_SUCCESS;
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

		    statusGetPix = GDgetpixvalues(gdID, presentNumPoints, 
						  rowArray, colArray, 
						  subsetInfo -> fieldName, 
						  (void *)
						  dataBuffFlt32);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		}

		if (statusGetPix == FAIL)
		{
		    /*ERROR retrieving data*/
		    sprintf(dynamicMsg, "Could not access data... "
			    "GDgetpixvalues failed on subgrid (%d) " 
			    "of subset (%d) (or PCF logical ID)",
			    presentSubgrid, subsetInfo -> subset);

		    statusSmf =  
		      PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					    dynamicMsg, 
					    "PGS_DEM_RecursiveSearchPix()");
		    
		    /*fatal error*/
		    goto fatalError;

		}

		
		/*write data to the dataPoints*/

		/*At this point things get a bit confusing-- as if they
		  weren't already...

		  First we write the data over to the dataPoints array.
		  Second, we check if this point is a fill value.
		  Third, if so- we check if this is first fill value found. If
		  this is true, we have to get information on the next
		  resolution (lower resolution) in the resolutionList. 
		  Fourth, we change the subgrid value of the particular
		  PGSt_DEM_PointRecord element of the array dataPoints to
		  correspond with subgrid indexing of the next resolution. 
		  Fifth, we assign the fill value indices to the beginning
		  of the orderedArray. */

		for(i = previousNumPoints; i < (presentNumPoints +
					     previousNumPoints); i++) 
		{
		    /*the horror of Indexing!!*/
		    /*remember, have to use orderedArray to get appropriate
		      indices for points in this subgrid. The beast on the left
		      side is necessary to get the appropriate point record and
		      the appropriate data type.
		      In addition, the buffers index starts at 0, so you have to
		      subtract the previousNumPoints.
		      Finally, the re-indexing of the orderedArray uses the
		      numFillPoints to keep track of its indexing */ 

		    (dataPoints[orderedArray[i]] -> dataValue.FourByteFlt) =
		      dataBuffFlt32[i - previousNumPoints];

		    /*check if fillvalue*/
		    if (subsetInfo -> fillvalue == 
			dataBuffFlt32[i - previousNumPoints])
		    {

			numFillPoints++;
			
			if (numFillPoints == 1)
			{
			    /*Get new subsetInfo and subset of the next
			      resolution in the resolutionList*/ 
			    statusSmf = PGS_DEM_Subset(resolutionList[1], 
						       subsetInfo -> layer,
						       PGSd_DEM_INFO,
						       &subsetNext,
						       &subsetInfoNext);
			    if (statusSmf != PGS_S_SUCCESS)
			    {
				/*ERROR getting info and subset from next
				  resolution*/
 
				statusError = statusSmf;

				/*fatal error*/
				goto fatalError;
				
			    }
			}

			/*Change subgrid value, first need to convert original
			  coordinates to pixel values in the next resolution*/
			latNearest = PGSm_DEM_PixToPix(
			    (dataPoints[orderedArray[i]]  -> positionOrig[0]),
			    subsetInfoOrig, subsetInfoNext);
			lonNearest = PGSm_DEM_PixToPix(
			    (dataPoints[orderedArray[i]]  -> positionOrig[1]),
			    subsetInfoOrig, subsetInfoNext);

#ifdef _PGS_THREADSAFE
            /* Reset TSD key */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSETINFOORIG],
                            subsetInfoOrig);
#endif
		
			dataPoints[orderedArray[i]] -> subgridValue =
			  PGSm_DEM_PixToSubgrid(latNearest, lonNearest, 
						subsetInfoNext);
		
			/*reassign fill value index to orderedArray*/
			orderedArray[numFillPoints - 1] = orderedArray[i];
			
		    }
		    

		}
		
		/*Free data buffer*/
		free(dataBuffFlt32);
		
	    }
	    else if (subsetInfo -> dataType == DFNT_INT16)
  	    {
		/*first calloc space for output buffer*/
		dataBuffInt16 = (int16 *) calloc(presentNumPoints, sizeof(int16));
		if (dataBuffInt16 == NULL)
		{
		    /*ERROR callocing*/
		    sprintf(dynamicMsg, "Error allocating memory for "
			    "dataBuffFlt16");
		    statusSmf = PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						      dynamicMsg, 
						      "PGS_DEM_RecursiveSearchPix()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal ERROR*/
		    goto fatalError;
		}
		
		/*get all the points from this particular subgrid*/

		if((layers_existvalue == 1 ) && (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataBuffInt16 with fillvalue */
		    
		    for (i= 0; i <presentNumPoints; i++)
		    {
			dataBuffInt16[i] = (subsetInfo -> fillvalue);
		    }
		    statusGetPix = PGS_S_SUCCESS;
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

		    statusGetPix = GDgetpixvalues(gdID, presentNumPoints, 
						  rowArray, colArray, 
						  subsetInfo -> fieldName, 
						  (void *)
						  dataBuffInt16);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		}

		if (statusGetPix == FAIL)
		{
		    /*ERROR retrieving data*/
		    sprintf(dynamicMsg, "Could not access data... "
			    "GDgetpixvalues failed on subgrid (%d) " 
			    "of subset (%d) (or PCF logical ID)",
			    presentSubgrid, subsetInfo -> subset);

		    statusSmf =  
		      PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					    dynamicMsg, 
					    "PGS_DEM_RecursiveSearchPix()");
		    
		    /*fatal error*/
		    goto fatalError;

		}

		
		/*write data to the dataPoints*/

		/*At this point things get a bit confusing-- as if they
		  weren't already...

		  First we write the data over to the dataPoints array.
		  Second, we check if this point is a fill value.
		  Third, if so- we check if this is first fill value found. If
		  this is true, we have to get information on the next
		  resolution (lower resolution) in the resolutionList. 
		  Fourth, we change the subgrid value of the particular
		  PGSt_DEM_PointRecord element of the array dataPoints to
		  correspond with subgrid indexing of the next resolution. 
		  Fifth, we assign the fill value indices to the beginning
		  of the orderedArray. */

		for(i = previousNumPoints; i < (presentNumPoints +
					     previousNumPoints); i++) 
		{
		    /*the horror of Indexing!!*/
		    /*remember, have to use orderedArray to get appropriate
		      indices for points in this subgrid. The beast on the left
		      side is necessary to get the appropriate point record and
		      the appropriate data type.
		      In addition, the buffers index starts at 0, so you have to
		      subtract the previousNumPoints.
		      Finally, the re-indexing of the orderedArray uses the
		      numFillPoints to keep track of its indexing */ 

		    (dataPoints[orderedArray[i]] -> dataValue.TwoByteInt) =
		      dataBuffInt16[i - previousNumPoints];

		    /*check if fillvalue*/
		    if (subsetInfo -> fillvalue == 
			dataBuffInt16[i - previousNumPoints])
		    {

			numFillPoints++;
			
			if (numFillPoints == 1)
			{
			    /*Get new subsetInfo and subset of the next
			      resolution in the resolutionList*/ 
			    statusSmf = PGS_DEM_Subset(resolutionList[1], 
						       subsetInfo -> layer,
						       PGSd_DEM_INFO,
						       &subsetNext,
						       &subsetInfoNext);
			    if (statusSmf != PGS_S_SUCCESS)
			    {
				/*ERROR getting info and subset from next
				  resolution*/
 
				statusError = statusSmf;

				/*fatal error*/
				goto fatalError;
				

			    }
			}

			/*Change subgrid value, first need to convert original
			  coordinates to pixel values in the next resolution*/
			latNearest = PGSm_DEM_PixToPix(
			    (dataPoints[orderedArray[i]]  -> positionOrig[0]),
			    subsetInfoOrig, subsetInfoNext);
			lonNearest = PGSm_DEM_PixToPix(
			    (dataPoints[orderedArray[i]]  -> positionOrig[1]),
			    subsetInfoOrig, subsetInfoNext);

#ifdef _PGS_THREADSAFE
            /* Reset TSD key */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSETINFOORIG],
                               subsetInfoOrig);
#endif

			dataPoints[orderedArray[i]] -> subgridValue =
			  PGSm_DEM_PixToSubgrid(latNearest, lonNearest, 
						subsetInfoNext);
		
			/*reassign fill value index to orderedArray*/
			orderedArray[numFillPoints - 1] = orderedArray[i];
			
		    }
		    

		}
		
		/*Free data buffer*/
		free(dataBuffInt16);
		
	    }
	    else if (subsetInfo -> dataType == DFNT_INT8)
  	    {
		/*first calloc space for output buffer*/
		dataBuffInt8 = (int8 *) calloc(presentNumPoints, sizeof(int8));
		if (dataBuffInt8 == NULL)
		{
		    /*ERROR callocing*/
		    sprintf(dynamicMsg, "Error allocating memory for "
			    "dataBuffFlt8");
		    statusSmf = PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						      dynamicMsg, 
						      "PGS_DEM_RecursiveSearchPix()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal ERROR*/
		    goto fatalError;
		}
		
		/*get all the points from this particular subgrid*/

		if((layers_existvalue == 1 ) && (current_resolution == PGSd_DEM_3ARC))
		{
		    /* in 3arc file there is no data for layer. fill 
		       dataBuffInt8 with fillvalue */
		    
		    for (i= 0; i <presentNumPoints; i++)
		    {
			dataBuffInt8[i] = (subsetInfo -> fillvalue);
		    }
		    statusGetPix = PGS_S_SUCCESS;
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

		statusGetPix = GDgetpixvalues(gdID, presentNumPoints, 
					      rowArray, colArray, 
					      subsetInfo -> fieldName, 
					      (void *)
					      dataBuffInt8);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

		}

		if (statusGetPix == FAIL)
		{
		    /*ERROR retrieving data*/
		    sprintf(dynamicMsg, "Could not access data... "
			    "GDgetpixvalues failed on subgrid (%d) " 
			    "of subset (%d) (or PCF logical ID)",
			    presentSubgrid, subsetInfo -> subset);

		    statusSmf =  
		      PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					    dynamicMsg, 
					    "PGS_DEM_RecursiveSearchPix()");
		    
		    /*fatal error*/
		    goto fatalError;

		}

		
		/*write data to the dataPoints*/

		/*At this point things get a bit confusing-- as if they
		  weren't already...

		  First we write the data over to the dataPoints array.
		  Second, we check if this point is a fill value.
		  Third, if so- we check if this is first fill value found. If
		  this is true, we have to get information on the next
		  resolution (lower resolution) in the resolutionList. 
		  Fourth, we change the subgrid value of the particular
		  PGSt_DEM_PointRecord element of the array dataPoints to
		  correspond with subgrid indexing of the next resolution. 
		  Fifth, we assign the fill value indices to the beginning
		  of the orderedArray. */

		for(i = previousNumPoints; i < (presentNumPoints +
					     previousNumPoints); i++) 
		{
		    /*the horror of Indexing!!*/
		    /*remember, have to use orderedArray to get appropriate
		      indices for points in this subgrid. The beast on the left
		      side is necessary to get the appropriate point record and
		      the appropriate data type.
		      In addition, the buffers index starts at 0, so you have to
		      subtract the previousNumPoints.
		      Finally, the re-indexing of the orderedArray uses the
		      numFillPoints to keep track of its indexing */ 

		    (dataPoints[orderedArray[i]] -> dataValue.OneByteInt) =
		      dataBuffInt8[i - previousNumPoints];

		    /*check if fillvalue*/
		    if (subsetInfo -> fillvalue == 
			dataBuffInt8[i - previousNumPoints])
		    {

			numFillPoints++;
			
			if (numFillPoints == 1)
			{
			    /*Get new subsetInfo and subset of the next
			      resolution in the resolutionList*/ 
			    statusSmf = PGS_DEM_Subset(resolutionList[1], 
						       subsetInfo -> layer,
						       PGSd_DEM_INFO,
						       &subsetNext,
						       &subsetInfoNext);
			    if (statusSmf != PGS_S_SUCCESS)
			    {
				/*ERROR getting info and subset from next
				  resolution*/
 
				statusError = statusSmf;

				/*fatal error*/
				goto fatalError;
				

			    }
			}

			/*Change subgrid value, first need to convert original
			  coordinates to pixel values in the next resolution*/
			latNearest = PGSm_DEM_PixToPix(
			    (dataPoints[orderedArray[i]]  -> positionOrig[0]),
			    subsetInfoOrig, subsetInfoNext);
			lonNearest = PGSm_DEM_PixToPix(
			    (dataPoints[orderedArray[i]]  -> positionOrig[1]),
			    subsetInfoOrig, subsetInfoNext);

#ifdef _PGS_THREADSAFE
            /* Reset TSD key */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSETINFOORIG],
                              subsetInfoOrig);
#endif

			dataPoints[orderedArray[i]] -> subgridValue =
			  PGSm_DEM_PixToSubgrid(latNearest, lonNearest, 
						subsetInfoNext);
		
			/*reassign fill value index to orderedArray*/
			orderedArray[numFillPoints - 1] = orderedArray[i];
			
		    }
		    

		}
		
		/*Free data buffer*/
		free(dataBuffInt8);
		
	    }

	    /*free the rowArray and the colArray*/
	    free(rowArray);
	    free(colArray);
	    
	    /*iterate the previous number of points so that at the beginning of
	      next subgrid.  Record this subgrid as the previous one, for
	      testing*/
	    previousNumPoints += presentNumPoints;
	    previousSubgrid = presentSubgrid;
	    
	}

	/*RECURSION SHTUFF*/


	/*One other possible base case-- if no fill values in the present
	  resolution*/

	if (numFillPoints == 0)
	{
	    /*Successful at acquiring all the data*/
	    /*Check if accuried all the data from one or multiple resolutions.
	      use the original resolution stored in the PointRecord and compare
	      it with the resolution presently in*/
	    if (dataPoints[orderedArray[0]] -> resolutionOrig == 
		resolutionList[0])
	    {
		/*all data was accessed from one resolution*/
		return(PGS_S_SUCCESS);
	    }
	    else
	    {
		/*data was accessed from multiple resolutions. Set message, and
		  return PGSDEM_M_MULTIPLE_RESOLUTIONS*/
	        statusSmf = PGS_SMF_SetStaticMsg(PGSDEM_M_MULTIPLE_RESOLUTIONS,
						"PGS_DEM_RecursionSearchPix()");
		return (PGSDEM_M_MULTIPLE_RESOLUTIONS);
	    }
	    
	}
	    
	    
	/*otherwise must reorder the ordered array of fill values and call
	  the function PGS_DEM_RecursiveSearch... again*/

	statusSmf = PGS_DEM_OrderIndicesSumPix(orderedArray, numFillPoints,
					    dataPoints, subsetNext,
					    &numSubgridCoverNext);
	
	if (statusSmf != PGS_S_SUCCESS)
	{
	    /*ERROR REORDERing array*/
	    statusSmf = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					      "Cannot reorganize points based "
					      "on subgrid value", 
					      "PGS_DEM_RecursiveSearchPix()");
	    return (PGSDEM_E_CANNOT_ACCESS_DATA);
	    
	}
	    

	/*Calls itself, increment resolution list, decriment number of
	  resolutions, pass in next resolution subsetInfo, pass in
	  reorganized orderedArray, and pass in the number of fill value
	  points*/

	statusSmf = PGS_DEM_RecursiveSearchPix((resolutionList + 1),
					       numResolutions - 1,
					       subsetInfoNext,
					       subsetNext,
					       dataPoints,
					       numSubgridCoverNext,
					       orderedArray,
					       numFillPoints);
	

	return(statusSmf);
	
    }

    /*fatal error-- this cleans up all the calloced space, closes all the
      opened files, and returns the error code*/
fatalError:
    {

	if (rowArray != NULL)
	{
	    free(rowArray);
	}
	if (colArray != NULL)
	{
	    free(colArray);
	}
	if (dataBuffInt8 != NULL)
	{
	    free(dataBuffInt8);
	}
	if (dataBuffInt16 != NULL)
	{
	    free(dataBuffInt16);
	}
	if (dataBuffFlt32 != NULL)
	{
	    free(dataBuffFlt32);
	}
	

	return (statusError);
    }
    

}
