/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_FixBuffer.c

DESCRIPTION:
	This file contains the function PGS_PC_PutPCSDataFixBuffer().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	05-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID().
	11-Mar-96 RM Added code to check file pointers to ensure
			that they are not NULL.  This change was 
			required per ECSed01815.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Close out the area storing the Process Control Status information.
 
NAME:  
	PGS_PC_PutPCSDataFixBuffer()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status
		PGS_PC_PutPCSDataFixBuffer(
			FILE			*locationPCS[NUMFILES]);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to clean up the areas that contain the 
	Process Control Status information.  Currently, this is just
	a file.  In a future release, this will be shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	locationPCS	Pointers to the files containing
			the PCS data.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
	PGSt_SMF_status		returnStatus;
	FILE			*locationPCS[NUMFILES];

		.
		.
		.
	returnStatus = PGS_PC_PutPCSDataFixBuffer(locationPCS);
		.
		.
		.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	The user (application programmer) must ensure that all data fields 
	listed as optional in the Process Control Inormation file are 
	initialized before being passed to this function.  Failing to 
	initialize any unused data will give undefined results.

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
	This closes "tempPCS.fil" and the file defined in the environment 
	variable "PGS_PC_INFO_FILE".
 
FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_SetStaticMsg             Set static error/status message

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_PutPCSDataFixBuffer(                /* update PCS area */
    FILE        *locationPCS[NUMFILES])    /* location of PCS data */
{
    PGSt_SMF_status     whoCalled;         /* user who called this function */
    PGSt_SMF_status     returnStatus;      /* function return */

/***************************************************************************
*    Initialize status.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Close our files.
***************************************************************************/
    if (locationPCS[PCSFILE] != (FILE *) NULL)
    {
        fclose(locationPCS[PCSFILE]);
    }

    if (locationPCS[TEMPFILE] != (FILE *) NULL)
    {
        fclose(locationPCS[TEMPFILE]);
    }

/***************************************************************************
*    Set our message log and return.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_PutPCSDataFixBuffer()");
    }
    return returnStatus;
}

