/**********************************************************
PGS_DEM_Interpolate--

This function actually performs the appropriate interpolation algorithm for
PGS_DEM_RecursiveSearchBil.

The term "bounding" refers to the pixels which surround the point of interest.
In this interpolation scheme, the bounding points are ideally the 4 nearest
pixels.

Assumptions: This program does not check to see if the bounding points are
outside the extent of the coordinate syste.  It also expects taht the bounding
points are equally spaced (in latitude and longitude).  In addition, the
algorithms expect that the bounding points are the 4 nearest pixels, ie. the
separation between each point is the size of one pixel at the present
resolution.


Note: I want to thank Peter Noerdlinger for his invaluable help with the
algorithmic development, especially with the problem of one fill value included
in the 4 bounding points.  The algorithjm from bilinear interpolation with all
4 values as valid points was adapted from the "Numerical Recipes in C" 2nd ed.,
pg 123.  The authors for the above source are Press,W., Teukolsky, S.,
Vetterling, W., and Flannery, B.


Author--
Alexis Zubrow/SAC
Peter Noerdlinger, PhD./SAC

Dates--
July 15, 1997       AZ/PN  First Created

**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_DEM.h>

PGSt_SMF_status
PGS_DEM_Interpolate(
    PGSt_DEM_SubsetRecord* subsetInfo,    /*Subset Information*/
    PGSt_DEM_PointRecordBil *interpPnts,  /*Information on 4 nearest points */
    PGSt_double boundingValue[4],         /*Values of the 4 nearest points*/
    PGSt_integer positionFill[4],         /*fill elements of interpPnts*/
    PGSt_integer numFillValues,           /*Number of fill elements*/
    PGSt_double *interpolationValue)      /*Value interpolated*/
{
    
    /*return status*/
    PGSt_SMF_status  statusReturn;       /*status returns for PGS_DEM tools*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

    /*Elements of the array defining the 4 bounding corners, create a right
      triangle with B being the right angle*/
    PGSt_integer elementA;
    PGSt_integer elementB;
    PGSt_integer elementC;
    
    /*Vectors used for determining if interpolation point is bound by 3 points.
      First element is latitude */

    PGSt_double vectorAyplusCy[2];        /* vector Ay plus vector Cy */
    PGSt_double vectorBy[2];              /* vector By */
    PGSt_double bound;                    /* if less than 0, point inside
					     triangle */

    PGSt_double normT;       /* Normalized distance from B to interpolation
				point, horizontal distance */
    PGSt_double normU;       /* Normalized distance between B and interpolation
				point, vertical distance */

    PGSt_double distT;       /* Normalized distance between interpolation point
				and the SW corner of bounding points, 
				horizontal distance*/

    PGSt_double distU;        /* Normalized distance between interpolation point
				and the SW corner of bounding points, vertical
				distance */ 
				
    
    /*Needed to calculate 3 point, non-bounded case */

    PGSt_double slopeHyp;            /* slopeof hypotenuse, AC */
    PGSt_double slopePerp;           /* slope of perpindicular to hypotenuse */
    PGSt_double intersection[2];     /* intersection of hypotenuse and
					perpindicular through the interpolation
					point. In decimal degrees, latitude is
					0th element. */
    
    PGSt_double boundPos[4][2];      /* position of the four bound points. the
					0th element is latitude, the 1st is
					longitude.  The other elements are
					dimension have the same meaning as
					boundingPnts (of the stuct
					PGSt_DEM_PointRecordBil): 0th element--
					SW Point, 1st-- SE point, 2nd-- NE
					point, 3rd element-- NW point. */
    
    PGSt_double interpPos[2];        /* position of interpolation point. 0th
					element is latitude. */
			    
    
    


    /*At this point we are ready to perform our bilinear interpolation.  There
      are 4 possible branches. (1) If more than one point is a fill value,
      assign nearest neighbor value to this point.  (2) If only one element is a
      fill value, check if the nearest element is a fill. if it is, assign it's
      value to the element.  (3) If it is not, see if one can interpolate using
      three values.  To successfully interpolate using three values, need to
      create a surface and one needs to know if the three remaining pixel values
      geographically surround the point of interest.  If they do,
      interpolate. (4) if they do not "bound" the point, then one will perform a
      distance weighted average. Drop the perpindicular from the point of
      interest to the hypotenous of the triangle formed by the three non-fill
      value points.  Use the distance defined by the intersection of the
      perpindicular and the hypotenous to perform a weighted average between the
      two corners defining the hypotenouse. (5) If none of the bounding points
      are fill values, do a straight bilinear interpolation. In the above
      branches, if one has to assign a fill value to the interpolationValue,
      return PGSDEM_M_FILLVALUE_INCLUDED, else return PGS_S_SUCCESS. */

	
    /*record the 4 bounding points and the interpolation position in
      separate arrays for calculations.  Check if the interpolation point
      and/or the bounding points "wrap" across the international date line.
      If they do, need to extend the coordinate system to those position
      values. */
    
    boundPos[0][0] = interpPnts -> boundingPnts[0][0];
    boundPos[0][1] = interpPnts -> boundingPnts[0][1];
    boundPos[1][0] = interpPnts -> boundingPnts[1][0];
    boundPos[1][1] = interpPnts -> boundingPnts[1][1];
    boundPos[2][0] = interpPnts -> boundingPnts[2][0];
    boundPos[2][1] = interpPnts -> boundingPnts[2][1];
    boundPos[3][0] = interpPnts -> boundingPnts[3][0];
    boundPos[3][1] = interpPnts -> boundingPnts[3][1];
    
    interpPos[0] = interpPnts -> positionInterp[0];
    interpPos[1] = interpPnts -> positionInterp[1];
    
    /*Check if bounding points have wrapped across international date line*/
    if (boundPos[0][1] > boundPos[1][1])
    {
	/* wrapping condition. longitude of western point is greater than the
	   longitude of the eastern point. recalculate the longitude, so that
	   the 4 points are in a continuous coordinate system. This calculation
	   of new position ASSUMES that the bounding points are only separated
	   by the distance of ONE pixel. in addition, check if interpolation
	   point has wrapped. */

	boundPos[1][1] = boundPos[0][1] + 
	  (1.0 / (PGSt_double)(subsetInfo -> pixPerDegree));
	boundPos[2][1] = boundPos[3][1] + 
	  (1.0 / (PGSt_double)(subsetInfo -> pixPerDegree));
	
	if (interpPos[1] < boundPos[0][1])
	{
	    /* interpolation point has wrapped across international date line,
	       recalculate longitude in the temporary coordinate system. Take
	       the longitude of the western boundary, add the difference in the
	       longitude from the interpolation point to -180 degrees and half
	       the size of 1 pixel.  The wrapping interpolation position case
	       should only occur when the data is located in the center of the
	       pixel. This calculation ASSUMES that the bounding points are 
	       only one pixel apart.*/

	    interpPos[1] = boundPos[0][1] + (interpPos[1] + 180.0) +
	      (0.5 / (PGSt_double)(subsetInfo -> pixPerDegree));
	}
    }
	
    /***********************/
    /* Interpolation cases */
    /***********************/

    if (numFillValues > 1)
    {
	/* more than one fill point.  Assign nearest neighbor to
	   interpolationValue. interpPnts -> nearest is the nearest neighbor
	   element of the array bounding values. Check if nearest neighbor is a
	   fill value.*/

	*interpolationValue = boundingValue[interpPnts -> nearest];
	if(*interpolationValue == subsetInfo -> fillvalue)
	{
	    statusReturn = PGSDEM_M_FILLVALUE_INCLUDED;
	}
	else
	{
	    statusReturn = PGS_S_SUCCESS;
	}
	
	
    }
	
    else if(numFillValues == 0)
    {
	/*no fill points, do a straight bilinear interpolation. */

	/*calculate distT and distU */
	distT = (interpPos[1] - boundPos[0][1]) / 
	  (boundPos[1][1] - boundPos[0][1]);
	
	distU = (interpPos[0] - boundPos[0][0]) /
	  (boundPos[3][0] - boundPos[0][0]);
	
	/* Bilinear interpolation, utilizing all 4 bounding points */
	*interpolationValue = 
	  ((1 - distT) * (1- distU) * boundingValue[0]) + 
	  (distT * (1 - distU) * boundingValue[1]) +
	  (distT * distU * boundingValue[2]) +
	  ((1 - distT) * distU * boundingValue[3]);

	statusReturn = PGS_S_SUCCESS;
	
	
    }
    
    else if (numFillValues == 1)
    {
	/* If nearest neighbor to the interpolation point is a fill value,
	   assign interpolationValue to fill and return 
	   PGSDEM_M_FILLVALUE_INCLUDED */

	if (boundingValue[interpPnts -> nearest] == (subsetInfo -> fillvalue))
	{
	    *interpolationValue = subsetInfo -> fillvalue;
	    return(PGSDEM_M_FILLVALUE_INCLUDED);
	}
	

	/*if the nearest neighbor is not a fill value, then we have to determine
	  if the interpolation point is "bound" by the other three
	  points. To do this we perform a bit of simple vector math.  Imagine
	  that the 3 points which have real values create the triangle ABC. Let
	  AC define the hypotenuse of this right triangle.  If we do a little
	  vector manipulation we can determine if the point is inside or outside
	  of the triangle ABC. If the interpolation point is "y", then
	  calculate the following expression (note: Ay is the vector between A
	  and y):

	  (Ay + Cy) dot product with By 
	  
	  If this expression is less than 0, the point is within the bounding
	  triangle. Otherwise, the point is outside the triangle.*/

	/*Figure out which element is A, B and C of the position Arrays*/
	elementA = (positionFill[0] + 1) % 4;
	elementB = (positionFill[0] + 2) % 4;
	elementC = (positionFill[0] + 3) % 4;

	/*Calculate the vector addition of Ay and Cy, and the vector By*/
	vectorAyplusCy[0] = (2 * interpPos[0]) -
	  boundPos[elementA][0] - boundPos[elementC][0];
	vectorAyplusCy[1] = (2 * interpPos[1]) -
	  boundPos[elementA][1] - boundPos[elementC][1];
	vectorBy[0] = interpPos[0] - boundPos[elementB][0];
	vectorBy[1] = interpPos[1] - boundPos[elementB][1];

	/*Determine if point is inside the triangle ABC. If bound is less
	  than 0, it is inside the triangle */
	bound = (vectorAyplusCy[0] * vectorBy[0]) + 
	  (vectorAyplusCy[1] * vectorBy[1]);
	
	if (bound < 0)
	{
	    /* interpolation point is bound by the three points.  In this case,
	       one can perform bilinear interpolation with the three points. The
	       variables normT and normU are the normalized vertical and
	       horizontal distance, respectively.  To see a definition of them,
	       and some of the impetuse for this nomencleture, look at "Numerical
	       Recipes in C", pg. 123.  The actual algorithm is derived from
	       solving the problem of a paralellpiped object with a volume equal
	       to 0 (ie. it is flat). To do this need to set the determinate
	       equal to 0.  After solving, get the following algorithm:
	       
	       interpValue = normT * (valueA - valueB) + valueB - 
	          normU * (valueB - valueC)

	       The orientation of the elements is not important for this
	       calculation.  On the other hand, element B is a very
	       specific element of the 4 bounding corners.  Element B is
	       the element which is directly opposite the fill value. */

	    normT = (interpPos[1] - boundPos[elementB][1]) /
	      (boundPos[elementA][1] -  boundPos[elementB][1]);
	    
	    normU = (interpPos[0] - boundPos[elementB][0]) /
	      (boundPos[elementC][0] - boundPos[elementB][0]);
	    
	    
	    *interpolationValue = normT * 
	      (boundingValue[elementA] - boundingValue[elementB]) +
	      boundingValue[elementB] - (normU * (boundingValue[elementB] -
						  boundingValue[elementC])); 

	    statusReturn = PGS_S_SUCCESS;

	}

	else
	{
	    /* the point of interest is not within the 3 points.  In this case,
	       drop the perpindicular between the point of interest and the
	       hypotenuse.  The point on the hypotenuse that marks the
	       intersection of the hypotenuse with the perpindicular will be
	       used for a distance weighted average. */

	    slopeHyp = (boundPos[elementC][0] - boundPos[elementB][0]) /
	      (boundPos[elementA][1] - boundPos[elementB][1]);
	    slopePerp = -slopeHyp;
	    
	    /*Calculate the intersection point of the hypotenuse and the
	      perpindicular */

	    intersection[1] = 
	      ((slopeHyp * boundPos[elementA][1]) -
	       (slopePerp * interpPos[1]) + 
	       interpPos[0] + boundPos[elementA][0]) /
	      (slopeHyp - slopePerp);
	    
	    intersection[0] = slopeHyp * 
	      (intersection[1] - boundPos[elementA][1]) +
	      boundPos[elementA][0];
	    
	    
	    /*Calculate the change in "value" (ie. elevation, etc.) from point A
	      to the interpolation value.  Add this delta to value at pointA to
	      determine the interpolationValue. To find the delta value, simply
	      use the relative distance in longitude to the interpolation
	      point.  If the value of A and C are the same, return the value. */

	    if (boundingValue[elementA] == boundingValue[elementC])
	    {
		/*prevent dividing by 0.  In this case, the slope of the line
		  between the two points is flat; therefore, the
		  interpolationValue equals the value at A and equals the Value
		  at B */

		*interpolationValue = boundingValue[elementA];

		statusReturn = PGS_S_SUCCESS;
		
	    }
	    else
	    {
		
		*interpolationValue = boundingValue[elementA] +
		  ((boundingValue[elementC] - boundingValue[elementA]) *
		   (interpPos[1] - boundPos[elementA][1]) /
		   (boundPos[elementC][1] - boundPos[elementA][1]));
	    
		statusReturn = PGS_S_SUCCESS;
	    }
	    
	}
    }

    else
    {
	/* Improper inputs to function */
	sprintf(dynamicMsg, "Improper Input..."
		"improper number of fill values (%d)", numFillValues);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG,
					  dynamicMsg, "PGS_DEM_Interpolate()");
	statusReturn = PGSDEM_E_IMPROPER_TAG;
    }

    /* return appropriate status */
    return(statusReturn);
    
}

