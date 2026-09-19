/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_CSC_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Coordinate System
  Conversion (CSC) tools. 

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  02-Jun-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

#include <cfortran.h>
#include <PGS_CSC.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_CSC_BorkowskiGeo() */

FCALLSCFUN6(INT,PGS_CSC_BorkowskiGeo,PGS_CSC_BORKOWSKIGEO,pgs_csc_borkowskigeo,\
	    DOUBLE, DOUBLE, DOUBLE, DOUBLE, PDOUBLE, PDOUBLE)

/* PGS_CSC_DayNight() */

FCALLSCFUN7(INT, PGS_CSC_DayNight, PGS_CSC_DAYNIGHT, pgs_csc_daynight, \
	    INT, PSTRING, DOUBLEV, DOUBLEV, DOUBLEV, INT, INTV)

/* PGS_CSC_ECItoECR() */

FCALLSCFUN5(INT, PGS_CSC_ECItoECR, PGS_CSC_ECITOECR, pgs_csc_ecitoecr, \
	    INT, STRING, DOUBLEV, DOUBLEVV, DOUBLEVV)

/* PGS_CSC_ECItoORB() */

FCALLSCFUN6(INT, PGS_CSC_ECItoORB, PGS_CSC_ECITOORB, pgs_csc_ecitoorb, INT, \
	    INT, STRING, DOUBLEV, DOUBLEVV, DOUBLEVV)

/* PGS_CSC_ECItoSC() */

FCALLSCFUN6(INT, PGS_CSC_ECItoSC, PGS_CSC_ECITOSC, \
	    pgs_csc_ecitosc, INT, INT, STRING, DOUBLEV, DOUBLEVV, DOUBLEVV)

/* PGS_CSC_ECRtoECI() */

FCALLSCFUN5(INT, PGS_CSC_ECRtoECI, PGS_CSC_ECRTOECI, pgs_csc_ecrtoeci, \
	    INT, STRING, DOUBLEV, DOUBLEVV, DOUBLEVV)

/* PGS_CSC_ECRtoGEO() */

FCALLSCFUN5(INT, PGS_CSC_ECRtoGEO, PGS_CSC_ECRTOGEO, pgs_csc_ecrtogeo, \
	    DOUBLEV, STRING, PDOUBLE, PDOUBLE, PDOUBLE)

/* PGS_CSC_EarthOccult() */

FCALLSCFUN7(INT, PGS_CSC_EarthOccult, PGS_CSC_EARTHOCCULT, \
	    pgs_csc_earthoccult, DOUBLEV, DOUBLE, DOUBLE, DOUBLEV, \
            DOUBLEV, DOUBLEV, PINT)	

/* PGS_CSC_Earthpt_FixedFOV() */

FCALLSCFUN13(INT, PGS_CSC_Earthpt_FixedFOV, PGS_CSC_EARTHPT_FIXEDFOV, \
	     pgs_csc_earthpt_fixedfov, INT, STRING, DOUBLEV, INT, STRING, \
	     DOUBLE, DOUBLE, DOUBLE, INT, DOUBLEVV, DOUBLEVVV, INTV, \
             DOUBLEVV)

/* PGS_CSC_Earthpt_FOV() */

FCALLSCFUN13(INT, PGS_CSC_Earthpt_FOV, PGS_CSC_EARTHPT_FOV, \
	     pgs_csc_earthpt_fov, INT, STRING, DOUBLEV, INT, STRING, DOUBLE, \
	     DOUBLE, DOUBLE, INT, DOUBLEVV, DOUBLEVVV, INTV, DOUBLEVV)

/* PGS_CSC_EulerToQuat() */

FCALLSCFUN3(INT, PGS_CSC_EulerToQuat, PGS_CSC_EULERTOQUAT, \
	    pgs_csc_eulertoquat, DOUBLEV, INTV, DOUBLEV)

/* PGS_CSC_FOVconicalHull() */

FCALLSCFUN8(INT, PGS_CSC_FOVconicalHull, PGS_CSC_FOVCONICALHULL, \
	    pgs_csc_fovconicalhull, INT, DOUBLEV, DOUBLEVV, DOUBLE, DOUBLEV, \
	    DOUBLEVV, PINT, PDOUBLE)

/* PGS_CSC_GEOtoECR() */

FCALLSCFUN5(INT, PGS_CSC_GEOtoECR, PGS_CSC_GEOTOECR, pgs_csc_geotoecr, \
	    DOUBLE, DOUBLE, DOUBLE, STRING, DOUBLEV)

/* PGS_CSC_GeoCenToRect() */

FCALLSCSUB4(PGS_CSC_GeoCenToRect, PGS_CSC_GEOCENTORECT, pgs_csc_geocentorect, \
	    DOUBLE, DOUBLE, DOUBLE, DOUBLEV)

/* PGS_CSC_GetEarthFigure() */

FCALLSCFUN3(INT, PGS_CSC_GetEarthFigure, PGS_CSC_GETEARTHFIGURE, \
		 pgs_csc_getearthfigure, STRING, PDOUBLE, PDOUBLE)

/* PGS_CSC_GetFOV_Pixel() */

FCALLSCFUN13(INT, PGS_CSC_GetFOV_Pixel, PGS_CSC_GETFOV_PIXEL, \
	     pgs_csc_getfov_pixel, INT, INT, STRING, DOUBLEV, STRING, INT, \
	     DOUBLEVV, DOUBLEVV, DOUBLEV, DOUBLEV, DOUBLEVV, DOUBLEV, DOUBLEV)

/* PGS_CSC_GrazingRay() */

FCALLSCFUN9(INT,PGS_CSC_GrazingRay,PGS_CSC_GRAZINGRAY,pgs_csc_grazingray, \
            STRING,DOUBLEV,DOUBLEV,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,  \
            DOUBLEV,DOUBLEV)

/* PGS_CSC_GreenwichHour() */

FCALLSCFUN4(INT, PGS_CSC_GreenwichHour, PGS_CSC_GREENWICHHOUR, \
	    pgs_csc_greenwichhour, INT, PSTRING, DOUBLEV, DOUBLEV)

/* PGS_CSC_J2000toTOD() */

FCALLSCFUN4(INT, PGS_CSC_TODtoJ2000, PGS_CSC_TODTOJ2000, pgs_csc_todtoj2000, \
            INT, DOUBLE, DOUBLEV, DOUBLEV)

/* PGS_CSC_LookTwice() */

FCALLSCFUN7(INT, PGS_CSC_LookTwice, PGS_CSC_LOOKTWICE, pgs_csc_looktwice, \
	    DOUBLEV, DOUBLEV, DOUBLE, DOUBLE, DOUBLE, DOUBLEV, DOUBLEVV)

/* PGS_CSC_ORBtoECI() */

FCALLSCFUN6(INT, PGS_CSC_ORBtoECI, PGS_CSC_ORBTOECI, pgs_csc_orbtoeci, INT, \
	    INT, STRING, DOUBLEV, DOUBLEVV, DOUBLEVV)

/* PGS_CSC_ORBtoSC() */

FCALLSCFUN6(INT, PGS_CSC_ORBtoSC, PGS_CSC_ORBTOSC, \
	    pgs_csc_orbtosc, INT, INT, STRING, DOUBLEV, DOUBLEVV, DOUBLEVV)

/* PGS_CSC_PointInFOVgeom() */

FCALLSCFUN5(INT, PGS_CSC_PointInFOVgeom, PGS_CSC_POINTINFOVGEOM, \
            pgs_csc_pointinfovgeom, INT, DOUBLEV, DOUBLEVV, DOUBLEV, \
            PINT)

/* PGS_CSC_QuatToEuler() */

FCALLSCFUN3(INT, PGS_CSC_QuatToEuler, PGS_CSC_QUATTOEULER, \
	    pgs_csc_quattoeuler, DOUBLEV, INTV, DOUBLEV)

/* PGS_CSC_QuatToMatrix() */

FCALLSCFUN2(INT, PGS_CSC_QuatToMatrix, PGS_CSC_QUATTOMATRIX, \
	    pgs_csc_quattomatrix, DOUBLEV, DOUBLEVV)

/* PGS_CSC_RectToGeoCen() */

FCALLSCSUB4(PGS_CSC_RectToGeoCen,PGS_CSC_RECTTOGEOCEN,pgs_csc_recttogeocen, \
	    DOUBLEV,PDOUBLE,PDOUBLE,PDOUBLE)

/* PGS_CSC_SCtoECI() */

FCALLSCFUN6(INT, PGS_CSC_SCtoECI, PGS_CSC_SCTOECI, \
	    pgs_csc_sctoeci, INT, INT, STRING, DOUBLEV, DOUBLEVV, DOUBLEVV)

/* PGS_CSC_SCtoORB() */

FCALLSCFUN6(INT, PGS_CSC_SCtoORB, PGS_CSC_SCTOORB, \
	    pgs_csc_sctoorb, INT, INT, STRING, DOUBLEV, DOUBLEVV, DOUBLEVV)

/* PGS_CSC_SpaceRefract() */

FCALLSCFUN5(INT, PGS_CSC_SpaceRefract, PGS_CSC_SPACEREFRACT, \
	    pgs_csc_spacerefract, DOUBLE, DOUBLE, DOUBLE, PDOUBLE, PDOUBLE)

/* PGS_CSC_SubSatPoint() */

FCALLSCFUN10(INT, PGS_CSC_SubSatPoint,PGS_CSC_SUBSATPOINT,pgs_csc_subsatpoint,\
	     INT, INT, PSTRING, DOUBLEV, STRING, INT, DOUBLEV, DOUBLEV, \
	     DOUBLEV, DOUBLEVV)

/* PGS_CSC_SubSatPointVel() */

FCALLSCFUN7(INT, PGS_CSC_SubSatPointVel, PGS_CSC_SUBSATPOINTVEL, \
	    pgs_csc_subsatpointvel, INT, DOUBLEVV, STRING, DOUBLEV, DOUBLEV, \
	    DOUBLEV, DOUBLEVV)

/* PGS_CSC_TODtoJ2000() */

FCALLSCFUN4(INT, PGS_CSC_J2000toTOD, PGS_CSC_J2000TOTOD, pgs_csc_j2000totod, \
            INT, DOUBLE, DOUBLEV, DOUBLEV)

/* PGS_CSC_TiltYaw() */

FCALLSCFUN5(INT, PGS_CSC_TiltYaw, PGS_CSC_TILTYAW, pgs_csc_tiltyaw, \
	    DOUBLEV, DOUBLEV, DOUBLE, DOUBLE, DOUBLEVV)

/* PGS_CSC_UTC_UT1Pole() */

FCALLSCFUN5(INT, PGS_CSC_UTC_UT1Pole, PGS_CSC_UTC_UT1POLE, pgs_csc_utc_ut1pole,\
	    DOUBLEV, PDOUBLE, PDOUBLE, PDOUBLE, PDOUBLE)

/* PGS_CSC_VecToVecAngle() */

FCALLSCFUN4(DOUBLE, PGS_CSC_VecToVecAngle, PGS_CSC_VECTOVECANGLE, \
	    pgs_csc_vectovecangle, DOUBLE, DOUBLE, DOUBLE, DOUBLE)

/* PGS_CSC_ZenithAzimuth() */

FCALLSCFUN10(INT, PGS_CSC_ZenithAzimuth, PGS_CSC_ZENITHAZIMUTH, \
	     pgs_csc_zenithazimuth, DOUBLEV, DOUBLE, DOUBLE, DOUBLE, \
	     INT, INT, INT, PDOUBLE, PDOUBLE, PDOUBLE)

/* PGS_CSC_nutate2000() */

FCALLSCFUN5(INT, PGS_CSC_nutate2000, PGS_CSC_NUTATE2000, pgs_csc_nutate2000, \
	    INT, DOUBLEV, DOUBLEV, INT, DOUBLEV)

/* PGS_CSC_precs2000() */

FCALLSCFUN4(INT, PGS_CSC_precs2000, PGS_CSC_PRECS2000, pgs_csc_precs2000, \
	    INT, DOUBLEV, INT, DOUBLEV)

/* PGS_CSC_quatMultiply() */

FCALLSCFUN3(INT,PGS_CSC_quatMultiply,PGS_CSC_QUATMULTIPLY,pgs_csc_quatmultiply,\
	    DOUBLEV, DOUBLEV, DOUBLEV)

/* PGS_CSC_wahr2() */

FCALLSCFUN2(INT, PGS_CSC_wahr2, PGS_CSC_WAHR2, pgs_csc_wahr2, DOUBLEV, DOUBLEV)
