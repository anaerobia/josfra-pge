############################################################################## 
# BEGIN_FILE_PROLOG:
# 
# FILENAME:
#   PGS_IO_1.t 	Return code definitions for PGS IO tools (SMF seed value 1)
#   
# 
# DESCRIPTION:
# 
#   This file contains PGS_SMF standard return code definitions for the
#   PGS_IO group of tools.
#
#   The file is intended to be used as input by the smfcompile utility,
#   which generates the message file, and the C and FORTRAN header files.
#
# AUTHOR:
#   Mike Sucher / Applied Research Corp.
#   
# HISTORY:
#  18-Mar-1994 MES Initial version
#  30-Mar-1994 MES Standardize prologs for all IO tools
#  05-Apr-1994 DPH Added several messages to support Gen_Open & Gen_Temp_Open
#  02-Aug-1994 MES Added several messages to support Gen_OpenF & Gen_Temp_OpenF
#  03-Aug-1994 MES Added several messages to support Gen_Track_LUN
#  11-Aug-1994 MES Added message to support Gen_OpenF & Gen_Temp_OpenF
#  07-Sep-1994 MES Removed messages that supported obsolete Release 2 tools
#  21-Sep-1994 DPH Added close message which was mistakenly removed
#  22-Sep-1994 DPH Added warning message PGSIO_W_GEN_FILE_NOT_FOUND
#  14-Oct-1994 DPH Added error message PGSIO_E_GEN_BAD_ENVIRONMENT
#  19-Dec-1994 MES Added error messages to support the Level 0 tools
#  12-Jan-1995 TWA Fixed wording of PGSIO_E_GEN_OPEN_RECL
#  30-Jan-1995 TWA L0 support
#  10-May-1995 DPH Modified text of PGSIO_E_GEN_BAD_ENVIRONMENT for TK5
#  04-Aug-2000 AT  Added warning message PGSIO_W_L0_BITFLIP_IN_MICSEC
#
# END_FILE_PROLOG:
##############################################################################

%INSTR	= PGSTK

%LABEL	= PGSIO

%SEED 	= 1


#
# messages for all IO tools
#

PGSIO_E_FILEMAP		Attempt to map logical to physical file failed.

#
# messages for Generic IO tools
#

# messages for open
PGSIO_E_GEN_OPEN	Generic IO - Attempt to open file failed.
PGSIO_E_GEN_OPENMODE	Generic IO - Invalid mode for file open. 
PGSIO_E_GEN_FILE_NOEXIST	File does not exist, or cannot be created.	

# additional messages for temp open
PGSIO_E_GEN_NO_TEMP_NAME     	New name could not be generated for this temporary file
PGSIO_E_GEN_REFERENCE_FAILURE	File reference could not be achieved
PGSIO_E_GEN_CREATE_FAILURE	File creation could not be achieved
PGSIO_E_GEN_BAD_FILE_DURATION   An invalid file duration setting was requested
PGSIO_W_GEN_ACCESS_MODIFIED 	File exists! resetting access mode to PGSd_IO_Gen_Append Update
PGSIO_W_GEN_DURATION_NOMOD	Intermediate file duration may not be modified in this access mode 
PGSIO_W_GEN_NEW_FILE		New file was created eventhough access mode was not explicit write
PGSIO_E_PARTIAL_NAME		Generated temporary file name missing key component

# additional messages for Track_LUN (FORTRAN only)

PGSIO_E_GEN_NO_FREE_LUN         No free logical unit number available to allocate
PGSIO_E_GEN_ILLEGAL_LUN         Illegal value for logical unit number: can't deallocate
PGSIO_W_GEN_UNUSED_LUN          The LUN marked for deallocation was not allocated

# additional messages for OpenF / Temp_OpenF (FORTRAN versions)

PGSIO_W_GEN_OLD_FILE            File exists! Changing access mode from Write to Update
PGSIO_E_GEN_OPEN_OLD            Attempt to do FORTRAN Open with STATUS=OLD failed!
PGSIO_E_GEN_OPEN_NEW            Attempt to do FORTRAN Open with STATUS=NEW failed!
PGSIO_E_GEN_OPEN_UNKNOWN        Attempt to do FORTRAN Open with STATUS=UNKNOWN failed!
PGSIO_E_GEN_OPEN_RECL           Illegal value for record length

# additional messages for temp delete
PGSIO_E_GEN_FILE_NODEL		File does not exist, or cannot be deleted
PGSIO_W_GEN_FILE_NOT_FOUND	File could not be located on disk

#messages for close
PGSIO_E_GEN_CLOSE       	Generic IO - Attempt to close file failed.

# additional messages for open
PGSIO_E_GEN_BAD_ENVIRONMENT	Bad default setting detected for I/O path



#
# messages for Level 0 IO tools
#

#
# messages returned by more than one user-level L0 tool
#

PGSIO_E_L0_BAD_BUF_SIZ               Buffer size must be a positive integer
PGSIO_E_L0_BAD_SPACECRAFT_TAG        Illegal spacecraft tag
PGSIO_E_L0_FSEEK                     Failed to locate requested byte in file
PGSIO_E_L0_MANAGE_TABLE              Error accessing internal virtual file table
PGSIO_E_L0_PHYSICAL_OPEN             Unable to open physical file
PGSIO_E_L0_UNEXPECTED_EOF            Encountered unexpected end-of-file
PGSIO_E_L0_VIRTUAL_DS_NOT_OPEN       Virtual data set is not open
PGSIO_M_L0_HEADER_CHANGED            New physical file open - 
                                        file header has changed
PGSIO_W_L0_PKT_BUF_TRUNCATE          Insufficient packet buffer size - data truncated
PGSIO_W_L0_PHYSICAL_CLOSE            Failed to close physical file

#
# messages returned only by PGS_IO_L0_Open
#

PGSIO_E_L0_INIT_FILE_TABLE           Error during read of physical file header 
                                        for initialization
PGSIO_E_L0_INVALID_FILE_LOGICAL      Failed to process this file logical in  
                                        process control file
PGSIO_E_L0_MAP_VERSIONS              Failed to initialize internal physical
                                        file table
PGSIO_E_L0_SEEK_1ST_PACKET           Can't find 1st packet in dataset

#
# messages returned only by PGS_IO_L0_Seek_Start
#
PGSIO_W_L0_TIME_NOT_FOUND            Requested start time not found; file
                                        pointer position was unchanged 
PGSIO_E_L0_SEEK_PACKET               Unable to find requested packet

#
# messages returned only by PGS_IO_L0_GetHeader
#
# (none)

#
# messages returned only by PGS_IO_L0_GetPacket
#
PGSIO_E_L0_PHYSICAL_NOT_OPEN         No physical file currently open for this
                                        virtual data set
PGSIO_E_L0_PKT_BUF_OVERFLOW          Packet buffer too small; no data was read
PGSIO_W_L0_END_OF_VIRTUAL_DS         Reached end of the current data set
PGSIO_E_L0_NEXT_PHYSICAL             Error opening next physical file
                                        in sequence
#
# messages returned only by PGS_IO_L0_Close
#
# (none)

# messages returned by lower-level tools

PGSIO_E_L0_ILLEGAL_COMMAND           Specified command code is illegal
PGSIO_E_L0_ILLEGAL_PACKET_NUM        Illegal packet number 
PGSIO_E_L0_INDEX_OUT_OF_RANGE        File table lookup index is out of range
PGSIO_E_L0_MAX_OPEN_EXCEEDED         Max no. of open Level 0 data sets exceeded
PGSIO_E_L0_MEM_ALLOC                 Error occurred during memory allocation
PGSIO_E_L0_UNUSED_TABLE_ENTRY        File table lookup index points to 
                                        unused table entry
PGSIO_E_L0_VERSION_COUNT             Unable to read number of file versions from PCF
PGSIO_E_L0_VERSION_INFO              Error occurred attempting to get physical 
                                        file header information
PGSIO_E_L0_PACKET_NOT_FOUND          Packet not found in current physical file
PGSIO_E_L0_TIME_CONVERSION           Error occurred during time conversion
PGSIO_E_L0_BAD_FILEINFO              Error occurred determining file information


#
# messages returned by PGS_IO_L0_FileSim
#
PGSIO_E_L0_BAD_NUM_PKTS              Illegal number of packets
PGSIO_E_L0_BAD_APP_ID                At least 1 packet had a bad Application ID
PGSIO_E_L0_BAD_FIRST_PKTNUM          Illegal first packet number
PGSIO_E_L0_BAD_DATA_LENGTH           At least 1 packet had a bad data length
PGSIO_E_L0_BAD_NUM_APP_IDS           Illegal number of differing Application IDs

#
# Updates for TK5
#
PGSIO_W_L0_HDR_BUF_TRUNCATE          Insufficient header buffer size - data truncated
PGSIO_W_L0_FTR_BUF_TRUNCATE          Insufficient footer buffer size - data truncated
PGSIO_W_L0_ALL_BUF_TRUNCATE          Insufficient header buffer AND footer buffer sizes - data truncated
PGSIO_W_L0_BUFTRUNC_END_DS           Insufficient packet buffer size - data truncated, reached end of the current data set
PGSIO_W_L0_BUFTRUNC_HDR_CHG          Insufficient packet buffer size - data truncated, new physical file open - file header has changed
PGSIO_E_L0_BUFTRUNC_NXTFILE          Insufficient packet buffer size - data truncated, error opening next physical file in sequence

PGSIO_W_L0_HDR_TIME_ORDER            Time of last packet is earlier than time of first packet in file header
PGSIO_E_L0_BAD_VAR_HDR_SIZE          The size of the variable header as indicated in the fixed header is invalid
PGSIO_W_L0_BAD_PKT_DATA_SIZE         The total size of the packet data as indicated in the file header is invalid
PGSIO_W_L0_BAD_PACKET_COUNT          The total number of packets as indicated in the file header is invalid
PGSIO_W_L0_BAD_FOOTER_SIZE           The size of the file footer as indicated in the file header is invalid
PGSIO_W_L0_ZERO_PACKET_COUNT         The total number of packets as indicated in the file header is zero
PGSIO_W_L0_CORRUPT_FILE_HDR          An error was found in the file header
PGSIO_W_L0_BITFLIP_IN_MICSEC         Bit flip problem in the micro second field of packet time
