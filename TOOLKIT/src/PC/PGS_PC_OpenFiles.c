/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_OpenFiles.c

DESCRIPTION:
	This file contains the function PGS_PC_PutPCSData().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	05-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	15-Dec-95 RM Work for TK6.  Added functionality to allow the
			Process Control Tools to work across filesystems.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Opens the files necessary for the update of PCS data.
 
NAME:  
	PGS_PC_PutPCSDataOpensFiles()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status
		PGS_PC_PutPCSDataOpensFiles(
			FILE			*locationPCS[NUMFILES]);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to open the files necessary to perform
	an update of the Process Control Status Information.  There
	are two files that would need to be opened.  The first is 
	defined in the environment variable "PGS_PC_INFO_FILE" and 
	the second is "tempPCS.fil".  The information is written to
	the temporary file and then renamed as the permanent file.
 
INPUTS:
	Name		Description			Units	Min	Max

	locationPCS	File pointers storing the location
			of the PCS data and the temporary
			file.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_FILE_OPEN_ERROR     error opening input file

EXAMPLES:

C:
	PGSt_SMF_status		returnStatus;
	FILE			*locationPCS[NUMFILES];

		.
		.
		.

	/# open the PCS files to edit the data #/

	returnStatus = PGS_PC_PutPCSDataOpenFiles(locationPCS);
	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		/# perform necessary operations on the files #/
		}
		.
		.
		.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	In oreder for this tool to function properly, a valid Process 
	Control file will need to be created first.  Please refer to 
	Appendix C (User's Guide) for instructions on how to create such 
	a file.

REQUIREMENTS:
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	No globals accessed.

FILES:
	This tool creates and opens "tempPCS.fil" and the file defined 
	in the environment variable "PGS_PC_INFO_FILE".
 
FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_PC_GetPCSDataOpenPCSFile     Open input and temporary files
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message
	PGS_PC_GetPCFTemp		 Get location of temporary file

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_PutPCSDataOpenFiles(                   /* open PCS files for updating */
    FILE              *locationPCS[NUMFILES]) /* input mode */
{
    char                buf[PGS_SMF_MAX_MSG_SIZE];    /* message buffer */
    char                msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char                tempFileName[PGSd_PC_FILE_PATH_MAX]; /* temp file name */
    PGSt_SMF_status     whoCalled;           /* user that called this function */
    PGSt_SMF_status     returnStatus;        /* function return */

/***************************************************************************
*    Initialize status.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Open the input file.
***************************************************************************/
    returnStatus = PGS_PC_GetPCSDataOpenPCSFile(&locationPCS[PCSFILE]);
    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*    Get the full path/file name of the output file and then open it.
***************************************************************************/
        returnStatus = PGS_PC_GetPCFTemp(tempFileName);
        if (returnStatus == PGS_S_SUCCESS)
        {
            if ((locationPCS[TEMPFILE] = fopen(tempFileName,"w")) == NULL)
            {
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_GetMsgByCode(PGSPC_E_FILE_OPEN_ERROR,msg);
                    sprintf(buf,msg,TEMPFILENAME);
                    PGS_SMF_SetDynamicMsg(PGSPC_E_FILE_OPEN_ERROR,buf,
                                     "PGS_PC_OpenFiles()");
                }
                return PGSPC_E_FILE_OPEN_ERROR;
            }
        }
    }   /* end if */

/***************************************************************************
*    Set the message log and leave the function.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_OpenFiles()");
    }
    return returnStatus;
}
