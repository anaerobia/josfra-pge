/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetFileAttr.c

DESCRIPTION:
	This file contains the function PGS_PC_GetFileAttr().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	05-Aug-94 RM Initial Version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
        11-Jul-95 RM Changed LONG to INT in cfortran.h macros to correct
                        "Unaligned access error" as per DR ECSed00931.
	23-Oct-96 RM Fixed per ECSed04042.  Added check at end of
			function to call PGS_SMF_SetDynamicMsg() if the
			return is equal to PGSPC_W_NO_REFERENCE_FOUND.
 
        17-Mar-97 CT Added three modes, PGSd_PC_OUTPUT_FILE_ATTRIBUT, 
                     PGSd_PC_SUPPORT_IN_ATTR, and PGSd_PC_SUPPORT_OUT_ATTR
                     for function PGS_PC_GetPCSData() to get the filename 
                     where the attribute is stored. This wiil provide users  
                     with capabilities to retrieve paramerter values from the
                     PCF table in the PRODUCT OUTPUT FILE section or SUPPORT  
                     INPUT FILES section or SUPPORT INPUT FILES section when 
                     calling the function PGS_MET_GetPCAttr(). 
                    
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get the attribute of the file associated with the particular 
	product ID and version.
 
NAME:
	PGS_PC_GetFileAttr()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetFileAttr(
			PGSt_PC_Logical		prodID,
			PGSt_integer		version
			PGSt_integer		formatFlag,
			PGSt_integer		maxSize,
			char			*fileAttribute)

FORTRAN:	
		include 'PGS_SMF.f'
		include 'PGS_PC.f'
		include 'PGS_PC_9.f'

		integer function pgs_pc_getfileattr(prodid,version,
					formatflag,fileAttribute)
			integer			prodid
			integer			version
			integer			formatflag
			integer			maxSize
			character*(*)		fileAttribute

DESCRIPTION:
	This tool may be used to retrieve an attribute associated with a 
	particular product ID and version number.  The data placed in the 
	attribute will be defined and interpreted by the user.  The PGS 
	Toolkit has no dependency on the attribute.
 
INPUTS:
	Name		Description			Units	Min	Max

	prodID		The logical identifier as 
			defined by the user.  The user's
			definitions will be mapped into 
			actual identifiers during the 
			Integration & Test procedure.

	version		The particular version of the 
			Product ID that the attribute 
			is being requested from.  With 
			files there may be a many-to-one
			relationship.

	formatFlag	Flag indicating method of 
			attribute return.  Possible
			values are:

			PGSd_PC_ATTRIBUTE_LOCATION
			PGSd_PC_ATTRIBUTE_STRING

	maxSize		Amount of space allocted for
			attribute if formatFlag is
			PGSd_PC_ATTRIBUTE_STRING.

OUTPUTS:
	Name		Description			Units	Min	Max

	fileAttribute	The actual file attribute.
	
			If formatFlag is 
			PGSd_PC_ATTRIBUTE_LOCATION then 
			fileAttribute will return the 
			file containing the attribute.

			If formatFlag is 
			PGSd_PC_ATTRIBUTE_STRING then 
			fileAttribute will return the 
			attribute as a string.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_REFERENCE_FOUND  no reference found matching product id and
                                    version number
	PGSPC_W_ATTR_TRUNCATED      not enough space passed in for attribute
	PGSPC_W_NO_ATTR_FOR_ID      a physical reference was found but no 
				    attribute exists for that reference
	PGSPC_E_DATA_ACCESS_ERROR   error accessing PCS data
	PGSPC_E_INVALID_MODE	    invalid format flag value passed in

EXAMPLES:

C:
	#define MODIS1A 4220

	PGSt_integer	version;
	PGSt_integer	maxSize;
	PGSt_SMF_status	returnStatus;
	char		fileAttribute[PGSd_PC_FILE_PATH_MAX];

	version = 1;
	maxSize = 0;

	/# get the attribute file name of the first MODIS1A file #/

	returnStatus = PGS_PC_GetFileAttr(MODIS1A,version,
					PGSd_PC_ATTRIBUTE_LOCATION,maxSize,
					fileAttribute);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
	{

	 /# open attribute file and search attribute for particular data #/

	}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
		IMPLICIT NONE

		integer		version
		integer		returnstatus
		integer		maxsize
		character*355	fileattribute
		integer		pgs_pc_getfileattr
		integer		modis1a
		parameter	(modis1a = 4220)

		version = 1
		maxsize = 355

C	get the attribute file name of the first modis1a file 

		returnstatus = pgs_pc_getfileattr(modis1a,version,
					PGSd_PC_ATTRIBUTE_LOCATION,maxsize,
					fileattribute)

		if (returnstatus .ne. pgs_s_success) then
			goto 9999

		else

C			open attribute file and search attribute for
C			particular data

		endif
			.
			.
			.
		return

NOTES:
	Allocating enough space for the attribute variable will be the 
	responsibility of the application programmer.  This function 
	will write the attribute into fileAttribute for maxSize bytes
	or the end of the attribute, which ever comes first.

REQUIREMENTS:  
	PGSTK-1290
	PGSTK-1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_PC_GetPCSData                Get process control information.
	PGS_PC_BuildAttribute		 Build the attribute string
        PGS_SMF_GetMsgByCode             Get error/status from code
        PGS_SMF_SetStaticMsg             Set static error/status message
        PGS_SMF_SetDynamicMsg            Set dynamic error/status message

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetFileAttr(                    /* get file attribute */
    PGSt_PC_Logical   prodID,	       /* logical value */
    PGSt_integer      version,         /* file version number */
    PGSt_integer      formatFlag,      /* flag indicating format to return */
    PGSt_integer      maxSize,         /* maximum size of attribute */
    char             *fileAttribute)   /* file attribute */

{
    char              buf[PGS_SMF_MAX_MSG_SIZE];  /* message buffer */
    char              msg[PGS_SMF_MAX_MSGBUF_SIZE];  /* message buffer */
    char              attributeLoc[PGSd_PC_FILE_PATH_MAX]; /* attribute file */
    int fileTypes[] = {PGSd_PC_INPUT_FILE_ATTRIBUTE,
                       PGSd_PC_OUTPUT_FILE_ATTRIBUTE,
                       PGSd_PC_SUPPORT_IN_ATTR,
                       PGSd_PC_SUPPORT_OUT_ATTR};  /* array of file types */

    int count;                          /* for-loop counter */
    int total;                          /* number of types of files */
    PGSt_integer      numFiles;        /* set to version for GetPCSData */
    PGSt_SMF_status   whoCalled;       /* user that called this function */
    PGSt_SMF_status   returnStatus;    /* function return */

/***************************************************************************
*   Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    total = sizeof(fileTypes) / sizeof(int);
    numFiles = version;

/***************************************************************************
*   Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/****************************************************************************
*    Loop the number of types of files times in order to get the name of the
*    file where the attribute is stored and check for a file each time.
****************************************************************************/
    for (count = 0; count < total; count++)
    {
         returnStatus = PGS_PC_GetPCSData(fileTypes[count],prodID,
                                          attributeLoc,&numFiles);
 
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

/***************************************************************************
*   Make sure we got what we needed.
***************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {

/***************************************************************************
*   Now, let's find out what the user wanted us to return.
***************************************************************************/
        switch (formatFlag)
        {

/***************************************************************************
*   The user just wants the name of the file.  We are basically done.
***************************************************************************/
             case PGSd_PC_ATTRIBUTE_LOCATION:
                 strcpy(fileAttribute,attributeLoc);
                 break;

/***************************************************************************
*   The user wants the attribute returned as a string.
***************************************************************************/
             case PGSd_PC_ATTRIBUTE_STRING:
                 returnStatus = PGS_PC_BuildAttribute(attributeLoc,maxSize,
    					fileAttribute);
                 break;

/***************************************************************************
*   The user made a boo-boo.  This is not a valid format flag (or mode)
*   to be used here.
***************************************************************************/
             default:
                 returnStatus = PGSPC_E_INVALID_MODE;
                 if (whoCalled != PGSd_CALLERID_SMF)
                 {
                     PGS_SMF_GetMsgByCode(PGSPC_E_INVALID_MODE,msg);
                     sprintf(buf,msg,formatFlag);
                     PGS_SMF_SetDynamicMsg(PGSPC_E_INVALID_MODE,buf,
                                       "PGS_PC_GetFileAttr()");
                 }
                 break;
        }  /* end switch */
    }   /* end if */

/***************************************************************************
*   Check our function return value to make sure we send something 
*   of value back to the user.
***************************************************************************/
    if (returnStatus != PGS_S_SUCCESS)
    {

/***************************************************************************
*   There is no file associated with this Product ID.
***************************************************************************/
         if (returnStatus == PGSPC_W_NO_FILES_EXIST)
         {
             returnStatus = PGSPC_W_NO_REFERENCE_FOUND;
         }

/***************************************************************************
*   Well, there is a file associated with this Product ID, but the 
*   file has no attribute (no big deal).
***************************************************************************/
         else if (returnStatus == PGSPC_W_NO_DATA_PRESENT)
         {
             returnStatus = PGSPC_W_NO_ATTR_FOR_ID;
         }

/***************************************************************************
*   We had to truncate the attribute since you did not give me enough
*   memory for the entire attribute.
***************************************************************************/
         else if (returnStatus == PGSPC_W_TRUNCATED)
         {
             returnStatus = PGSPC_W_ATTR_TRUNCATED;
         }

/***************************************************************************
*   There was an error.
***************************************************************************/
         else if (returnStatus != PGSPC_E_INVALID_MODE)
         {
              returnStatus = PGSPC_E_DATA_ACCESS_ERROR;
         }
    }

/***************************************************************************
*   Update the message log and return.
***************************************************************************/
    if ((whoCalled != PGSd_CALLERID_SMF) &&
        (returnStatus != PGSPC_E_INVALID_MODE))
    {
        if (returnStatus != PGSPC_W_NO_REFERENCE_FOUND)
        {
            PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetFileAttr()");
        }
        else
        {
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            sprintf(buf,msg,prodID,version);
            PGS_SMF_SetDynamicMsg(returnStatus,buf,"PGS_PC_GetFileAttr()");
        }
    }
    return returnStatus;
}
