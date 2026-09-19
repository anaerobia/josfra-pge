/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupCBP.c

DESCRIPTION:
	This file contains the function PGS_TSF_SetupCBP().

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
PGSt_double PGSg_TSF_CBPPlephpv[_POSIX_THREAD_THREADS_MAX][78];
PGSt_double PGSg_TSF_CBPPlephpvSun[_POSIX_THREAD_THREADS_MAX][6];
PGSt_double PGSg_TSF_CBPStatet[_POSIX_THREAD_THREADS_MAX][2];
PGSt_double PGSg_TSF_CBPStatejd[_POSIX_THREAD_THREADS_MAX][4];
PGSt_double PGSg_TSF_CBPInterppc[_POSIX_THREAD_THREADS_MAX][18];
PGSt_double PGSg_TSF_CBPInterpvc[_POSIX_THREAD_THREADS_MAX][18];
PGSt_integer PGSg_TSF_CBPPlephnemb[_POSIX_THREAD_THREADS_MAX];
PGSt_double PGSg_TSF_CBPStateauFac[_POSIX_THREAD_THREADS_MAX];
PGSt_boolean PGSg_TSF_CBPStatefirst[_POSIX_THREAD_THREADS_MAX];
PGSt_boolean PGSg_TSF_CBPPlephfirst[_POSIX_THREAD_THREADS_MAX];
PGSt_boolean PGSg_TSF_CBPEphemfirst[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_CBPInterpnp[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_CBPInterpnv[_POSIX_THREAD_THREADS_MAX];
PGSt_double PGSg_TSF_CBPInterptwot[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the CBP portion of the PGS Toolkit.
 
NAME:  
	PGS_TSF_ThreadInitCBP()

SYNOPSIS:

C:
		PGSt_SMF_status
		PGS_TSF_SetupCBP(PGSt_TSF_MasterStruct *threadMaster);

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
	PGSt_TSF_MasterStruct	threadSafeMaster;
		.
		.
		.
	returnStatus = PGS_TSF_ThreadInitCBP(&threadSafeMaster);
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

/* static int threadID = 100; */                     /* thread ID for SMF stuff */

void PGS_TSF_ThreadInitCBP(PGSt_TSF_MasterStruct *threadMaster,int index)
{
#ifdef _PGS_THREADSAFE
    int loopVar;
/*    PGSt_double stateAuFac;
    PGSt_double interpTwot;
    PGSt_double *cbpPVSun;
    PGSt_double *cbpStatet;
    PGSt_double *cbpStatejd;
    PGSt_double *cbpInterppc;
    PGSt_double *cbpInterpvc; */

/***************************
  Do pv variable in PGS_CBP_Pleph()
***************************/
    for (loopVar = 0; loopVar < 78; loopVar++)
    {
        PGSg_TSF_CBPPlephpv[index][loopVar] = 0.0;
    }

    for (loopVar = 0; loopVar < 6; loopVar++)
    {
        PGSg_TSF_CBPPlephpvSun[index][loopVar] = 0.0;
    }

    for (loopVar = 0; loopVar < 2; loopVar++)
    {
        PGSg_TSF_CBPStatet[index][loopVar] = 0.0;
    }

    for (loopVar = 0; loopVar < 4; loopVar++)
    {
        PGSg_TSF_CBPStatejd[index][loopVar] = 0.0;
    }

    for (loopVar = 0; loopVar < 18; loopVar++)
    {
        PGSg_TSF_CBPInterppc[index][loopVar] = 0.0;
    }

    for (loopVar = 0; loopVar < 18; loopVar++)
    {
        PGSg_TSF_CBPInterpvc[index][loopVar] = 0.0;
    }

    PGSg_TSF_CBPPlephnemb[index] = 1;
    PGSg_TSF_CBPStateauFac[index] = 1.0;
    PGSg_TSF_CBPInterpnp[index] = 2;
    PGSg_TSF_CBPInterpnv[index] = 3;
    PGSg_TSF_CBPInterptwot[index] = 0.0;
    PGSg_TSF_CBPEphemfirst[index] = PGS_TRUE;
    PGSg_TSF_CBPStatefirst[index] = PGS_TRUE;
    PGSg_TSF_CBPPlephfirst[index] = PGS_TRUE;

#endif
}
