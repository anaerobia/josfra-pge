/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetDataFromShm.c

DESCRIPTION:
	This file contains the function PGS_PC_GetDataFromShm().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	08-Dec-94 RM Initial version
	05-Apr-95 RM Updated for TK5.  Added default location code.
	12-Dec-95 RM Updated for TK6.  Added functionality to allow
			multiple instances of PRODUCT OUTPUT FILES.
        05-Jan-96 RM Updated for TK6.  Added Universal References to
                        file entries for files.
	21-Oct-96 RM Updated for DR ECSed04042.  Removed code that 
			called PGS_SMF_SetStaticMsg() at end of this
			function.  It was causing too many messages
			to be placed in the log file.
	24-Apr-97 RM Added functionality to allow multiple versions
			of SUPPORT INPUT and SUPPORT OUTPUT files.

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
	Get the requested PCS data from shared memory.
 
NAME:
	PGS_PC_GetDataFromShm()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetDataFromShm(
			char 			*addrPC,
			PGSt_integer		mode,
			PGSt_PC_Logical		identifier,
			PGSt_integer		*numFiles,
			char			*outData);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to locate requested PCS data from shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	addrPC		pointer to address of beginning
			location of Process Control Status
			information in shared memory.

	mode		context values that will be 
			#define'd in a header file.
			Possible values are:

			PGSd_PC_PRODUCTION_RUN_ID
			PGSd_PC_SOFTWARE_ID
			PGSd_PC_CONFIGURATION
			PGSd_PC_INPUT_FILE_NAME
			PGSd_PC_INPUT_FILE_ATTRIBUTE
			PGSd_PC_INPUT_FILE_NUMFILES
			PGSd_PC_PRODUCT_IN_DEFLOC
			PGSd_PC_PRODUCT_IN_UREF
			PGSd_PC_OUTPUT_FILE_NAME
			PGSd_PC_OUTPUT_FILE_ATTRIBUTE
			PGSd_PC_OUTPUT_FILE_NUMFILES
			PGSd_PC_PRODUCT_OUT_DEFLOC
			PGSd_PC_PRODUCT_OUT_UREF
			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_TEMP_FILE_DEFLOC
			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_SUPPORT_IN_NAME
			PGSd_PC_SUPPORT_IN_ATTR
			PGSd_PC_SUPPORT_IN_DEFLOC
			PGSd_PC_SUPPORT_IN_UREF
			PGSd_PC_SUPPORT_IN_NUMFILES
			PGSd_PC_SUPPORT_OUT_NAME
			PGSd_PC_SUPPORT_OUT_ATTR
			PGSd_PC_SUPPORT_OUT_DEFLOC
			PGSd_PC_SUPPORT_OUT_UREF
			PGSd_PC_SUPPORT_OUT_NUMFILES
			PGSd_PC_INTER_IN_DEFLOC
			PGSd_PC_INTER_IN_UREF
			PGSd_PC_INTER_OUT_DEFLOC
			PGSd_PC_INTER_OUT_UREF

	identifier	The logical identifier as defined 
			by the user.  The user's 
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

	numFiles	this value state which version 
			of the INPUT file to receive the 
			requested information on.

OUTPUTS:
	Name		Description			Units	Min	Max

	outData		reqeusted data.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_INVALID_MODE        mode value does not match those in PGS_PC.h
	PGSPC_W_INVALID_VERSION     version number is less than 1
	PGSPC_E_NULL_DIVPOINTER_SHM divider pointer address is NULL

EXAMPLES:

C:
	#define		CERESL4	101

	char		*addrPC;
	PGSt_uinteger	size;
	PGSt_SMF_status	returnStatus;
	PGSt_PC_Logical	identifier;
	PGSt_integer	*numFiles;
	PGSt_integer	mode;
	char		outData[PGSd_PC_FILE_PATH_MAX];


	/# get the first input file of identifier 101 #/

	identifier = CERESL4;
	*numFiles = 1;
	mode = PGSd_PC_INPUT_FILE_NAME;

	/# get the address in shared memory of the PC data #/
	
	returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,
				(void **) &addrPC,&size);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;

	returnStatus = PGS_PC_GetDataFromShm(addrPC,mode,identifier,
		numFiles,outData);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# continue with file operations #/
		}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	All configuration parameters, except for physical file name(s), will
	have a one-to-one correspondence between a logical value and a  
	parameter value (string).

	In order for this tool to function properly, a valid Process Control
	file will need to be created first.  Please refer to Appendix C 
	(User's Guide) for instructions on how to create such a file.  This
	file must be processed through the PC initiazation program 
	PGS_PC_InitCom.

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message
	PGS_SMF_TestStatusLevel		 Determine the level of the 
					 error/status code
	PGS_PC_SearchShm		 Search through shared memory for 
					 requested data

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetDataFromShm( 	                 /* get data from shared memory */
    char             *addrPC,            /* address of PC data */
    PGSt_integer      mode,              /* mode requested */
    PGSt_PC_Logical   identifier,        /* logical id */
    PGSt_integer     *numFiles,          /* number of files */
    char             *outData)           /* requested data */

{
    char            buf[PGS_SMF_MAX_MSG_SIZE];    /* message buffer */
    char            msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char            *sectionLoc;         /* section pointer */
    PGSt_uinteger   offSet;              /* offset in memory */
    PGSt_boolean    byPass;              /* flag to bypass NULL offset */
    PGSt_PC_HeaderStruct_Shm headStruct; /* header in shared memory */
    PGSt_SMF_status whoCalled;           /* user that called this function */
    PGSt_SMF_status returnStatus;        /* function return variable */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    byPass = PGS_FALSE;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Get the header structure from shared memory.
***************************************************************************/
    memcpy((void *) &headStruct,(void *) addrPC,
                        sizeof(PGSt_PC_HeaderStruct_Shm));

/***************************************************************************
*    Based on the mode let's set our offSet that is in the header 
*    structure.  This will speed up our search a bit.
***************************************************************************/
    switch (mode)  
    {

/***************************************************************************
*    System defined configuration parameters.
***************************************************************************/
        case PGSd_PC_PRODUCTION_RUN_ID:
        case PGSd_PC_SOFTWARE_ID:
            offSet = headStruct.divPtr[PGSd_PC_SYS_CONFIG-1];
            break;

/***************************************************************************
*    User defined configuration parameters.
***************************************************************************/
        case PGSd_PC_CONFIGURATION:
            offSet = headStruct.divPtr[PGSd_PC_CONFIG_COUNT-1];
            break;

/***************************************************************************
*    Input file information.
***************************************************************************/
        case PGSd_PC_INPUT_FILE_NAME:
        case PGSd_PC_INPUT_FILE_ATTRIBUTE:
        case PGSd_PC_INPUT_FILE_NUMFILES:
        case PGSd_PC_PRODUCT_IN_UREF:
            offSet = headStruct.divPtr[PGSd_PC_INPUT_FILES-1];
            break;

/***************************************************************************
*    Output file information.
***************************************************************************/
        case PGSd_PC_OUTPUT_FILE_NAME:
        case PGSd_PC_OUTPUT_FILE_ATTRIBUTE:
        case PGSd_PC_OUTPUT_FILE_NUMFILES:
        case PGSd_PC_PRODUCT_OUT_UREF:
            offSet = headStruct.divPtr[PGSd_PC_OUTPUT_FILES-1];
            break;

/***************************************************************************
*    Temporary file information.
***************************************************************************/
        case PGSd_PC_TEMPORARY_FILE:
            offSet = headStruct.divPtr[PGSd_PC_TEMP_INFO-1];
            break;

/***************************************************************************
*    Intermediate input file information.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_INPUT:
        case PGSd_PC_INTER_IN_UREF:
            offSet = headStruct.divPtr[PGSd_PC_INTER_INPUT-1];
            break;

/***************************************************************************
*    Intermediate output file information.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_OUTPUT:
        case PGSd_PC_INTER_OUT_UREF:
            offSet = headStruct.divPtr[PGSd_PC_INTER_OUTPUT-1];
            break;

/***************************************************************************
*    Auxiliary input file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_IN_NAME:
        case PGSd_PC_SUPPORT_IN_ATTR:
        case PGSd_PC_SUPPORT_IN_UREF:
        case PGSd_PC_SUPPORT_IN_NUMFILES:
            offSet = headStruct.divPtr[PGSd_PC_SUPPORT_INPUT-1];
            break;

/***************************************************************************
*    Auxiliary output file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_OUT_NAME:
        case PGSd_PC_SUPPORT_OUT_ATTR:
        case PGSd_PC_SUPPORT_OUT_UREF:
        case PGSd_PC_SUPPORT_OUT_NUMFILES:
            offSet = headStruct.divPtr[PGSd_PC_SUPPORT_OUTPUT-1];
            break;

/***************************************************************************
*    Find default location, this is the old environment variable that
*    was moved to the PCF for TK5.
***************************************************************************/
	case PGSd_PC_PRODUCT_IN_DEFLOC:
        case PGSd_PC_PRODUCT_OUT_DEFLOC:
        case PGSd_PC_TEMP_FILE_DEFLOC:
        case PGSd_PC_INTER_IN_DEFLOC:
        case PGSd_PC_INTER_OUT_DEFLOC:
        case PGSd_PC_SUPPORT_IN_DEFLOC:
        case PGSd_PC_SUPPORT_OUT_DEFLOC:
            offSet = 0;
            byPass = PGS_TRUE;
            break;

/***************************************************************************
*    Unknown mode.  See PGS_PC.h for a list of modes.
***************************************************************************/
        default:
            returnStatus = PGSPC_E_INVALID_MODE;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                sprintf(buf,msg,mode);
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                  "PGS_PC_GetDataFromShm()");
            }
            break;
    }   /* end switch */

/***************************************************************************
*    If everything has gone OK up to here then we will proceed.
***************************************************************************/
    if ((returnStatus == PGS_S_SUCCESS) && 
       ((offSet != (PGSt_uinteger) NULL) || (byPass == PGS_TRUE)))
    {

/***************************************************************************
*    Advance our PCS data location to the area that contains the section
*    of data that we are looking for.
***************************************************************************/
        sectionLoc = (char *) ((unsigned long) addrPC + offSet);

/***************************************************************************
*    Remember, there can be a many-to-one relationship with PRODUCT
*    INPUT, PRODUCT OUTPUT, SUPPORT INPUT and SUPPORT OUTPUT FILES
*    and the user has passed in the number of the file that he wants.  
*    So let's loop that many times to ensure we are at the correct 
*    address.
***************************************************************************/
        if ((offSet == headStruct.divPtr[PGSd_PC_INPUT_FILES-1]) ||
            (offSet == headStruct.divPtr[PGSd_PC_OUTPUT_FILES-1]) ||
            (offSet == headStruct.divPtr[PGSd_PC_SUPPORT_INPUT-1]) ||
            (offSet == headStruct.divPtr[PGSd_PC_SUPPORT_OUTPUT-1]))
        {
            if (*numFiles <= 0)
            {
                returnStatus = PGSPC_W_INVALID_VERSION;
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_GetMsgByCode(returnStatus,msg);
                    PGS_SMF_SetDynamicMsg(returnStatus,msg,
                                      "PGS_PC_GetDataFromShm()");
                }
            }
            else
            {
                returnStatus = PGS_PC_SearchShm(sectionLoc,mode,identifier,
                                                numFiles,outData);
            }   /* end else */
        }   /* end if */

/***************************************************************************
*    With all other types of data we have a one-to-one relationship so
*    we are only looking for the first occurance of the identifier.
***************************************************************************/
        else
        {
            returnStatus = PGS_PC_SearchShm(sectionLoc,mode,identifier,
                                            (PGSt_integer *) NULL,outData);

        }   /* end else */
    }   /* end if */
  
/***************************************************************************
*    Our division pointer address is NULL.
***************************************************************************/
    else
    {
        if (returnStatus != PGSPC_E_INVALID_MODE)
        {
            returnStatus = PGSPC_E_NULL_DIVPOINTER_SHM;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetDataFromShm()");
            }
        }
    }

/***************************************************************************
*   Exit function. 
***************************************************************************/
    return returnStatus;
}
