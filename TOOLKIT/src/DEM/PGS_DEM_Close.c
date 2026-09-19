/*********************************************************
PGS_DEM_Close.c--

This API closes the DEM access session. It assumes that all the layers marked
for closing, will be de-initialized on all of the resolutions in the resolution
list.



Author--
Alexis Zubrow

Dates--
First Programming       2/18/1997

*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>

PGSt_SMF_status 
PGS_DEM_Close(
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

    /*resolution and layer to be de-initialized*/
    PGSt_DEM_Tag resolution;
    PGSt_integer layer;
    

    PGSt_integer i;                      /*indexes for looping*/
    PGSt_integer j;
    
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
				       "PGS_DEM_Close()");
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
				       "PGS_DEM_Close()");
	return(PGSDEM_E_IMPROPER_TAG);
    }

    /*trapping for NULL pointers*/
    if (resolutionList == NULL)
    {
	/*ERROR  resolutionList is a NULL pointer*/
	sprintf(dynamicMsg, "Improper Tag... "
		"resolutionList equal to NULL");
	status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
				       "PGS_DEM_Close()");
	return(PGSDEM_E_IMPROPER_TAG);
    }
    
    if (layerList == NULL)
    {
	/*ERROR  layerList is a NULL pointer*/
	sprintf(dynamicMsg, "Improper Tag... "
		"layerList equal to NULL");
	status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
				       "PGS_DEM_Close()");
	return(PGSDEM_E_IMPROPER_TAG);
    }
    

    /**************
      FUNCTION
      **************/

    /*De-initialize each layer and each resolution*/
    for(i = 0; i < numResolutions; i++)
    {
	resolution = resolutionList[i];
	
	for (j = 0; j < numLayers; j++)
	{
	    layer = layerList[j];
	    
	    /*initialize the particular layer*/
	    status = PGS_DEM_Subset(resolution, layer, PGSd_DEM_DEINITIALIZE, 
				    NULL, NULL);
	    if (status != PGS_S_SUCCESS)
	    {
		/*ERROR closing the particular layer of particular
		  resolution*/
		sprintf(dynamicMsg,"Improper resolution (%d) and/or layer (%d)",
			resolutionList[i], layerList[j]);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Close()");
		return(PGSDEM_E_IMPROPER_TAG);
	    }
	}
    }
    

    /*Successfully completed*/
    return(PGS_S_SUCCESS);
}

	
