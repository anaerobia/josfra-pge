/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
        PGS_PC_GetFileSizeCom.c
 
DESCRIPTION:
        This file contains all functions necessary to build
        PGS_PC_GetFileSizeCom. 
 
AUTHOR:
        Carol S. W. Tsai / Applied Research Corp.
 
HISTORY:
        22-Jan-97 CSWT Initial Version
 
END_FILE_PROLOG:
***************************************************************************/
 
#include <stdio.h>
#include <PGS_PC.h>
#include <PGS_SMF.h>
 
/***************************************************************************
BEGIN_PROLOG:
 
TITLE:
        Get File Size Command
 
NAME:
        PGS_PC_GetFileSizeCom
 
SYNOPSIS:
        PGS_PC_GetFileSizeCom <logical ID> <version>
 
DESCRIPTION:
        This program will retrieve the file size of the file associated with
	the input logical ID and version in the users Process Control File
	(PCF). 

INPUTS:
        Name            Description
 
        <logical id>    logical ID (in the PCF) of the desired file
 
        <version>       file version number
 
OUTPUTS:
        Description                                   Units   Min     Max
        Size of file referenced in the users PCF      bytes   0       2^31 - 1
        
 
RETURNS:
        PGS_S_SUCCESS
        PGS_SH_SYS_PARAM
        PGS_SH_PC_TOOLERROR
 
EXAMPLES:
        #  This is within a shell script - probably within the
        #  PGE shell.  This example assumes there is an entry for
	#  for a file in the users PCF with logical ID 101

        LogicalID=101
        Version=1

        #  Get the physical file size associated with the user's input
        #  arguments LogicalID and Version 
 
        SIZE= `PGS_PC_GetFileSizeCom $LogicalID $Version`
        RETVAL=$?
 
        #  Check the return value

        if [ $RETVAL -eq 0 ]
        then

        #  SIZE now contains the file size.
        #  continue normal processing...
                 :
                 :
        else

        #  handle error case...
                 :
                 :
        fi
 
NOTES:
        This program is designed to be run from within the PGE.

        The user will be required to parse the file name, version,
        and the file size remaining from the output string. This can be 
        done using the cut command (See EXAMPLEs). The file name, version, 
        and the file size  will be seperated by a single space.
 
REQUIREMENTS:
        PGSTK-1290
 
DETAILS:
        NONE
 
GLOBALS:
        NONE
 
FILES:
        NONE
 
FUNCTIONS_CALLED:
        PGS_PC_GetReference             - Get reference associated with a
                                        logical ID from the location storing
                                        the PCS data.
        PGS_PC_FileSize                  - Get reference associated with a
                                        logical ID from the location storing
                                        the PCS data.
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
    PGSt_integer        retNum;          /* number returned */
    PGSt_integer        version;         /* version number */
    PGSt_PC_Logical     logicalID;       /* logical ID passed in */
    PGSt_SMF_status     returnStatus;    /* return from functions */
    PGSt_integer        filesize=0;

    
/***************************************************************************
*    Determine if the correct number of parameters were passed in.
***************************************************************************/

    if (argc != 3)
    {
        return PGS_SH_SYS_PARAM;
    }
 
/***************************************************************************
*    We have the correct number of input parameters, let's ensure that
*    the parameter sent in is all digit.  At the same time let's put it
*    in our own variable.
***************************************************************************/

    returnStatus = PGS_PC_BuildNumericInput(argv[1],&retNum);
    
    if (returnStatus != PGS_S_SUCCESS)
    {
	return returnStatus;
    }
    
    logicalID = (PGSt_PC_Logical) retNum;
    returnStatus = PGS_PC_BuildNumericInput(argv[2],&retNum);

/***************************************************************************
*    If everything is still OK then set our version variable and call
*    PGS_PC_FileSize().
***************************************************************************/

    if (returnStatus == PGS_S_SUCCESS)
    {
	version = (PGSt_integer) retNum;
	returnStatus = PGS_PC_GetFileSize(logicalID, version, &filesize);
	
/***************************************************************************
*    If our return is successful print out the answer so it can be
*    captured in a script file.
***************************************************************************/
 
	if (returnStatus == PGS_S_SUCCESS)
	{
	    printf("%d\n", filesize);
	}
	
/***************************************************************************
*    If we did not get success from our PGS_PC_GetFileSize() call
*    let's set up the proper return value.
***************************************************************************/
	else
	{
	    returnStatus = PGS_SH_PC_TOOLERROR;
	}
    }  /* end if */
 
/***************************************************************************
*    Exit program.
***************************************************************************/

    return returnStatus;
}
