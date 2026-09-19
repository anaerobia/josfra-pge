# Purpose: define error messages for PGS_CSC_ tools
# File:    PGS_CSC_4.t
# Author:  various

%INSTR = PGSTK
%LABEL = PGSCSC
%SEED  = 4

PGSCSC_W_JD_OUT_OF_RANGE       input julian day out of range for tabulated corrections
PGSCSC_W_DATA_FILE_MISSING     an open file call failed 
PGSCSC_E_BAD_DIRECTION_FLAG    the TRUE/FALSE flag has invalid value
PGSCSC_E_JD_WRONG_EPOCH        indicates input date not = to J2000 as required


PGSCSC_E_ZERO_INPUT_VECTOR     input vector with zero length when direction is required
PGSCSC_E_BAD_FLAT              error in flattening value used, flattening must be < 1
PGSCSC_E_NEG_OR_ZERO_RAD       radius must be greater than 0.0


PGSCSC_E_BAD_LAT               bad value for latitude
PGSCSC_E_BAD_LON               bad value for longitude
PGSCSC_W_PROLATE_BODY          prolate body assumed
PGSCSC_W_SPHERE_BODY           perfectly spherical body assumed
PGSCSC_W_BELOW_SURFACE         location is below surface
PGSCSC_E_BAD_ROT_AXIS_INDEX    value of rotation axis index invalid
PGSCSC_E_SC_TAG_UNKNOWN        unknown/unsupported spacecraft tag 
PGSCSC_E_TRANS_FLAG_ERR        coordinate system transformation flag not recognized/supported


# errors used by PGS_CSC_ZenithAzimuth()

PGSCSC_E_INVALID_VECTAG        invalid tag for vector whose zenith and azimuth are sought
PGSCSC_W_BELOW_HORIZON         vector whose zenith angle is sought is below horizon
PGSCSC_E_LOOK_PT_ALTIT_RANGE   look point altitude unreasonably low or high
PGSCSC_W_UNDEFINED_AZIMUTH     azimuth worthless because ray is at zenith
  
# errors used by PGS_CSC_SubSatPoint()

PGSCSC_W_SUBTERRANEAN          below the ground 
PGSCSC_W_TOO_MANY_ITERS        poor convergence - suggests bad Earth model or bad data
PGSCSC_W_ZERO_JACOBIAN_DET     Singularity at N or S pole - or bad data
PGSCSC_W_DEFAULT_EARTH_MODEL   Default Earth model used

# errors used by PGS_CSC_GetFOV_Pixel()

PGSCSC_W_MISS_EARTH            look vector does not intersect Earth
PGSCSC_W_INSTRUMENT_OFF_BOARD  user-provided instrument offset exceeds 120 meters
PGSCSC_W_ZERO_PIXEL_VECTOR     user-provided pixel unit vector has zero length
PGSCSC_W_BAD_ACCURACY_FLAG     Value other than PGS_TRUE or PGS_FALSE detected
PGSCSC_W_BAD_EPH_FOR_PIXEL     Bad SC ephemeris values encountered for some pixels

#errors used by PGS_CSC_UTC_UT1Pole()

PGSCSC_E_INACCURATE_UTCPOLE    status of data should be one of 'f' for final, 'p' for predicted or 'i' for interim
PGSCSC_W_INTERIM_UT1           status of data used is interim
PGSCSC_W_PREDICTED_UT1         status of data used is predicted

#stuff for TK3

PGSCSC_E_BAD_QUATERNION        the input quaternion is invalid, the sum of the squares of the components of a quaternion must = 1
PGSCSC_W_ERRORS_IN_GHA         error computing Greenwich Hour angle from either getting UTC time, Julian date, polar motion correction, or GMST time
PGSCSC_W_ERROR_IN_DAYNIGHT     an error occurred in computing at least one after dark value
PGSCSC_W_BAD_TRANSFORM_VALUE   an error occurred in at least one of the transformations
PGSCSC_E_INVALID_LIMITTAG      invalid sun zenith limit tag
PGSCSC_E_BAD_ARRAY_SIZE        incorrect array size
PGSCSC_W_INVALID_ALTITUDE      invalid altitude - probably indicates bad input data
PGSCSC_W_LARGE_FLATTENING      flattening should be about 1/300 for Earth
PGSCSC_W_ERROR_IN_SUBSATPT     an error occurred in computing at least one subsatellite point value
PGSCSC_W_NO_REFRACTION         unable to perform refraction calculation
PGSCSC_E_EULER_REP_INVALID     invalid euler angle representation
PGSCSC_E_EULER_INDEX_ERROR     euler angle index is out of range - should be one of 1, 2 or 3
PGSCSC_W_ACCURACY_SUSPECT      accuracy of the output is suspect
PGSCSC_E_QUAT_NOT_FOUND        the rotation could not be determined

#stuff for TK4

PGSCSC_E_INVALID_FOV_DATA      FOV perimeter vectors are invalid
PGSCSC_E_FOV_TOO_LARGE         FOV specification outside algorithmic limits
PGSCSC_E_INVALID_EARTH_PT      one of the Earth point vectors was zero
PGSCSC_M_EARTH_BLOCKS_CB       Earth blocks the celestial body 
PGSCSC_M_EARTH_BLOCKS_FOV      Earth blocks the FOV
PGSCSC_M_CHECK_EARTH_BULGE     check to see if the Earth's bulge occults the CB

#new for TK5

PGSCSC_E_BAD_EARTH_MODEL       bad Earth model specified (e.g. negative or zero Earth radii or prolate Earth)
PGSCSC_W_ERROR_IN_EARTHPTFOV   an error occurred in determining if point was in the FOV for at least one case
PGSCSC_E_INVALID_ZENITH        invalid zenith angle specified
PGSCSC_E_BELOW_SURFACE         altitude specified is more than 50,000 km below surface of Earth

#new for TK5.2

#errors used by PGS_CSC_UTC_UT1_update()

PGSCSC_E_ABORT_UTCPOLE_UPDATE  error condition in updating - see LogStatus file

#used by PGS_CSC_GrazingRay() - new in TK5.2

PGSCSC_W_ERROR_IN_GRAZINGRAY   an error occurred in a function called by this one
PGSCSC_W_HIT_EARTH             line of sight intended to miss the horizon (i.e. limb sounder) hits Earth
PGSCSC_W_LOOK_AWAY             line of sight of instrument points above the horizontal
