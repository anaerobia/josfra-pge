/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_GetHeader.c

DESCRIPTION:
  This file contains the Level 0 I/O tool to get the header and footer
  data for the currently open physical file.

AUTHOR:
  Mike Sucher / Applied Research Corp.
  Tom W. Atwater / Applied Research Corp.
  Guru Tej S. Khalsa / Applied Research Corp.
  Abe Taaheri / SM&A Corp.
  Xin Wang / EIT Inc.

HISTORY:
               MS  Designed
  30-Jan-1995 TWA  Initial version
  05-May-1995 TWA  Fixed bug; variable length part of TRMM header now returned
  13-Jul-1995 GTSK Altered error handling scheme; replace error message
                   PGSIO_L0_BUFFER_TRUNCATE with the more specific messages
		   PGSIO_L0_HDR_BUF_TRUNCATE, PGSIO_L0_FTR_BUF_TRUNCATE
		   and PGSIO_L0_ALL_BUF_TRUNCATE to indicate that the file
		   header, footer or header AND footer data (respectively) 
		   has/have been truncated.
  01-Dec-1999 AT   Modified to support GIRD and GIIS formats for PM spacecraft
  01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Get the Header and Footer Data from a Physical Level 0 File

NAME:
	PGS_IO_L0_GetHeader
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_GetHeader(
		PGSt_IO_L0_VirtualDataSet virtual_file,
		PGSt_integer		  header_buffer_size,
		PGSt_IO_L0_Header	  *header_buffer,
		PGSt_integer		  footer_buffer_size,
		PGSt_IO_L0_Footer	  *footer_buffer
		)
  FORTRAN:
	INCLUDE   'PGS_SMF.f'
	INCLUDE   'PGS_PC.f'
	INCLUDE   'PGS_PC_9.f'
	INCLUDE   'PGS_TD.f'
	INCLUDE   'PGS_IO.f'
	INCLUDE   'PGS_IO_1.f'

	integer function PGS_IO_L0_GetHeader(
       +                         virtual_file,
       +                         header_buffer_size,
       + 			 header_buffer,
       +			 footer_buffer_size,
       +			 footer_buffer)
	integer		virtual_file
	integer		header_buffer_size
	character*(*)	header_buffer
	integer		footer_buffer_size
	character*(*)	footer_buffer


DESCRIPTION:
	This tool reads header and footer information for the currently 
	open physical file into the user-supplied buffers.  It is intended
	to be called whenever the file header and footer data change,
        though it may be called at any time.  
	The file header and footer data change whenever a call 
	to one of the tools causes a new physical file to be opened.
	This will alway occur upon a call to PGS_IO_L0_Open, and may also
	occur upon calls to PGS_IO_L0_SetStart and PGS_IO_L0_GetPacket.  
	These latter two signal this event via a return status code of
	PGSIO_M_L0_HEADER_CHANGED. Typical use of this tool is in a loop of
	calls to read data packets.

INPUTS:
	virtual_file - 	The file descriptor for this virtual data set,
		returned by the call to PGS_IO_L0_Open.

	header_buffer_size - Size in bytes of user-supplied header buffer.
	
	footer_buffer_size - (TRMM only) Size in bytes of user-supplied
		 footer data buffer. If 0, do not read footer data.
	
OUTPUTS:
	header_buffer - User-supplied buffer containing the header, read	
		in from the current physical file.

	footer_buffer - (TRMM only) User-supplied buffer containing 
		 the footer data, read in from the current physical file.

NOTES:
	Memory must be allocated to the output buffers before this 
	tool is called. Failure to do this may result in a core dump.
	(In Fortran 77, the buffer CHARACTER array length must be hardcoded.)

	If the tool determines that the actual size of the file header or footer
	is larger than the user-supplied buffer size, the header or
	footer data is truncated to fit the user buffer.
	In this case, the return status will be:
	PGSIO_W_L0_HDR_BUF_TRUNCATE if the header data has been truncated.
	PGSIO_W_L0_FTR_BUF_TRUNCATE if the footer data has been truncated.
	PGSIO_W_L0_ALL_BUF_TRUNCATE if both the footer data and the header data
	have been truncated.

	To retrieve the header and footer information from the first
	physical file in a virtual data set, this tool must be called after
	first having opened the virtual data set using the tool
	PGS_IO_L0_Open.
	To retrieve the header and footer information from subsequent
	physical files within a virtual data set, this tool should be called
	after the science software receives the return status 
        PGSIO_M_L0_HEADER_CHANGED from the tool PGS_IO_L0_GetPacket.

        A virtual data set must have been opened by PGS_IO_L0_Open
        before this function is called.

	If the header of the currently open physical file is found to be
	corrupted, this function will return a warning to that effect:

	PGSIO_W_L0_HDR_TIME_ORDER
	PGSIO_E_L0_BAD_VAR_HDR_SIZE
	PGSIO_W_L0_BAD_PKT_DATA_SIZE
	PGSIO_W_L0_BAD_PACKET_COUNT
	PGSIO_W_L0_BAD_FOOTER_SIZE
	PGSIO_W_L0_ZERO_PACKET_COUNT
	
	The above returns indicate an error was found in the file header.  The
	header buffer will be returned, although it MAY be truncated.  Similarly
	the footer buffer (TRMM only) may be truncated or even missing if the
	corrupt header file indicated that the start of the footer buffer was
	at an offset (in the file) greater than the size of the physical file.
	The user is cautioned to check the returned buffer(s) carefully in these
	cases.  Further, the user is cautioned that while the function
	PGS_IO_L0_GetPacket() may still be called, that function may produce
	unexpected results if the file header is corrupt.

	TK4 RELEASE NOTES:
        For Level 0 access tools, this delivery consists of prototype
        code.
	This delivery supports the TRMM Level 0 File format, as defined in
	"Interface Control Document between the
	Sensor Data Processing Facility (SPDF) and the Tropical Rainfall
	Measuring Mission (TRMM) Customers", NASA Mission Operations
	and Data Systems Directorate, Draft, Nov. 1994.
        Support for EOS AM and PM Level 0 File formats is limited to
        packet data; file header data for these platforms is not
        defined at this writing (Feb. 1995). For now it is assumed that 
        EOS file headers are identical to the static part of TRMM file headers. 
        Unlike TRMM, it is assumed that EOS file headers have no 
        variable length part, nor do EOS files have footers.


EXAMPLES:
	The example shows how to use this function in conjunction with
        PGS_IO_L0_GetPacket to read Level 0 data from a single virtual data set.
        This algorithm works whether the virtual data set consists of
        only one, or of several physical files.
        All data in the virtual data set are read.

        For clarity, error handling is omitted from the examples.

  C:
#define HEADER_BUFFER_MAX    556  /# max # header bytes #/
#define FOOTER_BUFFER_MAX 100000  /# max # footer bytes #/
#define PACKET_BUFFER_MAX   7132  /# max # packet bytes #/

PGSt_IO_L0_VirtualDataSet  virtual_file;

PGSt_IO_L0_Header  header_buffer[HEADER_BUFFER_MAX];
PGSt_IO_L0_Footer  footer_buffer[FOOTER_BUFFER_MAX];
PGSt_IO_L0_Packet  packet_buf[PACKET_BUFFER_MAX];

PGSt_integer file_loop_flag;
PGSt_integer packet_loop_flag;

file_loop_flag = 1;
while( file_loop_flag )
{
   returnStatus = PGS_IO_L0_GetHeader( virtual_file,
                    HEADER_BUFFER_MAX, header_buffer,
                    FOOTER_BUFFER_MAX, footer_buffer );

/#   Unpack and/or save or process header and footer data here #/

   packet_loop_flag = 1;
   while( packet_loop_flag )
   {
      returnStatus = PGS_IO_L0_GetPacket(
                   virtual_file, PACKET_BUFFER_MAX, packet_buf );

      switch (returnStatus)
      {
	case PGSIO_M_L0_HEADER_CHANGED:
           packet_loop_flag = 0;      /# end of this physical file #/
	   break;
	
	case PGSIO_W_L0_END_OF_VIRTUAL_DS:
	   file_loop_flag = 0;        /# end of this virtual data set #/
           packet_loop_flag = 0;
	   break;
       }

/#   Unpack and/or save or process packet data here #/


   }     /# End while (packet_Loop_flag) #/

}    /# End while (file_Loop_flag) #/

	

  FORTRAN:
        implicit none
	INCLUDE   'PGS_SMF.f'
	INCLUDE   'PGS_PC.f'
	INCLUDE   'PGS_PC_9.f'
	INCLUDE   'PGS_TD.f'
	INCLUDE   'PGS_IO.f'
	INCLUDE   'PGS_IO_1.f'

	character*556  header_buffer
	character*7132  packet_buffer
	character*100000  footer_buffer
	integer pgs_io_l0_getheader
	integer pgs_io_l0_getpacket
	integer      virtual_file
	integer      file_loop_flag
	integer      packet_loop_flag
	integer      returnstatus
	
      file_loop_flag = 1
      do while( file_loop_flag )

         returnstatus = pgs_io_l0_getheader( virtual_file,
    +                556, header_buffer, 100000, footer_buffer )

C   Unpack and/or save or process header and footer data here

         packet_loop_flag = 1
         do while( packet_loop_flag )
   
            returnStatus = pgs_io_l0_getpacket(
    +                             virtual_file, 7132, packet_buf )

            if (returnstatus .eq. PGSIO_M_L0_HEADER_CHANGED) then
               packet_loop_flag = 0      ! end of this physical file
            else if (returnstatus .eq. PGSIO_W_L0_END_OF_VIRTUAL_DS) then
	       file_loop_flag = 0        ! end of this virtual data set
               packet_loop_flag = 0
            end if

C   Unpack and/or save or process packet data here

         end do

      end do

	
RETURNS:
	PGS_S_SUCCESS
	PGSIO_E_L0_BAD_BUF_SIZ
	PGSIO_E_L0_VIRTUAL_DS_NOT_OPEN
	PGSIO_E_L0_FSEEK
	PGSIO_W_L0_HDR_TIME_ORDER
	PGSIO_E_L0_BAD_VAR_HDR_SIZE
	PGSIO_W_L0_BAD_PKT_DATA_SIZE
	PGSIO_W_L0_BAD_PACKET_COUNT
	PGSIO_W_L0_BAD_FOOTER_SIZE
	PGSIO_W_L0_ZERO_PACKET_COUNT
	PGSIO_W_L0_HDR_BUF_TRUNCATE
	PGSIO_W_L0_FTR_BUF_TRUNCATE
	PGSIO_W_L0_ALL_BUF_TRUNCATE
	PGSIO_E_L0_UNEXPECTED_EOF
	PGS_E_UNIX
	PGSIO_E_L0_BAD_SPACECRAFT_TAG

REQUIREMENTS:
        PGSTK-0140, 0210, 0230, 0240

DETAILS:
	This tool first saves the current location of the file pointer.

	It then seeks to the beginning of the physical file, reads the
	header into the user-supplied header buffer.   If the file format 
	is such that the footer data is in a separate area of the file 
	(TRMM), the footer data is read in to the user-supplied footer
	data buffer.

	After the file header and footer data has been read in, the file 
	pointer is restored so that the next call to PGS_IO_L0_GetPacket
	will pick up where it left off.

GLOBALS:
        None

FILES:
	Physical files for the specified virtual data set

FUNCTIONS CALLED:
	PGS_IO_L0_ManageTable
	PGS_SMF_SetStaticMsg
	PGS_SMF_SetUNIXMsg

END_PROLOG
*****************************************************************************/

PGSt_SMF_status 
PGS_IO_L0_GetHeader(
	PGSt_IO_L0_VirtualDataSet virtual_file,
	PGSt_integer		  header_buffer_size,
	PGSt_IO_L0_Header	  *header_buffer,
	PGSt_integer		  footer_buffer_size,
	PGSt_IO_L0_Footer	  *footer_buffer
	)
{
    static char *toolname = "PGS_IO_L0_GetHeader()";

    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */
    PGSt_SMF_status  returnStatus1 = PGS_S_SUCCESS;

    PGSt_IO_L0_FileTable         table_entry;

    PGSt_IO_Gen_FileHandle       *open_fs;

    PGSt_IO_L0_Header           *hdr_EOS_AM;
    PGSt_IO_L0_Header           *hdr_EOS_PM_GIIS;
    PGSt_IO_L0_Header           *hdr_EOS_PM_GIRD;
    PGSt_IO_L0_Header           *hdr_EOS_AURA;

    PGSt_IO_L0_FileHdrTRMM      *hdr_TRMM;
    PGSt_IO_L0_FileHdrADEOS_II  *hdr_ADEOS_II;

    PGSt_integer                 seek_status;
    PGSt_integer                 count;

    long                         unixerror;   /* retains errno value */
    long                         next_packet_position;

    PGSt_IO_L0_VersionTable      *version_array;

    PGSt_IO_L0_Footer            *ftr_TRMM;       /* For reading footer */
    long                         data_to_read;  /* Size of footer to read */

    /* Sanity check on input */

    if ( ( header_buffer_size <= 0 ) || ( footer_buffer_size <= 0 ) )
    {
  	returnStatus = PGSIO_E_L0_BAD_BUF_SIZ;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

   /* Get table entry */

    returnStatus = PGS_IO_L0_ManageTable(
					 PGSd_IO_L0_GetTableEntry, 
					 &virtual_file, 
					 &table_entry, 0);
    if (returnStatus != PGS_S_SUCCESS) 
    {
  	returnStatus = PGSIO_E_L0_VIRTUAL_DS_NOT_OPEN;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    /* Setup physical file variables */

    version_array = &table_entry.version_array[table_entry.open_file_seq_no];

    /*
     *    Get Unix file descriptor for currently open physical file
     */
    open_fs = table_entry.open_file_stream;

    /*
     *    Save file pointer position for next call to PGS_IO_L0_GetPacket
     */
    next_packet_position = ftell( open_fs );


    /* 
     *    Seek to start of file
     */
    seek_status = fseek(open_fs, 0L, SEEK_SET);
    if (seek_status != 0)
    {
  	returnStatus = PGSIO_E_L0_FSEEK;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }
    

    /*
     *    Read in the file header
     *    for the appropriate spacecraft type
     *    
     */

    returnStatus1 = PGS_IO_L0_VersionInfoCheck(version_array,
					       table_entry.spacecraft_tag);
    
    switch (table_entry.spacecraft_tag)
    {
      case PGSd_EOS_AM:

	/* Reset physical file variables */
	
	version_array = &table_entry.version_array[0];

	returnStatus = PGS_IO_Gen_Open(table_entry.file_logical,
				       PGSd_IO_Gen_Read, &open_fs, 1);
	
	if (returnStatus != PGS_S_SUCCESS) 
	{
	    returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    return returnStatus;
	}
        /*
         *     Set up header buffer pointers
         */
        hdr_EOS_AM = header_buffer;

        if (header_buffer_size < version_array->stat_hdr_size) /* sanity check*/
        {
            returnStatus = PGSIO_W_L0_HDR_BUF_TRUNCATE;
           /* Truncate buffer if overflow */
            data_to_read = header_buffer_size;
        }
        else
        {
            data_to_read = version_array->stat_hdr_size;
        }

        /* 
         *    Read in the fixed-length portion of the file header
         */
        count = fread( hdr_EOS_AM, 1, data_to_read, open_fs);
        unixerror=errno;

	PGS_IO_Gen_Close( open_fs );              /* close the header file */
	open_fs = table_entry.open_file_stream;   /* reset the open file ptr */

	/* set the message buffer if the header was truncated, this is done
	   AFTER the call to PGSIO_Gen_Close() so that a successful close will
	   not overwrite the message buffer with a message indicating success */

	if (returnStatus != PGS_S_SUCCESS)
	{
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
	}
	
        if (count != data_to_read )               /* incomplete read */
        {
            if( unixerror == 0 )                  /* reached end-of-file */
            {
               returnStatus = PGSIO_E_L0_UNEXPECTED_EOF;
               PGS_SMF_SetStaticMsg( returnStatus, toolname);
            }
            else                                  /* Unix read error */
            {
               returnStatus = PGS_E_UNIX;
               PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
            }
	    seek_status = fseek(open_fs, next_packet_position, SEEK_SET);
            return returnStatus;
        }

        /* At this writing there is no variable-length part of EOS_AM
                 file headers defined */
	break;

      case PGSd_EOS_PM_GIIS:

	/* Reset physical file variables */
	
	version_array = &table_entry.version_array[0];

	returnStatus = PGS_IO_Gen_Open(table_entry.file_logical,
				       PGSd_IO_Gen_Read, &open_fs, 1);
	
	if (returnStatus != PGS_S_SUCCESS) 
	{
	    returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    return returnStatus;
	}
        /*
         *     Set up header buffer pointers
         */
        hdr_EOS_PM_GIIS = header_buffer;

        if (header_buffer_size < version_array->stat_hdr_size) /* sanity check*/
        {
            returnStatus = PGSIO_W_L0_HDR_BUF_TRUNCATE;
           /* Truncate buffer if overflow */
            data_to_read = header_buffer_size;
        }
        else
        {
            data_to_read = version_array->stat_hdr_size;
        }

        /* 
         *    Read in the fixed-length portion of the file header
         */
        count = fread( hdr_EOS_PM_GIIS, 1, data_to_read, open_fs);
        unixerror=errno;

	PGS_IO_Gen_Close( open_fs );              /* close the header file */
	open_fs = table_entry.open_file_stream;   /* reset the open file ptr */

	/* set the message buffer if the header was truncated, this is done
	   AFTER the call to PGSIO_Gen_Close() so that a successful close will
	   not overwrite the message buffer with a message indicating success */

	if (returnStatus != PGS_S_SUCCESS)
	{
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
	}
	
        if (count != data_to_read )               /* incomplete read */
        {
            if( unixerror == 0 )                  /* reached end-of-file */
            {
               returnStatus = PGSIO_E_L0_UNEXPECTED_EOF;
               PGS_SMF_SetStaticMsg( returnStatus, toolname);
            }
            else                                  /* Unix read error */
            {
               returnStatus = PGS_E_UNIX;
               PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
            }
	    seek_status = fseek(open_fs, next_packet_position, SEEK_SET);
            return returnStatus;
        }

        /* At this writing there is no variable-length part of EOS_PM_GIIS
                 file headers defined */
	break;

      case PGSd_EOS_PM_GIRD:

	/* Reset physical file variables */
	
	version_array = &table_entry.version_array[0];

	returnStatus = PGS_IO_Gen_Open(table_entry.file_logical,
				       PGSd_IO_Gen_Read, &open_fs, 1);
	
	if (returnStatus != PGS_S_SUCCESS) 
	{
	    returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    return returnStatus;
	}
        /*
         *     Set up header buffer pointers
         */
        hdr_EOS_PM_GIRD = header_buffer;

        if (header_buffer_size < version_array->stat_hdr_size) /* sanity check*/
        {
            returnStatus = PGSIO_W_L0_HDR_BUF_TRUNCATE;
           /* Truncate buffer if overflow */
            data_to_read = header_buffer_size;
        }
        else
        {
            data_to_read = version_array->stat_hdr_size;
        }

        /* 
         *    Read in the fixed-length portion of the file header
         */
        count = fread( hdr_EOS_PM_GIRD, 1, data_to_read, open_fs);
        unixerror=errno;

	PGS_IO_Gen_Close( open_fs );              /* close the header file */
	open_fs = table_entry.open_file_stream;   /* reset the open file ptr */

	/* set the message buffer if the header was truncated, this is done
	   AFTER the call to PGSIO_Gen_Close() so that a successful close will
	   not overwrite the message buffer with a message indicating success */

	if (returnStatus != PGS_S_SUCCESS)
	{
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
	}
	
        if (count != data_to_read )               /* incomplete read */
        {
            if( unixerror == 0 )                  /* reached end-of-file */
            {
               returnStatus = PGSIO_E_L0_UNEXPECTED_EOF;
               PGS_SMF_SetStaticMsg( returnStatus, toolname);
            }
            else                                  /* Unix read error */
            {
               returnStatus = PGS_E_UNIX;
               PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
            }
	    seek_status = fseek(open_fs, next_packet_position, SEEK_SET);
            return returnStatus;
        }

        /* At this writing there is no variable-length part of EOS_PM_GIRD
                 file headers defined */
	break;

      case PGSd_EOS_AURA:
 
        /* Reset physical file variables */
 
        version_array = &table_entry.version_array[0];
 
        returnStatus = PGS_IO_Gen_Open(table_entry.file_logical,
                                       PGSd_IO_Gen_Read, &open_fs, 1);
 
        if (returnStatus != PGS_S_SUCCESS)
        {
            returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
        }
        /*
         *     Set up header buffer pointers
         */
        hdr_EOS_AURA = header_buffer;
 
        if (header_buffer_size < version_array->stat_hdr_size) /* sanity check*/
        {
            returnStatus = PGSIO_W_L0_HDR_BUF_TRUNCATE;
           /* Truncate buffer if overflow */
            data_to_read = header_buffer_size;
        }
        else
        {
            data_to_read = version_array->stat_hdr_size;
        }
 
        /*
         *    Read in the fixed-length portion of the file header
         */
        count = fread( hdr_EOS_AURA, 1, data_to_read, open_fs);
        unixerror=errno;
 
        PGS_IO_Gen_Close( open_fs );              /* close the header file */
        open_fs = table_entry.open_file_stream;   /* reset the open file ptr */
 
        /* set the message buffer if the header was truncated, this is done
           AFTER the call to PGSIO_Gen_Close() so that a successful close will
           not overwrite the message buffer with a message indicating success */
 
        if (returnStatus != PGS_S_SUCCESS)
        {
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
        }
 
        if (count != data_to_read )               /* incomplete read */
        {
            if( unixerror == 0 )                  /* reached end-of-file */
            {
               returnStatus = PGSIO_E_L0_UNEXPECTED_EOF;
               PGS_SMF_SetStaticMsg( returnStatus, toolname);
            }
            else                                  /* Unix read error */
            {
               returnStatus = PGS_E_UNIX;
               PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
            }
            seek_status = fseek(open_fs, next_packet_position, SEEK_SET);
            return returnStatus;
        }
 
        /* At this writing there is no variable-length part of EOS_AURA
                 file headers defined */
        break;
 
      case PGSd_TRMM:
        /*
         *     Set up header buffer pointers
         */

        hdr_TRMM = ( PGSt_IO_L0_FileHdrTRMM *) header_buffer;

	data_to_read = version_array->stat_hdr_size +
	               version_array->var_hdr_size;
	
	/* check that the size of the user's input buffer is large enough to
	   hold the file header data (fixed and variable length portions) */

        if (header_buffer_size < data_to_read) /* sanity check*/
        {
            returnStatus = PGSIO_W_L0_HDR_BUF_TRUNCATE;
           /* Truncate buffer if overflow */
            data_to_read = header_buffer_size;
        }

	/* check that the size of the file header data (fixed and variable
	   length portions) is not larger than the file itself */

	if (returnStatus1 == PGSIO_E_L0_BAD_VAR_HDR_SIZE)
	{
	    returnStatus = returnStatus1;
	    if (version_array->file_size < data_to_read)
		data_to_read = version_array->file_size;
	}

	if (returnStatus != PGS_S_SUCCESS)
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);

        /* 
         *    Read in the file header
         */
        count = fread( hdr_TRMM, 1, data_to_read, open_fs);
        unixerror=errno;
        if (count != data_to_read )               /* incomplete read */
        {
            if( unixerror == 0 )                  /* reached end-of-file */
            {
               returnStatus = PGSIO_E_L0_UNEXPECTED_EOF;
               PGS_SMF_SetStaticMsg( returnStatus, toolname);
            }
            else                                  /* Unix read error */
            {
               returnStatus = PGS_E_UNIX;
               PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
            }
	    seek_status = fseek(open_fs, next_packet_position, SEEK_SET);
            return returnStatus;
        }
	
        /*  If user wants to read footer data
              Skip over packets to start of footer data
                (Position was determined previously in 
                 PGS_IO_L0_TRMM_HdrInfo)
              Read in the file footer
                 (For TRMM, this is optionally quality and missing data) 
            End if */

	if (returnStatus1 == PGSIO_W_L0_BAD_PKT_DATA_SIZE)
	{
	    returnStatus = returnStatus1;
	    PGS_SMF_SetStaticMsg(returnStatus, toolname);
	    break;
	}

 	if( footer_buffer_size > 0 )
 	{
 	   fseek( open_fs, version_array->footer_start, SEEK_SET ); 
           if ( footer_buffer_size < version_array->footer_size ) 
           {                                             /* sanity check */
	       if (returnStatus == PGSIO_W_L0_HDR_BUF_TRUNCATE)
	       {
		   returnStatus = PGSIO_W_L0_ALL_BUF_TRUNCATE;
	       }
	       else
	       {
		   returnStatus = PGSIO_W_L0_FTR_BUF_TRUNCATE;
	       }
               PGS_SMF_SetStaticMsg( returnStatus, toolname);
           /* Truncate buffer if overflow */
               data_to_read = footer_buffer_size;
           }
           else
           {           
              data_to_read = version_array->footer_size;
           }

	   if (returnStatus1 == PGSIO_W_L0_BAD_FOOTER_SIZE)
	   {
	       returnStatus = returnStatus1;
	       
	       count = version_array->stat_hdr_size +
		       version_array->var_hdr_size +
		       version_array->pkts_size;
	       
	       count = version_array->file_size - count;
	       if (count > data_to_read)
		   data_to_read = count;
               PGS_SMF_SetStaticMsg( returnStatus, toolname);
	   }

           ftr_TRMM = ( PGSt_IO_L0_Footer *) footer_buffer;
           count = fread( ftr_TRMM, 1, data_to_read, open_fs);
           unixerror=errno;
           if (count != data_to_read )          /* incomplete read */
           {
               if( unixerror == 0 )                  /* reached end-of-file */
               {
                  returnStatus = PGSIO_E_L0_UNEXPECTED_EOF;
                  PGS_SMF_SetStaticMsg( returnStatus, toolname);
               }
               else                                  /* Unix read error */
               {
                  returnStatus = PGS_E_UNIX;
                  PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
               }
	       seek_status = fseek(open_fs, next_packet_position, SEEK_SET);
               return returnStatus;
           }
 	   
 	}
 	break;

      case PGSd_ADEOS_II:
        /*
         *     Set up header buffer pointers
         */
        hdr_ADEOS_II = ( PGSt_IO_L0_FileHdrADEOS_II *) header_buffer;

        if (header_buffer_size < version_array->stat_hdr_size) /* sanity check*/
        {
            returnStatus = PGSIO_W_L0_HDR_BUF_TRUNCATE;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
           /* Truncate buffer if overflow */
            data_to_read = header_buffer_size;
        }
        else
        {
            data_to_read = version_array->stat_hdr_size;
        }

        /* 
         *    Read in the fixed-length portion of the file header
         */
        count = fread( hdr_ADEOS_II, 1, data_to_read, open_fs);
        unixerror=errno;
        if (count != data_to_read )     /* incomplete read */
        {
            if( unixerror == 0 )                  /* reached end-of-file */
            {
               returnStatus = PGSIO_E_L0_UNEXPECTED_EOF;
               PGS_SMF_SetStaticMsg( returnStatus, toolname);
            }
            else                                  /* Unix read error */
            {
               returnStatus = PGS_E_UNIX;
               PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
            }
	    seek_status = fseek(open_fs, next_packet_position, SEEK_SET);
            return returnStatus;
        }

        /* At this writing there is no variable-length part of ADEOS_II
                 file headers defined */
	break;

      default:
        returnStatus = PGSIO_E_L0_BAD_SPACECRAFT_TAG;
	PGS_SMF_SetStaticMsg(returnStatus, toolname);
	return returnStatus;
    }
    

    /* 
     *    Restore file pointer position for next call to PGS_IO_L0_GetPacket
     */
    seek_status = fseek(open_fs, next_packet_position, SEEK_SET);
    if (seek_status != 0)
    {
  	returnStatus = PGSIO_E_L0_FSEEK;
        PGS_SMF_SetStaticMsg( returnStatus, toolname);
    }

    if (returnStatus == PGS_S_SUCCESS)
    {
	if (returnStatus1 != PGS_S_SUCCESS)
	    returnStatus = returnStatus1;
	
	PGS_SMF_SetStaticMsg(returnStatus, toolname);
    }
    
    return returnStatus;
}
