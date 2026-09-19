# Purpose: define error messages for PGS_DEM tools
# File: PGS_DEM_14.t
# Author: Alexis Zubrow
# Abe Taaheri added PGSDEM_W_HDFEOS

%INSTR = PGSTK
%LABEL = PGSDEM
%SEED = 14


PGSDEM_E_IMPROPER_TAG         improper resolution tag or layer...%s
PGSDEM_E_CANNOT_ACCESS_DATA   cannot access datasets...%s
PGSDEM_E_HDFEOS               problem with accessing HDF-EOS file
PGSDEM_M_FILLVALUE_INCLUDED   fill values included in final output
PGSDEM_M_MULTIPLE_RESOLUTIONS data accessed from multiple resolutions
PGSDEM_W_HDFEOS               attr. layers_existvalue is not in HDF-EOS file
 
