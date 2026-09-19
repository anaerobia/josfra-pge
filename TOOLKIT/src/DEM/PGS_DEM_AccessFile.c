/*******************************************************
PGS_DEM_AccessFile--

This function opens, closes and keeps a record of the staged files. It's primary
purpose is to improve the efficiency of the DEM tools.  Previously, everytime a
file was accessed, it needed to be opened and the GRID needed to be attached.
This function allows these files to be opened and the GRIDS to be attached only
once in the DEM functionality.  There are three modes, or commands, which
regulate the functionality of PGS_DEM_AccessFile:

PGSd_DEM_ACCESS -- In this case, the particular file has been opened
and the GRID has been attached outside of this function.  The hdfID and
gdID's already have been assigned to the particular subgrid of the subset. In
this case, the function merely records the file specific info.

PGSd_DEM_OPEN -- This function determines if a file has been opened and
attached. If it has, it assigns the file info to the subgrid.  If the file has
not been opened, it is opened, attached to, and the file info is recorded both
in the subset and internally to this function.

PGSd_DEM_CLOSE -- This function closes and detaches from all of the files of a
particular subset.


Presently, it is assumed that there never will be greater than
PGSd_DEM_MAX_STAGED (200) DEM files staged at anyone time in the PCF. If at some
point in the futre, this is a possibl;e situation, this code must be modified.
In that eventuality, one would only leave the most recently accessed files open
and leave any files greater than PGSd_DEM_MAX_STAGED closed.  

Assumption:
all commands assume that the particular subgrid of the subset has space
allocated for it.
  Assign-- assumes that FileRecord fields hdfID and gdID have been already
  inputed into the subset.


Author--  Alexis Zubrow/ARC, Abe Taaheri/L3 Com. EER Systems, Inc.

history--
Jul-1-1997   AZ   First Programming
08-July-99   SZ   Updated for the thread-safe functionality
31-Dec-02    AT   Modified so that number of open files decremented when the 
                  files are closed.

********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <PGS_DEM.h>
#include <mfhdf.h>
#include <HdfEosDef.h>
#include <PGS_TSF.h>


PGSt_SMF_status
PGS_DEM_AccessFile(
    PGSt_DEM_SubsetRecord *subsetInfo,    /*subset Information*/
    PGSt_DEM_FileRecord **subset,         /*subset*/
    PGSt_integer subgrid,                 /*subgrid value*/
    PGSt_integer command)                 /*Command-- Access/Open/Close*/
  
{
     
      /*return status*/
    PGSt_SMF_status  statusError;        /*fatal error status returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];

    PGSt_integer i,j;                      /*looping index*/
    PGSt_integer openFile;               /*file to open, number in 
					   attachedFiles */
    PGSt_integer entryExists = PGSd_DEM_SUBGRID_CLOSE; /*File not added to
							 attachedFiles array*/
    
    /*HDF-EOS interface*/
    intn statusHdf;       /*status returns for HDF-EOS*/
    int32 presentGdID = PGSd_DEM_HDF_CLOSE;           /*GRID ID*/
    int32 presentHdfID = PGSd_DEM_HDF_CLOSE;          /*File Handle*/

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;          /* lock and unlock return */
#endif

    /*Stores file diagnostics for each staged file. element 0 = subgrid, element
     1 = subset value, element 2 = hdfID (ID returned by GDopen), element 3 =
     gdID (ID returned from GDattach)*/

#ifdef _PGS_THREADSAFE
    /* Create non-static variables and get globals for the thread-safe version */
    int32 attachedFiles[PGSd_DEM_MAX_STAGED][4];
    PGSt_integer numAttached;
    int masterTSFIndex;
    extern int32 PGSg_TSF_DEMattachedFiles[][PGSd_DEM_MAX_STAGED][4];
    extern PGSt_integer PGSg_TSF_DEMnumAttached[];
    int outLoop;
    int inLoop;

    /* Set up global index for the thread-safe */
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }

    /*  Set from globals counterpart for the thread-safe */
    for (outLoop = 0; outLoop < PGSd_DEM_MAX_STAGED; outLoop++)
    {
        for (inLoop = 0; inLoop < 4; inLoop++)
        {
            attachedFiles[outLoop][inLoop] = 
                   PGSg_TSF_DEMattachedFiles[masterTSFIndex][outLoop][inLoop];
        }
    }

    numAttached = PGSg_TSF_DEMnumAttached[masterTSFIndex];
#else
    static int32 attachedFiles[PGSd_DEM_MAX_STAGED][4];
    static PGSt_integer numAttached = 0;       /*number of elements in
                                                 attachedFiles - 1, zero based
                                               */
#endif


   if((command == PGSd_DEM_ASSIGN || command == PGSd_DEM_OPEN ) && 
       subgrid == PGSd_DEM_SUBGRID_NULL)
      {
	/*ERROR file not staged*/
	sprintf(dynamicMsg, "Cannot access data... "
		"Subgrid (%d), not properly staged", subgrid);
	PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
			      dynamicMsg, 
			      "PGS_DEM_AccessFile()");
	
	statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	
	/*fatal error*/
	goto fatalError;
      }


    /*Assign staged file info that was attached and opened outside of this
      function.  Take the diagnostic info (hdfID and gdID) from the particular
      subgrid fo the subset, and assign it to the attachedFiles array for
      storage. */

    if(command == PGSd_DEM_ASSIGN)
    {
	
	/*Double Check to see that the file is properly staged*/
	
	if (subset[subgrid] == NULL)
	{
	    /*ERROR file not staged*/
	    sprintf(dynamicMsg, "Cannot access data... "
		    "Subgrid (%d), not properly staged", subgrid);
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_AccessFile()");
	    
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	}
	
	presentHdfID = subset[subgrid] -> hdfID;
	presentGdID = subset[subgrid] -> gdID;
	
	/*Check to see that these files truely have been opened and the
	  GRID has been attached*/

	if ((presentHdfID == PGSd_DEM_HDF_CLOSE) || 
		(presentGdID == PGSd_DEM_HDF_CLOSE))
	{
	    /*These file have not been completly opened.  If the hdfID is
	      not intitialized, neeed to both open the file and attach to
	      the GRID*/ 

	    if (presentHdfID == PGSd_DEM_HDF_CLOSE)
	    {
		/*file was not properly opened, retry*/
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
		presentHdfID = GDopen(subset[subgrid] -> filePath, 
				      DFACC_RDONLY);
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

		if (presentHdfID == FAIL)
		{
		    /*ERROR opening hdf-eos file*/
		    statusError = PGSDEM_E_HDFEOS;
		    
		    /*fatal Error*/
		    goto fatalError;
		}
	    }
	    
	    /*Grid not properly attached, retry*/
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	    presentGdID = GDattach(presentHdfID, (subsetInfo -> gridName));
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	    if (presentGdID == FAIL)
	    {
		/*ERROR attaching to the GRID*/
		statusError = PGSDEM_E_HDFEOS;
		
		/*fatal Error*/
		goto fatalError;
		
	    }
	    
	    /*Add info to the particular subgrid of the subset*/
	    subset[subgrid] -> hdfID = presentHdfID;
	    subset[subgrid] -> gdID = presentGdID;
	    
	    
	}
	
	/* Record the file's diagnostic info */	
	attachedFiles[numAttached][0] = subgrid;
	attachedFiles[numAttached][1] = subsetInfo -> subset;
	attachedFiles[numAttached][2] = presentHdfID;
	attachedFiles[numAttached][3] = presentGdID;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_DEMattachedFiles[masterTSFIndex][numAttached][0] = 
                                                attachedFiles[numAttached][0];
        PGSg_TSF_DEMattachedFiles[masterTSFIndex][numAttached][1] = 
                                                attachedFiles[numAttached][1];
        PGSg_TSF_DEMattachedFiles[masterTSFIndex][numAttached][2] = 
                                                attachedFiles[numAttached][2];
        PGSg_TSF_DEMattachedFiles[masterTSFIndex][numAttached][3] = 
                                                attachedFiles[numAttached][3];
#endif
	
	/*Increment the number of files in the attachedFiles array*/
	numAttached++;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_DEMnumAttached[masterTSFIndex] = numAttached;
#endif

	
	/*Successful return*/

	return(PGS_S_SUCCESS);
	
    }
    
    /*Open a file within this function.  This will open a file, attach to the
      GRID, and assign it's diagnostic info to the attachedFiles array and to
      the particular subgrid of the subset.  This function always checks whether
      the file is already recorded in the attachedFiles array.  If so, the
      file is already opened and attached, and the info is merely transfered
      back to the subset. */

    else if (command == PGSd_DEM_OPEN)
    {

	/*Double Check to see that the file is properly staged*/
	
	if (subset[subgrid] == NULL)
	{
	    /*ERROR file not staged*/
	    sprintf(dynamicMsg, "Cannot access data... "
		    "Subgrid (%d), not properly staged", subgrid);
	    PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					      dynamicMsg, 
					      "PGS_DEM_AccessFile()");
	    
	    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
	    
	    /*fatal error*/
	    goto fatalError;
	}
	
	/*Check to see if file already accessed*/
	for (i = 0; i < numAttached; i++)
	{
	    /*locate particular file*/
	    if (attachedFiles[i][0] == subgrid)
	    {
		if(attachedFiles[i][1] == subsetInfo ->  subset)
		{
		    /*check to see that the file is actually open and attached*/
		    if ((attachedFiles[i][2] != PGSd_DEM_HDF_CLOSE) ||
			(attachedFiles[i][3] != PGSd_DEM_HDF_CLOSE))
		    {
			/*Assign opened and attached file diagnostics to the
			  particular subgrid of the subset*/

			subset[subgrid] -> hdfID = attachedFiles[i][2];
			subset[subgrid] -> gdID = attachedFiles[i][3];
			
			/*Successfully opened file*/

			return (PGS_S_SUCCESS);
		    }
		    
		    /*if we've gotten to this point, it means there is an entry
		      for this file, but the file has not been opened or
		      attached to.  Most probable explaination is that the
		      subset has been closed.  Indicate that this file has been
		      previously opened, so that the second branch of the Open
		      command updates this entry, instead of creating a second
		      entry. */

		    entryExists = PGSd_DEM_SUBGRID_EXIST;
		    openFile = i;
		    
		    /*kick us out of loop*/
		    i = numAttached + 1;
		}
	    }
	}
	
	/*At this point open the HDF-EOS file and attach to the GRID*/
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	presentHdfID = GDopen(subset[subgrid] -> filePath, 
			      DFACC_RDONLY);
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	if (presentHdfID == FAIL)
	{
	    /*ERROR opening hdf-eos file*/
	    statusError = PGSDEM_E_HDFEOS;
	    
	    /*fatal Error*/
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
    	presentGdID = GDattach(presentHdfID, (subsetInfo -> gridName));
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif

	if (presentGdID == FAIL)
	{
	    /*ERROR attaching to the GRID*/
	    statusError = PGSDEM_E_HDFEOS;
	    
	    /*fatal Error*/
	    goto fatalError;
	    
	}
	
	/*Add info to the particular subgrid of the subset*/
	subset[subgrid] -> hdfID = presentHdfID;
	subset[subgrid] -> gdID = presentGdID;
	

	/*Add info to the stored array of file info, attachedFiles.  Check if an
	  entry already exists.  if it does, simply update the entry with the
	  current hdfID and gdID. If it doesn't exist, add a new entry to the
	  end of the stored array.*/

	if (entryExists == PGSd_DEM_SUBGRID_EXIST)
	{
	    /*no new entry*/
	    attachedFiles[openFile][2] = presentHdfID;
	    attachedFiles[openFile][3] = presentGdID;
#ifdef _PGS_THREADSAFE
            /* Reset global */
            PGSg_TSF_DEMattachedFiles[masterTSFIndex][openFile][2] = 
                                                attachedFiles[openFile][2];
            PGSg_TSF_DEMattachedFiles[masterTSFIndex][openFile][3] = 
                                                attachedFiles[openFile][3];
#endif
	}
	else
	{
	    /* new entry appended to attachedFiles. Check if number of entries
	       is greater than the maximum number of GRIDS simultaneously
	       attached, PGSd_DEM_MAX_STAGED.  If it is, this is presently an
	       error.  In the future, could add section here which keeps track
	       of which files need to be opened and which can be temporarily
	       closed. If this is not a problem, add all of diagnostic iinfo to
	       the next element in attachedFiles.*/

	    if (numAttached >= PGSd_DEM_MAX_STAGED)
	    {
		/*ERROR Too many files to simultaneously have attached and 
		  open*/
		sprintf(dynamicMsg, "Cannot access data... "
			"Cannot simultaneously stage more than %d files\n",
			PGSd_DEM_MAX_STAGED); 
		PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
						  dynamicMsg, 
						  "PGS_DEM_AccessFile()");
		
		statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		
		/*fatal error*/
		goto fatalError;
	    }
	
	    attachedFiles[numAttached][0] = subgrid;
	    attachedFiles[numAttached][1] = subsetInfo -> subset;
	    attachedFiles[numAttached][2] = presentHdfID;
	    attachedFiles[numAttached][3] = presentGdID;
#ifdef _PGS_THREADSAFE
            /* Reset global */
            PGSg_TSF_DEMattachedFiles[masterTSFIndex][numAttached][0] = 
                                                attachedFiles[numAttached][0];
            PGSg_TSF_DEMattachedFiles[masterTSFIndex][numAttached][1] = 
                                                attachedFiles[numAttached][1];
            PGSg_TSF_DEMattachedFiles[masterTSFIndex][numAttached][2] = 
                                                attachedFiles[numAttached][2];
            PGSg_TSF_DEMattachedFiles[masterTSFIndex][numAttached][3] = 
                                                attachedFiles[numAttached][3];
#endif
	    

	    

	    /*Increment the number of files stored in attached array*/
	    numAttached++;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_DEMnumAttached[masterTSFIndex] = numAttached;
#endif

	}

	return (PGS_S_SUCCESS);
    }
    
    /*When the function is called with the Close command, it will close and
      detach from all of the files of that particular subset. This means one
      should not use close until one is fully done using any of the files which
      make up that subset (this includes different geographics region and
      different layers which may also be in that same subset).*/
		
    else if (command == PGSd_DEM_CLOSE)
    {

	/*loop through to find each element which is of this particular subset.
	  Detach from the subgrid, close the HDF-EOS file, and reinitialize the
	  gdID and hdfID to PGSd_DEM_HDF_CLOSE in both the attachedFiles array
	  and the subset*/

	for (i = 0; i < numAttached; i++)
	{
	    /*locate particular file of same subset*/
	    if (attachedFiles[i][1] == subsetInfo -> subset)
	    {
		presentGdID = attachedFiles[i][3];
		presentHdfID = attachedFiles[i][2];

		/* detach and close.  Check if these files are actually open */
		if (presentGdID != PGSd_DEM_HDF_CLOSE)
		{
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
		    statusHdf = GDdetach(presentGdID);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    if (statusHdf == FAIL)
		    {
			/*ERROR detaching from grid*/
			statusError = PGSDEM_E_HDFEOS;
			
			/*fatal error*/
			goto fatalError;
			
		    }
		}
		
		if (presentHdfID != PGSd_DEM_HDF_CLOSE)
		{
#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */

        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
		    statusHdf = GDclose(presentHdfID);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
		    if (statusHdf == FAIL)
		    {
			/*ERROR closing hdf file*/
			statusError = PGSDEM_E_HDFEOS;
			
			/*fatal error*/
			goto fatalError;
			
		    }
		}

		/*reinitialize the gdID and hdfID to PGSd_DEM_HDF_CLOSE*/
		attachedFiles[i][2] = PGSd_DEM_HDF_CLOSE;
		attachedFiles[i][3] = PGSd_DEM_HDF_CLOSE;
#ifdef _PGS_THREADSAFE
                /* Reset global */
                PGSg_TSF_DEMattachedFiles[masterTSFIndex][i][2] = 
                                                attachedFiles[i][2];
                PGSg_TSF_DEMattachedFiles[masterTSFIndex][i][3] = 
                                                attachedFiles[i][3];
#endif
		presentGdID = PGSd_DEM_HDF_CLOSE;
		presentHdfID = PGSd_DEM_HDF_CLOSE;

		/*Double Check to see that the file is properly
		  staged. attachedFiles[i][0] is the subgrid.*/
		
		if (subset[attachedFiles[i][0]] == NULL)
		{
		    /*ERROR file not staged*/
		    sprintf(dynamicMsg, "Cannot access data... "
			    "Subgrid (%d), not properly staged", 
			    attachedFiles[i][0]);
 
		      PGS_SMF_SetDynamicMsg(PGSDEM_E_CANNOT_ACCESS_DATA,
					    dynamicMsg, 
					    "PGS_DEM_AccessFile()");
		    
		    statusError = PGSDEM_E_CANNOT_ACCESS_DATA;
		    
		    /*fatal error*/
		    goto fatalError;
		}

		subset[attachedFiles[i][0]] -> hdfID = PGSd_DEM_HDF_CLOSE;
		subset[attachedFiles[i][0]] -> gdID = PGSd_DEM_HDF_CLOSE;
		/* reduce numAttached by 1 and rearrange attachedFiles array */
		numAttached--;

		for(j=i; j<numAttached; j++)
		  {
		    attachedFiles[j][0] = attachedFiles[j+1][0];
		    attachedFiles[j][1] = attachedFiles[j+1][1];
		    attachedFiles[j][2] = attachedFiles[j+1][2];
		    attachedFiles[j][3] = attachedFiles[j+1][3];
		  }
		i--;

	    }
	}

	return(PGS_S_SUCCESS);

    }
    
    else
    {
	/* ERROR, improper command. */
	sprintf(dynamicMsg, "Improper tag... "
			    "Command (%d), not recognized", command);
	 PGS_SMF_SetDynamicMsg(PGSDEM_E_IMPROPER_TAG,
					   dynamicMsg, "PGS_DEM_AccessFile()");
		    
	statusError = PGSDEM_E_IMPROPER_TAG;
	
	/*fatal error*/
	goto fatalError;
    }
    

    /*FATAL ERROR*/

fatalError:
    {

	/* detach and close */
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (HDF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKHDF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	if((presentGdID != PGSd_DEM_HDF_CLOSE) || (presentGdID != FAIL))
	{
	    statusHdf = GDdetach(presentGdID);
	}
	
	if ((presentHdfID != PGSd_DEM_HDF_CLOSE) || (presentHdfID != FAIL))
	{
	    statusHdf = GDclose(presentHdfID);
	}
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKHDF);
#endif
	/*Return appropriate error status*/

	return(statusError);
    }
}
