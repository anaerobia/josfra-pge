/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupEPH.c

DESCRIPTION:
	This file contains the function PGS_TSF_SetupEPH().

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	26-May-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>
#include <stdlib.h>
#include <string.h>
#include <PGS_EPH.h>

#ifdef _PGS_THREADSAFE
PGSt_uinteger PGSg_TSF_EPHqFlagsMasks[_POSIX_THREAD_THREADS_MAX][2];
PGSt_boolean PGSg_TSF_EPHfirst[_POSIX_THREAD_THREADS_MAX];
PGSt_tag PGSg_TSF_EPHoldSpacecraftTag[_POSIX_THREAD_THREADS_MAX];
PGSt_tag PGSg_TSF_EPHattitoldSpacecraftTag[_POSIX_THREAD_THREADS_MAX];
PGSt_tag PGSg_TSF_EPHheadersoldSpacecraftTag[_POSIX_THREAD_THREADS_MAX];
long PGSg_TSF_EPHnextRecordPtr[_POSIX_THREAD_THREADS_MAX];
long PGSg_TSF_EPHattitnextRecordPtr[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHlastArraySize[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHattitlastArraySize[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHfile_num[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHattitfile_num[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHnum_files[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHattitnum_files[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHattitloopcount[_POSIX_THREAD_THREADS_MAX];
PGSt_boolean PGSg_TSF_EPHtableread[_POSIX_THREAD_THREADS_MAX];
size_t PGSg_TSF_EPHnumArrayElements[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHtotalEphemRecords[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHtotalAttitRecords[_POSIX_THREAD_THREADS_MAX];
PGSt_double PGSg_TSF_EPHbeginTAI93eph[_POSIX_THREAD_THREADS_MAX];
PGSt_double PGSg_TSF_EPHendTAI93eph[_POSIX_THREAD_THREADS_MAX];
PGSt_double PGSg_TSF_EPHbeginTAI93att[_POSIX_THREAD_THREADS_MAX];
PGSt_double PGSg_TSF_EPHendTAI93att[_POSIX_THREAD_THREADS_MAX];
PGSt_tag PGSg_TSF_EPHspacecraft[_POSIX_THREAD_THREADS_MAX];
PGSt_tag PGSg_TSF_EPHlendcheck_old[_POSIX_THREAD_THREADS_MAX];
PGSt_tag PGSg_TSF_EPHhdfcheck[_POSIX_THREAD_THREADS_MAX];
PGSt_tag PGSg_TSF_EPHanum[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_EPHheadersnum_files[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the thread-safe version of the PGS Toolkit.
 
NAME:  
	PGS_TSF_SetupEPH()

SYNOPSIS:

C:
		PGSt_SMF_status
		PGS_TSF_SetupEPH(PGSt_TSF_MasterStruct *threadMaster);

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
	returnStatus = PGS_TSF_SetupEPH(&threadSafeMaster);
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

#define BLOCK_SIZE  6000

void PGS_TSF_ThreadInitEPH(PGSt_TSF_MasterStruct *threadMaster,int index)
{
#ifdef _PGS_THREADSAFE
    PGSt_hdrSummary *temSummary;
    int loopVar;
    PGSt_scTagInfo *temscTagInfo;
    PGSt_ephemRecord *temPGSt_ephemRecord;
    PGSt_attitRecord *temPGSt_attitRecord;

    temSummary = NULL;
    temscTagInfo = NULL;

/***************************************************************************
  Initialize globals.
***************************************************************************/
    PGSg_TSF_EPHfirst[index] = PGS_TRUE;
    PGSg_TSF_EPHoldSpacecraftTag[index] = 0;
    PGSg_TSF_EPHattitoldSpacecraftTag[index] = 0;
    PGSg_TSF_EPHheadersoldSpacecraftTag[index] = 0;
    PGSg_TSF_EPHnextRecordPtr[index] = 0;
    PGSg_TSF_EPHattitnextRecordPtr[index] = 0;
    PGSg_TSF_EPHlastArraySize[index] = -1;
    PGSg_TSF_EPHattitlastArraySize[index] = -1;
    PGSg_TSF_EPHfile_num[index] = -1;
    PGSg_TSF_EPHattitfile_num[index] = -1;
    PGSg_TSF_EPHattitloopcount[index] = 0;
    PGSg_TSF_EPHnum_files[index] = -1;
    PGSg_TSF_EPHheadersnum_files[index] = -1;  
    PGSg_TSF_EPHattitnum_files[index] = -1;  
    PGSg_TSF_EPHtableread[index] = PGS_FALSE;
    PGSg_TSF_EPHnumArrayElements[index] = 0U;
    PGSg_TSF_EPHtotalEphemRecords[index] = 0;
    PGSg_TSF_EPHtotalAttitRecords[index] = 0;
    PGSg_TSF_EPHbeginTAI93eph[index] = 1.0e50;
    PGSg_TSF_EPHendTAI93eph[index] = -1.0e50;
    PGSg_TSF_EPHbeginTAI93att[index] = 1.0e50;
    PGSg_TSF_EPHendTAI93att[index] = -1.0e50;
    PGSg_TSF_EPHspacecraft[index] = 0;
    PGSg_TSF_EPHlendcheck_old[index] = -1;
    PGSg_TSF_EPHhdfcheck[index] = 0;
    PGSg_TSF_EPHanum[index] = 0;

    for (loopVar = 0; loopVar < 2; loopVar++)
    {
        PGSg_TSF_EPHqFlagsMasks[index][loopVar] = 0;
    }

/***************************************************************************
  Allocate space and initialize TSDs.
***************************************************************************/
    temPGSt_ephemRecord = (PGSt_ephemRecord *) malloc(sizeof(
                                  PGSt_ephemRecord) * BLOCK_SIZE);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYEPHEPHEMRECORDARRAY],
                                  temPGSt_ephemRecord);

    temPGSt_attitRecord = (PGSt_attitRecord *) malloc(sizeof
                                  (PGSt_attitRecord) * BLOCK_SIZE);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYEPHATTITRECORDARRAY],
                                  temPGSt_attitRecord);

    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYEPHHEADER_ARRAY],
                      temSummary);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYEPHATTITHEADER_ARRAY],
                      temSummary);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYEPHSCINFOARRAY],
                      temscTagInfo);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYEPHSTATICHDRARRAY],
                      temSummary);
#endif
}


void PGS_TSF_ThreadEndEPH(PGSt_TSF_MasterStruct *masterTSF)
{
#ifdef _PGS_THREADSAFE
    PGSt_ephemRecord *temPGSt_ephemRecord;
    PGSt_attitRecord *temPGSt_attitRecord;

/***************************************************************************
  Free TSDs.
***************************************************************************/
    temPGSt_ephemRecord = (PGSt_ephemRecord *) pthread_getspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYEPHEPHEMRECORDARRAY]);
    free(temPGSt_ephemRecord);

    temPGSt_attitRecord = (PGSt_attitRecord *) pthread_getspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYEPHATTITRECORDARRAY]);
    free(temPGSt_attitRecord);

#endif
    return;
}




