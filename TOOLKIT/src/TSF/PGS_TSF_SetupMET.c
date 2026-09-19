/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupMET.c

DESCRIPTION:
	This file contains the following functions

	PGS_TSF_ThreadInitMET()
	PGS_TSF_ThreadEndMET()

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	08-Jun-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>
#include <PGS_MET.h>
#include <stdlib.h>
#include <string.h>
#include <CUC/odldef.h>
 
#ifdef _PGS_THREADSAFE
PGSt_integer PGSg_TSF_METnewFilePosition[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_METnewFilePositionF[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_METNumOfMCF[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the MET portion of the thread-safe version of 
	the PGS Toolkit.
 
NAME:  
	PGS_TSF_ThreadInit()

SYNOPSIS:

C:
		void 
		PGS_TSF_ThreadInitMET(
			PGSt_TSF_MasterStruct *threadMaster,
			int index)

FORTRAN:
		N/A

DESCRIPTION:
	This function performs the THREAD initialization for the MET
	portion of the PGS Toolkit.
	
 
INPUTS:
	Name		Description			Units	Min	Max

	threadMaster	master structure storing TSF data

	index  		master TSF global variable index

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	NONE

EXAMPLES:

C:
	int	index;
	PGSt_TSF_MasterStruct	*threadSafeMaster;
		.
		.
		.
	PGS_TSF_ThreadInitMET(threadSafeMaster,index);
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
	PGSg_TSF_METnewFilePosition
	PGSg_TSF_METnewFilePositionF
	PGSg_TSF_METNumOfMCF

FILES:
	N/A
 
FUNCTIONS_CALLED:

END_PROLOG:
***************************************************************************/

void PGS_TSF_ThreadInitMET(PGSt_TSF_MasterStruct *threadMaster,int index)
{
#ifdef _PGS_THREADSAFE
    AGGREGATE *masterNode;
    AGGREGATE *aggList;
    char *temChar;

/***************************************************************************
  Initialize globals.
***************************************************************************/
    PGSg_TSF_METnewFilePosition[index] = 0;
    PGSg_TSF_METnewFilePositionF[index] = 0;
    PGSg_TSF_METNumOfMCF[index] = -1;

/***************************************************************************
  Allocate space and initialize TSDs.
***************************************************************************/
    masterNode = (AGGREGATE *) malloc(sizeof(AGGREGATE) * PGSd_MET_NUM_OF_MCF);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                          masterNode);

    aggList = (AGGREGATE *) malloc(sizeof(AGGREGATE) * PGSd_MET_NUM_OF_SIG);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYMETAGGLIST],aggList);

    temChar = (char *) malloc(sizeof(char) * PGSd_MET_NUM_OF_SIG * 
                                                PGSd_MET_SIGNATURE_L);
    strcpy(temChar,"");
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYMETATTRHANDLE],temChar);

#endif
    return;
}



/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	De-initializes the MET portion of the thread-safe version of 
	the PGS Toolkit.
 
NAME:  
	PGS_TSF_ThreadEndMET()

SYNOPSIS:

C:
		void 
		PGS_TSF_ThreadEndMET(
			PGSt_TSF_MasterStruct *threadMaster)

FORTRAN:
		N/A

DESCRIPTION:
	This function performs the THREAD initialization for the MET
	portion of the PGS Toolkit.
	
 
INPUTS:
	Name		Description			Units	Min	Max

	threadMaster	master structure storing TSF data

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	NONE

EXAMPLES:

C:
	PGSt_TSF_MasterStruct	*threadSafeMaster;
		.
		.
		.
	PGS_TSF_ThreadEndMET(threadSafeMaster,index);
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
void PGS_TSF_ThreadEndMET(PGSt_TSF_MasterStruct *masterTSF)
{
#ifdef _PGS_THREADSAFE
    AGGREGATE *masterNode;
    AGGREGATE *aggList;
    char *temChar;

/***************************************************************************
  Free TSDs.
***************************************************************************/
    masterNode = (AGGREGATE *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE]);
    free(masterNode);

    aggList = (AGGREGATE *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYMETAGGLIST]);
    free(aggList);

    temChar = (char *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYMETATTRHANDLE]);
    free(temChar);

#endif
    return;
}
