/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_CUC_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Constants and Unit
  Conversion (CUC) tools.

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  11-Dec-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

#include <cfortran.h>
#include <PGS_CUC.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_CUC_Cons() */

#define pgs_cuc_cons_STRV_A2 NUM_ELEMS(1)

FCALLSCFUN3(INT, PGS_CUC_Cons, PGS_CUC_CONS, pgs_cuc_cons, INT, PSTRINGV, \
	    DOUBLEV)

/* PGS_CUC_Conv() */

FCALLSCFUN4(INT, PGS_CUC_Conv, PGS_CUC_CONV, pgs_cuc_conv, PSTRING, PSTRING, \
	    DOUBLEV,  DOUBLEV) 
