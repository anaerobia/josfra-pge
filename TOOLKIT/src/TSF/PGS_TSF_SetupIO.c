/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 1999, Raytheon Systems Company, its vendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_TSF_SetupIO.c

DESCRIPTION:
	This file contains the following functions: 

	PGS_TSF_ThreadInitIO()
	PGS_TSF_ThreadEndIO()

AUTHOR:
	Ray Milburn / Steven Myers & Associates

HISTORY:
	26-May-99 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <PGS_TSF.h>
#include <PGS_IO.h>
#include <stdlib.h>
#include <string.h>

#ifdef _PGS_THREADSAFE
int PGSg_TSF_IOGenLastTime[_POSIX_THREAD_THREADS_MAX];
PGSt_SMF_status PGSg_TSF_IOL0RetStat2[_POSIX_THREAD_THREADS_MAX];
PGSt_integer PGSg_TSF_IOL0InitFlag[_POSIX_THREAD_THREADS_MAX];
#endif

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Initializes the IO portion of the threadsafe PGS Toolkit.
 
NAME:  
	PGS_TSF_ThreadInitIO()

SYNOPSIS:

C:
		void 
		PGS_TSF_ThreadInitIO(
			PGSt_TSF_MasterStruct *threadMaster,
			int index)

FORTRAN:
		N/A

DESCRIPTION:
	This tool is called during the THREAD initialization phase of 
	the thread-safe version of the PGS Toolkit.
 
INPUTS:
	Name		Description			Units	Min	Max

	threadMaster    master structure for TSF

	index           master index of TSF globals

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
	PGS_TSF_ThreadInitIO(threadSafeMaster,index);
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
	PGSg_TSF_IOGenLastTime
	PGSg_TSF_IOL0RetStat2
	PGSg_TSF_IOL0InitFlag

FILES:
	N/A
 
FUNCTIONS_CALLED:

END_PROLOG:
***************************************************************************/

void PGS_TSF_ThreadInitIO(PGSt_TSF_MasterStruct *threadMaster,int index)
{
#ifdef _PGS_THREADSAFE
    PGSt_IO_L0_FileHdrADEOS_II *dumHeader2;
    PGSt_IO_L0_FileTable *tptr;
    PGSt_IO_L0_FileTable *fileTable;
    PGSt_IO_L0_SecPktHdrEOS_AM   *pktEOSAM;
    PGSt_IO_L0_SecPktHdrEOS_PM_GIIS   *pktEOSPMGIIS;
    PGSt_IO_L0_SecPktHdrEOS_PM_GIRD   *pktEOSPMGIRD;
    PGSt_IO_L0_SecPktHdrEOS_AURA   *pktEOSAURA;
    PGSt_IO_L0_SecPktHdrTRMM     *pktTRMM;
    PGSt_IO_L0_SecPktHdrADEOS_II *pktADEOSII;
    PGSt_IO_L0_FileHdrTRMM *trmmHdr;


/***************************************************************************
  Initialize IO globals.
***************************************************************************/
    PGSg_TSF_IOGenLastTime[index] = 0;
    PGSg_TSF_IOL0RetStat2[index] = PGS_S_SUCCESS;
    PGSg_TSF_IOL0InitFlag[index] = 0;

/***************************************************************************
  Set up file header TSDs.
***************************************************************************/
    dumHeader2 = (PGSt_IO_L0_FileHdrADEOS_II *) malloc(sizeof(
                              PGSt_IO_L0_FileHdrADEOS_II));
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL0FILEHDRADEOSII],
                               (void *) &dumHeader2);

    tptr = 0;
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL0TPTR],tptr);


/***************************************************************************
  Set up file table TSD.
***************************************************************************/
    fileTable = (PGSt_IO_L0_FileTable *) malloc(sizeof(PGSt_IO_L0_FileTable) 
                               * PGSd_IO_L0_MaxOpenFiles);
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL0FILETABLE],fileTable);

/***************************************************************************
  Set up packet header TSDs.
***************************************************************************/
    pktEOSAM = (PGSt_IO_L0_SecPktHdrEOS_AM *) malloc(sizeof(
                                PGSt_IO_L0_SecPktHdrEOS_AM));
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL02PKTEOSAM],
                               (void *) pktEOSAM);
    pktEOSPMGIIS = (PGSt_IO_L0_SecPktHdrEOS_PM_GIIS *) malloc(sizeof(
                                PGSt_IO_L0_SecPktHdrEOS_PM_GIIS));
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL02PKTEOSPMGIIS],
                               (void *) pktEOSPMGIIS);
    pktEOSPMGIRD = (PGSt_IO_L0_SecPktHdrEOS_PM_GIRD *) malloc(sizeof(
                                PGSt_IO_L0_SecPktHdrEOS_PM_GIRD));
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL02PKTEOSPMGIRD],
                               (void *) pktEOSPMGIRD);
    pktEOSAURA = (PGSt_IO_L0_SecPktHdrEOS_AURA *) malloc(sizeof(
                                PGSt_IO_L0_SecPktHdrEOS_AURA));
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL02PKTEOSAURA],
                               (void *) pktEOSAURA);
    pktTRMM = (PGSt_IO_L0_SecPktHdrTRMM *) malloc(sizeof(
                                PGSt_IO_L0_SecPktHdrTRMM));
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL02PKTTRMM],
                               (void *) pktTRMM);
    pktADEOSII = (PGSt_IO_L0_SecPktHdrADEOS_II *) malloc(sizeof(
                                PGSt_IO_L0_SecPktHdrADEOS_II));
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL02PKTADEOSII],
                               (void *) pktADEOSII);

    trmmHdr = (PGSt_IO_L0_FileHdrTRMM *) malloc(sizeof(PGSt_IO_L0_FileHdrTRMM));
    pthread_setspecific(threadMaster->keyArray[PGSd_TSF_KEYIOL0FILEHDRTRMM],
                               (void *) trmmHdr);

#endif
    return;
}



/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	De-initializes the IO portion of the threadsafe PGS Toolkit.
 
NAME:  
	PGS_TSF_ThreadEndIO()

SYNOPSIS:

C:
		void 
		PGS_TSF_ThreadEndIO(
			PGSt_TSF_MasterStruct *threadMaster)

FORTRAN:
		N/A

DESCRIPTION:
	This tool is called during the THREAD exit phase of 
	the thread-safe version of the PGS Toolkit.
 
INPUTS:
	Name		Description			Units	Min	Max

	threadMaster    master structure for TSF

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
	PGS_TSF_ThreadEndIO(threadSafeMaster,index);
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
void PGS_TSF_ThreadEndIO(PGSt_TSF_MasterStruct *masterTSF)
{
#ifdef _PGS_THREADSAFE
    PGSt_IO_L0_FileHdrADEOS_II *dumHeader2;
    PGSt_IO_L0_FileTable *temTable;
    PGSt_IO_L0_SecPktHdrEOS_AM *temHeadAM;
    PGSt_IO_L0_SecPktHdrEOS_PM_GIIS *temHeadPMGIIS;
    PGSt_IO_L0_SecPktHdrEOS_PM_GIRD *temHeadPMGIRD;
    PGSt_IO_L0_SecPktHdrTRMM *temHeadTM;
    PGSt_IO_L0_SecPktHdrADEOS_II *temHeadII;
    PGSt_IO_L0_FileHdrTRMM *trmmHdr;

/***************************************************************************
  Free all space allocated in the init function.
***************************************************************************/
    dumHeader2 = (PGSt_IO_L0_FileHdrADEOS_II *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYIOL0FILEHDRADEOSII]);
    free(dumHeader2);

    temTable = (PGSt_IO_L0_FileTable *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYIOL0FILETABLE]);
    free(temTable);

    temHeadAM = (PGSt_IO_L0_SecPktHdrEOS_AM *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSAM]);
    free(temHeadAM);
    temHeadPMGIIS = (PGSt_IO_L0_SecPktHdrEOS_PM_GIIS *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSPMGIIS]);
    free(temHeadPMGIIS);
    temHeadPMGIRD = (PGSt_IO_L0_SecPktHdrEOS_PM_GIRD *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSPMGIRD]);
    free(temHeadPMGIRD);
    temHeadTM = (PGSt_IO_L0_SecPktHdrTRMM *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTTRMM]);
    free(temHeadTM);
    temHeadII = (PGSt_IO_L0_SecPktHdrADEOS_II *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTADEOSII]);
    free(temHeadII);

    trmmHdr = (PGSt_IO_L0_FileHdrTRMM *) pthread_getspecific(
                          masterTSF->keyArray[PGSd_TSF_KEYIOL0FILEHDRTRMM]);
    free(trmmHdr);
#endif
    return;
}
