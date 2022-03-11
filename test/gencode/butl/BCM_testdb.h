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

typedef struct
{
  FLT_TEST_1_t FLT_TEST_1;
  SIG_TEST_1_t SIG_TEST_1;
  EMPTY_EXT_ID_t EMPTY_EXT_ID;
} testdb_rx_t;

typedef struct
{
  UTEST_2_t UTEST_2;
  EMPTY_0_t EMPTY_0;
  UTEST_3_t UTEST_3;
  EMPTY_EXT_ID_t EMPTY_EXT_ID;
} testdb_tx_t;

uint32_t testdb_Receive(testdb_rx_t* m, const uint8_t* d, uint32_t msgid, uint8_t dlc);

#ifdef __DEF_TESTDB__

extern testdb_rx_t testdb_rx;

extern testdb_tx_t testdb_tx;

#endif // __DEF_TESTDB__

#ifdef __cplusplus
}
#endif
