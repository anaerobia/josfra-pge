/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
  	PGS_AA_bindFORTRAN.c
 
DESCRIPTION:
	This file contain FORTRAN bindings for the SDP (aka PGS) Toolkit AA tools

AUTHOR: 
  	Alward N. Siyyid / EOSL
        Abe Taaheri /SM&A Corp.

HISTORY:
  	07-June-95 	ANS 	Initial version
        03-March-2000   AT      Added binder to PGS_AA_dcw

END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <cfortran.h>
#include <PGS_AA.h>

/*
 * cfortran.h MACROS 
 */

#define pgs_aa_2dread_STRV_A1 NUM_ELEMS(4)
FCALLSCFUN10(INT,PGS_AA_2DReadF,PGS_AA_2DREAD,pgs_aa_2dread,STRINGV,INT,INT, \
             INT,INT,INT,INT,INT,INT,PVOID)

#define pgs_aa_2dgeo_STRV_A1 NUM_ELEMS(4)
FCALLSCFUN9(INT,PGS_AA_2DgeoF,PGS_AA_2DGEO,pgs_aa_2dgeo,STRINGV,INT,DOUBLEV, \
            DOUBLEV,INT,INT,INT,INT,PVOID)

#define pgs_aa_3dgeo_STRV_A1 NUM_ELEMS(4)
FCALLSCFUN10(INT,PGS_AA_3DgeoF,PGS_AA_3DGEO,pgs_aa_3dgeo,STRINGV,INT,DOUBLEV, \
             DOUBLEV,INTV,INT,INT,INT,INT,PVOID)

#define pgs_aa_3dread_STRV_A1 NUM_ELEMS(4)
FCALLSCFUN12(INT,PGS_AA_3DReadF,PGS_AA_3DREAD,pgs_aa_3dread,STRINGV,INT,INT, \
             INT,INT,INT,INT,INT,INT,INT,INT,PVOID)

#define pgs_aa_dem_STRV_A1 NUM_ELEMS(4)
FCALLSCFUN9(INT,PGS_AA_demF,PGS_AA_DEM,pgs_aa_dem,STRINGV,INT,DOUBLEV, \
            DOUBLEV,PINT,INT,INT,INT,PVOID)

FCALLSCFUN3(INT,PGS_AA_PeV_string,PGS_AA_PEV_STRING, pgs_aa_pev_string, INT, \
            PSTRING, PSTRING)

FCALLSCFUN3(INT,PGS_AA_PeV_real,PGS_AA_PEV_REAL, pgs_aa_pev_real, INT, \
            PSTRING,PDOUBLE)

FCALLSCFUN3(INT,PGS_AA_PeV_integer,PGS_AA_PEV_integer, pgs_aa_pev_integer, \
            INT, PSTRING,PINT)

/* Added fortran bindings from PGS_AA_PeVA.c and PGS_AA_3Dgeo.c */
/* and removed from those two files		                */

/* PGS_AA_PeVA_stringF() */

FCALLSCFUN3(INT, PGS_AA_PeVA_stringF, PGS_AA_PEVA_STRING, pgs_aa_peva_string, \
            INT, PSTRING, STRVOID)

/* PGS_AA_PeVA_real() */

FCALLSCFUN3(INT, PGS_AA_PeVA_real, PGS_AA_PEVA_REAL, pgs_aa_peva_real, INT, \
            PSTRING, PDOUBLE)

/* PGS_AA_PeVA_integer() */
 
FCALLSCFUN3(INT, PGS_AA_PeVA_integer,PGS_AA_PEVA_INTEGER,pgs_aa_peva_integer, \
            INT, PSTRING, PINT)

/* PGS_AA_3Dgeo() */
/* Was commented out in original file, so left that way */
/*
FCALLSCFUN10(INT,PGS_AA_3Dgeo,PGS_AA_3Dgeo,pgs_aa_3dgeo,STRINGV,INT, \
             DOUBLEV,DOUBLEV,INTV,INT,INT,INT,INT,PVOID)
*/
#define pgs_aa_dcw_STRV_A1 NUM_ELEMS(4)
FCALLSCFUN6( INT, PGS_AA_dcw, PGS_AA_DCW, pgs_aa_dcw, STRINGV, INT, \
                  DOUBLEV, DOUBLEV, INT, PVOID)
