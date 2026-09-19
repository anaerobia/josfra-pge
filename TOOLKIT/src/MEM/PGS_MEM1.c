/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_MEM1.c

DESCRIPTION:
  This file contains shared memory routines. The routines in this file DO NOT conform 
  to POSIX. The routines are:
    PGS_MEM_ShmCreate()    
    PGS_MEM_ShmAttach()  
    PGS_MEM_ShmDetach() 
    PGS_MEM_Shmget()
    PGS_MEM_Shmat()   
    PGS_MEM_Shmdt()
    PGS_MEM_Shmctl()
    PGS_MEM_ShmSysInit()
    PGS_MEM_ShmSysTerm()
    PGS_MEM_ShmSysAddr()

  Planning & Processing Subsystem (PPS) has no plans for limiting memory usage through
  these toolkits (7/12/94). Thus no memory boundary checking is performed.

  In order to use shared memory, make sure that you add -DSHMMEM in MEM Makefile. Recompile
  PGS_MEM1.c. If -DSHMMEM is not set (which means don't use shared memory) and if user calls 
  shared MEM tool, the following error status will be returned automatically:

  (1) PGS_MEM_ShmCreate()  return PGS_E_UNIX   
  (2) PGS_MEM_ShmAttach()  return PGS_E_UNIX  
  (3) PGS_MEM_ShmDetach()  return PGS_E_UNIX
  (4) PGS_MEM_ShmSysInit() return PGS_SH_MEM_INIT
  (5) PGS_MEM_ShmSysTerm() return PGS_SH_MEM_TERM
  (6) PGS_MEM_ShmSysAddr() return PGS_E_UNIX

AUTHOR:
  Kelvin K. Wan / Applied Research Corp.
  Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
  25-Mar-1994 KCW Standard Convention  
   1-Jul-1994 KCW Revised routines to accomodate with new design.
  11-Aug-1994 KCW Revised to accomodate code inspection comment.   
  19-Aug-1994 KCW Revised to include status log files.
  22-Dec-1994 KCW Revised to include checking of SMF initialization
   9-Dec-1994 KCW Revised to add -DSHMMEM flag to turn ON/OFF using shared memory
  09-Aug-1996 GSK Changed shared memory addressing scheme
  07-Jul-1999 RM  Updated for TSF functionality

END_FILE_PROLOG:
*****************************************************************/

/* 
 * System Headers 
 */
#include <stdio.h>
#include <unistd.h>

/*
 * Toolkit Headers 
 */
#include <PGS_MEM1.h> 
#include <PGS_TSF.h> 

/*
 * Static Global Variables 
 */
static PGSt_SMF_boolean     glb_usrAttached = PGS_FALSE;           
static unsigned long        glb_usrAddr     = 0L;
static PGSt_SMF_boolean     glb_sysAttached = PGS_FALSE;
static PGSMemShm           *glb_sysData     = (PGSMemShm *)NULL;

/*
 * Static Functions 
 */
static PGSt_SMF_status PGS_MEM_ShmGlbVarUsr(PGSMemShm **global_var);

/*
 * Check if -DSHMMEM flag is ON/OFF.
 */
#ifdef SHMMEM

/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_ShmCreate

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>                                                                 

    PGSt_SMF_status 
    PGS_MEM_ShmCreate(
        PGSt_uinteger size);

FORTRAN:
    NONE

DESCRIPTION:
    This tool may be used to create a shared memory segment. This
    tool should be called once in a given processing script (PGE).

INPUTS:
    size - size of the shared memory segment in bytes

OUTPUTS:  
    NONE      

RETURNS:                                
    PGS_S_SUCCESS
    PGS_E_UNIX
    PGSMEM_E_SHM_ENV    
    PGSMEM_E_SHM_MAXSIZE
    PGSMEM_E_SHM_MULTICREATE
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:
    typedef struct
    {
        int  id;
        char msg[100];
    }TestStruct;

    PGSt_SMF_status  returnStatus;
    TestStruct      *shmPtr;
    PGSt_SMF_status  returnStatus;

    returnStatus = PGS_MEM_ShmCreate(sizeof(TestStruct); 
    if (returnStatus == PGS_S_SUCCESS)
    {
        returnStatus = PGS_MEM_ShmAttach((void **)&shmPtr);
        if (returnStatus == PGS_S_SUCCESS)
        {
            shmPtr->id = 123;
            strcpy(shmPtr->msg,"Writing data into shared memory");
        }
    }

NOTES: 
    This shared memory scheme is not a POSIX implementation and will therefore
    be subjected to change when the POSIX.4 implementation is available. System
    limitations will define the amount of memory that can be allocated as a 
    shared-memory segment. Note that only one memory segment may be created per
    PGE; it may however be attached/detached as many times as are required. 

    Note PPS (Planning & Processing Subsystem) has no plan for limiting memory
    usage through the toolkit.

REQUIREMENTS:
    PGSTK-1241

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_MEM_ShmGlbVarUsr()
    PGS_MEM_Shmget()
    PGS_SMF_SetStaticMsg()
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()
    PGS_SMF_TestErrorLevel()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_ShmCreate(        /* Create shared memory */
    PGSt_uinteger size)   /* Input size of the shared memory segment in bytes */
{
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;    
    PGSMemShm       *global_var;          
    PGSt_integer     shmid;   
    char             msg[PGS_SMF_MAX_MSG_SIZE];     
    char             mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];   
    PGSt_SMF_code    code;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status lockRet;

    /* lock all shared memory management */
    lockRet = PGS_TSF_LockIt(PGSd_TSF_SHMMEMLOCK);
    if (PGS_SMF_TestErrorLevel(lockRet))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
#endif


    if ((returnStatus = PGS_MEM_ShmGlbVarUsr(&global_var)) == PGS_S_SUCCESS)
    {                   
        if (global_var->user.created == PGS_TRUE)
        {       
            returnStatus = PGSMEM_E_SHM_MULTICREATE;
        }
        else
        {
            /*
             * Now create user shared-memory.
             */
            if ((returnStatus = PGS_MEM_Shmget(global_var->user.key,
                                               size,
                                               IPC_CREAT|PGS_MEM_SHM_FLAG,
                                               &shmid)) == PGS_S_SUCCESS)
            {           

                /*
                 * NOTE: Do not check memory boundary at this delivery. PPS (7/12/94)
                 * Uncomment this module if memory checking is needed.
                 * ==============================================================

                if (size < PPS_LIMIT)
                {
                    global_var->size.user     = size;
                    global_var->user.shmid    = shmid;
                    global_var->user.created  = PGS_TRUE;              
                }
                else
                {
                    returnStatus = PGSMEM_E_SHM_MAXSIZE;
                }  

                */

                global_var->size.user     = size;
                global_var->user.shmid    = shmid;
                global_var->user.created  = PGS_TRUE;           
            }
        }
    }

#ifdef _PGS_THREADSAFE
    /* unlock shared memory management */
    lockRet = PGS_TSF_UnlockIt(PGSd_TSF_SHMMEMLOCK);
#endif

    if ((returnStatus == PGSMEM_E_SHM_MAXSIZE) ||
        (returnStatus == PGSMEM_E_SHM_MULTICREATE))
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_MEM_ShmCreate()");
    }
    else if ((returnStatus == PGS_E_UNIX) ||
             (returnStatus == PGSMEM_E_SHM_ENV))
    {
        PGS_SMF_GetMsg(&code,mnemonic,msg);
        PGS_SMF_SetDynamicMsg(returnStatus,msg,"PGS_MEM_ShmCreate()");             
    }
    else
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_MEM_ShmCreate()");   
    }

    return(returnStatus);

} /* end PGS_MEM_ShmCreate */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_ShmAttach

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>      

    PGSt_SMF_status 
    PGS_MEM_ShmAttach(
        void **shm);

FORTRAN:
    NONE

DESCRIPTION:
    This tool may be used by an executable to attach to an existing 
    shared memory segment. PGS_MEM_ShmCreate() should already been called, 
    either within the same executable or from an earlier executable within 
    the PGE. If the shared memory segment has been detached by calling 
    PGS_MEM_ShmDetach(), then you may re-attach the segment to your 
    process-space again. 

INPUTS:       
    NONE

OUTPUTS:
    shm - pointer referencing the shared memory segment

RETURNS:                                
    PGS_S_SUCCESS
    PGS_E_UNIX
    PGSMEM_E_SHM_ENV
    PGSMEM_E_SHM_NOTCREATE
    PGSMEM_E_SHM_MULTIATTACH    
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:
    typedef struct
    {
        int  id;
        char msg[100];
    }TestStruct;

    PGSt_SMF_status  returnStatus;
    TestStruct      *shmPtr;

    PROCESS A:
    ==========   
    returnStatus = PGS_MEM_ShmCreate(sizeof(TestStruct));          
    if (returnStatus == PGS_S_SUCCESS)
    {
        returnStatus = PGS_MEM_ShmAttach((void **)&shmPtr);
        if (returnStatus == PGS_S_SUCCESS)
        {
            shmPtr->id = 123;
            strcpy(shmPtr->msg,"From Process A");
        }
    }

    PROCESS B:
    ==========  
    returnStatus = PGS_MEM_ShmAttach((void **)&shmPtr);                      
    if (returnStatus == PGS_S_SUCCESS)
    {
        if ((shmPtr->id = 123) && (strcmp(shmPtr->msg,"From Process A") == 0))
        {
            printf("Reading data from Process A successful");
        }
    }

NOTES:  
    This tool is not part of POSIX and is subjected to change when the POSIX.4 implementation
    becomes available.

REQUIREMENTS:
    PGSTK-1241

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_MEM_ShmGlbVarUsr()
    PGS_MEM_Shmat()
    PGS_SMF_SetStaticMsg()
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()
    PGS_SMF_TestErrorLevel()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_ShmAttach(  /* Attach shared memory */
    void **shm)     /* Input  pointer referencing the shared memory segment */
                    /* Output pointer referencing the shared memory segment */
{
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;
    PGSMemShm       *global_var;            
    char             msg[PGS_SMF_MAX_MSG_SIZE];    
    char             mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];   
    PGSt_SMF_code    code;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status lockRet;

    /* lock all shared memory management */
    lockRet = PGS_TSF_LockIt(PGSd_TSF_SHMMEMLOCK);
    if (PGS_SMF_TestErrorLevel(lockRet))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
#endif

    if ((returnStatus = PGS_MEM_ShmGlbVarUsr(&global_var)) == PGS_S_SUCCESS)
    {    
        if (global_var->user.created == PGS_FALSE)
        {        
            returnStatus = PGSMEM_E_SHM_NOTCREATE;
        }
        else if (glb_usrAttached == PGS_TRUE)
        {       
            returnStatus = PGSMEM_E_SHM_MULTIATTACH;
        }
        else
        {
            /*
             * Attach user shared-memory 
             */
            returnStatus = PGS_MEM_Shmat(global_var->user.shmid,
                                         SHM_RND,
                                         shm);                       
        }        
    }

    if (returnStatus == PGS_S_SUCCESS)
    {               
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_MEM_ShmAttach()");
        glb_usrAttached = PGS_TRUE; 
        glb_usrAddr     = (unsigned long) *shm;    
    }
    else
    {
        if ((returnStatus == PGSMEM_E_SHM_NOTCREATE) ||
            (returnStatus == PGSMEM_E_SHM_MULTIATTACH))
        {
            PGS_SMF_SetStaticMsg(returnStatus,"PGS_MEM_ShmAttach()");
        }
        else if ((returnStatus == PGS_E_UNIX) ||
                 (returnStatus == PGSMEM_E_SHM_ENV))
        {
            PGS_SMF_GetMsg(&code,mnemonic,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,msg,"PGS_MEM_ShmAttach()");      
        }

        *shm = (void *)NULL;    
    }

#ifdef _PGS_THREADSAFE
    /* unlock shared memory management */
    lockRet = PGS_TSF_UnlockIt(PGSd_TSF_SHMMEMLOCK);
#endif

    return(returnStatus);

} /* end PGS_MEM_ShmAttach */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_ShmDetach

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>                                                  

    PGSt_SMF_status
    PGS_MEM_ShmDetach(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    This tool may be used to detach a shared memory segment from a process
    that has been attached to it.

INPUTS:        
    NONE

OUTPUTS:        
    NONE

RETURNS:
    PGS_S_SUCCESS
    PGS_E_UNIX
    PGSMEM_E_SHM_NOTATTACH    
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:
    typedef struct
    {
        int  id;
        char msg[100];
    }TestStruct;

    PGSt_SMF_status  returnStatus;
    TestStruct      *shmPtr;

    returnStatus = PGS_MEM_ShmCreate(sizeof(TestStruct));
    if (returnStatus == PGS_S_SUCCESS)
    {
        returnStatus = PGS_MEM_ShmAttach((void **)&shmPtr);
        if (returnStatus == PGS_S_SUCCESS)
        {
            shmPtr->id = 123;
            strcpy(shmPtr->msg,"Writing data into shared memory");

            PGS_MEM_ShmDetach();
        }
    }

NOTES:  
    This tool is not part of POSIX and is subjected to change when the
    POSIX.4 implementation becomes available. This function will only
    detach the shared memory segment from the process. The shared memory
    segment will not be removed from the system by calling this tool;
    therefore one can re-attach it again.

REQUIREMENTS:
    PGSTK-1241

DETAILS:
    Before exiting from the PGE, the system will make sure that the attached
    shared memory segment will be removed from the system.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:   
    PGS_MEM_Shmdt()
    PGS_SMF_SetStaticMsg()
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()
    PGS_SMF_TestErrorLevel()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_MEM_ShmDetach(  /* Detach shared memory */
    void)
{   
    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;
    char            msg[PGS_SMF_MAX_MSG_SIZE];          
    char            mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];   
    PGSt_SMF_code   code;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status lockRet;

    /* lock all shared memory management */
    lockRet = PGS_TSF_LockIt(PGSd_TSF_SHMMEMLOCK);
    if (PGS_SMF_TestErrorLevel(lockRet))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
#endif

    if (glb_usrAttached == PGS_TRUE)
    {
        returnStatus = PGS_MEM_Shmdt((void *)glb_usrAddr);
    }
    else
    {
        returnStatus = PGSMEM_E_SHM_NOTATTACH;
    }

    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_MEM_ShmDetach()");
        glb_usrAttached = PGS_FALSE;
        glb_usrAddr     = 0L;
    }
    else
    {
        if (returnStatus == PGSMEM_E_SHM_NOTATTACH)                
        {
            PGS_SMF_SetStaticMsg(returnStatus,"PGS_MEM_ShmDetach()");
        }
        else
        {
            PGS_SMF_GetMsg(&code,mnemonic,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,msg,"PGS_MEM_ShmDetach()");      
        }
    }            

#ifdef _PGS_THREADSAFE
    /* unlock shared memory management */
    lockRet = PGS_TSF_UnlockIt(PGSd_TSF_SHMMEMLOCK);
#endif

    return(returnStatus);

} /* end PGS_MEM_ShmDetach */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_ShmGlbVarUsr

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>

    static PGSt_SMF_status
    PGS_MEM_ShmGlbVarUsr(
    PGSMemShm **global_var);

FORTRAN:
    NONE

DESCRIPTION:
    To encapsulate global variables for the processing of user
    shared-memory. Basically this routine will make sure that
    system shared memory has been attached to the process.This 
    routine is used in conjunction with PGS_MEM_ShmCreate(), 
    PGS_MEM_ShmAttach() and PGS_MEM_ShmDetach() routines only.

INPUTS:        
    NONE

OUTPUTS:        
    global_var - pointer to global data structure

RETURNS:        
    PGS_S_SUCCESS
    PGS_E_UNIX    
    PGSMEM_E_SHM_ENV    

EXAMPLES:
    PGSMemShm *global_var;

    PGS_MEM_ShmGlbVarUsr(&global_var);

NOTES:  
    Environment variable "PGSMEM_SHM_SYSKEY" must be set in the 
    shell-script (PGE). Prior to calling this routine, system shared memory
    should already have been created by calling PGS_MEM_ShmSysInit(). If 
    system shared memory has not been attached to the current process, then 
    this routine will make sure that the system shared memory is attached once. 
    System shared memory will contain user's shared memory information.

REQUIREMENTS:        
    PGSTK-1241

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:   
    PGS_MEM_Shmget()
    PGS_MEM_Shmat()
    PGS_SMF_SetStaticMsg()
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_status 
PGS_MEM_ShmGlbVarUsr(          /* Get global data sturcture */
    PGSMemShm **global_var)    /* Output pointer to global data structure */
{  
    PGSt_integer            shmid;
    PGSt_MEM_key            key_sys;   
    PGSt_SMF_status         returnStatus = PGS_S_SUCCESS;
    char                    msg[PGS_SMF_MAX_MSG_SIZE];        
    char                    mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];
    char                   *sysKey = (char *)NULL;   
    PGSt_SMF_code           code;


    /*
     * System shared memory might have been attached to this process via
     * PGS_MEM_ShmSysInit() routine.
     */
    if (glb_sysAttached == PGS_FALSE)
    {
        /*
         * Get system shared-memory key.
         */
        if ((sysKey = getenv("PGSMEM_SHM_SYSKEY")) == (char *)NULL)
        {
            returnStatus = PGSMEM_E_SHM_ENV;        
        }
        else
        {
            key_sys = (PGSt_MEM_key)atoi(sysKey);
            if ((returnStatus = PGS_MEM_Shmget(key_sys,
                                               sizeof(PGSMemShm),
                                               PGS_MEM_SHM_FLAG,
                                               &shmid)) == PGS_S_SUCCESS)
            {

                /*
                 * Attach system shared-memory.
                 */
                if ((returnStatus = PGS_MEM_Shmat(shmid,
                                                  SHM_RND,
                                                  (void **)(&glb_sysData))) == PGS_S_SUCCESS)
                {        
                    glb_sysAttached = PGS_TRUE;
                }                       
            }
        }
    }

    if (glb_sysAttached == PGS_TRUE) 
    {
        *global_var = glb_sysData;
    }
    else
    {
        *global_var = (PGSMemShm *)NULL; 

        if (returnStatus == PGSMEM_E_SHM_ENV)
        {
            PGS_SMF_SetStaticMsg(returnStatus,"PGS_MEM_ShmGlbVarUsr()");
        }
        else
        {
            PGS_SMF_GetMsg(&code,mnemonic,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,msg,"PGS_MEM_ShmGlbVarUsr()");
        }
    }        

    return(returnStatus);    

} /* end PGS_MEM_ShmGlbVarUsr */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_ShmPrintBuf

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>      

    void
    PGS_MEM_ShmPrintBuf(
        PGSMemShm *buf);

FORTRAN:
    NONE

DESCRIPTION:
    Print the contents of the structure PGSMemShm buffer.    

INPUTS:        
    buf - buffer 

OUTPUTS:        
    NONE

RETURNS:
    NONE

EXAMPLES:
    PGSMemShm *global_var;

    PGS_MEM_ShmGlbVarUsr(&global_var);
    PGS_MEM_ShmPrintBuf(global_var);

REQUIREMENTS:
    PGSTK-1241

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
PGS_MEM_ShmPrintBuf(   /* Print the structure */
    PGSMemShm  *buf)   /* Input buffer */
{






} /* end PGS_MEM_ShmPrintBuf */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_Shmget

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>

    PGSt_SMF_status 
    PGS_MEM_Shmget(
        PGSt_MEM_key  key,
        PGSt_integer  size,
        PGSt_integer  flag,
        PGSt_integer *shmid);

FORTRAN:
    NONE

DESCRIPTION:
    This routine allows user to create shared memory. This tool provides a wrapper 
    around shmget() routine. It should only be used by system and not for user.          

INPUTS:        
    key - key 
    size - size in bytes 
    flag - flag to create or reference

OUTPUTS:        
    shmid - shared-memory id

RETURNS:                                
    PGS_S_SUCCESS
    PGS_E_UNIX    

EXAMPLES:
    PGSt_integer shmid;   

    PGS_MEM_Shmget(1234,1000,IPC_CREAT|PGS_MEM_SHM_FLAG,&shmid);

REQUIREMENTS:
    PGSTK-1241

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_SMF_GetMsgByCode()
    PGS_SMF_SetDynamicMsg()
    PGS_SMF_CallerID()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_Shmget(            /* Create shared memory */
    PGSt_MEM_key   key,    /* Input  key */
    PGSt_integer   size,   /* Input  size in bytes */
    PGSt_integer   flag,   /* Input  flag to create or reference */
    PGSt_integer  *shmid)  /* Output shared-memory id */
{
    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;
    char            buf[PGS_SMF_MAX_MSGBUF_SIZE];
    char            msg[PGS_SMF_MAX_MSG_SIZE];


    *shmid = (PGSt_integer)shmget(key,(int)size,(int)flag);
    if (*shmid == -1)
    {       
        if (PGS_SMF_CallerID() == PGSd_CALLERID_USR)
        {
            PGS_SMF_GetMsgByCode(PGSMEM_E_SHM,msg);
            sprintf(buf,msg,errno,strerror(errno));           
            PGS_SMF_SetDynamicMsg(PGS_E_UNIX,buf,"PGS_SMF_Shmget()");           
        }

        returnStatus = PGS_E_UNIX;
    }

    return(returnStatus);

} /* end PGS_MEM_Shmget */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_Shmat

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>

    PGSt_SMF_status 
    PGS_MEM_Shmat(
        PGSt_integer  shmid,
        PGSt_integer  flag),
        void        **shmaddr);

FORTRAN:
    NONE

DESCRIPTION:
    This tool may be used by a process to attach to an existing shared memory
    segment. This tool provides a wrapper around shmat() routine. It should only 
    be used by system and not for user.

INPUTS:
    shmid - shared memory id
    flag - specify the access permissions on the segment

OUTPUTS:
    shmaddr - address pointer to the shared memory segment

RETURNS:                        
    PGS_S_SUCCESS
    PGS_E_UNIX    

EXAMPLES:    
    PGSt_integer shmid;
    char *shm;

    PGS_MEM_Shmget(1234,1000,IPC_CREAT|PGS_MEM_SHM_FLAG,&shmid); 
    PGS_MEM_Shmat(shmid,SHM_RND,&shm);          

REQUIREMENTS:           
    PGSTK-1241

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_SMF_GetMsgByCode()
    PGS_SMF_SetDynamicMsg()
    PGS_SMF_CallerID()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_Shmat(              /* Attached shared memory */
    PGSt_integer  shmid,    /* Input  shared memory id */
    PGSt_integer  flag,     /* Input  specify the access permissions on the segment */
    void        **shmaddr)  /* Output address pointer to the shared memory segment */
{
    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;
    char            buf[PGS_SMF_MAX_MSGBUF_SIZE];
    char            msg[PGS_SMF_MAX_MSG_SIZE];


    *shmaddr = (void *)shmat((int)shmid,(void *)NULL,(int)flag);
    if (*shmaddr == (char *) -1) 
    {      
        if (PGS_SMF_CallerID() == PGSd_CALLERID_USR)
        { 
            PGS_SMF_GetMsgByCode(PGSMEM_E_SHM,msg);
            sprintf(buf,msg,errno,strerror(errno));            
            PGS_SMF_SetDynamicMsg(PGS_E_UNIX,buf,"PGS_SMF_Shmat()");
        }

        returnStatus = PGS_E_UNIX;
    }

    return(returnStatus);

} /* end PGS_MEM_Shmat */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_Shmdt

SYNOPSIS:       
c:  
    #include <PGS_MEM1.h>

    PGSt_SMF_status 
    PGS_MEM_Shmdt(
        void *shmaddr);

FORTRAN:
    NONE

DESCRIPTION:
    This tool may be used to detach a shared memory segment from a process
    that attached it. This tool provides a wrapper around shmdt() routine.
    It should only be used by system and not for user.

INPUTS:
    shmaddr - segment address that has been attached

OUTPUTS:           
    NONE

RETURNS:        
    PGS_S_SUCCESS
    PGS_E_UNIX        

EXAMPLES:
    PGSt_integer shmid;
    char *shm;

    PGS_MEM_Shmget(1234,1000,IPC_CREAT|PGS_MEM_SHM_FLAG,&shmid); 
    PGS_MEM_Shmat(shmid,SHM_RND,&shm); 
    PGS_MEM_Shmdt(shm);

REQUIREMENTS:           
    PGSTK-1241

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_SMF_GetMsgByCode()
    PGS_SMF_SetDynamicMsg()
    PGS_SMF_CallerID()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_Shmdt(      /* Detach shared memory */
    void *shmaddr)  /* Input segment address that has been attached */
{
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;
    PGSt_integer     ret_val;
    char             buf[PGS_SMF_MAX_MSGBUF_SIZE];
    char             msg[PGS_SMF_MAX_MSG_SIZE];


    ret_val = shmdt((char *)shmaddr);
    if (ret_val == -1)
    {
        if (PGS_SMF_CallerID() == PGSd_CALLERID_USR)
        {
            PGS_SMF_GetMsgByCode(PGSMEM_E_SHM,msg);
            sprintf(buf,msg,errno,strerror(errno));
            PGS_SMF_SetDynamicMsg(PGS_E_UNIX,buf,"PGS_SMF_Shmdt()");
        }

        returnStatus = PGS_E_UNIX;
    }

    return(returnStatus);

} /* end PGS_MEM_Shmdt */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_Shmctl

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>

    PGSt_SMF_status      
    PGS_MEM_Shmctl(
        PGSt_integer  shmid,
        PGSt_integer  cmd,                                    
        void         *shmbuf);

FORTRAN:
    NONE

DESCRIPTION:
    This tool may be used to examine about a shared memory segment. It only
    allows you to fetch information and remove the shared memory segment
    set from the system. This tool provides a wrapper around shmctl() routine.
    It should only be used by system and not for user.

INPUTS:
    shmid - shared memory id
    cmd - IPC_STAT or IPC_RMID

OUTPUTS:
    shmbuf - structure pointed by buffer

RETURNS:                        
    PGS_S_SUCCESS
    PGS_E_UNIX    

EXAMPLES:
    struct shmid_ds shmbuf;
    PGSt_integer    shmid;
    char           *shm;

    PGS_MEM_Shmget(1234,1000,IPC_CREAT|PGS_MEM_SHM_FLAG,&shmid); 
    PGS_MEM_Shmat(shmid,SHM_RND,&shm); 
    PGS_MEM_Shmctl(shmid,IPC_RMID,&shmbuf);          

REQUIREMENTS:           
    PGSTK-1241

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_SMF_GetMsgByCode()
    PGS_SMF_SetDynamicMsg()
    PGS_SMF_CallerID()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_Shmctl(           /* Get information about shared memory */
    PGSt_integer  shmid,  /* Input  shared memory id */
    PGSt_integer  cmd,    /* Input  IPC_STAT or IPC_RMID */
    void         *shmbuf) /* Output structure pointed by buffer */
{
    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;
    PGSt_integer    ret_val;
    char            buf[PGS_SMF_MAX_MSGBUF_SIZE];
    char            msg[PGS_SMF_MAX_MSG_SIZE];


    if (cmd == IPC_RMID) 
    {
        ret_val = shmctl((int)shmid,(int)cmd,(struct shmid_ds *)NULL);
    }
    else 
    {
        ret_val = shmctl((int)shmid,(int)cmd,(struct shmid_ds *)shmbuf);
    }

    if (ret_val == -1)
    { 
        if (PGS_SMF_CallerID() == PGSd_CALLERID_USR)
        {
            PGS_SMF_GetMsgByCode(PGSMEM_E_SHM,msg);
            sprintf(buf,msg,errno,strerror(errno));
            PGS_SMF_SetDynamicMsg(PGS_E_UNIX,buf,"PGS_SMF_Shctl()");
        }

        returnStatus = PGS_E_UNIX;
    }

    return(returnStatus);

} /* end PGS_MEM_Shmctl */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_ShmSysInit

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>

    PGSt_SMF_status      
    PGS_MEM_ShmSysInit(
        PGSMemShmSize &size);

FORTRAN:
    NONE

DESCRIPTION:
    This routine must be called in the first executable within the 
    shell-script (PGE).It is used to set up internal shared-memory 
    used by PGE to keep track of user's attached shared memory and 
    to initialize internal data structure needed within PGE. This 
    routine must not be called from other processes other than the 
    initializing (first) process.

INPUTS:        
    size - application total memory usage           

OUTPUTS:   
    NONE        

RETURNS:                        
    PGS_S_SUCCESS
    PGS_SH_MEM_INIT

EXAMPLES:
    PGSMemShmSize    size;
    PGSt_SMF_status  returnStatus;

    size.pc = 1000;
    size.smf = sizeof(PGSSmfShm);
    returnStatus = PGS_MEM_ShmSysInit(&size);
    if (returnStatus == PGS_S_SUCCESS)
    {
        printf("Successful in creating system shared memory");
    }

NOTES:             
    Environment variable "PGSMEM_SHM_SYSKEY" must be set in the
    shell-script. This function is the only routine to create the 
    needed system shared memory for the entire PGE processes and should
    be called once within PGE.  

    As of TK4, this tool is only to be called by ARC developers. User
    should not call this tool. 

REQUIREMENTS:
    PGSTK-1241

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_MEM_Shmget()
    PGS_MEM_Shmat()    

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_ShmSysInit(        /* Setup system shared memory to be used within PGE */
    PGSMemShmSize *size)   /* Input application total memory usage */
{
    PGSt_MEM_key            key_sys;
    PGSt_MEM_key            key_user;
    PGSt_integer            shmid; 
    PGSt_integer            totalSize;   
    PGSt_SMF_status         returnStatus = PGS_SH_MEM_INIT;
    char                   *sysKey = (char *)NULL;  


    if (glb_sysAttached == PGS_TRUE)
    {
        returnStatus = PGS_S_SUCCESS;
    }
    else
    {
        /*
         * Use shell (PGE) and creator pid for unique keys. 
         */
        if ((sysKey = getenv("PGSMEM_SHM_SYSKEY")) != (char *)NULL)
        {
            /*
             * Note that even this initialization process terminates, system is
             * guaranteed not to use this process id for other process.          
             */
            key_sys  = (PGSt_MEM_key)atoi(sysKey);
            key_user = (PGSt_MEM_key)getpid();

            /*
             * Create system shared-memory.
             */
            totalSize = sizeof(PGSMemShm) + size->pc + size->smf;
            if ((returnStatus = PGS_MEM_Shmget(key_sys,
                                               totalSize,
                                               IPC_CREAT|PGS_MEM_SHM_FLAG,
                                               &shmid)) == PGS_S_SUCCESS)
            {

                /*
                 * Now attach the segment to our data space (process).
                 */ 
                if ((returnStatus = PGS_MEM_Shmat(shmid,
                                                  SHM_RND,
                                                  (void **)&glb_sysData)) == PGS_S_SUCCESS)
                {                                            
                    /*
                     * Record total memory used by system.
                     */
                    glb_sysData->size.smf   = size->smf; 
                    glb_sysData->size.pc    = size->pc;
                    glb_sysData->size.total = totalSize;

                    /*
                     * Offset address of PC and SMF.
                     */
                    glb_sysData->offset.smf = sizeof(PGSMemShm);
                    glb_sysData->offset.pc  = glb_sysData->offset.smf + glb_sysData->size.smf;

                    /*
                     * System shared memory information.
                     */
                    glb_sysData->sys.key   = key_sys;           
                    glb_sysData->sys.shmid = shmid;                               

                    /*
                     * User shared memory information.
                     */
                    glb_sysData->user.key     = key_user;
                    glb_sysData->user.shmid   = 0;
                    glb_sysData->user.created = PGS_FALSE;

                    /*
                     * Flag to indicate system memory has been attached to this process.
                     */
                    glb_sysAttached = PGS_TRUE;  


                }    /* shmat      */
            }        /* shmget     */
        }            /* user key   */
    }                /* system key */


    if (returnStatus != PGS_S_SUCCESS)
    {
        returnStatus = PGS_SH_MEM_INIT;
    }

    return(returnStatus);

} /* end PGS_MEM_ShmSysInit */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_ShmSysTerm

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>

    PGSt_SMF_status      
    PGS_MEM_ShmSysTerm(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    To clean up all shared memory that has been used within PGE. This
    routine must be called in the last executable within the executing 
    shell-script (PGE). This routine must not be called from other
    processes other than the termination process.

INPUTS:        
    NONE

OUTPUTS:           
    NONE

RETURNS:                        
    PGS_S_SUCCESS
    PGS_SH_MEM_TERM

EXAMPLES:
    PGS_MEM_ShmSysTerm();

NOTES:             
    Environment variable "PGSMEM_SHM_SYSKEY" must be set in the
    shell-script. This function is the only routine to remove all shared 
    memory for the entire PGE processes. This routine will attach the
    system shared memory if the termination process has not attach it
    prior to the calling of this routine.

REQUIREMENTS:           
    PGSTK-1241

DETAILS:
    As of TK4, this tool is only to be called by ARC developers. User
    should not call this tool. 

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_MEM_Shmget()
    PGS_MEM_Shmat()
    PGS_MEM_Shmctl()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_ShmSysTerm(  /* Clean up all shared memory used within PGE */
    void)
{
    PGSt_MEM_key      key_sys;
    PGSt_integer      shmid;
    PGSMemShm        *shmData;
    PGSt_SMF_status   returnStatus = PGS_SH_MEM_TERM;
    struct shmid_ds   buf;
    char             *sysKey = (char *)NULL;  
    PGSt_SMF_boolean  destroySys = PGS_TRUE;


    if (glb_sysAttached == PGS_FALSE)
    {
        if ((sysKey = getenv("PGSMEM_SHM_SYSKEY")) != (char *)NULL)
        {
            key_sys = (PGSt_MEM_key)atoi(sysKey);
            if ((returnStatus = PGS_MEM_Shmget(key_sys,
                                               sizeof(PGSMemShm),
                                               PGS_MEM_SHM_FLAG,
                                               &shmid)) == PGS_S_SUCCESS)
            {
                returnStatus = PGS_MEM_Shmat(shmid,SHM_RND,(void **)&shmData);        
            }
        }
    }
    else
    {
        shmData = glb_sysData;
        returnStatus = PGS_S_SUCCESS;
    }

    if (returnStatus == PGS_S_SUCCESS)
    {
        /*
         * Note that user might not even call to create shared memory.
         */

        if (shmData->user.created == PGS_TRUE)
        {
            /* 
             * Destroy user shared memory.
             */
            if ((returnStatus = PGS_MEM_Shmctl(shmData->user.shmid,IPC_RMID,&buf)) != PGS_S_SUCCESS)
            {     
                destroySys = PGS_FALSE;
            }
        }


        if (destroySys == PGS_TRUE)
        {
            /*
             * Destroy system shared memory.
             */
            returnStatus = PGS_MEM_Shmctl(shmData->sys.shmid,IPC_RMID,&buf);
        }
    }

    if (returnStatus == PGS_S_SUCCESS)
    {
        glb_sysAttached = PGS_FALSE;
        glb_sysData = (PGSMemShm *)NULL;    
    }
    else
    {
        returnStatus = PGS_SH_MEM_TERM;
    }

    return(returnStatus);

} /* end PGS_MEM_ShmSysTerm */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_ShmSysAddr

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>

    PGSt_SMF_status
    PGS_MEM_ShmSysAddr(
        short           keyAddr,
        void          **addr,
        PGSt_uinteger  *size);

FORTRAN:
    NONE

DESCRIPTION:
    This routine will provide facility to retrieve individual shared
    memory address based on unique key. Basically it is used only by 
    PGE internal. This routine will attach the system shared memory 
    if the current process has not attach it prior to the calling 
    of this routine.

INPUTS:
    keyAddr - PGS_MEM_SHM_PC
              PGS_MEM_SHM_SYS

OUTPUTS:
    addr - pointer to data address 
    size - total size available for this key

RETURNS:   
    PGS_S_SUCCESS
    PGS_E_UNIX
    PGSMEM_E_SHM_ENV
    PGSMEM_E_SHM_INVALIDKEY

EXAMPLES:
    typedef struct
    {
        int   id;
        char  msg[100];
    }PCStruct;

    PGSt_uinteger     size;
    PGSt_SMF_status   returnStatus;
    PCStruct         *pc;

    PGS_MEM_ShmSysAddr(PGS_MEM_SHM_PC,(void **)&pc,&size);               

REQUIREMENTS:          
    PGSTK-1241

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED: 
    PGS_MEM_Shmget()
    PGS_MEM_Shmat()   
    PGS_SMF_SetStaticMsg()
    PGS_SMF_GetMsg()
    PGS_SMF_SetDynamicMsg()
    PGS_SMF_CallerID()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status
PGS_MEM_ShmSysAddr(             /* Retrieve individual data address */
    PGSt_integer    keyAddr,    /* Input  key */
    void          **addr,       /* Output pointer to data address */
    PGSt_uinteger  *size)       /* Output size */
{        
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;
    PGSt_MEM_key     key_sys;
    PGSt_integer     shmid;
    char             msg[PGS_SMF_MAX_MSG_SIZE];   
    char             mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; 
    char            *sysKey = (char *)NULL;  
    PGSt_SMF_code    code;


    if (glb_sysAttached == PGS_FALSE)
    {
        if ((sysKey = getenv("PGSMEM_SHM_SYSKEY")) == (char *)NULL)
        {
            returnStatus = PGSMEM_E_SHM_ENV;
        }
        else
        {
            key_sys = (PGSt_MEM_key)atoi(sysKey);

            if ((returnStatus = PGS_MEM_Shmget(key_sys,
                                               sizeof(PGSMemShm),
                                               PGS_MEM_SHM_FLAG,
                                               &shmid)) == PGS_S_SUCCESS)
            {
                returnStatus = PGS_MEM_Shmat(shmid,SHM_RND,(void **)&glb_sysData);
            }
        }
    }
    else
    {
        returnStatus = PGSMEM_E_SHM_MULTIATTACH;
    }


    if ((returnStatus == PGS_S_SUCCESS) || (returnStatus == PGSMEM_E_SHM_MULTIATTACH))
    {        

        switch (keyAddr)
        {
            case PGS_MEM_SHM_SMF:                   
                 if (addr != NULL)
                 {         
                     *addr = (void *)((char*) glb_sysData + glb_sysData->offset.smf);
                 }

                 if (size != NULL)
                 {
                     *size = glb_sysData->size.smf;                                           
                 }
                 break;        

            case PGS_MEM_SHM_PC:  
                 if (addr != NULL)
                 {         
                     *addr = (void *)((char*) glb_sysData + glb_sysData->offset.pc);
                 }

                 if (size != NULL)
                 {
                     *size = glb_sysData->size.pc;                                           
                 }
                 break;             

            case PGS_MEM_SHM_SYS:  
                 if (addr != NULL)
                 {             
                     *addr = (void *)glb_sysData;
                 } 

                 if (size != NULL)
                 {
                     *size = 0;
                 }
                 break;

            default:
                 returnStatus = PGSMEM_E_SHM_INVALIDKEY;
                 break;
        }                                                  
    }

    if ((returnStatus == PGS_S_SUCCESS) || (returnStatus == PGSMEM_E_SHM_MULTIATTACH))
    { 
        /*
         * Note that glb_sysAttached should be set here first before calling
         * PGS_SMF_SetStaticMsg() since this routine will call PGS_MEM_ShmSysAddr()
         * in return. By setting it here, we will avoid cyclic problem.
         */        
        glb_sysAttached = PGS_TRUE;        

        returnStatus = PGS_S_SUCCESS;        
        if (PGS_SMF_CallerID() == PGSd_CALLERID_USR)
        { 
            PGS_SMF_SetStaticMsg(returnStatus,"PGS_MEM_ShmSysAddr()");
        }                
    }
    else
    {
        if (addr != NULL)
        {
            *addr = NULL;
        }

        if (size != NULL)
        {
            *size = 0;
        }        

        if (PGS_SMF_CallerID() == PGSd_CALLERID_USR)
        { 
            if ((returnStatus == PGSMEM_E_SHM_ENV) || (returnStatus == PGSMEM_E_SHM_INVALIDKEY))
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_MEM_ShmSysAddr()");
            }
            else
            {
                PGS_SMF_GetMsg(&code,mnemonic,msg);
                PGS_SMF_SetDynamicMsg(returnStatus,msg,"PGS_MEM_ShmSysAddr()");
            }
        }
    }                          

    return(returnStatus);

} /* end PGS_MEM_ShmSysAddr */



#else

PGSt_SMF_status 
PGS_MEM_ShmCreate(
    PGSt_uinteger size) 
{ 
    PGS_SMF_SetStaticMsg(PGS_E_UNIX,"PGS_MEM_ShmCreate()");
    return PGS_E_UNIX;
}

PGSt_SMF_status 
PGS_MEM_ShmAttach(  
    void **shm)
{
    PGS_SMF_SetStaticMsg(PGS_E_UNIX,"PGS_MEM_ShmAttach()");
    return PGS_E_UNIX;
}

PGSt_SMF_status
PGS_MEM_ShmDetach( 
    void)
{
    PGS_SMF_SetStaticMsg(PGS_E_UNIX,"PGS_MEM_ShmDetach()");
    return PGS_E_UNIX;
}    

PGSt_SMF_status 
PGS_MEM_Shmget(
    PGSt_MEM_key key,
    PGSt_integer size,
    PGSt_integer flag,
    PGSt_integer *shmid)
{
    PGS_SMF_SetStaticMsg(PGS_E_UNIX,"PGS_MEM_Shmget()");
    return PGS_E_UNIX;
}    

PGSt_SMF_status 
PGS_MEM_Shmat(
    PGSt_integer shmid,
    PGSt_integer flag,
    void       **shmaddr)
{
    PGS_SMF_SetStaticMsg(PGS_E_UNIX,"PGS_MEM_Shmat()");
    return PGS_E_UNIX;
}    

PGSt_SMF_status 
PGS_MEM_Shmdt(
    void *shmaddr)
{
    PGS_SMF_SetStaticMsg(PGS_E_UNIX,"PGS_MEM_Shmdt()");
    return PGS_E_UNIX;
}    

PGSt_SMF_status 
PGS_MEM_Shmctl(
    PGSt_integer shmid,
    PGSt_integer cmd,
    void        *shmbuf)
{
    PGS_SMF_SetStaticMsg(PGS_E_UNIX,"PGS_MEM_Shmctl()");
    return PGS_E_UNIX;
}    

PGSt_SMF_status 
PGS_MEM_ShmSysInit(
    PGSMemShmSize *size)
{
    PGS_SMF_SetStaticMsg(PGS_SH_MEM_INIT,"PGS_MEM_ShmSysInit()");
    return PGS_SH_MEM_INIT;
}    

PGSt_SMF_status 
PGS_MEM_ShmSysTerm(
    void)
{
    PGS_SMF_SetStaticMsg(PGS_SH_MEM_TERM,"PGS_MEM_ShmSysTerm()");
    return PGS_SH_MEM_TERM;   
}

PGSt_SMF_status 
PGS_MEM_ShmSysAddr(
    PGSt_integer    keyAddr,
    void          **addr,
    PGSt_uinteger  *size)
{
    PGS_SMF_SetStaticMsg(PGS_E_UNIX,"PGS_MEM_ShmSysAddr()");
    return PGS_E_UNIX;   
}

void
PGS_MEM_ShmPrintBuf(
    PGSMemShm *buf)
{

}    

#endif /* end -DSHMMEM */



