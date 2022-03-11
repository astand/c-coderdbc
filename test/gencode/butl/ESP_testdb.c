#include "ESP_testdb.h"

#ifdef __DEF_TESTDB__

testdb_rx_t testdb_rx;

testdb_tx_t testdb_tx;

#endif // __DEF_TESTDB__

uint32_t testdb_Receive(testdb_rx_t* _m, const uint8_t* _d, uint32_t _id, uint8_t dlc_)
{
 uint32_t recid = 0;
 if (_id == 0x14DU) {
  recid = Unpack_UTEST_2_testdb(&(_m->UTEST_2), _d, dlc_);
 } else if (_id == 0x1FFFFFF6U) {
  recid = Unpack_EMPTY_EXT_ID_testdb(&(_m->EMPTY_EXT_ID), _d, dlc_);
 }

 return recid;
}

