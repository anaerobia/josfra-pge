/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_GetTSFMaster.c

DESCRIPTION:
	This file contains the following functions: 

	PGS_TSF_GetTSFMaster().
	PGS_TSF_ThreadInit().
	PGS_TSF_ThreadEnd().
	PGS_TSF_GetMasterIndex().

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	09-Apr-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>
#include <PGS_DEM.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the thread-safe version of the PGS Toolkit.
 
NAME:  
	PGS_TSF_GetTSFMaster()

SYNOPSIS:

C:
		PGSt_SMF_status
		PGS_TSF_GetTSFMaster(PGSt_TSF_MasterStruct **threadMaster);

FORTRAN:
		N/A

DESCRIPTION:
	This tool sets up the master structure containing thread-safe
	data necessary for the PGS Toolkit.  On the first call to
	this tool all necessary initializations are performed.
 
INPUTS:
	Name		Description			Units	Min	Max

	NONE

OUTPUTS:
	Name		Description			Units	Min	Max

	threadMaster	Master variable containing thread-
			eafe essentials (i.e. locks).

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSTSF_E_NOT_THREADSAFE     toolkit not in threadsafe mode
        PGSTSF_E_MUTEXLOCK_FAIL     mutex lock failure
        PGSTSF_E_MUTEXINIT_FAIL     mutex initialization failure
	PGSTSF_E_KEYCREATE_FAIL     TSD key create failure
        PGSTSF_W_MUTEXUNLOCK_FAIL   mutex unlock failure

EXAMPLES:

C:
	PGSt_SMF_Status	returnStaus;
	PGSt_TSF_MasterStruct	*threadSafeMaster;
		.
		.
		.
	returnStatus = PGS_TSF_GetTSFMaster(&threadSafeMaster);
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
	All keys and locks are stored in the master structure so as to
	make everything transparent to the user.  Calling the thread-safe
	version of the PGS Toolkit will be exactly like calling the 
	non-threadsafe version of the PGS Toolkit.  THIS WAS NOT A 
	REQUIREMENT - but how it was designed.

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

#ifdef _PGS_THREADSAFE
static PGSt_TSF_MasterStruct globalVar;        /* global vars */

static PGSt_boolean threadUsed[_POSIX_THREAD_THREADS_MAX];

static PGSt_SMF_boolean initFlag = PGS_FALSE;  /* initialization flag */
#endif


PGSt_SMF_status
PGS_TSF_GetTSFMaster(
    PGSt_TSF_MasterStruct **threadMaster)    /* get master tsf variable */
{
#ifdef _PGS_THREADSAFE
    int                 lockRet;             /* lock () return */
    int                 loopCount;
    PGSt_SMF_status     returnStatus;        /* function return */
    PGSt_SMF_status     saveStatus;
    PGSt_SMF_boolean    oneTime;

    static pthread_mutex_t getGlobalLock = PTHREAD_MUTEX_INITIALIZER;          
                                /* above variable is to ensure 1 init */

/***************************************************************************
  Initialize status.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    saveStatus = PGS_S_SUCCESS;

/***************************************************************************
  Setup a lock so we only go through the init phase once.
***************************************************************************/
    if (initFlag == PGS_FALSE)
    {
        lockRet = pthread_mutex_init(&getGlobalLock,NULL);
        if (lockRet != PGSd_TSF_LOCK_SUCCESS)
        {
            return PGSTSF_E_MUTEXINIT_FAIL;
        }
    }

/***************************************************************************
  Set init lock.
***************************************************************************/
    lockRet = pthread_mutex_lock(&getGlobalLock);
    if (lockRet != PGSd_TSF_LOCK_SUCCESS)
    {
        return PGSTSF_E_MUTEXLOCK_FAIL;
    }

/***************************************************************************
  Do init phase.
***************************************************************************/
    if (initFlag == PGS_FALSE)
    {
        initFlag = PGS_TRUE;

/***************************************************************************
  Create keys that are high level, keep track of thread init (as opposed
  to process init) and the master index.
***************************************************************************/
        lockRet = pthread_key_create(&globalVar.keyTSFonce,NULL);
        if (lockRet != PGSd_TSF_LOCK_SUCCESS)
        {
            saveStatus = PGSTSF_E_KEYCREATE_FAIL;
        }

        lockRet = pthread_key_create(&globalVar.keyTSFMasterIndex,PGS_TSF_ThreadEnd);
        if (lockRet != PGSd_TSF_LOCK_SUCCESS)
        {
            saveStatus = PGSTSF_E_KEYCREATE_FAIL;
        }

/***************************************************************************
  Setup threads used buffer to false.
***************************************************************************/
        for (loopCount = 0; loopCount < _POSIX_THREAD_THREADS_MAX; loopCount++)
        {
            threadUsed[loopCount] = PGS_FALSE;
        }

/***************************************************************************
  Initailize all locks necessary for the toolkit.
***************************************************************************/
        for (loopCount = 0; loopCount < PGSd_TSF_TOTAL_LOCKS; loopCount++)
        {
            lockRet = pthread_mutex_init(&globalVar.lockArray[loopCount],NULL);
            if (lockRet != PGSd_TSF_LOCK_SUCCESS)
            {
                saveStatus = PGSTSF_E_MUTEXINIT_FAIL;
                break;
            }
        }

/***************************************************************************
  If all is well at this point then create the remaining keys.
***************************************************************************/
        if (saveStatus == PGS_S_SUCCESS)
        {
            for (loopCount = 0; loopCount < PGSd_TSF_TOTAL_KEYS; loopCount++)
            {
                lockRet = pthread_key_create(&globalVar.keyArray[loopCount],NULL);
                if (lockRet != PGSd_TSF_LOCK_SUCCESS)
                {
                    saveStatus = PGSTSF_E_KEYCREATE_FAIL;
                    break;
                }
            }
        }
    }

/***************************************************************************
  Determin if this is the first time that this thread has called
  a toolkit function.
***************************************************************************/
    oneTime = (PGSt_SMF_boolean) pthread_getspecific(globalVar.keyTSFonce);
    if (oneTime == (PGSt_SMF_boolean) NULL)
    {
        PGS_TSF_ThreadInit();
    }

/***************************************************************************
  Unlock init phase.
***************************************************************************/
    lockRet = pthread_mutex_unlock(&getGlobalLock);
    if (lockRet != PGSd_TSF_LOCK_SUCCESS)
    {
        saveStatus = PGSTSF_W_MUTEXUNLOCK_FAIL;
    }

/***************************************************************************
  Set global var and return.
***************************************************************************/
    *threadMaster = &globalVar;

    if (saveStatus != PGS_S_SUCCESS)
    {
        returnStatus = saveStatus;
    }

    return returnStatus;
#else
    return PGSTSF_E_NOT_THREADSAFE;
#endif
}


/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes thread specific data (TSD) in the toolkit
 
NAME:  
	PGS_TSF_ThreadInit()

SYNOPSIS:

C:
		void
		PGS_TSF_ThreadInit();

FORTRAN:
		N/A

DESCRIPTION:
	This tool sets the values in the TSDs to their initial value.
 
INPUTS:
	Name		Description			Units	Min	Max

	NONE

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	N/A

EXAMPLES:

C:
		.
		.
		.
	PGS_TSF_ThreadInit();
		.
		.
		.

FORTRAN:
	N/A

NOTES:
	Each group has its own initialization function.  This was done
	more out of form than function.  Each group function just 
	initializes its own TSDs to the proper value.

REQUIREMENTS:

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	N/A
 
FUNCTIONS_CALLED:
	PGS_TSF_ThreadInitSMF()
	PGS_TSF_ThreadInitAA()
	PGS_TSF_ThreadInitCBP()
	PGS_TSF_ThreadInitCSC()
	PGS_TSF_ThreadInitDEM()
	PGS_TSF_ThreadInitIO()
	PGS_TSF_ThreadInitTD()
	PGS_TSF_ThreadInitEPH()
	PGS_TSF_ThreadInitMET()
	PGS_TSF_ThreadInitGCT()

END_PROLOG:
***************************************************************************/
void PGS_TSF_ThreadInit()
{
#ifdef _PGS_THREADSAFE
    int countVar;

/***************************************************************************
  Set value to let future calls to PGS_TSF_GetTSFMaster know that
  this thread has been initialized.
***************************************************************************/
    pthread_setspecific(globalVar.keyTSFonce,(void *) PGS_TRUE);

/***************************************************************************
  Look in threadUsed array for a false, the first false will be set to
  true and become the master index.  This master index will be used in 
  global arrays to limit the number of TSDs used.
***************************************************************************/
    countVar = 0;
    while (threadUsed[countVar] == PGS_TRUE)
    {
        countVar++;
    }
    threadUsed[countVar] = PGS_TRUE;
    pthread_setspecific(globalVar.keyTSFMasterIndex,(void *) countVar);

/***************************************************************************
  Initialize the remaining groups.
***************************************************************************/
    PGS_TSF_ThreadInitSMF(&globalVar,countVar);
    PGS_TSF_ThreadInitAA(&globalVar,countVar);
    PGS_TSF_ThreadInitCBP(&globalVar,countVar);
    PGS_TSF_ThreadInitCSC(&globalVar,countVar);
    PGS_TSF_ThreadInitDEM(&globalVar,countVar);
    PGS_TSF_ThreadInitIO(&globalVar,countVar);
    PGS_TSF_ThreadInitTD(&globalVar,countVar);
    PGS_TSF_ThreadInitEPH(&globalVar,countVar);
    PGS_TSF_ThreadInitMET(&globalVar,countVar);
    PGS_TSF_ThreadInitGCT(&globalVar,countVar);

#endif
    return;
}



/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	This function is called when the thread exits.  This is where
	we reset the threadUsed array back to false and free any 
	memory as necessary.
 
NAME:  
	PGS_TSF_ThreadEnd()

SYNOPSIS:

C:
		void
		PGS_TSF_ThreadEnd(void *tag);

FORTRAN:
		N/A

DESCRIPTION:
	This tool sets the values in the TSDs to their initial value.
 
INPUTS:
	Name		Description			Units	Min	Max

	NONE

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	N/A

EXAMPLES:

C:
	An example is not necessary since this function is loaded during 
	a key create.  It will automatically be called when the current
	thread exits.  In other words, users will not call this.

FORTRAN:
	N/A

NOTES:
	Each group has its own de-initialization function.  This was done
	more out of form than function.  Each group function just 
	de-initializes its own malloc(ed) memory, etc., as necessary.

REQUIREMENTS:

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	N/A
 
FUNCTIONS_CALLED:
	PGS_TSF_ThreadEndAA()
	PGS_TSF_ThreadEndCSC()
	PGS_TSF_ThreadEndSMF()
	PGS_TSF_ThreadEndIO()
	PGS_TSF_ThreadEndEPH()
	PGS_TSF_ThreadEndMET()

END_PROLOG:
***************************************************************************/
void PGS_TSF_ThreadEnd(void *tag)
{
#ifdef _PGS_THREADSAFE
    int index = (int) tag;

/***************************************************************************
  The tag passed in equals the value of the value stored in the key
  that was created that loaded this function.  So, we can reset
  the threadUsed array to false.
***************************************************************************/
    threadUsed[index] = PGS_FALSE;

/***************************************************************************
  De-init the rest of the groups.
***************************************************************************/
    PGS_TSF_ThreadEndAA(&globalVar);
    PGS_TSF_ThreadEndCSC(&globalVar);
    PGS_TSF_ThreadEndSMF(&globalVar);
    PGS_TSF_ThreadEndIO(&globalVar);
    PGS_TSF_ThreadEndEPH(&globalVar);
    PGS_TSF_ThreadEndMET(&globalVar);

#endif
    return;
}



/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	This function returns the master index value.  This value is
	used as the index in global arrays which were implemented 
	due to the limit on the number of TSD's that can be active
	in a process.
 
NAME:  
	PGS_TSF_ThreadEnd()

SYNOPSIS:

C:
		int 
		PGS_TSF_GetMasterIndex();

FORTRAN:
		N/A

DESCRIPTION:
	This returns the master index value.
 
INPUTS:
	Name		Description			Units	Min	Max

	NONE

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	The master index value (int).

EXAMPLES:

C:
	int masterTSFIndex;
		.
		.
		.
	masterTSFIndex = PGS_TSF_GetMasterIndex();
		.
		.
		.

FORTRAN:
	N/A

NOTES:
	NONE

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
int PGS_TSF_GetMasterIndex()
{
    int index = PGSd_TSF_BAD_MASTER_INDEX;
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;
    PGSt_TSF_MasterStruct *threadMaster;

    if (initFlag == PGS_FALSE)
    {
        returnStatus = PGS_TSF_GetTSFMaster(&threadMaster);
    }

    if (!(PGS_SMF_TestErrorLevel(returnStatus)))
    {
        index = (int) pthread_getspecific(globalVar.keyTSFMasterIndex);
    }

#endif
    return index;
}
