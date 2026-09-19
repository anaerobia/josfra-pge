/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_FileVersionInfo.c

DESCRIPTION:
  
  This file contains the function PGS_IO_L0_VersionInfoCheck().
  This function is the Level 0 I/O tool to check file version information 
  determined in the function PGS_IO_L0_FileVersionInfo().

AUTHOR:
  Guru Tej S. Khalsa / Applied Research Corp.
  Abe Taaheri / SM&A Corp.
  Xin Wang / EIT Inc.

HISTORY:
  20-Jul-1995 GTSK Initial version
  01-Dec-1999 AT   Modified to support GIRD and GIIS formats for PM spacecraft
  01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:
   Check File Version Information
	
NAME:
   PGS_IO_L0_VersionInfoCheck()
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_FileVersionInfo(
		PGSt_IO_L0_VersionTable	*version_array,
		PGSt_tag		spacecraft_tag
		)
  FORTRAN:
	N/A

DESCRIPTION:
  This function is the Level 0 I/O tool to check file version information 
  determined in the function PGS_IO_L0_FileVersionInfo().

INPUTS:
   version_arrayy - The version table entry where the version number, packet 
   	            count, and start and stop times for the specified physical
   	            file are stored.
   
   spacecraft_tag - The tag identifying which of the supported spacecraft
   	            platforms generated this virtual data set.  The following
		    types are currently defined:
   

			PGSd_EOS_AM  	    the EOS AM platform
			PGSd_EOS_PM_GIIS    the EOS PM platform (GIIS)
			PGSd_EOS_PM_GIRD    the EOS PM platform (GIRD)
			PGSd_TRMM    	    the TRMM platform
			PGSd_ADEOS_II  	    the ADEOS-II platform

OUTPUTS:
   None

NOTES:
   This is a low-level tool.  It is not intended to be called directly by 
   any user code.

EXAMPLES:
   N/A

RETURNS:
   PGS_S_SUCCESS
   PGSIO_W_L0_HDR_TIME_ORDER
   PGSIO_E_L0_BAD_VAR_HDR_SIZE
   PGSIO_W_L0_BAD_PKT_DATA_SIZE
   PGSIO_W_L0_BAD_PACKET_COUNT
   PGSIO_W_L0_BAD_FOOTER_SIZE
   PGSIO_W_L0_ZERO_PACKET_COUNT
	
REQUIREMENTS:
   PGSTK-0200, 0240

DETAILS:
   This tool will the version_array as determined in the function
   PGS_IO_L0_MapVersions().  It does a rudimentary check to make sure that the
   sum of the sizes of all known sections of the file (as indicated in the
   file's static header section) does not exceed the physical file size.
   In addition this function will check whether the file start time occurs
   before the file stop time.  It will also check if number of packets is
   indicated is zero.

GLOBALS:
   None

FILES:
   None

FUNCTIONS CALLED:
   None

END_PROLOG
*******************************************************************************/

#include <PGS_IO.h>
#include <PGS_TD.h>

/* name of this function */

#define FUNCTION_NAME "PGS_IO_L0_VersionInfoCheck()"

PGSt_SMF_status
PGS_IO_L0_VersionInfoCheck(
    PGSt_IO_L0_VersionTable  *version_array,
    PGSt_tag                 spacecraft_tag)
{
    size_t                   pkt_hdr_size;

    long                     cum_file_size;
    
    PGSt_SMF_status          returnStatus=PGS_S_SUCCESS;
    
    if (version_array->start_time > version_array->stop_time)
    {
	returnStatus = PGSIO_W_L0_HDR_TIME_ORDER;
    }
    
    switch (spacecraft_tag)
    {
      case PGSd_TRMM:
	cum_file_size = version_array->var_hdr_size + 
	                version_array->stat_hdr_size;
	if (cum_file_size > version_array->file_size)
	{
	    returnStatus = PGSIO_E_L0_BAD_VAR_HDR_SIZE;
	    break;
	}
	
	cum_file_size += version_array->pkts_size;
	
	if (cum_file_size > version_array->file_size)
	{
	    returnStatus = PGSIO_W_L0_BAD_PKT_DATA_SIZE;
	    break;
	}

	cum_file_size += version_array->footer_size;
	
	if (cum_file_size != version_array->file_size)
	{
	    returnStatus = PGSIO_W_L0_BAD_FOOTER_SIZE;
	    break;
	}

	pkt_hdr_size = sizeof(PGSt_IO_L0_PriPktHdr) + 
	               sizeof(PGSt_IO_L0_SecPktHdrTRMM);

	if (version_array->pkts_size < version_array->packet_count*pkt_hdr_size)
	{
	    returnStatus = PGSIO_W_L0_BAD_PACKET_COUNT;
	    break;
	}
	
	if (version_array->packet_count == 0UL)
	    returnStatus = PGSIO_W_L0_ZERO_PACKET_COUNT;
	
	break;

	
	
      case PGSd_EOS_AM:
	cum_file_size = version_array->stat_hdr_size + 
	                version_array->pkts_size;

	if (cum_file_size != version_array->file_size)
	{
	    returnStatus = PGSIO_W_L0_BAD_PKT_DATA_SIZE;
	    break;
	}

	if (version_array->packet_count == 0UL)
	    returnStatus = PGSIO_W_L0_ZERO_PACKET_COUNT;

	break;

      case PGSd_EOS_PM_GIIS:
	cum_file_size = version_array->stat_hdr_size + 
	                version_array->pkts_size;

	if (cum_file_size != version_array->file_size)
	{
	    returnStatus = PGSIO_W_L0_BAD_PKT_DATA_SIZE;
	    break;
	}

	if (version_array->packet_count == 0UL)
	    returnStatus = PGSIO_W_L0_ZERO_PACKET_COUNT;

	break;

      case PGSd_EOS_PM_GIRD:
	cum_file_size = version_array->stat_hdr_size + 
	                version_array->pkts_size;

	if (cum_file_size != version_array->file_size)
	{
	    returnStatus = PGSIO_W_L0_BAD_PKT_DATA_SIZE;
	    break;
	}

	if (version_array->packet_count == 0UL)
	    returnStatus = PGSIO_W_L0_ZERO_PACKET_COUNT;

	break;

      case PGSd_EOS_AURA:
        cum_file_size = version_array->stat_hdr_size +
                        version_array->pkts_size;
 
        if (cum_file_size != version_array->file_size)
        {
            returnStatus = PGSIO_W_L0_BAD_PKT_DATA_SIZE;
            break;
        }
 
        if (version_array->packet_count == 0UL)
            returnStatus = PGSIO_W_L0_ZERO_PACKET_COUNT;
 
        break;
 
      case PGSd_ADEOS_II:
	cum_file_size = version_array->stat_hdr_size + 
	                version_array->pkts_size;

	if (cum_file_size != version_array->file_size)
	{
	    returnStatus = PGSIO_W_L0_BAD_PKT_DATA_SIZE;
	    break;
	}

	pkt_hdr_size = sizeof(PGSt_IO_L0_PriPktHdr) + 
	               sizeof(PGSt_IO_L0_SecPktHdrADEOS_II);
	
	if (version_array->pkts_size < version_array->packet_count*pkt_hdr_size)
	{
	    returnStatus = PGSIO_W_L0_BAD_PACKET_COUNT;
	    break;
	}
	
	if (version_array->packet_count == 0UL)
	    returnStatus = PGSIO_W_L0_ZERO_PACKET_COUNT;

	break;
	
      default:
	returnStatus = PGSIO_E_L0_BAD_SPACECRAFT_TAG;
    }
    
    return returnStatus;
}
