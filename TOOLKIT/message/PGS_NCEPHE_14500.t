############################################################################## 
# BEGIN_FILE_PROLOG:
# 
# FILENAME:
#   PGS_IO_1.t 	Return code definitions for PGS IO tools (SMF seed value 1)
#   
# 
# DESCRIPTION:
# 
#   This file contains PGS_SMF standard return code definitions for the
#   PGS_IO group of tools.
#
#   The file is intended to be used as input by the smfcompile utility,
#   which generates the message file, and the C and FORTRAN header files.
#
# AUTHOR:
#   Abe Taaheri / L3 Comm. EER Inc.
#   
# HISTORY:
#
#
# END_FILE_PROLOG:
##############################################################################

%INSTR	= NCEP

%LABEL	= NCEPHE

%SEED 	= 14500

NCEPHE_U_INFO1		Info: %s
NCEPHE_U_INFO2		Info: %s %s
NCEPHE_U_INFO3		Info: %s %s %s
NCEPHE_U_INT01		Info: %s %d
NCEPHE_U_INT02		Info: %s %d %d
NCEPHE_E_INFO1		Error: %s
NCEPHE_E_INFO2		Error: %s %s
NCEPHE_E_INFO4		Error: %s %s %s %s
NCEPHE_E_INT01		Error: %s %d

