/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_MEM_ShmReadF.c

DESCRIPTION:
   This file contains the function PGS_MEM_ShmReadF().
   This function copies the contents of shared memory into a user allocated
   (may be dynamically or statically allocated) memory area.

AUTHOR:
   Guru Tej S. Khalsa /	Applied Research Corp.

HISTORY:
   26-Mar-1994  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:  
    Read From Shared Memory

NAME:   
    PGS_MEM_ShmReadF

SYNOPSIS:       
C:  
    #include <PGS_MEM1.h>                                                  

    PGSt_SMF_status
    PGS_MEM_ShmReadF(
        void*        mem_ptr,
	PGSt_integer size);

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_MEM_9.f'

      integer function pgs_mem_shmread(mem_ptr, size)
      integer   size
      character mem_ptr(size)

DESCRIPTION:
      This function copies the contents of shared memory into a user allocated
      (may be dynamically or statically allocated) memory area.  This function
      is meant to be used by FORTRAN (77/90) users who cannot take advantage of
      the C shared memory tools PGS_MEM_ShmAttach() and PGS_MEM_ShmDetach().

INPUTS:        
    NAME         DESCRIPTION
    ----         -----------
    mem_ptr      pointer to memory area to which the contents of the shared
                 memory area will be written

    size         size (in bytes) of the memory area pointed to by mem_ptr

OUTPUTS:        
    NAME         DESCRIPTION
    ----         -----------
    mem_ptr      pointer to memory area to which the contents of the shared
                 memory area will be written

RETURNS:
    PGS_S_SUCCESS
    PGS_E_UNIX
    PGSMEM_E_SHM_ENV
    PGSMEM_E_SHM_NOTCREATE
    PGSMEM_E_SHM_MULTIATTACH    
    PGSMEM_E_SHM_NOTATTACH    

EXAMPLES:
 FORTRAN:
      integer    pgs_mem_shmread
      integer    size
      character  shm_buffer(1000)
      integer    returnstatus

      returnstatus = pgs_mem_shmread(shm_buffer, size)
      if (returnstatus .ne. pgs_s_success) goto 999

!  the contents of shared memory (which may contain data from
!  a previous process) has been copied to shm_buffer

 999  continue  ! process errors conditions

NOTES:
    This tool is meant to be used by FORTRAN (77/90) users ONLY.  C users should
    use the functions PGS_MEM_ShmAttach() and PGS_MEM_ShmDetach().

    This tool is not part of POSIX and is subjected to change when the POSIX.4
    implementation becomes available. This function will only detach the shared
    memory segment from the process. The shared memory segment will not be
    removed from the system by calling this tool; therefore one can re-attach it
    again.

REQUIREMENTS:
    PGSTK-1241

DETAILS:
    The user passes in a pointer to a user defined memory area (an area of
    memory which has been either statically or dynamically allocated by the
    user) and the size of that area.  This function will retrieve the pointer
    to the shared memory area and copy the contents of the shared memory into
    the users memory area. This function will then detach the shared memory from
    the current process.  Before exiting from the PGE, the system will make sure
    that the attached shared memory segment will be removed from the system.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:   
    PGS_MEM_ShmAttach()
    PGS_MEM_ShmDetach()

END_PROLOG:
*******************************************************************************/

#include <PGS_MEM1.h>

PGSt_SMF_status
PGS_MEM_ShmReadF(                     /* read from shared memory */
    void             *mem_ptr,        /* memory area to write to */
    PGSt_integer     size)            /* size of memory area to write to */
{
    void             *shared_mem_ptr; /* pointer to shared memory region */
    PGSt_SMF_status  returnStatus;    /* return value of this function */
    
    /* retrieve pointer to shared memory */

    returnStatus = PGS_MEM_ShmAttach(&shared_mem_ptr);
    if (returnStatus == PGS_S_SUCCESS)
    {
	/* copy contents of shared memory to input buffer and then detach shared
	   memory */

	memcpy(mem_ptr, shared_mem_ptr, size);
	returnStatus = PGS_MEM_ShmDetach();
    }

    return returnStatus;
}
