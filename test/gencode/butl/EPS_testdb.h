#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "dbccodeconf.h"

#include "testdb.h"

// This version definition comes from main driver version and
// can be compared in user code space for strict compatibility test
#define VER_TESTDB_MAJ (1U)
#define VER_TESTDB_MIN (10U)

// There is no any RX mapped massage.

typedef struct
{
  SIG_TEST_1_t SIG_TEST_1;
} testdb_tx_t;

#ifdef __DEF_TESTDB__

extern testdb_tx_t testdb_tx;

#endif // __DEF_TESTDB__

#ifdef __cplusplus
}
#endif
