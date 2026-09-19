/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2009, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_HDFToXML.c
 
DESCRIPTION:
         The file contains PGS_MET_HDFToXML.
	 The function is used to convert data from HDF attribute
	 to XML representation in memory  (stored in temporary 
         PGSd_MET_TEMP_ATTR_FILE file)

AUTHOR:
	Abe Taaheri / Raytheon

HISTORY:
  	18-May-2009 	AT 	Initial version

END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <PGS_MET.h>
#include <PGS_MEM.h>
#include <PGS_TSF.h>

#include <CUC/odldef.h>
#include <CUC/odlinter.h>

/* hdf files */

#ifdef DEC_ALPHA        /* DEC Alpha platform */
#include <rpc/types.h>  /* this avoids typedef conflict with int32, uint32 */
#endif /* DEC_ALPHA */
#include <hdf.h>
#include <mfhdf.h>
#include <hdf5.h>

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE:  
  	Converts HDF attribute (assumed to be in ODL format) to XML memory
	representation
 
NAME:
  	PGS_MET_HDFToXML()

SYNOPSIS:
  	N/A

DESCRIPTION:
	Converts HDF attribute (assumed to be in XML format) to XML memory
        representation

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
       	fileName	HDF file name		none	N/A	N/A
	hdfAttrName	HDF attribute Name	none    N/A     N/A
	

OUTPUTS:
	None                             	none    N/A     N/A


RETURNS:
	PGS_S_SUCCESS
	PGSMET_E_SD_START	Unable to open the HDF file
	PGSMET_E_SD_FINDATTR	Unable to get the attr index
	PGSMET_E_SD_INFO	Unable to retrieve sd attribute information
	PGSMET_E_MALLOC_ERR	Unable to allocate memory for the hdf attribute
	PGSMET_E_SD_READ	Unable to read hdf attribute
	PGSMET_E_FILETOODL_ERR	Unable to convert <filename> into odl format
	PGSTSF_E_GENERAL_FAILURE Problem in TSF code


EXAMPLES:
	N/A

NOTES:
	N/A

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	SDstart
	PGS_MET_ErrorMsg
	SDfindattr
	SDfileinfo
	SDselect
	SDattrinfo
	PGS_MEM_Malloc
	SDreadattr
	PGS_MET_ConvertToOdl
	PGS_MEM_Free
        PGS_TSF_LockIt
        PGS_TSF_UnlockIt
        PGS_SMF_TestErrorLevel
	

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_MET_HDFToXML(			    /* This function is used to convert 
					     * HDF attribute data to an ODL structure */
	    char 	*fileName,	    /* HDF file name */
	    char  	*hdfAttrName)	    /* HDF attribute Name */
{
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char * 			funcName = "PGS_MET_HDFToXML";
	char * 			sdAttrValue = NULL;
	int32			sdId = 0;
	int32			sdsId = 0;
	int32			hdfid = 0;
	int32			attrIndex = 0;
	int32			nDataSets = 0;
	int32			nGlobalAttrs = 0;
	int32                   sdNumType = 0;
	int32                   sdCount = 0;
	PGSt_integer		hdfRetVal = 0;
	PGSt_integer            loopCount;
	PGSt_integer            dataSize = 0;
	PGSt_IO_Gen_FileHandle  *fileHandle = NULL; 
	PGSt_integer 		attrCount = 0;
	char 			multiHdfName[200];
	char                    fileIdStr[PGSd_MET_FILE_ID_L]; /* file id value as string */
	PGSt_integer		attrEnd = 1;
	PGSt_integer            fileishdf = 0; /*if input file is hdf fileishdf
                                                = 0 */
	hid_t                   hdf5id;
	char *                  metabuf = NULL; /* Pointer (handle) to 
						   coremetadata array */
	PGSt_integer		FILE_IS_HDF4;
	PGSt_integer		FILE_IS_HDF5;
	PGSt_integer	        FILE_NOT_HDF;
	char                    msg[PGS_SMF_MAX_MSGBUF_SIZE];  /* message buffer */
        hid_t           ggid;
        herr_t          status = -1;
        size_t          size;
        hid_t           atype;
	hid_t           meta_id     = FAIL;/*  metadata ID */
	hid_t           aspace      = FAIL;/* dataspace ID */
	PGSt_integer	sdId5 = 0;
	size_t          type_size;
	PGSt_integer	lenmetabuf;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retTSF;           /* lock/unlock return */

    /* Since we are calling COTS (HDF) lock here */
    retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
    if (PGS_SMF_TestErrorLevel(retTSF))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
#endif

	/* check to see whether file is hdf or non-hdf file */
      	FILE_IS_HDF4 = 0;
	FILE_IS_HDF5 = 0;
	FILE_NOT_HDF = 0;
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

#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user to know
                   about the other error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		return(PGSMET_E_SD_START);
	      }
	  }
	else
	  {
	    sprintf(msg, " Cannot determine whether the file (%s) is HDF4, HDF5, or NONE-HDF type. ", fileName);
	    PGS_SMF_SetDynamicMsg(retVal, msg, funcName);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user to know
                   about the other error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	    return(retVal);
	  }
    if(fileishdf == 0) /* new form when not using PCF file */
    {
      if( FILE_IS_HDF4 == 1) /* file is HDF4 type */
      {
	sdId= SDstart(fileName, DFACC_RDONLY); 
        if( sdId == FAIL)
        {
                /* error message is:
                "Unable to open the HDF file" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_SD_START, funcName,
                                    0, errInserts);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user to know
                   about the other error */
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
                /* 
		   error message is:
		   "Unable to open <temporary> file with file id <aggregate name>" 
		*/
 
                (void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, funcName,
                                    2, errInserts);
		(void) SDend(sdId);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user to know
                   about the other error */
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
                	/* (void) PGS_IO_Gen_Temp_Delete(PGSd_MET_TEMP_ATTR_FILE); */
#ifdef _PGS_THREADSAFE
                        /* Unlock - do not check error since we want the user 
                           to know about the other error */
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
                /* (void) PGS_IO_Gen_Temp_Delete(PGSd_MET_TEMP_ATTR_FILE); */
		(void) SDend(sdId);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user 
                   to know about the other error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_SD_INFO);
        }
        dataSize = (PGSt_integer) ((DFKNTsize(sdNumType) * sdCount) + 1);

	if(sdAttrValue != NULL)
	  {
	    PGS_MEM_Free(sdAttrValue);
	    sdAttrValue = NULL;
	  }

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
                /* (void) PGS_IO_Gen_Temp_Delete(PGSd_MET_TEMP_ATTR_FILE); */
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user 
                   to know about the other error */
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
                /* (void) PGS_IO_Gen_Temp_Delete(PGSd_MET_TEMP_ATTR_FILE); */
		(void) SDend(sdId);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user 
                   to know about the other error */
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

        sdAttrValue = (char *) NULL;
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
                /* Unlock - do not check error since we want the user to know
                   about the other error */
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
                /* Unlock - do not check error since we want the user to know
                   about the other error */
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
	      PGS_SMF_SetDynamicMsg(PGSMET_E_SD_READ, msg , funcName);
	      (void) PGS_MET_SDend(sdId5);
	      (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user to know
                   about the other error */
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
                /* Unlock - do not check error since we want the user to know
                   about the other error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		  return(PGSMET_E_SD_SETATTR);
		}
	      atype   = H5Dget_type(meta_id);
	      if( atype == FAIL)
		{
		  status = PGSMET_E_SD_INFO;
		  sprintf(msg,"Cannot get the dataset datatype.");
		  PGS_SMF_SetDynamicMsg(PGSMET_E_SD_INFO, msg , funcName);
		  (void) H5Dclose(meta_id);
		  (void) H5Gclose(ggid);
		  (void) PGS_MET_SDend(sdId5);
		  (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user to know
                   about the other error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		  return(status);
		}
	      
	      aspace  = H5Dget_space(meta_id);
	      if( aspace == FAIL )
		{
		  status = PGSMET_E_SD_INFO;
		  sprintf(msg,"Cannot get the dataset dataspace.");
		  PGS_SMF_SetDynamicMsg(PGSMET_E_SD_INFO, msg , funcName);
		  (void) H5Dclose(meta_id);
		  (void) H5Gclose(ggid);
		  (void) PGS_MET_SDend(sdId5);
		  (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user to know
                   about the other error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		  return(status);
		}
	      type_size = H5Tget_size(atype);
	      size = H5Sget_select_npoints(aspace) * type_size + 1;
	      retVal = PGS_MEM_Calloc((void **)&metabuf, (size + 1), sizeof(char));

	      if(retVal != PGS_S_SUCCESS )
		{
		  sprintf(msg,"Cannot allocate memory for metabuffer.");
		  PGS_SMF_SetDynamicMsg(PGSMET_E_MALLOC_ERR, msg , funcName);
		  (void) H5Dclose(meta_id);
		  (void) H5Gclose(ggid);
		  (void) PGS_MET_SDend(sdId5);
		  (void) PGS_IO_Gen_Close(fileHandle);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check error since we want the user to know
                   about the other error */
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
                /* Unlock - do not check error since we want the user to know
                   about the other error */
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
	  while(lenmetabuf == MAX_ORDER &&attrEnd != 0);
	    
	  (void) PGS_IO_Gen_Close(fileHandle);
	  (void) H5Gclose(ggid);
	  (void) PGS_MET_SDend(sdId5);
	  
#ifdef _PGS_THREADSAFE
        retTSF = PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if(metabuf != NULL)
	  {
	    (void) PGS_MEM_Free(metabuf);
	    metabuf = (char *) NULL;
	  }
	}
    }
    return(PGS_S_SUCCESS);
}
