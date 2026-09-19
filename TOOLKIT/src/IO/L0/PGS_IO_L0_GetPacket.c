/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_GetPacket.c

DESCRIPTION:
  This file contains the I/O tool to get a single packet
  from the specified Level 0 Virtual Data Set.

AUTHOR:
  Mike Sucher / Applied Research Corp.
  Tom W. Atwater / Applied Research Corp.
  Guru Tej S. Khalsa / Applied Research Corp.
  Abe Taaheri / SM&A Corp.
  Xin Wang / EIT Inc.

HISTORY:
               MS  Designed
  30-Jan-1995 TWA  Initial version
  19-Jul-1995 GTSK Improved error handling
  06-Jul-1999  RM  updated for thread-safe functionality
  01-Dec-1999 AT   Modified to support GIRD and GIIS formats for PM spacecraft
  01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>
#include <PGS_TSF.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Get a Single Packet From the Specified Virtual Data Set
	
NAME:
	PGS_IO_L0_GetPacket
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_GetPacket(
		PGSt_IO_L0_VirtualDataSet virtual_file,
		PGSt_integer		  packet_buffer_size,
		PGSt_IO_L0_Packet	  *packet_buffer
		)
  FORTRAN:
	INCLUDE   'PGS_SMF.f'
	INCLUDE   'PGS_PC.f'
	INCLUDE   'PGS_PC_9.f'
	INCLUDE   'PGS_TD.f'
	INCLUDE   'PGS_IO.f'
	INCLUDE   'PGS_IO_1.f'

	integer function PGS_IO_L0_GetPacket(
       +                         virtual_file,
       +                         packet_buffer_size,
       +                         packet_buffer)
	integer		virtual_file
	integer		packet_buffer_size
	character*(*)	packet_buffer


DESCRIPTION:
	Each call to tool reads a single data packet from a Level 0 
	virtual data set into the user-supplied buffer.  The file
	position pointer is updated so that the next call to this 
	tool gets the next packet.
	
INPUTS:
	virtual_file - 	The file descriptor for this virtual data set,
		returned by the call to PGS_IO_L0_Open.

	packet_buffer_size - Size in bytes of user-supplied packet buffer.

OUTPUTS:
	packet_buffer - User-supplied buffer containing the data packet 
		read in from the specified virtual data set.
	
NOTES:
	Memory must be allocated to the output buffer before this 
	tool is called. Failure to do this may result in a core dump.
	(In Fortran 77, the buffer CHARACTER array length must be hardcoded.)

	Normal return is PGS_S_SUCCESS.  If getting the next packet 
	requires that a new physical file be opened, the header and
	quality data will change.   In this case, the return status
	is set to PGSIO_M_L0_HEADER_CHANGED.   This allows the user
	to test the return status and get updated header and quality 
	data using the tool PGS_IO_L0_GetHeader, in the case where
        there is more than one physical file per virtual data set.
	
	If the tool determines that the size of the packet is larger 
	than the user buffer size, as specified by the parameter
	packet_size, it will truncate the packet to fit the user buffer.
	In this case, the return status will be PGSIO_W_L0_PKT_BUF_TRUNCATE.

        Packet formats for TRMM, EOS AM, EOS PM (GIIS and GIRD formats) 
	and ADEOS-II are supported.

        A virtual data set must have been opened by PGS_IO_L0_Open
        before this function is called.

        This function returns no data if the packet buffer size
        is less than 6 bytes (the primary packet header size).
        It returns a warning and a truncated buffer if the packet
        buffer size is 6 or more bytes but less than the actual
        packet length.

	TK4 RELEASE NOTES:
        For Level 0 access tools, this delivery consists of prototype
        code.
	This function has not been optimized for speed.

EXAMPLES:
	The example shows how to use this function in conjunction with
        PGS_IO_L0_GetPacket to read Level 0 data from a single virtual data set.
        This algorithm works whether the virtual data set consists of
        only one, or of several physical files.
        All data in the virtual data set are read.

        For clarity, error handling is omitted from the examples.

  C:
#define HEADER_BUFFER_MAX    556  /# max # of bytes in header buffer #/
#define FOOTER_BUFFER_MAX 100000  /# max # of bytes in footer buffer #/
#define PACKET_BUFFER_MAX   7132  /# max # of bytes in packet buffer #/

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

	integer HEADER_BUFFER_MAX
	parameter (HEADER_BUFFER_MAX=556)
	character*556  header_buffer
	integer PACKET_BUFFER_MAX
	parameter (PACKET_BUFFER_MAX=7132)
	character*7132  packet_buffer
	integer FOOTER_BUFFER_MAX
	parameter (FOOTER_BUFFER_MAX=100000)
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
    >                HEADER_BUFFER_MAX, header_buffer,
    >                FOOTER_BUFFER_MAX, footer_buffer )

!   Unpack and/or save or process header and footer data here

         packet_loop_flag = 1
         do while( packet_loop_flag )
   
            returnStatus = pgs_io_l0_getpacket(
    >               virtual_file, PACKET_BUFFER_MAX, packet_buf )

            if (returnstatus .eq. PGSIO_M_L0_HEADER_CHANGED) then
               packet_loop_flag = 0      ! end of this physical file
            else if (returnstatus .eq. PGSIO_W_L0_END_OF_VIRTUAL_DS) then
	       file_loop_flag = 0        ! end of this virtual data set
               packet_loop_flag = 0
            end if

!   Unpack and/or save or process packet data here

         end do

      end do


RETURNS:
	PGS_S_SUCCESS
	PGSIO_E_L0_MANAGE_TABLE
	PGSIO_E_L0_PHYSICAL_NOT_OPEN
	PGSIO_E_L0_PKT_BUF_OVERFLOW
	PGSIO_E_L0_UNEXPECTED_EOF
	PGSIO_W_L0_PKT_BUF_TRUNCATE
	PGSIO_W_L0_END_OF_VIRTUAL_DS
	PGSIO_M_L0_HEADER_CHANGED
	PGSIO_E_L0_NEXT_PHYSICAL
	PGSIO_E_L0_SEEK_1ST_PACKET
	PGSIO_W_L0_BUFTRUNC_END_DS
	PGSIO_W_L0_BUFTRUNC_HDR_CHG
	PGSIO_E_L0_BUFTRUNC_NXTFILE
	PGSTSF_E_GENERAL_FAILURE
	PGS_E_UNIX
	
REQUIREMENTS:
	PGSTK-0140, 0200, 0240

DETAILS:
	This tool will, if necessary, call PGS_IO_L0_NextPhysical,
	which closes the current physical file, opens the correct 
	physical file containing the next packet and updates the file
	table to reflect this.  Because each physical file contains
	different header data, the user is notified via the return code 
	PGSIO_M_L0_HEADER_CHANGED.  The new header can be retrieved via
	the tool PGS_IO_L0_GetHeader.

	Because the data packets are variable-length, the packet read is
        actually a two-step process.  First, the primary packet header is
	read in.  The format of this header is standardized across all
	S/C platforms, and it includes a field that gives the length of the 
	remainder of the packet.  Once this value has been determined, 
	the rest of the packet is read in.

        Tool assumes high-order byte is first when converting byte data 
        to integer.

	CAUTION: This tool may produce unexpected results if the file header
                 of the physical file from which the packet is being retrieved
		 is corrupted.  The function PGS_IO_L0_GetHeader() should be
		 called before calling this function.  That function may
		 identify the file header as being corrupt.  This function may
		 still be called if the file header is corrupt, but the results
		 may be unexpected/unreliable in that case.

GLOBALS:
	PGSg_TSF_IOL0RetStat2

FILES:
	Physical files for the specified virtual data set

FUNCTIONS CALLED:
	PGS_IO_L0_ManageTable
	PGS_IO_L0_NextPhysical
	PGS_SMF_SetStaticMsg
	PGS_SMF_SetUNIXMsg
	PGS_SMF_TestErrorLevel
	PGS_TSF_GetTSFMaster
	PGS_TSF_GetMasterIndex

END_PROLOG
*****************************************************************************/

PGSt_SMF_status 
PGS_IO_L0_GetPacket(
	PGSt_IO_L0_VirtualDataSet   virtual_file,
	PGSt_integer                packet_buffer_size,
	PGSt_IO_L0_Packet           *packet_buffer
	)
{
    static char *toolname = "PGS_IO_L0_GetPacket()";

    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;    /* return status */
    PGSt_SMF_status returnStatus1 = PGS_S_SUCCESS;   /* return status */
#ifdef _PGS_THREADSAFE
    /* create non-static vars for TSF */
    PGSt_SMF_status returnStatus2;                   /* return status */
    PGSt_IO_L0_FileTable	*t_ptr;
#else
    static PGSt_SMF_status returnStatus2 = PGS_S_SUCCESS;   /* return status */

    static PGSt_IO_L0_FileTable	*t_ptr = 0;
#endif

    PGSt_IO_L0_PriPktHdr         *pri_pkt_header;
    PGSt_IO_Gen_FileHandle       *open_fs;

    PGSt_integer                 packet_actual_size;
    PGSt_integer                 packet_data_length;
    PGSt_integer                 packet_unused;

    PGSt_boolean                 use_pkt_size;
    PGSt_boolean                 single_hdr;
    
    long                         packet_read_count;
    long                         count;
    long                         unixerror;   /* retains errno value */

    PGSt_IO_L0_VersionTable      *version_array;

#ifdef _PGS_THREADSAFE
    /*
     *    Set up for TSF - get global, global index, and TSF key keeper.
     */
    extern PGSt_SMF_status PGSg_TSF_IOL0RetStat2[];
    PGSt_TSF_MasterStruct *masterTSF;
    int masterTSFIndex;

    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ((PGS_SMF_TestErrorLevel(returnStatus)) || 
        (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /*
     *    Set from global counterpart and TSD key
     */
    returnStatus2 = PGSg_TSF_IOL0RetStat2[masterTSFIndex];
    t_ptr = (PGSt_IO_L0_FileTable *) pthread_getspecific(
                         masterTSF->keyArray[PGSd_TSF_KEYIOL0TPTR]);
#endif


    /*
     *    If this function is being called for the first time,
     *    then get the file table pointer.  This pointer is used
     *    for quick lookup of the current file stream pointer.
     *    Other access to file table is done via standard calls
     *    to PGS_IO_L0_ManageTable for uniformity.
     */
    if (t_ptr == 0)
    {
       /* Sanity check on input */
       /* (Placed here to save time) */
       if ( packet_buffer_size <= 0 )
       {
  	   returnStatus = PGSIO_E_L0_BAD_BUF_SIZ;
	   PGS_SMF_SetStaticMsg( returnStatus, toolname);
	   return returnStatus;
       }
        returnStatus = PGS_IO_L0_ManageTable(
	     PGSd_IO_L0_GetTablePointer, &virtual_file, 
             0, &t_ptr);
#ifdef _PGS_THREADSAFE
        /*
         *    Re-set TSD key
         */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYIOL0TPTR],t_ptr);
#endif
        if (returnStatus != PGS_S_SUCCESS) 
        {
	    returnStatus = PGSIO_E_L0_MANAGE_TABLE;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    return returnStatus;
        }
    }


    /*
     *    Get Unix file descriptor for currently open physical file
     */
    open_fs = t_ptr[virtual_file].open_file_stream;
    if (open_fs == 0)/* virtual_file is incorrect or no physical file is open */
    {
   	returnStatus = PGSIO_E_L0_PHYSICAL_NOT_OPEN;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    /*
     *    if this is EOS AM or PM (GIIS or GIRD formats) count bytes 
     *    read NOT number of packets read since the number of packets 
     *    in any one physical file of the data set is not known.
     */

    switch (t_ptr[virtual_file].spacecraft_tag)
    {
      case PGSd_EOS_AM:
	use_pkt_size = PGS_TRUE;
	single_hdr = PGS_TRUE;
	break;

      case PGSd_EOS_PM_GIIS:
	use_pkt_size = PGS_TRUE;
	single_hdr = PGS_TRUE;
	break;

      case PGSd_EOS_PM_GIRD:
	use_pkt_size = PGS_TRUE;
	single_hdr = PGS_TRUE;
	break;

      case PGSd_EOS_AURA:
        use_pkt_size = PGS_TRUE;
        single_hdr = PGS_TRUE;
        break;
 
      default:
	use_pkt_size = PGS_FALSE;
	single_hdr = PGS_FALSE;
    }
    
    /*
     *    Set up to get other info about currently open physical file
     */
    version_array = 
     &t_ptr[virtual_file].version_array[ t_ptr[virtual_file].open_file_seq_no ];

    /*
     *    If no packets have been read yet, check the physical file header.  If
     *    the return status is not PGS_S_SUCCESS then we are likely to run into
     *    trouble in dealing with this file and special precautions will be
     *    taken to do the best we can with a corrupt file header.
     */

    if (version_array->num_pkts_read == 0UL)
    {
	returnStatus2 = PGS_IO_L0_VersionInfoCheck(
	                                  version_array,
					  t_ptr[virtual_file].spacecraft_tag);
#ifdef _PGS_THREADSAFE
        /*
         *    Re-set global
         */
        PGSg_TSF_IOL0RetStat2[masterTSFIndex] = returnStatus2;
#endif
    }

    /*
     *     Set up packet buffer pointers
     */
    if (packet_buffer_size < PGSd_IO_L0_PriPktHdrSize)  /* sanity check */
    {
        returnStatus = PGSIO_E_L0_PKT_BUF_OVERFLOW;
        PGS_SMF_SetStaticMsg( returnStatus, toolname);
        return returnStatus;
    }
    pri_pkt_header = ( PGSt_IO_L0_PriPktHdr *) packet_buffer;
    packet_buffer += PGSd_IO_L0_PriPktHdrSize;


    /*
     *    Get the next packet
     */
    
    /*
     *    Read the primary packet header
     */
    count = fread( pri_pkt_header, 1, PGSd_IO_L0_PriPktHdrSize, open_fs);
    unixerror=errno;
    
    if (count != PGSd_IO_L0_PriPktHdrSize) /* incomplete packet read */
    {
	if (unixerror == 0) /* reached end of file */
	{
	    returnStatus = PGSIO_E_L0_UNEXPECTED_EOF;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	}
	else	 /* assume Unix I/O error */        	
	{
	    if (version_array->num_pkts_read == 0UL)
	    {
		/* if this is the first attempt to read a packet in this
		   physical file, return error indicating that the 1st packet
		   could not be found... */

		returnStatus = PGSIO_E_L0_SEEK_1ST_PACKET;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    }
	    else
	    {
		/* ...otherwise return a generic unix error */

		returnStatus = PGS_E_UNIX;
		PGS_SMF_SetUNIXMsg( errno,NULL, toolname); /* error */
	    }
	}
	
	/* if this section of code was reached then a primary packet header
	   could not be properly read, which means no packet length can be
	   determined, so set file pointer to end of file and set the
	   num_pkts_read counter in the version_array to the value of
	   packet_count so that this function will know not try to read any
	   further values from this physical file */

	fseek(open_fs, 0L, SEEK_END);
	version_array->num_pkts_read = version_array->packet_count;
	returnStatus2 = PGS_E_TOOLKIT;
#ifdef _PGS_THREADSAFE
        /*
         *    Re-set global
         */
        PGSg_TSF_IOL0RetStat2[masterTSFIndex] = returnStatus2;
#endif
    }
    
    /*
     *    Calculate actual packet size, using the 16-bit value in
     *    the primary packet header. 
     */
    
    /*
     *
     * IMPORTANT TK4 PROTOTYPE NOTE (29-Dec-1994): 
     * The Packet Length field is defined in section 4.2.6 of the white 
     * paper, "Level 0 Data Issues for the ECS Project" (Sept 1994), as: 
     * "the length of the entire packet,  in bytes, less the length  
     * of the primary packet header (6 bytes) less one byte".
     * 
     * This is equivalent to the total size in bytes of Secondary 
     * Header plus the Application Data minus 1. 
     * 
     * The SDPF/TRMM ICD specifies that most significant byte/bit
     * is first.  We assume this is also true for EOS AM & PM.
     */
    
    packet_data_length = 
	(pri_pkt_header->pktLength[1]) +
	(pri_pkt_header->pktLength[0] * 256)
	+ 1;

    /* If returnStatus2 is not PGS_S_SUCCESS then we are dealing with a corrupt
       header.  Make sure that the packet_data_length which has been calculated
       above (and may have been calculated from the wrong bytes in the
       file--e.g. bytes other than those that are SUPPOSED to indicate packet
       length) does not indicate a value that takes us past the end of the
       physical file.  If so, set the maximum value of packet_data_length to the
       number of bytes between current file position and the end of the file. */

    if (returnStatus2 != PGS_S_SUCCESS)
    {
	if ((ftell(open_fs)+packet_data_length) > version_array->file_size)
	{
	    packet_data_length = version_array->file_size - ftell(open_fs);
	    version_array->num_pkts_read = version_array->packet_count;
	}
    }
    
    packet_actual_size = PGSd_IO_L0_PriPktHdrSize  + packet_data_length;
    
    
    /*
     *    Calculate the number of unused bytes in the user buffer.
     *    Read in the secondary packet and the data.  If the number 
     *    of unused bytes is negative, the buffer will be truncated.
     */
    packet_unused = packet_buffer_size - packet_actual_size;
    
    if (packet_unused >= 0 ) /* bytes to spare */
    {
	packet_read_count = packet_data_length;
	count = fread( packet_buffer, 1, packet_read_count, open_fs);
	unixerror=errno;
    }
    else			/* truncate read */
    {
	returnStatus = PGSIO_W_L0_PKT_BUF_TRUNCATE;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	
	packet_read_count = packet_data_length + packet_unused;
	count = fread( packet_buffer, 1, packet_read_count, open_fs);
	unixerror=errno;
	
	/* skip bytes not read in */
	fseek(open_fs, (long) -packet_unused, SEEK_CUR);
    }
    
    /*
     *    Check for end-of-file or other error during read
     */
    if (count != packet_read_count) /* didn't get full packet */
    {
	if (unixerror == 0)	/* EOF: this should never happen */
	{
	    /* if the end of the file was unexpectedly reached, set an error and
	       set num_pkts_read to the value of packet_count so that we don't
	       attempt to read anymore packets from this file */

	    returnStatus = PGSIO_E_L0_UNEXPECTED_EOF;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    fseek(open_fs, 0L, SEEK_END);
	    version_array->num_pkts_read = version_array->packet_count;
	}
	else		/* assume Unix I/O error */
	{
	    /* If an unexpected unix error occured reading in the packet
	       (actually the portion of the packet after the primary header),
	       then use fseek to manually move the file pointer to the end of
	       the current packet as defined in the packet primary header.  If
	       this takes us past the end of the file, then set num_pkts_read to
	       the value of packet_count so that we don't attempt to read
	       anymore packets from this file. */

	    returnStatus = PGS_E_UNIX;
	    PGS_SMF_SetUNIXMsg( errno,NULL, toolname); /* error */
	    fseek(open_fs, (packet_read_count-count), SEEK_CUR);
	    if (ftell(open_fs) >= version_array->file_size)
	    {
		fseek(open_fs, 0L, SEEK_END);
		version_array->num_pkts_read = version_array->packet_count;
	    }
	}
    }
    
    /*
     *    Increment num packets read in file.
     */

    if (use_pkt_size == PGS_TRUE)
    {
	version_array->num_pkts_read += packet_actual_size;
    }
    else
    {
	version_array->num_pkts_read++;
    }

    /* if current file header indicated a zero packet count then do some special
       handling here--namely continue reading from the file for as long as we
       are still in the data packets area of the file (as indicated by other
       data in the file header) */
      
    if (returnStatus2 == PGSIO_W_L0_ZERO_PACKET_COUNT)
    {
	if (ftell(open_fs) < version_array->footer_start)
	{
	    if (returnStatus == PGS_S_SUCCESS)
		PGS_SMF_SetStaticMsg(returnStatus, toolname);
	    return returnStatus;
	}
    }
    
    
    /*
     *     If there are no more packets in this physical file
     *        Attempt to open next physical file
     *        If unable to open it
     *           Set return status to indicate either
     *              -- No more L0 data left to process, or
     *              -- Error opening next physical file
     *        Else
     *           Set return status to indicate next physical file header
     *              now needs to be read
     *        End if/else
     *     End if
     *     Return
     */


    if ( version_array->num_pkts_read >= version_array->packet_count )
    {
        returnStatus1 = PGS_IO_L0_NextPhysical(virtual_file);

        if (returnStatus1 != PGS_S_SUCCESS) 
        {
           switch (returnStatus1)
           {
	     case PGSIO_W_L0_END_OF_VIRTUAL_DS:
	       if (returnStatus == PGSIO_W_L0_PKT_BUF_TRUNCATE)
               {
		   returnStatus = PGSIO_W_L0_BUFTRUNC_END_DS;
               }
	       else if (returnStatus == PGS_S_SUCCESS)
               {
		   returnStatus = PGSIO_W_L0_END_OF_VIRTUAL_DS;
               }
               else
               {
                   return returnStatus;
               }
	       break;
	     default:
	       if (returnStatus == PGSIO_W_L0_PKT_BUF_TRUNCATE)
               {
		   returnStatus = PGSIO_E_L0_BUFTRUNC_NXTFILE;
               }
	       else if (returnStatus == PGS_S_SUCCESS)
               {
		   returnStatus = PGSIO_E_L0_NEXT_PHYSICAL;
               }
               else
               {
                   return returnStatus;
               }
           }
        }
        else
        {

        /* set return status to indicate new header read needed (if this is NOT
	   EOS AM or PM which only has one header per data set) */

	    if (single_hdr == PGS_FALSE)
	    {
		if (returnStatus == PGSIO_W_L0_PKT_BUF_TRUNCATE)
		{
		    returnStatus = PGSIO_W_L0_BUFTRUNC_HDR_CHG;
		}
		else if (returnStatus == PGS_S_SUCCESS)
		{
		    returnStatus = PGSIO_M_L0_HEADER_CHANGED;
		}
                else
                {
                    return returnStatus;
                }
	    }
        }
        PGS_SMF_SetStaticMsg( returnStatus, toolname);
    }    

    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus, toolname);
    
    return returnStatus;

}
