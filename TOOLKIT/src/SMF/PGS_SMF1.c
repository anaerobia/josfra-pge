/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_SMF_SetUnknownMsg.c

DESCRIPTION:
  This file contains the function PGS_SMF_SetUnknownMsg().
  This function sets a dynamic message for an unknown input status code.

AUTHOR:
  Guru Tej S. Khalsa

HISTORY:
  20-Jul-1994  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Sets a dynamic message for an unknown input status code

NAME:
   PGS_SMF_SetUnknownMsg()

SYNOPSIS:
   #include <PGS_SMF.h>

   void
   PGS_SMF_SetUnknownMsg(
      PGSt_SMF_status unknownStatus,
      char            *callingFunction)

DESCRIPTION:
   This function sets a dynamic message for an unknown input status code.

INPUTS:
   Name              Description                  
   ----              -----------                  
   unknownReturn     return status from a call to a PGS toolkit function

   callingFunction   name of calling function

OUTPUTS:
   None

RETURNS:
   None

NOTES:
   None

REQUIREMENTS:
   None

DETAILS:
   This function sets a message indicating that the calling function has 
   received an unexpected return value from a call to a PGS Toolkit function.
   The input is the unexpected return status and the name of the function
   calling this function.  This function calls PGS_SMF_GetMsg and checks the
   returning SMF code from that call with unknownStatus.  It also checks
   to see if the mnemonic returned from the call to PGS_SMF_GetMsg is at
   least 7 characters long (min. valid mnemonic length).  If both these
   conditions test true then this function calls PGS_SMF_SetDynamicMsg with
   a code of PGS_E_TOOLKIT and a function name of callingFunction.  The message
   string explains that the return code was unexpected by the calling function
   and then gives the unexpected returned mnemonic and associated message.  If
   the returned SMF code from the call to PGS_SMF_GetMsg is not the same as
   unknownStatus or the returned mnemonic is not valid this function calls
   PGS_SMF_GetMsgByCode.  If the return status of this indicates that it
   successfully identified the input status value this function calls
   PGS_SMF_SetDynamicMsg with a code of PGS_E_TOOLKIT and a function name of
   callingFunction.  The message string explains that the return code was
   unexpected by the calling function and then gives the message associated with
   the unexpected returned status value.  If neither of these SMF calls is
   successful this function calls PGS_SMF_SetDynamicMsg with a code of
   PGS_E_TOOLKIT and a function name of callingFunction.  The message string
   explains that the return code was unexpected by the calling function and that
   furthermore the SMF message system has no knowledge of the returned status 
   value.

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()
   PGS_SMF_SetDynamicMsg()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <PGS_SMF.h>

void
PGS_SMF_SetUnknownMsg(
    PGSt_SMF_status unknownStatus,  /* return status from call to PGSTK func. */
    char            *callingFunction) /* name of calling function */
{
    PGSt_SMF_status code;
    char            mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];
    char            msg[PGS_SMF_MAX_MSG_SIZE];
    char            specifics[PGS_SMF_MAX_MSG_SIZE];

    PGS_SMF_GetMsg(&code,mnemonic,msg);

    /* if code is not the same as unknown status assume the message buffer
       contains old (irrelavant information.  Mnemonics must be at least
       seven characters long */

    if (unknownStatus == code && strlen(mnemonic) > 6UL)
    {

	/* mnemonics must be at least 7 characters long */

	sprintf(specifics,"%s%s%s%s","ERROR: encountered unexpected status ",
		"value => mnemonic: ",mnemonic,", message: ");

	/* concatenate the returned message but only as much as will fit in
	   string specifics which already has the above message in it.  The
	   above message has 67 characters and the string mnemonic is not
	   expected to have more than 40 characters for a total of 107.  This
	   is not a nice round number so 110 is used since it make the
	   programmer feel better. */

	strncat(specifics,msg,(PGS_SMF_MAX_MSG_SIZE-110));
    }
    else
    {
	code = PGS_SMF_GetMsgByCode(unknownStatus,msg);
	if (code == PGS_S_SUCCESS)
	{
	    sprintf(specifics,"%s%s%d%s","ERROR: encountered unexpected ",
		    "status value => value: ",unknownStatus,", message: ");
	    strncat(specifics,msg,(PGS_SMF_MAX_MSG_SIZE-75));
	}
	else
	{
	    sprintf(specifics,"%s%s%d%s%s","ERROR: encountered unexpected ",
		    "status value => value: ",unknownStatus," (no known ",
		    "mnemonic or message associated with this status value)");
	}
    }
    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,specifics,callingFunction);
}

