/**********************************************************
PGS_DEM_GetBoundingPnts--

This function determines the 4 points for the bilinear interpolation functions,
PGS_DEM_Interpolate and PGS_DEM_RecursiveSearchBil. It uses the position if the
interpolation point to determine the 4 nearest points.  It check to see if any
of these 4 points cross the international date line, "wrap".  If any of these
points cross the line, it determines the geographical coordinates of these
points.

The term "bounding" refers to the pixels which surround the point of interest.

Assumptions: Both PGS_DEM_Interpolation and PGS_DEM_RecursiveSearchBil assume
that the 4 points selected by this function are the 4 nearest points.  In other
words, the 4 points create a square around the point of interest with edge size
of one pixel. Also assumes that the interpoaltion point has been checked for
reasonableness. Point must be within the geographic extent of the world
coordinate system.


Author--
Alexis Zubrow/SAC


Dates--
July 22, 1997       AZ  First Created

**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_DEM.h>


PGSt_SMF_status
PGS_DEM_GetBoundingPnts(
    PGSt_DEM_SubsetRecord* subsetInfo,    /*Subset Information*/
    PGSt_DEM_PointRecordBil *interpPnts)  /*Information on 4 nearest points */
{
    
    PGSt_integer i;
    

    /*Nearest neighbor to interpolation point. 0th element is latitude. Global
      Pixel and degree format */

    PGSt_integer nearestPix[2];     
    PGSt_double nearestDeg[2];
    
    /*Delta distance in latitude and longitude.  Distance between interpolation
      point and the nearest neighbor */

    PGSt_double diffLat;
    PGSt_double diffLon;
  
    /* A second bounding position. 0th element is latitude */
    PGSt_integer boundPix[2];          /*In global pixels */
    PGSt_double boundDeg[2];           /*In decimal degrees */
    

    PGSt_integer maxHorizPixel;        /*maximum pixel value, horizontal (lon)
					 position*/



    /*calculate the largest possible horizontal pixel value. */
    maxHorizPixel = ((subsetInfo -> horizPixSubgrid) * 
		     (subsetInfo -> subgridHoriz))  - 1;

    /* calculate the Nearest neighbor */
    nearestPix[0] = PGSm_DEM_LatToPixel(interpPnts -> positionInterp[0],
					subsetInfo);
    nearestPix[1] = PGSm_DEM_LonToPixel(interpPnts -> positionInterp[1],
					subsetInfo);
    
    /* find the location of nearest neighbor in decimal degree format */
    nearestDeg[0] = PGSm_DEM_PixelToLat(nearestPix[0], subsetInfo);
    nearestDeg[1] = PGSm_DEM_PixelToLon(nearestPix[1], subsetInfo);
    
    /*Calculate the difference in latitude and longitude betweent he nearest
      neighbor and the interpolation point */

    diffLat = (interpPnts -> positionInterp[0]) - nearestDeg[0];
    diffLon = (interpPnts -> positionInterp[1]) - nearestDeg[1];
    
    /* Determine the other bounding latitude and longitude. Check for the
       "wrapping" condition.  If the interpolation point forces the bounding
       square to cross the international date line, then one needs to
       recalculate the "wrapped" end of the bounding square.  Remember it is
       important to fill the array interpPnts -> boundingPnts[4][2] to maintain
       the following order of elements: 0th-- SW point, 1st-- SE point, 
       2nd-- NE point, 3rd-- NW point.*/

    if (diffLat > 0)
    {
	/* interpolation Point is North of nearest neighbor. Assign nearest end
	   to southern elements of the array interpPnts -> boundingPnts
	   (ie.. 0th and 1st elements).  Calculate the Northern extent of
	   bounding region. Use global pixels. */

	interpPnts -> boundingPnts[0][0] = nearestDeg[0];
	interpPnts -> boundingPnts[1][0] = nearestDeg[0];
	
	/* Calculate Northern extent */
	boundPix[0] = nearestPix[0] - 1;
	
	boundDeg[0] = PGSm_DEM_PixelToLat(boundPix[0], subsetInfo);

	interpPnts -> boundingPnts[2][0] = boundDeg[0];
	interpPnts -> boundingPnts[3][0] = boundDeg[0];

    }

    else
    {
	/* Interpolation point is south of nearest neighbor */
	interpPnts -> boundingPnts[2][0] = nearestDeg[0];
	interpPnts -> boundingPnts[3][0] = nearestDeg[0];
	
	/* Calculate Southern extent */
	boundPix[0] = nearestPix[0] + 1;
	
	boundDeg[0] = PGSm_DEM_PixelToLat(boundPix[0], subsetInfo);

	interpPnts -> boundingPnts[0][0] = boundDeg[0];
	interpPnts -> boundingPnts[1][0] = boundDeg[0];
    }

    if (diffLon > 0)
    {
	/* Interpolation point is East of its nearest neighbor. Assign nearest
	   longitude to Western elements of the array interpPnts ->
	   boundingPnts[4][2] (ie. 0th and 3rd elements). Calculate Eastern
	   extent of bounding region. */

	interpPnts -> boundingPnts[0][1] = nearestDeg[1];
	interpPnts -> boundingPnts[3][1] = nearestDeg[1];

	/*Calculate Eastern extent*/
	boundPix[1] = nearestPix[1] + 1;
	
	/*Check for "wrapping" condition*/
	if (boundPix[1] > maxHorizPixel)
	{
	    /* Region crosses international Date Line. Change eastern extent to
	      the first pixel on the Eastern side of international date line */
	    boundPix[1] = 0;
	}
	
	boundDeg[1] = PGSm_DEM_PixelToLon(boundPix[1], subsetInfo);
	
	interpPnts -> boundingPnts[1][1] = boundDeg[1];
	interpPnts -> boundingPnts[2][1] = boundDeg[1];
    }
    
    else
    {
	/* Interpolation point is West of nearest neighbor */
	interpPnts -> boundingPnts[1][1] = nearestDeg[1];
	interpPnts -> boundingPnts[2][1] = nearestDeg[1];

	/*Calculate Western Extent */
	boundPix[1] = nearestPix[1] - 1;
	
	/*Check for wrapping condition*/
	if (boundPix[1] < 0)
	{
	    /* Region crosses international Date Line. Change Western extent to
	      the first pixel on the Western side of international date line */
	 
	    boundPix[1] = maxHorizPixel;
	}
	    
	      
	
	boundDeg[1] = PGSm_DEM_PixelToLon(boundPix[1], subsetInfo);
	
	interpPnts -> boundingPnts[0][1] = boundDeg[1];
	interpPnts -> boundingPnts[3][1] = boundDeg[1];
    }

    /*Determine which element of boundingPnts is the nearest neighbor */
    for (i = 0; i < 4; i++)
    {
	if(((interpPnts -> boundingPnts[i][0]) == nearestDeg[0]) &&
	   ((interpPnts -> boundingPnts[i][1]) == nearestDeg[1]))
	{
	    /*found nearest neighbor. record the element number in the struct
	      interpPnts */
	    interpPnts -> nearest = i;
	    i = 4;
	}
    }
    
    return(PGS_S_SUCCESS);
}

