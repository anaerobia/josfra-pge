/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_SMF_SendStatusReport.c

DESCRIPTION:	
  The procedure defined in this file will provide the means to transfer the Status
  Report, User Status and Status Message log files created throughout the run of a PGE; 
  as a side-effect, an electronic message will be transmitted announcing the availability 
  of the status log files to the interested parties.

  This file contains the following routines:
    PGS_SMF_SendStatusReport()
    PGS_SMF_SysSendStatusReport()

AUTHOR:
  Kelvin K. Wan / Applied Research Corp.

HISTORY:
  15-Aug-1994 Standard Convention  
  28-Dec-1994 Enhanced for TK4 
  13-Oct-1995 DPH Corrected oversight which prevented transmission of
                  files from begin disabled by the PCF switch in
                  function PGS_SMF_SysTermSendStatusReport().
                  Note that errors from the calls made in this
                  module are not recorded in the log since the log
                  files have been closed when this function is called.

END_FILE_PROLOG:
*****************************************************************/

/*
 * System Headers 
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Toolkit Headers 
 */
#include <PGS_MEM.h>
#include <PGS_PC.h>
#include <PGS_IO.h>
#include <PGS_SMF.h>

/*****************************************************************  
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SendStatusReport

SYNOPSIS:   

C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_SendStatusReport(
        void);

FORTRAN:
    integer function pgs_smf_sendstatusreport()

DESCRIPTION:
    This tool provides the means to transfer the Status Report, User Status
    and Status Message Log files to a temporary holding area for later retrieval 
    by the user. This tool also transmits an electronic message to notify the user 
    as to the availability of the status log files

INPUTS:    
    Name        Description Units                      Min        Max

OUTPUTS:    
    Name        Description Units                      Min        Max

RETURNS:    
    PGS_S_SUCCESS
    PGSSMF_E_SENDSTATUS_LOG    
    PGSSMF_M_TRANSMIT_DISABLE

EXAMPLES:

    C:	
    ==
    PGSt_SMF_status returnStatus;

    returnStatus = PGS_SMF_SendStatusReport();
    if (returnStatus == PGS_S_SUCCESS)
    {       
        /# send status report success #/
    }

    FORTRAN:
    ========
    implicit none
    integer pgs_smf_pgs_smf_sendstatusreport

    integer returnStatus

    returnStatus = pgs_smf_sendstatusreport()   
    if (return_status .EQ. PGS_S_SUCCESS) then
C       send status report success
    endif

NOTES:  
    As of TK4, this tool will not send the log files immediately. It will be sent 
    automatically during termination process. 

REQUIREMENTS:
    PGSTK-0580,0590,0600,0631

DETAILS:	        
    Warning
    =======
    Do not attempt to send the log files to the same host and directory
    that this program is running on. The original files will be deleted as 
    ftp protocol will upon determination that the destination file is 
    the same as the source, then it will remove the file first before 
    sending the same file back.

    Refer to PCF.v3 and look for the following logical ID:

        10106|RemoteHost|<hostname>
        10107|RemotePath|<directory to send the files to>

    Make sure that the <hostname> is not the host that the program is running
    on. As long as the <hostname> is not the same host for the client, then 
    sending log files to <directory to send the files to> would be safe.

    Of course the log files with the same name will be replaced no matter what. 
    The danger here is that the PCF that this routine will send will also be nullify; 
    thus creating an unpredictable behavior to the PGS API.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:   
    PGS_PC_GetConfigData()
    PGS_SMF_SetStaticMsg() 
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SendStatusReport(   /* Send status report */
    void)
{
    PGSt_SMF_status  returnStatus = PGS_E_TOOLKIT;          /* return status */
    char             transmit[PGSd_PC_VALUE_LENGTH_MAX];    /* transmission flag */
    char             msg[PGS_SMF_MAX_MSG_SIZE];             /* code message */
    char             mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];   /* code mnemonic */   
    PGSt_SMF_code    code;                                  /* code */


    /*
     * Get permission to transfer the log files.
     */
    if (PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_TRANSMIT,transmit) == PGS_S_SUCCESS)
    {
        if (strcmp(transmit,PGSd_SMF_ON) == 0)
        {
            returnStatus = PGS_S_SUCCESS;
        }
        else
        {
            returnStatus = PGSSMF_M_TRANSMIT_DISABLE;
        }
    }

    /*
     * Set exit state.
     * Actually, since the Log files are already closed when this function
     * is called, calls to *Set*Msg() will only return PGSSMF_E_LOGFILE.
     */
    if (returnStatus == PGS_E_TOOLKIT)
    {
        PGS_SMF_GetMsg(&code,mnemonic,msg);
        PGS_SMF_SetDynamicMsg(PGSSMF_E_SENDSTATUS_LOG,msg,"PGS_SMF_SendStatusReport()");
        returnStatus = PGSSMF_E_SENDSTATUS_LOG;    
    }
    else 
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_SMF_SendStatusReport()");
    }    

    return(returnStatus);

} /* end PGS_SMF_SendStatusReport */

/*****************************************************************  
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SysSendStatusReport

SYNOPSIS:   

C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_SysSendStatusReport(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    This tool provides the means to transfer the Status Report, User Status
    and Status Message Log files to a temporary holding area for later retrieval 
    by the user. This tool also transmits an electronic message to notify the user 
    as to the availability of the status log files

INPUTS:    
    Name        Description Units                      Min        Max

OUTPUTS:    
    Name        Description Units                      Min        Max

RETURNS:
    PGS_S_SUCCESS
    PGSSMF_E_NOHOSTNAME   
    PGSSMF_E_NOMAIL_ADDR         
    PGSSMF_E_REMOTEPATH        
    PGSSMF_E_SENDSTATUS_LOG

EXAMPLES:	
    PGSt_SMF_status returnStatus;

    returnStatus = PGS_SMF_SysSendStatusReport();
    if (returnStatus == PGS_S_SUCCESS)
    {       
        /# send status report success #/
    }

NOTES:  
    Need to open temporary files to transfer the log files to remote host so that
    SMF can still capture error into log files. Thus the files that are sent to remote 
    will be the current state.       

REQUIREMENTS:
    PGSTK-0580,0590,0600

DETAILS:	            
    This routine is called in the termination process. Therefore this tool is only
    to be used by ARC developers.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_SMF_GetGlobalVar()  
    PGS_PC_GetReference() 
    PGS_PC_GetConfigData()     
    PGS_IO_Gen_Open()
    PGS_MEM_Calloc()
    PGS_MEM_Free()       
    PGS_SMF_SendFile()    
    PGS_SMF_SendMail()    
    PGS_SMF_SetStaticMsg()
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SysSendStatusReport(   /* Send log files to remote host */ 
    void)
{
    PGSt_SMF_status          returnStatus;                                  /* return status */
    PGSt_SMF_status          retStat;                                       /* return status */
    char                    *token;                                         /* string token */
    char                     remoteHostName[PGSd_PC_VALUE_LENGTH_MAX];      /* remote host name */
    char                     remotePath[PGSd_PC_VALUE_LENGTH_MAX];          /* remote path */
    char                     remoteFileName[PGSd_PC_VALUE_LENGTH_MAX];      /* remote file name */  
    char                     tmpStatusFileName[PGSd_PC_VALUE_LENGTH_MAX];   /* temporary status log file */
    char                     tmpUserFileName[PGSd_PC_VALUE_LENGTH_MAX];     /* temporary user log file */
    char                     tmpReportFileName[PGSd_PC_VALUE_LENGTH_MAX];   /* temporary report log file */     
    char                     mailUser[PGSd_PC_VALUE_LENGTH_MAX];            /* user's mailing address */
    char                    *msgBuf = (char *)NULL;                         /* mail message */
    char                     mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];           /* code mnemonic */
    char                     buf[PGS_SMF_MAX_MSGBUF_SIZE];                  /* max. buffer message */
    char                     subText[100];                                  /* subject text for the mail message */
    PGSt_integer             version = 1;                                   /* version number */
    PGSt_integer             msgLen;                                        /* message length */
    PGSt_SMF_code            code;                                          /* code */
    PGSSmfGlbVar            *global_var;                                    /* global static buffer */
    PGSt_SMF_boolean         sendStatus = PGS_FALSE;                        /* send status log file flag */
    PGSt_SMF_boolean         sendReport = PGS_FALSE;                        /* send report log file flag */    
    PGSt_SMF_boolean         sendUser   = PGS_FALSE;                        /* send user log file flag */
    PGSt_SMF_boolean         copyStatus = PGS_FALSE;                        /* copy status log file flag */
    PGSt_SMF_boolean         copyReport = PGS_FALSE;                        /* copy report log file flag */    
    PGSt_SMF_boolean         copyUser   = PGS_FALSE;                        /* copy user log file flag */    
    struct stat              fileStat;                                      /* file status */
    FILE                    *fptr;                                          /* file pointer */

    PGS_SMF_GetGlobalVar(&global_var);

    /*
     * Initialize data.
     */     
    tmpStatusFileName[0] = '\0';
    tmpUserFileName[0]   = '\0';
    tmpReportFileName[0] = '\0';    

    /*
     * Get temporary log file names.
     */    
    PGS_PC_GetReference(PGSd_SMF_LOGICAL_TMPSTATUS,&version,tmpStatusFileName); version = 1;     
    PGS_PC_GetReference(PGSd_SMF_LOGICAL_TMPREPORT,&version,tmpReportFileName); version = 1;   
    PGS_PC_GetReference(PGSd_SMF_LOGICAL_TMPUSER,&version,tmpUserFileName);     version = 1; 

    /*
     * Get opened log file names.
     */      
    if ((tmpStatusFileName[0] != '\0') && (global_var->log.logStatus[0] != '\0'))
    {
        copyStatus = PGS_TRUE;         
    }     

    if ((tmpUserFileName[0] != '\0') && (global_var->log.logUser[0] != '\0'))
    {
        copyUser = PGS_TRUE;         
    }     

    if ((tmpReportFileName[0] != '\0') && (global_var->log.logReport[0] != '\0'))
    {
        copyReport = PGS_TRUE;         
    }     

    /*
     * Tell SMF routines not to record errors into the log files at this moment.
     */
    global_var->log.record = PGS_FALSE;

    /*
     * Close all opened log files.
     */              
    if (global_var->log.fptrStatus != (FILE *)NULL) fclose(global_var->log.fptrStatus);
    if (global_var->log.fptrReport != (FILE *)NULL) fclose(global_var->log.fptrReport);
    if (global_var->log.fptrUser   != (FILE *)NULL) fclose(global_var->log.fptrUser);

    global_var->log.fptrStatus = (FILE *)NULL;
    global_var->log.fptrReport = (FILE *)NULL;
    global_var->log.fptrUser   = (FILE *)NULL;           


    /*
     * Copy status log to temporary status log file. 
     */
    if (copyStatus == PGS_TRUE)
    {
        copyStatus = PGS_FALSE;

        msgLen = strlen(global_var->log.logStatus) + strlen(tmpStatusFileName) + 20;
        if (PGS_MEM_Calloc((void **)&msgBuf,msgLen,sizeof(char)) == PGS_S_SUCCESS)
        {
            /*
             * Check if status log file exist in the user workstation.
             */
            if (stat(global_var->log.logStatus,&fileStat) == 0)
            {

                /*
                 * Check if we can create tmpStatusFileName in user workstation.
                 */
                if ((fptr = fopen(tmpStatusFileName,"a")) != (FILE *)NULL)
                {

                    /*
                     * Okay, the temporary and log file are not bogus.
                     */
                    fclose(fptr);

                    /*
                     * Remove the temporary file so that system would not
                     * spit out prompt message for 'cp' command.
                     */
                    remove(tmpStatusFileName);

                    sprintf(msgBuf,"cp %s %s",global_var->log.logStatus,tmpStatusFileName);
                    if (PGS_SMF_System(msgBuf) == PGS_S_SUCCESS)
                    {
                        copyStatus = PGS_TRUE;
                    }
                }
            }                
        }

        if (msgBuf != (char *)NULL)
        {
            PGS_MEM_Free(msgBuf);
            msgBuf = (char *)NULL;
        }
    }

    /*
     * Copy report log to temporary report log file. 
     */
    if (copyReport == PGS_TRUE)
    {
        copyReport = PGS_FALSE;

        msgLen = strlen(global_var->log.logReport) + strlen(tmpReportFileName) + 20;
        if (PGS_MEM_Calloc((void **)&msgBuf,msgLen,sizeof(char)) == PGS_S_SUCCESS)
        {        
            /*
             * Check if report log file exist in the user workstation.
             */
            if (stat(global_var->log.logReport,&fileStat) == 0)
            { 

                /*
                 * Check if we can create tmpReportFileName in user workstation.
                 */
                if ((fptr = fopen(tmpReportFileName,"a")) != (FILE *)NULL)
                {

                    /*
                     * Okay, the temporary and log file are not bogus.
                     */
                    fclose(fptr);

                   /*
                    * Remove the temporary file so that system would not
                    * spit out prompt message for 'cp' command.
                    */
                   remove(tmpReportFileName);

                   sprintf(msgBuf,"cp %s %s",global_var->log.logReport,tmpReportFileName);
                   if (PGS_SMF_System(msgBuf) == PGS_S_SUCCESS)
                   {
                       copyReport = PGS_TRUE;
                   }
               }
            }
        }

        if (msgBuf != (char *)NULL)
        {
            PGS_MEM_Free(msgBuf);
            msgBuf = (char *)NULL;
        }
    }    

    /*
     * Copy user log to temporary user log file. 
     */
    if (copyUser == PGS_TRUE)
    {
        copyUser = PGS_FALSE;

        msgLen = strlen(global_var->log.logUser) + strlen(tmpUserFileName) + 20;
        if (PGS_MEM_Calloc((void **)&msgBuf,msgLen,sizeof(char)) == PGS_S_SUCCESS)
        {

            /*
             * Check if user log file exist in the user workstation.
             */
            if (stat(global_var->log.logUser,&fileStat) == 0)
            {
                /*
                 * Check if we can create tmpUserFileName in user workstation.
                 */
                if ((fptr = fopen(tmpUserFileName,"a")) != (FILE *)NULL)
                {

                    /*
                     * Okay, the temporary and log file are not bogus.
                     */
                    fclose(fptr);

                   /*
                    * Remove the temporary file so that system would not
                    * spit out prompt message for 'cp' command.
                    */
                   remove(tmpUserFileName);

                   sprintf(msgBuf,"cp %s %s",global_var->log.logUser,tmpUserFileName);
                   if (PGS_SMF_System(msgBuf) == PGS_S_SUCCESS)
                   {
                       copyUser = PGS_TRUE;
                   }
               }
            }
        }

        if (msgBuf != (char *)NULL)
        {
            PGS_MEM_Free(msgBuf);
            msgBuf = (char *)NULL;
        }
    }         


    /*
     * Now tell SMF routines to start to record errors into the log files.
     */
    global_var->log.record = PGS_TRUE;

    /*
     * Open the log files to start capturing errors.
     */ 
    if (global_var->log.logStatus[0] != '\0') 
    {
        global_var->log.fptrStatus = fopen(global_var->log.logStatus,"a");   
    }     

    if (global_var->log.logUser[0] != '\0')
    {
        global_var->log.fptrUser = fopen(global_var->log.logUser,"a");         
    }     

    if (global_var->log.logReport[0] != '\0')
    {
        global_var->log.fptrReport = fopen(global_var->log.logReport,"a");           
    }     


    returnStatus = PGSSMF_E_SENDSTATUS_LOG;

    /*
     * Send the newly created temporary log files to remote host.
     */  
    if ((copyStatus == PGS_TRUE) || (copyReport == PGS_TRUE) || (copyUser == PGS_TRUE))    
    {                
        /*
         * Get remote host name.
         */
        retStat = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_REMOTEHOSTNAME,remoteHostName);
        PGS_SMF_RemoveSpace(remoteHostName);

        if ((retStat == PGS_S_SUCCESS) && (remoteHostName[0] != '\0'))
        { 
             /*
              * Get user address to send the files to.
              */
             retStat = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_MAILUSER,mailUser);
             PGS_SMF_RemoveSpace(mailUser);                

             if ((retStat == PGS_S_SUCCESS) && (mailUser[0] != '\0'))
             {                                 
                 /*
                  * Get remote path to send the files to.
                  */
                 retStat = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_REMOTEPATH,remotePath);
                 PGS_SMF_RemoveSpace(remotePath); 

                 if ((retStat == PGS_S_SUCCESS) && (remotePath[0] != '\0'))
                 {                                        
                     /*
                      * Send temporary status log file.
                      */    
                     if (copyStatus == PGS_TRUE)
                     {                                              
                         token = strrchr(global_var->log.logStatus,'/');
                         if (token == (char *)NULL)
                         {
                             sprintf(remoteFileName,"%s/%s",remotePath,global_var->log.logStatus);
                         }
                         else
                         {
                             sprintf(remoteFileName,"%s%s",remotePath,token);
                         }

                         returnStatus = PGS_SMF_SendFile(tmpStatusFileName,remoteHostName,remoteFileName);
                         if (returnStatus == PGS_S_SUCCESS)
                         {
                             strcpy(tmpStatusFileName,remoteFileName);
                             sendStatus = PGS_TRUE;
                         }
                     }

                     /*
                      * Send temporary user log file.
                      */   
                     if (copyUser == PGS_TRUE)
                     {                                                                  
                         token = strrchr(global_var->log.logUser,'/');
                         if (token == (char *)NULL)
                         {
                             sprintf(remoteFileName,"%s/%s",remotePath,global_var->log.logUser);
                         }
                         else
                         {
                             sprintf(remoteFileName,"%s%s",remotePath,token);
                         }

                         returnStatus = PGS_SMF_SendFile(tmpUserFileName,remoteHostName,remoteFileName);
                         if (returnStatus == PGS_S_SUCCESS)
                         {
                              strcpy(tmpUserFileName,remoteFileName);
                              sendUser = PGS_TRUE;
                         }
                     }

                     /*
                      * Send temporary report log file.
                      */   
                     if (copyReport == PGS_TRUE)
                     {                                                    
                         token = strrchr(global_var->log.logReport,'/');
                         if (token == (char *)NULL)
                         {
                             sprintf(remoteFileName,"%s/%s",remotePath,global_var->log.logReport);
                         }
                         else
                         {
                             sprintf(remoteFileName,"%s%s",remotePath,token);
                         }

                         returnStatus = PGS_SMF_SendFile(tmpReportFileName,remoteHostName,remoteFileName);
                         if (returnStatus == PGS_S_SUCCESS)
                         {
                             strcpy(tmpReportFileName,remoteFileName);
                             sendReport = PGS_TRUE; 
                         }
                     }


                     /*
                      * Alert remote user that the log files are already send to destination.
                      */
                     if ((sendReport == PGS_TRUE) || (sendUser == PGS_TRUE) || (sendStatus == PGS_TRUE))
                     {
                         msgLen = strlen(tmpStatusFileName) + 
                                  strlen(tmpUserFileName)   +
                                  strlen(tmpReportFileName) +
                                  strlen(remoteHostName)    +
                                  1000;

                         if (PGS_MEM_Calloc((void **)&msgBuf,msgLen,sizeof(char)) == PGS_S_SUCCESS)
                         {
                             sprintf(msgBuf,"The following log files are located at %s:~%s>\n",remoteHostName,remotePath);

                             if (sendStatus == PGS_TRUE)
                             {
                                 token = strrchr(global_var->log.logStatus,'/');
                                 strcat(msgBuf,"    ");
                                 strcat(msgBuf,token+1);
                                 strcat(msgBuf,"\n");
                             }

                             if (sendUser == PGS_TRUE)
                             {
                                 token = strrchr(global_var->log.logUser,'/');
                                 strcat(msgBuf,"    ");
                                 strcat(msgBuf,token+1);
                                 strcat(msgBuf,"\n");
                             }

                             if (sendReport == PGS_TRUE)
                             {
                                 token = strrchr(global_var->log.logReport,'/');                      
                                 strcat(msgBuf,"    ");
                                 strcat(msgBuf,token+1);
                                 strcat(msgBuf,"\n");
                             }

                             msgBuf[strlen(msgBuf)] = '\0';
                             sprintf(subText,"PGE %s Log Files",global_var->log.msgTag);

                             if (PGS_SMF_SendMail(mailUser,"","",subText,msgBuf) == PGS_S_SUCCESS)
                             {
                                 returnStatus = PGS_S_SUCCESS;
                             }
                             else
                             {
                                 /* 
                                  * Whatever return from PGS_SMF_SendMail().
                                  */
                                 PGS_SMF_GetMsg(&code,mnemonic,buf);
                             }
                         } 
                         else
                         {
                             /*
                              * Whatever return from PGS_MEM_Calloc().
                              */
                             PGS_SMF_GetMsg(&code,mnemonic,buf);
                         }   

                         if (msgBuf != (char *)NULL)
                         {
                             PGS_MEM_Free(msgBuf);
                             msgBuf = (char *)NULL;
                         }                                     
                     }
                     else
                     { 
                         /*
                          * For some reasons we could not send status, user, and report log files.
                          */
                         PGS_SMF_GetMsgByCode(PGSSMF_E_SENDSTATUS_LOG,buf);
                     }
                 }                     
                 else
                 {      
                     /*
                      * Get RemotePath error.
                      */                    
                     returnStatus = PGSSMF_E_REMOTEPATH; 
                 }             
             }
             else
             {         
                 /*
                  * Get MailAddr error.
                  */
                 returnStatus = PGSSMF_E_NOMAIL_ADDR;
             }       
        }
        else
        {
            /*
             * Get RemoteHostName error.
             */
            returnStatus = PGSSMF_E_NOHOSTNAME;
        }
    }
    else
    {
        /*
         * For some reasons we could not send status, user, and report log files.
         */
        PGS_SMF_GetMsgByCode(PGSSMF_E_SENDSTATUS_LOG,buf);
    }


    /*
     * Set exit state.
     */
    if (returnStatus == PGS_S_SUCCESS         ||
        returnStatus == PGSSMF_E_REMOTEPATH   || 
        returnStatus == PGSSMF_E_NOMAIL_ADDR  ||
        returnStatus == PGSSMF_E_NOHOSTNAME)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_SMF_SysSendStatusReport()");       
    }
    else
    {
        PGS_SMF_SetDynamicMsg(returnStatus,buf,"PGS_SMF_SysSendStatusReport()");
    }       

    return(returnStatus);

} /* end PGS_SMF_SysSendStatusReport */
/*****************************************************************   
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SysTermSendStatusReport

SYNOPSIS:   

C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_SysTermSendStatusReport(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    Send log files to remote host during termination process.

INPUTS:
    Name        Description Units                      Min        Max    

OUTPUTS:    
    Name        Description Units                      Min        Max

RETURNS:    
    PGS_S_SUCCESS     
    PGS_SH_SMF_SENDLOGFILE

EXAMPLES:    
    Refer to PGS_SMF_TermProc();

NOTES:  
    During termination process, Ray will make sure that this tool will not be
    called when system pass in writeLogFile == PGSd_SMF_WRITELOG_OFF.

REQUIREMENTS:
    PGSTK-0630

DETAILS:	    
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_SMF_SysSendStatusReport()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SysTermSendStatusReport(
    void)
{
    PGSt_SMF_status  returnStatus;   /* return status */

    returnStatus = PGS_SMF_SendStatusReport();
    if (returnStatus != PGSSMF_M_TRANSMIT_DISABLE)
    {
        if ((returnStatus = PGS_SMF_SysSendStatusReport()) != PGS_S_SUCCESS)
        {
            returnStatus = PGS_SH_SMF_SENDLOGFILE;
        }   
    }

    return(returnStatus);

} /* end PGS_SMF_SysTermSendStatusReport */
