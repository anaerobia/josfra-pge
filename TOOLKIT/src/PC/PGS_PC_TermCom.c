/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_TermCom.c

DESCRIPTION:
	This file contains all functions necessary to build PGS_PC_TermCom.

AUTHOR:
	Ray Milburn / Applied Research Corp.
	Abe Taaheri / SM&A Corp.

HISTORY:
	15-Dec-94 RM Initial Version
	17-Feb-95 DH Updated prolog
	05-Apr-95 RM Updated for TK5.  Added functionality which 
			moves default file location from environment
			variables to PCF.
	20-Apr-95 RM Updated for TK5.  Wrote function 
			PGS_PC_UpdateRuntimeFiles() to mark files in the
			PCF which have been marked as RUNTIME in shared
			memory.
        24-Oct-95 DH Inserted call to PGS_SMF_LogEvent() to announce shutdown.
        28-Apr-97 RM Changed check for invalid file record to include 
                        check of file name as well as the product id.
	08-Feb-99 AT Added a few lines of code to PGS_PC_SavePCSData
	                to check for successful return from PGS_PC_GetPCSData
			and for "fromShm" not equal " ". If thre is problem,
			it constructs the file name that is marked for deletion
			and rmoves the file.
 
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
	Termination program.
 
NAME:  
	PGS_PC_TermCom

SYNOPSIS: 
	PGS_PC_TermCom <shared-memory-flag> <log-file-flag>

C:
	N/A

FORTRAN:
	N/A

DESCRIPTION:
	This program runs the functions necesary to clean up shared 
	memory, send runtime files, send logfiles, update the PCF, and
	remove temporary files.
 
INPUTS:
        Name            Description                     Units   Min     Max

	argc		number of command line arguments

	argv[0]		executable name (not processed 
			but listed here anyway)

	argv[1]		flag stating whether or not to 
			use shared memory

	argv[2]		flag stating whether or not to
			write to a log file


OUTPUTS:
        Name            Description                     Units   Min     Max

	NONE

RETURNS:
	PGS_S_SUCCESS
	PGS_SH_PC_DELETETEMP
	PGS_SH_SMF_SENDRUNTIME
	PGS_SH_SMF_SENDLOGFILE
	PGS_SH_MEM_TERM
        PGS_SH_SMF_MSSLOGFILE

EXAMPLES:
	PGS_PC_TermCom ShmOff LogOff
	PGS_PC_TermCom ShmOn LogOff

NOTES:
	This program is designed to be run from within the PGS_PC_Shell.sh script
	and is not intended to be run as a stand alone program from the command
	line.  Running this program outside the script PGS_PC_Shell.sh will give
	undefined results.

	Since this tool now supports the transfer of status and runtime files,
	certain steps need to be performed by the user in order to ensure that
	this transfer operation is carried-out properly.

	FILE TRANSFER SETUP
	-------------------	
	The current transfer mechanism (ftp) requires the use of a '.netrc' 
	file, which must reside in the user's home directory on the execution 
	host. 'ftp' accesses this file to establish a connection with the remote
	host. Once the connection is made, the process of performing the actual 
	file transfer can proceed.
	
      	This file must contain information in the following format:

        machine <hostname> login <username> password <userpassword>

	For example:

	machine adriatic login guest password anonymous

	For reasons of security, the '.netrc' file should ONLY have read 
	permission for the user, (i.e. -rw-------).

	(Refer to the man pages on netrc for more information).


	PROCESS CONTROL SETUP
 	---------------------	
	As part of the transfer operation, this tool also transmits a 
	notification message to the interested parties to inform them
	as to the disposition of the requested runtime and status files. 

	As with many other Process Control tools, this tool depends on certain
	entries in the Process Control File. The values of these entries however 
	are user defined according to their local environment. 

	Refer to the standard Process Control File to find the following entries: 

   	10109|TransmitFlag; 1=transmit,0=disable|0
	- Set to 1 to enable file/e-mail transmission.

   	10106|RemoteHost|<hostname>
	- Host should be the same as that which appears in the '.netrc' file.

   	10107|RemotePath|<destination directory>
	- Directory must be writeable and large enough to hold the transferred 
	  data.

   	10108|EmailAddresses|<list of notification addresses>
	- Notification message indicates which files have been transferred 
	  and where they currently reside.

    	WARNING Do not attempt to transfer files to the same host and directory
    	that this program is running on. The original files will be deleted in 
    	accordance with the ftp protocol for sending and receiving files. That is
	to say that, upon determination that the destination file is the same as 
	the source, the destination file will be removed before sending the source
	file.
 
REQUIREMENTS:
	PGSTK-1311

DETAILS:
    	The routine PGS_SMF_SendFile() provided by CSMC on August 31, 1994. 
	Internally, this function requires the use of ftp to work properly,
	thus requiring the use of a '.netrc' file.

GLOBALS:
	NONE

FILES:
	NONE	

FUNCTIONS_CALLED:
	PGS_SMF_TermProc		- notify SMF termination is occurring
	PGS_SMF_SysTermSendRuntime	- send runtime files to destination
	PGS_SMF_SysTermSendStatusReport	- send status report files to destination
	PGS_PC_SavePCSData		- update a section of the PCF
	PGS_MEM_ShmSysTerm		- terminate shared memory
	PGS_PC_DeleteTemps		- delete temporary files
	PGS_PC_UpdateRuntimeFiles	- update files marked as runtime

END_PROLOG:
***************************************************************************/

/***************************************************************************
*    Function prototypes for functions in this file.
***************************************************************************/
PGSt_SMF_status PGS_PC_SavePCSData(PGSt_integer,PGSt_integer,char *);
PGSt_SMF_status PGS_PC_DeleteTemps(void);
PGSt_SMF_status PGS_PC_UpdateRuntimeFiles(PGSt_integer,PGSt_integer,char *);

/***************************************************************************
*    Function main().
***************************************************************************/
int
main(                                    /* main function */
    int                 argc,            /* number of command line arguments */
    char               *argv[])          /* array of command line arguments */
{
    int                 count;
    PGSt_integer        sectionIndices[] = {PGSd_PC_INPUT_FILES,
                                            PGSd_PC_OUTPUT_FILES,
                                            PGSd_PC_SUPPORT_INPUT,
                                            PGSd_PC_SUPPORT_OUTPUT,
                                            PGSd_PC_INTER_INPUT,
                                            PGSd_PC_INTER_OUTPUT,
                                            PGSd_PC_TEMP_INFO};  /* sections */
    PGSt_integer        modes[] = {PGSd_PC_INPUT_FILE_NAME,
                                   PGSd_PC_OUTPUT_FILE_NAME,
                                   PGSd_PC_SUPPORT_IN_NAME,
                                   PGSd_PC_SUPPORT_OUT_NAME,
                                   PGSd_PC_INTERMEDIATE_INPUT,
                                   PGSd_PC_INTERMEDIATE_OUTPUT,
                                   PGSd_PC_TEMPORARY_FILE};   /* modes */
    char               *envs[] = {PGSd_PC_INPUT_FILE_ENVIRONMENT,
                                  PGSd_PC_OUTPUT_FILE_ENVIRONMENT,
                                  PGSd_PC_SUPPT_INPUT_ENVIRONMENT,
                                  PGSd_PC_SUPPT_OUT_ENVIRONMENT,
                                  PGSd_PC_INTER_INPUT_ENVIRONMENT,
                                  PGSd_PC_INTER_OUTPUT_ENVIRONMENT,
                                  PGSd_PC_TEMP_ENVIRONMENT};  /* environments */
    PGSt_SMF_status     useMemType;      /* flag for shared memory */
    PGSt_SMF_status     writeLogFile;    /* flag for writing to log file */
    PGSt_SMF_status     saveStatus;      /* temporary hold for status */
    PGSt_SMF_status     returnStatus;    /* return from functions */

#ifdef PGS_IR1
    PGSt_SMF_status     statusCode;      /* return from functions */
    PGSt_SMF_status     returnMSSStatus; /* return from functions */
    char                actionString[PGS_SMF_MAX_ACT_SIZE];
                                         /* MSS action message string */
    char                actionLabel[PGS_SMF_MAX_MNEMONIC_SIZE];
                                         /* MSS channel action mnemonic */
    PGSSmfGlbVar        *global_var;     /* global static buffer */
    char                *eventRef;       /* global event log file name */
    char                *eventSwitch;    /* global event log activation switch */
    char                statusMsg[PGS_SMF_MAX_MSG_SIZE];
                                         /* hold status/!action message */
#endif /* PGS_IR1 */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    saveStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Set the shared memory flag.
***************************************************************************/
    if (strcmp(argv[1],PGSd_PC_SHMOFF) == 0)
    {
        useMemType = PGSd_SMF_USE_ASCIIFILE;
    }
    else  
    {
        useMemType = PGSd_SMF_USE_SHAREDMEM;
    }

/***************************************************************************
*    Set the log file flag.
***************************************************************************/
    if (strcmp(argv[2],PGSd_PC_LOGOFF) == 0)
    {
        writeLogFile = PGSd_SMF_WRITELOG_OFF;
    }
    else  
    {
        writeLogFile = PGSd_SMF_WRITELOG_ON;
    }
    
/***************************************************************************
*   Inform SMF that this is the termination process.
***************************************************************************/
    PGS_SMF_TermProc(useMemType,writeLogFile);

/***************************************************************************
*   Send Message to Event Log to indicate that Termination is proceeding
***************************************************************************/
#ifdef PGS_IR1
    /*
     * Send the message to the event logger, if it has
     * an associated action string of the proper type.
     * EventLog Trigger MUST exist and be turned on
     * in the PCF for this communication channel to function.
     * NOTE: DAAC Toolkit ONLY
     */
 
    /*
     * Need test here to prevent LogEvent from being called with
     * every StatusLog update.
     * [TK6] - Add action-code to msginfo.msgdata structure to make
     *         this test more efficient; e.g. perform bit-wise level
     *         test.
     */
     statusCode = PGSPC_M_TOOLKIT_TERM_BEGIN;
     PGS_SMF_GetMsgByCode( statusCode,statusMsg );
     returnMSSStatus = PGS_SMF_GetActionMneByCode(
                                        statusCode,
                                        actionString,
                                        actionLabel);
 
     if (PGS_SMF_TestStatusLevel(returnMSSStatus) == PGS_SMF_MASK_LEV_E)
     {
         returnMSSStatus = PGS_SH_SMF_MSSLOGFILE;
     }
     else
     {
         returnMSSStatus = PGS_SMF_GetActionType( actionLabel );
         if ((returnMSSStatus == PGSSMF_M_EMAIL_ACTION) ||
             (returnMSSStatus == PGSSMF_M_INFO_ACTION))
         {
             PGS_SMF_GetMSSGlobals(&eventRef,&eventSwitch);
             if (strcmp(eventSwitch,PGSd_SMF_ON)==0)
             {
                /*
                 * Get the pointer to the global variable
                 */
                 PGS_SMF_GetGlobalVar(&global_var);
 
                 PGS_SMF_LogEvent(
                       global_var->msginfo.msgdata.code,
                       statusMsg,
                       global_var->msginfo.msgdata.mnemonic,
                       actionString,
                       returnMSSStatus );
             }
         } /* end Action type test */
         returnMSSStatus = PGS_S_SUCCESS;
     }
 
#endif /* PGS_IR1 */

/***************************************************************************
*   Send any runtime data that should have been sent.
***************************************************************************/
    returnStatus = PGS_SMF_SysTermSendRuntimeData();

/***************************************************************************
*   If we were writing to the log file then let's send it.
***************************************************************************/
    if (writeLogFile == PGSd_SMF_WRITELOG_ON)
    {
        returnStatus = PGS_SMF_SysTermSendStatusReport();
    }

/***************************************************************************
*   If we were using shared memory then let's clean up.  We need to 
*   update the PCF with the new Intermediate Output, and Temporary file
*   sections and then notify our MEM functions that we are done with
*   shared memory.
***************************************************************************/
    if (useMemType == PGSd_SMF_USE_SHAREDMEM)
    {
    
        returnStatus = PGS_PC_SavePCSData(PGSd_PC_INTER_OUTPUT,
                                          PGSd_PC_INTERMEDIATE_OUTPUT,
                                          PGSd_PC_INTER_OUTPUT_ENVIRONMENT);

/***************************************************************************
*   If we had a problem we want to keep going and try to get as much 
*   done as possible but we may want to let the user know there were
*   some problems.
***************************************************************************/
        if (returnStatus != PGS_S_SUCCESS)
        {
            saveStatus = returnStatus;
        }
    
        returnStatus = PGS_PC_SavePCSData(PGSd_PC_TEMP_INFO,
                                          PGSd_PC_TEMPORARY_FILE,
                                          PGSd_PC_TEMP_ENVIRONMENT);

/***************************************************************************
*   If we had a problem we want to keep going and try to get as much 
*   done as possible but we may want to let the user know there were
*   some problems.
***************************************************************************/
        if (returnStatus != PGS_S_SUCCESS)
        {
            saveStatus = returnStatus;
        }

/***************************************************************************
*   Loop through each type of file and mark the files in the PCF that
*   were flagged as RUNTIME during the PGE.
***************************************************************************/
        for (count = 0; count < PGSd_PC_FILE_TYPES; count++)
        {
            returnStatus = PGS_PC_UpdateRuntimeFiles(sectionIndices[count],
                                               modes[count],envs[count]);

/***************************************************************************
*   If we had a problem we want to keep going and try to get as much 
*   done as possible but we may want to let the user know there were
*   some problems.
***************************************************************************/
            if (returnStatus != PGS_S_SUCCESS)
            {
                saveStatus = returnStatus;
            }
        }

/***************************************************************************
*   Terminate shared memory.
***************************************************************************/
        returnStatus = PGS_MEM_ShmSysTerm();
    }

/***************************************************************************
*   If we have been working from an ASCII file we did not really delete
*   any files we just marked them.  Now we can go in and find all the
*   files that are marked and delete them.
***************************************************************************/
    else
    {
        returnStatus = PGS_PC_DeleteTemps();
    }

/***************************************************************************
*   End of program.  If return status is OK, let's send back what is in 
*   our saved value since it may report a problem; If the saved value is
*   also successful, then return any problems encountered with the Event
*   Logging functionality (if this is the DAAC version).
***************************************************************************/

    if (returnStatus == PGS_S_SUCCESS)
    {
        returnStatus = saveStatus;
#ifdef PGS_IR1
        if (returnStatus == PGS_S_SUCCESS)
	{
	    returnStatus = returnMSSStatus;
	}
#endif /* PGS_IR1 */
    }
    exit(returnStatus);
}




/***************************************************************************
BEGIN_PROLOG:

TITLE:  
 
NAME:  
	PGS_PC_SavePCSData

SYNOPSIS:
	This functions updates file sections of the PCF.

C:
		#include <PGS_PC.h>

		PGSt_SMF_status 
		PGS_PC_SavePCSData(
			PGSt_integer	sectionIndex,
			PGSt_integer	mode,
			char		*env);

FORTRAN:
	NONE

DESCRIPTION:
	This function receives as input the section identifiers of the section
	that needs to be updated.  It then updates the PCF to match what is
	in shared memory.
 
INPUTS:
        Name            Description                     Units   Min     Max

	sectionIndex	Division pointer index of section
			to be updated.

	mode		Section mode to be updated.

	env		Environment variable associated
			with that section of data.

OUTPUTS:
        Name            Description                     Units   Min     Max

	NONE

RETURNS:


EXAMPLES:


NOTES:


REQUIREMENTS:
	NONE

DETAILS:
	

GLOBALS:
	NONE

FILES:
	

FUNCTIONS_CALLED:
	PGS_MEM_ShmSysAddr		 Get address of PC data in shared
					 memory
	PGS_PC_GetPCSData		 Get PCS data
	PGS_PC_BuildFileShm		 Build a file name from structure
					 read from shared memory
	PGS_PC_PutPCSData		 Update PCS data

END_PROLOG:
***************************************************************************/


PGSt_SMF_status 
PGS_PC_SavePCSData(                      /* update a portion of the PCF */
    PGSt_integer        sectionIndex,    /* section array index */
    PGSt_integer        mode,            /* mode portion to update */
    char               *env)             /* environment of path */
{
    char               *addrPC;          /* PC address in shared memory */
    char               *sectionLoc;      /* address of beginning of section */
    char                firstChar;       /* first character to look at */
    char                fromAscii[PGSd_PC_FILE_PATH_MAX]; /* file/path name */
    char                fromShm[PGSd_PC_FILE_PATH_MAX]; /* file/path name */
    PGSt_boolean        done;            /* finished checking files */
    PGSt_boolean        putFlag;         /* flag to write out to file */
    PGSt_integer        numFiles;        /* number of files */
    PGSt_uinteger       size;            /* amount of shared memory for PC */
    PGSt_PC_HeaderStruct_Shm headStruct; /* header in shared memory */
    PGSt_PC_File_Shm    fileShm;         /* file structure */
    PGSt_SMF_status     returnStatus;    /* return from functions */
    PGSt_SMF_status     saveStatus;    /* return from function 
					   PGS_PC_GetPCSData saved */
    char                fileDeleted[PGSd_PC_FILE_PATH_MAX]=" "; /* file/path
								   name for the
								   temporary
								   file to be 
								   deleted */

/***************************************************************************
*   Initialize return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*   Get address in shared memory of PC data.
***************************************************************************/
    returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,
                                      (void **) &addrPC,&size);
    
    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*   The division pointer should not be NULL.
***************************************************************************/
        memcpy((void *) &headStruct,(void *) addrPC,
                            sizeof(PGSt_PC_HeaderStruct_Shm));

        if (headStruct.divPtr[sectionIndex-1] != (PGSt_uinteger) NULL)
        {
            sectionLoc = (char *) ((unsigned long) (addrPC) + 
                                          headStruct.divPtr[sectionIndex-1]);
    
/***************************************************************************
*   The first character should be a divider.
***************************************************************************/
            memcpy(&firstChar,(char *) sectionLoc,sizeof(char));
            if (firstChar == PGSd_PC_DIVIDER)
            {

/***************************************************************************
*   Initialize our variables before entering the loop.
***************************************************************************/
                sectionLoc = (char *) ((unsigned long) sectionLoc + sizeof(char));
                numFiles = 1;
                putFlag = PGS_FALSE;
                done = PGS_FALSE;

/***************************************************************************
*   Loop until.....we are done.
***************************************************************************/
                while (done == PGS_FALSE)
                {

/***************************************************************************
*   If we hit a divider that means we are at the next section, which in
*   turn means that we are done.
***************************************************************************/
                    memcpy((void *) &firstChar,(void *) sectionLoc,
                                 sizeof(char));
                    if (firstChar == PGSd_PC_DIVIDER)
                    {
                        done = PGS_TRUE;
                        continue;
                    }
        
/***************************************************************************
*   Get the next structure from shared memory.
***************************************************************************/
                    memcpy((void *) &fileShm,(void *) sectionLoc,
                                       sizeof(PGSt_PC_File_Shm));

/***************************************************************************
*   If the index is NULL then there is a problem.
***************************************************************************/
                    if ((fileShm.fileStruct.index == (PGSt_PC_Logical) NULL) &&
                        (fileShm.fileStruct.fileName[0] == (char) PGSd_PC_CHAR_NULL))
                    {
                       returnStatus = PGSPC_E_INDEX_ERROR_SHM;;
                       done = PGS_TRUE;
                       continue;
                    }

/***************************************************************************
*   Update the address pointer and get the file information from the
*   ASCII PCF for comparison.
***************************************************************************/
                    sectionLoc = (char *)
                     ((unsigned long) sectionLoc + sizeof(PGSt_PC_File_Shm));

                    returnStatus = PGS_PC_GetPCSData(mode+1,fileShm.fileStruct.index,
                                                    fromAscii,&numFiles);
		    saveStatus = returnStatus;
                    if (returnStatus == PGS_S_SUCCESS)
                    {

/***************************************************************************
*   Build the file/path string with the data was from shared memory.
***************************************************************************/
                        returnStatus = PGS_PC_BuildFileShm(env,PGSd_PC_TYPE_FILE,
                                                                &fileShm,fromShm);

/***************************************************************************
*   If the file/path strings received from the ASCII PCF and shared
*   memory are different then we need to update the PCF.
***************************************************************************/
                        if (strcmp(fromShm,fromAscii) != 0)
                        {
                            putFlag = PGS_TRUE;
                        }
                    } 

/***************************************************************************
*   If the logical has no reference then update the PCF.
***************************************************************************/
                    else if (returnStatus == PGSPC_W_NO_FILES_EXIST)
                    {
                        putFlag = PGS_TRUE;
                    }

/***************************************************************************
*   There was some kind of a problem reading the PCF.
***************************************************************************/
                    else
                    {
                        done = PGS_TRUE;
                        continue;
                    }

/***************************************************************************
*   Update the PCF.
***************************************************************************/
                    if (putFlag == PGS_TRUE)
                    {

                        returnStatus = PGS_PC_PutPCSData(mode+1,&fileShm.fileStruct);
                        putFlag = PGS_FALSE;
                    }

/***************************************************************************
*   If the file is marked for deletion, then make sure that the mark
*   is transferred to the PCF.
***************************************************************************/
                    if ((fileShm.flag == PGSd_PC_DELETE_FLAG) ||
                        (fileShm.flag == PGSd_PC_DELNRUN_FLAG))
                    {
                        returnStatus =
			  PGS_PC_PutPCSData(mode+2,&fileShm.fileStruct);
/***************************************************************************
*   If saveStatus == PGS_S_SUCCESS and fromShm contains the file name to be
*   deleted, we remove fromShm; Otherwise, construct the temporary file name
*   to be deleted and remove it.
***************************************************************************/
			if((saveStatus == PGS_S_SUCCESS) && (strcmp(fromShm," ") != 0))
			{
			    remove(fromShm);
			}
			else
			{
			    strcpy(fileDeleted,fileShm.fileStruct.path);
			    strcat(fileDeleted,"/");
			    strcat(fileDeleted, fileShm.fileStruct.fileName);
			    remove(fileDeleted);
			}
			    
                    }

                }  /* end while */
            }  /* end if */

/***************************************************************************
*   The division pointer was not a divider.
***************************************************************************/
            else
            {
            returnStatus = PGSPC_E_INV_DIVPOINTER_SHM;
            }

/***************************************************************************
*   The address of the division pointer was NULL.
***************************************************************************/
        }  /* end if (headstruct != NULL) */
        else
        {
            returnStatus = PGSPC_E_NULL_DIVPOINTER_SHM;
        }
    }  /* end if (return == SUCCESS) from PGS_MEM_ShmSysAddr() */

/***************************************************************************
*   Set return status to something our termination program will 
*   understand.
***************************************************************************/
    if (returnStatus != PGS_S_SUCCESS)
    {
        returnStatus = PGS_SH_PC_DELETETMP;
    }

/***************************************************************************
*   Return to calling function.
***************************************************************************/
return PGS_S_SUCCESS;
}





/***************************************************************************
BEGIN_PROLOG:

TITLE:  
 
NAME:  
	PGS_PC_DeleteTemps()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status 
		PGS_PC_DeleteTemps(void);

FORTRAN:
	NONE

DESCRIPTION:
	This function checks the PCF and removes files from disk that were
	flagged for deletion during the PGE.  This function is run only if
	the PGE was working from an ASCII file as opposed to shared memory.
 
INPUTS:
        Name            Description                     Units   Min     Max

	NONE

OUTPUTS:
        Name            Description                     Units   Min     Max

	NONE

RETURNS:
	PGS_S_SUCCESS
	PGS_SH_PC_DELETETMP

EXAMPLES:
	PGSt_SMF_status		returnStatus;

	returnStatus = PGS_PC_DeleteTemps();

	if (returnStatus == PGS_S_SUCCESS)
	{
	/# continue normal operations #/
	}
	else
	{
		goto EXCEPTION;
	}

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
	PGS_PC_GetPCSDataOpenPCSFile	- Open the PCF
	PGS_PC_GetPCSDataAdvanceArea	- Move to a particular area in the PCF
	PGS_PC_GetPCSDataGetFileName	- Build a file name from a line of
					PCF data
	PGS_PC_CheckFlags		- Check a line of file data for
					the existence of flags

END_PROLOG:
***************************************************************************/


PGSt_SMF_status 
PGS_PC_DeleteTemps(void)                 /* delete files marked for deletion */
{
    /*char                  *tem;           temporary hold for fgets() */
    char     line[PGSd_PC_LINE_LENGTH_MAX]; /* line read from file */
    char     fName[PGSd_PC_FILE_PATH_MAX];  /* full path/file name */
    FILE                 *filePtr;       /* file pointer */
    PGSt_integer          flags;         /* flags contained in line */
    PGSt_SMF_status       returnStatus;  /* function return */

/***************************************************************************
*   Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*   Open the PCS File (PCF).
***************************************************************************/
    returnStatus = PGS_PC_GetPCSDataOpenPCSFile(&filePtr);

/***************************************************************************
*   If the PCF opened OK, move the pointer to the Temporary File section.
***************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {
        returnStatus = PGS_PC_GetPCSDataAdvanceArea(filePtr,
                                               PGSd_PC_TEMP_INFO);

/***************************************************************************
*   Everything is OK up to here, get a line of data.
***************************************************************************/
        if (returnStatus == PGS_S_SUCCESS)
        {
            while ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,filePtr)) != NULL)
            {

/***************************************************************************
*   If we have hit a divider then we are done.
***************************************************************************/
                if (line[0] == PGSd_PC_DIVIDER)
                {
                    break;
                }

/***************************************************************************
*   If we hit a comment, just ignore this line and get the next one.
***************************************************************************/
                else if (line[0] == PGSd_PC_COMMENT)
                {
                    continue;
                }

/***************************************************************************
*   If we hit a default file location line, just ignore it and get 
*   the next one.
***************************************************************************/
                else if (line[0] == PGSd_PC_DEFAULT_LOC)
                {
                    continue;
                }

/***************************************************************************
*   If the first character is a digit we want to look at this closer.
***************************************************************************/
                else if ((line[0] >= PGSd_PC_LOWDIGIT) && 
                         (line[0] <= PGSd_PC_HIDIGIT))
                {

/***************************************************************************
*   Check for the delete flag, if it is there we want to get this file.
***************************************************************************/
                    returnStatus = PGS_PC_CheckFlags(line,&flags);

                    if ((flags == PGSd_PC_HAS_DELETE) || 
                        (flags == PGSd_PC_HAS_DELNRUN))
                    {

/***************************************************************************
*   Build the actual file name.
***************************************************************************/
                        returnStatus = PGS_PC_GetPCSDataGetFileName(
                                  PGSd_PC_TEMPORARY_FILE,line,
                                  PGSd_PC_TEMP_ENVIRONMENT,fName);

/***************************************************************************
*   If the file name string was built OK, remove the file.
***************************************************************************/
                        if (returnStatus == PGS_S_SUCCESS)
                        {
                            remove(fName);
                        }
                    }
                }  /* end else if */
            }  /* end while */
        }  /* end if */
    }

/***************************************************************************
*   Check the return status.
***************************************************************************/
    if (returnStatus != PGS_S_SUCCESS)
    {
        returnStatus = PGS_SH_PC_DELETETMP;
    }

/***************************************************************************
*   Return to the calling function.
***************************************************************************/
    return returnStatus;
}






/***************************************************************************
BEGIN_PROLOG:

TITLE:  
 
NAME:  
	PGS_PC_UpdateRuntimeFiles

SYNOPSIS:
	This functions updates files that have been marked as runtime
	files.

C:
		#include <PGS_PC.h>

		PGSt_SMF_status 
		PGS_PC_UpdateRuntimeFils(
			PGSt_integer	sectionIndex,
			PGSt_integer	mode,
			char		*env);

FORTRAN:
	NONE

DESCRIPTION:
	This function receives as input the section identifiers of the section
	that needs to be updated.  It then updates the PCF to match what is
	in shared memory.  This function only deals with RUNTIME flags.
 
INPUTS:
        Name            Description                     Units   Min     Max

	sectionIndex	Division pointer index of section
			to be updated.

	mode		Section mode to be updated.

	env		Environment variable associated
			with that section of data.

OUTPUTS:
        Name            Description                     Units   Min     Max

	NONE

RETURNS:


EXAMPLES:


NOTES:


REQUIREMENTS:
	NONE

DETAILS:
	

GLOBALS:
	NONE

FILES:
	

FUNCTIONS_CALLED:
	PGS_MEM_ShmSysAddr		 Get address of PC data in shared
					 memory
	PGS_PC_BuildFileShm		 Build a file name from structure
					 read from shared memory
	PGS_PC_MarkRuntimeAscii		 Mark files in the ASCII PCF as
					 runtime files

END_PROLOG:
***************************************************************************/



PGSt_SMF_status 
PGS_PC_UpdateRuntimeFiles(               /* update a portion of the PCF */
    PGSt_integer        sectionIndex,    /* section array index */
    PGSt_integer        mode,            /* mode portion to update */
    char               *env)             /* environment of path */
{
    char               *addrPC;          /* PC address in shared memory */
    char               *sectionLoc;      /* address of beginning of section */
    char                firstChar;       /* first character to look at */
    char                fromShm[PGSd_PC_FILE_PATH_MAX]; /* file/path name */
    PGSt_boolean        done;            /* finished checking files */
    PGSt_uinteger       size;            /* amount of shared memory for PC */
    PGSt_PC_HeaderStruct_Shm headStruct; /* header in shared memory */
    PGSt_PC_File_Shm    fileShm;         /* file structure */
    PGSt_SMF_status     returnStatus;    /* return from functions */


/***************************************************************************
*   Initialize return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*   Get address in shared memory of PC data.
***************************************************************************/
    returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,
                                      (void **) &addrPC,&size);
    
    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*   The division pointer should not be NULL.
***************************************************************************/
        memcpy((void *) &headStruct,(void *) addrPC,
                            sizeof(PGSt_PC_HeaderStruct_Shm));

        if (headStruct.divPtr[sectionIndex-1] != (PGSt_uinteger) NULL)
        {
            sectionLoc = (char *) ((unsigned long) (addrPC) + 
                                          headStruct.divPtr[sectionIndex-1]);
    
/***************************************************************************
*   The first character should be a divider.
***************************************************************************/
            memcpy(&firstChar,(char *) sectionLoc,sizeof(char));
            if (firstChar == PGSd_PC_DIVIDER)
            {

/***************************************************************************
*   Initialize our variables before entering the loop.
***************************************************************************/
                sectionLoc = (char *) ((unsigned long) sectionLoc + sizeof(char));
                done = PGS_FALSE;

/***************************************************************************
*   Loop until.....we are done.
***************************************************************************/
                while (done == PGS_FALSE)
                {

/***************************************************************************
*   If we hit a divider that means we are at the next section, which in
*   turn means that we are done.
***************************************************************************/
                    memcpy((void *) &firstChar,(void *) sectionLoc,
                                 sizeof(char));
                    if (firstChar == PGSd_PC_DIVIDER)
                    {
                        done = PGS_TRUE;
                        continue;
                    }
        
/***************************************************************************
*   Get the next structure from shared memory.
***************************************************************************/
                    memcpy((void *) &fileShm,(void *) sectionLoc,
                                       sizeof(PGSt_PC_File_Shm));

/***************************************************************************
*   If the index is NULL then there is a problem.
***************************************************************************/
                    if ((fileShm.fileStruct.index == (PGSt_PC_Logical) NULL) &&
                        (fileShm.fileStruct.fileName[0] == (char) PGSd_PC_CHAR_NULL))
                    {
                       returnStatus = PGSPC_E_INDEX_ERROR_SHM;;
                       done = PGS_TRUE;
                       continue;
                    }

/***************************************************************************
*   Update the address pointer.
***************************************************************************/
                    sectionLoc = (char *)
                     ((unsigned long) sectionLoc + sizeof(PGSt_PC_File_Shm));

/***************************************************************************
*   If the file is marked as RUNTIME, then make sure that the mark
*   is transferred to the PCF.
***************************************************************************/
                    if ((fileShm.flag == PGSd_PC_RUNTIME_FLAG) ||
                        (fileShm.flag == PGSd_PC_DELNRUN_FLAG))
                    {
                        returnStatus = PGS_PC_BuildFileShm(env,PGSd_PC_TYPE_FILE,
                                                                &fileShm,fromShm);
                        returnStatus = PGS_PC_MarkRuntimeAscii(fromShm,
                                                   fileShm.fileStruct.index,mode);
                    }

                }  /* end while */
            }  /* end if */

/***************************************************************************
*   The division pointer was not a divider.
***************************************************************************/
            else
            {
            returnStatus = PGSPC_E_INV_DIVPOINTER_SHM;
            }

/***************************************************************************
*   The address of the division pointer was NULL.
***************************************************************************/
        }  /* end if (headstruct != NULL) */
        else
        {
            returnStatus = PGSPC_E_NULL_DIVPOINTER_SHM;
        }
    }  /* end if (return == SUCCESS) from PGS_MEM_ShmSysAddr() */

/***************************************************************************
*   Set return status to something our termination program will 
*   understand.
***************************************************************************/
    if (returnStatus != PGS_S_SUCCESS)
    {
        returnStatus = PGS_SH_PC_DELETETMP;
    }

/***************************************************************************
*   Return to calling function.
***************************************************************************/
return PGS_S_SUCCESS;
}
