/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2009, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_MET_GetXMLAttr
 
DESCRIPTION:
        Retrieves parameter values from the XML EOS metadata located as 
	HDF attributes in defined product files or in separate ASCII files.
	
AUTHOR: 
	Abe Taaheri / Raytheon

HISTORY:
        18-MAY-2009    AT    Initial version

END_FILE_PROLOG
******************************************************************************/

/* include files */

#include "PGS_MET.h"
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "PGS_TSF.h"

/* odl include files */

#include <CUC/odldef.h>
#include <CUC/odlinter.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Accesses metadata parameters in Inventory metadata in HDF products 
        or independent ASCII files
  
NAME:  
        PGS_MET_GetXMLAttr()

SYNOPSIS:
C:
        #include "PGS_MET.h"

	PGSt_SMF_status
	PGS_MET_GetXMLAttr(
	PGSt_PC_Logical  fileId,
	PGSt_integer     version,
	char *           hdfAttrName,
	char *           parmName,
	void *           parmValue)

FORTRAN:
         include "PGS_MET_13.f"
	 include "PGS_MET.f" 
	 include "PGS_SMF.h"
         integer function pgs_met_getpcattr(fileId, version, hdfAttrName, parmName, parmValue) 

         character*      fileId
	 integer	 version
	 character*	 hdfAttrName
	 character*      parmName
	 'user defined'  parmValue
   
DESCRIPTION:
	Metadata parameters held in HDF attributes or in a separate ascii
        XML file can be read and parsed using this tool

INPUTS:
        Name         Description            Units    	Min          Max
        ----         -----------             -----   	---          ---
        fileId	     product file id	     none	variable    variable
	version	     product version number  none	 1	    variable
	hdfAttrName  name of HDF attribute   none	 N/A	     N/A
		     containing metadata
	parmName     metadata parameter name none        N/A         N/A
	

OUTPUTS:
	attrValue    value of attribute to   none        N/A         N/A
			be passed back to the 
			user

RETURNS:   
   	
PGS_S_SUCCESS			
PGSMET_E_PCREAD_ERR	"Unable to obtain <filename or attribute filename> 
                         from the PC table"
			 Most likely that <filename or attribute filename> 
                         is not defined in the PCF
PGSMET_E_FILETOODL_ERR   "Unable to convert <filename> into an ODL format"
			 error returns from lower level routines should 
                         expalin the problem
PGSMET_E_AGGREGATE_ERR	 Unable to create odl aggregate <aggregate name>
			 It definately means that ODL routine has failed 
                         to allocate enough memory
PGSMET_E_SYS_OPEN_ERR    Unable to open pc attribute file
	                 Usually if the file does not exist at the path 
                         given, check the name and path of the file
PGSMET_E_ODLTOVAL_ERR    Unable to convert attribute values from the odl format
			 error returns from lower level routines should 
                         expalin the problem
PGSTSF_E_GENERAL_FAILURE problem in TSF code

EXAMPLES:
C:

	for(i = 0; i<3; i++) strcpy(svals[i], "");
        ret = PGS_MET_GetPCAttr(MODIS_FILE, 1, "metadata",  "LocalityValue", svals);
        for(i = 0; i<3; i++) printf("%s ", svals[i]);
        printf("\n");
	for(i = 0; i<5; i++) ivals[i] = 0;

FORTRAN:

C	user is required to assign space for the strings as well 

	svals(1) = ""
	svals(2) = ""
	svals(3) = ""
	ret = pgs_met_getpcattr_s(MODIS_FILE,, 1, "xmlmetadata.0", "LocalityValue", svals)
	print *, svals(1), svals(2), svals(3)

NOTES:
	User is responsible for the returned buffers to be large enough to 
	hold the returned values. 
	It is very important that variable string pointers are used for 
        string manipulations.

REQUIREMENTS:
        PGSTK-0290 PGSTK-0235

DETAILS:
	See MET userguide
	
FILES:
	PCF.v5, generic product file, separate ascii file

FUNCTIONS_CALLED:
	PGS_PC_GetPCSData
	PGS_PC_GetFileAttr
	PGS_MET_ErrorMsg
	PGS_MET_NameAndClass
	PGS_TSF_GetTSFMaster
	PGS_TSF_GetMasterIndex
	PGS_SMF_TestErrorLevel

END_PROLOG:
***************************************************************************/


#ifndef _PGS_THREADSAFE
char  PGSg_MET_AttrHandle[PGSd_MET_NUM_OF_SIG][PGSd_MET_SIGNATURE_L] = {""};
#endif

PGSt_SMF_status
PGS_MET_GetPCAttr(
      _PC_Logical 	fileId, /* file id of file containing parameter data */
       PGSt_integer   	version,     /* product version number */
       char *	      	hdfAttrName, /* name of hdf attribute to be searched */
       char *         	parmName,    /* Parameter name to be retrieved */
       void *       	parmValue)   /* Parameter value buffer to hold 
					retrieved value */

{
  struct ODLDate
  {
    short        year;                 /* Year number                    */
    short        doy;                  /* Day of year (0-366)            */
    char         month;                /* Month of year (1-12)           */
    char         day;                  /* Day of month (1-31)            */
    short        zone_hours;           /* Zone hours from GMT (-12 - +12)*/
    char         zone_minutes;         /* Zone minutes from GMT (0-59)   */
    char         hours;                /* Hours of day (0-23)            */
    char         minutes;              /* Minutes of hour (0-59)         */
    char         seconds;              /* Seconds of minute (0-59)       */
    long         nanoseconds;          /* Nanoseconds (0-999999999)      */
  };
  
  
  long                  iyear;         
  long                  imonth;         
  long                  iday;         
  long                  nday;         
  long                  izonehours;       
  long                  izoneminutes;       
  long                  ihours;       
  long                  iminutes;    
  long                  iseconds;  
  double                inanoseconds;         
  
  struct ODLDate *      date_time_Ptr;
  FILE *		attrFile = NULL;
  PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
  PGSt_integer		odlRetVal = 0;  /* ODl function return value */
  char *	       	errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
  /* Dynamic strings inserted in 
   * error messages
   */
  char *		  funcName = "PGS_MET_GetXMLAttr";
  char * 		  HDFfilenameafterDot;
  char *                  hdfattrn;
  char                    hdfattrname[PGSd_PC_FILE_PATH_MAX] = "";
  char  		  Fileattr[PGSd_PC_FILE_PATH_MAX] = "";
  char  		  fileattr[PGSd_PC_FILE_PATH_MAX] = "";
  char  		  fileAttr[PGSd_PC_FILE_PATH_MAX] = "";
  char                    fileName[PGSd_PC_FILE_PATH_MAX] = "";
  char * 		  DotcheckPtr = NULL;
  char * 		  strPtr = NULL;
  char * 		  strPtrFile = NULL;
  PGSt_integer		  i;
  PGSt_integer		  mdFlag = PGS_FALSE;
  char **                 datetimePtr = NULL;
  char **                 strPtr1 = NULL;
  PGSt_integer*           intPtr = NULL;
  PGSt_double*            dblPtr = NULL;
  char                    metClass[PGSd_MET_CLASS_L] = "";
  char 			  cmonth[PGSd_MET_NAME_L] = "";
  char 			  cday[PGSd_MET_NAME_L] = "";
  char                    chours[PGSd_MET_NAME_L] = "";
  char                    czonehours[PGSd_MET_NAME_L] = "";
  char                    czoneminutes[PGSd_MET_NAME_L] = "";
  char                    cminutes[PGSd_MET_NAME_L] = "";
  char                    cseconds[PGSd_MET_NAME_L] = "";
  char                    date_time_name[PGSd_MET_NAME_L] = "";
  char *                  ccinanoseconds;
  char                    cinanoseconds[PGSd_MET_NAME_L] = "";
  char 			  name[PGSd_MET_NAME_L] = "";
  PGSt_integer		  locVersion;
  char                    signature[PGSd_MET_SIGNATURE_L];
  PGSt_integer		  fileInBuffer = 1;
  PGSt_integer            bufferIndex = 0;
  PGSt_integer            st_time = 0;
  static PGSt_integer	  month1[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  static PGSt_integer  	  month2[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
  char			  locAggrName[100];
  char *                  charPtr = NULL;
  PGSt_boolean            leapyear = PGS_FALSE;
  PGSt_boolean            landsatFlag = PGS_FALSE;
  PGSt_boolean            yearday = PGS_FALSE;
  PGSt_boolean            secondiszero = PGS_FALSE;
  PGSt_boolean            zonehoursiszero = PGS_FALSE;
  PGSt_boolean            Zchar = PGS_FALSE;
  PGSt_boolean            HDFflag = PGS_FALSE;
  PGSt_boolean            hdfattrnameflag = PGS_FALSE;
  int                     iattr;
  
  FILE                    *fp;
  PGSt_boolean            metflag = PGS_FALSE;
  
  
  struct stat buf;
  
#ifdef _PGS_THREADSAFE
  PGSt_SMF_status retTSF;
  char *lasts;
  PGSt_integer newFilePosition;
  char *PGSg_MET_AttrHandle;
  PGSt_TSF_MasterStruct *masterTSF;
  int masterTSFIndex;
  extern PGSt_integer PGSg_TSF_METnewFilePosition[];
  
  /* Get struct containing keys for TSDs and global master index */
  retTSF = PGS_TSF_GetTSFMaster(&masterTSF);
  masterTSFIndex = PGS_TSF_GetMasterIndex();
  if ((PGS_SMF_TestErrorLevel(retTSF)) ||
      (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX))
    {
      return PGSTSF_E_GENERAL_FAILURE;
    }
  
  /* Assign from keys and global counterparts */
  newFilePosition = PGSg_TSF_METnewFilePosition[masterTSFIndex];
  PGSg_MET_AttrHandle = (char *) pthread_getspecific(
						     masterTSF->keyArray[PGSd_TSF_KEYMETATTRHANDLE]);
  
#else
  static PGSt_integer	newFilePosition = 0;
#endif
  
  
  strcpy(hdfattrname,hdfAttrName);
  
  for (i=0; hdfattrname[i]; i++)
    {
      hdfattrname[i] = tolower(hdfattrname[i]);
    }
  
  
  /* set hdfattrnameflag to be PGS_TRUE if the HDF attribuet name is equal to
   * xmlmetadata.X where .x is a squeence number beginning at 0 and
   * running as high as 9
   */
  
  for (iattr=0; iattr < 10; iattr++){
    switch(iattr){
    case  0 : hdfattrn = "xmlmetadata.0"; break;
    case  1 : hdfattrn = "xmlmetadata.1"; break;
    case  2 : hdfattrn = "xmlmetadata.2"; break;
    case  3 : hdfattrn = "xmlmetadata.3"; break;
    case  4 : hdfattrn = "xmlmetadata.4"; break;
    case  5 : hdfattrn = "xmlmetadata.5"; break;
    case  6 : hdfattrn = "xmlmetadata.6"; break;
    case  7 : hdfattrn = "xmlmetadata.7"; break;
    case  8 : hdfattrn = "xmlmetadata.8"; break;
    case  9 : hdfattrn = "xmlmetadata.9"; break;
    }
    
    if (strcmp(hdfattrname, hdfattrn) ==0)
      {
	hdfattrnameflag = PGS_TRUE;
      }
  }
  
  /* set metflag to PGS_TRUE if attribute to be retrieved from an ASCII XML
     file from INVENTORYMETADATA section */ 
  if(hdfAttrName != NULL && *hdfAttrName != '\0')
    {
      if(strlen(hdfAttrName) > (strlen(PGSd_MET_GROUP_STR) + 1))
	{
	  if(strncmp(hdfAttrName, PGSd_MET_GROUP_STR, strlen(PGSd_MET_GROUP_STR)) == 0)
	    {
	      metflag = PGS_TRUE;
	    }
	}
      /* set metflag to PGS_TRUE if attribute to be retrieved from an ASCII XML
	 metadata file of landsat7 */ 
      else if(strlen(hdfAttrName) > (strlen(PGSd_MET_LSAT_GRP_STR) + 1))
	{
	  if(strncmp(hdfAttrName, PGSd_MET_LSAT_GRP_STR, strlen(PGSd_MET_LSAT_GRP_STR)) == 0)
	    {
	      metflag = PGS_TRUE;
	    }
	}
    }
  
  /* get product input filename and the name of the file containing the attributes */
  
  locVersion = version; 
  
  retVal = PGS_PC_GetReference(fileId, &version, fileName);
  errInserts[0] = "input filename";
  if(retVal == PGS_S_SUCCESS)
    {
      errInserts[0] = "attribute filename";
      retVal = PGS_PC_GetFileAttr(fileId, locVersion, PGSd_PC_ATTRIBUTE_LOCATION,
				  PGSd_PC_FILE_PATH_MAX, fileAttr);
      
      /* Set HDFflag to be PGS_TRUE if the attribute of the file associated with the 
       * particular ID and version contains ".xml" and the filename associated with the
       * logical identifier and version number is a HDF file existing in the same 
       * directory as the ASCII metadata file .xml is.
       */
      
      DotcheckPtr = strrchr(fileAttr,'.');
      if(DotcheckPtr != NULL)
	{
	  strcpy(Fileattr,fileAttr);
	  strcpy(fileattr,fileAttr);
	  
	  HDFfilenameafterDot = strrchr(fileattr, '.');
	  if(strcmp(HDFfilenameafterDot,".xml") == 0)
	    {
	      if ((fp = fopen(fileName, "r")) != NULL) 
                {
		  HDFflag = PGS_TRUE; 
		  strcpy(fileAttr,fileattr);
		  fclose(fp);
                }
	      else HDFflag = PGS_FALSE;
	    }
	  else HDFflag = PGS_FALSE;
	}
    }
  
  if(retVal != PGS_S_SUCCESS)
    {
      /* error message is 
	 "Unable to obtain <filename or attribute filename> from the PC table */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_PCREAD_ERR,
			      funcName, 1, errInserts);
      return(PGSMET_E_PCREAD_ERR);
    }
  
  /* if the two filenames are same then attributes are in HDF attribute */
  
  strPtr = strrchr(fileAttr, (int)'/') + 1;
  strPtrFile = strrchr(fileName, (int)'/') + 1;
  
  /* prepare a unique signature */
  
  if(strcmp(strPtr, strPtrFile) == 0 && HDFflag == PGS_FALSE)
    {
       sprintf(signature,"%s/%s", fileName, hdfAttrName);
    }
  
  /* set signature specially for retrieving the inventory metadata from the HDF file when
   * the user sets up the first parameter "fileId", file id for the file containing
   * parameter data is the ASCII metadata file .xml file
   */
  
  if( !(HDFflag == PGS_TRUE && hdfattrnameflag == PGS_FALSE && metflag
	== PGS_FALSE))
    {
      /* find modification time for the file and put it in the signature */
      stat(fileAttr, &buf);
      st_time = buf.st_mtime;
      sprintf(signature,"%s%d", fileAttr, st_time);
    }
  
  /* find out if the signature already exists */
  
  fileInBuffer = 1;
  bufferIndex = 0;
#ifdef _PGS_THREADSAFE
  /* we are dealing with pointers in TSF mode, so we have to differentiate */
  while(*(PGSg_MET_AttrHandle+(bufferIndex*PGSd_MET_SIGNATURE_L)) != '\0' && 
	bufferIndex < PGSd_MET_NUM_OF_GROUPS)
    {
      fileInBuffer = strcmp(signature, 
			    PGSg_MET_AttrHandle+
			    (bufferIndex*PGSd_MET_SIGNATURE_L));
#else
  while(PGSg_MET_AttrHandle[bufferIndex][0] != '\0' && 
	bufferIndex < PGSd_MET_NUM_OF_GROUPS)
    {
      fileInBuffer = strcmp(signature, 
			    PGSg_MET_AttrHandle[bufferIndex]);
#endif
      if(fileInBuffer == 0)
	{
	  break;
	}
      bufferIndex++;
    }
      
#ifdef _PGS_THREADSAFE
  /* we are calling COTS (ODL) so we need to lock */
  retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
  if (PGS_SMF_TestErrorLevel(retTSF))
    {
      return PGSTSF_E_GENERAL_FAILURE;
    }
#endif
      
  if(strcmp(strPtr, strPtrFile) == 0 && HDFflag == PGS_FALSE)
    {
      if(fileInBuffer != 0)
	{
	  retVal = PGS_MET_HDFToXML(fileAttr, hdfAttrName);
	  
	  if(retVal != PGS_S_SUCCESS)
	    {
	      /* error message is
		 "Unable to convert <filename> into an XML format */
	      errInserts[0] = "HDF attribute";
	      
	      (void) PGS_MET_ErrorMsg(PGSMET_E_FILETOXML_ERR,
				      funcName, 1, errInserts);
#ifdef _PGS_THREADSAFE
	      /* unlock - do not check return since we want the user
		 to know about previous error */
	      PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
	      return(PGSMET_E_FILETOODL_ERR);
	    }
	}
      else
	{
	  aggNode = PGSg_MET_AggList[bufferIndex];
	}
    }
  else /* attributes are contained in a separate file */
    {
      (void) PGS_MET_NameAndClass(parmName, name, metClass);
      if(fileInBuffer != 0)
	{
	  aggNode = NewAggregate(aggNode, KA_GROUP, "locAggr", "");
	  if (aggNode == NULL)
	    {
	      errInserts[0] = "";
	      /* error message is:
		 "Unable to create odl aggregate <aggregate name>" */
	      
	      (void) PGS_MET_ErrorMsg(PGSMET_E_AGGREGATE_ERR, funcName,
				      1, errInserts);
#ifdef _PGS_THREADSAFE
	      /* unlock - do not check return since we want the user
		 to know about previous error */
	      PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
	      return(PGSMET_E_AGGREGATE_ERR);
	    }
	  attrFile = fopen(fileAttr, "r");
	  if(attrFile == NULL)
	    {
	      /* error message is:
		 "Unable to open pc attribute file" */
	      
	      (void) PGS_MET_ErrorMsg(PGSMET_E_SYS_OPEN_ERR, funcName,
				      0, errInserts);
#ifdef _PGS_THREADSAFE
	      /* unlock - do not check return since we want the user
		 to know about previous error */
	      PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
	      return(PGSMET_E_SYS_OPEN_ERR);
	    }
	  odlRetVal = ReadLabel(attrFile, aggNode);
	  if(odlRetVal == 0)
	    {
	      /* error message is:
		 "Unable to convert <filename> into an odl structure" */
	      errInserts[0] = strPtr;
	      
	      (void) PGS_MET_ErrorMsg(PGSMET_E_FILETOODL_ERR, funcName,
				      1, errInserts);
	      fclose(attrFile);
#ifdef _PGS_THREADSAFE
	      /* unlock - do not check return since we want the user
		 to know about previous error */
	      PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
	      return(PGSMET_E_FILETOODL_ERR);
	    }
	  /* need to set the class attribute */
	  
	  objectNode = aggNode;
	  do
	    {
	      objectNode = NextObject(objectNode);
	      if(objectNode != NULL)
		{
		  classParmNode = FindParameter(objectNode, PGSd_MET_CLASS_STR);
		  if(classParmNode != NULL) /* object is a multiiple */
		    {
		      classValueNode = FirstValue(classParmNode);
		      if(classValueNode->item.type == TV_STRING ||
			 classValueNode->item.type == TV_SYMBOL)
			{
			  if(objectNode->objClass != NULL)
			    {
			      free(objectNode->objClass);
			      objectNode->objClass = NULL;
			      objectNode->objClass = (char *) malloc(strlen(classValueNode->item.value.string)+ 1);
			    }
			  strcpy(objectNode->objClass, classValueNode->item.value.string);
			}
		    }
		}
	    }
	  while(objectNode != NULL);
	}     
      else 
	{
	  aggNode = PGSg_MET_AggList[bufferIndex];
	}
      
      
      /* extract the values and dump in the buffer provided */
      parmNode = FindParameter(aggNode, name); 
      if(parmNode == NULL)
	{
	  if(attrFile != NULL)
	    {
	      fclose(attrFile);
	    }
	}
      else
	{
	  valueNode = FirstValue(parmNode);
	  
	  /* copy the value string(s) into user provided string */
	  if(valueNode->item.type == TV_STRING  || valueNode->item.type == TV_SYMBOL)
	    {
	      strPtr1 = (char **) parmValue;
	      while(valueNode != NULL)
		{
		  strcpy(*strPtr1, valueNode->item.value.string);
		  strPtr1++;
		      valueNode = NextValue(valueNode);
		}
	    }   
	  
	  if(valueNode->item.type == TV_INTEGER)
	    {
	      intPtr = (PGSt_integer*) parmValue;
	      while(valueNode != NULL)
		{
		  *intPtr = (PGSt_integer)valueNode->item.value.integer.number; 
		  intPtr++;
		  valueNode = NextValue(valueNode);
		}
	    }
	  else 
	    {    
	      dblPtr = (PGSt_double*) parmValue;
	      while(valueNode != NULL)
		{
		  *dblPtr = (PGSt_double)valueNode->item.value.real.number; 
		  dblPtr++;
		  valueNode = NextValue(valueNode);
		}
	    }
	  
	  if(attrFile != NULL)
	    {
	      fclose(attrFile);
	    }
	  if(fileInBuffer != 0) /* its a new file or attribute */
	    {
#ifdef _PGS_THREADSAFE
	      /* data types are different in TSF mode */
	      strcpy(PGSg_MET_AttrHandle+
		     (newFilePosition*PGSd_MET_SIGNATURE_L), signature);
#else
	      strcpy(PGSg_MET_AttrHandle[newFilePosition], signature);
#endif
	      PGSg_MET_AggList[newFilePosition] = RemoveAggregate(PGSg_MET_AggList[newFilePosition]);
	      if(landsatFlag == PGS_TRUE)
		{
		  PGSg_MET_AggList[newFilePosition] = landsatNode;
		}
	      else
		{
		  PGSg_MET_AggList[newFilePosition] = aggNode;
		}
	      
	      newFilePosition++;
	      if(newFilePosition == PGSd_MET_NUM_OF_SIG)
		{  
		  newFilePosition = 0;
		}
#ifdef _PGS_THREADSAFE
	      /* re-set global counterpart and TSD */
	      PGSg_TSF_METnewFilePosition[masterTSFIndex] = 
		newFilePosition;
	      pthread_setspecific(
				  masterTSF->keyArray[PGSd_TSF_KEYMETAGGLIST],
				  PGSg_MET_AggList);
	      pthread_setspecific(
				  masterTSF->keyArray[PGSd_TSF_KEYMETATTRHANDLE],
				  PGSg_MET_AttrHandle);
#endif
	    }
	  else
	    {
	      if(landsatFlag == PGS_TRUE)
		{
		  landsatNode = RemoveAggregate(landsatNode);
		}
	      else
		{
		  ; 
		}
	    }
#ifdef _PGS_THREADSAFE
	  /* unlock - do not check return
	     since we want the user
	     to know about previous error */
	  PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif	
	  return(PGS_S_SUCCESS);
	}
    } /* end else */
  
      /* MasterNode could be null, therfore just create a dummy one .
       * Its important to initialize masternode because GetSetAttr() 
       * only recognizes the master node and is used here to 
       * actually read the values in.
       */
  mdFlag = PGS_FALSE;
  if(PGSg_MET_MasterNode[0] == NULL)
    {
      PGSg_MET_MasterNode[0] = NewAggregate(PGSg_MET_MasterNode[0], KA_GROUP, "MCF", "");	
      mdFlag = PGS_TRUE;
    }
  /* attach the new node to the master node and the value of the parameter */
  
  aggNode = PasteAggregate(PGSg_MET_MasterNode[0], aggNode);
  
#ifdef _PGS_THREADSAFE
  /* re-set TSD */
  pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
		      PGSg_MET_MasterNode);
#endif
  
  /* This section of code is to resolve the the Aster problem 
     so that different groups in an ascii file can be read */
  
  strcpy(locAggrName, "locAggr#0");
  if(hdfAttrName != NULL && *hdfAttrName !='\0')
    {
      if(strlen(hdfAttrName) > (strlen(PGSd_MET_GROUP_STR) + 1))
	{
	  if(strncmp(hdfAttrName, PGSd_MET_GROUP_STR, strlen(PGSd_MET_GROUP_STR)) == 0)
	    /* hdf attr name contains the name of the master group in the file */
	    {
	      charPtr = strchr(hdfAttrName, (int)'=');
	      charPtr++;
	      sprintf(locAggrName, "%s#0", charPtr);
	    }
	}
    }
  
#ifdef _PGS_THREADSAFE
  /* unlock here because PGS_MET_GetSetAttr uses same lock */
  retTSF = PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
  retVal = PGS_MET_GetSetAttr(locAggrName, parmName, parmValue);
  
  /* a return status of PGSMET_E_NULL_PARAMETER will be set up if the requested
   * parameter is a NULL value, and a return status of PGSMET_W_METADATA_NOT_SET
   * will be set up if the requested parameter is not set yet
   */
  
  /* error message is:
     "the requested parameter <parameter name> could not be found in <agg node>" */
  
  if(retVal == PGSMET_E_DD_UNKNOWN_PARM)
    {
      errInserts[0] = parmName;
      /* error message is :
	 "The requested Parameter is a NULL value" */
      (void) PGS_MET_ErrorMsg(PGSMET_E_NULL_PARAMETER,
			      funcName, 1, errInserts);
      retVal = PGSMET_E_NULL_PARAMETER;
    }
  else
    {
      if(retVal == PGSMET_W_METADATA_NOT_SET)
	{
	  errInserts[0] = parmName;
	  /* error message is :
	     "The requested Parameter is not set yet" */
	  (void) PGS_MET_ErrorMsg(PGSMET_E_PARAMETER_NOT_SET,
				  funcName, 1, errInserts);
	  retVal = PGSMET_E_PARAMETER_NOT_SET;
	}
    }
  
#ifdef _PGS_THREADSAFE
  /* calling COTS again - lock up */
  retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
  if (PGS_SMF_TestErrorLevel(retTSF))
    {
      return PGSTSF_E_GENERAL_FAILURE;
    }
#endif
  
  /* clean up */
  
  aggNode = CutAggregate(aggNode);
  if(fileInBuffer != 0) /* its a new file or attribute */
    {
#ifdef _PGS_THREADSAFE
      /* TSF mode uses different data type */
      strcpy(PGSg_MET_AttrHandle+(newFilePosition*PGSd_MET_SIGNATURE_L), 
	     signature);
#else
      strcpy(PGSg_MET_AttrHandle[newFilePosition], signature);
#endif
      PGSg_MET_AggList[newFilePosition] = RemoveAggregate(PGSg_MET_AggList[newFilePosition]);
      PGSg_MET_AggList[newFilePosition] = aggNode;
      newFilePosition++;
      if(newFilePosition == PGSd_MET_NUM_OF_SIG)
	{
	  newFilePosition = 0;
	}
#ifdef _PGS_THREADSAFE
      /* re-set global and TSDs */
      PGSg_TSF_METnewFilePosition[masterTSFIndex] = newFilePosition;
      pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETAGGLIST],
			  PGSg_MET_AggList);
      pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETATTRHANDLE],
			  PGSg_MET_AttrHandle);
#endif
    }
  else
    {
      ;
    }
  if(mdFlag == PGS_TRUE)
    {
      PGSg_MET_MasterNode[0] = RemoveAggregate(PGSg_MET_MasterNode[0]);
      PGSg_MET_MasterNode[0] = NULL;
#ifdef _PGS_THREADSAFE
      /* re-set TSD */
      pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
			  PGSg_MET_MasterNode);
#endif
    }
  
#ifdef _PGS_THREADSAFE
  retTSF = PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
  
  return(retVal);
}
  
  

































































