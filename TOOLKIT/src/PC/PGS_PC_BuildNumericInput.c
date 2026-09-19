/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_BuildNumericInput.c

DESCRIPTION:
	This file contains the function PGS_PC_BuildNumericInput().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	09-Jan-95 RM Initial Version
 
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
	Check input to ensure all digits and build a number.
 
NAME:  
	PGS_PC_BuildNumericInput()

SYNOPSIS:

C:
	#include <PGS_PC.h>

	PGSt_SMF_status
	PGS_PC_BuildNumericInput(
		char		*inString,
		PGSt_integer	numReturned);

FORTRAN:
	NONE

DESCRIPTION:
	This function will receive an input string that had been passed in to
	a Com tool and determine if it contains all digits and convert it
	to a number.
 
INPUTS:
        Name            Description                     Units   Min     Max

	inString	String containing input from a
			Com tool.

OUTPUTS:
        Name            Description                     Units   Min     Max

	numReturned	The number form of the input
			string.

RETURNS:
	PGS_S_SUCCESS
        PGS_SH_SYS_PARAM

EXAMPLES:
	char		*strnum;
	PGSt_integer	numReturned;
	PGSt_SMF_status	returnStatus;

	/# strnum must point to some characters #/
	
	returnStatus = PGS_PC_BuildNumericInput(strnum,numReturned);

	if (returnStatus == PGS_S_SUCCESS)
	{
	/# continue processing with numReturned #/
	}
	else
	{
		goto EXCEPTION;
	}
	.
	.	
	.

	EXCEPTION:
		return returnStatus;

NOTES:
	NONE

REQUIREMENTS:
	NONE

DETAILS:
	NONE	

GLOBALS:
	NONE

FILES:
	NONE	

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_BuildNumericInput(                /* build a number from input */
    char               *inString,        /* string input */
    PGSt_integer       *numReturned)     /* output number */
{
    char                strnum[20];      /* number as a string */
    int                 pos;             /* position of digit in string */
    PGSt_integer        tempNum;         /* temporary number */
    PGSt_SMF_status     returnStatus;    /* return from functions */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

    pos = 0;

    while((inString[pos] != PGSd_PC_NEWLINE) && 
          (inString[pos] != '\0'))
    {
        if ((inString[pos] >= PGSd_PC_LOWDIGIT) && 
            (inString[pos] <= PGSd_PC_HIDIGIT))
        {
            strnum[pos] = inString[pos];
            pos++;
        }

/***************************************************************************
*    Invalid character.
***************************************************************************/
        else
        {
            returnStatus = PGS_SH_SYS_PARAM;
            break;
        }
    }  /* end while */
   
    if (returnStatus == PGS_S_SUCCESS)
    {
        strnum[pos] = '\0';
        tempNum = (PGSt_integer) atol(strnum);
        *numReturned = tempNum;
    }

/***************************************************************************
*    Return to calling function.
***************************************************************************/
    return returnStatus;
}
