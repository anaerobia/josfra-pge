/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_PC_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Process Control (PC)
  tools.

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  11-Dec-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

#include <cfortran.h>
#include <PGS_PC.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_PC_GenUniqueID() */

FCALLSCFUN2(INT, PGS_PC_GenUniqueID, PGS_PC_GENUNIQUEID, pgs_pc_genuniqueid, \
	    INT, PSTRING)

/* PGS_PC_GetConfigData() */

FCALLSCFUN2(INT, PGS_PC_GetConfigData, PGS_PC_GETCONFIGDATA, \
            pgs_pc_getconfigdata, INT, PSTRING)

/* PGS_PC_GetFileAttr() */

FCALLSCFUN5(INT, PGS_PC_GetFileAttr, PGS_PC_GETFILEATTR, pgs_pc_getfileattr, \
	    INT, INT, INT, INT, PSTRING)

/* PGS_PC_GetFileSize() */

FCALLSCFUN3(INT, PGS_PC_GetFileSize, PGS_PC_GETFILESIZE, pgs_pc_getfilesize, \
	    INT, INT, PINT)

/* PGS_PC_GetNumberOfFiles() */

FCALLSCFUN2(INT, PGS_PC_GetNumberOfFiles, PGS_PC_GETNUMBEROFFILES, \
            pgs_pc_getnumberoffiles, INT, PINT)

/* PGS_PC_GetPCEnv() */

FCALLSCFUN2(INT, PGS_PC_GetPCEnv, PGS_PC_GETPCENV, pgs_pc_getpcenv, STRING, \
	    PSTRING)

/* PGS_PC_GetPCSData() */

FCALLSCFUN4(INT, PGS_PC_GetPCSData, PGS_PC_GETPCSDATA, pgs_pc_getpcsdata, \
            INT, INT, PSTRING, PINT)

/* PGS_PC_GetReference() */

FCALLSCFUN3(INT, PGS_PC_GetReference, PGS_PC_GETREFERENCE, \
            pgs_pc_getreference, INT, PINT, PSTRING)

/* PGS_PC_GetReferenceType() */

FCALLSCFUN2(INT, PGS_PC_GetReferenceType, PGS_PC_GETREFERENCETYPE, \
	    pgs_pc_getreferencetype, INT, PINT)

/* PGS_PC_GetUniversalRef() */

FCALLSCFUN3(INT,PGS_PC_GetUniversalRef,PGS_PC_GETUNIVERSALREF, \
            pgs_pc_getuniversalref,INT,PINT,PSTRING)

/* PGS_PC_PutPCSData() */

FCALLSCFUN2(INT, PGS_PC_PutPCSData, PGS_PC_PUTPCSDATA, pgs_pc_putpcsdata, \
            INT, PVOID)
