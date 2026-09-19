/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_SMF_SendRuntimeData.c

DESCRIPTION:	
  The procedure defined in this file will provide the means to 
  transfer a collection of runtime files to a pre-specified host. 
  As a side-effect, an electronic message will be transmitted, 
  announcing the availability of the runtime files to the interested
  parties. 

  This file contains the following routines:
    PGS_SMF_SendRuntimeData()
    PGS_SMF_SysSendRuntimeData()

AUTHOR:
  Kelvin K. Wan / Applied Research Corp.

HISTORY:
  15-Aug-1994 KKW Standard Convention 
  28-Dec-1994 KKW Enhanced for TK4 Enhance PGS_SMF_SendRuntimeData() 
                  to support PC with file version
  21-Apr-1995 MES Enhanced for TK5: added call to PGS_PC_MultiRuntimes
                  Corrected data type for parameter 'files'.  It was
                  of type PGSt_integer.  Changed it to PGSt_PC_Logical.
  12-Jul-1995 MES Update FORTRAN EXAMPLES in prologs.
  13-Oct-1995 DPH Corrected oversight which prevented transmission of 
		  files from begin disabled by the PCF switch in 
		  function PGS_SMF_SysTermSendRuntimeData().
		  Note that errors from the calls made in this
		  module are not recorded in the log since the log
		  files have been closed when this function is called.
  05-Mar-2001 AT  modified for linux

END_FILE_PROLOG:
*****************************************************************/

/*
 * System Headers 
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Toolkit Headers 
 */
#include <PGS_SMF.h>
#include <PGS_IO_Gen.h>
#include <PGS_PC.h>
#include <PGS_MEM.h>

/*****************************************************************   
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SendRuntimeData

SYNOPSIS:   

C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_SendRuntimeData(
        PGSt_integer numfiles,
        PGSt_PC_Logical files[],
        PGSt_integer version[]);

FORTRAN:
    include 'PGS_SMF.f'      

    integer function pgs_smf_sendruntimedata(numfiles,files,version)
    integer numfiles
    integer files(*)
    integer version(*)

DESCRIPTION:
    This tool provides the user with a method for flagging specific runtime
    data files for subsequent post-processing retrieval.

INPUTS:
    Name        Description Units                      Min        Max

    numfiles    exact number of runtime logical       
                file identifiers loaded into the 
                array 'files'

    files       array of logical file identifiers      	
                which are to be preserved for later 
                retrieval
    version     an associated array for identifying 
                specific versions of the files
                identified in the preceeding array of
                logical identifiers


OUTPUTS:    
    Name        Description Units                      Min        Max

RETURNS:    
    PGS_S_SUCCESS     
    PGSSMF_E_SENDRUNTIME_DATA   
    PGSSMF_M_TRANSMIT_DISABLE

EXAMPLES:

    C:
    ==
    /# These constants may be defined in the users include file(s). #/
    /# Note that these logical file identifiers would have to appear #/
    /# in the Process Control file in order for this call to work. #/

    #define MODIS1A    10
    #define MODIS2     20
    #define TEMP1      50
    #define TEMP2      51
    #define TEMP3      52

    PGSt_SMF_status returnStatus;
    PGSt_integer    numberOfFiles;
    PGSt_integer    logIdArray[6];
    PGSt_integer    version[6];
    PGSt_integer    version_MODIS1A_1 = 1;
    PGSt_integer    version_MODIS1A_2 = 2;
    PGSt_integer    version_MODIS2    = 1;
    PGSt_integer    version_TEMP      = 1;

    logIdArray[0] = MODIS1A;  version[0] = version_MODIS1A_1;
    logIdArray[1] = MODIS1A;  version[1] = version_MODIS1A_2;
    logIdArray[2] = MODIS2;   version[2] = version_MODIS2;
    logIdArray[3] = TEMP1;    version[3] = version_TEMP;
    logIdArray[4] = TEMP2;    version[4] = version_TEMP;
    logIdArray[5] = TEMP3;    version[5] = version_TEMP;
    numberOfFiles = 6;

    returnStatus = PGS_SMF_SendRuntimeData(numberOfFiles,logIdArray,version);
    if (returnStatus == PGS_S_SUCCESS)
    {
        /# send runtime data success #/
    }

    FORTRAN:
    ========
C   The following constants may be defined in the users include file(s).
C   Note that the specific logical file identifiers would have to appear 
C   in the process control file in order for this call to work.

    implicit none
    integer pgs_smf_sendruntimedata

    integer modis1a
    parameter (modis1a = 10)
    integer modis2
    parameter (modis2 = 20)
    integer temp1
    parameter (temp1 = 50)
    integer temp2
    parameter (temp2 = 51)
    integer temp3
    parameter (temp2 = 52)

    integer    returnStatus
    integer    numberOfFiles
    integer    logIdArray(6)
    integer    version(6)
    integer    version_modis1a_1
    integer    version_modis1a_2
    integer    version_modis2
    integer    version_temp 

    version_modisa_1 = 1
    version_modisa_2 = 2
    version_modis2   = 1
    version_temp     = 1

    logIdArray(1) = modis1a
    version(1)    = version_modis1a_1

    logIdArray(2) = modis1a
    version(2)    = version_modis1a_2

    logIdArray(3) = modis2
    version(3)    = version_modis2

    logIdArray(4) = temp1
    version(4)    = version_temp

    logIdArray(5) = temp2
    version(5)    = version_temp

    logIdArray(6) = temp3
    version(6)    = version_temp

    numberOfFiles = 6

    return_status = pgs_smf_sendruntimedata(numberOfFiles,logIdArray,version)
    if (return_status .EQ. PGS_S_SUCCESS) then
C       send runtime data success
    endif

NOTES:  
    Repeated calls to this tool will cause previously requested files 
    to be superseded with the list provided during the last call.

    IMPORTANT TK4 NOTES
    -------------------
    As of TK4, this tool no longer triggers the spontaneous transmission 
    of runtime files and e-mail notification, as it did in TK3. Rather, 
    the requested files are saved/marked for transmission following the 
    normal termination of the PGE process. The actual transmission procedure 
    is performed by the termination process (See PGS_PC_TermCom() for more 
    information on the steps required to perform this transmission).

    Please refer to the documentation for PGS_PC_TermCom() for directions 
    on how to activate/deactivate the Toolkit's transmission capability.   

REQUIREMENTS:
    PGSTK-0630

DETAILS:
    For TK4, the interface to this tool was updated to support the multiple 
    versioning of Product Input files.  An additional array was added to the 
    argument list to provide for this capability.    

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_SMF_GetGlobalVar()   
    PGS_SMF_GetMsg()
    PGS_SMF_SetStaticMsg()
    PGS_SMF_SetDynamicMsg()
    PGS_PC_GetConfigData()
    PGS_PC_GetReference()
    PGS_PC_GetReference()
    PGS_PC_MultiRuntimes()
    PGS_IO_Gen_Open()
    PGS_IO_Gen_Temp_Reference()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SendRuntimeData(      /* Send runtime data */
    PGSt_integer numfiles,    /* Input number of files */
    PGSt_PC_Logical files[],  /* Input file logical ID */
    PGSt_integer version[])   /* Input file version */
{
    PGSt_SMF_status         returnStatus = PGS_S_SUCCESS;                  /* return status */
    char                    transmit[PGSd_PC_VALUE_LENGTH_MAX];            /* transmission flag */
    char                    msg[PGS_SMF_MAX_MSG_SIZE];                     /* code message */
    char                    mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];           /* code mnemonic */   
    char                    file[PGSd_PC_VALUE_LENGTH_MAX];                /* file to send */  
    char                    fileSent = '0';                                /* file to indicate file not send */
    char                    reference[36];                                 /* for IO */
    PGSt_SMF_code           code;                                          /* code */
    PGSSmfGlbVar           *global_var;                                    /* global static buffer */ 
    int                     i;                                             /* counter */
    PGSt_boolean            exit_flag;                                     /* for IO */
    PGSt_IO_Gen_FileHandle *fptrShmMem = (PGSt_IO_Gen_FileHandle *)NULL;   /* SMF shared memory file */ 
    PGSt_integer            ver;                                           /* version number */
    PGSt_SMF_boolean        writeToFile = PGS_FALSE;                       /* flag */



    PGS_SMF_GetGlobalVar(&global_var);

    PGS_PC_MultiRuntimes(files, version, numfiles);

    /*
     * Get permission to transfer the runtime files.
     */
    if (PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_TRANSMIT,transmit) == PGS_S_SUCCESS)
    {
         if (strcmp(transmit,PGSd_SMF_ON) == 0)
         {
             /*
              * Flag to indicate that we are allow to transmit the files to remote host.
              */
             global_var->rundata.transmit = PGS_TRUE;

             /*
              * Write the runtime data into the artificial SMF shared memory. Old data
              * will be over-written.
              */                       
             if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_SHMMEM,PGSd_IO_Gen_Write,&fptrShmMem,1) == PGS_S_SUCCESS)
             {          
                 fprintf(fptrShmMem,"%d\n",PGS_SMF_Get_WriteLogFile());
                 fprintf(fptrShmMem,"%s\n",global_var->log.msgTag);
                 fprintf(fptrShmMem,"%s\n",global_var->log.logStatus);
                 fprintf(fptrShmMem,"%s\n",global_var->log.logReport);
                 fprintf(fptrShmMem,"%s\n",global_var->log.logUser);

                 for (i=0 ; i < numfiles ; i++)
                 {
                     /*
                      * Need to call Dave Heroux's function to verify temporary file; eg. PGS_PC_GetTempRef()
                      */
                     writeToFile = PGS_TRUE;
                     ver = version[i];
                     if (PGS_PC_GetReference(files[i],&ver,file) != PGS_S_SUCCESS)
                     {
                         if (PGS_IO_Gen_Temp_Reference(PGSd_IO_Gen_NoEndurance,
                                                       files[i],PGSd_IO_Gen_Read,reference,&exit_flag) != PGS_S_SUCCESS)
                         {                             
                             writeToFile = PGS_FALSE;
                         }
                     }

                     if (writeToFile == PGS_TRUE)
                     {                     
                         /*
                          * Format: file_path_name logicalID version fileSent
                          */
                         fprintf(fptrShmMem,"%s %d %d %c\n",file,(int)files[i],(int)version[i],fileSent);
                     }
                 }

                 fclose((FILE *)fptrShmMem);
             } 
             else
             {
                 returnStatus = PGSSMF_E_SENDRUNTIME_DATA;
             }                                     
         }
         else
         {
             returnStatus = PGSSMF_M_TRANSMIT_DISABLE;
         }
    }

    /*
     * Set exit state.
     */
    if (returnStatus == PGSSMF_E_SENDRUNTIME_DATA)
    { 
        PGS_SMF_GetMsg(&code,mnemonic,msg);
        PGS_SMF_SetDynamicMsg(PGSSMF_E_SENDRUNTIME_DATA,msg,"PGS_SMF_SendRuntimeData()");
    }
    else
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_SMF_SendRuntimeData()");
    }    

    return(returnStatus);

} /* end PGS_SMF_SendRuntimeData */

/*****************************************************************  
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SysSendRuntimeData

SYNOPSIS:   

C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_SysSendRuntimeData(
        void);

FORTRAN:
    NONE

DESCRIPTION:    
    This tool provides the means to transfer user-requested runtime data 
    files to a temporary holding area for later retrieval by the user. 
    This tool also transmits an electronic message to notify the user as 
    to the availability of the requested data. 

INPUTS:    
    Name        Description Units                      Min        Max       

OUTPUTS:    
    Name        Description Units                      Min        Max

RETURNS:
    PGS_S_SUCCESS
    PGSSMF_E_SENDRUNTIME_DATA 
    PGSSMF_E_NOHOSTNAME   
    PGSSMF_E_NOMAIL_ADDR         
    PGSSMF_E_REMOTEPATH    

EXAMPLES:	
    PGSt_SMF_status returnStatus;

    returnStatus = PGS_SMF_SysSendRuntimeData();
    if (returnStatus == PGS_S_SUCCESS)
    {       
        /# send runtime files success #/
    }

NOTES:  
    This routine is called in the termination process. Therefore this 
    tool is only to be used by ARC developers.

REQUIREMENTS:    
    PGSTK-0630

DETAILS:	        
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_SMF_GetGlobalVar() 
    PGS_IO_Gen_Open()
    PGS_PC_GetConfigData()
    PGS_SMF_SendFile()
    PGS_MEM_Calloc()
    PGS_MEM_Realloc()
    PGS_MEM_Free()
    PGS_SMF_SendMail()
    PGS_SMF_SetStaticMsg()
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SysSendRuntimeData(    /* Send file to remote host */ 
    void)
{
    PGSt_SMF_status          returnStatus = PGS_S_SUCCESS;                  /* return status */
    char                    *envPtr = (char *)NULL;                         /* env. pointer */
    char                    *token;                                         /* string token */
    char                     remoteHostName[PGSd_PC_VALUE_LENGTH_MAX];      /* remote host name */
    char                     remotePath[PGSd_PC_VALUE_LENGTH_MAX];          /* remote path */
    char                     remoteFileName[PGSd_PC_VALUE_LENGTH_MAX];      /* remote file name */  
    char                     mailUser[PGSd_PC_VALUE_LENGTH_MAX];            /* user's mailing address */
    char                     file[PGSd_PC_VALUE_LENGTH_MAX];                /* file to send */    
    char                     fileSent;                                      /* file sent flag */
    char                    *msgBuf = (char *)NULL;                         /* mail message */
    char                     msg[PGS_SMF_MAX_MSG_SIZE];                     /* code message */
    char                     buf[PGS_SMF_MAX_MSGBUF_SIZE];                  /* max. buffer message */
    char                     mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];           /* code mnemonic */   
    char                     subText[100];                                  /* subject text for the mail message */
    PGSt_integer             version;                                       /* version number */    
    PGSt_integer             logicalID;                                     /* logical ID */
    PGSt_SMF_code            code;                                          /* code */
    PGSSmfGlbVar            *global_var;                                    /* global static buffer */ 
    int                      i;                                             /* counter */  
    int                      j;                                             /* counter */ 
    int                      numFileSent = 0;                               /* number of files sent to remote host */
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
    long                   startPos;              /* file position */
    long                   currPos;               /* current file position */
#else
    fpos_t                 startPos;              /* file position */
    fpos_t                 currPos;               /* current file position */
#endif
    PGSt_IO_Gen_FileHandle  *fptrShmMem = (PGSt_IO_Gen_FileHandle *)NULL;   /* SMF shared memory file */ 

    PGS_SMF_GetGlobalVar(&global_var);

    /*
     * Get remote host name.
     */
    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_REMOTEHOSTNAME,remoteHostName);

    PGS_SMF_RemoveSpace(remoteHostName);

    if ((returnStatus == PGS_S_SUCCESS) && (remoteHostName[0] != '\0'))
    {
        /*
         * Get user address to send the file to.
         */
        returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_MAILUSER,mailUser);

        PGS_SMF_RemoveSpace(mailUser);        

        if ((returnStatus == PGS_S_SUCCESS) && (mailUser[0] != '\0'))
        {         
            /*
             * Get remote path to send the file to.
             */
            returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_REMOTEPATH,remotePath);

            PGS_SMF_RemoveSpace(remotePath);    

            if ((returnStatus == PGS_S_SUCCESS) && (remotePath[0] != '\0'))
            {                       
                if (PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_SHMMEM,PGSd_IO_Gen_Update,&fptrShmMem,1) == PGS_S_SUCCESS)
                {
                    /*
                     * Read msgTag and log files.
                     */
                    for (i=0 ; i < 5 ; i++)
                    {   
                        if (fgets(file,sizeof(file),fptrShmMem) == (char *)NULL)
                        {
                            returnStatus = PGSSMF_E_SENDRUNTIME_DATA;
                            strcpy(buf,"Error reading data from the ascii file (shared memory)"); 
                            goto DONE;
                        }
                    }                    

                    /*
                     * Save current file position so that we could reread the file
                     * at this position.
                     */
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
                    fgetpos(fptrShmMem,(fpos_t *)(&startPos));
#else
                    fgetpos(fptrShmMem,&startPos);
#endif
                    currPos = startPos;

                    /*
                     * Read the runtime files.
                     */                      
                    while (fgets(msg,sizeof(msg),fptrShmMem) != (char *)NULL)
                    {                                               

                        sscanf(msg,"%s %d %d %c",file,&logicalID,&version,&fileSent);

                        /*
                         * Send file to host.
                         */          
                        token = strrchr(file,'/');
                        if (token == (char *)NULL)
                        {
                            sprintf(remoteFileName,"%s/%s",remotePath,file);
                        }
                        else
                        {
                            sprintf(remoteFileName,"%s%s",remotePath,token);
                        }

                        if (PGS_SMF_SendFile(file,remoteHostName,remoteFileName) == PGS_S_SUCCESS)
                        {
                            currPos += (strlen(msg) - 2);     

                            numFileSent++;
                            fileSent = '1';

                            /*
                             * Mark that this file has been sent.
                             */
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
                            fsetpos(fptrShmMem,(fpos_t *)(&currPos));
                            fprintf(fptrShmMem,"%c",fileSent);

                            currPos += 2; /* advance to next line */
                            fsetpos(fptrShmMem,(fpos_t *)(&currPos));
#else
                            fsetpos(fptrShmMem,&currPos);                            
                            fprintf(fptrShmMem,"%c",fileSent);

                            currPos += 2; /* advance to next line */
                            fsetpos(fptrShmMem,&currPos);
#endif 
                        }  
                    }
                }
            }
            else
            {
                /*
                 * Whatever return from PGS_PC_GetConfigData().
                 */     
                PGS_SMF_GetMsg(&code,mnemonic,buf);             
                returnStatus = PGSSMF_E_REMOTEPATH; 
                goto DONE;           
            }             
        }
        else
        {
            /*
             * Whatever return from PGS_PC_GetConfigData().
             */     
            PGS_SMF_GetMsg(&code,mnemonic,buf);      
            returnStatus = PGSSMF_E_NOMAIL_ADDR;
            goto DONE;
        }       
    }
    else
    {     
        /*
         * Whatever return from PGS_PC_GetConfigData().
         */        
        PGS_SMF_GetMsg(&code,mnemonic,buf);
        returnStatus = PGSSMF_E_NOHOSTNAME;
        goto DONE;
    }


    if (numFileSent == 0)
    { 
        if (fptrShmMem == (PGSt_IO_Gen_FileHandle *)NULL)
        {
            /*
             * (1) PGE process did not call PGS_SMF_SendRuntimeData() or
             * (2) PGE process did call PGS_SMF_SendRuntimeData() but 
             *     PGS_IO_Gen_Open(PGSd_SMF_LOGICAL_SHMMEM) detected error
             */
            strcpy(buf,"No runtime file to send.");
        }
        else        
        {   
            /*
             * (1) PGS_SMF_SendFile() could not send the files
             */
            strcpy(buf,"Error sending all the runtime files.");
        }

        returnStatus = PGSSMF_E_SENDRUNTIME_DATA;
        goto DONE;
    }


    /*
     * Alert remote user which files are already sent to destination and which are not. 
     */
    i = 1;
    j = 1;        
    returnStatus = PGS_S_SUCCESS;

    if (PGS_MEM_Calloc((void **)&msgBuf,PGSd_SMF_MAILMSG_LEN,sizeof(char)) == PGS_S_SUCCESS)
    {                
        sprintf(msgBuf,"The following runtime files are located at %s:~%s>\n",remoteHostName,remotePath);

        /*
         * Reset to where the file-data is located at in the ascii file (after the header).
         */
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
        fsetpos(fptrShmMem,(fpos_t *)(&startPos));
#else
        fsetpos(fptrShmMem,&startPos);
#endif

        while (fgets(msg,sizeof(msg),fptrShmMem) != (char *)NULL)
        { 

            sscanf(msg,"%s %d %d %c",file,&logicalID,&version,&fileSent);

            if ((j*PGSd_SMF_MAILMSG_LEN - (int)strlen(msgBuf)) <= ((int)strlen(file) + 20))
            {            
                j++;
                if (PGS_MEM_Realloc((void **)&msgBuf,sizeof(char) * j * PGSd_SMF_MAILMSG_LEN) != PGS_S_SUCCESS)
                {
                    /*
                     * Whatever return from PGS_MEM_Realloc().
                     */     
                    PGS_SMF_GetMsg(&code,mnemonic,buf);
                    returnStatus = PGSSMF_E_SENDRUNTIME_DATA;
                    break;
                }
            }

            /*
             * Create appropriate mail message to accomodate the file name.
             */                   
            sprintf(msg,"%d) ",i);                      
            strcat(msgBuf,msg);    

            if (token == (char *)NULL)
            {
                strcat(msgBuf,file);
            }
            else
            {                            
                strcat(msgBuf,token+1);                          
            }


            if (fileSent == '1')
            {                                                                                                                                                                     
                strcat(msgBuf," <file sent>");
            }
            else
            {
                strcat(msgBuf," <file not sent>");
            }

            strcat(msgBuf,"\n");
            msgBuf[strlen(msgBuf)] = '\0';
            i++;
        }            
    }
    else
    {
        /*
         * Whatever return from PGS_MEM_Calloc().
         */     
        PGS_SMF_GetMsg(&code,mnemonic,buf);
        returnStatus = PGSSMF_E_SENDRUNTIME_DATA;
    }    


    /*
     * Send e-mail to user of which files have been sent or not send.
     * Also send process step control file.
     */
    if (returnStatus == PGS_S_SUCCESS)
    {
        /*
         * Make sure that the message string is allocated enough space to 
         * accomodate process step control file. Note that adding 100 is for 
         * the message "<process step control file sent>".
         */     
        envPtr = (char *)getenv("PGS_PC_INFO_FILE");  
        if (envPtr == (char *)NULL)
        {
            strcpy(buf,"PGS_PC_INFO_FILE environment variable not set");
            returnStatus = PGSSMF_E_SENDRUNTIME_DATA;
        }
        else
        {           
            sprintf(file,"%s",envPtr);
            if ((j*PGSd_SMF_MAILMSG_LEN - (int)strlen(msgBuf)) <= (int)strlen(file))
            {
                j++;
                if (PGS_MEM_Realloc((void **)&msgBuf,sizeof(char) * j * PGSd_SMF_MAILMSG_LEN) != PGS_S_SUCCESS)
                {
                    /*
                     * Whatever return from PGS_MEM_Realloc().
                     */     
                    PGS_SMF_GetMsg(&code,mnemonic,buf);
                    returnStatus = PGSSMF_E_SENDRUNTIME_DATA;
                    goto DONE;
                }
            }

            /* 
             * Send process step control file to remote host.
             */
            sprintf(msg,"%d) ",i); 
            strcat(msgBuf,msg);            
            token = strrchr(file,'/');
            if (token == (char *)NULL)
            {            
                sprintf(remoteFileName,"%s/%s",remotePath,file); 
                strcat(msgBuf,file);
            }
            else
            {
                sprintf(remoteFileName,"%s%s",remotePath,token);
                strcat(msgBuf,token+1);   
            }          

            if (PGS_SMF_SendFile(file,remoteHostName,remoteFileName) == PGS_S_SUCCESS)
            {                                                                                                                                                                    
                strcat(msgBuf," <process step control file sent>");
            }
            else
            {
                strcat(msgBuf," <process step control file not sent>");
            }                        

            strcat(msgBuf,"\n");
            msgBuf[strlen(msgBuf)] = '\0';

            sprintf(subText,"PGE %s runtime file status",global_var->log.msgTag);
            if (PGS_SMF_SendMail(mailUser,"","",subText,msgBuf) != PGS_S_SUCCESS)
            {           
                PGS_SMF_GetMsg(&code,mnemonic,buf);
                returnStatus = PGSSMF_E_SENDRUNTIME_DATA;
            }            
        }
    }                

DONE: 

    if (fptrShmMem != (PGSt_IO_Gen_FileHandle *)NULL)
    {
        fclose((FILE *)fptrShmMem);    
    }

    if (msgBuf != (char *)NULL)
    {
        PGS_MEM_Free(msgBuf);    
    }                                 

    /*
     * Set exit state.
     * Actually, since this is the Termination Process, these calls
     * would set a return status of PGSSMF_E_LOGFILE.
     */
    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_SMF_SysSendRuntimeData()");
    }
    else
    {
        PGS_SMF_SetDynamicMsg(returnStatus,buf,"PGS_SMF_SysSendRuntimeData()");
    }

    return(returnStatus);

} /* end PGS_SMF_SysSendRuntimeData */
/*****************************************************************   
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_SysTermSendRuntimeData

SYNOPSIS:   

C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_status
    PGS_SMF_SysTermSendRuntimeData(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    Send runtime data to remote host during termination process.

INPUTS:
    Name        Description Units                      Min        Max    

OUTPUTS:    
    Name        Description Units                      Min        Max

RETURNS:    
    PGS_S_SUCCESS     
    PGS_SH_SMF_SENDRUNTIME	Problem transmitting runtime files, OR
			        problem with transmission switch in PCF.

EXAMPLES:    
    Refer to PGS_SMF_TermProc();

NOTES:  
    NONE.

REQUIREMENTS:
    PGSTK-0630

DETAILS:	    
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_SMF_SysSendRuntimeData()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_SMF_SysTermSendRuntimeData(
    void)
{
    char    transmit[PGSd_PC_VALUE_LENGTH_MAX];      /* transmission flag */
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */

    /*
     * Get permission to transfer the runtime files.
     * since caller is SMF, errors are not reported to log files anyway!
     */
    returnStatus = PGS_PC_GetConfigData(PGSd_SMF_LOGICAL_TRANSMIT,transmit);
    if ((returnStatus == PGS_S_SUCCESS) && (strcmp(transmit,PGSd_SMF_ON) == 0))
    {
        if ((returnStatus = PGS_SMF_SysSendRuntimeData()) != PGS_S_SUCCESS)
        {
            returnStatus = PGS_SH_SMF_SENDRUNTIME;
        }   
    }
    else
    {
        returnStatus = PGS_SH_SMF_SENDRUNTIME; 
    }

    return(returnStatus);

} /* end PGS_SMF_SysTermSendRuntimeData */
