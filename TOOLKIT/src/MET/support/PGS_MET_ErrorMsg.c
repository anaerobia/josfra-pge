/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_ErrorMsg.c
 
DESCRIPTION:
         The file contains PGS_MET_ErrorMsg.
         This function is used by all MET functions to output dynamic error
	 mesaages

AUTHOR:
  	Alward N.Siyyid / EOSL
        Carol S. W. Tsai / Space Applications Corporation

HISTORY:
  	18-May-1995 	ANS 	Initial version
	31-May-1995     ANS     Code inspection comments update
        19-May-1998     CSWT    Changed the length of character string
                                from PGS_SMF_MAX_MSG_SIZE defined as 241
                                to PGS_SMF_MAX_MSGBUF_SIZE defined as 481
                                for the variable msg, an input argument
                                defined to pass into the function 
                                PGS_SMF_SetDynamicMsg to hold the message
                                (This changing is for ECSed14746 about
                                SDPTK5.2.1 limits on user-defined log messages 
                                greater than 275 chars) 

END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <PGS_MET.h>

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE:  
  	Outputs dynamic error messages
 
NAME:
  	PGS_MET_ErrorMsg()

SYNOPSIS:
  	N/A

DESCRIPTION:
	This function inserts dynamic values such as metadata name, values
	etc. in predefined error strings. Up to four insertions can be used

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
       	code		SMF error code		none	N/A	N/A
	functionName	name of function	none    N/A     N/A
			where error occurred
	noOfInserts	number of strings to 	none	0	4
			be inserted
	errInserts	array of strings to 	none    N/A     N/A
			be inserted in the 
			error code string

OUTPUTS:
	None

RETURNS:
	None

EXAMPLES:
	N/A

NOTES:
	N/A

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_GetMsgByCode
	PGS_SMF_SetDynamicMsg

END_PROLOG:
***************************************************************************/

void
PGS_MET_ErrorMsg(			     /* prepares and sends error messages
                                              * to the log file
                                              */
		PGSt_SMF_code code,	     /* SMF code value */
		char	      *functionName, /* function name where error occurred */
		long	      noOfInserts,   /* number of inserts in the dynamic message */
		char	      *errInserts[]) /* inserts strings in the error message */
{
	char			codeMsg[PGS_SMF_MAX_MSG_SIZE];
	char                    msg[PGS_SMF_MAX_MSGBUF_SIZE];
	
	(void) PGS_SMF_GetMsgByCode(code, codeMsg);

	switch (noOfInserts)
	{
		case 0:
			sprintf(msg, codeMsg);
			break;
		case 1:
			sprintf(msg, codeMsg, errInserts[0]);
			break;
		case 2:
			sprintf(msg, codeMsg, errInserts[0], errInserts[1]);
			break;
		case 3:
			sprintf(msg, codeMsg, errInserts[0],
				errInserts[1], errInserts[2]);
			break;
		default: /* if 4 or greater than 4, only print the first four */
                        sprintf(msg, codeMsg, errInserts[0],
                                errInserts[1], errInserts[2], errInserts[3]);
	}

	(void) PGS_SMF_SetDynamicMsg(code, msg, functionName);

}

