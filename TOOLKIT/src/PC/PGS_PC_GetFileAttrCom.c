/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetFileAttrCom.c

DESCRIPTION:
	This file contains all functions necessary to build 
	PGS_PC_GetFileAttrCom.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	09-Jan-95 RM Initial Version
        26-Jan-95 RM Updated from comments of code inspection held
                        on 25-Jan-95.
	31-Jan-95 RM Updated examples sections due to DR ECSed00605.
	10-Feb-95 RM Added check of format flag to ensure a valid
			flag is being passed in per DR ECSed00702.
	17-Feb-95 RM Updated prolog placing note about c-shell limitation
			as per DR ECSed00692.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_PC.h>
#include <PGS_MEM.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Get the file attribute string or location associated with a product
	input file command.
 
NAME:  
	PGS_PC_GetFileAttrCom

SYNOPSIS:
	PGS_PC_GetFileAttrCom <logical ID> <version> <format flag>

DESCRIPTION:
	This program will retrieve a file attribute string or location 
	associated with a product input file from the PCF or shared memory 
	at the command line.
 
INPUTS:
        Name            Description                     Units   Min     Max

	argc		number of command line arguments

	argv[0]		executable name (not processed 
			but listed here anyway)

	argv[1]		logical ID of the configuration
			parameter

	argv[2]		version number of file to retrieve
			attribute for

	argv[3]		format flag which states whether
			to return the attribute or the
			location of the file attribute.
			Possible values are:
				PGSd_PC_ATTRIBUTE_LOCATION
				PGSd_PC_ATTRIBUTE_STRING

OUTPUTS:
        Name            Description                     Units   Min     Max

	NONE

RETURNS:
	PGS_S_SUCCESS
        PGS_SH_SYS_PARAM
	PGS_SH_PC_NODATA
	PGS_SH_PC_TOOLERROR
	PGS_SH_PC_TRUNC

EXAMPLES:
	The following example is valid for the bourne and korn shells
	only.

	#  This is within a shell script - probably within the 
	#  PGE script.

        #  Set our format flag values.  (This is bourne shell format)
        #  These values are set in PGS_PC_Shell.sh.
	: ${PGSd_PC_ATTRIBUTE_LOCATION=1}
	: ${PGSd_PC_ATTRIBUTE_STRING=2}

	LogicalID=12297
	Version=1
	FormatFlag=$PGSd_PC_ATTRIBUTE_STRING

	#  Get the file attribute string associated with
	#  the first file of product ID 12297
	ATTR=`PGS_PC_GetFileAttrCom $LogicalID $Version $FormatFlag`
	RETVAL=$?

	#  Check the return value
	if [ $RETVAL -eq 0 ]
	then
	#  continue normal processing
	#  Variable ATTR now contains the attribute string
	else
	#  report an error found
	fi
	.
	.	
	.

	If the user wishes to use a c-shell script this is the 
	recommended technique to use.  In a c-shell scipt if the 
	user fails to use this technique the scipt will give
	undefined results (see NOTES).

	#  This is within a shell script - probably within the 
	#  PGE script.

        #  Set our format flag values.
        #  These values are set in PGS_PC_Shell.sh.
	set PGSd_PC_ATTRIBUTE_LOCATION=1
	set PGSd_PC_ATTRIBUTE_STRING=2

	set LogicalID=12297
	set Version=1
	set FormatFlag=$PGSd_PC_ATTRIBUTE_STRING

	#  Get the file attribute string associated with
	#  the first file of product ID 12297
	PGS_PC_GetFileAttrCom $LogicalID $Version $FormatFlag >out.file
	set RETVAL=$status

	#  Check the return value
	if [ $RETVAL -eq 0 ]
	then
	#  continue normal processing
	#  File out.file now contains the attribute string
	else
	#  report an error found
	fi
	.
	.	
	.

NOTES:
	This program is designed to be run from within the PGE.

	If the format flag passed in is equal to PGSd_PC_ATTRIBUTE_STRING the
	return value is the attribute string appended as one long string.  If
	the format flag passed in is equal to PGSd_PC_ATTRIBUTE_LOCATION the
	return value is the attribute location which is a full path and file
	name of the file containing the attribute string.

	If the user wishes to use this program in a c-shell script the 
	output of the program must be re-directed to a file and the file
	can then be manipulated.  A long string can not be assigned to 
	a variable in a c-shell script.  Attempting to assign a long string
	to a variable will give undefined results in the c-shell.

REQUIREMENTS:
	PGSTK-1290
	PGSTK-1314

DETAILS:
        This program handles return values of level _M_, although at this
        time no function in this program is returning it.

GLOBALS:
	NONE

FILES:
	NONE	

FUNCTIONS_CALLED:
	PGS_PC_GetFileAttr		- Get attribute string or location
					associated with a product input
					file from the location storing the 
					PCS data.
        PGS_PC_BuildNumericInput        - build a number from Com Tool input.
        PGS_SMF_TestStatusLevel         - Determine the level of the
                                         error/status code
	PGS_MEM_Malloc			- Allocate memory
	PGS_MEM_Free			- Free memory

END_PROLOG:
***************************************************************************/

/***************************************************************************
*    Function main().
***************************************************************************/
int
main(                                    /* main function */
    int                 argc,            /* number of command line arguments */
    char               *argv[])          /* array of command line arguments */
{
    char               *attribute;       /* attribute string or location */
    PGSt_integer        retNum;          /* number returned */
    PGSt_PC_Logical     logicalID;       /* logical ID passed in */
    PGSt_integer        version;         /* version of file to get */
    PGSt_integer        formatFlag;      /* what form to get attribute */
    PGSt_integer        maxSize;         /* max size of attribute string */
    PGSt_SMF_status     returnStatus;    /* return from functions */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Determine if the correct number of parameters were passed in.
***************************************************************************/
    if (argc != 4)
    {
        returnStatus = PGS_SH_SYS_PARAM;
    }

/***************************************************************************
*    We have the correct number of input parameters, let's ensure that
*    the parameter sent in is all digit.  At the same time let's put it
*    in our own variable.
***************************************************************************/
    else
    {
        returnStatus = PGS_PC_BuildNumericInput(argv[1],&retNum);

        if (returnStatus == PGS_S_SUCCESS)
        {
            logicalID = (PGSt_PC_Logical) retNum;
            returnStatus = PGS_PC_BuildNumericInput(argv[2],&retNum);

            if (returnStatus == PGS_S_SUCCESS)
            {
                version = (PGSt_integer) retNum;
                returnStatus = PGS_PC_BuildNumericInput(argv[3],&retNum);
            }
        }  /* end if */

   
/***************************************************************************
*    If everything is still OK then we need to determine if the user
*    wants the attribute location or the attribute string.  Once we
*    have determined that we need to allocate the space for it.
***************************************************************************/
        if (returnStatus == PGS_S_SUCCESS)
        {
            formatFlag = (PGSt_integer) retNum;
            if (formatFlag == PGSd_PC_ATTRIBUTE_LOCATION)
            {
                returnStatus = PGS_MEM_Malloc((void **) &attribute,
                                           PGSd_PC_FILE_PATH_MAX);
                maxSize = 0;
            }
            else if (formatFlag == PGSd_PC_ATTRIBUTE_STRING)
            {
                returnStatus = PGS_MEM_Malloc((void **) &attribute,
                                           PGSd_PC_ATTRCOM_MEM);
                maxSize = PGSd_PC_ATTRCOM_MEM;
            }
            else
            {
                returnStatus = PGS_SH_SYS_PARAM;
            }
        
/***************************************************************************
*    If our malloc worked OK let's go ahead and get the attribute
*    location/string.
***************************************************************************/
            if (returnStatus == PGS_S_SUCCESS)
            {
                returnStatus = PGS_PC_GetFileAttr(logicalID,version,
                                       formatFlag,maxSize,attribute);
            }

/***************************************************************************
*    If our return is successful print out the answer so it can be 
*    captured in a script file.  Otherwise, set up the return value
*    for program exit.  If the attribute has been truncated, print it
*    out anyway, the user may still want to look at it.
***************************************************************************/
            if (returnStatus != PGS_SH_SYS_PARAM)
            {
                switch (PGS_SMF_TestStatusLevel(returnStatus))
                {
                    case PGS_SMF_MASK_LEV_S:
                    case PGS_SMF_MASK_LEV_M:
                        printf("%s",attribute);
                        break;
                    case PGS_SMF_MASK_LEV_E:
                        returnStatus = PGS_SH_PC_TOOLERROR;
                        break;
                    case PGS_SMF_MASK_LEV_W:
                        if (returnStatus == PGSPC_W_ATTR_TRUNCATED)
                        {
                            returnStatus = PGS_SH_PC_TRUNC;
                            printf("%s",attribute);
                        }
                        else
                        {
                            returnStatus = PGS_SH_PC_NODATA;
                        }
                        break;
                }  /* end switch */
                PGS_MEM_Free(attribute);
            }  /* end if */
        }  /* end if */
    }  /* end else */

/***************************************************************************
*    Exit program.
***************************************************************************/
    exit(returnStatus);
}
