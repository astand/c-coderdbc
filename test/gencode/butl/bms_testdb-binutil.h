#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <dbccodeconf.h>

#include <testdb.h>

typedef struct
{
  UTEST_2_t UTEST_2;
  EMPTY_0_t EMPTY_0;
  UTEST_3_t UTEST_3;
  EMPTY_EXT_ID_t EMPTY_EXT_ID;
} bms_testdb_rx_t;

typedef struct
{
  EMPTY_0_t EMPTY_0;
  FLT_TEST_1_t FLT_TEST_1;
} bms_testdb_tx_t;

uint32_t bms_testdb_Receive(bms_testdb_rx_t* m, const uint8_t* d, uint32_t msgid, uint8_t dlc);

#ifdef __DEF_BMS_TESTDB__

extern bms_testdb_rx_t bms_testdb_rx;

extern bms_testdb_tx_t bms_testdb_tx;

#endif // __DEF_BMS_TESTDB__

#ifdef __cplusplus
}
#endif
