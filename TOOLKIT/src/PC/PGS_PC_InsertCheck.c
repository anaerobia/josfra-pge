/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_InsertCheck.c

DESCRIPTION:
	This file contains the function PGS_PC_PutPCSDataInsertCheck().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	05-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	13-Feb-95 RM Added variable "divLine" and copied line to it
			when a divider line has been read.  This 
			fix was mandated by DR ECSed00696.
	14-Feb-95 RM Added check for DELETE_FLAG so a reference flagged
			for deletion will not be overwritten.  This
			fix was mandated by DR ECSed00710.
	05-Apr-95 RM Updated for TK5.  Added functionality to skip 
			lines containing default file location.
	19-Apr-95 RM Updated for TK5.  Added function call to 
			PGS_PC_CheckFlags() to modularize the checking
			of DELETE and RUNTIME flags.
        05-Jan-96 RM Updated for TK6.  Added Universal References to
                        file entries for files.  Edited the sprintf() 
			calls to reflect the universal reference 
			instead of the unused bufferSize.

END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Insert information in the Process Control Status file while checking
	for the existence of information already contianed under the Product
	ID.
 
NAME:  
	PGS_PC_PutPCSDataInsertCheck()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status
		PGS_PC_PutPCSDataInsertCheck(
			FILE			*locationPCS[NUMFILES],
			PGSt_PC_File_Struct	*PCS_data);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to insert information in the Process 
	Control Status file while checking for the existence of information 
	already contianed under the Product ID.  If there is information
	currently listed under the Product ID then that information is
	overwritten with what has been passed in.  If no information is
	listed under the Product ID then a new entry is made into the
	Process Control Status information file.
 
INPUTS:
	Name		Description			Units	Min	Max

	locationPCS	Pointer to files contain PCS data.

	PCS_data	The information to be written 
			out to the Process Control
			File.  The information is 
			received by this function in 
			the form of a PGSt_PC_File_Struct *.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_FILE_READ_ERROR     error reading input file
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file

EXAMPLES:

C:
	PGSt_SMF_status		returnStatus;
	PGSt_PC_File_Struct	*fileStruct;
	FILE			*locationPCS[NUMFILES];

	/# fill the file structure with the proper information #/
	/# even though some of these fields are optional they must #/
	/# be initialized #/
	fileStruct->index = 101;
	strcpy(fileStruct->name,"temp.fil");
	strcpy(fileStruct->path,"/u/temp");
	strcpy(fileStruct->attributeLoc,"MODIS-ATTRIB.fil");
	strcpy(fileStruct->universalRef,"REFERENCE-1A");
	fileStruct->size = 0;
	fileStruct->entries = 0;
		.
		.
		.
	/# assuming that the PCS files opened successfully #/

	returnStatus = PGS_PC_PutPCSDataInsertCheck(locationPCS,fileStruct);
	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		/# let the user know that the information is ready to be used #/
		}
		.
		.
		.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	The user (application programmer) must ensure that all data fields 
	listed as optional in the Process Control Inormation file are 
	initialized before being passed to this function.  Failing to 
	initialize any unused data will give undefined results.

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
	This tool writes to "tempPCS.fil" and reads from the file defined 
	in the environment variable "PGS_PC_INFO_FILE".
 
FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message
	PGS_PC_CheckFlags		 Check for the existence of any
					 flags on a line of file data 

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_PutPCSDataInsertCheck(               /* insert data */
    FILE          *locationPCS[NUMFILES],   /* input file */
    PGSt_PC_File_Struct *PCS_data)          /* information to be written */
{
    char            cnum[33];               /* character number */
    char            dummy[PGSd_PC_LINE_LENGTH_MAX];  /* line from input file */
    char            divLine[PGSd_PC_LINE_LENGTH_MAX];  /* copy of divider line */
    char            buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char            msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    int             endFlag;                /* task is over */
    int             linPos;                 /* line position */
    int             num;                    /* number read */
    PGSt_integer    flags;                  /* flags in line */
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
                                        "PGS_PC_PutPCSDataInsertCheck()");
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
*    If the first character is a DEFAULT_LOC flag, just write it out
*    to the outfile and go to the top of the loop.
***************************************************************************/
        else if (dummy[0] == PGSd_PC_DEFAULT_LOC)
        {
            fputs(dummy,locationPCS[TEMPFILE]);
            continue;
        }

/***************************************************************************
*    If the first character is a divider then we did not find a match
*    so we need to write out the line to the outfile and then put in
*    a divider.
***************************************************************************/
        else if (dummy[0] == PGSd_PC_DIVIDER)
        {
            strcpy(divLine,dummy);
            sprintf(dummy,"%u|%s|%s|%u|%s|%s|%u\n",PCS_data->index,
                    PCS_data->fileName,PCS_data->path,PCS_data->size,
                    PCS_data->universalRef,PCS_data->attributeLoc,
                    PCS_data->entries);
            fputs(dummy,locationPCS[TEMPFILE]); 
            fputs(divLine,locationPCS[TEMPFILE]); 
            endFlag = 1;
        }

/***************************************************************************
*    More than likely we have a valid line here, let's strip out the 
*    index number at the beginning.  We should have nothing but digits
*    up until the first delimiter.  First, we need to determine if the
*    line has been flagged for deletion.
***************************************************************************/
        else
        {

/***************************************************************************
*    If the DELETE_FLAG or the DELETE and RUNTIME flag is present we know
*    that this reference has been flagged for deletion.
***************************************************************************/
            returnStatus = PGS_PC_CheckFlags(dummy,&flags);
            if ((flags != PGSd_PC_HAS_DELETE) && (flags != PGSd_PC_HAS_DELNRUN))
            {
                linPos = 0;
                while (dummy[linPos] != PGSd_PC_DELIMITER)
                {

/***************************************************************************
*    If it is a digit load it into another character array.
***************************************************************************/
                    if ((dummy[linPos] >= PGSd_PC_LOWDIGIT) &&
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
                                          "PGS_PC_PutPCSDataInsertCheck()");
                        }
                        endFlag = 1;
                        break;
                    }
                }        /* end while */

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
*    If the number is the same then recreate the line with the 
*    information passed in and set the end task flag.
***************************************************************************/
                    if (num == PCS_data->index)
                    {
                        sprintf(dummy,"%u|%s|%s|%u|%s|%s|%u\n",
                                PCS_data->index,PCS_data->fileName,PCS_data->path,
                                PCS_data->size,PCS_data->universalRef,
                                PCS_data->attributeLoc,PCS_data->entries);
                        endFlag = 1;
                    }

/***************************************************************************
*    Write the line out to the file.
***************************************************************************/
                }
            }      /* end if (len != 2) */
            fputs(dummy,locationPCS[TEMPFILE]); 
        }        /* end else */
    }        /* end while */
 
/***************************************************************************
*    Write out to the message log and leave the function.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_PutPCSDataInsertCheck()");
            }
            break;
    }
    return returnStatus;
}
