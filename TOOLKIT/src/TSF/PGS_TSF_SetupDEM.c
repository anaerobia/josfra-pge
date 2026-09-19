/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupDEM.c

DESCRIPTION:
	This file contains the function PGS_TSF_SetupDEM().

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	26-May-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>
#include <PGS_DEM.h>
#include <stdlib.h>
#include <string.h>

#ifdef _PGS_THREADSAFE
int32 PGSg_TSF_DEMattachedFiles[_POSIX_THREAD_THREADS_MAX][PGSd_DEM_MAX_STAGED][4];
PGSt_integer PGSg_TSF_DEMnumAttached[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the thread-safe version of the PGS Toolkit.
 
NAME:  
	PGS_TSF_SetupDEM()

SYNOPSIS:

C:
		PGSt_SMF_status
		PGS_TSF_SetupDEM(PGSt_TSF_MasterStruct *threadMaster);

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
	returnStatus = PGS_TSF_SetupDEM(&threadSafeMaster);
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

void PGS_TSF_ThreadInitDEM(PGSt_TSF_MasterStruct *threadMaster,int index)
{
#ifdef _PGS_THREADSAFE
    PGSt_DEM_SubsetRecord *temRecord;
    PGSt_DEM_FileRecord *tem1Record;    
    PGSt_DEM_PointRecordPix *tem2Record;
    PGSt_DEM_PointRecordDeg *tem3Record;
    int outLoop;
    int inLoop;

/***************************************************************************
  Initialize globals.
***************************************************************************/
    PGSg_TSF_DEMnumAttached[index] = 0; 
    
    for (outLoop = 0; outLoop < PGSd_DEM_MAX_STAGED; outLoop++)
    {            
        for (inLoop = 0; inLoop < 4; inLoop++)
        {
            PGSg_TSF_DEMattachedFiles[index][outLoop][inLoop] = 0;
        }
    }

/***************************************************************************
  Initialize TSDs.
***************************************************************************/
    temRecord = NULL;
    tem1Record = NULL;
    tem2Record = NULL;
    tem3Record = NULL;
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMINFO3ARC1],temRecord);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMINFO3ARC2],temRecord);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMINFO3ARC3],temRecord);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMINFO30ARC1],temRecord);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMINFO30ARC2],temRecord);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMINFO30ARC3],temRecord);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMINFO30TEST],temRecord);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC1],
                                         tem1Record);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC2],
                                         tem1Record);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC3],
                                         tem1Record);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC1],
                                         tem1Record);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC2],
                                         tem1Record);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC3],
                                         tem1Record); 
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMSUBSET30TEST],
                                         tem1Record);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMINFOQUALITY],
                                         temRecord);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMSUBSETINFOORIG],
                                         temRecord);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMPOINTARRAY],
                                         tem2Record);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYDEMSUMDEGPOINTARRAY],
                                         tem3Record);

#endif
}
