/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupTD.c

DESCRIPTION:
	This file contains the function PGS_TSF_ThreadInit().

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
double PGSg_TSF_TDPeriod[_POSIX_THREAD_THREADS_MAX];
double PGSg_TSF_TDSCReference[_POSIX_THREAD_THREADS_MAX];
double PGSg_TSF_TDGroundReference[_POSIX_THREAD_THREADS_MAX];
double PGSg_TSF_TDutcf[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the TD portion of the PGS Toolkit.
 
NAME:  
	PGS_TSF_ThreadInitTD()

SYNOPSIS:

C:
		void 
		PGS_TSF_ThreadInitTD(
			PGSt_TSF_MasterStruct *threadMaster,
			int index)

FORTRAN:
		N/A

DESCRIPTION:
	This tool is called during the THREAD initialization phase 
	of the thread-safe version of the PGS Toolkit.

INPUTS:
	Name		Description			Units	Min	Max

	threadMaster    master TSF structure

	index           index used in TSF globals 

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	N/A

EXAMPLES:

C:
	int	index;
	PGSt_TSF_MasterStruct	*threadSafeMaster;
		.
		.
		.
	PGS_TSF_ThreadInitTD(threadSafeMaster,index);
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
	PGSg_TSF_TDPeriod
	PGSg_TSF_TDSCReference
	PGSg_TSF_TDGroundReference
	PGSg_TSF_TDutcf

FILES:
	N/A
 
FUNCTIONS_CALLED:

END_PROLOG:
***************************************************************************/

void PGS_TSF_ThreadInitTD(PGSt_TSF_MasterStruct *threadMaster,int index)
{
#ifdef _PGS_THREADSAFE

    PGSg_TSF_TDPeriod[index] = 0.0;
    PGSg_TSF_TDSCReference[index] = 0.0;
    PGSg_TSF_TDGroundReference[index] = 0.0;
    PGSg_TSF_TDutcf[index] = 0.0;

#endif
}
