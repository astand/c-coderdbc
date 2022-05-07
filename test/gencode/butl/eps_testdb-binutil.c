#include "eps_testdb-binutil.h"

// DBC file version
#if (VER_TESTDB_MAJ != (1U)) || (VER_TESTDB_MIN != (10U))
#error The EPS_TESTDB binutil source file has inconsistency with core dbc lib!
#endif

#ifdef __DEF_EPS_TESTDB__

eps_testdb_tx_t eps_testdb_tx;

#endif // __DEF_EPS_TESTDB__

