/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
         PGS_MET_ConvertToMCF.c 
 
DESCRIPTION:
         The file contains PGS_MET_ConvertToMCF 
         This function is used to convert HDF-EOS/non HDF-EOS Metadata files 
         to a file that is similiar to the metadata configuration file (MCF),
         this MCF will be read into the memory to provide a basis for setting 
         and checking metadata parameter values when function PGS_MET_Init()
         is called and also be used to set attribute value using the function
         PGS_MET_SetAttr() in the metadata updating

AUTHOR:
        Alward N.Siyyid / EOSL
        Carol S. W. Tsai / Space Applications Corporation
	Abe Taaheri / Emergent Information Technologies, Inc.
 
HISTORY:
        
        03-Oct-1997 ANS/CSWT   Initial version
        13-Oct-1997 CSWT       Added code to remove the VALUE parameter from 
                               the aggregate Node if attached value node  
                               containning any one of the following strings,
                               "NOT SET:STRING", "NOT SET:INTEGER", "NOT SET:DOUBLE", 
                               "NOT SET:DATETIME", "NOT SET:DATE", "NOT SET:TIME",      
                               "NOT SUPPLIED:STRING", "NOT SUPPLIED:INTEGER", "NOT 
                               SUPPLIED:DOUBLE", "NOT SUPPLOIED:DATETIME", "NOT 
                               SUPPLIED:DATE", "NOT SUPPLIED:TIME", "NOT FOUND:      
                               STRING", "NOT FOUND:INTEGER", "NOT FOUND:DOUBLE",
                               "NOT FOUND:DATETIME", "NOT FOUND:DATE", or "NOT
                               FOUND:TIME" that was written out on the HDF-EOS/  
                               non HDF-EOS meatadata file when attribute has not  
                               been set up with the proper value. And a TYPE 
                               parameter including the attached value node that
                               contains  any one of the following type of data,
                               "INTEGER", "STRING", "DOUBLE", "DATETIME", "DATE",
                               or "TIME", will be attached to this aggregate
                               Node
	07-Jul-1999 RM         Updated for TSF functionality
        05-Mar-1998 CSWT       Modified code in order to check there is a "TYPE" 
                               parameter already existing in the input data 
                               file or not and also added code to insert the 
                               aggregate Node "GROUPTYPE" into the ODL tree if 
                               it is not existing in the data file
        30-Mar-2001 AT         Modified for HDF5 support 
 
END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/
 
#include "PGS_MET.h"
#include "PGS_CUC.h"
#include "PGS_TSF.h"
 
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

/* hdf files */
 
#ifdef DEC_ALPHA        /* DEC Alpha platform */
#include <rpc/types.h>  /* this avoids typedef conflict with int32, uint32 */
#endif /* DEC_ALPHA */
#include <hdf.h>
#include <mfhdf.h>
#include <hdf5.h>

/***************************************************************************
BEGIN_PROLOG:
 
 
TITLE:
        This function is used to convert HDF-EOS/non HDF-EOS Metadata files
        to a file that is similiar to the metadata configuration file (MCF) 
NAME:
        PGS_MET_ConvertToMCF() 
 
SYNOPSIS:
        N/A
 
DESCRIPTION:
        This function is used to convert HDF-EOS/non HDF-EOS Metadata files
        to a file that is similiar to the metadata configuration file (MCF)
 
INPUTS:
        Name            Description             Units   Min     Max
        ----            -----------             -----   ---     ---
        fileId          product file id         none            variable     variable
        hdfAttrName     name of HDF attribute   none            N/A          N/A
                        containing metadata
OUTPUTS:
        None
 
RETURNS:
        PGS_S_SUCCESS
        PGSMET_E_SD_START       Unable to open the HDF file
        PGSMET_E_SD_FINDATTR    Unable to get the attr index
        PGSMET_E_SD_INFO        Unable to retrieve sd attribute information
        PGSMET_E_MALLOC_ERR     Unable to allocate memory for the hdf attribute
        PGSMET_E_SD_READ        Unable to read hdf attribute
        PGSMET_E_FILETOODL_ERR  Unable to convert <filename> into odl format
        PGSMET_E_CONVERT_ERR    Unable to convert HDF-EOS/non HDF-EOS metadata product 
                                file to a .MCF file
	PGSTSF_E_GENERAL_FAILURE problem in TSF code
 
EXAMPLES:
        N/A
 
NOTES:

       *******IMPORTANT*********

       In order to define the proper data type in the TYPE parameter for those  
       unset attribute values, that were written out on the HDF-EOS/non HDF-EOS 
       metadat file with the string "NOT SET" for Data Locataion is PGE, "NOT
       SUPPLIED" for Data Locataion is MCF, and "NOT FOUND" for Data Location is 
       NONE no matter which data type it belongs to, when calling this function to
       convert a HDF-EOS/non HDF-EOS metadata file to be a .MCF that will be  
       similiar to a Metadata Configuration file to be used to initialize the 
       memory for the contents of the MCF to provide a basis for setting and 
       checking metadata parameter values, the different string should be defined 
       for different type of data for unset attribute. The function PGS_MET_CheckAttr()
       that was designed to be used by the function PGS_MET_Write() to check that
       all the mandatory metadata is set prior to being written to the product file 
       become necessary to make this kind of modification for unset attributes; 
       therefore any product file that was generated before this function created can 
       not use this function to convrt to a .MCF file
 
REQUIREMENTS:
        N/A
 
DETAILS:
        NONE
 
GLOBALS:
        NONE
 
FILES:
        NONE
 
FUNCTIONS_CALLED:
        PGS_MET_ErrorMsg
        PGS_IO_Gen_Close
        PGS_IO_Gen_Temp_Delete
        SDfindattr
        SDfileinfo
        SDselect
        SDattrinfo
        PGS_MEM_Malloc
        SDreadattr
        PGS_MET_ConvertToOdl
        PGS_MEM_Free
        ReadLable
        NewAggregate
        NextObject
        FindParameter
        FirstValue
        CopyParameter
        PasteParameter
        NextSubObject
        RemoveAggregate
        PGS_TSF_LockIt
        PGS_TSF_UnlockIt
        PGS_SMF_TestErrorLevel

END_PROLOG:
***********************************************************************/
 
PGSt_SMF_status
PGS_MET_ConvertToMCF(PGSt_PC_Logical fileId,         /* file id for the file containing
                                                      * parameter data
                                                      */ 
                     char        *hdfAttrName)       /* HDF attribute Name */
{
        PGSt_IO_Gen_FileHandle  *fileHandle = NULL;
        char                    fileName[PGSd_PC_FILE_PATH_MAX];
        char                    filename[PGSd_PC_FILE_PATH_MAX];
        char                    fileIdStr[PGSd_MET_FILE_ID_L]; /* file id value as string */
        char                    fileAttr[PGSd_PC_FILE_PATH_MAX] = "";
        char                    charstr[PGSd_PC_FILE_PATH_MAX] = "";
        char                    multiHdfName[200];
        char *                  typePtr = NULL; 
        char *                  sdAttrValue;
        char *                  funcName = "PGS_MET_ConvertToMCF";
        char *                  errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};
                                                        /* Dynamic strings inserted in
                                                         * error messages
                                                         */
        int32                   sdId = 0;
        int32                   sdsId = 0;
        int32                   hdfid = 0;
        int32                   attrIndex = 0;
        int32                   nDataSets = 0;
        int32                   nGlobalAttrs = 0;
        int32                   sdNumType = 0;
        int32                   sdCount = 0;
        AGGREGATE               aggNode = NULL;
	AGGREGATE               childNode = NULL;
	AGGREGATE               objectNode = NULL;
	AGGREGATE               tempNode = NULL;
	AGGREGATE               grouptypenode = NULL;
        PGSt_integer            type = 0;
        PGSt_integer            odlRetVal = 0;
        PGSt_integer            version = 1;   /* The number of versions associated
                                                * with the requested product Logical
                                                * ID 
                                                */
        PGSt_integer            hdfRetVal = 0;
        PGSt_integer            loopCount;
        PGSt_integer            dataSize = 0;
        PGSt_integer            attrCount = 0;
        PGSt_integer            attrEnd = 1;
        PGSt_integer            locVersion;
        PARAMETER               groupnode = NULL;
        PARAMETER               groupNode = NULL;
        PARAMETER               typeNode = NULL;
        PARAMETER               parmNode = NULL;
	PARAMETER               strNode = NULL;
        PARAMETER               intNode = NULL;
        PARAMETER               doubleNode = NULL;
        PARAMETER               datetimeNode = NULL;
        PARAMETER               dateNode = NULL;
        PARAMETER               timeNode = NULL;
        PARAMETER               locNode = NULL;
        PARAMETER               mandNode = NULL;
        VALUE                   valueNode = NULL;
        PGSt_SMF_status         retVal = PGS_S_SUCCESS; /* SDPS toolkit ret value */
	PGSt_integer            fileishdf = 0; /*if input file is hdf fileishdf
                                                = 0 */
        VALUE                   locValue = NULL;
	char                    msg[PGS_SMF_MAX_MSGBUF_SIZE];/*message buffer*/
	hid_t                   hdf5id;
	char *                  metabuf = NULL; /* Pointer (handle) to 
					   coremetadata array */
	PGSt_integer		FILE_IS_HDF4 = 0;
	PGSt_integer		FILE_IS_HDF5 = 0;
	PGSt_integer	        FILE_NOT_HDF = 0;
        hid_t           ggid;
        size_t          size;
        hid_t           atype;
	hid_t           meta_id     = FAIL;/*  metadata ID */
	hid_t           aspace      = FAIL;/* dataspace ID */
	PGSt_integer	sdId5 = 0;
	size_t          type_size;
	PGSt_integer	lenmetabuf;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retTSF;        /* lock and unlock return */
#endif

#ifndef PGS_DAAC
	/* if this is not the DAAC this function should NOT be used (it is for
	   internal ECS processes ONLY).  Note that the "if (retVal..." test is
	   to keep the compiler from complaining about unreachable
	   statements. */

	if (retVal == PGS_S_SUCCESS)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "This function is for internal ECS usage "
				  "ONLY!", "PGS_MET_ConvertToMCF()");
	    return PGS_E_TOOLKIT;
	}
#endif
        /* clear the errno for the ODL routines */
       
        if (errno == ERANGE)
        {
            errno = 0;
        }

        locVersion = version;

        /* if the return status is successful, then call PGS_PC_GetRef-
         *  erence() to retrieve the physical file reference associated
         *  with the logical ID 
         */
 
        retVal = PGS_PC_GetReference(fileId, &version, fileName);
 
        if (retVal != PGS_S_SUCCESS)
        {
            errInserts[0] = "input filename";
            if(retVal != PGS_S_SUCCESS)
            {
		/* error message is
		   "Unable to obtain <filename or attribute filename> from the PC table */
 
		(void) PGS_MET_ErrorMsg(PGSMET_E_PCREAD_ERR,
					funcName, 1, errInserts);
		return(PGSMET_E_PCREAD_ERR);
            }
        }
        else
        {
	    errInserts[0] = "attribute filename";
	    retVal = PGS_PC_GetFileAttr(fileId, locVersion, PGSd_PC_ATTRIBUTE_LOCATION,
					PGSd_PC_FILE_PATH_MAX, fileAttr);
        } 
 
	aggNode = NewAggregate(aggNode, KA_GROUP, "locAggr", "");
	if (aggNode == NULL)
	{
	    /* this means that memory allocation has failed within ODL */
	    /* error message is:
                        "ODL routine failed to allocate memory" */
 
	    (void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
				    funcName, 0, errInserts);
	    return(PGSMET_E_ODL_MEM_ALLOC);

	}

	tempNode = NewAggregate(tempNode, KA_GROUP, "temp", "GROUP");
	if (tempNode == NULL)
	{
	    /* this means that memory allocation has failed within ODL */
	    /* error message is:
                        "ODL routine failed to allocate memory" */

	    (void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
				    funcName, 0, errInserts);
	    return(PGSMET_E_ODL_MEM_ALLOC);
	}

       /* check to see whether file is hdf or non-hdf file */

	retVal = PGS_MET_HDFFileType(fileName, &FILE_IS_HDF4,
				     &FILE_IS_HDF5, &FILE_NOT_HDF);
	if( retVal == PGS_S_SUCCESS)
	  {
	    if( FILE_IS_HDF4 == 1 || FILE_IS_HDF5 == 1)
	      {
		fileishdf = 0;
	      }
	    else
	      {
		/*input file is not HDF file */
		fileishdf = -1;
	      }
	  }
	else
	  {
	    sprintf(msg, " Cannot determine whether the file (%s) is HDF4, HDF5, or NONE-HDF type. ", filename);
	    PGS_SMF_SetDynamicMsg(retVal, msg, funcName);
	    return(retVal);
	  }
	
#ifdef _PGS_THREADSAFE
        /* We are calling cots (HDF) so we need to lock */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
            return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	/*        if(strcmp(fileName,fileAttr) == 0)*/
	if(fileishdf == 0) /* new form when not using PCF file */
	{
	  if( FILE_IS_HDF4 == 1) /* file is HDF4 type */
	  {
	    sdId = SDstart(fileName, DFACC_RDONLY);
	    if( sdId == FAIL)
	    {
                /* error message is:
		   "Unable to open the HDF file" */
 
                (void) PGS_MET_ErrorMsg(PGSMET_E_SD_START, funcName,
					0, errInserts);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_SD_START);
	    }

	    hdfid = sdId;

            /* Create a temporary file to hold the data */
 
	    retVal = PGS_IO_Gen_Open((PGSt_PC_Logical) PGSd_MET_TEMP_ATTR_FILE,
				     PGSd_IO_Gen_Write, &fileHandle, 1);
	    if(retVal != PGS_S_SUCCESS)
	    {
                sprintf(fileIdStr, "%d", PGSd_MET_TEMP_ATTR_FILE);
                errInserts[0] = "temporary";
                errInserts[1] = fileIdStr;
                /* error message is:
		   "Unable to open <temporary> file with file id <aggregate name>" */
 
                (void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, funcName,
					2, errInserts);
                (void) SDend(sdId);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_OPEN_ERR);
	    }
	    (void) rewind((FILE *)fileHandle);
	    strcpy(multiHdfName, hdfAttrName);
	    do
	    {
		if(attrCount > 0)
		{
		    sprintf(multiHdfName, "%s.%d", hdfAttrName, attrCount);
		}
 
		attrIndex = SDfindattr(sdId, multiHdfName);
		if(attrIndex == FAIL)
		{

		    /* could be a local attribute */

		    hdfRetVal = SDfileinfo(sdId, &nDataSets, &nGlobalAttrs);
		    if(hdfRetVal != FAIL)
		    {
                        loopCount = 0;
                        do
                        {
			    sdsId = SDselect(sdId, loopCount);
			    if(sdsId != FAIL)
			    {
				attrIndex = SDfindattr(sdsId, multiHdfName);
			    }
			    loopCount++;
			    if(attrIndex == FAIL)
			    {
				(void) SDendaccess(sdsId);
			    }
                        }
                        while(attrIndex == FAIL && loopCount < nDataSets);
 
		    }
		    if(attrIndex == FAIL)
		    {
                        /* error message is:
			   "Unable to get the attr index" */
 
			(void) PGS_MET_ErrorMsg(PGSMET_E_SD_FINDATTR, funcName,
						0, errInserts);
                        if(sdsId != 0)
                        {
			    (void) SDendaccess(sdsId);
                        }
                        (void) SDend(sdId);
                        (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check for error since we already had 
                           an error that we want the user to know about */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                        return(PGSMET_E_SD_FINDATTR);
		    }
		    hdfid = sdsId;
 
		}
		hdfRetVal = SDattrinfo(hdfid, attrIndex, multiHdfName, &sdNumType, &sdCount);
 
		if(hdfRetVal == FAIL)
		{
		    /* error message is:
		       "Unable to retrieve sd attribute information" */
 
		    (void) PGS_MET_ErrorMsg(PGSMET_E_SD_INFO, funcName,
					    0, errInserts);
		    if(sdsId != 0)
		    {
                        (void) SDendaccess(sdsId);
		    }
		    (void) PGS_IO_Gen_Close(fileHandle);
		    (void) SDend(sdId);
#ifdef _PGS_THREADSAFE
                    /* unlock - do not check for error since we already had 
                       an error that we want the user to know about */
                    PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    return(PGSMET_E_SD_INFO);
		}
		dataSize = (PGSt_integer) ((DFKNTsize(sdNumType) * sdCount) + 1);
 
		retVal = PGS_MEM_Malloc((void **)&sdAttrValue, dataSize + 1);
 
		if(retVal != PGS_S_SUCCESS)
		{
		    /* error message is:
		       "Unable to allocate memory for the hdf attribute" */
 
		    (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
					    0, errInserts);
		    if(sdsId != 0)
		    {
                        (void) SDendaccess(sdsId);
		    }
		    (void) SDend(sdId);
		    (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                    /* unlock - do not check for error since we already had 
                       an error that we want the user to know about */
                    PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    return(PGSMET_E_MALLOC_ERR);
		}
		hdfRetVal = SDreadattr(hdfid, attrIndex, sdAttrValue);
		if(hdfRetVal != SUCCEED)
		{
		    /* error message is:
		       "Unable to read hdf attribute" */
 
		    (void) PGS_MET_ErrorMsg(PGSMET_E_SD_READ, funcName,
					    0, errInserts);
		    (void) PGS_MEM_Free(sdAttrValue);
		    sdAttrValue = (char *) NULL;
		    if(sdsId != 0)
		    {
                        (void) SDendaccess(sdsId);
		    }
		    (void) PGS_IO_Gen_Close(fileHandle);
		    (void) SDend(sdId);
#ifdef _PGS_THREADSAFE
                    /* unlock - do not check for error since we already had 
                       an error that we want the user to know about */
                    PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    return(PGSMET_E_SD_READ);
		}

                /* write the buffer to the file */
 
		fwrite(sdAttrValue, sizeof(char), (size_t)sdCount, fileHandle);
		if(sdsId != 0)
		{
		    (void) SDendaccess(sdsId);
		}
		attrCount++;
		sdAttrValue[sdCount] = '\0';
		attrEnd = strcmp(&sdAttrValue[sdCount -4], "END\n");
		(void) PGS_MEM_Free(sdAttrValue);
		sdAttrValue = (char *) NULL;
	    }
	    while(sdCount == MAX_ORDER && attrEnd != 0);
 
	    (void) PGS_IO_Gen_Close(fileHandle);
	    (void) SDend(sdId);

#ifdef _PGS_THREADSAFE
            retTSF = PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
 
	    retVal = PGS_IO_Gen_Open((PGSt_PC_Logical) PGSd_MET_TEMP_ATTR_FILE, PGSd_IO_Gen_Read, &fileHandle, 1);
	    if(retVal != PGS_S_SUCCESS)
	    {
		sprintf(fileIdStr, "%d", (PGSt_PC_Logical) PGSd_MET_TEMP_ATTR_FILE);
		errInserts[0] = "input" ;
		errInserts[1] = fileIdStr;
 
		/* error message is:
		   "Unable to open <input> file with file id <aggregate name>" */

		(void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, "PGS_MET_ConvertToMCF",
					2, errInserts);
		aggNode = RemoveAggregate(aggNode);
		return(PGSMET_E_OPEN_ERR);
	    }
	}
	  else if( FILE_IS_HDF5 == 1) /* file is HDF5 type */
	  {
	    retVal = PGS_MET_SDstart(fileName, HDF5_ACC_RDONLY, &sdId5);
	      if( retVal != PGS_S_SUCCESS)
	      {
                /* error message is:
		   "Unable to open the HDF file" */
		
                (void) PGS_MET_ErrorMsg(PGSMET_E_SD_START, funcName,
					0, errInserts);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_SD_START);
	      }

	    hdf5id = (hid_t) sdId5;

            /* Create a temporary file to hold the data */
 
	    retVal = PGS_IO_Gen_Open((PGSt_PC_Logical) PGSd_MET_TEMP_ATTR_FILE,
				     PGSd_IO_Gen_Write, &fileHandle, 1);
	    if(retVal != PGS_S_SUCCESS)
	    {
                sprintf(fileIdStr, "%d", PGSd_MET_TEMP_ATTR_FILE);
                errInserts[0] = "temporary";
                errInserts[1] = fileIdStr;
                /* error message is:
		   "Unable to open <temporary> file with file id <aggregate name>" */
 
                (void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, funcName,
					2, errInserts);
                (void) PGS_MET_SDend(sdId5);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_OPEN_ERR);
	    }
	    (void) rewind((FILE *)fileHandle);
	    strcpy(multiHdfName, hdfAttrName);

	    /* probe: open group "HDFEOS INFORMATION" */
	    (void) H5Eset_auto(NULL, NULL);
	    ggid = H5Gopen(hdf5id,"HDFEOS INFORMATION");
	       
	    /* if it doesn't exist,  */
	    if( ggid < 0 ) 
	      {
		sprintf(msg, "Cannot open \"HDFEOS INFORMATION\" group.");
		PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, msg , funcName);
		(void) PGS_MET_SDend(sdId5);
		(void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		return(PGSMET_E_SD_SETATTR);
	      }
	    do
	      {
		if(attrCount > 0)
		  {
		    sprintf(multiHdfName, "%s.%d", hdfAttrName, attrCount);
		  }
		meta_id = H5Dopen(ggid, multiHdfName); 
		if( meta_id == FAIL)
		  {
		    sprintf(msg,"Cannot open (%s) dataset.",multiHdfName);
		    PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, msg , funcName);
		    (void) H5Gclose(ggid);
		    (void) PGS_MET_SDend(sdId5);
		    (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    return(PGSMET_E_SD_SETATTR);
		  }
		atype   = H5Dget_type(meta_id);
		if( atype == FAIL)
		  {
		    sprintf(msg,"Cannot get the dataset datatype.");
		    PGS_SMF_SetDynamicMsg(PGSMET_E_SD_SETATTR, msg , funcName);
		    (void) H5Dclose(meta_id);
		    (void) H5Gclose(ggid);
		    (void) PGS_MET_SDend(sdId5);
		    (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    return(PGSMET_E_SD_INFO);
		  }
		
		aspace  = H5Dget_space(meta_id);
		if( aspace == FAIL )
		  {
		    sprintf(msg,"Cannot get the dataset dataspace.");
		    PGS_SMF_SetDynamicMsg(PGSMET_E_SD_INFO, msg , funcName);
		    (void) H5Dclose(meta_id);
		    (void) H5Gclose(ggid);
		    (void) PGS_MET_SDend(sdId5);
		    (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    return(PGSMET_E_SD_INFO);
		  }
		type_size = H5Tget_size(atype);
		size = H5Sget_select_npoints(aspace) * type_size + 1;
		retVal = PGS_MEM_Calloc((void **)&metabuf, (size + 1), sizeof(char));

		if( retVal != PGS_S_SUCCESS )
		  {
		    sprintf(msg,"Cannot allocate memory for metabuffer.");
		    PGS_SMF_SetDynamicMsg(PGSMET_E_MALLOC_ERR, msg , funcName);
		    (void) H5Dclose(meta_id);
		    (void) H5Gclose(ggid);
		    (void) PGS_MET_SDend(sdId5);
		    (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    return(PGSMET_E_MALLOC_ERR);
		  }

		hdfRetVal = H5Dread(meta_id, atype,H5S_ALL, H5S_ALL, H5P_DEFAULT,metabuf);
		H5Tclose (atype);
		if( hdfRetVal == FAIL)
		  {
		    /* error message is:
		       "Unable to read hdf attribute" */
		    
		    sprintf(msg, "Cannot read (%s) metadata.", hdfAttrName);
		    PGS_SMF_SetDynamicMsg(PGSMET_E_SD_READ, msg , funcName);

		    (void) PGS_MEM_Free(metabuf);
		    (void) H5Dclose(meta_id);
		    (void) H5Gclose(ggid);
		    (void) PGS_MET_SDend(sdId5);
		    (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* unlock - do not check for error since we already had 
                   an error that we want the user to know about */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    return(PGSMET_E_SD_READ);
		  }
  
		/* write the buffer to the file */

		metabuf[size] = '\0';
		lenmetabuf = strlen(metabuf);
		fwrite( metabuf, sizeof(char), (size_t)lenmetabuf, fileHandle);

		(void) H5Dclose(meta_id);
		attrCount++;
		attrEnd = strcmp(&metabuf[lenmetabuf -4], "END\n");
		(void) PGS_MEM_Free(metabuf);
		metabuf = (char *) NULL;
	      }
	    while(lenmetabuf == MAX_ORDER && attrEnd != 0);
	    
	    (void) PGS_IO_Gen_Close(fileHandle);
	    (void) H5Gclose(ggid);
	    (void) PGS_MET_SDend(sdId5);
#ifdef _PGS_THREADSAFE
            retTSF = PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	    retVal = PGS_IO_Gen_Open((PGSt_PC_Logical) PGSd_MET_TEMP_ATTR_FILE, PGSd_IO_Gen_Read, &fileHandle, 1);
	    if(retVal != PGS_S_SUCCESS)
	    {
		sprintf(fileIdStr, "%d", (PGSt_PC_Logical) PGSd_MET_TEMP_ATTR_FILE);
		errInserts[0] = "input" ;
		errInserts[1] = fileIdStr;
 
		/* error message is:
		   "Unable to open <input> file with file id <aggregate name>" */

		(void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, "PGS_MET_ConvertToMCF",
					2, errInserts);
		aggNode = RemoveAggregate(aggNode);
		return(PGSMET_E_OPEN_ERR);
	    }
	  }
	}
        else
        {
            fileHandle = fopen(fileAttr, "r");
	    if(fileHandle == NULL)
	    {
		sprintf(fileIdStr, "%d", fileId);
		errInserts[0] = "input" ;
		errInserts[1] = fileIdStr;
 
		/* error message is:
		   "Unable to open <input> file with file id <aggregate name>" */
 
		(void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, "PGS_MET_ConvertToMCF",
					2, errInserts);
		aggNode = RemoveAggregate(aggNode);
		return(PGSMET_E_OPEN_ERR);
	    }
        }

	odlRetVal = ReadLabel(fileHandle, aggNode);
	if(odlRetVal == 0)
	{
	    sprintf(fileIdStr, "%d", fileId);
	    errInserts[0] = fileIdStr;
	    /* error message is:
	       "Unable to create ODL tree <aggName> with file id <fileId>" */
 
	    (void) PGS_MET_ErrorMsg(PGSMET_E_ODL_READ_ERR, "PGS_MET_ConvertToMCF",
				    1, errInserts);

	    aggNode = RemoveAggregate(aggNode);
	    if(fileHandle != NULL)
	    {
		(void) PGS_IO_Gen_Close(fileHandle);
	    }
	    return(PGSMET_E_ODL_READ_ERR);
	}

	/* close the file */
 
	if(fileHandle != NULL)
	{
	    (void) PGS_IO_Gen_Close(fileHandle);
	}

	/* extract the values and dump in the buffer provided */

	odlRetVal = ReadValue(tempNode, "GROUPTYPE", "MASTERGROUP");
	odlRetVal = ReadValue(tempNode, "MANDATORY", "FALSE");
	odlRetVal = ReadValue(tempNode, "TYPE", "STRING");
	odlRetVal = ReadValue(tempNode, "TYPE", "INTEGER");
	odlRetVal = ReadValue(tempNode, "TYPE", "DOUBLE");
	odlRetVal = ReadValue(tempNode, "TYPE", "DATETIME");
	odlRetVal = ReadValue(tempNode, "TYPE", "DATE");
	odlRetVal = ReadValue(tempNode, "TYPE", "TIME");
	odlRetVal = ReadValue(tempNode, "DATA_LOCATION", "MCF");

	mandNode = FindParameter(tempNode, "MANDATORY");
	strNode = FindParameter(tempNode, "TYPE");
	intNode = NextParameter(strNode);
	doubleNode = NextParameter(intNode);
	datetimeNode = NextParameter(doubleNode);
	dateNode = NextParameter(datetimeNode);
	timeNode = NextParameter(dateNode);
	locNode = FindParameter(tempNode, "DATA_LOCATION");
        /* to set DATA_LOCATION value to "TK" for PRODUCTIONDATETIME
	   and rest to "MCF" */

	locValue = FirstValue(locNode);

	groupNode = FindParameter(tempNode, "GROUPTYPE");
	objectNode = aggNode;
 
        
        grouptypenode = NextGroup(objectNode);

        if (strcmp(grouptypenode->name,"INVENTORYMETADATA") != 0)
        {
            free(grouptypenode->name);
            grouptypenode->name = (char *)malloc(strlen("INVENTORYMETADATA") + 1);
            strcpy(grouptypenode->name, "INVENTORYMETADATA");
        }
        else
        {
          ;
        }

        groupnode = FindParameter(grouptypenode,PGSd_MET_GROUP_TYPE_STR);

        if(groupnode == NULL)
        {
           parmNode = CopyParameter(groupNode);
           parmNode = PasteParameter(grouptypenode, parmNode);
        }
        else 
        {
           ;
        }

	do
	  {
	    objectNode = NextObject(objectNode);
	    if(objectNode != NULL)
	      {
		/* for PRODUCTIONDATETIME set DATA_LOCATION to "TK" */
		if(strcmp(objectNode->name,"PRODUCTIONDATETIME") == 0)
		  {
		    strcpy(locValue->item.value.string, PGSd_MET_SET_BY_TK);
		    parmNode = CopyParameter(locNode);
		    parmNode = PasteParameter(objectNode, parmNode);
		  }
		else
		  {
		    /* for others set DATA_LOCATION to "MCF" */
		    strcpy(locValue->item.value.string, PGSd_MET_SET_BY_MCF);
		    parmNode = CopyParameter(locNode);
		    parmNode = PasteParameter(objectNode, parmNode);
		  }
		
		parmNode = CopyParameter(mandNode);
		parmNode = PasteParameter(objectNode, parmNode);
		
		childNode = NextSubObject(objectNode, objectNode);
		if(childNode == NULL)
		  {
		    typeNode = FindParameter(objectNode, PGSd_MET_ATTR_TYPE_STR);				
                    if (typeNode == NULL)
		      {
		        parmNode = FindParameter(objectNode, PGSd_MET_ATTR_VALUE_STR);				
			valueNode = FirstValue(parmNode);
			
			if(valueNode != NULL)
			  {
			    type = valueNode->item.type;
			    switch (type)
			      {
			      case TV_STRING:
				if(strncmp(valueNode->item.value.string, "NOT SET:", strlen("NOT SET:")) == 0 ||
				   strncmp(valueNode->item.value.string, "NOT SUPPLIED:", strlen("NOT SUPPLIED:")) == 0 || 
				   strncmp(valueNode->item.value.string, "NOT FOUND:", strlen("NOT FOUND:")) == 0)  
				  {
#ifdef _PGS_THREADSAFE
				    char *lasts;     /* used by strtok_r() */
				    
				    /* strtok() is not threadsafe use strtok_r() */
				    strcpy(charstr, valueNode->item.value.string);
				    typePtr = strtok_r(charstr, ":",&lasts); 
				    typePtr = strtok_r(NULL, ":",&lasts); 
#else
				    strcpy(charstr, valueNode->item.value.string);
				    typePtr = strtok(charstr, ":"); 
				    typePtr = strtok(NULL, ":"); 
#endif
				    
				    /* Remove the VALUE parameter from the aggregate Node if attached
				     * value node containning any one of the following strings, "NOT
				     * SET:STRING", "NOT SET:INTEGER", "NOT SET:DOUBLE", "NOT SET:
				     * DATETIME", "NOT SET:DATE", "NOT SET:TIME", "NOT SUPPLIED:STRING",
				     * "NOT SUPPLIED:INTEGER", "NOT SUPPLIED:DOUBLE", "NOT SUPPLIED:
				     * DATETIME", "NOT SUPPLIED:DATE", "NOT SUPPLIED:TIME", "NOT FOUND:
				     * STRING", "NOT FOUND:INTEGER", "NOT FOUND:DOUBLE", "NOT FOUND:
				     * DATETIME", "NOT FOUND:DATE", or "NOT FOUND:TIME" that was written
				     * out on the HDF-EOS/non HDF-EOS meatadata file when attribute has
				     * not been set up with the proper value. And a TYPE parameter
				     * including the attached value node that contains  any one of the
				     * following type of data, "INTEGER", "STRING", "DOUBLE", "DATETIME",
				     * "DATE", or "TIME", will be attached to this aggregate Node
				     */
				    
				    if(typePtr != NULL &&
				       (strcmp(typePtr, PGSd_MET_STRING_STR) == 0 || 
					strcmp(typePtr, PGSd_MET_DATETIME_STR) == 0 ||
					strcmp(typePtr, PGSd_MET_DATE_STR) == 0 ||
					strcmp(typePtr, PGSd_MET_INTEGER_STR) == 0 ||
					strcmp(typePtr, PGSd_MET_DOUBLE_STR) == 0 ||
					strcmp(typePtr, PGSd_MET_TIME_STR) == 0)) 
				      {
					parmNode = RemoveParameter(parmNode);
					if(strcmp(typePtr, PGSd_MET_DATETIME_STR) == 0)
					  {
					    parmNode = CopyParameter(datetimeNode);
					  }
					else if(strcmp(typePtr, PGSd_MET_DATE_STR) ==0)
					  {
					    parmNode = CopyParameter(dateNode);
					  }
					else if(strcmp(typePtr, PGSd_MET_TIME_STR) == 0)
					  {
					    parmNode = CopyParameter(timeNode);
					  }
					else if(strcmp(typePtr, PGSd_MET_STRING_STR) == 0)
					  {
					    parmNode = CopyParameter(strNode);
					  }
					else if(strcmp(typePtr, PGSd_MET_INTEGER_STR) == 0)
					  {
					    parmNode = CopyParameter(intNode);
					  }
					else if(strcmp(typePtr, PGSd_MET_DOUBLE_STR) == 0)
					  {
					    parmNode = CopyParameter(doubleNode);
					  }
					parmNode = PasteParameter(objectNode, parmNode);
				      }
				    else
				      {
					/* error message is
					   "Unable to obtain data type for the unset attribute" */
					(void) PGS_MET_ErrorMsg(PGSMET_E_TYPE_ERR, funcName, 0, errInserts);              
					return(PGSMET_E_TYPE_ERR);
				      }
				  }
				else if (strcmp(valueNode->item.value.string, "NOT SET") == 0 ||
					 strcmp(valueNode->item.value.string, "NOT SUPPLIED") == 0 ||
					 strcmp(valueNode->item.value.string, "NOT FOUND") == 0)
				  {
				    /* error message is
				       "Unable to convert HDF-EOS/non HDF-EOS metadata product 
				       file, in which unset attributes were defined as NOT SET
				       for Data Location PGE, NOT SUPPLIED for Data Location
				       MCF, or NOT FOUND for Data Location NONE, to a .MCF file" */
				    
				    (void) PGS_MET_ErrorMsg(PGSMET_E_CONVERT_ERR, funcName, 0, errInserts);
				    return(PGSMET_E_CONVERT_ERR);
				  }
				else
				  {
				    parmNode = CopyParameter(strNode);
				    parmNode = PasteParameter(objectNode, parmNode);
				  }
				break; 
			      case TV_DATE_TIME:
				parmNode = CopyParameter(datetimeNode);
				parmNode = PasteParameter(objectNode, parmNode);
				break;
			      case TV_DATE:
				parmNode = CopyParameter(dateNode);
				parmNode = PasteParameter(objectNode, parmNode);
				break;
			      case TV_TIME:
				parmNode = CopyParameter(timeNode);
				parmNode = PasteParameter(objectNode, parmNode);
				break;
			      case TV_INTEGER:
				parmNode = CopyParameter(intNode);
				parmNode = PasteParameter(objectNode, parmNode);
				break;
			      case TV_REAL:
				parmNode = CopyParameter(doubleNode);
				parmNode = PasteParameter(objectNode, parmNode);
				break;
			      } 
			  }
		      }
		  }
	      }
	  }
	while(objectNode != NULL);

	    /* generate the MCF product output .MCF file will contains the same full path/name
	       as the input .met file */
 
	strcpy(filename,fileAttr);
	strcat(filename,".MCF");
	fileHandle = fopen(filename, "w");
 
	if(fileHandle == NULL)
	{
	    errInserts[0] = "output" ;
 
	    /* error message is:
	       "Unable to open <output> file with file id <aggregate name>" */
 
	    (void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, "PGS_MET_ConvertToMCF",
				    1, errInserts);
	    aggNode = RemoveAggregate(aggNode);
	    tempNode = RemoveAggregate(tempNode);
	    return(PGSMET_E_OPEN_ERR);
	}

	WriteLabel(fileHandle, aggNode);
	aggNode = RemoveAggregate(aggNode);
	tempNode = RemoveAggregate(tempNode);
	(void) PGS_IO_Gen_Close(fileHandle);
	return(PGS_S_SUCCESS);

}
