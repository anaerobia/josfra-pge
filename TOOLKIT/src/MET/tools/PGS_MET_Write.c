/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_MET_Write
 
DESCRIPTION:
	Enables the user to write different master groups of metadata to separate
	HDF attributes. Whole of the MCF can also be written to a separate ascii file. 
	Attributes can only be read back using PGS_MET_GetPCAttr tool.
	
AUTHOR: 
        Alward N. Siyyid/ EOSL
        Carol S. W. Tsai / Applied Reseach Corporation
	Abe Taaheri / Emergent Information Technologies, Inc.

HISTORY:
        18-MAY-95      ANS     Initial version
	01-JUN-95      ANS     Code inspection comments update
	13-July-95     ANS     Improved Fortran example
	20-July-95	ANS	Fixed bug ECSed01021
	21-July-95	ANS     Cleaned up fix for ECSed01021
	04-MAR-96	ANS	Updated for tk5+
	19-Aug-96       ANS     added fclose for the separate ascii file
	                        HDF attributes more than MAX_ORDER can now be
				written.
        06-Feb-97      CSWT    added PGS_PC_GetConfigData and
                               PGS_PC_GetReference to make use of the ASCII
                               metadata files the TOOLKIT creates for non
                               HDF-EOS product output files.

        14-APR-97      CSWT    added code to provide this function can automatically
                               populate ProcuctionDateTime with the value of the time
                               at which science software processing of a given data
                               granule is being completes.
        24-Jun-97      CSWT    Fixed the code for setting up the attribute
                               ProductionDateTime by calling the UNIX system 
                               time is also available when writing the Meta
                               data to an ASCCI file using this function in 
                               which the mdHandle argument, the first input
                               parameter of the function PGS_MET_Write() is 
                               not the group name of INVENTORYMETADATA.
        22-Sep-97      CSWT    Fixed code of writing .met files to the product/data
                               directory instead of writing to the runtime (current
                               working) directory
        04-Dec-97      CSWT    Fixed code of replacing the C library function strtok used  
                               to break the string into a sequence of tokens with the C 
                               standard I/O function sscanf used to read characters,
                               interpret them according to a format, and store the results
                               in its argument/arguments  
                               Removed all the status checking codes followed after calling   
                               the function WriteLabel. The function WriteLabel is a avoid
                               function, set up the status checking code will cause the 
                               memory problem of attempting to read from uninitialized 
                               memory.
                               Changed the character string "PGS_MET_LoadAggregate", the  
                               second parameter passed into the function PGS_MET_ErrorMsg, 
                               to funcName declared for character string "PGS_MET_Write" 
        14-Jul-98      CSWT    Added code to provide the capability of writing the metadata 
                               file  that are associated with a particular Product ID and
                               Version number listed in the PCF file for the modes of
                               INTERMEDIATE INPUT and INTERMEDIATE OUTPUT (This change is
                               for NCR ECSed15639 about PGS_MET_Write does not support 
                               intermediate files)
	27-Jul-98 Abe Taaheri  Fixed the problem with the number of arguments in
	                       PGS_IO_Gen_Temp_Open
	07-Jul-99 RM           Updated for TSF functionality
        20-Sep-99 Abe Taaheri  Added few lines after every fclose or
	                       PGS_IO_Gen_Close to write a message in the
			       LogStatus file, such as "Unexpected failure
			       closing file ...", and return error code 
			       PGS_E_TOOLKIT, if for any reason the opened 
			       input/output/temporary files cannot be closed.
        12-June-00 Abe Taaheri Modified writing PRODUCTIONDATETIME attribute so that 
                               the time can be set by PGE if it is attempted
			       (the time set by PGE will not be overwritten.
			       per NCR ECSed27009).
        12-Sep-00 Abe Taaheri  Modified writing PRODUCTIONDATETIME attribute
                               to set it only if data location is "TK" and
			       parameter PRODUCTIONDATETIME exists in MCF
        30-Mar-01 AT           Modified for HDF5 support and fixed an Array
                               Bounds Write problem with writing metadata to
                               HDF4 type files.
        17-Jan-02 AT           modified the code so that instead of using
                               PGS_IO_GEN_Open to open a temporary file, we
                               get the temporary filename, extend the name
                               with process ID and a number obtained using the
                               HDF filename, and open it with fopen. This will
                               avoid collision between two or more simultanous
                               processes that previously were using the same 
                               temporary filename when they were geting the
                               temporary filename from the same PCF file. 
        17-Nov-08 AT           Added error check for WriteLabel to catch meory problem
                               report from that function.Note that now WriteLabel is
                               "int" instead of "void"
        06-May-09 AT           Added routines to create XML metadata .xml file,
                               in addition to writing it to the hdf file.

END_FILE_PROLOG
*******************************************************************************/

/* include files */

#define _BSD_COMPAT /* this definition is necessary for the local_nc.h to compile with -ansiposix flag */
#include <stdio.h>
#include <time.h>
#include "PGS_MET.h"
#include <PGS_MEM.h>
#include <PGS_TSF.h>
#include <unistd.h>

#include <errno.h>

/* odl include files */

#include <CUC/odldef.h>
#include <CUC/odlinter.h>

/* hdf files */

#ifdef DEC_ALPHA        /* DEC Alpha platform */
#include <rpc/types.h>  /* this avoids typedef conflict with int32, uint32 */
#endif /* DEC_ALPHA */
#include <hdf.h>
#include <mfhdf.h>
#include <hdf5.h>
#include "local_nc.h"
#include "hfile.h"

extern struct HDF5files         /* structure to hold HDF5 filenames 
				   and their file IDs */
{
  char file_name[PGSd_SMF_PATH_MAX];              /* file name */
  PGSt_integer file_id;                           /* file ID */
  PGSt_integer open_flag;       /* flag that indicates file is open (=1) or
				   closed (=0) */
} cat_HDF5[PGSd_MET_MAX_NUM_FILES];

extern int N_HDF5_FILES_OPENED;

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Writes metadata and their values to HDF attributes
  
NAME:  
        PGS_MET_Write()

SYNOPSIS:
C:
        #include "PGS_MET.h"

	PGSt_SMF_status
		PGS_MET_Write(
			PGSt_MET_handle mdHandle,
			char *          hdfAttrName,
			PGSt_integer    hdfFileId)
FORTRAN:
         include 'PGS_MET_13.f'
	 include 'PGS_MET.f' 
	 include 'PGS_SMF.h'
         integer function pgs_met_write(mdHandle, hdfAttrName, hdfFileId)

	 character*   mdHandle
         character*   hdfAttrName
	 integer      hdfFileId
   
DESCRIPTION:
	This is the final tool that pge uses when all the metadata parameters
	are set in memory. The tool checks that all the mandatory parameters 
	are set. 

INPUTS:
        Name            Description            Units    	Min	Max
        ----            -----------             -----   	---     ---
	mdHandle	metadata group		none		N/A	N/A
			in MCF
        hdfAttrName	HDF file attribute 	none            N/A     N/A
			name
	hdfFileId	HDF file ID		none            N/A     N/A

OUTPUTS:
	None

RETURNS:   
   	PGS_S_SUCCESS			
	PGSMET_E_NO_INITIALIZATION	Metadata file is not initialized
	PGSMET_E_ODL_MEM_ALLOC		ODL routine failed to malloc memmory space
	PGSMET_E_GROUP_NOT_FOUND	No group called <name> found in the MCF
	PGSMET_E_OPEN_ERR		Unable to open <temporary> file with file id <fileId>
	PGSMET_E_SD_SETATTR		Unable to set the HDF file attribute
					NOTE: HDF4.0r1p1 and previous versions of HDF have imposed a limit
					of 32k on the size attribute
	PGSMET_E_MALLOC_ERR		Unable to allocate memory for the hdf attribute
	PGSMET_E_MAND_NOT_SET		Some of the mandatory parameters were not set
					Note: HDF attribute is still written out
	PGSMET_E_FGDC_ERR		Unable to conver UTC input date time string to FGDC values
	PGS_MET_E_ILLEGAL_HANDLE        Handle is illegal. Check that initialization has taken place
	PGS_MET_E_HDFFILENAME_ERR	Unable to obtain hdf file name
	PGSMET_E_MET_ASCII_ERR		Unable to open MET ascii file
	PGSTSF_E_GENERAL_FAILURE	problem in TSF code
	

EXAMPLES:
C:

This is an extract from the main example in PGS_MET_Init.c

			ret = PGS_MET_Write(handles[ODL_IN_MEMMORY], NULL, (PGSt_integer)NULL);
			if(ret != PGS_S_SUCCESS)
			{	
				printf("ASCII Write failed\n");
			}
			ret = PGS_MET_Write(handles[INVENTORYMETADATA], "metadata", sdid);
			if(ret != PGS_S_SUCCESS)
                        {
                                printf("HDF Write failed\n");
                        }

FORTRAN:
	This is just an extract of the call from a full example given in PGS_MET_Init() prolog.

C	ascii file is written to file with id 10255
			result = pgs_met_write(groups(ODL_IN_MEMORY), dummyStr, dummyInt)
			if(result.NE.PGS_S_SUCCESS  .AND. result.NE.PGSMET_MAND_NOT_SET) then
				print *,"ASCII Write failed"
			endif
C			write the first group as attribute 
			result = pgs_met_write(groups(INVENTORYMETADATA), "coremetadata.0", sdid)
			if(result.NE.PGS_S_SUCCESS .AND. result.NE.PGSMET_MAND_NOT_SET) then
				print *,"ASCII Write failed"
			endif
NOTES:
 	NONE

REQUIREMENTS:
        PGSTK-0290, PGSTK-0380, PGSTK-0400, PGSTK-0450 PGSTK-0510

DETAILS:
	This routine can be used multiple times to write/attach separate mastergroups
	as local or global HDF attributes.

	Addendum for tk5+

	A number of changes have been introduced for tk5+:

	1.	A separate ascii dump of the MCF in memory can now be written out. The name of the file 
		should be defined in the PCF. A file id has been reserved  for this purpose and is 10255
		(PGSd_MET_ASCII_DUMP). The user needs to call PGS_MET_Write with mdHandle[0] and the rest
		of the arguments set to null.

		**RELEASE A ADDENDUM**

		This is still true. However, its now a default behavior. If user wishes to use his/her 
		own file, he/she must supply the id in the "hdffileid" argument. Note that in this case
		the file id is the toolkit file id and not the hdf file id.

		**END RELEASE A ADDENDUM**

	2.	If MANDATORY parameters are not set, an error PGSMET_E_MAND_NOT_SET is returned. The value 
		of the metadata is set to as follows:
				DATA_LOCATION		VALUE
				PGE			"NOT SET"
				PCF			"NOT FOUND"
				MCF			"NOT SUPPLIED"
		The writing of the hdf header is not affected

		NOTE:	A warning PGSMET_W_METADATA_NOT_SET is issued if MANDATORY has the value FALSE in the MCF
			
		**RELEASE A ADDENDUM**

			Also anything which is not mandatory and not set by the user is removed

		**END RELEASE A ADDENDUM**

	3.	Only system errors such as memory failurer, file openings etc should be able to abort the write procedure.

	4.	NUM_VAL and CLASS fields are written in the hdf header

	5.	This routine creates two new metadata objects for metadata with type DATETIME. The UTC time
		string is used to create an FGDC date object and FGDC time object as strings. The object names 
		are same as parent metadata with DATE and TIME appended at the end.

		eg.	OBJECT                 = DATE_TIME
    			NUM_VAL              = 1
    			VALUE                = "1989-04-11T12:30:45.7Z"
  			END_OBJECT             = DATE_TIME
 
  			OBJECT                 = DATE_TIMEDATE
    			NUM_VAL              = 1
    			VALUE                = "19890411"
  			END_OBJECT             = DATE_TIMEDATE
 
  			OBJECT                 = DATE_TIMETIME
    			NUM_VAL              = 1
    			VALUE                = "123045700000Z"
  			END_OBJECT             = DATE_TIMETIME

		The original MCF simply contained the definition for DATE_TIME
	**RELEASE A ADDENDUM**

	1.	When inventory metadata group is written out, it is also dumped in a file as ascii with the name
		hdffilename.met in the current working directory. "hdffilename" is the hdf file that metadata is
		written to.

	2.	For the following metadata of type DATETIME additional metadata is produced as follows:

			CALENDERDATATIEM	produces	CALENDERDATE and CALENDERTIME
			RANGEBEGINNINGDATETIME  produces 	RANGEBEGINNINGDATE and RANGEBEGINNINGTIME
			RANGEENDINGDATETIME  	produces        RANGEENDINGDATE and RANGEENDINGTIME

		e.g.	"1989-04-11T12:30:45.7Z" produces	"1989-04-11" and "12:30:45.70000"

		This is only true for these three metadata and therefore point 5. in the previous
		addendum is now obsolete

	3.	User no longer has to worry about the size of MCF exceeding the HDF limit on attribute sizes
		This is now handled internally. 


	**END RELEASE A ADDENDUM**

			
GLOBALS:
	PGSg_MET_MasterNode

FILES:
	product files

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg
	PGS_MET_CheckAttr
	PGS_MEM_Malloc
	PGS_MEM_Free
	PGS_IO_Gen_Open
	PGS_IO_Gen_Temp_Open
	PGS_IO_Gen_Close
	PGS_IO_Gen_Close
	PGS_IO_Gen_Temp_Delete
	PGS_MET_ErrorMsg
	FindGroup
	NextGroup
	RemoveAggregate
	CopyAggregate
	WriteLabel
	SDsetattr
	PGS_TSF_GetTSFMaster
	PGS_SMF_TestErrorLevel
         H5Eset_auto
         H5Gopen
         H5Gcreate
         H5Screate
         H5Tcopy
         H5Tset_size
         H5Dopen
         H5Dwrite
         H5Sclose
         H5Dclose
         H5Gclose
	
END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_MET_Write(
             PGSt_MET_handle mdHandle,  /* Handle for the MCF meta data group in memory */
             char *          hdfAttrName, /* name of hdf attribute to be attached */
             PGSt_integer    hdfFileId) /* HDF product file id or local
                                          * data id
                                          */
{
        struct tm *local;
        time_t lt;

	PGSt_IO_Gen_FileHandle  *fileHandle = NULL;
	AGGREGATE		groupNode = NULL;
	AGGREGATE               tempNode = NULL;
	AGGREGATE               newNode = NULL;
	AGGREGATE 		oldParentNode  = NULL;
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	PGSt_SMF_status         checkRetVal = PGS_S_SUCCESS;
	PGSt_SMF_status         closestatus = PGS_S_SUCCESS;
	int32			hdfRetVal = 0;
	intn			hdfRet;
        char                    details[PGS_SMF_MAX_MSG_SIZE];
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char *			funcName = "PGS_MET_Write";
        char                    *createtime=NULL;
	char 			*mcfBuffer = NULL;
	char                    *mcfBuffer_temp = NULL;
	char 			fileIdStr[10];
      	char *			newAttrName= NULL;
        PGSt_integer            logicalId; /* user defined logical ID that
                                              represents the Product listed
                                              in the PCF  */

	PGSt_integer		i = 0; /* loop count */
	PGSt_integer		numBytesRead = 0;
        PGSt_integer            version;   /* The number of versions associated
                                              with the requested product Logical
                                              ID */
        PGSt_integer            Fileversion;   
	PGSt_integer		numAttr = 0;
	PGSt_integer		filType;
	FILE *			asciiFile = NULL;
        char                    configParam[PGSd_PC_VALUE_LENGTH_MAX]; 
                                           /* a string represents the value of
                                              the configration parameter */
        char                    configParamm[PGSd_PC_VALUE_LENGTH_MAX]; 
                                           /* a string represents the value of
                                              the configration parameter */
        char                    configDataString1[PGSd_PC_VALUE_LENGTH_MAX]; 
                                           /* a substring extracts from the value
                                              of the configration parameter */
        char                    configDataString2[PGSd_PC_VALUE_LENGTH_MAX]; 
                                           /* a substring extracts from the value
                                              of the configration parameter */
        char                    handleString[PGSd_PC_VALUE_LENGTH_MAX]=" ";
	char * 			hFileName = NULL;
	char                    asciiFileRef[PGSd_PC_FILE_PATH_MAX];
	char                    asciiFileName[PGSd_PC_FILE_PATH_MAX];
	intn                    access;
	intn			attach;
	NC			*hdfhandle = NULL;
	PGSt_integer            mcfNumber = 0;
        char *                  mcfNumPtr = NULL;
        char *                  handlePtr = NULL;
        char *                  errPtr = NULL;
        char                    groupHandle[PGSd_MET_GROUP_NAME_L] = "";
        char                    locMdHandle[PGSd_MET_GROUP_NAME_L] = "";

	char                    signature[PGSd_MET_SIGNATURE_L] = "Razia";
	PGSt_integer            fileInBuffer = 1;
        PGSt_integer            bufferIndex = 0;
        PGSt_IO_Gen_Duration    file_duration = 1;
#ifndef PGS_DAAC
        PGSt_boolean            tempReferenceflag = PGS_FALSE;
#endif
        PGSt_boolean            existFlag = PGS_FALSE;
	char                    dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
	char                   *proddatetime = NULL;
	PGSt_integer            tempfileversion;
	char                    tempfilename[PGSd_PC_FILE_PATH_MAX];
	char                    TempFilename[PGSd_PC_FILE_PATH_MAX];
	char                    pid_string[20];  /* process ID (PID) string */
	PGSt_integer            len_filename, len_count;
	PGSt_uinteger           sum_chars;
	char                    cmd[PGSd_PC_FILE_PATH_MAX];
	char                    xmlFileName[PGSd_PC_FILE_PATH_MAX]= "";
	char		        referenceID[PGSd_PC_FILE_PATH_MAX]= "";
	/* hdf5 related parameters */

        int filecount;
        hid_t           ggid;
        hid_t           sid2;
        hid_t           datid;
        herr_t          status = -1;
        size_t          size;
        hid_t           atype;
        hid_t           HDFfid;
        
	PGSt_integer		FILE_IS_HDF4 = 0;
	PGSt_integer		FILE_IS_HDF5 = 0;
        int                     writelbl_stat = 0;

	char                    XML_config_buf[10];
	PGSt_SMF_status 	xmlretVal = -1;
	PGSt_integer            xml_write_flag = 0;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retTSF;
    struct tm threadTime;
    AGGREGATE *PGSg_MET_MasterNode;
    char *PGSg_MET_AttrHandle;
    PGSt_TSF_MasterStruct *masterTSF;

    /* get master TSF struct */
    retTSF = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(retTSF))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* load locals from TSD values */
    PGSg_MET_MasterNode = (AGGREGATE *) pthread_getspecific(
                                  masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE]);
    PGSg_MET_AttrHandle = (char *) pthread_getspecific(
                                  masterTSF->keyArray[PGSd_TSF_KEYMETATTRHANDLE]);
#else
	extern AGGREGATE	PGSg_MET_MasterNode[];
	extern char		PGSg_MET_AttrHandle[][PGSd_MET_SIGNATURE_L];
#endif

	/* clear the errno for the ODL routines */

        if (errno == ERANGE)
        {
                errno = 0;
        }

        /* find out the MCF number contained in the mdHandle argument*/

	if(mdHandle != NULL)
	{
        	mcfNumPtr = strrchr(mdHandle, '#');
	}
        if(mcfNumPtr == NULL)
        {
                (void) PGS_SMF_SetStaticMsg(PGSMET_E_ILLEGAL_HANDLE, funcName);
                return(PGSMET_E_ILLEGAL_HANDLE);
        }
        /* Generate the Greenwich mean time of the system to be the value of
           metadata parameter, ProductionDataTime,  in order this attribute
           value will be automatically written to an ASCII file and to an HDF-EOS
           file as appropriate */
 
        strcpy(groupHandle, mdHandle);
        handlePtr = strrchr(groupHandle, '#');
 
        if (handlePtr != NULL)
        {
              sscanf(groupHandle, "%[^#]", handleString);
            if ((strcmp(handleString,PGSd_MET_INVENT_DATA) == 0) || (strcmp(handleString,"MCF") == 0))
            {
	        createtime=(char *) malloc(30);
                lt=time(NULL);
#ifdef _PGS_THREADSAFE
                /* gmtime() not threadsafe - use gmtime_r() */
                local=gmtime_r(&lt,&threadTime);
                local=&threadTime;
#else
                local=gmtime(&lt);
#endif
                strftime(createtime,80,"%Y-%m-%dT%X.000Z",local);

		/*check to see if ProductionDateTime is set by PGE */ 
		proddatetime = (char *) malloc(30);
		retVal = PGS_MET_GetSetAttrTD(mdHandle,"ProductionDateTime",
					    &proddatetime);
		if(retVal != PGS_S_SUCCESS)
		{
		    if(retVal ==  PGSMET_W_METADATA_NOT_SET)
		      /* Mandatory ProductionDateTime is supposed to be set by
			 PGE. But it has not been set by it. Leave it unset, 
			 But issue a warning so the user know what the problem
			 is. */
		    {
			free(createtime);
			free(proddatetime);
			sprintf(details,
			     "The data location for PRODUCTIONDATETIME is PGE,"
			     " but it has not been set by PGE");
			PGS_SMF_SetDynamicMsg(PGSMET_W_METADATA_NOT_SET, 
					      details, funcName);

		    }
		    else if( retVal ==  PGSMET_M_METADATA_NOT_SET_TK)
		    {/* TOOLKIT will set ProductionDateTime */
			retVal=PGS_MET_SetAttr(mdHandle,"ProductionDateTime",
					       &createtime);
			if (retVal != PGS_S_SUCCESS)/*problem in setting attr*/
			{
			    free(proddatetime);
			    free(createtime);
			    (void) PGS_SMF_SetStaticMsg(
				PGSMET_E_SET_ATTRIBUTE_ERROR, funcName);
			    return(PGSMET_E_SET_ATTRIBUTE_ERROR);
			}
			free(createtime);
			free(proddatetime);
			retVal = PGS_S_SUCCESS;
		    }
		    else if(retVal ==  PGSMET_M_METADATA_NOT_SET_PGE)
		    {/* PGE is supposed to set ProductionDateTime, but it is
			not mandatory and it has not been set by PGE. 
			TOOLKIT will not set it either */
			free(createtime);
			free(proddatetime);
			retVal = PGS_S_SUCCESS;
		    }
		    else if(retVal ==  PGSMET_E_DD_UNKNOWN_PARM)
		    {
			/* ProductionDateTime object is not in the MCF,
			 do not try to set it */
			free(createtime);
			free(proddatetime);
			retVal = PGS_S_SUCCESS;
		    }
		    else   /* an error occured in PGS_MET_GetSetAttrTD, i.e.
			      ( retVal ==  PGSMET_E_NO_DEFINITION ||
			      retVal ==  PGSMET_E_NO_INITIALIZATION ||
			      retVal ==  PGSMET_E_ILLEGAL_HANDL ||
			      retVal ==  PGSTSF_E_GENERAL_FAILURE) */
		    {
			free(createtime);
			free(proddatetime);
			(void) PGS_SMF_SetStaticMsg(
			    PGSMET_E_GETSET_ATTRIBUTE_ERROR, funcName);
			return(PGSMET_E_GETSET_ATTRIBUTE_ERROR);
		    }
		}
		else /* ProductionDateTime is set by PGE */
		{
		    free(createtime);
		    free(proddatetime);
		}	
            }
        }

        mcfNumPtr++;
        mcfNumber = (PGSt_integer)strtol(mcfNumPtr, &errPtr, 10);
        if(mcfNumber < 0 || mcfNumber > PGSd_MET_NUM_OF_MCF || errPtr == mcfNumPtr)
        {
                (void) PGS_SMF_SetStaticMsg(PGSMET_E_ILLEGAL_HANDLE, funcName);
                return(PGSMET_E_ILLEGAL_HANDLE);
        }

	

	/* if master node is null then initialization has not taken palce */

        if(PGSg_MET_MasterNode[mcfNumber] == NULL)
        {
                (void) PGS_SMF_SetStaticMsg(PGSMET_E_NO_INITIALIZATION, funcName);
                return(PGSMET_E_NO_INITIALIZATION);
        }

	/* now make a local copy of the mdHandle */

        strcpy(locMdHandle, mdHandle);
        mcfNumPtr = strrchr(locMdHandle, '#');
        *mcfNumPtr = '\0';

#ifdef _PGS_THREADSAFE
        /* calling COTS (ODL) - lock up */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
            return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	/* make a copy of the master node and use that */

	tempNode = CopyAggregate(PGSg_MET_MasterNode[mcfNumber]);

#ifdef _PGS_THREADSAFE
        /* re-set TSD value */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                 PGSg_MET_MasterNode);
#endif

	if(tempNode == NULL)
	{
		(void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                     funcName, 0, errInserts);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
		return(PGSMET_E_ODL_MEM_ALLOC);
	}

	/* find the group in the MCF to be written */

	groupNode = FindGroup(tempNode, locMdHandle);
	if(groupNode == NULL)
	{
            errInserts[0] = locMdHandle;

            /* error message is:
            "No group called <name> found in the MCF" */

            (void) PGS_MET_ErrorMsg(PGSMET_E_GROUP_NOT_FOUND,
				    funcName, 1, errInserts);
	    tempNode = RemoveAggregate(tempNode);
#ifdef _PGS_THREADSAFE
            /* unlock - do not check return - let user know about 
               previous error */
            PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
            return(PGSMET_E_GROUP_NOT_FOUND);
        }
	
	/* check that all the mandatory parameters, if any are set */
	
	checkRetVal = PGS_MET_CheckAttr(groupNode);
        if(checkRetVal != PGS_S_SUCCESS)
        {
                if(checkRetVal == PGSMET_E_ODL_MEM_ALLOC)
                {
                        tempNode = RemoveAggregate(tempNode);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - let user know about 
                           previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                        return(PGSMET_E_ODL_MEM_ALLOC);
                }
		if(checkRetVal == PGSMET_E_FGDC_ERR)
                {
                        tempNode = RemoveAggregate(tempNode);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - let user know about 
                           previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                        return(PGSMET_E_FGDC_ERR);
                }
        }
	if(strcmp(locMdHandle, PGSd_MET_MCF_NAME) == 0)
        {
	        /* write metadata for non-HDF file */

                /* if input hdfFileId is NULL use default value */

		if(hdfFileId == (PGSt_integer)NULL)
		{
                        hdfFileId = PGSd_MET_ASCII_DUMP;
		}
                
                /* retrieve the user defined configuration paratemeters 
                   associated with the input logical identifer from the
                   PCF */
		
                retVal = PGS_PC_GetConfigData(hdfFileId,configParam);
		
                /* if the return status is unsuccessful, then set up the 
                   return value */
		
                if (retVal != PGS_S_SUCCESS)
                {
#ifndef PGS_DAAC
		    /* if this is not the DAAC version then try the old method
		       of writing ASCII metadata */

		    goto OLD_CODE;
#else
                    retVal = PGSMET_E_MET_ASCII_ERR;
                    sprintf(details,
                            "Unable to write metadata for non-HDF data file. "
                            "Invalid hdfFileId value (%d) specified.",
                            hdfFileId);
                    PGS_SMF_SetDynamicMsg(retVal, details, funcName); 
#ifdef _PGS_THREADSAFE
                    /* unlock - do not check return - let user know about 
                       previous error */
                    PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                    return retVal;
#endif
                }
  		
                /* if the return status is successful, then call strtok()
                   to get tokens, the logical ID and version number
                   associated with a data file, from the configuration   
                   parameters  */
		
                strcpy(configParamm,configParam);
                sscanf(configParam,"%[^:]",configDataString1);

                if (configDataString1 == NULL)
                {
                    retVal = PGSMET_E_MET_ASCII_ERR;
                    sprintf(details,
                            "Unable to write metadata for non-HDF data file. "
                            "Invalid data file association for specified hdfFileId (%d).",
                            hdfFileId);
                    PGS_SMF_SetDynamicMsg(retVal, details, funcName);
#ifdef _PGS_THREADSAFE
                    /* unlock - do not check return - let user know about 
                       previous error */
                    PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                    return retVal;
                }
                logicalId=atoi(configDataString1);
 		
                sscanf(configParamm,"%*[^:]:%s",configDataString2);

                if (configDataString2 == NULL)
                {
                    retVal = PGSMET_E_MET_ASCII_ERR;
                    sprintf(details,
                            "Unable to write metadata for non-HDF data file. "
                            "Invalid data file association for specified hdfFileId (%d).",
                            hdfFileId);
                    PGS_SMF_SetDynamicMsg(retVal, details, funcName);
#ifdef _PGS_THREADSAFE
                    /* unlock - do not check return - let user know about 
                       previous error */
                    PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                    return retVal;
                }
		
                version=atoi(configDataString2);
                if (version <= 0)
                {
                    retVal = PGSMET_E_MET_ASCII_ERR;
                    sprintf(details,
                            "Unable to write metadata for non-HDF data file. "
                            "Invalid data file association for specified hdfFileId (%d).",
                             hdfFileId);
                    PGS_SMF_SetDynamicMsg(retVal, details, funcName);
#ifdef _PGS_THREADSAFE
                    /* unlock - do not check return - let user know about 
                       previous error */
                    PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                    return retVal;
                }
 		
                /* if the return status is successful, then call PGS_PC_GetReferenceType()
                   with the logical ID */
 
                retVal = PGS_PC_GetReferenceType(logicalId,
                                                 &filType);
 
                /* if the return status is successful, then call PGS_PC_GetRef-
                   erence() to retrieve the physical file reference associated 
                   with the logical ID */
  	
                if (retVal == PGS_S_SUCCESS &&
                    (filType == PGSd_PC_INPUT_FILE_NAME ||
                     filType == PGSd_PC_OUTPUT_FILE_NAME ||
                     filType == PGSd_PC_SUPPORT_IN_NAME || 
                     filType == PGSd_PC_SUPPORT_OUT_NAME ))
                {
                     retVal = PGS_PC_GetReference(logicalId,
		         			  &version,
					          asciiFileRef);
                }
                else if (retVal == PGS_S_SUCCESS &&
                         (filType == PGSd_PC_TEMPORARY_FILE ||
                          filType == PGSd_PC_INTERMEDIATE_INPUT ||
                          filType == PGSd_PC_INTERMEDIATE_OUTPUT))
                {
                     retVal = PGS_IO_Gen_Temp_Reference(file_duration,
                                                        logicalId,
                                                        PGSd_IO_Gen_Write,
                                                        asciiFileRef,
                                                        &existFlag);
#ifndef PGS_DAAC
                     tempReferenceflag = PGS_TRUE;
#endif
                }

		
                if (retVal != PGS_S_SUCCESS)
                {
                    retVal = PGSMET_E_MET_ASCII_ERR;
                    sprintf(details,
                            "Unable to write metadata for non-HDF data file. "
                            "Invalid data file association (%d:%d) "
                            "for specified hdfFileId (%d).",
                            logicalId,version,hdfFileId);
                    PGS_SMF_SetDynamicMsg(retVal, details, funcName);
#ifdef _PGS_THREADSAFE
                    /* unlock - do not check return - let user know about 
                       previous error */
                    PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                    return retVal;
                }

                /* non HDF_EOS product output .met files will contains the same full path/name 
                   as the non HDF_EOS product input files */
                  
	        strcpy(asciiFileName,asciiFileRef);	
                strcat(asciiFileName,".met");
                fileHandle = fopen(asciiFileName, "w"); 
                

           	if(fileHandle == NULL)
           	{
                	sprintf(fileIdStr, "%d", hdfFileId);
                	errInserts[0] = "output" ;
                	errInserts[1] = fileIdStr;
 
                	/* error message is:
                	"Unable to open <input> file with file id <aggregate name>" */
 
                	(void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, funcName,
                                    2, errInserts);
			tempNode = RemoveAggregate(tempNode);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - let user know about 
                           previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                	return(PGSMET_E_OPEN_ERR);
           	}

                writelbl_stat = WriteLabel(fileHandle, tempNode);

		if(writelbl_stat == -1 )
		  {
		    sprintf(dynamicMsg,"Unexpected failure in ODL WriteLabel (an attribute lengeth > ODLMAXSTMT)");
		    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		    return PGS_E_TOOLKIT;
		  }

                tempNode = RemoveAggregate(tempNode);
		if (fclose(fileHandle)  != 0)
		{
		    sprintf(dynamicMsg, "Unexpected failure closing file (%s).", 
		    asciiFileName);
 	    
		    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		    return PGS_E_TOOLKIT;
	
		}
		else
		{
		  /* use asciiFileName to write INVENTORY metadata into 
		     XML and hdf files */
		  
		  xmlretVal = PGS_PC_GetConfigData(PGSd_MET_LOGICAL_XML, XML_config_buf);
		  if(xmlretVal == PGS_S_SUCCESS) /* found config flag in the PCF */
		    {
		      xml_write_flag = atoi(XML_config_buf);
		      if(xml_write_flag == 1) /* create .xml file */
			{
			  xmlretVal = PGS_MET_ODLToXML(asciiFileName, xmlFileName);
			  if (xmlretVal != PGS_S_SUCCESS)
			    {
			      sprintf(dynamicMsg, "Unexpected failure creating XML file for (%s).", 
				      asciiFileName);
			      
			      PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
			      return PGS_E_TOOLKIT;   
			    }
			}
		    }

		  return(PGS_S_SUCCESS);
		}


#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
#ifndef PGS_DAAC
	OLD_CODE:
                if (tempReferenceflag == PGS_FALSE)
                {
         		if(hdfFileId != (PGSt_integer)NULL)
         		{
			    retVal = PGS_IO_Gen_Open(hdfFileId, PGSd_IO_Gen_Write, &fileHandle, 1);
	        	}
	        	else
	        	{
	         	    retVal = PGS_IO_Gen_Open(PGSd_MET_ASCII_DUMP, PGSd_IO_Gen_Write, &fileHandle, 1);
		        }
		}
                else
                {
                        if(hdfFileId != (PGSt_integer)NULL)
                        {
			    retVal = PGS_IO_Gen_Temp_Open(file_duration, hdfFileId, PGSd_IO_Gen_Write, &fileHandle);
                        }
                        else
                        {
                            retVal = PGS_IO_Gen_Open(PGSd_MET_ASCII_DUMP, PGSd_IO_Gen_Write, &fileHandle, 1);
                        }
                }
           	if(retVal != PGS_S_SUCCESS)
           	{
                	sprintf(fileIdStr, "%d", PGSd_MET_ASCII_DUMP);
                	errInserts[0] = "output" ;
                	errInserts[1] = fileIdStr;
 
                	/* error message is:
                	"Unable to open <input> file with file id <aggregate name>" */
 
                	(void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, funcName,
                                    2, errInserts);
			tempNode = RemoveAggregate(tempNode);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - let user know about 
                           previous error */
                        retTSF = PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                	return(PGSMET_E_OPEN_ERR);
           	}

		/* write a warning message to the log to let the user know that
		   an obsolete method of writing ASCII metadata is being used */

                /* this next open brace is only there to create the new 
                   message variable and call PGS_SMF_GenerateStatusReport(). */
                {
		char strPtr[] =
		  "\n"
		  "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
		  "!!                   W A R N I N G                     !!\n"
		  "!! Using  obsolete method of writing ASCII metadata.   !!\n"
		  "!! A direct reference to an ASCII metadata file should !!\n"
		  "!! no longer be used in the PCF.  Instead, replace the !!\n"
		  "!! logical ID of the ASCII metadata file in the PCF    !!\n"
		  "!! with a runtime parameter using the same logical ID  !!\n"
		  "!! and having a value of <logical_id>:<version_number> !!\n"
		  "!! where <logical_id> and <version_number> are the     !!\n"
		  "!! logical ID and version number (respectively) of the !!\n"
		  "!! non-HDF output product for which this ASCII         !!\n"
		  "!! metadata is being written out.                      !!\n"
		  "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

		PGS_SMF_GenerateStatusReport(strPtr);
                }  /* close brace - only opened for new variable */

                writelbl_stat = WriteLabel(fileHandle, tempNode);

		if(writelbl_stat == -1 )
		  {
		    sprintf(dynamicMsg,"Unexpected failure in ODL WriteLabel (an attribute lengeth > ODLMAXSTMT)");
		    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		    return PGS_E_TOOLKIT;
		  }

		tempNode = RemoveAggregate(tempNode);
		closestatus = PGS_IO_Gen_Close(fileHandle);
		if(closestatus != PGS_S_SUCCESS)
		{
		    sprintf(fileIdStr, "%d", PGSd_MET_ASCII_DUMP);
		    sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
		    fileIdStr);
 	    
		    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		    return PGS_E_TOOLKIT;
		}
		else
		{

		  /* use asciiFileName to write INVENTORY metadata into 
		     XML and hdf files */
		  
		  xmlretVal = PGS_PC_GetConfigData(PGSd_MET_LOGICAL_XML, XML_config_buf);
		  if(xmlretVal == PGS_S_SUCCESS) /* found config flag in the PCF */
		    {
		      xml_write_flag = atoi(XML_config_buf);
		      if(xml_write_flag == 1) /* create .xml file */
			{
			  /* get the asciiFileName */

			  Fileversion = 1;
			  if(hdfFileId != (PGSt_integer)NULL)
			    {
			      retVal = PGS_PC_GetReference(hdfFileId,&Fileversion,referenceID);
			    }
			  else
			    {
			      retVal = PGS_PC_GetReference(PGSd_MET_ASCII_DUMP,&Fileversion,referenceID);
			    }

			  xmlretVal = PGS_MET_ODLToXML(referenceID, xmlFileName);
			  if (xmlretVal != PGS_S_SUCCESS)
			    {
			      sprintf(dynamicMsg, "Unexpected failure creating XML file for (%s).", 
				      asciiFileName);
			      
			      PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
			      return PGS_E_TOOLKIT;   
			    }
		      }
		  }

		    return(PGS_S_SUCCESS);
		}
		
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
#endif
        }
	/* find out the hdf file name for later use */

#ifdef _PGS_THREADSAFE
        /* calling COTS (HDF) - lock it */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
            return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

        if(hdfFileId != (PGSt_integer)NULL)
        {
	  /* If the hdfFileId belongs to a hdf5 file, then user must have used
	     PGS_MET_SDstart() to get hdfFileId. In that case the hdfFileId
	     and the corresponding file name exist in the cat_HDF5 structure.
	     Otherwise, the hdfFileId is an ID for hdf4 file. So first we try
	     to get the file name for hdfFileId from cat_HDF5, if we still have
	     hdfRet = FAIL after the search, we try hdf4 calls to detemine
	     file name. If this fails, we issue error message. */
	     
            hdfRet = FAIL;

            for (filecount =0; filecount < PGSd_MET_MAX_NUM_FILES; filecount++)
            {
                if((cat_HDF5[filecount].file_id) == hdfFileId)
                {
		  hFileName = cat_HDF5[filecount].file_name;
		  hdfRet = SUCCEED;
		  FILE_IS_HDF5 = 1; /* file is HDF5 type */
		  FILE_IS_HDF4 = 0; /* file is not HDF4 type */
		  break;
                    
                }
                else
                {
                    continue;
                }
            }
	    if( hdfRet == FAIL)
	      {
#ifdef HDF4_NETCDF_HAVE_SD
		/* if HDF4 is installed with --disable-netcdf */
                hdfhandle = sd_NC_check_id((hdfFileId >> 20) & 0xfff);
#else
                hdfhandle = NC_check_id((hdfFileId >> 20) & 0xfff);
#endif
		hdfRet = FAIL;
		if(hdfhandle != NULL)
		{
                	hdfRet = Hfidinquire(hdfhandle->hdf_file, &hFileName, &access, &attach);
		}
		if(hdfRet == SUCCEED)
		  {
		    FILE_IS_HDF5 = 0; /* file is not HDF5 type */
		    FILE_IS_HDF4 = 1; /* file is HDF4 type */
		  }
		if(hdfRet != SUCCEED)
                {
                        /* error message is:
                        "Unable to obtain hdf filename" */

                        (void) PGS_MET_ErrorMsg(PGSMET_E_HDFFILENAME_ERR, funcName,
                                   0, errInserts);
                        tempNode = RemoveAggregate(tempNode);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - let user know about 
                           previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                        return(PGSMET_E_HDFFILENAME_ERR);
                }
	      }
        }

        /* prepare the signature */

        if(hFileName != NULL && hdfAttrName != NULL)
        {
                sprintf(signature,"%s/%s", hFileName, hdfAttrName);
        }

	/* open temporary file and write the group node to it */
	/*retVal = PGS_IO_Gen_Open(PGSd_MCF_TEMP_FILE,
	  PGSd_IO_Gen_Write, &fileHandle, 1);*/

	/*===== creating unique temporary filename =====*/

	/* Instead of opening PGSd_MCF_TEMP_FILE with PGS_IO_Gen_Open
	   we will get the file name for PGSd_MCF_TEMP_FILE from the
	   PCF file first. Then we will extend the file name with 
	   ProcessId and a number obtained from the hdf filename. This 
	   hopefully will create a unique temporary file name
	   such that if the same PCF is used in another process, the temporary
	   files will have different names, and the chance of collision
	   between the two processes will be slim. 
	*/
	tempfileversion = 1;
	retVal = PGS_PC_GetReference(PGSd_MCF_TEMP_FILE, &tempfileversion,
				     tempfilename);
        if(retVal != PGS_S_SUCCESS)
        {
                errInserts[0] = "temporary filename";

                /* error message is:
                "Unable to obtain <temporary filename> from the PC table" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_PCREAD_ERR, funcName,
                                    1, errInserts);
		tempNode = RemoveAggregate(tempNode);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_PCREAD_ERR);
        }

	/* get a number corresponding to the sum of the numbers 
	   represnting characheters in the hdf filename */

	len_filename = strlen(hFileName);
	sum_chars = 0;
	for(len_count=0; len_count<len_filename; len_count++)
	  {
	    sum_chars = sum_chars + (PGSt_uinteger)hFileName[len_count];
	  }

	/*get Process ID */

	sprintf(pid_string, "%u", getpid());

	/*extend temporary filename in the PCF with pid_string and sum_chars*/

	if(hdfFileId == (PGSt_integer)NULL)
	  {
	    sprintf(TempFilename,"%s_%s",tempfilename, pid_string);
	  }
	else
	  {
	    sprintf(TempFilename,"%s_%s_%u",tempfilename, pid_string, sum_chars);
	  }

	fileHandle = fopen(TempFilename, "w");
	if(fileHandle == NULL)
	  {
	    retVal = PGSMET_E_OPEN_ERR;
	  }
	else
	  {
	    retVal = PGS_S_SUCCESS;
	  }

	/*===== End of creating unique temporary filename =====*/

        if(retVal != PGS_S_SUCCESS)
        {
                sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
                errInserts[0] = "temporary";
                errInserts[1] = fileIdStr;
                /* error message is:
                "Unable to open <temporary> file with file id <fileId>" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, funcName,
                                    2, errInserts);
		tempNode = RemoveAggregate(tempNode);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_OPEN_ERR);
        }

	/* write the label to a temporary file. This is necessary to convert
	 * odl memory representation into an odl ASCII representation
	 */

	(void) rewind((FILE *)fileHandle);
	oldParentNode = NextGroup(tempNode);
	if(oldParentNode!= NULL)
	{
		newNode = oldParentNode->right_sibling;
	}
	else
	{
		newNode = NULL;
	}
	if(oldParentNode != NULL)
	{
		/* remove unnecessary groups */
		do
		{
			if(strcmp(groupNode->name, oldParentNode->name) != 0)
			{
				oldParentNode = RemoveAggregate(oldParentNode);
			}
			oldParentNode = newNode;
			if(newNode != NULL)
			{
				newNode = newNode->right_sibling;
			}
		}
		while(oldParentNode != NULL);
	}

	/* write out the inventory matadata as a separate ascii file */
	if(strcmp(locMdHandle, PGSd_MET_INVENT_DATA) == 0)
        {
        /*        hFileId = SDidtofid(hdfFileId); */

	  /* If the hdfFileId belongs to a hdf5 file, then user must have used
	     PGS_MET_SDstart() to get hdfFileId. In that case the hdfFileId
	     and the corresponding file name exist in the cat_HDF5 structure.
	     Otherwise, the hdfFileId is an ID for hdf4 file. So first we try
	     to get the file name for hdfFileId from cat_HDF5, if we still have
	     hdfRet = FAIL after the search, we try hdf4 calls to detemine
	     file name. If this fails, we issue error message. */
	     
            hdfRet = FAIL;
            for (filecount =0; filecount < PGSd_MET_MAX_NUM_FILES; filecount++)
            {
                if((cat_HDF5[filecount].file_id) == hdfFileId)
                {
		  hFileName = cat_HDF5[filecount].file_name;
		  hdfRet = SUCCEED;
		  FILE_IS_HDF5 = 1; /* file is HDF5 type */
		  FILE_IS_HDF4 = 0; /* file is not HDF4 type */
		  break;  
                }
                else
                {
                    continue;
                }
            }
	    if( hdfRet == FAIL)
	      {
#ifdef HDF4_NETCDF_HAVE_SD
		/* if HDF4 is installed with --disable-netcdf */
		hdfhandle = sd_NC_check_id((hdfFileId >> 20) & 0xfff);
#else
                hdfhandle = NC_check_id((hdfFileId >> 20) & 0xfff);
#endif
                hdfRet = Hfidinquire(hdfhandle->hdf_file, &hFileName, &access, &attach);
		if(hdfRet == SUCCEED)
		  {
		    FILE_IS_HDF5 = 0; /* file is not HDF5 type */
		    FILE_IS_HDF4 = 1; /* file is HDF4 type */
		  }
                if(hdfRet != SUCCEED)
                {
                        /* error message is:
                        "Unable to obtain hdf filename" */

                        (void) PGS_MET_ErrorMsg(PGSMET_E_HDFFILENAME_ERR, funcName,
                                   0, errInserts);
                        tempNode = RemoveAggregate(tempNode);
			closestatus = PGS_IO_Gen_Close(fileHandle);
			if(closestatus != PGS_S_SUCCESS)
			{
			    sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
			    sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
				    fileIdStr);
			    
			    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
			    return PGS_E_TOOLKIT;
			}
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - let user know about 
                           previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                        return(PGSMET_E_HDFFILENAME_ERR);
                }
	      }

                /* HDF_EOS product output .met files will contains the same full path/name
                   as the HDF_EOS product input files */

                sprintf(asciiFileName, "%s.met", hFileName);
                asciiFile = fopen(asciiFileName, "w");
                if(asciiFile == NULL)
                {
                        /* error message is:
                        "Unable to open MET ascii file" */

                        (void) PGS_MET_ErrorMsg(PGSMET_E_MET_ASCII_ERR, funcName,
                                   0, errInserts);
                        tempNode = RemoveAggregate(tempNode);
                        if (fclose(asciiFile)  != 0)
			{
			    sprintf(dynamicMsg, "Unexpected failure closing file (%s).", 
				    asciiFileName);
			    
			    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
			    return PGS_E_TOOLKIT;   
			}
			closestatus = PGS_IO_Gen_Close(fileHandle);
			if(closestatus != PGS_S_SUCCESS)
			{
			    sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
			    sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
				    fileIdStr);
			    
			    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
			    return PGS_E_TOOLKIT;
			}
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - let user know about 
                           previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                        return(PGSMET_E_MET_ASCII_ERR);
                }
                writelbl_stat = WriteLabel(asciiFile, tempNode);

		if(writelbl_stat == -1 )
		  {
		    sprintf(dynamicMsg,"Unexpected failure in ODL WriteLabel (an attribute lengeth > ODLMAXSTMT)");
		    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		    return PGS_E_TOOLKIT;
		  }

		if (fclose(asciiFile)  != 0)
		  {
		    sprintf(dynamicMsg, "Unexpected failure closing file (%s).", 
			    asciiFileName);
		    
		    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		    return PGS_E_TOOLKIT;   
		  }

		/* use asciiFileName to write INVENTORY metadata into 
		   XML and hdf files */

		xmlretVal = PGS_PC_GetConfigData(PGSd_MET_LOGICAL_XML, XML_config_buf);
		if(xmlretVal == PGS_S_SUCCESS) /* found config flag in the PCF */
		  {
		    xml_write_flag = atoi(XML_config_buf);
		    if(xml_write_flag == 1) /* create .xml file */
		      {
			xmlretVal = PGS_MET_ODLToXML(asciiFileName, xmlFileName);
			if (xmlretVal != PGS_S_SUCCESS)
			  {
			    sprintf(dynamicMsg, "Unexpected failure creating XML file for (%s).", 
				    asciiFileName);
			    
			      PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
			      return PGS_E_TOOLKIT;   
			  }

			/* use xmlFileName to write XML metadata into HDF file */

			xmlretVal = PGS_MET_WriteXML(xmlFileName, "xmlmetadata", hdfFileId); 
			if (xmlretVal != PGS_S_SUCCESS)
			  {
			    sprintf(dynamicMsg, "Unexpected failure writing xml metadata into HDF file.");
			    
			      PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
			      return PGS_E_TOOLKIT;   
			  }
		      }
		  }
        }

	writelbl_stat = WriteLabel(fileHandle, tempNode);

	if(writelbl_stat == -1 )
	  {
	    sprintf(dynamicMsg,"Unexpected failure in ODL WriteLabel (an attribute lengeth > ODLMAXSTMT)");
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
	    return PGS_E_TOOLKIT;
	  }

	tempNode = RemoveAggregate(tempNode);
	closestatus = PGS_IO_Gen_Close(fileHandle);

	if(closestatus != PGS_S_SUCCESS)
	{
	    sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
	    sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
		    fileIdStr);
	    
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
	    return PGS_E_TOOLKIT;
	}
#ifdef _PGS_THREADSAFE
        /* unlock - do not check return - let user know about previous error */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

	/* open the file again for reading */

	/*retVal = PGS_IO_Gen_Open(PGSd_MCF_TEMP_FILE,
	  PGSd_IO_Gen_Read, &fileHandle, 1);*/
	/* Open the unique temporary file created before instead of using
	   PGS_IO_Gen_Open
	*/
	fileHandle = fopen(TempFilename, "r");

	if(fileHandle == NULL)
	  {
	    retVal = PGSMET_E_OPEN_ERR;
	  }
	else
	  {
	    retVal = PGS_S_SUCCESS;
	  }

        if(retVal != PGS_S_SUCCESS)
        {
                sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
                errInserts[0] = "temporary";
                errInserts[1] = fileIdStr;
                /* error message is:
                "Unable to open <temporary> file with file id <aggregate name>" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, funcName,
                                    2, errInserts);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_OPEN_ERR);
        }

	/* read single characters from the file into a buffer */

	if(newAttrName != NULL)
	  {
	    PGS_MEM_Free(newAttrName);
	    newAttrName = NULL;
	  }

       retVal = PGS_MEM_Malloc((void **)&newAttrName, (PGSt_integer)(strlen(hdfAttrName) + 11));
        if(retVal != PGS_S_SUCCESS)
        {
                /* error message is:
                "Unable to allocate memory for the hdf attribute" */
 
                (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
                                    0, errInserts);
		closestatus = PGS_IO_Gen_Close(fileHandle);
		if(closestatus != PGS_S_SUCCESS)
		{
		    sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
		    sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
			    fileIdStr);
		    
		    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		    return PGS_E_TOOLKIT;
		}
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_MALLOC_ERR);
        }

	if(mcfBuffer !=NULL)
	  {
	    PGS_MEM_Free(mcfBuffer);
	    mcfBuffer = NULL;
	  }

       retVal = PGS_MEM_Malloc((void **)&mcfBuffer, (PGSt_integer)(MAX_ORDER + 1));
        if(retVal != PGS_S_SUCCESS)
        {
                /* error message is:
                "Unable to allocate memory for the hdf attribute" */
 
                (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
                                    0, errInserts);
		(void) PGS_MEM_Free(newAttrName);
		newAttrName = (char *) NULL;
		closestatus = PGS_IO_Gen_Close(fileHandle);
		if(closestatus != PGS_S_SUCCESS)
		{
		    sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
		    sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
			    fileIdStr);
		    
		    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		    return PGS_E_TOOLKIT;
		}
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_MALLOC_ERR);
        }
       numAttr = 0;
        while(feof(fileHandle) == 0)
        {

	  numBytesRead = fread(mcfBuffer, sizeof(char), (size_t)MAX_ORDER, fileHandle);
	  if(numBytesRead != 0) /* if numBytesRead is equal to MAX_ORDER */
	    {
	      numAttr++;
	    }
        }
	(void) rewind(fileHandle);

	/* if file type is HDF5 open (or create group "HDFEOS INFORMATION" */
	if( FILE_IS_HDF5 == 1 &&  FILE_IS_HDF4 == 0)
	  {
	    HDFfid =(hid_t) hdfFileId;
	    hdfRetVal = SUCCEED;
	    
	    /* probe: open group "HDFEOS INFORMATION" */
	    status = H5Eset_auto(NULL, NULL);
	    ggid = H5Gopen(HDFfid,"HDFEOS INFORMATION");
	    
	    /* if it doesn't exist, create it */
	    if( ggid < 0 ) 
	      {
		ggid = H5Gcreate(HDFfid, "HDFEOS INFORMATION", 0);
		if (ggid == -1)
		  {
		    sprintf(dynamicMsg, "Cannot create \"HDFEOS INFORMATION\" group.");                       
		    PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, dynamicMsg , funcName);
		    closestatus = PGS_IO_Gen_Close(fileHandle);
		    if(closestatus != PGS_S_SUCCESS)
		      {
			sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
			sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
				fileIdStr);
			
			PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
			return PGS_E_TOOLKIT;
		      }
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    return(PGSMET_E_SD_SETATTR);
		    
		  }
	      }
	  }

       for (i = 0; i < numAttr; i++)
	 {
	   numBytesRead = fread(mcfBuffer, sizeof(char), (size_t) MAX_ORDER, fileHandle);

	   mcfBuffer[numBytesRead ]= '\0';

	   /* attach the buffer to the HDF attribute */
	   if(i == 0)
	     {
	       sprintf(newAttrName, "%s", hdfAttrName);
	     }
	   else
	     {
	       sprintf(newAttrName, "%s.%d", hdfAttrName, i);
	     }
	   /* if hdf file is HDF4 type use SDsetattr to set attributes */
	   if(FILE_IS_HDF4 == 1 &&  FILE_IS_HDF5 == 0)
	     {
	       hdfRetVal = SDsetattr((int32)hdfFileId, newAttrName,
				 DFNT_CHAR, (int32) (numBytesRead), mcfBuffer);

	       if(hdfRetVal != SUCCEED)
		 {
		 /*
		   (void) HEprint(stderr, (int32) 0);
		 */
		 /* error message is:
		    "Unable to set the HDF file attribute" */
		 
		   (void) PGS_MET_ErrorMsg(PGSMET_E_SD_SETATTR, funcName,
					 0, errInserts);
		   (void) PGS_MEM_Free(mcfBuffer);
		   mcfBuffer = (char *) NULL;
		   (void) PGS_MEM_Free(newAttrName);
		   newAttrName = (char *) NULL;
		   closestatus = PGS_IO_Gen_Close(fileHandle);
		   if(closestatus != PGS_S_SUCCESS)
		     {
		       sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
		       sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
			       fileIdStr);
		       
		       PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		       return PGS_E_TOOLKIT;
		     }
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		   return(PGSMET_E_SD_SETATTR);
		 }
	     }
	   /* if hdf  file is HDF5 type use HDF5 scheme to set attributes */
	   else if(FILE_IS_HDF5 == 1 &&  FILE_IS_HDF4 == 0)
	     {
              
	       /* 
		  -------------------------------------------------------
		  |Create dataset  "coremetadata, coremetadata.0, etc   |
                  |and attach to the "HDFEOS INFORMATION"  group        | 
		  -------------------------------------------------------
	       */      
	       
	       sid2    = H5Screate(H5S_SCALAR);
	       if (sid2 == -1)
		 {
		    sprintf(dynamicMsg, "Cannot create dataspace for %s dataset.",newAttrName);                      
		    PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, dynamicMsg , funcName);
		   hdfRetVal = FAIL;
		 }
	       
	       if(hdfRetVal == SUCCEED)
		 {
		   atype   = H5Tcopy(H5T_C_S1);
		   size = (size_t)MAX_ORDER;
		   status  = H5Tset_size(atype,size);
		   if (status == -1)
		     {
		       sprintf(dynamicMsg, "Cannot set the total size for atomic datatype.");                       
		       PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, dynamicMsg , funcName);

		       hdfRetVal = FAIL;
		     }
		 }
	       if(hdfRetVal == SUCCEED)
		 {
		   mcfBuffer[ numBytesRead ] = '\0';
		   datid = H5Dopen(ggid, newAttrName);
		   if(datid == -1)
		     {
		       datid   = H5Dcreate(ggid,newAttrName,atype,sid2,H5P_DEFAULT);
		       if (datid == -1)
			 {
			   sprintf(dynamicMsg, "Cannot create %s dataset.",newAttrName);
			   PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, dynamicMsg, funcName);
			   hdfRetVal = FAIL;
			 }
		     }
		 }
           
	       /* 
		  -----------------------------------------------------
		  |    Write data to the dataset newAttrName          | 
		  -----------------------------------------------------
	       */ 
	       if(hdfRetVal == SUCCEED)
		 {
		   retVal = PGS_MEM_Calloc((void **)&mcfBuffer_temp, 
					   (PGSt_integer)(MAX_ORDER + 1), sizeof(char));
		   if(retVal != PGS_S_SUCCESS)
		     {
		       /* error message is:
			  "Unable to allocate memory for the hdf attribute" */
		       (void) PGS_MEM_Free(mcfBuffer);
		       mcfBuffer = (char *) NULL;
		       (void) PGS_MEM_Free(newAttrName);
		       newAttrName = (char *) NULL;
		       (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
					       0, errInserts);
		       closestatus = PGS_IO_Gen_Close(fileHandle);
		       if(closestatus != PGS_S_SUCCESS)
			 {
			   sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
			   sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
				   fileIdStr);
			   
			   PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
			   return PGS_E_TOOLKIT;
			 }
#ifdef _PGS_THREADSAFE
		       /* unlock - do not check return - let user know about 
			  previous error */
		       PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		       return(PGSMET_E_MALLOC_ERR);
		     }
		   strcpy(mcfBuffer_temp, mcfBuffer);
		   status = H5Dwrite(datid,atype,H5S_ALL,H5S_ALL,H5P_DEFAULT,mcfBuffer_temp);
		   (void) PGS_MEM_Free(mcfBuffer_temp);
		   mcfBuffer_temp = (char *) NULL;
		   if (status == -1)
		     {
		       sprintf(dynamicMsg, "Cannot write in data to the %s dataset.",newAttrName); 
		       PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, dynamicMsg, funcName);
		       hdfRetVal = FAIL;
		     }
		 }
           

	       /* 
		  ----------------------------------------------------
		  |     Release (close) the created objects          |
		  ----------------------------------------------------
	       */
	       if(hdfRetVal == SUCCEED)
		 {
		   status = H5Sclose(sid2);
		   if (status == -1)
		     {
		       sprintf(dynamicMsg, "Cannot release the dataspace ID.");
		       PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, dynamicMsg, funcName);
		       hdfRetVal = FAIL;
		     }
		 }
	       if(hdfRetVal == SUCCEED)
		 {
		   status = H5Dclose(datid);
		   if (status == -1)
		     {
		       sprintf(dynamicMsg, "Cannot release the dataset ID.");
		       PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, dynamicMsg, funcName);
		       hdfRetVal = FAIL;
		     }
		 }
	       
	       if(hdfRetVal != SUCCEED)
		 {
		   /* error message is:
		      "Unable to set the HDF file attribute" */
		   
		   (void) PGS_MET_ErrorMsg(PGSMET_E_SD_SETATTR, funcName,
					   0, errInserts);
		   (void) PGS_MEM_Free(mcfBuffer);
		   mcfBuffer = (char *) NULL;
		   (void) PGS_MEM_Free(newAttrName);
		   newAttrName = (char *) NULL;
		   closestatus = PGS_IO_Gen_Close(fileHandle);
		   if(closestatus != PGS_S_SUCCESS)
		     {
		       sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
		       sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
			       fileIdStr);
		       
		       PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
		       return PGS_E_TOOLKIT;
		     }
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		   return(PGSMET_E_SD_SETATTR);
		 }
	     }
	 }
#ifdef _PGS_THREADSAFE
       /* unlock - do not check return - let user know about previous error */
       PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

       /* close group "HDFEOS INFORMATION" if file is HDF5 type*/
       if(FILE_IS_HDF5 == 1 &&  FILE_IS_HDF4 == 0)
	 {
	   status = H5Gclose(ggid);
	 
	   if (status == -1)
	     {
	       sprintf(dynamicMsg, "Cannot release the group ID.");
	       PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, dynamicMsg, funcName);
	       (void) PGS_MEM_Free(mcfBuffer);
	       mcfBuffer = (char *) NULL;
	       (void) PGS_MEM_Free(newAttrName);
	       newAttrName = (char *) NULL;
	       return(PGSMET_E_SD_SETATTR);
	     }
	 }

       closestatus = PGS_IO_Gen_Close(fileHandle);
       if(closestatus != PGS_S_SUCCESS)
       {
	   sprintf(fileIdStr, "%d", PGSd_MCF_TEMP_FILE);
	   sprintf(dynamicMsg, "Unexpected failure closing file with ID (%s).", 
		   fileIdStr);
	   
	   PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
	   (void) PGS_MEM_Free(mcfBuffer);
	   mcfBuffer = (char *) NULL;
	   (void) PGS_MEM_Free(newAttrName);
	   newAttrName = (char *) NULL;
	   return PGS_E_TOOLKIT;
       }

       /* Remove temporay file created */
       /* (void) PGS_IO_Gen_Temp_Delete(PGSd_MCF_TEMP_FILE); */
       sprintf(cmd,"/bin/rm -f %s",TempFilename);
       system(cmd);

       (void) PGS_MEM_Free(mcfBuffer);
        mcfBuffer = (char *) NULL;
       (void) PGS_MEM_Free(newAttrName);
        newAttrName = (char *) NULL;
	/* check that this particular attribute is not in the GetPCAttr buffers */

	bufferIndex = 0;
	if(hFileName != NULL && hdfAttrName != NULL)
        {
#ifdef _PGS_THREADSAFE
               /* This block is separated because the variable name is now
                  a pointer */
               while(*(PGSg_MET_AttrHandle+(bufferIndex*PGSd_MET_SIGNATURE_L)) 
                          != '\0' && bufferIndex < PGSd_MET_NUM_OF_GROUPS)
        	{
                        fileInBuffer = strcmp(signature, 
                                PGSg_MET_AttrHandle+
                                      (bufferIndex*PGSd_MET_SIGNATURE_L));
                        if(fileInBuffer == 0)
                        {
				strcpy(PGSg_MET_AttrHandle+
                                    (bufferIndex*PGSd_MET_SIGNATURE_L), "alward");
                                pthread_setspecific(
                                  masterTSF->keyArray[PGSd_TSF_KEYMETATTRHANDLE],
                                  PGSg_MET_AttrHandle);
                                break;
                        }
                        bufferIndex++;
		}
#else  /* -D_PGS_THREADSAFE */
		while(PGSg_MET_AttrHandle[bufferIndex][0] != '\0' && bufferIndex < PGSd_MET_NUM_OF_GROUPS)
        	{
	
                        fileInBuffer = strcmp(signature, PGSg_MET_AttrHandle[bufferIndex]);
                        if(fileInBuffer == 0)
                        {
				strcpy(PGSg_MET_AttrHandle[bufferIndex], "alward");
                                break;
                        }
                        bufferIndex++;
		}
#endif  /* -D_PGS_THREADSAFE */
        }

       if(checkRetVal != PGS_S_SUCCESS && checkRetVal != PGSMET_E_ODL_MEM_ALLOC && checkRetVal != PGSMET_W_META_NOT_SET)
        {
                (void) PGS_MET_ErrorMsg(checkRetVal,
                                     funcName, 0, errInserts);
                return(checkRetVal);
        }

	return(PGS_S_SUCCESS);
}
