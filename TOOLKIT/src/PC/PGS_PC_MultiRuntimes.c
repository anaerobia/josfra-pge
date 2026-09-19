/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_MultiRuntimes.c

DESCRIPTION:
	This file contains the function PGS_PC_MultiRuntimes().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	17-Apr-95 RM Initial version
 
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
	Mark multiple files as runtime.
 
NAME:
	PGS_PC_MultiRuntimes()

SYNOPSIS:

C:
	#include <PGS_PC.h>

	PGSt_SMF_status
	PGS_PC_MultiRuntimes(
		PGSt_PC_Logical		logicalID[],
		PGSt_integer		versions[],
		PGSt_integer		amount);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to mark a multiple files as a runtime send
	files in the PCF or shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	logicalID	Array of logical ID's.

	versions	Array of versions.

	amount		Number of files to be marked
			as runtime send files.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:
C:	
	#define NUMBEROFIDS 20

	PGSt_PC_Logical	logicalID[NUMBEROFIDS];
	PGSt_integer	versions[NUMBEROFIDS];
	PGSt_integer	amount;
	PGSt_SMF_status	returnStatus;

	/# assuming that 20 (NUMBEROFIDS) logical ID's and #/
	/# versions were placed in the logical ID and version #/
	/# array #/

	amount = NUMBEROFIDS;

	if (returnStatus == PGS_S_SUCCESS)
	{
		returnStatus = PGS_PC_MultiRuntimes(logicalID,versions,amount);
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

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_MultiRuntimes(                        /* mark a file as runtime */
    PGSt_PC_Logical   logicalID[],           /* logical id to mark */
    PGSt_integer      versions[],            /* type of file to mark */
    PGSt_integer      amount)                /* type of file to mark */

{
    char              fileName[PGSd_PC_FILE_PATH_MAX];  /* name of file */
    int               count;                 /* loop counter */
    PGSt_integer      type;                  /* type of file */
    PGSt_integer      numFiles;              /* number of files remaining */
    PGSt_SMF_status   whoCalled;             /* user that called this function */
    PGSt_SMF_status   saveStatus;            /* save return status */
    PGSt_SMF_status   returnStatus;          /* SMF return */

/***************************************************************************
*    Initialize variables.     
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    saveStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.     
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Loop the number of ID's times.
***************************************************************************/
    for (count = 0; count < amount; count++)
    {

/***************************************************************************
*    Get the type of file.
***************************************************************************/
        returnStatus = PGS_PC_GetReferenceType(logicalID[count],&type);
        if (returnStatus == PGS_S_SUCCESS)
        {

/***************************************************************************
*    If the file is a product input file then let's use the version
*    passed in, otherwise just set it to one.  Remember, product input
*    files are the only files that are allowed to have a multiple-to-one
*    relationship.
***************************************************************************/
            if (type == PGSd_PC_INPUT_FILE_NAME)
            {
                numFiles = versions[count];
            }
            else
            {
                numFiles = 1;
            }

/***************************************************************************
*    Get the file name.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSData(type,logicalID[count],
                                                    fileName,&numFiles);
            if (returnStatus == PGS_S_SUCCESS)
            {

/***************************************************************************
*    Mark the file.
***************************************************************************/
                returnStatus = PGS_PC_OneMarkRuntime(fileName,
                                                    logicalID[count],type);
                if (returnStatus != PGS_S_SUCCESS) 
                {
                    saveStatus = returnStatus;
                }
            }
            else
            {
                saveStatus = returnStatus;
            }
        }
        else
        {
            saveStatus = returnStatus;
        }
    }  /* end for */

    returnStatus = saveStatus;

/***************************************************************************
*    Set the message in the log and return.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_OneMarkRuntime()");
    }
    return returnStatus;
}
