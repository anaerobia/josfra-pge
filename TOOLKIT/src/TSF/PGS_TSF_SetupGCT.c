/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupGCT.c

DESCRIPTION:
	This file contains the function PGS_TSF_SetupGCT().

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	26-May-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>
#include <stdlib.h>
#include <string.h>
#include "PGS_GCT.h"

#ifdef _PGS_THREADSAFE
double PGSg_TSF_GCTrMajor[_POSIX_THREAD_THREADS_MAX];
double PGSg_TSF_GCTrMinor[_POSIX_THREAD_THREADS_MAX];
int PGSg_TSF_GCTset[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the thread-safe version of the PGS Toolkit.
 
NAME:  
	PGS_TSF_SetupGCT()

SYNOPSIS:

C:
		PGSt_SMF_status
		PGS_TSF_SetupGCT(PGSt_TSF_MasterStruct *threadMaster);

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
	returnStatus = PGS_TSF_SetupGCT(&threadSafeMaster);
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

void PGS_TSF_ThreadInitGCT(PGSt_TSF_MasterStruct *threadMaster,int index)
{ 
#ifdef _PGS_THREADSAFE

/***************************************************************************
  Initialize globals.
***************************************************************************/
    PGSg_TSF_GCTrMajor[index] = 0.0;
    PGSg_TSF_GCTrMinor[index] = 0.0;
    PGSg_TSF_GCTset[index] = PGS_FALSE;

#endif
}
