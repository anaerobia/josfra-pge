/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_HandleLock.c

DESCRIPTION:
	This file contains the following functions: 

	PGS_TSF_LockIt()
	PGS_TSF_UnlockIt()

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	13-Apr-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Locks the specified mutex lock.
 
NAME:  
	PGS_TSF_LockIt()

SYNOPSIS:

C:
		PGSt_SMF_status
		PGS_TSF_LockIt(int lockFlag);

FORTRAN:
		N/A

DESCRIPTION:
	This function locks the specified mutex lock in the lock array.
 
INPUTS:
	Name		Description			Units	Min	Max

	lockFlag	Flag indicating which lock to set

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSTSF_E_MUTEXLOCK_FAIL     mutex lock failure
	PGSTSF_E_NOT_THREADSAFE     Toolkit not in thread safe mode

EXAMPLES:

C:
	PGSt_SMF_Status	returnStaus;
		.
		.
		.
	/# lock SMF code #/
	returnStatus = PGS_TSF_LockIt(PGSd_TSF_SMF_LOCK);
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
	NONE

FILES:
	N/A
 
FUNCTIONS_CALLED:
	PGS_TSF_GetTSFMaster()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_TSF_LockIt(
    int 		lockFlag)            /* which lock */
{
#ifdef _PGS_THREADSAFE
    int                 lockRet;             /* lock () return */
    PGSt_SMF_status     returnStatus;        /* function return */
    PGSt_TSF_MasterStruct *globalVar;        /* global vars */

    returnStatus = PGS_TSF_GetTSFMaster(&globalVar);

    if (returnStatus == PGS_S_SUCCESS)
    {
        lockRet = pthread_mutex_lock(&globalVar->lockArray[lockFlag]);
        if (lockRet != PGSd_TSF_LOCK_SUCCESS)
        {
            returnStatus = PGSTSF_E_MUTEXLOCK_FAIL;
        }
    }

    return returnStatus;
#else
    return PGSTSF_E_NOT_THREADSAFE;
#endif
}


/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Unlocks the specified mutex lock.
 
NAME:  
	PGS_TSF_UnlockIt()

SYNOPSIS:

C:
		PGSt_SMF_status
		PGS_TSF_UnlockIt(int lockFlag);

FORTRAN:
		N/A

DESCRIPTION:
	This tool unlocks the specified mutex lock in the lock array.
 
INPUTS:
	Name		Description			Units	Min	Max

	lockFlag	Flag indicating which lock to unset

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSTSF_E_NOT_THREADSAFE     Toolkit not in threadsafe mode
	PGSTSF_W_MUTEXUNLOCK_FAIL   mutex unlock failure

EXAMPLES:

C:
	PGSt_SMF_Status	returnStaus;
		.
		.
		.
	/# lock SMF code #/
	returnStatus = PGS_TSF_UnlockIt(PGSd_TSF_SMF_LOCK);
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
	NONE

FILES:
	N/A
 
FUNCTIONS_CALLED:
	PGS_TSF_GetTSFMaster()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_TSF_UnlockIt(
    int 		lockFlag)            /* which lock */
{
#ifdef _PGS_THREADSAFE
    int                 lockRet;             /* lock () return */
    PGSt_SMF_status     returnStatus;        /* function return */
    PGSt_TSF_MasterStruct *globalVar;        /* global vars */

    returnStatus = PGS_TSF_GetTSFMaster(&globalVar);

    if (returnStatus == PGS_S_SUCCESS)
    {
        lockRet = pthread_mutex_unlock(&globalVar->lockArray[lockFlag]);
        if (lockRet != PGSd_TSF_LOCK_SUCCESS)
        {
            returnStatus = PGSTSF_W_MUTEXUNLOCK_FAIL;
        }
    }

    return returnStatus;
#else
    return PGSTSF_E_NOT_THREADSAFE;
#endif
}
