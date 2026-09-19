/*******************************************************
PGS_DEM_Subset.c--

This function is the underlying access to an individual subset.  It allows one
to initialize a subset, to close a subset, or to accesss information about or
from a subset.

Explaination of subsets:


Future modification:  presently this has a switch statement to sort through the
different resolutions and the different layers.  In the futre I imagine that it
will be able to access a lookup file in the PCF to get all resolution specific
information.  allowing it to be easily updated with new resolutions

Added access to quality, source, and geoid data.  Two new flags can now be
passed to this function: PGSd_DEM_QUALITYINFO and PGSd_DEM_QUALITYCLOSE.  These
flags correspond to accessing info about the particular "qualityField" and
closing each "qualityField" (see documentation on PGS_DEM_GetQualityData for
details on quality fields).

Commands:
    PGSd_DEM_INITIALIZE
    PGSd_DEM_DEINITIALIZE
    PGSd_DEM_INFO
    PGSd_DEM_QUALITYINFO
    PGSd_DEM_QUALITYCLOSE


Author -- Alexis Zubrow
Note: I would like to thank Guru Tej Khalsa for his invaluable assistance in
design. 

history --
January 30, 1997  AZ         first created 
May 9, 1997       AZ         added Quality access 
July 3, 1997      AZ         added dataType to PGSd_DEM_INFO
December 8, 1997 Abe Taaheri Fixed memeory leak associated with initializeInfo
                             which needed to be freed before returning 
                             PGS_S_SUCCESS
July 19, 1999     AT         For 30 ARC subsets B and C changed missprint 
                             info3ARC1 to info30ARC2 and info30ARC3 in the
                             if statements
July 8, 1999      SZ         Updated for the thread-safe functionality      
June 5, 2000      AT         Added functionality for 3km resolution
Sept 14,2011   JK/AT	     Added functionality for 500m (15 arcsec) resolution.
Jan  26 2016      AT         changed NULL to an integer value for PGS_DEM_AccessFile call with 

*******************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <PGS_DEM.h>
#include <PGS_SMF.h>
#include <PGS_MEM.h>
#include <PGS_TSF.h>

PGSt_SMF_status
PGS_DEM_Subset(
    PGSt_DEM_Tag resolution,            /*Resolution  */
    PGSt_integer layer,                 /*Data layer, mask, ex. "Elevation"*/
    PGSt_integer command,               /*Commands-- Open, Close, Info., etc.*/
    PGSt_DEM_FileRecord ***subset,       /*Pointer to the complete subset*/
    PGSt_DEM_SubsetRecord **subsetInfo)  /*Pointer to structure of subset info*/
{

    /*return status*/
    PGSt_SMF_status  status;
    PGSt_SMF_status  statusError;        /*error status- PGS_DEM_Subset*/
    char errorBuf[PGS_SMF_MAX_MSG_SIZE]; /*Strings appended to error returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    char errorMsg[PGS_SMF_MAX_MSGBUF_SIZE];
    
    /*Initialization and Closing*/    
    PGSt_DEM_SubsetRecord *initializeInfo = NULL;  /*info for initialiazation step*/
    
    PGSt_DEM_FileRecord **subsetInitialize = NULL; /*Array pointers to file struct*/
    PGSt_DEM_FileRecord *stagedFiles = NULL;       /*temporarily holds space for
					       records of only the files
					       which were staged */

    PGSt_integer (*orderedArray)[2] = NULL;        /*Array of organized subgrids and
					      version numbers*/

    PGSt_integer numberFiles;               /*number of files staged*/
    
    /* Get Info */
    PGSt_DEM_SubsetRecord *getInfo;         /* for Get info step*/ 

    /*De-initialize, for PGS_DEM_Close function*/  
    PGSt_DEM_SubsetRecord deinitializeInfo;
    PGSt_DEM_SubsetRecord *closeSubsetInfo; /*holds pointer to subsetInfo to be
					     closed*/
    PGSt_DEM_FileRecord **closeSubset;      /*holds pointer to subset to be
					     closed*/
    PGSt_integer layersLeft;                /*layers still initialized */
    
    /*Quality intialize, accessing, and closeing*/
     PGSt_DEM_SubsetRecord *qualityInfo;  /*info for quality steps*/
   
    
    /*Storage of Subsets and subsetInfo for different resolutions*/
#ifdef _PGS_THREADSAFE
    /* Create non-static variable for the thread-safe version */
    PGSt_DEM_SubsetRecord *info3ARC1;
    PGSt_DEM_SubsetRecord *info3ARC2; 
    PGSt_DEM_SubsetRecord *info3ARC3;
    PGSt_DEM_SubsetRecord *info15ARC1;
    PGSt_DEM_SubsetRecord *info15ARC2;
    PGSt_DEM_SubsetRecord *info15ARC3;
    PGSt_DEM_SubsetRecord *info30ARC1;
    PGSt_DEM_SubsetRecord *info30ARC2;
    PGSt_DEM_SubsetRecord *info30ARC3;
    PGSt_DEM_SubsetRecord *info90ARC1;
    PGSt_DEM_SubsetRecord *info90ARC2;
    PGSt_DEM_SubsetRecord *info90ARC3;
    PGSt_DEM_SubsetRecord *info30TEST;
    PGSt_DEM_FileRecord **subset3ARC1;
    PGSt_DEM_FileRecord **subset3ARC2;
    PGSt_DEM_FileRecord **subset3ARC3;
    PGSt_DEM_FileRecord **subset15ARC1;
    PGSt_DEM_FileRecord **subset15ARC2;
    PGSt_DEM_FileRecord **subset15ARC3;
    PGSt_DEM_FileRecord **subset30ARC1;
    PGSt_DEM_FileRecord **subset30ARC2;
    PGSt_DEM_FileRecord **subset30ARC3;
    PGSt_DEM_FileRecord **subset90ARC1;
    PGSt_DEM_FileRecord **subset90ARC2;
    PGSt_DEM_FileRecord **subset90ARC3;
    PGSt_DEM_FileRecord **subset30TEST;
    PGSt_DEM_SubsetRecord *infoQUALITY;

    /* Set up thread-safe key keeper and TSD key */
    PGSt_TSF_MasterStruct *masterTSF;
    status = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(status))
    {
       return PGSTSF_E_GENERAL_FAILURE;;
    }
    info3ARC1 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO3ARC1]);  
    info3ARC2 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO3ARC2]);
    info3ARC3 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO3ARC3]);
    info15ARC1 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO15ARC1]); 
    info15ARC2 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO15ARC2]);
    info15ARC3 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO15ARC3]);
    info30ARC1 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30ARC1]); 
    info30ARC2 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30ARC2]);
    info30ARC3 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30ARC3]);
    info90ARC1 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO90ARC1]); 
    info90ARC2 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO90ARC2]);
    info90ARC3 = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO90ARC3]);
    info30TEST = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30TEST]);
    subset3ARC1 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC1]);
    subset3ARC2 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC2]);
    subset3ARC3 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC3]);
    subset15ARC1 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET15ARC1]);
    subset15ARC2 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET15ARC2]);
    subset15ARC3 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET15ARC3]);
    subset30ARC1 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC1]);
    subset30ARC2 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC2]);
    subset30ARC3 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC3]);
    subset90ARC1 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET90ARC1]);
    subset90ARC2 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET90ARC2]);
    subset90ARC3 = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET90ARC3]);
    subset30TEST = (PGSt_DEM_FileRecord **) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30TEST]);
    infoQUALITY = (PGSt_DEM_SubsetRecord *) pthread_getspecific(
                              masterTSF->keyArray[PGSd_TSF_KEYDEMINFOQUALITY]);
#else
    static PGSt_DEM_SubsetRecord *info3ARC1 = NULL;    /*3 arc second info*/   
    static PGSt_DEM_SubsetRecord *info3ARC2 = NULL;    /*3 arc second info*/   
    static PGSt_DEM_SubsetRecord *info3ARC3 = NULL;    /*3 arc second info*/  
    static PGSt_DEM_SubsetRecord *info15ARC1 = NULL;   /*15 arc second info*/
    static PGSt_DEM_SubsetRecord *info15ARC2 = NULL;   /*15 arc second info*/
    static PGSt_DEM_SubsetRecord *info15ARC3 = NULL;   /*15 arc second info*/
    static PGSt_DEM_SubsetRecord *info30ARC1 = NULL;   /*30 arc second info*/
    static PGSt_DEM_SubsetRecord *info30ARC2 = NULL;   /*30 arc second info*/
    static PGSt_DEM_SubsetRecord *info30ARC3 = NULL;   /*30 arc second info*/
    static PGSt_DEM_SubsetRecord *info90ARC1 = NULL;   /*90 arc second info*/
    static PGSt_DEM_SubsetRecord *info90ARC2 = NULL;   /*90 arc second info*/
    static PGSt_DEM_SubsetRecord *info90ARC3 = NULL;   /*90 arc second info*/

    static PGSt_DEM_SubsetRecord *info30TEST = NULL;/*30 arc second TEST info*/

    static PGSt_DEM_FileRecord **subset3ARC1 = NULL;  /*3 arc second subset*/
    static PGSt_DEM_FileRecord **subset3ARC2 = NULL;  /*3 arc second subset*/
    static PGSt_DEM_FileRecord **subset3ARC3 = NULL;  /*3 arc second subset*/
    static PGSt_DEM_FileRecord **subset15ARC1 = NULL; /*15 arc second subset*/
    static PGSt_DEM_FileRecord **subset15ARC2 = NULL; /*15 arc second subset*/
    static PGSt_DEM_FileRecord **subset15ARC3 = NULL; /*15 arc second subset*/
    static PGSt_DEM_FileRecord **subset30ARC1 = NULL; /*30 arc second subset*/
    static PGSt_DEM_FileRecord **subset30ARC2 = NULL; /*30 arc second subset*/
    static PGSt_DEM_FileRecord **subset30ARC3 = NULL; /*30 arc second subset*/
    static PGSt_DEM_FileRecord **subset90ARC1 = NULL; /*90 arc second subset*/
    static PGSt_DEM_FileRecord **subset90ARC2 = NULL; /*90 arc second subset*/
    static PGSt_DEM_FileRecord **subset90ARC3 = NULL; /*90 arc second subset*/

    static PGSt_DEM_FileRecord **subset30TEST = NULL;  /*30 arc second TEST*/

    /*storage for subsetInfo for different quality fields*/
    static PGSt_DEM_SubsetRecord *infoQUALITY = NULL; /*Quality, Source, or
                                                        Geoid data info*/
#endif							


    /*Determine Command*/
    if (command == PGSd_DEM_INITIALIZE)        /*Open subset*/
    {
	/*Calloc space for initializeInfo*/
	initializeInfo = (PGSt_DEM_SubsetRecord *) calloc(1,
                                    sizeof(PGSt_DEM_SubsetRecord));
	if (initializeInfo == NULL)
	{
	    /*ERROR callocing space for initialize SubsetRecord*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "initializeInfo");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					      dynamicMsg, 
					      "PGS_DEM_Subset()");
	    statusError = PGSMEM_E_NO_MEMORY;
	    return(PGSMEM_E_NO_MEMORY);
	}
	
	/*Lookup specific on resolution and layer*/
	status = PGS_DEM_Lookup(resolution, layer, initializeInfo);
	if (status != PGS_S_SUCCESS)
	{
	    statusError = status;

	    /*fatal error*/
	    goto fatalError;
	    
	}

	/*Check if subset has already been initialized, if so, only need to
	  transfer some elements which are layer dependent from initializeInfo
	  to the stored subsetInfo.*/
	/*here we also increment the layers which have been initialized*/
	switch(initializeInfo -> subset)
	{
	  case PGSd_DEM_SUBSET3A:
	    if (info3ARC1 != NULL)  /*i.e. it has already been initialized*/
	    {
		info3ARC1 -> layer = initializeInfo -> layer;
		info3ARC1 -> layersInit += initializeInfo -> layer;
		info3ARC1 -> fillvalue = initializeInfo -> fillvalue;
		info3ARC1 -> dataType = initializeInfo -> dataType;
		strcpy(info3ARC1 -> gridName, initializeInfo -> gridName);
		strcpy(info3ARC1 -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;
	  case PGSd_DEM_SUBSET3B:
	    if (info3ARC2 != NULL)  /*i.e. it has already been initialized*/
	    {
		info3ARC2 -> layer = initializeInfo -> layer;
		info3ARC2 -> layersInit += initializeInfo -> layer;
		info3ARC2 -> fillvalue = initializeInfo -> fillvalue;
		info3ARC2 -> dataType = initializeInfo -> dataType;
		strcpy(info3ARC2 -> gridName, initializeInfo -> gridName);
		strcpy(info3ARC2 -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;
	  case PGSd_DEM_SUBSET3C:
	    if (info3ARC3 != NULL)  /*i.e. it has already been initialized*/
	    {
		info3ARC3 -> layer = initializeInfo -> layer;
		info3ARC3 -> layersInit += initializeInfo -> layer;
		info3ARC3 -> fillvalue = initializeInfo -> fillvalue;
		info3ARC3 -> dataType = initializeInfo -> dataType;
		strcpy(info3ARC3 -> gridName, initializeInfo -> gridName);
		strcpy(info3ARC3 -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;	    
	  case PGSd_DEM_SUBSET15A:
	    if (info15ARC1 != NULL)  /*i.e. it has already been initialized*/
	    {
		info15ARC1 -> layer = initializeInfo -> layer;
		info15ARC1 -> layersInit += initializeInfo -> layer;
		info15ARC1 -> fillvalue = initializeInfo -> fillvalue;
		info15ARC1 -> dataType = initializeInfo -> dataType;
		strcpy(info15ARC1 -> gridName, initializeInfo -> gridName);
		strcpy(info15ARC1 -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;
	  case PGSd_DEM_SUBSET15B:
	    if (info15ARC2 != NULL)  /*i.e. it has already been initialized*/
	    {
		info15ARC2 -> layer = initializeInfo -> layer;
		info15ARC2 -> layersInit += initializeInfo -> layer;
		info15ARC2 -> fillvalue = initializeInfo -> fillvalue;
		info15ARC2 -> dataType = initializeInfo -> dataType;
		strcpy(info15ARC2 -> gridName, initializeInfo -> gridName);
		strcpy(info15ARC2 -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;
	  case PGSd_DEM_SUBSET15C:
	    if (info15ARC3 != NULL)  /*i.e. it has already been initialized*/
	    {
		info15ARC3 -> layer = initializeInfo -> layer;
		info15ARC3 -> layersInit += initializeInfo -> layer;
		info15ARC3 -> fillvalue = initializeInfo -> fillvalue;
		info15ARC3 -> dataType = initializeInfo -> dataType;
		strcpy(info15ARC3 -> gridName, initializeInfo -> gridName);
		strcpy(info15ARC3 -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;
	  case PGSd_DEM_SUBSET30A:
	    if (info30ARC1 != NULL)  /*i.e. it has already been initialized*/
	    {
		info30ARC1 -> layer = initializeInfo -> layer;
		info30ARC1 -> layersInit += initializeInfo -> layer;
		info30ARC1 -> fillvalue = initializeInfo -> fillvalue;
		info30ARC1 -> dataType = initializeInfo -> dataType;
		strcpy(info30ARC1 -> gridName, initializeInfo -> gridName);
		strcpy(info30ARC1 -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;
	  case PGSd_DEM_SUBSET30B:
	    if (info30ARC2 != NULL)  /*i.e. it has already been initialized*/
	    {
		info30ARC2 -> layer = initializeInfo -> layer;
		info30ARC2 -> layersInit += initializeInfo -> layer;
		info30ARC2 -> fillvalue = initializeInfo -> fillvalue;
		info30ARC2 -> dataType = initializeInfo -> dataType;
		strcpy(info30ARC2 -> gridName, initializeInfo -> gridName);
		strcpy(info30ARC2 -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;
	  case PGSd_DEM_SUBSET30C:
	    if (info30ARC3 != NULL)  /*i.e. it has already been initialized*/
	    {
		info30ARC3 -> layer = initializeInfo -> layer;
		info30ARC3 -> layersInit += initializeInfo -> layer;
		info30ARC3 -> fillvalue = initializeInfo -> fillvalue;
		info30ARC3 -> dataType = initializeInfo -> dataType;
		strcpy(info30ARC3 -> gridName, initializeInfo -> gridName);
		strcpy(info30ARC3 -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;
          case PGSd_DEM_SUBSET90A:
            if (info90ARC1 != NULL)  /*i.e. it has already been initialized*/
            {
                info90ARC1 -> layer = initializeInfo -> layer;
                info90ARC1 -> layersInit += initializeInfo -> layer;
                info90ARC1 -> fillvalue = initializeInfo -> fillvalue;
                info90ARC1 -> dataType = initializeInfo -> dataType;
                strcpy(info90ARC1 -> gridName, initializeInfo -> gridName);
                strcpy(info90ARC1 -> fieldName, initializeInfo -> fieldName);
                free(initializeInfo);
                return (PGS_S_SUCCESS);
            }
            break;
          case PGSd_DEM_SUBSET90B:
            if (info90ARC2 != NULL)  /*i.e. it has already been initialized*/
            {
                info90ARC2 -> layer = initializeInfo -> layer;
                info90ARC2 -> layersInit += initializeInfo -> layer;
                info90ARC2 -> fillvalue = initializeInfo -> fillvalue;
                info90ARC2 -> dataType = initializeInfo -> dataType;
                strcpy(info90ARC2 -> gridName, initializeInfo -> gridName);
                strcpy(info90ARC2 -> fieldName, initializeInfo -> fieldName);
                free(initializeInfo);
                return (PGS_S_SUCCESS);
            }
            break;
          case PGSd_DEM_SUBSET90C:
            if (info90ARC3 != NULL)  /*i.e. it has already been initialized*/
            {
                info90ARC3 -> layer = initializeInfo -> layer;
                info90ARC3 -> layersInit += initializeInfo -> layer;
                info90ARC3 -> fillvalue = initializeInfo -> fillvalue;
                info90ARC3 -> dataType = initializeInfo -> dataType;
                strcpy(info90ARC3 -> gridName, initializeInfo -> gridName);
                strcpy(info90ARC3 -> fieldName, initializeInfo -> fieldName);
                free(initializeInfo);
                return (PGS_S_SUCCESS);
            }
            break;
	  case PGSd_DEM_SUBSET30TEST:
	    if (info30TEST != NULL)  /*i.e. it has already been initialized*/
	    {
		info30TEST -> layer = initializeInfo -> layer;
		info30TEST -> layersInit += initializeInfo -> layer;
		info30TEST -> fillvalue = initializeInfo -> fillvalue;
		info30TEST -> dataType = initializeInfo -> dataType;
		strcpy(info30TEST -> gridName, initializeInfo -> gridName);
		strcpy(info30TEST -> fieldName, initializeInfo -> fieldName);
		free(initializeInfo);
		return (PGS_S_SUCCESS);
	    }
	    break;
	  default:
	    /*ERROR improper logical ID and subset, subset is not presently
	      recognized by software for storage of info */
	    /*Set dynamic message to improper subset number*/
	    sprintf(dynamicMsg, "subset (%d) is not recognized.", 
		    initializeInfo -> subset);
	    status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					   "PGS_DEM_Subset()");
	    statusError = PGSDEM_E_IMPROPER_TAG;

	    /*fatal error*/
	    goto fatalError;
	    
	}

	/*If the subset has not already been initialized by another layer...*/

	/*Access subset (logical ID), check if subset is staged and get the
	  number of files staged*/
	status = PGS_PC_GetNumberOfFiles(initializeInfo -> subset, 
					 &numberFiles);
	if (status == PGSPC_W_NO_FILES_FOR_ID)
	{
	    /*ERROR Subset not found, ie. Logical ID not staged or improper ID*/
	    /*Set dynamic message to improper Logical ID*/
	    sprintf(dynamicMsg,"subset number (%d), PCF logical ID, not staged",
		    initializeInfo -> subset);
	    status = PGS_SMF_GetMsgByCode(PGSDEM_E_CANNOT_ACCESS_DATA, 
					  errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					   errorMsg, "PGS_DEM_Subset()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	    /*fatal error*/
	    goto fatalError;
	    
	}
    
	else if (status != PGS_S_SUCCESS)
	{
	    /*Some other PCF ERROR*/
	    statusError = status;

	    /*fatal error*/
	    goto fatalError;
	    
	}
    
	/*Check to see that numberFiles is reasonable */
	if ((numberFiles <= 0) || (numberFiles > 
				   (initializeInfo -> numSubgrids)))
	{
    	
	    /*ERROR DETERMINING NUMBER OF FILES STAGED IN SUBSET*/
	    /*Set dynamic message to improper number of files staged*/
	    sprintf(dynamicMsg, "(%d) not valid number of files staged in PCF for subset number (%d) (PCF logical ID)",numberFiles,initializeInfo -> subset);
	    status = PGS_SMF_GetMsgByCode(PGSDEM_E_CANNOT_ACCESS_DATA, 
					  errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					   errorMsg, "PGS_DEM_Subset()");
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	    /*fatal error*/
	    goto fatalError;
	    
	}
    
	initializeInfo -> versionLength = numberFiles;
	
	/*If the subset has not been initialized, need to set up space for the
	  individual fileRecords in the subset*/ 

	/*Calloc space for the array of subgrid pointer, the set of
	  fileRecord structures which make up the subset, and the orderedArray*/
 	subsetInitialize = (PGSt_DEM_FileRecord **) calloc(
              initializeInfo -> numSubgrids, sizeof(PGSt_DEM_FileRecord *));
	if (subsetInitialize == NULL)
	{
	    /*ERROR PROBLEMS CALLOCING*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "subsetInitialize");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					      dynamicMsg, 
					      "PGS_DEM_Subset()");
	   statusError = PGSMEM_E_NO_MEMORY;

	   /*fatal ERROR*/
	   goto fatalError;
	   
	}
	
	stagedFiles = (PGSt_DEM_FileRecord *) calloc(
           initializeInfo -> versionLength, sizeof(PGSt_DEM_FileRecord));
	if (stagedFiles == NULL)
	{
	    /*ERROR PROBLEMS CALLOCING*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "stagedFiles");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					      dynamicMsg, 
					      "PGS_DEM_Subset()");
	    statusError = PGSMEM_E_NO_MEMORY;

	    /*fatal ERROR*/
	    goto fatalError;

	}
	
	orderedArray = (PGSt_integer(*)[2]) calloc(initializeInfo -> versionLength,
			      2*sizeof(PGSt_integer));
	if (orderedArray == NULL)
	{
	    /*ERROR PROBLEMS CALLOCING*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "orderedArray");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					      dynamicMsg, 
					      "PGS_DEM_Subset()");
	    statusError = PGSMEM_E_NO_MEMORY;

	   /*fatal ERROR*/
	   goto fatalError;	    
	}
	

	/*Organize the files staged in ascending order of subgrid*/
	status = PGS_DEM_OrderSubset(initializeInfo, orderedArray);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR ordering the files by subgrid number*/
	    statusError = status;

	    /*fatal ERRor*/
	    goto fatalError;
	    
	}

	
	/*Populate the whole subset*/
	status = PGS_DEM_Populate(initializeInfo, stagedFiles, 
				  subsetInitialize,orderedArray);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR populating the subset*/
	    statusError = status;

	    /*fatal Error*/
	    goto fatalError;
	    
	}

	/*Store subset and subsetInfo in proper location*/
	/*this allows for later access by Info command*/
	/*set the first layer for layers initialized*/
	switch(initializeInfo -> subset)
	{
	  case PGSd_DEM_SUBSET3A:
	    info3ARC1 = initializeInfo;
	    info3ARC1 -> layersInit = initializeInfo -> layer;
	    subset3ARC1 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO3ARC1],
                                    info3ARC1);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC1],
                                    subset3ARC1);
#endif
	    break;
	  case PGSd_DEM_SUBSET3B:
	    info3ARC2 = initializeInfo;
	    info3ARC2 -> layersInit = initializeInfo -> layer;
	    subset3ARC2 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO3ARC2],
                                    info3ARC2);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC2],
                                    subset3ARC2);
#endif
	    break;
	  case PGSd_DEM_SUBSET3C:
	    info3ARC3 = initializeInfo;
	    info3ARC3 -> layersInit = initializeInfo -> layer;
	    subset3ARC3 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO3ARC3],
                                    info3ARC3);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC3],
                                    subset3ARC3);
#endif
	    break;	    
	  case PGSd_DEM_SUBSET15A:
	    info15ARC1 = initializeInfo;
	    info15ARC1 -> layersInit = initializeInfo -> layer;
	    subset15ARC1 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO15ARC1],
                                    info15ARC1);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET15ARC1],subset15ARC1);
#endif
	    break;
	  case PGSd_DEM_SUBSET15B:
	    info15ARC2 = initializeInfo;
	    info15ARC2 -> layersInit = initializeInfo -> layer;
	    subset15ARC2 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO15ARC2],info15ARC2);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET15ARC2],subset15ARC2);
#endif
	    break;
	  case PGSd_DEM_SUBSET15C:
	    info15ARC3 = initializeInfo;
	    info15ARC3 -> layersInit = initializeInfo -> layer;
	    subset15ARC3 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO15ARC3],info15ARC3);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET15ARC3],subset15ARC3);
#endif
	    break;
	  case PGSd_DEM_SUBSET30A:
	    info30ARC1 = initializeInfo;
	    info30ARC1 -> layersInit = initializeInfo -> layer;
	    subset30ARC1 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30ARC1],
                                    info30ARC1);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC1],subset30ARC1);
#endif
	    break;
	  case PGSd_DEM_SUBSET30B:
	    info30ARC2 = initializeInfo;
	    info30ARC2 -> layersInit = initializeInfo -> layer;
	    subset30ARC2 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30ARC2],info30ARC2);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC2],subset30ARC2);
#endif
	    break;
	  case PGSd_DEM_SUBSET30C:
	    info30ARC3 = initializeInfo;
	    info30ARC3 -> layersInit = initializeInfo -> layer;
	    subset30ARC3 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30ARC3],info30ARC3);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC3],subset30ARC3);
#endif
	    break;
          case PGSd_DEM_SUBSET90A:
            info90ARC1 = initializeInfo;
            info90ARC1 -> layersInit = initializeInfo -> layer;
            subset90ARC1 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO90ARC1],
                                    info90ARC1);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET90ARC1],subset90ARC1);
#endif
            break;
          case PGSd_DEM_SUBSET90B:
            info90ARC2 = initializeInfo;
            info90ARC2 -> layersInit = initializeInfo -> layer;
            subset90ARC2 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO90ARC2],info90ARC2);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET90ARC2],subset90ARC2);
#endif
            break;
          case PGSd_DEM_SUBSET90C:
            info90ARC3 = initializeInfo;
            info90ARC3 -> layersInit = initializeInfo -> layer;
            subset90ARC3 = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO90ARC3],info90ARC3);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET90ARC3],subset90ARC3);
#endif
            break;
	  case PGSd_DEM_SUBSET30TEST:
	    info30TEST = initializeInfo;
	    info30TEST -> layersInit = initializeInfo -> layer;
	    subset30TEST = subsetInitialize;
#ifdef _PGS_THREADSAFE
            /* Reset TSD keys */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30TEST],
                                    info30TEST);
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30TEST],
                                    subset30TEST);
#endif
	    break;
	  default:
	    /*ERROR improper logical ID and subset, subset is not presently
	      recognized by software for storage of info */
	    /*Set dynamic message to improper subset number*/
	    sprintf(dynamicMsg, "subset (%d) is not recognized.", 
		    initializeInfo -> subset);
	    status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					   "PGS_DEM_Subset()");
	    statusError = PGSDEM_E_IMPROPER_TAG;

	    /*fatal error*/
	    goto fatalError;
	    
	}


	/*free orderedArray*/
	free(orderedArray);
	
    	/*Successful completion*/

	return (PGS_S_SUCCESS);
	
    }
    else if (command == PGSd_DEM_DEINITIALIZE) /*Close (free) subset*/
    {
	status = PGS_DEM_Lookup(resolution, layer, &deinitializeInfo);
	if (status != PGS_S_SUCCESS)
	{
	    /*ERROR finding info*/
	    return (status);
	}
	
	/*subtract the layer from layer Initialized. In addition, check if layer
	 has ever been initialized*/
	/*if layers left == 0, hold the pointers to the subset and subsetInfo,
	  and reset the static pointers to NULL*/
	switch(deinitializeInfo.subset)
	{
	  case PGSd_DEM_SUBSET3A:
	    if (info3ARC1 !=NULL)
	    {
		info3ARC1 -> layersInit -= deinitializeInfo.layer;
		layersLeft = info3ARC1 -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info3ARC1;
		    closeSubset = subset3ARC1;
		    info3ARC1 = NULL;
		    subset3ARC1 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                       masterTSF->keyArray[PGSd_TSF_KEYDEMINFO3ARC1],info3ARC1);
                    pthread_setspecific(
                       masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC1],subset3ARC1);
#endif
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;
	  case PGSd_DEM_SUBSET3B:
	    if (info3ARC2 !=NULL)
	    {
		info3ARC2 -> layersInit -= deinitializeInfo.layer;
		layersLeft = info3ARC2 -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info3ARC2;
		    closeSubset = subset3ARC2;
		    info3ARC2 = NULL;
		    subset3ARC2 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                       masterTSF->keyArray[PGSd_TSF_KEYDEMINFO3ARC2],info3ARC2);
                    pthread_setspecific(
                       masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC2],subset3ARC2);
#endif      
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;
	  case PGSd_DEM_SUBSET3C:
	    if (info3ARC3 !=NULL)
	    {
		info3ARC3 -> layersInit -= deinitializeInfo.layer;
		layersLeft = info3ARC3 -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info3ARC3;
		    closeSubset = subset3ARC3;
		    info3ARC3 = NULL;
                    subset3ARC3 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                        masterTSF->keyArray[PGSd_TSF_KEYDEMINFO3ARC3],info3ARC3);
                    pthread_setspecific(
                        masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET3ARC3],subset3ARC3);
#endif
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;	    
	  case PGSd_DEM_SUBSET15A:
	    if (info15ARC1 !=NULL)
	    {
		info15ARC1 -> layersInit -= deinitializeInfo.layer;
		layersLeft = info15ARC1 -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info15ARC1;
		    closeSubset = subset15ARC1;
		    info15ARC1 = NULL;
		    subset15ARC1 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                         masterTSF->keyArray[PGSd_TSF_KEYDEMINFO15ARC1],info15ARC1);
                    pthread_setspecific(
                         masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET15ARC1],subset15ARC1);
#endif
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;
	  case PGSd_DEM_SUBSET15B:
	    if (info15ARC2 !=NULL)
	    {
		info15ARC2 -> layersInit -= deinitializeInfo.layer;
		layersLeft = info15ARC2 -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info15ARC2;
		    closeSubset = subset15ARC2;
		    info15ARC2 = NULL;
		    subset15ARC2 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMINFO15ARC2],info15ARC2);
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET15ARC2],subset15ARC2);
#endif
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;
	  case PGSd_DEM_SUBSET15C:
	    if (info15ARC3 !=NULL)
	    {
		info15ARC3 -> layersInit -= deinitializeInfo.layer;
		layersLeft = info15ARC3 -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info15ARC3;
		    closeSubset = subset15ARC3;
		    info15ARC3 = NULL;
		    subset15ARC3 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMINFO15ARC3],info15ARC3);
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET15ARC3],subset15ARC3);
#endif
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;	    
	  case PGSd_DEM_SUBSET30A:
	    if (info30ARC1 !=NULL)
	    {
		info30ARC1 -> layersInit -= deinitializeInfo.layer;
		layersLeft = info30ARC1 -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info30ARC1;
		    closeSubset = subset30ARC1;
		    info30ARC1 = NULL;
		    subset30ARC1 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                         masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30ARC1],info30ARC1);
                    pthread_setspecific(
                         masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC1],subset30ARC1);
#endif
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;
	  case PGSd_DEM_SUBSET30B:
	    if (info30ARC2 !=NULL)
	    {
		info30ARC2 -> layersInit -= deinitializeInfo.layer;
		layersLeft = info30ARC2 -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info30ARC2;
		    closeSubset = subset30ARC2;
		    info30ARC2 = NULL;
		    subset30ARC2 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30ARC2],info30ARC2);
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC2],subset30ARC2);
#endif
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;
	  case PGSd_DEM_SUBSET30C:
	    if (info30ARC3 !=NULL)
	    {
		info30ARC3 -> layersInit -= deinitializeInfo.layer;
		layersLeft = info30ARC3 -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info30ARC3;
		    closeSubset = subset30ARC3;
		    info30ARC3 = NULL;
		    subset30ARC3 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30ARC3],info30ARC3);
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30ARC3],subset30ARC3);
#endif
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;
          case PGSd_DEM_SUBSET90A:
            if (info90ARC1 !=NULL)
            {
                info90ARC1 -> layersInit -= deinitializeInfo.layer;
                layersLeft = info90ARC1 -> layersInit;
                if (layersLeft == 0)
                {
                    closeSubsetInfo = info90ARC1;
                    closeSubset = subset90ARC1;
                    info90ARC1 = NULL;
                    subset90ARC1 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                         masterTSF->keyArray[PGSd_TSF_KEYDEMINFO90ARC1],info90ARC1);
                    pthread_setspecific(
                         masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET90ARC1],subset90ARC1);
#endif
                }
            }
            else
            {
                layersLeft = PGSd_DEM_ABSENT;
            }
            break;
          case PGSd_DEM_SUBSET90B:
            if (info90ARC2 !=NULL)
            {
                info90ARC2 -> layersInit -= deinitializeInfo.layer;
                layersLeft = info90ARC2 -> layersInit;
                if (layersLeft == 0)
                {
                    closeSubsetInfo = info90ARC2;
                    closeSubset = subset90ARC2;
                    info90ARC2 = NULL;
                    subset90ARC2 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMINFO90ARC2],info90ARC2);
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET90ARC2],subset90ARC2);
#endif
                }
            }
            else
            {
                layersLeft = PGSd_DEM_ABSENT;
            }
            break;
          case PGSd_DEM_SUBSET90C:
            if (info90ARC3 !=NULL)
            {
                info90ARC3 -> layersInit -= deinitializeInfo.layer;
                layersLeft = info90ARC3 -> layersInit;
                if (layersLeft == 0)
                {
                    closeSubsetInfo = info90ARC3;
                    closeSubset = subset90ARC3;
                    info90ARC3 = NULL;
                    subset90ARC3 = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMINFO90ARC3],info90ARC3);
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET90ARC3],subset90ARC3);
#endif
                }
            }
            else
            {
                layersLeft = PGSd_DEM_ABSENT;
            }
            break;
	  case PGSd_DEM_SUBSET30TEST:
	    if (info30TEST !=NULL)
	    {
		info30TEST -> layersInit -= deinitializeInfo.layer;
		layersLeft = info30TEST -> layersInit;
		if (layersLeft == 0)
		{
		    closeSubsetInfo = info30TEST;
		    closeSubset = subset30TEST;
		    info30TEST = NULL;
                    subset30TEST = NULL;
#ifdef _PGS_THREADSAFE
                    /* Reset TSD keys */
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMINFO30TEST],info30TEST);
                    pthread_setspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYDEMSUBSET30TEST],subset30TEST);
#endif
		}
	    }
	    else
	    {
		layersLeft = PGSd_DEM_ABSENT;
	    }
	    break;
	  default:
	    /*ERROR improper logical ID and subset, subset is not presently
	      recognized by software for storage of info */
	    /*Set dynamic message to improper subset number*/
	    sprintf(dynamicMsg, "subset (%d) is not recognized.", 
		    initializeInfo -> subset);
	    status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					   "PGS_DEM_Subset()");
	    return(PGSDEM_E_IMPROPER_TAG);
	}


	if (layersLeft == 0)
	{
	    /*close up the whole subset-- including subsetInfo.  This
	      means that one needs to detach from and close each
	      HDF-EOS file in that subset (i.e. call
	      PGS_DEM_AccessFile(_CLOSE).  After this is completed,
	      free all of the allocated space. */

	    /*Detach and close files*/
	    status = PGS_DEM_AccessFile(closeSubsetInfo, closeSubset, PGSd_DEM_SUBGRID_NULL,
					PGSd_DEM_CLOSE);
	    if (status != PGS_S_SUCCESS)
	    {
		/*ERROR closing and detaching from HDF-EOS files*/
		sprintf(dynamicMsg, "Cannot access data..."
			"Unable to close all of HDF-EOS files "
			"through PGS_DEM_AccessFile.");
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					       dynamicMsg, "PGS_DEM_Subset");
		
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

		/*fatal error*/
		goto fatalError;
	    }

	    /*free the array of pointers that make up subset and subsetInfo*/
	    free(closeSubset[closeSubsetInfo -> firstSubgrid]);
	    free(closeSubset);
	    free(closeSubsetInfo);
	}
	else if ((layersLeft > 0) && (layersLeft != PGSd_DEM_ABSENT))
	{
	    /*return success, don't yet need to close the layer's
	      subset. other's layers which share the subset are still
	      active. */
	    return(PGS_S_SUCCESS);
	}

	else
	{
	    /*ERROR subset has not been initialized*/
	}
	
	return(PGS_S_SUCCESS);
	
 
    }
    else if (command == PGSd_DEM_INFO)         /*Retrieve subset*/
    {
	/*Calloc space for getInfo*/
	getInfo = (PGSt_DEM_SubsetRecord *) calloc(1,
                            sizeof(PGSt_DEM_SubsetRecord));
	if (getInfo == NULL)
	{
	    /*ERROR callocing space for getInfo SubsetRecord*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "getInfo");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					      dynamicMsg, 
					      "PGS_DEM_Subset()");
	    statusError = PGSMEM_E_NO_MEMORY;
	    return(PGSMEM_E_NO_MEMORY);
	}
	
	/*Determine appropriate subset from Resolution and layer*/
	status = PGS_DEM_Lookup(resolution, layer, getInfo);
	if (status != PGS_S_SUCCESS)
	{
	    statusError = status;

	    /*fatal error*/
	    goto fatalError;
	    
	}

	/*Access appropriate subset*/
	/*assign output pointers to the subset and subset info, copy specific
	  layer number, fill value, grid name and field name t*/
	switch(getInfo -> subset)
	{
	  case PGSd_DEM_SUBSET3A:
	    /*Check if subset has already been initialized*/
	    if (subset3ARC1 == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal Error*/
		goto fatalError;
		
	    }
	    *subset = subset3ARC1;
	    *subsetInfo = info3ARC1;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;
	  case PGSd_DEM_SUBSET3B:
	    /*Check if subset has already been initialized*/
	    if (subset3ARC2 == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal error*/
		goto fatalError;
		
	    }
	    *subset = subset3ARC2;
	    *subsetInfo = info3ARC2;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;
	  case PGSd_DEM_SUBSET3C:
	    /*Check if subset has already been initialized*/
	    if (subset3ARC3 == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal error*/
		goto fatalError;

	    }
	    *subset = subset3ARC3;
	    *subsetInfo = info3ARC3;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;	    
	  case PGSd_DEM_SUBSET15A:
	    /*Check if subset has already been initialized*/
	    if (subset15ARC1 == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal error*/
		goto fatalError;

	    }
	    *subset = subset15ARC1;
	    *subsetInfo = info15ARC1;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;
	  case PGSd_DEM_SUBSET15B:
	    /*Check if subset has already been initialized*/
	    if (subset15ARC2 == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal error*/
		goto fatalError;

	    }
	    *subset = subset15ARC2;
	    *subsetInfo = info15ARC2;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;
	  case PGSd_DEM_SUBSET15C:
	    /*Check if subset has already been initialized*/
	    if (subset15ARC3 == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal error*/
		goto fatalError;

	    }
	    *subset = subset15ARC3;
	    *subsetInfo = info15ARC3;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;
	  case PGSd_DEM_SUBSET30A:
	    /*Check if subset has already been initialized*/
	    if (subset30ARC1 == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal error*/
		goto fatalError;

	    }
	    *subset = subset30ARC1;
	    *subsetInfo = info30ARC1;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;
	  case PGSd_DEM_SUBSET30B:
	    /*Check if subset has already been initialized*/
	    if (subset30ARC2 == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal error*/
		goto fatalError;

	    }
	    *subset = subset30ARC2;
	    *subsetInfo = info30ARC2;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;
	  case PGSd_DEM_SUBSET30C:
	    /*Check if subset has already been initialized*/
	    if (subset30ARC3 == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal error*/
		goto fatalError;

	    }
	    *subset = subset30ARC3;
	    *subsetInfo = info30ARC3;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;
          case PGSd_DEM_SUBSET90A:
            /*Check if subset has already been initialized*/
            if (subset90ARC1 == NULL)
            {
                /*Set dynamic message to improper subset number*/
                sprintf(dynamicMsg, "subset (%d) has not been initialized", 
                        getInfo -> subset);
                status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
                sprintf(errorMsg, errorBuf, dynamicMsg);
                status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
                                               "PGS_DEM_Subset()");
                statusError = PGSDEM_E_IMPROPER_TAG;

                /*fatal error*/
                goto fatalError;

            }
            *subset = subset90ARC1;
            *subsetInfo = info90ARC1;
            (*subsetInfo) -> layer = getInfo -> layer;
            (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
            (*subsetInfo) -> dataType = getInfo -> dataType;
            strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
            strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
            break;
          case PGSd_DEM_SUBSET90B:
            /*Check if subset has already been initialized*/
            if (subset90ARC2 == NULL)
            {
                /*Set dynamic message to improper subset number*/
                sprintf(dynamicMsg, "subset (%d) has not been initialized", 
                        getInfo -> subset);
                status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
                sprintf(errorMsg, errorBuf, dynamicMsg);
                status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
                                               "PGS_DEM_Subset()");
                statusError = PGSDEM_E_IMPROPER_TAG;

                /*fatal error*/
                goto fatalError;

            }
            *subset = subset90ARC2;
            *subsetInfo = info90ARC2;
            (*subsetInfo) -> layer = getInfo -> layer;
            (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
            (*subsetInfo) -> dataType = getInfo -> dataType;
            strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
            strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
            break;
          case PGSd_DEM_SUBSET90C:
            /*Check if subset has already been initialized*/
            if (subset90ARC3 == NULL)
            {
                /*Set dynamic message to improper subset number*/
                sprintf(dynamicMsg, "subset (%d) has not been initialized", 
                        getInfo -> subset);
                status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
                sprintf(errorMsg, errorBuf, dynamicMsg);
                status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
                                               "PGS_DEM_Subset()");
                statusError = PGSDEM_E_IMPROPER_TAG;

                /*fatal error*/
                goto fatalError;

            }
            *subset = subset90ARC3;
            *subsetInfo = info90ARC3;
            (*subsetInfo) -> layer = getInfo -> layer;
            (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
            (*subsetInfo) -> dataType = getInfo -> dataType;
            strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
            strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
            break;
	  case PGSd_DEM_SUBSET30TEST:
	    /*Check if subset has already been initialized*/
	    if (subset30TEST == NULL)
	    {
		/*Set dynamic message to improper subset number*/
		sprintf(dynamicMsg, "subset (%d) has not been initialized", 
			getInfo -> subset);
		status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
		sprintf(errorMsg, errorBuf, dynamicMsg);
		status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					       "PGS_DEM_Subset()");
		statusError = PGSDEM_E_IMPROPER_TAG;

		/*fatal error*/
		goto fatalError;

	    }
	    *subset = subset30TEST;
	    *subsetInfo = info30TEST;
	    (*subsetInfo) -> layer = getInfo -> layer;
	    (*subsetInfo) -> fillvalue = getInfo -> fillvalue;
	    (*subsetInfo) -> dataType = getInfo -> dataType;
	    strcpy((*subsetInfo) -> gridName, getInfo -> gridName);
	    strcpy((*subsetInfo) -> fieldName, getInfo -> fieldName);
	    break;
	  default:
	    /*ERROR improper logical ID and subset, subset is not presently
	      recognized by software for storage of info */
	    /*Set dynamic message to improper subset number*/
	    sprintf(dynamicMsg, "subset (%d) is not recognized.", 
		    getInfo -> subset);
	    status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
					   "PGS_DEM_Subset()");
	    statusError = PGSDEM_E_IMPROPER_TAG;

	    /*fatal error*/
	    goto fatalError;



	}

	/*free up temporary subset info*/
	free(getInfo);
	
	/*Successful completion*/
	return (PGS_S_SUCCESS);

    }

    /*Quality Info*/

    else if (command == PGSd_DEM_QUALITYINFO)       
    {
	/*Calloc space for initializeInfo*/
	qualityInfo = (PGSt_DEM_SubsetRecord *) calloc(1,
                                sizeof(PGSt_DEM_SubsetRecord));
	if (qualityInfo == NULL)
	{
	    /*ERROR callocing space for initialize SubsetRecord*/
	    sprintf(dynamicMsg, "Error allocating memory for "
		    "qualityInfo");
	    PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					      dynamicMsg, 
					      "PGS_DEM_Subset()");
	    statusError = PGSMEM_E_NO_MEMORY;
	    return(PGSMEM_E_NO_MEMORY);
	}
	
	/*Lookup specific on Quality layer. notice, instead of using resolution,
	  use the QUALITYINFO flag.  this is because quality data has unique
	  diagnostics which are not resolution specific. */
	status = PGS_DEM_Lookup(PGSd_DEM_QUALITYINFO, layer, qualityInfo);
	if (status != PGS_S_SUCCESS)
	{
	    statusError = status;

	    /*fatal error*/
	    goto fatalError;
	    
	}

	/*Check if subsetInfo has already been initialized, if so, only need to
	  transfer some elements which are layer dependent from qualityInfo
	  to the stored subsetInfo.*/
	if (infoQUALITY != NULL) /*it has already been initialized */
	{
	    infoQUALITY -> layer = qualityInfo -> layer;
	    infoQUALITY -> dataType = qualityInfo -> dataType;
	    infoQUALITY -> numBytes = qualityInfo -> numBytes;
	    strcpy(infoQUALITY -> gridName, qualityInfo -> gridName);
	    strcpy(infoQUALITY -> fieldName, qualityInfo -> fieldName);
	}
	else
	{
	    
	    /*store subset Info in the proper location, this allows later access
	      of quality Info after the function is called, beforethe field is
	      closed */
	    infoQUALITY = qualityInfo;
#ifdef _PGS_THREADSAFE
            /* Reset TSD key */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFOQUALITY],
                                 infoQUALITY);
#endif

	}
	

	/*return appropriate qualityInfo pointer*/
	*subsetInfo = infoQUALITY;
	

	return(PGS_S_SUCCESS);
	
    }
    
    /**QUALITY CLOSE*/
    else if(command == PGSd_DEM_QUALITYCLOSE)
    {
	/*close down the quality/source/geoid info.  Deallocate the memory and
	  set the static pointer back to NULL.  If the inputed pointer is not
	  equal to NULL, take the dereference of that pointer, the address, and
	  set that also to NULL.*/
	if (subsetInfo != NULL)
	{
	    *subsetInfo = NULL;
	}

	/*Check if qualityInfo has been allocated*/
	if (infoQUALITY == NULL)
	{
	    /*Error, space has not previously been allocated*/
	    sprintf(dynamicMsg, "Improper Tag... Cannot deallocate "
		    "space for infoQuality because no space has previously " 
		    "been allocated.");
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, dynamicMsg,
					   "PGS_DEM_Subset()");
	    
	    statusError = PGSDEM_E_IMPROPER_TAG;
	  
	    /*fatal error*/
	    goto fatalError;
	}
	
	/*free allocated space and set static pointer to NULL*/
	free(infoQUALITY);
	infoQUALITY = NULL;
#ifdef _PGS_THREADSAFE
            /* Reset TSD key */
            pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYDEMINFOQUALITY],
                                infoQUALITY);
#endif
	
	return(PGS_S_SUCCESS);
    }
    

    else
    {
	/*ERROR wrong command*/
	/*Set dynamic message to improper command*/
	sprintf(dynamicMsg, "command is not recognized.");
	status = PGS_SMF_GetMsgByCode(PGSDEM_E_IMPROPER_TAG, errorBuf);
	sprintf(errorMsg, errorBuf, dynamicMsg);
	status = PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG, errorMsg,
				       "PGS_DEM_Subset()");
	statusError = PGSDEM_E_IMPROPER_TAG;
	
	/*fatal error*/
	goto fatalError;

    }
    
fatalError:
    {
	/*fatal errors different for each command, need to shut down different
	  aspects*/ 
	if (command == PGSd_DEM_INITIALIZE)
	{
	    
	    /*free up all data buffers*/
if (initializeInfo != NULL)
{
            free(initializeInfo);
            initializeInfo = NULL;
}
if (subsetInitialize != NULL)
{
	    free(subsetInitialize);
            subsetInitialize = NULL;
}
if (stagedFiles != NULL)
{
	    free(stagedFiles);
            stagedFiles = NULL;
}
if (orderedArray != NULL)
{
	    free(orderedArray);
            orderedArray = NULL;
}

	}
	
	else if (command == PGSd_DEM_INFO)
	{	    
	    
	    /*free up all data buffers*/
	    free(getInfo);
	}
	
	return (statusError);

    }
    
}
