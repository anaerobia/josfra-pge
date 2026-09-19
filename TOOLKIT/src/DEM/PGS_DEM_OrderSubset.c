/*******************************************************
PGS_DEM_OrderSubset.c--

This function takes a particular subset and opens up each of the staged files in
that subset.  It retrieves the subgrid number from the attributes of the dem
GRID, and records the subgrid value and corresponding file version number in a
two dimensional array.  The array is then ordered in ascending order of subgrid numbers.

Author -- Alexis Zubrow

history --
first created    February 4, 1997
Updated for the thread-safe functionality   July 8, 1999
Updated for C++ July 15, 2002  (Phuong T. Nguyen)
modified for ncr37870  Sept 12, 2003 (Adura Adekunjo)
*******************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <PGS_DEM.h>
#include <PGS_PC.h>
#include <mfhdf.h>
#include <HdfEosDef.h>
#include <PGS_TSF.h>

#ifdef __cplusplus
    static int PGS_DEM_Compare2DArray(const void *, const void *);
#endif

PGSt_SMF_status
PGS_DEM_OrderSubset(
    PGSt_DEM_SubsetRecord *subsetInfo,  /*Pointer to structure of subset info*/
    PGSt_integer orderedArray[][2])    /*Pointer to ordered array of staged
					  files in the particular subset*/
{

    char filePath[PGSd_PC_FILE_PATH_MAX]; /*full name of file, including path*/
    PGSt_integer i;                       /*counter*/
    PGSt_integer version;                 /*version of particular subset*/ 

    /*return status*/    
    PGSt_SMF_status statusReturn;         /*status returned- PGS_PC function*/
    PGSt_SMF_status statusError;         /*error status- PGS_DEM_OrderSubset*/
    char errorBuf[PGS_SMF_MAX_MSG_SIZE]; /*Strings appended to error returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    char errorMsg[PGS_SMF_MAX_MSGBUF_SIZE];

    /*HDF-EOS interface*/
    intn status;             /*status return for HDF-EOS tools*/
    int32 hdfID;             /*HDF file handle*/
    int32 gdID;              /*HDF-EOS GRID handle*/
    int32 subgridValueHdf;   /*HDF-EOS subgrid value*/
   
    /*comparison function in qsort-- quick sort*/
    int PGS_DEM_Compare2DArray(const void *, const void *);

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif
    
    for(i = 0; i < (subsetInfo -> versionLength); i++)
    {
	version = i+1;
	statusReturn = PGS_PC_GetReference(subsetInfo -> subset, &version,
					   filePath);
	if (statusReturn == PGSPC_W_NO_FILES_FOR_ID)
	{
	    /*ERROR Subset not found, ie. Logical ID not staged or improper ID*/
	    /*Set dynamic message to improper Logical ID*/
	    sprintf(dynamicMsg, "subset number (%d) (logical ID), not staged",
		    subsetInfo -> subset);
	    status = PGS_SMF_GetMsgByCode(PGSDEM_E_CANNOT_ACCESS_DATA, 
					  errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					   errorMsg, "PGS_DEM_OrderSubset()");

	    return(PGSDEM_E_CANNOT_ACCESS_DATA);
	}
	
	else if (statusReturn != PGS_S_SUCCESS)
	{
	    /*Some other PCF ERROR*/
	    return (status);
	}

	/*Open HDF file and access dem grid */
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */ 
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	hdfID = GDopen(filePath, DFACC_RDONLY);
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (hdfID == FAIL)
	{
	    /*ERROR HDF file, version i+1, not properly staged*/
	    /*Set dynamic message to hdf file name*/
	    sprintf(dynamicMsg, "HDF file (%s), not properly staged",
		    filePath);
	    status = PGS_SMF_GetMsgByCode(PGSDEM_E_CANNOT_ACCESS_DATA, 
					  errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					   errorMsg, "PGS_DEM_OrderSubset()");
	    
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

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
	gdID = GDattach(hdfID, "demGRID");
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (gdID == FAIL)
	{
	    /*ERROR HDF GRID not properly written*/
	    /*Set dynamic message to hdf file name*/
	    sprintf(dynamicMsg, "GRID not properly written");
	    status = PGS_SMF_GetMsgByCode(PGSDEM_E_CANNOT_ACCESS_DATA, 
					  errorBuf);
	    sprintf(errorMsg, errorBuf, dynamicMsg);
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA, 
					   errorMsg, "PGS_DEM_OrderSubset()");
	    
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;

	    /*fatal error*/
	    goto fatalError;
	    
	}

	/*Read subgrid number from the attributes of dem GRID*/
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
	    status = PGS_SMF_SetDynamicMsg(PGSDEM_E_HDFEOS, 
					   "Cannot read subgrid attribute"
					   " from HDF-EOS file",
					   "PGS_DEM_OrderSubset()");
	    
	    statusError = PGSDEM_E_HDFEOS;
	    /*fatal Error*/
	    goto fatalError;
	    
	}
	
	/*Assign subgrid number and version number to orderedArray*/
	(orderedArray[i])[0] = subgridValueHdf;
	(orderedArray[i])[1] = i+1;   /*version number of logicalID*/


	/*detach and close HDF file*/
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	status = GDdetach(gdID);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (status == FAIL)
	{
	    /*ERROR datching from grid*/
	    statusError = PGSDEM_E_HDFEOS;

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
	status = GDclose(hdfID);
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	if (status == FAIL)
	{
	    /*ERROR closing file*/
	    statusError = PGSDEM_E_HDFEOS;

	    /*fatal error*/
	    goto fatalError;
	    
	}
	
    }

    /*organize in ascending value of subgrid number*/
    qsort((void *)orderedArray, subsetInfo -> versionLength, 2*sizeof(int), 
	  PGS_DEM_Compare2DArray);

    /*return SUCCESS*/
    return (PGS_S_SUCCESS);

fatalError:
    {
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
if (hdfID != FAIL)
{
      if (gdID != FAIL)
      {
	GDdetach(gdID);
      }
	GDclose(hdfID);
}

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	return (statusError);
    }
    
}

/*comparison program for qsort*/\
int PGS_DEM_Compare2DArray(const void *qpointer, const void *ypointer)
{
    int *apointer, *bpointer;
    
    apointer = (int *)qpointer;
    bpointer = (int *)ypointer;
    
    if (apointer[0] == bpointer[0])
    {
	return(0);
    }
    else if (apointer[0] < bpointer[0])
    {
	return(-1);
    }
    else
    {
	return(1);
    }
}
