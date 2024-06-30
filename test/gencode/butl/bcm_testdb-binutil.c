// DBC filename     : testdb.dbc
// Generator version: v3.1
// Generation time  : 2024.06.30 12:22:02
#include "bcm_testdb-binutil.h"

// DBC file version
#if (VER_TESTDB_MAJ != (1U)) || (VER_TESTDB_MIN != (10U))
#error The BCM_TESTDB binutil source file has inconsistency with core dbc lib!
#endif

#ifdef __DEF_BCM_TESTDB__

bcm_testdb_rx_t bcm_testdb_rx;

bcm_testdb_tx_t bcm_testdb_tx;

#endif // __DEF_BCM_TESTDB__

uint32_t bcm_testdb_Receive(bcm_testdb_rx_t* _m, const uint8_t* _d, uint32_t _id, uint8_t dlc_)
{
 uint32_t recid = 0;
 if (_id == 0x360U) {
  recid = Unpack_FLT_TEST_1_testdb(&(_m->FLT_TEST_1), _d, dlc_);
 } else {
  if (_id == 0x777U) {
   recid = Unpack_SIG_TEST_1_testdb(&(_m->SIG_TEST_1), _d, dlc_);
  } else if (_id == 0x1FFFFFF6U) {
   recid = Unpack_EMPTY_EXT_ID_testdb(&(_m->EMPTY_EXT_ID), _d, dlc_);
  }
 }

 return recid;
}

