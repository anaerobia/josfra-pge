/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_AdvanceArea.c

DESCRIPTION:
	Advance in area of PCS data.

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
	Advance in the area of the Process Control Status information.
 
NAME:
	PGS_PC_GetPCSDataAdvanceArea()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetPCSDataAdvanceArea(
			FILE			*locationPCS,
			int			loop);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used by PGS_PC_GetPCSData() to advance to the proper
	area in the PCS data.
 
INPUTS:
	Name		Description			Units	Min	Max

	locationPCS	location of PCS data.

	loop		number of dividers to loop over.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_FILE_READ_ERROR     error reading input file

EXAMPLES:
C:
	FILE		*locationPCS;
	int		loop;
	PGSt_SMF_status returnStatus;

	/# assuming locationPCS is open #/

	loop = PGSd_PC_INPUT_FILES;

	/# let's move to the area of input files #/

	returnStatus = PGS_PC_GetPCSDataAdvanceArea(locationPCS,loop);

	if (ret_value != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# work with input file data #/
		}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

NOTES:
	Currently, the location of the PCS data is in a flat file.  In a future
	release the location will be in shared memory.

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
PGS_PC_GetPCSDataAdvanceArea(           /* advance to proper area of data */
    FILE       *locationPCS,            /* location of PCS data */
    int         loop)                   /* number or separators to loop over */
{
    char        dummy[PGSd_PC_LINE_LENGTH_MAX]; /* line read and not used */
    char        buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char        msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    int         sepCounted;             /* number of separators counted */
    PGSt_SMF_status whoCalled;          /* user that called this function */
    PGSt_SMF_status returnStatus;       /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    sepCounted = 0;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Just loop through the file and count the number of question marks.
*    These are our section (category) dividers.  The calling function 
*    told us how many question marks to count.
***************************************************************************/
    while (sepCounted < loop)
    {
        if ((fgets(dummy,PGSd_PC_LINE_LENGTH_MAX,locationPCS)) == NULL)
        {
            returnStatus = PGSPC_E_FILE_READ_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(PGSPC_E_FILE_READ_ERROR,msg);
                sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                             "PGS_PC_GetPCSDataAdvanceArea()");
            }
            break;
        }

/***************************************************************************
*    Increment our counter here.
***************************************************************************/
        else if (dummy[0] == PGSd_PC_DIVIDER)
        {
            sepCounted++;
        }
    }   /* end while */
       
/***************************************************************************
*    Set a message in the log and leave.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetPCSDataAdvanceArea()");
            }
            break;
    }
    return returnStatus;
}

