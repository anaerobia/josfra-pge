!-------------------------------------------------------------------------!
!                                                                         !
!  COPYRIGHT[copyright mark] 1996, Hughes Aircraft Company, its vendors,  !
!  and suppliers.  ALL RIGHTS RESERVED.                                   !
!                                                                         !
!-------------------------------------------------------------------------!
!***************************************************************************
! BEGIN_FILE_PROLOG
! 
! FILENAME:   
!     PGS_IO_Gen_OpenF.f
! 
! DESCRIPTION:
!     The tool listed in this file provides the FORTRAN user with generic
!     file  access.
! 
! AUTHORS:
!     Mike Sucher / Applied Research Corp.
!     Peter D. Noerdlinger / SM&A, Inc.
! 
! HISTORY:
!     01-Aug-1994 MES Initial version
!     12-Aug-1994 MES Fixes following code inspection
!     24-Aug-1994 MES Removed TK 2.5 patch that forced version=1 
!     26-Aug-1994 MES Removed obsolete FORTRAN SMF header files.
!     06-Sep-1994 MES Upgraded for TK3: 'Support File' sections of PCF are
!                     now searched as well as the 'Product File' sections.
!     06-Oct-1994 MES Fixed logic bug in modes loop, like C version of tool.
!     18-Oct-1994 MES Updated NOTES section of prolog
!     20-Oct-1994 MES Updated prolog, moved label 1000 so that it traps the
!                     PGSPC_W_NO_FILES_EXIST return and converts it to the
!                     value PGSIO_E_GEN_FILE_NOEXIST.
!     14-Jul-1995 DPH Updating prolog to reflect discontinued use of
!                     environment variables.
!     01-Oct-1999 PDN Fixed error messaging to write user file handle
!                     (up to 6 digits) of any file that cannot be opened
!                     to the LogStatus file.
! 
! END_FILE_PROLOG
!***************************************************************************

!***************************************************************************
! BEGIN_PROLOG
! 
! TITLE: 
!       Open a Generic File (FORTRAN version)
!  
! NAME: 
!       PGS_IO_Gen_OpenF
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
!     integer function pgs_io_gen_openf(
!    +     file_logical, file_access, record_length,
!    +     file_handle, file_version)
!     
!     integer       file_logical
!     integer       file_access
!     integer       record_length
!     integer       file_handle
!     integer       file_version
! 
! DESCRIPTION:  
!     Upon a successful call, this function will allocate a logical unit 
!     number to support FORTRAN READ and WRITE statements.  This is 
!     returned to the user via the parameter file_handle. The user 
!     provides the logical file identifier and file version number,  
!     which internally get mapped to the associated physical file. 
! 
! INPUTS:               
!     file_logical - User defined logical file identifier
!     
!     file_access  - type of access granted to opened file:
!
!                          PGS            Rd/Wr/  FORTRAN 77   FORTRAN 77
!                   FORTRAN Access Mode   Update  'access='    'form='
!                   ===================   ======  ==========   ===========
!                   PGSd_IO_Gen_RSeqFrm   Read    Sequential   Formatted
!                   PGSd_IO_Gen_RSeqUnf   Read    Sequential   Unformatted
!                   PGSd_IO_Gen_RDirFrm   Read    Direct       Formatted
!                   PGSd_IO_Gen_RDirUnf   Read    Direct       Unformatted
!       
!                   PGSd_IO_Gen_WSeqFrm   Write   Sequential   Formatted
!                   PGSd_IO_Gen_WSeqUnf   Write   Sequential   Unformatted
!                   PGSd_IO_Gen_WDirFrm   Write   Direct       Formatted
!                   PGSd_IO_Gen_WDirUnf   Write   Direct       Unformatted
!       
!                   PGSd_IO_Gen_USeqFrm   Update  Sequential   Formatted
!                   PGSd_IO_Gen_USeqUnf   Update  Sequential   Unformatted
!                   PGSd_IO_Gen_UDirFrm   Update  Direct       Formatted
!                   PGSd_IO_Gen_UDirUnf   Update  Direct       Unformatted
! 
!     record_length - record length for direct access IO:
!                     mandatory for direct access (minimum value = 1)
!                     ignored otherwise
!
!     file_version - version of file to open (minimum value = 1)
!
! OUTPUTS:      
!     file_handle  - used to manipulate files READ and WRITE
!
! 
! RETURNS: 
!     PGS_S_SUCCESS                     Successful completion
!     PGSIO_E_NO_FREE_LUN               All logical unit numbers are in use
!     PGSIO_E_GEN_OPENMODE              Illegal open mode was specified
!     PGSIO_E_GEN_OPEN_OLD              Attempt to open with STATUS=OLD failed 
!     PGSIO_E_GEN_OPEN_NEW              Attempt to open with STATUS=NEW failed
!     PGSIO_E_GEN_OPEN_RECL             Invalid record length specified
!     PGSIO_E_GEN_FILE_NOEXIST          File not found, cannot create
!     PGSIO_E_GEN_REFERENCE_FAILURE     Can't do Temporary file reference
!
! EXAMPLES:
!     implicit none
!     include   'PGS_SMF.f'
!     include   'PGS_PC.f'
!     include   'PGS_PC_9.f'
!     include   'PGS_IO.f'
!     include   'PGS_IO_1.f'
!     integer pgs_io_gen_openf
!     integer returnstatus
!     integer file_logical
!     integer file_access
!     integer record_length
!     integer file_handle
!     integer file_version
!
!     file_version  = 3
!     file_logical  = 101
!     file_access   = PGSd_IO_Gen_WSeqFrm
! 
!     returnstatus = PGS_IO_Gen_OpenF(
!    +     file_logical, file_access, record_length, 
!    +     file_handle, file_version)
!
!     if (returnstatus .NE. PGS_S_SUCCESS) then
!   C     goto 1000
!     endif
!     .
!     .
!     .
!1000 <error handling goes here>
! 
! NOTES:                
!     
!     While all modes of access are supported for this tool, those
!     modes which allow for writing to a file (i.e., not
!     PGSd_IO_Gen_Read) are intended for Toolkit access only. The only
!     files which the science software should write to are product
!     output files (Hdf) and Temporary, or Intermediate files.
!     
!     In order to ascertain the number of versions currently
!     associated with the logical identifier in question, make
!     a call to PGS_PC_Get_NumberOfFiles() first (TK3
!     and later).
!     
!     Due to the nature of FORTRAN IO, it is possible to write a file opened
!     for reading as well as read a file opened for writing.  The matching
!     of access mode to IO statement cannot be enforced by the tool.  This
!     is up to the user.
!
!     Once a file has been opened with this tool, it must be closed with a
!     call to PGS_IO_Gen_CloseF before being re-opened.  Failure to do
!     this will result in undefined behavior.
!
!     IMPORTANT TK5 NOTES
!     -------------------
!     The following environment variable MUST be set to assure proper operation:
! 
!             PGS_PC_INFO_FILE        path to process control file
! 
!     However, the following environment variables are NO LONGER recognized by
!     the Toolkit as such:
! 
!             PGS_PRODUCT_INPUT  default path to Product input files
!             PGS_PRODUCT_OUTPUT default path to Product output files
!             PGS_SUPPORT_INPUT  default path to Supporting input files
!             PGS_SUPPORT_OUTPUT default path to Supporting output files
! 
!     Instead, the default paths, which were defined by these environment variables
!     in previous Toolkit releases, may now be specified as part of the Process
!     Control File (PCF). Essentially, each has been replaced by a global path
!     statement for each of the respective subject fields within the PCF. To
!     define a global path statement, simply create a record which begins with the
!     '!' symbol defined in the first column, followed by the global path to be
!     applied to each of the records within that subject field. Only one such
!     statement can be defined per subject field and it must be appear prior to
!     any dependent subject entry.
!
!     It is error condition to have an input file specified in the PCF
!     that does not exist on disk.  The behavior of the tool is undefined
!     when attempting to open such a file for reading.
!
! 
! REQUIREMENTS: 
!     PGSTK-0360
! 
! DETAILS:      
!     This routine first calls another toolkit routine to map the PGS file
!     logical to its physical file name.  The tool then attempts to open
!     the file, going to exception handling if it encounters an error.
!     Otherwise, the call returns successfully.
! 
! GLOBALS:
!     NONE
! 
! FILES:
!     The file to be opened and the access mode are determined by the input
!     parameters.
!
! FUNCTIONS_CALLED:
!     PGS_PC_GetPCSData          Map PGS logical to physical file
!     PGS_IO_Gen_Track_LUN       Allocate logical unit number
!     PGS_SMF_SetStaticMsg       Register error return with SMF
!     PGS_SMF_GetMsgByCode       Retrieve message string from code
! 
! END_PROLOG
!***************************************************************************

      integer function pgs_io_gen_openf(
     +     file_logical, file_access, record_length,
     +     file_handle, file_version)

      implicit none

! Include header file definitions

      INCLUDE   'PGS_SMF.f'
      INCLUDE   'PGS_PC.f'
      INCLUDE   'PGS_PC_9.f'
      INCLUDE   'PGS_IO.f'
      INCLUDE   'PGS_IO_1.f'

!
! Parameters: 
!     in:  file logical, access mode, record length 
!     out: file handle LUN 
!
      integer       file_logical
      integer       file_access
      integer       record_length
      integer       file_handle
      integer       file_version
      integer       GET_String_Length

!
! Functions Called
!

      integer pgs_pc_getpcsdata 
      integer pgs_io_gen_track_lun
      integer pgs_smf_setstaticmsg
      integer pgs_smf_setdynamicmsg
      integer pgs_smf_getmsgbycode

!
! Local variables
!

!     mode        - value sent to PGS_PC_GetPCSData 
!     modes       - loop control variable
!     mode_array  - array of values for PGS_PC_GetPCSData 
!     inputFileVersion - buffer for file_version
!     existFlag   - indicates existence of file
!     supportFile - indicates match on SUPPORT type file
!     outstr      - file name buffer for PGS_PC_GetPCSData 

      integer         mode
      integer         modes
      integer         mode_array(4)
      integer         inputFileVersion 
      integer         existFlag
      integer         supportFile
      character*500   outstr
      character*240 tmp_message
      character*240  log_message
      character*480  error_buf
      integer size_tmpmess
      integer size_errorbuf


!     lun - logical unit number from  PGS_IO_Gen_Track_LUN
!     lun_command - command for PGS_IO_Gen_Track_LUN

      integer         lun
      integer         lun_command

!     temp status return for calls to Toolkit routines

      integer         tmp_status

!     ------------------------------------------------------
!     Initialize 
!     ------------------------------------------------------

!     default return status: success 
      pgs_io_gen_openf = PGS_S_SUCCESS

!     default mode: get physical input file name 
      mode = PGSd_PC_INPUT_FILE_NAME

      existFlag = PGS_FALSE
      supportFile = PGS_FALSE

!     set up mode array for loop
      mode_array(1) = PGSd_PC_INPUT_FILE_NAME
      mode_array(2) = PGSd_PC_OUTPUT_FILE_NAME
      mode_array(3) = PGSd_PC_SUPPORT_IN_NAME
      mode_array(4) = PGSd_PC_SUPPORT_OUT_NAME


!     ------------------------------------------------------
!     Convert logical identifier into a physical one
!     ------------------------------------------------------
!
!     NOTE:
!       This section is coded to reflect, as closely as possible,
!     the logic used in the C version of this tool  This has 
!     necessitated the use of a few 'goto' statements to emulate
!     the behavior of C 'continue' and 'break' statements.
!     ------------------------------------------------------

      do 100 modes = 1,4

!     -  get access mode
         mode = mode_array(modes)

!     -  file version is overwritten by PGS_PC_GetPCSData: buffer it
         inputFileVersion  = file_version

!     - map the file logical and version to the physical file
         pgs_io_gen_openf = PGS_PC_GetPCSData(
     +        mode, file_logical, outstr, inputFileVersion)

!     -  return status of PGS_S_SUCCESS indicates that file exists
!     -  or is being created in accordance with access mode

         if (pgs_io_gen_openf .NE. PGS_S_SUCCESS) then

            if (pgs_io_gen_openf .EQ. PGSPC_W_NO_FILES_EXIST) then

!     -        file not found: try another mode

               if (modes .lt. 4) then
                  goto 100
               else
                  goto 1000
               endif

            else

!     -        this is a fatal error
               pgs_io_gen_openf = PGSIO_E_GEN_REFERENCE_FAILURE
               goto 1000
                  
            endif

        else

            existFlag = PGS_TRUE

            if (
     $           (mode_array(modes) .eq. PGSd_PC_SUPPORT_IN_NAME) .or.
     $           (mode_array(modes) .eq. PGSd_PC_SUPPORT_OUT_NAME)
     $           ) then

               supportFile = PGS_TRUE

            endif
            
            goto 101

         endif

!     end of DO loop (goto 100 equiv to 'continue' in C version)
 100  continue

!     exit from DO loop (goto 101 equiv to 'break' in C version)
 101  continue


!     ------------------------------------------------------
!     set the command for PGS_IO_Gen_Track_LUN
!     then try to get a free LUN
!     ------------------------------------------------------

      lun_command = 0
      tmp_status = PGS_IO_Gen_Track_LUN(lun, lun_command)

      if (tmp_status .NE. PGS_S_SUCCESS) then
!     ... failed: (no such thing as a free LUN)
         pgs_io_gen_openf = PGSIO_E_GEN_NO_FREE_LUN
         goto 1000
      else
!     ... success: assign file handle
         file_handle = lun
      endif

!     ------------------------------------------------------
!     If this is a direct access mode, check record length
!     minimum value is 1
!     ------------------------------------------------------

      if (
     +     (file_access .EQ. PGSd_IO_Gen_RDirFrm) .or.
     +     (file_access .EQ. PGSd_IO_Gen_RDirUnf) .or.
     +     (file_access .EQ. PGSd_IO_Gen_WDirFrm) .or.
     +     (file_access .EQ. PGSd_IO_Gen_WDirUnf) .or.
     +     (file_access .EQ. PGSd_IO_Gen_UDirFrm) .or.
     +     (file_access .EQ. PGSd_IO_Gen_UDirUnf)
     +     ) then 
         if (record_length .LT. 1) then
            pgs_io_gen_openf = PGSIO_E_GEN_OPEN_RECL
            goto 1000
         endif
      endif

!     ------------------------------------------------------
!     Open physical file under proper access mode
!     ------------------------------------------------------

!     read access: file must exist

      if (file_access .EQ. PGSd_IO_Gen_RSeqFrm) then 
         open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'old', 
     +        err = 500)

      else if (file_access .EQ. PGSd_IO_Gen_RSeqUnf) then 
         open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'old', 
     +        err = 500)

      else if (file_access .EQ. PGSd_IO_Gen_RDirFrm) then 
         open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'direct', status = 'old', 
     +        recl = record_length, err = 500)

      else if (file_access .EQ. PGSd_IO_Gen_RDirUnf) then 
         open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'direct', status = 'old',
     +        recl = record_length, err = 500 )

!     write access: file is created 

      else if (file_access .EQ. PGSd_IO_Gen_WSeqFrm) then 
         open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'new', 
     +        err = 510)

      else if (file_access .EQ. PGSd_IO_Gen_WSeqUnf) then 
         open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'new', 
     +        err = 510)

      else if (file_access .EQ. PGSd_IO_Gen_WDirFrm) then 
         open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'direct', status = 'new',
     +        recl = record_length, err = 510)

      else if (file_access .EQ. PGSd_IO_Gen_WDirUnf) then 
         open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'direct', status = 'new',
     +        recl = record_length, err = 510)

!     update access: file must exist

      else if (file_access .EQ. PGSd_IO_Gen_USeqFrm) then 
         open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'old', 
     +        err = 500)

      else if (file_access .EQ. PGSd_IO_Gen_USeqUnf) then 
         open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'old', 
     +        err = 500)

      else if (file_access .EQ. PGSd_IO_Gen_UDirFrm) then 
         open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'direct', status = 'old',
     +        recl = record_length, err = 500)

      else if (file_access .EQ. PGSd_IO_Gen_UDirUnf) then 
         open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'direct', status = 'old',
     +        recl = record_length, err = 500)

!     invalid open mode:  deallocate LUN, set return status

      else
         lun_command = 1
         tmp_status = PGS_IO_Gen_Track_LUN(lun, lun_command)
         pgs_io_gen_openf = PGSIO_E_GEN_OPENMODE
         goto 1000
      endif


!     ------------------------------------------------------
!     Normal return
!     ------------------------------------------------------

      tmp_status = PGS_SMF_SetStaticMsg( pgs_io_gen_openf,
     +     'PGS_IO_Gen_Temp_OpenF' )
      return

!     ------------------------------------------------------
!     Open failures come to one of these points
!     ------------------------------------------------------

 500  pgs_io_gen_openf = PGSIO_E_GEN_OPEN_OLD
      goto 1000

 510  pgs_io_gen_openf = PGSIO_E_GEN_OPEN_NEW


!     ------------------------------------------------------
!     Exceptions jump to this point
!     ------------------------------------------------------

 1000 if (pgs_io_gen_openf .eq. PGSPC_W_NO_FILES_EXIST) then
    
         pgs_io_gen_openf = PGSIO_E_GEN_FILE_NOEXIST

      endif

      write(tmp_message,'(27HUnable to open file with ID,I6)') 
     #file_logical
      size_tmpmess = GET_String_Length(tmp_message,240)
      tmp_status = PGS_SMF_GetMsgByCode(pgs_io_gen_openf,error_buf)
      size_errorbuf = GET_String_Length(error_buf,240)
      log_message = error_buf(1:size_errorbuf) // 
     #tmp_message(1:size_tmpmess)
      tmp_status = PGS_SMF_SetDynamicMsg( pgs_io_gen_openf,log_message,
     +     'PGS_IO_Gen_OpenF' )

      return
      end


!#######################################################################
 
!---- This INTEGER FUNCTION returns the length of a string.  If it finds
!---- six spaces together it assumes it has found end of string.
 
      integer function GET_String_Length ( char, maxlength )
 
      implicit       none
      integer*4      i, istop, maxlength
      character*1    dum, dum1,dum2,dum3,dum4,dum5
      character*(*)  char
 
      istop = 0
      if(char(1:1).ne.' ') then
         i = 1
         do while (istop.eq.0)
            dum  = char(i:i)
            dum1 = char(i+1:i+1)
            dum2 = char(i+2:i+2)
            dum3 = char(i+3:i+3)
            dum4 = char(i+4:i+4)
            dum5 = char(i+5:i+5)
            if(dum.eq.' '.and.dum1.eq.' '.and.dum2.eq.' '
     2         .and.dum3.eq.' '.and.dum4.eq.' '.and.dum5.eq.' ') then
               GET_String_Length = i
               istop             = 1
            else
               i = i + 1
               if(i.gt.maxlength) then
                  GET_String_Length = i - 1
                  istop             = 1
               end if
            end if
         end do
      else
         GET_String_Length = 1
      end if
 
      return
      end
!#######################################################################

