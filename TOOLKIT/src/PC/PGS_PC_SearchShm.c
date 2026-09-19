/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_SearchShm.c

DESCRIPTION:
	This file contains the function PGS_PC_SearchShm().

AUTHOR:
	Ray Milburn / Applied Research Corp.
	Abe Taaheri / SM&A Corp.
HISTORY:
	08-Dec-94 RM Initial version
	12-Jan-94 RM Correcting DR ECSed00548 - changed function name
			in PGS_SMF_SetStaticMsg() call to this function.
			- cleaned up some documentation problems.
	05-Apr-95 RM Updated for TK5.  Added functionality to retrieve
			default file location from header structure
			in shared memory.
	06-Apr-95 RM Fixing error.  Incorrect environment variable was
			being passed for Product Input file name and
			attribute retrieval.
	19-Apr-95 RM Updated for TK5.  Added functionality to allow 
			check of DELETE/RUNTIME flags in Temporary
			file sections.  Also, modified call to 
			PGS_PC_GetFileFromShm() to pass in number
			of records (to update pointer in case of
			INPUT files).
	13-Dec-95 RM Updated for TK6.  Added functionality to allow
			multiple instances of PRODUCT OUTPUT FILES.
        05-Jan-96 RM Updated for TK6.  Added Universal References to
                        file entries for files.
	21-Oct-96 RM Updated for DR ECSed04042.  Removed call to 
			PGS_SMF_SetStaticMsg() at end of function.  This
			was causing too many messages to be written to
			the log file.
	24-Apr-97 RM Added functionality to allow multiple instance of
			SUPPORT INPUT and SUPPORT OUTPUT FILES.
        05-Jan-99 AT Added few lines of code in the  
                        "Temporary file - file name." section so that when
                        file is found (i.e. returnStatus = PGS_S_SUCCESS) and 
                        it has PGSd_PC_DELETE_FLAG or PGSd_PC_DELNRUN_FLAG 
                        flags (i.e. the file is as is deleted and does not 
                        exist) we update memTracker and continue our search.
                        (for NCR ECSed19619)
        15-June-99 AT Added one line of code for the case 
	                PGSd_PC_CONFIGURATION to initialize the loopCount.
			Uninitialized value may result in a bad value for 
			loopCount (a few lines later when the loopCount is
			incremented). This in turm may result in core dump.
			(for NCR ECSed23036)
 
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
	Search shared memory for the requested data.
 
NAME:
	PGS_PC_SearchShm()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_SearchShm(
			char 			*sectionLoc,
			PGSt_integer		mode,
			PGSt_PC_Logical		identifier,
			PGSt_integer		*numFiles,
			char			*outData);

FORTRAN:
	NONE

DESCRIPTION:
	This tool is used to search shared memory for the proper PCS
	data.
 
INPUTS:
	Name		Description			Units	Min	Max

	sectionLoc	pointer to address of section
			in shared memory.

	mode		context values that will be 
			#define'd in a header file.
			Possible values are:

			PGSd_PC_PRODUCTION_RUN_ID
			PGSd_PC_SOFTWARE_ID
			PGSd_PC_CONFIGURATION
			PGSd_PC_INPUT_FILE_NAME
			PGSd_PC_INPUT_FILE_ATTRIBUTE
			PGSd_PC_INPUT_FILE_NUMFILES
			PGSd_PC_PRODUCT_IN_DEFLOC
			PGSd_PC_PRODUCT_IN_UREF
			PGSd_PC_OUTPUT_FILE_NAME
			PGSd_PC_OUTPUT_FILE_ATTRIBUTE
			PGSd_PC_OUTPUT_FILE_NUMFILES
			PGSd_PC_PRODUCT_OUT_DEFLOC
			PGSd_PC_PRODUCT_OUT_UREF
			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_TEMP_FILE_DEFLOC
			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_INTER_IN_DEFLOC
			PGSd_PC_INTER_IN_UREF
			PGSd_PC_INTERMEDIATE_OUTPUT
			PGSd_PC_INTER_OUT_DEFLOC
			PGSd_PC_INTER_OUT_UREF
			PGSd_PC_SUPPORT_IN_NAME
			PGSd_PC_SUPPORT_IN_ATTR
			PGSd_PC_SUPPORT_IN_DEFLOC
			PGSd_PC_SUPPORT_IN_UREF
			PGSd_PC_SUPPORT_IN_NUMFILES
			PGSd_PC_SUPPORT_OUT_NAME
			PGSd_PC_SUPPORT_OUT_ATTR
			PGSd_PC_SUPPORT_OUT_DEFLOC
			PGSd_PC_SUPPORT_OUT_UREF
			PGSd_PC_SUPPORT_OUT_NUMFILES

	identifier	The logical identifier as defined 
			by the user.  The user's 
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

	numFiles	this value state which version 
			of the INPUT or OUTPUT file to 
			receive the requested information 
			on.

OUTPUTS:
	Name		Description			Units	Min	Max

	outData		reqeusted data.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_CONFIG_VALUE	    incorrect number of configuration 
				    parameters
	PGSPC_E_INDEX_ERROR_SHM     index value was not an integer at 
                                    initialization
	PGSPC_W_NULL_DATA_SHM	    data for this request was not present
                                    at initialization
	PGSPC_E_INV_DIVPOINTER_SHM  address of division pointer does not 
                                    point to a divider
        PGSPC_E_NO_DEFAULT_LOC      no default file location specified

EXAMPLES:

C:
	char		*sectionLoc;
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

	returnStatus = PGS_PC_SearchShm(sectionLoc,mode,identifier,
		numFiles,outData);

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
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_PC_BuildFileShm              Build a file name from structure
                                         read from shared memory
	PGS_PC_GetFileFromShm            Get file information from shared 
                                         memory
        PGS_PC_CalcArrayIndex            Get index in default location
					 array


END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_SearchShm( 	                 /* get data from shared memory */
    char             *sectionLoc,        /* address of PC data */
    PGSt_integer      mode,              /* mode requested */
    PGSt_PC_Logical   identifier,        /* logical id */
    PGSt_integer     *numFiles,          /* number of files */
    char             *outData)           /* requested data */

{
    char            firstChar;           /* first character of data */
    char            *memTracker;         /* memory pointer tracker */
    int             loopCount;           /* loop counter variable */
    int             counter;             /* loop counter variable */
    int             pos;                 /* line position */
    int             index;               /* arrray index */
    int             records;             /* number of files searched */
    PGSt_boolean    found;               /* found flag */
    PGSt_PC_SysConfig_Shm sysConfig;     /* system configuration structure */
    PGSt_PC_UserConfig_Shm userConfig;   /* user configuration structure */
    PGSt_PC_HeaderStruct_Shm headStruct; /* header */
    PGSt_PC_File_Shm fileShm;            /* file structure */
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
*    Determine if the pointer is pointing at a divider.
***************************************************************************/
    memcpy(&firstChar,(char *) sectionLoc,sizeof(char));

    if (firstChar == PGSd_PC_DIVIDER)
    {

/***************************************************************************
*    We know we pointed at a divider, increment our address accordingly.
***************************************************************************/
        memTracker = (char *) ((unsigned long) sectionLoc + sizeof(char));

/***************************************************************************
*    Let's switch on our mode.
***************************************************************************/
        switch (mode)
        {

/***************************************************************************
*    System configuration parameters.
***************************************************************************/
            case PGSd_PC_PRODUCTION_RUN_ID:
            case PGSd_PC_SOFTWARE_ID:
                loopCount = 0;
                do
                {
                    memcpy(&firstChar,(char *) memTracker,sizeof(char));
                    if (firstChar == PGSd_PC_DIVIDER)
                    {
                       returnStatus = PGSPC_W_NO_CONFIG_VALUE;
                       break;
                    }

                    memcpy((void *) &sysConfig,(void *) memTracker,
                                       sizeof(PGSt_PC_SysConfig_Shm));
                    memTracker = (char *) 
                     ((unsigned long) memTracker + sizeof(PGSt_PC_SysConfig_Shm));
                    loopCount++;
                }
                while (loopCount < mode);
                if (returnStatus == PGS_S_SUCCESS)
                {
                    pos = 0;
                    while (sysConfig.data[pos] != PGSd_PC_NEWLINE)
                    {
                        outData[pos] = sysConfig.data[pos];
                        pos++;
                    }
                    outData[pos] = '\0';
                }
                break;

/***************************************************************************
*    User defined configuration parameters.
***************************************************************************/
            case PGSd_PC_CONFIGURATION:
                loopCount = 0;
                do
                {
                    memcpy(&firstChar,(char *) memTracker,sizeof(char));
                    if (firstChar == PGSd_PC_DIVIDER)
                    {
                       returnStatus = PGSPC_W_NO_CONFIG_VALUE;
                       break;
                    }

                    memcpy((void *) &userConfig,(void *) memTracker,
                                       sizeof(PGSt_PC_UserConfig_Shm));
                    if (userConfig.configStruct.index == (PGSt_PC_Logical) NULL)
                    {
                       returnStatus = PGSPC_E_INDEX_ERROR_SHM;
                       break;
                    }
                    memTracker = (char *) 
                     ((unsigned long) memTracker + sizeof(PGSt_PC_UserConfig_Shm));
                    loopCount++;
                }
                while (userConfig.configStruct.index != identifier);
                if (returnStatus == PGS_S_SUCCESS)
                {
                    if (userConfig.configStruct.value[0] != PGSd_PC_CHAR_NULL)
                    {
                        strcpy(outData,userConfig.configStruct.value);
                    }
                    else
                    {
                        returnStatus = PGSPC_W_NULL_DATA_SHM;
                    }
                }
                break;

/***************************************************************************
*    Input file - file name.
***************************************************************************/
            case PGSd_PC_INPUT_FILE_NAME:
                for (loopCount = 0; loopCount < *numFiles; loopCount++)
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                               identifier,&fileShm,&records);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                       break;
                    }

                    for (counter = 0; counter < records; counter++)
                    {
                        memTracker = (char *) 
                         ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
                    }
                }

                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_INPUT_FILE_ENVIRONMENT,PGSd_PC_TYPE_FILE,
                                 &fileShm,outData);

                    *numFiles = fileShm.fileStruct.entries;
                }
                break;

/***************************************************************************
*    Input file - attribute file name.
***************************************************************************/
            case PGSd_PC_INPUT_FILE_ATTRIBUTE:
                for (loopCount = 0; loopCount < *numFiles; loopCount++)
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                                identifier,&fileShm,&records);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                       break;
                    }

                    for (counter = 0; counter < records; counter++)
                    {
                        memTracker = (char *) 
                         ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
                    }
                }

                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_INPUT_FILE_ENVIRONMENT,PGSd_PC_TYPE_ATTR,
                                 &fileShm,outData);
                    *numFiles = fileShm.fileStruct.entries;
                }
                break;

/***************************************************************************
*    Output file - file name.
***************************************************************************/
            case PGSd_PC_OUTPUT_FILE_NAME:
                for (loopCount = 0; loopCount < *numFiles; loopCount++)
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                               identifier,&fileShm,&records);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                       break;
                    }

                    for (counter = 0; counter < records; counter++)
                    {
                        memTracker = (char *) 
                         ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
                    }
                }

                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_OUTPUT_FILE_ENVIRONMENT,PGSd_PC_TYPE_FILE,
                                 &fileShm,outData);
                    *numFiles = fileShm.fileStruct.entries;
                }
                break;

/***************************************************************************
*    Output file - attribute file name.
***************************************************************************/
            case PGSd_PC_OUTPUT_FILE_ATTRIBUTE:
                for (loopCount = 0; loopCount < *numFiles; loopCount++)
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                               identifier,&fileShm,&records);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                       break;
                    }

                    for (counter = 0; counter < records; counter++)
                    {
                        memTracker = (char *) 
                         ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
                    }
                }
                
                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_OUTPUT_FILE_ENVIRONMENT,PGSd_PC_TYPE_ATTR,
                                 &fileShm,outData);
                    *numFiles = fileShm.fileStruct.entries;
                }
                break;

/***************************************************************************
*    Intermediate input file - file name.
***************************************************************************/
            case PGSd_PC_INTERMEDIATE_INPUT:
                returnStatus = PGS_PC_GetFileFromShm(memTracker,identifier,
                                                         &fileShm,&records);
                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_INTER_INPUT_ENVIRONMENT,PGSd_PC_TYPE_FILE,
                                 &fileShm,outData);
                }
                break;

/***************************************************************************
*    Intermediate output - file name.
***************************************************************************/
            case PGSd_PC_INTERMEDIATE_OUTPUT:
                returnStatus = PGS_PC_GetFileFromShm(memTracker,identifier,
                                                          &fileShm,&records);
                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_INTER_OUTPUT_ENVIRONMENT,PGSd_PC_TYPE_FILE,
                                 &fileShm,outData);
                }
                break;

/***************************************************************************
*    Support input - file name.
***************************************************************************/
            case PGSd_PC_SUPPORT_IN_NAME:
                for (loopCount = 0; loopCount < *numFiles; loopCount++)
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                               identifier,&fileShm,&records);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                       break;
                    }

                    for (counter = 0; counter < records; counter++)
                    {
                        memTracker = (char *) 
                         ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
                    }
                }

                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_SUPPT_INPUT_ENVIRONMENT,PGSd_PC_TYPE_FILE,
                                 &fileShm,outData);

                    *numFiles = fileShm.fileStruct.entries;
                }
                break;

/***************************************************************************
*    Support input - attribute file name.
***************************************************************************/
            case PGSd_PC_SUPPORT_IN_ATTR:
                for (loopCount = 0; loopCount < *numFiles; loopCount++)
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                               identifier,&fileShm,&records);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                       break;
                    }

                    for (counter = 0; counter < records; counter++)
                    {
                        memTracker = (char *) 
                         ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
                    }
                }

                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_SUPPT_INPUT_ENVIRONMENT,PGSd_PC_TYPE_ATTR,
                                 &fileShm,outData);
                    *numFiles = fileShm.fileStruct.entries;
                }
                break;

/***************************************************************************
*    Support output - file name.
***************************************************************************/
            case PGSd_PC_SUPPORT_OUT_NAME:
                for (loopCount = 0; loopCount < *numFiles; loopCount++)
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                               identifier,&fileShm,&records);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                       break;
                    }

                    for (counter = 0; counter < records; counter++)
                    {
                        memTracker = (char *) 
                         ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
                    }
                }

                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_SUPPT_OUT_ENVIRONMENT,PGSd_PC_TYPE_FILE,
                                 &fileShm,outData);

                    *numFiles = fileShm.fileStruct.entries;
                }
                break;

/***************************************************************************
*    Support output - attribute file name.
***************************************************************************/
            case PGSd_PC_SUPPORT_OUT_ATTR:
                for (loopCount = 0; loopCount < *numFiles; loopCount++)
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                               identifier,&fileShm,&records);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                       break;
                    }

                    for (counter = 0; counter < records; counter++)
                    {
                        memTracker = (char *) 
                         ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
                    }
                }

                if (returnStatus == PGS_S_SUCCESS)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                                 PGSd_PC_SUPPT_INPUT_ENVIRONMENT,PGSd_PC_TYPE_ATTR,
                                 &fileShm,outData);
                    *numFiles = fileShm.fileStruct.entries;
                }
                break;

/***************************************************************************
*    Temporary file - file name.
***************************************************************************/
            case PGSd_PC_TEMPORARY_FILE:
                found = PGS_TRUE;
                do
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                               identifier,&fileShm,&records);
                    if (returnStatus == PGS_S_SUCCESS)
                    {
                        if ((fileShm.flag == PGSd_PC_DELETE_FLAG) ||
                            (fileShm.flag == PGSd_PC_DELNRUN_FLAG))
                        {

/***************************************************************************
*    If an SMF function wanted this we will give it to him, otherwise
*    it does not exist for anybody else.  We actually delete this file
*    in PGS_PC_Term.
***************************************************************************/
                           if (whoCalled == PGSd_CALLERID_SMF)
                           {
                              break;
                           }
                           else
                           {
/***************************************************************************
*    If file found (i.e. returnStatus = PGS_S_SUCCESS) and it has 
*    PGSd_PC_DELETE_FLAG or PGSd_PC_DELNRUN_FLAG flags, the file is as is 
*    deleted and does not exist. We need to update memTracker and continue 
*    our search. (for NCR ECSed19619)
*    
***************************************************************************/
			       for (counter = 0; counter < records; counter++)
			       {
				   memTracker = (char *) 
				     ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
			       }
                               continue;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                       found = PGS_FALSE;
                    }
                }
                while (found == PGS_TRUE);

                if (found == PGS_TRUE)
                {
                    returnStatus = PGS_PC_BuildFileShm(
                             PGSd_PC_TEMP_ENVIRONMENT,PGSd_PC_TYPE_FILE,
                             &fileShm,outData);
                }
                break;

/***************************************************************************
*    Number of files - many different file types.
***************************************************************************/
            case PGSd_PC_INPUT_FILE_NUMFILES:
            case PGSd_PC_OUTPUT_FILE_NUMFILES:
            case PGSd_PC_SUPPORT_IN_NUMFILES:
            case PGSd_PC_SUPPORT_OUT_NUMFILES:
                returnStatus = PGS_PC_GetFileFromShm(memTracker,identifier,
                                                          &fileShm,&records);

                if (fileShm.fileStruct.entries == (PGSt_PC_Logical) NULL)
                {
                   returnStatus = PGSPC_E_INDEX_ERROR_SHM;
                }
                else
                {
                    *numFiles = fileShm.fileStruct.entries;
                 }

                break;

/***************************************************************************
*    The user has requested default file location data.  Let's retrieve
*    the proper index value get the data and ensure that it is not
*    blank.
***************************************************************************/
            case PGSd_PC_PRODUCT_IN_DEFLOC:
            case PGSd_PC_PRODUCT_OUT_DEFLOC:
            case PGSd_PC_TEMP_FILE_DEFLOC:
            case PGSd_PC_INTER_IN_DEFLOC:
            case PGSd_PC_INTER_OUT_DEFLOC:
            case PGSd_PC_SUPPORT_IN_DEFLOC:
            case PGSd_PC_SUPPORT_OUT_DEFLOC:
                memcpy((void *) &headStruct,(void *) sectionLoc,
                                    sizeof(PGSt_PC_HeaderStruct_Shm));
                returnStatus = PGS_PC_CalcArrayIndex(mode,
                                       PGSd_PC_MODE_VALUE,&index);
                if (returnStatus == PGS_S_SUCCESS)
                {
                    if (strlen(headStruct.defaultLoc[index]) > (size_t) 0)
                    {
                        strcpy(outData,headStruct.defaultLoc[index]);
                    }
                    else
                    {
                        returnStatus = PGSPC_E_NO_DEFAULT_LOC;
                        if (whoCalled != PGSd_CALLERID_SMF)
                        {
                            PGS_SMF_SetStaticMsg(returnStatus,
                                               "PGS_PC_SearchShm()");
                        }
                    }
                }
                break;

/***************************************************************************
*    Product Input file and Product Output file universal reference.
***************************************************************************/
            case PGSd_PC_PRODUCT_IN_UREF:
            case PGSd_PC_PRODUCT_OUT_UREF:
                for (loopCount = 0; loopCount < *numFiles; loopCount++)
                {
                    returnStatus = PGS_PC_GetFileFromShm(memTracker,
                                               identifier,&fileShm,&records);
                    if (returnStatus != PGS_S_SUCCESS)
                    {
                       break;
                    }

                    for (counter = 0; counter < records; counter++)
                    {
                        memTracker = (char *) 
                         ((unsigned long) memTracker + sizeof(PGSt_PC_File_Shm));
                    }
                }

                if (returnStatus == PGS_S_SUCCESS)
                {
                    *numFiles = fileShm.fileStruct.entries;
                    if (fileShm.fileStruct.universalRef[0] != 
                                                    PGSd_PC_CHAR_NULL)
                    {
                        strcpy(outData,fileShm.fileStruct.universalRef);
                    }
                    else
                    {
                        returnStatus = PGSPC_W_NULL_DATA_SHM;
                    }
                }
                break;

/***************************************************************************
*    Intermediate Input file, Intermediate Output file, Support Input
*    file, and Support Output file universal reference.
***************************************************************************/
            case PGSd_PC_INTER_IN_UREF:
            case PGSd_PC_INTER_OUT_UREF:
            case PGSd_PC_SUPPORT_IN_UREF:
            case PGSd_PC_SUPPORT_OUT_UREF:
                returnStatus = PGS_PC_GetFileFromShm(memTracker,identifier,
                                                      &fileShm,&records);
                if (returnStatus == PGS_S_SUCCESS)
                {
                    if (fileShm.fileStruct.universalRef[0] != 
                                                    PGSd_PC_CHAR_NULL)
                    {
                        strcpy(outData,fileShm.fileStruct.universalRef);
                    }
                    else
                    {
                        returnStatus = PGSPC_W_NULL_DATA_SHM;
                        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_SearchShm()");
                    }
                }
                break;

        }  /* end switch */
    }  /* end if */

/***************************************************************************
*    Unfortunately, our address was not pointing to a divider.
***************************************************************************/
    else
    {
        returnStatus = PGSPC_E_INV_DIVPOINTER_SHM;
        if (whoCalled != PGSd_CALLERID_SMF)
        {
            PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_SearchShm()");
        }
    }

/***************************************************************************
*    Exit the function.
***************************************************************************/
    return returnStatus;
}
