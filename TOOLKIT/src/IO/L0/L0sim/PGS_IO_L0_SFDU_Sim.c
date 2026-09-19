/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
   PGS_IO_L0_SFDU_Sim.c

DESCRIPTION:
   This file contains the tool to create a simulated TRMM
   SFDU (Standard Formatted Data Unit) detached header file.

AUTHOR:
   Tom Atwater / Applied Research Corp.
   Xin Wang / EIT Inc.

HISTORY:
   03-Feb-1995 TWA   Initial version
   01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:
	Create a Simulated TRMM SFDU Detached Header File

NAME: PGS_IO_L0_SFDU_Sim
	

SYNOPSIS

  C:
	#include <PGS_IO.h>
	#include <PGS_IO_L0.h>

	PGSt_SMF_status
	PGS_IO_L0_SFDU_Sim(
            PGSt_tag         spacecraftTag,
	    char             *filename,
	    char             *sfdu_filename,
	    PGSt_integer     appID,        
	    char             startUTC[28], 
	    char             stopUTC[28], 
	    char             data_type_flag)

  FORTRAN:
        N/A

DESCRIPTION:
	This tool accepts input from PGS_IO_L0_File_Sim, and writes 
	a simulated SFDU detached header file, which is primarily
	in PARAMETER=Value format.

INPUTS:     
	spacecraftTag - The spacecraft identifier desired for the output data.

	filename      - The name of the Level 0 production or quicklook
	                file associated with the file to be created.

	sfdu_filename - The name of the file to be created containing the 
	                SFDU header information.

	appID         - Application identifier (APID) for this file.
	                Only one APID is allowed.
	                Use "1" for housekeeping files.

	startUTC      - Start UTC time, in format YYYY-MM-DDThh:mm:ssZ

	stopUTC       - Stop  UTC time, in format YYYY-MM-DDThh:mm:ssZ

	data_type_flag - =1, Level 0 production data set;
	                 =2, Qulcklook data set


OUTPUTS:
	None

RETURNS:  
	PGSIO_E_L0_PHYSICAL_OPEN
	PGSIO_E_L0_BAD_SPACECRAFT_TAG

NOTES: 
	This tool is low-level, and is not intended to be called by science
        software. It is called by PGS_IO_L0_File_Sim.

	Format of the parameters written are as best could be determined
	from the specification document, which is
	"Interface Control Document between the
	Sensor Data Processing Facility (SPDF) and the Tropical Rainfall
	Measuring Mission (TRMM) Customers", NASA Mission Operations
	and Data Systems Directorate, Draft, Nov. 1994.
	Some mistakes were found in that document, so it is imperative
	that the format here be reviewed when that document is finalized.

	The detached SFDU header file is used in TRMM processing.
	It is not known at this writing whether EOS AM and PM will
	use this file; support for those platforms in this code 
	is mainly of the placeholder variety.

	There is no Toolkit tool written specifically for reading this file.
        It may be read by using the tool PGS_IO_PC_GetFileAttr or 
        PGS_IO_PC_GetFileByAttr.

EXAMPLES:
	Create a detached SFDU production TRMM header file. 

  C:
#include <PGS_IO.h>
#include <PGS_IO_L0.h>

PGSt_tag         spacecraftTag;
char             filename[1024];
char             sfdu_filename[1024];
PGSt_integer     appID;        
char             startUTC[21]; 
char             stopUTC[21]; 
char             data_type_flag;

PGSt_SMF_status returnStatus;

spacecraftTag = PGSd_TRMM;
strcpy( filename, "TRMM_G001_1995-12-22T14:59:34Z_V01.dataset_01");
appID = 61;
strcpy( startUTC, "1995-12-22T14:59:34Z");
strcpy( stopUTC, "1995-12-23T14:59:34Z");
data_type_flag = 1;

returnStatus = PGS_IO_L0_SFDU_Sim( spacecraftTag, sfdu_filename. filename,
    appID, startUTC, stopUTC, data_type_flag);


Listing of the detached SFDU header file produced:

CCSD3ZF0000100000001CCSD3ZF0000100000001
PROJECT="TRMM>Tropical Rainfall Measuring Mission";
DISCIPLINE="CERES>Clouds and Earth Radiant Energy System";
SOURCE_NAME="TRMM>Tropical Rainfall Measuring Mission";
DATA_TYPE="LZ>Level Zero";
DESCRIPTOR=G0088>APID54;
START_DATE=1995-12-22T14:59:34Z;
STOP_DATE=1995-12-23T14:59:34Z;
DATA_VERSION=01;
GENERATION_DATE=1995-12-22T14:59:34Z;
FILE_ID="TRMM_G001_1995-12-22T14:59:34Z_V01.dataset_01";
REF_FILE="TRMM_G001_1995-12-22T14:59:34Z_V01.dataset_01";
CCSD3RF0000300000001
REFERENCE_TYPE=$CCSDS3;
LABEL=NSSDnXXnnnnnnnnnnnnn;
REFERENCE="TRMM_G001_1995-12-22T14:59:34Z_V01.dataset_01";

End listing

  FORTRAN: N/A


REQUIREMENTS:
	There is no SDP Toolkit requirement for this functionality. 
	This tool was created to support internal ECS SDP Toolkit development 
	and testing, and it is being provided as a service to the user.

DETAILS:
	None

GLOBALS:
	None

FILES:
        This tool creates the data file specified by the input parameter
        sfdu_filename.

FUNCTIONS CALLED:
        PGS_SMF_SetStaticMsg

END_PROLOG
*******************************************************************************/
#include <PGS_IO.h>
#include <PGS_IO_L0.h>
#include <string.h>
#include <sys/types.h>

#define MAXLBL 1024

PGSt_SMF_status
PGS_IO_L0_SFDU_Sim(
            PGSt_tag         spacecraftTag,
	    char             *filename,
	    char             *sfdu_filename,
	    PGSt_integer     appID,        
	    char             startUTC[21], 
	    char             stopUTC[21], 
	    char             data_type_flag )

{

    char sfdu_edu_label[MAXLBL];
    char project[MAXLBL];
    char discipline[MAXLBL];
    char source_name[MAXLBL];
    char data_type[MAXLBL];
    char descriptor[MAXLBL];
    char start_date[MAXLBL];
    char stop_date[MAXLBL];
    char data_version[MAXLBL];
    char generation_date[MAXLBL];
    char file_id[MAXLBL];
    char ref_file[MAXLBL];
    
    char sfdu_cio_label[MAXLBL];
    char reference_type[MAXLBL];
    char label[MAXLBL];
    char reference[MAXLBL];
    
    char appID_str[10];
    FILE *sfdu;
    char *toolname="PGS_IO_L0_SFDU_Sim";
    PGSt_SMF_status returnStatus;
    
    returnStatus = PGS_S_SUCCESS;
    
    /* Open SFDU file for write
       (We have to use fopen and not the Toolkit function PGS_IO_Gen_Temp_Open
	because we need to create a file with a specific file name) */
    
    sfdu = fopen( sfdu_filename, "w" );
    if( sfdu == NULL )
    {
	returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }
    
    /* References below are to "Interface Control Document between the
       Sensor Data Processing Facility (SPDF) and the Tropical Rainfall
       Measuring Mission (TRMM) Customers", NASA Mission Operations
   and Data Systems Directorate, Draft, Nov. 1994.
   References are denoted by "ICD". */
    
    /* Print each field to file in order given in ICD */
    
    /* ICD Sec. 3.2.2.2 */
    strcpy( sfdu_edu_label, "CCSD3ZF0000100000001" );
    fprintf( sfdu, "%s", sfdu_edu_label );
    
    /* ICD Sec. 3.2.2.3 */
    strcpy( sfdu_cio_label, "NSSD3KF0006000000001" );
    fprintf( sfdu, "%s\n", sfdu_edu_label );
    
    /* ICD Sec. 3.2.2.4 and Fig. 10.4.9: S/C and instrument specific data */
    
    sprintf( appID_str, "%2d", (int) appID );
    
    switch (spacecraftTag)
    {
      case PGSd_TRMM:
	strcpy( project, "\"TRMM>Tropical Rainfall Measuring Mission\"" );
	strcpy( source_name, project );
	switch( appID )
	{
	    /*       TRMM "Discipline" labels are unknown at this writing;
		     values are made up */
	  case 1:
            strcpy( discipline, "\"TRMM>Housekeeping\"" );
            sprintf( descriptor, "G000%1d", (int) data_type_flag );
            break;
	  case 54:
	  case 55:
	  case 56:
            strcpy( discipline,
		   "\"CERES>Clouds and Earth Radiant Energy System\"" );
            sprintf( descriptor, "G00%2d>APID%2d", (int) (appID+34), 
		     (int) appID );
            break;
	  case 61:
            strcpy( discipline, "\"LIS>Lightning Imaging Sensor\"" );
            strcpy( descriptor, "G0091>APID61" );
            break;
	  default:
            sprintf( discipline, "\"Gxxxx>APID%2d\"", (int) appID );
            sprintf( descriptor, "Gxxxx>APID%2d", (int) appID );
	}
	break;
      case PGSd_EOS_AM:
	strcpy( project, "EOS_AM>Earth Observing System AM platform" );
	strcpy( source_name, project );
	strcpy( discipline, "APID>APID " );
	strcat( discipline, appID_str );
	strcpy( descriptor, appID_str );
	break;
      case PGSd_EOS_PM:
	strcpy( project, "EOS_PM>Earth Observing System PM platform" );
	strcpy( source_name, project );
	strcpy( discipline, "APID>APID " );
	strcat( discipline, appID_str );
	strcpy( descriptor, appID_str );
	break;
      case PGSd_EOS_AURA:
        strcpy( project, "EOS_AURA>Earth Observing System AURA platform" );
        strcpy( source_name, project );
        strcpy( discipline, "APID>APID " );
        strcat( discipline, appID_str );
        strcpy( descriptor, appID_str );
        break;
      default:
	returnStatus = PGSIO_E_L0_BAD_SPACECRAFT_TAG;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }
    
    fprintf( sfdu, "PROJECT=%s;\n", project );
    fprintf( sfdu, "DISCIPLINE=%s;\n", discipline );
    fprintf( sfdu, "SOURCE_NAME=%s;\n", source_name );
    
    /* ICD Sec. 3.2.2.4 and Fig. 10.4.9 */
    if( data_type_flag == 2)
      strcpy( data_type, "\"QL>QuickLook\"" );
    else
      strcpy( data_type, "\"LZ>Level Zero\"" );
    fprintf( sfdu, "DATA_TYPE=%s;\n", data_type );
    
    /* ICD Sec. 3.2.2.4 and Fig. 10.4.9, table 10.4.6 */
    fprintf( sfdu, "DESCRIPTOR=%s;\n", descriptor );
    
    /* ICD Sec. 3.2.2.4 and Fig. 10.4.9 */
    
    strcpy( start_date, startUTC );
    fprintf( sfdu, "START_DATE=%s;\n", start_date );
    
    strcpy( stop_date, stopUTC );
    fprintf( sfdu, "STOP_DATE=%s;\n", stop_date );
    
    strcpy( data_version, "01" );
    fprintf( sfdu, "DATA_VERSION=%s;\n", data_version );
    
    /* Arbitrarily use start time as "Generation Date" */
    strcpy( generation_date, startUTC );
    fprintf( sfdu, "GENERATION_DATE=%s;\n", generation_date );
    
    strcpy( file_id, filename );
    fprintf( sfdu, "FILE_ID=\"%s\";\n", file_id );
    
    strcpy( ref_file, filename );
    fprintf( sfdu, "REF_FILE=\"%s\";\n", ref_file );
    
    /* ICD Sec. 3.2.2.5 (missing from Fig. 10.4.9) */
    
    strcpy( sfdu_cio_label, "CCSD3RF0000300000001" );
    fprintf( sfdu, "%s\n", sfdu_cio_label );
    
    /* ICD Sec. 3.2.2.6 and Fig. 10.4.9 */
    
    strcpy( reference_type, "$CCSDS3" );
    fprintf( sfdu, "REFERENCE_TYPE=%s;\n", reference_type );
    
    /* This value is not explained well in the ICD, so it is a guess
       from ICD Fig. 3-7 */
    strcpy( label, "NSSDnXXnnnnnnnnnnnnn" );
    fprintf( sfdu, "LABEL=%s;\n", label );
    
    strcpy( reference, filename );
    fprintf( sfdu, "REFERENCE=\"%s\";\n", reference );
    
    fclose(sfdu);
    
    return returnStatus;
}
