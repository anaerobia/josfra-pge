/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetRequest.c

DESCRIPTION:
	Get parse out user request from line of PCS data.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	02-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
 
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
	Get requested data from line of Process Control
	Status data.
 
NAME:
	PGS_PC_GetPCSDataGetRequest()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetPCSDataGetRequest(
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
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file
	PGSPC_W_NO_DATA_PRESENT     no data existed for request

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
	returnStatus = PGS_PC_GetPCSDataGetRequest(num,endChar,line,out);

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
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetPCSDataGetRequest(  /* get reqested data from line */
    int         num,          /* number of DELIMITER(s) to count */
    char        endChar,      /* last character to look for */
    char       *line,         /* line to search */
    char       *out)          /* string returned */
{
    char        buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char        msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    int         count;        /* number of DELIMITER(s) - so far */
    int         inPos;        /* input line position */
    int         outPos;       /* output string position */
    PGSt_SMF_status whoCalled;       /* user that called this function */
    PGSt_SMF_status returnStatus;    /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS; 
    count = 0;
    inPos = 0;
    out[0] = '\0';

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

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
            returnStatus = PGSPC_E_LINE_FORMAT_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                  "PGS_PC_GetPCSDataGetRequest()");
            }
            break;
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
    if (returnStatus == PGS_S_SUCCESS)
    {
        outPos = 0;
        while (line[inPos] != endChar) 
        {
            out[outPos] = line[inPos];
            outPos++;
            inPos++;
        }

/***************************************************************************
*    If outPos is zero then the requested data is not on the line.  This 
*    may not be a problem so just send back a warning.
***************************************************************************/
        if (outPos == 0)
        {
            returnStatus = PGSPC_W_NO_DATA_PRESENT;
        }

        out[outPos] = '\0';
    }   /* end if */

    return returnStatus;
}
