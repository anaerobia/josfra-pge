/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_InitCom.c

DESCRIPTION:
	This file contains all functions necessary to build PGS_PC_InitCom.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	21-Nov-94 RM Initial Version
	27-Dec-94 RM Updated from comments received at code inspection
			held on 22-Dec-94.
	05-Apr-95 RM Updated for TK5.  Moved default file locations
			from environment variables to the PCF.
	24-Apr-95 RM Updated for TK5.  Added functionality to receive
			the number of SMF records to store in shared
			memory from the command line.
	25-Apr-95 RM Changed type of smfCacheCount from PGSt_uinteger
			to PGSt_integer.
	19-Oct-95 DH Added return value support for MSS event log error.
        20-Oct-95 DH Inserted call to PGS_SMF_LogEvent() to announce startup.
        24-Oct-95 DH Removed declaration of static globals since they are now
		     private to SMF and an SMF routine has been developed to
		     provide this information.
        28-Dec-95 MS Replaced call in main() to PGS_SMF_GetSysShm with a call 
                     to PGS_SMF_GetGlobalVar, which in turn calls function
                     PGS_SMF_GetSysShm, but also does proper initialization
                     of the globalvar structure.   This allows later calls
                     to SMF functions from with this program to work properly.
                     Revised command line argument reading logic to make
                     sure that each argument was supplied, before attempting
                     to test its value.
        05-Jan-96 RM Updated for TK6.  Added Universal References to
                        file entries for files.  Changed function 
			PGS_PC_PutDataInArea() to write out universalRef
			to shared memory.
	15-Aug-96 RM Fixed error from DR ECSed01930.  An error code of 
			"15" was being returned from PGS_PC_InitCom.  This
			was being caused by a return value of PGS_E_ENV 
			being returned from PGS_SMF_GetGlobalVar().  It 
			was determined that if PGS_SMF_GetSysShm() were 
			still being called the error PGS_SH_SMF_LOGFILE
			would be returned.  
			PROGRAMMERS BEWARE:  IF ANY NEW FUNCTIONS ARE ADDED
			TO PGS_PC_InitCom YOU NEED TO ENSURE THAT ONLY _SH_
			TYPE RETURN VALUES MAY BE SENT OUT OF PGS_PC_InitCom.
        28-Apr-97 RM Changed check for invalid file record to include 
                        check of file name as well as the product id.

END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Main function for program PGS_PC_InitCom.
 
NAME:  
	PGS_PC_InitCom

SYNOPSIS:

C:
	SEE EXAMPLES

FORTRAN:
	NONE

DESCRIPTION:
	This program performs the initialization for the PGE.
 
INPUTS:
        Name            Description                     Units   Min     Max

	argc		number of command line arguments

	argv[0]		executable name (not processed 
			but listed here anyway)

	argv[1]		flag stating whether or not to 
			use shared memory

	argv[2]		flag stating whether or not to
			write to a log file

	argv[3]		number of SMF records to store in
			shared memory

OUTPUTS:
        Name            Description                     Units   Min     Max

	NONE

RETURNS:
	PGS_S_SUCCESS
	PGS_SH_MEM_INIT
	PGS_SH_SMF_LOGFILE
	PGS_SH_PC_LOADDATA
	PGS_SH_PC_ENV
	PGS_SH_SMF_SHMMEM
	PGS_SH_SMF_MSSLOGFILE

EXAMPLES:
	PGS_PC_InitCom ShmOn LogOn 50
	PGS_PC_InitCom ShmOff LogOn 100

NOTES:
	This program is intended to be run from within PGS_PC_Shell.sh and 
	is not designed to be run from the command line as a stand alone
	program.


REQUIREMENTS:
	PGSTK-1311

DETAILS:
	NONE
	

GLOBALS:
	NONE

FILES:
	NONE	

FUNCTIONS_CALLED:
	PGS_SMF_InitProc		- intialize the SMF process
	PGS_PC_GetMemoryNeeded		- get amount of shared memory needed
	PGS_MEM_ShmSysInit		- intialize shared memory
	PGS_PC_LoadPCSArea		- load PCS data into shared memory
	PGS_SMF_GetSysShm		- intialize the SMF & MSS log files
	PGS_SMF_GetGlobalVar		- retrieve Global SMF structure
        PGS_SMF_GetMsgByCode		- retrieve actual Status message
        PGS_SMF_GetMSSGlobals		- retrieve Event Log attributes
        PGS_SMF_GetActionMneByCode	- get action attributes
        PGS_SMF_TestStatusLevel		- check return status of prior call
        PGS_SMF_GetActionType		- get the type of action 
        PGS_SMF_LogEvent		- communicate with event log via SNMP

END_PROLOG:
***************************************************************************/

/***************************************************************************
*    Function prototypes for functions in this file.
***************************************************************************/
PGSt_SMF_status PGS_PC_GetMemoryNeeded(PGSMemShmSize *);
PGSt_SMF_status PGS_PC_LoadPCSArea(void);
PGSt_SMF_status PGS_PC_GetDataLocation(FILE **);
PGSt_SMF_status PGS_PC_PutDataInArea(FILE *);
PGSt_SMF_status PGS_PC_ParseNumFromLine(int, char, char *, char *);
PGSt_SMF_status PGS_PC_ParseCharFromLine(int, char, char *, char *);
PGSt_SMF_status PGS_PC_WriteHeader();
PGSt_SMF_status PGS_PC_DataToMem(void *, PGSt_uinteger);
PGSt_SMF_status PGS_PC_DataToMemChar(char,PGSt_uinteger,int);
PGSt_SMF_status PGS_PC_PutDefaultInMem(char [],int);

/***************************************************************************
*    Function main().
***************************************************************************/
int
main( 					 /* function main() */
    int                 argc,            /* number of command line arguments */
    char               *argv[])          /* array of command line arguments */
{
    PGSt_integer        smfCacheCount;   /* SMF message cache count */
    PGSMemShmSize       memNeeded;       /* amount of shared memory needed */
    PGSt_SMF_status     useMemType;      /* flag for shared memory */
    PGSt_SMF_status     writeLogFile;    /* flag for writing to log file */
    PGSt_SMF_status     returnStatus;    /* return from functions */

#ifdef PGS_IR1
    PGSt_SMF_status     statusCode;      /* return from functions */
    PGSt_SMF_status     returnMSSStatus; /* return from functions */
    char                actionString[PGS_SMF_MAX_ACT_SIZE];
                                         /* MSS action message string */
    char                actionLabel[PGS_SMF_MAX_MNEMONIC_SIZE];
                                         /* MSS channel action mnemonic */
    PGSSmfGlbVar    	*global_var;  	 /* global static buffer */
    char 		*eventRef;	 /* global event log file name */
    char 		*eventSwitch;	 /* global event log activation switch */
    char           	statusMsg[PGS_SMF_MAX_MSG_SIZE];
					 /* hold status/!action message */
#endif /* PGS_IR1 */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;



/***************************************************************************
*    Set the default values for command line arguments
***************************************************************************/

    useMemType = PGSd_SMF_USE_SHAREDMEM;    /* shared memory flag */
    writeLogFile = PGSd_SMF_WRITELOG_ON;    /* log file flag */
    smfCacheCount = 50;                     /* size of the SMF message count */


/***************************************************************************
*    Process command line arguments
***************************************************************************/

    switch (argc)
    {

      case 4:
        if (strlen(argv[3]) != 0)
        {
            smfCacheCount = atoi(argv[3]);
        }

      case 3:
        if (strcmp(argv[2],PGSd_PC_LOGOFF) == 0)
        {
            writeLogFile = PGSd_SMF_WRITELOG_OFF;
        }

      case 2:
        if (strcmp(argv[1],PGSd_PC_SHMOFF) == 0)
        {
            useMemType = PGSd_SMF_USE_ASCIIFILE;
        }

      case 1:
        break;

      default:
        fprintf (stderr, "PGS_PC_InitCom accepts up to 3 arguments.\n");
        return 1;

    }


/***************************************************************************
*   Inform SMF that this is the initialization process.
***************************************************************************/
    PGS_SMF_InitProc(useMemType,writeLogFile);

/***************************************************************************
*   Get amount of memory that Process Control Tools will need.
***************************************************************************/
    if (useMemType == PGSd_SMF_USE_SHAREDMEM)
    {

        returnStatus = PGS_PC_GetMemoryNeeded(&memNeeded);

        if (returnStatus != PGS_S_SUCCESS)
        {

/***************************************************************************
*   If we have not received success at this point then we know something
*   is seriously wrong with the PCF that will not allow the PGE to 
*   process.
***************************************************************************/
            exit(PGS_SH_PC_ENV);
        }

/***************************************************************************
*   Calculate amount of shared memory needed by SMF and initialize 
*   shared memory.
***************************************************************************/
        returnStatus = PGS_SMF_GetShmSize(&memNeeded,smfCacheCount);

        returnStatus = PGS_MEM_ShmSysInit(&memNeeded);

        if (returnStatus == PGS_S_SUCCESS)
        {

/***************************************************************************
*   Everything is OK.  Load the PC data into shared memory.
***************************************************************************/
            returnStatus = PGS_PC_LoadPCSArea();
        }
    }  /* end if */

/***************************************************************************
*   Initialize the SMF process.
***************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {
        returnStatus = PGS_SMF_GetGlobalVar((PGSSmfGlbVar **) NULL);

/***************************************************************************
*   It is possible to receive and environment error from 
*   PGS_SMF_GetGlobalVar().  We need to massage this to something the 
*   user can understand from PGS_PC_InitCom.  The reason that this 
*   was set to PGS_SM_SMF_LOGFILE is because this function call replaced
*   a call to PGS_SMF_GetSysShm() and the LOGFILE error would have been
*   returned at this point by that function.
*   Ray Milburn		15-Aug-96	DR ECSed01930
***************************************************************************/
        if (returnStatus == PGS_E_ENV)
        {
            returnStatus = PGS_SH_SMF_LOGFILE;
        }

        if (returnStatus == PGS_SH_SMF_MSSLOGFILE)
	{
	    goto EGRESS;
	}
    }

/***************************************************************************
*   Send Message to Event Log Indicating Status of Initialization
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
     if (returnStatus != PGS_S_SUCCESS)
     {
	statusCode = PGSPC_E_TOOLKIT_INIT_FAIL;
     }
     else
     {	
	statusCode = PGSPC_M_TOOLKIT_INIT_PASS;
     }
     PGS_SMF_GetMsgByCode( statusCode,statusMsg );
     returnMSSStatus = PGS_SMF_GetActionMneByCode(
                                        statusCode,
                                        actionString,
                                        actionLabel);

     if (PGS_SMF_TestStatusLevel(returnMSSStatus) == PGS_SMF_MASK_LEV_E)
     {
         returnStatus = PGS_SH_SMF_MSSLOGFILE;
	 goto EGRESS;
     }
 
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
 
#endif /* PGS_IR1 */

EGRESS:
/***************************************************************************
*   Return (for this it is exit) back to the shell.
***************************************************************************/
    exit(returnStatus);
}






/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get amount of shared memory needed by the Process Control Tools.
 
NAME:
	PGS_PC_GetMemoryNeeded()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetMemoryNeeded(
			PGSMemShmSize	*memNeeded);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to calculate the amount of shared memory that
	will be needed by the Process Control Tools during the PGE.
 
INPUTS:
	Name		Description			Units	Min	Max

	NONE

OUTPUTS:
	Name		Description			Units	Min	Max

	memNeeded       Amount of memory needed by the 
			Process Control Tools.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGS_SH_PC_ENV               problem accessing PCF

EXAMPLES:

C:
	PGSMemShmSize   memNeeded;
	PGSt_SMF_status	returnStatus;

	/# get amount of memory needed #/

	returnStatus = PGS_PC_GetMemoryNeeded(&memNeeded);

	if (returnStatus != PGS_S_SUCCESS)
	{
		goto EXCEPTION;
	}
	else
	{
		 /# continue operations with shared memory #/
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

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	This tool reads from the file defined in the environment variable 
	"PGS_PC_INFO_FILE" (see PGS_PC.h)

FUNCTIONS_CALLED:
	PGS_PC_GetDataLocation		- get location of PCS data

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_GetMemoryNeeded(           /* get amount of shared memory needed */
    PGSMemShmSize   *memNeeded)         /* amount of memory needed */

{
    /*char            *ret;                temp return for fgets() */
    char            line[PGSd_PC_LINE_LENGTH_MAX]; /* line read in fgets() */
    int             numLines;           /* number of lines in file */
    FILE            *tempPtr;           /* temporary file pointer */
    long            sizeOfFile;         /* size of PCF */
    PGSt_SMF_status returnStatus;       /* function return */

/***************************************************************************
*    Initialize our return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    tempPtr = (FILE *) NULL;

/***************************************************************************
*    Open the PCS file.
***************************************************************************/
    returnStatus = PGS_PC_GetDataLocation(&tempPtr);

    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*    Count the number of lines in the file.
***************************************************************************/
        numLines = 0;
        while ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,tempPtr)) != NULL)
        {
            numLines++;
        }

/***************************************************************************
*    Mutltiply the number of lines by the size of the file buffer, add
*    the size of the header structure and an extra buffer.
***************************************************************************/
        sizeOfFile = (sizeof(PGSt_PC_HeaderStruct_Shm) + 
                            (numLines * sizeof(PGSt_PC_File_Shm)) + 
                             PGSd_PC_EXTRA_MEM);
        memNeeded->pc = (PGSt_integer) sizeOfFile;
    }

/***************************************************************************
*    If the file opened successfully, close it.
***************************************************************************/
    if (tempPtr != (FILE *) NULL)
    {
        fclose(tempPtr);
    }     

/***************************************************************************
*    Return to the calling function.
***************************************************************************/
    return returnStatus;
}



/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Load the PCS data into shared memory.
 
NAME:
	PGS_PC_LoadPCSArea()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_LoadPCSArea(void);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to load the data located in the PCF into
	shared memory for accessing during the PGE.
 
INPUTS:
	Name		Description			Units	Min	Max

	NONE

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGS_SH_PC_LOADDATA          problem loading PC data into shared memory

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;

	/# loading the PCS data #/

	returnStatus = PGS_PC_LoadPCSArea();

	if (returnStatus != PGS_S_SUCCESS)
	{
		goto EXCEPTION;
	}
	else
	{
		 /# continue operations on the file #/
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

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	This tool opens the file defined in the environment variable 
	"PGS_PC_INFO_FILE" (see PGS_PC.h)

FUNCTIONS_CALLED:
	PGS_PC_GetDataLocation		- get location of PCS data
	PGS_MEM_ShmSysAddr		- get location of PC area of shared
					memory
	PGS_PC_PutDataInArea		- write data in shared memory

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_LoadPCSArea(void)        /* load PCS info into shared memory */

{
    char            *addrPC;         /* PC address in shared memory */
    PGSt_uinteger   size;            /* amount of shared memory for PC */
    FILE            *locationPCS;           /* file containing PCS data */
    PGSt_SMF_status returnStatus;           /* function return */

/***************************************************************************
*    Initialize our return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    locationPCS = (FILE *) NULL;

/***************************************************************************
*    Open the PCS file.
***************************************************************************/
    returnStatus = PGS_PC_GetDataLocation(&locationPCS);

    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*    If we get the address OK, then load the PC data.
***************************************************************************/
        returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC, 
                                          (void **) &addrPC,&size);

        if (returnStatus == PGS_S_SUCCESS)
        {
            returnStatus = PGS_PC_PutDataInArea(locationPCS);
        }
    }

/***************************************************************************
*    If the file opened successfully, close it.
***************************************************************************/
    if (locationPCS != (FILE *) NULL)
    {
        fclose(locationPCS);
    }     

/***************************************************************************
*    Set the return variable.
***************************************************************************/
    if (returnStatus != PGS_S_SUCCESS)
    {
        returnStatus = PGS_SH_PC_LOADDATA;
    }

/***************************************************************************
*    Return to the calling function.
***************************************************************************/
    return returnStatus;
}



/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Open the file containing the Process Control Status information.
 
NAME:
	PGS_PC_GetDataLocation()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetDataLocation(
			FILE		**locationPCS);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to open the file conataining the Process Control
	Status information but contains no SMF functionality.
 
INPUTS:
	Name		Description			Units	Min	Max

	NONE

OUTPUTS:
	Name		Description			Units	Min	Max

	locationPCS	the file pointer to the file 
			containing the Process Control 
			Status information.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGS_SH_PC_ENV               environment error

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;
	FILE		*locationPCS;

	/# opening the PCS file #/

	returnStatus = PGS_PC_GetDataLocation(&locationPCS);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# continue operations on the file #/
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

	This function is identical to PGS_PC_GetPCSDataOpenPCSFile() except
	that the SMF functionality was stripped out.  This was necessary due 
	to the fact that the SMF utilities will not be initialized when this
	function is called.  In other words this function was designed 
	primarily to be used in PGS_PC_InitCom.

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	This tool opens the file defined in the environment variable 
	"PGS_PC_INFO_FILE" (see PGS_PC.h)

FUNCTIONS_CALLED:

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_GetDataLocation(                /* open PCS info file */
    FILE            **locationPCS)           /* location of PCS data */

{
    char         *inFileN;                      /* input file */
    PGSt_SMF_status returnStatus;               /* function return */

/***************************************************************************
*    Initialize our return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Get the file name and path from the environment variable, if it
*    is not set, let's get outta here.
***************************************************************************/
    inFileN = getenv(PGSd_PC_INFO_FILE_ENVIRONMENT);
    if (!inFileN)
    {
        returnStatus = PGS_SH_PC_ENV;
    }

/***************************************************************************
*    Open input file, if there is a problem then just return to the 
*    calling function and tell them that they are not going to get
*    what they wanted due to a file problem. 
***************************************************************************/
    else if ((*locationPCS = fopen(inFileN,"r")) == NULL)
    {
        returnStatus = PGS_SH_PC_ENV;
    }

/***************************************************************************
*    Return to the calling function.
***************************************************************************/
    return returnStatus;
}







/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Read the PCS data and put it into the PC area of shared memory.
 
NAME:
	PGS_PC_PutDataInArea()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_PutDataInArea(
			FILE		*locationPCS);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to open the file conataining the Process Control
 
INPUTS:
	Name		Description			Units	Min	Max

	locationPCS	pointer to file contain PCS
			data

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_DATA		    problem loading data

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;

	/# loading the PCS data #/

	returnStatus = PGS_PC_PutDataInArea();

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# continue operations on the file #/
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

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	This tool opens the file defined in the environment variable 
	"PGS_PC_INFO_FILE" (see PGS_PC.h)

FUNCTIONS_CALLED:
	PGS_PC_WriteHeader		- initialize and write header 
					structure to shared memory
	PGS_PC_DataToMem		- write the data to shared memory
					while updating the header
	PGS_PC_ParseNumFromLine		- parse a number from line of
					input data
	PGS_PC_ParseCharFromLine	- parse a string from line of
					input data

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_PutDataInArea(             /* put PCS info into shared memory */
    FILE            *locationPCS)           /* file containing PCS data */

{
    char            line[PGSd_PC_LINE_LENGTH_MAX];  /* line from file */
    char            hold[PGSd_PC_LINE_LENGTH_MAX];  /* temporary hold */
   /*char            *tem;                    return check from fgets() */
    int             numDivs;                /* number of dividers found */
   /*int             len;                     length of string */
    int             delPos;                 /* delimiter position */
    PGSt_PC_SysConfig_Shm sysConfig;        /* system configuration data */
    PGSt_PC_UserConfig_Shm userConfig;      /* user configuration data */
    PGSt_PC_File_Shm fileData;              /* file data */
    PGSt_boolean    dividerFlag;            /* flag set when divider hit */
    PGSt_SMF_status returnStatus;           /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    numDivs = 0;
    dividerFlag = PGS_FALSE;

/***************************************************************************
*    Write a header out to shared memory.
***************************************************************************/
    returnStatus = PGS_PC_WriteHeader();

/***************************************************************************
*    Read a line from the PCF.
***************************************************************************/
    while ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,locationPCS)) != NULL)
    {

/***************************************************************************
*    If the line is a divider, write a divider out to shared memory,
*    increment our divider counter and continue.
***************************************************************************/
        if (line[0] == PGSd_PC_DIVIDER)
        {
            returnStatus = PGS_PC_DataToMemChar(PGSd_PC_DIVIDER,
                                             sizeof(char),numDivs++);
            dividerFlag = PGS_TRUE;
            continue;
        }

/***************************************************************************
*    If the line is a comment line or we have not hit any dividers
*    yet, just skip it.
***************************************************************************/
        if ((line[0] == PGSd_PC_COMMENT) || (numDivs == 0))
        {    
            continue;
        }

        switch (numDivs)
        {

/***************************************************************************
*    We are in the land of system configuration data.  Determine that 
*    the length of data is within limits and load it into the structure.  
*    Then, write it out to shared memory.
***************************************************************************/
            case PGSd_PC_SYS_CONFIG:
                sysConfig.flag = PGSd_PC_CHAR_NULL;
                if ((int) strlen(line) > PGSd_PC_VALUE_LENGTH_MAX)
                { 
                   strcpy(sysConfig.data,PGSd_PC_STRING_NULL);
                }
                else
                {
                    strcpy(sysConfig.data,line);
                }
                returnStatus = PGS_PC_DataToMem((void *) &sysConfig,
				sizeof(PGSt_PC_SysConfig_Shm));
                break;

/***************************************************************************
*    Here is where we handle file information.
***************************************************************************/
          case PGSd_PC_INPUT_FILES:
          case PGSd_PC_OUTPUT_FILES:
          case PGSd_PC_SUPPORT_INPUT:
          case PGSd_PC_SUPPORT_OUTPUT:
          case PGSd_PC_INTER_INPUT:
          case PGSd_PC_INTER_OUTPUT:
          case PGSd_PC_TEMP_INFO:

/***************************************************************************
*    If the line is a default location line and the last non-comment 
*    line read was a divider line then load the default location
*    into memory.
***************************************************************************/
              if ((line[0] == PGSd_PC_DEFAULT_LOC) && (dividerFlag == PGS_TRUE))
              {
                  returnStatus = PGS_PC_PutDefaultInMem(line,numDivs);
                  dividerFlag = PGS_FALSE;
                  continue;
              }

/***************************************************************************
*    If the line begins with a DEFAULT_LOC flag and the default
*    location has already been stored, just skip the line.
***************************************************************************/
              if ((line[0] == PGSd_PC_DEFAULT_LOC) && (dividerFlag == PGS_FALSE))
              {
                  continue;
              }

/***************************************************************************
*    We probably have a valid line.
***************************************************************************/
              else
              {

                  dividerFlag = PGS_FALSE;
                  fileData.flag = PGSd_PC_CHAR_NULL;
                  delPos = 0;

/***************************************************************************
*    Get the index - if there was a problem just load a NULL into the 
*    structure, this will ensure that the PC tools act the same if the
*    data is loaded into shared memory or read from a flat file at run-
*    time.  THIS IS OUR FLAG - BOTH THE INDEX AND FILENAME LOADED IN AS
*    NULL!!  THAT MEANS THAT THERE WAS A PROBLEM WITH THIS LINE AND STILL
*    ALLOWS THE USER TO USE ZERO (0) AS A VALID PRODUCT ID.
***************************************************************************/
                  returnStatus = PGS_PC_ParseNumFromLine(delPos++,
                                       PGSd_PC_DELIMITER,line,hold);

                  if (hold[0] == PGSd_PC_CHAR_NULL)
                  {
                      fileData.fileStruct.index = (PGSt_PC_Logical) NULL;
                      fileData.fileStruct.fileName[0] = PGSd_PC_CHAR_NULL;
                  }
                  else
                  {
                      fileData.fileStruct.index = (PGSt_PC_Logical) atol(hold);

/***************************************************************************
*    Get the file name.
***************************************************************************/
                      returnStatus = PGS_PC_ParseCharFromLine(delPos++,
                                      PGSd_PC_DELIMITER,line,hold);
                      if ((hold[0] == PGSd_PC_CHAR_NULL) || 
                          (strlen(hold) > (size_t) PGSd_PC_FILE_NAME_MAX))
                      {
                          strcpy(fileData.fileStruct.fileName,PGSd_PC_STRING_NULL);
                      }
                      else
                      {
                          strcpy(fileData.fileStruct.fileName,hold);
                      }
                  }  /* end else */

/***************************************************************************
*    Get the path.
***************************************************************************/
                  returnStatus = PGS_PC_ParseCharFromLine(delPos++,
                                      PGSd_PC_DELIMITER,line,hold);
                  if ((hold[0] == PGSd_PC_CHAR_NULL) || 
                      (strlen(hold) > (size_t) PGSd_PC_PATH_LENGTH_MAX))
                  {
                      strcpy(fileData.fileStruct.path,PGSd_PC_STRING_NULL);
                  }
                  else
                  {
                      strcpy(fileData.fileStruct.path,hold);
                  }

/***************************************************************************
*    Get the size.
***************************************************************************/
                  returnStatus = PGS_PC_ParseNumFromLine(delPos++,
                                      PGSd_PC_DELIMITER,line,hold);
                  if (hold[0] == PGSd_PC_CHAR_NULL)
                  {
                      fileData.fileStruct.size = 0;
                  }
                  else
                  {
                      fileData.fileStruct.size = (PGSt_uinteger) atol(hold);
                  }

/***************************************************************************
*    Get the universal reference.
***************************************************************************/
                  returnStatus = PGS_PC_ParseCharFromLine(delPos++,
                                      PGSd_PC_DELIMITER,line,hold);
                  if ((hold[0] == PGSd_PC_CHAR_NULL) || 
                      (strlen(hold) > (size_t) PGSd_PC_UREF_LENGTH_MAX))
                  {
                      strcpy(fileData.fileStruct.universalRef,PGSd_PC_STRING_NULL);
                  }
                  else
                  {
                      strcpy(fileData.fileStruct.universalRef,hold);
                  }

/***************************************************************************
*    Get the attribute location.
***************************************************************************/
                  returnStatus = PGS_PC_ParseCharFromLine(delPos++,
                                      PGSd_PC_DELIMITER,line,hold);
                  if ((hold[0] == PGSd_PC_CHAR_NULL) || 
                      (strlen(hold) > (size_t) PGSd_PC_FILE_NAME_MAX))
                  {
                      strcpy(fileData.fileStruct.attributeLoc,
                                                   PGSd_PC_STRING_NULL);
                  }
                  else
                  {
                      strcpy(fileData.fileStruct.attributeLoc,hold);
                  }

/***************************************************************************
*    Get the number of entries.
***************************************************************************/
                  returnStatus = PGS_PC_ParseNumFromLine(delPos++,
                                      PGSd_PC_NEWLINE,line,hold);
                  if (hold[0] == PGSd_PC_CHAR_NULL)
                  {
                      fileData.fileStruct.entries = 0;
                  }
                  else
                  {
                      fileData.fileStruct.entries = (PGSt_uinteger) atol(hold);
                  }

                  returnStatus = PGS_PC_DataToMem((void *) &fileData,
		                             sizeof(PGSt_PC_File_Shm));
              }
              break;

/***************************************************************************
*    Here is where we handle user configuration parameters.
***************************************************************************/
          case PGSd_PC_CONFIG_COUNT:
              userConfig.flag = PGSd_PC_CHAR_NULL;
              delPos = 0;

/***************************************************************************
*    Get the index.
***************************************************************************/
              returnStatus = PGS_PC_ParseNumFromLine(delPos++,PGSd_PC_DELIMITER,
                                                     line,hold);
              if (hold[0] == PGSd_PC_CHAR_NULL)
              {
                  userConfig.configStruct.index = (PGSt_PC_Logical) NULL;
              }
              else
              {
                  userConfig.configStruct.index = (PGSt_PC_Logical) atol(hold);
              }

/***************************************************************************
*    Get the ID.
***************************************************************************/
              returnStatus = PGS_PC_ParseCharFromLine(delPos++,PGSd_PC_DELIMITER,
                                                      line,hold);
              if ((hold[0] == PGSd_PC_CHAR_NULL) || 
                  (strlen(hold) > (size_t) PGSd_PC_ID_LENGTH_MAX))
              {
                  strcpy(userConfig.configStruct.identity,PGSd_PC_STRING_NULL);
              }
              else
              {
                  strcpy(userConfig.configStruct.identity,hold);
              }

/***************************************************************************
*    Get the value.
***************************************************************************/
              returnStatus = PGS_PC_ParseCharFromLine(delPos++,PGSd_PC_NEWLINE,
                                                      line,hold);
              if ((hold[0] == PGSd_PC_CHAR_NULL) || 
                  (strlen(hold) > (size_t) PGSd_PC_VALUE_LENGTH_MAX))
              {
                  strcpy(userConfig.configStruct.value,PGSd_PC_STRING_NULL);
              }
              else
              {
                  strcpy(userConfig.configStruct.value,hold);
              }

              returnStatus = PGS_PC_DataToMem((void *) &userConfig,
                                         sizeof(PGSt_PC_UserConfig_Shm));
              break;

/***************************************************************************
*    The file has information beyond the last divider, just ignore it.
***************************************************************************/
          default:
              fseek(locationPCS,0,SEEK_END);
              break;

        }  /* end switch */

    }  /* end while */

/***************************************************************************
*    Return to the calling function.
***************************************************************************/
    return returnStatus;
}





/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get requested data from line of Process Control
	Status data.
 
NAME:
	PGS_PC_ParseNumFromLine()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_ParseNumFromLine(
			int			num,
			char			endChar,
			char			*line,
			char			*out);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to parse a line of Process Control Status
	information data.
 
INPUTS:
	Name		Description			Units	Min	Max

	num		number of delimiters to count

	endChar		after counting enough delimiters
			stop after seeing this character

	line		line of Process Control Status
			information data

OUTPUTS:
	Name		Description			Units	Min	Max

	out		output in string format

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;
	int		num;
	char		endChar;
	char		line[PGSd_PC_LINE_LENGTH_MAX];
	char		out[PGSd_PC_LINE_LENGTH_MAX];

	/# after successfully getting a line of data get the data
	   between the 3rd and 4th delimiters #/
	   

	num = 3;
	endChar = PGSd_PC_DELIMITER;
	returnStatus = PGS_PC_ParseNumFromLine(num,endChar,line,out);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# perform necessary operations on "out" #/
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

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_ParseNumFromLine(      /* get reqested data from line */
    int         num,          /* number of DELIMITER(s) to count */
    char        endChar,      /* last character to look for */
    char       *line,         /* line to search */
    char       *out)          /* string returned */
{
    int         count;        /* number of DELIMITER(s) - so far */
    int         inPos;        /* input line position */
    int         outPos;       /* output string position */
    int         len;          /* length of line */
    PGSt_SMF_status returnStatus;    /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS; 
    count = 0;
    inPos = 0;
    strcpy(out,PGSd_PC_STRING_NULL);

/***************************************************************************
*    Let's just loop through the line counting the number of endChar's.
***************************************************************************/
    while (count < num)
    {

/***************************************************************************
*    If we hit a new-line character then we have a problem since we 
*    have not hit the number of endChar's that we are supposed to.
***************************************************************************/
        if (line[inPos] == PGSd_PC_NEWLINE)
        {
            return returnStatus;
        }

/***************************************************************************
*    If we hit a DELIMITER character then we increment our counter.
***************************************************************************/
        else if (line[inPos] == PGSd_PC_DELIMITER)
        {
            count++;
        }

/***************************************************************************
*    Of course we need to increment our position indicator...how else
*    we going to look at the next character.
***************************************************************************/
        inPos++;
    }   /* end while */


/***************************************************************************
*    returnStatus did not change.  That means that we have success.  
*    Just load the characters up to our endChar into the string that 
*    gets passed back and we are in good shape (programmatically speaking).
***************************************************************************/
    outPos = 0;
    len = (int) strlen(line);
    while ((line[inPos] != endChar) && (inPos <= len))
    {
        if ((line[inPos] >= PGSd_PC_LOWDIGIT) && 
            (line[inPos] <= PGSd_PC_HIDIGIT))
        {
            out[outPos] = line[inPos];
            outPos++;
            inPos++;
        }
        else
        {
            outPos = 0;
            break;
        }
    }  /* end while */

/***************************************************************************
*    If outPos is zero then the requested data is not on the line.
***************************************************************************/
    if (outPos == 0)
    {
        strcpy(out,PGSd_PC_STRING_NULL);
    }
    else
    {
        out[outPos] = '\0';
    }

/***************************************************************************
*    Return to the calling function.
***************************************************************************/
    return returnStatus;
}







/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get requested data from line of Process Control
	Status data.
 
NAME:
	PGS_PC_ParseCharFromLine()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_ParseCharFromLine(
			int			num,
			char			endChar,
			char			*line,
			char			*out);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to parse a line of Process Control Status
	information data.
 
INPUTS:
	Name		Description			Units	Min	Max

	num		number of delimiters to count

	endChar		after counting enough delimiters
			stop after seeing this character

	line		line of Process Control Status
			information data

OUTPUTS:
	Name		Description			Units	Min	Max

	out		output in string format

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;
	int		num;
	char		endChar;
	char		line[PGSd_PC_LINE_LENGTH_MAX];
	char		out[PGSd_PC_LINE_LENGTH_MAX];

	/# after successfully getting a line of data get the data
	   between the 3rd and 4th delimiters #/
	   

	num = 3;
	endChar = PGSd_PC_DELIMITER;
	returnStatus = PGS_PC_ParseCharFromLine(num,endChar,line,out);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# perform necessary operations on "out" #/
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

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_ParseCharFromLine(     /* get reqested data from line */
    int         num,          /* number of DELIMITER(s) to count */
    char        endChar,      /* last character to look for */
    char       *line,         /* line to search */
    char       *out)          /* string returned */
{
    int         count;        /* number of DELIMITER(s) - so far */
    int         inPos;        /* input line position */
    int         outPos;       /* output string position */
    int         len;          /* length of line */
    PGSt_SMF_status returnStatus;    /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS; 
    count = 0;
    inPos = 0;
    strcpy(out,PGSd_PC_STRING_NULL);

/***************************************************************************
*    Let's just loop through the line counting the number of endChar's.
***************************************************************************/
    while (count < num)
    {

/***************************************************************************
*    If we hit a new-line character then we have a problem since we 
*    have not hit the number of endChar's that we are supposed to.
***************************************************************************/
        if (line[inPos] == PGSd_PC_NEWLINE)
        {
            return returnStatus;
        }

/***************************************************************************
*    If we hit a DELIMITER character then we increment our counter.
***************************************************************************/
        else if (line[inPos] == PGSd_PC_DELIMITER)
        {
            count++;
        }

/***************************************************************************
*    Of course we need to increment our position indicator...how else
*    we going to look at the next character.
***************************************************************************/
        inPos++;
    }   /* end while */


/***************************************************************************
*    returnStatus did not change.  That means that we have success.  
*    Just load the characters up to our endChar into the string that 
*    gets passed back and we are in good shape (programmatically speaking).
***************************************************************************/
    outPos = 0;
    len = (int) strlen(line);
    while ((line[inPos] != endChar) && (inPos <= len))
    {
        out[outPos] = line[inPos];
        outPos++;
        inPos++;
    }  /* end while */

/***************************************************************************
*    If outPos is zero then the requested data is not on the line.
***************************************************************************/
    if (outPos == 0)
    {
        strcpy(out,PGSd_PC_STRING_NULL);
    }
    else
    {
        out[outPos] = '\0';
    }

/***************************************************************************
*    Set our message in the message log and go back to calling function.
***************************************************************************/
    return returnStatus;
}






/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Write out header to shared memory at initialization of PC portion
	shared memory.
 
NAME:
	PGS_PC_WriteHeader()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_WriteHeader();

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to initialize and write the header structure
	out to shared memory before PC has written any data to shared
	memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	NONE

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;

	/# initialize header structure and write to shared memory #/
	   
	returnStatus = PGS_PC_WriteHeader();

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# perform necessary operations #/
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

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_WriteHeader()          /* init header */
{
    char            *addrPC;         /* PC address in shared memory */
    int             count;           /* loop counter */
    PGSt_uinteger   size;            /* amount of shared memory for PC */
    PGSt_PC_HeaderStruct_Shm headStruct;  /* structure to initialize */
    PGSt_SMF_status returnStatus;    /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS; 

/***************************************************************************
*    Get address of PC area of shared memory.
***************************************************************************/
    returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,(void **) &addrPC,&size);

/***************************************************************************
*    Determine amount of memory that will be used by writing the header
*    and set all the division and default location pointers to NULL.
***************************************************************************/
    headStruct.amountUsed = sizeof(PGSt_PC_HeaderStruct_Shm);

    headStruct.divider = PGSd_PC_DIVIDER;

    for (count = 0; count < PGSd_PC_TOTAL_SEPARATORS; count++)
    {
       headStruct.divPtr[count] = (PGSt_uinteger) NULL;
    }

    for (count = 0; count < PGSd_PC_FILE_TYPES; count++)
    {
        strcpy(headStruct.defaultLoc[count],PGSd_PC_STRING_NULL);
    }

/***************************************************************************
*    Write out the header to shared memory.
***************************************************************************/
    memcpy(addrPC,(char *) &headStruct,sizeof(PGSt_PC_HeaderStruct_Shm));

/***************************************************************************
*    Return to calling function.
***************************************************************************/
    return returnStatus;
}






/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Write out the data to shared memory.
 
NAME:
	PGS_PC_DataToMem()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_DataToMem(
			void		*dataToWrite,
			PGSt_uinteger	sizeOfData,
			int		locationPtr);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to initialize and write the header structure
	out to shared memory before PC has written any data to shared
	memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	dataToWrite	The actual data to be written
			out to shared memory.

	sizeOfData	Size (in bytes) of data type
			that was passed in to
			dataToWrite.

	locationPtr	If we are writing a divider
			this will contain the divider
			number, otherwise a NULL should
			be passed in.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
	SomeStruct	dataToWrite;
	PGSt_SMF_status	returnStatus;

	/# populate structure and write to shared memory #/
	   
	returnStatus = PGS_PC_DataToMem((void *) &dataToWrite,sizeof(SomeStruct),NULL);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# perform necessary operations #/
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

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_DataToMem(              /* write data to shared memory */
    void             *dataToWrite,     /* data to be written */
    PGSt_uinteger     sizeOfData)      /* size of data to be written */
{
    char            *addrPC;         /* PC address in shared memory */
    char            *writeTo;        /* write to this address */
    PGSt_uinteger   size;            /* amount of shared memory for PC */
    PGSt_PC_HeaderStruct_Shm headStruct;  /* header */
    PGSt_SMF_status   returnStatus;    /* function return */

/***************************************************************************
*    Initialize variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Get address of PC area of shared memory.
***************************************************************************/
    returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,(void **) &addrPC,&size);

    if (returnStatus == PGS_S_SUCCESS)
    {
/***************************************************************************
*    Get header from shared memory and determine if we have enough
*    space left in shared memory.
***************************************************************************/
        memcpy((void *) &headStruct,(void *) addrPC,
                            sizeof(PGSt_PC_HeaderStruct_Shm)); 

        if (size >= (headStruct.amountUsed + sizeOfData))
        {

/***************************************************************************
*    Calculate address to write to, write the data, re-calc amount of 
*    shared memory used and write the header structure.
***************************************************************************/
            writeTo = (char *) ((unsigned long) (addrPC) + headStruct.amountUsed);

            memcpy(writeTo,(char *) dataToWrite,sizeOfData);

            headStruct.amountUsed += sizeOfData;

            memcpy(addrPC,(char *) &headStruct,sizeof(PGSt_PC_HeaderStruct_Shm));
        }

/***************************************************************************
*    We exceeded our bounds.
***************************************************************************/
        else
        {
            returnStatus = PGS_SH_PC_LOADDATA;
        }
    }  /* end if */

/***************************************************************************
*    Return to the calling function.
***************************************************************************/
    return returnStatus;
}




/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Write out the character data to shared memory.
 
NAME:
	PGS_PC_DataToMemChar()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_DataToMemChar(
			char		dataToWrite,
			PGSt_uinteger	sizeOfData,
			int		locationPtr);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to initialize and write the header structure
	out to shared memory before PC has written any data to shared
	memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	dataToWrite	The actual data to be written
			out to shared memory.

	sizeOfData	Size (in bytes) of data type
			that was passed in to
			dataToWrite.

	locationPtr	If we are writing a divider
			this will contain the divider
			number, otherwise a NULL should
			be passed in.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
	SomeStruct	dataToWrite;
	PGSt_SMF_status	returnStatus;

	/# populate structure and write to shared memory #/
	   
	returnStatus = PGS_PC_DataToMem((void *) &dataToWrite,sizeof(SomeStruct),NULL);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# perform necessary operations #/
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

END_PROLOG:
***************************************************************************/



PGSt_SMF_status
PGS_PC_DataToMemChar(              /* write data to shared memory */
    char              dataToWrite,     /* data to be written */
    PGSt_uinteger     sizeOfData,      /* size of data to be written */
    int               locationPtr)     /* location pointer */
{
    char            *addrPC;         /* PC address in shared memory */
    char            *writeTo;        /* write to this address */
    PGSt_uinteger   size;            /* amount of shared memory for PC */
    PGSt_PC_HeaderStruct_Shm headStruct;  /* header */
    PGSt_SMF_status   returnStatus;    /* function return */

/***************************************************************************
*    Initialize variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Get address of PC area of shared memory.
***************************************************************************/
    returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,(void **) &addrPC,&size);

    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*    Get header from shared memory and determine if we have enough
*    space left in shared memory.
***************************************************************************/
        memcpy((void *) &headStruct,(void *) addrPC,
                            sizeof(PGSt_PC_HeaderStruct_Shm));

        if (size >= (headStruct.amountUsed + sizeOfData))
        {

/***************************************************************************
*    Calculate address to write to, write the data, re-calc amount of 
*    shared memory used, set the divider pointer offset,  and write 
*    the header structure.
***************************************************************************/
            writeTo = (char *) ((unsigned long) (addrPC) + headStruct.amountUsed);

            memcpy(writeTo,(char *) &dataToWrite,sizeOfData);
    
            headStruct.divPtr[locationPtr] = headStruct.amountUsed;
            headStruct.amountUsed += sizeOfData;

            memcpy(addrPC,(char *) &headStruct,sizeof(PGSt_PC_HeaderStruct_Shm));
        }

/***************************************************************************
*    We exceeded our bounds.
***************************************************************************/
        else
        {
            returnStatus = PGS_SH_PC_LOADDATA;
        }
    }  /* end if */

/***************************************************************************
*    Return to the calling function.
***************************************************************************/
    return returnStatus;
}





/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Put default location in header structure and load it into
	shared memory.
 
NAME:
	PGS_PC_PutDefaultInMem()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_PutDefaultInMem(
    			char            line[PGSd_PC_LINE_LENGTH_MAX],
			int		numDivs);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to parse the default location of
	files (paths) from the PCF and place the location into
	the header structure and write that structure out to
	shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	line		Line of data read from the
			PCF containing the path of
			the default location of the
			type of file.

	numDivs		Number of dividers read from
			the PCF.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
    	char            line[PGSd_PC_LINE_LENGTH_MAX];
	int		numDivs;
	PGSt_SMF_status	returnStatus;

	/# read from PCF and increment numDivs as necessary #/

	/# if the line contains the default location ..... #/
	   
	if (line[0] == PGSd_PC_DEFAULT_LOC)
	{
		returnStatus = PGS_PC_PutDefaultInMem(line,numDivs);
	}

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# perform necessary operations #/
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

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:

END_PROLOG:
***************************************************************************/



PGSt_SMF_status
PGS_PC_PutDefaultInMem(              /* put default location in Shm */
    char            line[PGSd_PC_LINE_LENGTH_MAX],  /* line from PCF */
    int             numDivs)         /* number of dividers read */
{
    char            *addrPC;         /* PC address in shared memory */
    char            temp[PGSd_PC_LINE_LENGTH_MAX];  /* temp hold */
    int             pos;             /* character position in line */
    int             tempPos;         /* character position in temp */
    int             index;           /* array index */
    PGSt_uinteger   size;            /* amount of shared memory for PC */
    PGSt_PC_HeaderStruct_Shm headStruct;  /* header */
    PGSt_SMF_status   returnStatus;    /* function return */

/***************************************************************************
*    Initialize variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Get address of PC area of shared memory.
***************************************************************************/
    returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,(void **) &addrPC,&size);

    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*    Get header from shared memory.
***************************************************************************/
        memcpy((void *) &headStruct,(void *) addrPC,
                            sizeof(PGSt_PC_HeaderStruct_Shm));

/***************************************************************************
*    Find first character of default location.
***************************************************************************/
        pos = 1;
        while ((!(isgraph(line[pos]))) && (line[pos] != PGSd_PC_NEWLINE))
        {
            pos++;
        }

/***************************************************************************
*    Copy default location from line to a temporary string.
***************************************************************************/
        tempPos = 0;
        while (line[pos] != PGSd_PC_NEWLINE)
        {
            temp[tempPos] = line[pos];
            tempPos++;
            pos++;
        }

/***************************************************************************
*    Ensure that our string has at least one character and that it does
*    not exceed our allowable path length.  Determine which default
*    location we are dealing with and put the string into the header
*    structure and write the structure back out to shared memory.
***************************************************************************/
        if (tempPos > 0)
        {
            temp[tempPos] = PGSd_PC_CHAR_NULL;

            if ((int) strlen(temp) <= PGSd_PC_PATH_LENGTH_MAX)
            {

                returnStatus = PGS_PC_CalcArrayIndex(numDivs,
                                        PGSd_PC_DIVS_VALUE,&index);
                if (returnStatus == PGS_S_SUCCESS)
                {
                    strcpy(headStruct.defaultLoc[index],temp);
                    memcpy(addrPC,(char *) &headStruct,
                           sizeof(PGSt_PC_HeaderStruct_Shm));
                }
            }
        }

    }  /* end if */

/***************************************************************************
*    Return to the calling function.
***************************************************************************/
    return returnStatus;
}
