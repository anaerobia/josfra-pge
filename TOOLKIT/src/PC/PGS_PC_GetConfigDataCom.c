/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetConfigDataCom.c

DESCRIPTION:
	This file contains all functions necessary to build 
	PGS_PC_GetConfigDataCom.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	29-Dec-94 RM Initial Version
	26-Jan-95 RM Updated from comments of code inspection held
			on 25-Jan-95.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Get user defined configuration parameters command.
 
NAME:  
	PGS_PC_GetConfigDataCom

SYNOPSIS:
	PGS_PC_GetConfigDataCom <logical ID>

DESCRIPTION:
	This program will retrieve user defined configuration parameters 
	from the PCF or shared memory at the command line.
 
INPUTS:
        Name            Description                     Units   Min     Max

	argc		number of command line arguments

	argv[0]		executable name (not processed 
			but listed here anyway)

	argv[1]		logical ID of the configuration
			parameter

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
	#  PGE script

	LogicalID=12297

	#  Get the parameter associated with ID 12297
	CONFIG=`PGS_PC_GetConfigDataCom $LogicalID`
	RETVAL=$?

	#  Check the return value
	if [ $RETVAL -eq 0 ]
	then
	#  continue normal processing
	else
	#  report an error found
	fi
	.
	.	
	.

NOTES:
	This program is designed to be run from within the PGE.

REQUIREMENTS:
	PGSTK-1290
	PGSTK-1291

DETAILS:
	NONE	

GLOBALS:
	NONE

FILES:
	NONE	

FUNCTIONS_CALLED:
	PGS_PC_GetConfigData		- Get configuration data from the
					location storing the PCS data.
        PGS_PC_BuildNumericInput        - build a number from Com Tool input.

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
    char                configParam[PGSd_PC_VALUE_LENGTH_MAX];  /* config data */
    PGSt_integer        retNum;          /* number returned */
    PGSt_PC_Logical     logicalID;       /* logical ID passed in */
    PGSt_SMF_status     returnStatus;    /* return from functions */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Determine if the correct number of parameters were passed in.
***************************************************************************/
    if (argc != 2)
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
   
/***************************************************************************
*    Convert our number return to the type we need and go ahead and call
*    PGS_PC_GetConfigData().
***************************************************************************/
        if (returnStatus == PGS_S_SUCCESS)
        {
            logicalID = (PGSt_PC_Logical) retNum;
            returnStatus = PGS_PC_GetConfigData(logicalID,configParam);

/***************************************************************************
*    If our return is successful print out the answer so it can be 
*    captured in a script file.
***************************************************************************/
            if (returnStatus == PGS_S_SUCCESS)
            {
                printf("%s",configParam);
            }

/***************************************************************************
*    If we did not get success from our PGS_PC_GetConfigData() call
*    let's set up the proper return value.
***************************************************************************/
            else
            {
                if (returnStatus == PGSPC_W_NO_CONFIG_FOR_ID)
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
