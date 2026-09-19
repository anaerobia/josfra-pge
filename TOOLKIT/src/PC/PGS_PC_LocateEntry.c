/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_LocateEntry.c

DESCRIPTION:
	Get correct line in Process Control Status information area.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	02-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	05-Apr-95 RM Updated for TK5.  Move default file locations
			from environment variables to PCF.
	10-Jul-95 RM Removed call to PGS_SMF_SetStaticMsg at end of this
			function per DR ECSed00096, ECSed00097, and 
			ECSed00103.
	17-Oct-95 RM Modified function calls PGS_PC_GetPCSDataGetIndex().
			A new parameter (locator) was added to that function 
			to correct a bug as per ECSed01190.
	13-Dec-95 RM Updated for TK6.  Added functionality to allow
			multiple instances of PRODUCT OUTPUT FILES.
        05-Jan-96 RM Updated for TK6.  Added Universal References to
                        file entries for files.
	24-Apr-97 RM Added functionality to allow multiple instances of
			SUPPORT INPUT and SUPPORT OUTPUT files.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cfortran.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get correct line in Process Control Status information area.
 
NAME:
	PGS_PC_GetPCSDataLocateEntry()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetPCSDataLocateEntry(
			FILE			*locationPCS,
			PGSt_integer		mode,
			PGSt_PC_Logical		identifier,
			PGSt_integer		*numFiles,
			char			*line);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to locate the correct line of data in the location
	of the Process Control Status information.
 
INPUTS:
	Name		Description			Units	Min	Max

	locationPCS	pointer to location of Process 
			Control Status information.

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
			PGSd_PC_INTER_IN_DEFLOC
			PGSd_PC_INTER_IN_UREF
			PGSd_PC_INTERMEDIATE_OUTPUT
			PGSd_PC_INTER_OUT_DEFLOC
			PGSd_PC_INTER_OUT_UREF
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

	line		actual line of data as read 
			from location of PCS data. 

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_INVALID_MODE        mode value does not match those in PGS_PC.h
        PGSPC_W_INVALID_VERSION     version must be greater than zero
        PGSPC_W_NO_CONFIG_VALUE     configuration value for this id does not
				    exist

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;
	PGSt_PC_Logical	identifier;
	PGSt_integer	*numFiles;
	PGSt_integer	mode;
	FILE		*locationPCS;
	char		line[PGSd_PC_LINE_LENGTH_MAX];

	/# assuming the PCS file opened without incident #/

	identifier = 101;

	/# get the first input file of identifier 101 #/

	*numFiles = 1;
	mode = PGSd_PC_INPUT_FILE_NAME;

	returnStatus = PGS_PC_GetPCSDataLocateEntry(locationPCS,mode,identifier,
		numFiles,line);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# parse line for file name #/
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
	PGS_SMF_CallerID		 Get value of user variable
	PGS_PC_GetPCSDataAdvanceArea     Advance to proper section of file
	PGS_PC_GetPCSDataGetIndex        Return line with proper index
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message
        PGS_PC_FindDefaultLocLine        Get line with default file location

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetPCSDataLocateEntry(            /* locate line of data */
    FILE             *locationPCS,       /* input data location */
    PGSt_integer      mode,              /* mode requested */
    PGSt_PC_Logical   identifier,        /* logical id */
    PGSt_integer     *numFiles,          /* number of files */
    char             *line)              /* line found */

{
    char            buf[PGS_SMF_MAX_MSG_SIZE];    /* message buffer */
    char            msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    int             loopCount;           /* loop counter */
    PGSt_integer    locator;             /* number of dividers */
    PGSt_boolean    deflocFlag;          /* flag for default location */
    PGSt_SMF_status whoCalled;           /* user that called this function */
    PGSt_SMF_status returnStatus;        /* function return variable */

/***************************************************************************
*    Initialize return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    deflocFlag = PGS_FALSE;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Based on the mode that is requested let's set a locator.  This will
*    tell us how far into the area of data that we need to go to get to the
*    section of data that contains what we want.  The locator is the number
*    of dividers.
***************************************************************************/
    switch (mode)  
    {

/***************************************************************************
*    System defined configuration parameters.
***************************************************************************/
        case PGSd_PC_PRODUCTION_RUN_ID:
        case PGSd_PC_SOFTWARE_ID:
            locator = PGSd_PC_SYS_CONFIG;
            break;

/***************************************************************************
*    User defined configuration parameters.
***************************************************************************/
        case PGSd_PC_CONFIGURATION:
            locator = PGSd_PC_CONFIG_COUNT;
            break;

/***************************************************************************
*    Input file information.
***************************************************************************/
        case PGSd_PC_PRODUCT_IN_DEFLOC:
            deflocFlag = PGS_TRUE;
        case PGSd_PC_INPUT_FILE_NAME:
        case PGSd_PC_INPUT_FILE_ATTRIBUTE:
        case PGSd_PC_INPUT_FILE_NUMFILES:
        case PGSd_PC_PRODUCT_IN_UREF:
            locator = PGSd_PC_INPUT_FILES;
            break;

/***************************************************************************
*    Output file information.
***************************************************************************/
        case PGSd_PC_PRODUCT_OUT_DEFLOC:
            deflocFlag = PGS_TRUE;
        case PGSd_PC_OUTPUT_FILE_NAME:
        case PGSd_PC_OUTPUT_FILE_ATTRIBUTE:
        case PGSd_PC_OUTPUT_FILE_NUMFILES:
        case PGSd_PC_PRODUCT_OUT_UREF:
            locator = PGSd_PC_OUTPUT_FILES;
            break;

/***************************************************************************
*    Temporary file information.
***************************************************************************/
        case PGSd_PC_TEMP_FILE_DEFLOC:
            deflocFlag = PGS_TRUE;
        case PGSd_PC_TEMPORARY_FILE:
            locator = PGSd_PC_TEMP_INFO;
            break;

/***************************************************************************
*    Intermediate input file information.
***************************************************************************/
        case PGSd_PC_INTER_IN_DEFLOC:
            deflocFlag = PGS_TRUE;
        case PGSd_PC_INTERMEDIATE_INPUT:
        case PGSd_PC_INTER_IN_UREF:
            locator = PGSd_PC_INTER_INPUT;
            break;

/***************************************************************************
*    Intermediate output file information.
***************************************************************************/
        case PGSd_PC_INTER_OUT_DEFLOC:
            deflocFlag = PGS_TRUE;
        case PGSd_PC_INTERMEDIATE_OUTPUT:
        case PGSd_PC_INTER_OUT_UREF:
            locator = PGSd_PC_INTER_OUTPUT;
            break;

/***************************************************************************
*    Support input file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_IN_DEFLOC:
            deflocFlag = PGS_TRUE;
        case PGSd_PC_SUPPORT_IN_NAME:
        case PGSd_PC_SUPPORT_IN_ATTR:
        case PGSd_PC_SUPPORT_IN_UREF:
        case PGSd_PC_SUPPORT_IN_NUMFILES:
            locator = PGSd_PC_SUPPORT_INPUT;
            break;

/***************************************************************************
*    Support output file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_OUT_DEFLOC:
            deflocFlag = PGS_TRUE;
        case PGSd_PC_SUPPORT_OUT_NAME:
        case PGSd_PC_SUPPORT_OUT_ATTR:
        case PGSd_PC_SUPPORT_OUT_UREF:
        case PGSd_PC_SUPPORT_OUT_NUMFILES:
            locator = PGSd_PC_SUPPORT_OUTPUT;
            break;

/***************************************************************************
*    Unknown mode.  See PGS_PC.h for a list of modes.
***************************************************************************/
        default:
            returnStatus = PGSPC_E_INVALID_MODE;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(PGSPC_E_INVALID_MODE,msg);
                sprintf(buf,msg,mode);
                PGS_SMF_SetDynamicMsg(PGSPC_E_INVALID_MODE,buf,
                                  "PGS_PC_GetPCSDataLocateEntry()");
            }
            break;
    }   /* end switch */

/***************************************************************************
*    If everything has gone OK up to here then we will proceed.
***************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*    Advance our PCS data location to the area that contains the section
*    of data that we are looking for.
***************************************************************************/
        returnStatus = PGS_PC_GetPCSDataAdvanceArea(locationPCS,locator);

/***************************************************************************
*    Once again, If everything went OK then we proceed as planned.
***************************************************************************/
        if ((returnStatus == PGS_S_SUCCESS) && 
            (locator != PGSd_PC_SYS_CONFIG) &&
            (deflocFlag == PGS_FALSE))
        {

/***************************************************************************
*    Remember, there can be a many-to-one relationship with PRODUCT
*    INPUT FILES, PRODUCT OUTPUT, SUPPORT INPUT, and SUPPORT OUTPUT 
*    FILES and the user has passed in the file number that he/she wants.  
*    So let's loop that many times to ensure we are at the correct line.
***************************************************************************/
            if ((locator == PGSd_PC_INPUT_FILES) || 
                (locator == PGSd_PC_OUTPUT_FILES) ||
                (locator == PGSd_PC_SUPPORT_INPUT) ||
                (locator == PGSd_PC_SUPPORT_OUTPUT))
            {
                if (*numFiles <= 0)
                {
                    returnStatus = PGSPC_W_INVALID_VERSION;
                    if (whoCalled != PGSd_CALLERID_SMF)
                    {
                        PGS_SMF_GetMsgByCode(PGSPC_W_INVALID_VERSION,msg);
                        PGS_SMF_SetDynamicMsg(PGSPC_W_INVALID_VERSION,msg,
                                          "PGS_PC_GetPCSDataLocateEntry()");
                    }
                }
                else
                {
                    for (loopCount = 0; loopCount < *numFiles; loopCount++)
                    {
                        returnStatus = PGS_PC_GetPCSDataGetIndex(locationPCS,
                                         identifier,locator,line);
                        if (returnStatus != PGS_S_SUCCESS)
                        {
                            break;
                        }
                    }   /* end for */
                }   /* end else */
            }   /* end if */

/***************************************************************************
*    With all other types of data we have a one-to-one relationship so
*    we are only looking for the first occurance of the identifier.
***************************************************************************/
            else
            {
                returnStatus = PGS_PC_GetPCSDataGetIndex(locationPCS,
                                 identifier,locator,line);

                if ((returnStatus == PGSPC_W_NO_FILES_EXIST) && 
                    (locator == PGSd_PC_CONFIG_COUNT))
                {
                    returnStatus = PGSPC_W_NO_CONFIG_VALUE;
                }
            }   /* end else */
        }   /* end if */

/***************************************************************************
*    The user must be requesting default location data.
***************************************************************************/
        else if ((returnStatus == PGS_S_SUCCESS) && (deflocFlag == PGS_TRUE))
        {
            returnStatus = PGS_PC_FindDefaultLocLine(locationPCS,line);
        }  /* end else if */

    }   /* end if */

/***************************************************************************
*    Leave the function.
***************************************************************************/
    return returnStatus;
}
