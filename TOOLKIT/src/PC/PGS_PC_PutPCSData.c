/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_PutPCSData.c

DESCRIPTION:
	This file contains the function PGS_PC_PutPCSData().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	23-Mar-94 RM Initial version
	12-Apr-94 RM Corrected unsigned long stuff - added intermediate
		file functionality and delete temporary file
	21-Apr-94 RM Repaired DR's
	25-Apr-94 RM Updated prolog
	05-Aug-94 RM Broke out functionality into seperate files.
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	18-Dec-94 RM Added new mode values - to handle shared memory
		for release of TK4.
	15-Dec-95 RM Updated for TK6.  Added functionality to allow the
		Process Control Tools to function across filesytems.  
		Also added the check of the return values from rename()
		and remove().
	11-Mar-96 RM Added code to set file pointers to NULL.  This code
		was required per DR ECSed01815.
	07-Jul-99 RM Updated for TSF functionality
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <PGS_PC.h>
#include <PGS_TSF.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Put Process Control Status information.
 
NAME:  
	PGS_PC_PutPCSData()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status
		PGS_PC_PutPCSData(
			PGSt_integer		mode,
			void			*PCS_data);

FORTRAN:
		include 'PGS_SMF.f'
		include 'PGS_PC.f'
		include 'PGS_PC_9.f'
	
		integer function pgs_pc_putpcsdata(mode,identifier)

		common /filestruct/ identifier,size,bufsize,
					entry,attribute,name,path
		integer		identifier
		integer		size
		integer		bufsize
		integer		entry
		character*256	attribute
		character*256	name
		character*100	path

DESCRIPTION:
	This tool may be used to write information concerning the 
	availability of PGE input files, Process ID's, Science Software
	ID's, configuration values, temporary input files, temporary
	output files, etc to the Process Control Info file.
 
INPUTS:
	Name		Description			Units	Min	Max

	mode		Context values that will be 
			#define'd in a header file.
			Possible values are:

			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_DELETE_TEMP
			PGSd_PC_TEMPDEL_TERM
			PGSd_PC_TEMP_INFO_USEASCII
			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_INTERMEDIATE_OUTPUT
			PGSd_PC_INTER_OUT_USEASCII

	PCS_data	The information to be written 
			out to the Process Control
			File.  If a file is to be deleted 
			from the Process Control
			Information file then a value of 
			type PGSt_PC_Logical will need to 
			be passed in and type cast as a 
			void *.  If a temporary file is 
			to be added to the Process Control 
			Information file then the address 
			of a PGSt_PC_File_Struct will need 
			to be passed in and type cast as 
			a void *.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_EXIST      link number does not have the data that 
                                    mode is requesting
	PGSPC_E_FILE_OPEN_ERROR     error opening input file
	PGSPC_E_INVALID_MODE        mode value does not match those in PGS_PC.h
	PGSPC_E_FILE_READ_ERROR     error reading input file
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file
	PGSTSF_E_GENERAL_FAILURE    error in TSF code

EXAMPLES:

C:
	PGSt_SMF_status		returnStatus;
	PGSt_PC_File_Struct	fileStruct;

	/# fill the file structure with the proper information #/
	/# even though some of these fields are optional they must #/
	/# be initialized #/

	fileStruct.index = 101;
	strcpy(fileStruct.name,"temp.fil");
	strcpy(fileStruct.path,"/u/temp");
	strcpy(fileStruct.attributeLoc,"MODIS-ATTRIB.fil");
	fileStruct.size = 0;
	fileStruct.bufferSize = 0;
	fileStruct.validityFlag = 0;
	fileStruct.entries = 0;
		.
		.
		.
	returnStatus = PGS_PC_PutPCSData(PGSd_PC_TEMPORARY_INPUT_NAME,
			(void *) &fileStruct);
	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
		.
		.
		.
	EXCEPTION:
		return returnStatus;

FORTRAN:
		common /filestruct/ identifier,size,bufsize,entry,
	       *                    attribute,name,path

		integer		identifier
		integer		size
		integer		bufsize
		integer		entry
		character*256	attribute
		character*256	name
		character*100	path

		integer		retstat

C	Fill the structure with the proper information even though some of these
C	fields are optional they must be initialized.

		identifier = 101
		size = 0
		bufsize = 0
		entry = 0
		attribute = 'MODIS-ATTRIB.fil'
		name = 'temp.fil'
		path = '/u/temp'
			.
			.
			.

		retstat = pgs_pc_putpcsdata(pgsd_pc_temporary_file,identifier)
		if (retstat .ne. pgs_s_success) then
			goto 9999
		else

C		Continue as usual
			.
			.
			.
		endif

	9999	return

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
	This tool creates and writes to "tempPCS.fil" and then moves the file
	defined in the environment variable "PGS_PC_INFO_FILE".  This tool
	also reads from the file defined in the environment variable
	"PGS_PC_INFO_FILE".
 
FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_PC_PutPCSDataOpenFiles       Open input and temporary files
	PGS_PC_PutPCSDataAdvanceToLoc    Advance and write to temp file
	PGS_PC_PutPCSDataPutInArea       Put requested data in area
	PGS_PC_PutPCSDataFixBuffer       Ensure that location is updated 
					 from buffer
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_MEM_ShmSysAddr		 Get the address in shared memory
					 of the PC data
	PGS_PC_PutPCSDataMarkAtTerm 	 Mark data in the PCF at PGE
					 termination
	PGS_PC_GetPCFTemp		 Get location of temporary file
	PGS_SMF_TestErrorLevel           Determine if message is _E_ level
	PGS_TSF_LockIt                   Lock section of code for TSF
	PGS_TSF_UnlockIt                 Unlock section of code for TSF

END_PROLOG:
***************************************************************************/



PGSt_SMF_status
PGS_PC_PutPCSData(                         /* update PCS area */
    PGSt_integer        mode,              /* input mode */
    void               *PCS_data)          /* information to write */
{
    char               *addrPC;            /* address of PC data in shm */
    char               *useShm;            /* SHM environment variable */
    char                buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char                msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char                tempFileName[PGSd_PC_FILE_PATH_MAX]; /* temp file name */
    int                 reCheck;           /* check of rename()/remove() */
    PGSt_integer        useMode;           /* mode passed to other functions */
    PGSt_uinteger       size;              /* amount of shm allocated for PC */
    PGSt_SMF_status     whoCalled;         /* user that called this function */
    PGSt_SMF_status     returnStatus;      /* function return */
    PGSt_SMF_status     saveVal;           /* function value holder */
    FILE               *locationPCS[NUMFILES]; /* location of PCS data */

/***************************************************************************
*    Initialize status.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Determine if we have passed in a mode value that forces us to use
*    the ascii PCF.
***************************************************************************/
    if ((mode == PGSd_PC_INTER_OUT_USEASCII) ||
        (mode == PGSd_PC_TEMP_INFO_USEASCII) ||
        (mode == PGSd_PC_TEMPDEL_TERM))
    {
        useShm = (char *) PGSd_PC_USESHM_NO;
        if (mode == PGSd_PC_TEMPDEL_TERM)
        {
            useMode = mode - 2;
        }
        else
        {
            useMode = mode - 1;
        }
    }

/***************************************************************************
*    Use whatever form of access is available.
***************************************************************************/
    else
    {
        useShm = getenv(PGSd_PC_USESHM_ENV);
        if (useShm == NULL)
        {
            useShm = (char *) PGSd_PC_USESHM_NO;
        }
        useMode = mode;
    }

#ifdef _PGS_THREADSAFE
/***************************************************************************
*    Since we are changing the PCF we need to lock on TSF mode.
***************************************************************************/
    returnStatus = PGS_TSF_LockIt(PGSd_TSF_PCLOCK);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
#endif

/***************************************************************************
*    Get the address in shared memory of the PC Data.
***************************************************************************/
    if (strcmp(useShm,PGSd_PC_USESHM_YES) == 0)
    {
        returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,
                                      (void **) &addrPC,&size);

        if (returnStatus == PGS_S_SUCCESS)
        {

/***************************************************************************
*    Perform our put operation in shared memory.
***************************************************************************/
            returnStatus = PGS_PC_PutDataInShm(addrPC,useMode,PCS_data,size);
        }
    }

/***************************************************************************
*    The data is not in shared memory.
***************************************************************************/
    else
    {

/***************************************************************************
*    Initialize file pointers.
***************************************************************************/
        locationPCS[PCSFILE] = (FILE *) NULL;
        locationPCS[TEMPFILE] = (FILE *) NULL;

/***************************************************************************
*    Open the files necessary.
***************************************************************************/
        returnStatus = PGS_PC_PutPCSDataOpenFiles(locationPCS);
        if (returnStatus == PGS_S_SUCCESS)
        {

/***************************************************************************
*    Advance to the area that we need to work on.
***************************************************************************/
             returnStatus = PGS_PC_PutPCSDataAdvanceToLoc(useMode,locationPCS);
             if (returnStatus == PGS_S_SUCCESS)
             {

/***************************************************************************
*    Mark data.
***************************************************************************/
                 if (mode == PGSd_PC_TEMPDEL_TERM)
                 {
                     returnStatus = PGS_PC_PutPCSDataMarkAtTerm(locationPCS,mode,
                                            (PGSt_PC_File_Struct *) PCS_data,
                                            PGSd_PC_DELETE_STRING);
                 } 
/***************************************************************************
*    Make the actual changes to the data.
***************************************************************************/
                 else
                 {
                     returnStatus = PGS_PC_PutPCSDataPutInArea(useMode,
                                            locationPCS,PCS_data);
                 }
             }
        }

/***************************************************************************
*    Let's save our return from the functions above.  That way we can 
*    go ahead and close the files and check our return later to see 
*    if we want to rename the file.
***************************************************************************/
        saveVal = returnStatus;
        returnStatus = PGS_PC_PutPCSDataFixBuffer(locationPCS);

/***************************************************************************
*    Everything came back successful from above, go ahead and rename
*    the file.
***************************************************************************/
        if ((saveVal == PGS_S_SUCCESS) || (saveVal == PGSPC_W_FILE_NOT_ON_DISK))
        {
            returnStatus = PGS_PC_GetPCFTemp(tempFileName);
            if (returnStatus == PGS_S_SUCCESS)
            {
                reCheck = rename(tempFileName,
                            getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                if (reCheck == -1)
                {
                    saveVal = PGSPC_E_PCF_UPDATE_FAILED;
                    if (whoCalled != PGSd_CALLERID_SMF)
                    {
                        PGS_SMF_GetMsgByCode(saveVal,msg);
                        sprintf(buf,msg,errno);
                        PGS_SMF_SetDynamicMsg(saveVal,buf,
                                         "PGS_PC_PutPCSData()");
                    }
                }
            }
        }
        else
        {
            returnStatus = PGS_PC_GetPCFTemp(tempFileName);
            if (returnStatus == PGS_S_SUCCESS)
            {
                reCheck = remove(tempFileName);
                if (reCheck == -1)
                {
                    saveVal = PGSPC_W_PCF_CLEANUP_WARN;
                    if (whoCalled != PGSd_CALLERID_SMF)
                    {
                        PGS_SMF_GetMsgByCode(saveVal,msg);
                        sprintf(buf,msg,errno);
                        PGS_SMF_SetDynamicMsg(saveVal,buf,
                                     "PGS_PC_PutPCSData()");
                    }
                }
            }
        }

/***************************************************************************
*    Update the return value and return.
***************************************************************************/
        returnStatus = saveVal;

    }  /* end else */

#ifdef _PGS_THREADSAFE
/***************************************************************************
*    Unlock here.
***************************************************************************/
    saveVal = PGS_TSF_UnlockIt(PGSd_TSF_PCLOCK);
    if ((returnStatus == PGS_S_SUCCESS) &&
        (PGS_SMF_TestErrorLevel(saveVal)))
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
    }
#endif

    return returnStatus;
}
