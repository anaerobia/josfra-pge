/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupCSC.c

DESCRIPTION:
	This file contains the function PGS_TSF_SetupCSC().

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	09-Apr-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>
#include <PGS_CSC.h>
#include <stdlib.h>
#include <string.h>

#ifdef _PGS_THREADSAFE
/**************************************************************************
      GLOBALS  DECLARED  & LISTED BY  FUNCTION
***************************************************************************/
PGSt_integer    PGSg_TSF_CSCQuatpsi[_POSIX_THREAD_THREADS_MAX];
PGSt_integer    PGSg_TSF_CSCQuatiIndex[_POSIX_THREAD_THREADS_MAX];
PGSt_integer    PGSg_TSF_CSCQuatjIndex[_POSIX_THREAD_THREADS_MAX];
PGSt_integer    PGSg_TSF_CSCQuatkIndex[_POSIX_THREAD_THREADS_MAX];
PGSt_integer    PGSg_TSF_CSCQuatoldEAOrder[_POSIX_THREAD_THREADS_MAX][3];
PGSt_boolean    PGSg_TSF_CSCQuatindicesDiff[_POSIX_THREAD_THREADS_MAX];

PGSt_integer    PGSg_TSF_CSCEuleriIndex[_POSIX_THREAD_THREADS_MAX];
PGSt_integer    PGSg_TSF_CSCEulerjIndex[_POSIX_THREAD_THREADS_MAX];
PGSt_integer    PGSg_TSF_CSCEulerkIndex[_POSIX_THREAD_THREADS_MAX];
PGSt_integer    PGSg_TSF_CSCEuleralpha[_POSIX_THREAD_THREADS_MAX];
PGSt_integer    PGSg_TSF_CSCEuleroldEulerAngleOrder[_POSIX_THREAD_THREADS_MAX][3];
PGSt_boolean    PGSg_TSF_CSCEulerindicesDiff[_POSIX_THREAD_THREADS_MAX];

PGSt_double     PGSg_TSF_CSCQuickWahroldsecTAI93[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCQuickWahrdvnutOld[_POSIX_THREAD_THREADS_MAX][4];
PGSt_double     PGSg_TSF_CSCQuickWahrjedTDBold[_POSIX_THREAD_THREADS_MAX][2];

PGSt_SMF_status PGSg_TSF_CSCGetEarthFigurereturnStatus[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCGetEarthFigurelastEquatRad[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCGetEarthFigurelastPolarRad[_POSIX_THREAD_THREADS_MAX];

PGSt_SMF_status PGSg_TSF_CSCECItoECRsetupStatus[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCECItoECRlastTAI93[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCECItoECRjedTDB[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCECItoECRjedTDT[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCECItoECRdvnut[_POSIX_THREAD_THREADS_MAX][4];
PGSt_double     PGSg_TSF_CSCECItoECRearthrot[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCECItoECRxpol[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCECItoECRypol[_POSIX_THREAD_THREADS_MAX][2];
char            PGSg_TSF_CSCECItoECRsetup_msg[_POSIX_THREAD_THREADS_MAX][PGS_SMF_MAX_MSG_SIZE];

PGSt_SMF_status PGSg_TSF_CSCECRtoECIsetupStatus[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCECRtoECIlastTAI93[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCECRtoECIjedTDB[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCECRtoECIjedTDT[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCECRtoECIdvnut[_POSIX_THREAD_THREADS_MAX][4];
PGSt_double     PGSg_TSF_CSCECRtoECIearthrot[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCECRtoECIxpol[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCECRtoECIypol[_POSIX_THREAD_THREADS_MAX][2];

PGSt_SMF_status PGSg_TSF_CSCECRtoGEOmodelStatus[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCECRtoGEOequatRad_A[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCECRtoGEOpolarRad_C[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCECRtoGEOellipsEccSq[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCECRtoGEOflatFac[_POSIX_THREAD_THREADS_MAX];

PGSt_double     PGSg_TSF_CSCNutateold_et[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCNutateobm[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCNutateobt[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCNutatedpsi[_POSIX_THREAD_THREADS_MAX][2];

PGSt_double     PGSg_TSF_CSCPrecsold_jedTDT[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCPrecsz[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCPrecsze[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double     PGSg_TSF_CSCPrecsth[_POSIX_THREAD_THREADS_MAX][2];

PGSt_SMF_status PGSg_TSF_CSCGEOtoECRmodelStatus[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCGEOtoECRequatRad_A[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCGEOtoECRpolarRad_C[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCGEOtoECRflatFac[_POSIX_THREAD_THREADS_MAX];

float           PGSg_TSF_CSCUTC_UT1Polexpoles[_POSIX_THREAD_THREADS_MAX][MAX_RECS];
float           PGSg_TSF_CSCUTC_UT1Poleypoles[_POSIX_THREAD_THREADS_MAX][MAX_RECS];
float           PGSg_TSF_CSCUTC_UT1Poleut1utcTab[_POSIX_THREAD_THREADS_MAX][MAX_RECS];
PGSt_double     PGSg_TSF_CSCUTC_UT1Polejd[_POSIX_THREAD_THREADS_MAX][MAX_RECS];
PGSt_double     PGSg_TSF_CSCUTC_UT1Polejd0[_POSIX_THREAD_THREADS_MAX];
PGSt_double     PGSg_TSF_CSCUTC_UT1Polejdlast[_POSIX_THREAD_THREADS_MAX];
int             PGSg_TSF_CSCUTC_UT1Polecnt[_POSIX_THREAD_THREADS_MAX];
int             PGSg_TSF_CSCUTC_UT1Polenumrecs[_POSIX_THREAD_THREADS_MAX];
PGSt_boolean    PGSg_TSF_CSCUTC_UT1Poletablerd[_POSIX_THREAD_THREADS_MAX];
PGSt_boolean    PGSg_TSF_CSCUTC_UT1Polefilenotfound[_POSIX_THREAD_THREADS_MAX];

PGSt_boolean    PGSg_TSF_CSCWahr2first[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the CSC portion of the PGS Toolkit.
 
NAME:  
	PGS_TSF_ThreadInitCSC()

SYNOPSIS:

C:
		PGSt_SMF_status
		PGS_TSF_SetupCSC(PGSt_TSF_MasterStruct **threadMaster);

FORTRAN:
		N/A

DESCRIPTION:
	This tool is called during the THREAD initialization phase
        of the thread-safe version of the PGS Toolkit.
 
INPUTS:
	Name		Description			Units	Min	Max

	NONE

OUTPUTS:
	Name		Description			Units	Min	Max

	threadMaster	Master variable containing thread-
			eafe essentials (i.e. locks).

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
	PGSt_SMF_Status	returnStaus;
	PGSt_TSF_MasterStruct	*threadSafeMaster;
		.
		.
		.
	returnStatus = PGS_TSF_SetupCSC(&threadSafeMaster);
	if (returnStatus != PGS_S_SUCCESS)
	{
		goto EXCEPTION
	}
		.
		.
		.
EXCEPTION:
	return returnStatus;

FORTRAN:
	N/A

NOTES:

REQUIREMENTS:

DETAILS:
	NONE

GLOBALS:
	Accesses global mutexes in PGS_TSF.h

FILES:
	N/A
 
FUNCTIONS_CALLED:

END_PROLOG:
***************************************************************************/

void PGS_TSF_ThreadInitCSC(
    PGSt_TSF_MasterStruct *threadMaster,
    int index)
{
#ifdef _PGS_THREADSAFE
    char             *cscGetEarthFigureoldEarthTag;
    char             *cscGEOtoECRoldEarthTag;
    char             *cscECRtoGEOoldEarthTag;
    char             *cscECRtoECIsetup_msg;
    char             *cscUTC_UT1Poleaccuracy;
    int               x;
/***********************************************************
      GLOBALS  INITIALIZED  &  LISTED BY FUNCTION

****************  QuickWahr  ******************************/ 
    PGSg_TSF_CSCQuickWahroldsecTAI93[index] = -1.e50;
   for (x=0; x<2; x++)
   {
       PGSg_TSF_CSCQuickWahrdvnutOld[index][x] = 0.0;
       PGSg_TSF_CSCQuickWahrjedTDBold[index][x] = 0.0;
   }
/***************     QuatToEuler   *********************************/
    PGSg_TSF_CSCQuatpsi[index] = 0;
    PGSg_TSF_CSCQuatiIndex[index] = 0;
    PGSg_TSF_CSCQuatjIndex[index] = 0;
    PGSg_TSF_CSCQuatkIndex[index] = 0;
    PGSg_TSF_CSCQuatindicesDiff[index] = 0;
   for (x=0; x<3; x++)
   {
       PGSg_TSF_CSCQuatoldEAOrder[index][x] = 0;
   }
/***************     Euler   *********************************/
    PGSg_TSF_CSCEuleriIndex[index] = 0;
    PGSg_TSF_CSCEulerjIndex[index] = 0;
    PGSg_TSF_CSCEulerkIndex[index] = 0;
    PGSg_TSF_CSCEuleralpha[index] = 0;
    PGSg_TSF_CSCEulerindicesDiff[index] = 0;
   for (x=0; x<3; x++)
   {
       PGSg_TSF_CSCEuleroldEulerAngleOrder[index][x] = 0;
   }
/***************   ECItoECR   *******************************/
    PGSg_TSF_CSCECItoECRsetup_msg[index][0] = '\0';
    PGSg_TSF_CSCECItoECRlastTAI93[index] = 1.e50;
    PGSg_TSF_CSCECItoECRsetupStatus[index] = 0;
   for (x=0; x<4; x++)
       PGSg_TSF_CSCECItoECRdvnut[index][x] = 0.0;
   for (x=0; x<2; x++)
   {
       PGSg_TSF_CSCECItoECRjedTDB[index][x] = 0.0;
       PGSg_TSF_CSCECItoECRjedTDT[index][x] = 0.0;
       PGSg_TSF_CSCECItoECRearthrot[index][x] = 0.0;
       PGSg_TSF_CSCECItoECRxpol[index][x] = 0.0;
       PGSg_TSF_CSCECItoECRypol[index][x] = 0.0;
   }
/****************   GetEarthFigure     ****************************/
    PGSg_TSF_CSCGetEarthFigurereturnStatus[index] = PGS_S_SUCCESS;
    PGSg_TSF_CSCGetEarthFigurelastPolarRad[index] = PGSd_DEFAULT_POLAR_RAD;
    PGSg_TSF_CSCGetEarthFigurelastEquatRad[index] = PGSd_DEFAULT_EQUAT_RAD;
/****************     ECRtoECI    *********************************/
    PGSg_TSF_CSCECRtoECIsetupStatus[index] = 0;
    PGSg_TSF_CSCECRtoECIlastTAI93[index] = 1.e50;
   for (x=0; x<4; x++)
       PGSg_TSF_CSCECItoECRdvnut[index][x] = 0.0;
   for (x=0; x<2; x++)
   {
       PGSg_TSF_CSCECRtoECIjedTDB[index][x] = 0.0;
       PGSg_TSF_CSCECRtoECIjedTDT[index][x] = 0.0;
       PGSg_TSF_CSCECRtoECIearthrot[index][x] = 0.0;
       PGSg_TSF_CSCECRtoECIxpol[index][x] = 0.0;
       PGSg_TSF_CSCECRtoECIypol[index][x] = 0.0;
   }
/***************   ECRtoGEO        *****************************/
    PGSg_TSF_CSCECRtoGEOmodelStatus[index] = PGS_S_SUCCESS;
    PGSg_TSF_CSCECRtoGEOequatRad_A[index] = 6378137.0;
    PGSg_TSF_CSCECRtoGEOpolarRad_C[index] = 6356752.314245;
    PGSg_TSF_CSCECRtoGEOellipsEccSq[index] = 6.69438002301199714E-3;
    PGSg_TSF_CSCECRtoGEOflatFac[index] = 3.35281068123811075E-3;
/***************   GEOto ECR       *****************************/
    PGSg_TSF_CSCGEOtoECRmodelStatus[index] = PGS_S_SUCCESS;
    PGSg_TSF_CSCGEOtoECRequatRad_A[index] = 6378137.0;
    PGSg_TSF_CSCGEOtoECRpolarRad_C[index] = 6356752.314245;
    PGSg_TSF_CSCGEOtoECRflatFac[index] = 3.35281068123811075E-3;
/***************  UTC_UT1Pole   **********************************/
  for(x=0; x<MAX_RECS;x++)
  {
    PGSg_TSF_CSCUTC_UT1Polexpoles[index][x] = 0.0;
    PGSg_TSF_CSCUTC_UT1Poleypoles[index][x] = 0.0;
    PGSg_TSF_CSCUTC_UT1Poleut1utcTab[index][x] = 0.0;
    PGSg_TSF_CSCUTC_UT1Polejd[index][x] = 0.0;
  }
    PGSg_TSF_CSCUTC_UT1Polejd0[index] = 0.0;
    PGSg_TSF_CSCUTC_UT1Polejdlast[index] = 0.0;
    PGSg_TSF_CSCUTC_UT1Polecnt[index] = 0;
    PGSg_TSF_CSCUTC_UT1Polenumrecs[index] = 0;
    PGSg_TSF_CSCUTC_UT1Poletablerd[index] = PGS_FALSE;
    PGSg_TSF_CSCUTC_UT1Polefilenotfound[index] = PGS_FALSE;
/*****************    NUTATE    **********************************/
    PGSg_TSF_CSCNutateold_et[index] = -1.E50;
  for(x=0; x<2;x++)
  {
     PGSg_TSF_CSCNutateobm[index][x] = 0.0;
     PGSg_TSF_CSCNutateobm[index][x] = 0.0;
     PGSg_TSF_CSCNutateobm[index][x] = 0.0;
  }
/*****************     PRECS     **********************************/
    PGSg_TSF_CSCPrecsold_jedTDT[index] = -1.E50;
  for(x=0; x<2;x++)
  {
     PGSg_TSF_CSCPrecsz[index][x] = 0.0;
     PGSg_TSF_CSCPrecsze[index][x] = 0.0;
     PGSg_TSF_CSCPrecsth[index][x] = 0.0;
  }
/*****************    quickWahr      **********************************/
    PGSg_TSF_CSCWahr2first[index] = PGS_TRUE;

/**************************************************************** 
                 ALLOCATE SPACE FOR KEYS   
******************************************************************/
cscUTC_UT1Poleaccuracy = (char *) malloc(sizeof(char) * MAX_RECS);
           cscUTC_UT1Poleaccuracy[0] = '\0';
pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYCSCUTC_UT1POLEACCURACY],
                               (void *) cscUTC_UT1Poleaccuracy);

    cscGetEarthFigureoldEarthTag = (char *) malloc(sizeof(char) * 50);
    strcpy(cscGetEarthFigureoldEarthTag, PGSd_DEFAULT_EARTH_TAG);
pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYCSCGETEARTHFIGUREOLDEARTHTAG],
                               (void *) cscGetEarthFigureoldEarthTag);

    cscGEOtoECRoldEarthTag = (char *) malloc(sizeof(char) * 50);
    strcpy(cscGEOtoECRoldEarthTag, PGSd_DEFAULT_EARTH_TAG);
   pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYCSCGEOTOECROLDEARTHTAG],
                               (void *) cscGEOtoECRoldEarthTag);

    cscECRtoGEOoldEarthTag = (char *) malloc(sizeof(char) * 50);
    strcpy(cscECRtoGEOoldEarthTag, PGSd_DEFAULT_EARTH_TAG);
   pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYCSCECRTOGEOOLDEARTHTAG],
                               (void *) cscECRtoGEOoldEarthTag);

    cscECRtoECIsetup_msg = (char *) malloc(sizeof(char) * PGS_SMF_MAX_MSG_SIZE);
           cscECRtoECIsetup_msg[0] = '\0';
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYCSCECRTOECISETUP_MSG],
                               (void *) cscECRtoECIsetup_msg);

#endif
    return;
}
/***************************************************************************
BEGIN_PROLOG:
 
TITLE:
        De-initializes the CSC portion of the threadsafe PGS Toolkit.
 
NAME:
        PGS_TSF_ThreadEndCSC()
 
SYNOPSIS:
 
C:
                void
                PGS_TSF_ThreadEndCSC(
                        PGSt_TSF_MasterStruct *threadMaster)
 
FORTRAN:
                N/A
 
DESCRIPTION:
        This tool is called during the THREAD exit phase of
        the thread-safe version of the PGS Toolkit.
 
INPUTS:
        Name            Description                     Units   Min     Max
 
        threadMaster    master structure for TSF
 
OUTPUTS:
        Name            Description                     Units   Min     Max
 
        NONE
 
RETURNS:
        NONE
 
EXAMPLES:
 
C:
        PGSt_TSF_MasterStruct   *threadSafeMaster;
                .
                .
                .
        PGS_TSF_ThreadEndCSC(threadSafeMaster,index);
                .
                .
                .
 
FORTRAN:
        N/A
 
NOTES:
 
REQUIREMENTS:
 
DETAILS:
        NONE
 
GLOBALS:
        NONE
 
FILES:
        N/A
 
FUNCTIONS_CALLED:
 
END_PROLOG:
***************************************************************************/
void PGS_TSF_ThreadEndCSC(PGSt_TSF_MasterStruct *masterTSF)
{
#ifdef _PGS_THREADSAFE
    char             *cscGetEarthFigureoldEarthTag;
    char             *cscGEOtoECRoldEarthTag;
    char             *cscECRtoGEOoldEarthTag;
    char             *cscECRtoECIsetup_msg;
    char             *cscUTC_UT1Poleaccuracy;
/***************************************************************************
  Free all space allocated in the init function.
***************************************************************************/
    cscGetEarthFigureoldEarthTag = (char *) pthread_getspecific(
          masterTSF->keyArray[PGSd_TSF_KEYCSCGETEARTHFIGUREOLDEARTHTAG]);   
    free(cscGetEarthFigureoldEarthTag);
    cscGEOtoECRoldEarthTag = (char *) pthread_getspecific(
          masterTSF->keyArray[PGSd_TSF_KEYCSCGEOTOECROLDEARTHTAG]);
    free(cscGEOtoECRoldEarthTag);
    cscECRtoGEOoldEarthTag = (char *) pthread_getspecific(
	  masterTSF->keyArray[PGSd_TSF_KEYCSCECRTOGEOOLDEARTHTAG]);
    free(cscECRtoGEOoldEarthTag);
    cscECRtoECIsetup_msg = (char *) pthread_getspecific(
          masterTSF->keyArray[PGSd_TSF_KEYCSCECRTOECISETUP_MSG]);
    free(cscECRtoECIsetup_msg);
    cscUTC_UT1Poleaccuracy = (char *) pthread_getspecific(
          masterTSF->keyArray[PGSd_TSF_KEYCSCUTC_UT1POLEACCURACY]);
    free(cscUTC_UT1Poleaccuracy);

#endif
    return;
}
