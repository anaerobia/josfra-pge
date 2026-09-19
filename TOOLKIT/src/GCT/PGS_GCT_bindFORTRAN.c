/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_GCT_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Geocoordinate
  Transformation (GCT) tools.

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  11-Dec-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

#include <cfortran.h>
#include <PGS_GCT.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_GCT_Init() */

#define pgs_gct_init_STRV_A1 NUM_ELEMS(1)

FCALLSCFUN3(INT, PGS_GCT_Init, PGS_GCT_INIT, pgs_gct_init, INT, DOUBLEV, INT)

/* PGS_GCT_Proj() */

FCALLSCFUN8(INT, PGS_GCT_Proj, PGS_GCT_PROJ, pgs_gct_proj, INT, INT, INT, \
	    DOUBLEV, DOUBLEV, DOUBLEV, DOUBLEV, INTV)
