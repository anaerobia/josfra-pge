/*****************************************************************
PGS_DEM_WriteSubgridCalculator.c--

This function will be used in the writing of the DEM files to HDF-EOS format.
it calculates a subgrid number from signed, decimal degreees longituted and
latitude.


Author --
Alexis Zubrow
Abe Taaheri

Date --
Initial programming  January 30, 1997

Revision 
 June 5, 2000    AT Added functionality for 3km resolution
 Sept 14,2011 JK/AT Added functionality for 500m (15 arcsec)
                    resolution

*******************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>

PGSt_SMF_status
PGS_DEM_WriteSubgridCalculator(
    PGSt_DEM_Tag resolution,        /*resolution tag*/
    PGSt_double signedDegreesLat,   /*latitude in deccimal degrees*/
    PGSt_double signedDegreesLon,   /*longitude in decimal degrees*/
    PGSt_integer *unsignedPixLat,   /*latitude in unsigned global pixels*/
    PGSt_integer *unsignedPixLon,   /*longitude in unsigned global pixels*/
    PGSt_integer *subgrid)          /*subgrid value calculated*/

{
    PGSt_integer pixRow;             /*Vertical position in global pixels*/
    PGSt_integer pixCol;             /*Horizontal position in global pixels*/
    PGSt_integer horizPixPerSubgrid; /*Number horizontal pixels per subgrid*/
    PGSt_integer vertPixPerSubgrid;  /*Number vertical pixels per subgrid*/
    PGSt_integer subgridVert;        /*Number subgrids vert. spanning world*/ 
    PGSt_integer pixPerDegree;       /*Number pixels per degree (lat. or lon.)*/
    
 
    /*inititialize for each possible resolution*/
 
    switch (resolution)
    {
      case PGSd_DEM_3ARC:  /*3 arc second*/
	horizPixPerSubgrid = 12000;
	vertPixPerSubgrid = 12000;
	pixPerDegree = 1200;
	subgridVert = 18;
	break;
      case PGSd_DEM_15ARC:   /*15 arc second*/
	horizPixPerSubgrid = 14400;
	vertPixPerSubgrid = 10800;
	pixPerDegree = 240;
	subgridVert = 4;
	break;
      case PGSd_DEM_30ARC:   /*30 arc second*/
	horizPixPerSubgrid = 14400;
	vertPixPerSubgrid = 10800;
	pixPerDegree = 120;
	subgridVert = 2;
	break;
      case PGSd_DEM_90ARC:   /*90 arc second*/
        horizPixPerSubgrid = 14400;
        vertPixPerSubgrid = 7200;
        pixPerDegree = 40;
        subgridVert = 1;
        break;
      case PGSd_DEM_30TEST:  /*30 arc second used for testing,
			       in 40 by 50 degree size files
			       instead of the real datas 120 by
			       90 degree files*/
	horizPixPerSubgrid = 4800;
	vertPixPerSubgrid = 6000;
	pixPerDegree = 120;
	subgridVert = 4;
	break;
      default:
	/*ERROR-- improper resolution tag*/
	break;
    }
    
    /*convert from signed decimal degrees to unsigned pixels*/
    pixRow = (PGSt_integer)((90.0 - signedDegreesLat)*pixPerDegree);
    pixCol = (PGSt_integer)((180.0 + signedDegreesLon)*pixPerDegree);
    *unsignedPixLat = pixRow;
    *unsignedPixLon = pixCol;
    
    /*convert from pixels to subgrid*/
    *subgrid = ((pixCol/horizPixPerSubgrid)*subgridVert) +
      (pixRow/vertPixPerSubgrid);

    return (PGS_S_SUCCESS);
    
}
