/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
  PGS_IO_L0_bindFORTRAN_L0.c
 
DESCRIPTION:
  This file contain FORTRAN bindings for the SDP (aka PGS) Toolkit Input and
  Output Level 0 (IO L0) tools.
 
AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation
 
HISTORY:
  02-Jun-1995  GTSK  Initial version
 
END_FILE_PROLOG:
*******************************************************************************/
 
#include <cfortran.h>
#include <PGS_IO_L0.h>

/*********************
 * cfortran.h MACROS *
 *********************/
 
/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_IO_L0_BYTEtoINT() */

FCALLSCFUN2(INT,PGS_IO_L0_BYTEtoINT, PGS_IO_L0_BYTETOINT, pgs_io_l0_bytetoint, \
	    PVOID,INT)

/* PGS_IO_L0_Close() */

FCALLSCFUN1(INT,PGS_IO_L0_Close,PGS_IO_L0_CLOSE,pgs_io_l0_close, INT)

/* PGS_IO_L0_GetHeader() */

FCALLSCFUN5(INT,PGS_IO_L0_GetHeader,PGS_IO_L0_GETHEADER,pgs_io_l0_getheader,\
	    INT,INT,PVOID,INT,PVOID)

/* PGS_IO_L0_GetPacket() */

FCALLSCFUN3(INT,PGS_IO_L0_GetPacket,PGS_IO_L0_GETPACKET,pgs_io_l0_getpacket,\
	    INT,INT,PVOID)

/* PGS_IO_L0_Open() */

FCALLSCFUN5(INT,PGS_IO_L0_Open,PGS_IO_L0_OPEN,pgs_io_l0_open,\
	    INT,INT,PINT,PDOUBLE,PDOUBLE)

/* PGS_IO_L0_SetStart() */

FCALLSCFUN2(INT,PGS_IO_L0_SetStart,PGS_IO_L0_SETSTART,pgs_io_l0_setstart,\
	    INT,DOUBLE)

/* PGS_IO_L0_SetStartCntPkts() */

FCALLSCFUN3(INT,PGS_IO_L0_SetStartCntPkts, PGS_IO_L0_SETSTARTCNTPKTS,
	    pgs_io_l0_setstartcntpkts, INT, DOUBLE, PINT)
