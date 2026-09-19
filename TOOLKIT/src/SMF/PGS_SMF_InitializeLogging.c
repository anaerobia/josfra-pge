/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_SMF_InitializeLogging.c

DESCRIPTION:
   This file contains the function PGS_SMF_InitializeLogging().
   This function initializes the various SMF logging options.

AUTHOR:
   Guru Tej S. Khalsa

HISTORY:
   10-Jan-1996  GTSK  Initial version
   07-Jul-1999  RM    Updated for TSF functionality

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Manage List of SMF Messages For Which Logging Has Been Disabled

NAME:
   PGS_SMF_InitializeLogging()

SYNOPSIS:
C:
   #include <PGS_SMF.h>

   void
   PGS_SMF_InitializeLogging()

FORTRAN:		  
   N/A

DESCRIPTION:
   This function initializes the various SMF logging options.  These are:

     logging on/off
     trace level
     PID logging on/off
     specific SMF status level disabling
     specific SMF seed disabling
     specific SMF status code disabling

INPUTS:
   None

OUTPUTS:
   None
          
RETURNS:
   None

EXAMPLES:
C:

FORTRAN:
  
NOTES:
   The SMF logging utility can be customized in various ways via the process
   control file (PCF).  This function should be run before any attempts to write
   to the SMF log files are made by the various SMF tools.  Following is a
   discussion of the various logging options:

   In all cases a "header" entry will be made to the log file indicating the
   logging options for a given process.

   Logging On/Off:
   SMF logging may be turned off entirely for a given process.  No SMF status
   messages will be logged to any of the SMF log files in this case.

   
   Trace Level:
   The "trace level" of the log entries may be specified.  Trace level
   determines how detailed the calling history of the program is recorded in the
   log files.  The default is no tracing.  With no tracing individual status
   messages are recorded in the log file with the name of the function recording
   the message.  Error tracing may instead be specified.  If error tracing is
   specified then each status message entry is preceded by a call history of the
   functions that called the function that recorded the message.  In the case of
   error tracing this call history is only recorded at the time a status message
   is written to the log file.  The last alternative is full tracing.  If full
   tracing is specified an entry is made in the SMF status log file every time a
   function is entered or exited within the running program.  This is in
   addition to status messages recorded in the file at the request of functions
   (and occurs whether or not any such messages are recorded in the file).

   PID Logging:
   The process ID (PID) may be recorded with each entry in an SMF log file.
   This functionality was added so that if multiple processes of a single PGE
   are running simultaneously (and therefore all accessing the SMF log files
   simultaneously) the resulting messages in the SMF log files may be sorted by
   post-processing the log files.

   Status Level Logging Control:
   Logging of all messages of given SMF status levels (e.g. W, M, N) may be
   disabled.  No SMF status messages of the indicated status level(s) will be
   logged to any of the SMF log files in this case.

   Seed Logging Control:
   Logging of all messages derived from particular SMF seeds may be disabled.
   No SMF status messages derived for the indicated seed(s) will be logged to
   any of the SMF log files in this case.

   Status Message Logging Control:
   Logging of individual SMF status messages may be disabled.  No SMF status
   message(s) thus disabled will be logged to the SMF log files in this case.

REQUIREMENTS:
   PGSTK - ????, ????

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <PGS_SMF.h>
#include <PGS_PC.h>

void
PGS_SMF_InitializeLogging()             /* initialize SMF logging options */
{
    static PGSt_boolean first=PGS_TRUE; /* this function should only be executed
					   once per process, therefore keep a
					   static boolean value to track whether
					   or not this function has already
					   been called */

    char                *chrPtr;        /* used to mark specific character
					   locations in a character string */
    char                buf[5000];      /* input buffer for line of text */

    char                next_item[PGS_SMF_MAX_FUNC_SIZE];  /* variable to hold
							      sub-strings parsed
							      from input buffer
							      "buf" (above) */ 

    PGS_SMF_MsgInfo     codeinfo;       /* structure containing SMF status codes
					   to be disabled (i.e. not logged) */
    
    PGSt_SMF_code       flag;           /* numeric flag used to indicate logging
					   status */

    PGSt_SMF_status     callerID;       /* caller ID, copy of SMF static
					   variable used by other Toolkit
					   sub-systems (e.g. IO, PC) to
					   determined if they are being called
					   by and SMF routine (in which case
					   they behave differently in order to
					   avoid conflicts in the interaction
					   between SMF and these modules) */
    PGSt_SMF_status     code;           /* value of status code read from PCF */
    PGSt_SMF_status     returnStatus;   /* return status of Toolkit function
					   calles */

    size_t              check;          /* used to check the return value of
					   sscanf() */

#ifdef _PGS_THREADSAFE
    char *lasts;                        /* used by strtok_r() */
#endif
    
    /* if this is NOT the first time this function is being called, return
       immediately */

    if (first != PGS_TRUE)
    {
	return;
    }
    
    /* set first to be false so that the following code is never executed again
       in this process */

    first = PGS_FALSE;
    
    /* save the current value of the SMF static variable "callerID" (defined
       elsewhere) in our local copy since we may be altering the SMF value and
       we want to restore it to its original value when we are done */

    callerID = PGS_SMF_CallerID();

    /* set the SMF static variable "callerID" (defined elsewhere) to indicated
       that calls to PC or IO routines are being done from within and SMF
       function */

    PGS_SMF_SetCallerID(PGSd_CALLERID_SMF);

    /* get the LOGGING flag from the PCF, if it is 0 then disable logging */

    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_LOGGING, buf);
    if (returnStatus == PGS_S_SUCCESS)
    {
	check = sscanf(buf,"%d", &flag);
	
	/* check if the flag is zero AND that the value of the flag is not some
	   default value but was properly determined by the sscanf() call above
	   (default behavior is to have logging enabled) */ 

	if (check == 1 && flag == 0)
	{
	    PGS_SMF_LoggingControl(PGSd_DISABLE_ALL, (int) NULL);
	}
    }
    
    /* get the TRACE flag from the PCF, if it is 0 then don't any tracing
       (default behavior), if it is 1 then do error tracing, if it is 2 then do
       full tracing */

#ifdef _PGS_THREADSAFE
    /* There will be NO TRACING when in threadsafe mode */
    PGS_SMF_TraceControl(PGSd_SET_NO_TRACE);
#else
    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_TRACE, buf);
    if (returnStatus == PGS_S_SUCCESS)
    {
	check = sscanf(buf,"%d", &flag);
	if (check == 1)
	{
	    switch (flag)
	    {
	      case 1:
		PGS_SMF_TraceControl(PGSd_SET_ERROR_TRACE);
		break;
		
	      case 2:
		PGS_SMF_TraceControl(PGSd_SET_FULL_TRACE);
		break;
		
	      case 0:
	      default:
		PGS_SMF_TraceControl(PGSd_SET_NO_TRACE);
		break;
	    }
	}
    }
#endif
    
    /* get the PID LOGGING flag from the PCF, if it is 1 then include the
       process ID with SMF logging entries (this is useful for multiple
       processes writing simultaneously to the same log file--not recommended
       but here made a little more easy to deal with), default behavior is to
       NOT include the PID with log entries */

    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_PID, buf);
    if (returnStatus == PGS_S_SUCCESS)
    {
	check = sscanf(buf,"%d", &flag);
	if (check == 1)
	{
	    switch (flag)
	    {
	      case 1:
		PGS_SMF_LogPID(PGSd_ENABLE_PID_LOGGING);
		break;
		
	      case 0:
	      default:
		PGS_SMF_LogPID(PGSd_DISABLE_PID_LOGGING);
		break;
	    }
	    }
    }
    
    /* get the list of SMF status levels for which logging is to be disabled */

    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_LEVEL, buf);
    if (returnStatus == PGS_S_SUCCESS)
    {
#ifdef _PGS_THREADSAFE
        /* strtok() is not threadsafe - use strtok_r() */
	chrPtr = strtok_r(buf, " ,\t",&lasts);
#else
	chrPtr = strtok(buf, " ,\t");
#endif
	while (chrPtr != NULL)
	{
	    switch (*chrPtr)
	    {
	      case 'S':
		PGS_SMF_LoggingControl(PGSd_DISABLE_STATUS_LEVEL,
				       PGS_SMF_MASK_LEV_S);
		break;
		    
	      case 'A':
		PGS_SMF_LoggingControl(PGSd_DISABLE_STATUS_LEVEL,
				       PGS_SMF_MASK_LEV_A);
		break;
		
	      case 'M':
		PGS_SMF_LoggingControl(PGSd_DISABLE_STATUS_LEVEL,
				       PGS_SMF_MASK_LEV_M);
		break;
		
	      case 'U':
		PGS_SMF_LoggingControl(PGSd_DISABLE_STATUS_LEVEL,
				       PGS_SMF_MASK_LEV_U);
		break;
		
	      case 'N':
		PGS_SMF_LoggingControl(PGSd_DISABLE_STATUS_LEVEL,
				       PGS_SMF_MASK_LEV_N);
		break;
		
	      case 'W':
		PGS_SMF_LoggingControl(PGSd_DISABLE_STATUS_LEVEL,
				       PGS_SMF_MASK_LEV_W);
		break;
		
	      case 'E':
		PGS_SMF_LoggingControl(PGSd_DISABLE_STATUS_LEVEL,
				       PGS_SMF_MASK_LEV_E);
		break;
		
	      case 'F':
		PGS_SMF_LoggingControl(PGSd_DISABLE_STATUS_LEVEL,
				       PGS_SMF_MASK_LEV_F);
		break;
		
	      case 'C':
		PGS_SMF_LoggingControl(PGSd_DISABLE_STATUS_LEVEL,
				       PGS_SMF_MASK_LEV_C);
		break;
		
	      default:
		break;
	    }
#ifdef _PGS_THREADSAFE
            /* strtok() is not threadsafe - use strtok_r() */
	    chrPtr = strtok_r(NULL, " ,\t",&lasts);
#else
	    chrPtr = strtok(NULL, " ,\t");
#endif
	}
    }
    
    /* get the list of SMF seed numbers, the SMF status codes derived from which
       are to have logging disabled */

    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_SEED, buf);
    if (returnStatus == PGS_S_SUCCESS)
    {
#ifdef _PGS_THREADSAFE
        /* strtok() is not threadsafe - use strtok_r() */
	chrPtr = strtok_r(buf, " ,\t",&lasts);
#else
	chrPtr = strtok(buf, " ,\t");
#endif
	while (chrPtr != NULL)
	{
	    check = sscanf(chrPtr, "%d", &flag);
	    if (check == 1)
	    {
		PGS_SMF_LoggingControl(PGSd_DISABLE_SEED, flag);
	    }
#ifdef _PGS_THREADSAFE
            /* strtok() is not threadsafe - use strtok_r() */
	    chrPtr = strtok_r(NULL, " ,\t",&lasts);
#else
	    chrPtr = strtok(NULL, " ,\t");
#endif
	}
    }
    
    /* get the list of SMF status codes for which logging is to be disabled, the
       codes are actually being read in from the PCF as their associated
       mnemonic strings and are here converted to the appropriate numerical
       values */

    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_CODE, buf);
    if (returnStatus == PGS_S_SUCCESS)
    {
	/* for reasons unknown to me strtok was not working here */
	/* (some sort of memory access error in PGS_SMF_DecodeCode?) */

	chrPtr = buf;
	while (*chrPtr != (char) NULL)
	{
	    check = sscanf(chrPtr, "%s", next_item);
	    if (check == 0)
	    {
		break;
	    }
	    
	    if (strchr(next_item,',') != (char*) NULL)
	    {
		*(strchr(next_item,',')) = '\0';
	    }
	    
	    chrPtr = chrPtr + strlen(next_item);

	    while (*chrPtr == ',' || *chrPtr == ' ' || *chrPtr == '\t')
	    {
		chrPtr++;
	    }

	    /* check to see if "next_item" is a number; if it is, assume that
	       the item is a status code and NOT a status mnemonic */

	    code = (PGSt_SMF_status) strtoul(next_item, (char**) NULL, 10);
	    if ( code != 0 )
	    {
		/* next_item was a number, no need to attempt to decode it as a
		   status mnemonic, just disable this code and get the next
		   token */

		PGS_SMF_LoggingControl(PGSd_DISABLE_CODE, code);
		continue;
	    }
	    
	    /* retrieve the SMF information associated with the mnemonic
	       (contained in "next_item") parsed from the PCF, note that to do
	       this we are attempting to decode the mnemonic string--this
	       ONLY works for Toolkit mnemonics */

	    returnStatus = PGS_SMF_DecodeCode((int) NULL, next_item,
					      &codeinfo,
					      "PGS_SMF_InitializeLogging()",
					      1);
	    
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		PGS_SMF_LoggingControl(PGSd_DISABLE_CODE,
				       codeinfo.msgdata.code);
	    }
	}
    }

    /* reset the SMF static variable "callerID" (defined elsewhere) to its value
       prior to the execution of this function (as saved in the local variable
       callerID) */

    PGS_SMF_SetCallerID(callerID);
    return;
}
