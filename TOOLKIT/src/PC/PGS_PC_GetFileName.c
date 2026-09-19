/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetFileName.c

DESCRIPTION:
	Get a file name from a line of data.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	02-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
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
	21-Dec-95 RM Edited code which builds path in case where first
			character is a TILDE to work on an array of 
			char instead of a pointer to char.  This 
			change was mandated by DR ECSed01575.
 
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
	Get a file name from a line of data.
 
NAME:
	PGS_PC_GetPCSDataGetFileName()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetPCSDataGetFileName(
			PGSt_integer		mode,
			char			*line,
			char			*envVar,
			char			*fName);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to obtain a file name from a line of data.
	This line of data has been pulled from the Process Control Status
	information location.
 
INPUTS:
	Name		Description			Units	Min	Max

	mode		context values that will be 
			#define'd in a header file.
			Possible values are:

			PGSd_PC_INPUT_FILE_NAME
			PGSd_PC_INPUT_FILE_ATTRIBUTE
			PGSd_PC_OUTPUT_FILE_NAME
			PGSd_PC_OUTPUT_FILE_ATTRIBUTE
			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_INTERMEDIATE_OUTPUT
			PGSd_PC_SUPPORT_IN_NAME
			PGSd_PC_SUPPORT_IN_ATTR
			PGSd_PC_SUPPORT_OUT_NAME
			PGSd_PC_SUPPORT_OUT_ATTR

	line		The line of data received from 	
			the Process Control Status
			information.

	envVar		environment variable storing 
			path location if not present in
			line of data.

OUTPUTS:
	Name		Description			Units	Min	Max

	fName		The file name with full path 
			attached.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file
	PGSPC_E_ENVIRONMENT_ERROR   environment variable error

EXAMPLES:

C:
	PGSt_SMF_status	ret_value;
	PGSt_PC_Logical	prodID;
	PGSt_int	numFiles;
	char		outstr[PGSd_PC_FILE_PATH_MAX];

	/# getting the name of a file in a product group #/

	prodID = 101;
	numFiles = 1;
	ret_value = PGS_PC_GetPCSData(PGSd_PC_INPUT_FILE_NAME,prodID,
		outstr,&numFiles);

	if (ret_value != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# open the file in "outstr" #/
		}
			.
			.
			.
	EXCEPTION:
		return ret_value;

FORTRAN:
	NONE

NOTES:
	The file name can be the actual product or the file storing the 
	attribute of that product.

	All configuration parameters, except for physical file name(s), will
	have a one-to-one correspondence between a logical value and a  
	parameter value (string).

	In order for this tool to function properly, a valid Process Control
	file will need to be created first.  Please refer to Appendix C 
	(User's Guide) for instructions on how to create such a file.

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
	PGS_SMF_CallerID		 Get value of user variable
	PGS_PC_GetPCSDataGetRequest      Get requested data
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message
        PGS_PC_GetPCEnv                  Get default file location

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetPCSDataGetFileName(        /* get a file name */
    int         mode,                /* input mode to search for */
    char       *line,                /* line read from file */
    char       *envVar,              /* environment variable name */
    char       *fName)               /* full file and path name */
{
    char       *tem;                 /* temporary hold from getenv() */
    char 	tempPath[PGSd_PC_PATH_LENGTH_MAX];  /* temporary path variable */
    char       *dum;                 /* temp hold from strrchr() */
    char 	pathHold[PGSd_PC_PATH_LENGTH_MAX];  /* temporary path variable */
    char        fileN[PGSd_PC_FILE_NAME_MAX]; /* file name */
    char        buf[PGS_SMF_MAX_MSG_SIZE];  /* message buffer */
    char        msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    int         numDelimiters;       /* number of delimiters */
    int         pos;                 /* position to start prepending to */
    PGSt_SMF_status  whoCalled;      /* user who called this function */
    PGSt_SMF_status  returnStatus;   /* function return */

/***************************************************************************
*    Initialize variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    If the user wants an attribute the set the delimiter count 
*    accordingly.
***************************************************************************/
    if ((mode == PGSd_PC_INPUT_FILE_ATTRIBUTE) || 
        (mode == PGSd_PC_OUTPUT_FILE_ATTRIBUTE) ||
        (mode == PGSd_PC_SUPPORT_IN_ATTR) ||
        (mode == PGSd_PC_SUPPORT_OUT_ATTR))
    {
        numDelimiters = 5;
    }
    else
    {
        numDelimiters = 1;
    }

/***************************************************************************
*    Parse out the file name from the line passed in.
***************************************************************************/
    returnStatus = PGS_PC_GetPCSDataGetRequest(numDelimiters,
                                             PGSd_PC_DELIMITER,line,fileN);

    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*    Make sure the user is not trying to pull a fast one on us.
***************************************************************************/
        if ((strlen(fileN)) > (size_t) PGSd_PC_FILE_NAME_MAX)
        {
            returnStatus =  PGSPC_E_LINE_FORMAT_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                      "PGS_PC_GetPCSDataGetFileName()");
            }
        }

/***************************************************************************
*    See if there is a path in the line, if there is then concatenate
*    the file name to it, otherwise we will get the path from the
*    environment variable and concatenate the file name to it.
***************************************************************************/
        else
        {
            returnStatus = PGS_PC_GetPCSDataGetRequest(2,PGSd_PC_DELIMITER,
                                                       line,fName);

            if (returnStatus == PGSPC_W_NO_DATA_PRESENT)
            {
                returnStatus = PGS_PC_GetPCEnv(envVar,tempPath);
                if (returnStatus == PGS_S_SUCCESS)
                {
                    strcpy(fName,tempPath);
                    returnStatus = PGS_S_SUCCESS;
                }
                else
                {
                    returnStatus = PGSPC_E_ENVIRONMENT_ERROR;
                    if (whoCalled != PGSd_CALLERID_SMF)
                    {
                        PGS_SMF_GetMsgByCode(returnStatus,msg);
                        sprintf(buf,msg,envVar);
                        PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                              "PGS_PC_GetPCSDataGetFileName()");
                    }
                }
            }

/***************************************************************************
*    If there is a path and the first character of the path is a tilde (~)
*    then we are going to append the path to the path specified in the
*    environment PGSHOME.
***************************************************************************/
            if ((fName[0] == PGSd_PC_TILDE) && 
                (returnStatus == PGS_S_SUCCESS))
            {
                tem = getenv(PGSd_PC_PGSHOME_ENVIRONMENT);
                if (tem)
                {
                    strcpy(tempPath,tem);
                    if (fName[1] == PGSd_PC_SLASH)
                    {
                        pos = 2;
                    }
                    else
                    {
                        pos = 1;
                    }
                    dum = strrchr(tempPath,PGSd_PC_SLASH);
                    if ((dum != (char *) NULL) && (strlen(dum) > (size_t) 1))
                    {
                        strcat(tempPath,PGSd_PC_STRING_SLASH);
                    }
		    else if ((dum == (char*) NULL) && (strlen(tempPath) != 0U))
		    {
                        strcat(tempPath,PGSd_PC_STRING_SLASH);
		    }
		    
                    strcpy(pathHold,tempPath);
                    strcat(pathHold,fName+pos);
                    strcpy(fName,pathHold);
                }
                else
                {
                    returnStatus = PGSPC_E_ENVIRONMENT_ERROR;
                    if (whoCalled != PGSd_CALLERID_SMF)
                    {
                        PGS_SMF_GetMsgByCode(returnStatus,msg);
                        sprintf(buf,msg,envVar);
                        PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                          "PGS_PC_GetPCSDataGetFileName()");
                    }
                }
            }

/***************************************************************************
*    Make sure the user is not trying to pull a fast one on us before
*    we concatenate the file name with the path.
***************************************************************************/
            if ((strlen(fName)) > (size_t) PGSd_PC_PATH_LENGTH_MAX)
            {
                returnStatus = PGSPC_E_LINE_FORMAT_ERROR;
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_GetMsgByCode(returnStatus,msg);
                    sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                    PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                          "PGS_PC_GetPCSDataGetFileName()");
                }
            }
            else
            {
                if (strlen(fName) > (size_t) 0)
                {
                    dum = strrchr(fName,PGSd_PC_SLASH);
                    if ((dum != (char *) NULL) && (strlen(dum) > (size_t) 1))
                    {
                        strcat(fName,PGSd_PC_STRING_SLASH);
                    }
		    else if (dum == (char*) NULL)
		    {
                        strcat(fName,PGSd_PC_STRING_SLASH);
		    }
                }
                strcat(fName,fileN);
            }
        } /* end else */
    } /* end if */

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
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetPCSDataGetFileName()");
            }
            break;
    }
    return returnStatus;
}
