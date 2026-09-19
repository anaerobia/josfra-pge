/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetFileFromShm.c

DESCRIPTION:
	This file contains the function PGS_PC_GetFileFromShm().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	08-Dec-94 RM Initial version
	19-Apr-95 RM Updated for TK5.  Added parameter 'records' to 
			allow traversal file multi-files for marking
			of RUNTIME.
	17-Oct-95 RM Edited function name in PGS_SMF_SetStaticMsg() as
			per bug ECSed01191.
	21-Oct-96 RM Updated for DR ECSed04042.  Added switch structure
			at end of function to only print messages to 
			log file that are level ERROR messages.
	28-Apr-97 RM Changed check for invalid file record to include 
			check of file name as well as the product id.
 
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
	Get file information from shared memory.
 
NAME:
	PGS_PC_GetFileFromShm()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetFileFromShm(
			char 			*startAddr,
			PGSt_PC_Logical		identifier,
			PGSt_PC_File_Shm	*outStruct,
			int			*records);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to get file information from shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	startAddr	pointer to address of memory
			to begin search.

	identifier	The logical identifier as defined 
			by the user.  The user's 
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

OUTPUTS:
	Name		Description			Units	Min	Max

	outStruct	requested structure.

	records		number of file records viewed
			before a match is hit 

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_EXIST      identifier does not have the data that
				    mode is requesting
	PGSPC_E_INDEX_ERROR_SHM     index value was not an integer at initialization

EXAMPLES:

C:
	#define		TRMM-3G	101

	char		*startAddr;
        char		*addrPC;
	int		records;
	PGSt_PC_HeaderStruct_Shm headStruct;
	PGSt_SMF_status	returnStatus;
	PGSt_PC_Logical	identifier;
	PGSt_PC_File_Shm outStruct;


	/# assume that the address of the PC data and #/
	/# the header structure were read in successfully #/
	
	/# get the Support Output File for TRMM-3G #/

	identifier = TRMM-3G;
	startAddr = (char *) ((unsigned long) addrPC +
				headStruct.divPtr[PGSd_PC_SUPPORT_OUTPUT];

	returnStatus = PGS_PC_GetFileFromShm(startAddr,identifier,
				&outStruct,&records);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# continue with file operations #/
		}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
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
PGS_PC_GetFileFromShm( 	                 /* get file info from shared memory */
    char             *startAddr,         /* address of start search */
    PGSt_PC_Logical   identifier,        /* logical id */
    PGSt_PC_File_Shm *outStruct,         /* requested data */
    int              *records)           /* number of files searched */

{
    char            firstChar;           /* first character hold */
    PGSt_SMF_status whoCalled;           /* user that called this function */
    PGSt_SMF_status returnStatus;        /* function return variable */

/***************************************************************************
*    Initialize return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    *records = 0;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Loop until we find an index match.
***************************************************************************/
    do
    {

/***************************************************************************
*    If our first character is a divider then we have no information.
***************************************************************************/
        memcpy(&firstChar,(char *) startAddr,sizeof(char));
        if (firstChar == PGSd_PC_DIVIDER)
        {
           returnStatus = PGSPC_W_NO_FILES_EXIST;
           break;
        }

/***************************************************************************
*    Read a structure, check the index, and increment our address 
*    pointer and records counter.
***************************************************************************/

        memcpy((void *) outStruct,(void *) startAddr,
                           sizeof(PGSt_PC_File_Shm));
        if ((outStruct->fileStruct.index == (PGSt_PC_Logical) NULL) &&
            (outStruct->fileStruct.fileName[0] == (char) PGSd_PC_CHAR_NULL))
        {
           returnStatus = PGSPC_E_INDEX_ERROR_SHM;;
           break;
        }
        startAddr = (char *) 
         ((unsigned long) startAddr + sizeof(PGSt_PC_File_Shm));

        *records = *records + 1;
    }
    while (outStruct->fileStruct.index != identifier);

/***************************************************************************
*    Update our message log and leave.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        switch (PGS_SMF_TestStatusLevel(returnStatus))
        {
            case PGS_SMF_MASK_LEV_E:
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetFileFromShm()");
                break;

            default:
                break;
        }  /* end switch */
    }

    return returnStatus;
}
