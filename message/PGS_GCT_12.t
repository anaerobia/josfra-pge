############################################################################## 
# BEGIN_FILE_PROLOG:
# 
# FILENAME:
#   PGS_GCT_12.t Return code definitions for PGS GCT tools (SMF seed value 12)
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
#   Carol S. W. Tsai // Applied Research Corp.
#   
# HISTORY:
#  30-Dec-1994 MES Added error messages to support the Level 0 tools
#  08-Jan-1995 ANS Added GCTP error messages
#  08-Feb-1995 ANS Added changes due to bug fix ECSed00622
#  11-Jul-1997 CSWT Added error messages to support the MODIS level 3 dataset
# END_FILE_PROLOG:
##############################################################################

%INSTR	= PGSTK

%LABEL	= PGSGCT

%SEED 	= 12

# errors used in PGS_GCT_Init

PGSGCT_E_NO_DATA_FILES		Unable to find data files for State Plane
PGSGCT_E_GCTP_ERROR		Error has occurred in the GCTP library
PGSGCT_E_BAD_INC_ANGLE		Inclination angle of the satellite should be between 0 and PI
PGSGCT_E_BAD_RADIUS		Semi major axis value should be greater than zero
PGSGCT_E_BAD_MINOR_AXIS		Semi minor axis value should be greater than zero
PGSGCT_E_MINOR_GT_MAJOR		Semi major axis greater than Semi minor axis
PGSGCT_E_BAD_LONGITUDE  	Longitude value should range between -PI to PI
PGSGCT_E_BAD_LATITUDE  		Latitude value should range between -PI/2 to PI/2
PGSGCT_E_INVD_SPCS_SPHEROID	Invalid SPCS Spheroid (only CLARK66 or GRS80_WGS84)

# errors used in PGSGCT_Proj
PGSGCT_E_BAD_ZONE		UTM zone value should only be between -60 to 60
PGSGCT_E_BAD_DIRECTION		Direction should either be forward or inverse
PGSGCT_E_INVD_PROJECTION	The given projection does not exist
PGSGCT_E_NO_POINTS		Number of points should be greater tahn or equal to one
PGSGCT_W_INTP_REGION		interrupted region encountered for some of the input points

# GCTP error messages
PGSGCT_E_STD_PARALLEL		Equal latitudes for St. Parallels on opposite sides of equator
PGSGCT_E_ITER_EXCEEDED		Too many iterations in inverse
PGSGCT_E_POINT_PROJECT		Point projects into a circle of radius 2 * PI * radius_major
PGSGCT_E_INPUT_DATA_ERROR	Input data error
PGSGCT_E_STD_PARALLEL_OPP	Standard Parallels on opposite sides of equator
PGSGCT_E_INFINITE		Point projects into infinity
PGSGCT_E_ITER_FAILED		Iteration failed to converge
PGSGCT_E_PROJECT_FAILED		Point cannot be projected
PGSGCT_E_POINTS_ON_POLES	Transformation cannot be computed at the poles
PGSGCT_E_ITER_SOM		50 iterations without conv
PGSGCT_E_SPCS_ZONE		Illegal zone for the given spheroid
PGSGCT_E_SPCS_FILE		Error opening State Plane parameter file
PGSGCT_E_CONV_ERROR		Convergence Error
PGSGCT_E_LAT_15			Latitude failed to converge after 15 iterations
PGSGCT_E_LAT_CONVERGE		Lattitude failed to converge
PGSGCT_E_ISIN_ERROR	        Bad parameter	
PGSGCT_E_DMS_ERROR	        Illegal DMS field	

PGSGCT_E_INVERSE_FAILED         Error in inversing equations--mapping x,y to lat/long 
PGSGCT_E_NO_RMAJORRMINOR        Semi major axis value or Semi minor axis value should be defined
PGSGCT_E_INVALID_SWITCH         Switch for setting or getting Major and Minor should be defined

