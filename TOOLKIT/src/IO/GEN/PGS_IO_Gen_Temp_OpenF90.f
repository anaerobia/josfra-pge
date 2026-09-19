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
!     PGS_IO_Gen_Temp_OpenF90.f
! 
! DESCRIPTION:
!     The tool listed in this file provides the FORTRAN user with access
!     to temporary and intermediate files.
! 
! AUTHORS:
!     Mike Sucher / Applied Research Corp.
! 
! HISTORY:
!     01-Aug-1994 MES Initial version
!     12-Aug-1994 MES Fixes following code inspection
!     26-Aug-1994 MES Remove obsolete FORTRAN SMF header files.
!     07-Sep-1994 MES Revised prolog for TK3.
!     18-Oct-1994 MES Update NOTES section of prolog
!     18-Jan-1995 TWA ***F90 SPECIFIC*** Added support for F90 Open call:
!                         (1) POSITION=APPEND value
!                         (2) RECL keyword for sequential files
!     14-Jul-1995 DPH Updated prolog for new environment variable usage
!     11-Jun-1998 MEDS Updated the notes on the File characteristics 
! 
! END_FILE_PROLOG
!***************************************************************************

!***************************************************************************
! BEGIN_PROLOG
! 
! TITLE: 
!       Open a Temporary or Intermediate File (FORTRAN version)
!  
! NAME: 
!       PGS_IO_Gen_Temp_OpenF
! 
! SYNOPSIS:
!   C:    (not applicable)
!
!   FORTRAN:
!     INCLUDE   'PGS_SMF.f'
!     INCLUDE   'PGS_PC_9.f'
!     INCLUDE   'PGS_IO.f'
!     INCLUDE   'PGS_IO_1.f'
!
!     integer function pgs_io_gen_temp_openf(file_duration, file_logical, 
!    +                             file_access, record_length, file_handle)
!     integer       file_duration
!     integer       file_logical
!     integer       file_access
!     integer       record_length
!     integer       file_handle
! 
! DESCRIPTION:  
!     Upon a successful call, this function will return a logical unit 
!     number for use with FORTRAN READ and WRITE statements.  This is 
!     returned to the user via the parameter file_handle.  The user 
!     provides the logical file identifier which internally gets mapped 
!     to the associated physical file.  The user also provides the file 
!     duration parameter, to specify whether the file being opened is 
!     to be temporary or intermediate.
! 
! INPUTS:               
!     file_duration - specifies how long file will last:
!
!                   PGS-defined value          description
!                   ========================   =====================
!                   PGSd_IO_Gen_Endurance      intermediate file
!                   PGSd_IO_Gen_NoEndurance    temporary file
!
!     file_logical  - User defined logical file identifier
! 
!     file_access   - type of access granted to opened file:
!
!                                         Rd/Wr/
!                          PGS            Update/ FORTRAN      FORTRAN
!                   FORTRAN Access Mode   Append  'access='    'form='
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
!                   PGSd_IO_Gen_ASeqFrm   Append  Sequential   Formatted
!                   PGSd_IO_Gen_ASeqUnf   Append  Sequential   Unformatted
! 
!     record_length - record length
!                     must be greater than 0 for direct access 
! ***F90 SPECIFIC***  must be greater than or equal to 0 for sequential access
! ***F90 SPECIFIC***  if 0, file is opened with default record length
!                    
! OUTPUTS:      
!     file_handle    - used to manipulate files with READ and WRITE
! 
! RETURNS: 
!     PGS_S_SUCCESS                     Successful completion
!     PGSIO_E_NO_FREE_LUN               All logicall unit numbers are in use
!     PGSIO_W_GEN_ACCESS_MODIFIED       The access mode has been modified
!     PGSIO_E_GEN_OPENMODE              Illegal open mode was specified
!     PGSIO_E_GEN_OPEN_OLD              Attempt to open with STATUS=OLD failed 
!     PGSIO_E_GEN_OPEN_NEW              Attempt to open with STATUS=NEW failed
!     PGSIO_E_GEN_OPEN_RECL             Invalid record length specified
!
!     PGSIO_W_GEN_OLD_FILE              File exists: changing access to update
!     PGSIO_W_GEN_NEW_FILE              File not found, created new one
!     PGSIO_W_GEN_DURATION_NOMOD        Illegal attempt to modify file duration
!     PGSIO_E_GEN_REFERENCE_FAILURE     Can't do Temporary file reference
!     PGSIO_E_GEN_BAD_FILE_DURATION     Illegal file duration value
!     PGSIO_E_GEN_FILE_NOEXIST          File not found, cannot create
!     PGSIO_E_GEN_CREATE_FAILURE        Unable to create new file
!     PGSIO_E_GEN_NO_TEMP_NAME          New name could not be generated
!
! 
! EXAMPLES:
!     implicit none
!     include   'PGS_SMF.f'
!     include   'PGS_PC.f'
!     include   'PGS_PC_9.f'
!     include   'PGS_IO.f'
!     include   'PGS_IO_1.f'
!     integer pgs_io_gen_temp_openf
!     integer returnstatus
!     integer file_duration
!     integer file_logical
!     integer file_access
!     integer record_length
!     integer file_handle
!
!     file_duration = PGSd_IO_Gen_NoEndurance
!     file_logical  = 101
!     file_access   = PGSd_IO_Gen_WDirUnf
!     record_length = 1
! 
!     returnstatus = PGS_IO_Gen_Temp_OpenF(
!    +     file_duration, file_logical, file_access, 
!    +     record_length, file_handle)
!
!     if (returnstatus .NE. PGS_S_SUCCESS) then
!   C     goto 100
!     endif
!     .
!     .
!     .
! 100 <error handling goes here>
! 
! NOTES:
!     Logical identifiers used for Temporary and Intermediate files may NOT 
!     be duplicated. Existing files will NOT be overwritten by calling this 
!     function in any of the write modes.  Instead, they will be opened 
!     in the corresponing update mode; a warning will be issued signifying 
!     that this is the case. Warnings will also be issued in the event that
!     a non-existent file is opened in modes other than explicit write
!
!     By using this tool, the user understands that a Temporary file 
!     may only exist for the duration of a PGE. Whether or not the user 
!     deletes this file prior to PGE termination, it will be purged by the 
!     PGS system during normal cleanup operations. If the user requires a 
!     more static instance of a file, one that will exist beyond normal PGE 
!     termination, that user may elect to create an Intermediate file 
!     instead by specifying some persistence value (currently, 
!     PGSd_IO_Gen_Endurance is the only value recognized); note that this 
!     value is only valid for the initialcreation of a file and will not be
!     applied to subsequent accesses of the same file.
!     
!	
!	FILE CHARACTERISTICS
!	____________________
!
!	All files created by this function have the following form:
!
!               [label][global-network-IP-address][process-id][date][time]
!
!        where:
! 
!        label                    : SDP Toolkit Process Control 		    -> pc
!        global-network-IP-address: complete IP address iii.iii.iii.iii      -> iiiiiiiiiiii
!                                   (0's padded to maintain triplet groupings)
!        process-id               : process identifier of current executable -> pppppp
!        date                     : days from beginning of year & the year   -> dddyy
!        time                     : time from midnight local time  	    -> hhmmss
! 
!        or 	'pciiiiiiiiiiiippppppdddddtttttt'
!
!	ex.      pc19811819201701028000395104034  
!
!                                      pc     198118192017 010280 00395 104034  
!                                      |      |            |      |     |
!        (pc) label____________________|      |            |      |     |
!        (i)  full-network-IP-address ________|            |      |     |
!        (p)  process-id___________________________________|      |     |             
!        (d)  date________________________________________________|     |
!        (t)  time______________________________________________________|
! 
!
!     All temporary and intermediate files generated by this tool are unique
!     within the global ECS community. Also, all file names are NOW exactly
!     31 characters in length; this should help with the diagnosis of suspect
!     temporary files (i.e., check the length first).
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
!             PGS_TEMPORARY_IO        default path to temporary files
!             PGS_INTERMEDIATE_INPUT  default path to intermediate input files
!             PGS_INTERMEDIATE_OUTPUT default path to intermediate output files
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
!     The environment variable PGS_HOST_PATH, formerly used to direct the Toolkit 
!     to the location of the internet protocol address for the local host, has been
!     replaced by PDPS functionality which can perform this function in a more effective
!     manner. For this reason, the use of this environment variable is no longer 
!     supported. FAILURE TO HEED THIS WARNING MAY RESULT IN UNPREDICTABLE RESULTS FOR 
!     THE PGE! To properly emulate the manner in which the PDPS system provides this
!     information to the Toolkit, continue to use the runtime parameter 
!     PGSd_IO_Gen_HostAddress to advertise the IP address of the local host.	
!
!     It is error condition to have an input file specified in the PCF
!     that does not exist on disk.  The behavior of the tool is undefined
!     when attempting to open such a file for reading.
!
!
!     ***F90 SPECIFIC*** NOTES
!     ------------------------
!     In addition to supporting all modes of the ANSI Fortran 77 OPEN
!     statement, this module also supports the following functionality
!     of the ANSI Fortran 90 OPEN statement:
!        (1) APPEND value of POSITION keyword
!               Sequential files may be opened in APPEND mode.
!        (2) RECL keyword for sequential files
!               This is used to specify the maximum record length
!               of it is greater than the processor-dependent 
!               default record length.
!               If this value is 0, the default value is used.
!     This code module is very similar to the code in module
!     PGS_IO_Gen_Temp_OpenF.f . Differences are highlighted by the 
!     string "***F90 SPECIFIC***".
!     At installation time, this code module is compiled if
!     the user specifies Fortran 90 as his/her language choice;
!     otherwise, code module PGS_IO_Gen_Temp_OpenF.f is compiled.
!     The function name "PGS_IO_Gen_Temp_OpenF" is the same in both cases.
! 
!     
! REQUIREMENTS: 
!     PGSTK-0530, PGSTK-0531
! 
! DETAILS:      
!     This function calls a PGS_PC toolkit routine to map the file logical 
!     to the appropriate physical file name.  If this is successful, a call
!     to allocate a free FORTRAN logical unit number is made.  Then the file
!     is opened using the FORTRAN OPEN statement. If an error occurs, control
!     passes to exception handling.  Otherwise, the call returns successfully.
! 
! GLOBALS:
!     NONE
! 
! FILES:
!     The file to be opened and the access mode are determined by the input
!     parameters.
!
! FUNCTIONS_CALLED:
!     PGS_IO_GEN_Temp_Delete     Delete a temporary file
!     PGS_IO_GEN_Temp_Reference  Map PGS logical to new or old physical file
!     PGS_IO_Gen_Track_LUN       Allocate logical unit number
!     PGS_SMF_SetStaticMsg       Register error return with SMF
! 
! END_PROLOG
!***************************************************************************

      integer function pgs_io_gen_temp_openf(
     +     file_duration, file_logical, 
     +     file_access, record_length, file_handle)
 
      implicit none

! Include header file definitions

      INCLUDE   'PGS_SMF.f'
      INCLUDE   'PGS_PC_9.f'
      INCLUDE   'PGS_IO.f'
      INCLUDE   'PGS_IO_1.f'

!
! Parameters: 
!     in:  file duration, file logical, access mode, record length
!     out: file handle LUN 
!
      integer       file_duration
      integer       file_logical
      integer       file_access
      integer       record_length
      integer       file_handle

!
! Functions Called
!

      integer pgs_io_gen_temp_delete
      integer pgs_io_gen_temp_reference
      integer pgs_io_gen_track_lun
      integer pgs_smf_setstaticmsg

!
! Local variables
!

!     boolean flag for PGS_IO_Gen_Temp_Reference to test for existence

      integer file_exists

!     file name buffer for PGS_IO_Gen_Temp_Reference

      character*500   outstr

!     file access mode mapping for PGS_IO_Gen_Temp_Reference

      integer         file_access_map

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
      pgs_io_gen_temp_openf = PGS_S_SUCCESS

!     map FORTRAN file access modes to something that
!     PGS_IO_Gen_Temp_Reference can understand

      if (file_access .EQ. PGSd_IO_Gen_RSeqFrm) then
         file_access_map = PGSd_IO_Gen_Read
      else if (file_access .EQ. PGSd_IO_Gen_RSeqUnf) then
         file_access_map = PGSd_IO_Gen_Read
      else if (file_access .EQ. PGSd_IO_Gen_RDirFrm) then
         file_access_map = PGSd_IO_Gen_Read
      else if (file_access .EQ. PGSd_IO_Gen_RDirUnf) then
         file_access_map = PGSd_IO_Gen_Read
      else if (file_access .EQ. PGSd_IO_Gen_WSeqFrm) then
         file_access_map = PGSd_IO_Gen_Write
      else if (file_access .EQ. PGSd_IO_Gen_WSeqUnf) then
         file_access_map = PGSd_IO_Gen_Write
      else if (file_access .EQ. PGSd_IO_Gen_WDirFrm) then
         file_access_map = PGSd_IO_Gen_Write
      else if (file_access .EQ. PGSd_IO_Gen_WDirUnf) then
         file_access_map = PGSd_IO_Gen_Write
      else if (file_access .EQ. PGSd_IO_Gen_USeqFrm) then
         file_access_map = PGSd_IO_Gen_Update
      else if (file_access .EQ. PGSd_IO_Gen_USeqUnf) then
         file_access_map = PGSd_IO_Gen_Update
      else if (file_access .EQ. PGSd_IO_Gen_UDirFrm) then
         file_access_map = PGSd_IO_Gen_Update
      else if (file_access .EQ. PGSd_IO_Gen_UDirUnf) then
         file_access_map = PGSd_IO_Gen_Update
!
!     ***F90 SPECIFIC***
!
      else if (file_access .EQ. PGSd_IO_Gen_ASeqFrm) then
         file_access_map = PGSd_IO_Gen_Append
      else if (file_access .EQ. PGSd_IO_Gen_ASeqUnf) then
         file_access_map = PGSd_IO_Gen_Append
      endif 


!     ------------------------------------------------------
!     Get File Reference; create new file if necessary
!     ------------------------------------------------------

      pgs_io_gen_temp_openf = pgs_io_gen_temp_reference( 
     +     file_duration, file_logical, file_access_map,
     +     outstr, file_exists )
      
!     -------------------------------------------------------
!     Acceptable return values from PGS_IO_Gen_Temp_Reference
!     are PGS_S_SUCCESS, PGSIO_W_GEN_DURATION_NOMOD,   
!     ***F90 SPECIFIC*** and PGSIO_W_GEN_NEW_FILE (for APPEND).
!     All other values force a jump to exception handling.
!     -------------------------------------------------------


      if ( ( pgs_io_gen_temp_openf .NE. PGS_S_SUCCESS) .AND.
     +     ( pgs_io_gen_temp_openf .NE. PGSIO_W_GEN_DURATION_NOMOD)
     + .AND. ( pgs_io_gen_temp_openf .NE. PGSIO_W_GEN_NEW_FILE) )
     + then
          goto 1000
      endif

!     ------------------------------------------------------
!     Filter : Warn of write access on existing files
!     Action : Change access mode to Update
!     REMEMBER: intermediate files last beyond termination
!     ------------------------------------------------------

      if ( (file_exists .EQ. PGS_TRUE) .AND.
     +     (file_access_map .EQ. PGSd_IO_Gen_Write) ) then

         pgs_io_gen_temp_openf = PGSIO_W_GEN_OLD_FILE

         file_access_map = PGSd_IO_Gen_Update

         if (file_access .EQ. PGSd_IO_Gen_WSeqFrm) then
            file_access = PGSd_IO_Gen_USeqFrm
         else if (file_access .EQ. PGSd_IO_Gen_WSeqUnf) then
            file_access = PGSd_IO_Gen_USeqUnf
         else if (file_access .EQ. PGSd_IO_Gen_WDirFrm) then
            file_access = PGSd_IO_Gen_UDirFrm
         else if (file_access .EQ. PGSd_IO_Gen_WDirUnf) then
            file_access = PGSd_IO_Gen_UDirUnf
         endif

!     trace log 

         tmp_status = PGS_SMF_SetStaticMsg( 
     +        pgs_io_gen_temp_openf,'PGS_IO_Gen_Temp_OpenF' )

      endif


!     ------------------------------------------------------
!     set the command for PGS_IO_Gen_Track_LUN
!     then try to get a free LUN
!     ------------------------------------------------------

      lun_command = 0
      tmp_status = PGS_IO_Gen_Track_LUN(lun, lun_command)

      if (tmp_status .NE. PGS_S_SUCCESS) then
!          ... failed: (no such thing as a free LUN)
         pgs_io_gen_temp_openf = PGSIO_E_GEN_NO_FREE_LUN
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
            pgs_io_gen_temp_openf = PGSIO_E_GEN_OPEN_RECL
            goto 1000
         endif
      endif

!     ------------------------------------------------------
!     ***F90 SPECIFIC***
!     If this is a sequential mode, check record length
!     minimum value is 0
!     ------------------------------------------------------

      if (
     +     (file_access .EQ. PGSd_IO_Gen_RSeqFrm) .or.
     +     (file_access .EQ. PGSd_IO_Gen_RSeqUnf) .or.
     +     (file_access .EQ. PGSd_IO_Gen_WSeqFrm) .or.
     +     (file_access .EQ. PGSd_IO_Gen_WSeqUnf) .or.
     +     (file_access .EQ. PGSd_IO_Gen_USeqFrm) .or.
     +     (file_access .EQ. PGSd_IO_Gen_USeqUnf) .or.
     +     (file_access .EQ. PGSd_IO_Gen_ASeqFrm) .or.
     +     (file_access .EQ. PGSd_IO_Gen_ASeqUnf)
     +     ) then 
         if (record_length .LT. 0) then
            pgs_io_gen_temp_openf = PGSIO_E_GEN_OPEN_RECL
            goto 1000
         endif
      endif

!     ------------------------------------------------------
!     Open physical file under proper access mode
!
!     ***F90 SPECIFIC***
!     NOTE: Sequential files with record length = 0 are opened
!        with processor dependent default maximum record length
!     ------------------------------------------------------

!     read access: file must exist

      if (file_access .EQ. PGSd_IO_Gen_RSeqFrm) then 
!     ***F90 SPECIFIC *** (recl keyword)
         if (record_length .GT. 0) then 
            open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'old', 
     +        recl = record_length, err = 500)
         else
            open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'old', 
     +        err = 500)
         endif

      else if (file_access .EQ. PGSd_IO_Gen_RSeqUnf) then 
!     ***F90 SPECIFIC*** (recl keyword)
         if (record_length .GT. 0) then 
            open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'old', 
     +        recl = record_length, err = 500)
         else
            open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'old', 
     +        err = 500)
         endif

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
         if (record_length .GT. 0) then 
!     ***F90 SPECIFIC*** (recl keyword)
            open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'new', 
     +        recl = record_length, err = 510)
         else
            open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'new', 
     +        err = 510)
         endif

      else if (file_access .EQ. PGSd_IO_Gen_WSeqUnf) then 
         if (record_length .GT. 0) then 
!     ***F90 SPECIFIC*** (recl keyword)
            open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'new', 
     +        recl = record_length, err = 510)
         else
            open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'new', 
     +        err = 510)
         endif

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
         if (record_length .GT. 0) then 
!     ***F90 SPECIFIC*** (recl keyword)
            open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'old', 
     +        recl = record_length, err = 500)
         else
            open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'old', 
     +        err = 500)
         endif

      else if (file_access .EQ. PGSd_IO_Gen_USeqUnf) then 
         if (record_length .GT. 0) then 
!     ***F90 SPECIFIC*** (recl keyword)
            open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'old', 
     +        recl = record_length, err = 500)
         else
            open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'old', 
     +        err = 500)
         endif

      else if (file_access .EQ. PGSd_IO_Gen_UDirFrm) then 
         open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'direct', status = 'old',
     +        recl = record_length, err = 500)

      else if (file_access .EQ. PGSd_IO_Gen_UDirUnf) then 
         open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'direct', status = 'old',
     +        recl = record_length, err = 500)

!     append access: file need not exist

!     ***F90 SPECIFIC***
      else if (file_access .EQ. PGSd_IO_Gen_ASeqFrm) then 
         if (record_length .GT. 0) then 
            open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'unknown', 
     +        recl = record_length, position = 'append',
     +        err = 500)
         else
            open (unit = lun, file = outstr, form = 'formatted',
     +        access = 'sequential', status = 'unknown', 
     +        position = 'append', 
     +        err = 500)
         endif

!     ***F90 SPECIFIC***
      else if (file_access .EQ. PGSd_IO_Gen_ASeqUnf) then 
         if (record_length .GT. 0) then 
            open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'unknown', 
     +        recl = record_length, position = 'append',
     +        err = 500)

         else
            open (unit = lun, file = outstr, form = 'unformatted',
     +        access = 'sequential', status = 'unknown', 
     +        position = 'append',
     +        err = 500)
         endif

!     invalid open mode:  deallocate LUN, set return status

      else
         lun_command = 1
         tmp_status = PGS_IO_Gen_Track_LUN(lun, lun_command)
         pgs_io_gen_temp_openf = PGSIO_E_GEN_OPENMODE
         goto 1000
      endif


!     ------------------------------------------------------
!     Normal return
!     ------------------------------------------------------

      tmp_status = PGS_SMF_SetStaticMsg( pgs_io_gen_temp_openf,
     +     'PGS_IO_Gen_Temp_OpenF' )
      return

!     ------------------------------------------------------
!     Open failures come to one of these points
!     ------------------------------------------------------

 500  pgs_io_gen_temp_openf = PGSIO_E_GEN_OPEN_OLD
      goto 1000

 510  pgs_io_gen_temp_openf = PGSIO_E_GEN_OPEN_NEW

!     ------------------------------------------------------
!     Issue delete instruction to Process Control
!     if NEW Temp file could not be opened
!     ------------------------------------------------------
      if (file_exists .EQ. PGS_FALSE) then
         tmp_status = PGS_IO_Gen_Temp_Delete(file_logical)
      endif
      goto 1000


!     ------------------------------------------------------
!     Exceptions jump to this point
!     ------------------------------------------------------

 1000 if (
     +     (pgs_io_gen_temp_openf .EQ. PGSIO_E_GEN_OPEN_OLD)  .or. 
     +     (pgs_io_gen_temp_openf .EQ. PGSIO_E_GEN_OPEN_NEW)  .or.
     +     (pgs_io_gen_temp_openf .EQ. PGSIO_E_GEN_OPEN_RECL) .or.
     +     (pgs_io_gen_temp_openf .EQ. PGSIO_E_GEN_OPENMODE)  .or.
     +     (pgs_io_gen_temp_openf .EQ. PGSIO_E_GEN_NO_FREE_LUN) 
     +     ) then 

         tmp_status = PGS_SMF_SetStaticMsg( pgs_io_gen_temp_openf,
     +        'PGS_IO_Gen_Temp_OpenF' )
      endif
        
      return
      end

