/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetReference.c

DESCRIPTION:
	This file contains the function PGS_PC_GetReference().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	05-Aug-94 RM Initial Version
	11-Oct-94 RM Update version return value per Bug ECSed00183
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
        11-Jul-95 RM Changed LONG to INT in cfortran.h macros to correct
                        "Unaligned access error" as per DR ECSed00931.
	13-Dec-95 RM Updated for TK6.  Edited documentation to reflect
			new functionality of allowing multiple instances
			of PRODUCT OUTPUT FILES.
	21-Oct-96 RM Updated for ECSed04042.  Added if check at end of
			function to set up dynamic message if the 
			reference is not found.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get file reference from logical.
 
NAME:
	PGS_PC_GetReference()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetReference(
			PGSt_PC_Logical		prodID,
			PGSt_integer		*version,
			char			*referenceID)

FORTRAN:
		include 'PGS_SMF.f'
		include 'PGS_PC.f'
		include 'PGS_PC_9.f'

		integer function pgs_pc_getreference(prodid,version,referenceid)
			integer			prodid
			integer			version
			character*200		referenceid

DESCRIPTION:
	This tool may be used to obtain a physical reference (file name or 
	universal identifier) from a logical identifier.
 
INPUTS:
	Name		Description			Units	Min	Max

	prodID		User defined constant identifier 
			that internally represents the 
			current product.

	version		Version of reference to get.  
			Remember,  for Product Input 
			files and Product Output files
			there can be a many-to-one 
			relationship.  See OUTPUTS.

OUTPUTS:
	Name		Description			Units	Min	Max

	referenceID	The actual file reference 
			returned as a string.

	version		The number of versions remaining
			for the requested Product ID.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_REFERENCE_FOUND  link number does not have the data that
                                    mode is requesting
	PGSPC_E_DATA_ACCESS_ERROR   problem while accessing PCS data

EXAMPLES:

C:
	#define		MODIS1A 2530

	PGSt_integer	version;
	char		referenceID[PGSd_PC_FILE_PATH_MAX];
	PGSt_SMF_status	returnStatus;

	/# Get first version of the file #/
	version = 1;

	returnStatus = PGS_PC_GetReference(MODIS1A,&version,referenceID);

	/# version now contains the number of versions remaining #/

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
	{ /# perform necessary operations on file #/ }
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
		IMPLICIT NONE

		integer		version
		character*355	referenceid
		integer		returnstatus
		integer		pgs_pc_getreference
		integer		modis1a
		parameter	(modis1a = 2530)

C		Get the first version of the file
		version = 1

		returnstatus = getreference(modis1a,version,referenceid)

		if (returnstatus .ne. pgs_s_success)
			goto 9999
		else
C			perform necessary operations on file
			.
			.
			.
	9999	return

NOTES:
	All reference identifier strings are guaranteed to be no greater 
	than PGSd_PC_FILE_PATH_MAX characters in length (see PGS_PC.h).

	The version returns the number of files remaining for the product
	group.  For example, if there are eight (8) versions of a file 
	when the user requests version one (1) the value seven (7) is 
	returned in version.  When the user requests version two (2) the 
	value six (6) is returned in version, etc.  Therefore, it is not 
	recommended to use version as a loop counter that is also passed 
	into PGS_PC_GetReference().

	The one-to-many relationship is only supported with Product Input
	and Product Output files.  Files listed in other sections of the 
	Process Control File are only supported on a one-to-one relationship.  
	Therefore, the variable version in not used when searching for files 
	in sections of the PCF other than the Product Input and Product Output 
	files.  If the user is knowingly searching for a file that is not in 
	the Product Input or Product Output files of the PCF the user should 
	create a version variable, set it equal to one, and pass in the address 
	of that variable.


REQUIREMENTS:  
	PGSTK-1290

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable.
	PGS_PC_GetPCSData                Get process control information.
	PGS_SMF_SetStaticMsg		 Set static error/status message.

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetReference(                    /* get physical reference */
    PGSt_PC_Logical   prodID,           /* logical value */
    PGSt_integer      *version,         /* version of reference requested */
    char             *referenceID)      /* actual reference ID string */

{
    char              buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char              msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    int fileTypes[] = {PGSd_PC_INPUT_FILE_NAME, 
                       PGSd_PC_OUTPUT_FILE_NAME,
                       PGSd_PC_SUPPORT_IN_NAME,
                       PGSd_PC_SUPPORT_OUT_NAME};  /* array of file types */
    int count;                          /* for-loop counter */
    int total;                          /* number of types of files */
    PGSt_integer      numFiles;         /* number of files in product group */
    PGSt_SMF_status   whoCalled;        /* user that called this function */
    PGSt_SMF_status   returnStatus;     /* function return value */

/****************************************************************************
*    Initialize variables.
****************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    total = sizeof(fileTypes) / sizeof(int); 
    numFiles = *version;

/****************************************************************************
*    Initialize caller variable.
****************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/****************************************************************************
*    Loop the number of types of files times and check for a file each
*    time.
****************************************************************************/
    for (count = 0; count < total; count++)
    {
         returnStatus = PGS_PC_GetPCSData(fileTypes[count],prodID,
                                          referenceID,&numFiles);

/****************************************************************************
*    If the return is other than NO FILES EXIST then we want out of this
*    loop for several reasons.  One, there may be an error somewhere and
*    we do not want to continue with a problem.  Two, we may have found a
*    match and a logical can only be used once in a file reference 
*    (although we can have a one-to-many relationship with Product Input 
*    and Product Output files).  This is important, a logical can only be 
*    used once, except in the case mentioned above.  Even when there is a 
*    one-to-many relationship among Product Input and Product Output files, 
*    they must be listed consecutively in the Product Input file or Product 
*    Output file section of the Process Control information file.
****************************************************************************/
         if (returnStatus != PGSPC_W_NO_FILES_EXIST)
         {
             break;
         }
    }  /* end for */

/****************************************************************************
*    Let's determine what type of return we have at this point.  If we 
*    have success, just go ahead and return success.  We will update the
*    version here.
****************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {
        if (numFiles <= 0)
        {
            *version = 0;
        }
        else
        {
            *version = numFiles - 1;
        }
    }

/****************************************************************************
*    If there were no files around that matched, then let the user know
*    that there is no reference to pass back.    
****************************************************************************/
    else if ((returnStatus == PGSPC_W_NO_FILES_EXIST) ||
             (returnStatus == PGSPC_W_INVALID_VERSION))
    {
        returnStatus = PGSPC_W_NO_REFERENCE_FOUND;
    }

/****************************************************************************
*    There was some type of error.  We do not need to give any specifics
*    to the user, just let them know that we had a problem accessing the 
*    data.
****************************************************************************/
    else
    {
        returnStatus = PGSPC_E_DATA_ACCESS_ERROR;
    }

/****************************************************************************
*    Update message log and return.
****************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        if (returnStatus != PGSPC_W_NO_REFERENCE_FOUND)
        {
            PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetReference()");
        }
        else
        {
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            sprintf(buf,msg,prodID,*version);
            PGS_SMF_SetDynamicMsg(returnStatus,buf,"PGS_PC_GetReference()");
        }
    }
    return returnStatus;
}
