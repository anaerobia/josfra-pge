/****************************************************************************
PGS_DEM_Open.c--

This API opens the DEM access session.  It must be called oneach resolution and
layer that one wants to access.  It asssumes that all the layers marked for
initializing, will be initialized on all of the resolutions in the resolution
list.



Author--
Alexis Zubrow
Abe Taaheri

Dates--
First Programming       2/18/1997

Revision
6/05/2000        AT   Added functinality for 3km resolution
10/17/2000       AT   Added functinality for 3km resolution's
                      Land/sea mask
9/14/2011        AT   Added functinality for 500m (15 arcsec) resolution
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>

PGSt_SMF_status 
PGS_DEM_Open(
    PGSt_DEM_Tag resolutionList[],  /*list of resolutions to be initialized*/ 
    PGSt_integer numResolutions,    /*number of resolutions in list*/
    PGSt_integer layerList[],       /*list of layers to be initialized*/
    PGSt_integer numLayers)        /*number of layers in layerList*/
  
{
    
    /*return status*/
    PGSt_SMF_status  status;
    char errorBuf[PGS_SMF_MAX_MSG_SIZE]; /*Strings appended to error returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    char errorMsg[PGS_SMF_MAX_MSGBUF_SIZE];

    /*resolution and layer to be initialized*/
    PGSt_DEM_Tag resolution;
    PGSt_integer layer;
    

    PGSt_integer i;                      /*indexes for looping*/
    PGSt_integer j;
    PGSt_integer iresolution;            /*indexes for looping*/
    PGSt_integer jlayer;
    PGSt_integer data_exist = 0;
    
    /*resolution and layer which caused an error in initialization.  actually
      just the index of their respective lists*/
    PGSt_integer errorResolution = PGSd_DEM_NO_ERROR;
    PGSt_integer errorLayer = PGSd_DEM_NO_ERROR;
    
    /**************
      ERROR TRAPPING
      **************/

    /*error trapping for improper number of resolutions or layers*/
    if (numResolutions <= 0)
    {
	/*ERROR improper number of resolutions*/
	sprintf(dynamicMsg, "(%d) is an improper number of resolutions.", 
		numResolutions);
	status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	sprintf(errorMsg, errorBuf, dynamicMsg);
	status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
				       "PGS_DEM_Open()");
	return(PGSDEM_E_IMPROPER_TAG);
    }
    if (numLayers <= 0)
    {
	/*ERROR improper number of layers*/
	sprintf(dynamicMsg, "(%d) is an improper number of layers.", 
		numLayers);
	status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	sprintf(errorMsg, errorBuf, dynamicMsg);
	status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
				       "PGS_DEM_Open()");
	return(PGSDEM_E_IMPROPER_TAG);
    }

    /*trapping for NULL pointers*/
    if (resolutionList == NULL)
    {
	/*ERROR  resolutionList is a NULL pointer*/
	sprintf(dynamicMsg, "Improper Tag... "
		"resolutionList equal to NULL");
	status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
				       "PGS_DEM_Open()");
	return(PGSDEM_E_IMPROPER_TAG);
    }
    
    if (layerList == NULL)
    {
	/*ERROR  layerList is a NULL pointer*/
	sprintf(dynamicMsg, "Improper Tag... "
		"layerList equal to NULL");
	status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
				       "PGS_DEM_Open()");
	return(PGSDEM_E_IMPROPER_TAG);
    }

    /* trapping for layers that are not included 
       in the 90ARC and 15ARC DEM data */
    for(iresolution = 0; iresolution < numResolutions; iresolution++)
    {
      if((resolutionList[iresolution] == PGSd_DEM_90ARC) ||
	 (resolutionList[iresolution] == PGSd_DEM_15ARC))
	{
	    for( jlayer = 0; jlayer < numLayers; jlayer++)
	    {
		if((layerList[jlayer] == PGSd_DEM_ELEV) || 
		   (layerList[jlayer] == PGSd_DEM_WATER_LAND) )
		{
		  /* Layers elevation and land/sea mask are implemented */
		}
		else
		{
		  /* for 15 arcsec STD DEV Elev also inplemented */
		  if(layerList[jlayer] == PGSd_DEM_STDEV_ELEV)
		    {
		      /* Layer STD DEV Elev is implemented */
		    }
		  else
		    {
		      sprintf(dynamicMsg,"DEM data for layer (%d) of reolution (%d) does not exist.",
			      layerList[jlayer],resolutionList[iresolution]);
		      status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
						     "PGS_DEM_Open()");
		      data_exist = -1;
		    }
		}
	    }
	}
    }

    if(data_exist == -1)
    {
	return(PGSDEM_E_IMPROPER_TAG);
    }



    /**************
      FUNCTION
      **************/

    /*Initialize each layer and each resolution*/
    for(i = 0; i < numResolutions; i++)
    {
	resolution = resolutionList[i];
	
	for (j = 0; j < numLayers; j++)
	{
	    layer = layerList[j];
	    
	    /*initialize the particular layer*/
	    status = PGS_DEM_Subset(resolution, layer, PGSd_DEM_INITIALIZE,
				    NULL, NULL);
	    if (status != PGS_S_SUCCESS)
	    {
		/*ERROR initializing the particular layer of particular
		  resolution*/
		sprintf(dynamicMsg,"Improper resolution (%d) and/or layer (%d)",
			resolutionList[i], layerList[j]);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Open()");

		/*if problem opening any of the layers, then close all the
		  layers already intialized, unless it is the first resolution
		  and first layer*/
		if ((i == 0) && (j == 0))
		{
		    return(PGSDEM_E_IMPROPER_TAG);
		}

		/*record the resolution and layer which were not able to be
		  initialized*/ 
		errorResolution = i;
		errorLayer = j;
		
		/*kick us out of the initialization loop*/
		i = numResolutions;
		j = numLayers;
	    }
	}
    }
   
    /*Check if there was an error in initializing. IF there was, de-initalize
      the resolutions and layers which have already been initialized by this
      call to PGS_DEM_Open*/ 
    if ((errorResolution != PGSd_DEM_NO_ERROR) && 
	(errorLayer != PGSd_DEM_NO_ERROR))
    {
	/*close the layers*/
	for ( i = 0; i <= errorResolution; i++)
	{
	    resolution = resolutionList[i];
	    
	    for (j = 0; j < numLayers; j++)
	    {
		/*if this was the last layer that failed successfull 
		  initialization, kick out of loops, return error*/
 
		if ((i == errorResolution) && (j == errorLayer))
		{
		    return(PGSDEM_E_IMPROPER_TAG);
		}

		layer = layerList[j];
		
		status = PGS_DEM_Subset(resolution, layer,
					PGSd_DEM_DEINITIALIZE, NULL, NULL);
	    }
	}
    }
    

    /*Successfully completed*/
    return(PGS_S_SUCCESS);
}

	
