#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <dbccodeconf.h>

#include <testdb.h>

// There is no any RX mapped massage.

typedef struct
{
  SIG_TEST_1_t SIG_TEST_1;
} eps_testdb_tx_t;

#ifdef __DEF_EPS_TESTDB__

extern eps_testdb_tx_t eps_testdb_tx;

#endif // __DEF_EPS_TESTDB__

#ifdef __cplusplus
}
#endif
