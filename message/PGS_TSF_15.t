#
#  This file handles the Error/Status reporting for the
#  Thread-Safe Functionality (TSF) portion of the PGS Toolkit
#
#  FILE:  PGS_TSF_15.t
#  AUTHOR:  Ray Milburn
#

%INSTR = PGSTK
%LABEL = PGSTSF
%SEED = 15

PGSTSF_E_NOT_THREADSAFE      The toolkit is not built in threadsafe mode.
PGSTSF_E_MUTEXINIT_FAIL      The mutex failed to initialize.
PGSTSF_E_MUTEXLOCK_FAIL      The mutex failed to lock.
PGSTSF_W_MUTEXUNLOCK_FAIL    Unlock operation had a problem.
PGSTSF_E_KEYCREATE_FAIL      The key create failed.
PGSTSF_E_GENERAL_FAILURE     There was a problem in TSF.
