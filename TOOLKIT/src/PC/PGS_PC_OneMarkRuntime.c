/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_OneMarkRuntime.c

DESCRIPTION:
	This file contains the function PGS_PC_OneMarkRuntime().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	13-Apr-95 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Mark a file as a runtime file.
 
NAME:
	PGS_PC_OneMarkRuntime()

SYNOPSIS:

C:
	#include <PGS_PC.h>

	PGSt_SMF_status
	PGS_PC_OneMarkRuntime(
		char			*fileName,
		PGSt_PC_Logical		identifier);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to mark a single file as a runtime send
	file in either the PCF or in shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	fileName	The full path and file name of
			the file to be marked.

	identifier	The logical identifier as defined 
			by the user.  The user's 
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

	fileType	Type of file to mark.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:
C:	
	#define MODIS1A 2511

	PGSt_PC_Logical	identifier;
	char		fileName[PGSd_PC_FILE_PATH_MAX];
	PGSt_integer	version;
	PGSt_SMF_status	returnStatus;

	/# getting the file name of the first file in the #/
	/# product group MOSID1A #/

	prodID = MODIS1A;
	version = 1;
	returnStatus = PGS_PC_GetReferenceType(identifier,&fileType);
	returnStatus = PGS_PC_GetReference(identifier,&version,fileName);

	/# mark that file as a runtime file #/

	if (returnStatus == PGS_S_SUCCESS)
	{
		returnStatus = PGS_PC_OneMarkRuntime(fileName,identifier,fileType);
	}

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# continue normal processing #/
		}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	In order for this tool to function properly, a valid Process Control
	file will need to be created first.  Please refer to Appendix C 
	(User's Guide) for instructions on how to create such a file.

	This tool has been made specifically to be called by the SMF tools
	and may not function properly otherwise.

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE"

FUNCTIONS_CALLED:
	PGS_MEM_ShmSysAddr		- Get address in shared memory
					of PC data.
	PGS_SMF_CallerID		- Determine who is calling.
	PGS_SMF_SetStaticMsg		- Set a static error/status 
					message in the message log.
	PGS_PC_MarkRuntimeAscii		- Mark a file as runtime in
					the PCF.
	PGS_PC_MarkRuntimeShm		- Mark a file as runtime in
					shared memory.

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_OneMarkRuntime(                       /* mark a file as runtime */
    char             *fileName,              /* file name to mark */
    PGSt_PC_Logical   identifier,            /* logical id to mark */
    PGSt_integer      fileType)              /* type of file to mark */

{
    char             *useShm;                /* SHM environment variable */
    char             *addrPC;                /* address in Shm of PC data */
    PGSt_uinteger     size;                  /* amount of shm allocated for PC */
    PGSt_SMF_status   whoCalled;             /* user that called this function */
    PGSt_SMF_status   returnStatus;          /* SMF return */

/***************************************************************************
*    Initialize variables.     
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.     
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Determine if we are using shared memory.
***************************************************************************/
    useShm = getenv(PGSd_PC_USESHM_ENV);
    if (useShm == NULL)
    {
        useShm = (char *) PGSd_PC_USESHM_NO;
    }

/***************************************************************************
*    We are using shared memory.
***************************************************************************/
    if (strcmp(useShm,PGSd_PC_USESHM_YES) == 0)
    {
        returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,
                                      (void **) &addrPC,&size);

        if (returnStatus == PGS_S_SUCCESS)
        {
            returnStatus = PGS_PC_MarkRuntimeShm(addrPC,fileName,
                                                      identifier,fileType);
        }
    }

/***************************************************************************
*    The data is not in shared memory.
***************************************************************************/
    else
    {
        returnStatus = PGS_PC_MarkRuntimeAscii(fileName,identifier,fileType);
    }

/***************************************************************************
*    Set the message in the log and return.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_OneMarkRuntime()");
    }
    return returnStatus;
}
