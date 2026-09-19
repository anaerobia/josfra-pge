/*********************************************************
PGS_DEM_ReplaceFillPoints.c--

This function reads the region of data which has been extracted and records any
fillvalues. This function has two different branches.  If the number of
resolutions in the resolutionList is equal to 1, it only searches until it finds
the first fill value point.  This branch is solely for error reportoing. In
other words, if the number of resolutions equals one, the function only needs to
determine if there are any fill values in the region.  The second branch, only
is invoked if the number of resolutions is greater than 1.  In this case, every
fill value point is recorded in a PointRecord array. This PointRecord array
becomes the source of a second search on these points.  In order to
interpolate their values from the next resolution in the resolutionList, the
points are passed to PGS_DEM_RecursiveSearch. This function assumes that the
corner pixels of the region were checked to see if they were reasonable.

All this function actual;ly does is switch between the data type.  These
searches were divided by data type. Eventually, these functions will be
rewritten so there is not 3 versions to maintain.


Author--
Alexis Zubrow

Dates--
First Programming       2/25/1997

*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>
#include <PGS_MEM.h>

PGSt_SMF_status
PGS_DEM_ReplaceFillPoints(
    PGSt_DEM_Tag resolutionList[],  /*list of resolutions to access*/ 
    PGSt_integer numResolutions,    /*number of resolutions in list*/
    PGSt_integer layer,             /*layer to be accessed*/
    PGSt_integer interpolation,     /*interpolation flag*/
    void *dataRegionIn,               /*data buffer for output*/
    PGSt_DEM_SubsetRecord *subsetInfo,  /*subset Information*/
    PGSt_integer cornerRowPixels[2],    /*corners of region, row pixels*/
    PGSt_integer cornerColPixels[2])    /*corners of region, column pixels*/

  
{
    
      /*return status*/
    PGSt_SMF_status  status;             /*status returns for PGS_DEM tools*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];



    switch(subsetInfo -> dataType)
    {
      case DFNT_INT8:
	status = PGS_DEM_ReplaceFillPointsInt8(resolutionList,
					       numResolutions, layer,
					       interpolation, dataRegionIn,
					       subsetInfo, cornerRowPixels,
					       cornerColPixels);
	break;
	
      case DFNT_INT16:
	status = PGS_DEM_ReplaceFillPointsInt16(resolutionList,
						numResolutions, layer,
						interpolation, dataRegionIn,
						subsetInfo, cornerRowPixels,
						cornerColPixels);
	break;
	
      case DFNT_FLOAT32:
	status = PGS_DEM_ReplaceFillPointsFlt32(resolutionList,
						numResolutions, layer,
						interpolation, dataRegionIn,
						subsetInfo, cornerRowPixels,
						cornerColPixels);
	break;

      default:
	/*ERROR data type not currently recognized*/
	sprintf(dynamicMsg, "Cannot Access Data... "
		"data type (%d) not currently reecognized", 
		subsetInfo -> dataType);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, dynamicMsg,
                              "PGS_DEM_ReplaceFillPoints()");
	return(PGSDEM_E_CANNOT_ACCESS_DATA);
    }

    /*return status from the appropriate data type ReplaceFillPoints */
    return(status);
    
}

	
