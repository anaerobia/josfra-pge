/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_CheckFlags.c

DESCRIPTION:
	This file contains the function PGS_PC_CheckFlags().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	12-Apr-95 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Determine which flags exist, if any, in a line of data from the
	PCF.

NAME:  
	PGS_PC_CheckFlags()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status
		PGS_PC_CheckFlags(
			char			*line,
			PGSt_integer		*flags);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to determine which flags exist on a line
	of file information data in the PCF.
 
INPUTS:
	Name		Description			Units	Min	Max

	line		The line of file information
			data from the PCF.

OUTPUTS:
	Name		Description			Units	Min	Max

	flags		Types of flags contained on the
			line of data from the PCF.
			Possible values are:

			PGSd_PC_NO_FLAGS
			PGSd_PC_HAS_DELETE
			PGSd_PC_HAS_RUNTIME
			PGSd_PC_HAS_DELNRUN

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
	PGSt_SMF_status		returnStatus;
	char			line[PGSd_PC_LINE_LENGTH_MAX];
	PGSt_integer		flags;

		.
		.
		.

	/# assuming a line of data has been successfully read from the PCF #/

	returnStatus = PGS_PC_CheckFlags(line,&flags);
	if (flag == PGSd_PC_NO_FLAGS)
	{
		/# proceed as line with no flags #/
	}
	else if (flag == PGSd_PC_HAS_DELETE)
	{
		/# proceed as line with delete flag #/
	}
	else if (flag == PGSd_PC_HAS_RUNTIME)
	{
		/# proceed as line with runtime flag #/
	}
	else if (flag == PGSd_PC_HAS_DELNRUN)
	{
		/# proceed as line with runtime and delete flags #/
	}
		.
		.
		.
	return returnStatus;

FORTRAN:
	NONE

NOTES:
	In order for this tool to function properly, a valid Process 
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
	NONE
 
FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_SetStaticMsg             Set static error/status message

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_CheckFlags(                   /* check line for flags */
    char           *line,            /* line of data */
    PGSt_integer   *flags)           /* flags contained in line */
{
    char           *hold;            /* temporary hold string */
    int             len;             /* string length */
    PGSt_SMF_status whoCalled;       /* user that called this function */
    PGSt_SMF_status returnStatus;    /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    *flags = PGSd_PC_NO_FLAGS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Check line for DELETE flag.
***************************************************************************/
    hold = (char *) strrchr(line,PGSd_PC_DELETE_FLAG);
    if (hold == NULL)
    {
        len = 0;
    }
    else
    {
        len = strlen(hold);
    }

/***************************************************************************
*    If DELETE flag exists in proper location set return value.
***************************************************************************/
    if (len == 2)
    {
        *flags = PGSd_PC_HAS_DELETE;
    }
    else
    {

/***************************************************************************
*    Check for existence of RUNTIME flag.
***************************************************************************/
        hold = (char *) strrchr(line,PGSd_PC_RUNTIME_FLAG);
        if (hold == NULL)
        {
            len = 0;
        }
        else
        {
            len = strlen(hold);
        }

/***************************************************************************
*    If RUNTIME flag exists in proper location set return value.
***************************************************************************/
        if (len == 2) 
        {
            *flags = PGSd_PC_HAS_RUNTIME;
        }
        else 
        {

/***************************************************************************
*    Check for existence of DELETE and RUNTIME flag.
***************************************************************************/
            hold = (char *) strrchr(line,PGSd_PC_DELNRUN_FLAG);
            if (hold == NULL)
            {
                len = 0;
            }
            else
            {
                len = strlen(hold);
            }

/***************************************************************************
*    If DELETE and RUNTIME flag exists in proper location set 
*    return value.
***************************************************************************/
            if (len == 2) 
            {
                *flags = PGSd_PC_HAS_DELNRUN;
            }
        }  /* end else */
    }  /* end else */

/***************************************************************************
*    Set message log and return to calling function.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_CheckFlags()"); 
    }
    return returnStatus;
}
