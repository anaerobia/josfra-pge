/*********************************************************
PGS_DEM_DataPresent.c--

This function queries a number of points from a particular resolution and
layer.  It determines whether all the points are valid data, or whether some of
them have fill values. This function pasically is a wrapper around
PGS_DEM_GetPoint. 

Author--
Alexis Zubrow

Dates--
First Programming       3/3/1997

*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>

PGSt_SMF_status 
PGS_DEM_DataPresent(
    PGSt_DEM_Tag resolution,        /*resolution to access*/ 
    PGSt_integer layer,             /*layer to be accessed*/
    PGSt_integer positionCode,      /*position format, pixels or degrees*/
    PGSt_double pntLatitude[],      /*latitude values of the points*/
    PGSt_double pntLongitude[],     /*longitude values of the points*/
    PGSt_integer numPoints,         /*number of points to access*/
    PGSt_boolean *dataPresent)      /*true if no fill values, else false*/
  
{
    
    /*return status*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    PGSt_SMF_status statusReturn;        /*status returned by _DataPresent*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];


    PGSt_integer numResolutions;         /*number resolutions to access*/
    PGSt_DEM_Tag resolutionList[PGSd_DEM_MAX_RESOLUTIONS];  /*resolutions to
							      access, from
							      PGS_DEM_GetPoint*/
          
    /*intialize the inputs for PGS_DEM_GetPoint*/
    numResolutions = 1;
    resolutionList[0] = resolution;
    
    /*call GetPoint.  use nearest neighbor interpolation*/
    status = PGS_DEM_GetPoint(
	resolutionList, numResolutions, layer, positionCode, pntLatitude,
	pntLongitude, numPoints, PGSd_DEM_NEAREST_NEIGHBOR, NULL);

    /*check status returns*/
    switch(status)
    {
	
      case PGS_S_SUCCESS:
	/*no fill values in return data*/
	*dataPresent = PGS_TRUE;
	statusReturn = status;
	break;
	
      case PGSDEM_M_FILLVALUE_INCLUDED:
	/*fill values included in the data*/
	*dataPresent = PGS_FALSE;
	
	/*upgrade status to success, this a successful result for _DataPresent*/
	statusReturn = PGS_S_SUCCESS;	
	break;
	
      case PGSDEM_E_IMPROPER_TAG:
	/*error improper tag*/
	sprintf(dynamicMsg, "Improper tag... improper input(s) to function.");
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
				  "PGS_DEM_DataPresent()");
	statusReturn = status;
        break;
	
      default:
	/*cannot access data for some other reason*/
	sprintf(dynamicMsg, "Cannot access data... from resolution (%d) " 
		"and layer (%d)", resolution, layer);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					  dynamicMsg, "PGS_DEM_DataPresent()");
	statusReturn = PGSDEM_E_CANNOT_ACCESS_DATA;
	
    }
    
    /*return appropriate status*/
    return(statusReturn);
    
}
