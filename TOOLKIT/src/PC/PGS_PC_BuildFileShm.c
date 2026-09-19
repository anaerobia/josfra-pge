/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_BuildFileShm.c

DESCRIPTION:
	This file contains the function PGS_PC_BuildFileShm().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	09-Dec-94 RM Initial version
	06-Apr-95 RM Updated for TK5.  Installed PGS_PC_GetPCEnv()
			to replace getenv() calls that retrieve
			environment variables which contain default
			file locations.
	27-Apr-95 RM Updated to added functionality to check for 
			existence of TILDE from default location.
        07-Jul-95 RM Added checks around strrchr() in case the slash (/)
                        does not exist in the path.  This case should
                        never happen, but if it would it could case a
                        memory problem.  This change was mandated by
                        DR ECSed00782.
	21-Dec-95 RM Edited code that builds path when TILDE is found
			to work on array of char instead of char 
			pointer.  This change was mandated by DR
			ECSed01575.

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
	Construct the file and path name received from shared memory data
	structure.
 
NAME:
	PGS_PC_BuildFileShm()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_BuildFileShm(
			char 			*env,
			PGSt_integer		fileFlag,
			PGSt_PC_File_Shm	*fileData,
			char			*builtFile);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to extract file information from a structure
	that had been retrieved from shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	env		environment variable storing 
			default path.

	fileFlag	file type to retrieve.  Possible
			entries are:
				PGSd_PC_TYPE_ATTR
				PGSd_PC_TYPE_FILE

	fileData	structure containing file data
			from which we need to build 
			the string containing full
			path and file name.

OUTPUTS:
	Name		Description			Units	Min	Max

	builtFile	full file and path built from 
			data received from shared memory.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_ENVIRONMENT_ERROR   environment variable not set
	PGSPC_W_NO_DATA_PRESENT     requested data is not available

EXAMPLES:

C:
	char		*env = "ENVIRONMENT_VAR_A";
	char		builtFile[PGSd_PC_FILE_PATH_MAX];
        PGSt_PC_File_Shm fileData;
	PGSt_SMF_status	returnStatus;

	/# assuming that the proper structure was retrieved from shared memory #/

	/# build the file name from the structure #/

	returnStatus = PGS_MEM_BuildFileShm(env,PGSd_PC_TYPE_FILE,
                                         &fileData,builtFile);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;

	else
		{
		 /# continue with file operations #/
		}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	All configuration parameters, except for physical file name(s), will
	have a one-to-one correspondence between a logical value and a  
	parameter value (string).

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
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_TestStatusLevel		 Determine the level of an 
					 error/status code
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message
	PGS_PC_GetPCEnv                  Get default file location

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_BuildFileShm( 	                 /* build file string from shm */
    char             *env,               /* environment variable */
    PGSt_integer      fileFlag,          /* file type to get */
    PGSt_PC_File_Shm *fileData,          /* struct with file data from shm */
    char             *builtFile)         /* built file name */

{
    char            *resEnv;             /* resolved environment from getenv() */
    char            tempName[PGSd_PC_FILE_NAME_MAX]; /* temp hold for file name */
    char            tempPath[PGSd_PC_PATH_LENGTH_MAX]; /* temp hold for path */
    char            envHold[PGSd_PC_PATH_LENGTH_MAX]; /* temp hold for path */
    char            buf[PGS_SMF_MAX_MSG_SIZE];  /* message buffer */
    char            msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char            *dum;                /* temp hold from strrchr() */
    int             pos;                 /* file name position */
    PGSt_SMF_status whoCalled;           /* user that called this function */
    PGSt_SMF_status returnStatus;        /* function return variable */

/***************************************************************************
*    Initialize return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Determine if the user wants to retrieve the file name or the 
*    attribute location.
***************************************************************************/
    if (fileFlag == PGSd_PC_TYPE_FILE)
    {
        strcpy(tempName,fileData->fileStruct.fileName); 
    }
    else
    {
        strcpy(tempName,fileData->fileStruct.attributeLoc); 
    }

    strcpy(tempPath,fileData->fileStruct.path);

/***************************************************************************
*    If the file (or attribute file) is NULL then there was never any
*    data there when we ran through PGS_PC_InitCom.  This may not be bad
*    since we may have been looking for an attribute file which is not
*    mandatory.
***************************************************************************/
    if (tempName[0] != PGSd_PC_CHAR_NULL)
    {

/***************************************************************************
*    If there is no path in the structure then we know the user wants us
*    to use what is in the environment variable.
***************************************************************************/
        if (tempPath[0] == PGSd_PC_CHAR_NULL)
        {
            returnStatus = PGS_PC_GetPCEnv(env,tempPath);
            if (returnStatus == PGS_S_SUCCESS)
            {
                dum = strrchr(tempPath,PGSd_PC_SLASH);
                if ((dum != (char *) NULL) && (strlen(dum) > (size_t) 1))
                {
                    strcat(tempPath,PGSd_PC_STRING_SLASH);
                }
                strcpy(builtFile,tempPath);
                dum = strrchr(builtFile,PGSd_PC_SLASH);
                if ((dum != (char *) NULL) && (strlen(dum) > (size_t) 1))
                {
                    strcat(builtFile,PGSd_PC_STRING_SLASH);
                }
            }

/***************************************************************************
*    If the environment variable was not set then we have a problem.
***************************************************************************/
            else
            {
                returnStatus = PGSPC_E_ENVIRONMENT_ERROR;
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_GetMsgByCode(returnStatus,msg);
                    sprintf(buf,msg,env);
                    PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                          "PGS_PC_BuildFileShm()");
                }
            }
        }  /* end if */

/***************************************************************************
*    If the first character of the path is a tilde (~) then what we 
*    want to do is append the path to what is in the environment variable
*    "PGSHOME."
***************************************************************************/
        if ((tempPath[0] == PGSd_PC_TILDE) &&
            (returnStatus == PGS_S_SUCCESS))
        {
            resEnv = getenv(PGSd_PC_PGSHOME_ENVIRONMENT);
            if (resEnv)
            {
                strcpy(envHold,resEnv);
                if (tempPath[1] == PGSd_PC_SLASH)
                {
                    pos = 2;
                }
                else
                {
                    pos = 1;
                }
                dum = strrchr(envHold,PGSd_PC_SLASH);
                if ((dum != (char *) NULL) && (strlen(dum) > (size_t) 1))
                {
                    strcat(envHold,PGSd_PC_STRING_SLASH);
                }
                strcpy(builtFile,envHold);
                strcat(builtFile,tempPath+pos);
                dum = strrchr(builtFile,PGSd_PC_SLASH);
                if ((dum != (char *) NULL) && (strlen(dum) > (size_t) 1))
                {
                    strcat(builtFile,PGSd_PC_STRING_SLASH);
                }

            }

/***************************************************************************
*    The environment "PGSHOME" was not set, we have more problems.
***************************************************************************/
            else
            {
                returnStatus = PGSPC_E_ENVIRONMENT_ERROR;
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_GetMsgByCode(returnStatus,msg);
                    sprintf(buf,msg,env);
                    PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                      "PGS_PC_BuildFileShm()");
                }
            }

        }  /* end else */

/***************************************************************************
*    Just use the path that is in the structure.
***************************************************************************/
        else
        {
            dum = strrchr(tempPath,PGSd_PC_SLASH);
            if ((dum != (char *) NULL) && (strlen(dum) > (size_t) 1))
            {
                strcat(tempPath,PGSd_PC_STRING_SLASH);
            }
            strcpy(builtFile,tempPath);
            dum = strrchr(builtFile,PGSd_PC_SLASH);
            if ((dum != (char *) NULL) && (strlen(dum) > (size_t) 1))
            {
                strcat(builtFile,PGSd_PC_STRING_SLASH);
            }
        }
        if (returnStatus == PGS_S_SUCCESS)
        {
            strcat(builtFile,tempName);
        }
    }  /* end if */

/***************************************************************************
*    This is where we set the warning telling the user that there was not
*    a file name in the requested field.
***************************************************************************/
    else
    {
        returnStatus = PGSPC_W_NO_DATA_PRESENT;
    }

/***************************************************************************
*    Set the message in the log and go back to calling function.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
           break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_BuildFileShm()");
            }
            break;
    }

    return returnStatus;
}
