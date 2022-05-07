#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <dbccodeconf.h>

#include <testdb.h>

typedef struct
{
  UTEST_2_t UTEST_2;
  EMPTY_EXT_ID_t EMPTY_EXT_ID;
} esp_testdb_rx_t;

typedef struct
{
  EMPTY_0_t EMPTY_0;
} esp_testdb_tx_t;

uint32_t esp_testdb_Receive(esp_testdb_rx_t* m, const uint8_t* d, uint32_t msgid, uint8_t dlc);

#ifdef __DEF_ESP_TESTDB__

extern esp_testdb_rx_t esp_testdb_rx;

extern esp_testdb_tx_t esp_testdb_tx;

#endif // __DEF_ESP_TESTDB__

#ifdef __cplusplus
}
#endif
