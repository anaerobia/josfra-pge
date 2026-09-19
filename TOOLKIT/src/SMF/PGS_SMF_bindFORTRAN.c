/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_SMF_bindFORTRAN.c

DESCRIPTION:
  This file contains FORTRAN bindings for the SDP Toolkit Status Message
  Facility (SMF) tools.

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
  11-Dec-1995  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

#include <cfortran.h>
#include <PGS_SMF.h>

/*********************
 * cfortran.h MACROS *
 *********************/

/* The following macros are expanded by cfortran.h to C functions which allow
   FORTRAN users to access the SDP Toolkit relatively painlessly. */

/* PGS_SMF_Begin() */

FCALLSCFUN1(INT, PGS_SMF_Begin, PGS_SMF_BEGIN, pgs_smf_begin, STRING)

/* PGS_SMF_CreateMsgTag() */

FCALLSCFUN1(INT, PGS_SMF_CreateMsgTag, PGS_SMF_CREATEMSGTAG,  \
	    pgs_smf_createmsgtag, PSTRING)       

/* PGS_SMF_End() */

FCALLSCFUN1(INT, PGS_SMF_End, PGS_SMF_END, pgs_smf_end, STRING)

/* PGS_SMF_GenerateStatusReport() */

FCALLSCFUN1(INT, PGS_SMF_GenerateStatusReport, PGS_SMF_GENERATESTATUSREPORT, \
	    pgs_smf_generatestatusreport,STRING)      

FCALLSCFUN2(INT,PGS_SMF_GetActionByCode, PGS_SMF_GETACTIONBYCODE, \
	    pgs_smf_getactionbycode,INT,PSTRING)

/* PGS_SMF_GetInstrName() */

FCALLSCFUN2(INT, PGS_SMF_GetInstrName, PGS_SMF_GETINSTRNAME, \
	    pgs_smf_getinstrname,INT,PSTRING)

/* PGS_SMF_GetMsg() */

FCALLSCSUB3(PGS_SMF_GetMsg, PGS_SMF_GETMSG, pgs_smf_getmsg, PINT, PSTRING, \
	    PSTRING)

/* PGS_SMF_GetMsgByCode() */

FCALLSCFUN2(INT, PGS_SMF_GetMsgByCode, PGS_SMF_GETMSGBYCODE, \
	    pgs_smf_getmsgbycode, INT, PSTRING)

/* PGS_SMF_GetToolkitVersion() */

FCALLSCSUB1(PGS_SMF_GetToolkitVersion, PGS_SMF_GETTOOLKITVERSION, \
	    pgs_smf_gettoolkitversion, PSTRING)

/* PGS_SMF_SendRuntimeData() */

FCALLSCFUN3(INT, PGS_SMF_SendRuntimeData, PGS_SMF_SENDRUNTIMEDATA, \
	    pgs_smf_sendruntimedata,INT,PVOID,INTV)

/* PGS_SMF_SendStatusReport() */

FCALLSCFUN0(INT, PGS_SMF_SendStatusReport, PGS_SMF_SENDSTATUSREPORT, \
	    pgs_smf_sendstatusreport)

/* PGS_SMF_SetDynamicMsg() */

FCALLSCFUN3(INT, PGS_SMF_SetDynamicMsg, PGS_SMF_SETDYNAMICMSG, \
	    pgs_smf_setdynamicmsg,INT,STRING,STRING)

/* PGS_SMF_SetStaticMsg() */

FCALLSCFUN2(INT, PGS_SMF_SetStaticMsg, PGS_SMF_SETSTATICMSG, \
	    pgs_smf_setstaticmsg,INT,STRING)

/* PGS_SMF_SetUNIXMsg() */

FCALLSCFUN3(INT, PGS_SMF_SetUNIXMsg, PGS_SMF_SETUNIXMSG, pgs_smf_setunixmsg, \
	    INT, STRING,STRING)

/* PGS_SMF_TestErrorLevel() */

FCALLSCFUN1(INT, PGS_SMF_TestErrorLevel, PGS_SMF_TESTERRORLEVEL, \
	    pgs_smf_testerrorlevel, INT)

/* PGS_SMF_TestFatalLevel() */

FCALLSCFUN1(INT, PGS_SMF_TestFatalLevel, PGS_SMF_TESTFATALLEVEL,\
	    pgs_smf_testfatallevel, INT)

/* PGS_SMF_TestMessageLevel() */

FCALLSCFUN1(INT, PGS_SMF_TestMessageLevel, PGS_SMF_TESTMESSAGELEVEL, \
	    pgs_smf_testmessagelevel, INT)

/* PGS_SMF_TestNoticeLevel() */

FCALLSCFUN1(INT, PGS_SMF_TestNoticeLevel, PGS_SMF_TESTNOTICELEVEL, \
	    pgs_smf_testnoticelevel, INT)

/* PGS_SMF_TestStatusLevel() */

FCALLSCFUN1(INT, PGS_SMF_TestStatusLevel, PGS_SMF_TESTSTATUSLEVEL, \
	    pgs_smf_teststatuslevel, INT)

/* PGS_SMF_TestSuccessLevel() */

FCALLSCFUN1(INT, PGS_SMF_TestSuccessLevel, PGS_SMF_TESTSUCCESSLEVEL, \
	    pgs_smf_testsuccesslevel, INT)

/* PGS_SMF_TestUserInfoLevel() */

FCALLSCFUN1(INT, PGS_SMF_TestUserInfoLevel, PGS_SMF_TESTUSERINFOLEVEL, \
	    pgs_smf_testuserinfolevel, INT)

/* PGS_SMF_TestWarningLevel() */

FCALLSCFUN1(INT, PGS_SMF_TestWarningLevel, PGS_SMF_TESTWARNINGLEVEL, \
	    pgs_smf_testwarninglevel, INT)
