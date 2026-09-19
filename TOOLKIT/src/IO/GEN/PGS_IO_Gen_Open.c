/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG

FILENAME:   
	PGS_IO_Gen_Open.c

DESCRIPTION:
	The tool listed in this file provides the user with the means to open
        NON-PRODUCT SUPPORT FILES that are to be accessed by the standard
        "C" library stream I/O routines e.g., fscanf().

AUTHOR:
	David P. Heroux / Applied Research Corp.

HISTORY:
	18-Mar-1994 CR  Initial version
  	30-Mar-1994 MES Standardize prologs, SMF support
	31-Mar-1994 DPH Modified for Process Control Access
	03-Aug-1994 DPH Added version argument to support one-to-many 
			logical-to-physical file relationships
	12-Aug-1994 DPH For TK2.5, version # must = 1 to be compatible
			with TK2 release of PC tools.
			I/O tools will no longer return the remaining number
			of file versions. This is to safeguard against users
			passing loop control variables (lcv's) to this tool.
	12-Aug-1994 TWA Misc prolog changes
	29-Aug-1994 DPH Upgraded for TK3: 'file_version' passed onto low-level
			functions; 'Support File' sections of PCF are now 
			searched as well as the 'Product File' sections. 
	22-Sep-1994 DPH Added PGSIO_E_GEN_REFERENCE_FAILURE to exception block
			in order to set the accompanying message. 
	29-Sep-1994 DPH Added Warning to prolog for appendupdate mode
			Corrected condition which let SW attempt to open an
			undefined file reference even when none were found.
	14-Oct-1994 DPH	In order to compensate for recursive behaviour of the
			low-level Toolkit interdependencies, checks for SMF
			invocation are performed prior to invoking other SMF
			functionality. 
	17-Oct-1994 DPH	Added comments regarding version number support
	10-Feb-1995 TWA Deleting unused variables to eliminate compiler warnings
        14-Jul-1995 DPH Updating prolog to reflect discontinued use of 
                        environment variables.
        02-Nov-2000 DPH/YLH  Completed changes to identify the Logical ID of the
                             file the function is trying to open in case of failure;
                             assisted by Abe Taaheri


END_FILE_PROLOG
***************************************************************************/

#define  PGSd_IO_Gen_c          /* flag PGS_IO.h that this is Generic
                                   tools source code */

#include <PGS_IO.h>

/****************************************************************************
BEGIN_PROLOG

TITLE: 
	Open a Generic File
 
NAME:	
	PGS_IO_Gen_Open

SYNOPSIS:
C:      
	#include <PGS_IO.h>

        PGSt_SMF_status
        PGS_IO_Gen_Open(
                PGSt_PC_Logical         file_logical,
                PGSt_IO_Gen_AccessType  file_access,
                PGSt_IO_Gen_FileHandle  **file_handle,
		PGSt_integer		file_version)
Fortran:
	(not applicable)

DESCRIPTION: 	
	Upon a successful call, this function will provide the argument 
	PGSt_IO_Gen_FileHandle to support other "C" library stream manipulation 
	routines.

INPUTS:		
	file_logical 	- User defined logical file identifier

  	file_access 	- type of access granted to opened file:
	   Toolkit			C	Description
	   ------		-	-----------
	   PGSd_IO_Gen_Read	"r"	Open file for reading
	   PGSd_IO_Gen_Write	"w"	Open file for writing, truncating existing
					file to 0 length, or creating a new file
	   PGSd_IO_Gen_Append	"a"	Open file for writing, appending to the
					end of existing file, or creating file
	   PGSd_IO_Gen_Update	"r+"	Open file for reading and writing
	   PGSd_IO_Gen_Trunc	"w+"	Open file for reading and writing,
					truncating existing file to zero length,
					or creating new file
	   PGSd_IO_Gen_Append   "a+"	Open file for reading and writing,
		       Update	  	to the end of existing file,
					or creating a new file; whole file
					can be read, but writing only appended

	file_version	- specific version of the logical Product Input file.  
	
OUTPUTS:	
	file_handle	- used to manipulate files with other "C" library stream 
                          I/O routines.

RETURNS: 
	PGS_S_SUCCESS			Success
	PGS_E_UNIX			Unix system error
	PGSIO_E_GEN_OPENMODE		Invalid access mode
	PGSIO_E_GEN_FILE_NOEXIST	No entry for file logical in $PGS_PC_INFO_FILE
	PGSIO_E_GEN_REFERENCE_FAILURE 	Other error accessing $PGS_PC_INFO_FILE
	PGSIO_E_GEN_BAD_ENVIRONMENT 	Environment error reported by Process Control

	(NOTE: the above are short descriptions only; full text of messages appears 
	 in files $PGSMSG/PGS_IO_1.t. Descriptions may change in future releases depending 
	 on external ECS design.)

EXAMPLE:
	// This example illustrates how to open a Product Output File for writing //

	PGSt_SMF_status			returnStatus;
	PGSt_PC_Logical			logical;
	PGSt_IO_Gen_AccessType 		access;
	PGSt_IO_Gen_FileHandle 		*handle;
	PGSt_integer 			version;

	logical = 10;
	version = 1;			// will default to 1 for TK3 on out //
	access = PGSd_IO_Gen_Write;
	returnStatus = PGS_IO_Gen_Open( logical,access,&handle,version );
	if (returnStatus != PGS_S_SUCCESS)
	{
	    goto EXCEPTION;
	}
	    .
	    .
	    .
	EXCEPTION:

NOTES:		
	A file opened for write that already exists will be overwritten. 

	This function will support all POSIX modes of fopen.

	While all modes of access are supported for this tool, those modes 
	which allow for writing to a file (i.e., not PGSd_IO_Gen_Read) are 
	intended for Toolkit access only. The only files which the science 
	software should write to are product output files (Hdf) and Temporary, 
	or Intermediate files. The only exceptions to this are for Support 
	Output files which may need to be archived, but which are not 
	considered to be products.

        !!!!!!!!!!! During testing of this tool, the mode AppendUpdate (a+)
        !! ALERT !! was found to produce results that were not consistent
        !!!!!!!!!!! with the documented POSIX standard.The sort of behavior
        that was typically observed was for data, buffered during a read
        operation, to be appended to the file along with other data that was
        being written to the file. Note that this behavior could not be attributed
        to the Toolkit since the same behavior was revealed when purely "POSIX"
        calls were used.

	Support for the one-to-many logical-to-physical file relationshsip only
	extends to Product Input Files. Therefore, files may only have more than
	one version if they are entered in the PRODUCT INPUT FILES section of the
	Process Control File. In order to ascertain the number of versions 
	currently associated with a logical identifier, make a call to 
	PGS_PC_Get_NumberOfFiles() first.

        IMPORTANT TK5 NOTES
        -------------------
        The following environment variable MUST be set to assure proper operation:
 
                PGS_PC_INFO_FILE        path to process control file
 
        However, the following environment variables are NO LONGER recognized by
        the Toolkit as such:
 
                PGS_PRODUCT_INPUT  default path to Product input files
                PGS_PRODUCT_OUTPUT default path to Product output files
                PGS_SUPPORT_INPUT  default path to Supporting input files
                PGS_SUPPORT_OUTPUT default path to Supporting output files
 
        Instead, the default paths, which were defined by these environment variables
        in previous Toolkit releases, may now be specified as part of the Process
        Control File (PCF). Essentially, each has been replaced by a global path
        statement for each of the respective subject fields within the PCF. To
        define a global path statement, simply create a record which begins with the
        '!' symbol defined in the first column, followed by the global path to be
        applied to each of the records within that subject field. Only one such
        statement can be defined per subject field and it must be appear prior to
        any dependent subject entry.
 
        The status condition PGSIO_E_GEN_BAD_ENVIRONMENT now indicates an error
        status on the global path statement as defined in the PCF, and NOT on an
        environment variable. However, as with previous releases, the status message
        associated with this condition may reference the above "tokens", but this is
        only to indicate which of the global path statements is problematic.


REQUIREMENTS:	
	PGSTK-0360, 1360

DETAILS: 	
	This function will use the "C" fopen() call to open a "stream". It 
	will call another toolkit routine to map the file logical to its 
	physical file name. Check for errors while opening and go 
	to exception handling if so, otherwise return success 

	Due to the interdependence among some of the Toolkit functions, Checks 
	for recursion are being made in order to prevent the recursion from 
	going out of control. Fortunately, this interdependence only lasts for 
	an initialization period and can therfore be controlled. This recursive 
	side effect will go away in Toolkit 4 since all Toolkit initialization 
	will be performed external to the science software and can therefore
	be carried-out in a more logical fashion (i.e. we will control which
	Toolkit functionality calls who first, which we can't do now since we
	rely on performing the initialization as a side effect of the user 
	naturally using the Toolkit. That is, we cannot not control which 
	Toolkit function the user will call first in TK3). 

GLOBALS:
        None
 
FILES:
        Process Control File
 
FUNCTIONS CALLED:
        PGS_PC_GetPCSData
        PGS_SMF_SetStaticMsg
        PGS_SMF_SetUNIXMsg

END_PROLOG
***************************************************************************/

PGSt_SMF_status PGS_IO_Gen_Open(  	 	    	 
    PGSt_PC_Logical 	   file_logical, /* file logical */
    PGSt_IO_Gen_AccessType file_access,  /* mode to open file in e.g. READ */
    PGSt_IO_Gen_FileHandle **file_handle,/* file handle to be returned */
    PGSt_integer	   file_version) /* file version required */
{   
    PGSt_SMF_status 	returnStatus; 	/* value returned indicating success or failure */
    FILE 		*temp_ptr; 	/* POSIX stream pointer */
    char 		outstr[PGSd_IO_GEN_REFERENCE_MAX]; 
					/* variable to retrieve file name in */
    PGSt_PC_Long_Int 	mode; 		/* value sent to PGS_PC_GetPCSData */
    short		modes;		/* Loop Control Variable (LCV) */
    PGSt_PC_Long_Int    mode_array[4];  /* modes used by PGS_PC_GetPCSData */
    PGSt_integer	inputFileVersion;/* input/output argument */

    char                msg[PGS_SMF_MAX_MSG_SIZE];      /* predefined status message */
    char                newMsg[PGS_SMF_MAX_MSGBUF_SIZE];/* new status message */

    PGSt_SMF_status	caller;		/* determines who's calling who */

    /*------------------------------------------------------
   	Get Toolkit CallerId 
	if SMF is caller do NOT make SMF calls!
    ------------------------------------------------------*/
    caller = PGS_SMF_CallerID();

    /*------------------------------------------------------
   	Initialize 
    ------------------------------------------------------*/
    returnStatus = PGS_S_SUCCESS; 	/* success */
    *file_handle = NULL;		/* nullify handle until real one assigned */
    mode = PGSd_PC_INPUT_FILE_NAME;  	/* get physical input file name */
    strcpy( outstr,"" );           	/* return empty string */

    mode_array[0] = PGSd_PC_INPUT_FILE_NAME;
    mode_array[1] = PGSd_PC_OUTPUT_FILE_NAME;
    mode_array[2] = PGSd_PC_SUPPORT_IN_NAME;
    mode_array[3] = PGSd_PC_SUPPORT_OUT_NAME;

    /*------------------------------------------------------
   	Convert logical identifier into a physical one;
	examine specific fields of the PC table to find
	if reference exists for the logical ID specified.
    ------------------------------------------------------*/
    for (modes=0;modes<4;modes++)
    {	
        mode = mode_array[modes];
	inputFileVersion = file_version;	/* reset each time */
	returnStatus = PGS_PC_GetPCSData( mode,file_logical,outstr,&inputFileVersion );
	if (returnStatus == PGSPC_E_ENVIRONMENT_ERROR)
	{
	    returnStatus = PGSIO_E_GEN_BAD_ENVIRONMENT;
	    goto EXCEPTION;
	}
    	if (returnStatus != PGS_S_SUCCESS)
    	{
	    if (returnStatus == PGSPC_W_NO_FILES_EXIST)
	    {
		if (modes < 3)
		{
	    	    continue;
		}
		else
		{
	    	    goto EXCEPTION;
		}
	    }
	    else
	    {
		returnStatus = PGSIO_E_GEN_REFERENCE_FAILURE;
	    	goto EXCEPTION;
	    }
    	}
        else
        {
            break;
        }
    } /* end for (modes) */


    /*------------------------------------------------------
	Open physical file under proper access mode
    ------------------------------------------------------*/
    switch (file_access) 
    {
    case PGSd_IO_Gen_Write:
   	if ((temp_ptr = fopen(outstr,"w"))== NULL)
   	{
	    if (caller != PGSd_CALLERID_SMF)
	    {
	        PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Open");
	    }
            returnStatus = PGS_E_UNIX;
   	}
   	else
   	{
	    *file_handle = temp_ptr;	
   	}
	break;

    case PGSd_IO_Gen_Read:
   	if ((temp_ptr = fopen(outstr,"r"))== NULL)
   	{
            if (caller != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Open");
            }
            returnStatus = PGS_E_UNIX;
   	}
   	else
   	{
	    *file_handle = temp_ptr;
      	} 
 	break;

    case PGSd_IO_Gen_Append:
 	if ((temp_ptr =fopen(outstr,"a"))== NULL)
   	{
            if (caller != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Open");
            }
            returnStatus = PGS_E_UNIX;
   	}
   	else
   	{
   	    *file_handle = temp_ptr;
   	}
	break;

    case PGSd_IO_Gen_Update:
  	if ((temp_ptr =fopen(outstr,"r+"))== NULL)
   	{
            if (caller != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Open");
            }
            returnStatus = PGS_E_UNIX;
   	}
   	else
   	{
	    *file_handle = temp_ptr;
      	}
	break;

    case PGSd_IO_Gen_Trunc:
	if ((temp_ptr=fopen(outstr,"w+"))== NULL)
	{
            if (caller != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Open");
            }
            returnStatus = PGS_E_UNIX;
	}
	else
	{
	    *file_handle = temp_ptr;
    	}
	break;

    case PGSd_IO_Gen_AppendUpdate:
	if ((temp_ptr=fopen(outstr,"a+"))== NULL)
	{
            if (caller != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Open");
            }
            returnStatus = PGS_E_UNIX;
	}
	   else
	{
    	    *file_handle = temp_ptr;
       	}
	break;

    default:
	returnStatus = PGSIO_E_GEN_OPENMODE;
    }

EXCEPTION:

    if (returnStatus == PGSPC_W_NO_FILES_EXIST)
    {   
        returnStatus = PGSIO_E_GEN_FILE_NOEXIST;
    }

    switch (returnStatus)
    {
    case PGSIO_E_GEN_OPENMODE:
        if (caller != PGSd_CALLERID_SMF)
        {
            sprintf(newMsg,"Invalid mode with LID %d. ",(int)file_logical);
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            strcat(newMsg,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Open");
        }
        break;
    case PGSIO_E_GEN_BAD_ENVIRONMENT:
        if (caller != PGSd_CALLERID_SMF)
	{
            sprintf(newMsg,"Environment error for LID %d. ",(int)file_logical);
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            strcat(newMsg,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Open");
	}
        break;
   case PGSIO_E_GEN_FILE_NOEXIST:
        if (caller != PGSd_CALLERID_SMF)
        {
            sprintf(newMsg,"No entry for file with LID %d. ",(int)file_logical);
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            strcat(newMsg,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Open");

        }
        break;
   case PGSIO_E_GEN_REFERENCE_FAILURE:
    if (caller != PGSd_CALLERID_SMF)
        {
            sprintf(newMsg,"Can not find Physical file name with LID %d. ",(int)file_logical);
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            strcat(newMsg,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Open");
        }
        break;
    }

    if(*file_handle == NULL)
      {
	if (caller != PGSd_CALLERID_SMF)
	  {
	    if((returnStatus != PGS_S_SUCCESS) &&
	       (returnStatus != PGSIO_E_GEN_OPENMODE) &&
	       (returnStatus != PGSIO_E_GEN_BAD_ENVIRONMENT) &&
	       (returnStatus != PGSIO_E_GEN_FILE_NOEXIST) &&
	       (returnStatus != PGSIO_E_GEN_REFERENCE_FAILURE))
	      {
		sprintf(newMsg,"No entry for file with LID %d. ",(int)file_logical);
		PGS_SMF_GetMsgByCode(returnStatus,msg);
		strcat(newMsg,msg);
		PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Open");
	      }
	  }
	if(returnStatus == PGS_S_SUCCESS)
	  {
	    returnStatus = PGS_E_TOOLKIT;
	  }
      }
    
    return( returnStatus );
}
