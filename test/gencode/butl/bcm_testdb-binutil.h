#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <dbccodeconf.h>

#include <testdb.h>

typedef struct
{
  FLT_TEST_1_t FLT_TEST_1;
  SIG_TEST_1_t SIG_TEST_1;
  EMPTY_EXT_ID_t EMPTY_EXT_ID;
} bcm_testdb_rx_t;

typedef struct
{
  UTEST_2_t UTEST_2;
  EMPTY_0_t EMPTY_0;
  UTEST_3_t UTEST_3;
  EMPTY_EXT_ID_t EMPTY_EXT_ID;
} bcm_testdb_tx_t;

uint32_t bcm_testdb_Receive(bcm_testdb_rx_t* m, const uint8_t* d, uint32_t msgid, uint8_t dlc);

#ifdef __DEF_BCM_TESTDB__

extern bcm_testdb_rx_t bcm_testdb_rx;

extern bcm_testdb_tx_t bcm_testdb_tx;

#endif // __DEF_BCM_TESTDB__

#ifdef __cplusplus
}
#endif
