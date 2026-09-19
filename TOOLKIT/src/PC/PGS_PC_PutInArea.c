/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_PutInArea.c

DESCRIPTION:
	This file contains the function PGS_PC_PutPCSDataPutInArea().

AUTHOR:
	Ray Milburn / Applied Research Corporation

HISTORY:
	05-Aug-94 RM Initial version 
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	22-Dec-94 RM Removed functionality that actually removes the 
			temporary file.
	14-Feb-95 RM Added function call stat() to determine if file
			being marked is on disk as per DR ECSed00568.

END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Put Process Control Status information.

NAME:  
	PGS_PC_PutPCSDataPutInArea()

SYNOPSIS:

C:
		#include <PGS_PC.h>

		PGSt_SMF_status
		PGS_PC_PutPCSDataPutInArea(
			PGSt_integer		mode,
			FILE			*locationPCS[NUMFILES],
			void			*PCS_data);

FORTRAN:
	NONE

DESCRIPTION:
	This tool actually makes the changes to the Process Control Status
	Information.

INPUTS:
	Name		Description			Units	Min	Max

	mode		Context values that will be 
			#define'd in a header file.
			Possible values are:

			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_DELETE_TEMP
			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_INTERMEDIATE_OUTPUT

	locationPCS	Pointer to the actual locations
			of PCS data.

	PCS_data	The information to be written 
			out to the Process Control
			File.  If a file is to be deleted 
			from the Process Control
			Information file then a value of 
			type PGSt_PC_Logical will need to 
			be passed in and type cast as a 
			void *.  If a temporary file is 
			to be added to the Process Control 
			Information file then the address 
			of a PGSt_PC_File_Struct will need 
			to be passed in and type caast as 
			a void *.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_FILE_NOT_ON_DISK    file not on disk but listed in PCF

EXAMPLES:

C:
	PGSt_SMF_status		returnStatus;
	FILE			*locationPCS[NUMFILES]
	PGSt_PC_File_Struct	fileStruct;

	/# fill the file structure with the proper information #/
	/# even though some of these fields are optional they must #/
	/# be initialized #/
	fileStruct.index = 101;
	strcpy(fileStruct.name,"temp.fil");
	strcpy(fileStruct.path,"/u/temp");
	strcpy(fileStruct.attribute,"MODIS-ATTRIB.fil");
	fileStruct.size = 0;
	fileStruct.bufferSize = 0;
	fileStruct.validityFlag = 0;
	fileStruct.entries = 0;
		.
		.
		.
	/# assuming the PCS files opened successfully #/

	/# put new temporary file information #/
	returnStatus = PGS_PC_PutPCSDataPutInArea(PGSd_PC_TEMPORARY_FILE,
			locationPCS,(void *) &fileStruct);
	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
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
	This tool creates writes to "temp.fil" and reads from the file 
	defined in the environment variable "PGS_PC_INFO_FILE".

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_PC_GetPCSData                Get Process Control Information
	PGS_PC_PutPCSDataAdvanceToLoc    Advance and write to temp file
	PGS_PC_PutPCSDataPutInArea       Put requested data in area
	PGS_PC_PutPCSDataInsertCheck     Insert data, checking index
	PGS_PC_PutPCSDataSkipCheck       Check index, skip line with match
	PGS_SMF_SetStaticMsg             Set static error/status message

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_PutPCSDataPutInArea(                 /* update PCS area */
    PGSt_integer        mode,               /* input mode */
    FILE	       *locationPCS[NUMFILES], /* location of PCS data */
    void               *PCS_data)           /* information to write */
{
    char                str[PGSd_PC_LINE_LENGTH_MAX]; /* string */
    int                 checkFile;          /* determine if the file exists */
    struct stat         statptr;            /* stat pointer for stat() */
    PGSt_integer        numFiles;           /* number of files */
    PGSt_SMF_status     whoCalled;          /* user that called this function */
    PGSt_SMF_status     returnStatus;       /* function return */
    PGSt_PC_Logical     logicalID;          /* logical identifier */
    long                temVal;             /* temporary hold variable */

/***************************************************************************
*    Initialize status.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Delete temporary file information from the Process Control 
*    Information file.
***************************************************************************/
    if (mode == PGSd_PC_DELETE_TEMP)
    {

/***************************************************************************
*    Put the identifier into our ID.
***************************************************************************/
        temVal = (long) PCS_data;
        logicalID = temVal;

/***************************************************************************
*    Get the file name. 
***************************************************************************/
        returnStatus = PGS_PC_GetPCSData(PGSd_PC_TEMPORARY_FILE,
                                 logicalID,str,&numFiles);
        if (returnStatus == PGS_S_SUCCESS)
        {

/***************************************************************************
*    Check all the indexes in this section, if we find a match then let's
*    skip that line.
***************************************************************************/
            returnStatus = PGS_PC_PutPCSDataSkipCheck(logicalID,
                                 locationPCS);
	    if (returnStatus == PGS_S_SUCCESS)
            {

/***************************************************************************
*    Mark a file to be removed (it will actually be removed in the 
*    program PGS_PC_TermCom) and advance to the end of the file.  While 
*    we are advancing we will write out to the temporary file.
***************************************************************************/
                returnStatus = PGS_PC_PutPCSDataAdvanceToLoc(
                                          PGSd_PC_ENDOFFILE,locationPCS);
                
                if (returnStatus == PGS_S_SUCCESS)
                {
                    checkFile = stat(str,&statptr);
                    if (checkFile != 0)
                    {
                        returnStatus = PGSPC_W_FILE_NOT_ON_DISK;
                    }
                }
            }
        }   /* end if */
    }   /* end if */

/***************************************************************************
*    A file needs to be added to the PCS information, let's take care of
*    it here.
***************************************************************************/
    else 
    {

/***************************************************************************
*    Insert the information into the file.  We need to see if some
*    file information already exists for this logical, if it does
*    then we are going to write over it, otherwise it will be a new
*    entry.
***************************************************************************/
        returnStatus = PGS_PC_PutPCSDataInsertCheck(locationPCS,
                                     (PGSt_PC_File_Struct *) PCS_data);
	if (returnStatus == PGS_S_SUCCESS)
        {

/***************************************************************************
*    Advance to the end of the file.  While we are advancing we will
*    write out to the temporary file.
***************************************************************************/
            returnStatus = PGS_PC_PutPCSDataAdvanceToLoc(
                                          PGSd_PC_ENDOFFILE,locationPCS);

        }
    }   /* end else */

/***************************************************************************
*    Update our message log and leave this function.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_PutPCSDataPutInArea()");
    }
    return returnStatus;
}
