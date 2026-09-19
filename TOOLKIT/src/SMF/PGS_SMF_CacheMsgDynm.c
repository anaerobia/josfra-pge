/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_SMF_CacheMsgDynm.c

DESCRIPTION:
   This file contains the function PGS_SMF_CacheMsgDynm().
   This function implements a cache for return status code information that is
   stored in the SMF message files.

AUTHOR:
   Guru Tej S. Khalsa

HISTORY:
   06-Jan-1996  GTSK  Initial version - based on code by Micheal Sucher

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CacheMsgDynm

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_CacheMsgDynm (
        int               cmd,
        PGSt_SMF_code     code,
        char              *mnemonic,
        PGS_SMF_MsgInfo   *codeinfo);

FORTRAN:
    NONE

DESCRIPTION:
    This tool implements a cache for return status code information
    that is stored in the SMF message files.  It was added with version
    6 of the toolkit.  In normal use, it will only be called by the 
    tool PGS_SMG_GetUserCode.   It stores message information in and 
    retrieves message information from a memory buffer.  This eliminates 
    the overhead associated with opening, reading and closing the message
    files, when the same message is encountered multiple times during
    a Product Generation Executable (PGE) run.

    The cache depends on the shared memory region set up by the PGE 
    initialization tool PGS_PC_InitCom.  The size of the cache, in records, 
    is specified via a command line argument to this tool, (which in normal 
    use is always passed in from the PGE shell script PGS_PC_Shell.sh).   
    For the best performance, the cache should be made large enough to hold 
    all the error codes that are likely to be encountered during a PGE run, 
    assuming that adequate sytem memory is available.  A nominal value would 
    be in the range of 100 to 300 records.
 

INPUTS:    
    Name        Description

    cmd         command; current legal values are:

                    PGSd_SMF_FindByCode
                    PGSd_SMF_FindByMnemonic
                    PGSd_SMF_AddRecord
                    PGSd_SMF_LastFindStatus
                    PGSd_SMF_DisplayBuffer
                    PGSd_SMF_InitCacheInfo1
                    PGSd_SMF_InitCacheInfo2

    code        numeric value of error/status code generated 
                by message compiler
                (also used to set the value of cache_count
                when cmd is set to PGSd_SMF_InitCacheInfo)

    mnemonic    mnemonic string for error/status code generated 
                by message compiler

    codeinfo    code information to store
                (cmd set to PGSd_SMF_AddRecord)

OUTPUTS:    
    Name        Description

    codeinfo    code information retrieved
                (cmd set to PGSd_SMF_FindByCode or PGSd_SMF_FindByMnemonic)

RETURNS:        
    PGS_S_SUCCESS    
    PGSSMF_M_NOT_IN_CACHE

EXAMPLES:
    PGSt_SMF_status   returnStatus;
    PGS_SMF_MsgInfo   msg_info;
    PGSt_SMF_code     code,
    char buf[PGSd_SMF_MsgBufColumns];

    returnStatus = PGS_SMF_CacheMsgDynm(
                            PGSd_SMF_FindByCode, code, 0, &msg_info);

NOTES: 
    1)  This is a low-level tool.  It is only intended to be called by
        toolkit code.
    2)  Commands PGSd_SMF_LastFindStatus is included for test purposes only.
        In normal use, it will not be called.

REQUIREMENTS:               
    PGSTK-0580,0590,0650

DETAILS:	   
    The current version uses a simple first-in-first-out (FIFO) circular
    buffer to store the records.  There are hooks in the code that store
    the value of an access counter in the unused funcname member of the
    message record structure.  This may be used in a future upgrade to
    the buffering algorithm that replaces records on the basis of which
    has the oldest access time, read OR write, rather than which one was 
    written first.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*******************************************************************************/

#include <string.h>
#include <PGS_SMF.h>

PGSt_SMF_status 
PGS_SMF_CacheMsgDynm (
    int              cmd,
    PGSt_SMF_code    code,
    char             *mnemonic,
    PGS_SMF_MsgInfo  *codeinfo)        /* code information */
{
    PGSt_SMF_status        returnStatus = PGS_S_SUCCESS; /* return status */

    int                     i;               /* loop count variable */

    static PGSt_uinteger    cache_count=100; /* count of records allocated */
    static PGSt_integer     newest=-1;       /* index to newest entry */
    static PGSt_integer     oldest=-1;       /* index to oldest entry */

    static PGS_SMF_MsgInfo  msg_buf[100];    /* message cache structure array */

    static int              cache_hit=0;     /* cache hit flag */

    static PGSt_uinteger    access_count=0;  /* count of cache accesses */


    access_count++;                          /* count this cache access */

    /*  
     * Handle supported commands
     */

    switch (cmd)
    {
      case PGSd_SMF_AddRecord:               /* Add a record */

        newest++;                            /* point new_ptr at next record */
        if (newest >= cache_count) 
        {                                    /* wrap at end of buffer */
            newest = 0;
        }

        if (newest == oldest)
        {                                    /* adjust index to oldest */
            oldest++;
            if (oldest >= cache_count) 
            {
                oldest = 0;
            }
        }
	if (oldest == -1) oldest = 0;        /* first time only */

        memcpy( (char *) &msg_buf[newest],   /* copy in record */
		(char *) codeinfo, 
		sizeof(PGS_SMF_MsgInfo) );

	memset( msg_buf[newest].funcname,    /* clear the function name */
                0,
                sizeof(msg_buf[newest].funcname) );

        sprintf(msg_buf[newest].funcname, "%u", access_count);

        break;


      case PGSd_SMF_FindByCode:              /* Find by code value */

	returnStatus = PGSSMF_M_NOT_IN_CACHE;        /* assume not found */
        cache_hit = 0;                               /* ... */

        for (i = 0; i < cache_count ; i++)
	{
	    if (msg_buf[i].msgdata.code == code)
	    {                                        /* found it */
		memcpy((char *) codeinfo,            /* copy it */
                       (char *) &msg_buf[i],
                       sizeof(PGS_SMF_MsgInfo) );
		returnStatus = PGS_S_SUCCESS;        /* flag it */
                cache_hit = 1;                       /* ... */
		break;                               /* done */
	    }
        }
	break;
        

      case PGSd_SMF_FindByMnemonic:          /* Find by mnemonic string */

	returnStatus = PGSSMF_M_NOT_IN_CACHE;        /* assume not found */
        cache_hit = 0;                               /* ... */

        for (i = 0; i < cache_count ; i++)
	{
	    if (strcmp(msg_buf[i].msgdata.mnemonic, mnemonic) == 0)
	    {                                        /* found it */
		memcpy((char *) codeinfo,            /* copy it */
                       (char *) &msg_buf[i],
                       sizeof(PGS_SMF_MsgInfo) );
		returnStatus = PGS_S_SUCCESS;        /* flag it */
                cache_hit = 1;                       /* ... */
		break;                               /* done */
	    }
        }
	break;
        

      case PGSd_SMF_LastFindStatus:          /* Get last find cmd status */
	
        if (cache_hit == 0)                     /* last call was cache miss */
        {
            returnStatus = PGSSMF_M_NOT_IN_CACHE;
        }
        else                                    /* last call was cache hit  */
        {
            returnStatus = PGS_S_SUCCESS;
        }

	break;
        

      default:                        /* unsupported command */

        returnStatus = PGS_E_TOOLKIT;
        break;

    }

    return returnStatus;
}


