/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupSMF.c

DESCRIPTION:
	This file contains the following functions 

	PGS_TSF_ThreadInitSMF()
	PGS_TSF_ThreadEndSMF()

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	26-May-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>
#include <stdlib.h>
#include <string.h>

#ifdef _PGS_THREADSAFE
PGSt_SMF_boolean PGSg_TSF_SMFFlag[_POSIX_THREAD_THREADS_MAX];
PGSt_SMF_status PGSg_TSF_SMFCallerID[_POSIX_THREAD_THREADS_MAX];
PGSt_SMF_code PGSg_TSF_SMFLastCode[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_SMFRepMsgCount[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the SMF portion of the thread-safe version of the
	PGS Toolkit.
 
NAME:  
	PGS_TSF_ThreadInitSMF()

SYNOPSIS:

C:
		void 
		PGS_TSF_ThreadInitSMF(
			PGSt_TSF_MasterStruct *threadMaster,
			int index)

FORTRAN:
		N/A

DESCRIPTION:
 
INPUTS:
	Name		Description			Units	Min	Max

	threadMaster	Global structure used to store
			locks and keys

	index		master thread index

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	N/A

EXAMPLES:

C:
	PGSt_TSF_MasterStruct *masterStruct;
	int index;
		.
		.
		.
	PGS_TSF_ThreadInitSMF(masterStruct, index);
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
	PGSg_TSF_SMFFlag
	PGSg_TSF_SMFCallerID
	PGSg_TSF_SMFLastCode
	PGSg_TSF_SMFRepMsgCount

FILES:
	N/A
 
FUNCTIONS_CALLED:

END_PROLOG:
***************************************************************************/

void PGS_TSF_ThreadInitSMF(PGSt_TSF_MasterStruct *threadMaster,int index)
{
#ifdef _PGS_THREADSAFE
    static int threadID = 100;
    char *temChar;

/***************************************************************************
  Create a mock thread id that will be printed in the SMF reports.
***************************************************************************/
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYSMFTHREADID],
                        (void *) threadID);
    threadID += 100;

/***************************************************************************
  Determine if SMF has been used in this thread (important), and init
  the caller id.
***************************************************************************/
    PGSg_TSF_SMFFlag[index] = PGS_FALSE;
    PGSg_TSF_SMFCallerID[index] = PGSd_CALLERID_USR;

/***************************************************************************
  Initialize remaining globals and TSDs as they are in existing SMF code.
***************************************************************************/
    temChar = (char *) malloc(sizeof(char) * PGS_SMF_MAX_FUNC_SIZE);
    temChar[0] = 0;
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYSMFLASTFUNC],temChar);

    temChar = (char *) malloc(sizeof(char) * PGS_SMF_MAX_MNEMONIC_SIZE);
    temChar[0] = 0;
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYSMFLASTMNE],temChar);

    temChar = (char *) malloc(sizeof(char) * PGS_SMF_MAX_MSGBUF_SIZE);
    temChar[0] = 0;
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYSMFLASTMSG],temChar);

    PGSg_TSF_SMFLastCode[index] = 0;
    PGSg_TSF_SMFRepMsgCount[index] = 0;

#endif
}



/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	De-initializes the SMF portion of the thread-safe version of the
	PGS Toolkit.
 
NAME:  
	PGS_TSF_ThreadEndSMF()

SYNOPSIS:

C:
		void 
		PGS_TSF_ThreadEndSMF(
			PGSt_TSF_MasterStruct *threadMaster)

FORTRAN:
		N/A

DESCRIPTION:
 
INPUTS:
	Name		Description			Units	Min	Max

	threadMaster	Global structure used to store
			locks and keys

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	N/A

EXAMPLES:

C:
	PGSt_TSF_MasterStruct *masterStruct;
		.
		.
		.
	PGS_TSF_ThreadEndSMF(masterStruct);
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
void PGS_TSF_ThreadEndSMF(PGSt_TSF_MasterStruct *masterTSF)
{
#ifdef _PGS_THREADSAFE
    char *temChar;

/***************************************************************************
  Release allocated memory.
***************************************************************************/
    temChar = (char *) pthread_getspecific(
                     masterTSF->keyArray[PGSd_TSF_KEYSMFLASTFUNC]);
    free(temChar);
    temChar = (char *) pthread_getspecific(
                     masterTSF->keyArray[PGSd_TSF_KEYSMFLASTMNE]);
    free(temChar);
    temChar = (char *) pthread_getspecific(
                     masterTSF->keyArray[PGSd_TSF_KEYSMFLASTMSG]);
    free(temChar);
#endif
    return;
}
