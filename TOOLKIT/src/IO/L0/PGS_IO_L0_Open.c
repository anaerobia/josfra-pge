/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_Open.c

DESCRIPTION:
  This file contains the I/O tool to open a Virtual Level 0 Data Set.

AUTHOR:
  Mike Sucher / Applied Research Corp.
  Carol S. W. Tsai / Applied Research Corp.
  Abe Taaheri / SM&A Corp.
  Xin Wang / EIT Inc.

HISTORY:
  30-Jan-1995 TWA  Initial version
  03-Mar-1997 CSWT Added an output parameter packet_ct for function 
                   PGS_IO_L0_SeekPacket() 
  01-Dec-1999 AT   Modified to support GIRD and GIIS formats for PM spacecraft
  01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Open a Virtual Level 0 Data Set

NAME:
	PGS_IO_L0_Open
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_Open(
		PGSt_PC_Logical		file_logical,
		PGSt_tag		spacecraft_tag,
		PGSt_IO_L0_VirtualDataSet *virtual_file,
		PGSt_double		*start_time,
		PGSt_double		*stop_time
		)
  FORTRAN:
	include   'PGS_SMF.f'
	include   'PGS_PC.f'
	include   'PGS_PC_9.f'
	include   'PGS_TD.f'
	include   'PGS_IO.f'
	include   'PGS_IO_1.f'

	integer function PGS_IO_L0_Open(
       +                         file_logical,
       +                         spacecraft_tag,
       +                         virtual_file,
       +                         start_time,
       +                         stop_time)
	integer		 file_logical
	integer		 spacecraft_tag
	integer		 virtual_file
	double precision start_time
	double precision stop_time

DESCRIPTION:
	This tool opens the virtual data set pointed to by file_logical.
	A virtual Level 0 data set is defined by the set of physical
	data files that have been staged for this Level 0 process.

	The tool returns a descriptor which is used by all of the Level 0 
	tools to access the specified virtual data set.  The tool also 
	returns the start and stop times of this virtual data set.
	
INPUTS:
	file_logical - 	The logical file descriptor for this virtual data 
		set, as given in the Process Control File.

	spacecraft_tag - The tag identifying which of the supported spacecraft
		platforms generated this virtual data set.  Must be either
		PGSd_TRMM, PGSd_EOS_AM, PGSd_EOS_PM_GIIS, PGSd_EOS_PM_GIRD
                or PGSd_ADEOS_II.

OUTPUTS:
	virtual_file -	The file descriptor used by all other Level 0
		access tools to refer to the virtual data set.

	start_time -	The start time of this virtual data set.

	stop_time -	The stop time of this virtual data set.

            Time format is TAI: continuous seconds since 12AM UTC Jan 1, 1993

NOTES:
	A virtual data set is defined by a set of one or more related Level 0
	physical files. For example, it might consist of all physical
        files corresponding to a single EOS AM science application ID
        (APID) for a single production run.

	The maximum number of virtual data sets that may be open
        at any one time is 20. 
 
	This function must be called first, before any other 
        Toolkit Level 0 access tools are called. 
 
	A virtual data set may consist of several physical files. 
        In this case the files are listed in the process control file
        with the same logical ID (1st field) but different instance numbers
        (last field).

	The physical file version corresponding to the first time-ordered
	set of packets for the virtual data set is opened by this tool.
	The file pointer is left positioned so that the next call to
	PGS_IO_L0_GetPacket will read the first packet in the file. 

	To get file header and footer (TRMM only) information for the newly 
	opened physical file, use tool PGS_IO_L0_GetHeader.

	A rudimentary check is done on the header of the first physical file
	of the virtual data set.  If an error is found in the header this
	function will return the value PGSIO_W_L0_CORRUPT_HEADER.  The file
	will be opened anyway and the user may use the function
	PGS_IO_L0_GetHeader() to retrieve the header.  That function will give
	a more detailed analysis of the problem.  Users should be aware, though,
	that if they proceed after getting the return PGSIO_W_L0_CORRUPT_HEADER
	from this function they do so at THEIR OWN RISK.  This return value
	indicates that the file header is corrupt and the use of any further
	Toolkit functions to attempt to read the file may produce unexpected
	results.

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

	NOTE REGARDING USE OF THE PROCESS CONTROL FILE:
	If more than one physical file is associated with a given virtual
	data set, the entries in the Process Control File which map the data
	set from file_logical to the physical files must appear in reverse
	numerical order. For example, in a three-file data set, file instance #3
	is listed first and file instance #1 is listed last.
        This mechanism will become transparent in the production system.

EXAMPLES:
	Prepare in part for LIS Level 0 processing by opening the LIS/TRMM
	Level 0 virtual data set for science APID 61.
        For TRMM, there is expected to be only one physical file
        per APID per day. In this case each virtual
        data set (APID) corresponds to exactly one physical file.

        At the SCF, you must prepare entries of the following form
        in the Process Control File:
        
?   PRODUCT INPUT FILES
# [ set env var PGS_PRODUCT_INPUT for default location ]
#
61|TRMM_G0091_1997-11-01T00:00:00Z_dataset_V01_01|
            |||TRMM_G0091_1997-11-01T00:00:00Z_sfdu_V01_01|1

        (Here the logical ID used is arbitrarily set to the APID.)

        Note: In the above Process Control File entry, the file name in
        the next-to-last field is the TRMM SFDU header file, which is
        a file that contains data associated with the given L0 file.
        Use functions PGS_IO_PC_GetFileAttr or PGS_IO_PC_GetFileByAttr
        to retrieve data from this file. Also the PCF entry must appear
        on a single line, and not be broken into several lines as shown here.
        
        
  C:
	#define SCIENCE_FILE 61

	PGSt_IO_L0_VirtualDataSet  virtual_file;
	PGSt_PC_Logical            file_logical;
	PGSt_tag                   spacecraft_tag;
	PGSt_double                start_time;
	PGSt_double                stop_time;
	PGSt_SMF_status            returnStatus;

	file_logical = SCIENCE_FILE;
	spacecraft_tag = PGSd_TRMM;

	returnStatus = PGS_IO_L0_Open(
		file_logical,
		spacecraft_tag,
		&virtual_file,
		&start_time,
		&stop_time);

/#     Virtual file handle virtual_file may now be used as input to
       other L0 access tools #/


  FORTRAN:

        implicit   none

	include   'PGS_SMF.F'
	include   'PGS_PC.F'
	include   'PGS_PC_9.F'
	include   'PGS_TD.F'
	include   'PGS_IO.F'
	include   'PGS_IO_1.F'

	integer   science_file
        parameter (science_file=61)

 	integer            pgs_io_l0_open
 	integer            pgs_io_l0_setstart
	integer            file_logical
	integer            spacecraft_tag
	integer            virtual_file
	double precision   start_time
	double precision   stop_time
	integer            returnstatus

	file_logical = science_file
	spacecraft_tag = pgsd_trmm

	returnstatus = pgs_io_l0_open(
     .   	file_logical,
     .   	spacecraft_tag,
     .   	virtual_file,
     .   	start_time,
     .   	stop_time)

!     Virtual file handle virtual_file may now be used as input to
!     other L0 access tools

	

RETURNS:
	PGS_S_SUCCESS
	PGSIO_W_L0_CORRUPT_FILE_HDR
	PGSIO_E_L0_BAD_SPACECRAFT_TAG
	PGSIO_E_L0_INIT_FILE_TABLE
	PGSIO_E_L0_INVALID_FILE_LOGICAL
	PGSIO_E_L0_MAP_VERSIONS
	PGSIO_E_L0_PHYSICAL_OPEN
	PGSIO_E_L0_MANAGE_TABLE
	PGSIO_E_L0_SEEK_1ST_PACKET

REQUIREMENTS:
	PGSTK-0190, 0140, 0240

DETAILS:
	The tool sets up an internal table to handle time sequence to file 
	version mapping and tracking of current position in the virtual 
	data set.  It also opens the first time-sequenced physical file
	and positions the file pointer so that the next call to the tool
	PGS_IO_L0_GetPacket will return the first packet in the virtual 
	data set.

GLOBALS:
        None

FILES:
	Process Control File (PCF) ($PGS_PC_INFO_FILE)
	Physical file(s) for the specified virtual data set, as defined in
           the PCF

FUNCTIONS CALLED:
	PGS_IO_L0_MapVersions
	PGS_IO_Gen_Open
	PGS_SMF_SetStaticMsg
	PGS_IO_L0_ManageTable
	PGS_IO_L0_SeekPacket

END_PROLOG
*****************************************************************************/

PGSt_SMF_status 
PGS_IO_L0_Open(
	PGSt_PC_Logical		file_logical,
	PGSt_tag 		spacecraft_tag,
	PGSt_IO_L0_VirtualDataSet *virtual_file,
	PGSt_double		*start_time,
	PGSt_double		*stop_time
	)
{
    static char *toolname = "PGS_IO_L0_Open()";
    
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */
    PGSt_SMF_status  returnStatus1 = PGS_S_SUCCESS;   /* return status */

    PGSt_IO_L0_FileTable    table_entry;

    PGSt_IO_Gen_AccessType  file_access;      /* mode to open file in e.g. READ */
    PGSt_IO_Gen_FileHandle  *file_handle;     /* file handle to be returned */
    PGSt_integer	    file_version;     /* file version required */
    PGSt_integer            version_count;    /* number of versions found */
    PGSt_integer            packet_ct;        /* number of packet */
    PGSt_IO_L0_VersionTable *version_array;   /* pointer to version mapping array */

    PGSt_IO_L0_VirtualDataSet local_v_file;   /* L0 virtual data set descriptor */

    int                       hdr_file;
    
    char                msg[PGS_SMF_MAX_MSG_SIZE];      /* predefined status message */
    char                newMsg[PGS_SMF_MAX_MSGBUF_SIZE];/* new status message */
 
    PGSt_SMF_status     caller;         /* determines who's calling who */
 
    /*------------------------------------------------------
        Get Toolkit CallerId
        if SMF is caller do NOT make SMF calls!
    ------------------------------------------------------*/
    caller = PGS_SMF_CallerID();
 
    /*
     *    Make sure the user has specified a valid spacecraft tag type.
     */
    switch (spacecraft_tag)
    {
      case PGSd_EOS_AM:
	hdr_file = 1;
	break;

      case PGSd_EOS_PM_GIIS:
	hdr_file = 1;
	break;

      case PGSd_EOS_PM_GIRD:
	hdr_file = 1;
	break;
      case PGSd_EOS_AURA:
        hdr_file = 1;
        break;
 
      case PGSd_TRMM:
      case PGSd_ADEOS_II:
	hdr_file = 0;
	break;
      default:
        returnStatus = PGSIO_E_L0_BAD_SPACECRAFT_TAG;
        PGS_SMF_SetStaticMsg( returnStatus, toolname);
        return returnStatus;
    }
	    

    /*
     *    Get the version mapping array.  Each element of this array
     *    contains the physical file version, start time, stop time, 
     *    and various other fields that maps physical file versions
     *    to time sequenced files.  See the structure definition of
     *    PGSt_IO_L0_VersionTable in PGS_IO_L0.h for full details.
     */

    returnStatus1 = PGS_IO_L0_MapVersions(
	file_logical, spacecraft_tag, &version_count, &version_array);
    if (returnStatus1 != PGS_S_SUCCESS) 
    {
        switch( returnStatus1 )
        {
	  case PGSIO_W_L0_HDR_TIME_ORDER:
	  case PGSIO_E_L0_BAD_VAR_HDR_SIZE:
	  case PGSIO_W_L0_BAD_PKT_DATA_SIZE:
	  case PGSIO_W_L0_BAD_PACKET_COUNT:
	  case PGSIO_W_L0_BAD_FOOTER_SIZE:
	  case PGSIO_W_L0_ZERO_PACKET_COUNT:
	    break;
	  case PGSIO_E_L0_VERSION_COUNT:   /* can't find data set */
	    returnStatus = PGSIO_E_L0_INVALID_FILE_LOGICAL;
              if (caller != PGSd_CALLERID_SMF)
                {
                   sprintf(newMsg,"LID %d is invalid or does not exist in PCF. ",(int)file_logical);
                   PGS_SMF_GetMsgByCode(returnStatus,msg);
                   strcat(newMsg,msg);
                   PGS_SMF_SetDynamicMsg(returnStatus,newMsg, toolname);
                }
	    return returnStatus;
	  case PGSIO_E_L0_VERSION_INFO:    /* can't get header info */
	    returnStatus = PGSIO_E_L0_INIT_FILE_TABLE;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    return returnStatus;
	  default:                         /* other error */
	    returnStatus = PGSIO_E_L0_MAP_VERSIONS;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    return returnStatus;
        }
    }


    /*
     *    Get the start and stop times for this virtual data set
     *    from the version mapping array.
     */

    *start_time = 			/* get start time of first file */
        version_array[hdr_file].start_time; 

    *stop_time  = 			/* get stop time of last file */
        version_array[version_count - 1].stop_time;


    /*
     *    Set parameters for the call to PGS_IO_Gen_Open.
     */
    file_access = PGSd_IO_Gen_Read;                      /* open for reading,
							    must exist  */
    file_version = version_array[hdr_file].file_version; /* get earliest
							    time-ordered file */

    /*
     *    Open the first physical file.
     */
    returnStatus = PGS_IO_Gen_Open(  	 	    	 
        file_logical, file_access, &file_handle, file_version);
    if (returnStatus != PGS_S_SUCCESS) 
    {
 	returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    /*
     *    Set up values for file table entry.
     */
    table_entry.file_logical = file_logical;
    table_entry.version_count = version_count;
    table_entry.spacecraft_tag = spacecraft_tag;
    table_entry.open_file_seq_no = hdr_file;
    table_entry.open_file_stream = file_handle;
    table_entry.version_array = version_array;

    /*
     *    Add entry to the file table.
     *    Variable local_v_file gets back the descriptor that is
     *    used to access this virtual data set in all Level 0 tools.
     */
    returnStatus = PGS_IO_L0_ManageTable(
	 PGSd_IO_L0_AddTableEntry, &local_v_file, &table_entry, 0);
    if (returnStatus != PGS_S_SUCCESS) 
    {
  	returnStatus = PGSIO_E_L0_MANAGE_TABLE;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    /*
     *    Pass the virtual data set descriptor back to the caller
     */
    *virtual_file = local_v_file;

    /*
     *    Set the file pointer so the next call to PGS_IO_L0_GetPacket
     *    will return the first packet in the file.
     */
    returnStatus = PGS_IO_L0_SeekPacket(local_v_file, 1, &packet_ct,  0.0);
    if (returnStatus != PGS_S_SUCCESS) 
    {
  	returnStatus = PGSIO_E_L0_SEEK_1ST_PACKET;
    }
    
    if (returnStatus1 != PGS_S_SUCCESS)
    {
	
	if (returnStatus == PGS_S_SUCCESS)
	{
	    PGS_SMF_GetMsgByCode(returnStatus1, msg);
	    returnStatus = PGSIO_W_L0_CORRUPT_FILE_HDR;
	    PGS_SMF_SetDynamicMsg(returnStatus, msg, toolname);
	}
    }
    else
    {
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
    }
    
    return returnStatus;
}
