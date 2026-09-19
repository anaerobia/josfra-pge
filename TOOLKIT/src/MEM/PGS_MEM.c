/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_MEM.c

DESCRIPTION:
  This file contains the commnon memory management functions for PGS.
  The routines are:  
    PGS_MEM_Malloc()
    PGS_MEM_Calloc()        
    PGS_MEM_Realloc()       
    PGS_MEM_Zero()           
    PGS_MEM_Free()           
    PGS_MEM_FreeAll()

  Note that all these routines make used of link-list utilities. The link-list
  utilities are used to keep track of addresses that have been allocated and
  total amout of memory used (including the link-list itself) within this process.        

  Planning & Processing Subsystem (PPS) has no plans for limiting memory usage through
  these toolkits (7/12/94). Thus no memory boundary checking is performed. 

AUTHOR:
  Kelvin K. Wan / Applied Research Corp.

HISTORY:
  25-Mar-1994 KCW Standard Convention.
   1-Jul-1994 KCW Make sure toolkit routine return UNIX status if it is a UNIX error.
  11-Jul-1994 KCW Revised to accomodate new design.
  11-Aug-1994 KCW Revised to accomodate code inspection comment.
  19-Oct-1994 KCW Revised to include comment on initialization of deallocated pointers.
  07-Jul-1999 RM  Updated for TSF functionality

END_FILE_PROLOG:
*****************************************************************/

/*
 * System Headers 
 */
#include <stdio.h>

/*
 * Toolkit Headers 
 */
#include <PGS_MEM.h> /* contain SMF and all necessary include files */
#include <PGS_TSF.h>

/*
 * Structure Definition
 */
typedef struct pgsmemlinklist
{
    void                   *addr;  /* point to allocated memory */
    size_t                  size;  /* size of the addr in bytes */
    struct pgsmemlinklist  *next;  /* point to right node       */
    struct pgsmemlinklist  *prev;  /* point to left node        */
}PGSMEMLinkList;

/*
 * Static Variables 
 */
static PGSMEMLinkList      *pgsmem_node_head = (PGSMEMLinkList *)NULL;  /* pointer to head node in the link-list */
static int                  pgsmem_node_cnt  = 0;                       /* number of nodes in the link-list */

/*
 * Static Functions 
 */
static PGSt_SMF_status      PGS_MEM_LinkNewNode     (void *addr,size_t *numBytes,PGSMEMLinkList **node,char *funcName);
static void                 PGS_MEM_LinkAddNode     (PGSMEMLinkList *node);
static void                 PGS_MEM_LinkPrintNode   (PGSMEMLinkList *node);
static void                 PGS_MEM_LinkPrintAll    (void);
static void                 PGS_MEM_LinkReplaceAddr (PGSMEMLinkList **node,void *addr,size_t *numBytes);
static PGSt_SMF_boolean     PGS_MEM_LinkMemberExist (void *addr,PGSMEMLinkList **node);
static void                 PGS_MEM_LinkDelNode     (PGSMEMLinkList **node);
static PGSt_SMF_status      PGS_MEM_TrackUsage      (size_t numBytes,char *funcName);



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_Malloc

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>                                                  

    PGSt_SMF_status
    PGS_MEM_Malloc(
        void   **addr,
        size_t   numBytes);

FORTRAN:
    NONE

DESCRIPTION:
    Allocates an arbitrary number of bytes in memory.

INPUTS:   
    Name            Description                                Units        Min        Max

    numBytes        number of bytes to allocate

OUTPUTS:
    Name            Description                                Units        Min        Max

    addr            pointer to beginning of address that 
                    has been allocated

RETURNS:        			
    PGS_S_SUCCESS
    PGSMEM_E_NO_MEMORY
    PGSMEM_W_MEMORY_USED
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:
    int              i;
    int             *intPtr = (int *)NULL;
    PGSt_SMF_status  returnStatus;      

    returnStatus = PGS_MEM_Malloc((void **)&intPtr,sizeof(int)*10);
    if (returnStatus == PGS_S_SUCCESS)
    {
        for (i=0 ; i < 10 ; i++)
        {
            intPtr[i] = i;
        }
    }

NOTES:
    This tool will control the amount of memory that may be allocated at any
    one time. You should call PGS_MEM_Free() to free the memory allocated
    once you are done using it; failure to do so may cause future memory
    allocation requests to fail within the same process.

    Because the Toolkit memory functions track memory usage, it is
    imperative that pointer variables, which have been freed, be
    initialized to NULL prior to re-use. As a reminder, ALL local
    pointer variables MUST be initialized to NULL prior to use.
    Failure to heed these warnings may result in anomalous behavior 
    within your process.

REQUIREMENTS:
    PGSTK-1240

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_MEM_TrackUsage()
    PGS_SMF_SetStaticMsg()
    PGS_MEM_LinkNewNode()
    PGS_MEM_LinkAddNode()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()
    PGS_SMF_TestErrorLevel()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_Malloc(          /* Allocate memory */
    void   **addr,       /* Output pointer to beginning of address that has been allocated */
    size_t   numBytes)   /* Input  number of bytes to allocate */
{
    PGSMEMLinkList   *newnode = (PGSMEMLinkList *)NULL;
    PGSMEMLinkList   *node    = (PGSMEMLinkList *)NULL;
    PGSt_SMF_status   returnStatus = PGS_S_SUCCESS;    

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status lockRet;

    /* lock all memory management */
    lockRet = PGS_TSF_LockIt(PGSd_TSF_MEMLOCK);
    if (PGS_SMF_TestErrorLevel(lockRet))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
#endif

    /*
     * Check if memory exceed limit.
     */
    if ((returnStatus = PGS_MEM_TrackUsage(numBytes,"PGS_MEM_Malloc()")) == PGS_S_SUCCESS)
    {
        /*
         * Check if memory address exists in the link list.
         */
        if (PGS_MEM_LinkMemberExist(*addr,&node) == PGS_TRUE)
        {
            PGS_SMF_SetStaticMsg(PGSMEM_W_MEMORY_USED,"PGS_MEM_Malloc()"); 
            returnStatus = PGSMEM_W_MEMORY_USED;
        }
        else
        {
            *addr = malloc(numBytes);

            if (*addr == (char *)NULL)
            {
                PGS_SMF_SetStaticMsg(PGSMEM_E_NO_MEMORY,"PGS_MEM_Malloc()");        
                returnStatus = PGSMEM_E_NO_MEMORY;       
            }                
            else
            {       
                if ((returnStatus = PGS_MEM_LinkNewNode(*addr,&numBytes,&newnode,"PGS_MEM_Malloc()")) == PGS_S_SUCCESS)
                {
                    PGS_MEM_LinkAddNode(newnode);
                }
            }
        }       
    }

#ifdef _PGS_THREADSAFE
    /* unlock memory management */
    lockRet = PGS_TSF_UnlockIt(PGSd_TSF_MEMLOCK);
#endif

    if (returnStatus == PGS_S_SUCCESS) 
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_MEM_Malloc()");
    }


    return(returnStatus);

} /* end PGS_MEM_Malloc */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_Calloc

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>                                                        

    PGSt_SMF_status 
    PGS_MEM_Calloc(
        void   **addr,
        size_t   num_elems,         
        size_t   elem_size);

FORTRAN:
    NONE

DESCRIPTION:
    Allocates an arbitrary number of bytes in memory. All bytes
    of the allocated memory will be intialized to zero.

INPUTS:
    Name            Description                                Units        Min        Max

    num_elems       number of elements
    elem_size       size of the element in bytes

OUTPUTS:
    Name            Description                                Units        Min        Max

    addr            pointer to beginning address of the memory 
                    that has been allocated

RETURNS:        			
    PGS_S_SUCCESS
    PGSMEM_E_NO_MEMORY
    PGSMEM_W_MEMORY_USED
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:
    int              i;
    int             *intPtr = (int *)NULL;      
    PGSt_SMF_status  returnStatus;      

    returnStatus = PGS_MEM_Calloc((void **)&intPtr,10,sizeof(int));
    if (returnStatus == PGS_S_SUCCESS)
    {
        for (i=0 ; i < 10 ; i++)
        {
            intPtr[i] = i;
        }
    }

NOTES:
    This tool will control the amount of memory that may be allocated at any
    one time. You should call PGS_MEM_Free() to free the memory allocated
    once you are done using it; failure to do so may cause future memory
    allocation requests to fail within the same process.

    Because the Toolkit memory functions track memory usage, it is
    imperative that pointer variables, which have been freed, be
    initialized to NULL prior to re-use. As a reminder, ALL local
    pointer variables MUST be initialized to NULL prior to use.
    Failure to heed these warnings may result in anomalous behavior 
    within your process.   

REQUIREMENTS:
    PGSTK-1240

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_MEM_TrackUsage()
    PGS_SMF_SetStaticMsg()
    PGS_MEM_LinkNewNode()
    PGS_MEM_LinkAddNode()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()
    PGS_SMF_TestErrorLevel()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_Calloc(             /* Allocate memory */
    void   **addr,          /* Output pointer to beginning address of the memory that has been allocated */
    size_t   num_elems,     /* Input  number of elements */
    size_t   elem_size)     /* Input  size of the element in bytes */
{
    PGSMEMLinkList  *newnode = (PGSMEMLinkList *)NULL;
    PGSMEMLinkList  *node    = (PGSMEMLinkList *)NULL;
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;
    size_t           numBytes = num_elems * elem_size;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status lockRet;

    /* lock all memory management */
    lockRet = PGS_TSF_LockIt(PGSd_TSF_MEMLOCK);
    if (PGS_SMF_TestErrorLevel(lockRet))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
#endif


    /*
     * Check if memory exceed limit.
     */
    if ((returnStatus = PGS_MEM_TrackUsage(elem_size*num_elems,"PGS_MEM_Calloc()")) == PGS_S_SUCCESS)
    { 
        if (PGS_MEM_LinkMemberExist(*addr,&node) == PGS_TRUE)
        {
            PGS_SMF_SetStaticMsg(PGSMEM_W_MEMORY_USED,"PGS_MEM_Malloc()"); 
            returnStatus = PGSMEM_W_MEMORY_USED;
        }
        else
        {
            *addr = calloc(num_elems,elem_size);

            if (*addr == (char *)NULL)
            {
                PGS_SMF_SetStaticMsg(PGSMEM_E_NO_MEMORY,"PGS_MEM_Calloc()");        
                returnStatus = PGSMEM_E_NO_MEMORY;       
            }
            else
            {
                if ((returnStatus = PGS_MEM_LinkNewNode(*addr,&numBytes,&newnode,"PGS_MEM_Calloc()")) == PGS_S_SUCCESS)
                {
                    PGS_MEM_LinkAddNode(newnode);
                }            
            }        
        }
    }

#ifdef _PGS_THREADSAFE
    /* unlock memory management */
    lockRet = PGS_TSF_UnlockIt(PGSd_TSF_MEMLOCK);
#endif

    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_MEM_Calloc()");
    }               

    return(returnStatus);

} /* end PGS_MEM_Calloc */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_Realloc

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>                                                         

    PGSt_SMF_status 
    PGS_MEM_Realloc(
        void   **addr,
        size_t   newsize);

FORTRAN:
    NONE

DESCRIPTION:
    Reallocates the number of bytes requested.

INPUTS:
    Name            Description                                Units        Min        Max

    addr            pointer to the starting address of 
                    previously allocated memory
    newsize         new total memory size to reallocate

OUTPUTS:    
    Name            Description                                Units        Min        Max

    addr            pointer to starting address of newly 
                    allocated memory

RETURNS:        			
    PGS_S_SUCCESS
    PGSMEM_E_NO_MEMORY
    PGSMEM_E_ADDR_NOTALLOC
    PGSTSF_E_GENERAL_FAILURE

EXAMPLES:
    int              i;
    int             *intPtr = (int *)NULL;      
    PGSt_SMF_status  returnStatus;   

    returnStatus = PGS_MEM_Calloc((void **)&intPtr,10,sizeof(int));
    if (returnStatus == PGS_S_SUCCESS)
    {
        for (i=0 ; i < 10 ; i++)
        {
            intPtr[i] = i;
        }
    }

    returnStatus = PGS_MEM_Realloc((void **)&intPtr,sizeof(int)*20);    
    if (returnStatus == PGS_S_SUCCESS)
    {
        # Realloc success #
    }

NOTES:    
    This tool will control the amount of memory that needs to be reallocated to
    a pointer which has already been used to obtain an initial allocation of memory
    through one of the available Toolkit routines. You should call PGS_MEM_Free()
    to deallocate the memory once you are done using it; failure to do so may
    cause future memory allocation requests to fail within the same process.

    Because the Toolkit memory functions track memory usage, it is
    imperative that pointer variables, which have been freed, be
    initialized to NULL prior to re-use. As a reminder, ALL local
    pointer variables MUST be initialized to NULL prior to use.
    Failure to heed these warnings may result in anomalous behavior 
    within your process.       

REQUIREMENTS:
    PGSTK-1240

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_MEM_Malloc()
    PGS_MEM_LinkMemberExist()
    PGS_MEM_TrackUsage()
    PGS_SMF_SetStaticMsg()
    PGS_MEM_LinkReplaceAddr()
    PGS_SMF_GetMsgByCode()
    PGS_SMF_SetDynamicMsg()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()
    PGS_SMF_TestErrorLevel()

END_PROLOG:
*****************************************************************/
PGSt_SMF_status 
PGS_MEM_Realloc(       /* Re-allocate memory */
    void   **addr,     /* Input  pointer to starting address of allocated memory */
                       /* Output pointer to starting address of allocated memory */
    size_t   newsize)  /* Input  new total memory size to reallocate */
{
    PGSMEMLinkList  *node  = (PGSMEMLinkList *)NULL;
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;
    char             msg[PGS_SMF_MAX_MSG_SIZE];
    char             buf[PGS_SMF_MAX_MSG_SIZE];
    void            *newaddr = (void *)NULL;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status lockRet;
#endif


    if (*addr == (void *)NULL)
    {
        returnStatus = PGS_MEM_Malloc(addr,newsize);
    }
    else
    {
#ifdef _PGS_THREADSAFE
        /* lock all memory management */
        lockRet = PGS_TSF_LockIt(PGSd_TSF_MEMLOCK);
        if (PGS_SMF_TestErrorLevel(lockRet))
        {
            return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
        /*
         * Check if memory address exists in the link list.
         */
        if (PGS_MEM_LinkMemberExist(*addr,&node) == PGS_TRUE)
        {
            if ((newsize > node->size) &&
                ((returnStatus = PGS_MEM_TrackUsage(newsize - node->size,"PGS_MEM_Realloc()")) == PGS_S_SUCCESS))
            {            
                newaddr = realloc(*addr,newsize);

                if (newaddr == (void *)NULL)
                {
                    PGS_SMF_SetStaticMsg(PGSMEM_E_NO_MEMORY,"PGS_MEM_Realloc()");        
                    returnStatus = PGSMEM_E_NO_MEMORY;   
                }                                
                else
                {
                    PGS_MEM_LinkReplaceAddr(&node,newaddr,&newsize);
                    *addr = newaddr;
                    returnStatus = PGS_S_SUCCESS;
                }
            }
        }                
        else
        {
            PGS_SMF_GetMsgByCode(PGSMEM_E_ADDR_NOTALLOC,msg);
            sprintf(buf,msg,*addr);
            PGS_SMF_SetDynamicMsg(PGSMEM_E_ADDR_NOTALLOC,msg,"PGS_MEM_Realloc()");
            returnStatus = PGSMEM_E_ADDR_NOTALLOC;
        }        
#ifdef _PGS_THREADSAFE
        /* unlock memory management */
        lockRet = PGS_TSF_UnlockIt(PGSd_TSF_MEMLOCK);
#endif
    }

    if (returnStatus == PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(PGS_S_SUCCESS,"PGS_MEM_Realloc()");
    }


    return(returnStatus);

} /* end PGS_MEM_Realloc */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_Zero

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>                                               

    void 
    PGS_MEM_Zero(
        void   *addr,
        size_t  numBytes);

FORTRAN:
    NONE

DESCRIPTION:
    Initializes a memory block or structure to zero.

INPUTS:    
    Name            Description                                Units        Min        Max

    addr            beginning address of the memory block 
                    or structure
    numBytes        number of bytes

OUTPUTS:     
    Name            Description                                Units        Min        Max 

RETURNS:
    NONE

EXAMPLES:
    typedef struct
    {
        int   i;
        char  c;            
        float f;
    }TestStruct;

    TestStruct     test;
    int           *intPtr = (int *)NULL;   
    returnStatus   returnStatus;  

    PGS_MEM_Zero(&test,sizeof(test));
    returnStatus = PGS_MEM_Malloc((void **)&intPtr,sizeof(int)*10);
    if (returnStatus == PGS_S_SUCCESS)        
    {
        PGS_MEM_Zero(intPtr,sizeof(intPtr)*10);
    }

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-1240

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
PGS_MEM_Zero(          /* Initialize block of memory to zero */
    void   *addr,      /* Input beginning address of the memory block or structure */
    size_t  numBytes)  /* Input number of bytes */
{
    char *p;


    if ((addr != (void *)NULL) && (numBytes > 0))                
    {
        for (p =(char *) addr ; p < (char *)addr + numBytes ; p++)
        {
            *p = '\0';
        }
    }


} /* end PGS_MEM_Zero */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_Free

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>

    void
    PGS_MEM_Free(
        void *addr);

FORTRAN:
    NONE

DESCRIPTION:
    Deallocates memory that was previously allocated through the use of
    a Toolkit allocation routine.

INPUTS:
    Name            Description                                Units        Min        Max

    addr            address of previously allocated memory

OUTPUTS:
    Name            Description                                Units        Min        Max

RETURNS:
    NONE

EXAMPLES:        
    int           *intPtr = (int *)NULL; 
    returnStatus   returnStatus;      

    returnStatus = PGS_MEM_Malloc((void **)&intPtr,sizeof(int)*10);       
    if (returnStatus == PGS_S_SUCCESS)        
    {
        PGS_MEM_Free(intPtr);
        intPtr = (int *)NULL;
    }

NOTES:
    Because the Toolkit memory functions track memory usage, it is
    imperative that pointer variables, which have been freed, be
    initialized to NULL prior to re-use. As a reminder, ALL local
    pointer variables MUST be initialized to NULL prior to use.
    Failure to heed these warnings may result in anomalous behavior 
    within your process.  

REQUIREMENTS:
    PGSTK-1240

DETAILS:
    Refer to PGS_MEM_Malloc() for further information.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_MEM_LinkMemberExist()
    PGS_MEM_LinkDelNode()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()
    PGS_SMF_TestErrorLevel()

END_PROLOG:
*****************************************************************/
void 
PGS_MEM_Free(    /* Deallocate allocated memory */ 
    void *addr)  /* Input address of previously allocated memory */
{
    PGSMEMLinkList *node = (PGSMEMLinkList *)NULL;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status lockRet;

    /* lock all memory management */
    lockRet = PGS_TSF_LockIt(PGSd_TSF_MEMLOCK);
    if (PGS_SMF_TestErrorLevel(lockRet))
    {
        return;
    }
#endif

    if ((addr != (void *)NULL) && (pgsmem_node_cnt != 0))
    {
        /*
         * Check if memory address exists in the link list. 
         */
        if (PGS_MEM_LinkMemberExist(addr,&node) == PGS_TRUE)
        {
            PGS_MEM_LinkDelNode(&node);
        }                
    }

    PGS_MEM_LinkPrintAll();

#ifdef _PGS_THREADSAFE
    /* unlock memory management */
    lockRet = PGS_TSF_UnlockIt(PGSd_TSF_MEMLOCK);
#endif

} /* end PGS_MEM_Free */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_FreeAll

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>                                                 

    void
    PGS_MEM_FreeAll(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    Deallocates all memory that was previously allocated, through the use of
    Toolkit allocation routines, within an executable. Calls
    to PGS_MEM_Free() and PGS_MEM_FreeAll() may be interlaced.

INPUTS:
    Name            Description                                Units        Min        Max

OUTPUTS:
    Name            Description                                Units        Min        Max

RETURNS:
    NONE

EXAMPLES:
    typedef struct
    {
        int   i;
        char  c;            
        float f;
    }TestStruct;

    TestStruct *test   = (TestStruct *)NULL;
    int        *intPtr = (int *)NULL;

    PGS_MEM_Malloc((void **)&intPtr,sizeof(int)*10);
    PGS_MEM_Malloc((void **)&test,sizeof(TestStruct)*10);
    PGS_MEM_FreeAll();   

    test   = (TestStruct *)NULL;
    intPtr = (int *)NULL;           

NOTES:
    In general, this tool should only be called near the end of processing, or when
    no further allocation of dynamic memory will be required.

    Due to the comprehensive nature of this tool, all allocated memory references,
    which have not yet been freed, will be disposed of. Because the Toolkit memory
    functions track memory usage, it is imperative that pointer variables, which
    have been freed, be initialized to NULL prior to use. Failure to heed these
    warnings may result in anomalous behavior within your process.

REQUIREMENTS:
    PGSTK-1240

DETAILS:
    Refer to PGS_MEM_Malloc() for further information.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_MEM_LinkDelNode()
    PGS_TSF_LockIt()
    PGS_TSF_UnlockIt()
    PGS_SMF_TestErrorLevel()

END_PROLOG:
*****************************************************************/
void 
PGS_MEM_FreeAll(  /* Deallocate all allocated memory within the process */
    void)
{    
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status lockRet;

    /* lock all memory management */
    lockRet = PGS_TSF_LockIt(PGSd_TSF_MEMLOCK);
    if (PGS_SMF_TestErrorLevel(lockRet))
    {
        return;
    }
#endif

    /*
     * Note that pgsmem_node_head always point to right most node in the linked list. 
     */
    while (pgsmem_node_head != (PGSMEMLinkList *)NULL)
    {
        PGS_MEM_LinkDelNode(&pgsmem_node_head);
    }

#ifdef _PGS_THREADSAFE
    /* unlock memory management */
    lockRet = PGS_TSF_UnlockIt(PGSd_TSF_MEMLOCK);
#endif

} /* end PGS_MEM_FreeAll */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_LinkNewNode

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>    

    static PGSt_SMF_status
    PGS_MEM_LinkNewNode(
        void            *addr,
        size_t          *numBytes,
        PGSMEMLinkList **node,
        char            *funcName);

FORTRAN:
    NONE

DESCRIPTION:
    Create new node to be inserted into the link-list.

INPUTS:    
    Name            Description                                Units        Min        Max

    addr            allocated memory address to be inserted 
                    into the node
    numBytes        size of addr in bytes
    funcName        routine calling this function

OUTPUTS:    
    Name            Description                                Units        Min        Max

    node            newly created node 

RETURNS:
    PGS_S_SUCCESS
    PGSMEM_E_NO_MEMORY

EXAMPLES:
    PGSMEMLinkList *newnode;
    int            *intPtr;
    size_t          numBytes = sizeof(int) * 10;

    PGS_MEM_Calloc(&intPtr,10,sizeof(int));
    PGS_MEM_LinkNewNode(intPtr,&numBytes,&newnode,"funcName()");
    PGS_MEM_LinkAddNode(newnode);

REQUIREMENTS:
    PGSTK-1240

DETAILS:
    NONE           

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_SetStaticMsg()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_status
PGS_MEM_LinkNewNode(            /* Create new node */
    void            *addr,      /* Input  allocated memory address to be inserted into the node */
    size_t          *numBytes,  /* Input  size of addr in bytes */
    PGSMEMLinkList **node,      /* Output newly created node */
    char            *funcName)  /* Input  routine calling this function */
{   
    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;


    *node = (PGSMEMLinkList *) calloc(1,sizeof(PGSMEMLinkList));
    if (*node != (PGSMEMLinkList *)NULL)
    {
        (*node)->addr = addr;
        (*node)->size = *numBytes;
        (*node)->next = (PGSMEMLinkList *)NULL;
        (*node)->prev = (PGSMEMLinkList *)NULL;
    }
    else
    {
        PGS_SMF_SetStaticMsg(PGSMEM_E_NO_MEMORY,funcName);        
        returnStatus = PGSMEM_E_NO_MEMORY;   
    }

    return(returnStatus);

} /* end PGS_MEM_LinkNewNode */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_LinkAddNode

SYNOPSIS:       
C:  
    #include <PGS_MEM.h> 

    static void 
    PGS_MEM_LinkAddNode(
        PGSMEMLinkList *node);

FORTRAN:
    NONE

DESCRIPTION:
    To add new node into the link-list.

INPUTS:    
    Name            Description                                Units        Min        Max

    node            newly created node

OUTPUTS:    
    Name            Description                                Units        Min        Max

RETURNS:
    NONE       

EXAMPLES:
    PGSMEMLinkList *newnode;
    int            *intPtr;
    size_t          numBytes = sizeof(int) * 10;

    PGS_MEM_Calloc(&intPtr,10,sizeof(int));
    PGS_MEM_LinkNewNode(intPtr,&numBytes,&newnode,"funcName()");
    PGS_MEM_LinkAddNode(newnode);        

REQUIREMENTS:
    PGSTK-1240

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
static void 
PGS_MEM_LinkAddNode(         /* Add node to link-list */
    PGSMEMLinkList *node)    /* Input newly created node */
{
    PGSMEMLinkList *curr_node;


    if (pgsmem_node_head == (PGSMEMLinkList *)NULL)
    {
        pgsmem_node_head = node;
    }
    else
    {
        /*
         * Note that pgsmem_node_head always point to the right most node in the linked list. 
         */
        curr_node = pgsmem_node_head;

        pgsmem_node_head = node;   /* set pointer to latest node */
        pgsmem_node_head->prev = curr_node;

        curr_node->next = node;   /* set previous node to point to this latest node */
    }

    pgsmem_node_cnt++;


} /* end PGS_MEM_LinkAddNode */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_LinkPrintNode

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>

    static void 
    PGS_MEM_LinkPrintNode(
        PGSMEMLinkList *node);

FORTRAN:
    NONE

DESCRIPTION:
    To print the contents of a given node in the link-list.

INPUTS:    
    Name            Description                                Units        Min        Max

    node            node to print

OUTPUTS:      
    Name            Description                                Units        Min        Max      			

RETURNS:
    NONE

EXAMPLES:       
    PGSMEMLinkList *newnode;
    int            *intPtr;
    size_t          numBytes = sizeof(int) * 10;

    PGS_MEM_Calloc(&intPtr,10,sizeof(int));
    PGS_MEM_LinkNewNode(intPtr,&numBytes,&newnode,"funcName()");
    PGS_MEM_LinkAddNode(newnode); 
    PGS_MEM_LinkPrintNode(newnode);               

REQUIREMENTS:
    PGSTK-1240

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
static void 
PGS_MEM_LinkPrintNode(     /* Print contents of a node */
    PGSMEMLinkList *node)  /* Input node to print */
{

} /* end PGS_MEM_LinkPrintNode */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_LinkPrintAll

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>

    static void 
    PGS_MEM_LinkPrintAll(
        void);

FORTRAN:
    NONE

DESCRIPTION:
    To print the contents of all the nodes in the link-list.

INPUTS:     
    Name            Description                                Units        Min        Max       

OUTPUTS:     
    Name            Description                                Units        Min        Max 

RETURNS:
    NONE    

EXAMPLES:
    PGSMEMLinkList *newnode;
    int            *intPtr;
    size_t          numBytes = sizeof(int) * 10;

    PGS_MEM_Calloc(&intPtr,10,sizeof(int));
    PGS_MEM_LinkNewNode(intPtr,&numBytes,&newnode,"funcName()");
    PGS_MEM_LinkAddNode(newnode); 
    PGS_MEM_LinkPrintNode(newnode); 

REQUIREMENTS:
    PGSTK-1240

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_MEM_LinkPrintNode()

END_PROLOG:
*****************************************************************/
static void 
PGS_MEM_LinkPrintAll(  /* Print all nodes in the link-list */
    void)
{
    PGSMEMLinkList *node_curr = (PGSMEMLinkList *)NULL;
    PGSMEMLinkList *node      = (PGSMEMLinkList *)NULL;


    /*
     * Note that pgsmem_node_head always point to right most node in the linked list 
     */
    for (node_curr = pgsmem_node_head ; node_curr != (PGSMEMLinkList *)NULL ; node_curr = node)
    {
        PGS_MEM_LinkPrintNode(node_curr);
        node = node_curr->prev;
    }


} /* end PGS_MEM_LinkPrintAll */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_LinkReplaceAddr

SYNOPSIS:       		
C:  
    #include <PGS_MEM.h>

    static void 
    PGS_MEM_LinkReplaceAddr(
        PGSMEMLinkList **node,
        void            *addr,
        size_t          *numBytes);

FORTRAN:
    NONE

DESCRIPTION:
    To replace old allocated memory address with the new re-allocated
    memory address. This routine is used in conjunction with
    PGS_MEM_Realloc() routine.

INPUTS:    
    Name            Description                                Units        Min        Max

    node            node that needs to update with new address
    addr            newly allocated address
    numBytes        size of add in bytes

OUTPUTS:      
    Name            Description                                Units        Min        Max  

RETURNS:
    NONE

EXAMPLES:     
    PGSMEMLinkList *newnode;
    int            *intPtr;
    size_t          numBytes = sizeof(int) * 10;

    PGS_MEM_Calloc(&intPtr,10,sizeof(int));
    PGS_MEM_LinkNewNode(intPtr,&numBytes,&newnode,"funcName()");
    PGS_MEM_LinkAddNode(newnode); 
    PGS_MEM_Realloc(&intPtr,20*sizeof(int));    

    numBytes = sizeof(int) * 20;
    PGS_MEM_LinkReplaceAddr(&newnode,intPtr,&newBytes);

REQUIREMENTS:              
    PGSTK-1240

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
static void 
PGS_MEM_LinkReplaceAddr(        /* Replace old allocated address with new address */
    PGSMEMLinkList **node,      /* Input node that needs to update with new address */
    void            *addr,      /* Input newly allocated address */
    size_t          *numBytes)  /* Input size of addr in bytes */
{

    (*node)->addr = addr;
    (*node)->size = *numBytes;


} /* end PGS_MEM_LinkReplaceAddr */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_LinkMemberExist

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>

    static PGSt_SMF_boolean 
    PGS_MEM_LinkMemberExist(
        void           *addr,
        PGSMEMLinkList *node);

FORTRAN:
    NONE

DESCRIPTION:
    To check if the allocated memory address exist in the linked list.

INPUTS:    
    Name            Description                                Units        Min        Max

    addr            address to be checked

OUTPUTS:    
    Name            Description                                Units        Min        Max

    node            node containing the address to check

RETURNS:
    PGS_TRUE
    PGS_FALSE

EXAMPLES:             
    PGSMEMLinkList *newnode;
    PGSMEMLinkList *node;
    int            *intPtr;
    size_t          numBytes = sizeof(int) * 10;

    PGS_MEM_Calloc(&intPtr,10,sizeof(int));
    PGS_MEM_LinkNewNode(intPtr,&numBytes,&newnode,"funcName()");
    PGS_MEM_LinkAddNode(newnode); 
    PGS_MEM_Realloc(&intPtr,20*sizeof(int));    

    numBytes = sizeof(int) * 20;
    PGS_MEM_LinkReplaceAddr(&newnode,intPtr,&newBytes);
    PGS_MEM_LinkMemberExist(intPtr,&node);

REQUIREMENTS:              
    PGSTK-1240

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
static PGSt_SMF_boolean 
PGS_MEM_LinkMemberExist(    /* Check address in the node of the link-list */
    void            *addr,  /* Input  address to be checked */
    PGSMEMLinkList **node)  /* Output node containing the address to check */
{
    PGSMEMLinkList   *node_curr  = (PGSMEMLinkList *)NULL;
    PGSMEMLinkList   *node_prev  = (PGSMEMLinkList *)NULL;
    PGSt_SMF_boolean  returnBoolean = PGS_FALSE;


    for (node_curr = pgsmem_node_head ; node_curr != (PGSMEMLinkList *)NULL ; node_curr = node_prev)
    {
        if (node_curr->addr == addr)
        {
            returnBoolean = PGS_TRUE;
            *node = node_curr;
            break;
        }
        else
        {
            node_prev = node_curr->prev;
        }
    }

    return(returnBoolean);

} /* end PGS_MEM_LinkMemberExist */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_LinkDelNode

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>

    static void 
    PGS_MEM_LinkDelNode(
        PGSMEMLinkList **node);

FORTRAN:
    NONE

DESCRIPTION:
    To delete a node from the linked list.

INPUTS:    
    Name            Description                                Units        Min        Max

    node            pointer to the node

OUTPUTS:    
    Name            Description                                Units        Min        Max   

RETURNS:
    NONE

EXAMPLES:
    PGSMEMLinkList *newnode;
    PGSMEMLinkList *node;
    int            *intPtr;
    size_t          numBytes = sizeof(int) * 10;

    PGS_MEM_Calloc(&intPtr,10,sizeof(int));
    PGS_MEM_LinkNewNode(intPtr,&numBytes,&newnode,"funcName()");
    PGS_MEM_LinkAddNode(newnode); 
    PGS_MEM_Realloc(&intPtr,20*sizeof(int));    

    numBytes = sizeof(int) * 20;
    PGS_MEM_LinkReplaceAddr(&newnode,intPtr,&newBytes);
    PGS_MEM_LinkMemberExist(intPtr,&node);
    PGS_MEM_LinkDelNode(&node);

REQUIREMENTS:             
    PGSTK-1240

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
static void 
PGS_MEM_LinkDelNode(        /* Delete a node from the link-list */
    PGSMEMLinkList **node)  /* Input pointer to the node */
{
    PGSMEMLinkList *node_prev;
    PGSMEMLinkList *node_next;


    if (*node == pgsmem_node_head)
    {
        if (pgsmem_node_cnt == 1)
        {
            free((*node)->addr);
            free(*node);

            pgsmem_node_head = (PGSMEMLinkList *)NULL;
        }
        else
        { 
            node_prev = (*node)->prev;   

            free((*node)->addr);
            free(*node);

            pgsmem_node_head = node_prev;
            pgsmem_node_head->next = (PGSMEMLinkList *)NULL;
        }
    }
    else
    {
        node_prev = (*node)->prev;
        node_next = (*node)->next;

        if (node_prev != (PGSMEMLinkList *)NULL)
        {
            node_prev->next = node_next;
        }

        node_next->prev = node_prev;

        free((*node)->addr);
        free(*node);
    }

    pgsmem_node_cnt--;


} /* end PGS_MEM_LinkDelNode */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Memory Management

NAME:   
    PGS_MEM_TrackUsage

SYNOPSIS:       
C:  
    #include <PGS_MEM.h>

    static PGSt_SMF_status
    PGS_MEM_TrackUsage(
        size_t  numBytes,
        char   *funcName)

FORTRAN:
    NONE

DESCRIPTION:
    To keep track on how much memory has been allocated for the 
    current process.

INPUTS:    
    Name            Description                                Units        Min        Max

    numBytes        number of bytes to allocate
    funcName        function invoking this routine 

OUTPUTS:
    Name            Description                                Units        Min        Max       	

RETURNS:
    PGS_S_SUCCESS        
    PGSMEM_E_MAXSIZE

EXAMPLES:
    PGS_MEM_TrackUsage(1000,"funcCaller()");

NOTES:  
    Need to know how I can access the maximum (limit) memory usage allowed for 
    a given process. This limit is machine dependent; thus assumption is that
    system management will provide us the limit for memory allocation for a given 
    process.          

    Note, as of 7/12/94, Planning & Processing Subsystem (PPS) has no plans for
    limiting memory usage through the toolkit. 

REQUIREMENTS:             
    PGSTK-1240

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_GetMsgByCode()
    PGS_SMF_SetDynamicMsg()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_status
PGS_MEM_TrackUsage(     /* Track total memory usage */
    size_t  numBytes,   /* Input number of bytes to allocate */
    char   *funcName)   /* Input function invoking this routine */
{

    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;

/*******      
    PGSMEMLinkList  *node_curr  = (PGSMEMLinkList *)NULL;
    PGSMEMLinkList  *node_prev  = (PGSMEMLinkList *)NULL;
    size_t           totalBytes = 0;    
    char             msg[PGS_SMF_MAX_MSG_SIZE];
    char             buf[PGS_SMF_MAX_MSG_SIZE];



    NOTE: IF LATER PPS DECIDE TO LIMIT MEMORY USAGE PER PROCESS, THEN UNCOMMENT THIS MODULE.
    ========================================================================================    

    for (node_curr = pgsmem_node_head ; node_curr != (PGSMEMLinkList *)NULL ; node_curr = node_prev)
    {        
        totalBytes += node_curr->size;
        node_prev = node_curr->prev;

    }

    if (totalBytes + numBytes + sizeof(PGSMEMLinkList) * (pgsmem_node_cnt + 1) > PPS_LIMIT)
    {
        PGS_SMF_GetMsgByCode(PGSMEM_E_MAXSIZE,msg);
        sprintf(buf,msg,limit);        
        PGS_SMF_SetDynamicMsg(PGSMEM_E_MAXSIZE,buf,funcName);        
        returnStatus = PGSMEM_E_MAXSIZE;
    }


********/

    return(returnStatus);


} /* end PGS_MEM_TrackUsage */



