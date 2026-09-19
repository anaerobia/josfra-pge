/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
  PGS_IO_L0_L0Sim_bindFORTRAN.c
 
DESCRIPTION:
  This file contain FORTRAN bindings for the SDP (aka PGS) Toolkit Input and
  Output Level 0 (IO L0) tools.
 
AUTHOR:
  Ray Milburn / Steven Myers & Associates
 
HISTORY:
  29-Jan-1999  RM  Initial version
 
END_FILE_PROLOG:
*******************************************************************************/
 
#include <cfortran.h>
#include <PGS_IO_L0.h>

/*********************
 * cfortran.h MACROS *
 *********************/

FCALLSCFUN13(INT,PGS_IO_L0_File_Sim,PGS_IO_L0_FILE_SIM,pgs_io_l0_file_sim,\
            INT,INTV,INT,STRING,INT,DOUBLE,INTV,INTV,STRING, \
            PVOID,PVOID,PVOID,PVOID)
