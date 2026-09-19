/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
        PGS_PC_GetFileSize.c
 
DESCRIPTION:
        This file contains the function PGS_PC_GetFileSize().
 
AUTHOR:
        Carol S. W. Tsai / Applied Research Corp.
 
HISTORY:
        30-Jan-97 CSWT Initial Version
 
END_FILE_PROLOG:
***************************************************************************/
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <PGS_PC.h>
#include <PGS_SMF.h>
#include <errno.h>
 
/***************************************************************************
BEGIN_PROLOG:
 
TITLE:
        Get the file size listed in PCF associated with  the particular input  
        parameters, logical ID and version that wll be defined by the user. 
 
NAME:
        PGS_PC_GetFileSize()
 
SYNOPSIS:
 
C:
        #include <PGS_PC.h>
 
        PGSt_SMF_status
        PGS_PC_GetFileSize(
            PGSt_PC_Logical  prodID,
            PGSt_integer     version,
            PGSt_integer*    filesize)      
 
FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_PC_9.f'

      integer function pgs_pc_get_file_size(prodid, version, filesize)
      integer     prodid
      integer     version
      integer     filesize

DESCRIPTION:
        This tool may be used to obtain the size of a file from a  
        logical identifier.
 
INPUTS:
        Name            Description                     Units   Min     Max
        ----            -----------                     -----   ---     ---
        prodID          User defined constant identifier
                        that internally represents the
                        current product.
 
        version         Version of reference to get.
                        Remember,  for Product Input
                        files and Product Output files
                        there can be a many-to-one
 
OUTPUTS:
        Name            Description                     Units   Min     Max
        ----            -----------                     -----   ---     ---
        filesize        The size of a file.             N/A
 
RETURNS:
        PGS_S_SUCCESS               successful execution
        PGSPC_W_NO_REFERENCE_FOUND  link number does not have the data that
                                    mode is requesting
        PGSPC_E_DATA_ACCESS_ERROR   problem while accessing PCS data
        PGS_E_UNIX                  Unix system error 
        PGS_E_TOOLKIT               an unexpected error occured

 
EXAMPLES:
 
C:
        #define         PROD_ID 10501 
 
        PGSt_integer    version;
        PGSt_integer    filesize;
        PGSt_SMF_status returnStatus;

        /# Get first version of the file #/
        version = 1;
 
        returnStatus = PGS_PC_GetFileSize(PROD_ID,version,&filesize);
 
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
 
NOTES:
        In order for this tool to function properly, a valid Process Control
        file will need to be created first.  Please refer to Appendix C
        (User's Guide) for instructions on how to create such a file.
 
REQUIREMENTS:
        PGSTK-1290
 
DETAILS:
        NONE
 
GLOBALS:
        NONE
 
FILES:
        NONE
 
FUNCTIONS_CALLED:
        PGS_SMF_CallerID            Get value of user variable
        PGS_SMF_SetUNIXMsg          sets the message buffer
        PGS_SMF_SetUnknownMsg       sets the message buffer
 
END_PROLOG:
***************************************************************************/

/* name of this function */

#define FUNCTION_NAME "PGS_PC_GetFileSize()"
 
PGSt_SMF_status
PGS_PC_GetFileSize(               /* get the size of a file listed in the PCF */
    PGSt_PC_Logical  prodID,      /* version of PCFSize requested */
    PGSt_integer     version,     /* logical ID */
    PGSt_integer     *filesize)   /* the size of a file */
 
{
 
/***************************************************************************
 Declarations.
***************************************************************************/

    char            referenceID[PGSd_PC_LINE_LENGTH_MAX];
    char            details[PGS_SMF_MAX_MSG_SIZE];       /* detailed error
                                                            message */
 
    PGSt_integer    numFiles;

    PGSt_SMF_status returnStatus;                        /* return value of
                                                            PGS function
                                                            calls */
 
    PGSt_SMF_status whoCalled;                           /* user that called
							    this function */
 
    size_t          statusCheck;
 
    struct stat     buf;                                 /* a pointer to a
                                                            stat() structure
                                                            into which infor-
                                                            mation is placed
                                                            concerning the
                                                            file */
 
/***************************************************************************
 Initialize variables.
***************************************************************************/

    numFiles = version;
    statusCheck = 0;
 
/***************************************************************************
 Initialize caller variable.
***************************************************************************/

    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
 Associated logical with physical filename.
***************************************************************************/
 
    returnStatus = PGS_PC_GetReference( prodID,
					&numFiles,
					referenceID );
    if (returnStatus != PGS_S_SUCCESS)
    {
	return returnStatus;
    }

/**************************************************************************
 Call a system function "stat" to  obtain information about the file 
 pointed to by the filename, an input parameter.     
**************************************************************************/
    
    statusCheck = stat( referenceID, &buf );

/**************************************************************************
 Upon successful completion a value of 0 is returned. Otherwise, a value of
 -1 is returned and errno is set to indicate the error. 
**************************************************************************/

    if ( statusCheck != 0)
    {
	switch(errno)
	{
	  case EACCES:
	    
	    sprintf(details, "Search permission is denied for " 
		    "the referenced file (ID: %d, ver.: %d).",
		    prodID, version);
	    break;
	    
	  case EFAULT:
	    sprintf(details, "Referenced file (ID: %d, ver.: %d) points to an "
		    "illegal address.", prodID, version);
	    break;
	    
	  case EINTR:
	    sprintf(details, "A signal was caught during the stat() "
		    "function.");
	    break;
	    
	  case ELOOP:
	    sprintf(details, "Too many symbolic links were encountered "
		    "in translating the referenced file (ID: %d, ver.: %d).",
		    prodID, version);
	    break;
	    
	  case ENAMETOOLONG:
	    sprintf(details, "The length of the referenced file name (%s) "
		    "exceeds the maximum allowable file name length.",
		    referenceID);
	    break;
	    
	  case ENOENT:
	    sprintf(details, "The referenced file (ID: %d, ver.: %d) does not "
		    "exist or is the null file.", prodID, version);
	    break;
	    
	  case ENOTDIR:
	    sprintf(details, "A component of the named file (%s) prefix is not "
		    "a directory.", referenceID);
	    break;
	    
	  default:
	    sprintf(details, "A UNIX system error occurred, unable to "
		    "determine size of reference file (ID: %d, ver.: %d).",
		    prodID, version);
	    break;
	}

	if (whoCalled != PGSd_CALLERID_SMF)
	{
	    PGS_SMF_SetUNIXMsg(errno, details, FUNCTION_NAME);
	}

        returnStatus = PGS_E_UNIX;
        *filesize = -1;
	return returnStatus;
    }

/***************************************************************************
  If the returnCheck is equal to 0, then set the filesize variable and 
  return the status with PGS_S_SUCCESS.
***************************************************************************/

    *filesize = (PGSt_integer) (buf.st_size);
    if (whoCalled != PGSd_CALLERID_SMF)
    {
	PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);
    }

    return PGS_S_SUCCESS;
} 
