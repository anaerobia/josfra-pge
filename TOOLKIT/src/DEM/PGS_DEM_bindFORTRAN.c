/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_DEM_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Digital Elevation
  Model Data Access (DEM) tools.

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation
  Alexis Zubrow / Applied Research Corporation

HISTORY:
  02-Jun-1995  GTSK  Initial version
  03-25-1997         Modified for DEM calls

END_FILE_PROLOG:
*******************************************************************************/

#define PGSd_DEM_PROTOS_ONLY
#include <cfortran.h>
#include <PGS_DEM.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_DEM_Open() */

FCALLSCFUN4(INT, PGS_DEM_Open, PGS_DEM_OPEN, pgs_dem_open, INTV, INT, INTV, INT)

/* PGS_DEM_Close() */
 
FCALLSCFUN4(INT, PGS_DEM_Close, PGS_DEM_CLOSE, pgs_dem_close, INTV, INT, \
	      INTV, INT)

/* PGS_DEM_DataPresent() */

FCALLSCFUN7(INT, PGS_DEM_DataPresent, PGS_DEM_DATAPRESENT,
	    pgs_dem_datapresent, INT, INT, INT, DOUBLEV, DOUBLEV, INT, PINT)

/* PGS_DEM_SortModels() */

FCALLSCFUN7(INT, PGS_DEM_SortModels, PGS_DEM_SORTMODELS, pgs_dem_sortmodels, \
	    INTV, INT, INT, INT, DOUBLEV, DOUBLEV, PINT)

/* PGS_DEM_GetPoint() */

FCALLSCFUN9(INT, PGS_DEM_GetPoint, PGS_DEM_GETPOINT, pgs_dem_getpoint, INTV, \
	    INT, INT, INT, DOUBLEV, DOUBLEV, INT, INT, PVOID)

/* PGS_DEM_GetRegion() */

FCALLSCFUN11(INT, PGS_DEM_GetRegion,  PGS_DEM_GETREGION,  pgs_dem_getregion, \
	     INTV, INT, INT, INT, INT, DOUBLEV, DOUBLEV, PVOID, DOUBLEV, \
	     DOUBLEV, DOUBLEV)

/* PGS_DEM_GetSize() */

FCALLSCFUN8(INT, PGS_DEM_GetSize, PGS_DEM_GETSIZE,  pgs_dem_getsize, INT, INT, \
	    INT, DOUBLEV, DOUBLEV, PINT, PINT, PINT)

/* PGS_DEM_GetMetadata() */

FCALLSCFUN11(INT, PGS_DEM_GetMetadata, PGS_DEM_GETMETADATA, \
	    pgs_dem_getmetadata, INT, INT, DOUBLEV, DOUBLEV, PSTRING, \
	    PDOUBLE, PDOUBLE, PDOUBLE, PSTRING, PINT, PINT)

/*PGS_DEM_GetQualityData() */

FCALLSCFUN6(INT, PGS_DEM_GetQualityData, PGS_DEM_GETQUALITYDATA, \
	    pgs_dem_getqualitydata, INT, INT, INT, DOUBLEV, DOUBLEV, PVOID)



