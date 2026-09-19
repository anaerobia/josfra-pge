/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
   PGS_IO_L0_GetEOSPMGIISfileTimes.c

DESCRIPTION:
   This file contains the function PGS_IO_L0_GetEOSPMGIISfileTimes().  This function
   parses an EDOS construction record and extracts the start and stop times of
   the L0 data packet files contained in the PDS/EDS represented by the
   construction record.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.

HISTORY:
   02-Apr-1996 GTSK  Initial version

END_FILE_PROLOG
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:
	Get EDOS L0 Packet File Start and Stop Times

NAME:
	PGS_IO_L0_GetEOSPMGIISfileTimes

SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status
	PGS_IO_L0_GetEOSPMGIISfileTimes(
            unsigned char*       construction_record,
	    PGSt_double          start_stop[][2],
	    PGSt_integer*        num_files,
	    unsigned long*       size)

  FORTRAN:
	N/A

DESCRIPTION:
   This function parses an EDOS construction record and extracts the start and
   stop times of the L0 data packet files contained in the PDS/EDS represented
   by the construction record.

INPUTS:
   construction_record - buffer containing EDS/PDS contruction record

OUTPUTS:
   start_stop - start and stop times of PDS/EDS L0 data files

   num_files  - number of files in the PDS/EDS

   size       - size in bytes of the construction record

NOTES:
   This is a low-level tool.  It is not intended to be called directly by 
   any user code.

EXAMPLES:
   None	

RETURNS:
   PGS_S_SUCCESS               successful execution
   PGSTD_E_BAD_2ND_HDR_FLAG    bad value of secondary header ID flag
   PGSTD_E_MILSEC_TOO_BIG      millisecond field too large (>=86401000)
   PGSTD_E_MICSEC_TOO_BIG      microsecond field too large (>=1000)
   PGSTD_E_NO_LEAP_SECS        leap seconds correction unavailable at 
                               requested time
   PGS_E_TOOLKIT               an unexpected error occurred

REQUIREMENTS:
   PGSTK-0200, 0240

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS CALLED:
   PGS_IO_L0_BYTEtoINT()
   PGS_TD_EOSPMGIIStoTAI()
   PGS_SMF_TestStatusLevel()


END_PROLOG
*******************************************************************************/

#include <PGS_IO.h>

PGSt_SMF_status
PGS_IO_L0_GetEOSPMGIISfileTimes(
    unsigned char*       construction_record, /* PDS/EDS construction record */
    PGSt_double          start_stop[][2],     /* packet data files start/stop
						 times */
    PGSt_integer*        num_files,           /* number of files in PDS/EDS */
    unsigned long*       size)                /* size of construction record */
{
    unsigned long        offset;        /* offset into construction_record
					   buffer */
    unsigned int         numAPIDs;      /* number of APIDs in PDS/EDS */
    
    unsigned int         appID;         /* value of APID */
    unsigned int         file;          /* loop counter */
    
    PGSt_double          startTAI93;    /* start time of a L0 data file */
    PGSt_double          stopTAI93;     /* stop time of a L0 data file */
    
    PGSt_SMF_status      returnStatus;  /* return status of TK function calls,
					   as well as return status of this
					   function */
    
    /* move to item 8 (items 1 - 7 are fixed length) */

    offset = 50;

    /* move to item 11 (item 8 has variable length: 8 two bytes fixed which
       indicate the number of times that items 8-1, 8-2, 8-3, and 8-4 (totalling
       16 bytes) repeat, items 9 and 10 are fixed length (total 12 bytes)...so
       to get to item 11 move a total of 14 fixed bytes and 16 bytes times the
       number indicated in item 8) */

    offset += 16*(PGS_IO_L0_BYTEtoINT(construction_record+offset, 2));
    offset += 14;
    
    /* item 11 (8 bytes) is the time of the first packet in the PDS/EDS, convert
       it to secTAI93 format */

    returnStatus = PGS_TD_EOSPMGIIStoTAI(construction_record+offset,
				     &start_stop[0][0]);
    if ( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
    {
	return returnStatus;
    }
    
    /* move to item 12 (item 11 is 8 bytes) */

    offset += 8;

    /* item 12 (8 bytes) is the time of the last packet in the PDS/EDS, convert
       it to secTAI93 format */

    returnStatus = PGS_TD_EOSPMGIIStoTAI(construction_record+offset,
				     &start_stop[0][1]);
    if ( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
    {
	return returnStatus;
    }

    /* move to item 24 (items 12 to 23 are fixed length) */

    offset += 59;

    /* item 24 is the number of APIDs in the PDS/EDS */

    numAPIDs = PGS_IO_L0_BYTEtoINT(construction_record+offset, 1);

    /* move to item 24-1 */

    offset += 1;

    /* items 24-1 to 24-19 are repeated once for each APID in the PDS/EDS */

    for (appID=0;appID<numAPIDs;appID++)
    {
	/* move to item 24-5 (items 24-1 to 24-4 are fixed length) */

	offset += 15;

	/* move to item 24-6 (item 24-5 is fixed length (1 byte) followed by the
	   variable length items 24-5.1 and 24-5.2 which total 4 bytes and are
	   repeated the number of times specified in item 24-5) */

	offset += 4*(PGS_IO_L0_BYTEtoINT(construction_record+offset, 1));
	offset += 1;

	/* move to item 24-7 (item 24-6 is fixed length (4 bytes) followed by
	   the variable length items 24-6.1 to 24-6.9 which total 48 bytes and
	   are repeated the number of times specified in item 24-6) */

	offset += 48*(PGS_IO_L0_BYTEtoINT(construction_record+offset, 4));
	offset += 4;

	/* move to item 24-9 (item 24-7 is fixed length (4 bytes) followed by
	   the variable length items 24-7.1 to 24-7.3 which total 16 bytes and
	   are repeated the number of times specified in item 24-7, item 24-8 is
	   fixed length (8 bytes)) */

	offset += 16*(PGS_IO_L0_BYTEtoINT(construction_record+offset, 4));
	offset += 12;

	/* move to item 24-20 (item 24-9 is fixed length (4 bytes) followed by
	   the variable length item 24-9.1 totals 4 bytes and ais repeated the
	   number of times specified in item 24-9, items 24-10 to 24-19 are
	   fixed length (total 56 bytes)) */

	offset += 4*(PGS_IO_L0_BYTEtoINT(construction_record+offset, 4));
	offset += 60;

	/* NOTE: the offset pointer is now at the end of item 24-19 (the item
	   24-20 mentioned above follows 24-19 in the documentation but is not a
	   physical quantity in the file, it is a directive to repeat items 24-1
	   to 24-19 if necessary (these items occur once for each APID)). */
    }
    
    /* move to item 25-1 (item 25 is fixed length (3 bytes)) */

    offset += 3;
    
    /* item 25-1 is the number of files in the PDS/EDS (this number includes the
       Construction Record, so it should always be at LEAST 2) */

    *num_files = (int) PGS_IO_L0_BYTEtoINT(construction_record+offset, 1);

    /* move to item 25-2 (item 25-1 is fixed length (1 byte)) */

    offset += 1;
    
    /* items 25-2 to 25-4.6 repeat the number of times indicated by item 25-1,
       the first set of values of these items correspond to the Construction
       Record--skip these and move to the next set which correspond to the first
       file containing L0 packet data */

    offset += 68;

    for (file=1;file<*num_files;file++)
    {
	start_stop[file][0] = 1.0E50;
	start_stop[file][1] = -1.0E50;

	/* move to item 25-4 (item 25-2 and 25-5 are fixed length (total 43
	   bytes) */

	offset += 43;

	/* item 25-4 is the number of APIDs in the current file */

	numAPIDs = PGS_IO_L0_BYTEtoINT(construction_record+offset, 1);

	/* move to item 25-4.1 (item 25-4 is fixed length (1 byte)) */

	offset += 1;

	for (appID=0;appID<numAPIDs;appID++)
	{
	    offset += 4;
	    returnStatus = PGS_TD_EOSPMGIIStoTAI(construction_record+offset,
					     &startTAI93);
	    if  ( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
	    {
		return returnStatus;
	    }
	    
	    offset += 8;
	    returnStatus = PGS_TD_EOSPMGIIStoTAI(construction_record+offset,
					     &stopTAI93);	
	    if  ( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
	    {
		return returnStatus;
	    }
	    
	    offset += 12;

	    if (startTAI93 < start_stop[file][0]) 
	    {
		start_stop[file][0] = startTAI93;
	    }
	    if (stopTAI93 > start_stop[file][1]) 
	    {
		start_stop[file][1] = stopTAI93;
	    }	    
	} /* END: for (appID=... */
    } /* END: for (file=... */

    *size = offset;  /* offset is now = total size of construction record */
    
    return PGS_S_SUCCESS;
}
