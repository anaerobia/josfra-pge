!-------------------------------------------------------------------------!
!                                                                         !
!  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  !
!  and suppliers.  ALL RIGHTS RESERVED.                                   !
!                                                                         !
!-------------------------------------------------------------------------!
!***************************************************************************
! BEGIN_FILE_PROLOG
! 
! FILENAME:   
!     PGS_IO_Gen_CloseF.f
! 
! DESCRIPTION:
!     The tool listed in this file provides the FORTRAN user with generic
!     file access.
! 
! AUTHORS:
!     Mike Sucher / Applied Research Corp.
! 
! HISTORY:
!     01-Aug-1994 MES Initial version
!     11-Aug-1994 MES Fixes following code inspection
!     26-Aug-1994 MES Remove obsolete FORTRAN SMF header files.
!     18-Oct-1994 MES Fix to call SMF set message upon successful completion
!                     Update NOTES section of prolog
!     14-Jul-1995 DPH Remove prolog references to environment variables
! 
! END_FILE_PROLOG
!***************************************************************************

!****************************************************************************
! BEGIN_PROLOG
! 
! TITLE: 
!       Close a Generic File (FORTRAN version)
!  
! NAME: 
!       PGS_IO_Gen_CloseF
! 
! SYNOPSIS:
!   C:    (not applicable)
! 
!   FORTRAN:
!     INCLUDE   'PGS_SMF.f'
!     INCLUDE   'PGS_PC.f'
!     INCLUDE   'PGS_PC_9.f'
!     INCLUDE   'PGS_IO.f'
!     INCLUDE   'PGS_IO_1.f'
!
!     integer pgs_io_gen_closef(file_handle)
!     integer       file_handle
! 
! DESCRIPTION:  
!     This tool closes a FORTRAN IO unit opened by call to PGS_IO_Gen_OpenF
!     or PGS_IO_Gen_Temp_OpenF. 
! 
! 
! INPUTS:               
!     file_handle - file handle returned by PGS_IO_Gen_OpenF or 
!                   PGS_IO_Gen_Temp_OpenF
!
! OUTPUTS:      
!     NONE
! 
! RETURNS:
!     PGS_S_SUCCESS            Successful completion
!     PGSIO_E_GEN_CLOSE        Attempt to close file failed
!     PGSIO_E_GEN_ILLEGAL_LUN  file_handle LUN was out-of-bounds
!     PGSIO_W_GEN_UNUSED_LUN   file_handle LUN was not in use
! 
! EXAMPLES:
!     implicit none
!     include   'PGS_SMF.f'
!     include   'PGS_PC.f'
!     include   'PGS_PC_9.f'
!     include   'PGS_IO.f'
!     include   'PGS_IO_1.f'
!     integer pgs_io_gen_closef
!     integer handle
!     integer returnstatus
!
!     returnstatus =  pgs_io_gen_closef(handle)
!     if (returnstatus != PGS_S_SUCCESS) goto 100
!     
!     .
!     .
!     .
! 100 <error handling goes here>
! 
! NOTES:        
!     Failure to close a file could result in loss of data, destroyed
!     files, or possible intermittent errors in your program.
!
!     This tool expects the input file_handle to point to a file that was
!     successfully opened via a call to either the tool PGS_IO_Gen_OpenF  
!     or the tool PGS_IO_Gen_Temp_OpenF.  If this is not the case, the 
!     result of calling the tool is undefined.
!
! REQUIREMENTS: 
!     PGSTK-0360
! 
! DETAILS:      
!     This routine first  calls PGS_IO_Gen_Track_LUN to deallocate 
!     the logical unit number that was used for file access.  If the
!     LUN value is out-of-bounds or not marked as allocated, control
!     goes to exception handling. The tool then tries to close the file,
!     going to exception handling if it encounters an error.  
!
! GLOBALS:
!     NONE
! 
! FILES:
!     The file to be closed is determined by input parameter.
!
! FUNCTIONS_CALLED:
!     PGS_IO_Gen_Track_LUN       Deallocate logical unit number
!     PGS_SMF_SetStaticMsg()     Register error return with SMF
! 
! END_FUNCTION_PROLOG
!***************************************************************************

      integer function pgs_io_gen_closef(file_handle)
         
      implicit none

! Include header file definitions

      INCLUDE  'PGS_SMF.f'
      INCLUDE  'PGS_PC.f'
      INCLUDE  'PGS_PC_9.f'
      INCLUDE 'PGS_IO.f'
      INCLUDE 'PGS_IO_1.f'

!
! Parameters: 
!     in:  file handle LUN 
!
      integer       file_handle

!
! Functions Called
!

      integer pgs_io_gen_track_lun
      integer pgs_smf_setstaticmsg

!
! Local variables
!

!     lun_command - command for PGS_IO_Gen_Track_LUN

      integer         lun_command

!     temp status return for calls to Toolkit routines

      integer         tmp_status


!     ------------------------------------------------------
!     Initialize 
!     ------------------------------------------------------

!     default return status: success 
      pgs_io_gen_closef = PGS_S_SUCCESS


!     ------------------------------------------------------
!     set the command for PGS_IO_Gen_Track_LUN
!     then try to deallocate LUN
!     ------------------------------------------------------

      lun_command = 1
      tmp_status = PGS_IO_Gen_Track_LUN(file_handle, lun_command)

      if (tmp_status .EQ. PGSIO_E_GEN_ILLEGAL_LUN) then
!     ... failed: (LUN value was out of bounds)
         pgs_io_gen_closef = PGSIO_E_GEN_ILLEGAL_LUN
         goto 1000
      endif

      if (tmp_status .EQ. PGSIO_W_GEN_UNUSED_LUN) then
!     ... warning: (LUN value was not in use)
         pgs_io_gen_closef = PGSIO_W_GEN_UNUSED_LUN
         goto 1000
      endif

!     ------------------------------------------------------
!     Close the file
!     ------------------------------------------------------

      close (unit = file_handle, status = 'keep', err = 500)

!     ------------------------------------------------------
!     Operation was successful:  return
!     ------------------------------------------------------

      tmp_status = PGS_SMF_SetStaticMsg( pgs_io_gen_closef,
     +     'PGS_IO_Gen_CloseF' )
      return

!     ------------------------------------------------------
!     Close failures come here
!     ------------------------------------------------------

 500  pgs_io_gen_closef = PGSIO_E_GEN_CLOSE


!     ------------------------------------------------------
!     Exceptions jump to this point
!     ------------------------------------------------------

 1000 tmp_status = PGS_SMF_SetStaticMsg( pgs_io_gen_closef,
     +     'PGS_IO_Gen_CloseF' )

      return
      end



