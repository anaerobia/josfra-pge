/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_bindFORTRAN.c

DESCRIPTION:
  This file contain FORTRAN bindings for the SDP (aka PGS) Toolkit Time and 
  Date (TD) tools. 

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  02-Jun-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

#include <cfortran.h>
#include <PGS_TD.h>

/**************************
 * INTERMEDIARY FUNCTIONS *
 **************************/

/* cfortran.h does not allow FORTRAN to bind gracefully to SOME functions,
   for these special cases intermediary functions are defined here */

/* PGS_TD_ADEOSIItoUTC() */

static PGSt_SMF_status
PGS_TD_ADEOSIItoUTCf(
    char *scTime,
    char *asciiUTC)
{
    return PGS_TD_ADEOSIItoUTC((PGSt_scTime*) scTime, asciiUTC);
}

/* PGS_TD_EOSAMtoUTC() */

static PGSt_SMF_status
PGS_TD_EOSAMtoUTCf(
    char *scTime,
    char *asciiUTC)
{
    return PGS_TD_EOSAMtoUTC((PGSt_scTime*) scTime, asciiUTC);
}

/* PGS_TD_EOSPMtoUTC() */

static PGSt_SMF_status
PGS_TD_EOSPMtoUTCf(
    char *scTime,
    char *asciiUTC)
{
    return PGS_TD_EOSPMtoUTC((PGSt_scTime*) scTime, asciiUTC);
}
/* PGS_TD_EOSPMGIIStoUTC() */

static PGSt_SMF_status
PGS_TD_EOSPMGIIStoUTCf(
    char *scTime,
    char *asciiUTC)
{
    return PGS_TD_EOSPMGIIStoUTC((PGSt_scTime*) scTime, asciiUTC);
}

/* PGS_TD_EOSPMGIRDtoUTC() */

static PGSt_SMF_status
PGS_TD_EOSPMGIRDtoUTCf(
    char *scTime,
    char *asciiUTC)
{
    return PGS_TD_EOSPMGIRDtoUTC((PGSt_scTime*) scTime, asciiUTC);
}

/* PGS_TD_EOSAURAtoUTC() */

static PGSt_SMF_status
PGS_TD_EOSAURAtoUTCf(
    char *scTime,
    char *asciiUTC)
{
    return PGS_TD_EOSAURAtoUTC((PGSt_scTime*) scTime, asciiUTC);
}

/* PGS_TD_TRMMtoUTC() */

static PGSt_SMF_status
PGS_TD_TRMMtoUTCf(
    char *scTime,
    char *asciiUTC)
{
    return PGS_TD_TRMMtoUTC((PGSt_scTime*) scTime, asciiUTC);
}

/* PGS_TD_SCtime_to_UTC() */

static PGSt_SMF_status
PGS_TD_SCtime_to_UTCf(
    PGSt_tag     spacecraftTag,
    char         *scTime,
    PGSt_integer numValues,
    char         *asciiUTC,
    PGSt_double  offsets[])
{
    return PGS_TD_SCtime_to_UTC(spacecraftTag,(PGSt_scTime (*)[8])scTime,
				numValues,asciiUTC,offsets);
}


/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_TD_ADEOSIItoTAI() */

FCALLSCFUN2(INT,PGS_TD_ADEOSIItoTAI,PGS_TD_ADEOSIITOTAI,pgs_td_adeosiitotai, \
	    PVOID, PDOUBLE)

/* PGS_TD_ADEOSIItoUTC() */

FCALLSCFUN2(INT,PGS_TD_ADEOSIItoUTCf,PGS_TD_ADEOSIITOUTC,pgs_td_adeosiitoutc, \
	    PPSTRING, PSTRING)

/* PGS_TD_ASCIItime_AtoB() */

FCALLSCFUN2(INT, PGS_TD_ASCIItime_AtoB, PGS_TD_ASCIITIME_ATOB, \
	    pgs_td_asciitime_atob, STRING, PSTRING)

/* PGS_TD_ASCIItime_BtoA() */

FCALLSCFUN2(INT, PGS_TD_ASCIItime_BtoA, PGS_TD_ASCIITIME_BTOA, \
	    pgs_td_asciitime_btoa, STRING, PSTRING)

/* PGS_TD_EOSAMtoTAI() */

FCALLSCFUN2(INT, PGS_TD_EOSAMtoTAI, PGS_TD_EOSAMTOTAI, pgs_td_eosamtotai, \
	    PVOID, PDOUBLE)

/* PGS_TD_EOSAMtoUTC() */

FCALLSCFUN2(INT, PGS_TD_EOSAMtoUTCf, PGS_TD_EOSAMTOUTC, pgs_td_eosamtoutc, \
	    PPSTRING, PSTRING)

/* PGS_TD_EOSPMtoTAI() */

FCALLSCFUN2(INT, PGS_TD_EOSPMtoTAI, PGS_TD_EOSPMTOTAI, pgs_td_eospmtotai, \
	    PVOID, PDOUBLE)

/* PGS_TD_EOSPMGIIStoTAI() */

FCALLSCFUN2(INT, PGS_TD_EOSPMGIIStoTAI, PGS_TD_EOSPMGIISTOTAI, \
            pgs_td_eospmgiistotai, PVOID, PDOUBLE)

/* PGS_TD_EOSPMGIRDtoTAI() */

FCALLSCFUN2(INT, PGS_TD_EOSPMGIRDtoTAI, PGS_TD_EOSPMGIRDTOTAI, \
            pgs_td_eospmgirdtotai, PVOID, PDOUBLE)

/* PGS_TD_EOSPMtoUTC() */

FCALLSCFUN2(INT, PGS_TD_EOSPMtoUTCf, PGS_TD_EOSPMTOUTC, pgs_td_eospmtoutc, \
	    PPSTRING, PSTRING)

/* PGS_TD_EOSPMGIIStoUTC() */

FCALLSCFUN2(INT, PGS_TD_EOSPMGIIStoUTCf, PGS_TD_EOSPMGIISTOUTC, \
            pgs_td_eospmgiistoutc, PPSTRING, PSTRING)

/* PGS_TD_EOSPMGIRDtoUTC() */

FCALLSCFUN2(INT, PGS_TD_EOSPMGIRDtoUTCf, PGS_TD_EOSPMGIRDTOUTC, \
            pgs_td_eospmgirdtoutc, PPSTRING, PSTRING)

/* PGS_TD_EOSAURAGIIStoTAI() */

FCALLSCFUN2(INT, PGS_TD_EOSAURAGIIStoTAI, PGS_TD_EOSAURAGIISTOTAI, pgs_td_eosauragiistotai, \
            PVOID, PDOUBLE)

/* PGS_TD_EOSAURAGIRDtoTAI() */
 
FCALLSCFUN2(INT, PGS_TD_EOSAURAGIRDtoTAI, PGS_TD_EOSAURAGIRDTOTAI, pgs_td_eosauragirdtotai, \
            PVOID, PDOUBLE)

/* PGS_TD_EOSAURAtoUTC() */

FCALLSCFUN2(INT, PGS_TD_EOSAURAtoUTCf, PGS_TD_EOSAURATOUTC, pgs_td_eosauratoutc, \
            PPSTRING, PSTRING)

/* PGS_TD_FGDCtoUTC() */

FCALLSCFUN3(INT, PGS_TD_FGDCtoUTC, PGS_TD_FGDCTOUTC, pgs_td_fgdctoutc, \
	    STRING, STRING, PSTRING)

/* PGS_TD_ISOinttoTAI() */

FCALLSCFUN2(INT, PGS_TD_ISOinttoTAI, PGS_TD_ISOINTTOTAI, pgs_td_isointtotai, \
	    INT, PDOUBLE)

/* PGS_TD_ISOinttoUTCjd() */

FCALLSCFUN2(INT, PGS_TD_ISOinttoUTCjd, PGS_TD_ISOINTTOUTCJD, \
	    pgs_td_isointtoutcjd, INT, DOUBLEV)

/* PGS_TD_JDtoMJD() */

FCALLSCSUB2(PGS_TD_JDtoMJD, PGS_TD_JDTOMJD, pgs_td_jdtomjd, DOUBLEV, DOUBLEV)

/* PGS_TD_JDtoTJD() */

FCALLSCSUB2(PGS_TD_JDtoTJD, PGS_TD_JDTOTJD, pgs_td_jdtotjd, DOUBLEV, DOUBLEV)

/* PGS_TD_GPStoUTC() */

FCALLSCFUN2(INT, PGS_TD_GPStoUTC, PGS_TD_GPSTOUTC, pgs_td_gpstoutc, DOUBLE, \
	    PSTRING)

/* PGS_TD_JulianDateSplit() */

FCALLSCSUB2(PGS_TD_JulianDateSplit, PGS_TD_JULIANDATESPLIT, \
	    pgs_td_juliandatesplit, DOUBLEV, DOUBLEV)

/* PGS_TD_LeapSec() */

FCALLSCFUN5(INT, PGS_TD_LeapSec, PGS_TD_LEAPSEC, pgs_td_leapsec, DOUBLEV, \
	    PDOUBLE, PDOUBLE, PDOUBLE, PSTRING)

/* PGS_TD_MJDtoJD() */

FCALLSCSUB2(PGS_TD_MJDtoJD, PGS_TD_MJDTOJD, pgs_td_mjdtojd, DOUBLEV, DOUBLEV)

/* PGS_TD_ManageTMDF() */

FCALLSCFUN4(INT, PGS_TD_ManageTMDF,  PGS_TD_MANAGETMDF,  pgs_td_managetmdf, \
	    INT, PDOUBLE, PDOUBLE, PDOUBLE)

/* PGS_TD_ManageUTCF() */

FCALLSCFUN2(INT, PGS_TD_ManageUTCF,  PGS_TD_MANAGEUTCF,  pgs_td_manageutcf, \
	    INT, PDOUBLE)

/* PGS_TD_PB5toTAI() */

FCALLSCFUN2(INT, PGS_TD_PB5toTAI, PGS_TD_PB5TOTAI, pgs_td_pb5totai, PVOID, \
	    PDOUBLE)

/* PGS_TD_PB5CtoUTCjd() */

FCALLSCFUN2(INT, PGS_TD_PB5CtoUTCjd, PGS_TD_PB5CTOUTCJD, pgs_td_pb5ctoutcjd, \
	    PVOID, DOUBLEV)

/* PGS_TD_PB5toUTCjd() */

FCALLSCFUN2(INT, PGS_TD_PB5toUTCjd, PGS_TD_PB5TOUTCJD, pgs_td_pb5toutcjd, \
	    PVOID, DOUBLEV)

/* PGS_TD_SCtime_to_UTC() */

FCALLSCFUN5(INT, PGS_TD_SCtime_to_UTCf, PGS_TD_SCTIME_TO_UTC, \
	    pgs_td_sctime_to_utc, INT, PPSTRING, INT, PSTRING, DOUBLEV)

/* PGS_TD_TAIjdtoTAI() */

FCALLSCFUN1(DOUBLE, PGS_TD_TAIjdtoTAI, PGS_TD_TAIJDTOTAI, pgs_td_taijdtotai, \
	    DOUBLEV)

/* PGS_TD_TAIjdtoTDTjed() */

FCALLSCSUB2(PGS_TD_TAIjdtoTDTjed, PGS_TD_TAIJDTOTDTJED, pgs_td_taijdtotdtjed, \
	    DOUBLEV, DOUBLEV)

/* PGS_TD_TAIjdtoUTCjd() */

FCALLSCFUN2(INT, PGS_TD_TAIjdtoUTCjd, PGS_TD_TAIJDTOUTCJD, pgs_td_taijdtoutcjd,\
	    DOUBLEV, DOUBLEV)

/* PGS_TD_TAItoGAST() */

FCALLSCFUN2(INT, PGS_TD_TAItoGAST, PGS_TD_TAITOGAST, pgs_td_taitogast, \
	    DOUBLE, PDOUBLE)

/* PGS_TD_TAItoISOint() */

FCALLSCFUN2(INT, PGS_TD_TAItoISOint, PGS_TD_TAITOISOINT, pgs_td_taitoisoint,\
	    DOUBLE, PINT)

/* PGS_TD_TAItoTAIjd() */

FCALLSCSUB2(PGS_TD_TAItoTAIjd, PGS_TD_TAITOTAIJD, pgs_td_taitotaijd, \
	    DOUBLE, DOUBLEV)

/* PGS_TD_TAItoUDTF() */

FCALLSCFUN2(INT, PGS_TD_TAItoUDTF, PGS_TD_TAITOUDTF, pgs_td_taitoudtf, \
	    DOUBLE, INTV)

/* PGS_TD_TAItoUT1jd() */

FCALLSCFUN2(INT, PGS_TD_TAItoUT1jd, PGS_TD_TAITOUT1JD, pgs_td_taitout1jd, \
	    DOUBLE, DOUBLEV)

/* PGS_TD_TAItoUT1pole() */

FCALLSCFUN6(INT, PGS_TD_TAItoUT1pole, PGS_TD_TAITOUT1POLE, pgs_td_taitout1pole,\
	    DOUBLE, DOUBLEV, PDOUBLE, PDOUBLE, PDOUBLE, PDOUBLE)

/* PGS_TD_TAItoUTC() */

FCALLSCFUN2(INT, PGS_TD_TAItoUTC, PGS_TD_TAITOUTC, pgs_td_taitoutc, DOUBLE, \
	    PSTRING)

/* PGS_TD_TAItoUTCjd() */

FCALLSCFUN2(INT, PGS_TD_TAItoUTCjd, PGS_TD_TAITOUTCJD, pgs_td_taitoutcjd, \
            DOUBLE, DOUBLEV)

/* PGS_TD_TDBjedtoTDTjed() */

FCALLSCSUB2(PGS_TD_TDBjedtoTDTjed,PGS_TD_TDBJEDTOTDTJED,pgs_td_tdbjedtotdtjed,\
	    DOUBLEV, DOUBLEV)

/* PGS_TD_TDTjedtoTAIjd() */

FCALLSCSUB2(PGS_TD_TDTjedtoTAIjd, PGS_TD_TDTJEDTOTAIJD, pgs_td_tdtjedtotaijd, \
	    DOUBLEV, DOUBLEV)

/* PGS_TD_TDTjedtoTDBjed() */

FCALLSCSUB2(PGS_TD_TDTjedtoTDBjed,PGS_TD_TDTJEDTOTDBJED,pgs_td_tdtjedtotdbjed,\
	    DOUBLEV, DOUBLEV)

/* PGS_TD_TJDtoJD() */

FCALLSCSUB2(PGS_TD_TJDtoJD, PGS_TD_TJDTOJD, pgs_td_tjdtojd, DOUBLEV, DOUBLEV)

/* PGS_TD_TRMMtoTAI() */

FCALLSCFUN2(INT, PGS_TD_TRMMtoTAI, PGS_TD_TRMMTOTAI, pgs_td_trmmtotai, PVOID, \
	    PDOUBLE)

/* PGS_TD_TRMMtoUTC() */

FCALLSCFUN2(INT, PGS_TD_TRMMtoUTCf, PGS_TD_TRMMTOUTC, pgs_td_trmmtoutc, \
	    PPSTRING, PSTRING)

/* PGS_TD_TimeInterval() */

FCALLSCFUN3(INT, PGS_TD_TimeInterval, PGS_TD_TIMEINTERVAL, pgs_td_timeinterval,\
	    DOUBLE, DOUBLE, PDOUBLE)

/* PGS_TD_UDTFtoTAI() */

FCALLSCFUN2(INT, PGS_TD_UDTFtoTAI, PGS_TD_UDTFTOTAI, pgs_td_udtftotai,\
	    INTV, DOUBLEV)

/* PGS_TD_UDTFtoUTCjd() */

FCALLSCFUN2(INT, PGS_TD_UDTFtoUTCjd, PGS_TD_UDTFTOUTCJD, pgs_td_udtftoutcjd,\
	    INTV, DOUBLEV)

/* PGS_TD_UT1jdtoUTCjd() */

FCALLSCFUN2(INT, PGS_TD_UT1jdtoUTCjd, PGS_TD_UT1JDTOUTCJD, pgs_td_ut1jdtoutcjd,\
	    DOUBLEV, DOUBLEV)

/* PGS_TD_UTC_to_SCtime() */

FCALLSCFUN3(INT,PGS_TD_UTC_to_SCtime,PGS_TD_UTC_TO_SCTIME,pgs_td_utc_to_sctime,\
	    INT, STRING, PVOID)

/* PGS_TD_UTCjdtoISOint() */

FCALLSCFUN2(INT, PGS_TD_UTCjdtoISOint, PGS_TD_UTCJDTOISOINT, \
	    pgs_td_utcjdtoisoint, DOUBLEV, PINT)

/* PGS_TD_UTCjdtoPB5C() */

FCALLSCFUN3(INT, PGS_TD_UTCjdtoPB5C, PGS_TD_UTCJDTOPB5C, pgs_td_utcjdtopb5c,\
            DOUBLEV, INT, PVOID)

/* PGS_TD_UTCjdtoPB5() */

FCALLSCFUN3(INT, PGS_TD_UTCjdtoPB5, PGS_TD_UTCJDTOPB5, pgs_td_utcjdtopb5,\
            DOUBLEV, INT, PVOID)

/* PGS_TD_UTCjdtoTAIjd() */

FCALLSCFUN3(INT, PGS_TD_UTCjdtoTAIjd, PGS_TD_UTCJDTOTAIJD, pgs_td_utcjdtotaijd,\
            DOUBLEV, INT, DOUBLEV)

/* PGS_TD_UTCjdtoUDTF() */

FCALLSCFUN3(INT, PGS_TD_UTCjdtoUDTF, PGS_TD_UTCJDTOUDTF, pgs_td_utcjdtoudtf, \
	    DOUBLEV, INT, INTV)

/* PGS_TD_UTCjdtoUT1jd() */

FCALLSCFUN3(INT, PGS_TD_UTCjdtoUT1jd, PGS_TD_UTCJDTOUT1JD, pgs_td_utcjdtout1jd,\
	    DOUBLEV, INT, DOUBLEV)

/* PGS_TD_UTCjdtoUTC() */

FCALLSCFUN3(INT, PGS_TD_UTCjdtoUTC, PGS_TD_UTCJDTOUTC, pgs_td_utcjdtoutc, \
	    DOUBLEV, INT, PSTRING)

/* PGS_TD_UTCtoEOSAM() */

FCALLSCFUN2(INT, PGS_TD_UTCtoEOSAM, PGS_TD_UTCTOEOSAM, pgs_td_utctoeosam, \
	    STRING, PVOID)

/* PGS_TD_UTCtoADEOSII() */

FCALLSCFUN2(INT, PGS_TD_UTCtoADEOSII, PGS_TD_UTCTOADEOSII, pgs_td_utctoadoesii,\
	    STRING, PVOID)

/* PGS_TD_UTCtoEOSPM() */

FCALLSCFUN2(INT, PGS_TD_UTCtoEOSPM, PGS_TD_UTCTOEOSPM, pgs_td_utctoeospm, \
	    STRING, PVOID)

/* PGS_TD_UTCtoEOSPMGIIS() */

FCALLSCFUN2(INT, PGS_TD_UTCtoEOSPMGIIS, PGS_TD_UTCTOEOSPMGIIS, \
            pgs_td_utctoeospmgiis, STRING, PVOID)

/* PGS_TD_UTCtoEOSPMGIRD() */

FCALLSCFUN2(INT, PGS_TD_UTCtoEOSPMGIRD, PGS_TD_UTCTOEOSPMGIRD, \
            pgs_td_utctoeospmgird, STRING, PVOID)

/* PGS_TD_UTCtoFGDC() */

FCALLSCFUN4(INT, PGS_TD_UTCtoFGDC, PGS_TD_UTCTOFGDC, pgs_td_utctofgdc, \
	    STRING, STRING, PSTRING, PSTRING)

/* PGS_TD_UTCtoGPS() */

FCALLSCFUN2(INT, PGS_TD_UTCtoGPS, PGS_TD_UTCTOGPS, pgs_td_utctogps, STRING, \
	    PDOUBLE)

/* PGS_TD_UTCtoTAI() */

FCALLSCFUN2(INT, PGS_TD_UTCtoTAI, PGS_TD_UTCTOTAI, pgs_td_utctotai, PSTRING, \
	    PDOUBLE)

/* PGS_TD_UTCtoTAIjd() */

FCALLSCFUN2(INT, PGS_TD_UTCtoTAIjd, PGS_TD_UTCTOTAIJD, pgs_td_utctotaijd, \
	    STRING, DOUBLEV)

/* PGS_TD_UTCtoTDBjed() */

FCALLSCFUN2(INT, PGS_TD_UTCtoTDBjed, PGS_TD_UTCTOTDBJED, pgs_td_utctotdbjed, \
	    STRING, DOUBLEV)

/* PGS_TD_UTCtoTDTjed() */

FCALLSCFUN2(INT, PGS_TD_UTCtoTDTjed, PGS_TD_UTCTOTDTJED, pgs_td_utctotdtjed, \
	    STRING, DOUBLEV)

/* PGS_TD_UTCtoTRMM() */

FCALLSCFUN2(INT, PGS_TD_UTCtoTRMM, PGS_TD_UTCTOTRMM, pgs_td_utctotrmm, \
	    STRING, PVOID)

/* PGS_TD_UTCtoUT1() */

FCALLSCFUN2(INT, PGS_TD_UTCtoUT1, PGS_TD_UTCTOUT1, pgs_td_utctout1, STRING, \
	    PDOUBLE)

/* PGS_TD_UTCtoUT1jd() */

FCALLSCFUN2(INT, PGS_TD_UTCtoUT1jd, PGS_TD_UTCTOUT1JD, pgs_td_utctout1jd, \
            STRING, DOUBLEV)

/* PGS_TD_UTCtoUTCjd() */

FCALLSCFUN2(INT, PGS_TD_UTCtoUTCjd, PGS_TD_UTCTOUTCJD, pgs_td_utctoutcjd, \
            STRING, DOUBLEV)

/* PGS_TD_calday() */

FCALLSCSUB4(PGS_TD_calday, PGS_TD_CALDAY, pgs_td_calday, INT, PINT, PINT, PINT)

/* PGS_TD_julday() */

FCALLSCFUN3(INT, PGS_TD_julday, PGS_TD_JULDAY, pgs_td_julday, INT, INT, INT)

/* PGS_TD_gast() */

FCALLSCFUN3(DOUBLE, PGS_TD_gast, PGS_TD_GAST, pgs_td_gast, DOUBLE, DOUBLE, \
	    DOUBLEV)

/* PGS_TD_gmst() */

FCALLSCFUN1(DOUBLE, PGS_TD_gmst, PGS_TD_GMST, pgs_td_gmst, DOUBLEV)

/* PGS_TD_sortArrayIndices() */

FCALLSCFUN3(INT, PGS_TD_sortArrayIndices, PGS_TD_SORTARRAYINDICES, \
	    pgs_td_sortarrayindices, DOUBLEV, INT, INTV)

/* PGS_TD_timeCheck() */

FCALLSCFUN1(INT, PGS_TD_timeCheck, PGS_TD_TIMECHECK, pgs_td_timecheck, STRING)





