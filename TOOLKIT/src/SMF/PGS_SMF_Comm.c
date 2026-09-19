/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_SMF_Comm.c

DESCRIPTION:	
  This file contains the common error handling functions for PGS.
  The routines are: 
      PGS_SMF_System()
      PGS_SMF_SendMail()
      PGS_SMF_SendFile()
      PGS_SMF_CheckNetrcFile()

AUTHOR:
  Kelvin K. Wan / Applied Research Corp.

HISTORY:
  15-Aug-1994 Standard Convention  
  03-Feb-2005 MP  Modified to use strerror instead of sys_errlist 
                  for Linux (ECSed41850)

END_FILE_PROLOG:
*****************************************************************/

/*
 * System Headers 
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Static Variables
 */
static char CMD_NAME[] = "ftp.csh";
static char GET_CMD[]  = "get ";
static char PUT_CMD[]  = "put ";
static char BLANK[]    = " ";

/*
 * Toolkit Headers 
 */
#include <PGS_SMF.h>
#include <PGS_MEM.h>
#include <PGS_IO.h>


/*****************************************************************    
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_System

SYNOPSIS:   
C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_System
        char *cmdStr);

FORTRAN:
    NONE

DESCRIPTION:
    This tool implements the POSIX system() routine. 

INPUTS:    
    Name            Description                                Units        Min        Max

    cmdStr          command string	

OUTPUTS:    
    Name            Description                                Units        Min        Max

RETURNS:
    PGS_S_SUCCESS 
    PGS_E_UNIX

EXAMPLES:

NOTES:  
    The code to implement this routine is taken largely from "Advanced programming
    in the UNIX Environment" by W. Richard Stevens on page 314. The reason of not using
    system() routine directly is that the code should conform to POSIX.1. 

REQUIREMENTS:
    PGSTK-0610

DETAILS:	    
    POSIX.2 requires that system() ignore SIGINT and SIGQUIT and block SIGCHLD.
    According to Steven's explanation, the catching of SIGCHLD should be blocked
    while system() function is executing otherwise when the child created by the 
    system() terminates, it would fool the caller of the system() into thinking
    that one of its own children terminated. system() will fork() a child
    process and when child terminates, it will generate SIGCHLD to the parent
    process.

    Also SIGINT should be blocked because when an interrupt signal is generated
    while system() is executing, the SIGINT will be send to parent and child
    processes. This signal should only be sent to the child process.

    Thus POSIX.2 states that the caller of system() should not be receiving SIGCHLD
    and SIGINT signals.   

    PGS_SMF_System() make use of C-Shell to execute the command.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_SetStaticMsg()
    PGS_SMF_SetDynamicMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_System(   /* Execute command string */
    char *cmdStr) /* Input command string */  
{
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;          /* return status */   
    pid_t            pid;                                   /* process id */
    int              status;                                /* status */
    struct sigaction ignore;                                /* ignore signal */
    struct sigaction saveintr;                              /* save signal interrupt */
    struct sigaction savequit;                              /* save signal quit */
    sigset_t         childmask;                             /* child mask */
    sigset_t         savemask;                              /* save mask */    
    char             buf[PGS_SMF_MAX_MSGBUF_SIZE];          /* max. buffer message */


    if (cmdStr != (char *)NULL)
    {
        returnStatus = PGS_E_UNIX;

        /*
         * Ignore SIGINT and SIGQUIT so that caller of PGS_SMF_System() should not be
         * receiving these signals. 
         */
        ignore.sa_handler = SIG_IGN;
        sigemptyset(&ignore.sa_mask);
        ignore.sa_flags = 0;

        if (sigaction(SIGINT,&ignore,&saveintr) == 0)
        {        
            if (sigaction(SIGQUIT,&ignore,&savequit) == 0)
            {            
                /* 
                 * Now block SIGCHLD so that parent process (which is the caller of 
                 * PGS_SMF_System()) should not be receiving this signal when child process
                 * terminates (either abnormal or normal termination).
                 */
                sigemptyset(&childmask);
                sigaddset(&childmask,SIGCHLD);

                if (sigprocmask(SIG_BLOCK,&childmask,&savemask) == 0)
                {
                    if ((pid = fork()) < 0)
                    {
                        /*
                         * Probably out of processes.
                         */
                    }
                    else if (pid == 0)
                    {
                        /*
                         * Child process.
                         */

                        /*
                         * Restore previous dispositions of the signal actions and reset signal mask. 
                         * Reason is to allow execl() to change their dispositions to the default
                         * based on the caller's dispositions. In other words, the child will inherit
                         * all the caller's signals.
                         */
                        sigaction(SIGINT,&saveintr,NULL);
                        sigaction(SIGQUIT,&savequit,NULL);
                        sigprocmask(SIG_SETMASK,&savemask,NULL);

                        /*
                         * Execute the shell command. execl() will not return on success.
                         */ 
                        execl("/bin/sh","sh","-c",cmdStr,(char *)0);                        

                        /*
                         * If execl() failed (implying that the shell can't be executed), the
                         * return value is as if the shell had executed exit(127).
                         * The call to _exit() instead of exit() is to prevent any standard
                         * I/O buffers (which would have been copied from the parent to the child
                         * across the fork()) from being flushed into the child. Note that the call
                         * to _exit(127) only happens if execl() fails.
                         */  
                        _exit(127); /* exec error */
                    }
                    else
                    {
                        /*
                         * Parent process.
                         */

                        /*
                         * waitpid() will return child pid when successful. Note that 
                         * waitpid() will wait for the specific child process to finish.
                         */
                        while (waitpid(pid,&status,0) < 0)
                        {                            
                            if (errno != EINTR)
                            {     
                                status = -1;                          
                                break;
                            }
                        }

                        if (WIFEXITED(status))
                        {
                            /*
                             * TRUE if status was returned for a child that terminated normally.
                             * In this case, call WEXITSTATUS() to fetch the low-order 8 bits
                             * of the argument that the child passed to exit() or _exit(). The
                             * status returned by WEXITSTATUS() is implementation dependent; in other
                             * words, it depends on exit() in the main() routine of the process. Usually,
                             * exit(0) means success otherwise error.
                             */
                            status = WEXITSTATUS(status);
                            if (status == 0)
                            {
                                returnStatus = PGS_S_SUCCESS;  
                            }
                        }
                        else if (WIFSIGNALED(status))
                        {
                            /*
                             * TRUE if status was returned for a child that terminated abnormally.
                             * In this case, call WTERMSIG() to fetch the signal number that 
                             * caused the termination.
                             */
                        }
                        else if (WIFSTOPPED(status))
                        {
                            /*
                             * TRUE if status was returned for a child that is currently stopped.
                             * In this case, call WWSTOPSIG() to fetch the signal number that 
                             * caused the child to stop.
                             */
                        }
                    }

                    /*
                     * Restore previous signal actions and reset signal mask for the caller.
                     */
                    if (sigaction(SIGINT,&saveintr,NULL) == 0)
                    {
                        if (sigaction(SIGQUIT,&saveintr,NULL) == 0)
                        {
                            if (sigprocmask(SIG_SETMASK,&savemask,NULL) == 0)
                            {
                                /* 
                                 * Whatever the returnStatus from the while() loop above.
                                 */
                            }
                            else
                            {
                                returnStatus = PGS_E_UNIX;  
                            }
                        }
                        else
                        {
                            returnStatus = PGS_E_UNIX;  
                        }                            
                    }
                    else
                    {
                        returnStatus = PGS_E_UNIX;  
                    }
                }
            }
        }
    }                                              

    /*
     * Set exit state.
     */
    if (returnStatus == PGS_E_UNIX)
    {
        sprintf(buf,"Error executing command: %s",cmdStr);        
        PGS_SMF_SetDynamicMsg(returnStatus,buf,"PGS_SMF_System()");
    }
    else
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_SMF_System()");    
    }

    return(returnStatus);

} /* end PGS_SMF_System */
/*****************************************************************    
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_SendFile

SYNOPSIS:   
C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_SendFile(
        char *LocalFileName,
        char *RemoteHostName,
        char *RemoteFileName)      

FORTRAN:
    NONE

DESCRIPTION:
   Equivalent to put of ftp. Can be used to transfer a file
   on the local host to a remote host.

INPUTS:    
    Name            Description                                Units        Min        Max

    LocalFileName   local file name and must contain all
                    information to locate the file on the 
                    source host (eg. full path)

    RemoteHostName  name of the remote host receiving 
                    LocalFileName file 

    RemoteFileName  LocalFileName is copied to RemoteFileName
                    at the RemoteHostName; if NULL then name
                    of the remote file will be the same as 
                    LocalFileName                                                          

OUTPUTS:    
    Name            Description                                Units        Min        Max

RETURNS:    
    PGS_S_SUCCESS  
    PGSSMF_E_CANT_OPEN_FILE       
    PGSSMF_E_SENDFILE
    PGSSMF_E_INVALID_FILE   
    PGSSMF_E_NOHOSTNAME  
    PGSSMF_E_NONETRC_FILE       
    PGSSMF_E_NETRC_MODE

EXAMPLES:

NOTES:  
    This routine is largely provided by Nitin of HAIS on August 31, 1994. This
    implementation uses the '.netrc' file in the user's home directory (on the source host).
    This file needs to be setup for file transfers. Refer to man netrc for more information.
    Each entry in the file is of form:
        machine <hostname> login <username> password <userpassword>
    For instance:
        machine adriatic login guest password anonymous

    The '.netrc' should have read permission only for the user (eg. rw-------). Otherwise
    ftp would not work.

    ftp reads this file to establish connection with the remote host. API routines uses ftp
    to complete the actual transfer. '.netrc' file must have correct information for
    the user calling the API routines for the transfer to be successful.        

REQUIREMENTS:
    PGSTK-0610

DETAILS:	    
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CheckNetrcFile() 
    PGS_MEM_Calloc()
    PGS_MEM_Free()
    PGS_SMF_System()
    PGS_SMF_GetMsgByCode()
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()    
    PGS_SMF_SetStaticMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SendFile(             /* Send file to remote host */
    char *LocalFileName,      /* Input local file name */
    char *RemoteHostName,     /* Input remote host name */
    char *RemoteFileName)     /* Input remote file name */
{
    PGSt_SMF_status  returnStatus;                          /* return status */    
    char             msg[PGS_SMF_MAX_MSG_SIZE];             /* code message */
    char             mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];   /* code mnemonic */   
    char             buf[PGS_SMF_MAX_MSGBUF_SIZE];          /* max. buffer message */
    char            *cmdbuf = (char *)NULL;                 /* command buffer */
    struct stat      statInfo;                              /* stat information */
    size_t           len;                                   /* command len */
    PGSt_SMF_code    code;                                  /* code */
    int              statVal;                               /* stat value */


    /* 
     * Check if the file exists.
     */
    if (stat(LocalFileName,&statInfo) == 0)
    {    
        /* 
         * File must be regular file or a link for ftp to work.
         */ 
        if (!S_ISREG(statInfo.st_mode) && !S_ISLNK(statInfo.st_mode))
	{
	    PGS_SMF_GetMsgByCode(PGSSMF_E_INVALID_FILE,msg);
            sprintf(buf,msg,LocalFileName);
            PGS_SMF_SetDynamicMsg(PGSSMF_E_INVALID_FILE,buf,"PGS_SMF_SendFile()");
            returnStatus = PGSSMF_E_INVALID_FILE;
	}
	else
	{                                        
            /*
             * Check for .netrc entry.
             * 1. file exist in user home directory 
             * 2. permissions must be rw only for user
             * 3. check entry for remote host name                         
             */
            returnStatus = PGS_SMF_CheckNetrcFile(RemoteHostName);
            if (returnStatus == PGS_S_SUCCESS)
            {                         
                /*
                 * Check for the existence of the ftp shell-script.
                 */
                sprintf(msg,"%s/%s",getenv("PGSBIN"),CMD_NAME);
                statVal = stat(msg,&statInfo);

                if (statVal == 0)
                {
                    /*
                     * Make sure it can be executed.
                     */
                    if ((S_IXUSR & statInfo.st_mode) && 
                        (S_IXGRP & statInfo.st_mode) && 
                        (S_IXOTH & statInfo.st_mode)) 
                    {
                        len = strlen(LocalFileName)  + 
                              strlen(RemoteHostName) + 
                              strlen(RemoteFileName) + 
                              strlen(CMD_NAME)       + 
                              strlen(GET_CMD)        +
                              strlen(PUT_CMD)        +
                              strlen(BLANK) * 4      +
                              200;

                        if (PGS_MEM_Calloc((void **)&cmdbuf,len,sizeof(char)) == PGS_S_SUCCESS)
                        {
                            /* 
                             * Create the command to execute.
                             */
                            sprintf(cmdbuf,"%s/%s ",getenv("PGSBIN"),CMD_NAME);                        
                            strcat (cmdbuf,PUT_CMD);
                            strcat (cmdbuf,RemoteHostName);
                            strcat (cmdbuf,BLANK);
                            strcat (cmdbuf,LocalFileName);
                            strcat (cmdbuf,BLANK);
                            strcat (cmdbuf,RemoteFileName);
                            strcat (cmdbuf, BLANK);        
                            strcat (cmdbuf, "> /dev/null ");

                            if (PGS_SMF_System(cmdbuf) == PGS_S_SUCCESS)
                            { 
                                PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_SMF_SendFile()");
                                returnStatus = PGS_S_SUCCESS;                                
                            }
                            else
                            {
                                /*
                                 * UNIX system error.
                                 */
                                PGS_SMF_GetMsg(&code,mnemonic,msg);            
                                PGS_SMF_SetDynamicMsg(PGSSMF_E_SENDFILE,msg,"PGS_SMF_SendFile()");
                                returnStatus = PGSSMF_E_SENDFILE;
                            }


                            /*
                             * Release resource.
                             */
                            PGS_MEM_Free(cmdbuf);
                        }
                        else
                        {   
                            /*
                             * Whatever return from PGS_MEM_Calloc().
                             */                     
                            PGS_SMF_GetMsg(&code,mnemonic,msg);            
                            PGS_SMF_SetDynamicMsg(PGSSMF_E_SENDFILE,msg,"PGS_SMF_SendFile()");
                            returnStatus = PGSSMF_E_SENDFILE;
                        }
                    }
                    else
                    { 
                        /*
                         * File ftp.csh mode be executable.
                         */
                        sprintf(buf,"File %s/%s mode must be set to execute --x--x--x)",getenv("PGSBIN"),CMD_NAME);  
                        PGS_SMF_SetDynamicMsg(PGSSMF_E_CANT_OPEN_FILE,buf,"PGS_SMF_SendFile()");
                        returnStatus = PGSSMF_E_CANT_OPEN_FILE;                    
                    }
                }
                else
                {       
                    /*
                     * Shell-script ftp.csh not exists in user's workstation.
                     */         
                    sprintf(buf,"File %s/%s not exists",getenv("PGSBIN"),CMD_NAME);  
                    PGS_SMF_SetDynamicMsg(PGSSMF_E_CANT_OPEN_FILE,buf,"PGS_SMF_SendFile()");
                    returnStatus = PGSSMF_E_CANT_OPEN_FILE;                    
                }
            }
            else
            {
                /*
                 * Whatever return from PGS_SMF_CheckNetrcFile().
                 */
                PGS_SMF_GetMsg(&code,mnemonic,msg);            
                PGS_SMF_SetDynamicMsg(code,msg,"PGS_SMF_SendFile()");
                returnStatus = code;
            }
        }
    }
    else
    { 
        /*
         * LocalFileName not exists in user's workstation.
         */
        PGS_SMF_GetMsgByCode(PGSSMF_E_CANT_OPEN_FILE,msg);
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) )
        sprintf(buf,msg,LocalFileName,strerror(errno)); 
#else
        sprintf(buf,msg,LocalFileName,sys_errlist[errno]);
#endif
        PGS_SMF_SetDynamicMsg(PGSSMF_E_CANT_OPEN_FILE,buf,"PGS_SMF_SendFile()");
        returnStatus = PGSSMF_E_CANT_OPEN_FILE;
    }        

    return(returnStatus);

} /* end PGS_SMF_SendFile */
/*****************************************************************    
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_SendMail

SYNOPSIS:   
C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_SendMail(
        char *toAddresses,
        char *ccAddresses,
        char *bcAddresses,
        char *subText,
        char *msgText);

FORTRAN:
    NONE

DESCRIPTION:
    Send mail from PGS API.

INPUTS:    
    Name            Description                                Units        Min        Max

    toAddresses     address of the recipients
    ccAddresses     who should receive this mail
    bccAddresses    blind copy; list the recipients that you do
                    not want recipients of ccAddresses to know that
                    the recipients of bccAddresses have been sent
                    mail to
    subText         subject of the mail
    msgText         message text                                  

OUTPUTS:    
    Name            Description                                Units        Min        Max

RETURNS:
    PGS_S_SUCCESS
    PGSSMF_E_NOMAIL_ADDR   
    PGSSMF_E_SENDMAIL

EXAMPLES:

NOTES:  
    NONE

REQUIREMENTS:
    PGSTK-0610

DETAILS:	    
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_IO_Gen_Open()
    PGS_IO_Gen_Close()
    PGS_PC_GetReference()
    PGS_MEM_Calloc()
    PGS_MEM_Free()
    PGS_SMF_System()
    PGS_SMF_SetStaticMsg()
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SendMail(         /* Send mail */
    char *toAddresses,    /* Input address to send to */
    char *ccAddresses,    /* Input cc the mail to */
    char *bccAddresses,   /* Input bc the mail to */
    char *subText,        /* Input subject of the text */
    char *msgText)        /* Input message text */
{
    PGSt_SMF_status          returnStatus = PGS_S_SUCCESS;                /* return status */    
    PGSt_IO_Gen_FileHandle  *fptrMail = (PGSt_IO_Gen_FileHandle *)NULL;   /* file pointer for mail */   
    char                    *cmdbuf = (char *)NULL;                       /* command buffer */
    char                     fileMail[PGSd_PC_FILE_PATH_MAX];             /* file name for mail */
    char                     msg[PGS_SMF_MAX_MSG_SIZE];                   /* code message */
    char                     mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];         /* code mnemonic */   
    char                     buf[PGS_SMF_MAX_MSGBUF_SIZE];                /* max. buffer message */
    PGSt_integer             version = 1;                                 /* version number */
    size_t                   len;                                         /* command len */    
    PGSt_SMF_code            code;                                        /* code */


    if ((strlen(toAddresses) == 0) || (strlen(subText) == 0))
    {
        returnStatus = PGSSMF_E_NOMAIL_ADDR;
    }
    else
    { 
        returnStatus = PGSSMF_E_SENDMAIL;

        /*
         * Get temporary file to write message.
         */
        if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_TMPFILE,PGSd_IO_Gen_Write,&fptrMail,1) == PGS_S_SUCCESS)        
        {

            if (PGS_PC_GetReference(PGSd_SMF_LOGICAL_TMPFILE,&version,fileMail) == PGS_S_SUCCESS)
            { 

                /*
                 * Proceed to send the mail.
                 */
                fprintf((FILE *)fptrMail, "To: %s\n",        toAddresses); 
                fprintf((FILE *)fptrMail, "Cc: %s\n",        ccAddresses); 
                fprintf((FILE *)fptrMail, "Subject: %s\n\n", subText);
                fprintf((FILE *)fptrMail, "%s\n",            msgText);
                fprintf((FILE *)fptrMail, ".\n");

                if (PGS_IO_Gen_Close(fptrMail) == PGS_S_SUCCESS)
                {    
                    fptrMail = (PGSt_IO_Gen_FileHandle *)NULL;

                    len = strlen(toAddresses)  + 
                          strlen(ccAddresses)  + 
                          strlen(bccAddresses) + 
                          strlen(fileMail)     +
                          strlen(BLANK) * 3    +
                          50;

                    if (PGS_MEM_Calloc((void **)&cmdbuf,len,sizeof(char)) == PGS_S_SUCCESS)
                    {
                        strcpy(cmdbuf, "/usr/lib/sendmail -io -t ");

                        strcat(cmdbuf, "< ");         /* file containing the message */
                        strcat(cmdbuf, fileMail);

                        strcat(cmdbuf, BLANK);         /* any error generated will be directed to /dev/null */ 
                        strcat(cmdbuf, "> /dev/null 2>&1 &");

			if (PGS_SMF_System(cmdbuf) == PGS_S_SUCCESS)
			{
			    returnStatus = PGS_S_SUCCESS;
			}
                    }                    
                }                
            }    
        }                     
    }

    if (fptrMail != (PGSt_IO_Gen_FileHandle *)NULL) 
    {
        PGS_IO_Gen_Close(fptrMail);
    }

    PGS_MEM_Free(cmdbuf);

    /*
     * Set exit state.
     */
    if (returnStatus == PGSSMF_E_SENDMAIL)
    {
        /*
         * Whatever return from Toolkit.
         */      
        PGS_SMF_GetMsg(&code,mnemonic,msg);        
        sprintf(buf,"Could not send mail to %s. Error could be: %s",toAddresses,msg);
        PGS_SMF_SetDynamicMsg(PGSSMF_E_SENDMAIL,buf,"PGS_SMF_SendMail()");       
    }  
    else
    {         
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_SMF_SendMail()");
    }

    return(returnStatus);

} /* end PGS_SMF_SendMail */    
/*****************************************************************    
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_CheckNetrcFile
        char *RemoteHostName);

SYNOPSIS:   
C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_CheckNetrcFile(
        char *RemoteHostName);

FORTRAN:
    NONE

DESCRIPTION:
    Check if '.netrc' exists in the user home directory. Also check if
    RemoteHostName is in .netrc file.

INPUTS:    
    Name            Description                                Units        Min        Max 

    RemoteHostName  remote host name                                                        

OUTPUTS:    
    Name            Description                                Units        Min        Max

RETURNS:
    PGS_S_SUCCESS
    PGSSMF_E_NOHOSTNAME  
    PGSSMF_E_NONETRC_FILE       
    PGSSMF_E_NETRC_MODE

EXAMPLES:

NOTES:  
    Refer to PGS_SMF_SendFile().

REQUIREMENTS:
    PGSTK-0610

DETAILS:	    
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_SetStaticMsg()
    PGS_SMF_GetMsgByCode()
    PGS_SMF_SetDynamicMsg()      

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_CheckNetrcFile(       /* Check .netrc. file */
    char *RemoteHostName)     /* Input remote host name */
{   
    PGSt_SMF_status   returnStatus = PGSSMF_E_NOHOSTNAME;  /* return status */ 
    char             *token;                               /* string token */
    char              line[200];                           /* line read from .netrc */
    char              filename[100];                       /* path to .netrc */    
    char              msg[PGS_SMF_MAX_MSG_SIZE];           /* code message */    
    char              buf[PGS_SMF_MAX_MSGBUF_SIZE];        /* max. buffer message */
    FILE             *fptr = (FILE *)NULL;                 /* file pointer for .netrc */
    struct stat       statInfo;                            /* file .netrc information */


    /*
     * Make sure that there is no blank/tab space in front/end of RemoteHostName.
     */
    PGS_SMF_RemoveSpace(RemoteHostName);

    sprintf(filename,"%s/.netrc",getenv("HOME"));

    fptr = fopen(filename,"r");
    if (fptr == (FILE *)NULL)
    {
        /*
         * File .netrc not exist in user's home directory.
         */
        returnStatus = PGSSMF_E_NONETRC_FILE;   
    }
    else
    {
        /*
         * Make sure .netrc file is rw------- for user only.
         */
        stat(filename,&statInfo);
        if ( (S_IRUSR & statInfo.st_mode) &&  (S_IWUSR & statInfo.st_mode) && !(S_IXUSR & statInfo.st_mode) &&  /* user:  rw- */
            !(S_IRGRP & statInfo.st_mode) && !(S_IWGRP & statInfo.st_mode) && !(S_IXGRP & statInfo.st_mode) &&  /* group: --- */   
            !(S_IROTH & statInfo.st_mode) && !(S_IWOTH & statInfo.st_mode) && !(S_IXOTH & statInfo.st_mode))    /* other: --- */
        {

            /*
             * Scan for RemoteHostName in .netrc file.
             */
            while (fgets(line,200,fptr) != (char *)NULL)
            {
#ifdef _PGS_THREADSAFE
                char *lasts;   /* used by strtok_r() */

                /*
                 * strtok() is not threadsafe so we use strtok_r() in 
                 * threadsafe mode 
                 */
                token = strtok_r(line," \t",&lasts);        /* machine */
                token = strtok_r(NULL," \t",&lasts);        /* machine's name */
#else
                token = strtok(line," \t");                 /* machine */
                token = strtok(NULL," \t");                 /* machine's name */
#endif

                if (strcmp(token,RemoteHostName) == 0)
                {
                    returnStatus = PGS_S_SUCCESS;
                    break;
                }
            }
        }
        else
        {
            /*
             * File .netrc must be mode rw-------.
             */
            returnStatus = PGSSMF_E_NETRC_MODE;
        }

        fclose(fptr);
    }

    /*
     * Set exit state.
     */
    if (returnStatus == PGSSMF_E_NONETRC_FILE)
    {
        PGS_SMF_GetMsgByCode(PGSSMF_E_NONETRC_FILE,msg);
        sprintf(buf,msg,filename);
        PGS_SMF_SetDynamicMsg(PGSSMF_E_NONETRC_FILE,buf,"PGS_SMF_CheckNetrcFile()");
    }
    else if (returnStatus == PGSSMF_E_NOHOSTNAME)
    {
        PGS_SMF_GetMsgByCode(PGSSMF_E_NOHOSTNAME,msg);
        sprintf(buf,"%s : %s",msg,filename);
        PGS_SMF_SetDynamicMsg(PGSSMF_E_NOHOSTNAME,buf,"PGS_SMF_CheckNetrcFile()");
    }    
    else
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_SMF_CheckNetrcFile()");
    }

    return(returnStatus);

} /* end PGS_SMF_CheckNetrcFile */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_RemoveSpace

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void 
    PGS_SMF_RemoveSpace(
        char *str);

FORTRAN:
    NONE

DESCRIPTION:
    To remove all the front and end character space of a line string.

INPUTS:
    Name            Description                                Units        Min        Max   

    str             line string with front/end space

OUTPUTS:
    Name            Description                                Units        Min        Max   

    str             line string without front/end space

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

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
PGS_SMF_RemoveSpace(  /* Remove front and back space */
    char *str)    /* Input/Output line string */
{
    short i;  /* counter */
    short j;  /* counter */
    short l;  /* counter */


    if (strlen(str) != 0)
    {
        if (str[strlen(str)-1] == '\n')
        {
            str[strlen(str)-1] = '\0';
        }

        /*
         * Remove front space.
         */
        for (i=0 ;  i < (short)strlen(str) ; i++)
        {
            if (str[i] != ' ' || str[i] != '\t')
            {
                break;
            }
        }

        /*
         * Remove back space.
         */
        for (j=strlen(str)-1 ;  j > i ; j--)
        {
            if (str[j] != ' ' || str[j] != '\t')
            {
                break;
            }
        }

        if (i == strlen(str))
        {
            /*
             * All space characters.
             */
            str[0] = '\0';
        }
        else if (i == j)
        {
            /*
             * Only one character in str.
             */
            str[0] = str[i];
            str[1] = '\0';
        }
        else 
        { 
            /*
             * Characters within str.
             */
            for (l=0 ; l <= (j-i) ; l++)
            {
                str[l] = str[l+i];
            }

            str[l] = '\0';
        }
    }


} /* end PGS_SMF_RemoveSpace */




