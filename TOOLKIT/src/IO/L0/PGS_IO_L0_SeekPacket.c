/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_SeekPacket.c

DESCRIPTION:
  This file contains the low-level I/O tool to seek to the specified packet 
  in the current Level 0 physical file.

AUTHOR:
  Mike Sucher / Applied Research Corp.
  Carol S. W. Tsai / Applied Research Corp.
  Abe Taaheri / SM&A Corp.
  Xin Wang / EIT Inc.

HISTORY:
  29-Jan-1995 TWA  Initial version
  22-Jan-1997 CSWT Added an output parameter packet_ct as a return value of 
                   the position index of the file poineter at the start of the
                   desired packet selected at or after the start specified time. 
  06-Jul-1999  RM  updated for thread-safe functionality
  01-Dec-1999  AT  Modified to support GIRD and GIIS formats for PM spacecraft
  04-Aug-2000  AT  Modified code for AM so that when the micro second part of
                   a packet has bit flip problem a warning is issued and search
                   for desired packet is continued.
  01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>
#include <PGS_TSF.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Seek to the specified packet in current physical file
NAME:
	PGS_IO_L0_SeekPacket
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_SeekPacket(
		PGSt_IO_L0_VirtualDataSet virtual_file,
		PGSt_integer		  packet_number,
		PGSt_integer*		  packet_ct,
		PGSt_double		  start_time
		)
  FORTRAN:
	N/A

DESCRIPTION:
	This tool goes to the currently opened physical file and sets the
	file pointer so that the next call to PGS_IO_L0_GetPacket returns
	the specified packet.   Packet may be specified by either packet
	number or by packet start time.   To select packet by start time,
	the packet number must be set to 0.  The actual packet selected 
	will be the first available packet at or after the specified time.
	
INPUTS:
	virtual_file - 	The file descriptor for this virtual data set,
		returned by the call to PGS_IO_L0_Open.

	packet_number - The toolkit internal sequence number of the packet to 
		be read in by the next call to PGS_IO_L0_Getpacket.

		If packet_number is set to zero, use the parameter start_time 
		to seek to the desired packet.

	start_time - The start time of the packet to  be read in by the next
		call to PGS_IO_L0_Getpacket.   The actual packet to be read
		will be the first available packet at or after the specified
		time.  This parameter is ignored if packet_number is non-zero.

	        Format is TAI: continuous seconds since 12AM UTC Jan 1, 1993

OUTPUTS:
        packet_ct - The position index of the packet selected at or after the
                    specified start_time or packet_number (see above)

NOTES:
	This is a low-level tool.  It is not intended to be called directly by 
	any user code.

        IMPORTANT TK4 PROTOTYPE NOTES (28-Dec-1994):
        ============================================
        The prototype version of the Level 0 tools uses fake file header
        formats because the real formats were not available in time for
        use in the code.  This version of the tool assumes that packets 
	appear from the end of the file header section to the end of file.
	In the later releases this will change for any file format which
	appends additional data after the last packet in the file.
	
EXAMPLES:
	

RETURNS:
	PGS_S_SUCCESS
	PGSIO_E_L0_PACKET_NOT_FOUND  
	PGSIO_E_L0_ILLEGAL_PACKET_NUM
	PGSIO_E_L0_BAD_SPACECRAFT_TAG
	PGSIO_E_L0_FSEEK
	PGSIO_E_L0_MANAGE_TABLE
	PGSIO_E_L0_TIME_CONVERSION
	PGSTSF_E_GENERAL_FAILURE
	PGS_E_UNIX

REQUIREMENTS:
	PGSTK-0200, 0240

DETAILS:
	The first call to fseek repositions the file pointer at the start of 
	the current physical file.   It then reads enough of the file header 
	to determine the file header length.  The rest of the header is skipped 
	via a call to fseek.  Then one of two loops is entered, depending on 
	the search method.   

	If the position is selected by packet number, a loop reads the 
	primary packet header and uses fseek to skip past the variable 
	length portion of each packet.  This continues for the specified 
	number of iterations.

	If the position is selected by packet start, time, a loop reads both 
	the primary and spacecraft-dependent secondary packet headers.  The 
	tool PGS_IO_LO_PacketTime is then called to decode the packet time 
	stamp, which is then compared to the desired start time to determine 
	whether the correct packet has been reached.

        Tool assumes high-order byte is first when converting byte data 
        to integer.

GLOBALS:
        None

FILES:
	Physical files for the specified virtual data set

FUNCTIONS CALLED:
	PGS_SMF_SetStaticMsg
	PGS_SMF_SetUNIXMsg
	PGS_IO_L0_ManageTable
	PGS_TD_EOSAMtoTAI
	PGS_TD_EOSPMGIIStoTAI
	PGS_TD_EOSPMGIRDtoTAI
	PGS_TD_EOSAURAtoTAI
        PGS_TD_TRMMtoTAI
	PGS_TSF_GetTSFMaster

END_PROLOG
*****************************************************************************/

PGSt_SMF_status 
PGS_IO_L0_SeekPacket(
	PGSt_IO_L0_VirtualDataSet virtual_file,
	PGSt_integer		packet_number,
	PGSt_integer*		npacket_ct,
	PGSt_double		start_time
	)
{
    static char *toolname = "PGS_IO_L0_SeekPacket";

    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */
    
    PGSt_IO_Gen_FileHandle   *open_fs;

    PGSt_IO_L0_FileTable     table_entry;


    PGSt_IO_L0_PriPktHdr     pri_pkt_header;    /* primary packet header */

#ifdef _PGS_THREADSAFE
    /*
     *  Non-static vars
     */
    PGSt_IO_L0_SecPktHdrEOS_AM   *sec_pkt_hdr_EOS_AM;  /* 2ndary packet header */
    PGSt_IO_L0_SecPktHdrEOS_PM_GIIS   *sec_pkt_hdr_EOS_PM_GIIS;  /* 2ndary packet
							       header */
						      
    PGSt_IO_L0_SecPktHdrEOS_PM_GIRD    *sec_pkt_hdr_EOS_PM_GIRD;  /* 2ndary packet
							       header  */
    PGSt_IO_L0_SecPktHdrEOS_AURA   *sec_pkt_hdr_EOS_AURA;  /* 2ndary packet header */
    PGSt_IO_L0_SecPktHdrTRMM     *sec_pkt_hdr_TRMM;    /* 2ndary packet header */
    PGSt_IO_L0_SecPktHdrADEOS_II *sec_pkt_hdr_ADEOS_II;/* 2ndary packet header */
#else
    static PGSt_IO_L0_SecPktHdrEOS_AM   sec_pkt_hdr_EOS_AM;  /* 2ndary packet
								header */
    static PGSt_IO_L0_SecPktHdrEOS_PM_GIIS  sec_pkt_hdr_EOS_PM_GIIS;  
                                                   /* 2ndary packet header */
    static PGSt_IO_L0_SecPktHdrEOS_PM_GIRD   sec_pkt_hdr_EOS_PM_GIRD;  
                                                   /* 2ndary packet header */
    static PGSt_IO_L0_SecPktHdrEOS_AURA   sec_pkt_hdr_EOS_AURA;  /* 2ndary packet
                                                                header */
    static PGSt_IO_L0_SecPktHdrTRMM     sec_pkt_hdr_TRMM;    /* 2ndary packet header */
    static PGSt_IO_L0_SecPktHdrADEOS_II sec_pkt_hdr_ADEOS_II;/* 2ndary packet header */
#endif

    PGSt_integer             sec_pkt_hdr_size;

    PGSt_integer             packet_ct;
    PGSt_integer             count;
    PGSt_integer             seek_status;
    PGSt_integer             packet_data_length;

    PGSt_integer             packet_found;

    PGSt_boolean             use_pkt_size;

    PGSt_double		     packet_time;

    long                     unixerror;              /* retains errno value */

    PGSt_IO_L0_VersionTable  *version_entry;
    PGSt_SMF_status  	     returnStatus2 = PGS_S_SUCCESS;
    PGSt_SMF_status  	     returnStatus1 = PGS_S_SUCCESS;
    char         specifics[PGS_SMF_MAX_MSG_SIZE];     /* detailed error msg */
    char         errorBuf[PGS_SMF_MAX_MSGBUF_SIZE];   /* appended error msg */

#ifdef _PGS_THREADSAFE
    /*
     *  Setup to get TSD from keys
     */
    PGSt_TSF_MasterStruct *masterTSF;

    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /*
     *  Get data from keys
     */
    sec_pkt_hdr_EOS_AM = (PGSt_IO_L0_SecPktHdrEOS_AM *) pthread_getspecific(
                                 masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSAM]);
    sec_pkt_hdr_EOS_PM_GIIS = (PGSt_IO_L0_SecPktHdrEOS_PM_GIIS *) pthread_getspecific(
                                 masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSPMGIIS]);
    sec_pkt_hdr_EOS_PM_GIRD = (PGSt_IO_L0_SecPktHdrEOS_PM_GIRD *) pthread_getspecific(
                                 masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSPMGIRD]);
    sec_pkt_hdr_EOS_AURA = (PGSt_IO_L0_SecPktHdrEOS_AURA *) pthread_getspecific(
                                 masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSAURA]);
    sec_pkt_hdr_TRMM = (PGSt_IO_L0_SecPktHdrTRMM *) pthread_getspecific(
                                 masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTTRMM]);
    sec_pkt_hdr_ADEOS_II = (PGSt_IO_L0_SecPktHdrADEOS_II *) pthread_getspecific(
                                 masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTADEOSII]);
#endif

    /*
     *  Initialize variable
     */


    if (packet_number < 0)                      /* illegal packet number */
    {
        returnStatus = PGSIO_E_L0_ILLEGAL_PACKET_NUM; 
        PGS_SMF_SetStaticMsg( returnStatus, toolname);
        return returnStatus;
    }


    /* 
     *    Get table entry for the specified virtual data set
     */

    returnStatus = PGS_IO_L0_ManageTable(
					 PGSd_IO_L0_GetTableEntry, 
					 &virtual_file, 
					 &table_entry, 0);
    if (returnStatus != PGS_S_SUCCESS) 
    {
        returnStatus = PGSIO_E_L0_MANAGE_TABLE;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    switch (table_entry.spacecraft_tag)
    {
      case PGSd_EOS_AM:
	use_pkt_size = PGS_TRUE;
	break;

      case PGSd_EOS_PM_GIIS:
	use_pkt_size = PGS_TRUE;
	break;

      case PGSd_EOS_PM_GIRD:
	use_pkt_size = PGS_TRUE;
	break;

      case PGSd_EOS_AURA:
        use_pkt_size = PGS_TRUE;
        break;
 
      default:
	use_pkt_size = PGS_FALSE;
    }

    /* 
     *    Get the pointer to the current physical file
     *       and the file table entry
     */

    open_fs = table_entry.open_file_stream;
    version_entry = &table_entry.version_array[table_entry.open_file_seq_no];

    /*
     *    Seek to start of 1st packet, as previously
     *    determined in PGS_IO_L0_FileVersionInfo()
     */


    seek_status = fseek(open_fs, version_entry->pkts_start, SEEK_SET);
    if (seek_status != 0)
    {
  	returnStatus = PGSIO_E_L0_FSEEK;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    /* Reset no. packets currently read */
    version_entry->num_pkts_read = 0L;


    /*
     *    Position the file pointer at the start of the desired packet.
     */
    if ( packet_number > 0)          /* search by packet number */
    {
        /*
         * Loop to skip specified number of packets
         */

        for (packet_ct=1; packet_ct < packet_number; packet_ct++)
        {

	    /*
	     *    Read the primary packet header
	     */
	    count = fread( &pri_pkt_header, 1, PGSd_IO_L0_PriPktHdrSize,
			  open_fs);
	    unixerror=errno;

	    if (count != PGSd_IO_L0_PriPktHdrSize) /* incomplete packet read */
	    {
                if (count == 0)
		{
		    returnStatus = PGSIO_E_L0_PACKET_NOT_FOUND;
		    PGS_SMF_SetStaticMsg( returnStatus, toolname );
                    return returnStatus;
                }
                else /* Unix I/O error */
                {
                    returnStatus = PGS_E_UNIX;
                    PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
                    return returnStatus;
                }
	    }

	    /*
	     *    Calculate the size of the variable-length portion of the
	     *    packet, using the 16-bit value in the primary packet header. 
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
             * For TRMM,  the first byte is the most significant byte and the
             *  second is the least significant byte.
             * We assume this is also true for EOS AM & PM.
             */

	    packet_data_length =  (pri_pkt_header.pktLength[1]) +
	      (pri_pkt_header.pktLength[0] * 256) + 1;
	    
	    /*
	     *    Skip past the variable-length portion of the packet, leaving 
	     *    file pointer positioned at the start of the next packet.
	     */
	    fseek(open_fs, (long) packet_data_length, SEEK_CUR);
	    
	    /*
	     *    Increment num packets read in file.
	     */
	    if (use_pkt_size == PGS_TRUE)
	    {
		version_entry->num_pkts_read += PGSd_IO_L0_PriPktHdrSize +
		                                packet_data_length;	
	    }
	    else
	    {
		version_entry->num_pkts_read++;
	    }
        }

	packet_ct ++;

    }
    else                            /* search by packet start time */
    {
        /*
         * Loop to find packet at desired start time
         */
        for (packet_ct=1, packet_found=0; packet_found == 0; packet_ct++)
	
	{    
	    count = fread( &pri_pkt_header, 1, PGSd_IO_L0_PriPktHdrSize,
			   open_fs);
	    unixerror=errno;
	    
	    if (count != PGSd_IO_L0_PriPktHdrSize) /* incomplete packet read */
	    {
		
                if (count == 0)	/* EOF: this should never happen */
                {
                    returnStatus = PGSIO_E_L0_PACKET_NOT_FOUND;
                    PGS_SMF_SetStaticMsg( returnStatus, toolname);
                    return returnStatus;
                }
                else		/* assume Unix I/O error */
                {
                    returnStatus = PGS_E_UNIX;
                    PGS_SMF_SetUNIXMsg( errno,NULL, toolname); 
                    return returnStatus;
                }
		
	    }
	    
	    /*
	     *    Read in the secondary packet header and use it to
	     *    determine the packet timestamp.
	     *
	     *    note: this is a platform-dependent operation
	     */
	    switch (table_entry.spacecraft_tag)
	    {
	    
	      case PGSd_EOS_AM:
		sec_pkt_hdr_size = PGSd_IO_L0_SecPktHdrSizeEOS_AM;
#ifdef _PGS_THREADSAFE
                /*
                 *  Read new header and set in TSD key
                 */
		count = fread( sec_pkt_hdr_EOS_AM, 1, sec_pkt_hdr_size,
			       open_fs);
                pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSAM],
				    (void *) sec_pkt_hdr_EOS_AM);
#else
		count = fread( &sec_pkt_hdr_EOS_AM, 1, sec_pkt_hdr_size,
			       open_fs);
#endif
		unixerror=errno;
		if (count != sec_pkt_hdr_size) /* incomplete read */
		{
                    if (count == 0) /* assume EOF */
		    {
			returnStatus = PGSIO_E_L0_PACKET_NOT_FOUND;
                        PGS_SMF_SetStaticMsg( returnStatus, toolname );
                        return returnStatus;
                    }
                    else /* Unix I/O error */
                    {
                        returnStatus = PGS_E_UNIX;
                        PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
                        return returnStatus;
                    }
		}
		
#ifdef _PGS_THREADSAFE
                /*
                 *  Packet name is now a pointer
                 */
		returnStatus =       /* get the packet timestamp */
		  PGS_TD_EOSAMtoTAI(sec_pkt_hdr_EOS_AM->scTime, &packet_time);
#else
		returnStatus =       /* get the packet timestamp */
		  PGS_TD_EOSAMtoTAI(sec_pkt_hdr_EOS_AM.scTime, &packet_time);
#endif
		
		if (returnStatus != PGS_S_SUCCESS)  
		{
		    if(returnStatus == PGSTD_E_MICSEC_TOO_BIG)
		    {
			returnStatus1 = PGSIO_W_L0_BITFLIP_IN_MICSEC;
			sprintf(specifics,"Packet %d has ", packet_ct);
			PGS_SMF_GetMsgByCode(returnStatus1,errorBuf);
			strcat(specifics,errorBuf);
			PGS_SMF_SetDynamicMsg(returnStatus1,specifics,
					      toolname);
		    }
		    else
		    {
			returnStatus = PGSIO_E_L0_TIME_CONVERSION;
			PGS_SMF_SetStaticMsg( returnStatus, toolname);
			return returnStatus;
		    }
		}
		
		break;
		
	      case PGSd_EOS_PM_GIIS:
		sec_pkt_hdr_size = PGSd_IO_L0_SecPktHdrSizeEOS_PM;
#ifdef _PGS_THREADSAFE
                /*
                 *  Read new header and set in TSD key
                 */
		count = fread( sec_pkt_hdr_EOS_PM_GIIS, 1, sec_pkt_hdr_size,
			       open_fs);
                pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSPMGIIS],
				    (void *) sec_pkt_hdr_EOS_PM_GIIS);
#else
		count = fread( &sec_pkt_hdr_EOS_PM_GIIS, 1, sec_pkt_hdr_size,
			       open_fs);
#endif
		unixerror=errno;
		if (count != sec_pkt_hdr_size) /* incomplete read */
		{
                    if (count == 0) /* assume EOF */
		    {
			returnStatus = PGSIO_E_L0_PACKET_NOT_FOUND;
                        PGS_SMF_SetStaticMsg( returnStatus, toolname );
                        return returnStatus;
                    }
                    else /* Unix I/O error */
                    {
                        returnStatus = PGS_E_UNIX;
                        PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
                        return returnStatus;
                    }
		}

#ifdef _PGS_THREADSAFE
                /*
                 *  Packet name is now a pointer
                 */
		returnStatus =       /* get the packet timestamp */
		  PGS_TD_EOSPMGIIStoTAI(sec_pkt_hdr_EOS_PM_GIIS->scTime, &packet_time);
#else
		returnStatus =       /* get the packet timestamp */
		  PGS_TD_EOSPMGIIStoTAI(sec_pkt_hdr_EOS_PM_GIIS.scTime, &packet_time);
#endif
		
		if (returnStatus != PGS_S_SUCCESS) 
		{
                    returnStatus = PGSIO_E_L0_TIME_CONVERSION;
                    PGS_SMF_SetStaticMsg( returnStatus, toolname);
                    return returnStatus;
		}
		
		break;
		
	      case PGSd_EOS_PM_GIRD:
		sec_pkt_hdr_size = PGSd_IO_L0_SecPktHdrSizeEOS_PM;
#ifdef _PGS_THREADSAFE
                /*
                 *  Read new header and set in TSD key
                 */
		count = fread( sec_pkt_hdr_EOS_PM_GIRD, 1, sec_pkt_hdr_size,
			       open_fs);
                pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSPMGIRD],
				    (void *) sec_pkt_hdr_EOS_PM_GIRD);
#else
		count = fread( &sec_pkt_hdr_EOS_PM_GIRD, 1, sec_pkt_hdr_size,
			       open_fs);
#endif
		unixerror=errno;
		if (count != sec_pkt_hdr_size) /* incomplete read */
		{
                    if (count == 0) /* assume EOF */
		    {
			returnStatus = PGSIO_E_L0_PACKET_NOT_FOUND;
                        PGS_SMF_SetStaticMsg( returnStatus, toolname );
                        return returnStatus;
                    }
                    else /* Unix I/O error */
                    {
                        returnStatus = PGS_E_UNIX;
                        PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
                        return returnStatus;
                    }
		}
		
#ifdef _PGS_THREADSAFE
                /*
                 *  Packet name is now a pointer
                 */
		returnStatus =       /* get the packet timestamp */
		  PGS_TD_EOSPMGIRDtoTAI(sec_pkt_hdr_EOS_PM_GIRD->scTime, &packet_time);
#else
		returnStatus =       /* get the packet timestamp */
		  PGS_TD_EOSPMGIRDtoTAI(sec_pkt_hdr_EOS_PM_GIRD.scTime, &packet_time);
#endif
		
		if (returnStatus != PGS_S_SUCCESS) 
		{
                    returnStatus = PGSIO_E_L0_TIME_CONVERSION;
                    PGS_SMF_SetStaticMsg( returnStatus, toolname);
                    return returnStatus;
		}
		
		break;
	
              case PGSd_EOS_AURA:
                sec_pkt_hdr_size = PGSd_IO_L0_SecPktHdrSizeEOS_AURA;
#ifdef _PGS_THREADSAFE
                /*
                 *  Read new header and set in TSD key
                 */
                count = fread( sec_pkt_hdr_EOS_AURA, 1, sec_pkt_hdr_size,
                               open_fs);
                pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTEOSAURA],
                                    (void *) sec_pkt_hdr_EOS_AURA);
#else
                count = fread( &sec_pkt_hdr_EOS_AURA, 1, sec_pkt_hdr_size,
                               open_fs);
#endif
                unixerror=errno;
                if (count != sec_pkt_hdr_size) /* incomplete read */
                {
                    if (count == 0) /* assume EOF */
                    {
                        returnStatus = PGSIO_E_L0_PACKET_NOT_FOUND;
                        PGS_SMF_SetStaticMsg( returnStatus, toolname );
                        return returnStatus;
                    }
                    else /* Unix I/O error */
                    {
                        returnStatus = PGS_E_UNIX;
                        PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
                        return returnStatus;
                    }
                }
 
#ifdef _PGS_THREADSAFE
                /*
                 *  Packet name is now a pointer
                 */
                returnStatus =       /* get the packet timestamp */
                  PGS_TD_EOSAURAGIRDtoTAI(sec_pkt_hdr_EOS_AURA->scTime, &packet_time);
#else
                returnStatus =       /* get the packet timestamp */
                  PGS_TD_EOSAURAGIRDtoTAI(sec_pkt_hdr_EOS_AURA.scTime, &packet_time);
#endif
 
                if (returnStatus != PGS_S_SUCCESS)
                {
                    returnStatus = PGSIO_E_L0_TIME_CONVERSION;
                    PGS_SMF_SetStaticMsg( returnStatus, toolname);
                    return returnStatus;
                }
 
                break;
 
	      case PGSd_TRMM:
		sec_pkt_hdr_size = PGSd_IO_L0_SecPktHdrSizeTRMM;
#ifdef _PGS_THREADSAFE
                /*
                 *  Read header and set in TSD key
                 */
 		count = fread( sec_pkt_hdr_TRMM, 1, sec_pkt_hdr_size, 
			      open_fs);
                pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTTRMM],
                                     (void *) sec_pkt_hdr_TRMM);
#else
 		count = fread( &sec_pkt_hdr_TRMM, 1, sec_pkt_hdr_size, 
			      open_fs);
#endif
		unixerror=errno;
		if (count != sec_pkt_hdr_size) /* incomplete read */
		{
                    if (count == 0) /* assume EOF */
		    {
			returnStatus = PGSIO_E_L0_PACKET_NOT_FOUND;
                        PGS_SMF_SetStaticMsg( returnStatus, toolname );
                        return returnStatus;
                    }
                    else /* Unix I/O error */
                    {
                        returnStatus = PGS_E_UNIX;
                        PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
                        return returnStatus;
                    }
		}

#ifdef _PGS_THREADSAFE
                /*
                 *  Packet name is a pointer for TSF
                 */
		returnStatus =       /* get the packet timestamp */
		    PGS_TD_TRMMtoTAI(sec_pkt_hdr_TRMM->scTime, &packet_time);
#else
		returnStatus =       /* get the packet timestamp */
		    PGS_TD_TRMMtoTAI(sec_pkt_hdr_TRMM.scTime, &packet_time);
#endif

		if (returnStatus != PGS_S_SUCCESS) 
		{
                    returnStatus = PGSIO_E_L0_TIME_CONVERSION;
                    PGS_SMF_SetStaticMsg( returnStatus, toolname);
                    return returnStatus;
		}
		
		break;

	      case PGSd_ADEOS_II:
		sec_pkt_hdr_size = PGSd_IO_L0_SecPktHdrSizeADEOS_II;
#ifdef _PGS_THREADSAFE
                /*
                 *  Read header info and set in TSD key
                 */
		count = fread( sec_pkt_hdr_ADEOS_II, 1, sec_pkt_hdr_size,
			      open_fs);
                pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYIOL02PKTADEOSII],
                                     (void *) sec_pkt_hdr_ADEOS_II);
#else
		count = fread( &sec_pkt_hdr_ADEOS_II, 1, sec_pkt_hdr_size,
			      open_fs);
#endif
		unixerror=errno;
		if (count != sec_pkt_hdr_size) /* incomplete read */
		{
                    if (count == 0) /* assume EOF */
		    {
			returnStatus = PGSIO_E_L0_PACKET_NOT_FOUND;
                        PGS_SMF_SetStaticMsg( returnStatus, toolname );
                        return returnStatus;
                    }
                    else /* Unix I/O error */
                    {
                        returnStatus = PGS_E_UNIX;
                        PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname);
                        return returnStatus;
                    }
		}

#ifdef _PGS_THREADSAFE
                /*
                 *  Packet name is a pointer for TSF
                 */
		returnStatus =       /* get the packet timestamp */
		    PGS_TD_ADEOSIItoTAI(sec_pkt_hdr_ADEOS_II->scTime,
					&packet_time);
#else
		returnStatus =       /* get the packet timestamp */
		    PGS_TD_ADEOSIItoTAI(sec_pkt_hdr_ADEOS_II.scTime,
					&packet_time);
#endif

		if (returnStatus != PGS_S_SUCCESS) 
		{
                    returnStatus = PGSIO_E_L0_TIME_CONVERSION;
                    PGS_SMF_SetStaticMsg( returnStatus, toolname);
                    return returnStatus;
		}
		
		break;

	      default:
	        returnStatus = PGSIO_E_L0_BAD_SPACECRAFT_TAG;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		return returnStatus;
	    }
	    
	    /*    Calculate the size of the variable-length portion of the 
	     *    packet, using the 16-bit value in the primary packet header.
	     */

	    packet_data_length =  (pri_pkt_header.pktLength[1]) +
	      (pri_pkt_header.pktLength[0] * 256) + 1;

	    /* if there is no BITFLIP problem, check to see it is the desired
	       packet, otherwise go to next packet */
	
	    if(returnStatus1 != PGSIO_W_L0_BITFLIP_IN_MICSEC) /* Packet time OK */
	    {
		
		/*
		 *    Check to see if this is the desired packet
		 */
		if (packet_time < start_time )         /* NO */
		{
		    /*
		     * Skip past the variable-length portion of the packet, 
		     * leaving file pointer positioned at the start of the next
		     * packet.
		     */
		    fseek(open_fs, 
			  (long) (packet_data_length - sec_pkt_hdr_size), 
			  SEEK_CUR);
		    /*
		     *    Increment num packets read in file.
		     */
		    if (use_pkt_size == PGS_TRUE)
		    {
			version_entry->num_pkts_read += PGSd_IO_L0_PriPktHdrSize +
			  packet_data_length;	
		    }
		    else
		    {
			version_entry->num_pkts_read++;
		    }  
		}
		else                                  /* YES */
		{
		    /*
		     *    Skip backwards to the start of this packet, leaving 
		     *    file pointer positioned there for the next call to
		     *    PGS_IO_L0_GetPacket.
		     */
		    fseek(open_fs, 
			  (long) -(PGSd_IO_L0_PriPktHdrSize + sec_pkt_hdr_size),
			  SEEK_CUR);
		    
		    packet_found = 1;    /* flag that packet has been found */
		}
	    }
	    else
	    {
		/*
		 * Skip past the variable-length portion of the packet, 
		 * leaving file pointer positioned at the start of the next
		 * packet.
		 */
		fseek(open_fs, 
		      (long) (packet_data_length - sec_pkt_hdr_size), 
		      SEEK_CUR);
		/*
		 *    Increment num packets read in file.
		 */
		if (use_pkt_size == PGS_TRUE)
		{
		    version_entry->num_pkts_read += PGSd_IO_L0_PriPktHdrSize +
		      packet_data_length;	
		}
		else
		{
		    version_entry->num_pkts_read++;
		}
		returnStatus2 = returnStatus1;
		returnStatus1 = PGS_S_SUCCESS;
		
	    }
	    
        } /* end: for (packet_ct=1, packet_found=0 ... {} */
    
	packet_ct --;
	
      }  /* end: if(packet_number > 0) {} else {} */
     
    *npacket_ct = packet_ct ;

    if(returnStatus2 != PGS_S_SUCCESS)
    {
	return returnStatus2;
    }
    else
    {
	return returnStatus;
    }
    
}


