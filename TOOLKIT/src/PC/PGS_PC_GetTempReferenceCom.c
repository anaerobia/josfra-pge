/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetTempReferenceCom.c

DESCRIPTION:
	This file contains all functions necessary to build 
	PGS_PC_GetTempReferenceCom.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	11-Jan-95 RM Initial Version
        26-Jan-95 RM Updated from comments of code inspection held
                        on 25-Jan-95.
	31-Jan-95 RM Updated examples section due to DR ECSed00605.
	17-Feb-95 RM Updated examples section due to DR ECSed00759.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Get the temporary file reference.
 
NAME:  
	PGS_PC_GetTempReferenceCom

SYNOPSIS:
	PGS_PC_GetTempReferenceCom <logical ID> <duration of file>

DESCRIPTION:
	This program will retrieve a temporary file reference from the 
	PCF.  If a reference does not exist it will create one.
 
INPUTS:
        Name            Description                     Units   Min     Max

	argc		number of command line arguments

	argv[0]		executable name (not processed 
			but listed here anyway)

	argv[1]		logical ID of the temporary file
			reference

	argv[2]		file duration

OUTPUTS:
        Name            Description                     Units   Min     Max

	NONE

RETURNS:
	PGS_S_SUCCESS
        PGS_SH_SYS_PARAM
	PGS_SH_PC_TOOLERROR

EXAMPLES:
        #  This is within a shell script - probably within the
	#  PGE script.

	#  Set our endurance values.  (This is bourne shell format)
	#  These values are set in PGS_PC_Shell.sh.
	: ${PGSd_IO_Gen_NoEndurance=0}
	: ${PGSd_IO_Gen_Endurance=1}

	LogicalID=12297
	Endurance=$PGSd_IO_Gen_NoEndurance

	#  Get the temporary physical file reference associated 
	#  with ID 12297
	TEMPREFERENCE=`PGS_PC_GetTempReferenceCom $LogicalID $Endurance`
	RETVAL=$?

	#  Check the return value
	if [ $RETVAL -eq 0 ]
	then
        #  continue normal processing
        #  This is how the file name and versions remaining
        #  can be parsed.
                FILENAME=`echo $TEMPREFERENCE | cut -f1 -d" "`
                EXISTS=`echo $TEMPREFERENCE | cut -f2 -d" "`
        #  FILENAME now contains the file reference.
        #  EXISTS now contains the existence flag.
	else
	#  report an error found
	fi
	.
	.	
	.

	Another method of performing this task is as listed below.  This
	method only works in the korn and bourne shells.


        #  This is within a shell script - probably within the
	#  PGE script.

	#  Set our endurance values.  (This is bourne shell format)
	#  These values are set in PGS_PC_Shell.sh.
	: ${PGSd_IO_Gen_NoEndurance=0}
	: ${PGSd_IO_Gen_Endurance=1}

	LogicalID=12297
	Endurance=$PGSd_IO_Gen_NoEndurance

	#  Get the temporary physical file reference associated 
	#  with ID 12297
	set `PGS_PC_GetTempReferenceCom $LogicalID $Endurance`
        #  The file reference and existence flag will
        #  now appear in two separate tokens.
	RETVAL=$?

	#  Check the return value
	if [ $RETVAL -eq 0 ]
	then
        #  continue normal processing
                FILENAME=$1
                EXISTS=$2
        #  FILENAME now contains the file reference.
        #  EXISTS now contains the existence flag.
	else
	#  report an error found
	fi
	.
	.	
	.


	A final method of performing this task is as listed below.  This
	method only works in the korn and bourne shells.


        #  This is within a shell script - probably within the
	#  PGE script.

	#  Set our endurance values.  (This is bourne shell format)
	#  These values are set in PGS_PC_Shell.sh.
	: ${PGSd_IO_Gen_NoEndurance=0}
	: ${PGSd_IO_Gen_Endurance=1}

	LogicalID=12297
	Endurance=$PGSd_IO_Gen_NoEndurance

	#  Get the temporary physical file reference associated 
	#  with ID 12297
	set "`PGS_PC_GetTempReferenceCom $LogicalID $Endurance`"
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
                EXISTS=`echo $1 | cut -f2 -d" "`
        #  FILENAME now contains the file reference.
        #  EXISTS now contains the existence flag.
	else
	#  report an error found
	fi
	.
	.	
	.

NOTES:
	This program is designed to be run from within the PGE.

	If a temporary file reference does not exist for the logical ID 
	then a reference is created.  The user will be able to determine
	if the reference existed by checking the existence flag portion
	of the program return (See EXAMPLES).

        The user will be required to parse the file name and the existence
	flag from the output string.  This can be done using the cut
        command (See EXAMPLES).  The file name and the existence flag will
        be separated by a single space.

REQUIREMENTS:
	PGSTK-0360
	PGSTK-1291
	PGSTK-530
	PGSTK-531
	PGSTK-535

DETAILS:
	This program handles return values of level _M_, although at this
	time no function in this program is returning it.

GLOBALS:
	NONE

FILES:
	NONE	

FUNCTIONS_CALLED:
	PGS_IO_Gen_Temp_Reference	- Get a temporary file reference, if 
					a reference does not currently exist
					then create one.
        PGS_PC_BuildNumericInput        - build a number from Com Tool input.
        PGS_SMF_TestStatusLevel         - Determine the level of the
                                         error/status code

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
    char                tempReference[PGSd_PC_FILE_PATH_MAX];  /* temp reference */
    PGSt_integer        retNum;          /* number returned */
    PGSt_boolean        existFlag;       /* existence flag */
    PGSt_PC_Logical     logicalID;       /* logical ID passed in */
    PGSt_IO_Gen_Duration duration;       /* file duration */
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

/***************************************************************************
*    If everything is still OK put our logical ID in our variable and
*    get our duration variable.
***************************************************************************/
        if (returnStatus == PGS_S_SUCCESS)
        {
            logicalID = (PGSt_PC_Logical) retNum;
            returnStatus = PGS_PC_BuildNumericInput(argv[2],&retNum);
        }
   
/***************************************************************************
*    If everything is still OK put proper duration value in the proper
*    variable.
***************************************************************************/
        if (returnStatus == PGS_S_SUCCESS)
        {
            if (retNum == PGSd_IO_Gen_NoEndurance)
            {
                duration = (PGSt_IO_Gen_Duration) PGSd_IO_Gen_NoEndurance;
            }
            else if (retNum == PGSd_IO_Gen_Endurance)
            {
                duration = (PGSt_IO_Gen_Duration) PGSd_IO_Gen_Endurance;
            }
            else
            {
                returnStatus = PGS_SH_SYS_PARAM;
            }
            
/***************************************************************************
*    If everything is still OK call PGS_IO_Gen_Temp_Reference().
***************************************************************************/
            if (returnStatus == PGS_S_SUCCESS)
            {
                returnStatus = PGS_IO_Gen_Temp_Reference(duration,
                  logicalID,PGSd_IO_Gen_Write,tempReference,&existFlag);

/***************************************************************************
*    If our return is successful print out the answer so it can be 
*    captured in a script file.  Otherwise set our return value to
*    something that the script can understand.
***************************************************************************/
                switch (PGS_SMF_TestStatusLevel(returnStatus))
                {
                    case PGS_SMF_MASK_LEV_E:
                        returnStatus = PGS_SH_PC_TOOLERROR;
                        break;
                    case PGS_SMF_MASK_LEV_M:
                    case PGS_SMF_MASK_LEV_W:
                        returnStatus = PGS_S_SUCCESS;
                    case PGS_SMF_MASK_LEV_S:
                        printf("%s %d",tempReference,existFlag);
                        break;
                }
            }  /* end if */
        }  /* end if */
    }  /* end else */

/***************************************************************************
*    Exit program.
***************************************************************************/
    exit(returnStatus);
}
