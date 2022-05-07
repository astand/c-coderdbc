#include "bms_testdb-binutil.h"

// DBC file version
#if (VER_TESTDB_MAJ != (1U)) || (VER_TESTDB_MIN != (10U))
#error The BMS_TESTDB binutil source file has inconsistency with core dbc lib!
#endif

#ifdef __DEF_BMS_TESTDB__

bms_testdb_rx_t bms_testdb_rx;

bms_testdb_tx_t bms_testdb_tx;

#endif // __DEF_BMS_TESTDB__

uint32_t bms_testdb_Receive(bms_testdb_rx_t* _m, const uint8_t* _d, uint32_t _id, uint8_t dlc_)
{
 uint32_t recid = 0;
 if ((_id >= 0x14DU) && (_id < 0x22BU)) {
  if (_id == 0x14DU) {
   recid = Unpack_UTEST_2_testdb(&(_m->UTEST_2), _d, dlc_);
  } else if (_id == 0x160U) {
   recid = Unpack_EMPTY_0_testdb(&(_m->EMPTY_0), _d, dlc_);
  }
 } else {
  if (_id == 0x22BU) {
   recid = Unpack_UTEST_3_testdb(&(_m->UTEST_3), _d, dlc_);
  } else if (_id == 0x1FFFFFF6U) {
   recid = Unpack_EMPTY_EXT_ID_testdb(&(_m->EMPTY_EXT_ID), _d, dlc_);
  }
 }

 return recid;
}

