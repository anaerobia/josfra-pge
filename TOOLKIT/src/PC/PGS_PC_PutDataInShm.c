/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_PutDataInShm.c

DESCRIPTION:
	This file contains the function PGS_PC_PutDataInShm().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	14-Dec-94 RM Initial version
 
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
	Put the requested PCS data into shared memory.
 
NAME:
	PGS_PC_PutDataInShm()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_PutDataInShm(
			char 			*addrPC,
			PGSt_integer		mode,
			void			*PCS_data,
			PGSt_uinteger		size);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to locate requested PCS data from shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	addrPC		pointer to address of beginning
			location of Process Control Status
			information in shared memory.

	mode		context values that will be 
			#define'd in a header file.
			Possible values are:

			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_INTERMEDIATE_OUTPUT
			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_DELETE_TEMP

        PCS_data        The information to be written
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

	size		amount of shared memory allocated
			for the Process Control Tools.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_INVALID_MODE        mode value does not match those in PGS_PC.h

EXAMPLES:

C:
	char		*addrPC;
	PGSt_uinteger	size;
	PGSt_SMF_status	returnStatus;
	PGSt_PC_Logical	identifier;
	PGSt_integer	*numFiles;
	PGSt_integer	mode;
	char		outData[PGSd_PC_LINE_LENGTH_MAX];

	/# assuming the PCS file opened without incident #/

	identifier = 101;

	/# get the first input file of identifier 101 #/

	*numFiles = 1;
	mode = PGSd_PC_INPUT_FILE_NAME;

	/# get the address in shared memory of the PC data #/
	
	returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,
				(void **) &addrPC,&size);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;

	returnStatus = PGS_PC_PutDataInShm(addrPC,mode,PCS_data);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# parse line for file name #/
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
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_PutDataInShm( 	                 /* get data from shared memory */
    char             *addrPC,            /* address of PC data */
    PGSt_integer      mode,              /* mode requested */
    void             *PCS_data,          /* PCS data to write */
    PGSt_uinteger     size)              /* amount of shm for PC Tools */

{
    char            buf[PGS_SMF_MAX_MSG_SIZE];    /* message buffer */
    char            msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char            *sectionLoc;         /* section pointer */
    PGSt_uinteger   offSet;              /* offset in memory */
    PGSt_PC_Logical logicalID;           /* ID to be deleted */
    PGSt_PC_File_Struct *fileStructPtr;  /* new file information */
    PGSt_PC_HeaderStruct_Shm headStruct; /* header in shared memory */
    PGSt_SMF_status whoCalled;           /* user that called this function */
    PGSt_SMF_status returnStatus;        /* function return variable */
    long            temVal;              /* temporary hold variable */

/***************************************************************************
*    Initialize return variable.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Get the header structure from shared memory.
***************************************************************************/
    memcpy((void *) &headStruct,(void *) addrPC,
                        sizeof(PGSt_PC_HeaderStruct_Shm));

/***************************************************************************
*    Based on the mode that is requested let's set a locator.  This will
***************************************************************************/
    switch (mode)  
    {

/***************************************************************************
*    Temporary file information.
***************************************************************************/
        case PGSd_PC_TEMPORARY_FILE:
        case PGSd_PC_DELETE_TEMP:
            offSet = headStruct.divPtr[PGSd_PC_TEMP_INFO-1];
            break;

/***************************************************************************
*    Intermediate input file information.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_INPUT:
            offSet = headStruct.divPtr[PGSd_PC_INTER_INPUT-1];
            break;

/***************************************************************************
*    Intermediate output file information.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_OUTPUT:
            offSet = headStruct.divPtr[PGSd_PC_INTER_OUTPUT-1];
            break;

/***************************************************************************
*    Unknown mode.  See PGS_PC.h for a list of modes.
***************************************************************************/
        default:
            returnStatus = PGSPC_E_INVALID_MODE;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(PGSPC_E_INVALID_MODE,msg);
                sprintf(buf,msg,mode);
                PGS_SMF_SetDynamicMsg(PGSPC_E_INVALID_MODE,buf,
                                  "PGS_PC_PutDataInShm()");
            }
            break;
    }   /* end switch */

/***************************************************************************
*    If everything has gone OK up to here then we will proceed.
***************************************************************************/
    if ((returnStatus == PGS_S_SUCCESS) && (offSet != (PGSt_uinteger) NULL))
    {

/***************************************************************************
*    Advance our PCS data location to the area that contains the section
*    of data that we are looking for.
***************************************************************************/
        sectionLoc = (char *) ((unsigned long) addrPC + offSet);

/***************************************************************************
*    If we are deleting then call the function that handles it otherwise 
*    call the function to write the new data.
***************************************************************************/
        if (mode == PGSd_PC_DELETE_TEMP)
        {
            temVal = (long) PCS_data;
            logicalID = temVal;
            returnStatus = PGS_PC_DeleteFileShm(sectionLoc,logicalID);
        }
        else
        {
            fileStructPtr = (PGSt_PC_File_Struct *) PCS_data;
            returnStatus = PGS_PC_WriteNewToShm(&headStruct,sectionLoc,
                                         fileStructPtr,size,offSet,addrPC);
        }
    }
/***************************************************************************
*    Set our message in the message log and leave.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetPCSDataLocateEntry()");
            }
            break;
    }
    return returnStatus;
}
