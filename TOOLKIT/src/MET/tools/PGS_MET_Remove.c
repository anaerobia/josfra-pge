/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_Remove.c
 
DESCRIPTION:
	This file contains PGS_MET_Remove() which frees the memory held by
	the MCF 

AUTHOR:
  	Alward N.Siyyid / EOSL
        Carol S. W. Tsai / Space Applications Corporation

HISTORY:
  	01-JUN-1995 	ANS 	Initial version
	13-July-95      ANS     Improved Fortran example
	25-July-95	ANS	Fixed ECSed01028
	20-MAR-96	ANS 	update for tk5+
        01-Oct-97       CSWT    Deleted code that removes the aggregate node in 
                                the memory (This is for NCR ECSed09220 about 
                                PGS_MET_GetPCAttr failure after PGS_MET_Remove 
                                and PGS_MET_GetPCAttr fails if pgs_met_init is 
                                called twice) 
	07-Jul-99	RM	updated for TSF functionality

END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <stdio.h>
#include <CUC/odldef.h>
#include <CUC/odlinter.h>

#include <errno.h>
#include <PGS_MET.h>
#include <PGS_TSF.h>

/***************************************************************************
BEGIN_PROLOG:
 
	Removes MCF from the memory 
TITLE:
	Removes ODL representation of MCF file 
 
NAME:
        PGS_MET_Remove()
 
SYNOPSIS:        
C:
        #include "PGS_MET.h"

        PGSt_SMF_status
        PGS_MET_Remove()

FORTRAN:
         include "PGS_MET_13.f"
         include "PGS_MET.f"
         include "PGS_SMF.h"
         integer function pgs_met_remove()
 
DESCRIPTION:
	This routine removes ODL representation of MCF file plus some internal buffers used by the system
	
 
INPUTS:
	None

OUTPUTS:
        None
 
RETURNS:
	None
 
EXAMPLES:
C:

	This is an extract from the main example in PGS_MET_Init()

	PGS_MET_Remove();
        printf("SUCCESS\n");
        return 0;
FORTRAN:
 
	This is an extract from the main example in PGS_MET_Init()
	
	print *, ival, dval, datetime
        result = pgs_met_remove()
        print *, "SUCCESS"
 
        end
NOTES:
        This routine must be called by the user before exiting from his/her program
	It removes all the met handles initiated by the user
 
REQUIREMENTS:
        PGSTK-0430
 
DETAILS:
        NONE
 
GLOBALS:
	PGSg_MET_MasterNode 
	PGSg_TSF_METNumOfMCF

FILES:
        NONE
 
FUNCTIONS_CALLED:
	RemoveAggregate
	PGS_TSF_GetTSFMaster
	PGS_TSF_GetMasterIndex
	PGS_SMF_TestErrorLevel
 
END_PROLOG:
***************************************************************************/
void
PGS_MET_Remove() /* removes MCF and the data dictionary from memory */
{

	PGSt_integer i;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retVal;
    AGGREGATE *PGSg_MET_MasterNode;
    PGSt_integer PGSg_MET_NumOfMCF;
    PGSt_TSF_MasterStruct *masterTSF;
    int masterTSFIndex;
    extern PGSt_integer PGSg_TSF_METNumOfMCF[];

    /* get struct with TSD keys and master global index */
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ((PGS_SMF_TestErrorLevel(retVal)) ||
        (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX))
    {
        return;
    }

    /* set locals from TSD value and TSF globals - lock up, we're calling COTS */
    PGSg_MET_MasterNode = (AGGREGATE *) pthread_getspecific(
                                  masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE]);
    PGSg_MET_NumOfMCF = PGSg_TSF_METNumOfMCF[masterTSFIndex];
    retVal = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
    if (PGS_SMF_TestErrorLevel(retVal))
    {
        return;
    }
#else
	extern AGGREGATE PGSg_MET_MasterNode[];
	extern AGGREGATE PGSg_MET_AggList[];
	extern PGSt_integer     PGSg_MET_NumOfMCF;
#endif

	/* removing MCF */


	for(i = 0; i<= PGSg_MET_NumOfMCF; i++)
        {
        	PGSg_MET_MasterNode[i] = RemoveAggregate(PGSg_MET_MasterNode[i]);
	}

	PGSg_MET_NumOfMCF = -1;

#ifdef _PGS_THREADSAFE
    /* unlock - re-set TSD value and global */
    PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
    pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                        PGSg_MET_MasterNode);
    PGSg_TSF_METNumOfMCF[masterTSFIndex] = PGSg_MET_NumOfMCF;
#endif
}
