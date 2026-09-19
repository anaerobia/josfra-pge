/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_MarkAtTerm.c

DESCRIPTION:
	This file contains the function PGS_PC_PutPCSDataMarkAtTerm().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	17-Dec-94 RM Initial version
	05-Apr-95 RM Updated for TK5.  Added code to skip lines 
			beginning with the DEFAULT_LOC flag.
        05-Jan-96 RM Updated for TK6.  Added Universal References to
                        file entries for files.  Edited sprintf() to
			reflect new variable universalRef which replaces
			the unused bufferSize.
 
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
	PGS_PC_PutPCSDataMarkAtTerm()

SYNOPSIS:

C:
	#include <PGS_PC.h>

	PGSt_SMF_status
	PGS_PC_PutPCSDataMarkAtTerm(
		FILE			*locationPCS[NUMFILES],
		PGSt_integer		mode,
		PGSt_PC_File_Struct	*PCS_data,
		char			*mark);

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

	mode		Type of operation.

	PCS_data	The information to be written 
			out to the Process Control
			File.  The information is 
			received by this function in 
			the form of a PGSt_PC_File_Struct *.

	mark		Type of mark to place in on PCS
			data in PCF.

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
	PGSt_PC_File_Struct	*PCS_dat;
	char			*mark = "D";
	FILE			*locationPCS[NUMFILES];

	/# fill the file structure with the proper information #/
	/# even though some of these fields are optional they must #/
	/# be initialized #/
	PCS_data->index = 101;
	strcpy(PCS_data->name,"temp.fil");
	strcpy(PCS_data->path,"/u/temp");
	strcpy(PCS_data->attributeLoc,"MODIS-ATTRIB.fil");
	strcpy(PCS_data->universalRef,"REFERENCE-1A");
	PCS_data->size = 0;
	PCS_data->entries = 0;
		.
		.
		.
	/# assuming that the PCS files opened successfully #/

	returnStatus = PGS_PC_PutPCSDataMarkAtTerm(locationPCS,mode,PCS_data,mark);
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

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_PutPCSDataMarkAtTerm(                /* mark data in term program */
    FILE          *locationPCS[NUMFILES],   /* input file */
    PGSt_integer   mode,                    /* mode location */
    PGSt_PC_File_Struct *PCS_data,          /* information to be written */
    char          *mark)                    /* mark at end */
{
    char            cnum[33];               /* character number */
    char            dummy[PGSd_PC_LINE_LENGTH_MAX];  /* line from input file */
    char            buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char            msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char            fromAscii[PGSd_PC_FILE_PATH_MAX]; /* file/path name */
    char            fromShm[PGSd_PC_FILE_PATH_MAX]; /* file/path name */
    int             endFlag;                /* task is over */
    int             linPos;                 /* line position */
    int             num;                    /* number read */
    PGSt_PC_File_Shm tempShm;               /* temp struct for file SHM data */
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
*    If the first character is a DEFAULT_LOC flag just write it out 
*    to the outfile and go to the top of the loop.
***************************************************************************/
        else if (dummy[0] == PGSd_PC_DEFAULT_LOC)
        {
            fputs(dummy,locationPCS[TEMPFILE]);
            continue;
        }

/***************************************************************************
*    If the first character is a divider then we did not find a match
*    which means we have a problem.
***************************************************************************/
        else if (dummy[0] == PGSd_PC_DIVIDER)
        {
            returnStatus = PGSPC_W_NO_FILES_EXIST;
            endFlag = 1;
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
*    If the number is the same then get the file name from both the 
*    ASCII file method and the shared memory method.
***************************************************************************/
                if (num == PCS_data->index)
                {
                    returnStatus = PGS_PC_GetPCSDataGetFileName(mode,
                                     dummy,PGSd_PC_TEMP_ENVIRONMENT,fromAscii);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                        break;
                    }

                    strcpy(tempShm.fileStruct.fileName,PCS_data->fileName);
                    strcpy(tempShm.fileStruct.path,PCS_data->path);
                    returnStatus = PGS_PC_BuildFileShm(PGSd_PC_TEMP_ENVIRONMENT,
                                      PGSd_PC_TYPE_FILE,&tempShm,fromShm); 
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                        break;
                    }

/***************************************************************************
*    If the file names are the same then add a mark to the end of the 
*    line and write it out.
***************************************************************************/
                    if (strcmp(fromAscii,fromShm) == 0)
                    {
                        sprintf(dummy,"%u|%s|%s|%u|%s|%s|%u%s\n",
                                PCS_data->index,PCS_data->fileName,PCS_data->path,
                                PCS_data->size,PCS_data->universalRef,
                                PCS_data->attributeLoc,PCS_data->entries,
                                PGSd_PC_DELETE_STRING);
                        endFlag = 1;
                    }
                }

/***************************************************************************
*    Write the line out to the file.
***************************************************************************/
                fputs(dummy,locationPCS[TEMPFILE]); 
            }       /* end if */
        }        /* end else */
    }        /* end while */

/***************************************************************************
*    If everything went OK write out the end of the file.
***************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {
        returnStatus = PGS_PC_PutPCSDataAdvanceToLoc(PGSd_PC_ENDOFFILE,
                              locationPCS);
    }
 
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
