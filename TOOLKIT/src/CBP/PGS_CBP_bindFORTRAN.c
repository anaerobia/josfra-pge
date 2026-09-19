/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_CBP_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Celestial Body
  Position (CBP) tools. 

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  02-Jun-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

#include <cfortran.h>
#include <PGS_CBP.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_CBP_Earth_CB_Vector() */

FCALLSCFUN5(INT, PGS_CBP_Earth_CB_Vector, PGS_CBP_EARTH_CB_VECTOR, \
	    pgs_cbp_earth_cb_vector, INT, STRING, DOUBLEV, INT, DOUBLEVV)
    
/* PGS_CBP_Sat_CB_Vector() */
    
FCALLSCFUN6(INT, PGS_CBP_Sat_CB_Vector, PGS_CBP_SAT_CB_VECTOR, \
	    pgs_cbp_sat_cb_vector, INT, INT, STRING, DOUBLEV, INT, DOUBLEVV) 

/* PGS_CBP_SolarTimeCoords() */

FCALLSCFUN7(INT,PGS_CBP_SolarTimeCoords, PGS_CBP_SOLARTIMECOORDS, \
	    pgs_cbp_solartimecoords, PSTRING, DOUBLE, PDOUBLE, PDOUBLE, \
	    PDOUBLE, PDOUBLE, PDOUBLE)

/* PGS_CBP_body_inFOV() */

FCALLSCFUN11(INT, PGS_CBP_body_inFOV, PGS_CBP_BODY_INFOV,pgs_cbp_body_infov, \
	     INT, STRING, DOUBLEV, INT, INT, DOUBLEVV, DOUBLEVVV, INT, INTV, \
	     DOUBLEVV, DOUBLEVV) 
