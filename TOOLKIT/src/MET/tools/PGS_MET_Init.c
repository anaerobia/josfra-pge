/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_MET_Init
 
DESCRIPTION:
	Initializes a metadata configuration file (MCF). The contents of the
	MCF are read into memory to provide a basis for setting and checking 
	metadata parameter values. Metadata other than that obtained from the
	PGE and described in the MCF are at this point automatically obtained
	from the PCF and attached to the relevant object
AUTHOR: 
        Alward N. Siyyid/ EOSL
        Carol S. W. Tsai / Applied Reseach Corporation

HISTORY:
        18-MAY-95      ANS     Initial version
	31-May-1995     ANS     Code inspection comments update
	13-July-95     ANS     Improved Fortran example
	20-July-95	ANS	Fixed so that values supposed to be set by the PGE
				are removed if already set in the MCF
				Attributes set by PGE and mandatory are now given NOT_SET 
				status at initialization
	05-MAR-96      ANS     updated for tk5+
        08-Apr-97      CSWT    Added DSS, DAAC, PD, and TK for the checking of Data   
                               location during the initialization.
	07-Jul-99      RM      updated for TSF functionality
        08-Jan-07      AT      Modified If statement not exceed the limit of
                               opened MCF files. PGSg_MET_NumOfMCF is used as
                               idex to PGSg_MET_MasterNode[], and therefore,
                               cannot exceed (PGSd_MET_NUM_OF_MCF - 1)

END_FILE_PROLOG
*******************************************************************************/

/* include files */

#include <stdio.h>
#include <CUC/odldef.h>
#include <CUC/odlinter.h>
#include "PGS_MET.h"
#include "PGS_TSF.h"
#include <errno.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Initializes the metadata parameters already set in the memory
  
NAME:  
        PGS_MET_Init()

SYNOPSIS:
C:
        #include "PGS_MET.h"

	PGSt_SMF_status
	PGS_MET_Init(
		PGSt_PC_Logical fileId,
		PGSt_MET_all_handles mdHandles)

FORTRAN:
         include "PGS_MET_13.f"
	 include "PGS_MET.f" 
	 include "PGS_SMF.h"
         integer function pgs_met_init(fileId, mdHandles) 

	 integer	  fileId
	 character*(49)   mdHandles(20)
   
DESCRIPTION:
	Initializes MCF file containing metadata.

INPUTS:
        Name            Description            Units    	Min          Max
        ----            -----------             -----   	---          ---
	fileId		MCF file id 		none		variable     variable

OUTPUTS:
	mdHandles       metadata groups		none		N/A	     N/A
                        in MCF

RETURNS:   
   	PGS_S_SUCCESS			
	PGSMET_E_LOAD_ERR		Unable to load <MCF> information
					Lower level routines contain more information
	PGSMET_E_GRP_ERR		Master groups are not supposed to be enclosed under any other group or object.
                                 	The offending group is <name>
	PGSMET_E_NO_INVENT_DATA		Inventory data section not defined in the MCF
	PGSMET_E_DUPLICATE_ERR		There is a another object with the same name for object <name>
					Duplicate names are not allowed within master groups
	PGSMET_E_PCF_VALUE_ERR		Metadata objects to be set from values defined in PCF could not be set.
					See error returns form the lower level routines. Initialization takes
					place nevertheless.
	PGS_MET_E_GRP_NAME_ERR		Group name length should not exceed PGSd_MET_GROUP_NAME_L - 5
	PGS_MET_E_NUMOFMCF_ERR		Unable to load. The number of MCFs allowed has exceeded
	PGSTSF_E_GENERAL_FAILURE	problem in TSF code
	

EXAMPLES:
C:

	#include "PGS_MET.h"

	#define	INVENTORYMETADATA 1
	#define  MODIS_FILE 10253 / theis value must also be defined in the PCF as: /
				  / 10253|hdftestfile|/home/asiyyid/pgetest/fortran/|||hdftestfile|1 /
	#define  ODL_IN_MEMORY 0
	
	int main()
{
	PGSt_MET_all_handles handles;
	char * fileName = "/home/modis/hdftestfile"; / the user should change this accordingly /
	int32   hdfRet, sdid;
	extern AGGREGATE PGSg_MET_MasterNode;
	PGSt_SMF_status  ret = PGS_S_SUCCESS;
	char *sval = "sage_atmos_dyn_Winter"; 
	char *svals[4]; 
	char *datetime = NULL;
	PGSt_integer ival = 3;
	PGSt_double  dval = 203.2;
	PGSt_double  dvals[6] = {1.1, 2.1, 3.3, 4.4, 5.5, DBL_MAX};
	PGSt_integer ivals[6] = {1, 2, 3, 4, 5, INT_MAX};
	char *configval[2];

	PGSt_integer    fileId = PGSd_MET_MCF_FILE;
	PGSt_integer i;

	datetime = (char *) malloc(30);
	svals[0] = (char *) malloc(30);
	svals[1] = (char *) malloc(30);
	svals[2] = (char *) malloc(30);
	svals[3] = NULL;
	strcpy(svals[0], "string 1");
	strcpy(svals[1], "string 1");
	strcpy(svals[2], "string 1");
	strcpy(datetime, "1989-04-11T12:30:45.7Z");
	ret= PGS_MET_Init(fileId, handles);
	
	if(ret != PGS_S_SUCCESS)
        {
                printf("initialization failed\n");
                return 0;
        }
	ret = PGS_MET_SetAttr(handles[INVENTORYMETADATA],  "SizeMBECSDataGranule", &ival);
	ret = PGS_MET_SetAttr(handles[INVENTORYMETADATA],  "RangeBeginningDateTime", &datetime);
	ret = PGS_MET_SetAttr(handles[INVENTORYMETADATA],  "EastBoundingCoordinate", &dval);
	ret = PGS_MET_SetAttr(handles[INVENTORYMETADATA],  "ZoneIdentifier", ivals);
	ret = PGS_MET_SetAttr(handles[INVENTORYMETADATA],  "LocalityValue", svals);

	strcpy(datetime, "");
	printf("getting single string\n");
        ret = PGS_MET_GetSetAttr(handles[INVENTORYMETADATA],  "RangeBeginningDateTime", &datetime);
	for(i = 0; i<3; i++) strcpy(svals[i], "");
	printf("getting multiple strings\n");
	ret = PGS_MET_GetSetAttr(handles[INVENTORYMETADATA],  "LocalityValue", svals);
	for(i = 0; i<3; i++) printf("%s ", svals[i]);
	printf("\n");
	

	printf("%s\n", datetime);

	sdid = SDstart(fileName, DFACC_CREATE);
	if(sdid == FAIL)
	{
		 printf("SDstart failed\n");
		 exit(1)
	}
	

/ testing multiplicity /

	ret = PGS_MET_SetAttr(handles[INVENTORYMETADATA],  "GRingPointSequenceNo.1", &sval);
	sval = "NAWAZISH";

	ret = PGS_MET_SetAttr(handles[INVENTORYMETADATA],  "GRingPointSequenceNo.2", &sval);
	ret = PGS_MET_SetAttr(handles[INVENTORYMETADATA],  "GRingPointLatitude.1", dvals);

			ret = PGS_MET_Write(handles[ODL_IN_MEMORY], NULL, (PGSt_integer)NULL);
			if(ret != PGS_S_SUCCESS && ret != PGSMET_W_METADATA_NOT_SET)
			{	
				if(ret == PGSMET_E_MAND_NOT_SET) printf("some mandatory parameters were not set\n");
				else
				printf("ASCII Write failed\n");
			}
			ret = PGS_MET_Write(handles[INVENTORYMETADATA], "metadata", sdid);
			if(ret != PGS_S_SUCCESS && ret != PGSMET_W_METADATA_NOT_SET)
                        {
				if(ret == PGSMET_E_MAND_NOT_SET) printf("some mandatory parameters were not set\n");
                                else
                                printf("HDF Write failed\n");
                        }
	(void) SDend(sdid);
	ival =0;
	ret = PGS_MET_GetPCAttr(MODIS_FILE, 1, "metadata", "SizeMBECSDataGranule", &ival);
	strcpy(datetime, "");
	ret = PGS_MET_GetPCAttr(MODIS_FILE, 1, "metadata", "RangeBeginningDateTime", &datetime);       
	dval = 0;
	ret = PGS_MET_GetPCAttr(MODIS_FILE, 1, "metadata", "EastBoundingCoordinate", &dval);
	printf("%d %lf %s\n", ival, dval, datetime);
	for(i = 0; i < 6; i++) dvals[i] = 0.0;
	ret = PGS_MET_GetPCAttr(MODIS_FILE, 1, "metadata", "GRingPointLatitude.1", dvals);
	for(i = 0; i < 6; i++) printf("%lf", dvals[i]);
	printf("\n");

	for(i = 0; i<3; i++) strcpy(svals[i], "");
        ret = PGS_MET_GetPCAttr(MODIS_FILE, 1, "metadata",  "LocalityValue", svals);
        for(i = 0; i<3; i++) printf("%s ", svals[i]);
        printf("\n");
	for(i = 0; i<5; i++) ivals[i] = 0;
        ret = PGS_MET_GetPCAttr(MODIS_FILE, 1, "metadata",  "ZoneIdentifier", ivals);
        for(i = 0; i<5; i++) printf("%d ", ivals[i]);
        printf("\n");

/ These values must be defined in the PCF otherwise error is returned /
	ret = PGS_MET_GetConfigData("REV_NUMBER", &ival);
	strcpy(datetime, "");
	ret = PGS_MET_GetConfigData("LONGNAME", &datetime);
	dval = 0;
        ret = PGS_MET_GetConfigData("CENTRELATITUDE", &dval);
	printf("%d %lf %s\n", ival, dval, datetime);
	PGS_MET_Remove();
	printf("SUCCESS\n");
	return 0;
}

FORTRAN:
	include "PGS_SMF.f"
        include "PGS_MET_13.f"
        include "PGS_MET.f"
 
C       the file id must also be defined in the PCF as follows
C       10253|hdftestfile|/home/asiyyid/pgetest/fortran/|||hdftestfile|1
        integer pgs_met_init
        integer MODIS_FILE
        parameter(MODIS_FILE = 10253)
 
        integer INVENTORYMETADATA
        parameter(INVENTORYMETADATA = 2)
 
        integer ODL_IN_MEMMORY
        parameter(ODL_IN_MEMMORY = 1) 
C       the groups have to be defined as PGS_MET_GROUP_NAME_LEN characters long. The C interface is PGS_MET_GROUP_NAME_LEN +1.
C       The cfortran.h mallocs an extra 1 byte for the null character '\0/', therefore
C       making the actual length of a string pass as 50.
        character*PGSd_MET_GROUP_NAME_L groups(PGSd_MET_NUM_OF_GROUPS)
        character*40 attrName
        character*20 sval(1)
        character*30 datetime
        character*30 dateTimeRet
        character*20 svals(4)
        character*20 svalsRet(4)
        character*50 fileName
        integer   result
        integer   ret
        integer   ival(1)
        integer   ivals(6)
        double precision dval(1)
        double precision dvals(6)
        integer   pgs_met_init
        integer   pgs_met_setattr_i
        integer   pgs_met_setattr_s
        integer   pgs_met_setattr_d        
	integer   pgs_met_getsetattr_s
        integer   pgs_met_getpcattr_s
        integer   pgs_met_getpcattr_i
        integer   pgs_met_getpcattr_d
        integer   pgs_met_write
        integer   pgs_met_getconfigdata_i
        integer   pgs_met_getconfigdata_s
        integer   pgs_met_getconfigdata_d
        integer   pgs_met_remove
        integer   hdfReturn
        integer   access
        integer   sdid
        integer   sfstart
        integer   sfend
 
C       you must change this file spec in the PCF and the example before running this example
        fileName = "/home/asiyyid/pgetest/fortran/hdftestfile"
        attrName = "GRingPointSequenceNo.1"
        result = pgs_met_init(PGSd_MET_MCF_FILE, groups)
        if(result.NE.PGS_S_SUCCESS) then
           print *, "Initialization error. See Logstatus for details"
        endif
	Set various values
 
        ival(1) = 3
        result = pgs_met_setattr_i(groups(INVENTORYMETADATA), "SizeMBECSDataGranule", ival)
        datetime = "1989-04-11T12:30:45.7Z"
        result = pgs_met_setattr_s(groups(INVENTORYMETADATA),  "RangeBeginningDateTime", datetime)
        dval(1) = 203.2
        result = pgs_met_setattr_d(groups(INVENTORYMETADATA),  "EastBoundingCoordinate", dval)
        do 11 i = 1,6
                ivals(i) = i
 11     continue
        ivals(6) = PGSd_MET_INT_MAX
        result = pgs_met_setattr_i(groups(INVENTORYMETADATA),  "ZoneIdentifier", ivals)
        svals(1) = "string 1"
        svals(2) = "string 2"
        svals(3) = "string 3"
        svals(4) = PGSd_MET_STR_END
        result = pgs_met_setattr_s(groups(INVENTORYMETADATA),  "LocalityValue", svals)
        if(result.NE.PGS_S_SUCCESS) then
               print *, "SetAttr failed. See Logstatus for details"
               stop
        endif
	 
C       Getting string values set previously.
        result = pgs_met_getsetattr_s(groups(INVENTORYMETADATA),
     1  "RangeBeginningDateTime", dateTimeRet)
 
        result = pgs_met_getsetattr_s(groups(INVENTORYMETADATA),  "LocalityValue", svalsRet)
        print *, svalsRet(1), svalsRet(2), svalsRet(3)
 
        if(result.NE.PGS_S_SUCCESS) then
               print *,"GetSetAttr failed. See Logstatus for details"
        endif
 
        print *, dateTimeRet
C       hdf file where the metadata attribute will be attched
C       open the file with access = 4 (DFACC_CREATE)
        access = 4
        sdid = sfstart(fileName, access)
        if(sdid.EQ.-1) then
                print *, "Failed to open the hdf file"
        endif
 
 
C       testing multiplicity
 
        sval(1) = "31"
        result = pgs_met_setattr_s(groups(INVENTORYMETADATA),  "GRingPointSequenceNo.1", sval)
        sval(1) = "33"
 
        result = pgs_met_setattr_s(groups(INVENTORYMETADATA),  "GRingPointSequenceNo.2", sval)
        do 12 i = 1,6
                dvals(i) = i
 12     continue
        dvals(6) = PGSd_MET_DBL_MAX
        result = pgs_met_setattr_d(groups(INVENTORYMETADATA),  "GRingPointLatitude.1", dvals)
 
        if(result.NE.PGS_S_SUCCESS) then
               print *, "SetAttr failed. See Logstatus for details"
        endif
C       ascii file is written to file with id 10255
                        result = pgs_met_write(groups(ODL_IN_MEMMORY), dummyStr, dummyInt)
                        if(result.NE.PGS_S_SUCCESS  .AND. result.NE.PGSMET_W_METADATA_NOT_SET) then
                           if(result.EQ.PGSMET_E_MAND_NOT_SET) then
                                print *,"Some of the mandatory parameters were not set"
                           else
                                print *,"ASCII Write failed"
                           endif
                        endif
C                       write the first group as attribute
                        result = pgs_met_write(groups(INVENTORYMETADATA), "coremetadata.0", sdid)
                        if(result.NE.PGS_S_SUCCESS .AND. result.NE.PGSMET_W_METADATA_NOT_SET) then
                           if(result.EQ.PGSMET_E_MAND_NOT_SET) then
        print *,"Some of the mandatory parameters were not set"
                           else
                                print *,"ASCII Write failed"
                           endif
                        endif
        hdfReturn = sfend(sdid)
C        retrieve some of the values previously set from the written hdf attrbute
        ival(1) =0
        result = pgs_met_getpcattr_i(MODIS_FILE, 1, "coremetadata.0", "SizeMBECSDataGranule", ival)
 
        datetime = ""
        result = pgs_met_getpcattr_s(MODIS_FILE, 1, "coremetadata.0", "RangeBeginningDateTime", datetime)
        dval(1) = 0
        ret = pgs_met_getpcattr_d(MODIS_FILE, 1, "coremetadata.0", "EastBoundingCoordinate", dval)
        print *, ival(1), dval(1), datetime
        do 10 i = 1,6
                dvals(i) = 0.0
 10     continue
        ret = pgs_met_getpcattr_d(MODIS_FILE, 1, "coremetadata.0", "GRingPointLatitude.1", dvals)
        print *, dvals(1), dvals(2), dvals(3), dvals(4), dvals(5), dvals(6)
 
C       user is required to assign space for the strings as well
 
        svals(1) = ""
        svals(2) = ""
        svals(3) = ""
        ret = pgs_met_getpcattr_s(MODIS_FILE, 1, "coremetadata.0", "LocalityValue", svals)
        print *, svals(1), svals(2), svals(3)
        do 20 i = 1,6
                ivals(i) = 0
 20     continue
        ret = pgs_met_getpcattr_i(MODIS_FILE, 1, "coremetadata.0", "ZoneIdentifier", ivals)
        print *, ivals(1), ivals(2), ivals(3), ivals(4), ivals(5), ivals(6)
        if(ret.NE.PGS_S_SUCCESS) then
                print *, "GetPCAttr failed. See Logstatus for details\n"
        endif
C       Retrieve some values from the PCF files. These mut be defined in the PCF, otherwise the routine would return error
 
        ret = pgs_met_getconfigdata_i("REV_NUMBER", ival)
        datetime = ""
        ret = pgs_met_getconfigdata_s("LONGNAME", datetime)
        dval(1) = 0
        ret = pgs_met_getconfigdata_d("CENTRELATITUDE", dval)
        if(ret.NE.PGS_S_SUCCESS) then
                print *, "GetConfigData failed. See Logstatus for details"
        endif
        print *, ival, dval, datetime
        result = pgs_met_remove()
        print *, "SUCCESS"
 
        end
NOTES:
	MCF file must be in the format described in the MET userguide
	Multiple MCFs can now be generated by repeated calls to this function
 
REQUIREMENTS:
        PGSTK-0290, PGSTK-0370

DETAILS:
	Type PGSt_MET_All_Handles is defined as an array of strings. This routine
	returns the group names found in the MCF file as PGSt_MET_All_Handles. For e.g.
	if there are two groups called INVENTORYMETADATA and ARCHIVEDMETADATA defined in the 
	MCF, then PGS_MET_Init would return INVENTORYMETADATA in mdHandles[1] and ARCHIVEDMETADATA 
	in mdHandles[2]. mdHandles[0] is reserved to represent the whole MCF file whose name 
	is #defined as "MCF".

	In this way, the user has the ability to manipulate groups independently from each other.

	Type definitions in capitols (eg. AGGREGATE) are odl types and the user is referred
	to the ODL user guide for more information

	Addendum for tk5+

	There have been few changes in the structure of MCF which is as follows:

	1.	Groups were only used to define mdHandles in tk5. Now the user can use groups within groups
		to enhance readability. However, mdHandles groups should now contain a parameter
		GROUPTYPE = MASTERGROUP in the MCF to distinguish from other groups. Master groups
		should contain all the other groups and metadata objects.

	2.	Each metadata object (except for Container objects) should now have a TYPE = type field 
		and the NUM_VAL = number field defined. The supported types are:
			PGSt_integer
			PGSt_uinteger
			PGSt_double
			char * (string)

		There is a new TYPE called DATETIME which is a string in UTC format.

		PGSt_real has been omitted.

		The NUM_VAL field defines the maximum number of values in an array
			
	3.	Every object definition must contain the data location field and the MANDATORY field. Values 
		for MANDATORY are either TRUE or FALSE. 

	4.	The routine should not fail unless there is an error in the format of MCF 

GLOBALS:
	PGSg_MET_MasterNode
	PGSg_TSF_METNumOfMCF

FILES:
	MCF. PCF

FUNCTIONS_CALLED:
	PGS_MET_LoadAggregate
	PGS_MET_Remove
	PGS_MET_RetrieveConfigData
	PGS_MET_ErrorMsg
	NextGroup
	ParentGroup
	NextSubAggregate
	NextSubObject
	FindParameter
	FirstValue 
	PGS_TSF_GetTSFMaster
	PGS_TSF_GetMasterIndex
        PGS_SMF_TestErrorLevel

END_PROLOG:
***************************************************************************/



/* Master node for the MCF file is global */

#ifndef _PGS_THREADSAFE
AGGREGATE PGSg_MET_MasterNode[PGSd_MET_NUM_OF_MCF]= {NULL};
PGSt_integer     PGSg_MET_NumOfMCF = -1;
#endif

PGSt_SMF_status
PGS_MET_Init(				 /* Initializes metadata configuration
					  * file (MCF)
					  */
             PGSt_PC_Logical fileId,        /* file id for the file containing
                                          * MCF data
                                          */
             PGSt_MET_all_handles mdHandles) /* Handles for the MCF in memory */
{
	AGGREGATE		groupNode = NULL; 	
	AGGREGATE		objectNode = NULL;
	AGGREGATE               parentGrp = NULL;
        AGGREGATE               duplicateNode = NULL;
	PARAMETER		dataLocationNode = NULL;
	PARAMETER               pgeValNode = NULL;
	PARAMETER		masterGroupFlag = NULL;
	VALUE			valueNode = NULL;
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	PGSt_integer            dupFound = 0;
	PGSt_integer            groupCount = 0;
	PGSt_integer            granuleFound = 0;
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status tsfVal;
    AGGREGATE *PGSg_MET_MasterNode;
    PGSt_integer PGSg_MET_NumOfMCF;
    PGSt_TSF_MasterStruct *masterTSF;
    int masterTSFIndex;
    extern PGSt_integer PGSg_TSF_METNumOfMCF[];

    /* get master struct and master TSF global index */
    tsfVal = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ((PGS_SMF_TestErrorLevel(tsfVal)) ||
        (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* set locals from TSD value and global counterpart */
    PGSg_MET_MasterNode = (AGGREGATE *) pthread_getspecific(
                                  masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE]);
    PGSg_MET_NumOfMCF = PGSg_TSF_METNumOfMCF[masterTSFIndex];
#endif

	/* clear the errno for the ODL routines */

        if (errno == ERANGE)
        {
                errno = 0;
        }
	/* find out if number of MCFs has not exceeded 
	   or has reached the limit . Note that PGSg_MET_NumOfMCF can be
           0 to (PGSd_MET_NUM_OF_MCF - 1) */
	if(PGSg_MET_NumOfMCF == (PGSd_MET_NUM_OF_MCF - 1))
	{
                /* error message is:
                  "Unable to load. The number of MCFs allowed has exceeded */

                (void) PGS_MET_ErrorMsg(PGSMET_E_NUMOFMCF_ERR,
                                    "PGS_MET_Init", 0, errInserts);
                return(PGSMET_E_NUMOFMCF_ERR);
        }
	PGSg_MET_NumOfMCF++;
#ifdef _PGS_THREADSAFE
        /* re-set global counterpart and get ready to call COTS by locking */
        PGSg_TSF_METNumOfMCF[masterTSFIndex] = PGSg_MET_NumOfMCF;
        tsfVal = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
        if (PGS_SMF_TestErrorLevel(tsfVal))
        {
            return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
	
	/* Load MCF */
        
	retVal = PGS_MET_LoadAggregate(fileId, PGSd_MET_MCF_NAME, PGSd_PC_INPUT_FILE_NAME, &PGSg_MET_MasterNode[PGSg_MET_NumOfMCF]);
#ifdef _PGS_THREADSAFE
        /* re-set TSD value */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                                  PGSg_MET_MasterNode);
#endif
	if(retVal != PGS_S_SUCCESS)
        {
        	/* error message is:
                  "Unable to load <MCF> information"*/
		errInserts[0] = PGSd_MET_MCF_NAME;

                (void) PGS_MET_ErrorMsg(PGSMET_E_LOAD_ERR,
                                    "PGS_MET_Init", 1, errInserts);
		if(PGSg_MET_MasterNode[PGSg_MET_NumOfMCF] != NULL)
		{
			PGSg_MET_MasterNode[PGSg_MET_NumOfMCF] = RemoveAggregate(PGSg_MET_MasterNode[PGSg_MET_NumOfMCF]);
		}
		PGSg_MET_NumOfMCF--;
#ifdef _PGS_THREADSAFE
                /* reset global and TSD value - unlock but ignore return */
                PGSg_TSF_METNumOfMCF[masterTSFIndex] = PGSg_MET_NumOfMCF;
                pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                                      PGSg_MET_MasterNode);
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                return(PGSMET_E_LOAD_ERR);
        }

	/* find all the groups in the file */

	groupCount = 0;
	groupNode = PGSg_MET_MasterNode[PGSg_MET_NumOfMCF];
	granuleFound = 0;
	while(groupNode != NULL)
	{
	    /* Determine the Group type. Only consider the master groups */

	    if(groupNode == PGSg_MET_MasterNode[PGSg_MET_NumOfMCF])
	    {
		sprintf(mdHandles[groupCount], "%s#%d", groupNode->name, PGSg_MET_NumOfMCF);
		groupCount++;
		masterGroupFlag = FindParameter(groupNode, PGSd_MET_GROUP_TYPE_STR);
		if(masterGroupFlag != NULL)
		{
			masterGroupFlag = RemoveParameter(masterGroupFlag);
		}
	    }
            masterGroupFlag = FindParameter(groupNode, PGSd_MET_GROUP_TYPE_STR);
	    if(masterGroupFlag != NULL)
	    {
            	valueNode = FirstValue(masterGroupFlag);

            	if(strcmp(valueNode->item.value.string, PGSd_MET_MASTER_GROUP) == 0)
            	{
			if(strlen(groupNode->name) > (size_t)(PGSd_MET_GROUP_NAME_L - 5))
			{
				errInserts[0] = "PGSd_MET_GROUP_NAME_L - 5";
                                /* error message is:
                                "Group Name length should not exceed PGSd_MET_GROUP_NAME_L - 5"
                                 The offending group is <name>"*/

                                (void) PGS_MET_ErrorMsg(PGSMET_E_GRP_NAME_ERR,
                                    "PGS_MET_Init", 1, errInserts);
                                PGSg_MET_MasterNode[PGSg_MET_NumOfMCF] = RemoveAggregate(PGSg_MET_MasterNode[PGSg_MET_NumOfMCF]);
                                PGSg_MET_NumOfMCF--;
#ifdef _PGS_THREADSAFE
                                /* reset global and TSD value - 
                                   unlock but ignore return */
                                PGSg_TSF_METNumOfMCF[masterTSFIndex] = 
                                                           PGSg_MET_NumOfMCF;
                                pthread_setspecific(
                                    masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                                    PGSg_MET_MasterNode);
                                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                                return(PGSMET_E_GRP_NAME_ERR);
                        }
			sprintf(mdHandles[groupCount],"%s#%d", groupNode->name, PGSg_MET_NumOfMCF);
			if(strcmp(groupNode->name, PGSd_MET_INVENT_DATA) == 0)
			{
				granuleFound = groupCount;
			}
			groupCount++;
			parentGrp = ParentGroup(groupNode);
			if(parentGrp != PGSg_MET_MasterNode[PGSg_MET_NumOfMCF])
			{
				errInserts[0] = groupNode->name;
				/* error message is:
                  		"Master groups are not supposed to be enclosed under any other group or object.
				 The offending group is <name>"*/
 
                		(void) PGS_MET_ErrorMsg(PGSMET_E_GRP_ERR,
                                    "PGS_MET_Init", 1, errInserts);
				PGSg_MET_MasterNode[PGSg_MET_NumOfMCF] = RemoveAggregate(PGSg_MET_MasterNode[PGSg_MET_NumOfMCF]);
                        	PGSg_MET_NumOfMCF--;
#ifdef _PGS_THREADSAFE
                                /* reset global and TSD value - 
                                   unlock but ignore return */
                                PGSg_TSF_METNumOfMCF[masterTSFIndex] = 
                                                           PGSg_MET_NumOfMCF;
                                pthread_setspecific(
                                    masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                                    PGSg_MET_MasterNode);
                                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                		return(PGSMET_E_GRP_ERR);
			}
			
	    	}
	    }
            groupNode = NextGroup(groupNode);
	}

	if(granuleFound == 0)
	{
		/* error message is:
                  "Granule data section not in the MCF"*/

                (void) PGS_MET_ErrorMsg(PGSMET_E_NO_INVENT_DATA,
                                    "PGS_MET_Init", 0, errInserts);
		PGSg_MET_MasterNode[PGSg_MET_NumOfMCF] = RemoveAggregate(PGSg_MET_MasterNode[PGSg_MET_NumOfMCF]);
                PGSg_MET_NumOfMCF--;
#ifdef _PGS_THREADSAFE
                /* reset global and TSD value - unlock but ignore return */
                PGSg_TSF_METNumOfMCF[masterTSFIndex] = PGSg_MET_NumOfMCF;
                pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                                      PGSg_MET_MasterNode);
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                return(PGSMET_E_NO_INVENT_DATA);
        } 

	/* loop through each group and its object */
	/* find the group containing core data  */

	groupNode = PGSg_MET_MasterNode[PGSg_MET_NumOfMCF];

	do
	{
	    groupNode = NextGroup(groupNode);

	    /* Determine the Group type. Only consider the master groups */

	    masterGroupFlag = FindParameter(groupNode, PGSd_MET_GROUP_TYPE_STR);
            valueNode = FirstValue(masterGroupFlag);
	    dupFound = 0;
            if(valueNode != NULL)
            {	

		/* loop through all the objects within this particular group */
		objectNode = groupNode;
		duplicateNode = objectNode;
		do
		{
			objectNode = NextSubAggregate(groupNode, objectNode);
			/* Now see if the data is set locally, from the PCF table or from 
			 * the PGE
			 */

			if(objectNode != NULL && objectNode->kind == KA_OBJECT)
			{
				/* check that the group node is unique */
                        	duplicateNode = NextSubObject(groupNode, duplicateNode);
                        	while(duplicateNode != NULL)
                        	{
                                	if(strcmp(duplicateNode->name, objectNode->name) == 0)
                                	{
                                        	dupFound = dupFound + 1;
                                	}
                                	duplicateNode = NextSubObject(groupNode, duplicateNode);
                        	}
                        	if(dupFound > 1)
                        	{
                                	errInserts[0] = objectNode->name;
                                	/* error message is:
                                	There is a anothe object with the same name for object <name> */
 
                                	(void) PGS_MET_ErrorMsg(PGSMET_E_DUPLICATE_ERR, "PGS_MET_LoadAggregate",
                                        1, errInserts);
					PGSg_MET_MasterNode[PGSg_MET_NumOfMCF] = RemoveAggregate(PGSg_MET_MasterNode[PGSg_MET_NumOfMCF]);
                                	PGSg_MET_NumOfMCF--;
#ifdef _PGS_THREADSAFE
                                        /* reset global and TSD value - 
                                           unlock but ignore return */
                                        PGSg_TSF_METNumOfMCF[masterTSFIndex] = 
                                                              PGSg_MET_NumOfMCF;
                                        pthread_setspecific(
                                     masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                                             PGSg_MET_MasterNode);
                                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                                	return(PGSMET_E_DUPLICATE_ERR);
                        	}
				dataLocationNode = FindParameter(objectNode, PGSd_MET_DATA_LOC_STR);
				valueNode = FirstValue(dataLocationNode);
				
				if(strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_MCF) == 0 ||
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_PCF) ==0 ||
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_PGE) ==0 ||
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_DSS) ==0 ||
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_TK) ==0 ||
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_NONE) ==0)
				{
				   if(strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_PCF) ==0)
				   {
					pgeValNode = FindParameter(objectNode, PGSd_MET_ATTR_VALUE_STR);
                                        if(pgeValNode != NULL)
                                        {
                                                pgeValNode = RemoveParameter(pgeValNode);
                                        }
					retVal = PGS_MET_RetrieveConfigData(objectNode);
					if(retVal != PGS_S_SUCCESS)
					{
						errInserts[0] = objectNode->name;
						(void) PGS_MET_ErrorMsg(PGSMET_E_PCF_VALUE_ERR,
		                                    "PGS_MET_Init", 1, errInserts);
						PGSg_MET_MasterNode[PGSg_MET_NumOfMCF] = RemoveAggregate(PGSg_MET_MasterNode[PGSg_MET_NumOfMCF]);
                                		PGSg_MET_NumOfMCF--;
#ifdef _PGS_THREADSAFE
                                                /* reset global and TSD value */
                                                PGSg_TSF_METNumOfMCF[masterTSFIndex] = 
                                                              PGSg_MET_NumOfMCF;
                                                pthread_setspecific(
                                   masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                                                     PGSg_MET_MasterNode);
#endif
						retVal = PGSMET_E_PCF_VALUE_ERR;
					}
							
				   }
				   else if(strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_MCF) ==0)
				   {
					;
				   }
				   else if(strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_PGE) == 0)
				   {
					/* remove PGE value node if any */
					pgeValNode = FindParameter(objectNode, PGSd_MET_ATTR_VALUE_STR);
					if(pgeValNode != NULL)
                                        {
                                                pgeValNode = RemoveParameter(pgeValNode);
                                        }
				   }
				   else if(strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_TK) == 0)
				   {
					/* remove TK value node if any */
					pgeValNode = FindParameter(objectNode, PGSd_MET_ATTR_VALUE_STR);
					if(pgeValNode != NULL)
                                        {
                                                pgeValNode = RemoveParameter(pgeValNode);
                                        }
				   }
				   else if(strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_DSS) == 0)
				   {
                                         
                                        /* remove DSS value node if any */
                                        pgeValNode = FindParameter(objectNode, PGSd_MET_ATTR_VALUE_STR);
                                        if(pgeValNode != NULL)
                                        {
                                                pgeValNode = RemoveParameter(pgeValNode);
                                        }
                                   }
				   else if(strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_NONE) == 0)
				   {
					;/* do nothing */
				   }
				}
			} /* end object not null */
		}
		while(objectNode != NULL);
	    } /* end if master group or not */ 
	}
	while(groupNode != NULL);

#ifdef _PGS_THREADSAFE
        tsfVal = PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

	return(retVal);
}
