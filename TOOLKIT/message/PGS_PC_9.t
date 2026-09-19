#
#  This file handles the Error/Status reporting for the
#  Process Control Tool (PC) portion of the PGS Toolkit
#
#  FILE:  PGS_PC_9.t
#  AUTHOR:  Ray Milburn
#

%INSTR = PGSTK
%LABEL = PGSPC
%SEED = 9

PGSPC_E_FILE_OPEN_ERROR     Error opening file:  %s
PGSPC_E_INVALID_MODE        Mode value not defined:  %d
PGSPC_W_NO_FILES_EXIST      File not found in product group:  %d
PGSPC_W_NO_CONFIG_VALUE     No configuration value exists for logical
PGSPC_E_FILE_READ_ERROR     Error while reading from Process Control file:  %s
PGSPC_E_LINE_FORMAT_ERROR   Incorrect format in line of file:  %s
PGSPC_E_ENVIRONMENT_ERROR   Environment variable not set:  %s
PGSPC_W_NO_DATA_PRESENT     The data requested is not in the line found.
PGSPC_E_DATA_ACCESS_ERROR   Error accessing Process Control Status data.
PGSPC_W_NO_REFERENCE_FOUND  No reference was found matching Product ID (%d) and Version number (%d).
PGSPC_W_NO_CONFIG_FOR_ID    The Product ID does not contain a configuration value.
PGSPC_W_NO_FILES_FOR_ID     The Logical ID (%d) does not contain a physical File Name.
PGSPC_W_NO_ATTR_FOR_ID      The reference does not contain an attribute.
PGSPC_W_TRUNCATED           The value was truncated.
PGSPC_W_ATTR_TRUNCATED      The attribute was truncated due to maxSize being exceeded.
PGSPC_W_NO_ATTR_MATCH       No attribute was found that matches the attribute passed in.
PGSPC_W_INVALID_VERSION     Version number must be greater than or equal to one (1).
PGSPC_W_FILE_NOT_ON_DISK    The file requested to be removed was not on disk:  %s
PGSPC_E_NULL_DIVPOINTER_SHM Division pointer equal to NULL - PCF was invalid at initialization.
PGSPC_E_INDEX_ERROR_SHM     Logical ID error from PCF.
PGSPC_W_NULL_DATA_SHM       Data field is NULL.
PGSPC_E_INV_DIVPOINTER_SHM  Division pointer not pointing to divider symbol.
PGSPC_E_EXCEEDED_SHM        Exceeded amount of shared memory requested - write failed.
PGSPC_E_NO_DEFAULT_LOC      Default file location not specified.
PGSPC_E_ENV_NOT_PC          Environment not defined by Process Control Tools:  %s
PGSPC_C_PGSINFO_PGE_START   PGE Processing starting up.
PGSPC_C_PGSINFO_PGE_STOP    PGE Processing will NOT be performed.
PGSPC_C_PGSINFO_TERM_START  Toolkit Termination Procedure is underway.
PGSPC_M_TOOLKIT_INIT_PASS   Toolkit Initialization Procedure has succeeded! ::PGSPC_C_PGSINFO_PGE_START
PGSPC_E_TOOLKIT_INIT_FAIL   Toolkit Initialization Procedure has failed! ::PGSPC_C_PGSINFO_PGE_STOP
PGSPC_M_TOOLKIT_TERM_BEGIN  Initialization or PGE Processing has completed. ::PGSPC_C_PGSINFO_TERM_START
PGSPC_E_PCF_UPDATE_FAILED   PCF Update failed - errno = %d.
PGSPC_W_PCF_CLEANUP_WARN    PCF Cleanup warning - errno = %d.
PGSPC_W_NO_UREF_DATA        The Product ID contains no Universal Reference.
