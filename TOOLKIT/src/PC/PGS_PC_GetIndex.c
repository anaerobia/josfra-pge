/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetIndex.c

DESCRIPTION:
	Get the line with the specific index.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	02-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	22-Dec-94 RM Added functionality to check for the existence of
			the delete flag.
	05-Apr-95 RM Updated for TK5.  Added functionality to check
			for the default location flags.
	19-Apr-95 RM Updated for TK5.  Added function call to 
			PGS_PC_CheckFlags to modularize the check of
			DELETE and RUNTIME flags.
	10-Jul-95 RM Removed call to PGS_SMF_SetStaticMsg() at end of
			function per DR ECSed00096, ECSed00097, and 
			ECSed00103.
	17-Oct-95 RM Added if check before PGS_PC_CheckFlags() call
			to jump around this call if the data being
			requested was not file data.  Also, had to 
			add new variable locator as a new parameter 
			per ECSed01190.
 
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
	Get the line with the specific index in the area of the Process
	Control Status information.
 
NAME:
	PGS_PC_GetPCSDataGetIndex()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetPCSDataGetIndex(
			FILE			*locationPCS,
			PGSt_PC_Logical		identifier,
			PGSt_integer		locator,
			char			*line);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to obtain the line in the area of Process Control
	Status data that matches the index value 
 
INPUTS:
	Name		Description			Units	Min	Max

	locationPCS	location of PCS data

	identifier	The logical identifier as 
			defined by the user.  The 
			user's definitions will be 
			mapped into actual identifiers
			during the Integration & Test 
			procedure.

	locator		Flag which indicates which section
			of the PCF we are working in.

OUTPUTS:
	Name		Description			Units	Min	Max

	line		The line of data that contains 
			the index value that matches
			identifier.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_EXIST      identifier does not have the data that
                                    mode is requesting
	PGSPC_E_FILE_READ_ERROR     error reading input file
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;
	PGSt_PC_Logical	identifier;
	PGSt_integer	locator;
	char		line[PGSd_PC_LINE_LENGTH_MAX];

	/# get the line of PRODUCT INPUT FILE data containing #/
	/# the index value 101 #/

	identifier = 101;
        locator = PGSd_PC_INPUT_FILES;
	returnStatus = PGS_PC_GetPCSDataGetIndex(locationPCS,identifier,
						locator,line);

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
	All configuration parameters, except for physical file name(s), will
	have a one-to-one correspondence between a logical value and a  
	parameter value (string).  This many-to-one relationship will be 
	handled outside of this function.

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
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message
	PGS_PC_CheckFlags		 Determine which flags exist on
					 a line of file location data

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetPCSDataGetIndex(             /* search for the proper identifier */
    FILE            *locationPCS,      /* location of PCS data */
    PGSt_PC_Logical  identifier,       /* identifier to search for */
    PGSt_integer     locator,          /* flag indicating section of file */
    char            *line)             /* output line when found */
{
    char         cnum[33];             /* holds a character number */
    char         buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char         msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    int          linePos;              /* line position indicator */
    PGSt_integer flags;                /* flags in line */
    PGSt_PC_Logical check;             /* integer form of cnum[] */
    PGSt_boolean foundDefLoc;          /* default location flag */
    PGSt_SMF_status whoCalled;         /* user who called this function */
    PGSt_SMF_status returnStatus;      /* function return */

/***************************************************************************
*    Initialize the variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    check = ~identifier;
    foundDefLoc = PGS_FALSE;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Loop through the file.
***************************************************************************/
    do
    {

/***************************************************************************
*    If we get to the end of the file then we have a problem since the
*    last line is supposed to be a DIVIDER (currently a question mark).
***************************************************************************/
        if ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,locationPCS)) == NULL)
        {
            returnStatus = PGSPC_E_FILE_READ_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                      "PGS_PC_GetPCSDataGetIndex()");
            }
            break;
        }

/***************************************************************************
*    Skip comment lines.
***************************************************************************/
        else if (line[0] == PGSd_PC_COMMENT)
        {
            continue;
        }

/***************************************************************************
*    If this is the first non-comment line after the divider and the
*    first character is the DEFAULT_LOC flag, then we found the 
*    default file location.  Just set the flag to TRUE and continue.
***************************************************************************/
        else if ((line[0] == PGSd_PC_DEFAULT_LOC) && (foundDefLoc == PGS_FALSE))
        {
            foundDefLoc = PGS_TRUE;
            continue;
        }

/***************************************************************************
*    If we hit a divider then there was not a match.  No big deal just
*    let the calling function know about it.
***************************************************************************/
        else if ((line[0] == PGSd_PC_DIVIDER) || (line[0] < PGSd_PC_LOWDIGIT) ||
               (line[0] > PGSd_PC_HIDIGIT))
        {
            returnStatus = PGSPC_W_NO_FILES_EXIST;
            break;
        }

/***************************************************************************
*    We hit a valid line, lets load the number up to the divider into 
*    a smaller string and convert it into an integer and see if we 
*    have a match.
***************************************************************************/
        else
        {

/***************************************************************************
*    Check for the flag that signifies both a Runtime Send file and
*    a deleted file.  Of course, we only care about the delete part.
*    We only want to check these flags if we are dealing with file data.
***************************************************************************/
            if (locator != PGSd_PC_CONFIG_COUNT)
            {
                returnStatus = PGS_PC_CheckFlags(line,&flags);

                if (((flags == PGSd_PC_HAS_DELETE) || (flags == PGSd_PC_HAS_DELNRUN))
                   && (whoCalled != PGSd_CALLERID_SMF))
                {
                    continue;
                }
            }

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
                        sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                        PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                        "PGS_PC_GetPCSDataGetIndex()");
                    }
                    break;
                }
            }   /* end while */

/***************************************************************************
*    If everything we found was a digit then carry on, otherwise we're
*    outta here.
***************************************************************************/
            if (returnStatus == PGS_S_SUCCESS)
            {
                cnum[linePos] = '\0';
                check = (PGSt_PC_Logical) atol(cnum);
            }
            else
            {
                break;
            }
        }    /* end else */
    }   
    while (identifier != check); /* end do */

/***************************************************************************
*    Leave the function.
***************************************************************************/
    return returnStatus;
}

