/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_WriteNewToShm.c

DESCRIPTION:
	This file contains the function PGS_PC_WriteNewToShm().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	14-Dec-94 RM Initial version
	20-Apr-95 RM Updated for TK5.  Added check for DELETE and RUNTIME
			flags.
        05-Jan-96 RM Updated for TK6.  Added Universal References to
                        file entries for files.  Added strcpy() for
			universalRef item of file struct.  This replaced
			the unused bufferSize.
 
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
	Search shared memory for the requested data and overwrite if it
	exists, otherwise insert as new.
 
NAME:
	PGS_PC_WriteNewToShm()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_WriteNewToShm(
			PGSt_PC_HeaderStruct_Shm *headPtr,
			char 			*sectionLoc,
			PGSt_PC_File_Struct	*fileStructPtr,
			PGSt_uinteger		size);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to update file information in shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	headPtr		pointer to head structure read
			from beginning of shared memory.

	sectionLoc	pointer to address of memory
			to begin search.

	fileStructPtr	The file information loaded into
			the structure.

	size		amount of shared memory 
			allocated for the Process Control 
			Tools.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE	

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_INV_DIVPOINTER_SHM  address of division pointer does not 
                                    point to divider
	PGSPC_W_NO_FILES_EXIST      no file data exists for this logical id
	PGSPC_E_INDEX_ERROR_SHM     index value was not an integer at 
                                    initialization
	PGSPC_E_EXCEEDED_SHM        write operation would cause bounds of
                                    requested shared memory to be exceeded

EXAMPLES:

C:
	#define		CERES-3A 29110

	char		*sectionLoc;
	char		*addrPC;
	PGSt_uinteger	size;
	PGSt_SMF_status	returnStatus;
	PGSt_HeaderStruct_Shm headStruct;
	PGSt_PC_File_Struct fileStruct;

	/# assuming that the address in shared memory of the #/
	/# PC data was correctly returned and that the header #/
	/# structure has been read in successfully #/

	/# set section locator to address of the temporary
	/# files section #/

	sectionLoc = (char *) ((unsigned long) addrPC + 
	                           headStruct.divPtr[PGSd_PC_TEMP_INFO]);

	/# populate file structure with file information #/

	fileStruct.index = CERES-3A;
	fileStruct.size = 0;
	fileStruct.entries = 1;
	strcpy(fileStruct.universalRef,"REFERENCE-1A");
	strcpy(fileStruct.attributeLoc,"CERES-3A.attributes");
	strcpy(fileStruct.fileName,"CERES-3A.data");
	strcpy(fileStruct.path,"~/runtime/data/CERES");

	returnStatus = PGS_PC_WriteNewToShm(&headStruct,sectionLoc,
	                                             &fileStruct,size);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# file information can now be received using PC tools #/
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
PGS_PC_WriteNewToShm( 	                 /* write file info to Shm */
    PGSt_PC_HeaderStruct_Shm *headPtr,   /* pointer to header struct */
    char             *sectionLoc,        /* address of start search */
    PGSt_PC_File_Struct *fileStructPtr,  /* file information */
    PGSt_uinteger     size,              /* amount of shm for PC Tools */   
    PGSt_uinteger     offSet,            /* offset from beginnig of shm */
    char             *addrPC)            /* beginning of PC shm */

{
    char            firstChar;           /* first character hold */
    char            *writeTo;            /* address to write to on adding */
    char            temChar;             /* temporary hold character */
    int             index;               /* divPtr array index */
    int             dividerCount;        /* number of dividers counted */
    int	            holdStruct;          /* how many structures to be written */
    PGSt_PC_File_Shm *tempStruct;        /* SHM file structure */
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
*   Ensure that we are actually pointing at a divider.
***************************************************************************/
    memcpy(&firstChar,(char *) sectionLoc,sizeof(char));
    if (firstChar != PGSd_PC_DIVIDER)
    {
       returnStatus = PGSPC_E_INV_DIVPOINTER_SHM;
    }
    else
    {

/***************************************************************************
*   We know that we were pointing at a divider, increment our address
*   pointer.
***************************************************************************/
        sectionLoc = (char *) ((unsigned long) sectionLoc + sizeof(char));

/***************************************************************************
*   Loop until we find a file that has the same index but is not flagged
*   for deletion OR we hit the next section, which means the file info 	
*   is new to this section.
***************************************************************************/
        do
        {

/***************************************************************************
*   Check to see if we are at the next section.
***************************************************************************/
            memcpy(&firstChar,(char *) sectionLoc,sizeof(char));
            if (firstChar == PGSd_PC_DIVIDER)
            {
               returnStatus = PGSPC_W_NO_FILES_EXIST;
               break;
            }
    
/***************************************************************************
*   Get the next structure, check the index and update the address 
*   pointer.
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
        while ((fileShm.fileStruct.index != fileStructPtr->index) || 
               (fileShm.flag == PGSd_PC_DELETE_FLAG) ||
               (fileShm.flag == PGSd_PC_DELNRUN_FLAG));

/***************************************************************************
*   If our return status is still a success then we have a match.  The
*   structure that we just read in is going to be over written.  Load
*   our passed in structure into a shared memory structure.
***************************************************************************/
        if (returnStatus == PGS_S_SUCCESS)
        {
            fileShm.flag = PGSd_PC_CHAR_NULL;
            fileShm.fileStruct.index = fileStructPtr->index;
            fileShm.fileStruct.size = fileStructPtr->size;
            fileShm.fileStruct.entries = fileStructPtr->entries;
            strcpy(fileShm.fileStruct.universalRef,fileStructPtr->universalRef);
            strcpy(fileShm.fileStruct.fileName,fileStructPtr->fileName);
            strcpy(fileShm.fileStruct.attributeLoc,fileStructPtr->attributeLoc);
            strcpy(fileShm.fileStruct.path,fileStructPtr->path);

/***************************************************************************
*   Move the pointer back to the beginning of the structure and write 
*   out what we just loaded into our shared memory structure.
***************************************************************************/
            sectionLoc = (char *) 
             ((unsigned long) sectionLoc - sizeof(PGSt_PC_File_Shm));
            memcpy(sectionLoc,(char *) &fileShm,sizeof(PGSt_PC_File_Shm));
            returnStatus = PGS_S_SUCCESS;
        }

/***************************************************************************
*   If no files exist then we need to add our passed in structure into
*   shared memory.  This is going to be tricky so pay close attention.
***************************************************************************/
        else if (returnStatus == PGSPC_W_NO_FILES_EXIST)
        {

/***************************************************************************
*   Make sure we have the room.
***************************************************************************/
            if (size > (headPtr->amountUsed + sizeof(PGSt_PC_File_Shm)))
            {

/***************************************************************************
*   Load the data into the shared memory structure.
***************************************************************************/
                fileShm.flag = PGSd_PC_CHAR_NULL;
                fileShm.fileStruct.index = fileStructPtr->index;
                fileShm.fileStruct.size = fileStructPtr->size;
                fileShm.fileStruct.entries = fileStructPtr->entries;
                strcpy(fileShm.fileStruct.universalRef,fileStructPtr->universalRef);
                strcpy(fileShm.fileStruct.fileName,fileStructPtr->fileName);
                strcpy(fileShm.fileStruct.attributeLoc,fileStructPtr->attributeLoc);
                strcpy(fileShm.fileStruct.path,fileStructPtr->path);
                tempStruct = &fileShm;
    
/***************************************************************************
*   Set some variables (to keep track of where we are and if we need
*   to write anything).
***************************************************************************/
                writeTo = sectionLoc;
                index = 0;
                dividerCount = 1;
                holdStruct = 1;

/***************************************************************************
*   Find out which section we were in.
***************************************************************************/
                while (headPtr->divPtr[index] != offSet)
                {
                    index++;
                }

/***************************************************************************
*   Loop until we are at the end of the data (the last divider).
***************************************************************************/
                while (index < (PGSd_PC_TOTAL_SEPARATORS - 1))
                {

/***************************************************************************
*   If the next character in memory is a divider then increment our
*   divider counter and our read address and increase the division
*   pointer by the size of what will be written.
***************************************************************************/
                    memcpy(&firstChar,(char *) sectionLoc,sizeof(char));
                    if (firstChar == PGSd_PC_DIVIDER)
                    {
                        dividerCount++;
                        sectionLoc = (char *) 
                               ((unsigned long) sectionLoc + sizeof(char));
                        index++;
                        headPtr->divPtr[index] += sizeof(PGSt_PC_File_Shm);
                    }

/***************************************************************************
*   Read the next structure and increment our hold struct flag and our
*   read from address.
***************************************************************************/
                    else
                    {
                        memcpy((void *) &fileShm,(void *) sectionLoc,
                                   sizeof(PGSt_PC_File_Shm));
                        sectionLoc = (char *) 
                               ((unsigned long) sectionLoc + sizeof(PGSt_PC_File_Shm));
                        holdStruct++;

/***************************************************************************
*   If our hold struct flag is greater than one then we know we should 
*   write out a structure (we know our hold struct is always going to be
*   one because we came into this function with one structure).
***************************************************************************/
                        if (holdStruct > 1)
                        {
                            memcpy(writeTo,(char *) tempStruct,sizeof(PGSt_PC_File_Shm));
                            writeTo = (char *) 
                               ((unsigned long) writeTo + sizeof(PGSt_PC_File_Shm));
                            holdStruct--;
                        }

/***************************************************************************
*   If our divider count is greater than one we know we need to write out
*   a divider (we know our divider count will always be one since the 
*   reason we are in this loop is because we read a divider and did not
*   find an index match).
***************************************************************************/
                        if (dividerCount > 1)
                        {
                            temChar = PGSd_PC_DIVIDER;
                            memcpy((void *) writeTo,(void *) &temChar,
                                             sizeof(char));
                            writeTo = (char *) 
                               ((unsigned long) writeTo + sizeof(char));
                            dividerCount--;
                        }

/***************************************************************************
*   Load the structure that was just read in into our temporary holding
*   (cell) structure.
***************************************************************************/
                        tempStruct = &fileShm;
                    }  /* end else */
                            
                }  /* end while */

/***************************************************************************
*   We may have to write out a structure here.
***************************************************************************/
                if (holdStruct > 0)
                {
                    memcpy(writeTo,(char *) tempStruct,sizeof(PGSt_PC_File_Shm));
                    writeTo = (char *) 
                       ((unsigned long) writeTo + sizeof(PGSt_PC_File_Shm));
                }

/***************************************************************************
*   Finish writing out the last divider(s).
***************************************************************************/
                while (dividerCount > 0)
                {
                    temChar = PGSd_PC_DIVIDER;
                    memcpy(writeTo,(void *) &temChar,sizeof(char));
                    writeTo = (char *) ((unsigned long) writeTo + sizeof(char));
                    dividerCount--;
                }

/***************************************************************************
*   Update our header structure and write it out to shared memory.
***************************************************************************/
                headPtr->amountUsed += sizeof(PGSt_PC_File_Shm);
                memcpy(addrPC,(char *) headPtr,sizeof(PGSt_PC_HeaderStruct_Shm));
                returnStatus = PGS_S_SUCCESS;
         
            } /* end if (size check) */

/***************************************************************************
*   Not enough memory to do this.
***************************************************************************/
            else
            {
                returnStatus = PGSPC_E_EXCEEDED_SHM;
            }
        }  /* end else if */
    }  /* end else */
            
/***************************************************************************
*    Update our message log and leave this function.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_WriteNewToShm()");
    }
    return returnStatus;
}
