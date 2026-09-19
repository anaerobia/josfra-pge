/*****************************************************************
PGS_DEM_WriteDegreeToPixel.c--

This function will be used in the writing of the DEM files to HDF-EOS format.
it calculates a the unsigned global pixel numbers from latitude or longitude in
decimal degrees.



Author --
Alexis Zubrow
Abe Taaheri

Date --
Initial programming  February 3, 1997
June 5, 2000      AT         Added functionality for 3km resolution
Sept 14,2011      AT         Added functionality for 500m (15 arcsec) 
                             resolution

*******************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <PGS_SMF.h>
#include <PGS_DEM.h>

PGSt_SMF_status
PGS_DEM_WrieDegreeToPixel(
    PGSt_DEM_Tag resolution,     /*resolution tag*/
    PGSt_double positionDegrees, /*position in unsigned decimal degrees*/
    PGSt_integer *pixels)        /*position in unsigned global pixels*/
{
    
    PGSt_integer pixPerDegree;   /*Number of pixels per decimal degree*/
    
    /*inititialize for each possible resolution*/
 
    switch (resolution)
    {
      case PGSd_DEM_3ARC:    /* 3 arc second*/
	pixPerDegree = 1200;
	break;
      case PGSd_DEM_15ARC:   /*15 arc second*/
        pixPerDegree = 240;
        break;
      case PGSd_DEM_30ARC:   /*30 arc second*/
	pixPerDegree = 120;
	break;
      case PGSd_DEM_90ARC:   /*90 arc second*/
        pixPerDegree = 40;
        break;
      case PGSd_DEM_30TEST:  /*30 arc second used for testing,
			       in 40 by 50 degree size files
			       instead of the real datas 120 by
			       90 degree files*/
	pixPerDegree = 120;
	break;
      default:
	/*ERROR-- improper resolution tag*/
	break;
    }

    /*convert from signed decimal degrees to unsigned pixels*/
    *pixels = positionDegrees*pixPerDegree;

    return (PGS_S_SUCCESS);
}
