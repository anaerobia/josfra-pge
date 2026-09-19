/*********************************************************
PGS_DEM_SortModels.c--

This function queries a region from a particular resolution and
layer.  It determines whether all the points are valid data, or whether some of
them have fill values. If all the points are valid data, it returns the
resolution tag of that resolution. If the function detects some fill values, it
extracts the data from the next resolution in the resolution
list. It continues to move through resolutions until it either finds a
resolution with no fill values, or it comes to the end of the resolutionList.
This function pasically is a wrapper around PGS_DEM_GetRegion. 

Author--
Alexis Zubrow

Dates--
First Programming       3/5/1997
Bug fix in stepping to next resolution in resolution list  8/15/97  Abe Taaheri
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>

PGSt_SMF_status 
PGS_DEM_SortModels(
    PGSt_DEM_Tag resolutionList[],  /*list of resolutions to access*/ 
    PGSt_integer numResolutions,    /*number of resolutions in list*/
    PGSt_integer layer,             /*layer to be accessed*/
    PGSt_integer positionCode,      /*position format, pixels or degrees*/
    PGSt_double latitude[],   /*latitude of upper left, and lower right pnts.*/
    PGSt_double longitude[],  /*longitude of upper left, and lower right pnts.*/
    PGSt_DEM_Tag *completeDataSet)  /*resolution in which region is complete*/
  
{
    
    /*return status*/
    PGSt_DEM_Tag nocompleteDataSet= -1; /* temporary completeDataSet tag */
    
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

    PGSt_integer i;                      /*looping index*/

    PGSt_DEM_SubsetRecord *subsetInfoOrig;   /*subset info for first resolution
					       and layer in resolutionList*/ 

    PGSt_DEM_SubsetRecord *subsetInfo;   /*subset info for resolution and 
					   layer*/
    PGSt_DEM_FileRecord **subset;        /*subset*/


    /*corner positions of data region, for pixel coordinate system*/
    PGSt_double crnRowPixel[2];         /*global pixel coordinates of next
					   resolution in resolutionList, upper
					   and lower pixels, respectively*/ 
    PGSt_double crnColPixel[2];         /*farthest West and East,
					   respectively. position of the corners
					   of the data region*/

    /*diagnostics for the region*/
    PGSt_integer numPixVert;        /*number pixels vertically spanning region*/
    PGSt_integer numPixHoriz;       /*number horizontally spanning region*/
    PGSt_integer pixBytes;          /*number of bytes in one pixel*/
    
    PGSt_byte *tempBuff = NULL;                 /*temporary buffer to hold region*/ 
    
    /**************
      ERROR TRAPPING
      **************/
    if ((positionCode != PGSd_DEM_PIXEL) && (positionCode != PGSd_DEM_DEGREE))
    {
	/*ERROR improper positionCode*/
	sprintf(dynamicMsg, "Improper Tag... (%d) is an improper "
		"positionCode.", positionCode);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					  "PGS_DEM_SortModels()");

	return(PGSDEM_E_IMPROPER_TAG);
    }
  
    if (numResolutions <= 0)
    {
	/*ERROR improper number of resolutions*/
	sprintf(dynamicMsg, "Improper Tag... (%d) is an improper "
		"number of resolutions.", numResolutions);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					  "PGS_DEM_SortModels()");
	
	return(PGSDEM_E_IMPROPER_TAG);
    }

    /*trapping for NULL pointers*/
    if (resolutionList == NULL)
    {
	/*ERROR  resolutionList is a NULL pointer*/
	sprintf(dynamicMsg, "Improper Tag... "
		"resolutionList equal to NULL");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
				       "PGS_DEM_SortModels()");
	return(PGSDEM_E_IMPROPER_TAG);
    }
    
 
    /***********************
      ACTUALLY ACCESS DATA
      *********************/

    /*what basically happens is that we loop through each resolution of
      resolutionList. for each resolution, we call PGS_DEM_GetRegion. based on
      the status return from _GetRegion, we can determine if there are any fill
      values in the data. If there are no fill values, then kick out of the loop
      and the function. If there are fill values, continue on to the next
      resolution. */
 
    for (i = 0; i < numResolutions; i++)
    {

	/*this is position code specific. slightly different procedures
	  depending on the coordinate system of the corner points*/

	if (positionCode == PGSd_DEM_PIXEL)
	{
	    /*get subset information for this resolution*/
	    status = PGS_DEM_Subset(resolutionList[i], layer, PGSd_DEM_INFO,
				    &subset, &subsetInfo);
	    if (status != PGS_S_SUCCESS)
	    {
		/*ERROR accessing subset and subset info*/
		sprintf(dynamicMsg, "Cannot access data.. "
			"error attempting to get subset information "
			"for resolution (%d), layer (%d)",
			resolutionList[0], layer);
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						  dynamicMsg, 
						  "PGS_DEM_SortModels()");
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal error*/
		goto fatalError;
		
	    }

	    /*if this is the first resolution in the resolution list, record
	      original subset information.*/
	    if (i == 0)
	    {
		subsetInfoOrig = subsetInfo;
	    }
	    
	    /*convert the original global pixels into  the global pixels
	      specific for this resolution.  In other words, converting from the
	      coordinate system which was valid for the first resolution, to the
	      coordinate system which is valid in the present resolution.*/

	    crnRowPixel[0] = PGSm_DEM_PixToPix(latitude[0], subsetInfoOrig,
					       subsetInfo);
	    crnRowPixel[1] = PGSm_DEM_PixToPix(latitude[1], subsetInfoOrig,
					       subsetInfo);
	    crnColPixel[0] = PGSm_DEM_PixToPix(longitude[0], subsetInfoOrig,
					       subsetInfo);
	    crnColPixel[1] = PGSm_DEM_PixToPix(longitude[1], subsetInfoOrig,
					       subsetInfo);
	    
	    /*Now need to allocate temporary space for the data region.  to
	      determine necessary size of the buffer, call PGS_DEM_GetSize*/
	    status = PGS_DEM_GetSize(resolutionList[i], layer, positionCode,
				     (PGSt_double *)crnRowPixel, 
				     (PGSt_double *)crnColPixel, 
				     &numPixVert, &numPixHoriz, &pixBytes);
	    if (status != PGS_S_SUCCESS)
	    {
		/*ERROR finding size of the space for the region*/
		sprintf(dynamicMsg, "Cannot access data... "
			"error determining size of buffer to store region.");
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						  dynamicMsg,
						  "PGS_DEM_SortModels()");
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal error*/
		goto fatalError;
	    }
	    
	    /*allocate space for temporary buffer*/
	    tempBuff = (PGSt_byte *) calloc((numPixVert*numPixHoriz), pixBytes);
	    if (tempBuff == NULL)
	    {
		/*ERROR callocing space*/
		sprintf(dynamicMsg, "Cannot access data... "
			"error allocating space for temporary buffer.");
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						  dynamicMsg,
						  "PGS_DEM_SortModels()");
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal error*/
		goto fatalError;
	    }
		
	    /*call PGSDEM_GetRegion. if the status return is success, return the
	      present resolution in parameter completeDataSet. if the status is
	      an error, continue to next resolution*/
	    status = PGS_DEM_GetRegion(&resolutionList[i], 1, layer,
				       positionCode, PGSd_DEM_NEAREST_NEIGHBOR,
				       crnRowPixel, crnColPixel, 
				       (void *)tempBuff, NULL, NULL, NULL);

	    free(tempBuff);
	    
	    if (status == PGS_S_SUCCESS)
	    {
		/*no fill values in data region. return present resolution*/
		*completeDataSet = resolutionList[i];
		return(PGS_S_SUCCESS);
	    }
	    else if ((status == PGSDEM_E_CANNOT_ACCESS_DATA) || 
		     (status == PGSDEM_E_IMPROPER_TAG)) 
	    {
		if(i == (numResolutions-1))
		{
		    /* last resolution checked. if previous resolutions had data
		       with fillvalues, return success with completeDataSet as 
		       PGSd_DEM_NO_COMPLETE_DATA */

		    if(nocompleteDataSet == PGSd_DEM_NO_COMPLETE_DATA)
		    {
			*completeDataSet = PGSd_DEM_NO_COMPLETE_DATA;
			return(PGS_S_SUCCESS);
		    }
		    else
		    {
			/*error in accessing data region*/
			/*fatal error*/
		    sprintf(dynamicMsg, "Cannot access data... "
			    "error in extracting data region.");
		    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						      dynamicMsg,
						      "PGS_DEM_SortModels()");
		    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		    
		    goto fatalError;
		    }
		}
		else
		{
		    /* go to next resolution */
		    goto contforloop;
		}
	    }
	    else if(status == PGSDEM_M_FILLVALUE_INCLUDED)
	    {
		/*fillvalue included in data for this resolution. register this
		  in nocompleteDataSet and go to next resolution */
		nocompleteDataSet = PGSd_DEM_NO_COMPLETE_DATA;
	    }
	    
	}
	
	else if (positionCode == PGSd_DEM_DEGREE)
	{
	    /*need to allocate temporary space for the data region.  to
	      determine necessary size of the buffer, call PGS_DEM_GetSize*/
	    status = PGS_DEM_GetSize(resolutionList[i], layer, positionCode,
				     latitude, longitude, &numPixVert,
				     &numPixHoriz, &pixBytes);
	    if (status != PGS_S_SUCCESS)
	    {
		/*ERROR finding size of the space for the region*/
		sprintf(dynamicMsg, "Cannot access data... "
			"error determining size of buffer to store region.");
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						  dynamicMsg,
						  "PGS_DEM_SortModels()");
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal error*/
		goto fatalError;
	    }
	    
	    /*allocate space for temporary buffer*/
	    tempBuff = (PGSt_byte *) calloc((numPixVert*numPixHoriz), pixBytes);
	    if (tempBuff == NULL)
	    {
		/*ERROR callocing space*/
		sprintf(dynamicMsg, "Cannot access data... "
			"error allocating space for temporary buffer.");
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						  dynamicMsg,
						  "PGS_DEM_SortModels()");
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal error*/
		goto fatalError;
	    }
		
	    /*call PGSDEM_GetRegion. if the status return is success, return the
	      present resolution in parameter completeDataSet. if the status is
	      an error, continue to next resolution*/
	    status = PGS_DEM_GetRegion(&resolutionList[i], 1, layer,
				       positionCode, PGSd_DEM_NEAREST_NEIGHBOR,
				       latitude, longitude, 
				       (void *)tempBuff, NULL, NULL, NULL);

	    free(tempBuff);
	    
	    if (status == PGS_S_SUCCESS)
	    {
		/*no fill values in data region. return present resolution*/
		*completeDataSet = resolutionList[i];
		return(PGS_S_SUCCESS);
	    }
	    else if ((status == PGSDEM_E_CANNOT_ACCESS_DATA) || 
		     (status == PGSDEM_E_IMPROPER_TAG)) 
	    {
		if(i == (numResolutions-1))
		{
		    /* last resolution checked. if previous resolutions had data
		       with fillvalues, return success with completeDataSet as 
		       PGSd_DEM_NO_COMPLETE_DATA */
		    if(nocompleteDataSet == PGSd_DEM_NO_COMPLETE_DATA)
		    {
			*completeDataSet = PGSd_DEM_NO_COMPLETE_DATA;
			return(PGS_S_SUCCESS);
		    }
		    else
		    {
			/*error in accessing data region */
			/*fatal error*/
		    sprintf(dynamicMsg, "Cannot access data... "
			    "error in extracting data region.");
		    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						      dynamicMsg,
						      "PGS_DEM_SortModels()");
		    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		    
		    goto fatalError;
		    }
		}
		else
		{
		    /* go to next resolution */
		    goto contforloop;
		}
	    }
	    else if(status == PGSDEM_M_FILLVALUE_INCLUDED)
	    {
		/*fillvalue included in data for this resolution. register this
		  in nocompleteDataSet and go to next resolution */
		nocompleteDataSet = PGSd_DEM_NO_COMPLETE_DATA;
	    }
	}
	
    contforloop:
	{	    
	}
    }
   
    
    /*If we have gotten to this point it means that none of the resolutions in
      the resolutionList have complete data in the specified region.
      i.e. we have data with fill values, so we should return success*/  

    *completeDataSet = PGSd_DEM_NO_COMPLETE_DATA;

    return(PGS_S_SUCCESS);
    

    /*fatal error*/
    
 fatalError:
    {
	if (tempBuff != NULL)
	{
	    free(tempBuff);
	}
	
	/*return appropriate error return*/
	return(statusError);
    }
}
