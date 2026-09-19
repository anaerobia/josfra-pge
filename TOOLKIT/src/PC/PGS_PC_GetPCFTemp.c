/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetPCFTemp.c

DESCRIPTION:
	This file contains the function PGS_PC_GetPCFTemp().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	14-Dec-95 RM Initial Version
	11-Mar-96 RM Fixed code setting error message for an environment
			error.  This went along with DR ECSed01815.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Get location of a temporary file used to update the PCF.
 
NAME:  
	PGS_PC_GetPCFTemp()

SYNOPSIS:

C:
	#include <PGS_PC.h>

	PGSt_SMF_status
	PGS_PC_GetPCFTemp(
		char		tempFileName[]);

FORTRAN:
	NONE

DESCRIPTION:
	This function will receive an input string that had been passed in to
	a Com tool and determine if it contains all digits and convert it
	to a number.
 
INPUTS:
        Name            Description                     Units   Min     Max

	NONE

OUTPUTS:
        Name            Description                     Units   Min     Max

	tempFileName	The full path/file name of
			the temporary file used to 
			update the PCF.

RETURNS:
	PGSPC_E_ENVIRONMENT_ERROR
	PGS_S_SUCCESS

EXAMPLES:
	char		tempFileName[PGSd_PC_FILE_PATH_MAX];
	PGSt_SMF_status	returnStatus;

	/# get the full path/file name of the temporary file #/
	
	returnStatus = PGS_PC_GetPCFTemp(tempFileName);

	if (returnStatus == PGS_S_SUCCESS)
	{
	/# continue processing with tempFileName #/
	}
	else
	{
		goto EXCEPTION;
	}
	.
	.	
	.

	EXCEPTION:
		return returnStatus;

NOTES:
	NONE

REQUIREMENTS:
	NONE

DETAILS:
	NONE	

GLOBALS:
	NONE

FILES:
	NONE	

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_GetPCFTemp(                       /* get temporay file name */
    char                tempFileName[])  /* temporay file name */
{
    char               *pcfFile;         /* actual PCF */
    char               *parsed;          /* parsed PCF */
    char                pid_string[20];  /* process ID (PID) string */
    char                buf[PGS_SMF_MAX_MSG_SIZE];    /* message buffer */
    char                msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */

    PGSt_SMF_status     whoCalled;       /* user that called this function */
    PGSt_SMF_status     returnStatus;    /* return from functions */

    
/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    whoCalled = PGS_SMF_CallerID();
/***************************************************************************
*    We want to place the temporary file in the same directory as the
*    PCF, this way when we copy back to the PCF we will not be going
*    across filesystems.  So, here we get the location of the PCF.
***************************************************************************/
    pcfFile = getenv(PGSd_PC_INFO_FILE_ENVIRONMENT);
    if (!pcfFile)
    {
        returnStatus = PGSPC_E_ENVIRONMENT_ERROR;
        if (whoCalled != PGSd_CALLERID_SMF)
        {
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            sprintf(buf,msg,PGSd_PC_INFO_FILE_ENVIRONMENT);
            PGS_SMF_SetDynamicMsg(returnStatus,buf,"PGS_PC_GetPCFTemp()");
        }
    }

/***************************************************************************
*    We received the location of the PCF.  Now let's parse out the 
*    directory and append our temporary file name to it.
***************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {
	strcpy(tempFileName,pcfFile);
        parsed = strrchr(tempFileName,PGSd_PC_SLASH);
	if ( parsed == (char *) NULL)
	{
	    parsed = tempFileName;
	}
        else if ( (++parsed)[0] == PGSd_PC_CHAR_NULL )
        {
            returnStatus = PGSPC_E_ENVIRONMENT_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                sprintf(buf,msg,PGSd_PC_INFO_FILE_ENVIRONMENT);
                PGS_SMF_SetDynamicMsg(returnStatus,buf,"PGS_PC_GetPCFTemp()");
            }
        }
	parsed[0] = PGSd_PC_CHAR_NULL;
	strcat(tempFileName,TEMPFILENAME);
	sprintf(pid_string, ".%u", getpid());
	strcat(tempFileName, pid_string);       

    }

/***************************************************************************
*    Return to calling function.
***************************************************************************/
    return returnStatus;
}
