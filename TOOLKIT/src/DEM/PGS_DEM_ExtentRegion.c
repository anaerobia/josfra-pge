/*********************************************************
PGS_DEM_ExtentRegion.c--

This function helps determine the extent of the region to be extracted.  In this
function, extent means the number of subgrids covered by a region.  The function
takes the upper left, and the lower right corners of a region and calculates
the number of subgrids covered horizontaly and vertically.  In addition, it
takes those subgrids covered and initializes some space for their respective
RegionRecords.  Then the function calculates the subregions extent. Finally,
the function calculates the total number of pixels in the subregion.  This
function assumes that input coordinates have already been checked for
reasonableness. It also assumes that all the subgrids covered by the region are
staged.  It will NOT give back an error return if one attempts to find a
region's extent over unstaged subgrids.  On the other hand, it also will not
segmentation fault.  This means, that one MUST test that the subgrid is actually
staged before attempting to extract data.  I do this test in
PGS_DEM_ExtractRegion. 

Added wrapping capabilities.  If region requested overlaps the 180West/180East
boundary, can now access the data and stitch it into a continous region.


Author--
Alexis Zubrow

Dates--
2/24/1997      AZ           First Programming
5/14/1997      AZ           Added wrapping capability
12/8/1997      Abe Taaheri  Fixed leaking memory associated with multiple 
                            callocs for extractedRegion[subgridTemp]
12/8/1998      AT           Fixed problem with extracting region when the
                            region wraps across the 180 degrees east and
			    both upperleft corner and upperright corner of
			    the region (or lowerleft corner and lowerright 
			    corner) fall inside the same subgrid.

*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>
#include <PGS_MEM.h>


PGSt_SMF_status
PGS_DEM_ExtentRegion(
    PGSt_integer cornerRowPixels[2],    /*corners of region, row pixels*/
    PGSt_integer cornerColPixels[2],    /*corners of region, col pixels*/
    PGSt_DEM_SubsetRecord *subsetInfo,        /*subset Information*/
    PGSt_DEM_RegionRecord **extractedRegion,  /*Array of subregions*/
    PGSt_integer *extentSubgridsVert,   /*number subgrids vertically spanned*/
    PGSt_integer *extentSubgridsHoriz)  /*number subgrids horizontally spanned*/

{
      /*return status*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

    PGSt_integer i;                      /*looping index*/
    PGSt_integer j;                      /*looping index*/


    PGSt_integer subgridUpprLft;       /*subgrid value of upper left pixel*/
    PGSt_integer subgridUpprRgt;       /*subgrid value of upper right*/
    PGSt_integer subgridLwrLft;        /*subgrid value of lower left*/
    
    
    
    PGSt_integer subgridTemp;          /*Temporary subgrid, presently
					 analyzing*/ 

    PGSt_integer vertPixSubregion;     /*number pixels vertically spanning
					 subregion*/ 
    PGSt_integer horizPixSubregion;    /*number horizontal pixels*/
    PGSt_integer subregionSize;        /*the size of the subregion, interms of
					 number of pixels*/
    
    PGSt_integer crnRowSubgrid[2];     /*Global coordinates of upper and lower
					 most pixels of a subgrid, in pixels*/
    PGSt_integer crnColSubgrid[2];     /*global coordinates of farthest East and
					 West pixel in a subgrid, first and
					 second element, respectively.*/

    PGSt_integer maxHorizPixel;          /*maximum pixel value, horizontal (lon)
					   position*/

    /*information from subsetInfo*/
    PGSt_integer numSubgridsVert;  /*number subgrids vertically spanning world*/
    PGSt_integer pixVertSubgrid;   /*number vertical pixels in a subgrid*/ 
    PGSt_integer pixHorizSubgrid;  /*number horizontal pixels in a subgrid*/


    /*Wrapping case, when the region wraps around from Eastern Hemisphere to
      Western Hemisphere. */
    PGSt_integer subgridWestBndry;     /*subgrid value of Western Boundary*/
    PGSt_integer subgridEastBndry;     /*subgrid value of Eastern Boundary,
					 these are the subgrids at the same
					 latitude as the upper left
					 subgrid. only important for wrapping
					 case */
    PGSt_integer extentSubgridsRgt;    /*Number of subgrids for upper left to
					 East boundary*/
    PGSt_integer extentSubgridsWrpd;   /*number of subgrids from West Boundary
					 to upper right subgrid*/
    

    /*Initialize the subgrid diagnostics for this particular subset*/
    numSubgridsVert = subsetInfo -> subgridVert;
    pixVertSubgrid = subsetInfo -> vertPixSubgrid;
    pixHorizSubgrid = subsetInfo -> horizPixSubgrid;
    

    /*Calculate the subgrids values for the 4 corner points of the region to be
      extracted*/  
    subgridUpprLft = PGSm_DEM_PixToSubgrid(cornerRowPixels[0],
					  cornerColPixels[0], subsetInfo);
    subgridUpprRgt = PGSm_DEM_PixToSubgrid(cornerRowPixels[0],
					  cornerColPixels[1], subsetInfo);
    subgridLwrLft = PGSm_DEM_PixToSubgrid(cornerRowPixels[1],
					  cornerColPixels[0], subsetInfo);
    /*calculate the largest possible pixel value for vertical and
      horizontal. */
    maxHorizPixel = ((subsetInfo -> horizPixSubgrid) * 
		     (subsetInfo -> subgridHoriz))  - 1;

    /* Calculate the boundary subgrids at the same latitude of the Upper left
       subgrid. */
    subgridEastBndry = PGSm_DEM_PixToSubgrid(cornerRowPixels[0],
					     maxHorizPixel, subsetInfo);
    subgridWestBndry = PGSm_DEM_PixToSubgrid(cornerRowPixels[0], 0,
					     subsetInfo);
    /*Check if region wraps across 180E/180W */
    if ((subgridUpprLft > subgridUpprRgt) || 
	(cornerColPixels[0] > cornerColPixels[1]))
    {
	/*Wrapping condition. calculate the number of subgrids
	  from Upper Left subgrid to the Eastern Boundary, and the number of
	  subgrids from Western Boundary to the Upper right subgrid.*/

	extentSubgridsRgt = ((subgridEastBndry - subgridUpprLft)/
			    numSubgridsVert) + 1;
	extentSubgridsWrpd = ((subgridUpprRgt - subgridWestBndry)/
			    numSubgridsVert) + 1;
	
	/*the total number of subgrids horizontally spanned*/
	*extentSubgridsHoriz = extentSubgridsRgt + extentSubgridsWrpd;
	
    }
    else
    {
	/*total number of subgrids horizontally spanned*/
	*extentSubgridsHoriz = (((subgridUpprRgt - subgridUpprLft)/
				 numSubgridsVert) + 1);
	
    }
    
    /*Determine the number of subsets vertically covered*/
    *extentSubgridsVert = subgridLwrLft - subgridUpprLft + 1;
    

    /*At this point divide the region into the component subregions.  First, we
      determine the extent of a subregion over a particular subgrid.  We calloc
      a _RegionRecord for each subgrid covered.  Then we need to calculate the
      cooresponding subregion's size interms of the internal pixels of a
      particular HDF-EOS GRID. Finally we check if this region Wraps around the
      180E/180W boundary.  If this region does wrap, we shift the appropriate
      subregions to the subgrids starting at the Western Boundary.  NOTE: if one
      changes this for looping stucture, then one must maintain that the last
      subgrids indicated are those farthest South or farthest to the "right". If
      one doesn't, the IF statements are no longdr valid.*/

    /*Cycle through each subgrid covered by the region*/
    for(i = 0; i < *extentSubgridsVert; i++)
    {
	/*Calculate the left most subgrids of one's region*/
	subgridTemp = subgridUpprLft + i;
	
	for(j = 0; j < *extentSubgridsHoriz; j++)
	{    

	    /*Calloc space for the RegionRecord for this subgrid
	      only if its has not been done before*/
	    /* following if statement added to fix memory leak problem
	       when bilinear interpolation is used to interpolate 
	       fillvalues 12/8/97 */
	    if((*extentSubgridsHoriz > 1) && 
	       ((j+1) == *extentSubgridsHoriz) && 
	       (subgridUpprLft == subgridUpprRgt))
	    {
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		if(extractedRegion[subgridTemp + subsetInfo -> numSubgrids]
		   == NULL)
		{
		    
		    extractedRegion[subgridTemp + subsetInfo -> numSubgrids] = 
		      (PGSt_DEM_RegionRecord *) calloc(1, 
                             sizeof(PGSt_DEM_RegionRecord));
		}
		if (extractedRegion[subgridTemp + subsetInfo -> numSubgrids] == NULL)
		{
		    /*ERROR callocing*/
		    sprintf(dynamicMsg, "Error allocating memory for "
			    "subregion data buffers");
		    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,dynamicMsg,
                                          "PGS_DEM_ExtentRegion()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal error*/
		    goto fatalError;
		}
		
	    }
	    else
	    {
		if(extractedRegion[subgridTemp] == NULL)
		{
		    extractedRegion[subgridTemp] = (PGSt_DEM_RegionRecord *)
		      calloc(1, sizeof(PGSt_DEM_RegionRecord));
		}
		
		if (extractedRegion[subgridTemp] == NULL)
		{
		    /*ERROR callocing*/
		    sprintf(dynamicMsg, "Error allocating memory for "
			    "subregion data buffers");
		    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,dynamicMsg, 
                                          "PGS_DEM_ExtentRegion()");
		    statusError = PGSMEM_E_NO_MEMORY;
		    
		    /*fatal error*/
		    goto fatalError;
		    
		}
	    }
		

	    
	    /*Now calculate the outermost extent of the pixels of this subgrid
	      interms of global pixel coordinates*/
	    crnRowSubgrid[0] = ((subgridTemp % numSubgridsVert) *
				pixVertSubgrid);
	    crnRowSubgrid[1] = (crnRowSubgrid[0] + pixVertSubgrid - 1);

	    crnColSubgrid[0] = ((PGSt_integer)(subgridTemp/numSubgridsVert) * 
				pixHorizSubgrid);
	    crnColSubgrid[1] = (crnColSubgrid[0] + pixHorizSubgrid - 1);

	    /*At this point we can calculate the internal coordinates of this
	      particular subgrid's subregion.  The subregion is the section of
	      the region to be extracted which overlaps the geographic extent of
	      this file.  In the following if statements, we determine if the
	      region extends to the border of the particular subgrid. We first
	      check if the region's Southern or Eastern extent is within this
	      subgrid. Then we determine if the Northern or Western Extent is
	      within this subgrid.*/
	    
	    /*Southern Extent*/
	    if ((i + 1) == *extentSubgridsVert)
	    {
		/*Southern boundary of region falls with this subgrid*/
		if((*extentSubgridsHoriz > 1) && 
		   ((j+1) == *extentSubgridsHoriz) && 
		   (subgridUpprLft == subgridUpprRgt))
		{
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		    extractedRegion[subgridTemp + subsetInfo -> numSubgrids] -> rowExtent[1] =
		      cornerRowPixels[1] - crnRowSubgrid[0];
		}
		else
		{
		    extractedRegion[subgridTemp] -> rowExtent[1] =
		      cornerRowPixels[1] - crnRowSubgrid[0];
		}
		
	    }
	    else
	    {
		/*extends beyond this subgrid. record maximum vertical pixel
		  value of subgrid*/
		if((*extentSubgridsHoriz > 1) && 
		   ((j+1) == *extentSubgridsHoriz) && 
		   (subgridUpprLft == subgridUpprRgt))
		{
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		    extractedRegion[subgridTemp + subsetInfo -> numSubgrids] -> rowExtent[1] = 
		      pixVertSubgrid - 1;
		}
		else
		{
		    extractedRegion[subgridTemp] -> rowExtent[1] = 
		  pixVertSubgrid - 1;
		}  
	    }
	    
	    /*Eastern Extent*/
	    if ((j + 1) == *extentSubgridsHoriz)
	    {
		/*Eastern boundary of region falls with this subgrid*/
		if((*extentSubgridsHoriz > 1) && 
		   ((j+1) == *extentSubgridsHoriz) && 
		   (subgridUpprLft == subgridUpprRgt))
		{
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		    extractedRegion[subgridTemp + subsetInfo -> numSubgrids] -> colExtent[1] =  cornerColPixels[1] - crnColSubgrid[0];
		}
		else
		{
		extractedRegion[subgridTemp] -> colExtent[1] =
		  cornerColPixels[1] - crnColSubgrid[0];
		}
		
	    }
	    else
	    {
		/*extends beyond this subgrid. record maximum Horizontal pixel
		  value of subgrid*/
		if((*extentSubgridsHoriz > 1) && 
		   ((j+1) == *extentSubgridsHoriz) && 
		   (subgridUpprLft == subgridUpprRgt))
		{
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		    extractedRegion[subgridTemp + subsetInfo -> numSubgrids] -> colExtent[1] =  pixHorizSubgrid - 1;
		}
		else
		{
		extractedRegion[subgridTemp] -> colExtent[1] = 
		  pixHorizSubgrid - 1;
		}
		
	    }


	    /*Northern Extent*/
	    if ((crnRowSubgrid[1] - cornerRowPixels[0]) >= pixVertSubgrid)
	    {
		/*region extends beyon Northern boundary of subgrid*/
		if((*extentSubgridsHoriz > 1) && 
		   ((j+1) == *extentSubgridsHoriz) && 
		   (subgridUpprLft == subgridUpprRgt))
		{
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		    extractedRegion[subgridTemp + subsetInfo -> numSubgrids] -> rowExtent[0] = 0;
		}
		else
		{
		    extractedRegion[subgridTemp] -> rowExtent[0] = 0;
		}
		
	    }
	    else
	    {
		/*Northern extent of region falls within this subgrid*/
		if((*extentSubgridsHoriz > 1) && 
		   ((j+1) == *extentSubgridsHoriz) && 
		   (subgridUpprLft == subgridUpprRgt))
		{
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		    extractedRegion[subgridTemp + subsetInfo -> numSubgrids] -> rowExtent[0] = 
		      cornerRowPixels[0]  - crnRowSubgrid[0];
		}
		else
		{
		extractedRegion[subgridTemp] -> rowExtent[0] = 
		  cornerRowPixels[0]  - crnRowSubgrid[0];
		}    
	    }
	  	  

	    /*Western extent.  Need to check for a negative result.  Negative
	      indicates that this is a wrapping region.  hence, the western
	      extent is NOT is this subgrid. */
	    
	    if (((crnColSubgrid[1] - cornerColPixels[0]) >= pixHorizSubgrid) ||
		((crnColSubgrid[1] - cornerColPixels[0]) < 0))
	    {
		/*border case*/
		if((*extentSubgridsHoriz > 1) && 
		   ((j+1) == *extentSubgridsHoriz) && 
		   (subgridUpprLft == subgridUpprRgt))
		{
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		    extractedRegion[subgridTemp + subsetInfo -> numSubgrids] -> colExtent[0] = 0;
		}
		else
		{
		    
		    extractedRegion[subgridTemp] -> colExtent[0] = 0;
		}
		
	    }
	    else
	    {
		if((*extentSubgridsHoriz > 1) && 
		   ((j+1) == *extentSubgridsHoriz) && 
		   (subgridUpprLft == subgridUpprRgt))
		{
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		    extractedRegion[subgridTemp + subsetInfo -> numSubgrids] -> colExtent[0] = 0;
		}
		else
		{
		    extractedRegion[subgridTemp] -> colExtent[0] = 
		      cornerColPixels[0] - crnColSubgrid[0];
		}
	    }
	    

	    /*determine the size of the subregion.*/
	    if((*extentSubgridsHoriz > 1) && 
	       ((j+1) == *extentSubgridsHoriz) && 
	       (subgridUpprLft == subgridUpprRgt))
	    {
		/* wrapping across the 180 east and both upperleft corner and
		   upperright corner of region inside the same subgrid */
		horizPixSubregion = 
		  (extractedRegion[subgridTemp + 
				  subsetInfo -> numSubgrids] -> colExtent[1]) -
		  (extractedRegion[subgridTemp + 
				  subsetInfo -> numSubgrids] -> colExtent[0]) + 1;
		vertPixSubregion = 
		  (extractedRegion[subgridTemp + 
				  subsetInfo -> numSubgrids] -> rowExtent[1]) -
		  (extractedRegion[subgridTemp + 
				  subsetInfo -> numSubgrids] -> rowExtent[0]) + 1;
		
		subregionSize = horizPixSubregion * vertPixSubregion;
		
		extractedRegion[subgridTemp + subsetInfo -> numSubgrids] -> sizeSubregion = subregionSize;
	    }
	    else
	    {
		horizPixSubregion = 
		  (extractedRegion[subgridTemp] -> colExtent[1]) -
		  (extractedRegion[subgridTemp] -> colExtent[0]) + 1;
		vertPixSubregion = 
		  (extractedRegion[subgridTemp] -> rowExtent[1]) -
		  (extractedRegion[subgridTemp] -> rowExtent[0]) + 1;
		
		subregionSize = horizPixSubregion * vertPixSubregion;
		
		extractedRegion[subgridTemp] -> sizeSubregion = subregionSize;
	    }
		
	    /*Check if the present temporary subgrid is equal to Eastern
	      Boundary subgrid.  If this is satisfied, it indicates that this
	      region potentially wraps, dependent on whether we have already
	      reached the horizontal extent of the whole region or not.
	      independently, we change the subgrid to the appropriate western
	      boundary subgrid.  If this isn't the wrapping section of a region
	      we increment to the (horizontally) adjacent subgrid. */
	    if(subgridTemp == subgridEastBndry + i)
	    {
		subgridTemp = subgridWestBndry + i;
	    }
	    else
	    {
		subgridTemp += numSubgridsVert;
	    }
	    
	}
    }
    
    /*Successfully completed*/
    return (PGS_S_SUCCESS);
    

fatalError:
    {
	/*free up allocated space and return error*/
	for (i = 0; i < 2*(subsetInfo -> numSubgrids); i++)
	{
	    /*check if particular subgrid was allocated*/
	    if (extractedRegion[i] != NULL)
	    {
		/*free subregion's RegionRecord*/
		
		free(extractedRegion[i]);
		extractedRegion[i] = NULL;
		
	    }
	}

	return(statusError);
	
    }
}


	      
	  
	       
 
								   

