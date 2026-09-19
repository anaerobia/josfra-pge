/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetNumberOfFiles.c

DESCRIPTION:
	This file contains the function PGS_PC_GetNumberOfFiles().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	05-Aug-94 RM Initial Version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
        11-Jul-95 RM Changed LONG to INT in cfortran.h macros to correct
                        "Unaligned access error" as per DR ECSed00931.
	12-Dec-95 RM Updated for TK6.  Added functionality to allow
			multiple instances of PRODUCT OUTPUT FILES.
	24-Apr-97 RM Added functionality to allow multiple instances
			of SUPPORT INPUT and SUPPORT OUTPUT FILES.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get the number of files associated with a product.
 
NAME:
	PGS_PC_GetNumberOfFiles()

SYNOPSIS:

C:	
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetNumberOfFiles(
			PGSt_PC_Logical		prodID,
			PGSt_integer		*numFiles)

FORTRAN:	
		include 'PGS_SMF.f'
		include 'PGS_PC.f'
		include 'PGS_PC_9.f'

		integer function pgs_pc_getnumberoffiles(prodid,numfiles)
			integer			prodid,
			integer			numfiles)

DESCRIPTION:
	This tool may be used to determine the number of files that are 
	associated with a particular Product ID.  A many-to-one relationship
	may exist with PRODUCT INPUT, PRODUCT OUTPUT, SUPPORT INPUT and 
	SUPPORT OUTPUT files.  This function will give the user a way to 
	determine how many files exist for a product ID.
 
INPUTS:
	Name		Description			Units	Min	Max

	prodID		The logical identifier as 
			defined by the user.  The user's
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

OUTPUTS:
	Name		Description			Units	Min	Max

	numberOfFiles	Total number of files for a 
			particular product ID.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_FILES_FOR_ID     incorrect number of configuration 
                                    parameters
	PGSPC_E_DATA_ACCESS_ERROR   error accessing PCS data

EXAMPLES:

C:	
	#define		CERES4 7090

	PGSt_integer	numFiles;
	PGSt_integer	version;
	PGSt_SMF_status	returnStatus;
	int		loopCounter;
	char		ceresFiles[10][PGSd_PC_FILE_PATH_MAX];

	returnStatus = PGS_PC_GetNumberOfFiles(CERES4,&numFiles);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
	{

	 /# loop and get file names #/

		for (loopCounter = 0; loopCounter < numFiles; loopCounter++)
		{

		/# specify which file to get #/

			version = loopCounter + 1;

		/# save references for future use #/

			returnStatus = PGS_PC_GetReference(CERES4,&version,
						ceresFiles[loopCounter]);
		} 
	}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:	
		IMPLICIT NONE

		integer		numfiles
		integer		version
		integer		returnstatus
		integer		loopcounter
		character*355	referenceid
		character*355	ceresfiles(10)
		integer		pgs_pc_getnumberoffiles
		integer		pgs_pc_getreference
		integer		ceres4
		parameter	(ceres4 = 7090)

		returnstatus = pgs_pc_getnumberoffiles(ceres4,numfiles)

		if (returnstatus .ne. pgs_s_success)
			goto 9999
		else
			do 100 loopcounter = 1,numfiles
 				version = loopcounter
				returnstatus = pgs_pc_getreference(ceres4,
	       *                                        version,
	       *                                     ceresfiles(loopcounter))
	100		continue   
			.
			.
			.
	9999	return


NOTES:
	This function will allow a many-to-one relationship to exist between 
	logical and physical file name.  The file version number is returned
	in reverse order.  For example if there are eight (8) versions of a 
	Product ID and the user requests the first one, the value eight (8)
	would be returned in numFiles.

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
	PGS_SMF_SetStaticMsg		 Set a static message in message buffer.

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetNumberOfFiles(               /* get number of files */
    PGSt_PC_Logical   prodID,	       /* logical value */
    PGSt_integer     *numberOfFiles)   /* number of files */
{
    char              dummy[PGSd_PC_FILE_PATH_MAX]; /* dummy string */
    int               loopCount;       /* for-loop counter */
    int               fileTypes[] = {PGSd_PC_INPUT_FILE_NUMFILES,
                                     PGSd_PC_OUTPUT_FILE_NUMFILES,
                                     PGSd_PC_SUPPORT_IN_NUMFILES,
                                     PGSd_PC_SUPPORT_OUT_NUMFILES};  
                                     /* array of file types */
    int               totalTypes;      /* number of file types */
    PGSt_integer      numFiles;        /* for PGS_PC_GetPCSData() */
    PGSt_SMF_status   whoCalled;       /* user that called this function */
    PGSt_SMF_status   returnStatus;    /* function return */
    char                msg[PGS_SMF_MAX_MSG_SIZE];      /* predefined status message */
    char                newMsg[PGS_SMF_MAX_MSGBUF_SIZE];/* new status message */


/***************************************************************************
*    Initialize status.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    *numberOfFiles = 0;
    numFiles = 1;
    totalTypes = sizeof(fileTypes) / sizeof(int);

/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Loop through the different file types until either an error is
*    found, success is hit, or we run out of file types.
***************************************************************************/
    for (loopCount = 0; loopCount < totalTypes; loopCount++)
    {
        returnStatus = PGS_PC_GetPCSData(fileTypes[loopCount],prodID,
                                         dummy,&numFiles);
        if (returnStatus != PGSPC_W_NO_FILES_EXIST)
        {
            break;
        }
    }  /* end for */

/***************************************************************************
*    Let's see if everything went OK.
***************************************************************************/
    if (returnStatus != PGS_S_SUCCESS)
    {

/***************************************************************************
*    It looks like we do not have any files listed for this ID.
***************************************************************************/
        if ((returnStatus == PGSPC_W_NO_FILES_EXIST) || 
	    (returnStatus == PGSPC_W_NO_FILES_FOR_ID))
        {
            returnStatus = PGSPC_W_NO_FILES_FOR_ID;
             if (whoCalled != PGSd_CALLERID_SMF)
               {
                 PGS_SMF_GetMsgByCode(returnStatus, msg);
                 sprintf(newMsg, msg, (int)prodID);
                 PGS_SMF_SetDynamicMsg(returnStatus, newMsg,
                                         "PGS_PC_GetNumberOfFiles()");
               }
        }

/***************************************************************************
*    We have some kind of problem.  Just tell the user that we had a
*    problem accessing the data.
***************************************************************************/
        else
        {
            returnStatus = PGSPC_E_DATA_ACCESS_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
              {
                 PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetNumberOfFiles()");
              }
        }
    }

/***************************************************************************
*    Everything was a success, set the return variable to numFiles.
***************************************************************************/
    else
    {
        *numberOfFiles = numFiles;
    }

    return returnStatus;
}
