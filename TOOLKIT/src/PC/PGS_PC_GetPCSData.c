/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetPCSData.c

DESCRIPTION:
	Get PCS data.

AUTHOR:
	Ray Milburn / Applied Research Corp.
	Abe Taaheri / Space Applications Corporation

HISTORY:
	23-Mar-94 RM Initial version
        12-Apr-94 RM Added intermediate file functionality
        21-Apr-94 RM Repaired DR's
        25-Apr-94 RM Updated prolog
        02-Aug-94 RM Broke out some functionality in preparation for
			next release.
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	18-Dec-94 RM Updated for TK4 - using shared memory
	10-Jul-95 RM Added functionality to set message for NO_FILES_EXIST
			return type.  Ref. DR ECSed00096, ECSed00097, and
			ECSed00103.
	18-Jan-96 RM Work for TK6.  Removed call to PGS_SMF_SetDynamicMsg()
			due to the fact that it was causing an unnecessary
			repeat of message in the SMF log file.
	16-Oct-98 AT Added if(returnStatus != PGSPC_E_LINE_FORMAT_ERROR)
	             not to report error if returnStatus is 
		     PGSPC_E_LINE_FORMAT_ERROR. For this returnStatus error is
		     reported by PGS_PC_GetPCSDataGetRequest(). 
		     Ref. NCR ECSed18453
 
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
	Get Process Control Status information.
 
NAME:
	PGS_PC_GetPCSData()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetPCSData(
			PGSt_integer		mode,
			PGSt_PC_Logical		identifier,
			char			*outstr,
			PGSt_integer		*numFiles);

FORTRAN:
	 	include 'PGS_SMF.f'
		include 'PGS_PC.f'
		include 'PGS_PC_9.f'

		integer function pgs_pc_getpcsdata(mode,identifier,
	       *                                 outstr,numfiles)
			integer			mode
			integer			identifier
			character*200		outstr
			integer			numfiles

DESCRIPTION:
	This tool may be used to obtain information concerning the 
	availability of PGE input files, Process ID's, Science Software
	ID's, configuration values, etc.
 
INPUTS:
	Name		Description			Units	Min	Max

	mode		context values that will be 
			#define'd in a header file.
			Possible values are:

			PGSd_PC_PRODUCTION_RUN_ID
			PGSd_PC_SOFTWARE_ID
			PGSd_PC_CONFIGURATION
			PGSd_PC_INPUT_FILE_NAME
			PGSd_PC_INPUT_FILE_ATTRIBUTE
			PGSd_PC_INPUT_FILE_NUMFILES
			PGSd_PC_OUTPUT_FILE_NAME
			PGSd_PC_OUTPUT_FILE_ATTRIBUTE
			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_TEMP_INFO_USEASCII
			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_INTERMEDIATE_OUTPUT
			PGSd_PC_INTER_OUT_USEASCII
			PGSd_PC_SUPPORT_IN_NAME
			PGSd_PC_SUPPORT_IN_ATTR
			PGSd_PC_SUPPORT_OUT_NAME
			PGSd_PC_SUPPORT_OUT_ATTR

	identifier	The logical identifier as defined 
			by the user.  The user's 
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

	numFiles	this value state which version 
			of the INPUT file to receive the 
			requested information on.

OUTPUTS:
	Name		Description			Units	Min	Max

	outstr		The output string value of the 
			requested parameter.  Output
			results are:

			If mode is PGSd_PC_PRODUCTION_RUN_ID 
			outstr is a generic string.

			If mode is PGSd_PC_SOFTWARE_ID 
			outstr is a generic string.

			If mode is PGSd_PC_CONFIGURATION 
			outstr is a generic string.

			If mode is PGSd_PC_INPUT_FILE_NAME 
			outstr is a file name containing 
			the full path, numFiles will 
			contain the number of files remaining 
			for the specified identifier.

			If mode is PGSd_PC_INPUT_FILE_ATTRIBUTE 
			outstr is a file name containing 
			the file attribute.

			If mode is PGSd_PC_INPUT_FILE_NUMFILES 
			numFiles contains the the number of 
			input files associated with the 
			specified identifier.

			If mode is PGSd_PC_OUTPUT_FILE_NAME 
			outstr is a file name containing 
			the full path.

			If mode is PGSd_PC_OUTPUT_FILE_ATTRIBUTE 
			outstr is a file name containing 
			the file attribute.

			If mode is PGSd_PC_TEMPORARY_FILE 
			outstr is a file name containing 
			the full path.

			If mode is PGSd_PC_TEMP_INFO_USEASCII
			outstr is a file name containing 
			the full path.  This mode forces the 
			ASCII PCF to be used.

			If mode is PGSd_PC_INTERMEDIATE_INPUT 
			outstr is a file name containing 
			the full path.

			If mode is PGSd_PC_INTERMEDIATE_OUTPUT 
			outstr is a file name containing 
			the full path.

			If mode is PGSd_PC_INTER_OUT_USEASCII
			outstr is a file name containing 
			the full path.  This mode forces the 
			ASCII PCF to be used.

			If mode is PGSd_PC_SUPPORT_IN_NAME 
			outstr is a file name containing 
			the full path.

			If mode is PGSd_PC_SUPPORT_IN_ATTR 
			outstr is a file name containing 
			the file attribute.

			If mode is PGSd_PC_SUPPORT_OUT_ATTR 
			outstr is a file name containing 
			the file attribute.

			If mode is PGSd_PC_SUPPORT_OUT_NAME 
			outstr is a file name containing 
			the full path.

	numFiles	this value will contain the total 
			number of INPUT files available 
			for the specified identifier.  If 
			the mode value is 
			PGSd_PC_INPUT_FILE_NAME then this 
			value will be the number of files 
			remaining for the specified identifier.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_EXIST      link number does not have the data that
                                    mode is requesting
	PGSPC_W_NO_CONFIG_VALUE     incorrect number of configuration 
                                    parameters
	PGSPC_E_FILE_OPEN_ERROR     error opening input file
	PGSPC_E_INVALID_MODE        mode value does not match those in PGS_PC.h
	PGSPC_E_FILE_READ_ERROR     error reading input file
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file

EXAMPLES:
C:	
	PGSt_SMF_status	ret_value;
	PGSt_PC_Logical	prodID;
	PGSt_int	numFiles;
	char		outstr[PGSd_PC_FILE_PATH_MAX];

	/# getting the name of a file in a product group #/

	prodID = 101;
	numFiles = 1;
	ret_value = PGS_PC_GetPCSData(PGSd_PC_INPUT_FILE_NAME,prodID,
		outstr,&numFiles);

	if (ret_value != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# open the file in "outstr" #/
		}
			.
			.
			.
	EXCEPTION:
		return ret_value;

FORTRAN:
	integer	retval
	integer prodid
	integer numfiles
	integer pgs_pc_getpcsdata
	character*134 outstr

C  getting the name of a file in a product group

	prodid = 101
	numfiles = 1;

	retval = pgs_pc_getpcsdata(pgsd_pc_input_file_name,prodid,
       *                           outstr,numfiles)
	if (retval .ne. pgs_s_success) then 
		goto 9999
	else

C  now have access to file name
			.
			.
			.
	endif

9999	return

NOTES:
	All configuration parameters, except for physical file name(s), will
	have a one-to-one correspondence between a logical value and a  
	parameter value (string).

	In order for this tool to function properly, a valid Process Control
	file will need to be created first.  Please refer to Appendix C 
	(User's Guide) for instructions on how to create such a file.

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE"

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_PC_GetPCSDataOpenPCSFile     Open the PCS file
	PGS_PC_GetPCSDataLocateEntry     Get the correct line in the data file
	PGS_PC_GetPCSDataRetreiveData    Get requested information
	PGS_PC_GetDataFromShm	         Get PCS data from shared memory
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_MEM_ShmSysAddr		 Get the address of the PC data

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_GetPCSData(                           /* get PCS data */
    PGSt_integer      mode,                  /* input mode */
    PGSt_PC_Logical   identifier,            /* logical id */
    char             *outstr,                /* output string */
    PGSt_integer     *numFiles)              /* number of files */

{
    char              line[PGSd_PC_LINE_LENGTH_MAX]; /* line from file */
    char             *addrPC;                /* address of PC data in shm */
    char             *useShm;                /* SHM environment variable */
    PGSt_integer      useMode;               /* mode passed to other functions */
    PGSt_uinteger     size;                  /* amount of shm allocated for PC */
    PGSt_SMF_status   whoCalled;             /* user that called this function */
    PGSt_SMF_status   returnStatus;          /* SMF return */
    FILE             *locationPCS;           /* input file pointer */

/***************************************************************************
*    Initialize variables.     
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    locationPCS = (FILE *) NULL;

/***************************************************************************
*    Initialize caller variable.     
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Some modes force us to use the ASCII PCF, let's determine that here.
***************************************************************************/
    if ((mode == PGSd_PC_INTER_OUT_USEASCII) ||
        (mode == PGSd_PC_TEMP_INFO_USEASCII))
    {
        useShm = (char *) PGSd_PC_USESHM_NO;
        useMode = mode - 1;
    }

/***************************************************************************
*    We are using a mode that will use whatever the user wants (or whatever
*    the environment variable is set to).
***************************************************************************/
    else
    {
        useShm = getenv(PGSd_PC_USESHM_ENV);
        if (useShm == NULL)
        {
            useShm = (char *) PGSd_PC_USESHM_NO;
        }
        useMode = mode;
    }

/***************************************************************************
*    We are using shared memory.
***************************************************************************/
    if (strcmp(useShm,PGSd_PC_USESHM_YES) == 0)
    {
        returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,
                                      (void **) &addrPC,&size);

        if (returnStatus == PGS_S_SUCCESS)
        {
            returnStatus = PGS_PC_GetDataFromShm(addrPC,useMode,identifier,
                                             numFiles,outstr);
        }
    }

/***************************************************************************
*    The data is not in shared memory.
***************************************************************************/
    else
    {

/***************************************************************************
*    Open the PCS data file.
***************************************************************************/
        returnStatus = PGS_PC_GetPCSDataOpenPCSFile(&locationPCS);

/***************************************************************************
*    If all went OK then continue.
***************************************************************************/
        if (returnStatus == PGS_S_SUCCESS)
        {

/***************************************************************************
*    Get the line in the file with the requested data.
***************************************************************************/
             returnStatus = PGS_PC_GetPCSDataLocateEntry(locationPCS,useMode,
                                   identifier,numFiles,line);

/***************************************************************************
*    If all went OK again, then continue.
***************************************************************************/
             if (returnStatus == PGS_S_SUCCESS)
             {

/***************************************************************************
*    Get the actual requested data.
***************************************************************************/
                 returnStatus = PGS_PC_GetPCSDataRetrieveData(locationPCS,
                                               useMode,line,outstr,numFiles);
             }
        }  /* end if */
    }  /* end else-if */

/***************************************************************************
*    Make sure the PCF gets closed.
***************************************************************************/
    if (locationPCS != (FILE *) NULL)
    {
        fclose(locationPCS);
    }

/***************************************************************************
*    Set the message in the log and return.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        if (returnStatus != PGSPC_W_NO_FILES_EXIST)
	{
	    if(returnStatus != PGSPC_E_LINE_FORMAT_ERROR)
	    {
	      if(identifier != 10256) /* PGSd_MET_LOGICAL_XML is 
					    defined for 10256 in PGS_MET.h */
		{
		  PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetPCSData()");
		}
	    }
	}
    }
    return returnStatus;
}
