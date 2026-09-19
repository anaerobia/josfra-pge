# Purpose: define error messages for PGS_AA_ tools
# File:    PGS_AA_10.t
# Author:  Graham Bland 

%INSTR = PGSTK
%LABEL = PGSAA
%SEED  = 10

# Checkfile flag 

PGSAA_S_NEWFILE	        need to open new file

# errors used in PGSAA_CHECK_ARGS
PGSAA_E_SUPPORTFILE	support or format files inaccessible
PGSAA_E_CHECK_CALL	an error in mapping parm from suppSupport list file
PGSAA_E_PARMSNOTFOUND    parameter(s) not found in the support support file
PGSAA_E_DATARATEUNSET   dataRate attribute unset in support file
PGSAA_E_PARMSFROMANYFILES  parameters requested from more than one physical file
PGSAA_E_INVALIDNOPARMS	no of parms incorrect
PGSAA_E_BADSUPPSUPPORT	tool support file is corrupted or incomplete
PGSAA_E_CANTFINDFILE	format or input data file inaccessible

# PGSAA_FFSETUP and specific FF messages 

PGSAA_E_FFERROR		a freeform error has occured
PGSAA_E_FFDBIN		failure in Freeform make_dbin function
PGSAA_E_FFDBSET		failure in Freeform db_set function
PGSAA_E_FFDBEVENTS	failure in Freeform db_events function
PGSAA_E_MALLOC		failure to malloc 

# errors used by PGSAA_PeV() and tools which call it

PGSAA_E_CANT_PARSE_FILE		cant parse data file
PGSAA_E_PEV_ERROR		an error has occured in the PeV tool
PGSAA_E_PEV_XS_SUPPFILES	too many PeV files open, increase MAXFILES
PGSAA_E_CANT_GET_VALUE		unable to extract value from dbin
PGSAA_E_CANT_GET_FILE		error obtaining file name
PGSAA_E_GETDBIN			error in PeV tool obtaining dbin 


# errors used by PGSAA_PeV_array

PGSAA_E_CANT_GET_FILE_ID		cant map from pevlogical to filename
PGSAA_E_CANT_OPEN_INPUT_FILE		cant open file to create ODL tree
PGSAA_E_AGG_CANT_BE_INSERTED	cant insert a new aggregate into the tree
PGSAA_E_READLABEL_PARSE_ERROR	Pointer to aggregate label parsing error	
PGSAA_E_PARAMETER_INVALID		Parameter not found within input file
PGSAA_E_FIRST_NODE_NOT_FOUND	Parameter node not found
PGSAA_E_PEVA_ERROR			an error has occurred in the PeVarray Tool

#  errors used by PGSAA_GetSupp (most errors reported through PeV specific handling)

PGSAA_E_GETSUPP			an error was detected while extracting support data

# error used by PGSAA_2D/3D Read

PGSAA_E_POSITION_CALC_FAILURE		the position in the parmBuffer of the requested values was miscalculated
PGSAA_E_TWOD_READ_ERROR			function failure to read parameter values from buffer
PGSAA_E_EXTRACTORESULTSERROR		failure to transfer selected values from parmBuffer to results
PGSAA_E_THREED_READ_ERROR		function failure to read parameter values from buffer
PGSAA_E_OUTOFRANGE			input values out of data set range

# errors from PGS_AA_2D/3D geo

PGSAA_E_NPOINTSINVALID		nPOints argument out of range	
PGSAA_E_GEOERROR		error in GEO extraction 
PGSAA_E_AUTOOPERATION		error in executing autoOperation
PGSAA_E_AUTOOPERATIONUNSET	no autOperation found in support file
PGSAA_E_OPERATION		error in executing operation
PGSAA_E_OPERATIONUNSET		operation not set by user
PGSAA_E_GEOTOSTRUCT		failure in calculation of structure from lat/lon

#errors from PGS_AA_dcw

PGSAA_E_DCW_ERROR		DCW error
PGSAA_W_DCW_NODATA		No data found in data base
PGSAA_E_CANT_FIND_PARM		Parm input from user cannot be found in dbase
PGSAA_E_CANT_GET_POINT_INFO	Cant extract point information
PGSAA_E_CANT_GET_DATABASE_PATH	Location of data base information not found
PGSAA_E_CANT_GET_AFT_PATH	Cant get path to Area Feature Table
PGSAA_E_NPARMSINVALID		Number of parameters for input invalid
PGSAA_E_CANT_FIND_FACE		Cannot find location in any data base faces

#errors from PGS_AA_dem

PGSAA_E_TILE_STATUS		could not establish tile status of the DEM file
PGSAA_E_2DGEO			error returned from PGS_AA_2Dgeo
PGSAA_E_SUPPORTID		could not establish support file id
PGSAA_E_MINMAX			could not establish min/max range for the DEM
PGSAA_E_DATATYPE		could not establish parameter datatype
PGSAA_E_UNKNOWN_DATATYPE	DEM datafile datatype is unknown
PGSAA_W_MISSING_POINTS		Some points were not located in the DEM
