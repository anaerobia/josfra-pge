/*******************************************************
PG_DEM_Lookup.c--

This function accesses the general information on a generic subset. It takes a
resolution and layer and determines the subset number (it's logical ID from
PCF).

Presently, this is simply a lookup table.  Eventually it will acces a lookup
file on the PCF with all the appropriate information for each resolution and 
the corresponding layers.

The determination of which layers are grouped together occurs at when the data
was/is written into HDF-EOS format.

Added access to the quality data layers.  These are not resolution specific.
Therefore, instead of passing a resolution to the function, one passes the flag
PGSd_DEM_QUALITYINFO. The possible "qualityFields" are source, quality and 
geoid data, which correspond to the flags PGSd_DEM_SOURCE, PGSd_DEM_QUALITY,and
PGSd_DEM_GEOID, respectively.


Author -- Alexis Zubrow
          Abe Taaheri

history --
January 31, 1997  AZ  first created              
May 9, 1997       AZ  added quality data info    
July 3, 1997      AZ  Updated data types
July, 14, 1997    AZ  Changed quality/source diagnostic info.
January, 9, 1999  AT  Because of changes to binary data produced by EDC
                      changed:
		      data types for PGSd_DEM_STDEV_ELEV, PGSd_DEM_SLOPE, and
		      PGSd_DEM_STDEV_SLOPE from DFNT_INT8 to DFNT_INT16
		      Also for 3ARC data changed subsetInfo -> offset from
		      0.5 to 0.0 because of change in ULXMAP and ULYMAP 
		      offsets for 100m data.
June, 5, 2000     AT  Added functionality for 3km reolution
Sep., 14, 2011	  AT  Added functionality for 500m (15 arcsec) resolution.
Sept, 14, 2011 JK/AT  Changed to reflect actual fillvalue (consistent with
                      INT8 datatype for land/water mask
                      (intoduced by James Kuyper of MODIS) since
                      the new 15 arcsecond land/water mask DOES 
                      have fillvalues. Also all PGSd_DEM_NO_FILLVALUE was
                      changed to PGSd_DEM_LW_FILLVALUE for all INT8 data types.

*******************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <PGS_SMF.h>
#include <PGS_PC.h>
#include <PGS_DEM.h>



PGSt_SMF_status
PGS_DEM_Lookup(
    PGSt_DEM_Tag resolution,            /*Resolution  */
    PGSt_integer layer,                 /*Data layer, mask, ex. "Elevation"*/
    PGSt_DEM_SubsetRecord *subsetInfo)  /*Pointer to structure of subset info*/
{

    
    char errorBuf[PGS_SMF_MAX_MSG_SIZE];
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    char errorMsg[PGS_SMF_MAX_MSGBUF_SIZE];
    

    /*Determine the resolution*/
    switch (resolution)
    {
      case PGSd_DEM_3ARC:    /*3 arc second resolution*/
	/*Assign resolution specific info to subsetInfo*/
	subsetInfo -> resolutionTag = resolution;
	subsetInfo -> numSubgrids = 648;
	subsetInfo -> horizPixSubgrid = 12000;
	subsetInfo -> vertPixSubgrid = 12000;
	subsetInfo -> subgridHoriz = 36;
	subsetInfo -> subgridVert = 18;
	subsetInfo -> pixPerDegree = 1200;
	subsetInfo -> offset = 0;
	
	/*Determine appropriate subset by layer*/
	switch (layer)
	{
	    /*In this case Elevation, slope, aspect, and land/water are
	      grouped in one subset*/
	  case PGSd_DEM_ELEV:
	    subsetInfo -> subset = PGSd_DEM_SUBSET3A;
	    subsetInfo -> layer = PGSd_DEM_ELEV;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Elevation");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_SLOPE:
	    subsetInfo -> subset = PGSd_DEM_SUBSET3A;
	    subsetInfo -> layer = PGSd_DEM_SLOPE;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Slope");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_ASPECT:
	    subsetInfo -> subset = PGSd_DEM_SUBSET3A;
	    subsetInfo -> layer = PGSd_DEM_ASPECT;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Aspect");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_WATER_LAND:
	    subsetInfo -> subset = PGSd_DEM_SUBSET3A;
	    subsetInfo -> layer = PGSd_DEM_WATER_LAND;
	    /* JK: The NO_Fillvalue SDS attribute for the land/water mask 
             * was apparantly originally written from a int16 containing 
             * PGSd_DEM_NO_FILLVALUE, which is originally written from a int16
	     * containing PGSd_DEM_NO_FILLVALUE, which is
	     * #defined as -8888 in PGS_DEM.h and PGS_DEM.f . 
             * However, a _Fillvalue attribute necessarily has the same
	     * data type as the SDS itself. Therefore, only the first byte
	     * was actually written to the _Fillvalue attribute, which 
	     * was therefore interpreted as an int8 value of -35.
	     */
	    /* 9-14-2011 replaced PGSd_DEM_NO_FILLVALUE with 
	     * PGSd_DEM_LW_FILLVALUE
	     */
	    subsetInfo -> fillvalue = PGSd_DEM_LW_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "LandWater");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	      
	    /* Standard deviation of slop and elevation are grouped together*/
	  case PGSd_DEM_STDEV_ELEV:
	    subsetInfo -> subset = PGSd_DEM_SUBSET3B;
	    subsetInfo -> layer = PGSd_DEM_STDEV_ELEV;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "StdDevElevation");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_STDEV_SLOPE:
	    subsetInfo -> subset = PGSd_DEM_SUBSET3B;
	    subsetInfo -> layer = PGSd_DEM_STDEV_SLOPE;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "StdDevSlope");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;

	    /*Topological obscuration and shadow are grouped together*/
	  case PGSd_DEM_TOP_OBSC:
	    subsetInfo -> subset = PGSd_DEM_SUBSET3C;
	    subsetInfo -> layer = PGSd_DEM_TOP_OBSC;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "TopoObscuration");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_TOP_SHAD:
	    subsetInfo -> subset = PGSd_DEM_SUBSET3C;
	    subsetInfo -> layer = PGSd_DEM_TOP_SHAD;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "TopoShadow");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  default:
	    /*ERROR IMPROPER layer type */
	    /*Set dynamic message to improper layer number*/
	    sprintf(dynamicMsg, "(%d) is an invalid layer number", layer);
	    PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					  "PGS_DEM_Lookup()");
	    return(PGSDEM_E_IMPROPER_TAG);
	}

	break;

      case PGSd_DEM_15ARC:    /*15 arc second resolution*/
	/*Assign resolution specific diagnostics to subsetInfo*/
	subsetInfo -> resolutionTag = resolution;
	subsetInfo -> numSubgrids = 24;
	subsetInfo -> horizPixSubgrid = 14400;
	subsetInfo -> vertPixSubgrid = 10800;
	subsetInfo -> subgridHoriz = 6;
	subsetInfo -> subgridVert = 4;
	subsetInfo -> pixPerDegree = 240;
	subsetInfo -> offset = 0.0;
	
	/*Determine appropriate subset by layer*/
	switch (layer)
	{
	    /*In this case Elevation, slope, aspect, and land/water are
	      grouped in one subset*/
	  case PGSd_DEM_ELEV:
	    subsetInfo -> subset = PGSd_DEM_SUBSET15A;
	    subsetInfo -> layer = PGSd_DEM_ELEV;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Elevation");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_SLOPE:
	    subsetInfo -> subset = PGSd_DEM_SUBSET15A;
	    subsetInfo -> layer = PGSd_DEM_SLOPE;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Slope");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_ASPECT:
	    subsetInfo -> subset = PGSd_DEM_SUBSET15A;
	    subsetInfo -> layer = PGSd_DEM_ASPECT;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Aspect");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_WATER_LAND:
	    subsetInfo -> subset = PGSd_DEM_SUBSET15A;
	    subsetInfo -> layer = PGSd_DEM_WATER_LAND;
	    /* The new 15 arcsecond land/water mask DOES have fillvalues.*/
	    subsetInfo -> fillvalue = PGSd_DEM_LW_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "LandWater");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	      
	    /* Standard deviation of slop and elevation are grouped together*/
	  case PGSd_DEM_STDEV_ELEV:
	    subsetInfo -> subset = PGSd_DEM_SUBSET15B;
	    subsetInfo -> layer = PGSd_DEM_STDEV_ELEV;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "StdDevElevation");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_STDEV_SLOPE:
	    subsetInfo -> subset = PGSd_DEM_SUBSET15B;
	    subsetInfo -> layer = PGSd_DEM_STDEV_SLOPE;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "StdDevSlope");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;

	    /*Topological obscuration and shadow are grouped together*/
	  case PGSd_DEM_TOP_OBSC:
	    subsetInfo -> subset = PGSd_DEM_SUBSET15C;
	    subsetInfo -> layer = PGSd_DEM_TOP_OBSC;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "TopoObscuration");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_TOP_SHAD:
	    subsetInfo -> subset = PGSd_DEM_SUBSET15C;
	    subsetInfo -> layer = PGSd_DEM_TOP_SHAD;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "TopoShadow");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  default:
	    /*ERROR IMPROPER layer type */
	    /*Set dynamic message to improper layer number*/
	    sprintf(dynamicMsg, "(%d) is an invalid layer number", layer);
	    PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					  "PGS_DEM_Lookup()");
	    return(PGSDEM_E_IMPROPER_TAG);
	}
	break;

      case PGSd_DEM_30ARC:    /*30 arc second resolution*/
	/*Assign resolution specific diagnostics to subsetInfo*/
	subsetInfo -> resolutionTag = resolution;
	subsetInfo -> numSubgrids = 6;
	subsetInfo -> horizPixSubgrid = 14400;
	subsetInfo -> vertPixSubgrid = 10800;
	subsetInfo -> subgridHoriz = 3;
	subsetInfo -> subgridVert = 2;
	subsetInfo -> pixPerDegree = 120;
	subsetInfo -> offset = 0;
	
	/*Determine appropriate subset by layer*/
	switch (layer)
	{
	    /*In this case Elevation, slope, aspect, and land/water are
	      grouped in one subset*/
	  case PGSd_DEM_ELEV:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30A;
	    subsetInfo -> layer = PGSd_DEM_ELEV;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Elevation");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_SLOPE:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30A;
	    subsetInfo -> layer = PGSd_DEM_SLOPE;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Slope");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_ASPECT:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30A;
	    subsetInfo -> layer = PGSd_DEM_ASPECT;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Aspect");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_WATER_LAND:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30A;
	    subsetInfo -> layer = PGSd_DEM_WATER_LAND;
	    subsetInfo -> fillvalue = PGSd_DEM_LW_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "LandWater");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	      
	    /* Standard deviation of slop and elevation are grouped together*/
	  case PGSd_DEM_STDEV_ELEV:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30B;
	    subsetInfo -> layer = PGSd_DEM_STDEV_ELEV;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "StdDevElevation");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_STDEV_SLOPE:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30B;
	    subsetInfo -> layer = PGSd_DEM_STDEV_SLOPE;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "StdDevSlope");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;

	    /*Topological obscuration and shadow are grouped together*/
	  case PGSd_DEM_TOP_OBSC:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30C;
	    subsetInfo -> layer = PGSd_DEM_TOP_OBSC;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "TopoObscuration");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_TOP_SHAD:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30C;
	    subsetInfo -> layer = PGSd_DEM_TOP_SHAD;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "TopoShadow");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  default:
	    /*ERROR IMPROPER layer type */
	    /*Set dynamic message to improper layer number*/
	    sprintf(dynamicMsg, "(%d) is an invalid layer number", layer);
	    PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					  "PGS_DEM_Lookup()");
	    return(PGSDEM_E_IMPROPER_TAG);
	}
	break;

      case PGSd_DEM_90ARC:    /*90 arc second resolution*/
        /*Assign resolution specific diagnostics to subsetInfo*/
        subsetInfo -> resolutionTag = resolution;
        subsetInfo -> numSubgrids = 1;
        subsetInfo -> horizPixSubgrid = 14400;
        subsetInfo -> vertPixSubgrid = 7200;
        subsetInfo -> subgridHoriz = 1;
        subsetInfo -> subgridVert = 1;
        subsetInfo -> pixPerDegree = 40;
        subsetInfo -> offset = 0;
        
        /*Determine appropriate subset by layer*/
        switch (layer)
        {
            /*In this case Elevation, slope, aspect, and land/water are
              grouped in one subset*/
          case PGSd_DEM_ELEV:
            subsetInfo -> subset = PGSd_DEM_SUBSET90A;
            subsetInfo -> layer = PGSd_DEM_ELEV;
            subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
            subsetInfo -> dataType = DFNT_INT16;
            strcpy(subsetInfo -> fieldName, "Elevation");
            strcpy(subsetInfo -> gridName, "demGRID");
            break;
          case PGSd_DEM_SLOPE:
            subsetInfo -> subset = PGSd_DEM_SUBSET90A;
            subsetInfo -> layer = PGSd_DEM_SLOPE;
            subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
            subsetInfo -> dataType = DFNT_INT16;
            strcpy(subsetInfo -> fieldName, "Slope");
            strcpy(subsetInfo -> gridName, "demGRID");
            break;
          case PGSd_DEM_ASPECT:
            subsetInfo -> subset = PGSd_DEM_SUBSET90A;
            subsetInfo -> layer = PGSd_DEM_ASPECT;
            subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
            subsetInfo -> dataType = DFNT_INT16;
            strcpy(subsetInfo -> fieldName, "Aspect");
            strcpy(subsetInfo -> gridName, "demGRID");
            break;
          case PGSd_DEM_WATER_LAND:
            subsetInfo -> subset = PGSd_DEM_SUBSET90A;
            subsetInfo -> layer = PGSd_DEM_WATER_LAND;
            subsetInfo -> fillvalue = PGSd_DEM_LW_FILLVALUE;
            subsetInfo -> dataType = DFNT_INT8;
            strcpy(subsetInfo -> fieldName, "LandWater");
            strcpy(subsetInfo -> gridName, "demGRID");
            break;
              
            /* Standard deviation of slop and elevation are grouped together*/
          case PGSd_DEM_STDEV_ELEV:
            subsetInfo -> subset = PGSd_DEM_SUBSET90B;
            subsetInfo -> layer = PGSd_DEM_STDEV_ELEV;
            subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
            subsetInfo -> dataType = DFNT_INT16;
            strcpy(subsetInfo -> fieldName, "StdDevElevation");
            strcpy(subsetInfo -> gridName, "demGRID");
            break;
          case PGSd_DEM_STDEV_SLOPE:
            subsetInfo -> subset = PGSd_DEM_SUBSET90B;
            subsetInfo -> layer = PGSd_DEM_STDEV_SLOPE;
            subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
            subsetInfo -> dataType = DFNT_INT16;
            strcpy(subsetInfo -> fieldName, "StdDevSlope");
            strcpy(subsetInfo -> gridName, "demGRID");
            break;

            /*Topological obscuration and shadow are grouped together*/
          case PGSd_DEM_TOP_OBSC:
            subsetInfo -> subset = PGSd_DEM_SUBSET90C;
            subsetInfo -> layer = PGSd_DEM_TOP_OBSC;
            subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
            subsetInfo -> dataType = DFNT_INT8;
            strcpy(subsetInfo -> fieldName, "TopoObscuration");
            strcpy(subsetInfo -> gridName, "demGRID");
            break;
          case PGSd_DEM_TOP_SHAD:
            subsetInfo -> subset = PGSd_DEM_SUBSET90C;
            subsetInfo -> layer = PGSd_DEM_TOP_SHAD;
            subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
            subsetInfo -> dataType = DFNT_INT8;
            strcpy(subsetInfo -> fieldName, "TopoShadow");
            strcpy(subsetInfo -> gridName, "demGRID");
            break;
          default:
            /*ERROR IMPROPER layer type */
            /*Set dynamic message to improper layer number*/
            sprintf(dynamicMsg, "(%d) is an invalid layer number", layer);
            PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
            sprintf(errorMsg, errorBuf, dynamicMsg);
            PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
                                          "PGS_DEM_Lookup()");
            return(PGSDEM_E_IMPROPER_TAG);
        }
        break;

      case PGSd_DEM_30TEST: /*30 arc second resolution, test data*/
	  
	/*Assign resolution specific diagnostics to subsetInfo*/
	subsetInfo -> resolutionTag = resolution;
	subsetInfo -> numSubgrids = 36;
	subsetInfo -> horizPixSubgrid = 4800;
	subsetInfo -> vertPixSubgrid = 6000;
	subsetInfo -> subgridHoriz = 9;
	subsetInfo -> subgridVert = 4;
	subsetInfo -> pixPerDegree = 120;
	subsetInfo -> offset = 0;
	
	/*Determine appropriate subset by layer*/
	switch (layer)
	{
	    /*In this case Elevation, slope, aspect, and land/water are
	      grouped in one subset*/
	  case PGSd_DEM_ELEV:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30TEST;
	    subsetInfo -> layer = PGSd_DEM_ELEV;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Elevation");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_SLOPE:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30A;
	    subsetInfo -> layer = PGSd_DEM_SLOPE;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Slope");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_ASPECT:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30A;
	    subsetInfo -> layer = PGSd_DEM_ASPECT;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Aspect");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_WATER_LAND:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30A;
	    subsetInfo -> layer = PGSd_DEM_WATER_LAND;
	    subsetInfo -> fillvalue = PGSd_DEM_LW_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "LandWater");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	      
	    /* Standard deviation of slop and elevation are grouped together*/
	  case PGSd_DEM_STDEV_ELEV:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30B;
	    subsetInfo -> layer = PGSd_DEM_STDEV_ELEV;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "StdDevElevation");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_STDEV_SLOPE:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30B;
	    subsetInfo -> layer = PGSd_DEM_STDEV_SLOPE;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "StdDevSlope");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;

	    /*Topological obscuration and shadow are grouped together*/
	  case PGSd_DEM_TOP_OBSC:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30C;
	    subsetInfo -> layer = PGSd_DEM_TOP_OBSC;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "TopoObscuration");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  case PGSd_DEM_TOP_SHAD:
	    subsetInfo -> subset = PGSd_DEM_SUBSET30C;
	    subsetInfo -> layer = PGSd_DEM_TOP_SHAD;
	    subsetInfo -> fillvalue = PGSd_DEM_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    strcpy(subsetInfo -> fieldName, "TopoShadow");
	    strcpy(subsetInfo -> gridName, "demGRID");
	    break;
	  default:
	    /*ERROR IMPROPER layer type */
	    /*ERROR IMPROPER layer type */
	    /*Set dynamic message to improper layer number*/
	    sprintf(dynamicMsg, "(%d) is an invalid layer number", layer);
	    PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					  "PGS_DEM_Lookup()");
	    return(PGSDEM_E_IMPROPER_TAG);


	}
	break;


	/*Quality/source/geoid information, not really related to resolutions,
	  but using this structure to store necessary info */
      case PGSd_DEM_QUALITYINFO:
	subsetInfo -> numSubgrids = 1;
	subsetInfo -> horizPixSubgrid = 360;
	subsetInfo -> vertPixSubgrid = 180;
	subsetInfo -> subgridHoriz = 1;
	subsetInfo -> subgridVert = 1;
	subsetInfo -> pixPerDegree = 1;
	subsetInfo -> offset = 0;
	switch(layer)
	{
	    /*Geoid data*/
	  case PGSd_DEM_GEOID:
	    subsetInfo -> layer = PGSd_DEM_GEOID;
	    subsetInfo -> numBytes = sizeof(int16);
	    subsetInfo -> fillvalue = PGSd_DEM_NO_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    strcpy(subsetInfo -> fieldName, "Geoid");
	    strcpy(subsetInfo -> gridName, "qualityGRID");
	    break;
	    /*Method  data*/
	  case PGSd_DEM_METHOD :
	    subsetInfo -> layer = PGSd_DEM_METHOD ;
	    subsetInfo -> fillvalue = PGSd_DEM_LW_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    subsetInfo -> numBytes = sizeof(int8 ); 
	    strcpy(subsetInfo -> fieldName, "Quality");
	    strcpy(subsetInfo -> gridName, "qualityGRID");
	    break;
	    /*Source Data*/
	  case PGSd_DEM_SOURCE:
	    subsetInfo -> layer = PGSd_DEM_SOURCE;
	    subsetInfo -> fillvalue = PGSd_DEM_LW_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT8;
	    subsetInfo -> numBytes = sizeof(int8);
	    strcpy(subsetInfo -> fieldName, "Source");
	    strcpy(subsetInfo -> gridName, "qualityGRID");
	    break;
	    /*Vertical Accuracy */
	  case PGSd_DEM_VERTICAL_ACCURACY:
	    subsetInfo -> layer = PGSd_DEM_VERTICAL_ACCURACY;
	    subsetInfo -> fillvalue = PGSd_DEM_NO_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    subsetInfo -> numBytes = sizeof(int16);
	    strcpy(subsetInfo -> fieldName, "VertAccuracy");
	    strcpy(subsetInfo -> gridName, "qualityGRID");
	    break;
	    /*Horizontal Accuracy Data*/
	  case PGSd_DEM_HORIZONTAL_ACCURACY:
	    subsetInfo -> layer = PGSd_DEM_HORIZONTAL_ACCURACY;
	    subsetInfo -> fillvalue = PGSd_DEM_NO_FILLVALUE;
	    subsetInfo -> dataType = DFNT_INT16;
	    subsetInfo -> numBytes = sizeof(int16);
	    strcpy(subsetInfo -> fieldName, "HorizAccuracy");
	    strcpy(subsetInfo -> gridName, "qualityGRID");
	    break;
	  default:
	    /*ERROR improper qualityField*/
	    sprintf(dynamicMsg, "Improper tag... "
		    "qualityField (%d) is not recognized", layer);
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					   "PGS_DEM_Lookup()");
	    return(PGSDEM_E_IMPROPER_TAG);
	}
	break;
	
      default:
	/*ERROR IMPROPER resolution Tag */
	/*Set dynamic message to improper layer number*/
	sprintf(dynamicMsg, "(%d) is an invalid resolution tag", resolution);
	PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	sprintf(errorMsg, errorBuf, dynamicMsg);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
				       "PGS_DEM_Lookup()");
	return(PGSDEM_E_IMPROPER_TAG);

    }
    

    return (PGS_S_SUCCESS);
}
