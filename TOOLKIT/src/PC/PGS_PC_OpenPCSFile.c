/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_OpenPCSFile.c

DESCRIPTION:
	Open file containing PCS information.

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
	Open the file containing the Process Control Status information.
 
NAME:
	PGS_PC_GetPCSDataOpenPCSFile()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetPCSDataOpenPCSFile(
			FILE		**locationPCS);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to open the file conataining the Process Control
	Status information.
 
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
	PGSPC_E_FILE_OPEN_ERROR     error opening input file
	PGSPC_E_ENVIRONMENT_ERROR   environment variable error

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;
	FILE		*locationPCS;

	/# opening the PCS file #/

	returnStatus = PGS_PC_GetPCSDataOpenPCSFile(&locationPCS);

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
	In a future release of the Process Control Tools the Process Control
	Status information will be loaded into shared memory.

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
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_GetPCSDataOpenPCSFile(                /* open PCS info file */
    FILE            **locationPCS)           /* location of PCS data */

{
    char         *inFileN;                      /* input file */
    char          buf[PGS_SMF_MAX_MSG_SIZE];    /* message buffer */
    char          msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    PGSt_SMF_status whoCalled;                  /* user that called */
    PGSt_SMF_status returnStatus;               /* function return */

/***************************************************************************
*    Initialize our return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Get the file name and path from the environment variable, if it
*    is not set, let's get outta here.
***************************************************************************/
    inFileN = getenv(PGSd_PC_INFO_FILE_ENVIRONMENT);
    if (!inFileN)
    {
        returnStatus = PGSPC_E_ENVIRONMENT_ERROR;
        if (whoCalled != PGSd_CALLERID_SMF) 
        {
            PGS_SMF_GetMsgByCode(PGSPC_E_ENVIRONMENT_ERROR,msg);
            sprintf(buf,msg,PGSd_PC_INFO_FILE_ENVIRONMENT);
            PGS_SMF_SetDynamicMsg(PGSPC_E_ENVIRONMENT_ERROR,buf,
                             "PGS_PC_GetPCSDataOpenPCSFile()");
        }
    }

/***************************************************************************
*    Open input file, if there is a problem then just return to the 
*    calling function and tell them that they are not going to get
*    what they wanted due to a file problem. 
***************************************************************************/
    else if ((*locationPCS = fopen(inFileN,"r")) == NULL)
    {
        returnStatus = PGSPC_E_FILE_OPEN_ERROR;
        if (whoCalled != PGSd_CALLERID_SMF) 
        {
            PGS_SMF_GetMsgByCode(PGSPC_E_FILE_OPEN_ERROR,msg);
            sprintf(buf,msg,inFileN);
            PGS_SMF_SetDynamicMsg(PGSPC_E_FILE_OPEN_ERROR,buf,
                             "PGS_PC_GetPCSDataOpenPCSFile()");
        }
    }

/***************************************************************************
*    Set a message in the message log and leave.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF) 
            {
                PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_PC_GetPCSDataOpenPCSFile()");
            }
            break;
    }
    return returnStatus;
}
