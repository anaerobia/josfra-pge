/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_MET_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Metadata (MET) tools.

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation
  Abe Taaheri / Emergent Information Technologies, Inc.

HISTORY:
  11-Dec-1995  GTSK  Initial version
  23-Jan-2001  AT    Added bindings for PGS_MET_SetMultiAttr
  30-Mar-2001  AT    Modified for HDF5 support

END_FILE_PROLOG:
*******************************************************************************/

#include <PGS_MET.h>
#include <cfortran.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_MET_GetConfigDataF() - generic interface */

FCALLSCFUN2(INT, PGS_MET_GetConfigDataF, PGS_MET_GETCONFIGDATA, \
	    pgs_met_getconfigdata, STRING, STRVOID)

/* PGS_MET_GetConfigDataF() - integer interface */

FCALLSCFUN2(INT, PGS_MET_GetConfigDataF, PGS_MET_GETCONFIGDATA_I, \
	    pgs_met_getconfigdata_i, STRING, STRVOID)

/* PGS_MET_GetConfigDataF() - real interface */

FCALLSCFUN2(INT, PGS_MET_GetConfigDataF, PGS_MET_GETCONFIGDATA_R, \
	    pgs_met_getconfigdata_r, STRING, STRVOID)

/* PGS_MET_GetConfigDataF() - double precision interface */

FCALLSCFUN2(INT, PGS_MET_GetConfigDataF, PGS_MET_GETCONFIGDATA_D, \
	    pgs_met_getconfigdata_d, STRING, STRVOID)

/* PGS_MET_GetConfigDataF() - character string interface */

FCALLSCFUN2(INT, PGS_MET_GetConfigDataF, PGS_MET_GETCONFIGDATA_S, \
	    pgs_met_getconfigdata_s, STRING, STRVOID)

/* PGS_MET_GetPCAttrF() - generic interface */

FCALLSCFUN5(INT, PGS_MET_GetPCAttrF, PGS_MET_GETPCATTR, pgs_met_getpcattr, \
	    INT, INT, STRING, STRING, STRVOID)

/* PGS_MET_GetPCAttrF() - integer interface */

FCALLSCFUN5(INT, PGS_MET_GetPCAttrF, PGS_MET_GETPCATTR_I, pgs_met_getpcattr_i, \
	    INT, INT, STRING, STRING, STRVOID)

/* PGS_MET_GetPCAttrF() - double precision interface */

FCALLSCFUN5(INT, PGS_MET_GetPCAttrF, PGS_MET_GETPCATTR_D, pgs_met_getpcattr_d, \
	    INT, INT, STRING, STRING, STRVOID)

/* PGS_MET_GetPCAttrF() - real interface */

FCALLSCFUN5(INT, PGS_MET_GetPCAttrF, PGS_MET_GETPCATTR_R, pgs_met_getpcattr_r, \
	    INT, INT, STRING, STRING, STRVOID)

/* PGS_MET_GetPCAttrF() - character string interface */

FCALLSCFUN5(INT, PGS_MET_GetPCAttrF, PGS_MET_GETPCATTR_S, pgs_met_getpcattr_s, \
	    INT, INT, STRING, STRING, STRVOID)

/* PGS_MET_GetSetAttrF() - generic interface */

FCALLSCFUN3(INT, PGS_MET_GetSetAttrF, PGS_MET_GETSETATTR, pgs_met_getsetattr, \
	    STRING, STRING, STRVOID)

/* PGS_MET_GetSetAttrF() integer interface */

FCALLSCFUN3(INT, PGS_MET_GetSetAttrF, PGS_MET_GETSETATTR_I, \
	    pgs_met_getsetattr_i, STRING, STRING, STRVOID)

/* PGS_MET_GetSetAttrF() - real interface */

FCALLSCFUN3(INT, PGS_MET_GetSetAttrF, PGS_MET_GETSETATTR_R, \
	    pgs_met_getsetattr_r, STRING, STRING, STRVOID)

/* PGS_MET_GetSetAttrF() - double precision interface */

FCALLSCFUN3(INT, PGS_MET_GetSetAttrF, PGS_MET_GETSETATTR_D, \
	    pgs_met_getsetattr_d, STRING, STRING, STRVOID)

/* PGS_MET_GetSetAttrF() - character string interface */

FCALLSCFUN3(INT, PGS_MET_GetSetAttrF, PGS_MET_GETSETATTR_S, \
	    pgs_met_getsetattr_s, STRING, STRING, STRVOID)

/* PGS_MET_Init() */

#define pgs_met_init_STRV_A2 NUM_ELEMS(PGSd_MET_NUM_OF_GROUPS)

FCALLSCFUN2(INT, PGS_MET_Init, PGS_MET_INIT, pgs_met_init, INT, PSTRINGV)

/* PGS_MET_Remove() */

FCALLSCFUN0(VOID, PGS_MET_Remove, PGS_MET_REMOVE, pgs_met_remove)

/* PGS_MET_SetAttrF() - generic interface */

FCALLSCFUN3(INT, PGS_MET_SetAttrF, PGS_MET_SETATTR, pgs_met_setattr, STRING, \
	    STRING, STRVOID)

/* PGS_MET_SetAttrF() - integer interface */

FCALLSCFUN3(INT, PGS_MET_SetAttrF, PGS_MET_SETATTR_I, pgs_met_setattr_i, \
	    STRING, STRING, STRVOID)

/* PGS_MET_SetAttrF() - real interface */

FCALLSCFUN3(INT, PGS_MET_SetAttrF, PGS_MET_SETATTR_R, pgs_met_setattr_r, \
	    STRING, STRING, STRVOID)

/* PGS_MET_SetAttrF() - double precision interface */

FCALLSCFUN3(INT, PGS_MET_SetAttrF, PGS_MET_SETATTR_D, pgs_met_setattr_d, \
	    STRING, STRING, STRVOID)

/* PGS_MET_SetAttrF() - character string interface */

FCALLSCFUN3(INT, PGS_MET_SetAttrF, PGS_MET_SETATTR_S, pgs_met_setattr_s, \
	    STRING, STRING, STRVOID)

/* PGS_MET_SetMultiAttrF() - generic interface */

FCALLSCFUN4(INT, PGS_MET_SetMultiAttrF, PGS_MET_SETMULTIATTR, \
	    pgs_met_setmultiattr, STRING, STRING, INT, STRVOID)

/* PGS_MET_SetMultiAttrF() - integer interface */

FCALLSCFUN4(INT, PGS_MET_SetMultiAttrF, PGS_MET_SETMULTIATTR_I, \
	    pgs_met_setmultiattr_i, STRING, STRING, INT, STRVOID)

/* PGS_MET_SetMultiAttrF() - real interface */

FCALLSCFUN4(INT, PGS_MET_SetMultiAttrF, PGS_MET_SETMULTIATTR_R, \
	    pgs_met_setmultiattr_r, STRING, STRING, INT, STRVOID)

/* PGS_MET_SetMultiAttrF() - double precision interface */

FCALLSCFUN4(INT, PGS_MET_SetMultiAttrF, PGS_MET_SETMULTIATTR_D, \
	    pgs_met_setmultiattr_d, STRING, STRING, INT, STRVOID)

/* PGS_MET_SetMultiAttrF() - character string interface */

FCALLSCFUN4(INT, PGS_MET_SetMultiAttrF, PGS_MET_SETMULTIATTR_S, \
	    pgs_met_setmultiattr_s, STRING, STRING, INT, STRVOID)

/* PGS_MET_Write() */

FCALLSCFUN3(INT, PGS_MET_Write, PGS_MET_WRITE, pgs_met_write, STRING, STRING, \
	    INT)

/* PGS_MET_HDFFileType() */

FCALLSCFUN4(INT, PGS_MET_HDFFileType, PGS_MET_HDFFILETYPE, \
            pgs_met_hdffiletype, STRING, PINT, PINT, PINT)

/* PGS_MET_SDstartF() */

FCALLSCFUN3(INT, PGS_MET_SDstartF, PGS_MET_SFSTART, pgs_met_sfstart,\
            STRING, INT, PINT)

/* PGS_MET_SDend() */

FCALLSCFUN1(INT, PGS_MET_SDend, PGS_MET_SFEND, pgs_met_sfend, INT)
