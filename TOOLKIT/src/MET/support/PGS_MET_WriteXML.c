/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2009, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_MET_WriteXML
 
DESCRIPTION:
	Enables the user to write XML metadata into HDF file
	
AUTHOR: 
	Abe Taaheri / Raytheon

HISTORY:
        05-MAY-2009      AT     Initial version

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
        Writes XML metadata and their values to HDF attributes
  
NAME:  
        PGS_MET_WriteXML()

SYNOPSIS:
C:
        #include "PGS_MET.h"

	PGSt_SMF_status
		PGS_MET_WriteXML(
			char *          xmlFileName,
			char *          hdfxmlAttrName,
			PGSt_integer    hdfFileId)
INPUTS:
        Name            Description            Units    	Min	Max
        ----            -----------             -----   	---     ---
	xmlFileName	ASCII XML filename     	none		N/A	N/A
			
        hdfxmlAttrName	HDF file attribute 	none            N/A     N/A
			name for XML
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

           xmlretVal = PGS_MET_WriteXML(xmlFileName, "xmlmetadata", hdfFileId) 
           if (xmlretVal != PGS_S_SUCCESS)
             {
	        printf("ASCII Write failed\n");
             }
NOTES:
 	NONE

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
PGS_MET_WriteXML(
             char *          xmlFileName,  /* ASCII XML file name */
             char *          hdfAttrName,  /* name of XML hdf attribute to be attached */
             PGSt_integer    hdfFileId)    /* HDF product file id or local data id */
{
	PGSt_IO_Gen_FileHandle  *fileHandle = NULL;
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
	char *			funcName = "PGS_MET_WriteXML";
	char 			*mcfBuffer = NULL;
	char                    *mcfBuffer_temp = NULL;
	char 			fileIdStr[10];
      	char *			newAttrName= NULL;
        PGSt_integer            logicalId; /* user defined logical ID that
                                              represents the Product listed
                                              in the PCF  */

	PGSt_integer		i = 0; /* loop count */
	PGSt_integer		numBytesRead = 0;

	PGSt_integer		numAttr = 0;
	PGSt_integer		filType;

	char * 			hFileName = NULL;
	intn                    access;
	intn			attach;
	NC			*hdfhandle = NULL;
	PGSt_integer            mcfNumber = 0;
        char *                  mcfNumPtr = NULL;
        char *                  handlePtr = NULL;
        char *                  errPtr = NULL;

	char                    signature[PGSd_MET_SIGNATURE_L] = "Razia";
	PGSt_integer            fileInBuffer = 1;
        PGSt_integer            bufferIndex = 0;
        PGSt_IO_Gen_Duration    file_duration = 1;
	char                    dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

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

 	/* open the  XML ASCII file for reading */

	fileHandle = fopen(xmlFileName, "r");

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
	  /* error message is:
                        "Unable to open MET ascii file  ...." */
                sprintf(dynamicMsg, "Unable to open XML MET ascii file %s", xmlFileName);
		PGS_SMF_SetDynamicMsg(PGSMET_E_MET_ASCII_ERR, dynamicMsg, funcName);
 
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - let user know about 
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
                return(PGSMET_E_MET_ASCII_ERR);
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

	if(mcfBuffer != NULL)
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
	   sprintf(dynamicMsg, "Unexpected failure closing file %s", xmlFileName);
	   
	   PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,dynamicMsg, funcName);
	   (void) PGS_MEM_Free(mcfBuffer);
	   mcfBuffer = (char *) NULL;
	   (void) PGS_MEM_Free(newAttrName);
	   newAttrName = (char *) NULL;
	   return PGS_E_TOOLKIT;
       }

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
