/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_DeleteFileShm.c

DESCRIPTION:
	This file contains the function PGS_PC_DeleteFileShm().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	14-Dec-94 RM Initial version
	19-Apr-95 RM Updated for TK5.  Added checks to determine if file
			is marked for RUNTIME or RUNTIME and DELETE.
 
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
	Mark a file as deleted in shared memory.
 
NAME:
	PGS_PC_DeleteFileShm()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_DeleteFileShm(
			char 			*sectionLoc,
			PGSt_PC_Logical		logicalID);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to mark files as deleted in shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	sectionLoc	pointer to address of memory
			to begin search.

	logicalID	The logical identifier as defined 
			by the user.  The user's 
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

OUTPUTS:
	Name		Description			Units	Min	Max

	sectionLoc	pointer to address of file data
			in shared memory.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_EXIST      no file data exists for this logical id
	PGSPC_E_INDEX_ERROR_SHM     index value was not an integer at initialization
	PGSPC_E_INV_DIVPOINTER_SHM  address of division pointer does not point to
				    a divider

EXAMPLES:

C:
	#define		MODIS1A	101

	char		*sectionLoc;
	char 		*addrPC;      /# address of PC data in Shm #/
	PGSt_PC_HeaderStruct_Shm headStruct;
	PGSt_SMF_status	returnStatus;
	PGSt_PC_Logical	identifier;

	/# assuming the address in shared memory of the PC #/
	/# was successfully retrieved and that the #/
	/# header structure was read in from shared #/
	/# memory successfully #/

	/# set the section locator to point to a file type #/
	/# in this example we are using Product Input #/

	sectionLoc = (char *) ((unsigned long) addrPC + 
                           headStruct.divPtr[PGSd_PC_INPUT_FILES]);

	identifier = MODIS1A;

	/# get the first input file of identifier 101 #/

	returnStatus = PGS_PC_DeleteFileShm(sectionLoc,identifier);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# file marked as deleted - continue with normal operations #/
		}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	Currently, only Temporary Files are eligible to be marked for deletion.  
	This will probably never change, but the PC Tools have been written to 
	handle the marking of deletion for any type of file.

	All configuration parameters, except for physical file name(s), will
	have a one-to-one correspondence between a logical value and a  
	parameter value (string).

	In order for this tool to function properly, a valid Process Control
	file will need to be created first.  Please refer to Appendix C 
	(User's Guide) for instructions on how to create such a file.  This
	file must be processed through the PC initiazation program 
	PGS_PC_InitCom.

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_SMF_SetStaticMsg             Set static error/status message

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_DeleteFileShm( 	                 /* mark file as delete in Shm */
    char             *sectionLoc,        /* address of start search */
    PGSt_PC_Logical   logicalID)         /* logical id */

{
    char            firstChar;           /* first character hold */
    PGSt_PC_File_Shm fileShm;            /* SHM file structure */
    PGSt_SMF_status whoCalled;           /* user that called this function */
    PGSt_SMF_status returnStatus;        /* function return variable */

/***************************************************************************
*    Initialize return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Determine if the first character really is a divider.
***************************************************************************/
    memcpy(&firstChar,(char *) sectionLoc,sizeof(char));
    if (firstChar == PGSd_PC_DIVIDER)
    {
        sectionLoc = (char *) ((unsigned long) sectionLoc + sizeof(char));

/***************************************************************************
*    Loop until we find a file that has the same logical ID and does not
*    have the delete flag or the delete and runtime flag already set.
***************************************************************************/
        do
        {

/***************************************************************************
*    Check to see if we have hit the next divider yet.
***************************************************************************/
            memcpy(&firstChar,(char *) sectionLoc,sizeof(char));
            if (firstChar == PGSd_PC_DIVIDER)
            {
               returnStatus = PGSPC_W_NO_FILES_EXIST;
               break;
            }

/***************************************************************************
*    Get the structure, make sure the index value is a valid entry and 
*    increment the address.
***************************************************************************/
            memcpy((void *) &fileShm,(void *) sectionLoc,
                               sizeof(PGSt_PC_File_Shm));

            if (fileShm.fileStruct.index == (PGSt_PC_Logical) NULL)
            {
               returnStatus = PGSPC_E_INDEX_ERROR_SHM;;
               break;
            }
            sectionLoc = (char *) 
             ((unsigned long) sectionLoc + sizeof(PGSt_PC_File_Shm));
        }
        while ((fileShm.fileStruct.index != logicalID) ||
               (fileShm.flag == PGSd_PC_DELETE_FLAG) ||
               (fileShm.flag == PGSd_PC_DELNRUN_FLAG));
    }  /* end if */

/***************************************************************************
*    Our division pointer was not a divider.
***************************************************************************/
    else
    {
        returnStatus = PGSPC_E_INV_DIVPOINTER_SHM;
    }

/***************************************************************************
*    We found the right file, set the flag to delete, decrement our 
*    address, and write it back out to shared memory.  If the file
*    is already marked as a RUNTIME file then set it to both DELETE
*    and RUNTIME.
***************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {
        if (fileShm.flag == PGSd_PC_RUNTIME_FLAG)
        {
            fileShm.flag = PGSd_PC_DELNRUN_FLAG;
        }
        else
        {
            fileShm.flag = PGSd_PC_DELETE_FLAG;
        }
        sectionLoc = (char *) 
         ((unsigned long) sectionLoc - sizeof(PGSt_PC_File_Shm));
        memcpy(sectionLoc,(char *) &fileShm,sizeof(PGSt_PC_File_Shm));
    }

/***************************************************************************
*    Set our message in the message log and return to the calling o
*    function.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_DeleteFileShm()");
    }

    return returnStatus;
}
