/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_SMF.c

DESCRIPTION:
  This file contains the common error handling functions for PGS.
  The routines are:
    PGS_SMF_SetUNIXMsg()         
    PGS_SMF_SetStaticMsg()       
    PGS_SMF_SetDynamicMsg()       
    PGS_SMF_GetActionByCode()     
    PGS_SMF_GetMsgByCode()        
    PGS_SMF_GetMsg()              
    PGS_SMF_CreateMsgTag()        
    PGS_SMF_GetInstrName()        
    PGS_SMF_GenerateStatusReport()  
    PGS_SMF_TestErrorLevel() 
    PGS_SMF_TestMessageLevel()         
    PGS_SMF_TestFatalLevel()       
    PGS_SMF_TestWarningLevel()     
    PGS_SMF_TestUserInfoLevel()     
    PGS_SMF_TestSuccessLevel()      
    PGS_SMF_TestNoticeLevel()       
    PGS_SMF_TestStatusLevel()       

AUTHOR:
  Kelvin K. Wan      / Applied Research Corp.
  David P. Heroux    / Applied Research Corp.
  Michael E. Sucher  / Applied Research Corp.
  Guru Tej S. Khalsa / Applied Research Corp.
  Carol S. W. Tsai   / Space Applications Corporation
  Ray Milburn        / Steven Myers & Associates
  Phuong T. Nguyen   / Emergent Information Technology, Inc.
HISTORY:
  25-Mar-1994 KCW Standard Convention  
  24-Aug-1994 KCW Update to include code inspection comment
  14-Oct-1994 KCW Include new technique on PGS_SMF_CallerID() 
  22-Dec-1994 KCW Enhanced to include shared memory for TK4
  05-Jan-1995 DPH Commented out setting of System Signal Handling Func.

  22-Mar-1995 MES Split PGS_SMF.c into individual files for each tool 
  03-May-1995 MES Recombined separate files into new PGS_SMF.c

                  New functionality includes:

                    1) Message caching in shared memory region
                    2) New high level tools PGS_SMF_Begin and PGS_SMF_End 
                       allow log file indenting and selective enabling and 
                       disabling of message logging
                    3) Repetitive status message compression in log files

                  The following modules were added:

                    PGS_SMF_CacheMsgShm
                    PGS_SMF_GetShmSize
                    PGS_SMF_GetUserCode
                    PGS_SMF_Begin
                    PGS_SMF_End
                    PGS_SMF_MsgLevel

                  The following modules were updated:

                    PGS_SMF_CorrectFuncName
                    PGS_SMF_DecodeCode
                    PGS_SMF_ExtractFileInfo
                    PGS_SMF_ExtractMsgInfo
                    PGS_SMF_GetCodeFile
                    PGS_SMF_GetGlobalVar
                    PGS_SMF_GetSysShm
                    PGS_SMF_HandleLogErr
                    PGS_SMF_IsSystemCode
                    PGS_SMF_SetShmMemData
                    PGS_SMF_SetShmMemToGlbVar
                    PGS_SMF_WriteBanner
                    PGS_SMF_WriteLogFile (major rewrite)

  29-Jun-1995 MES Updated PGS_SMF_DecodeCode and PGS_SMF_GetUserCode
                  to fix problem with PGS_SMF_GetActionByCode.
  10-Jul-1995 MES Updated PGS_SMF_WriteLogFile repetitive message
                  compression logic to handle the case where function
                  name and code are the same, but the message text is 
                  different.  This can happen when the SMF tool 
                  PGS_SMF_SetDynamicMsg is being used.

  12-Jul-1995 MES Updated REQUIREMENTS AND FORTRAN EXAMPLES sections
                  in tool prologs.

  22-Sep-1995 MES Added function PGS_SMF_GetActionMneByCode, in preparation
                  for MSS interface.  Returns action mnemonic and message.
                  Fixed bug in PGS_SMF_CacheMsgShm that caused it to crash
                  when called outside of a PGE (shared memory disabled).
                  
  26-Sep-1995 MES Added functions PGS_SMF_GetActionType and PGS_SMF_LogEvent,
                  to implement the MSS event logger interface.
                  Modifed PGS_SMF_WriteLogFile to call PGS_SMF_LogEvent.
                  NOTE:
                  Mods to PGS_SMF_WriteLogFile and all of PGS_SMF_LogEvent
                  use conditional compilation.  The flag -DPGS_IR1 must be
                  specifed on the compilation command line for these
                  modifications to take effect.

  12-Oct-1995 DPH Modified function PGS_SMF_DecodeCode() to correct 
		  typo in Toolkit SMF file labels: GPGSCT -> PGSGCT &
		  added label for MET tools into seed array.

		  Used newly created constant identifier for number of 
		  SMF files used by the Toolkit.

  13-Oct-1995 DPH Modified PGS_SMF_HandleLogErr() to set 'msg' variable
	   	  when the Termination command is running; Log files are
		  no longer open so the existing messages made little 
		  sense. This change is made more for documentation	
		  purposes than anything else since the 'msg' string
		  does not come into play for the remainder of processing.

	          Added switching logic to PGS_SMF_WriteLogFile to toggle 
		  CSMS Event Logging functionality on and off through a 
		  PCF setting.

		  Fixed PGS_SMF_LogEvent to retrieve MSS Event Log from 
		  PCF.

		  Introduced additional SMF status to deal with bad
		  MSS Event Log reference; PGS_SMF_GetSMFCode updated.

  16-Oct-1995 DPH Modified PGS_SMF_GetActionType() to check for SMF level
		  'C' instead of 'A'. This modification was performed to 
		  help users differentiate between standard Action codes
	          and Channel Action codes which support SNMP communication
		  methods.

  17-OCT-1995 DPH Corrected some logic in PGS_SMF_WriteBanner() which 
		  prevented the function from initializing the files.

  18-Oct-1995 DPH Modified PGS_SMF_LogEvent() interface to support
                  additional argument for action string. 
	          
		  PGS_SMF_WriteLogFile() know supports an additional 
		  return value PGSSMF_E_MSSLOGFILE.

                  Modified PGS_SMF_HandleLogErr() to support both 
		  types of LOG errors; made the necessary changes 
		  where this function was called: *Set*Msg*().

  19-Oct-1995 DPH Modified PGS_SMF_GetSysShm() to initialize the 
		  MSS static global variables to support Event Logging.

		  Updated PGS_SMF_GetGlobalVar() to support new MSS
		  return value.

                  Added global variables to maintain initialized
                  values for SNMP channel activation switch and
                  MSS event log reference. ( [TK6] - insert these
                  globals into the Global SMF structure
                  PGSSmfGlbVar.PGSSmfLog)

		  Call to PGS_SMF_SetStaticMsg() was removed 
	          from PGS_SMF_GetActionMneByCode() to correct for 
	    	  side-effect which resets the value of 'code' in 
		  the global substructure 'msgdata' to 0 
		  (i.e. PGS_S_SUCCESS). 

		  Discovered that global substructure 'msgdata'
		  gets rewritten to hold that contents of Action
		  information whenever there is an associated 
		  action mnemonic as part of the status record.
		  While this action provides a quick and dirty
		  means for the action processing code to function
		  properly, it unfortunately causes other code to
		  offer unexpected results; namely calls to 
		  PGS_SMF_GetMsg() return the Action information, 
		  instead of the anticipated Status information.
		  While this does not affect the operation of the 
		  user's software, it may be disconcerting if one
		  were to make this call. (note that the Status Log
		  retrieves the proper information due to the fact
		  that this log is written prior to the exchange of
		  information in the global area.

  20-Oct-1995 DPH Moved the following routine declarations to the
		  include file: PGS_SMF_LogEvent()
				PGS_SMF_GetActionType()
				PGS_SMF_GetActionMneByCode()

		  Introduced additional MSS Code type 'INFO' to 
		  support non-email communications.

		  Fixed call to PGS_SMF_LogEvent in 
		  PGS_SMF_WriteLogFile(); msg/mnemonic fields
		  were swapped.

		  The routine PGS_SMF_GetMSSGlobals() was added
		  to allow outside routines to access the Global
		  MSS attributes which control Event Logging
		  operations.

  31-Oct-1995 GSK Fixed function PGS_SMF_GetSysShm() which was failing to return
                  any value at all under certain circumstances.  Fixed several
		  other minor "bugs" (e.g. a statement ending with ;;) and
		  typos.

  16-Nov-1995 MES Revised header files inclusion section to include 
                  cfortran.h AFTER the Event Logger header file is 
		  included, adding a #ifdef _INT block to undefine
		  this macro because it clashes with the cfortran.h
		  definition.   This was necessary to get the DAAC
		  version of SMF to compile on the SGI Challenge
		  in -n32 and -64 compiler modes.

  28-Nov-1995 DPH Corrected problem in PGS_SMF_GetActionMneByCode() 
                  which indirectly caused the static global structure
                  'globalvar' to become overwritten, with the status
                  information for PGSSMF_W_NOACTION, when Status 
                  conditions were encountered which had no associated
                  Action conditions. While this problem did not affect
                  the Status codes returned to the user's SW, nor
                  prevent the Status Log from being properly updated,
                  it did result in a loss of Status information, which
                  prevented subsequent retrieval of such information
                  by way of calls to PGS_SMF_GetMsg().
                  
  29-Nov-1995 DPH Altered the input function argument in calls to
                  PGS_SMF_DecodeCode() from PGS_SMF_GetActionByCode()
                  and PGS_SMF_GetActionMneByCode() so that the 
                  original function name (as stored in the global
                  buffer) is preserved when the function returns.
                  This measure was necessary due to the fact that
                  PGS_SMF_DecodeCode() is called twice each time
                  a status condition has been set.

  13-Dec-1995 MES Revised PGS_SMF_LogEvent to use different disposition
                  codes and message formatting to handle different event
                  types.  This required a major rewrite of the logic.  
                  The high severity disposition code is no longer used.  
                  Also, error checking was added to the event logger calls.

  14-Dec-1995 DPH Modified PGS_SMF_GetSMFCode() to maintain the proper 
                  mnemonic for Unix errors. The code PGSSMF_M_UNIX acts
                  as a surrogate for PGS_E_UNIX and is never returned from
                  an SMF, or other Toolkit function. Recent modifications 
                  to support Event Logging inadvertantly caused the 
                  mnemonic field of the Global SMF structure to be 
                  overwritten with the mnemonic for PGSSMF_M_UNIX. 
                  Since only the code for this status is necessary 
                  (see PGS_SMF_GetMsg for more details), this mnemonic
                  was replaced with that for PGS_E_UNIX. 

                  The banner for each of the Log files was corrected
                  to generate the proper file name for each; 'LogReport'
                  was being issued for each previously.

  18-Dec-1995 MES Revised PGS_SMF_LogEvent event logger error checking
                  to reflect updated return status codes.

  28-Dec-1995 MES Revised PGS_SMF_GetSysShm so that it does not 
                  overwrite the cache tracking variables when it 
                  updates the shared memory structure.

                  Revised PGS_SMF_GetGlobalVar to transparently 
                  pass back the PGS_SH_SMF_MSSLOGFILE return status  
                  from PGS_SMF_GetSysShm, because may need to be  
                  checked by the calling function.  Also modified to 
                  accept a null pointer argument, in which case no 
                  attempt is made to update the caller's global 
                  variable pointer.

  04-Dec-1996 GSK Removed inclusion of cfortran.h header file and all cfortran.h
                  macros (used to create FORTRAN bindings).  These macros were
		  all moved to their own file, PGS_SMF_bindFORTRAN.c.  This was
		  done to minimize conflict between various macros defined in
		  the file cfortran.h (there are THOUSANDS in there) and those
		  defined by system header files (since SMF utilities may
		  require these header files but the header files are not
		  required to construct the FORTRAN bindings).

		  Added the ability to switch off message logging (i.e. logging
		  SMF messages to the log file (on disk), internal message
		  buffering (setting and getting of messages) is not turned off
		  when this option is used).
  19-May-1998 CST Changed the length of character string from PGS_SMF_MAX_MSG_SIZE
                  defined as 241 to PGS_SMF_MAX_MSGBUF_SIZE defined as 481 for the
                  variables, msg, prevAction, prevMsg, and subject, defined to
                  hold the message (This changing is for ECSed14746 about SDPTK5.2.1
                  limits on user-defined log messages greater than 275 chars)
  07-Jul-1999 RM  Updated for TSF functionality
  13-Nov-2001 PTN Modified to have the notice of processing logs while PCF 
                  files are running and having problems for creating LogStus, 
                  LogReport and LogUser (ECSed26769).
  03-Feb-2005 MP  Modified to use strerror instead of sys_errlist for Linux 
                  (ECSed41850)
              
END_FILE_PROLOG:
*****************************************************************/


/*
 * System Headers 
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <PGS_math.h>

/*
 * Toolkit Headers 
 */
#include <PGS_SMF.h>
#include <PGS_MEM.h>
#include <PGS_MEM1.h>
#include <PGS_IO_Gen.h>
#include <PGS_EcsVersion.h>
#include <PGS_TSF.h>
#include <math.h>

#ifdef PGS_IR1
/*
 * MSS Event Logger Headers
 */

#include <EcUtLoggerC.h>

static char     eventLogRef[PGSd_PC_FILE_PATH_MAX]="";
static char     comm_snmp[PGSd_PC_VALUE_LENGTH_MAX]=PGSd_SMF_OFF;
                                                    /* SNMP comm flag */
#endif /* PGS_IR1 */


/*
 * Static Variables
 */

static PGSt_SMF_status callerID=PGSd_CALLERID_USR;        /* callerID to stop
							     cyclic situation */

static PGSt_SMF_status writeLogFile=PGSd_SMF_WRITELOG_ON; /* flag to capture
							     errors to log
							     file */

static PGSt_SMF_status useMemType=PGSd_SMF_USE_SHAREDMEM; /* flag to use either
							     shared memory or
							     ASCII file */

static PGSt_SMF_status whichProc=PGSd_SMF_PROC_USER;      /* which process are
							     we in? (init., PGE,
							     or term.) */

/*
 * Private Low-level Function Prototypes 
 */

PGSt_SMF_boolean PGS_SMF_IsSystemCode(
    PGSt_SMF_code );

PGSt_SMF_status  PGS_SMF_WriteLogFile(
    PGSSmfGlbVar *);

void             PGS_SMF_HandleLogErr(
    PGSSmfGlbVar *);

PGSt_SMF_status  PGS_SMF_SetShmMemData(
    PGSSmfShm *);

void             PGS_SMF_SetShmMemToGlbVar(
    PGSSmfShm *,
    PGSSmfGlbVar *);

void             PGS_SMF_WriteBanner(
    PGSSmfShm *);

void             PGS_SMF_CorrectFuncName(
    char *,
    char *);

char *PGS_SMF_GetThreadID();

PGSt_SMF_status  PGS_SMF_GetUserCode(
    PGSt_SMF_code, 
    char *,
    PGS_SMF_MsgInfo *,
    char *, short);

PGSt_SMF_status  PGS_SMF_CacheMsgShm(
    int, 
    PGSt_SMF_code, 
    char *, 
    PGS_SMF_MsgInfo *);

PGSt_SMF_status  PGS_SMF_CacheMsgDynm(
    int, 
    PGSt_SMF_code, 
    char *, 
    PGS_SMF_MsgInfo *);

PGSt_SMF_status  PGS_SMF_MsgLevel(
    PGSt_integer,
    PGSt_integer *,
    PGSt_integer *,
    char **,
    char **);

PGSt_integer PGS_SMF_FEQ(
      PGSt_double, PGSt_double );

/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_Begin

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_Begin(
        char *funcname);


FORTRAN:
    implicit none
    integer pgs_smf_begin

    include 'PGS_SMF.f'

    integer function pgs_smf_begin(funcname)
    character*100 funcname
    

DESCRIPTION:
    A call to this tool signals to SMF that a function has started,
    and thus, the current message indent level should be incremented.

INPUTS:   
    Name        Description                               
    
    funcname	The name of the function which calls this routine.


OUTPUTS:
    NONE


RETURNS:       
    PGS_S_SUCCESS

EXAMPLES:        

    C:
    ==
    PGSt_SMF_status returnStatus;    
    returnStatus = PGS_SMF_Begin("CallingFunction");


    FORTRAN:
    ========
    integer returnStatus
    returnStatus = pgs_smf_begin('CallingFunction')


NOTES:           
    A message will be written to the status log file indicating that
    the specified function has started.

REQUIREMENTS:
    PGSTK-0580,0590,0650,0663

DETAILS:	   

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar
    PGS_SMF_MsgLevel
    PGS_SMF_WriteLogFile

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_Begin(
    char *funcname)
{
    PGSt_SMF_status returnStatus;

    static char     *toolname = "PGS_SMF_Begin";

    char            buf[200];

    PGSSmfGlbVar    *global_var;    /* global static buffer */

    PGSt_integer    msg_level;      /* message level */
    PGSt_integer    log_off1;       /* level logging disabled flag */
    char            *indent_str;    /* indent string */
    char            *func_str;      /* function name string */
    char            *log_str;       /* logging status string */

    size_t          indent_length;
    size_t          i;

    /* 
     * Get the pointer to the global variable
     */

    PGS_SMF_GetGlobalVar(&global_var);
    
    /*
     * Check to see if tracing has been enabled, if it has NOT been enabled,
     * exit now.
     */

    returnStatus = PGS_SMF_TraceControl(PGSd_QUERY);
    if (returnStatus == PGSSMF_M_TRACE_DISABLED)
    {
	return PGS_S_SUCCESS;
    }
    
    /* 
     * Set the new message level
     */

    PGS_SMF_MsgLevel(PGSd_SMF_IncLevel, 0, 0, 0, &funcname);

    if (returnStatus != PGSSMF_M_FULL_TRACE_ENABLED)
    {
	return PGS_S_SUCCESS;
    }
    
    returnStatus = PGS_SMF_LoggingControl(PGSd_QUERY_ALL, (PGSt_SMF_code) NULL);
    if (returnStatus == PGSSMF_M_LOGGING_DISABLED)
    {
	return PGS_S_SUCCESS;
    }

    /*
     * Get the message level, logging flag, 
     * indent string and function string
     */

    PGS_SMF_MsgLevel(PGSd_SMF_GetLevel, &msg_level, &log_off1, &indent_str,
		     &func_str);

    /* 
     * Write the Begin Level message to the log file(s)
     * 
     * We do this by temporarily setting the function name in the
     * global variable to the name of this tool, and then calling
     * PGS_SMF_WriteLogFile.  Afterwards, we restore the old function 
     * name.
     */

    if (global_var->log.fptrStatus == (FILE *)NULL)
    {      
        global_var->msginfo.fileinfo.seed = PGS_SMF_SYS_SEED;
        strcpy(global_var->msginfo.fileinfo.instr,PGS_SMF_SYS_INSTR);
        strcpy(global_var->msginfo.fileinfo.label,PGS_SMF_SYS_LABEL);
        global_var->msginfo.msgdata.action[0] = '\0';
        global_var->msginfo.msgdata.code = PGSSMF_E_LOGFILE;
        PGS_SMF_GetSMFCode(PGSSMF_E_LOGFILE,
                           global_var->msginfo.msgdata.mnemonic,
                           global_var->msginfo.msgdata.msg);
        strcpy(global_var->msg,"Error opening status log file");
        return PGSSMF_E_LOGFILE;
    }
    else
    {        
        if (log_off1 == 0)
        {
            log_str = "";     /* "(logging enabled)"; */
        }
        else
        {
            log_str = " (status logging disabled)";  /* "(logging disabled)"; */
        }

        if (msg_level > 0)    /* derive indent string for previous level */
        {
            indent_length = strlen(indent_str);
            if (indent_length > 0)
            {
                strcpy(buf, indent_str);
                i = indent_length / msg_level;
                buf[indent_length-i] = 0;
            }
        }

        fprintf(global_var->log.fptrStatus,"%s%s: %s%s%s\n\n",
                buf, toolname, func_str, log_str, PGS_SMF_LogPID(PGSd_QUERY));
        fflush(global_var->log.fptrStatus);
    }

    return PGS_S_SUCCESS;
} /* end: PGS_SMF_Begin */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CacheMsgShm

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_CacheMsgShm (
        int               cmd,
        PGSt_SMF_code     code,
        char              *mnemonic,
        PGS_SMF_MsgInfo   *codeinfo);

FORTRAN:
    NONE

DESCRIPTION:
    This tool implements a cache for return status code information
    that is stored in the SMF message files.  It was added with version
    5 of the toolkit.  In normal use, it will only be called by the 
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
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:
    PGSt_SMF_status   returnStatus;
    PGS_SMF_MsgInfo   msg_info;
    PGSt_SMF_code     code,
    char buf[PGSd_SMF_MsgBufColumns];

    returnStatus = PGS_SMF_CacheMsgShm(
                            PGSd_SMF_FindByCode, code, 0, &msg_info);

NOTES: 
    1)  This is a low-level tool.  It is only intended to be called by
        toolkit code.

    2)  Real shared memory must be active for this tool to work.  If it
        is not available, the tool simply returns PGSSMF_M_NOT_IN_CACHE.

    3)  In order for the the tool to be properly initialized, it must be
        invoked as follows (this is done automatically by PGS_PC_InitCom):

        - first call within a PGE MUST be with 'cmd' set to the value 
          PGSd_SMF_InitCacheInfo1, with the size of the cache in records 
          passed in parameter 'code'

        - second call within a PGE MUST be from same process that made the 
          first call, with 'cmd' set to the value PGSd_SMF_InitCacheInfo2

    4)  Commands PGSd_SMF_LastFindStatus and PGSd_SMF_DisplayBuffer are 
        included for test purposes only.  In normal use, they will not be
        called.

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
    useMemType

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_TSF_LockIt
    PGS_TSF_UnlockIt
    PGS_SMF_TestErrorLevel

END_PROLOG:
*****************************************************************/

PGSt_SMF_status 
PGS_SMF_CacheMsgShm (
    int cmd,
    PGSt_SMF_code code,
    char *mnemonic,
    PGS_SMF_MsgInfo  *codeinfo)        /* code information */
{
    PGSt_SMF_status        returnStatus = PGS_S_SUCCESS; /* return status */

    int i;                               /* loop count variable */

    static char             *smf_base;   /* SMF shared memory base */
    static PGSt_uinteger    smf_size;    /* SMF shared memory size */

    static char             *cache_base; /* message cache base */
    static PGSt_uinteger    cache_count; /* count of records allocated */
    static PGSt_integer     newest;      /* index to newest entry */
    static PGSt_integer     oldest;      /* index to oldest entry */

    static PGS_SMF_MsgInfo  *msg_buf;    /* message cache structure pointer */
    static PGSSmfShm        *shm_ptr;    /* shared memory structure pointer */

    static int process_init = 1;         /* process initialization flag */
    static int pge_init     = 0;         /* PGE     initialization flag */

    static int cache_hit    = 0;         /* cache hit flag */

    static PGSt_uinteger    access_count=0; /* count of cache accesses */

    /*  
     * TWO-PART PGE INITIALIZATION PHASE
     * 
     * This takes place ONLY during the run of PGS_PC_InitCom.
     * 
     * Part One happens when PGS_PC_InitCom calls PGS_SMF_GetShmSize.  
     * 
     * Part two happens when PGS_PC_InitCom calls PGS_SMF_GetSysShm.
     * 
     */

    if (cmd == PGSd_SMF_InitCacheInfo1)
    { 
          /* 
             * PGE INITIALIZATION PHASE: PART ONE
             * 
             * Set the initial values of cache tracking variables
             * 
             * This section MUST be called, ONCE and ONLY ONCE, 
             * before any other operations may take place.
             * 
             * The ONLY time this happens is during the call from 
             * PGS_SMF_GetShmSize, during the run of PGS_PC_InitCom.
             */

            cache_count = (PGSt_uinteger) code;
            newest      = -1;
            oldest      = -1;
            pge_init    =  1;   /* flag PGE Initialiazation Phase: Part 2 */

            goto DONE;

    } /* END: if (cmd == PGSd_SMF_InitCacheInfo1) */

 
    if ( (cmd == PGSd_SMF_InitCacheInfo2) && (pge_init == 1) )
    { 
        /*  
         * PGE INITIALIZATION PHASE: PART TWO
         * 
         * Copy the initial values of cache tracking variables
         * into shared memory.
         * 
         * The ONLY time this happens the first call AFTER the
         * call from PGS_SMF_GetShmSize,  This happens when
         * PGS_PC_InitCom calls PGS_SMF_GetSysShm.
         */

        if (useMemType == PGSd_SMF_USE_SHAREDMEM)
        {
            /* 
             * Get pointer to shared memory structure 
             */

            returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_SMF, 
                               (void **) &smf_base, &smf_size);
            shm_ptr = (PGSSmfShm *) smf_base;   /* get struct pointer */

            /* 
             * Store the initial values
             */

            shm_ptr->msgCacheInfo.count = cache_count;
            shm_ptr->msgCacheInfo.newest = newest;
            shm_ptr->msgCacheInfo.oldest = oldest;

            pge_init = 0;    /* clear the PGE initialization flag */
            goto DONE;
        }
        else
        {
            /*
             * Without shared memory, the cache is not supported.
             */

            returnStatus = PGSSMF_M_NOT_IN_CACHE;
            goto DONE;

         } /* END: if (useMemType == PGSd_SMF_USE_SHAREDMEM) */


    } /* END: if ((cmd == PGSd_SMF_InitCacheInfo2) ... */


    /*
     * Without shared memory, the cache is not supported.
     * Tell the caller to find the message somewhere else.
     */

    if (useMemType != PGSd_SMF_USE_SHAREDMEM)  /* shared memory N/A */
    {
        returnStatus = PGSSMF_M_NOT_IN_CACHE;
        goto DONE;
    }

    /*  
     * Process initialization
     * 
     * This takes place at the start of each user PGE process, following
     * the PGE initialization phase handled by PGS_PC_InitCom
     */

    if (process_init == 1)
    {
        /*
         * Since shared memory is available, we get a pointer to the base
         * of SMF shared memory section of system shared memory.  Then, we
         * determine the base of the cache area.
         */

        returnStatus = PGS_MEM_ShmSysAddr(PGS_MEM_SHM_SMF, 
                               (void **) &smf_base, &smf_size);

        /*
         * Check again to make sure that useMemType didn't change
         */
        if (useMemType != PGSd_SMF_USE_SHAREDMEM)  /* shared memory N/A */
        {
            returnStatus = PGSSMF_M_NOT_IN_CACHE;
            goto DONE;
        }
        else if (returnStatus != PGS_S_SUCCESS)
        {
	    goto DONE;
        }

        cache_base = smf_base + sizeof(PGSSmfShm);

        shm_ptr = (PGSSmfShm       *) smf_base;   /* SmfShm struct pointer */
        msg_buf = (PGS_SMF_MsgInfo *) cache_base; /* cache  struct pointer */

        /* 
         * Get cache tracking variables from shared memory
         * 
         * This happens during all initializations EXCEPT
         * the one immediately following the call from 
         * PGS_SMF_GetShmSize, during the run of PGS_PC_InitCom
         */

        cache_count = shm_ptr->msgCacheInfo.count;
        newest = shm_ptr->msgCacheInfo.newest;
        oldest = shm_ptr->msgCacheInfo.oldest;

        /* 
         * Here, it would be a good idea to do a sanity check
         * to make sure that the shared memory tracking variables
         * were properly initialized.
         * 
         * If the shared memory region is guaranteed to be all zeros,
         * initially, then we can check the value of cache_count
         */

        if (cache_count == 0) 
        {
            returnStatus = PGS_E_TOOLKIT;
            goto DONE;
        }

	process_init = 0;

        if (cmd == PGSd_SMF_InitCacheInfo2) goto DONE;
    }
    

#ifdef _PGS_THREADSAFE
    /*  
     * Since we are modifying shared memory we need to lock on TSF mode
     */
    returnStatus = PGS_TSF_LockIt(PGSd_TSF_SMFWRITELOCK);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
        goto DONE;
    }
#endif
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
	if (oldest == -1) oldest = 0;   /* first time only */

        memcpy( (char *) &msg_buf[newest],   /* copy in record */
		(char *) codeinfo, 
		sizeof(PGS_SMF_MsgInfo) );

	memset( msg_buf[newest].funcname,    /* clear the function name */
                0,
                sizeof(msg_buf[newest].funcname) );

        sprintf(msg_buf[newest].funcname, "%u", access_count);

        /*
         * Update the values of the cache tracking variables in 
         * the shared memory area.
         */

        shm_ptr->msgCacheInfo.newest = newest;
        shm_ptr->msgCacheInfo.oldest = oldest;

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
        

      case PGSd_SMF_DisplayBuffer:          /* Display Contents of Buffer */

        for (i=0; i < cache_count; i++)
        {
	    if ( msg_buf[i].msgdata.mnemonic[0] != 0) /* only non-blank entries */
	    {
                printf("%03d: %d %s %s\n    %d,%s,%s,%s\n    %s\n",
		   i+1,
		   msg_buf[i].fileinfo.seed,
		   msg_buf[i].fileinfo.instr,
		   msg_buf[i].fileinfo.label,

		   msg_buf[i].msgdata.code,
		   msg_buf[i].msgdata.mnemonic,
		   msg_buf[i].msgdata.action,
		   msg_buf[i].msgdata.msg,

		   msg_buf[i].funcname);
            }

	}
	printf("\n");

	printf("newest (%5d): %s\n", 
	       newest + 1, 
	       msg_buf[newest].msgdata.mnemonic);

	printf("oldest (%5d): %s\n", 
	       oldest + 1, 
	       msg_buf[oldest].msgdata.mnemonic);

	printf("cache size: %5d records\n", cache_count);

	printf("\n");
	break;


      case PGSd_SMF_InitCacheInfo1:   /* this is handled elsewhere */
      case PGSd_SMF_InitCacheInfo2:   /* this is handled elsewhere */

	break;


      default:                        /* unsupported command */

        returnStatus = PGS_E_TOOLKIT;
        break;

    }

#ifdef _PGS_THREADSAFE
    /*  
     * Unlock here
     */
    returnStatus = PGS_TSF_UnlockIt(PGSd_TSF_SMFWRITELOCK);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
    }
#endif

/*
 *
 * Common exit point
 *
 */
DONE:

    return returnStatus;
}


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CallerID

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_CallerID(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    Return the static variable status 'callerID'.

INPUTS:    
    Name        Description                               Units        Min        Max       

OUTPUTS:
    Name        Description                               Units        Min        Max 

RETURNS:       
    PGSd_CALLERID_SMF
    PGSd_CALLERID_USR

EXAMPLES:    
    PGSt_SMF_status callerID;

    callerID = PGS_SMF_CallerID();

NOTES:           
    This routine is only to be used internally by ARC toolkit developers.
    It is used to return the static variable 'callerID' to indicate to PC
    and IO tools that SMF is in initilization phase so do not call
    PGS_SMF_Set... tools.

REQUIREMENTS:

DETAILS:	   
    NONE

GLOBALS:
    PGSg_TSF_SMFCallerID

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_TSF_GetMasterIndex

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_CallerID(   /* Get caller ID */
    void)
{    
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retVal;
    int masterTSFIndex;
    extern PGSt_SMF_status PGSg_TSF_SMFCallerID[];

    /* if a problem set to id SMF so we can't enter an endless loop */
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        retVal = PGSd_CALLERID_SMF;
    }
    else
    {
        retVal = PGSg_TSF_SMFCallerID[masterTSFIndex];
    }

    return retVal;
#else
    return callerID;
#endif

} /* end PGS_SMF_CallerID */

void
PGS_SMF_SetCallerID(
    PGSt_SMF_status new_value)
{
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retVal;
    int masterTSFIndex;
    extern PGSt_SMF_status PGSg_TSF_SMFCallerID[];

    /* if a problem set to id SMF so we can't enter an endless loop */
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        PGSg_TSF_SMFCallerID[masterTSFIndex] = PGSd_CALLERID_SMF;
    }
    else
    {
        PGSg_TSF_SMFCallerID[masterTSFIndex] = new_value;
    }

#else
    callerID = new_value;
#endif
}

/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CorrectFuncName

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_CorrectFuncName(
        char *func,
        char *correctFunc);

FORTRAN:
    NONE

DESCRIPTION:
    Make sure that the funcname should end with '()'.

INPUTS:   
    Name         Description                               Units        Min        Max              

    func         function need to correct

OUTPUTS:
    Name         Description                               Units        Min        Max     

    correctFunc  corrected function name

RETURNS:       
    NONE

EXAMPLES:        

NOTES:           
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void
PGS_SMF_CorrectFuncName(   /* Correct func name */
    char *func,            /* Input  function need to correct */
    char *correctFunc)     /* Output corrected function name */ 
{
    if (func == (char *)NULL)
    {
        correctFunc[0] = '\0';
    }
    else if (func[strlen(func) - 1] == ')')
    {            
        sprintf(correctFunc,"%s",func);
    }
    else
    {
        sprintf(correctFunc,"%s()",func);
    }       

} /* end PGS_SMF_CorrectFuncName */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_CreateMsgTag

SYNOPSIS:   
C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_CreateMsgTag(
        char systemTag[]);

FORTRAN:
    integer function pgs_smf_createmsgtag(systemtag)
    char*60 systemtag

DESCRIPTION:

    The tool described here allows the user to generate a runtime specific
    character string which may be useful for tagging important items of
    data. The string contains system defined identifiers which, when combined,
    can be useful for stamping NON-PRODUCT specific data for system
    traceability.

INPUTS:    
    Name            Description                                Units        Min        Max

OUTPUTS:    
    Name            Description                                Units        Min        Max

    systemTag       system defined message string 

RETURNS:
    PGS_S_SUCCESS
    PGSSMF_W_NO_CONSTRUCT_TAG
    PGSSMF_E_BAD_REFERENCE

EXAMPLES:
    C:
    ==
    char            systemTag[PGSd_SMF_TAG_LENGTH_MAX];
    PGSt_SMF_status returnStatus;

    returnStatus = PGS_SMF_CreateMsgTag(systemTag);
    if (returnStatus == PGS_S_SUCCESS)
    {
        /# create message tag successful #/
    }

    FORTRAN:
    ========
    implicit none
    integer pgs_smf_createmsgtag

    char*60 systemTag
    integer returnStatus    

    returnStatus = pgs_smf_createmsgtag(systemTag)
    if (returnStatus .EQ. PGS_S_SUCCESS) then
C       create message tag successful
    endif        

NOTES:  
    Currently, the only system identifiers used are the Science Software
    Configuration ID, and  the Production Run ID.    

REQUIREMENTS:
    PGSTK-0610

DETAILS:	    
    As of TK4, PGSSMF_E_BAD_REFERENCE is no longer return since information will
    be process in the init. process via shared memory.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()    
    PGS_SMF_SetStaticMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_CreateMsgTag(  /* Create message tag */ 
    char systemTag[])  /* Output system defined message string */
{    
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;    /* return status */
    PGSSmfGlbVar    *global_var;                      /* global static buffer */

    PGS_SMF_GetGlobalVar(&global_var);

    if (global_var->log.msgTag[0] == '\0')
    {
        systemTag[0] = '\0';
        returnStatus = PGSSMF_W_NO_CONSTRUCT_TAG;
    }
    else
    {      
        strcpy(systemTag,global_var->log.msgTag);
    }

    PGS_SMF_SetStaticMsg(returnStatus,"PGS_SMF_CreateMsgTag()");        
    return(returnStatus);

} /* end PGS_SMF_CreateMsgTag */
       


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_DecodeCode

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_DecodeCode(
        PGSt_SMF_code     code,
        char             *mnemonic,
        PGS_SMF_MsgInfo  *codeinfo,
        char             *func,
        short             op);

FORTRAN:
    NONE

DESCRIPTION:
    This tool is used to extract all information pertaining to a mnemonic code.

INPUTS:    
    Name        Description                               Units        Min        Max 

    code        numeric value for error/status code generated 
                by message compiler.  Also used to determine
                the file seed number.

    mnemonic    mnemonic label for error/status code created 
                by developers and users

    func        function that called this routine

    op          operation                                              0          1
                 0 == search based on code value
                 1 == search based on code mnemonic

OUTPUTS:    
    Name        Description                               Units        Min        Max 

    codeinfo    code information retrieved

RETURNS:        
    PGS_S_SUCCESS    
    PGS_E_ENV 
    PGS_E_UNIX
    PGSSMF_E_UNDEFINED_CODE   

EXAMPLES:
    PGSt_SMF_status   returnStatus;
    PGS_SMF_MsgInfo   codeinfo, 

    returnStatus = PGS_SMF_DecodeCode(PGS_S_SUCCESS,"",&codeinfo,
                                      "PGS_SMF_GetInstrName()",0);  
    if (returnStatus == PGS_E_ENV)
    {
        /# Environment variable 'PGSMSG' is not set #/
    }

NOTES:          
    There should be an environment variable "PGSMSG" defined to point to 
    the directory that contains all the ASCII message files. 
    It should be available during runtime. If not, then appropriate action
    will be taken.

    Also note that this routine is the heart of SMF; in other words, all SMF
    routines must call this routine to retrieve code information from the 
    ASCII message files.

    This routine makes no attempt to store information into the SMF static 
    buffer, so the calling routine must do it when deemed appropriate.

    When mnemonic search is specified, there are two ways of determining the
    file seed value, which is used to open the correct message file:

     1) If input parameter 'code' is set to a positive number, it will be
        used to derive the file seed value.  This method is used when the 
        tool is called by PGS_SMF_GetActionByCode.

     2) If input parameter 'code' is zero or negative, an internal table
        is consulted.  In this case, only the PGS-defined message files
        will be searched.   If new tool groups are added, this table will 
        have to be updated.  This search capability is not currently 
        supported for SMF or toolkit SYSTEM messages.

REQUIREMENTS:               
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetEnv()
    PGS_SMF_GetSystemCode()   
    PGS_SMF_GetSMFCode()
    PGS_SMF_GetUserCode()
    PGS_SMF_CorrectFuncName()       

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_DecodeCode(               /* Decode code to retrieve information */
    PGSt_SMF_code     code,       /* Input  code */
    char             *mnemonic,   /* Input  mnemonic label */
    PGS_SMF_MsgInfo  *codeinfo,   /* Output code information retrieved */
    char             *func,       /* Input  function that called this routine */
    short             op)         /* Input  operation */
                                  /*    0 == search based on code value */
                                  /*    1 == search based on code mnemonic */
{
    PGSt_SMF_status         returnStatus;                    /* return status */
    char                    msg[PGS_SMF_MAX_MSGBUF_SIZE];       /* code message */
    char                    funcname[PGS_SMF_MAX_FUNC_SIZE]; /* function noted
								this error */
    PGSt_SMF_code           fileno = 0;                      /* file seed 
								number */
    char                    msgbuf[PGS_SMF_MAX_MSGBUF_SIZE]; /* msg buffer */
    char                    *label;                          /* label */

    /*
     * List of tool labels used to derive the value of fileno
     * when mnemonic search is specified.  The index to a given
     * label is equal to seed value for that label.
     * 
     * NOTES:
     *   User supplied tools are not supported.  
     *   If the new toolkit groups are added, this must be updated.
     */ 

    static char *seeds[PGSd_SMF_TOOLKIT_SMFS] = 
    { 
        "PGSxxx", "PGSIO",  "PGSxxx", "PGSTD",  "PGSCSC", 
        "PGSEPH", "PGSCBP", "PGSMEM", "PGSxxx", "PGSPC",  
	"PGSAA",  "PGSCUC", "PGSGCT", "PGSMET", "PGSDEM",
        "PGSTSF"
    };

    /*
     * Make sure that 'funcname' is of format: funcname()
     */  
    PGS_SMF_CorrectFuncName(func,funcname);            

    /*
     * Environment must be set to where the ascii message files are located.
     */
    if ((returnStatus = PGS_SMF_GetEnv(codeinfo,funcname)) == PGS_S_SUCCESS)
    {                  
        /*
         * Get the file seed number. 
         */
        
        if (op == 0) /* derive seed number based on code */
        {
            fileno = (PGSt_SMF_code)(code & PGS_SMF_MASK_SEED) >> 13;
        }
        else   /* derive seed number based on mnemonic, if code not avail. */
        {
            if (code > 0) /* derive seed number based on code */
            {
                fileno = (PGSt_SMF_code)(code & PGS_SMF_MASK_SEED) >> 13;
            }
            else
            {
#ifdef _PGS_THREADSAFE
                /*
                 * Since strtok() is not threadsafe we will use strtok_r when
                 * in threadsafe mode
                 */
                char *lasts;

                strcpy(msgbuf, mnemonic);
                label = strtok_r(msgbuf,"_",&lasts);
#else
                strcpy(msgbuf, mnemonic);
                label = strtok(msgbuf,"_");
#endif

                for( fileno=0; fileno <PGSd_SMF_TOOLKIT_SMFS; fileno++)
                {
                    if( strcmp(seeds[fileno], label) == 0 ) break;
                }
                if (fileno == PGSd_SMF_TOOLKIT_SMFS) 
		{
		  return PGS_E_TOOLKIT;  /* unknown seed */
		}
            }
        }

        /*
         * Initialize data.
         * This effectively destroys the contents of the
         * global status message buffer!!!
         */
        memset((char *)codeinfo,'\0',sizeof(PGS_SMF_MsgInfo));
        strcpy(codeinfo->funcname,funcname);
        codeinfo->msgdata.action[0] = '\0';

        if (fileno == PGS_SMF_SYS_SEED)
        {               
            /*
             * SYSTEM code found.
             */
            PGS_SMF_GetSystemCode(code,codeinfo->msgdata.mnemonic,
                                  codeinfo->msgdata.msg);      
            if (codeinfo->msgdata.mnemonic[0] != '\0')
            {                
                codeinfo->fileinfo.seed = PGS_SMF_SYS_SEED;
                strcpy(codeinfo->fileinfo.instr,PGS_SMF_SYS_INSTR);
                strcpy(codeinfo->fileinfo.label,PGS_SMF_SYS_LABEL);
                codeinfo->msgdata.code = code;
            }
            else
            {
                returnStatus = PGSSMF_E_UNDEFINED_CODE;
            }
        }
        else if (fileno == PGS_SMF_ASSIGN_SEED)
        {
            /*
             * SMF code found.
             */
            PGS_SMF_GetSMFCode(code,codeinfo->msgdata.mnemonic,
                               codeinfo->msgdata.msg);
            if (codeinfo->msgdata.mnemonic[0] != '\0')
            { 
                codeinfo->fileinfo.seed = PGS_SMF_ASSIGN_SEED;
                strcpy(codeinfo->fileinfo.instr,PGS_SMF_SYS_INSTR);
                strcpy(codeinfo->fileinfo.label,PGS_SMF_ASSIGN_LABEL); 
                codeinfo->msgdata.code = code;
            }
            else
            {
                returnStatus = PGSSMF_E_UNDEFINED_CODE;
            }
        }
        else
        {
            /*
             * USER code found.
             */

            returnStatus = PGS_SMF_GetUserCode(code, mnemonic,
                                 codeinfo, funcname, op);
            switch (returnStatus)
            {             
              case PGS_S_SUCCESS:   /* success */
                
                break;


              case PGSSMF_E_UNDEFINED_CODE:   /* code not found */
                
                break;

              case PGS_E_UNIX:      /* message file could not be opened */

                break;

              default:              /* other error: this should never happen */
                break;

            }

        } 

    }

    /*
     * Possible causes of undefined code:
     *                 (1) code does not exist in the message file
     *                 (2) code does not belong to either SYSTEM or SMF 
     */
    if (returnStatus == PGSSMF_E_UNDEFINED_CODE)
    { 
        codeinfo->fileinfo.seed = PGS_SMF_ASSIGN_SEED;
        strcpy(codeinfo->fileinfo.instr,PGS_SMF_SYS_INSTR);
        strcpy(codeinfo->fileinfo.label,PGS_SMF_ASSIGN_LABEL);
        codeinfo->msgdata.code = PGSSMF_E_UNDEFINED_CODE;

        PGS_SMF_GetSMFCode(PGSSMF_E_UNDEFINED_CODE,codeinfo->msgdata.mnemonic,
                           msg);                 
        sprintf(codeinfo->msgdata.msg,msg,code);                
    }  

    return(returnStatus);

} /* end PGS_SMF_DecodeCode */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_End

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_End(
        char *funcname);


FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_end(funcname)
    character*100 funcname
    

DESCRIPTION:
 
    A call to this tool signals to SMF that a function has completed,
    and thus, the current message indent level should be decremented.
    

INPUTS:   
    Name        Description                               
    
    funcname	The name of the function which calls this routine.

OUTPUTS:
    NONE


RETURNS:       
    PGS_S_SUCCESS

EXAMPLES:        

    C:
    ==
    PGSt_SMF_status returnStatus;    
    returnStatus = PGS_SMF_End("CallingFunction");


    FORTRAN:
    ========
    implicit none
    integer pgs_smf_end

    integer returnStatus
    returnStatus = pgs_smf_end('CallingFunction')


NOTES:           
    A message will be written to the status log file indicating that
    the specified function has completed.
    

REQUIREMENTS:
    PGSTK-0580,0590,0650,0663

DETAILS:	   

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar
    PGS_SMF_MsgLevel
    PGS_SMF_WriteLogFile

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_End(
    char *funcname)
{
    PGSt_SMF_status traceStatus;
    PGSt_SMF_status loggingStatus;

    static char     *toolname = "PGS_SMF_End";

    char            buf[200];

    PGSSmfGlbVar    *global_var;         /* global static buffer */

    PGSt_integer    msg_level;      /* message level */
    char            *indent_str;    /* indent string */
    char            *func_str;      /* function name string */

    size_t          indent_length;
    size_t          i;

    /* 
     * Get the pointer to the global variable
     */
    
    PGS_SMF_GetGlobalVar(&global_var);

    /*
     * Check to see if tracing has been enabled, if it has NOT been enabled,
     * exit now.
     */

    traceStatus = PGS_SMF_TraceControl(PGSd_QUERY);
    if (traceStatus == PGSSMF_M_TRACE_DISABLED)
    {
	return PGS_S_SUCCESS;
    }

    loggingStatus = PGS_SMF_LoggingControl(PGSd_QUERY_ALL, (PGSt_SMF_code) NULL);
    
    /* 
     * Write the End Level message to the log file(s)
     * 
     * We do this by temporarily setting the function name in the
     * global variable to the name of this tool, and then calling
     * PGS_SMF_WriteLogFile.  Afterwards, we restore the old function 
     * name.
     */

    if (traceStatus == PGSSMF_M_FULL_TRACE_ENABLED &&
	! (loggingStatus == PGSSMF_M_LOGGING_DISABLED) )
    {
	
	if (global_var->log.fptrStatus == (FILE *)NULL)
	{      
	    global_var->msginfo.fileinfo.seed = PGS_SMF_SYS_SEED;
	    strcpy(global_var->msginfo.fileinfo.instr,PGS_SMF_SYS_INSTR);
	    strcpy(global_var->msginfo.fileinfo.label,PGS_SMF_SYS_LABEL);
	    global_var->msginfo.msgdata.action[0] = '\0';
	    global_var->msginfo.msgdata.code = PGSSMF_E_LOGFILE;
	    PGS_SMF_GetSMFCode(PGSSMF_E_LOGFILE,
			       global_var->msginfo.msgdata.mnemonic,
			       global_var->msginfo.msgdata.msg);
	    strcpy(global_var->msg,"Error opening status log file");
	    return PGSSMF_E_LOGFILE;
	}
	else
	{        
	    /*
	     * Get the message level, logging flag, 
	     * indent string and function string
	     */

	    PGS_SMF_MsgLevel(PGSd_SMF_GetLevel, &msg_level, NULL,
			     &indent_str, &func_str);

	    if (msg_level > 0)    /* derive indent string for previous level */
	    {
		indent_length = strlen(indent_str);
		if (indent_length > 0)
		{
		    strcpy(buf, indent_str);
		    i = indent_length / msg_level;
		    buf[indent_length-i] = 0;
		}
	    }
	    
	    fprintf(global_var->log.fptrStatus,"%s%s: %s%s\n\n",
		    buf, toolname, func_str, PGS_SMF_LogPID(PGSd_QUERY));
	    fflush(global_var->log.fptrStatus);
	}
    }


    /* 
     * Return to the previous message level
     */

    PGS_SMF_MsgLevel(PGSd_SMF_DecLevel, 0, 0, 0, &funcname);
        
    return PGS_S_SUCCESS;
}



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_ExtractFileInfo

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_ExtractFileInfo(
        PGS_SMF_MsgInfo  *codeinfo,
        char             *line);

FORTRAN:
    NONE

DESCRIPTION:
    To extract information from a line retrieved from ASCII message file and
    saved it into file buffer. 

INPUTS:    
    Name        Description                               Units        Min        Max  

    line        line string

OUTPUTS:     
    Name        Description                               Units        Min        Max         

    codeinfo   file information			

RETURNS:
    NONE

EXAMPLES:
    FILE            *fptr;
    char             nextln[1000];
    PGS_SMF_MsgInfo  codeinfo, 	

    fptr = fopen("PGS_2","r");
    fgets(nextln,sizeof(nextln),fptr);
    PGS_SMF_ExtractFileInfo(&codeInfo,nextln);

    while(fgets(nextln,sizeof(nextln),fptr) != (char *)NULL)
    {
        PGS_SMF_ExtractMsgInfo(&codeInfo,nextln);                    
    }

    fclose(fptr);


NOTES:          
    This routine is normally used in conjunction with PGS_SMF_DecodeCode().    

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE     

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void 
PGS_SMF_ExtractFileInfo(        /* Extract file information */
    PGS_SMF_MsgInfo *codeinfo,  /* Output file information */
    char            *line)      /* Input  line string */
{        
    char        *token;   /* string token */
    char         buf[PGS_SMF_MAX_MSGBUF_SIZE]; /* temp buffer */

    short        len;     /* length of string */  

#ifdef _PGS_THREADSAFE
    char         *lasts;  /* needed for strtok_r */
#endif


    /*
     * copy input line to temporary buffer
     * because strtok does destructive editing
     */
    strncpy(buf, line, PGS_SMF_MAX_MSGBUF_SIZE);

    if (buf != (char *)NULL)
    {
        len = strlen(buf);
        if (buf[len-1] == '\n')
        {
            buf[len-1] = '\0';
        }

        /*
         * Get instrument.
         */

#ifdef _PGS_THREADSAFE
        /*
         * Since strtok() is not threadsafe we will use strtok_r when
         * in threadsafe mode
         */
        token = strtok_r(buf,",",&lasts);
        sprintf(codeinfo->fileinfo.instr,"%s",token);

        /*
         * Get label.
         */
        token = strtok_r((char *)NULL,",",&lasts);
        sprintf(codeinfo->fileinfo.label,"%s",token);

        /*
         * Get seed.
         */
        token = strtok_r((char *)NULL,",",&lasts);
#else
        token = strtok(buf,",");
        sprintf(codeinfo->fileinfo.instr,"%s",token);

        /*
         * Get label.
         */
        token = strtok((char *)NULL,",");
        sprintf(codeinfo->fileinfo.label,"%s",token);

        /*
         * Get seed.
         */
        token = strtok((char *)NULL,",");
#endif
        codeinfo->fileinfo.seed = atoi(token);
    }


} /* end PGS_SMF_ExtractFileInfo */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_ExtractMsgInfo

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>                                                         

    void
    PGS_SMF_ExtractMsgInfo(
        PGS_SMF_MsgInfo  *codeinfo,
        char             *line);

FORTRAN:
    NONE

DESCRIPTION:            
    To extract information from a line retrieved from ASCII message file and
    saved it into code buffer.

INPUTS:    
    Name        Description                               Units        Min        Max   

    line        line string

OUTPUTS:       
    Name        Description                               Units        Min        Max  

    codeinfo    code information

RETURNS:
    NONE

EXAMPLES:
    See example for PGS_SMF_ExtractFileInfo().

NOTES:          
    This routine is normally used in conjunction with PGS_SMF_DecodeCode().

REQUIREMENTS:         
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void 
PGS_SMF_ExtractMsgInfo(          /* Extract code information */
    PGS_SMF_MsgInfo *codeinfo,   /* Output code information */
    char            *line)       /* Input  line string */
{      
    char        *token;   /* string token */
    char         buf[PGS_SMF_MAX_MSGBUF_SIZE]; /* temp buffer */

    short        len;     /* length of string */

#ifdef _PGS_THREADSAFE
    char         *lasts;  /* needed for strtok_r */
#endif


    /*
     * copy input line to temporary buffer
     * because strtok does destructive editing
     */
    strncpy(buf, line, PGS_SMF_MAX_MSGBUF_SIZE);

    if (buf != (char *)NULL)
    {
        len = strlen(buf);
        if (buf[len-1] == '\n')
        {
            buf[len-1] = '\0';
        }

        /*
         * Get code.
         */
#ifdef _PGS_THREADSAFE
        /*
         * Since strtok() is not threadsafe we will use strtok_r when
         * in threadsafe mode
         */
        token = strtok_r(buf,",",&lasts);
        codeinfo->msgdata.code = atoi(token);

        /*
         * Get mnemonic.
         */
        token = strtok_r((char *)NULL,",",&lasts);
        sprintf(codeinfo->msgdata.mnemonic,"%s",token);

        /*
         * Get action mnemonic.
         */
        token = strtok_r((char *)NULL,",",&lasts);
        if (strcmp(token,PGS_NULL_STR) == 0)
        {
            codeinfo->msgdata.action[0] = '\0';
        }
        else
        {
            sprintf(codeinfo->msgdata.action,"%s",token);
        }

        /*
         * Get message.
         */
        token = strtok_r((char *)NULL,"\n",&lasts);
#else
        token = strtok(buf,",");
        codeinfo->msgdata.code = atoi(token);

        /*
         * Get mnemonic.
         */
        token = strtok((char *)NULL,",");
        sprintf(codeinfo->msgdata.mnemonic,"%s",token);

        /*
         * Get action mnemonic.
         */
        token = strtok((char *)NULL,",");
        if (strcmp(token,PGS_NULL_STR) == 0)
        {
            codeinfo->msgdata.action[0] = '\0';
        }
        else
        {
            sprintf(codeinfo->msgdata.action,"%s",token);
        }

        /*
         * Get message.
         */
        token = strtok((char *)NULL,"\n");
#endif
        sprintf(codeinfo->msgdata.msg,"%s",token);
        len = strlen(codeinfo->msgdata.msg);
        codeinfo->msgdata.msg[len] = '\0';
    }


} /* end PGS_SMF_ExtractMsgInfo */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GenerateStatusReport

SYNOPSIS:   
C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_GenerateStatusReport(
        char *report);

FORTRAN:
    INCLUDE 'PGS_SMF.f'

    integer function pgs_smf_generatestatusreport(report)
    char*1024 report

DESCRIPTION:
    This tool provides the method for the user to create status reports for use
    by Science Computing Facility personnel. Each call to this procedure causes
    the user defined report to be appended to the status report log.

INPUTS:    
    Name            Description                                Units        Min        Max

    report          user report 

OUTPUTS:    
    Name            Description                                Units        Min        Max

RETURNS:
    PGS_S_SUCCESS
    PGSSMF_E_LOGFILE	
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:
    C:
    ==  
    PGSt_SMF_status returnStatus;

    returnStatus = PGS_SMF_GenerateStatusReport("Write it into status report file");
    if (returnStatus == PGS_S_SUCCESS)
    {
        /# write to status report successful #/
    }

    FORTRAN:
    ========   
    implicit none
    integer pgs_smf_generatestatusreport

    integer returnStatus    

    returnStatus = pgs_smf_generatestatusreport("Write it into status report file")
    if (returnStatus .EQ. PGS_S_SUCCESS) then
C       write to status report successful
    endif        

NOTES:  
    The system defined message tag will automatically be added to the
    user-provided report.   

REQUIREMENTS:
    PGSTK-0650

DETAILS:	    
    Status log files: 'Status Message Log', 'User Status Log', & 'Status Report
    Log', should all be entered in the SUPPORT OUTPUT section of the Process
    Control Table with the properly defined logical identifiers. These logical
    identifiers are predefined (in the SMF include file) from a pool of Toolkit
    reserved identifiers. (Toolkit range is 10,000 - 11,000). The defined
    constants used are: PGSd_SMF_LOGICAL_LOGSTATUS, PGSd_SMF_LOGICAL_LOGREPORT,
    PGSd_SMF_LOGICAL_LOGUSER, PGSd_SMF_LOGICAL_TMPSTATUS,
    PGSd_SMF_LOGICAL_TMPREPORT and PGSd_SMF_LOGICAL_TMPUSER.

    (Not sure about size of 'report' argument. We don't really need to set a
     limit except that Fortran needs one ... check on this)

GLOBALS:
    NONE

FILES:
    Report log file.

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()
    PGS_SMF_GetSMFCode()
    PGS_SMF_TestErrorLevel()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_GenerateStatusReport(  /* Generate status report */
    char *report)              /* Input user report */ 
{
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;  /* return status */
    PGSt_SMF_status  loggingStatus;                 /* logging status */
    
    PGSSmfGlbVar    *global_var;                    /* global static buffer */

    time_t           calPtr;                        /* calendar time */    
#ifdef _PGS_THREADSAFE
    char temChar[100];      /* used for ctime_r() */
#endif

    PGS_SMF_GetGlobalVar(&global_var);

    /* If user has turned off logging, exit here, BEFORE writing anything to the
       log file. */

    loggingStatus = PGS_SMF_LoggingControl(PGSd_QUERY_ALL, 0);
    
    if ( loggingStatus == PGSSMF_M_LOGGING_DISABLED )
    {
	PGS_SMF_SetStaticMsg(PGSSMF_M_LOGGING_DISABLED, "");
	return PGSSMF_M_LOGGING_DISABLED;
    }

#ifdef _PGS_THREADSAFE
    /*
     * Since ctime() is not threadsafe we need to use ctime_r() when
     * in threadsafe mode
     */
    time(&calPtr);            
    ctime_r(&calPtr,temChar);

    /*
     * We are actually going to write to reports, so we need 
     * to lock when in TSF mode.
     */
    returnStatus = PGS_TSF_LockIt(PGSd_TSF_SMFWRITELOCK);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    if (global_var->log.logReport[0] == '\0')
    {
        global_var->log.fptrReport = (FILE *)NULL;
    }
    else
    {
        global_var->log.fptrReport = fopen(global_var->log.logReport, "a");
    }
#endif

    /*
     * Write it into report log file.
     */
    if (global_var->log.fptrReport == (FILE *)NULL)
    {      
        global_var->msginfo.fileinfo.seed = PGS_SMF_SYS_SEED;
        strcpy(global_var->msginfo.fileinfo.instr,PGS_SMF_SYS_INSTR);
        strcpy(global_var->msginfo.fileinfo.label,PGS_SMF_SYS_LABEL);
        global_var->msginfo.msgdata.action[0] = '\0';
        global_var->msginfo.msgdata.code = PGSSMF_E_LOGFILE;
        PGS_SMF_GetSMFCode(PGSSMF_E_LOGFILE,
                           global_var->msginfo.msgdata.mnemonic,
                           global_var->msginfo.msgdata.msg);
        strcpy(global_var->msg,"Error opening report log file");
        returnStatus = PGSSMF_E_LOGFILE;
    }
    else
    {        
#ifdef _PGS_THREADSAFE
        fprintf(global_var->log.fptrReport,"%s%s%s\n",temChar,
                    PGS_SMF_LogPID(PGSd_QUERY),
                    PGS_SMF_GetThreadID());
#else
        time(&calPtr);            
        fprintf(global_var->log.fptrReport,"%s%s\n",ctime(&calPtr),
		PGS_SMF_LogPID(PGSd_QUERY));
#endif
        fprintf(global_var->log.fptrReport,"%s\n",report);
        fprintf(global_var->log.fptrReport,"==========================================================\n\n");
        fflush(global_var->log.fptrReport);
    }

#ifdef _PGS_THREADSAFE
    if (global_var->log.fptrReport != (FILE *)NULL)
    {
        fclose(global_var->log.fptrReport);
    }

    returnStatus = PGS_TSF_UnlockIt(PGSd_TSF_SMFWRITELOCK);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
    }
#endif

    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"");
    }

    return(returnStatus);

} /* end PGS_SMF_GenerateStatusReport */
      


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetActionByCode

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_GetActionByCode(
        PGSt_SMF_code code,                       
        char          action[]);

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_getactionbycode(code,action)
    integer       code
    character*240 action

DESCRIPTION:
    This tool will provide the means to retrieve an action
    string corresponding to the specify mnemonic code.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:    
    Name        Description                               Units        Min        Max

    action      action string

RETURNS:        
    PGS_S_SUCCESS    
    PGS_E_ENV
    PGS_E_UNIX
    PGSSMF_W_NOACTION
    PGSSMF_E_UNDEFINED_CODE

EXAMPLES:
    C:
    ==
    char action[PGS_SMF_MAX_ACT_SIZE];
    PGS_SMF_GetActionByCode(PGSSMF_E_UNDEFINED_UNIXERROR,action);


    FORTRAN:
    ========
    implicit none
    integer pgs_smf_getactionbycode

    character*240 action
    pgs_smf_getactionbycode(PGSSMF_E_UNDEFINED_UNIXERROR,action);

NOTES:          
    This routine may not return any associated action string if user did not
    associate any action label when creating the status message. If that is the
    case, then the parameter action[0] = '\0'. Refer to utility "smfcompile" for
    additional information on the format of the message compiler.

REQUIREMENTS:
    PGSTK-0580,0590,0591,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()
    PGS_SMF_DecodeCode()
    PGS_SMF_SetStaticMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_GetActionByCode(       /* Get action */
    PGSt_SMF_code code,        /* Input  code */
    char          action[])    /* Output action string */
{ 
    PGSt_SMF_status       returnStatus = PGS_S_SUCCESS;             /* return status */
    char                  actionLabel[PGS_SMF_MAX_MNEMONIC_SIZE];   /* action label */
    static char           prevAction[PGS_SMF_MAX_MSGBUF_SIZE];         /* previous action */
    static PGSt_SMF_code  prevCode = PGS_M_NULL;                    /* previous code */
    PGSSmfGlbVar         *global_var;                               /* global static buffer */


    PGS_SMF_GetGlobalVar(&global_var);

    action[0] = '\0';
    if (prevCode == PGS_M_NULL)
    {
        prevAction[0] = '\0';
    }

    if (code == prevCode)
    {
        if (prevAction[0] == '\0')
        {
            returnStatus = PGSSMF_W_NOACTION;
        }
    }
    else
    {      
        /*
         * Check if user's code is a valid code.
         */
        returnStatus = PGS_SMF_DecodeCode(code,"",&global_var->msginfo,
                                          global_var->msginfo.funcname,0);
        if (returnStatus == PGS_S_SUCCESS)
        {                         
            if (strlen(global_var->msginfo.msgdata.action) == 0)
            {
                /*
                 * No action associates with user's code.
                 */
                prevCode = code;
                prevAction[0] = '\0';
                returnStatus = PGSSMF_W_NOACTION;
            }
            else
            {       
                /* 
                 * Get the associated action message.
                 */
                strcpy(actionLabel,global_var->msginfo.msgdata.action);
                returnStatus = PGS_SMF_DecodeCode(code,
						  actionLabel,
						  &global_var->msginfo,
                                                  global_var->msginfo.funcname,
						  1);
                if (returnStatus == PGS_S_SUCCESS)
                {
                    prevCode = code;
                    sprintf(prevAction,"%s",global_var->msginfo.msgdata.msg);
                }                
            }            
        }        
    }                     

    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"");
        strcpy(action,prevAction);
    }
    else if (returnStatus == PGSSMF_W_NOACTION)
    {
        PGS_SMF_SetStaticMsg(PGSSMF_W_NOACTION,"PGS_SMF_GetActionByCode()");
    }
    else
    {
        strcpy(global_var->msg,global_var->msginfo.msgdata.msg);     
    }    

    return(returnStatus);

} /* end PGS_SMF_GetActionByCode */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetActionMneByCode

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_GetActionMneByCode(
        PGSt_SMF_code code,
        char          action[],
        char          mnemonic[]);

DESCRIPTION:
    This tool will provide the means to retrieve an action string
    and label corresponding to the specified status code.

INPUTS:    
    Name        Description

    code        error/status code generated 
                by message compiler

OUTPUTS:    
    Name        Description

    action      action string
    mnemonic	action mnemonic

RETURNS:        
    PGS_S_SUCCESS    
    PGS_E_ENV
    PGS_E_UNIX
    PGSSMF_W_NOACTION
    PGSSMF_E_UNDEFINED_CODE

EXAMPLES:
    C:
    ==
    char action[PGS_SMF_MAX_ACT_SIZE];
    char mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];
    PGS_SMF_GetActionMneByCode(PGSSMF_E_UNDEFINED_UNIXERROR,action, mnemonic);

NOTES:          
    This routine may not return any associated action string if user did not
    associate any action label when creating the status message. If that is the
    case, then the parameter action[0] = '\0'. Refer to utility "smfcompile" for
    additional information on the format of the message compiler.

REQUIREMENTS:
    PGSTK-0580,0590,0591,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()
    PGS_SMF_DecodeCode()
    PGS_SMF_SetStaticMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_GetActionMneByCode(    /* Get action */
    PGSt_SMF_code code,        /* Input  code */
    char          action[],    /* Output action string */
    char          mnemonic[])  /* Output action mnemonic */
{ 
    PGSt_SMF_status       
        returnStatus = PGS_S_SUCCESS;             /* return status */
    PGSSmfGlbVar         
        *global_var;                              /* global static buffer */

    char                  
        actionLabel[PGS_SMF_MAX_MNEMONIC_SIZE];   /* action label */

    static char           
        prevActionMsg[PGS_SMF_MAX_MSGBUF_SIZE];      /* previous action message */
    static char           
        prevActionMne[PGS_SMF_MAX_MNEMONIC_SIZE]; /* previous action mnemonic */
    static PGSt_SMF_code  
        prevCode = PGS_M_NULL;                    /* previous code */

    PGS_SMF_GetGlobalVar(&global_var);

    action[0] = '\0';
    mnemonic[0] = '\0';
    if (prevCode == PGS_M_NULL)
    {
        prevActionMsg[0] = '\0';
        prevActionMne[0] = '\0';
    }

    if (code == prevCode)
    {
        if (prevActionMsg[0] == '\0')
        {
            returnStatus = PGSSMF_W_NOACTION;
        }
    }
    else
    {      
        /*
         * Check if user's code is a valid code.
         */
        returnStatus = PGS_SMF_DecodeCode(
            code,
            "",
            &global_var->msginfo,
            global_var->msginfo.funcname,
            0);

        if (returnStatus == PGS_S_SUCCESS)
        {                         
            if (strlen(global_var->msginfo.msgdata.action) == 0)
            {
                /*
                 * No action associates with user's code.
                 */
                prevCode = code;
                prevActionMsg[0] = '\0';
                returnStatus = PGSSMF_W_NOACTION;
            }
            else
            {       
                /* 
                 * Get the associated action message.
                 */
                strcpy(				/* preserve action label */
                   actionLabel,
                   global_var->msginfo.msgdata.action);

		/*
		 * Global substructure 'msgdata' gets modified 
		 * to hold action information instead of status
		 * information. 
		 */
                returnStatus = PGS_SMF_DecodeCode(
                    code,
                    actionLabel,
                    &global_var->msginfo,
                    global_var->msginfo.funcname,
                    1);

                if (returnStatus == PGS_S_SUCCESS)
                {
                    prevCode = code;

                    sprintf(
                        prevActionMsg,
                        "%s",
                        global_var->msginfo.msgdata.msg);                   

                    strcpy( 
                        prevActionMne,
                        actionLabel);
                }                
            }            
        }        
    }                     

    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"");
       /*
	* Rectifying Global substructure due to side-effect of
	* call to *SetStaticMsg() which sets this to 0.
	*/
        global_var->msginfo.msgdata.code = code;

        if ( action != 0 ) strcpy( action, prevActionMsg );
        if ( mnemonic != 0 ) strcpy( mnemonic, prevActionMne );

    }
    else if (returnStatus == PGSSMF_W_NOACTION)
    {
        if ( action != 0 ) action[0] = 0;
        if ( mnemonic != 0 ) mnemonic[0] = 0;

   /*  Deactivated code on 11/28/95 due to problems which
    *  resulted in the loss of "internal" status information
    *  for non-action type status conditions.
    *
    *   PGS_SMF_SetStaticMsg(
    *       PGSSMF_W_NOACTION,
    *       "PGS_SMF_GetActionMneByCode()");
    */
    }
    else
    {
        strcpy(
            global_var->msg,
            global_var->msginfo.msgdata.msg);     
    }    

    return(returnStatus);

} /* end PGS_SMF_GetActionMneByCode */

/*************************************************************************
BEGIN_PROLOG:

TITLE:  
    Determine type of action from mnemonic label.

NAME:   
    PGS_SMF_GetActionType

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_GetActionType(
        char *actionLabel);

FORTRAN:
    NONE

DESCRIPTION:
    Check an action label to determinine what type it is.  This
    allows special handling if the action label contains the
    proper token.  The action type is indicated by the return
    status code.

INPUTS:   
    Name            Description
       
    actionLabel     mnemonic label for an action string

OUTPUTS:
    NONE

RETURNS:       
    PGSSMF_M_REGULAR_ACTION 
    PGSSMF_M_EMAIL_ACTION 
    PGSSMF_M_INFO_ACTION
    PGSSMF_M_NOT_ACTION 
    PGS_E_TOOLKIT

EXAMPLES:        


NOTES:           
    This is a low-level tool. 

    Currently, (initial version: 26-Sep-1995 Mike Sucher), 
    the tool will return the code:

        PGSSMF_M_EMAIL_ACTION 

    if the action is an email type, 

        PGSSMF_M_INFO_ACTION 

    if the action is of another type, otherwise it will return:

        PGSSMF_M_REGULAR_ACTION 

    In the future, other dispositions may be added.

REQUIREMENTS:
    PGSTK-

DETAILS:
    The system library function strtok is used to break apart 
    the mnemonic label into tokens, where the separator is "_".
    The second token must be "C" for an channel-action string.  The third
    is checked to determine the action (MSS disposition) type.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
**************************************************************************/

PGSt_SMF_status
PGS_SMF_GetActionType (
    char *actionLabel)
{
    char buf[PGS_SMF_MAX_MNEMONIC_SIZE];
    char level[PGS_SMF_LEV_DISPLAY+1];
    char *token;
    char *emailString = PGS_SMF_MSSCODE_EMAIL;
    char *infoString = PGS_SMF_MSSCODE_INFO;

#ifdef _PGS_THREADSAFE
    char *lasts;      /* used by strtok_r() */
#endif

    if ( strlen(actionLabel) == 0 ) 
    {
        return PGS_E_TOOLKIT;
    }

    strcpy(buf, actionLabel);           /* run strtok on a copy of the label */

#ifdef _PGS_THREADSAFE
    /* 
     * Since strtok() is not threadsafe we will use strtok_r() while in
     * threadsafe mode.
     */
    token = strtok_r(buf,PGS_SMF_LEV_DELIMIT,&lasts);        	/* get instrument */

    token = strtok_r((char *)NULL,PGS_SMF_LEV_DELIMIT,&lasts);	/* get message level */
    sprintf( level,"%s%s%s",PGS_SMF_LEV_DELIMIT,token,PGS_SMF_LEV_DELIMIT );
    if ((strcmp(PGS_SMF_STAT_LEV_A,level) != 0) &&
        (strcmp(PGS_SMF_STAT_LEV_C,level) != 0))
    {
        return PGSSMF_M_NOT_ACTION;
    }

    token = strtok_r((char *)NULL,PGS_SMF_LEV_DELIMIT,&lasts);	/* get next token */
#else
    token = strtok(buf,PGS_SMF_LEV_DELIMIT);        	/* get instrument */

    token = strtok((char *)NULL,PGS_SMF_LEV_DELIMIT);	/* get message level */
    sprintf( level,"%s%s%s",PGS_SMF_LEV_DELIMIT,token,PGS_SMF_LEV_DELIMIT );
    if ((strcmp(PGS_SMF_STAT_LEV_A,level) != 0) &&
        (strcmp(PGS_SMF_STAT_LEV_C,level) != 0))
    {
        return PGSSMF_M_NOT_ACTION;
    }

    token = strtok((char *)NULL,PGS_SMF_LEV_DELIMIT);	/* get next token */
#endif
    if (strcmp(PGS_SMF_STAT_LEV_C,level) == 0)
    {
	if (strcmp(token, emailString) == 0)
	{
            return PGSSMF_M_EMAIL_ACTION;
	}
	else if (strcmp(token, infoString) == 0)
	{
            return PGSSMF_M_INFO_ACTION;
	}
    }

    return PGSSMF_M_REGULAR_ACTION;

} /* end PGS_SMF_GetActionType */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetEnv

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_GetEnv(
        PGS_SMF_MsgInfo *codeinfo,
        char            *funcname);

FORTRAN:
    NONE

DESCRIPTION:
    To make sure that environment 'PGSMSG' is set. This routine
    is used in conjunction with all the SMF routines.

INPUTS:   
    Name        Description                               Units        Min        Max 

    funcname    function name

OUTPUTS:      
    Name        Description                               Units        Min        Max       

    codeinfo    code info. buffer		

RETURNS:
    PGS_S_SUCCESS
    PGS_E_ENV

EXAMPLES:
    PGS_SMF_MsgInfo codeinfo;
    PGSt_SMF_status returnStatus;

    returnStatus = PGS_SMF_GetEnv(&codeinfo,"func()");
    if (returnStatus == PGS_E_ENV)
    {
        /# Environment variable 'PGSMSG' is not set #/
    }

NOTES:  
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE 

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetSystemCode()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_GetEnv(                   /* Get environment */
    PGS_SMF_MsgInfo *codeinfo,    /* Output code info. buffer */
    char            *funcname)    /* Input function name */
{       
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */   
    char             msg[PGS_SMF_MAX_MSGBUF_SIZE];      /* code message */


    if (getenv("PGSMSG") == (char *)NULL) 
    {
        memset((char *)codeinfo,'\0',sizeof(PGS_SMF_MsgInfo));

        codeinfo->msgdata.code = PGS_E_ENV;
        PGS_SMF_GetSystemCode(PGS_E_ENV,codeinfo->msgdata.mnemonic,msg); 

        sprintf(codeinfo->msgdata.msg,
		"%s: the environment variable 'PGSMSG' is not set",msg);        
        sprintf(codeinfo->funcname,"%s",funcname); 
        returnStatus = PGS_E_ENV;                  
    }       

    return(returnStatus);

} /* end PGS_SMF_GetEnv */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetGlobalVar

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    static PGSt_SMF_status
    PGS_SMF_GetGlobalVar(
        PGSSmfGlbVar **global_var);

FORTRAN:
    NONE

DESCRIPTION:
    This tool is used to encapsulate all the global variables needed in SMF
    program.

INPUTS:    
    Name        Description                               Units        Min        Max 

    global_var  pointer to structure that contains 
                global data (static buffer)  

    whichFlag   Used for the thread-safe version - states
		whether to return the global variable or
		the thread specific data (TSD).

OUTPUTS:    
    Name        Description                               Units        Min        Max 

    global_var  pointer to structure that contains 
                global data (static buffer)

RETURNS:
    PGS_S_SUCCESS
    PGS_E_ENV
    PGS_SH_SMF_MSSLOGFILE
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:
    PGSSmfGlbVar    *global_var;   
    PGSt_SMF_status  returnStatus;

    returnStatus = PGS_SMF_GetGlobalVar(&global_var);

NOTES:               
    All global data should be accessed via this function, including 
    functions that are declared in another file.    

    The overhead of using this method to encapsulate global data is that 
    calling function will need to call PGS_SMF_GetGlobalVar() to retrieve  
    the pointer to the static buffer.  Other than that, this method serves 
    encapsulation, initialization and tracking very well.

REQUIREMENTS:       
    PGSTK-0580,0590,0650

DETAILS:	   
    Note that the log files will be closed automatically by the system 
    when the process exits. If run under Purify, it will report MIU
    (Memory In Use - at least 3 chunks) meant for the opened log files.

    In TK4, the log files and message tag information will be retrieved 
    from shared memory.  If there is error in creating shared memory, SMF 
    will instead attempt to use virtual shared memory via an ASCII file.
    If an error is encountered when attempting to use the ASCII file method,
    then SMF will proceed to work accordingly.  In this situation, no errors
    will be written into the log files (since no information is available
    to open the log files).  The message tag will be set to NULL (again, no
    information is available).

GLOBALS:
    PGSg_TSF_SMFFlag
    PGSg_TSF_SMFCallerID

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetSysShm()
    PGS_SMF_TestErrorLevel()
    PGS_TSF_GetTSFMaster()
    PGS_TSF_GetMasterIndex()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_GetGlobalVar(              /* Get global data (static buffer) */
    PGSSmfGlbVar **global_var)     /* Input:  pointer to structure that 
                                      contains global data (static buffer) */
                                   /* Output: pointer to structure that 
                                      contains global data (static buffer) */
{

    static PGSt_SMF_boolean   initStaticBuf = PGS_FALSE;    /* initialization
							       flag */
    static PGSSmfGlbVar       globalvar;                    /* static buffer */
    char                      msg[PGS_SMF_MAX_MSGBUF_SIZE];    /* message */
    PGSt_SMF_status           returnStatus = PGS_S_SUCCESS; /* return status */

#ifdef _PGS_THREADSAFE
    /*
     * This is kind of tricky here.  We have the global that is now
     * thread-specific (each thread has its own), but each thread has
     * to have its global initialized properly.  So, we store an init
     * flag in one of our globals.  It is initialized to PGS_FALSE and
     * then set to PGS_TRUE once the global has been initialized.
     */
    PGSt_TSF_MasterStruct *masterTSF;
    PGSt_SMF_boolean initFlag;
    PGSSmfGlbVar *globalVarRet;
    extern PGSt_SMF_boolean PGSg_TSF_SMFFlag[];
    int masterTSFIndex;

    /*
     * Get struct with keys and get global index value.  This should 
     * give us access to any data we need.
     */
    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ((PGS_SMF_TestErrorLevel(returnStatus)) ||
        (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    initFlag = PGSg_TSF_SMFFlag[masterTSFIndex];

    if (initFlag == PGS_FALSE)
    {
        PGSSmfGlbVar *globalVar;
        extern PGSt_SMF_status PGSg_TSF_SMFCallerID[];

        globalVar = (PGSSmfGlbVar *) malloc(sizeof(PGSSmfGlbVar));
        memset((char *)globalVar,'\0',sizeof(PGSSmfGlbVar)); 

        /*
         * Initialize former static variables
         */
        globalVar->other.callerID     = PGSd_CALLERID_USR;      /* callerID to stop deadlock */
        globalVar->other.writeLogFile = PGSd_SMF_WRITELOG_ON;   /* enable/disable error logging */
        globalVar->other.useMemType   = PGSd_SMF_USE_SHAREDMEM; /* flag to use either shared memory or ascii file */
        globalVar->other.whichProc    = PGSd_SMF_PROC_USER;     /* which process are we in? (init., PGE, or term.) */

        /*
         * Attach system shared memory to this process and
         * reload global variable data from shared memory.
         *
         * NOTE:
         * The callerID value must be set because PGS_SMF_GetSysShm calls 
         * PGS_MEM_ShmSysAddr, which must know that it is being called 
         * from SMF in order to avoid a deadlock condition.
         */
        PGSg_TSF_SMFCallerID[masterTSFIndex] = PGSd_CALLERID_SMF;
        returnStatus = PGS_SMF_GetSysShm(globalVar);
        if ( (returnStatus != PGS_S_SUCCESS) && 
	     (returnStatus != PGS_SH_SMF_MSSLOGFILE) )
	{
	    returnStatus = PGS_E_ENV; 
	}
        PGSg_TSF_SMFCallerID[masterTSFIndex] = PGSd_CALLERID_USR;

        /*
         * Set flag to indicate that permission is granted to write into
         * log files.  Note that this flag is reset in the tool 
         * PGS_SMF_SendStatusReport() when the system is trying 
         * to send log files to remote hosts.
         */
        globalVar->log.record = PGS_TRUE;

        /* 
         * To be delivered in TK5 if implementation can be worked out 
         * (1/5/95 Heroux.)
         * Default arithmetic trap.
         *
         * globalVar->signal.func = PGS_SMF_SysSignalHandler;
         */

        /*
         * If PC environment variable is not set, then the IO tools 
         * won't work: return the appropriate error message.
         */
        if (getenv("PGS_PC_INFO_FILE") == (char *)NULL)
        {

            globalVar->msginfo.msgdata.code = PGS_E_ENV;
            PGS_SMF_GetSystemCode(PGS_E_ENV, globalVar->msginfo.msgdata.mnemonic,
				  msg); 

            strcpy(globalVar->msginfo.msgdata.msg,
                   "Environment variable 'PGS_PC_INFO_FILE' is not set"); 
            strcpy(globalVar->msginfo.funcname,"PGS_SMF_GetGlobalVar()");
            strcpy(globalVar->msg,globalvar.msginfo.msgdata.msg);            
            returnStatus = PGS_E_ENV;                  
        }
	else
	{
	    PGS_SMF_InitializeLogging();
	}

        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYSMFGLOBAL], globalVar);
        PGSg_TSF_SMFFlag[masterTSFIndex] = PGS_TRUE;
    }

    globalVarRet = (PGSSmfGlbVar *) pthread_getspecific(
                                 masterTSF->keyArray[PGSd_TSF_KEYSMFGLOBAL]);
    if (global_var != 0 )
    {
        *global_var = globalVarRet; 
    }

#else  /* -D_PGS_THREADSAFE */


    if (initStaticBuf == PGS_FALSE) 
    {
        /*
         * This is the first time this routine has been invoked in the 
         * current process, so initialize the static buffer.
         */
        initStaticBuf = PGS_TRUE;        
        memset((char *)&globalvar,'\0',sizeof(PGSSmfGlbVar)); 

        /*
         * Initialize former static variables
         */
        globalvar.other.callerID     = PGSd_CALLERID_USR;      /* callerID to stop deadlock */
        globalvar.other.writeLogFile = PGSd_SMF_WRITELOG_ON;   /* enable/disable error logging */
        globalvar.other.useMemType   = PGSd_SMF_USE_SHAREDMEM; /* flag to use either shared memory or ascii file */
        globalvar.other.whichProc    = PGSd_SMF_PROC_USER;     /* which process are we in? (init., PGE, or term.) */

        /*
         * Attach system shared memory to this process and
         * reload global variable data from shared memory.
         *
         * NOTE:
         * The callerID value must be set because PGS_SMF_GetSysShm calls 
         * PGS_MEM_ShmSysAddr, which must know that it is being called 
         * from SMF in order to avoid a deadlock condition.
         */
        callerID = PGSd_CALLERID_SMF;
        returnStatus = PGS_SMF_GetSysShm(&globalvar);
        if ( (returnStatus != PGS_S_SUCCESS) && 
	     (returnStatus != PGS_SH_SMF_MSSLOGFILE) )
	{
	    returnStatus = PGS_E_ENV; 
	}
        callerID = PGSd_CALLERID_USR;     

        /*
         * Set flag to indicate that permission is granted to write into
         * log files.  Note that this flag is reset in the tool 
         * PGS_SMF_SendStatusReport() when the system is trying 
         * to send log files to remote hosts.
         */
        globalvar.log.record = PGS_TRUE;

        /* 
         * To be delivered in TK5 if implementation can be worked out 
         * (1/5/95 Heroux.)
         * Default arithmetic trap.
         *
         * globalvar.signal.func = PGS_SMF_SysSignalHandler;
         */

        /*
         * If PC environment variable is not set, then the IO tools 
         * won't work: return the appropriate error message.
         */
        if (getenv("PGS_PC_INFO_FILE") == (char *)NULL)
        {

            globalvar.msginfo.msgdata.code = PGS_E_ENV;
            PGS_SMF_GetSystemCode(PGS_E_ENV, globalvar.msginfo.msgdata.mnemonic,
				  msg); 

            strcpy(globalvar.msginfo.msgdata.msg,
                   "Environment variable 'PGS_PC_INFO_FILE' is not set"); 
            strcpy(globalvar.msginfo.funcname,"PGS_SMF_GetGlobalVar()");
            strcpy(globalvar.msg,globalvar.msginfo.msgdata.msg);            
            returnStatus = PGS_E_ENV;                  
        }
	else
	{
	    PGS_SMF_InitializeLogging();
	}
	

    }  /* END: if (initStaticBuf == PGS_FALSE) */

    /*
     * Return the address of the static buffer to the outside world.
     */
    if (global_var != 0 )
    {
        *global_var = &globalvar; 
    }
#endif  /* -D_PGS_THREADSAFE */

    return returnStatus;

} /* end PGS_SMF_GetGlobalVar */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetInstrName

SYNOPSIS: 
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_GetInstrName(
        PGSt_SMF_code code,
        char          instr[]);

FORTRAN: 
    include 'PGS_SMF.f'   

    integer function pgs_smf_getinstrname(code,instr)
    integer      code
    character*10 instr

DESCRIPTION:
    This tool may be used to retrieve the instrument name
    from a given error/status code.

INPUTS:    
    Name        Description                               Units        Min        Max       

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:    
    Name        Description                               Units        Min        Max       

    instr       corresponding instrument name

RETURNS:        
    PGS_S_SUCCESS    
    PGS_E_ENV
    PGS_E_UNIX
    PGSSMF_E_UNDEFINED_CODE

EXAMPLES:
    C:
    ==    
    char instr[PGS_SMF_MAX_INSTR_SIZE];    
    PGS_SMF_GetInstrName(MODIS_E_BAD_CALIBRATION ,instr);    

    FORTRAN:
    ========
    implicit none
    integer pgs_smf_getinstrname

    character*10 instr
    pgs_smf_getinstrname(MODIS_E_BAD_CALIBRATION,instr)

NOTES:          
    Refer to utility "smfcompile" for additional information on the format 
    of the message compiler. The instrument name is defined in the status
    message file where the status code mnemonic is defined.

REQUIREMENTS:   
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()
    PGS_SMF_DecodeCode()
    PGS_SMF_SetStaticMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_GetInstrName(         /* Get instrument */
    PGSt_SMF_code code,       /* Input  code */
    char          instr[])    /* Output corresponding instrument name */
{
    PGSt_SMF_status       returnStatus = PGS_S_SUCCESS;      /* return status */
    static char           prevInstr[PGS_SMF_MAX_INSTR_SIZE]; /* previous
								instrument */
    static PGSt_SMF_code  prevCode = PGS_M_NULL;             /* previous code */
    PGSSmfGlbVar         *global_var;                        /* global static
								buffer */

    PGS_SMF_GetGlobalVar(&global_var);

    instr[0] = '\0';
    if (prevCode == PGS_M_NULL)
    {
        prevInstr[0] = '\0';
    }       

    if (code != prevCode)
    {
        returnStatus = PGS_SMF_DecodeCode(code,"",&global_var->msginfo,
					  "PGS_SMF_GetInstrName()",0);
        if (returnStatus == PGS_S_SUCCESS)
        {
            prevCode = code;
            sprintf(prevInstr,"%s",global_var->msginfo.fileinfo.instr);
        }
    }

    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"");       
        sprintf(instr,"%s",prevInstr);
    }
    else
    {
        strcpy(global_var->msg,global_var->msginfo.msgdata.msg);
    }

    return(returnStatus);

} /* end PGS_SMF_GetInstrName */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetMsg

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_GetMsg(
        PGSt_SMF_code *code,
        char           mnemonic[],
        char           msg[]);

FORTRAN:
    call pgs_smf_getmsg(code,mnemonic,msg)
    integer       code
    character*32  mnemonic
    character*480 msg

DESCRIPTION:
    This tool will provide the means to retrieve the previously
    set message from the static buffer (PGS_SMF_Set...()). It
    should be called immediately; otherwise, the intended message
    will be over-written by subsequent calls to the set message routines.

INPUTS:     
    Name        Description                               Units        Min        Max       

OUTPUTS:    
    Name        Description                               Units        Min        Max

    code        previously set status code
    mnemonic    previously set mnemonic error/status string
    msg         previously set message string

RETURNS:
    NONE

EXAMPLES:
     See example for PGS_SMF_SetDynamicMsg().

NOTES:
     The message must be set just prior to returning from a tool in order
     for this tool to retrieve that message. If there is nothing that has been
     set in the static buffer, then the parameters are returned with the 
     following value: code=0, mnemonic= and msg=.

     Refer to utility "smfcompile" for additional information on the format 
     of the message compiler.     

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:
    Note that PGSSMF_M_UNIX is interpreted as PGS_S_SUCCESS. Refer to
    PGS_SMF_SetUNIXMsg() for detail. Therefore the return parameters will
    contain: code=unixErrno, mnemonic=PGS_E_UNIX and msg=(whatever message is in
    the static buffer)

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()
    PGS_SMF_GetSystemCode()     

END_PROLOG:
*****************************************************************/
void 
PGS_SMF_GetMsg(                  /* Get previousy set message */
    PGSt_SMF_code *code,         /* Output previously set status code */
    char           mnemonic[],   /* Output previously set mnemonic error/status
				    string */
    char           msg[])        /* Output previously set message string */
{
    PGSSmfGlbVar *global_var;                     /* global static buffer */ 

    PGS_SMF_GetGlobalVar(&global_var);

    if (global_var->msginfo.msgdata.code == PGS_S_SUCCESS)
    {
        PGS_SMF_GetSystemCode(PGS_S_SUCCESS,mnemonic,msg);
        *code = PGS_S_SUCCESS;
    }
    else if (global_var->msginfo.msgdata.code == PGSSMF_M_UNIX)
    {
        *code = global_var->unixErrno; 
        sprintf(mnemonic,"%s",global_var->msginfo.msgdata.mnemonic);
        sprintf(msg,"%s",global_var->msg);
    }
    else
    {
        *code = global_var->msginfo.msgdata.code;
        sprintf(mnemonic,"%s",global_var->msginfo.msgdata.mnemonic);
        sprintf(msg,"%s",global_var->msg);
    }


} /* end PGS_SMF_GetMsg */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetMsgByCode

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_GetMsgByCode(
        PGSt_SMF_code code,
        char          msg[]);

FORTRAN:
    include 'PGS_SMF.f'    

    integer function pgs_smf_getmsgbycode(code,msg)
    integer        code
    character*240  msg

DESCRIPTION:
    This tool will provide the means to retrieve the
    corresponding message string to the specify mnemonic
    code.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:    
    Name        Description                               Units        Min        Max

    msg         user pre-defined message string

RETURNS:        
    PGS_S_SUCCESS   
    PGS_E_ENV
    PGS_E_UNIX
    PGSSMF_E_UNDEFINED_CODE

EXAMPLES:
    See example for PGS_SMF_SetDynamicMsg().

NOTES:   
    Refer to utility "smfcompile" for additional information on the format 
    of the message compiler.       

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()
    PGS_SMF_DecodeCode()
    PGS_SMF_SetStaticMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_GetMsgByCode(     /* Get message by code */
    PGSt_SMF_code code,   /* Input  code */
    char          msg[])  /* Output user pre-defined message string */
{
    PGSt_SMF_status       returnStatus = PGS_S_SUCCESS;   /* return status */
    static char           prevMsg[PGS_SMF_MAX_MSGBUF_SIZE];  /* previous message */
    static PGSt_SMF_code  prevCode = PGS_M_NULL;          /* previous code */
    PGSSmfGlbVar         *global_var;                     /* global static
							     buffer */

    PGS_SMF_GetGlobalVar(&global_var);

    msg[0] = '\0';
    if (prevCode == PGS_M_NULL)
    {
        prevMsg[0] = '\0';
    }        

    if (code != prevCode)
    {   
        returnStatus = PGS_SMF_DecodeCode(code,"",&global_var->msginfo,
					  "PGS_SMF_GetMsgByCode()",0);       
        if (returnStatus == PGS_S_SUCCESS)
        {
            prevCode = code;
            sprintf(prevMsg,"%s",global_var->msginfo.msgdata.msg);            
        }
    }

    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"");
        sprintf(msg,"%s",prevMsg);
    }
    else
    {
        strcpy(global_var->msg,global_var->msginfo.msgdata.msg);
    }

    return(returnStatus);

} /* end PGS_SMF_GetMsgByCode */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetSMFCode

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_GetSMFCode(       
        PGSt_SMF_code code,
        char         *mnemonic,
        char         *msg);

FORTRAN:
    NONE

DESCRIPTION:        
    Retrieve SMF information (SEED = 2); code that start with PGSSMF_....

INPUTS:    
    Name        Description                               Units        Min        Max 

    code        SMF code

OUTPUTS:    
    Name        Description                               Units        Min        Max     

    mnemonic    SMF mnemonic
    msg         SMF message             

RETURNS:
    NONE

EXAMPLES:    
    char   mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];
    char   msg[PGS_SMF_MAX_MSGBUF_SIZE]; 

    PGS_SMF_GetSMFCode(PGSSMF_E_UNDEFINED_CODE,mnemonic,msg);

NOTES:                
    This routine is normally used in conjunction with PGS_SMF_DecodeCode().

DETAILS:      	
    Any changes to PGS_SMF_2.t should be reflect in here.

REQUIREMENTS:
    PGSTK-0580,0590,0650 

DETAILS:	   
    This function is used in conjunction with PGS_SMF_DecodeCode(); thus
    environment variable 'PGSMSG' is checked in the caller routine.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void
PGS_SMF_GetSMFCode(    
    PGSt_SMF_code  code,         /* Input  SMF code */
    char          *mnemonic,     /* Output SMF mnemonic */
    char          *msg)          /* Output SMF message */
{     
    
    switch (code)
    {
        case PGSSMF_E_UNDEFINED_CODE:          strcpy(mnemonic,"PGSSMF_E_UNDEFINED_CODE");
	                                       strcpy(msg,"Undefined code: %d");
                                               break;                                                                    

        case PGSSMF_E_UNDEFINED_UNIXERRNO:     strcpy(mnemonic,"PGSSMF_E_UNDEFINED_UNIXERRNO");
                                               strcpy(msg,"Undefined unix errno: %d");
                                               break;

        case PGSSMF_E_CANT_OPEN_FILE:          strcpy(mnemonic,"PGSSMF_E_CANT_OPEN_FILE");
                                               strcpy(msg,"Can't open file %s; %s");
                                               break;

        case PGSSMF_E_MSG_TOOLONG:             strcpy(mnemonic,"PGSSMF_E_MSG_TOOLONG");
                                               strcpy(msg,"Message length exceed %d");
                                               break;

        case PGSSMF_E_SIGFPE:                  strcpy(mnemonic,"PGSSMF_E_SIGFPE");
                                               strcpy(msg,"Arithmetic error SIGFPE");
                                               break;

        case PGSSMF_E_SIGACTION:               strcpy(mnemonic,"PGSSMF_E_SIGACTION");
                                               strcpy(msg,"Signal SIGFPE error, %s");
                                               break;

        case PGSSMF_E_INVALID_FORMAT:          strcpy(mnemonic,"PGSSMF_E_INVALID_FORMAT");
                                               strcpy(msg,"Invalid format specifier in the message string");
                                               break;

        case PGSSMF_E_BAD_REFERENCE:           strcpy(mnemonic,"PGSSMF_E_BAD_REFERENCE");
                                               strcpy(msg,"Bad reference");
                                               break;

        case PGSSMF_E_INVALID_FILE:            strcpy(mnemonic,"PGSSMF_E_INVALID_FILE");
                                               strcpy(msg,"File %s is not a regular or symbolically link file");
                                               break;                                       

        case PGSSMF_E_SENDFILE:                strcpy(mnemonic,"PGSSMF_E_SENDFILE");
                                               strcpy(msg,"%s");
                                               break;    

        case PGSSMF_E_NOMAIL_ADDR:             strcpy(mnemonic,"PGSSMF_E_NOMAIL_ADDR");
                                               strcpy(msg,"No mail address or subject is defined");
                                               break;  

        case PGSSMF_E_NONETRC_FILE:            strcpy(mnemonic,"PGSSMF_E_NONETRC_FILE");
                                               strcpy(msg,"No netrc file exist: %s");
                                               break;    

        case PGSSMF_E_NETRC_MODE:              strcpy(mnemonic,"PGSSMF_E_NETRC_MODE");
                                               strcpy(msg,"netrc file not correct mode; should be set to rw-------");
                                               break;    

        case PGSSMF_E_SENDMAIL:                strcpy(mnemonic,"PGSSMF_E_SENDMAIL");
                                               strcpy(msg,"%s");
                                               break;

        case PGSSMF_E_REMOTEPATH:              strcpy(mnemonic,"PGSSMF_E_REMOTEPATH");
                                               strcpy(msg,"Remote path is not defined");
                                               break;

        case PGSSMF_E_NOHOSTNAME:              strcpy(mnemonic,"PGSSMF_E_NOHOSTNAME");
                                               strcpy(msg,"No host name defined");
                                               break;

        case PGSSMF_E_LOGFILE:                 strcpy(mnemonic,"PGSSMF_E_LOGFILE");
                                               strcpy(msg,"Error opening status or report or user files");
                                               break;

        case PGSSMF_E_MSSLOGFILE:              strcpy(mnemonic,"PGSSMF_E_MSSLOGFILE");
                                               strcpy(msg,"Error opening MSS Event Log files");
                                               break;

        case PGSSMF_E_SENDRUNTIME_DATA:        strcpy(mnemonic,"PGSSMF_E_SENDRUNTIME_DATA");
                                               strcpy(msg,"Send runtime file data error");
                                               break;

        case PGSSMF_E_SENDSTATUS_LOG:          strcpy(mnemonic,"PGSSMF_E_SENDSTATUS_LOG");
                                               strcpy(msg,"Send status, report, user and process control files error");
                                               break;

        case PGSSMF_E_BAD_EVENTLOG_ACCESS:     strcpy(mnemonic,"PGSSMF_E_BAD_EVENTLOG_ACCESS");
                                               strcpy(msg,"MSS Event Log Could  not be accessed");
                                               break;

        case PGSSMF_E_INVALID_SWITCH:          strcpy(mnemonic,"PGSSMF_E_INVALID_SWITCH");
                                               strcpy(msg,"Invalid control switch input");
                                               break;

        case PGSSMF_W_NOACTION:                strcpy(mnemonic,"PGSSMF_W_NOACTION");
                                               strcpy(msg,"No action define");
                                               break;

        case PGSSMF_W_SENDRUNTIME_DATA:        strcpy(mnemonic,"PGSSMF_W_SENDRUNTIME_DATA");
                                               strcpy(msg,"Send runtime file data error");
                                               break;

        case PGSSMF_W_NO_CONSTRUCT_TAG:        strcpy(mnemonic,"PGSSMF_W_NO_CONSTRUCT_TAG");
                                               strcpy(msg,"No information to construct message tag");
                                               break;

        case PGSSMF_M_TRANSMIT_DISABLE:        strcpy(mnemonic,"PGSSMF_M_TRANSMIT_DISABLE");
                                               strcpy(msg,"Transmission of files is disabled");
                                               break;  

        case PGSSMF_M_LOGGING_DISABLED:        strcpy(mnemonic,"PGSSMF_M_LOGGING_DISABLED");
                                               strcpy(msg,"Logging of SMF messages disabled");
                                               break;  

        case PGSSMF_M_LOGGING_ENABLED:         strcpy(mnemonic,"PGSSMF_M_LOGGING_ENABLED");
                                               strcpy(msg,"Logging of SMF messages enabled");
                                               break;  

        case PGSSMF_M_UNIX:                 /* strcpy(mnemonic,"PGSSMF_M_UNIX"); */
                                               strcpy(mnemonic,"PGS_E_UNIX");
                                               strcpy(msg,"UNIX errno message");
                                               break;                     

        default:                               mnemonic[0] = '\0';
                                               msg[0] = '\0';
                                               break;                                                         
    }    


} /* end PGS_SMF_GetSMFCode */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetShmSize

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_GetShmSize(
        PGSMemShmSize   *memsize,
        PGSt_uinteger   count);

FORTRAN:
    NONE

DESCRIPTION:
    Routine called by PGS_PC_InitCom to determine amount of memory
    needed by SMF.  Also tells SMF how big the message cache will be.

INPUTS:   
    Name        Description                               Units        Min

    count	number of records to be allocated in      records      20  
                shared memory message cache.

OUTPUTS:
    Name        Description                               Units

    memsize 	the amount of memory in bytes that        bytes
            	we are requesting, in struct member
		'.smf' .

RETURNS:       
    PGS_S_SUCCESS

EXAMPLES:        


NOTES:           
    This routine is only to be used internally by ARC toolkit developers.    

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    This tool is provided to allow the user to specify the size of the 
    shared memory message cache.   It calculates the total amount of 
    shared memory, in bytes, that SMF will require.  This value is
    passed back to PGS_PC_InitCom, which then calls the appropriate
    MEM tools to set up the shared memory region.

    The size of the message cache is also passed to PGS_SMF_CacheMsgShm,
    which allows that tool to initialize its internal variables and the 
    appropriate areas in shared memory.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_GetShmSize(
    PGSMemShmSize *memsize,
    PGSt_integer   count)
{

    count = (count <= 20 ? 20 : count);    /* enforce minumum value */

    memsize->smf  = sizeof(PGSSmfShm);                /* size of struct */

    memsize->smf += sizeof(PGS_SMF_MsgInfo) * count;  /* size of cache */

    /*
     * Now that that count of message structures is known,
     * pass it to PGS_SMF_CacheMsgShm, so it may set the
     * initial values.
     */

    PGS_SMF_CacheMsgShm(
			PGSd_SMF_InitCacheInfo1,
                        (PGSt_SMF_code) count,
                        0, 0);

    return PGS_S_SUCCESS;
} 


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetSysShm

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_GetSysShm(
        PGSSmfGlbVar *global_var);

FORTRAN:
    NONE

DESCRIPTION:
    Get system shared memory into SMF static buffer.

INPUTS:   
    Name        Description                               Units        Min        Max              

    global_var  global static buffer

OUTPUTS:
    Name        Description                               Units        Min        Max 

RETURNS:       
    PGS_S_SUCCESS
    PGS_SH_SMF_SHMMEM
    PGS_SH_SMF_LOGFILE
    PGS_SH_SMF_MSSLOGFILE
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:        
    /#
     # This example here is meant to be followed in the init. process in main()
     # routine
     #/
    PGSt_SMF_status  returnStatus;
    PGSMemShmSize   *size;
    PGSt_SMF_status  useMemType;                  /# PGSd_SMF_USE_SHAREDMEM or PGSd_SMF_USE_ASCIIFILE #/
    PGSt_SMF_status  writeLogFile;                /# PGSd_SMF_WRITELOG_OFF  or PGSd_SMF_WRITELOG_ON #/

    PGS_SMF_InitProc(useMemType,writeLogFile);    /# first executable statement in main() #/
                                                  /# useMemType and writeLogFile are passed in by system #/
                                                  /# so Ray make sure you interprete it correctly and pass #/
                                                  /# in the correct constants #/

    ...
    ...
    ...

    size.pc = 1000;
    size.smf = sizeof(PGSSmfShm);
    PGS_MEM_ShmSysInit(&size);

    ...
    ...
    ...

    PGS_SMF_GetSysShm((PGSSmfGlbVar *)NULL);

NOTES:           
    This routine is only to be used internally by ARC toolkit developers.  

    It is used to set up the SMF shared memory so that a PGE process will 
    get data from the shared memory region.  If the SMF shared memory area
    could not be created, then virtual shared memory (ASCII file method)
    will be employed, with logicalID = PGSd_SMF_LOGICAL_SHMMEM.  The ASCII 
    file will be created in any case, since we need it to save the runtime 
    files.

    If an error occurs, either while trying to get system shared memory,
    or creating the ascii file, then SMF will set the message tag and log 
    files to NULL.

    This routine is called by the PGS_PC_InitCom utility just before it
    exits.  It is also called by the tool PGS_SMF_GetGlobalVar, during 
    the initialization phase - once at the start of each process.  This
    means that:

        ANY SMF TOOL THAT REQUIRES AN INITIALIZATION CALL AT THE
        START OF A PROCESS SHOULD BE CALLED AT THE END OF THIS TOOL.


REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    In normal operation, this tool is only called by PGS_SMF_GetGlobalVar
    during its initialization phase, i.e. the first time that the tool 
    PGS_SMF_GetGlobalVar gets called within a PGE process.

GLOBALS:
    PGSg_TSF_SMFFlag

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_MEM_ShmSysAddr()
    PGS_SMF_SetShmMemData()
    PGS_SMF_WriteBanner()
    PGS_SMF_SetShmMemToGlbVar()
    PGS_IO_Gen_Open()
    PGS_TSF_GetMasterIndex()
    PGS_SMF_TestErrorLevel()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_GetSysShm(             /* Get system shared memory */
    PGSSmfGlbVar *global_var)  /* Input global static buffer */
{    
    PGSt_SMF_status           returnStatus;                   /* return status */ 
    PGSt_SMF_status           returnStatus1;                  /* return status */ 
    PGSt_uinteger             sizeSmf;                        /* SMF shared memory size */
    char                     *smfAddr;                        /* SMF shared memory pointer */
    char                     *useSharedMem = (char*)NULL;     /* use shared memory or ascii file */
    char                      file[PGSd_SMF_PATH_MAX];        /* ascii file for shared memory */
    char                      msg[PGS_SMF_MAX_MSGBUF_SIZE];      /* message */
    PGSt_IO_Gen_FileHandle   *fptrShmMem =                    /* SMF shared memory file */
                                  (PGSt_IO_Gen_FileHandle *)NULL;      
    PGSSmfShm                 dataSmf;                        /* SMF shared memory data */
    PGSt_integer              version = 1;                    /* version number */
    static PGSt_SMF_boolean   alreadyDelete = PGS_FALSE;      /* delete ascii file */
    PGSt_SMF_status           returnMSSStatus;                /* return status */ 

#ifdef _PGS_THREADSAFE
    /*
     * Set up to get the initialization flag
     */
    PGSt_SMF_boolean initFlag;
    extern PGSt_SMF_boolean PGSg_TSF_SMFFlag[];
    int masterTSFIndex;

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
        goto DONE;
    }
    initFlag = PGSg_TSF_SMFFlag[masterTSFIndex];
#endif


    /*
     * These variables MUST be initialized at the beginning of each
     * process in order for the MSS Event Logging functionality to 
     * operate properly.
     */
#ifdef PGS_IR1
     returnMSSStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_SNMP_CONNECT,
                                            comm_snmp);
     returnStatus = PGS_PC_GetReference(PGSd_SMF_LOGICAL_EVENTLOG,
                                        &version,
                                        eventLogRef);
     if ((returnMSSStatus != PGS_S_SUCCESS) ||
	 (returnStatus != PGS_S_SUCCESS))
     {
         strcpy( comm_snmp,PGSd_SMF_OFF ); /* this will short-circuit the 
					      Event Log code entirely */
	 returnMSSStatus = PGS_SH_SMF_MSSLOGFILE;  
     }
#else
     /* 
      * returnMSSStatus is not interesting in this (non-PGS_IR1) case.
      * Since this function (PGS_SMF_GetSysShm()) will return returnMSSStatus
      * on exit IF returnStatus is PGS_S_SUCCESS, setting returnMSSStatus to
      * PGS_S_SUCCESS here has the effect of returning the value of returnStatus
      * for this non-PGS_IR1 case (see DONE: label below).
      */
     returnMSSStatus = PGS_S_SUCCESS;
#endif /* PGS_IR1 */

    /*
     * Check if system wants to use real shared memory or ASCII file or 
     * the TK3 method. Take note that useMemType is also being set in 
     * PGS_SMF_InitProc().
     */
    useSharedMem = getenv("PGSMEM_USESHM");

    if (useSharedMem == (char *)NULL)
    {
        /*
         * The environment variable PGSMEM_USESHM was not set, so use the 
         * TK3 method. In this method, only one big process exists.  We 
         * still need to create the ASCII file to support the function
         * PGS_SMF_SendRuntimeData().  Note that this ASCII file is an 
         * enhancement for TK4. It did not exist in TK3.
         */ 

        useMemType = PGSd_SMF_USE_ASCIIFILE;

        if (alreadyDelete == PGS_FALSE)    /* delete the old file if found */
        {
            if (PGS_PC_GetReference(PGSd_SMF_LOGICAL_SHMMEM, &version, file)
                 == PGS_S_SUCCESS)
            {
                alreadyDelete = PGS_TRUE;
                remove(file);
            }
        }         

        /*
         * Copy the log file names and message tag information to the  
         * SMF shared memory variable (dataSmf).
         * Then use it to write a banner to each of the log files.
         */
        if ((returnStatus = PGS_SMF_SetShmMemData(&dataSmf)) == PGS_S_SUCCESS)
        {
            PGS_SMF_WriteBanner(&dataSmf);
        }

        /*
         * Copy the values from dataSmf into global_var.
         */            
        if (global_var != (PGSSmfGlbVar *)NULL)
        { 
            PGS_SMF_SetShmMemToGlbVar(&dataSmf, global_var);                    
        }

        /*
         * Create the ASCII file and write the data to it.
         */
#ifdef _PGS_THREADSAFE
        /*
         * Do this the first time in each THREAD.
         */
        if ((PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_SHMMEM,PGSd_IO_Gen_Write,
			    &fptrShmMem,1) == PGS_S_SUCCESS) &&
                            (initFlag == PGS_FALSE))
        {  
            /*
             * The following values let SMF running within a PGE know about:
             * 
             * (1) dataSmf.writeLogFile = enable/disable error logging
             * (2) dataSmf.msgTag       = message tag to use
             * (3) dataSmf.logStatus    = status log file path and filename
             * (4) dataSmf.logReport    = report log file path and filename
             * (5) dataSmf.logUser      = user log file path and filename 
             *
             *  AND since we are in threadsafe mode we need to lock
             *  when we write to the file
             */
            returnStatus = PGS_TSF_LockIt(PGSd_TSF_SMFWRITELOCK);
            if (PGS_SMF_TestErrorLevel(returnStatus))
            {
                returnStatus = PGSTSF_E_GENERAL_FAILURE;
                goto DONE;
            }

            fprintf(fptrShmMem,"%d\n",dataSmf.writeLogFile);   
            fprintf(fptrShmMem,"%s\n",dataSmf.msgTag);
            fprintf(fptrShmMem,"%s\n",dataSmf.logStatus);
            fprintf(fptrShmMem,"%s\n",dataSmf.logReport);
            fprintf(fptrShmMem,"%s\n",dataSmf.logUser);
            fclose((FILE *)fptrShmMem);
            returnStatus = PGS_TSF_UnlockIt(PGSd_TSF_SMFWRITELOCK);

            /*
             *  Unlock here
             */
            if (PGS_SMF_TestErrorLevel(returnStatus))
            {
                returnStatus = PGSTSF_E_GENERAL_FAILURE;
                goto DONE;
            }
        }
#else
        if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_SHMMEM,PGSd_IO_Gen_Write,
			    &fptrShmMem,1) 
            == PGS_S_SUCCESS)
        {  
            /*
             * The following values let SMF running within a PGE know about:
             * 
             * (1) dataSmf.writeLogFile = enable/disable error logging
             * (2) dataSmf.msgTag       = message tag to use
             * (3) dataSmf.logStatus    = status log file path and filename
             * (4) dataSmf.logReport    = report log file path and filename
             * (5) dataSmf.logUser      = user log file path and filename 
             */
            fprintf(fptrShmMem,"%d\n",dataSmf.writeLogFile);   
            fprintf(fptrShmMem,"%s\n",dataSmf.msgTag);
            fprintf(fptrShmMem,"%s\n",dataSmf.logStatus);
            fprintf(fptrShmMem,"%s\n",dataSmf.logReport);
            fprintf(fptrShmMem,"%s\n",dataSmf.logUser);
            fclose((FILE *)fptrShmMem);
        }
#endif
        else
        {
            /*
             * If unable to create the ASCII file, then 
	     * PGS_SMF_SendRuntimeData() could not save the runtime files.
             */
        }

        /* useMemType = PGSd_SMF_USE_ASCIIFILE; */
        returnStatus = PGS_S_SUCCESS;
        goto DONE;
    }
    else
    {         
        /*
         * The environment variable PGSMEM_USESHM was set, so we try to use 
         * real shared memory.
         */

        /*
         * If this process is an initialization process, we must get rid of
         * the old ASCII file (logicalID == PGSd_SMF_LOGICAL_SHMMEM) to prevent
         * SMF from reading in obsolete data from the previous session, i.e the
         * last run of an ARC-PGE shell script. 
         *
         * We also the set the alreadyDelete flag is to prevent SMF from 
         * deleting the ASCII file the next time PGS_SMF_GetSysShm() is
         * called.  This happens when PGS_SMF_GetGlobalVar() is called 
         * during PGE processes.
         */
        if (whichProc == PGSd_SMF_PROC_INIT)    /* initialization process */
        {    
            if (alreadyDelete == PGS_FALSE)
            {
                if (PGS_PC_GetReference(PGSd_SMF_LOGICAL_SHMMEM,&version,file)
                    == PGS_S_SUCCESS)
                {
                    alreadyDelete = PGS_TRUE;
                    remove(file);
                }
            }                
        }      

        if (strcmp(useSharedMem,"YES") == 0)
        {            
            /*
             * The environment variable PGSMEM_USESHM was set to YES.  
             * Get SMF shared memory from system shared memory.
             */
            returnStatus = 
                PGS_MEM_ShmSysAddr(PGS_MEM_SHM_SMF, 
                                   (void **)&smfAddr, &sizeSmf);
        }
        else        
        {
            /*
             * The environment variable PGSMEM_USESHM was NOT set to YES.  
             * Override the system default (use shared memory) and set up
             * to use the ASCII file.
             */
            useMemType = PGSd_SMF_USE_ASCIIFILE;
            returnStatus = PGS_SH_SMF_SHMMEM;
        }
    }

    if (returnStatus == PGS_S_SUCCESS) 
    {       
        /*
         * We reach this point when the following conditions are satisfied:
         *
         * (1) Real (not ASCII) shared memory has been specified
         * (2) PGS_MEM_ShmSysInit() was called in the initialization process 
         *     (PGS_PC_InitCom), so system shared memory has been created 
         * (3) PGS_MEM_ShmSysAddr() has successfully attached to the system 
         *     shared memory region
         * (4) SMF shared memory has been retrieved
         */        

        if (whichProc == PGSd_SMF_PROC_INIT)    /* initialization process */
        {
            /*
             * Copy the log file names and message tag information to the  
             * SMF shared memory variable (dataSmf).
             * Then use it to write a banner to each of the log files.
             */
            if ((returnStatus = PGS_SMF_SetShmMemData(&dataSmf)) 
                == PGS_S_SUCCESS)
            {
                PGS_SMF_WriteBanner(&dataSmf);
            }

            /*
             * Copy SMF shared memory variable (dataSmf) to shared memory 
             * area using memcpy().   This will preserve the data for use
             * by the next process in the current PGE.
             * 
             * NOTE:   
             * We don't copy in the PGS_SMF_MsgCacheInfo sub-structure
             * values.  They are set elsewhere, (in the initialization section 
             * of PGS_SMF_CcheMsgShm), and we don't want to clobber them here.
             * 
             */
            memcpy( (char *) smfAddr,
                    (char *) &dataSmf, 
                    sizeof(PGSSmfShm) - sizeof(PGS_SMF_MsgCacheInfo) );  
        }
        else                                    /* termination or PGE process */
        {
            /*
             * populate the static global buffer
             */  
            if (global_var != (PGSSmfGlbVar *)NULL)
            { 
                /*
                 * Copy from shared memory area to the SMF shared memory 
                 * variable (dataSmf) using memcpy()
                 */
                memcpy( (char *) &dataSmf,
                        (char *) smfAddr,
                        sizeof(PGSSmfShm) ); 
  
                /*
                 * populate static global buffer from the SMF shared memory
                 */  
                PGS_SMF_SetShmMemToGlbVar(&dataSmf,global_var);
            }

            returnStatus = PGS_S_SUCCESS; 
        }  
    }
    else
    {
        /*
         * We reach this point when either:
         *
         * (1) PGS_MEM_ShmSysInit() has NOT been called in init. process. 
         *     This causes PGS_MEM_ShmSysAddr() to return a UNIX error.
         *     (PGS_PC_InitCom will always call PGS_MEM_ShmSysInit() if 
         *     useMemType = PGSd_SMF_USE_SHAREDMEM)
         *     or
         * (2) PGS_MEM_ShmSysInit() has been called in the init. process but 
         *     PGS_MEM_ShmSysAddr() detected UNIX error (either in init., 
         *     term. or PGE process)
         *     or
         * (3) useMemType = PGSd_SMF_USE_ASCIIFILE (system says use ASCII 
         *     file as shared memory)
         * 
         * Note:
         *     No matter what system says about using real shared memory method
         *     or ASCII file, we still have to have the artificial shared memory
         *     (ASCII file) available so PGS_SMF_SendRuntimeData() can save the
         *     logicalID to it.  This allows SMF to retrieve the logicalID
         *     during the termination process, PGS_PC_TermCom.  The ASCII file
         *     will also contain the log file information, message tag and flag
         *     that enables writing to log files.
         */
        if (whichProc == PGSd_SMF_PROC_INIT)    /* initialization process */  
        {   
            /*
             * (1) proceed to create virtual SMF shared memory (ASCII file)
             * (2) since this is called in init. process (or TK3 method), 
             *     we will create a new ascii file (mode = PGSd_IO_Gen_Write)
             * (3) also we have to make sure that the ascii file has been
             *     deleted first (done at the very top)
             */             
            if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_SHMMEM,PGSd_IO_Gen_Write,
				&fptrShmMem,1) 
                == PGS_S_SUCCESS)
            {          
                /*
                 * Copy the log file names and message tag information to the  
                 * SMF shared memory variable (dataSmf).
                 * Then use it to write a banner to each of the log files.
                 */
                if ((returnStatus = PGS_SMF_SetShmMemData(&dataSmf)) == 
                    PGS_S_SUCCESS)
                {
                    PGS_SMF_WriteBanner(&dataSmf);
                }

                /*
                 * The following values let SMF running within a PGE know about:
                 * 
                 * (1) dataSmf.writeLogFile = enable/disable error logging
                 * (2) dataSmf.msgTag       = message tag to use
                 * (3) dataSmf.logStatus    = status log file path and filename
                 * (4) dataSmf.logReport    = report log file path and filename
                 * (5) dataSmf.logUser      = user log file path and filename 
                 */
                fprintf(fptrShmMem,"%d\n",dataSmf.writeLogFile);   
                fprintf(fptrShmMem,"%s\n",dataSmf.msgTag);
                fprintf(fptrShmMem,"%s\n",dataSmf.logStatus);
                fprintf(fptrShmMem,"%s\n",dataSmf.logReport);
                fprintf(fptrShmMem,"%s\n",dataSmf.logUser);
                fclose((FILE *)fptrShmMem);
            }
            else
            {
                /*
                 * We reach this point when PGS_IO_Gen_Open() detected an error
                 * while trying to open the ASCII shared memory file. Probably
                 * because of  IO internal error or UNIX error.
                 * 
                 *   - No banner will be written, since subsequent SMF calls 
                 *     within the PGE process will not know where to get log 
                 *     files and message tag information
                 *     
                 *   - Expect PGS_SMF_Set... routines SMF within PGE process to 
                 *     return PGSSMF_E_LOGFILE along the way
                 */
                returnStatus = PGS_SH_SMF_SHMMEM;
            }

        }
        else                                    /* termination or PGE process */
        {
            /*
             * Since this is called in termination or PGE process, we will 
             * read information from the ASCII file.
             */       
            if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_SHMMEM, 
                PGSd_IO_Gen_Read, &fptrShmMem, 1) == PGS_S_SUCCESS)
            {   

                /*
                 * (1) read log files and message tag information 
                 *     (mode == PGSd_IO_Gen_Read)
                 * (2) remove '\n' character from the data
                 */ 
                if (fgets(msg, sizeof(msg), fptrShmMem) == (char *)NULL)
                {                    
                    dataSmf.writeLogFile = PGSd_SMF_WRITELOG_OFF;
                } 
                else
                {
                    dataSmf.writeLogFile = atoi(msg);
                }                    

                if (fgets(dataSmf.msgTag, sizeof(dataSmf.msgTag), fptrShmMem) 
                    == (char *)NULL)
                {
                    dataSmf.msgTag[0] = '\0';
                }
                else
                {                    
                    if (strlen(dataSmf.msgTag) != (size_t) 0)
                    {
                        dataSmf.msgTag[strlen(dataSmf.msgTag)-1] = '\0';
                    }
                }

                if (fgets(dataSmf.logStatus, sizeof(dataSmf.logStatus),
                    fptrShmMem) == (char *)NULL) 
                {
                    dataSmf.logStatus[0] = '\0';
                }
                else
                {
                    if (strlen(dataSmf.logStatus) != 0)
                    {
                        dataSmf.logStatus[strlen(dataSmf.logStatus)-1] = '\0';
                    }
                }

                if (fgets(dataSmf.logReport, sizeof(dataSmf.logReport), 
                    fptrShmMem) == (char *)NULL) 
                {
                    dataSmf.logReport[0] = '\0';
                }
                else
                {
                    if (strlen(dataSmf.logReport) != 0)
                    {
                        dataSmf.logReport[strlen(dataSmf.logReport)-1] = '\0';
                    }
                }

                if (fgets(dataSmf.logUser, sizeof(dataSmf.logUser), 
                    fptrShmMem) == (char *)NULL) 
                {
                    dataSmf.logUser[0] = '\0';
                }
                else
                {
                    if (strlen(dataSmf.logUser) != 0)
                    {
                        dataSmf.logUser[strlen(dataSmf.logUser)-1] = '\0';
                    }
                }

                fclose((FILE *) fptrShmMem);  
                returnStatus = PGS_S_SUCCESS;                        
            }
            else                        /* unable to open ASCII file */
            {
                /*
                 * We reach this point if either:
                 * (1) during init. process, PGS_IO_Gen_Open() could not create
		 *     the ascii file so there is no ascii file to read
                 *     or
                 * (2) ASCII file did get created, but it was during termination
		 *     or PGE process.  PGS_IO_Gen_Open() could not open the
                 *     ASCII file for reading for some reason.
                 */

                /* 
                 * Remedy is:
                 * (1) set log files to NULL so that SMF can interprete error
		 *     condition in log files
                 * (2) set message tag to NULL
                 * (3) expect PGS_SMF_Set... routines SMF within PGE process to
                 *     return PGSSMF_E_LOGFILE along the way
                 */
                dataSmf.writeLogFile = PGSd_SMF_WRITELOG_OFF;
                dataSmf.msgTag[0]    = '\0';
                dataSmf.logStatus[0] = '\0';
                dataSmf.logReport[0] = '\0';
                dataSmf.logUser[0]   = '\0';

                returnStatus = PGS_SH_SMF_SHMMEM;

            }   /* END: if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_SHMMEM... */

            /*
             * Populate dataSmf into the static global variable.
             */  
            if (global_var != (PGSSmfGlbVar *)NULL)
            {
                PGS_SMF_SetShmMemToGlbVar(&dataSmf,global_var); 
            }

        }  /* END: if (whichProc == PGSd_SMF_PROC_INIT) */                

    }  /* END: if (returnStatus == PGS_S_SUCCESS) */     


/*
 * Common exit point
 */

DONE:

    if (useMemType == PGSd_SMF_USE_SHAREDMEM)  
    {
        returnStatus1 = PGS_SMF_CacheMsgShm(PGSd_SMF_InitCacheInfo2, 0, 0, 0);
        if (returnStatus1 != PGS_S_SUCCESS)
        {
            return returnStatus1;
        }
    }
    
    /*
     * If returnStatus is NOT equal to PGS_S_SUCCESS at this point
     * then return returnStatus, otherwise...return returnMSSStatus.
     */

    if (returnStatus != PGS_S_SUCCESS)
    {
	return returnStatus;
    }
    
    /* 
     * If all else checks-out, then return any MSS error which 
     * may be set.
     */

    return returnMSSStatus;

} /* end PGS_SMF_GetSysShm */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetSystemCode

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_GetSystemCode(
        PGSt_SMF_code code,
        char         *mnemonic,
        char         *msg);

FORTRAN:
    NONE

DESCRIPTION:           
    Retrieve system information (SEED = 0); code that start with PGS_....

INPUTS:    
    Name        Description                               Units        Min        Max 

    code        system code

OUTPUTS:     
    Name        Description                               Units        Min        Max             

    mnemonic    system mnemonic
    msg         system message    

RETURNS:
    NONE

EXAMPLES:
    char   mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];
    char   msg[PGS_SMF_MAX_MSGBUF_SIZE]; 

    PGS_SMF_GetSystemCode(PGS_S_SUCCESS,mnemonic,msg);

NOTES:         
    This routine is normally used in conjunction with PGS_SMF_DecodeCode().

REQUIREMENTS:               
    PGSTK-0580,0590,0650 

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void 
PGS_SMF_GetSystemCode(           /* Get system information */
    PGSt_SMF_code  code,         /* Input  system code */
    char          *mnemonic,     /* Output system mnemonic */
    char          *msg)          /* Output system message */
{   

    switch (code)
    {
        case PGS_S_SUCCESS:     strcpy(mnemonic,"PGS_S_SUCCESS");
                                strcpy(msg,"SUCCESSFUL operation");
                                break;

        case PGS_M_UNIX:        strcpy(mnemonic,"PGS_M_UNIX");
                                strcpy(msg,"UNIX errno message");
                                break;                              

        case PGS_E_HDF:         strcpy(mnemonic,"PGS_E_HDF");
                                strcpy(msg,"HDF error");
                                break; 

        case PGS_E_UNIX:        strcpy(mnemonic,"PGS_E_UNIX");
                                strcpy(msg,"UNIX error");
                                break; 

        case PGS_E_ECS:         strcpy(mnemonic,"PGS_E_ECS");
                                strcpy(msg,"ECS error");
                                break; 

        case PGS_E_TOOLKIT:     strcpy(mnemonic,"PGS_E_TOOLKIT");
                                strcpy(msg,"TOOLKIT error");
                                break;  

        case PGS_F_TOOLKIT:     strcpy(mnemonic,"PGS_F_TOOLKIT");
                                strcpy(msg,"TOOLKIT fatal error");
                                break; 

        case PGS_E_GEO:         strcpy(mnemonic,"PGS_E_GEO");
                                strcpy(msg,"GEOLOCATION error");
                                break; 

        case PGS_E_DCE:         strcpy(mnemonic,"PGS_E_DCE");
                                strcpy(msg,"DCE error");
                                break; 

        case PGS_E_MATH:        strcpy(mnemonic,"PGS_E_MATH");
                                strcpy(msg,"MATH error");
                                break; 

        case PGS_E_ENV:         strcpy(mnemonic,"PGS_E_ENV");
                                strcpy(msg,"ENVIRONMENT error");
                                break;                                        

        default:                mnemonic[0] = '\0';
                                msg[0] = '\0';
                                break;                                  
    }                               


} /* end PGS_SMF_GetSystemCode */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Get thread ID

NAME:   
    PGS_SMF_GetThreadID

SYNOPSIS:   
C:      
    #include <PGS_SMF.h>      

    char *
    PGS_SMF_GetThreadID();

FORTRAN:
    N/A

DESCRIPTION:
    This tool gets the internal thread id that is printed int the 
    log status report.

INPUTS:    
    Name            Description                                Units        Min        Max

    NONE

OUTPUTS:    
    Name            Description                                Units        Min        Max

RETURNS:

EXAMPLES:
    C:
    ==  
    char threadID[100];

    threadID = PGS_SMF_GetThreadID();

    FORTRAN:
    NONE

NOTES:  
    The thread id is not a POSIX supplied thread id.  It is made up in the 
    TSF code.

REQUIREMENTS:

DETAILS:	    

GLOBALS:
    NONE

FILES:

FUNCTIONS_CALLED:
    PGS_SMF_TestErrorLevel()
    PGS_TSF_GetTSFMaster()

END_PROLOG:
*****************************************************************/
char *PGS_SMF_GetThreadID()
{
    static char threadInfo[20] = "";
  
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status returnStatus;
    int threadId;
    PGSt_TSF_MasterStruct *masterTSF;

    /* 
     *  Get TSF thread info and then get thread id from TSD
     */
    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        return (char *) NULL;
    }
    threadId = (int) pthread_getspecific(masterTSF->keyArray[PGSd_TSF_KEYSMFTHREADID]);

    sprintf(threadInfo," [TID=%d] ",threadId);

#else
    threadInfo[0] = '\0';
#endif

    return threadInfo;
}

/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetUserCode

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_GetUserCode(
        PGSt_SMF_code    code, 
        char             *mnemonic, 
        PGS_SMF_MsgInfo  *codeinfo,  
        char             *funcname,
        short             op) 

FORTRAN:
    NONE

DESCRIPTION:
    This tool searches for the specified code.  If found, the code information 
    is stored in the output variable.

INPUTS:    
    Name        Description                               

    code        numeric value for error/status code generated 
                by message compiler.  Also used to determine
                the file seed number.

    mnemonic    mnemonic label for error/status code created 
                by developers and users

    funcname    function name passed from caller

    op          operation 
                 0 == search based on code value
                 1 == search based on code mnemonic


OUTPUTS:    
    Name        Description 

    codeinfo    code information retrieved

RETURNS:        
    PGS_S_SUCCESS    
    PGS_E_UNIX
    PGSSMF_E_UNDEFINED_CODE


EXAMPLES:
    PGSt_SMF_status   returnStatus;

    PGSt_SMF_code    code;
    char             *mnemonic;
    PGS_SMF_MsgInfo  code_info;
    short             op;

    code = PGS_GRP_SomeTool();

    returnStatus =
        PGS_SMF_GetUserCode(code, 0, &code_info,"PGS_GRP_SomeTool()",0);  
    if (returnStatus == PGS_E_UNIX)
    {
        /# Encountered a Unix error #/
    }

NOTES:          
    There should be an environment variable "PGSMSG" defined to point to 
    the directory that contains all the ASCII message files. 

    When mnemonic search is specied, there are two ways of determining the
    file seed value, which is used to open the correct message file:

     1) If input parameter 'code' is set to a positive number, it will be
        used to derive the file seed value.  This method is used when the 
        tool is called by PGS_SMF_GetActionByCode.

     2) If input parameter 'code' is zero or negative, an internal table
        is consulted.  In this case, only the PGS-defined message files
        will be searched.   If new tool groups are added, this table will 
        have to be updated.  This search capability is not currently 
        supported for SMF or toolkit SYSTEM messages.

REQUIREMENTS:               
    PGSTK-0580,0590,0650

DETAILS:	   
    This tool is only intended to be called by PGS_SMF_DecodeCode.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CacheMsgShm()
    PGS_SMF_GetSystemCode()
    PGS_SMF_ExtractFileInfo()
    PGS_SMF_ExtractMsgInfo()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_GetUserCode(
    PGSt_SMF_code    code,            /* Input  code */
    char             *mnemonic,       /* Input  mnemonic label */
    PGS_SMF_MsgInfo  *codeinfo,       /* Output code information retrieved */
    char             *funcname,       /* Input  function name */
    short             op)             /* Input  operation: */
                                      /*   0 == search based on code value */
                                      /*   1 == search based on code mnemonic */
{
    PGSt_SMF_status         returnStatus = PGS_S_SUCCESS; /* return status */

    PGSt_SMF_code           fileno = 0;         /* file seed number */

    PGSt_SMF_boolean        code_found = PGS_FALSE;

    PGS_SMF_MsgInfo         ci_local;           /* local code info buffer */

    int                     search_type;        /* type of cache search to do */
    
    char                    msg[PGS_SMF_MAX_MSGBUF_SIZE];       /* code message */
    char                    mne[PGS_SMF_MAX_MNEMONIC_SIZE];  /* code mnemonic */
    char                    filepath[500];                   /* msg file path */
    char                    msgbuf[PGS_SMF_MAX_MSGBUF_SIZE]; /* msg buffer */

    char                    *label;                          /* label */

    FILE                    *fp = (FILE *)NULL;
    PGSt_integer	    pgs_errno;		/* temporary errno holder */

    /*
     * List of tool labels used to derive the value of fileno
     * when mnemonic search is specified.  The index to a given
     * label is equal to seed value for that label.
     * 
     * NOTES:
     *   User supplied tools are not supported.  
     *   If the new toolkit groups are added, this must be updated.
     */ 

    static char *seeds[PGSd_SMF_TOOLKIT_SMFS] = 
    { 
        "PGSxxx", "PGSIO",  "PGSxxx", "PGSTD",  "PGSCSC", 
        "PGSEPH", "PGSCBP", "PGSMEM", "PGSxxx", "PGSPC",  
	"PGSAA",  "PGSCUC", "PGSGCT", "PGSxxx", "PGSDEM",
        "PGSTSF"
    };


    memset((void*) &ci_local, 0, sizeof(PGS_SMF_MsgInfo));
    
    if (op == 0) /* derive seed number based on code */
    {
        fileno = (PGSt_SMF_code)(code & PGS_SMF_MASK_SEED) >> 13;
    }
    else   /* derive seed number based on mnemonic, if code not avail. */
    {
        if (code > 0) /* derive seed number based on code */
        {
            fileno = (PGSt_SMF_code)(code & PGS_SMF_MASK_SEED) >> 13;
        }
        else
        {
#ifdef _PGS_THREADSAFE
            char *lasts;    /* used by strtok_r() */

            /*
             * Since strtok() is not threadsafe we will use strtok_r() when
             * in threadsafe mode
             */
            strcpy(msgbuf, mnemonic);
            label = strtok_r(msgbuf,"_",&lasts);
#else
            strcpy(msgbuf, mnemonic);
            label = strtok(msgbuf,"_");
#endif

            for( fileno=0; fileno <PGSd_SMF_TOOLKIT_SMFS; fileno++)
            {
                if( strcmp(seeds[fileno], label) == 0 ) break;
            }
            if (fileno == PGSd_SMF_TOOLKIT_SMFS) 
	    {
	      return PGS_E_TOOLKIT;  /* unknown seed */
	    }
        }
    }

    /*
     * Search the cache before attempting to find the code in a message file.
     * This is done to improve performance.
     */

    if (op == 0) /* search based on code */
    {
	search_type = PGSd_SMF_FindByCode;
    }
    else         /* search based on mnemonic */
    {
	search_type = PGSd_SMF_FindByMnemonic;
    }
    
    if (useMemType == PGSd_SMF_USE_SHAREDMEM)  
    {
        returnStatus = PGS_SMF_CacheMsgShm(search_type,code,mnemonic,&ci_local);
    }
    else
    {
        returnStatus = PGS_SMF_CacheMsgDynm(search_type,code,mnemonic,
					    &ci_local);
    }
    
    if (returnStatus == PGS_S_SUCCESS) /* Code was found in the cache */
    {
	
	code_found = PGS_TRUE;         /* flag code found */
	
    }


    /* 
     * Code was not yet found, either because it was not in the cache
     * or shared memory is disabled and caching is turned off.  
     * 
     * So, we must try to find it in a message file.
     */

    if (code_found == PGS_FALSE)
    {

        /*
         * Get the path-extended name of the message file
         */

	sprintf(filepath, "%s/%s_%d", getenv("PGSMSG"), 
		PGS_SMF_SYS_FILEPREFIX,fileno);

        /*
         * Attempt to open the file
         */

	fp = fopen( filepath, "r" );

        /*
         * Search the file for the message
         */

 	if (fp != 0) 
	{
	    /* file was opened succesfully */

	    returnStatus = PGSSMF_E_UNDEFINED_CODE;  
	    
	    fgets(msgbuf, sizeof(msgbuf), fp);           /* read header */
	    PGS_SMF_ExtractFileInfo(&ci_local, msgbuf);  /* get file info */
	    
	    while(fgets(msgbuf, sizeof(msgbuf),fp) != (char *)NULL)
	    {
		PGS_SMF_ExtractMsgInfo(&ci_local, msgbuf); /* get code info */

		if (op == 0) /* search based on code */
		{
		    if (ci_local.msgdata.code == code)
		    {
			if (strcmp(ci_local.msgdata.action,PGS_NULL_STR) == 0)
			{
			    ci_local.msgdata.action[0] = '\0';
			}

			code_found = PGS_TRUE;             /* flag code found */

			break;    /* exit while() */
		    }
		}
		else         /* search based on mnemonic */
		{
		    if (strcmp(ci_local.msgdata.mnemonic, mnemonic) == 0)
		    {
			if (strcmp(ci_local.msgdata.action,PGS_NULL_STR) == 0)
			{
			    ci_local.msgdata.action[0] = '\0';
			}

			code_found = PGS_TRUE;             /* flag code found */

			break;    /* exit while() */
		    }
		}
		
	    } /* end: while(fgets(... */


	    /* if message found, add to the appropriate cache */ 
	    
	    if (code_found == PGS_TRUE)
	    {
		if (useMemType == PGSd_SMF_USE_SHAREDMEM)
		{
		    returnStatus = PGS_SMF_CacheMsgShm(PGSd_SMF_AddRecord, 
						       0, 0, &ci_local);
		}
		else
		{
		    returnStatus = PGS_SMF_CacheMsgDynm(PGSd_SMF_AddRecord, 
							0, 0, &ci_local);
		}
	    }
	    

            /*
             * If shared memory is available then the message cache is active.
             * We open one file at a time and close it when we're done.
             */

	    fclose(fp);             /* close file */
	}
	else                        /* file not found */
	{
            /* 
             * This is the case where no ascii message file is found, 
             * given the specified code.
             */

            /*
	     * added this safety feature to guard against original
	     * errno value from being clobbered.
	     */
	    pgs_errno = errno;
	    
            memset((char *)codeinfo, '\0', sizeof(PGS_SMF_MsgInfo));   
            codeinfo->fileinfo.seed = PGS_SMF_SYS_SEED;
            strcpy(codeinfo->fileinfo.instr, PGS_SMF_SYS_INSTR);
            strcpy(codeinfo->fileinfo.label, PGS_SMF_SYS_LABEL);

            strcpy(codeinfo->funcname, funcname);
            codeinfo->msgdata.action[0] = '\0';
            codeinfo->msgdata.code = PGS_E_UNIX;

            PGS_SMF_GetSMFCode(PGSSMF_E_CANT_OPEN_FILE, mne, msg);

            PGS_SMF_GetSystemCode(PGS_E_UNIX,
                                  codeinfo->msgdata.mnemonic,
                                  codeinfo->msgdata.msg);
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
            sprintf(codeinfo->msgdata.msg, msg, filepath, 
		    strerror(pgs_errno));
#else
            sprintf(codeinfo->msgdata.msg, msg, filepath,
                    sys_errlist[pgs_errno]);
#endif
            returnStatus = PGS_E_UNIX;

	} /* end: if (fp != 0) */

    } /* end: if (code_found == PGS_FALSE) */
    
    /*
     * Code was found: store it in the return variable
     */
    if (code_found == PGS_TRUE)
    {
	strcpy(ci_local.funcname, funcname);
	memcpy((char *)codeinfo,
	       (char *)&ci_local,
	       sizeof(PGS_SMF_MsgInfo));
	returnStatus = PGS_S_SUCCESS;
    }
    
    return returnStatus;
}







/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_Get_WriteLogFile

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_Get_WriteLogFile(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    Return the static variable status 'writeLogFile'.

INPUTS:    
    Name        Description                               Units        Min        Max       

OUTPUTS:
    Name        Description                               Units        Min        Max 

RETURNS:       
    PGSd_SMF_WRITELOG_ON
    PGSd_SMF_WRITELOG_OFF

EXAMPLES:    
    PGSt_SMF_status writeLogFile;

    writeLogFile = PGS_SMF_WriteLogFile();

NOTES:           
    This routine is only to be used internally by ARC toolkit developers.
    It is used to return the static variable 'callerID' to indicate to PC
    and IO tools that SMF is in initilization phase so do not call
    PGS_SMF_Set... tools.

REQUIREMENTS:

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_Get_WriteLogFile(   /* Get writeLogFile flag */
    void)
{    
    return writeLogFile;

} /* end PGS_SMF_Get_WriteLogFile */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_HandleLogErr

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_HandleLogErr(
        PGSSmfGlbVar *global_var);

FORTRAN:
    NONE

DESCRIPTION:
    Handles both SMF and MSS log file errors.

INPUTS:    
    Name        Description                               Units        Min        Max       

    global_var  global static buffer

OUTPUTS:
    Name        Description                               Units        Min        Max 

RETURNS:       
    NONE

EXAMPLES:    

NOTES:           
    NONE

REQUIREMENTS:

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void
PGS_SMF_HandleLogErr(             /* Handle log file error */
    PGSSmfGlbVar *global_var)     /* Input global static buffer */
{    
    global_var->msginfo.fileinfo.seed = PGS_SMF_SYS_SEED;
    strcpy(global_var->msginfo.fileinfo.instr,PGS_SMF_SYS_INSTR);
    strcpy(global_var->msginfo.fileinfo.label,PGS_SMF_SYS_LABEL);
    global_var->msginfo.msgdata.action[0] = '\0';            
    PGS_SMF_GetSMFCode(global_var->msginfo.msgdata.code,
		       global_var->msginfo.msgdata.mnemonic,
		       global_var->msginfo.msgdata.msg);

    if (getenv("PGS_PC_INFO_FILE") == (char *)NULL)
    {
        strcpy(global_var->msg,
	       "Error due to 'PGS_PC_INFO_FILE' environment not set");
    }
    /*
     * For what it's worth, this is the correct msg to set.
     */
    else if (whichProc == PGSd_SMF_PROC_TERM)
    {
       strcpy(global_var->msg,"Log files have already been closed!");
    }
    else if (global_var->msginfo.msgdata.code == PGSTSF_E_MUTEXLOCK_FAIL)
    {
       strcpy(global_var->msg,"Problems setting SMF Write lock");

    }
    else
    {
        if (global_var->log.fptrStatus == (FILE *)NULL) 
        {
            strcpy(global_var->msg,"Error opening status log file");
        }
        else if (global_var->log.fptrUser == (FILE *)NULL)
        {
            strcpy(global_var->msg,"Error opening user log file");
        }
        else
        {                    
            strcpy(global_var->msg,"Error opening status and user log files");
        }
    }

} /* end PGS_SMF_HandleLogErr */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_InitProc

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_InitProc(
        PGSt_SMF_status useMem,
        PGSt_SMF_status writeLog);

FORTRAN:
    NONE

DESCRIPTION:
    Set status to indicate to SMF that the running process is an
    initialization process. It has to be called in the initialization
    process main() (the first executable statetememt).

INPUTS:   
    Name        Description                               Units        Min        Max              

    useMem      use shared memory or ascii file
    writeLog    write errors to log file

OUTPUTS:
    Name        Description                               Units        Min        Max 

RETURNS:       
    NONE

EXAMPLES:        
    Refer to  PGS_SMF_GetSysShm().

NOTES:           
    This routine is only to be used internally by ARC toolkit developers.    

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    The main reason why this function is needed is to let SMF
    know that the current process is in fact initialization
    process. If current process is initialization process, then
    PGS_SMF_GetGlobalVar() will not call PGS_SMF_GetSysShm()
    to intialize system shared memory to SMF. Otherwise cyclic
    phenomena will occur.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void
PGS_SMF_InitProc(                 /* Initialization process */
    PGSt_SMF_status useMem,       /* Input use shared memory or ascii file */
    PGSt_SMF_status writeLog)     /* Input write errors to log file */
{

    whichProc    = PGSd_SMF_PROC_INIT;
    callerID     = PGSd_CALLERID_SMF;
    useMemType   = useMem;
    writeLogFile = writeLog;


} /* end PGS_SMF_InitProc */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_IsSystemCode

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_IsSystemCode(
        PGSt_SMF_code code);

FORTRAN:
    NONE

DESCRIPTION:
    To determine if the specified code belongs to system code (SEED = 0);
    code that start with PGS_...       

INPUTS:    
    Name        Description                               Units        Min        Max 

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:           
    NONE

RETURNS:
    PGS_TRUE
    PGS_FALSE

EXAMPLES:
    PGSt_SMF_boolean returnBoolean;

    returnBoolean = PGS_SMF_IsSystemCode(PGS_S_SUCCESS);
    if (returnStatus == PGS_TRUE)
    {
        /# It is system code #/
    }

NOTES:          
    This routine is normally used in conjunction with PGS_SMF_DecodeCode().

REQUIREMENTS:              
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_boolean 
PGS_SMF_IsSystemCode(       /* Determine if it is system code */
    PGSt_SMF_code code)     /* Input code */
{
    if (((PGSt_SMF_code)(code & PGS_SMF_MASK_SEED) >> 13) == PGS_SMF_SYS_SEED)
    {
        return(PGS_TRUE);
    }
    else
    {
        return(PGS_FALSE);
    }

} /* end PGS_SMF_IsSystemCode */

/*************************************************************************
BEGIN_PROLOG:

TITLE:  
    Send a message to then MSS event logger

NAME:   
    PGS_SMF_LogEvent

SYNOPSIS:       
C:  
    #include <EcUtLoggerC.h>
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_LogEvent(
        PGSt_SMF_code statusCode,
        char *messageString,
        char *messageLabel,
	char *actionString,
        PGSt_SMF_code mssDisposition);

FORTRAN:
    NONE

DESCRIPTION:
    This tool checks the specified status code to see if an associated
    action string is of a type that triggers the event logger interface.
    If this is so, then a message packet is constucted and passed to the 
    MSS event logger interface.

INPUTS:   
    Name            Description
       
    statusCode      status code
    messageString   associated message string
    messageLabel    associated mnemonic message label
    actionString    associated action string
    mssDisposition  Action descriptor

OUTPUTS:
    NONE

RETURNS:       
    PGS_S_SUCCESS
    PGSSMF_E_BAD_EVENTLOG_ACCESS
    PGS_E_TOOLKIT

EXAMPLES:        


NOTES:           
    This is a low-level tool. 

REQUIREMENTS:
    PGSTK-

DETAILS:	   

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetActionMneByCode
    PGS_SMF_GetActionType
    EcUtInitLogger
    EcUtLogC
    EcUtDeleteLogger

END_PROLOG:
**************************************************************************/

#ifdef PGS_IR1

PGSt_SMF_status 
PGS_SMF_LogEvent(
    PGSt_SMF_code statusCode,
    char *messageString,
    char *messageLabel,
    char *actionString,
    PGSt_SMF_code mssDisposition)
{

    PGSt_SMF_status         returnStatus = PGS_S_SUCCESS;   

    /*
     * MSS Interface Support Variables : DAAC Toolkit only 
     */

    char 		eventBuf[1024];

    EcTUtLogger         *loggerObject = 0;       /* event logger object */
    EcTInt		loggerStatus = 0;        /* return status */

    EcTChar *appName =  "SDP Toolkit";
    EcTChar *appVersion = "DAAC Release";
    EcTChar *fileName = 0;

    EcTULongInt eventNo;
    EcTUShortInt severityCode;
    EcTUShortInt eventDisp;
    time_t eventTime;
    EcTChar *eventMessage;

    PGSt_integer    version;

    PGSt_integer valid_mss_event;                /* indicate valid MSS event */


    /*
     * Determine the event type
     * If MSS event, set flag and do type-specific processing
     */

    switch ( mssDisposition ) 
    {

      case PGSSMF_M_INFO_ACTION:     /* write info message to local log file */

        valid_mss_event = 1;         /* this is a valid MSS event */

        /* set disposition to write to the local log file only */

        eventDisp = EcDUtNone;

        /* format the event message buffer */

	sprintf (
	    eventBuf,
	    "Action String: %s\nReturn status: %s\nMessage: %s\n",
	    actionString ,
	    messageLabel , 
	    messageString );

        break;


      case PGSSMF_M_EMAIL_ACTION:    /* send email, write log files */

        valid_mss_event = 1;         /* this is a valid MSS event */

        /* 
         * set disposition to inform operator via snmp trap
         * (local and remote log files, but no OpenView pop-up)
         */

        eventDisp = EcDUtInformSMCLogOnly;

        /* format the event message buffer */

	sprintf (
	    eventBuf,
	    "To: %s\nSubject: Toolkit return status %s\n\n%s\n",
	    actionString ,
	    messageLabel , 
	    messageString );

        break;


      default:

        valid_mss_event = 0;         /* this is NOT a valid MSS event */

        break;

    } /* end : switch( mssDisposition ) */


    /*
     * If MSS event, do common event processing
     */

    if ( valid_mss_event )
    {

        /*
         *  Retrieve Event Logger file name from PCF
         *  Static variable 'eventLogRef' defined globally
         *  (old: fileName = "./eventLogger.log"; )
         */

        fileName = eventLogRef;

        /* derive severity from statusCode */

        switch(PGS_SMF_TestStatusLevel(statusCode))
        {

          case PGS_SMF_MASK_LEV_M:	    /* message level status */
            severityCode = EcDUtMessage;    /* map to EcUt severity level */
            break;

          case PGS_SMF_MASK_LEV_U:	    /* user information level status */
            severityCode = EcDUtUserInfo;   /* map to EcUt severity level */
            break;

          case PGS_SMF_MASK_LEV_S:	    /* success level status */
            severityCode = EcDUtSuccess;    /* map to EcUt severity level */
            break;

          case PGS_SMF_MASK_LEV_N:	    /* notice level status */
            severityCode = EcDUtNotice;	    /* map to EcUt severity level */
            break;

          case PGS_SMF_MASK_LEV_W:	    /* warning level status */
            severityCode = EcDUtWARNING;    /* map to EcUt severity level */
            break;

          case PGS_SMF_MASK_LEV_F:	     /* fatal level status */
            severityCode = EcDUtFATAL;	     /* map to EcUt severity level */
            break;

          case PGS_SMF_MASK_LEV_E:	     /* error level status */
            severityCode = EcDUtERROR;	     /* map to EcUt severity level */
            break;

          default:           		     /* undefined status level */
            severityCode = EcDUtUndefined;   /* map to EcUt severity level */
            break;   

        } /* end : switch(PGS_SMF_TestStatusLevel(statusCode)) */


        /* set event number */

        eventNo = statusCode;

        /* get event time */

        time( &eventTime );

        /* get event message */

        eventMessage = eventBuf;
		
        /* initialize the event logger object */

        loggerObject = EcUtInitLogger(
            appName,
            appVersion,
            fileName,
            &loggerStatus);

        /* check status and proceed if OK */

        if ( loggerStatus != EcDUtSuccess )
        {
            returnStatus = PGSSMF_E_BAD_EVENTLOG_ACCESS;
        }
        else
        {
            /* call the main logger function. */

            loggerStatus = EcUtLogC(
                loggerObject,
                eventNo,
                severityCode,
                eventDisp,
                &eventTime,
                eventMessage);

            if ( loggerStatus != EcDUtSuccess )
            {
                returnStatus = PGSSMF_E_BAD_EVENTLOG_ACCESS;
            }

            /* delete the event logger object */

            EcUtDeleteLogger( loggerObject );

        } /* end : if ( (loggerStatus ...  ) */

    } /* end : if ( valid_mss_event ) */

    return returnStatus;         /* done */
    
} /* end PGS_SMF_LogEvent */

#endif /* PGS_IR1 */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_MsgLevel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGSt_SMF_status
    PGS_SMF_MsgLevel(
        PGSt_integer    cmd,
        PGSt_integer    *msg_level,
        PGSt_integer    *log_off,
        char            **indent_str,
        char            *func_name);

FORTRAN:
    NONE

DESCRIPTION:
    This tool keeps track of the current message level and indent string.
    The action to be take depends on the value of the input parameter 'cmd'.
    Setting cmd PGSd_SMF_IncLevel will cause the level to be incremented,
    while setting it to PGSd_SMF_DecLevel will cause the level to be
    decremented.  Setting cmd to PGSd_SMF_GetLevel allows the caller to 
    retrieve the current level and function name along with an associated 
    indent string, suitable for status logging.  

    By default, message logging is turned on for a new level, whenever it 
    is set.  Message logging may be disabled for the current level by setting 
    cmd to PGSd_SMF_DisableLevel and re-enabled by setting cmd to 
    PGSd_SMF_EnableLevel. 

    A call is also provided, PGSd_SMF_SetIndentString, that allows the string 
    which is appended to the indent string to be changed from the default.

    This tool is provided to allow SMF to do indented message logging and
    to allow message logging to be selectively disabled for a specified tool
    group.

INPUTS:   
    Name        Description       
                        
    cmd    	Command specifying action to be taken.  Current values:

                    PGSd_SMF_IncLevel
                    PGSd_SMF_DecLevel
                    PGSd_SMF_GetLevel
                    PGSd_SMF_SetIndentString
                    PGSd_SMF_EnableLevel
                    PGSd_SMF_DisableLevel

    indent_str	String to be appended to the indent string with
        	each successive call. (cmd == PGSd_SMF_IncLevel)

    func_name	The name of the tool which is incrementing or decrementing 
                the message level. (cmd == PGSd_SMF_IncLevel or 
		PGSd_SMF_DecLevel)


OUTPUTS:
    Name        Description
                             
    msg_level	The current message level is returned here if msg_level
        	is not set to the null pointer. (cmd == PGSd_SMF_GetLevel)

    log_off	A flag will be returned here, set to 1 (logging disabled), 
        	or 0 (logging enabled), for the current message level, 
        	assuming log_off is not set to the null pointer, (cmd == 
        	PGSd_SMF_GetLevel)

    indent_str	The current indent string is returned here if indent_str
        	is not set to the null pointer. (cmd == PGSd_SMF_GetLevel)

    func_name	The name of the tool which set the current message label
        	is returned here if func_name is not set to the null pointer. 
        	(cmd == PGSd_SMF_GetLevel)

RETURNS:       
    PGS_S_SUCCESS

EXAMPLES:        


NOTES:           
    This is a low-level tool. 

    Parameters indent_str and func_name are defined as type char **,
    because when used as output variables, they are set to point to 
    static internal strings maintained by this tool.

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    Static variables and arrays are used to keep track of the current
    message level, indent string, and function name.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_MsgLevel(
    PGSt_integer    cmd,
    PGSt_integer    *msg_level,
    PGSt_integer    *log_off,
    char            **indent_str,
    char            **func_name)
{
    PGSt_SMF_status      returnStatus = PGS_S_SUCCESS;

    /* static char          *toolname = "PGS_SMF_MsgLevel"; */ /* not used */

    static PGSt_integer  init_flag = 1;

    static PGSt_integer  disable_logging[PGSd_SMF_MaxMsgLevel];
    static char          func_array[PGSd_SMF_MaxMsgLevel][PGS_SMF_MAX_FUNC_SIZE];
    static char          trace_path[PGSd_SMF_MaxMsgLevel*PGS_SMF_MAX_FUNC_SIZE];

    static PGSt_integer  current_level;
    static char          current_indent[500];

    static char          space[100];
    static PGSt_integer  size = 0;

    int                  i;
    int                  j;
    

    /* 
     * Initialize
     */

    if (init_flag != 0)
    {
        strcpy(space, "  ");      /* set the default space string */
        size = strlen(space);     /* get length */

        current_indent[0] = 0;    /* set current indent to the null string */
        current_level = 0;        /* set starting message level to zero */
	func_array[0][0] = '\0';
	
	trace_path[0] = '\0';     /* set current trace path to the null
				     string */

        for (i=0; i<PGSd_SMF_MaxMsgLevel; i++) /* init arrays */
        {
            disable_logging[i] = 0; /* default: logging on */
            func_array[i][0] = 0;     /* set function name to null string */
        }

        init_flag = 0;
    }

    /* 
     * Handle command
     */

    switch (cmd) 
    {

      case PGSd_SMF_IncLevel:    /* increment message level */

        current_level++;

        if (current_level < PGSd_SMF_MaxMsgLevel)
        {
            disable_logging[current_level] = 0; /* default: logging is on */
            strncpy(func_array[current_level], *func_name, 
		    PGS_SMF_MAX_FUNC_SIZE-1);
        }

	strcat(current_indent, space);

        break;


      case PGSd_SMF_DecLevel:    /* decrement message level */

        if (current_level > 0)
        {
            if (current_level < PGSd_SMF_MaxMsgLevel)
            {
                func_array[current_level][0]= 0;
            }

            current_level--;
            current_indent[strlen(current_indent) - size] = 0;
	    
        }

        break;


      case PGSd_SMF_GetTracePath:    /* get trace path */
	trace_path[0] = '\0';
	for (i=1;i<=current_level-1;i++)
	{
	    for (j=1;j<i;j++)
	    {
		strcat(trace_path, space);
	    }
	    strcat(trace_path, func_array[i]);
	    strcat(trace_path, ":\n");
	}
	
	
	*indent_str = trace_path;
        break;


      case PGSd_SMF_GetLevel:    /* get current message level */

        if (msg_level != 0)
        {
            *msg_level = current_level;
        }

        if (log_off != 0)
        {
            if (current_level < PGSd_SMF_MaxMsgLevel)
            {
                *log_off = disable_logging[current_level];
            }
        }

        if (indent_str != 0)
        {
            *indent_str = current_indent;
        }

        if (func_name != 0)
        {
            *func_name = func_array[current_level];
        }

        break;


      case PGSd_SMF_SetIndentString:  /* set the indent space string */

        if (indent_str != 0)
        {
            size = strlen(*indent_str);
            strncpy(space, *indent_str, 100);
        }

        break;


      case PGSd_SMF_EnableLevel:    /* enable logging for this level */

        if (current_level < PGSd_SMF_MaxMsgLevel)
        {
            disable_logging[current_level] = 0; /*  logging is on */
        }

        break;


      case PGSd_SMF_DisableLevel:    /* disable logging for this level */

        if (current_level < PGSd_SMF_MaxMsgLevel)
        {
            disable_logging[current_level] = 1; /*  logging is off */
        }

        break;


      default:

        break;

    }


    return returnStatus;
} 


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SetDynamicMsg

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_SetDynamicMsg(
        PGSt_SMF_code  code,
        char          *msg,               
        char          *funcname);     

FORTRAN:
    include 'PGS_SMF.f'    

    integer function pgs_smf_setdynamicmsg(code,msg,funcname)
    integer       code
    character*240 msg
    character*32  funcname

DESCRIPTION:
    This tool will provide the means to set a user-defined error/status message
    in response to the outcome of some segment of processing. Also user can
    attach a message string to the defined mnemonic code; thus overriding the
    defined message string that was created by smfcompile.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler
    msg         message string to be saved into the 
                static buffer
    funcname    routine which noted this error

OUTPUTS:     
    Name        Description                               Units        Min        Max          

RETURNS:        
    PGS_S_SUCCESS   
    PGS_E_ENV
    PGS_E_UNIX
    PGSSMF_E_LOGFILE
    PGSSMF_E_UNDEFINED_CODE

EXAMPLES:
    C:
    ==
    Lets say we have defined a mnemonic code in the SMF file:

    MODIS_E_BAD_CALIBRATION   Calibration value %7.2f is not within tolerance

    Also we would like to insert the calibration factor into the message
    template defined in the SMF file above. The resultant message (buf) would
    appear as follows after calling PGS_SMF_GetMsg(): 
    "Calibration value 356.23 is not within tolerance"

    PGSt_SMF_status returnStatus;
    PGSt_SMF_code   code;
    char            mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];
    char            msg[PGS_SMF_MAX_MSGBUF_SIZE];
    char            buf[PGS_SMF_MAX_MSGBUF_SIZE];
    float           calibration_factor = 356.23;

    returnStatus = PGS_SMF_GetMsgByCode(MODIS_E_BAD_CALIBRATION,msg);
    sprintf(buf,msg,calibration_factor);

    PGS_SMF_SetDynamicMsg(MODIS_E_BAD_CALIBRATION,buf,"funcNotedThisError()");
    PGS_SMF_GetMsg(&code,mnemonic,buf);        

    FORTRAN:
    ========
    implicit none
    integer pgs_smf_setdynamicmsg

    pgs_smf_setdynamicmsg(MODIS_E_BAD_CALIBRATION,'Calibration error occured',
                          'funcNotedThisError()');   

NOTES:          
    Note that you can have the flexibility of associating any dynamic message
    string to the defined mnemonic code via this routine.

    This tool can be used in various situations. For instance the user might
    want to concatenate some message strings together and assigned this
    concatenated string to an existing mnemonic code so that this message can be
    passed forward to another module for further processing. Alternatively it
    can be used to embed runtime variables in the defined message template
    before saving this message string to the static message buffer.

    The parameter "funcname" can be passed in as NULL if you do not wish to 
    record the routine noted this error. However it is strongly recommended
    to pass the routine name for tracking purposes. 

    The parameter "msg" can be passed in as NULL. If you do, no message is
    associated with the mnemonic code.

    Refer to utility "smfcompile"
    for additional information on the format of the message compiler.

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    You cannot override PGS_S_SUCCESS message: eg.

    PGS_SMF_SetDynamicMsg(PGS_S_SUCCESS,"PGS_S_SUCCESS",
                          "Success process, thus no problem");
    PGS_SMF_GetMsg(&code,mnemonic,msg);    

    The 'msg' will not contain the override message; it will return hardcoded
    message.  The reason is due to enhancement to speed up Geolocation
    processsing (Marek).  The rest of the code can be overriden.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()   
    PGS_SMF_WriteLogFile()
    PGS_SMF_HandleLogErr()
    PGS_SMF_GetSMFCode()
    PGS_SMF_DecodeCode()
    PGS_SMF_CorrectFuncName()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_SetDynamicMsg(          /* Set dynamic message */
    PGSt_SMF_code  code,        /* Input code */
    char          *msg,         /* Input message string to be saved into the
				   static buffer */
    char          *funcname)    /* Input function which noted this error */
{
    PGSt_SMF_status   returnStatus = PGS_S_SUCCESS;  /* return status */
    PGSt_SMF_status   writeStatus  = PGS_S_SUCCESS;  /* write log file status */
    PGSSmfGlbVar     *global_var;                    /* global static buffer */
    PGSt_SMF_boolean  writeLog = PGS_TRUE;           /* flag to write to log
							file */

    PGS_SMF_GetGlobalVar(&global_var);

    if (code == PGS_S_SUCCESS)
    {      
        /*
         * Update funcname, msg and code in the staticbuf.
         */    
        PGS_SMF_CorrectFuncName(funcname,global_var->msginfo.funcname);   
        global_var->msginfo.msgdata.code = code; 
        strcpy(global_var->msginfo.msgdata.msg,msg);
        strcpy(global_var->msg,global_var->msginfo.msgdata.msg);  

        writeLog = PGS_FALSE;       
    }
    else if (global_var->msginfo.msgdata.code == code)
    {
        /*
         * Update funcname and msg as the code already exist in the staticbuf.
         */ 
        PGS_SMF_CorrectFuncName(funcname,global_var->msginfo.funcname);
    }
    else
    {      
        /*
         * New code to save into static buffer.
         */              
        returnStatus = PGS_SMF_DecodeCode(code,"",&global_var->msginfo,
					  funcname,0);             
    }   

    /*
     * Write to log file.
     */
    if (writeLog == PGS_TRUE)
    {   
        if (returnStatus == PGS_S_SUCCESS)
        {
            strcpy(global_var->msginfo.msgdata.msg,msg);
            strcpy(global_var->msg,global_var->msginfo.msgdata.msg);       
        }

        writeStatus = PGS_SMF_WriteLogFile(global_var); 
        if ((writeStatus == PGSSMF_E_LOGFILE) ||
	    (writeStatus == PGSSMF_E_MSSLOGFILE) ||
            (writeStatus == PGSTSF_E_MUTEXLOCK_FAIL))
        {      
           /*
            * This statement was introduced to effectively overwrite
            * the current status condition with a more imperative
            * one; this really only pertains to those select processes
            * which require knowledge of this return value. 
            * NOTE: the previous status condition has already been logged!
            */
	    global_var->msginfo.msgdata.code = writeStatus;

            PGS_SMF_HandleLogErr(global_var);            
            returnStatus = writeStatus;                                  
        }
    } 

    return(returnStatus);

} /* end PGS_SMF_SetDynamicMsg */



/*****************************************************************
BEGIN_PROLOG:

TITLE:
    Error Status Handling

NAME:
    PGS_SMF_SetShmMemData

SYNOPSIS:
C:
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_SetShmMemData(
        PGSSmfShm *shmMem);

FORTRAN:
    NONE

DESCRIPTION:
    Set the values of the shared memory data structure.

INPUTS:
    None

OUTPUTS:
    Name        Description

    shmMem      shared memory structure

RETURNS:
    PGS_S_SUCCESS
    PGS_SH_SMF_LOGFILE

EXAMPLES:
    PGSSmfShm shmMem;

    PGS_SMF_SetShmMemData(&shmMem);

NOTES:
    This tool is only intended to be called by PGS_SMF_GetSysShm
    during an initialization process when running a PGE, or once
    at the beginning of a standalone run.

    This routine gets the message tags and the names of the log files.

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:
    This routine uses calls to the PC and IO tools to determine and verify
    the information to be stored in the shared memory structure.  It does
    NOT read or write the shared memory area directly.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_PC_GetPCSData()
    PGS_IO_Gen_Open()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SetShmMemData(    /* Set shared memory data */
    PGSSmfShm *shmMem)    /* Output shared memory strcuture */
{
    char                      msgTag1[PGSd_SMF_TAG_LENGTH_MAX];               /* message tag */
    char                      msgTag2[PGSd_SMF_TAG_LENGTH_MAX];               /* message tag */
    PGSt_integer              version;                                        /* version number */
    PGSt_integer              numFiles;                                       /* number of files return */
    PGSt_IO_Gen_FileHandle   *fptrStatus = (PGSt_IO_Gen_FileHandle *)NULL;    /* status log file */
    PGSt_IO_Gen_FileHandle   *fptrReport = (PGSt_IO_Gen_FileHandle *)NULL;    /* report log file */
    PGSt_IO_Gen_FileHandle   *fptrUser   = (PGSt_IO_Gen_FileHandle *)NULL;    /* user log file */
    PGSt_SMF_status           returnStatus;                                   /* return status */


    shmMem->writeLogFile = writeLogFile;
    shmMem->msgTag[0]    = '\0';
    shmMem->logStatus[0] = '\0';
    shmMem->logReport[0] = '\0';
    shmMem->logUser[0]   = '\0';

    /*
     * Get message tag.
     */
    returnStatus = PGS_PC_GetPCSData(PGSd_PC_SOFTWARE_ID, 0, msgTag1,
				     &numFiles);
    if (returnStatus == PGS_S_SUCCESS)
    {
        returnStatus = PGS_PC_GetPCSData(PGSd_PC_PRODUCTION_RUN_ID, 0, msgTag2,
					 &numFiles);
        if ( returnStatus == PGS_S_SUCCESS)
        {
            /*
             * Note that if the length of the message tag is greater than the
             * maximum size, then nothing will be copied to the shared memory.
             */
            PGS_SMF_RemoveSpace(msgTag1);
            PGS_SMF_RemoveSpace(msgTag2);

            if ((int) (strlen(msgTag1) + strlen(msgTag2)) <
		PGSd_SMF_TAG_LENGTH_MAX)
            {
                sprintf(shmMem->msgTag,"%s%s",msgTag1,msgTag2);
            }
        }
    }

    /*
     * Get log files.
     */               
    if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_LOGSTATUS,PGSd_IO_Gen_Append,
			&fptrStatus,1) 
        == PGS_S_SUCCESS)
    {            
        version = 1;
        PGS_PC_GetReference(PGSd_SMF_LOGICAL_LOGSTATUS,&version,
			    shmMem->logStatus);
        fclose((FILE *)fptrStatus);
    }

    else
    {
        fprintf(stderr, "Can not open LogStatus file; there may be problem with PCF file regarding the directory for LogStatus.\n");
    }

    if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_LOGREPORT,PGSd_IO_Gen_Append,
			&fptrReport,1) 
        == PGS_S_SUCCESS)
    {            
        version = 1;
        PGS_PC_GetReference(PGSd_SMF_LOGICAL_LOGREPORT,&version,
			    shmMem->logReport);
        fclose((FILE *)fptrReport);
    }        

    else
    {
        fprintf(stderr, "Can not open LogReport file; there may be problem with PCF file regarding the directory for LogReport.\n");
    }

    if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_LOGUSER,PGSd_IO_Gen_Append,
			&fptrUser,1) 
        == PGS_S_SUCCESS)
    {      
        version = 1;
        PGS_PC_GetReference(PGSd_SMF_LOGICAL_LOGUSER,&version,shmMem->logUser);
        fclose((FILE *)fptrUser);
    }            

    else
    {
      fprintf(stderr, "Can not open LogUser file; there may be problem with PCF file regarding the directory for LogUser.\n");
    }

    if ((fptrStatus == (PGSt_IO_Gen_FileHandle *)NULL) || 
        (fptrReport == (PGSt_IO_Gen_FileHandle *)NULL) ||
        (fptrUser   == (PGSt_IO_Gen_FileHandle *)NULL))
    {
        /*
         * If any are nil, then one or more 
	 * files could not be opened.
         */
        returnStatus = PGS_SH_SMF_LOGFILE;
    }
    else
    {
        returnStatus = PGS_S_SUCCESS;
    }

    return(returnStatus);

} /* end PGS_SMF_SetShmMemData */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SetShmMemToGlbVar

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_SetShmMemToGlbVar(
        PGSSmfShm    *shmMem,
        PGSSmfGlbVar *global_var);

FORTRAN:
    NONE

DESCRIPTION:
    Set (populate) shared memory data into static global variable.

INPUTS:   
    Name        Description                               Units        Min        Max              

    shmMem      shared memory structure    

OUTPUTS:
    Name        Description                               Units        Min        Max 

    global_var  global static buffer

RETURNS:       
    NONE

EXAMPLES:        
    PGSSmfShm     shmMem;
    PGSSmfGlbVar *global_var;

    PGS_SMF_GetGlobalVar(&global_var);  
    PGS_SMF_SetShmMemData(&shmMem);
    PGS_SMF_SetShmMemToGlbVar(&shmMem,global_var);
    PGS_SMF_WriteBanner(&shmMem);

NOTES:           
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    Notice that this routine did not call any IO or PC tools. This is 
    to avoid unnecessary execution steps, and also to further avoid 
    cyclic problems.

    Actually the cyclic problems were resolved in TK3. Here we simply
    to avoid creating new ones.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void
PGS_SMF_SetShmMemToGlbVar(     /* Set shared memory data to global variable */
    PGSSmfShm    *shmMem,      /* Input  shared memory strcuture */
    PGSSmfGlbVar *global_var)  /* Output global static buffer */
{

    writeLogFile = shmMem->writeLogFile;    
    strcpy(global_var->log.msgTag, shmMem->msgTag);
    strcpy(global_var->log.logStatus, shmMem->logStatus);
    strcpy(global_var->log.logReport, shmMem->logReport);
    strcpy(global_var->log.logUser, shmMem->logUser); 

#ifdef _PGS_THREADSAFE
    /*
     * We do not want to keep these files open in thread safe mode, that
     * way each thread can open, write, and close (while locked) and not
     * interfere with one another.
     */
    if (shmMem->logStatus[0] == '\0')
    {
        global_var->log.fptrStatus = (FILE *)NULL; 
    }

    if (shmMem->logReport[0] == '\0')
    {
        global_var->log.fptrReport = (FILE *)NULL;
    }

    if (shmMem->logUser[0] == '\0')
    {            
        global_var->log.fptrUser = (FILE *)NULL;
    }
#else
    if (shmMem->logStatus[0] == '\0')
    {
        global_var->log.fptrStatus = (FILE *)NULL; 
    }
    else
    {
        global_var->log.fptrStatus = fopen(shmMem->logStatus, "a");
    }

    if (shmMem->logReport[0] == '\0')
    {
        global_var->log.fptrReport = (FILE *)NULL;
    }
    else
    {
        global_var->log.fptrReport = fopen(shmMem->logReport, "a");
    }

    if (shmMem->logUser[0] == '\0')
    {            
        global_var->log.fptrUser = (FILE *)NULL;
    }
    else
    {
        global_var->log.fptrUser = fopen(shmMem->logUser, "a");  
    }              
#endif


} /* end PGS_SMF_SetShmMemToGlbVar */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SetStaticMsg

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_SetStaticMsg(
        PGSt_SMF_code  code,
        char          *funcname);   

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_setstaticmsg(code,funcname)
    integer      code
    character*32 funcname

DESCRIPTION:
    This tool will provide the means to set a user-defined
    error/status message in response to the outcome of some
    segment of processing.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler (see "smfcompile")
    funcname    routine which noted this error

OUTPUTS:      
    Name        Description                               Units        Min        Max          

RETURNS:        
    PGS_S_SUCCESS    
    PGS_E_ENV
    PGS_E_UNIX
    PGSSMF_E_LOGFILE
    PGSSMF_E_UNDEFINED_CODE

EXAMPLES:
    C:
    ==
    PGSt_SMF_status returnStatus;    
    returnStatus = PGS_SMF_SetStaticMsg(PGSSMF_E_UNDEFINED_UNIXERROR,
                                        "funcNotedThisError()");


    FORTRAN:
    ========
    implicit none
    integer pgs_smf_setstaticmsg

    integer returnStatus
    returnStatus = pgs_smf_setstaticmsg(PGSSMF_E_UNDEFINED_UNIXERROR,
   >                                    'funcNotedThisError()')


NOTES:          
    The parameter "funcname" can be passed in as NULL if you do not wish to
    record that routine noted this error. However it is strongly recommended to
    pass the routine name for tracking purposes. Refer to utility "smfcompile"
    for additional information on the format of the message compiler.

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    Even if the code is in static buffer, this routine still call
    PGS_SMF_DecodeCode() routine to retrieve the message because system do not
    know if the message in the static buffer has been changed, eg. call
    PGS_SMF_SetDynamicMsg().

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()
    PGS_SMF_DecodeCode()                
    PGS_SMF_WriteLogFile()
    PGS_SMF_HandleLogErr()
    PGS_SMF_GetSMFCode()
    PGS_SMF_CorrectFuncName()    

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_SetStaticMsg(            /* Set static message */
    PGSt_SMF_code  code,         /* Input code */
    char          *funcname)     /* Input function which noted this error */
{
    PGSt_SMF_status   returnStatus = PGS_S_SUCCESS;  /* return status */
    PGSt_SMF_status   writeStatus = PGS_S_SUCCESS;   /* write log file status */
#ifdef _PGS_THREADSAFE
    PGSSmfGlbVar *global_var;                        /* global static buffer */

    /*
     * Let's use the global variable for this thread instead of a static
     */
    PGS_SMF_GetGlobalVar(&global_var);
#else
    static PGSSmfGlbVar *global_var=NULL;            /* global static buffer */

    if (global_var == NULL)
    {
	PGS_SMF_GetGlobalVar(&global_var);
    }
#endif

    /*
     * Due to optimization, we only record the PGS_S_SUCCESS to the static
     * buffer so as to speed up processing.
     */
    if (code == PGS_S_SUCCESS)
    {      
        /*
         * Just update the code in the static buffer.
         */  

        global_var->msginfo.msgdata.code = code;
    }
    else
    {                 
        returnStatus = PGS_SMF_DecodeCode(code, "", &global_var->msginfo,
					  funcname, 0);

        strcpy(global_var->msg,global_var->msginfo.msgdata.msg);    
	
        /*
         * Write to log files.
         */
        writeStatus = PGS_SMF_WriteLogFile(global_var); 
        if ((writeStatus == PGSSMF_E_LOGFILE) ||
            (writeStatus == PGSSMF_E_MSSLOGFILE) ||
            (writeStatus == PGSTSF_E_MUTEXLOCK_FAIL))
        {      
           /*
            * This statement was introduced to effectively overwrite
            * the current status condition with a more imperative
            * one; this really only pertains to those select processes
            * which require knowledge of this return value. 
            * NOTE: the previous status condition has already been logged! 
            */
	    global_var->msginfo.msgdata.code = writeStatus;

            PGS_SMF_HandleLogErr(global_var);            
            returnStatus = writeStatus;                                 
        }
    }              

    return(returnStatus);

} /* end PGS_SMF_SetStaticMsg */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SetUNIXMsg

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_SetUNIXMsg(
        PGSt_integer  unix_errcode, 
        char         *msg,
        char         *funcname);

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_setunixmsg(unix_errcode,msg,funcname)
    integer       unix_errcode
    character*240 msg
    character*32  funcname

DESCRIPTION:
    This tool provides the means to retain UNIX error messages for
    later retrieval. Additionally, the user has the flexibility to
    append a user defined message to the UNIX message for further clarity.

INPUTS:    
    Name            Description                                Units        Min        Max

    unix_errcode    the error code set by C library and 
                    system calls (the value stored in "errno")
    msg             user defined error message string
    funcname        routine which noted this error

OUTPUTS:      
    Name            Description                                Units        Min        Max

RETURNS:
    PGS_S_SUCCESS    
    PGSSMF_E_LOGFILE
    PGSSMF_E_UNDEFINED_UNIXERRNO  
    PGSSMF_E_MSG_TOOLONG   

EXAMPLES:
    C:
    ==
    This example uses the 'popen()' C library routine merely to illustrate how
    the SMF tool PGS_SMF_SetUNIXMsg() might be used to preserve the Unix
    error condition. Be advised that 'popen()' is not part of the POSIX
    standard and therefore not be used within the science software.

    PGSt_SMF_status Get_Listing()
    {
        FILE            *stream;
        char             buffer[101];
        char             directoryEntry[101];        
        PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;

        if (stream = popen("ls","r") != NULL)
        {
            while (fgets(buffer,100,stream) != NULL)
            {
                scanf(buffer,"%s",directoryEntry);
            }
        }
        else
        {           
            pclose(stream);
            PGS_SMF_SetUNIXMsg(errno,NULL,"Get_Listing()");
            returnStatus = PGS_E_UNIX;
        }
    }


    FORTRAN:    
    ========
    implicit none
    integer pgs_smf_setunixmsg

    character*1 chr
    integer     errNo

    PXFFGETC(IPXFCONST("STDIN_UNIT"),chr,errNo)
    if (errNo .NE. 0) then
        pgs_smf_Setunixmsg(errNo,'PXFFGETC() error occured','Get_Listing()')
    endif

NOTES:  
    The parameter "funcname" can be passed in as NULL if you do not wish to
    record the routine noted this error. However it is strongly recommended
    to pass the routine name for tracking purposes. Likewise, the parameter 
    "msg" can be NULL unless you wish to have an additional message appended 
    to the system defined Unix message. The static variable 'errno' has been
    declared in 'PGS_SMF.h'. Since Unix treats errno as a static parameter, 
    the user will have to save the value returned from the critical call unless
    the call to 'PGS_SMF_SetUNIXMsg()' is made immediately. If unix_errno is not
    a valid constant, the static buffer will be updated with appropriate error
    message.

    The following Unix errors that is being passed from Fortran
    functions are currently being investigated.

    ENONAME
    ENOHANDLE
    ETRUNC
    ERRAYLEN
    EEND

    Please refer to POSIX FORTRAN 77 IEEE Std 1003.9-1992 on page 14,
    Section 2.4 (Error Nummbers).            

REQUIREMENTS:
    PGSTK-0580,0590,0632,0650

DETAILS:
    When this toolkit returns PGS_S_SUCCESS, the static buffer is set to
    PGSSMF_M_UNIX.  The reason of not setting the static buffer to PGS_S_SUCCESS
    is to avoid PGS_SMF_GetMsg() of interpreting it as truly PGS_S_SUCCESS which
    in turn will return PGS_S_SUCCESS's mnemonic and message. Thus
    PGS_SMF_GetMsg() must make sure that when PGSSMF_M_UNIX is in static buffer,
    then return code=unixErrno (actual errno), mnemonic=PGS_E_UNIX,
    msg="whatever the message is in the static buffer".

    Refer to PGS_SMF_GetMsg() for further detail.    

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetGlobalVar()
    PGS_SMF_GetSystemCode()
    PGS_SMF_GetSMFCode()
    PGS_SMF_DecodeCode()
    PGS_SMF_WriteLogFile()
    PGS_SMF_HandleLogErr()
    PGS_SMF_CorrectFuncName()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SetUNIXMsg(               /* Set UNIX error */
    PGSt_integer  unix_errcode,   /* Input unix error number */
    char         *msg,            /* Input user defined error message string */
    char         *funcname)       /* Input routine which noted this error */
{
    PGSSmfGlbVar    *global_var;                     /* global static buffer */
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */
    PGSt_SMF_status  writeStatus  = PGS_S_SUCCESS;   /* write log file status */

    PGS_SMF_GetGlobalVar(&global_var);
    
    if ((msg != (char *)NULL) && ((short)strlen(msg) > 
				  (PGS_SMF_MAX_MSGBUF_SIZE - 1)))
    {
        /*
         * User message is too long.
         */
        PGS_SMF_DecodeCode(PGSSMF_E_MSG_TOOLONG,"",&global_var->msginfo,
			   funcname,0);

        sprintf(global_var->msg,global_var->msginfo.msgdata.msg,
		PGS_SMF_MAX_MSGBUF_SIZE-1);

        returnStatus = PGSSMF_E_MSG_TOOLONG;
    }
    
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
    else if(strerror(unix_errcode) == NULL || strstr(strerror(unix_errcode), "Unknown error") != NULL) 
#else
    else if ((unix_errcode <= 0) || (unix_errcode >= sys_nerr))
#endif
    {  	
         /*
         * Unix error number does not exists.
         */  
       PGS_SMF_DecodeCode(PGSSMF_E_UNDEFINED_UNIXERRNO,"",
       		   &global_var->msginfo,funcname,0);

       sprintf(global_var->msg,global_var->msginfo.msgdata.msg,unix_errcode);       
	
       returnStatus = PGSSMF_E_UNDEFINED_UNIXERRNO;
    }
    else
    {                  
        /*
         * Ok, unix error message retrieved.
         */  
        global_var->unixErrno = (PGSt_SMF_code)unix_errcode;
        memset((char *)&global_var->msginfo,'\0',sizeof(PGS_SMF_MsgInfo));
        global_var->msginfo.fileinfo.seed = PGS_SMF_SYS_SEED;
        strcpy(global_var->msginfo.fileinfo.instr,PGS_SMF_SYS_INSTR);
        strcpy(global_var->msginfo.fileinfo.label,PGS_SMF_SYS_LABEL);
        global_var->msginfo.msgdata.action[0] = '\0';            
        global_var->msginfo.msgdata.code = PGSSMF_M_UNIX;
        PGS_SMF_GetSystemCode(PGS_E_UNIX,
                              global_var->msginfo.msgdata.mnemonic,
                              global_var->msginfo.msgdata.msg);

        /*
         * Make sure that funcname is of format: funcname()
         */ 
        PGS_SMF_CorrectFuncName(funcname,global_var->msginfo.funcname); 
	
        if (msg == (char *)NULL)
        {
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
            sprintf(global_var->msg,"%s",strerror(unix_errcode));
#else
            sprintf(global_var->msg,"%s",sys_errlist[unix_errcode]);
#endif
        }
        else
        {
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
            sprintf(global_var->msg,"%s; %s",msg,strerror(unix_errcode));
#else
            sprintf(global_var->msg,"%s; %s",msg,sys_errlist[unix_errcode]);
#endif
        }      
	                 
    }            

    /*
     * Write it to log file.
     */   
    writeStatus = PGS_SMF_WriteLogFile(global_var); 
    if ((writeStatus == PGSSMF_E_LOGFILE) ||
        (writeStatus == PGSSMF_E_MSSLOGFILE) ||
        (writeStatus == PGSTSF_E_MUTEXLOCK_FAIL))
    {      
       /*
        * This statement was introduced to effectively overwrite
        * the current status condition with a more imperative
        * one; this really only pertains to those select processes
        * which require knowledge of this return value. 
        * NOTE: the previous status condition has already been logged!
        */
        global_var->msginfo.msgdata.code = writeStatus;

        PGS_SMF_HandleLogErr(global_var);        
        returnStatus = writeStatus;                              
    }

    return(returnStatus);

} /* end PGS_SMF_SetUNIXMsg */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TermProc

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_TermProc(
        PGSt_SMF_status useMem,
        PGSt_SMF_status writeLog);

FORTRAN:
    NONE

DESCRIPTION:
    Set status to indicate to SMF that the running process is a
    termination process. It has to be called in the termination
    process main() (the first executable statetememt).

INPUTS:   
    Name        Description                               Units        Min        Max              

    useMem      use shared memory or ascii file
    writeLog    write errors to log file         

OUTPUTS:
    Name        Description                               Units        Min        Max 

RETURNS:       
    NONE

EXAMPLES:        
    /#
     # This example here is meant to be followed in the term. process in main() routine
     #/
    PGSt_SMF_status  returnStatus;
    PGSt_SMF_status  useMemType;                  /# PGSd_SMF_USE_SHAREDMEM or PGSd_SMF_USE_ASCIIFILE #/
    PGSt_SMF_status  writeLogFile;                /# PGSd_SMF_WRITELOG_OFF  or PGSd_SMF_WRITELOG_ON #/

    PGS_SMF_TermProc(useMemType,writeLogFile);    /# first executable statement in main() #/
                                                  /# useMemType and writeLogFile are passed in by system #/
                                                  /# so Ray make sure you interprete it correctly and pass #/
                                                  /# in the correct constants #/

    ...
    ...
    ...  

    PGS_SMF_SysTermSendRuntimeData();           /# send runtime data to remote
                                                   host #/

    if (writeLogFile == PGSd_SMF_WRITELOG_ON)   /# send log files to remote host
                                                   only if system says so #/
    {
        PGS_SMF_SysTermSendStatusReport();         
    }

    ...
    ...
    ...

    PGS_MEM_ShmSysTerm();                       /# release shared memory
                                                   resources back to system #/

NOTES:           
    This routine is only to be used internally by ARC toolkit developers.    

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    The main reason why this function is needed is to let SMF
    know that the current process is in fact termination
    process. If current process is initialization process, then
    PGS_SMF_GetGlobalVar() will not call PGS_SMF_GetSysShm()
    to intialize system shared memory to SMF. Otherwise cyclic
    phenomena might occur.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void
PGS_SMF_TermProc(                 /* Termination process */
    PGSt_SMF_status useMem,       /* Input use shared memory or ascii file */
    PGSt_SMF_status writeLog)     /* Input write errors to log file */
{

    whichProc    = PGSd_SMF_PROC_TERM;
    callerID     = PGSd_CALLERID_SMF;
    useMemType   = useMem;
    writeLogFile = writeLog;


} /* end PGS_SMF_TermProc */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TermSMF

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_TermSMF(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    Terminate SMF and close all opened files.

INPUTS:   
    Name         Description                               Units        Min        Max                    

OUTPUTS:
    Name         Description                               Units        Min        Max         

RETURNS:       
    NONE

EXAMPLES:        

NOTES:           
    This routine should be called before exit() in the main() routine.

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
void
PGS_SMF_TermSMF(    /* Terminate SMF */
    void)
{
    PGSSmfGlbVar *global_var;    /* global static buffer */
    int           i;             /* index */

    PGS_SMF_GetGlobalVar(&global_var);

    for (i=0 ; i < PGSd_SMF_NUM_OPENFILE ; i++)
    {
        if (global_var->open.filePtr[i] != (FILE *)NULL) 
        {
            fclose(global_var->open.filePtr[i]);
        }
    }

    if (global_var->log.fptrStatus != (FILE *)NULL)
    {
	fclose(global_var->log.fptrStatus);    
    }

    if (global_var->log.fptrReport != (FILE *)NULL)
    {
	fclose(global_var->log.fptrReport);
    }

    if (global_var->log.fptrUser   != (FILE *)NULL)
    {
	fclose(global_var->log.fptrUser);        
    }


} /* end PGS_SMF_TermSMF */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TestErrorLevel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>                                                      

    PGSt_SMF_boolean 
    PGS_SMF_TestErrorLevel(
        PGSt_SMF_status code);

FORTRAN:
    include 'PGS_SMF.f'    

    integer function pgs_smf_testerrorlevel(code)
    integer code

DESCRIPTION:
    Given the mnemonic status code, this tool will return a Boolean value
    indicating whether or not the returned code has level 'E'.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:     
    Name        Description                               Units        Min        Max

RETURNS:
    PGS_FALSE
    PGS_TRUE

EXAMPLES:
    C:
    ==
    PGSt_SMF_status  returnStatus;
    int             *intPtr;

    returnStatus = PGS_MEM_Malloc(&intPtr,sizeof(int)*10);
    if (PGS_SMF_TestErrorLevel(returnStatus) == PGS_TRUE)
    {
        /# Branch to handle error condition #/
    }
    else
    {
        /# Some other status level returned #/
    }


    FORTRAN:
    ========    
    implicit none
    integer pgs_smf_testerrorlevel

    integer       code
    integer       flag
    character*480 msg       
    character*32  mnemonic

    pgs_smf_getmsg(code,msg,mnemonic)
    flag = pgs_smf_testerrorlevel(code)
    if (flag .EQ. PGS_TRUE) then
C       Branch to handle error condition
    else
C       Some other status level returned
    endif   

NOTES:          
    NONE

REQUIREMENTS:
    PGSTK-0580

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_boolean 
PGS_SMF_TestErrorLevel(       /* Test Error level */
    PGSt_SMF_status code)     /* Input code */
{
    PGSt_SMF_boolean returnBoolean = PGS_FALSE; /* return status */


    if ((code & PGS_SMF_MASK_LEVEL) == PGS_SMF_MASK_LEV_E)
    {
        returnBoolean = PGS_TRUE;    
    }  

    return(returnBoolean);

} /* end PGS_SMF_TestErrorLevel */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TestFatalLevel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>                                                     

    PGSt_SMF_boolean 
    PGS_SMF_TestFatalLevel(
        PGSt_SMF_status code);

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_testfatallevel(code)
    integer code

DESCRIPTION:
    Given the mnemonic status code, this tool will return a Boolean value
    indicating whether or not the returned code has level 'F'.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:     
    Name        Description                               Units        Min        Max          

RETURNS:        
    PGS_FALSE
    PGS_TRUE

EXAMPLES:
    See example for PGS_SMF_TestErrorLevel().

NOTES:          
    NONE

REQUIREMENTS:
    PGSTK-0580

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_boolean 
PGS_SMF_TestFatalLevel(       /* Test Fatal level */
    PGSt_SMF_status code)     /* Input code */
{
    PGSt_SMF_boolean returnBoolean = PGS_FALSE; /* return status */


    if ((code & PGS_SMF_MASK_LEVEL) == PGS_SMF_MASK_LEV_F) 
    {
        returnBoolean = PGS_TRUE;    
    }

    return(returnBoolean);

} /* end PGS_SMF_TestFatalLevel */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TestMessageLevel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>                                                     

    PGSt_SMF_boolean 
    PGS_SMF_TestMessageLevel(
        PGSt_SMF_status code);

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_testMessagelevel(code)
    integer code

DESCRIPTION:
    Given the mnemonic status code, this tool will return a Boolean value
    indicating whether or not the returned code has level 'M'.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:     
    Name        Description                               Units        Min        Max          

RETURNS:        
    PGS_FALSE
    PGS_TRUE

EXAMPLES:
    See example for PGS_SMF_TestErrorLevel().

NOTES:          
    NONE

REQUIREMENTS:
    PGSTK-0580

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_boolean 
PGS_SMF_TestMessageLevel(     /* Test Fatal level */
    PGSt_SMF_status code)     /* Input code */
{
    PGSt_SMF_boolean returnBoolean = PGS_FALSE; /* return status */


    if ((code & PGS_SMF_MASK_LEVEL) == PGS_SMF_MASK_LEV_M) 
    {
        returnBoolean = PGS_TRUE;    
    }

    return(returnBoolean);

} /* end PGS_SMF_TestMessageLevel */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TestNoticeLevel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>                                                      

    PGSt_SMF_boolean 
    PGS_SMF_TestNoticeLevel(
        PGSt_SMF_status code);

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_testnoticelevel(code)
    integer code

DESCRIPTION:
    Given the mnemonic status code, this tool will return a Boolean value
    indicating whether or not the returned code has level 'N'.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:      
    Name        Description                               Units        Min        Max          

RETURNS:     
    PGS_FALSE
    PGS_TRUE

EXAMPLES:
    See example for PGS_SMF_TestErrorLevel().

NOTES:          
    NONE

REQUIREMENTS:
    PGSTK-0580

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_boolean 
PGS_SMF_TestNoticeLevel(       /* Test Notice level */
    PGSt_SMF_status code)      /* Input code */
{
    PGSt_SMF_boolean returnBoolean = PGS_FALSE; /* return status */


    if ((code & PGS_SMF_MASK_LEVEL) == PGS_SMF_MASK_LEV_N) 
    {
        returnBoolean = PGS_TRUE;    
    }

    return(returnBoolean);

} /* end PGS_SMF_TestNoticeLevel */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TestStatusLevel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status 
    PGS_SMF_TestStatusLevel(
        PGSt_SMF_status code);

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_teststatuslevel(code)
    integer code

DESCRIPTION:
    Given the mnemonic status code, this tool will return a defined status level
    constant.

INPUTS:
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:      
    Name        Description                               Units        Min        Max          

RETURNS:        			
    PGS_SMF_MASK_LEV_M
    PGS_SMF_MASK_LEV_U
    PGS_SMF_MASK_LEV_S
    PGS_SMF_MASK_LEV_N
    PGS_SMF_MASK_LEV_W
    PGS_SMF_MASK_LEV_F
    PGS_SMF_MASK_LEV_E
    PGSSMF_E_UNDEFINED_CODE

EXAMPLES:
    C:
    ==
    PGSt_SMF_status  returnStatus;
    int             *intPtr;

    returnStatus = PGS_MEM_Malloc(&intPtr,sizeof(int)*10);
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_M:
             /# This is a message level status #/
             break;

        case PGS_SMF_MASK_LEV_U:
             /# This is a user information level status #/
             break;

        case PGS_SMF_MASK_LEV_S:
             /# This is a success level status #/
             break;

        case PGS_SMF_MASK_LEV_N:
             /# This is a notice level status #/
             break;            

        case PGS_SMF_MASK_LEV_W:
             /# This is a warning level status #/
             break;   

        case PGS_SMF_MASK_LEV_F:
             /# This is a fatal level status #/
             break;               

        case PGS_SMF_MASK_LEV_E:
             /# This is a error level status #/
             break;  

        default:           
             /# Undefined status level #/
             break;   
    } 


    FORTRAN:
    ======== 
    implicit none
    integer pgs_smf_teststatuslevel

    integer       returnstatus    
    integer       code    

    returnstatus = pgs_<some-function>(<values ...>)
    code = pgs_smf_teststatuslevel(returnstatus)

    if (code .EQ. PGS_SMF_MASK_LEV_M) then
C       This is a message level status
    else if (code .EQ. PGS_SMF_MASK_LEV_U) then
C       This is a user information level status
    else if (code .EQ. PGS_SMF_MASK_LEV_S) then
C       This is a success level status     
    else if (code .EQ. PGS_SMF_MASK_LEV_N) then
C       This is a notice level status    
    else if (code .EQ. PGS_SMF_MASK_LEV_W) then
C       This is a warning level status       
    else if (code .EQ. PGS_SMF_MASK_LEV_F) then
C       This is a fatal level status        
    else if (code .EQ. PGS_SMF_MASK_LEV_E) then
C       This is a error level status       
    else
C       Undefined status level        
    endif

NOTES:          
    Action levels are not considered since they are not meant as 
    function return status'.

REQUIREMENTS:
    PGSTK-0580

DETAILS:	   
    Note that PGS_S_SUCCESS is special as it is defined as 0.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_SetStaticMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_SMF_TestStatusLevel(       /* Test Status level */
    PGSt_SMF_status code)      /* Input code */
{
    PGSt_SMF_status returnStatus;  /* return status */


    if (code == PGS_S_SUCCESS)
    {
        returnStatus = PGS_SMF_MASK_LEV_S;
    }
    else
    {        
        /*
         * Check to make sure that the code is within valid level.
         */
        returnStatus = code & PGS_SMF_MASK_LEVEL;

        if ((returnStatus == PGS_SMF_MASK_LEV_SH) ||
            ((returnStatus >= PGS_SMF_MASK_LEV_S) && 
	     (returnStatus <= PGS_SMF_MASK_LEV_F)))
        {
            if (PGS_SMF_CallerID() == PGSd_CALLERID_USR)
            {
                PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_SMF_TestStatusLevel()");
            }
        }
        else
        {   
            if (PGS_SMF_CallerID() == PGSd_CALLERID_USR)
            {   
                PGS_SMF_SetStaticMsg(PGSSMF_E_UNDEFINED_CODE,
				     "PGS_SMF_TestStatusLevel()");
            }

            returnStatus = PGSSMF_E_UNDEFINED_CODE;
        }
    }

    return(returnStatus);

} /* end PGS_SMF_TestStatusLevel */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TestSuccessLevel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>                                                     

    PGSt_SMF_boolean 
    PGS_SMF_TestSuccessLevel(
        PGSt_SMF_status code);

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_testsuccesslevel(code)
    integer code

DESCRIPTION:
    Given the mnemonic status code, this tool will return a Boolean value
    indicating whether or not the returned code has level 'S'.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:      
    Name        Description                               Units        Min        Max                

RETURNS:        	
    PGS_FALSE
    PGS_TRUE

EXAMPLES:
    See example for PGS_SMF_TestErrorLevel().

NOTES:          
    NONE

REQUIREMENTS:
    PGSTK-0580

DETAILS:	   
    PGS_S_SUCCESS does not have _S_ bit encoded; thus it is
    treated as a special case (PGS_S_SUCCESS is actually defined
    as 0).

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_boolean 
PGS_SMF_TestSuccessLevel(       /* Test Success level */
    PGSt_SMF_status code)       /* Input code */
{
    PGSt_SMF_boolean returnBoolean = PGS_FALSE; /* return status */


    if (code == PGS_S_SUCCESS)
    {
        returnBoolean = PGS_TRUE;
    }
    else
    {
        if ((code & PGS_SMF_MASK_LEVEL) == PGS_SMF_MASK_LEV_S)
        { 
            returnBoolean = PGS_TRUE;    
        }
    }

    return(returnBoolean);

} /* end PGS_SMF_TestSuccessLevel */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TestUserInfoLevel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>                                                      

    PGSt_SMF_boolean 
    PGS_SMF_TestUserInfoLevel(
        PGSt_SMF_status code);

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_testuserinfolevel(code)
    integer code

DESCRIPTION:
    Given the mnemonic status code, this tool will return a Boolean value
    indicating whether or not the returned code has level 'U'.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:      
    Name        Description                               Units        Min        Max           

RETURNS:        
    PGS_FALSE
    PGS_TRUE

EXAMPLES:
    See example for PGS_SMF_TestErrorLevel().

NOTES:          
    NONE

REQUIREMENTS:
    PGSTK-0580

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_boolean 
PGS_SMF_TestUserInfoLevel(       /* Test UserInfo level */
    PGSt_SMF_status code)        /* Input code */
{
    PGSt_SMF_boolean returnBoolean = PGS_FALSE; /* return status */


    if ((code & PGS_SMF_MASK_LEVEL) == PGS_SMF_MASK_LEV_U) 
    {
        returnBoolean = PGS_TRUE;    
    }

    return(returnBoolean);

} /* end PGS_SMF_TestUserInfoLevel */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_TestWarningLevel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>                                                     

    PGSt_SMF_boolean 
    PGS_SMF_TestWarningLevel(
        PGSt_SMF_status code);

FORTRAN:
    include 'PGS_SMF.f'

    integer function pgs_smf_testwarninglevel(code)
    integer code

DESCRIPTION:
    Given the mnemonic status code, this tool will return a Boolean value
    indicating whether or not the returned code has level 'W'.

INPUTS:    
    Name        Description                               Units        Min        Max

    code        mnemonic error/status code generated 
                by message compiler

OUTPUTS:      
    Name        Description                               Units        Min        Max    

RETURNS:
    PGS_FALSE
    PGS_TRUE

EXAMPLES:
    See example for PGS_SMF_TestErrorLevel().

NOTES:          
    NONE

REQUIREMENTS:
    PGSTK-0580

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
PGSt_SMF_boolean 
PGS_SMF_TestWarningLevel(       /* Test Warning level */
    PGSt_SMF_status code)       /* Input code */
{
    PGSt_SMF_boolean returnBoolean = PGS_FALSE; /* return status */


    if ((code & PGS_SMF_MASK_LEVEL) == PGS_SMF_MASK_LEV_W) 
    {
        returnBoolean = PGS_TRUE;    
    }

    return(returnBoolean);

} /* end PGS_SMF_TestWarningLevel */



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_WriteBanner

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_WriteBanner(
        PGSSmfShm *shmMem);

FORTRAN:
    NONE

DESCRIPTION:
    Write banner to the log files.

INPUTS:   
    Name        Description                               Units        Min        Max              

    shmMem      shared memory structure    

OUTPUTS:
    Name        Description                               Units        Min        Max     

RETURNS:       
    NONE

EXAMPLES:        
    Refer to PGS_SMF_SetShmMemToGlbVar();

NOTES:           
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()
    PGS_SMF_TestErrorLevel()

END_PROLOG:
*****************************************************************/
void
PGS_SMF_WriteBanner(        /* Write banner to log files */
    PGSSmfShm *shmMem)      /* Input shared memory structure */
{
    FILE   *fptr;

    char   timeStr[100];
    char   banner1[2000];
    char   banner2[5000];
    char   next_line[100];
    char   version[21] = "Unknown";
    char   pcfVersion[PGSd_PC_VALUE_LENGTH_MAX] = "Missing";

    char   next_item[PGS_SMF_MAX_FUNC_SIZE]; /* variable to hold sub-strings
						parsed from input buffer "buf"
						(above) */

    char   buf[5000];      /* input buffer for line of text */
    char   *chrPtr;        /* used to mark specific character locations in a
			      character string */ 
    char   *pid_string;

    int    index;

    time_t calPtr;

    size_t check;          /* used to check the return value of
			      sscanf() */

    PGSt_integer numFiles; /* dummy variable, not used */

    PGS_SMF_MsgInfo     codeinfo;       /* structure containing SMF status codes
					   to be disabled (i.e. not logged) */
    
    PGSt_SMF_code       flag;           /* numeric flag used to indicate logging
					   status */

    PGSt_SMF_status loggingStatus;
    PGSt_SMF_status traceStatus;
    PGSt_SMF_status returnStatus;   /* return status of Toolkit function calls */
    PGSt_SMF_status code;           /* value of status code read from PCF */
    PGSt_SMF_status localCallerID;  /* caller ID, copy of SMF static variable
				       used by other Toolkit sub-systems
				       (e.g. IO, PC) to determined if they are
				       being called by and SMF routine (in which
				       case they behave differently in order to
				       avoid conflicts in the interaction
				       between SMF and these modules) */

#ifdef _PGS_THREADSAFE
    char temChar[100];
    char *lasts;
    static PGSt_SMF_boolean doneFlag = PGS_FALSE;

    /*
     * Ensure that banner is written only once!
     */
    if (doneFlag == PGS_TRUE)
    {
        return;
    }
    else
    {
        doneFlag = PGS_TRUE;
    }

    /*
     * Since ctime() is not threadsafe we will use ctime_r() while in
     * threadsafe mode
     */
    time(&calPtr);
    ctime_r(&calPtr,temChar);
    strcpy(timeStr,temChar);
#else
    time(&calPtr);
    strcpy(timeStr,ctime(&calPtr));
#endif
    PGS_SMF_RemoveSpace(timeStr);

    PGS_SMF_InitializeLogging();
    loggingStatus = PGS_SMF_LoggingControl(PGSd_QUERY_ALL, (PGSt_SMF_code) NULL);
    traceStatus = PGS_SMF_TraceControl(PGSd_QUERY);
    pid_string = PGS_SMF_LogPID(PGSd_QUERY);

    strcpy(banner1, "\n****************************************\n");
    sprintf(next_line, "BEGIN_PGE: %s\n", timeStr);
    strcat(banner1, next_line);
    sprintf(next_line, "MSG_TAG: %s\n", shmMem->msgTag);
    strcat(banner1, next_line);
    strcat(banner1, "FILE: ");
    strcpy(banner2, "\n");

    if (loggingStatus == PGSSMF_M_LOGGING_DISABLED ||
	PGS_SMF_Get_WriteLogFile() == PGSd_SMF_WRITELOG_OFF)
    {
	strcat(banner2, "LOGGING: status message logging disabled\n");
    }
    else
    {
	strcat(banner2, "LOGGING: status message logging enabled\n");
    }

    switch (traceStatus)
    {
      case PGSSMF_M_ERROR_TRACE_ENABLED:
	strcat(banner2, "TRACE_LEVEL: error tracing enabled\n");
	break;

     case PGSSMF_M_FULL_TRACE_ENABLED:
	strcat(banner2, "TRACE_LEVEL: full tracing enabled\n");
	break;

      case PGSSMF_M_TRACE_DISABLED:
      default:
	strcat(banner2, "TRACE_LEVEL: tracing disabled\n");
	break;
    }

    if (*pid_string != (char) NULL)
    {
	strcat(banner2, "PID_LOGGING: enabled\n");
    }
    else
    {
	strcat(banner2, "PID_LOGGING: disabled\n");
    }

    /* determine disabled SMF seeds, levels and codes; indicate these in the
       banner */

    /* (the following code is culled from PGS_SMF_InitializeLogging.c and
       modified--perhaps some of this stuff should be put into functions or some
       such) */

    /* save the current value of the SMF static variable "callerID" (defined
       elsewhere) in our local copy since we may be altering the SMF value and
       we want to restore it to its original value when we are done */

    localCallerID = PGS_SMF_CallerID();

    /* set the SMF static variable "callerID" (defined elsewhere) to indicated
       that calls to PC or IO routines are being done from within and SMF
       function */

    PGS_SMF_SetCallerID(PGSd_CALLERID_SMF);

    /* get the list of SMF status levels for which logging is to be disabled */

    strcat(banner2, "DISABLED_LEVELS:");
    next_line[0] = '\0';
    index = 0;
    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_LEVEL, buf);
    if (returnStatus == PGS_S_SUCCESS)
    {
#ifdef _PGS_THREADSAFE
        /*
         * strtok() is not threadsafe, use strtok_r() in threadsafe mode
         */
	chrPtr = strtok_r(buf, " ,\t",&lasts);
#else
	chrPtr = strtok(buf, " ,\t");
#endif
	while (chrPtr != NULL)
	{
	    switch (*chrPtr)
	    {
	      case 'S':
	      case 'A':
	      case 'M':
	      case 'U':
	      case 'N':
	      case 'W':
	      case 'E':
	      case 'F':
	      case 'C':
		next_line[index++] = ' ';
		next_line[index++] = *chrPtr;
		next_line[index] = '\0';
		break;
		
	      default:
		break;
	    }
#ifdef _PGS_THREADSAFE
            /*
             * strtok() is not threadsafe, use strtok_r() in threadsafe mode
             */
	    chrPtr = strtok_r(NULL, " ,\t",&lasts);
#else
	    chrPtr = strtok(NULL, " ,\t");
#endif
	}
    }
    if ( strlen(next_line) == (size_t) 0 )
    {
	strcpy(next_line, " none");
    }
    strcat(banner2, next_line);
    strcat(banner2, "\n");

    /* get the list of SMF seed numbers, the SMF status codes derived from which
       are to have logging disabled */

    strcat(banner2, "DISABLED_SEEDS:");
    next_line[0] = '\0';
    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_SEED, buf);
    if (returnStatus == PGS_S_SUCCESS)
    {
#ifdef _PGS_THREADSAFE
        /*
         * strtok() is not threadsafe, use strtok_r() in threadsafe mode
         */
	chrPtr = strtok_r(buf, " ,\t",&lasts);
#else
	chrPtr = strtok(buf, " ,\t");
#endif
	while (chrPtr != NULL)
	{
	    check = sscanf(chrPtr, "%d", &flag);
	    if (check == 1)
	    {
		chrPtr = next_line+strlen(next_line);
		sprintf(chrPtr, " %d", flag);
	    }
#ifdef _PGS_THREADSAFE
                /*
                 * strtok() is not threadsafe, use strtok_r() in threadsafe mode
                 */
		chrPtr = strtok_r(NULL, " ,\t",&lasts);
#else
		chrPtr = strtok(NULL, " ,\t");
#endif
	}
    }
    if ( strlen(next_line) == (size_t) 0 )
    {
	strcpy(next_line, " none");
    }
    strcat(banner2, next_line);
    strcat(banner2, "\n");
    
    /* get the list of SMF status codes for which logging is to be disabled, the
       codes are actually being read in from the PCF as their associated
       mnemonic strings and are here converted to the appropriate numerical
       values */

    /* READ ME: if you change the length of this string see the READ ME comments
       below */

    strcpy(next_line, "DISABLED_CODES:");
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

		sprintf(next_item, " %d", code);
		if ( strlen(next_item)+strlen(next_line) > 80 )
		{
		    strcat(banner2, next_line);
		    strcat(banner2, "\n");

		    /* READ ME: the following set of "blanks" should contain the
		       same number of characters as next_line is initialized to
		       above (see the above READ ME) */

		    sprintf(next_line, "               %s", next_item);
		}
		else
		{
		    strcat(next_line, next_item);
		}
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
		if ( strlen(next_item)+strlen(next_line) > 80 )
		{
		    strcat(banner2, next_line);
		    strcat(banner2, "\n");

		    /* READ ME: the following set of "blanks" should contain the
		       same number of characters as next_line is initialized to
		       above + 1 (see the above READ ME) */

		    sprintf(next_line, "                %s", next_item);
		}
		else
		{
		    strcat(next_line, " ");
		    strcat(next_line, next_item);
		}
	    }
	}
    }

    strcat(banner2, next_line);
    if ( strcmp(next_line, "DISABLED_CODES:") == 0 )
    {
	strcat(banner2, " none");
    }
    strcat(banner2, "\n");

    strcat(banner2, "THREAD-SAFE MODE:  ");
#ifdef _PGS_THREADSAFE
    /*
     * set mode in banner
     */
    strcat(banner2, "enabled\n");
#else
    strcat(banner2, "disabled\n");
#endif
    
    /* reset the SMF static variable "callerID" (defined elsewhere) to its value
       prior to the execution of this function (as saved in the local variable
       callerID) */

    PGS_SMF_SetCallerID(localCallerID);

    PGS_SMF_GetToolkitVersion(version);
    strcat(banner2, "TOOLKIT_VERSION: ");
    strcat(banner2, version);
    strcat(banner2, "\n");

    strcat(banner2, "****************************************\n\n");

    /* check the toolkit version number against the toolkit version number in
       the users PCF, they should be the same--if not issue a garrish warning
       message in the various log files */

    loggingStatus = PGS_PC_GetPCSData(PGSd_PC_CONFIGURATION,
				     PGSd_TOOLKIT_VERSION_LOGICAL,
				     pcfVersion, &numFiles);
    if ( loggingStatus != PGS_S_SUCCESS )
    {
	strcat(banner2,
	       "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
	       "!!                   W A R N I N G                     !!\n"
	       "!!  The Toolkit version was not found in the PCF.  The !!\n"
	       "!!  PCF in use should be replaced with a PCF           !!\n"
	       "!!  constructed from the template PCF delivered with   !!\n"
	       "!!  THIS version of the  Toolkit (see banner, above).  !!\n"
	       "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
    }
    else
    {
	/* we're done using pid_string (above) so recycle it here, this has
	   nothing to do with the process ID */

	pid_string = pcfVersion;
	while (*pid_string == ' ')
	{
	    pid_string++;
	}
	
	for (index=strlen(pid_string)-1;index>=0;index--)
	{
	    if (pid_string[index] != ' ')
	    {
		break;
	    }
	    pid_string[index] = '\0';
	}

	if ( strcmp(pid_string, version) != 0 )
	{
	    strcat(banner2,
		   "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
		   "!!                   W A R N I N G                     !!\n"
		   "!!  The Toolkit version found in the PCF does not      !!\n"
		   "!!  match the current Toolkit version.  The PCF in use !!\n"
		   "!!  should be replaced with a PCF constructed from the !!\n"
		   "!!  template PCF delivered with THIS version of the    !!\n"
		   "!!  Toolkit (see TOOLKIT_VERSION in banner, above).    !!\n"
		   "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
		   "\n");
	}
    }

#ifdef _PGE_THREADSAFE
    /*
     * We are writing to the files, let's lock.  For the banner, let's try 
     * to write even if we have a problem.
     */
    returnStatus = PGS_TSF_LockIt(PGSd_TSF_SMFWRITELOCK);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        strcat(banner2,"\n*** Problem with SMF lock - will try to write banner... ***\n"
    }
#endif
    if (shmMem->logStatus[0] != '\0')
    {
        if ((fptr = fopen(shmMem->logStatus,"a")) != (FILE *)NULL)
        {
            fprintf(fptr,"%s%s%s",banner1,shmMem->logStatus,banner2);
	    fclose(fptr);
        }
    }

    if (shmMem->logReport[0] != '\0')
    {
        if ((fptr = fopen(shmMem->logReport,"a")) != (FILE *)NULL)
        {
            fprintf(fptr,"%s%s%s",banner1,shmMem->logReport,banner2);
            fclose(fptr);
        }
    }

    if (shmMem->logUser[0] != '\0')
    {
        if ((fptr = fopen(shmMem->logUser,"a")) != (FILE *)NULL)
        {
            fprintf(fptr,"%s%s%s",banner1,shmMem->logUser,banner2);
            fclose(fptr);
        }
    }
#ifdef _PGE_THREADSAFE
    PGS_TSF_UnlockIt(PGSd_TSF_SMFWRITELOCK);
#endif


} /* end PGS_SMF_WriteBanner */


/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_WriteLogFile

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_status
    PGS_SMF_WriteLogFile(
        PGSSmfGlbVar *global_var);

FORTRAN:
    NONE

DESCRIPTION:
    To write messages to log files. 

INPUTS:    
    Name        Description

    global_var  global static buffer

OUTPUTS:     
    NONE

RETURNS:       
    PGS_S_SUCCESS
    PGSSMF_E_LOGFILE	
    PGSSMF_E_MSSLOGFILE	
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:    
    PGSSmfGlbVar    *global_var;
    PGSt_SMF_status  returnStatus;

    PGS_SMF_GetGlobalVar(&global_var);
    returnStatus = PGS_SMF_WriteLogFile(global_var);

NOTES:       
    If either status or user log files are not opened correctly,     
    (probably due to error from the PC tools), this
    routine  will return PGSSMF_E_LOGFILE.

REQUIREMENTS:
    PGSTK-0580,0590,0650

DETAILS:	   
    The format of the log files is:

      function:mnemonic:code
      message

      function:mnemonic:code
      message   

      ...
      ...
      ...

GLOBALS:
    PGSg_TSF_SMFLastCode
    PGSg_TSF_SMFRepMsgCount

FILES:
    Status log file
    User log file

FUNCTIONS_CALLED:
    PGS_SMF_CreateMsgTag()
    PGS_SMF_TestUserInfoLevel()
    PGS_SMF_TestNoticeLevel()
    PGS_SMF_MsgLevel()
    PGS_TSF_GetTSFMaster()
    PGS_SMF_TestErrorLevel()
    PGS_TSF_GetMasterIndex()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_WriteLogFile(             /* Write into log file */
    PGSSmfGlbVar *global_var)     /* Input global static buffer */
{    
    PGSt_SMF_status         returnStatus = PGS_S_SUCCESS;   
    PGSt_SMF_status         loggingStatus;   /* determines if input code should
						be logged or not */
    static PGSt_SMF_status  traceStatus;     /* determines if function tracing
						is on */ 

    static PGSt_integer     init_flag = 0;

    char            *trace_path;
    static char     last_trace_path[PGSd_SMF_MaxMsgLevel*PGS_SMF_MAX_FUNC_SIZE];
    /*
     * Message compression variables
     */
    static char             last_func[PGS_SMF_MAX_FUNC_SIZE];  /* last func */
    static char             last_mne[PGS_SMF_MAX_MNEMONIC_SIZE]; /* last mne */
    static char             last_msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* last msg */
    char                    rep_msg_buf[PGS_SMF_MAX_FUNC_SIZE+50]; /* msg buf */
    static PGSt_SMF_code    last_code;         /* last code */
    static PGSt_integer     rep_msg_count;     /* repetitive message count */
    PGSt_integer            output_flag;       /* output flag */


    /*
     * Message level variables
     */
    PGSt_integer            msg_level;      /* message level */
    PGSt_integer            log_off;        /* level logging disabled flag */
    char                    *indent_str;    /* indent string */
    char                    *func_str;      /* function name string */
 
    PGSt_SMF_status         statusLevel;    /* SMF level mask value */

    char                    actionString[PGS_SMF_MAX_ACT_SIZE];
					    /* MSS action message string */
    char                    actionLabel[PGS_SMF_MAX_MNEMONIC_SIZE];
					    /* MSS channel action mnemonic */
    char                    subject[PGS_SMF_MAX_MSGBUF_SIZE];
					    /* email action mnemonic */
#ifndef PGS_IR1
    char                    message[PGS_SMF_MAX_MSGBUF_SIZE];
					    /* email message text */
#endif /* PGS_IR1 */

#ifdef _PGS_THREADSAFE
    /*
     * Set up for threadsafe by getting globals and keys structure
     */
    PGSt_TSF_MasterStruct *masterTSF;
    PGSt_SMF_status extraStatus;
    char *lastFuncTSF;
    char *lastMneTSF;
    char *lastMsgTSF;
    PGSt_SMF_code lastCodeTSF;
    PGSt_integer repMsgCountTSF;
    int masterTSFIndex;
    extern PGSt_SMF_code PGSg_TSF_SMFLastCode[];
    extern PGSt_integer PGSg_TSF_SMFRepMsgCount[];

    /*
     * Get globals index and keys structure
     */
    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ((PGS_SMF_TestErrorLevel(returnStatus)) ||
        (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    lastFuncTSF = (char *) pthread_getspecific(
                               masterTSF->keyArray[PGSd_TSF_KEYSMFLASTFUNC]);
    lastMneTSF = (char *) pthread_getspecific(
                               masterTSF->keyArray[PGSd_TSF_KEYSMFLASTMNE]);
    lastMsgTSF = (char *) pthread_getspecific(
                               masterTSF->keyArray[PGSd_TSF_KEYSMFLASTMSG]);
    lastCodeTSF = PGSg_TSF_SMFLastCode[masterTSFIndex];
    repMsgCountTSF = PGSg_TSF_SMFRepMsgCount[masterTSFIndex];

    if (init_flag == 0)    
    {
	traceStatus = PGS_SMF_TraceControl(PGSd_QUERY);
        init_flag = 1;
    }
#else /* -D_PGS_THREADSAFE */

    /*
     * Initializations done for first call only 
     */
    if (init_flag == 0)    
    {
        last_func[0] = 0;
        last_mne[0] = 0;
        last_msg[0] = 0;
        last_code = 0;
        rep_msg_count = 0;

	last_trace_path[0] = '\0';
	traceStatus = PGS_SMF_TraceControl(PGSd_QUERY);
	
        init_flag = 1;
    }
#endif /* -D_PGS_THREADSAFE */

    /* PGS_SMF_LoggingControl() will check the input SMF status code to see if
       it should be logged.  PGS_SMF_LoggingControl() will check if logging has
       been turned off for the status level or the seed of the input status
       code, or if logging has been turned off for the specific status code
       itself (see PGS_SMF_LoggingControl() for details). */

    loggingStatus = PGS_SMF_LoggingControl(PGSd_QUERY,
					   global_var->msginfo.msgdata.code);
    if (loggingStatus == PGSSMF_M_LOGGING_DISABLED)
    {
	return PGS_S_SUCCESS;
    }
	
    
    /*
     * Initializations done for each call
     */
    rep_msg_buf[0] = 0;    /* clear repetitive message buffer */
    output_flag = 1;       /* set output flag to default: on */


    /*
     * Get the current message level, logging flag , 
     * indent string and function string
     */

    PGS_SMF_MsgLevel(PGSd_SMF_GetLevel, &msg_level, &log_off, &indent_str,
		     &func_str);


#ifdef _PGS_THREADSAFE
    /*
     * This section controls repetitive messages compression
     * The variable 'output_flag' is set to control output
     * in the log file write section, below.
     *
     * For threadsafe mode we use all the threadsafe variable names.
     */
    if ( ( (lastCodeTSF == global_var->msginfo.msgdata.code) &&
         (strcmp(lastFuncTSF, global_var->msginfo.funcname) == 0) ) &&
         (strcmp(lastMsgTSF, global_var->msg) == 0) )
    {
        /*
         * Got same function name and code as last call.  This means 
         * that the message is a duplicate.  Count it and set the flag
         * so it won't be logged..
         */
        output_flag = 0;    /* collecting repetitive messages: don't log */
        repMsgCountTSF++;     /* count this message */
        PGSg_TSF_SMFRepMsgCount[masterTSFIndex] = repMsgCountTSF;
    }
    else 
    {
        /*
         * Got a different code and/or function name
         */
        if (rep_msg_count != 0)
        {
            /*
             * If the repetitive message count is non-zero, then create 
             * the repetitive message output string. Then set output_flag
             * accordingly and clear the message count.
             */
            sprintf(rep_msg_buf,"%s:%s - %d additional occurrences%s%s",
                    lastFuncTSF,
                    lastMneTSF,
                    repMsgCountTSF,
		    PGS_SMF_LogPID(PGSd_QUERY),
                    PGS_SMF_GetThreadID());

	    if ((PGS_SMF_TestUserInfoLevel(lastCodeTSF) == PGS_TRUE) ||
		(PGS_SMF_TestNoticeLevel(lastCodeTSF) == PGS_TRUE))
	    {
		output_flag = 3; /* log total to User and Status log as well */
	    }
	    else
	    {
		output_flag = 2;    /* done collecting messages: log total */
	    }

            repMsgCountTSF = 0;  /* zero the repetitive message counter */
            PGSg_TSF_SMFRepMsgCount[masterTSFIndex] = repMsgCountTSF;

	    if ((PGS_SMF_TestUserInfoLevel(last_code) == PGS_TRUE) ||
		(PGS_SMF_TestNoticeLevel(last_code) == PGS_TRUE))
	    {
		output_flag = 3; /* log total to User log as well */
	    }
        }

        /*
         * Save the current code, mnemonic, message and function name 
         * for comparison on next call.
         */
        strcpy(lastFuncTSF, global_var->msginfo.funcname);
        strcpy(lastMneTSF, global_var->msginfo.msgdata.mnemonic);
        strcpy(lastMsgTSF, global_var->msg);
        lastCodeTSF = global_var->msginfo.msgdata.code;
        PGSg_TSF_SMFLastCode[masterTSFIndex] = lastCodeTSF;
    }

    returnStatus = PGS_TSF_LockIt(PGSd_TSF_SMFWRITELOCK);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
        goto DONE;
    }

    if (global_var->log.logStatus[0] == '\0')
    {            
        global_var->log.fptrStatus = (FILE *)NULL;
    }
    else
    {
        global_var->log.fptrStatus = fopen(global_var->log.logStatus, "a");  
    }

    if (global_var->log.logUser[0] == '\0')
    {            
        global_var->log.fptrUser = (FILE *)NULL;
    }
    else
    {
        global_var->log.fptrUser = fopen(global_var->log.logUser, "a");  
    }

#else /* -D_PGS_THREADSAFE */
    /*
     * This section controls repetitive messages compression
     * The variable 'output_flag' is set to control output
     * in the log file write section, below.
     */
    if ( ( (last_code == global_var->msginfo.msgdata.code) &&
         (strcmp(last_func, global_var->msginfo.funcname) == 0) ) &&
         (strcmp(last_msg, global_var->msg) == 0) )
    {
        /*
         * Got same function name and code as last call.  This means 
         * that the message is a duplicate.  Count it and set the flag
         * so it won't be logged..
         */
        output_flag = 0;    /* collecting repetitive messages: don't log */
        rep_msg_count++;     /* count this message */
    }
    else 
    {
        /*
         * Got a different code and/or function name
         */
        if (rep_msg_count != 0)
        {
            /*
             * If the repetitive message count is non-zero, then create 
             * the repetitive message output string. Then set output_flag
             * accordingly and clear the message count.
             */
            sprintf(rep_msg_buf,"%s:%s - %d additional occurrences%s%s",
                    last_func,
                    last_mne,
                    rep_msg_count,
		    PGS_SMF_LogPID(PGSd_QUERY),
                    PGS_SMF_GetThreadID());

	    if ((PGS_SMF_TestUserInfoLevel(last_code) == PGS_TRUE) ||
		(PGS_SMF_TestNoticeLevel(last_code) == PGS_TRUE))
	    {
		output_flag = 3; /* log total to User and Status log as well */
	    }
	    else
	    {
		output_flag = 2;    /* done collecting messages: log total */
	    }

            rep_msg_count = 0;  /* zero the repetitive message counter */

	    if ((PGS_SMF_TestUserInfoLevel(last_code) == PGS_TRUE) ||
		(PGS_SMF_TestNoticeLevel(last_code) == PGS_TRUE))
	    {
		output_flag = 3; /* log total to User log as well */
	    }
        }

        /*
         * Save the current code, mnemonic, message and function name 
         * for comparison on next call.
         */
        strcpy(last_func, global_var->msginfo.funcname);
        strcpy(last_mne, global_var->msginfo.msgdata.mnemonic);
        strcpy(last_msg, global_var->msg);
        last_code = global_var->msginfo.msgdata.code;
    }
#endif /* -D_PGS_THREADSAFE */

    /*
     * Make sure permission to record errors to log file is turned ON.
     * If turned OFF, then simply return PGS_S_SUCCESS.
     */
    if ((writeLogFile != PGSd_SMF_WRITELOG_ON) ||
        (global_var->log.record != PGS_TRUE))
    {
        goto DONE;                        /* go to common exit point */
    }

    /* 
     * If permissions were turned on, but log files were not open
     * then we return PGSSMF_E_LOGFILE
     */
    if ((global_var->log.fptrStatus == (FILE *)NULL) ||
        (global_var->log.fptrUser   == (FILE *)NULL))
    {
        returnStatus = PGSSMF_E_LOGFILE;  /* set return status */
        goto DONE;                        /* go to common exit point */
    }


    /*
     * Write status messages to the log files, assuming that
     * logging is enabled for the current message level.
     * 
     * This section checks the value of output_flag and begin_end
     * to determine what message, if any, should be output.
     */

    if ( log_off == 0 )
    {  
        /*
         * All codes should be recorded into the status log file.
         */

        switch (output_flag)    /* check for proper output mode */
        {
          case 3:    /* done collecting repetitive messages:
			output total to User log file */

	    fprintf(global_var->log.fptrUser,"%s%s%s%s\n\n",
		    indent_str,
		    rep_msg_buf,
		    PGS_SMF_LogPID(PGSd_QUERY),
                    PGS_SMF_GetThreadID());

          case 2:    /* done collecting repetitive messages:
			output total to Status log file */
	    
            fprintf(global_var->log.fptrStatus,"%s%s%s%s\n\n",
		    indent_str,
		    rep_msg_buf,
		    PGS_SMF_LogPID(PGSd_QUERY),
                    PGS_SMF_GetThreadID());


	  case 1:    /* default: output the message */

	    /* if Error tracing has been enabled, get the trace path and print
	       it to the log file */

	    if (traceStatus == PGSSMF_M_ERROR_TRACE_ENABLED)
	    {
		PGS_SMF_MsgLevel(PGSd_SMF_GetTracePath, NULL, NULL, &trace_path,
				 NULL);
		if (strcmp(trace_path, last_trace_path) != 0)
		{
		    fprintf(global_var->log.fptrStatus, "%s", trace_path);
		    strcpy(last_trace_path, trace_path);
		}
	    }
	    
            fprintf(global_var->log.fptrStatus,"%s%s:%s:%d%s%s\n",
		    indent_str,
		    global_var->msginfo.funcname,
		    global_var->msginfo.msgdata.mnemonic,
		    global_var->msginfo.msgdata.code,
		    PGS_SMF_LogPID(PGSd_QUERY),
                    PGS_SMF_GetThreadID()); 
            fprintf(global_var->log.fptrStatus,"%s%s\n\n",
                indent_str,
                global_var->msg);                 

            fflush(global_var->log.fptrStatus);

#ifdef PGS_IR1
            /*
             * Send the message to the event logger, if it has
             * an associated action string of the proper type.
	     * EventLog Trigger MUST exist and be turned on 
	     * in the PCF for this communication channel to function.
             * NOTE: DAAC Toolkit ONLY
             */

	    /*
	     * Need test here to prevent LogEvent from being called with
	     * every StatusLog update. 
	     * [TK6] - Add action-code to msginfo.msgdata structure to make
	     *         this test more efficient; e.g. perform bit-wise level 
	     *	       test.
	     */

	    /* preserve original mnemonic, which is about to get overwritten
	       witht the action code mnemonic */
	    
	    strcpy(subject, global_var->msginfo.msgdata.mnemonic);

	    returnStatus = PGS_SMF_GetActionMneByCode(
				        global_var->msginfo.msgdata.code,
        		        	actionString,
        		        	actionLabel);
	     if (returnStatus == PGSSMF_W_NOACTION)
	     {
                 /* No action associated with status condition! */
                 /* CAUTION: DO NOT replace conditional expression
                  *          with a call to *_Test*Level(); doing
                  *          so overwrites the 'msgdata.code' member
                  *          with a 0 (success) value!
                  */
                 break;
             }
             statusLevel = PGS_SMF_TestStatusLevel(returnStatus);
             if (statusLevel == PGS_SMF_MASK_LEV_E)
             {
	         returnStatus = PGSSMF_E_MSSLOGFILE;
                 break;
	     }
 
    	     returnStatus = PGS_SMF_GetActionType( actionLabel );
             if ((returnStatus == PGSSMF_M_EMAIL_ACTION) ||
		 (returnStatus == PGSSMF_M_INFO_ACTION))
	     {
                 if (strcmp(comm_snmp,PGSd_SMF_ON)==0)
                 {
                     returnStatus = PGS_SMF_LogEvent(
                        global_var->msginfo.msgdata.code ,
                        global_var->msg,
                        subject,
		        actionString,
			returnStatus );
	         }
	     } /* end Action type test */
#else
	     /* preserve original mnemonic, which is about to get overwritten
		witht the action code mnemonic */

	    sprintf(subject, "Toolkit return status %s", 
		    global_var->msginfo.msgdata.mnemonic);
	    
    	     returnStatus = PGS_SMF_GetActionMneByCode(
				        global_var->msginfo.msgdata.code,
        		        	actionString,
        		        	actionLabel);
             if (returnStatus == PGSSMF_W_NOACTION)
             {
                 /* No action associated with status condition! */
                 /* CAUTION: DO NOT replace conditional expression
                  *          with a call to *_Test*Level(); doing
                  *          so overwrites the 'msgdata.code' member
                  *          with a 0 (success) value!
                  */
                 break;
             }
             statusLevel = PGS_SMF_TestStatusLevel(returnStatus);
             if (statusLevel == PGS_SMF_MASK_LEV_E)
             {
	         returnStatus = PGSSMF_E_MSSLOGFILE;	
                 break;
	     }

#ifndef PGS_DAAC

	     /* sending e-mail is only allowed at the SCFs, this functionality
		is disabled at the DAACs (note that email functionality may be
		sisabled in any case by adding the mnemonic
		PGSSMF_M_EMAIL_ACTION to the list of disabled status code
		mnemonics in the PCF) */

    	     returnStatus = PGS_SMF_GetActionType( actionLabel );
             if (returnStatus == PGSSMF_M_EMAIL_ACTION)
	     {
		 loggingStatus = PGS_SMF_LoggingControl(PGSd_QUERY,
							returnStatus);
		 if (loggingStatus == PGSSMF_M_LOGGING_ENABLED)
		 {
		     strcpy(message, global_var->msg);
		     returnStatus = PGS_SMF_SendMail(actionString,"","",subject,
						     message);
		 }
	     } /* end Action type test */

#endif /* PGS_DAAC */
#endif /* PGS_IR1 */

            break;


          case 0:    /* collecting repetitive messages: don't output */

            break;


        }  /* end: switch (output_flag) */


        /*
         * Any _U_ or _N_ level codes should also be recorded into 
         * the user log file.
         */
        if ((PGS_SMF_TestUserInfoLevel(global_var->msginfo.msgdata.code) == 
	     PGS_TRUE)
	    ||
            (PGS_SMF_TestNoticeLevel(global_var->msginfo.msgdata.code) == 
	     PGS_TRUE))
        {                                     
            switch (output_flag)    /* check for proper output mode */
            {
              case 3:  /* default: output the message */
              case 2:  /* default: output the message */
              case 1:  /* default: output the message */

                fprintf(global_var->log.fptrUser,"%s%s:%s:%d%s%s\n",
                        indent_str,
                        global_var->msginfo.funcname,
                        global_var->msginfo.msgdata.mnemonic,
                        global_var->msginfo.msgdata.code,
			PGS_SMF_LogPID(PGSd_QUERY),
                        PGS_SMF_GetThreadID());
                fprintf(global_var->log.fptrUser,"%s%s\n\n",
                        indent_str,
                        global_var->msg);                    
    
                fflush(global_var->log.fptrUser);

                break;

    
              case 0:  /* collecting repetitive messages: don't output */

                break;


            }  /* end: switch (output_flag) */

        }  /* end: if ((PGS_SMF_TestUserInfoLevel ... */

    }  /* end: if (log_off == 0) */

/*
 * Common exit point
 */

DONE:
#ifdef _PGS_THREADSAFE
    if (global_var->log.fptrStatus != (FILE *) NULL)
    {
        fclose(global_var->log.fptrStatus);
    }
    if (global_var->log.fptrUser != (FILE *) NULL)
    {
        fclose(global_var->log.fptrUser);
    }

    extraStatus = PGS_TSF_UnlockIt(PGSd_TSF_SMFWRITELOCK);
    if ((returnStatus == PGS_S_SUCCESS) &&
        (PGS_SMF_TestErrorLevel(extraStatus)))
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
    }
#endif
    return returnStatus;

} /* end PGS_SMF_WriteLogFile */

/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Retrieve MSS Global values

NAME:   
    PGS_SMF_GetMSSGlobals

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void
    PGS_SMF_GetMSSGlobals(
        char**    eventLogRef,
	char**    comm_snmp);

FORTRAN:
    NONE

DESCRIPTION:
    Return key MSS global parameters to activate Event Logging Code
    from an external routine.

INPUTS:   
    Name        Description                               Units        Min        Max              


OUTPUTS:
    Name        Description                               Units        Min        Max 
    eventLogRef	Reference name of Event Log file as 
		defined in the PCF
    comm_snmp	Value of activation switch defined
	        in the PCF

RETURNS:       
    NONE

EXAMPLES:        

    char *eventRef;
    char *eventSwitch;

    PGS_SMF_GetMSSGlobals(&eventRef,&eventSwitch);

NOTES:           
    This routine is only to be used internally by ARC toolkit developers.    
    More specifically, this call should only be made from PGS_PC_InitCom
    and PGS_PC_TermCom.

REQUIREMENTS:
    N/A

DETAILS:	   
    The main reason why this function is needed is to let PC
    initialization and termination applications know what
    the values of these settings are before attempting to
    access the MSS event logging software.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
#ifdef PGS_IR1
void
PGS_SMF_GetMSSGlobals( 		  /* Get Global MSS attributes */               
    char** eventRef,		  /* Get Event Log file name */
    char** eventSwitch)     	  /* Get Event Log activation switch */
{

    *eventSwitch = comm_snmp;
    *eventRef = eventLogRef;


} /* end PGS_SMF_GetMSSGlobals */
#endif /* PGS_IR1 */

/*****************************************************************
BEGIN_PROLOG:

TITLE:
       FLOATING POINT EQUALITY
NAME:
    PGS_SMF_FEQ

SYNOPSIS:
C:
    #include <PGS_SMF.h>

    PGSt_integer 
    PGS_SMF_FEQ(
        double    e,
        double    f);

FORTRAN:
    NONE
DESCRIPTION:
      Returns either a 1 or 0 when comparing two floating point numbers that are very small.
INPUTS:  
    Name        Description                Units        Min    Max
    a           The first floating value                             
    b           The second floating value

OUTPUTS:
    Name        Description                Units        Min   Max

RETURNS:
    NONE
GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/


PGSt_integer PGS_SMF_FEQ(PGSt_double a, PGSt_double b)
{
            PGSt_integer retval;
            if ( a == 0 && fabs(b) < EPSABS_64)
                    retval = 1;
            else if (b == 0 && fabs(a) < EPSABS_64)
                    retval =1;
            else
            {
                    retval = (fabs((a-b)) < (EPSREL_64 * fabs(a)) );
            }
            return retval;
    }

