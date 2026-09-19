/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_EPH_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Spacecraft Ephemeris
  and Attitude Data Access (EPH) tools. 

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation
  Abe Taaheri / Applied Research Corporation
  Peter Noerdlinger  / SM & A, Inc.


HISTORY:
  02-Jun-1995  GTSK          Initial version
  28-Sep-1998  Abe Taaheri   changed binding for pgs_eph_getephmet 
                             (NCR ECSed17947)
  24-Mar-1999  PDN           Added PGS_EPH_ManageMasks
  03-Sep-2003  AT            Added binding for PGS_EPH_UnInterpEphAtt() and
                             PGS_EPH_EphAtt_unInterpolate()

END_FILE_PROLOG:
*******************************************************************************/

#include <cfortran.h>
#include <PGS_EPH.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_EPH_EphemAttit() */

FCALLSCFUN12(INT, PGS_EPH_EphemAttit, PGS_EPH_EPHEMATTIT, pgs_eph_ephemattit, \
             INT, INT, STRING, DOUBLEV, INT, INT, PVOID, DOUBLEVV, DOUBLEVV, \
	     DOUBLEVV, DOUBLEVV, DOUBLEVV)

/* PGS_EPH_GetEphMet() *//* note that the following means: size of arguments
			     8 and 9 is the 3rd argument */

#define pgs_eph_getephmetc_STRV_A8 NUM_ELEM_ARG(3) 
#define pgs_eph_getephmetc_STRV_A9 NUM_ELEM_ARG(3)

FCALLSCFUN10(INT, PGS_EPH_GetEphMetAux, PGS_EPH_GETEPHMETC, \
	     pgs_eph_getephmetc, \
	     INT, INT, INT, STRING, DOUBLEV, PINT, INTV, PSTRINGV, PSTRINGV, \
	     DOUBLEV)


/* PGS_EPH_unInterpolate() *//* note that the following means: size of 
				argument 9 is the 8th argument */

#define pgs_eph_ephatt_uninterpolatec_STRV_A9 NUM_ELEM_ARG(8)

FCALLSCFUN14(INT, PGS_EPH_EphAtt_unInterpolateAux, \
	     PGS_EPH_EPHATT_UNINTERPOLATEC, pgs_eph_ephatt_uninterpolatec, \
	     INT, STRING, STRING, INT, INT, PVOID, \
	     PINT, INT, PSTRINGV, DOUBLEVV, DOUBLEVV, DOUBLEVV, DOUBLEVV, \
	     DOUBLEVV)

/* PGS_EPH_EphAtt_UnInterpEphAtt() */ 

#define pgs_eph_uninterpephattc_STRV_A8 NUM_ELEM_ARG(7)
#define pgs_eph_uninterpephattc_STRV_A9 NUM_ELEM_ARG(7)

FCALLSCFUN14(INT, PGS_EPH_UnInterpEphAttAux, PGS_EPH_UNINTERPEPHATTC, \
	     pgs_eph_uninterpephattc, INT, STRING, STRING, INTV, PVOID, \
	     INTV, INT, PSTRINGV, PSTRINGV, DOUBLEVV, DOUBLEVV, \
	     DOUBLEVV, DOUBLEVV, DOUBLEVV)


/* PGS_EPH_interpolateAttitude() */

FCALLSCFUN10(INT, PGS_EPH_interpolateAttitude, PGS_EPH_INTERPOLATEATTITUDE, \
 	     pgs_eph_interpolateattitude, DOUBLE, DOUBLEV, DOUBLEV, DOUBLE, \
 	     DOUBLEV, DOUBLEV, INTV, DOUBLE, DOUBLEV, DOUBLEV)

/* PGS_EPH_interpolatePosVel() */

FCALLSCFUN9(INT, PGS_EPH_interpolatePosVel, PGS_EPH_INTERPOLATEPOSVEL, \
	    pgs_eph_interpolateposvel, DOUBLE, DOUBLEV, DOUBLEV, DOUBLE, \
	    DOUBLEV, DOUBLEV, DOUBLE, DOUBLEV, DOUBLEV)

/* PGS_EPH_ManageMasks() */

FCALLSCFUN2(INT, PGS_EPH_ManageMasks, PGS_EPH_MANAGEMASKS, pgs_eph_managemasks, \
             INT, INTV)
