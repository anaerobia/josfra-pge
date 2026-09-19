# Purpose:  define error messages for PGS_CUC_ tools
# File:	    PGS_CUC_11.t
# Author:   Richard Morris

%INSTR = PGSTK
%LABEL = PGSCUC
%SEED  = 11

# errors used in PGS_CUC_Cons

PGSCUC_E_CANT_GET_FILE_ID		No matching filepath to input fileid
PGSCUC_E_CANT_OPEN_INPUT_FILE		Cant open input file
PGSCUC_E_AGG_CANT_BE_INSERTED		Base node, aggregate cannot be formed
PGSCUC_E_READLABEL_PARSE_ERROR		Pointer to aggregate label parsing error
PGSCUC_E_PARAMETER_INVALID		Parameter not found within input file
PGSCUC_E_FIRST_NODE_NOT_FOUND		Parameter node not found
PGSCUC_E__ERROR				CUC runtime error

# errors used in PGS_CUC_Conv

PGSCUC_E_COULDNT_INIT_UDUNITS3		Couldnt initialise Data file
PGSCUC_E_DONT_KNOW_INP_UNIT		Input unit is not known
PGSCUC_E_DONT_KNOW_OUTP_UNIT		Output unit is not known
PGSCUC_E_UNITS_ARE_INCOMPATIBLE		No conversion between units
PGSCUC_E_A_UNIT_IS_CORRUPTED		A unit in the data file is corrupted			
