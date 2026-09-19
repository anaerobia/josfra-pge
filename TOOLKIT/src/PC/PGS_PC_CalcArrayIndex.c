/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_CalcArrayIndex.c

DESCRIPTION:
	This file contains the function PGS_PC_CalcArrayIndex().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	29-Mar-95 RM Initial version
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Calculate array index used in header structure.
 
NAME:  
	PGS_PC_CalcArrayIndex()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status
		PGS_PC_CalcArrayIndex(
			PGSt_integer		value,
			int			whatUsed,
			int			*index);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to calculate the array index needed
	to access the array of default locations in the header
	structure stored in shared memory.  The calculation is 
	based upon either the number of dividers counted in the
	PCF or the mode value used in PGS_PC_GetPCSData().
 
INPUTS:
	Name		Description			Units	Min	Max

	value		The actual value to be 
			interpreted.

	whatUsed	Flag indicating what type of
			input was used.  Possible
			values are:
				PGSd_PC_DIVS_VALUE
				PGSd_PC_MODE_VALUE

OUTPUTS:
	Name		Description			Units	Min	Max

	index		The actual array index to be
			used.

RETURNS:
	PGS_S_SUCCESS               successful execution

EXAMPLES:

C:
	PGSt_integer		value;
	int			index;
	PGSt_SMF_status		returnStatus;

	/# The number of dividers counted is stored in value #/

	returnStatus = PGS_PC_CalcArrayIndex(value,
					PGSd_PC_DIVS_VALUE,&index);
		.
		.
		.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	In oreder for this tool to function properly, a valid Process 
	Control file will need to be created first.  Please refer to 
	Appendix C (User's Guide) for instructions on how to create such 
	a file.

REQUIREMENTS:
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE
 
FUNCTIONS_CALLED:

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_CalcArrayIndex(                     /* calculate array index */
    PGSt_integer        value,             /* value passed in */
    int                 whatUsed,          /* method of calculation */
    int                *index)             /* actual index value */
{
    PGSt_SMF_status     returnStatus;      /* function return */

/***************************************************************************
*    Initialize status.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    The number of dividers counted is being passed in.  Set the 
*    index based on that.
***************************************************************************/
    if (whatUsed == PGSd_PC_DIVS_VALUE)
    {
        switch (value)
        {
            case PGSd_PC_INPUT_FILES:
                *index = 0;
                break;
            case PGSd_PC_OUTPUT_FILES:
                *index = 1;
                break;
            case PGSd_PC_SUPPORT_INPUT:
                *index = 2;
                break;
            case PGSd_PC_SUPPORT_OUTPUT:
                *index = 3;
                break;
            case PGSd_PC_INTER_INPUT:
                *index = 4;
                break;
            case PGSd_PC_INTER_OUTPUT:
                *index = 5;
                break;
            case PGSd_PC_TEMP_INFO:
                *index = 6;
                break;
            default:
                returnStatus = PGSPC_E_INVALID_MODE;
                break;
        }
    }

/***************************************************************************
*    The mode value is being passed in.  Set the index based on that.
*    This mode value is that which was probably passed into 
*    PGS_PC_GetPCSData().
***************************************************************************/
    else if (whatUsed == PGSd_PC_MODE_VALUE)
    {
        switch (value)
        {
            case PGSd_PC_INPUT_FILE_NAME:
            case PGSd_PC_INPUT_FILE_ATTRIBUTE:
            case PGSd_PC_PRODUCT_IN_DEFLOC:
                *index = 0;
                break;
            case PGSd_PC_OUTPUT_FILE_NAME:
            case PGSd_PC_OUTPUT_FILE_ATTRIBUTE:
            case PGSd_PC_PRODUCT_OUT_DEFLOC:
                *index = 1;
                break;
            case PGSd_PC_SUPPORT_IN_NAME:
            case PGSd_PC_SUPPORT_IN_ATTR:
            case PGSd_PC_SUPPORT_IN_DEFLOC:
                *index = 2;
                break;
            case PGSd_PC_SUPPORT_OUT_NAME:
            case PGSd_PC_SUPPORT_OUT_ATTR:
            case PGSd_PC_SUPPORT_OUT_DEFLOC:
                *index = 3;
                break;
            case PGSd_PC_INTERMEDIATE_INPUT:
            case PGSd_PC_INTER_IN_DEFLOC:
                *index = 4;
                break;
            case PGSd_PC_INTERMEDIATE_OUTPUT:
            case PGSd_PC_INTER_OUT_DEFLOC:
                *index = 5;
                break;
            case PGSd_PC_TEMPORARY_FILE:
            case PGSd_PC_TEMP_INFO_USEASCII:
            case PGSd_PC_TEMPDEL_TERM:
            case PGSd_PC_DELETE_TEMP:
            case PGSd_PC_TEMP_FILE_DEFLOC:
                *index = 6;
                break;
            default:
                returnStatus = PGSPC_E_INVALID_MODE;
                break;
        }
    }

    else
    {
        returnStatus = PGSPC_E_INVALID_MODE;
    }

/***************************************************************************
*    Return to calling function.
***************************************************************************/
    return returnStatus;
}

