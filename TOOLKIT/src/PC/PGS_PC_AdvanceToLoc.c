/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_AdvanceToLoc.c

DESCRIPTION:
	This file contains the function PGS_PC_PutPCSDataAdvanceToLoc().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	05-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID().
	19-Apr-94 RM Work for TK5.  Added additional file types to 
			advance to.  Mandated by marking of runtime
			files.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Advance to a specific area in the Process Control Status
	Information data while placing the data in a temporary area.
 
NAME:  
	PGS_PC_PutPCSDataAdvanceToLoc()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status
		PGS_PC_PutPCSDataAdvanceToLoc(
			PGSt_integer		mode,
			FILE			*locationPCS[NUMFILES]);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to advance to a specific area in the Process
	Control Status Information file while writing the data to a
	temporary file.
 
INPUTS:
	Name		Description			Units	Min	Max

	mode		Context values that will be 
			#define'd in a header file.
			Possible values are:

			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_DELETE_TEMP
			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_INTERMEDIATE_OUTPUT
			PGSd_PC_ENDOFFILE
			PGSd_PC_INPUT_FILE_NAME
			PGSd_PC_OUTPUT_FILE_NAME
			PGSd_PC_SUPPORT_IN_NAME
			PGSd_PC_SUPPORT_OUT_NAME

	locationPCS	Pointer to files containing the 
			PCS data and the temporary file.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_INVALID_MODE        mode value does not match those in PGS_PC.h
	PGSPC_E_FILE_READ_ERROR     error reading input file

EXAMPLES:

C:
	PGSt_SMF_status		returnStatus;
	FILE			*locationPCS[NUMFILES];

	/# assuming the PCS files opened successfully #/
		.
		.
		.

	/# advance to the area of data that contain temporary file information #/

	returnStatus = PGS_PC_PutPCSDataAdvanceToLoc(PGSd_PC_TEMPORARY_FILE,
			locationPCS);
	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# work with the temporary file information #/
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
PGS_PC_PutPCSDataAdvanceToLoc(             /* move to area in PCS data */
    PGSt_integer       mode,               /* input mode */
    FILE              *locationPCS[NUMFILES]) /* location of data */
{
    char               dummy[PGSd_PC_LINE_LENGTH_MAX]; /* dummy line */
    char               buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char               msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    int                look;                      /* number of dividers */
    int                locFlag;                   /* dividers to find */
    PGSt_SMF_status    whoCalled;                 /* user who called this function */
    PGSt_SMF_status    returnStatus;              /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    look = 0;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Switch on the mode value.
***************************************************************************/
    switch (mode)
    {

/***************************************************************************
*    We need to move to the section that contains temporary file
*    information.
***************************************************************************/
        case PGSd_PC_TEMPORARY_FILE:
        case PGSd_PC_DELETE_TEMP: 
            locFlag = PGSd_PC_TEMP_INFO;
            break;


/***************************************************************************
*    We need to move to the section that contains the intermediate 
*    input file information.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_INPUT:
            locFlag = PGSd_PC_INTER_INPUT;
            break;

/***************************************************************************
*    We need to move to the section that contains the intermediate 
*    output file information.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_OUTPUT:
            locFlag = PGSd_PC_INTER_OUTPUT;
            break;

/***************************************************************************
*    We need to move to the section that contains the product
*    input file information.
***************************************************************************/
        case PGSd_PC_INPUT_FILE_NAME:
            locFlag = PGSd_PC_INPUT_FILES;
            break;

/***************************************************************************
*    We need to move to the section that contains the product
*    output file information.
***************************************************************************/
        case PGSd_PC_OUTPUT_FILE_NAME:
            locFlag = PGSd_PC_OUTPUT_FILES;
            break;

/***************************************************************************
*    We need to move to the section that contains the support
*    input file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_IN_NAME:
            locFlag = PGSd_PC_SUPPORT_INPUT;
            break;

/***************************************************************************
*    We need to move to the section that contains the support
*    output file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_OUT_NAME:
            locFlag = PGSd_PC_SUPPORT_OUTPUT;
            break;

/***************************************************************************
*    We need to move to the end of the file.
***************************************************************************/
        case PGSd_PC_ENDOFFILE:
            locFlag = 0;
            while ((fgets(dummy,PGSd_PC_LINE_LENGTH_MAX,
                          locationPCS[PCSFILE])) != NULL)
            {
                fputs(dummy,locationPCS[TEMPFILE]);
            }
            break;

/***************************************************************************
*    Where did you get this mode value?  Not from PGS_PC.h where you
*    were supposed to get it.
***************************************************************************/
        default:
            returnStatus = PGSPC_E_INVALID_MODE;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(PGSPC_E_INVALID_MODE,msg);
                sprintf(buf,msg,mode);
                PGS_SMF_SetDynamicMsg(PGSPC_E_INVALID_MODE,buf,
                                     "PGS_PC_PutPCSDataAdvanceToLoc()");
            }
            break;

    }   /* end switch */


/***************************************************************************
*    Loop until we have found the correct number of separators.
***************************************************************************/
    while (look < locFlag)
    {

/***************************************************************************
*    Read a line from the input file, if it is the EOF then we have
*    a problem, since we handle EOF up in the switch-case statement.
***************************************************************************/
        if ((fgets(dummy,PGSd_PC_LINE_LENGTH_MAX,locationPCS[PCSFILE]))
                   == NULL)
        {
            returnStatus = PGSPC_E_FILE_READ_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                      "PGS_PC_PutPCSDataAdvanceToLoc()");
            }
            break;
        }

/***************************************************************************
*    If the first character is a separator then increment our variable.
***************************************************************************/
        else if (dummy[0] == PGSd_PC_DIVIDER)
        {
            look++;
        }

/***************************************************************************
*    Write the line to the output file.
***************************************************************************/
        fputs(dummy,locationPCS[TEMPFILE]);

    }  /* end while */

/***************************************************************************
*    Update the message log and leave the function.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_PutPCSDataAdvanceToLoc()");
            }
            break;
    }
    return returnStatus;
} 


