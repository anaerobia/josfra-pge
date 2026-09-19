/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_MarkRuntimeAscii.c

DESCRIPTION:
	This file contains the function PGS_PC_MarkRuntimeAscii().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	13-Apr-95 RM Initial version
	15-Dec-95 RM Updated for TK6.  Added ability to allow the Process
			Control Tools to function across filesystems.
	02-Jul-99 RM Updated for TSF functionality
 
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
	Mark a file as runtime in the PCF.
 
NAME:
	PGS_PC_MarkRuntimeAscii()

SYNOPSIS:

C:
	#include <PGS_PC.h>
	
	PGSt_SMF_status
	PGS_PC_MarkRuntimeAscii(
		char			*fileName,
		PGSt_PC_Logical		identifier,
		PGSt_integer		fileType);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to mark a file in the PCF.
 
INPUTS:
	Name		Description			Units	Min	Max

	fileName	The full path and file name of
			the file to be marked.

	identifier	The logical identifier as defined 
			by the user.  The user's 
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

	fileType	Type of file to mark (section
			that file is located in).

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_EXIST      link number does not have the data that
                                    mode is requesting
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file
	PGSTSF_E_GENERAL_FAILURE    a problem occurred in TSF code

EXAMPLES:
C:	
        #define MODIS1A 2511
 
        PGSt_PC_Logical prodID;
        char            fileName[PGSd_PC_FILE_PATH_MAX];
        PGSt_integer    version;
        PGSt_SMF_status returnStatus;
 
        /# getting the file name of the first file in the #/
        /# product group MOSID1A #/
 
        prodID = MODIS1A;
        version = 1;
        returnStatus = PGS_PC_GetReference(prodID,&version,fileName);
 
        /# mark that file as a runtime file #/
	/# this is assuming we know that shared memory is NOT being used #/
 
        if (returnStatus == PGS_S_SUCCESS)
        {
                returnStatus = PGS_PC_MarkRuntimeAscii(fileName,prodID);
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
	PGS_SMF_TestErrorLevel
	PGS_TSF_LockIt
	PGS_TSF_UnlockIt
 
END_PROLOG:
***************************************************************************/



PGSt_SMF_status
PGS_PC_MarkRuntimeAscii(                     /* mark a file in the PCF */
    char             *fileName,              /* file to be marked */
    PGSt_PC_Logical   identifier,            /* logical id to be marked */
    PGSt_integer      fileType)              /* type of file */

{
    char              line[PGSd_PC_LINE_LENGTH_MAX]; /* line from file */
    char              cnum[33];              /* character number */
    char              msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char              buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char              envVar[100];           /* environment variable */
    char              fName[PGSd_PC_FILE_PATH_MAX];  /* file name */
    char              tempFileName[PGSd_PC_FILE_PATH_MAX]; /* temp file name */
    int               count;                 /* loop counter */
    int               linePos;               /* line position */
    int               newPos;                /* new line position */
    int               reCheck;               /* check of rename()/remove() */
    PGSt_PC_Logical   check;                 /* hold logical ID to compare */
    PGSt_integer      flags;                 /* flags in line of data */
    PGSt_boolean      changedPCF;            /* has the PCF been altered */
    PGSt_SMF_status   whoCalled;             /* user that called this function */
    PGSt_SMF_status   saveStatus;            /* hold for error value */
    PGSt_SMF_status   returnStatus;          /* SMF return */
    FILE             *locationPCS[NUMFILES]; /* input file pointer */

/***************************************************************************
*    Initialize variables.     
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    saveStatus = PGS_S_SUCCESS;
    changedPCF = PGS_FALSE;
    for (count = 0; count < NUMFILES; count++)
    {
        locationPCS[count] = (FILE *) NULL;
    }

/***************************************************************************
*    Initialize caller variable.     
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

#ifdef _PGS_THREADSAFE
/***************************************************************************
*    Since we are creating a new PCF we need to lock this area.
***************************************************************************/
    returnStatus = PGS_TSF_LockIt(PGSd_TSF_PCLOCK);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
#endif

/***************************************************************************
*    Open the PCS data file.
***************************************************************************/
    returnStatus = PGS_PC_PutPCSDataOpenFiles(locationPCS);

/***************************************************************************
*    If all went OK then continue.
***************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*    Advance to proper section of PCF.
***************************************************************************/
        returnStatus = PGS_PC_PutPCSDataAdvanceToLoc(fileType,locationPCS);
    }
    
    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*    Get a line from the PCF.  If we hit the EOF we have a big problem.
***************************************************************************/
	while ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,
                      locationPCS[PCSFILE])) != NULL)
	{

/***************************************************************************
*    We hit a DIVIDER, no problem, just let the calling function know.
***************************************************************************/
	    if (line[0] == PGSd_PC_DIVIDER)
	    {
		returnStatus = PGSPC_W_NO_FILES_EXIST;
		break;
	    }

/***************************************************************************
*    We hit a COMMENT or a DEFAULT_LOC, just skip this line.
***************************************************************************/
	    else if ((line[0] == PGSd_PC_COMMENT) ||
		     (line[0] == PGSd_PC_DEFAULT_LOC))
	    {
		fputs(line,locationPCS[TEMPFILE]);
		continue;
	    }

/***************************************************************************
*    Should be a valid file information line.
***************************************************************************/
	    else
	    {
		linePos = 0;
		while (line[linePos] != PGSd_PC_DELIMITER)
		{
		    
/***************************************************************************
*    Let's make sure that the character is actually a digit.
***************************************************************************/
		    if ((line[linePos] >= PGSd_PC_LOWDIGIT) &&
			(line[linePos] <= PGSd_PC_HIDIGIT))
		    {
			cnum[linePos] = line[linePos];
			linePos++;
		    }
		    else
		    {
			returnStatus = PGSPC_E_LINE_FORMAT_ERROR;
			if (whoCalled != PGSd_CALLERID_SMF)
			{
			    PGS_SMF_GetMsgByCode(returnStatus,msg);
			    sprintf(buf,msg,
				    getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
			    PGS_SMF_SetDynamicMsg(returnStatus,buf,
						  "PGS_PC_MarkRuntimeAscii()");
			}
			break;
		    }
		}   /* end while */
 
/***************************************************************************
*    If everything we found was a digit then carry on, otherwise we're
*    outta here.
***************************************************************************/
		if (returnStatus != PGS_S_SUCCESS)
		{
		    break;
		}
		
		cnum[linePos] = '\0';
		check = (PGSt_PC_Logical) atol(cnum);
		
/***************************************************************************
*    If the ID in this line matches the ID passed in we need to set up
*    our variables to get the file name.
***************************************************************************/
		if (check == identifier) 
		{
		    switch (fileType)
		    {
		      case PGSd_PC_INPUT_FILE_NAME:
			strcpy(envVar,
			       PGSd_PC_INPUT_FILE_ENVIRONMENT);
			break;
		      case PGSd_PC_OUTPUT_FILE_NAME:
			strcpy(envVar,
			       PGSd_PC_OUTPUT_FILE_ENVIRONMENT);
			break;
		      case PGSd_PC_SUPPORT_IN_NAME:
			strcpy(envVar,
			       PGSd_PC_SUPPT_INPUT_ENVIRONMENT);
			break;
		      case PGSd_PC_SUPPORT_OUT_NAME:
			strcpy(envVar,
			       PGSd_PC_SUPPT_OUT_ENVIRONMENT);
			break;
		      case PGSd_PC_INTERMEDIATE_INPUT:
			strcpy(envVar,
			       PGSd_PC_INTER_INPUT_ENVIRONMENT);
			break;
		      case PGSd_PC_INTERMEDIATE_OUTPUT:
			strcpy(envVar,
			       PGSd_PC_INTER_OUTPUT_ENVIRONMENT);
			break;
		      case PGSd_PC_TEMPORARY_FILE:
			strcpy(envVar,
			       PGSd_PC_TEMP_ENVIRONMENT);
			break;
		    }  /* end switch */
		    
		    returnStatus = PGS_PC_GetPCSDataGetFileName(fileType,
								line,envVar,fName);

/***************************************************************************
*    If the file name in the line matches the file name passed in then
*    we are going to need to mark this line.
***************************************************************************/
		    if ((strcmp(fileName,fName) == 0) &&
			(returnStatus == PGS_S_SUCCESS))
		    {
			returnStatus = PGS_PC_CheckFlags(line,&flags);
			switch (flags)
			{
			  case PGSd_PC_NO_FLAGS:
			    newPos = strlen(line) -
			      strlen(strchr(line,PGSd_PC_NEWLINE));
			    line[newPos] = '\0';
			    strcat(line,PGSd_PC_RUNTIME_STR);
			    changedPCF = PGS_TRUE;
			    break;
			  case PGSd_PC_HAS_DELETE:
			    newPos = strlen(line) -
			      strlen(strchr(line,PGSd_PC_DELETE_FLAG));
			    line[newPos] = '\0';
			    strcat(line,PGSd_PC_DELNRUN_STR);
			    changedPCF = PGS_TRUE;
			    
			}  /* end switch */
		    }
		}  /* end if (check == identifier) */

/***************************************************************************
*    Write the line out to the temp location.
***************************************************************************/
		fputs(line,locationPCS[TEMPFILE]);
		if (changedPCF == PGS_TRUE)
		{
		    fputs("\n",locationPCS[TEMPFILE]);
		    break;
		}
	    }  /* end else */
	}  /* end while */
    }  /* end if (returnStatus == PGS_S_SUCCESS) */

/***************************************************************************
*    If there was a problem above, save it and report it later.
***************************************************************************/
    if (returnStatus != PGS_S_SUCCESS)
    {
        saveStatus = returnStatus;
    }

/***************************************************************************
*    If the PCF has changed, move the temp location over to the PCF.
*    Otherwise just close both locations and remove the temp location.
***************************************************************************/
    if ((changedPCF == PGS_TRUE) && (returnStatus == PGS_S_SUCCESS))
    {
        returnStatus = PGS_PC_PutPCSDataAdvanceToLoc(
                                    PGSd_PC_ENDOFFILE,locationPCS);
        returnStatus = PGS_PC_PutPCSDataFixBuffer(locationPCS);
        returnStatus = PGS_PC_GetPCFTemp(tempFileName);
        if (returnStatus == PGS_S_SUCCESS)
        {
            reCheck = rename(tempFileName,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
            if (reCheck == -1)
            {
                saveStatus = PGSPC_E_PCF_UPDATE_FAILED;
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_GetMsgByCode(saveStatus,msg);
                    sprintf(buf,msg,errno);
                    PGS_SMF_SetDynamicMsg(saveStatus,buf,
                                          "PGS_PC_MarkRuntimeAscii()");
                }
            }
        }
    }
    else
    {
        returnStatus = PGS_PC_PutPCSDataFixBuffer(locationPCS);
        returnStatus = PGS_PC_GetPCFTemp(tempFileName);
        if (returnStatus == PGS_S_SUCCESS)
        {
            reCheck = remove(tempFileName);
            if (reCheck == -1)
            {
                saveStatus = PGSPC_W_PCF_CLEANUP_WARN;
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_GetMsgByCode(saveStatus,msg);
                    sprintf(buf,msg,errno);
                    PGS_SMF_SetDynamicMsg(saveStatus,buf,
                                          "PGS_PC_MarkRuntimeAscii()");
                }
            }
        }
    }

/***************************************************************************
*    If there was a problem above, we saved it earlier now let's report
*    it.
***************************************************************************/
    if (saveStatus != PGS_S_SUCCESS)
    {
        returnStatus = saveStatus;
    }

/***************************************************************************
*    Set the message in the log and return.
***************************************************************************/
    if (returnStatus == PGSPC_W_NO_FILES_EXIST)
    {
        if (whoCalled != PGSd_CALLERID_SMF)
        {
            PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_MarkRuntimeAscii()");
        }
    }

#ifdef _PGS_THREADSAFE
/***************************************************************************
*    Unlock now.
***************************************************************************/
    saveStatus = PGS_TSF_UnlockIt(PGSd_TSF_PCLOCK);
    if ((returnStatus == PGS_S_SUCCESS) && 
        (PGS_SMF_TestErrorLevel(saveStatus)))
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
    }
#endif

    return returnStatus;
}
