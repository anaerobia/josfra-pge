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
!     PGS_IO_Gen_Track_LUN.f
! 
! DESCRIPTION:
!     The tool listed in this file provides support for the FORTRAN 
!     generic file access tools.
! 
! AUTHORS:
!     Mike Sucher / Applied Research Corp.
! 
! HISTORY:
!     01-Aug-1994 MES Initial version
!     24-Aug-1994 MES Fixed up comments, prolog
!     26-Aug-1994 MES Remove obsolete FORTRAN SMF header files.
!     08-Dec-1995 MES Use a parameter to set maximum number of open LUN's,
!                     instead of hard-coding, to simplify future updates.
!                     Increase maximum open LUN's from 99 to 2500.
! 
! END_FILE_PROLOG
!***************************************************************************

!****************************************************************************
! BEGIN_PROLOG
! 
! TITLE: 
!     Allocate or deallocate FORTRAN I/O logical unit number
!  
! NAME: 
!     PGS_IO_Gen_Track_LUN
! 
! SYNOPSIS:
!   C:    (not applicable)
! 
!   FORTRAN:
!
!     INCLUDE   'PGS_SMF.f'
!     INCLUDE   'PGS_IO.f'
!     INCLUDE   'PGS_IO_1.f'
!
!     integer function pgs_io_gen_track_lun(lun, cmd)
!     integer lun
!     integer cmd
!
! DESCRIPTION:  
!     This function allocates and deallocates FORTRAN logical unit numbers
!     for use by the FORTRAN versions of the PGS Generic Open and Close
!     tools.  It keeps track of which logical unit numbers are in use,
!     ensuring that successive calls the Generic Open tools do not 
!     inadvertently use the same logical unit number.
! 
! INPUTS:               
!     lun             - logical unit number to deallocate (cmd=free)
!     cmd             - command (0=allocate, 1=free)
! 
!     file_logical    - User defined logical file identifier
!     
!     file_access     - type of access granted to opened file:
!                         PGSd_IO_Gen_Write
!                         PGSd_IO_Gen_Read
!                         PGSd_IO_Gen_Append
!                         PGSd_IO_Gen_Update
!                         PGSd_IO_Gen_Trunc
!                         PGSd_IO_Gen_AppendUpdate
! 
! OUTPUTS:      
!     lun             - logical unit number allocated (cmd=allocate)
! 
! RETURNS: 
!     PGS_S_SUCCESS
!     PGSIO_E_GEN_NO_FREE_LUN      no free LUN available
!     PGSIO_E_GEN_ILLEGAL_LUN      specified LUN is out of range
!     PGSIO_W_GEN_UNUSED_LUN       specified LUN was not in use
! 
! EXAMPLE:
! 
! NOTES:                
!     This is an internal support routine called by all PGS file open and 
!     close tools that return logical unit numbers for use in standard
!     FORTRAN I/O statements.  Because it is internal, it does not use
!     the PGS Toolkit return convention.
! 
! REQUIREMENTS: 
!       PGSTK-0360
! 
! DETAILS:      
!     This routine maintains a static table of allocated LUNs. LUNs 1-9
!     are reserved for use by FORTRAN, 10-99 are available.  When called
!     in allocate mode, the table is searched, and the first free LUN is
!     marked as allocated and return in parameter 'lun'.
!
! FUNCTIONS_CALLED:
!     NONE
!
! END_PROLOG
!***************************************************************************

      integer function pgs_io_gen_track_lun(lun, cmd)

      implicit none
      save lun_list

! Include header file definitions

      INCLUDE   'PGS_SMF.f'
      INCLUDE   'PGS_IO.f'
      INCLUDE   'PGS_IO_1.f'

! Set maximum number of open LUN's

      integer PGSd_IO_Gen_MaxLun
      parameter (PGSd_IO_Gen_MaxLun = 2500)


! parameters:
!     name    in/out    description
!     ====    ======    ===========
!     lun     in/out     logical unit number 
!     cmd     in         command 

      integer lun
      integer cmd

! misc local variables:
!     name        description
!     ====        ===========
!     i           loop counter

      integer i


! static local variables:
!     name        description
!     ====        ===========
!     init_flag   initialization flag (1 on first call, 0 thereafter)
!     lun_list    logical unit number free list (0=free, 1=in use)

      integer init_flag
      integer lun_list(PGSd_IO_Gen_MaxLun)

      data init_flag/1/


! set default return status 

      pgs_io_gen_track_lun = pgs_s_success


! initializations;
! clear initialization flag
! reserve the first 9 LUN's for use by FORTRAN

      if (init_flag .eq. 1) then

         init_flag = 0

         do 10 i=1,9
            lun_list(i) = 1
 10      continue

         do 20 i=10,PGSd_IO_Gen_MaxLun
            lun_list(i) = 0
 20      continue

      endif


! carry out user command
      
      if (cmd .eq. 0) then

!     loop to find first free LUN and allocate it

         do 30 i=10,PGSd_IO_Gen_MaxLun

            if (lun_list(i) .ne. 0) goto 30
  
            if (i .lt. PGSd_IO_Gen_MaxLun) then
               lun_list(i) = 1
               lun = i
               return
            else
               pgs_io_gen_track_lun = pgsio_e_gen_no_free_lun
               return
            endif

 30      continue

      else

!     deallocate a used LUN

         if ((lun .ge. 10) .and. (lun .le. PGSd_IO_Gen_MaxLun)) then

            if (lun_list(lun) .ne. 0) then
               lun_list(lun) = 0
               return
            else
               pgs_io_gen_track_lun = pgsio_w_gen_unused_lun
               return
            endif
         else
            pgs_io_gen_track_lun = pgsio_e_gen_illegal_lun
            return
         endif
         
      endif
      
      return
      end


