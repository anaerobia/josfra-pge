/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_MEM_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Memory Management
  (MEM) tools.

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  26-Mar-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

#include <cfortran.h>
#include <PGS_MEM.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_MEM_ShmReadF() */

FCALLSCFUN2(INT, PGS_MEM_ShmReadF, PGS_MEM_SHMREAD, pgs_mem_shmread, PVOID, INT)

/* PGS_MEM_ShmCreate() */

FCALLSCFUN1(INT, PGS_MEM_ShmCreate, PGS_MEM_SHMCREATE, pgs_mem_shmcreate, INT)

/* PGS_MEM_ShmWriteF() */

FCALLSCFUN2(INT, PGS_MEM_ShmWriteF, PGS_MEM_SHMWRITE, pgs_mem_shmwrite, PVOID, \
	    INT)
