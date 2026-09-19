/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_SkipCheck.c

DESCRIPTION:
	This file contains the function PGS_PC_PutPCSDataSkipCheck().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	05-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	22-Dec-94 RM Added functionality to put a delete flag in the 
			file and not remove the line from the file.
	05-Apr-95 RM Updated for TK5.  Added code to skip line in PCF
			that contain the default file location.
	19-Apr-95 RM Updated for TK5.  Added call to PGS_PC_CheckFlags()
			to modularize checking for existence of file
			flags.  Added code to insert DELETE and RUNTIME
			flag.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Remove information in the Process Control Status file.

NAME:  
	PGS_PC_PutPCSDataSkipCheck()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status
		PGS_PC_PutPCSDataSkipCheck(
			PGSt_PC_Logical		logicalID,
			FILE			*locationPCS[NUMFILES]);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to delete information from the Process
	Control Status information file.  It performs this by checking
	for the existence of a Product ID, if the ID exists then that
	line is not written out to the temporary file.
 
INPUTS:
	Name		Description			Units	Min	Max

	logicalID	The information to be written 
			out to the Process Control
			File.  The information is 
			received by this function in
			the form of a PGSt_PC_Logical.

	locationPCS	Pointers to the files containing
			Process Control Status information.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_EXIST      link number does not have the data that 
                                    mode is requesting
	PGSPC_E_FILE_READ_ERROR     error reading input file
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file

EXAMPLES:

C:
	PGSt_SMF_status		returnStatus;
	PGSt_PC_Logical		logicalID;
	FILE			*locationPCS[NUMFILES];

	logicalID = 101;
		.
		.
		.

	/# assuming the PCS files opened successfully #/

	returnStatus = PGS_PC_PutPCSDataSkipCheck(logicalID,locationPCS);
	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		/# inform user that the data is deleted #/
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
	This tool writes to "tempPCS.fil" and reads from the file 
	defined in the environment variable "PGS_PC_INFO_FILE".
 
FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message
	PGS_PC_CheckFlags		 Check line of file location data
					 for existence of flags 

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_PutPCSDataSkipCheck(                 /* skip line of data */
    PGSt_PC_Logical logicalID,              /* index file to search for */
    FILE           *locationPCS[NUMFILES])  /* input file */
{
    char            cnum[33];               /* character number */
    char            dummy[PGSd_PC_LINE_LENGTH_MAX];  /* line from input file */
    char            buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char            msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    int             endFlag;                /* task is over */
    int             linPos;                 /* line position */
    int             newPos;                 /* position of newline char */
    PGSt_integer    flags;                  /* flags in line */
    PGSt_PC_Logical num;                    /* number read */
    PGSt_SMF_status whoCalled;              /* user that called this function */
    PGSt_SMF_status returnStatus;           /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    endFlag = 0;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Loop until we know we have finished the task.
***************************************************************************/
    while (!endFlag)
    {

/***************************************************************************
*    Read a line from the input file, if it is the EOF then we have
*    a problem.
***************************************************************************/
        if ((fgets(dummy,PGSd_PC_LINE_LENGTH_MAX,
                   locationPCS[PCSFILE])) == NULL)
        {
            returnStatus = PGSPC_E_FILE_READ_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(PGSPC_E_FILE_READ_ERROR,msg);
                sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                         "PGS_PC_PutPCSDataSkipCheck()");
            }
            break;
        }

/***************************************************************************
*    If the first character is a comment just write it out to the 
*    outfile and go to the top of the loop.
***************************************************************************/
        else if (dummy[0] == PGSd_PC_COMMENT)
        {
            fputs(dummy,locationPCS[TEMPFILE]);
            continue;
        }

/***************************************************************************
*    If the first character is a DEFAULT_LOC flag just write it 
*    out to the outfile and go to the top of the loop.
***************************************************************************/
        else if (dummy[0] == PGSd_PC_DEFAULT_LOC)
        {
            fputs(dummy,locationPCS[TEMPFILE]);
            continue;
        }

/***************************************************************************
*    If the first character is a divider then we did not find a match.
*    Let's send a warning back to the user (they may not care).
***************************************************************************/
        else if (dummy[0] == PGSd_PC_DIVIDER)
        {
            fputs(dummy,locationPCS[TEMPFILE]); 
            endFlag = 1;
            returnStatus = PGSPC_W_NO_FILES_EXIST;
        }

/***************************************************************************
*    More than likely we have a valid line here, let's strip out the 
*    index number at the beginning.  We should have nothing but digits
*    up until the first delimiter.
***************************************************************************/
        else
        {
            linPos = 0;
            while (dummy[linPos] != PGSd_PC_DELIMITER)
            {

/***************************************************************************
*    If it is a digit load it into another character array.
***************************************************************************/
                if ((dummy[linPos] >= PGSd_PC_LOWDIGIT) ||
                    (dummy[linPos] <= PGSd_PC_HIDIGIT))
                {
                    cnum[linPos] = dummy[linPos];
                    linPos++;
                }

/***************************************************************************
*    If it is not a digit we have a problem that needs attention.
***************************************************************************/
                else
                {
                    returnStatus = PGSPC_E_LINE_FORMAT_ERROR;
                    if (whoCalled != PGSd_CALLERID_SMF)
                    {
                        PGS_SMF_GetMsgByCode(PGSPC_E_LINE_FORMAT_ERROR,msg);
                        sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                        PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                          "PGS_PC_PutPCSDataSkipCheck()");
                    }
                    endFlag = 1;
                    break;
                }
            }   /* end while */

/***************************************************************************
*    Either we hit a delimiter or a non-digit character, if the end
*    task flag is not set then we are OK.  Convert the character 
*    number to integer and see if it is the same as the one passed in.
***************************************************************************/
            if (!endFlag)
            {
                cnum[linPos] = '\0';
                num = (PGSt_PC_Logical) atol(cnum);

/***************************************************************************
*    If the number is the same then this is the line we need to mark
*    as to be deleted.  If it is already marked as to be deleted just  
*    skip and continue looking.
***************************************************************************/
                 returnStatus = PGS_PC_CheckFlags(dummy,&flags);
                if ((num == logicalID) && 
                    (flags != PGSd_PC_HAS_DELETE) && 
                    (flags != PGSd_PC_HAS_DELNRUN))
                {

/***************************************************************************
*    If the file is marked as a RUNTIME file then mark it as a DELETE
*    and RUNTIME file.
***************************************************************************/
                    if (flags == PGSd_PC_HAS_RUNTIME)
                    {
                        newPos = strlen(dummy) - 
                                 strlen(strchr(dummy,PGSd_PC_RUNTIME_FLAG));
                        dummy[newPos] = '\0'; 
                        strcat(dummy,PGSd_PC_DELNRUN_STR);
                    }
                    else
                    {
                        newPos = strlen(dummy) - 
                                 strlen(strchr(dummy,PGSd_PC_NEWLINE));
                        dummy[newPos] = '\0'; 
                        strcat(dummy,PGSd_PC_DELETE_STRING);
                    }
                    endFlag = 1;
                }
                fputs(dummy,locationPCS[TEMPFILE]);
                if (endFlag == 1)
                {
                    fputs("\n",locationPCS[TEMPFILE]);
                }

            }   /* end if */
        }   /* end else */
    }   /* end while */

/***************************************************************************
*    Write a message to the log and return.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_PutPCSDataSkipCheck()"); 
            }
            break;
    }
    return returnStatus;
}
