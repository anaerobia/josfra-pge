/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
        PGS_PC_GetReferenceType.c
 
DESCRIPTION:
        This file contains the function PGS_PC_GetReferenceType().
 
AUTHOR:
        Ray Milburn / Applied Research Corporation
 
HISTORY:
        11-Apr-95 RM Initial version
	14-Jul-95 RM Cleaned up code at end of function where return
			value is set.
 
END_FILE_PROLOG:
***************************************************************************/
 
#include <stdio.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:		
	Access File Reference type from PCF
        
NAME: 	
	PGS_PC_GetReferenceType()

SYNOPSIS:	

C:
	#include <PGS_PC.h>
	
	PGSt_SMF_status
	PGS_PC_GetReferenceType(
	PGSt_PC_Logical		identifier,
	PGSt_integer		*type);

FORTRAN:
	include 'PGS_SMF.f'
	include 'PGS_PC.f'
	include 'PGS_PC_9.f'

	integer function pgs_pc_getreferencetype(identifier,type)
	integer			identifier
	integer			type

DESCRIPTION: 	
	This tool may be used to ascertain the type of file reference which 
	is associated with a logical identifier within the science software.

INPUTS: 	
        Name            Description                     Units   Min     Max

	identifier	The logical identifier as defined 
			by the user. (this value must be 
			mapped to an actual value via the PCF)

OUTPUTS:	
        Name            Description                     Units   Min     Max

	type		Reference types that are defined 
			in the PGS_PC header file.
			Possible values are:

			PGSd_PC_INPUT_FILE_NAME
			PGSd_PC_OUTPUT_FILE_NAME
			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_INTERMEDIATE_OUTPUT
			PGSd_PC_SUPPORT_IN_NAME
			PGSd_PC_SUPPORT_OUT_NAME
		
RETURNS:	
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_FOR_ID     The Product ID does not contain a 
			            physical reference.
	PGSPC_E_ENVIRONMENT_ERROR   Environment variable not set:  %s
	PGSPC_E_DATA_ACCESS_ERROR   Error accessing Process Control Status data.

EXAMPLES:

C: 	
	#define	INSTR_SCRATCH_SPACE 2001

        PGSt_SMF_status returnStatus;
        PGSt_PC_Logical fileIdentifier;
	PGSt_integer	fileType;

        fileIdentifier = INSTR_SCRATCH_SPACE;

        /# getting the type attribute of a file #/

        returnStatus = PGS_PC_GetReferenceType(fileIdentifier,&fileType);
        if (returnStatus != PGS_S_SUCCESS)
	{
            goto EXCEPTION;
	}
        else
        {
	    switch (fileType)
	    {
	    case PGSd_PC_INPUT_FILE_NAME:
	    case PGSd_PC_OUTPUT_FILE_NAME:
	    case PGSd_PC_SUPPORT_IN_NAME:
	    case PGSd_PC_SUPPORT_OUT_NAME:
		/# 
		    open standard product or support file 
		#/
		returnStatus = PGS_IO_Gen_Open( );
			.
			.
			.
		break;

	    case PGSd_PC_INTERMEDIATE_INPUT:
	    case PGSd_PC_INTERMEDIATE_OUTPUT:
	    case PGSd_PC_TEMPORARY_FILE:
		/# 
		    open temporary or intermediate file 
		#/
		returnStatus = PGS_IO_Gen_Temp_Open( );
			.
			.
			.
		break;
	    default:
		/# 
		    invalid type returned only in the event that
		    call to *GetReferenceType was not successful
		#/
		
	    } /# end switch (fileType) #/
        }
                        .
                        .
                        .
        EXCEPTION:
                return returnStatus;

FORTRAN:
	IMPLICIT NONE
	
	INTEGER INSTR_SCRATCH_SPACE
        PARAMETER (INSTR_SCRATCH_SPACE = 2001)

        integer returnstatus
        integer fileidentifier
        integer filetype
        integer pgs_pc_getreferencetype

        fileidentifier = INSTR_SCRATCH_SPACE

C  getting the type attribute of a file

        returnstatus = pgs_pc_getreferencetype(fileidentifier,filetype)
        if (returnstatus .ne. pgs_s_success) then 
            goto 9999
        else if (
		(filetype .eq. PGSd_PC_INPUT_FILE_NAME) .or.
		(filetype .eq. PGSd_PC_OUTPUT_FILE_NAME) .or.
		(filetype .eq. PGSd_PC_SUPPORT_IN_NAME) .or.
		(filetype .eq. PGSd_PC_SUPPORT_OUT_NAME)
		) then

C  open standard product or support file

                returnstatus = PGS_IO_Gen_OpenF(...);
                        .
                        .
                        .
	else if (
		(filetype .eq. PGSd_PC_INTERMEDIATE_INPUT) .or.
		(filetype .eq. PGSd_PC_INTERMEDIATE_OUTPUT) .or.
		(filetype .eq. PGSd_PC_TEMPORARY_FILE)
		) then

C  open temporary or intermediate file

                returnstatus = PGS_IO_Gen_Temp_OpenF(...);
                        .
                        .
                        .
	else

C  invalid type returned only in the event that 
C  call to *GetReferenceType was not successful

        endif

9999    return

NOTES: 		
	This tool will return the reference type (mode) for files which have
	references in a Process Control File (PCF). This tool will not identify
	runtime parameters as such. 

	In order for this tool to function properly, a valid Process Control
	file will need to be created first.  Please refer to Appendix C 
	(User's Guide) for instructions on how to create and validate such a file.

REQUIREMENTS:
        PGSTK-1290

DETAILS: 	
	This "interim" implementation will iteratively call PGS_PC_GetPCSData with
	a different mode value until the input logical identifier is recognized as 
	a member of the current PCF. When recognition is achieved, the mode value
	in effect at the time will be returned by the tool as the "type" attribute 
	for the logical identifier in question.

GLOBALS: 	
	NONE

FILES: 		
	This tool reads from the file defined by the environment
	variable "PGS_PC_INFO_FILE"

FUNCTIONS_CALLED: 
	PGS_PC_GetPCSData	poll PCF for reference type
       	PGS_SMF_SetStaticMsg    Set static error/status message

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetReferenceType(
    PGSt_PC_Logical     identifier,     /* logical value in the PCF */
    PGSt_integer       *type)           /* file type */
{
    PGSt_integer	currentMode;    /* file mode currently being examined */ 
    PGSt_integer 	pcfFileModes;   /* total file modes available */
    static PGSt_integer	pcsModes[] = {  /* array of file modes */
					PGSd_PC_INPUT_FILE_NAME,
				       	PGSd_PC_OUTPUT_FILE_NAME,
					PGSd_PC_SUPPORT_IN_NAME,
					PGSd_PC_SUPPORT_OUT_NAME,
					PGSd_PC_INTERMEDIATE_INPUT,
					PGSd_PC_INTERMEDIATE_OUTPUT,
					PGSd_PC_TEMPORARY_FILE };

    PGSt_integer	numFiles;         /* satisfy numFiles argument */
    char	        fileName[PGSd_PC_FILE_PATH_MAX];  /* satisfy fileName */
    PGSt_SMF_status     whoCalled;      /* user that called this function */
    PGSt_SMF_status   	returnStatus;   /* function return value */
    char                msg[PGS_SMF_MAX_MSG_SIZE];      /* predefined status message */
    char                newMsg[PGS_SMF_MAX_MSGBUF_SIZE];/* new status message */
/****************************************************************************
*    Initialize variables.
****************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    pcfFileModes = sizeof(pcsModes) / sizeof(PGSt_integer);
    numFiles = 1;

/****************************************************************************
*    Initialize caller variable.
****************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/****************************************************************************
*    Try each PCS mode in succession until the logical identifier 
*    is located.
****************************************************************************/
    for (currentMode=0; currentMode<pcfFileModes; currentMode++)
    {
	returnStatus = PGS_PC_GetPCSData(pcsModes[currentMode],identifier,
                                         fileName,&numFiles);

/****************************************************************************
*    Pass reference back to user.
****************************************************************************/
        if (returnStatus == PGS_S_SUCCESS)
        {
	    *type = pcsModes[currentMode];
            break;
	}

/****************************************************************************
*    Try next PCS mode.
****************************************************************************/
	else if (returnStatus == PGSPC_W_NO_FILES_EXIST)
	{
	    continue;
	}

/****************************************************************************
*    Serious problems with PCF.
****************************************************************************/
	else
	{
	    break;
	}
    }

/****************************************************************************
*    Parse and set the proper error code.
****************************************************************************/
    switch (returnStatus)
    {
        case PGSPC_E_FILE_OPEN_ERROR:
        case PGSPC_E_INVALID_MODE:
        case PGSPC_E_FILE_READ_ERROR:
        case PGSPC_E_LINE_FORMAT_ERROR:
            returnStatus = PGSPC_E_DATA_ACCESS_ERROR; 
            break;
        case PGSPC_W_NO_FILES_EXIST:
        case PGSPC_W_NO_CONFIG_VALUE:
            returnStatus = PGSPC_W_NO_FILES_FOR_ID;
            break;
        case PGS_S_SUCCESS:
            break;
    }

/****************************************************************************
*    Update message log and return.
****************************************************************************/
    if ((whoCalled != PGSd_CALLERID_SMF) && 
        (returnStatus != PGSPC_E_ENVIRONMENT_ERROR))
      {
	if ((returnStatus == PGSPC_W_NO_FILES_EXIST) || (returnStatus == PGSPC_W_NO_FILES_FOR_ID))
	  {
	    PGS_SMF_GetMsgByCode(returnStatus, msg);
	    sprintf(newMsg, msg, (int)identifier);
	    PGS_SMF_SetDynamicMsg(returnStatus, newMsg,
				  "PGS_PC_GetReferenceType()");
	  }
	else
	  {
	    PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetReferenceType()");
	  }
      }

    return returnStatus;
}
