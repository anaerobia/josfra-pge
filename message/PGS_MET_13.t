############################################################################## 
# BEGIN_FILE_PROLOG:
# 
# FILENAME:
#   PGS_MET_13.t Return code definitions for PGS MET tools (SMF seed value 13)
#   
# 
# DESCRIPTION:
# 
#   This file contains PGS_SMF standard return code definitions for the
#   PGS_GCT group of tools.
#
#   The file is intended to be used as input by the smfcompile utility,
#   which generates the message file, and the C and FORTRAN header files.
#
# AUTHOR:
#   Mike Sucher / Applied Research Corp.
#   Carol S. W. Tsai / Space Applications Corp.
#   Abe Taaheri / Emergent Information Technologies, Inc.
#   
# HISTORY:
#  30-Dec-1994 MES  Added error messages to support the Level 0 tools
#  23-Sep-1997 CSWT Added error message to support function PGS_MET_Write()
#                   for any metadata parameter specified in the MCF with
#                   a data location is equal to PGE and Mandatory is set to
#                   TRUE or FALSE has not been set value by the PGE
#  16-Oct-1997 CSWT Added error messages to support function PGS_MET_ConvertToMCF()
#                   for converting the HDF-EOS/non HDF-EOS metadata product file,
#                   in which unset attributes were defined as NOT SET for Data Location 
#                   PGE, NOT SUPPLIED for Data Location MCF, or NOT FOUND for Data
#                   Location NONE, to a .MCF file, and Unable to obtain data type for 
#                   the unset attribute                  
#  21-Oct-1997 CSWT Added error message to support functions PGS_MET_SetAttr() and
#                   PGS_MET_SetAttrF() if the Data Location is MCF and user attends 
#                   to set up the attribute value that already been set up
#  11-Dec-1997 CSWT Added error messages to support functions PGS_MET_SetAttr() and
#                   PGS_MET_SetAttrF() if the length of character string assigned
#                   to be the value of the attribute name exceeds 
#
#                   PGSd_MET_MAX_STRING_SET_L(255) and if the size of array element of 
#                   the attribute value assigned for the attribute name exceeds 
#                   PGSd_MET_MAX_ARRAY_ELEMENT_SIZE(1000)
# 12-June-2000 AT   Added error message PGSMET_E_GETSET_ATTRIBUTE_ERROR to report
#                   errors occuring in PGS_MET_GetSetAttrTD() for PRODUCTIONDATETIME
#                   while called in PGS_MET_Write() (per NCR ECSed27009).
#
# 12-Sep-2000 AT    Added warning messages PGSMET_M_METADATA_NOT_SET_TK, and
#                   PGSMET_M_METADATA_NOT_SET_PGE
# 30-Mar-2001 AT    Added error messages for PGSMET_E_HDF5_FILE_TYPE_ERROR
#                   and PGSMET_E_SD_END
#
#
# END_FILE_PROLOG:
##############################################################################

%INSTR	= PGSTK

%LABEL	= PGSMET

%SEED 	= 13

PGSMET_E_NO_INITIALIZATION	Metadata control file is not yet loaded using PGS_MET_Init()
PGSMET_E_AGGREGATE_ERR		Unable to create odl aggregate %s
PGSMET_E_OPEN_ERR		Unable to open %s file with file id %s
PGSMET_E_ODL_READ_ERR		Unable to create ODL tree %s with file id %s
PGSMET_E_LOAD_ERR		Unable to load %s information
PGSMET_E_DD_UNKNOWN_PARM	The requested parameter %s could not be found in %s
PGSMET_E_UNKNOWN_PARM_ATTR	Attribute %s is not defined for parameter %s
PGSMET_E_CORRUPT_MCF		Unable to find %s definition in the %s
PGSMET_E_PARM_NOT_SET		Mandatory Parameter %s not set in %s
PGSMET_W_NO_MAND_PARM		No Mandatory Parameter found in %s
PGSMET_E_INV_ODL_TYPE		Input odl type is invalid
PGSMET_E_OUTOFRANGE		Value of metadata %s at position %s is out of range
PGSMET_E_DD_ERR			Unable to access the data dictionary to obtain %s of parameter %s
PGSMET_E_MCF_DD_CONFLICT	Conflict with data dictionary for Meta data %s, The data dictionary definition of %s is %sPGSMET_E_PCS_OPEN_ERR		Unable to open PCS file
PGSMET_E_LABEL_NOT_FOUND	Unable to find %s  in file %s
PGSMET_E_CONFIG_VAL_STR_ERR	Unable to obtain the value of configuration parameter %s from the PCS file
PGSMET_E_CONFIG_CONV_ERR	Unable to convert the value of configuration parameter %sfrom the PCS file into an ODL format
PGSMET_E_INCORRECT_VAL		Illegal value of the parameter %s defined in the PC table
PGSMET_E_NO_INVENT_DATA		Granule data section not in the MCF
PGSMET_E_CONFIG_DATA_ERR	Unable to retrieve data from the PC table
PGSMET_E_CONFIRM_ERR		Unable to confirm %s's value against DD
PGSMET_E_LOCATION_ERR		In object %s value %s for parameter DATA_LOCATION is invalid
PGSMET_E_NEW_ODL_DATA_ERR	Unable to create a new odl %s , probably due to lack of memeory
PGSMET_E_INVALID_DATATYPE	Invalid data type definition in DD for parameter %s
PGSMET_E_CHECK_RANGE_ERR	Unable to check the range for metaData %s
PGSMET_E_PCREAD_ERR		Unable to obtain %s from the PC table
PGSMET_E_FILETOODL_ERR		Unable to convert %s into an ODL format
PGSMET_E_SYS_OPEN_ERR		Unable to open pc attribute file
PGSMET_E_ODLTOVAL_ERR		Unable to convert attribute values from the odl format
PGSMET_E_SD_START		Unable to open the HDF file
PGSMET_E_SD_FINDATTR		Unable to get the attr index
PGSMET_E_SD_INFO		Unable to retrieve sd attribute information
PGSMET_E_MALLOC_ERR		Unable to allocate memory for the hdf attribute
PGSMET_E_SD_READ		Unable to read hdf attribute
PGSMET_W_CLASS_TYPE		Illegal class type for parameter %s
PGSMET_E_SD_SETATTR		Unable to set the HDF file attribute
PGSMET_E_GROUP_NOT_FOUND	No group called %s found in the MCF
PGSMET_E_INCOMP_GROUP		Some of the mandatory parameters are not set
PGSMET_E_PCS_OPEN_ERR           Unable to open PCS file
PGSMET_E_ODL_ERROR		"%s:%s"
PGSMET_W_ODL_WARNING		"%s:%s"
PGSMET_M_ODL_INFO		"%s:%s"
PGSMET_E_MINMAX_ERR		Minimum value is greater than the maximum value of parameter %s in %s

PGSMET_E_MCF_TYPE_CONFLICT	Value type in PCF does not match the type given for metadata %s in MCF
PGSMET_E_MCF_NUMVAL_CONFLICT	Number of values in PCF does not match the number defined for metadata %s in MCF
PGSMET_E_NESTED_OBJECTS		Object descriptions enclosing related objects must not be enclosed themselves by other objects
PGSMET_E_ODL_MEM_ALLOC		ODL routine failed to allocate memory
PGSMET_E_CLASS_PARAMETER	Container object must also have class parameter defined
PGSMET_E_METADATA_CHILD		MetaData Objects are not allowed to enclose other objects
PGSMET_W_NOT_MULTIPLE		Object is not supposed to be multiple therefore resetting the value
PGSMET_E_NO_DEFINITION		Unable to obtain %s of metadata %s
PGSMET_E_ILLEGAL_NUMVAL		Illegal NUMVAL definition for metadata %s . It should be an integer
PGSMET_E_ILLEGAL_TYPE		Illegal type definition for metadata %s . It should bea string
PGSMET_E_INV_DATATYPE		Invalid data type definition in MCF for parameter %s
PGSMET_E_PARENT_GROUP		Multiple objects must have enclosing groups around them
PGSMET_E_MANDATORY_FIELD	Mandatory field is not defined for %s
PGSMET_E_MANDATORY_DEF		MANDATORY field type is not correct for metadata %s. It should be a STRING
PGSMET_E_MANDATORY_VALUE	MANDATORY field value is not correct for metadata %s. It should be a TRUE or FALSE
PGSMET_E_LOC_FIELD		Data Location is not defined for %s
PGSMET_E_LOC_DEF		Data Location field type is not correct for metadata %s. It should be a STRING
PGSMET_E_LOC_VALUE		Data Location value is not correct for metadata %s. It should be a PCF, MCF, PGE, or NONE
PGSMET_E_CONTAINER_LEVELS	Metadata objects can only be enclosed by one level of Container objects. The offending object is %s
PGSMET_E_DUPLICATE_ERR		There is a another object with the same name for object %s
PGSMET_E_CLASS			Illegal class definition for metadata %s. It should be always be M
PGSMET_E_CLASS_STATEMENTS	Class statements should be defined for all the sister objects as well as thecontainer objects.The offending object is %s
PGSMET_E_INV_NUMVAL		Illegal NUMVAL value for metadata %s. It should be greater than or equal to 1
PGSMET_E_CLASS_DEF		Illegal class definition for metadata <metedata name>. It shoualways be PGSd_MET_MULTIPLE_FLAG
PGSMET_E_CLASS_TYPE		Illegal class type for metadata %s
PGSMET_E_MAND_NOT_SET		Some of the Mandatory metadata were not set
PGSMET_W_META_NOT_SET       Some of the metadata descriptions were not set 
PGSMET_W_METADATA_NOT_SET	The metadata %s is not yet set
PGSMET_E_FGDC_ERR		Unable to convert UTC to FGDC format for metadata %s
PGSMET_E_OUTFILE_ERR		Unable to open output file
PGSMET_E_PCF_VALUE_ERR		Metadata %s could not be set from a value in PCF file
PGSMET_E_GRP_ERR		Master groups are not supposed to be enclosed under any other group or object.The offending group is %s
PGSMET_E_HDFNOTSET		The value was originally not set in the hdf header
PGSMET_E_HDFFILENAME_ERR	Unable to obtain hdf filename
PGSMET_E_MET_ASCII_ERR		Unable to open MET ascii file
PGSMET_E_ILLEGAL_HANDLE		Handle is illegal. Check that initialization has taken place
PGSMET_E_NUMOFMCF_ERR		Unable to load. The number of MCFs allowed has exceeded
PGSMET_E_GRP_NAME_ERR		Group Name length should not exceed %s
PGSMET_E_INVALID_LOCATION       Invalid data location for setting attribute value %s
PGSMET_E_NULL_PARAMETER         The requested Parameter %s is a NULL value 
PGSMET_E_PARAMETER_NOT_SET      The requested Parameter %s is not set yet 
PGSMET_E_SET_ATTRIBUTE_ERROR    Unable to set attribute for ProductionDateTime
PGSMET_E_SEARCH_FAILED          The requested parameter could not be found after searching the file listing
PGSMET_E_GROUP_NOT_FOUND_IN_L7  No group called %s found in the LANDSAT7 Metadata
PGSMET_W_OBJECT_NOT_SET         Object for data location of PGE is not set 
PGSMET_E_CONVERT_ERR            unable to convert HDF-EOS/non HDF-EOS metadata product file, in which unset attributes were defined as NOT SET for Data Location PGE, NOT SUPPLIED for Data Location MCF, or NOT FOUND for Data Location NONE, to a .MCF file 
PGSMET_E_TYPE_ERR               Unable to obtain data type for the unset attribute 
PGSMET_E_SET_ERR                Attribute value set up in the data locateion MCF is not allowable to be written over 
PGSMET_E_ILLEGAL_LENGTH         The length of character string assigned to be the value of the attribute name %s can not exceed PGSd_MET_MAX_STRING_SET_L(255) 
PGSMET_E_ARRAY_ELEMENT_SIZE     The size of array element of the attribute value assigned for the attribute name %s can not exceed PGSd_MET_MAX_ARRAY_ELEMENT_SIZE(1000) 
PGSMET_E_GETSET_ATTRIBUTE_ERROR Unable to get attribute for ProductionDateTime 
PGSMET_M_METADATA_NOT_SET_TK    Metadata has not been set, TOOLKIT may set it
PGSMET_M_METADATA_NOT_SET_PGE   Metadata has not been set, TOOLKIT will not set it
PGSMET_E_HDF5_FILE_TYPE_ERROR   HDF5 failed determining file type
PGSMET_E_SD_END	                Unable to close the HDF file
