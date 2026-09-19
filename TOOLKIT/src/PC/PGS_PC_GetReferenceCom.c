/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetReferenceCom.c

DESCRIPTION:
	This file contains all functions necessary to build 
	PGS_PC_GetReferenceCom.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	10-Jan-95 RM Initial Version
        26-Jan-95 RM Updated from comments of code inspection held
                        on 25-Jan-95.
	17-Feb-95 RM Updated examples section due to DR ECSed00759.
	13-Dec-95 RM Updated for TK6.  Edited documentation to reflect
			new functionality of allowing multiple instances
			of PRODUCT OUTPUT FILES.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Get physical file reference command.
 
NAME:  
	PGS_PC_GetReferenceCom

SYNOPSIS:
	PGS_PC_GetReferenceCom <logical ID> <version>

DESCRIPTION:
	This program will retrieve the physical file reference associated
	with a logical ID.
 
INPUTS:
        Name            Description                     Units   Min     Max

	argc		number of command line arguments

	argv[0]		executable name (not processed 
			but listed here anyway)

	argv[1]		logical ID of the configuration
			parameter

	argv[2]		version of the physical file 
			reference to retrieve.  A one-to-one
			relationship exists between all 
			files except for product input and
			product output files.

OUTPUTS:
        Name            Description                     Units   Min     Max

	NONE

RETURNS:
	PGS_S_SUCCESS
        PGS_SH_SYS_PARAM
	PGS_SH_PC_NODATA
	PGS_SH_PC_TOOLERROR

EXAMPLES:
	#  This is within a shell script - probably within the 
	#  PGE shell.

	LogicalID=12297
	Version=1

	#  Get the physical file reference associated
	#  with ID 12297
	REFERENCE=`PGS_PC_GetReferenceCom $LogicalID $Version`
	RETVAL=$?

	#  Check the return value
	if [ $RETVAL -eq 0 ]
	then
	#  continue normal processing
	#  This is how the file name and versions remaining 	
	#  can be parsed.
		FILENAME=`echo $REFERENCE | cut -f1 -d" "`
		VERSIONS=`echo $REFERENCE | cut -f2 -d" "`
	#  FILENAME now contains the file reference.
	#  VERSIONS now contains the versions remaining.
	else
	#  report an error found
	fi
	.
	.	
	.

        Another method of performing this task is as listed below.  This
        method only works in the korn and bourne shells.

	#  This is within a shell script - probably within the 
	#  PGE shell.

	LogicalID=12297
	Version=1

	#  Get the physical file reference associated
	#  with ID 12297
	set `PGS_PC_GetReferenceCom $LogicalID $Version`
	#  The file reference and versions remaining will
	#  now appear in two separate tokens.
	RETVAL=$?

	#  Check the return value
	if [ $RETVAL -eq 0 ]
	then
	#  continue normal processing
		FILENAME=$1
		VERSIONS=$2
	#  FILENAME now contains the file reference.
	#  VERSIONS now contains the versions remaining.
	else
	#  report an error found
	fi
	.
	.	
	.

        A final method of performing this task is as listed below.  This
        method only works in the korn and bourne shells.

	#  This is within a shell script - probably within the 
	#  PGE shell.

	LogicalID=12297
	Version=1

	#  Get the physical file reference associated
	#  with ID 12297
	set "`PGS_PC_GetReferenceCom $LogicalID $Version`"
	#  Placing double quotes around the command causes
	#  the string to be placed in one token.	
	RETVAL=$?

	#  Check the return value
	if [ $RETVAL -eq 0 ]
	then
	#  continue normal processing
	#  This is how the file name and versions remaining 	
	#  can be parsed.
		FILENAME=`echo $1 | cut -f1 -d" "`
		VERSIONS=`echo $1 | cut -f2 -d" "`
	#  FILENAME now contains the file reference.
	#  VERSIONS now contains the versions remaining.
	else
	#  report an error found
	fi
	.
	.	
	.

NOTES:
	This program is designed to be run from within the PGE.
	
	The user will be required to parse the file name and number of files
	remaining from the output string.  This can be done using the cut 
	command (See EXAMPLES).  The file name and versions remaining will 
	be separated by a single space.

REQUIREMENTS:
	PGSTK-1290

DETAILS:
	NONE	

GLOBALS:
	NONE

FILES:
	NONE	

FUNCTIONS_CALLED:
	PGS_PC_GetReference		- Get reference associated with a
					logical ID from the location storing 
					the PCS data.
	PGS_PC_BuildNumericInput	- build a number from Com Tool input.

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
    char                referenceID[PGSd_PC_FILE_PATH_MAX];  /* reference */
    PGSt_integer        retNum;          /* number returned */
    PGSt_integer        version;         /* version number */
    PGSt_PC_Logical     logicalID;       /* logical ID passed in */
    PGSt_SMF_status     returnStatus;    /* return from functions */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Determine if the correct number of parameters were passed in.
***************************************************************************/
    if (argc != 3)
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
        }
   
/***************************************************************************
*    If everything is still OK then set our version variable and call
*    PGS_PC_GetReference().
***************************************************************************/
        if (returnStatus == PGS_S_SUCCESS)
        {
            version = (PGSt_integer) retNum;
            returnStatus = PGS_PC_GetReference(logicalID,&version,referenceID);

/***************************************************************************
*    If our return is successful print out the answer so it can be 
*    captured in a script file.
***************************************************************************/
            if (returnStatus == PGS_S_SUCCESS)
            {
                printf("%s %d",referenceID,version);
            }

/***************************************************************************
*    If we did not get success from our PGS_PC_GetReference() call 
*    let's set up the proper return value.
***************************************************************************/
            else
            {
                if (returnStatus == PGSPC_W_NO_REFERENCE_FOUND)
                {
                    returnStatus = PGS_SH_PC_NODATA;
                }
                else
                {
                    returnStatus = PGS_SH_PC_TOOLERROR;
                }
            }
        }  /* end if */
    }  /* end else */

/***************************************************************************
*    Exit program.
***************************************************************************/
    exit(returnStatus);
}
