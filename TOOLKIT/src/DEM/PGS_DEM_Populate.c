/*******************************************************
PGS_DEM_Populate.c--

This function populates the subset.   It uses the orderedArray to determine
which subgrid entries are to be populated.  Only those files which are staged on
the PCF will be populated.  It directs a pointer for each subgrd to some
allocated space (stagedFiles). At thsi point it opens each file, and populates
in the information into the PGS_DEM_FileRecord for that particular file.


Author -- Alexis Zubrow

history --
February 3, 1997    AZ   First Created
July  2, 1997       AZ   Added PGS_DEM_AccessFile
July  8, 1999       SZ   Updated for the thread-safe functionality

*******************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <PGS_DEM.h>
#include <PGS_PC.h> 
#include <mfhdf.h>
#include <HdfEosDef.h>
#include <PGS_TSF.h>

PGSt_SMF_status
PGS_DEM_Populate(
    PGSt_DEM_SubsetRecord *subsetInfo,  /*Info on particular subset*/
    PGSt_DEM_FileRecord *stagedFiles,   /*space for populated _FileRecords*/
    PGSt_DEM_FileRecord **subset,       /*Subset to populate*/
    PGSt_integer orderedArray[][2])        /*Array files organized by subgrid*/

{
    PGSt_integer i;              /*looping index*/
    PGSt_integer subgridValue;   /*subgrid number from HDF-EOS attribute*/
    
    PGSt_SMF_status statusSMF;      /*status return for PGS tools*/
    PGSt_SMF_status statusReturn;   /*status return for PGS_DEM_Populate*/
    PGSt_SMF_status statusPGS;      /*status return for PGS_tools*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    
    PGSt_integer version;        /*Input to PC functions, version #, DO NOT USE
				   AS LOOPING INDEX, PC functions modify version
				   value */ 
    
    
    /*HDF-EOS interface*/
    intn status;             /*status return for HDF-EOS tools*/
    int32 hdfID = PGSd_DEM_HDF_CLOSE;             /*HDF file handle*/
    int32 gdID = PGSd_DEM_HDF_CLOSE;              /*HDF-EOS GRID handle*/
    int32 subgridValueHdf;   /*HDF-EOS subgrid value*/
    int32 pixLat[2];         /*corner pix. latitude, 0th element (upper left)*/
    int32 pixLon[2];         /*corner pix. longitude, 0th element upper left*/
    int32 subsetValue; /*subset number from HDF-EOS attribute*/
    int32  resolution;    /*resolution tag from HDF-EOS attribute*/
    float64 cornLatitude[2];   /*corner latitude, decimal degrees*/
    float64 cornLongitude[2];  /*corner longitude, decimal degrees*/

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif
    
    /*loop through number of staged files in PCF*/
    for (i = 0; i < (subsetInfo ->versionLength); i++)
    {
	/*get subgrid value from ordered Array*/
	subgridValue = (orderedArray[i])[0];

	/*record the first staged subgrid in the subsetInfo*/
	if (i == 0)
	{
	    subsetInfo -> firstSubgrid = subgridValue;
	}
	
	/*Point the particular fileRecord pointer (in the subset "array") 
	  to the space delineated for the struct.  Therefore, only those files
	  which are staged, will have non-NULL pointers in the subset "array"*/
	subset[subgridValue] = (stagedFiles + i);
	
	/* use version variable so one doesn't corrupt the value of
	   orderedArray[i][1] */
	version = orderedArray[i][1];
	
	/*Populate the new fileRecord struct with the complete physical name*/
	statusSMF = PGS_PC_GetReference(subsetInfo -> subset,
					&version,
					subset[subgridValue] -> filePath);
	if (statusSMF !=PGS_S_SUCCESS)
	{
	    return (statusSMF);
	}

	

	/*Open HDF-EOS file and attach to grid*/
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	hdfID = GDopen(subset[subgridValue] -> filePath, DFACC_RDONLY);
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (hdfID == FAIL)
	{
	    /*ERROR OPENING FILE, subset #, subgrid #*/
	    sprintf(dynamicMsg, "Cannot access data "
		    "HDF file (%s), not properly staged",
		    subset[subgridValue] -> filePath);
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, "PGS_DEM_Populate()");
	    
	    statusReturn = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	    
	}
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	gdID = GDattach(hdfID, subsetInfo -> gridName);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (gdID == FAIL)
	{
	    /*ERROR attaching to grid*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      "Cannot attach to GRID", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_HDFEOS;

	    /*fatal error*/
	    goto fatalError;
	    
	}
	
	/*Get attribute information*/
	/*subgrid number--check*/
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	status = GDreadattr(gdID, "_subgrid", &subgridValueHdf);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (status == FAIL)
	{
	    /*ERROR getting subgrid attribute*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      "Cannot access subgrid attr.", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_HDFEOS;

	    /*fatal error*/
	    goto fatalError;

	}
	if (subgridValueHdf != subgridValue)
	{
	    /*ERROR subgrid values don;t correspond*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      "Wrong subgrid value recorded", 
					      "PGS_DEM_Populate()");

	    statusReturn = PGSDEM_E_CANNOT_ACCESS_DATA;

	    /*fatal error*/
	    goto fatalError;
	    
	}
	
	/*Check to see if resolution, subset correspond-- attempt to
	  detect corruption of file*/
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	status = GDreadattr(gdID, "_resolution", &resolution);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (status == FAIL)
	{
	    /*ERROR getting resolution tag attribute*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      "Cannot access resolution attr.", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_HDFEOS;

	    /*fatal error*/
	    goto fatalError;

	}
	
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	status = GDreadattr(gdID, "_subsetNumber", &subsetValue);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (status == FAIL)
	{
	    /*ERROR getting subset number attribute*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      "Cannot access subset attribute", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_HDFEOS;

	    /*fatal error*/
	    goto fatalError;

	}

	if (resolution != (subsetInfo -> resolutionTag))
	{
	    /*ERROR corruption, wrong resolution*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      "Wrong resolution recorded "
					      "in HDF-EOS file", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_CANNOT_ACCESS_DATA;

	    /*fatal error*/
	    goto fatalError;
	}
	if (subsetValue != (subsetInfo -> subset))
	{
	    /*ERROR corruption, wrong subset number*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      "Wrong subset number recorded "
					      "in HDF-EOS file",
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_CANNOT_ACCESS_DATA;

	    /*fatal error*/
	    goto fatalError;
	}
	

	/*Get pixel and degree decimal upper left and lower right 
	  corner values*/
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	status = GDreadattr(gdID, "_pixelLat", pixLat);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (status == FAIL)
	{
	    /*ERROR getting pixel latitude attribute*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      "Cannot access pixel latitude "
					      "attribute", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_HDFEOS;

	    /*fatal error*/
	    goto fatalError;
	}

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	status = GDreadattr(gdID, "_pixelLon", pixLon);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (status == FAIL)
	{
	    /*ERROR getting pixel longitude attribute*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      "Cannot access pixel longitude "
					      "attribute", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_HDFEOS;

	    /*fatal error*/
	    goto fatalError;
	}

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	status = GDreadattr(gdID, "_cornerLatitude", cornLatitude);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (status == FAIL)
	{
	    /*ERROR getting decimal degree latitude attribute*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      "Cannot access decimal "
					      "degree latitude attribute", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_HDFEOS;

	    /*fatal error*/
	    goto fatalError;
	}

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	status = GDreadattr(gdID, "_cornerLongitude", cornLongitude);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (status == FAIL)
	{
	    /*ERROR getting decimal degree longitude attribute*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS,
					      "Cannot access decimal "
					      "degree longitude attribute", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_HDFEOS;

	    /*fatal error*/
	    goto fatalError;
	}
	
	/*Assign diagnostics to fileRecord*/
	subset[subgridValue] -> subgrid = subgridValueHdf;
	subset[subgridValue] -> version = orderedArray[i][1];
	subset[subgridValue] -> cornerRow[0] = pixLat[0];
	subset[subgridValue] -> cornerRow[1] = pixLat[1];
	subset[subgridValue] -> cornerCol[0] = pixLon[0];
	subset[subgridValue] -> cornerCol[1] = pixLon[1];
	subset[subgridValue] -> cornerLat[0] = cornLatitude[0];
	subset[subgridValue] -> cornerLat[1] = cornLatitude[1];
	subset[subgridValue] -> cornerLon[0] = cornLongitude[0];
	subset[subgridValue] -> cornerLon[1] = cornLongitude[1];
	subset[subgridValue] -> hdfID = hdfID;
	subset[subgridValue] -> gdID = gdID;
		
	/*One last check of subgrid number-- recalculate and compare*/
	subgridValueHdf = PGSm_DEM_PixToSubgrid(pixLat[0], pixLon[0],
						subsetInfo);

	if ((subset[subgridValue] -> subgrid) != subgridValueHdf)
	{
	    /*ERROR corrupt file, wrong subgrid value*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      "Wrong subgrid value recorded", 
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_CANNOT_ACCESS_DATA;

	    /*fatal error*/
	    goto fatalError;
	}

	/*Assign attached File to list*/
	statusPGS = PGS_DEM_AccessFile(subsetInfo, subset, subgridValue,
				       PGSd_DEM_ASSIGN);
	if (statusPGS != PGS_S_SUCCESS)
	{
	    /*ERROR Opening and accessing data*/
	    statusSMF = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					      "Cannot access and attach to file"
					      "PGS_DEM_AccessFile failed.",
					      "PGS_DEM_Populate()");
	    statusReturn = PGSDEM_E_CANNOT_ACCESS_DATA;	    

	    /*fatal error*/
	    goto fatalError;
	}

	
    }

    return(PGS_S_SUCCESS);

    /*IF any problems with the opening or accessing the attributes of the
      HDF_EOS file, close the file and return error return*/
fatalError:
    {
	statusPGS = PGS_DEM_AccessFile(subsetInfo, subset, subgridValue,
				       PGSd_DEM_CLOSE);
	
	return (statusReturn);
    }
    
}
