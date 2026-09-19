/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_MarkRuntimeShm.c

DESCRIPTION:
	This file contains the function PGS_PC_MarkRuntimeShm().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	14-Apr-95 RM Initial version
	07-Jul-99 RM Updated for TSF functionality
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <PGS_PC.h>
#include <PGS_TSF.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Mark a file as a runtime file in shared memory.
 
NAME:
	PGS_PC_MarkRuntimeShm()

SYNOPSIS:

C:
	#include <PGS_PC.h>
	
	PGSt_SMF_status
	PGS_PC_MarkRuntimeShm(
		char 			*addrPC,
		char 			*fileName,
		PGSt_PC_Logical		logicalID,
		PGSt_integer		fileType);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to mark files as a runtime file in shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	addrPC		pointer to address in shared
			memory of PC data.

	fileName	full file and path name of file
			to be marked as runtime.

	logicalID	The logical identifier as defined 
			by the user.  The user's 
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

	fileType	Type of file (section that the 
			file is located in).

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_EXIST      no file data exists for this logical id
	PGSPC_E_INDEX_ERROR_SHM     index value was not an integer at initialization
	PGSPC_E_INV_DIVPOINTER_SHM  address of division pointer does not point to
				    a divider
	PGSTSF_E_GENERAL_FAILURE    problem in TSF code

EXAMPLES:

C:
	#define		MODIS1A	101

	char 		*addrPC;      /# address of PC data in Shm #/
	char		fileName[PGSd_PC_FILE_PATH_MAX];
	PGSt_integer	fileType;
	PGSt_SMF_status	returnStatus;
	PGSt_PC_Logical	identifier;

	/# assuming the address in shared memory of the PC #/
	/# was successfully retrieved and that the #/
	/# header structure was read in from shared #/
	/# memory successfully #/

	identifier = MODIS1A;

	/# assuming that we have successfully received the file #/
	/# name and the file type. #/

	returnStatus = PGS_PC_MarkRuntimeShm(addrPC,fileName,
			identifier,fileType);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# file marked as runtime - continue with normal operations #/
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
	(User's Guide) for instructions on how to create such a file.  This
	file must be processed through the PC initiazation program 
	PGS_PC_InitCom.

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_PC_BuildFileShm		 Build a file name from data from
					 shared memory
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_TestErrorLevel           Check if return is _E_ level
	PGS_TSF_LockIt                   Lock section of code for TSF.
	PGS_TSF_UnlockIt                 Unlock section of code for TSF.

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_MarkRuntimeShm( 	                 /* mark file as runtime in Shm */
    char             *addrPC,            /* address of start of Shm */
    char             *fileName,          /* full path and file name to mark */
    PGSt_PC_Logical   logicalID,         /* logical id of file to mark */
    PGSt_integer      fileType)          /* type of file to mark */

{
    char              firstChar;         /* first character hold */
    char              env[100];          /* environment variable */
    char              builtFile[PGSd_PC_FILE_PATH_MAX];  /* file name */
    char             *sectionLoc;        /* address of file section */
    PGSt_boolean      doneFlag;          /* finished our loop */
    PGSt_uinteger     offSet;            /* section offset in Shm */
    PGSt_PC_File_Shm  fileShm;           /* SHM file structure */
    PGSt_PC_HeaderStruct_Shm headStruct; /* header in shared memory */
    PGSt_SMF_status   whoCalled;         /* user that called this function */
    PGSt_SMF_status   returnStatus;      /* function return variable */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    doneFlag = PGS_FALSE;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Get the header structure from shared memory.
***************************************************************************/
    memcpy((void *) &headStruct,(void *) addrPC,
                        sizeof(PGSt_PC_HeaderStruct_Shm));


    switch (fileType)
    {
 
/***************************************************************************
*    Input file information.
***************************************************************************/
        case PGSd_PC_INPUT_FILE_NAME:
            offSet = headStruct.divPtr[PGSd_PC_INPUT_FILES-1];
            strcpy(env,PGSd_PC_INPUT_FILE_ENVIRONMENT);
            break;
 
/***************************************************************************
*    Output file information.
***************************************************************************/
        case PGSd_PC_OUTPUT_FILE_NAME:
            offSet = headStruct.divPtr[PGSd_PC_OUTPUT_FILES-1];
            strcpy(env,PGSd_PC_OUTPUT_FILE_ENVIRONMENT);
            break;

/***************************************************************************
*    Temporary file information.
***************************************************************************/
        case PGSd_PC_TEMPORARY_FILE:
            offSet = headStruct.divPtr[PGSd_PC_TEMP_INFO-1];
            strcpy(env,PGSd_PC_TEMP_ENVIRONMENT);
            break;
 
/***************************************************************************
*    Intermediate input file information.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_INPUT:
            offSet = headStruct.divPtr[PGSd_PC_INTER_INPUT-1];
            strcpy(env,PGSd_PC_INTER_INPUT_ENVIRONMENT);
            break;
 
/***************************************************************************
*    Intermediate output file information.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_OUTPUT:
            offSet = headStruct.divPtr[PGSd_PC_INTER_OUTPUT-1];
            strcpy(env,PGSd_PC_INTER_OUTPUT_ENVIRONMENT);
            break;
 
/***************************************************************************
*    Support input file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_IN_NAME:
            offSet = headStruct.divPtr[PGSd_PC_SUPPORT_INPUT-1];
            strcpy(env,PGSd_PC_SUPPT_INPUT_ENVIRONMENT);
            break;
 
/***************************************************************************
*    Support output file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_OUT_NAME:
            offSet = headStruct.divPtr[PGSd_PC_SUPPORT_OUTPUT-1];
            strcpy(env,PGSd_PC_SUPPT_OUT_ENVIRONMENT);
            break;
    }  /* end switch */

/***************************************************************************
*    Advance our PCS data location to the area that contains the section
*    of data that we are looking for.
***************************************************************************/
    sectionLoc = (char *) ((unsigned long) addrPC + offSet);

/***************************************************************************
*    Determine if the first character really is a divider.
***************************************************************************/
    memcpy(&firstChar,(char *) sectionLoc,sizeof(char));
    if (firstChar == PGSd_PC_DIVIDER)
    {
        sectionLoc = (char *) ((unsigned long) sectionLoc + sizeof(char));

/***************************************************************************
*    Loop until we find a file that has the same logical ID and does not
*    have the delete flag already set.
***************************************************************************/
        do
        {

/***************************************************************************
*    Check to see if we have hit the next divider yet.
***************************************************************************/
            memcpy(&firstChar,(char *) sectionLoc,sizeof(char));
            if (firstChar == PGSd_PC_DIVIDER)
            {
               returnStatus = PGSPC_W_NO_FILES_EXIST;
               doneFlag = PGS_TRUE;
               break;
            }

/***************************************************************************
*    Get the structure, make sure the index value is a valid entry and 
*    increment the address.
***************************************************************************/
            memcpy((void *) &fileShm,(void *) sectionLoc,
                               sizeof(PGSt_PC_File_Shm));

            if (fileShm.fileStruct.index == (PGSt_PC_Logical) NULL)
            {
               returnStatus = PGSPC_E_INDEX_ERROR_SHM;;
               break;
            }
 
            if (fileShm.fileStruct.index == logicalID)   
            {

/***************************************************************************
*    Build the file name and compare it to the one passed in.
***************************************************************************/
                returnStatus = PGS_PC_BuildFileShm(env,PGSd_PC_TYPE_FILE,
                                                        &fileShm,builtFile);
                if ((returnStatus == PGS_S_SUCCESS) &&
                    (strcmp(builtFile,fileName) == 0))
                {

#ifdef _PGS_THREADSAFE
/***************************************************************************
*    We are changing PCF info so we need to lock for TSF stuff.
***************************************************************************/
                    returnStatus = PGS_TSF_LockIt(PGSd_TSF_PCLOCK);
                    if (PGS_SMF_TestErrorLevel(returnStatus))
                    {
                        return PGSTSF_E_GENERAL_FAILURE;
                    }
#endif

/***************************************************************************
*    We have a match, set the RUNTIME flag, if the DELETE flag is 
*    already set then set the DELETE and RUNTIME flag.  Write the
*    new information back out to shared memory.
***************************************************************************/
                    if (fileShm.flag == PGSd_PC_DELETE_FLAG)
                    {
                        fileShm.flag = PGSd_PC_DELNRUN_FLAG;
                    }
                    else
                    {
                        fileShm.flag = PGSd_PC_RUNTIME_FLAG;
                    }
                    memcpy(sectionLoc,(char *) &fileShm,
                                           sizeof(PGSt_PC_File_Shm));
                    doneFlag = PGS_TRUE;
                }
#ifdef _PGS_THREADSAFE
/***************************************************************************
*    Unlock here.
***************************************************************************/
                    returnStatus = PGS_TSF_UnlockIt(PGSd_TSF_PCLOCK);
                    if (PGS_SMF_TestErrorLevel(returnStatus))
                    {
                        returnStatus = PGSTSF_E_GENERAL_FAILURE;
                    }
#endif
            }

/***************************************************************************
*    Increment shared memory pointer.
***************************************************************************/
            sectionLoc = (char *) 
             ((unsigned long) sectionLoc + sizeof(PGSt_PC_File_Shm));
        }
        while (doneFlag == PGS_FALSE);
    }  /* end if */

/***************************************************************************
*    Our division pointer was not a divider.
***************************************************************************/
    else
    {
        returnStatus = PGSPC_E_INV_DIVPOINTER_SHM;
    }

/***************************************************************************
*    Set our message in the message log and return to the calling
*    function.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_DeleteFileShm()");
    }

    return returnStatus;
}
