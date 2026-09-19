/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetFileByAttr.c

DESCRIPTION:
	This file contains the function PGS_PC_GetFileByAttr().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	05-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	28-Dec-94 RM Fixing memory leak - DR ECSed00384.
 	06-Jul-95 RM/MES/GTSK  Move FORTRAN bindings to PGS_PC_GetFileByAttrF.c

END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <cfortran.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get the version number of the particular file matching the attribute.
 
NAME:
	PGS_PC_GetFileByAttr()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetFileByAttr(
			PGSt_PC_Logical		prodID,
			PGSt_integer		(*searchFunc)(char *attr),
			PGSt_integer		maxSize,
			PGSt_integer		*version)

FORTRAN:	
		include 'PGS_SMF.f'
		include 'PGS_PC.f'
		include 'PGS_PC_9.f'

		integer function pgs_pc_getfilebyattr(prodid,searchfunc,
	       *		maxsize,version)
			integer			prodid
			integer			searchfunc
			integer			maxSize
			integer			version

DESCRIPTION:
	This tool may be used to retrieve the version number associated with
	a file with a particular attribute.  
 
INPUTS:
	Name		Description			Units	Min	Max

	prodID		The logical identifier as 
			defined by the user.  The user's
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

	searchFunc	A user defined function that 
			performs the search on the 
			attribute.  This function must 
			be passed in as a type 
			PGSt_integer function.  It should 
			return type PGSd_PC_MATCH upon a 
			successful attribute match or 
			PGSd_PC_NO_MATCH upon an 
			unsuccessful attribute match.

	maxSize		Maximum amount of space to place
			into attribute.

OUTPUTS:
	Name		Description			Units	Min	Max

	version		The version number of the file 
			with the successful attribute
			match.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_ATTR_MATCH       did not find a match with the specified
				    product ID
	PGSPC_W_NO_ATTR_FOR_ID      the product ID contains no attribute
	PGSPC_E_DATA_ACCESS_ERROR   error accessing PCS data

EXAMPLES:

C:	
	#define MODIS1A 5775

	PGSt_integer	searchfunc_(char *attr);   /# function prototype #/

	PGSt_integer	maxSize;
	PGSt_integer	version;
	PGSt_SMF_status	returnStatus;
	char		referenceID[PGSd_PC_FILE_PATH_MAX];

	maxSize = 300;

	returnStatus = PGS_PC_GetFileByAttr(MODIS1A,searchfunc_,maxSize,
			&version);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
	{

	 /# get file reference #/

		returnStatus = PGS_PC_GetReference(MODIS1A,version,referencID);

	}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
		IMPLICIT NONE
	
		integer		version
		integer		searchfunc

C	The function passed into pgs_pc_getfilebyattr() MUST be called searchfunc

		integer		maxsize
		integer		returnstatus
		integer		pgs_pc_getfilebyattr
		integer		pgs_pc_getreference
		character*355	referenceid
		integer		modis1a 
		parameter	(modis1a = 5775)

		maxsize = 300

		returnstatus = pgs_pc_getfilebyattribute(modis1a,
	       *		searchfunc,maxsize,version)

                if (returnstatus .ne. pgs_s_success) then
			goto 9999
		else
C		
C		get file reference
C
			returnstatus = pgs_pc_getreference(modis1a,version,
	*				referenceid)
		endif
			.
			.
			.
		return

NOTES:
	The attribute checking is left to the application programmer.  The
	attribute for comparison must be passed into searchFunc by means of
	a global variable.  The attribute to be compared against will be passed
	into searchFunc by the function PGS_PC_GetFileByAttr().  The function
	searchFunc must have declared a variable large enough to handle the
	incoming attribute.  The attribute will be read until maxSize bytes
	or EOF, which ever come first.

REQUIREMENTS:  
	PGSTK-1290

GLOBALS:
	NONE

DETAILS:
	If PGS_PC_GetFileByAttr() is being called by FORTRAN then the search 
	function MUST be called searchfunc.  This naming of the search function 
	is case sensitive.  Failing to follow these instructions will result in 
	a compile time error.  If PGS_PC_GetFileByAttr() is being called by C
	the search function may be called whatever the application programmer
	desires.

	IMPORTANT:  ANY CHANGES MADE TO THIS FUNCTION MUST ALSO BE MADE TO THE
	FUNCTION PGS_PC_GetFileByAttrF().  PGS_PC_GetFileByAttrF() IS THE 
	FORTRAN COUNTERPART TO PGS_PC_GetFileByAttr().

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_PC_GetNumberOfFiles          Get the number of files for a particular
					 product ID.
	PGS_PC_GetFileAttr		 Get the attribute of a particular file
	PGS_SMF_SetStaticMsg		 Set a static message in message buffer
	searchfunc			 user defined function

 
END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_GetFileByAttr(                  /* get file by attribute */
    PGSt_PC_Logical   prodID,	       /* logical value */
    PGSt_integer      (*searchFunc)(char *), /* user supplied function */
    PGSt_integer      maxSize,         /* max size of attribute */
    PGSt_integer     *version)         /* file version number */

{
    char             *fileAttribute;   /* attribute file name */
    int               loopCount;       /* for-loop counter */
    PGSt_integer      numFiles;        /* number of files */
    PGSt_integer      foundFlag;       /* flag for found attribute */
    PGSt_SMF_status   whoCalled;       /* user who called this function */
    PGSt_SMF_status   returnStatus;    /* function return */

/***************************************************************************
*   Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    fileAttribute = (char *) malloc(maxSize);
    foundFlag = PGSd_PC_NO_MATCH;

/***************************************************************************
*   Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*   Determine how many files are associated with this product ID.
***************************************************************************/
    returnStatus = PGS_PC_GetNumberOfFiles(prodID,&numFiles);

/***************************************************************************
*   Only continue if everything went OK.
***************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*   Loop through all of the files.
***************************************************************************/
        for (loopCount = 1; loopCount <= numFiles; loopCount++)
        { 

/***************************************************************************
*   Get the attribute.
***************************************************************************/
             returnStatus = PGS_PC_GetFileAttr(prodID,loopCount,
                              PGSd_PC_ATTRIBUTE_STRING,maxSize,fileAttribute);

/***************************************************************************
*   If this file did not have an attribute then let's get the next one.
***************************************************************************/
             if (returnStatus == PGSPC_W_NO_ATTR_FOR_ID)
             {
                 continue;
             }

/***************************************************************************
*   If this everything went OK, then check it.  We don't care if it
*   was truncated and it is possible that the user does not care.
***************************************************************************/
             else if ((returnStatus == PGS_S_SUCCESS) || 
                    (returnStatus == PGSPC_W_ATTR_TRUNCATED))
             {
                 foundFlag = searchFunc(fileAttribute);

/***************************************************************************
*   If there is a match, then break out of the loop.
***************************************************************************/
                 if (foundFlag == PGSd_PC_MATCH)
                 {
                     returnStatus = PGS_S_SUCCESS;
                     *version = loopCount;
                     break;
                 }
             }
        }  /* end for */

/***************************************************************************
*   If there was not a match set the return value accordingly.
***************************************************************************/
        if (foundFlag == PGSd_PC_NO_MATCH)
        {
            switch (PGS_SMF_TestStatusLevel(returnStatus))
            {
                case PGS_SMF_MASK_LEV_S:
                case PGS_SMF_MASK_LEV_W:
                    returnStatus = PGSPC_W_NO_ATTR_MATCH;
                    break;
                case PGS_SMF_MASK_LEV_E:
                    returnStatus = PGSPC_E_DATA_ACCESS_ERROR;
                    break;
            }
        }
    }  /* end if */

/***************************************************************************
*   Free up the memory allocated for the string variable storing the
*   actual file attribute.
***************************************************************************/
    free(fileAttribute);

/***************************************************************************
*   Update the status log and return.
***************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetFileByAttr()");
    }
    return returnStatus;
}
