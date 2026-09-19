/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_FindDefaultLocLine.c

DESCRIPTION:
	Get the line containing the default file location.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	31-Mar-95 RM Initial version
 
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
	Get the line containing the default file location in the area of 
	the Process Control Status information.
 
NAME:
	PGS_PC_FindDefaultLocLine()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_FindDefaultLocLine(
			FILE			*locationPCS,
			char			*line);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to obtain the line in the area of Process Control
	Status data that contains the default file location.  This
	information was contained in environment variables prior to TK5.
 
INPUTS:
	Name		Description			Units	Min	Max

	locationPCS	location of PCS data

OUTPUTS:
	Name		Description			Units	Min	Max

	line		The line of data that contains 
			the index value that matches
			identifier.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_FILE_READ_ERROR     error reading input file
        PGSPC_E_NO_DEFAULT_LOC      no default location specified

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;
	char		line[PGSd_PC_LINE_LENGTH_MAX];

	/# get the line of data containing the default file location #/

	returnStatus = PGS_PC_FindDefaultLocLine(locationPCS,line);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# parse line for correct data #/
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
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message
        PGS_SMF_TestStatusLevel          Get level of error/status code

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_FindDefaultLocLine(             /* search for the proper line */
    FILE            *locationPCS,      /* location of PCS data */
    char            *line)             /* output line when found */
{
    char         buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char         msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    PGSt_boolean finished;             /* search is complete flag */
    PGSt_SMF_status whoCalled;         /* user who called this function */
    PGSt_SMF_status returnStatus;      /* function return */

/***************************************************************************
*    Initialize the variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    finished = PGS_FALSE;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Loop through the file.
***************************************************************************/
    while (finished == PGS_FALSE)
    {

/***************************************************************************
*    If we get to the end of the file then we have a problem.
***************************************************************************/
        if ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,locationPCS)) == NULL)
        {
            finished = PGS_TRUE;
            returnStatus = PGSPC_E_FILE_READ_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                      "PGS_PC_FindDefaultLocLine()");
            }
        }

/***************************************************************************
*    Skip comment lines.
***************************************************************************/
        else if (line[0] == PGSd_PC_COMMENT)
        {
            continue;
        }

/***************************************************************************
*    If we hit a line containing the default location (and it was in
*    the correct spot in the file), let's get out of the loop and 
*    send this line back.
***************************************************************************/
        else if (line[0] == PGSd_PC_DEFAULT_LOC)
        {
            finished = PGS_TRUE;
        }

/***************************************************************************
*    We hit a line that contains something other than a COMMENT symbol
*    or a DEFAULT_LOC symbol.  In other words, there is a problem.
*    Remember, the default location is to be stored on the line after
*    the divider (not counting comment lines which are ignored).
***************************************************************************/
        else
        {
            finished = PGS_TRUE;
            returnStatus = PGSPC_E_NO_DEFAULT_LOC;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_FindDefaultLocLine()");
            }
        }
    }   /* end while */

/***************************************************************************
*    Set a message in the log and then leave.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_FindDefaultLocLine()");
            }
            break;
    }
    return returnStatus;
}

