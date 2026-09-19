/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupAA.c

DESCRIPTION:
	This file contains the function PGS_TSF_SetupAA().

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	26-May-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>
#include <stdlib.h>
#include <string.h>
#undef DEFINE_DATA
#include <freeform.h>
#include <databin.h>
#include <PGS_AA_Tools.h>

#ifdef _PGS_THREADSAFE
PGSt_uinteger PGSg_TSF_AApevSavedLogicals[_POSIX_THREAD_THREADS_MAX][PGSd_AA_PEVMAXFILES];
DATA_BIN_PTR PGSg_TSF_AApevSavedDbins[_POSIX_THREAD_THREADS_MAX][PGSd_AA_PEVMAXFILES];
PGSt_integer PGSg_TSF_AApevNFiles[_POSIX_THREAD_THREADS_MAX];
short PGSg_TSF_AAnoOfIds[_POSIX_THREAD_THREADS_MAX];
short PGSg_TSF_AAnoOfNewIds[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the thread-safe version of the PGS Toolkit.
 
NAME:  
	PGS_TSF_SetupAA()

SYNOPSIS:

C:
		PGSt_SMF_status
		PGS_TSF_SetupAA(PGSt_TSF_MasterStruct *threadMaster);

FORTRAN:
		N/A

DESCRIPTION:
	This tool needs to be called before ANY other toolkit functions
	if the Toolkit is to be run in a multi-threaded environment.
 
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
	PGSt_TSF_MasterStruct	threadSafeMaster;
		.
		.
		.
	returnStatus = PGS_TSF_SetupAA(&threadSafeMaster);
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

void PGS_TSF_ThreadInitAA(PGSt_TSF_MasterStruct *threadMaster,int index)
{
#ifdef _PGS_THREADSAFE
    DATA_BIN_PTR *temDATA_BIN_PTR;
    char *temchar;
 
    int loopVar;

/***************************************************************************
  Initialize globals.
***************************************************************************/
    PGSg_TSF_AApevNFiles[index] = 0;
    PGSg_TSF_AAnoOfIds[index] = 0;
    PGSg_TSF_AAnoOfNewIds[index] = 0;

    for (loopVar = 0; loopVar < PGSd_AA_PEVMAXFILES; loopVar++)
    {
          PGSg_TSF_AApevSavedLogicals[index][loopVar] = 0;
          PGSg_TSF_AApevSavedDbins[index][loopVar] = '\0';
    }

/***************************************************************************
  Allocate space and initialize TSDs.
***************************************************************************/
    temchar = (char *) malloc(sizeof(char) * PGSd_AA_MAXNOCACHES);
    for (loopVar = 0; loopVar < PGSd_AA_MAXNOCACHES; loopVar++)
    {
          temchar[loopVar] = '\0';
    } 
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYAAOLDFILES],temchar);

    temDATA_BIN_PTR = (DATA_BIN_PTR *) malloc(sizeof(DATA_BIN_PTR) * PGSd_AA_MAXNOCACHES);
    for (loopVar = 0; loopVar < PGSd_AA_MAXNOCACHES; loopVar++)
    {
          temDATA_BIN_PTR[loopVar] = '\0';
    } 
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYAAPHYSFILEBIN],
                              temDATA_BIN_PTR);

#endif
}


void PGS_TSF_ThreadEndAA(PGSt_TSF_MasterStruct *masterTSF)
{
#ifdef _PGS_THREADSAFE
    DATA_BIN_PTR *temDATA_BIN_PTR;
    char *temchar;

/***************************************************************************
  Free TSDs.
***************************************************************************/
    temchar = (char *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYAAOLDFILES]);
    free(temchar);

    temDATA_BIN_PTR = (DATA_BIN_PTR *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYAAPHYSFILEBIN]);
    free(temDATA_BIN_PTR);
  
#endif
    return;
}

