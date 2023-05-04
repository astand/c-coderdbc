#include "testdb.h"


// DBC file version
#if (VER_TESTDB_MAJ != (1U)) || (VER_TESTDB_MIN != (10U))
#error The TESTDB dbc source files have different versions
#endif

#ifdef TESTDB_USE_DIAG_MONITORS
// Function prototypes to be called each time CAN frame is unpacked
// FMon function may detect RC, CRC or DLC violation
#include <testdb-fmon.h>

#endif // TESTDB_USE_DIAG_MONITORS


// This function performs extension of sign for the signals
// which have non-aligned to power of 2 bit's width.
// The types 'bitext_t' and 'ubitext_t' define maximal bit width which
// can be correctly handled. You need to select type which can contain
// n+1 bits where n is the largest signed signal width. For example if
// the most wide signed signal has a width of 31 bits you need to set
// bitext_t as int32_t and ubitext_t as uint32_t
// Defined these typedefs in @dbccodeconf.h or locally in 'dbcdrvname'-config.h
static bitext_t __ext_sig__(ubitext_t val, uint8_t bits)
{
  ubitext_t const m = 1u << (bits - 1);
  return (val ^ m) - m;
}

uint32_t Unpack_UTEST_2_testdb(UTEST_2_t* _m, const uint8_t* _d, uint8_t dlc_)
{
  (void)dlc_;
  _m->U28_TEST_1 = ((_d[3] & (0x0FU)) << 24) | ((_d[2] & (0xFFU)) << 16) | ((_d[1] & (0xFFU)) << 8) | (_d[0] & (0xFFU));
  _m->ValTest = ((_d[3] >> 5) & (0x03U));
  _m->U8_TEST_1 = (_d[4] & (0xFFU));
  _m->U7_TEST_1_ro = ((_d[5] >> 1) & (0x7FU));
#ifdef TESTDB_USE_SIGFLOAT
  _m->U7_TEST_1_phys = TESTDB_U7_TEST_1_ro_fromS(_m->U7_TEST_1_ro);
#endif // TESTDB_USE_SIGFLOAT

#ifdef TESTDB_USE_DIAG_MONITORS
  _m->mon1.dlc_error = (dlc_ < UTEST_2_DLC);
  _m->mon1.last_cycle = GetSystemTick();
  _m->mon1.frame_cnt++;

  FMon_UTEST_2_testdb(&_m->mon1, UTEST_2_CANID);
#endif // TESTDB_USE_DIAG_MONITORS

  return UTEST_2_CANID;
}

#ifdef TESTDB_USE_CANSTRUCT

uint32_t Pack_UTEST_2_testdb(UTEST_2_t* _m, __CoderDbcCanFrame_t__* cframe)
{
  uint8_t i; for (i = 0; (i < UTEST_2_DLC) && (i < 8); cframe->Data[i++] = 0);

#ifdef TESTDB_USE_SIGFLOAT
  _m->U7_TEST_1_ro = TESTDB_U7_TEST_1_ro_toS(_m->U7_TEST_1_phys);
#endif // TESTDB_USE_SIGFLOAT

  cframe->Data[0] |= (_m->U28_TEST_1 & (0xFFU));
  cframe->Data[1] |= ((_m->U28_TEST_1 >> 8) & (0xFFU));
  cframe->Data[2] |= ((_m->U28_TEST_1 >> 16) & (0xFFU));
  cframe->Data[3] |= ((_m->U28_TEST_1 >> 24) & (0x0FU)) | ((_m->ValTest & (0x03U)) << 5);
  cframe->Data[4] |= (_m->U8_TEST_1 & (0xFFU));
  cframe->Data[5] |= ((_m->U7_TEST_1_ro & (0x7FU)) << 1);

  cframe->MsgId = UTEST_2_CANID;
  cframe->DLC = UTEST_2_DLC;
  cframe->IDE = UTEST_2_IDE;
  return UTEST_2_CANID;
}

#else

uint32_t Pack_UTEST_2_testdb(UTEST_2_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide)
{
  uint8_t i; for (i = 0; (i < UTEST_2_DLC) && (i < 8); _d[i++] = 0);

#ifdef TESTDB_USE_SIGFLOAT
  _m->U7_TEST_1_ro = TESTDB_U7_TEST_1_ro_toS(_m->U7_TEST_1_phys);
#endif // TESTDB_USE_SIGFLOAT

  _d[0] |= (_m->U28_TEST_1 & (0xFFU));
  _d[1] |= ((_m->U28_TEST_1 >> 8) & (0xFFU));
  _d[2] |= ((_m->U28_TEST_1 >> 16) & (0xFFU));
  _d[3] |= ((_m->U28_TEST_1 >> 24) & (0x0FU)) | ((_m->ValTest & (0x03U)) << 5);
  _d[4] |= (_m->U8_TEST_1 & (0xFFU));
  _d[5] |= ((_m->U7_TEST_1_ro & (0x7FU)) << 1);

  *_len = UTEST_2_DLC;
  *_ide = UTEST_2_IDE;
  return UTEST_2_CANID;
}

#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_EMPTY_0_testdb(EMPTY_0_t* _m, const uint8_t* _d, uint8_t dlc_)
{
  (void)dlc_;
  _m->CS = ((_d[4] >> 1) & (0x3FU));
  _m->RC = ((_d[6] & (0x07U)) << 1) | ((_d[5] >> 7) & (0x01U));

#ifdef TESTDB_USE_DIAG_MONITORS
  _m->mon1.dlc_error = (dlc_ < EMPTY_0_DLC);
  _m->mon1.last_cycle = GetSystemTick();
  _m->mon1.frame_cnt++;

#ifdef TESTDB_AUTO_ROLL
  _m->mon1.roll_error = (_m->RC != _m->RC_expt);
  _m->RC_expt = (_m->RC + 1) & (0x0FU);
#endif // TESTDB_AUTO_ROLL

#ifdef TESTDB_AUTO_CSM
  _m->mon1.csm_error = (((uint8_t)GetFrameHash(_d, EMPTY_0_DLC, EMPTY_0_CANID, kXor8, 1)) != (_m->CS));
#endif // TESTDB_AUTO_CSM

  FMon_EMPTY_0_testdb(&_m->mon1, EMPTY_0_CANID);
#endif // TESTDB_USE_DIAG_MONITORS

  return EMPTY_0_CANID;
}

#ifdef TESTDB_USE_CANSTRUCT

uint32_t Pack_EMPTY_0_testdb(EMPTY_0_t* _m, __CoderDbcCanFrame_t__* cframe)
{
  uint8_t i; for (i = 0; (i < EMPTY_0_DLC) && (i < 8); cframe->Data[i++] = 0);

#ifdef TESTDB_AUTO_ROLL
  _m->RC = (_m->RC + 1) & (0x0FU);
#endif // TESTDB_AUTO_ROLL

#ifdef TESTDB_AUTO_CSM
  _m->CS = 0U;
#endif // TESTDB_AUTO_CSM

  cframe->Data[4] |= ((_m->CS & (0x3FU)) << 1);
  cframe->Data[5] |= ((_m->RC & (0x01U)) << 7);
  cframe->Data[6] |= ((_m->RC >> 1) & (0x07U));

#ifdef TESTDB_AUTO_CSM
  _m->CS = ((uint8_t)GetFrameHash(cframe->Data, EMPTY_0_DLC, EMPTY_0_CANID, kXor8, 1));
  cframe->Data[4] |= ((_m->CS & (0x3FU)) << 1);
#endif // TESTDB_AUTO_CSM

  cframe->MsgId = EMPTY_0_CANID;
  cframe->DLC = EMPTY_0_DLC;
  cframe->IDE = EMPTY_0_IDE;
  return EMPTY_0_CANID;
}

#else

uint32_t Pack_EMPTY_0_testdb(EMPTY_0_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide)
{
  uint8_t i; for (i = 0; (i < EMPTY_0_DLC) && (i < 8); _d[i++] = 0);

#ifdef TESTDB_AUTO_ROLL
  _m->RC = (_m->RC + 1) & (0x0FU);
#endif // TESTDB_AUTO_ROLL

#ifdef TESTDB_AUTO_CSM
  _m->CS = 0U;
#endif // TESTDB_AUTO_CSM

  _d[4] |= ((_m->CS & (0x3FU)) << 1);
  _d[5] |= ((_m->RC & (0x01U)) << 7);
  _d[6] |= ((_m->RC >> 1) & (0x07U));

#ifdef TESTDB_AUTO_CSM
  _m->CS = ((uint8_t)GetFrameHash(_d, EMPTY_0_DLC, EMPTY_0_CANID, kXor8, 1));
  _d[4] |= ((_m->CS & (0x3FU)) << 1);
#endif // TESTDB_AUTO_CSM

  *_len = EMPTY_0_DLC;
  *_ide = EMPTY_0_IDE;
  return EMPTY_0_CANID;
}

#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_UTEST_3_testdb(UTEST_3_t* _m, const uint8_t* _d, uint8_t dlc_)
{
  (void)dlc_;
  _m->U32_TEST_1 = ((_d[3] & (0xFFU)) << 24) | ((_d[2] & (0xFFU)) << 16) | ((_d[1] & (0xFFU)) << 8) | (_d[0] & (0xFFU));
  _m->TestValTableID = (_d[4] & (0x07U));

#ifdef TESTDB_USE_DIAG_MONITORS
  _m->mon1.dlc_error = (dlc_ < UTEST_3_DLC);
  _m->mon1.last_cycle = GetSystemTick();
  _m->mon1.frame_cnt++;

  FMon_UTEST_3_testdb(&_m->mon1, UTEST_3_CANID);
#endif // TESTDB_USE_DIAG_MONITORS

  return UTEST_3_CANID;
}

#ifdef TESTDB_USE_CANSTRUCT

uint32_t Pack_UTEST_3_testdb(UTEST_3_t* _m, __CoderDbcCanFrame_t__* cframe)
{
  uint8_t i; for (i = 0; (i < UTEST_3_DLC) && (i < 8); cframe->Data[i++] = 0);

  cframe->Data[0] |= (_m->U32_TEST_1 & (0xFFU));
  cframe->Data[1] |= ((_m->U32_TEST_1 >> 8) & (0xFFU));
  cframe->Data[2] |= ((_m->U32_TEST_1 >> 16) & (0xFFU));
  cframe->Data[3] |= ((_m->U32_TEST_1 >> 24) & (0xFFU));
  cframe->Data[4] |= (_m->TestValTableID & (0x07U));

  cframe->MsgId = UTEST_3_CANID;
  cframe->DLC = UTEST_3_DLC;
  cframe->IDE = UTEST_3_IDE;
  return UTEST_3_CANID;
}

#else

uint32_t Pack_UTEST_3_testdb(UTEST_3_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide)
{
  uint8_t i; for (i = 0; (i < UTEST_3_DLC) && (i < 8); _d[i++] = 0);

  _d[0] |= (_m->U32_TEST_1 & (0xFFU));
  _d[1] |= ((_m->U32_TEST_1 >> 8) & (0xFFU));
  _d[2] |= ((_m->U32_TEST_1 >> 16) & (0xFFU));
  _d[3] |= ((_m->U32_TEST_1 >> 24) & (0xFFU));
  _d[4] |= (_m->TestValTableID & (0x07U));

  *_len = UTEST_3_DLC;
  *_ide = UTEST_3_IDE;
  return UTEST_3_CANID;
}

#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_FLT_TEST_1_testdb(FLT_TEST_1_t* _m, const uint8_t* _d, uint8_t dlc_)
{
  (void)dlc_;
  _m->ValTable = (_d[0] & (0x03U));
  _m->Position = ((_d[0] >> 4) & (0x0FU));
  _m->INT_TEST_2_ro = __ext_sig__(( ((_d[1] >> 1) & (0x7FU)) ), 7);
#ifdef TESTDB_USE_SIGFLOAT
  _m->INT_TEST_2_phys = TESTDB_INT_TEST_2_ro_fromS(_m->INT_TEST_2_ro);
#endif // TESTDB_USE_SIGFLOAT

  _m->RC = (_d[2] & (0x0FU));
  _m->CS = ((_d[2] >> 4) & (0x0FU));
  _m->Accel_ro = ((_d[4] & (0x0FU)) << 8) | (_d[3] & (0xFFU));
#ifdef TESTDB_USE_SIGFLOAT
  _m->Accel_phys = (sigfloat_t)(TESTDB_Accel_ro_fromS(_m->Accel_ro));
#endif // TESTDB_USE_SIGFLOAT

  _m->FLT4_TEST_1_ro = ((_d[4] >> 4) & (0x0FU));
#ifdef TESTDB_USE_SIGFLOAT
  _m->FLT4_TEST_1_phys = (sigfloat_t)(TESTDB_FLT4_TEST_1_ro_fromS(_m->FLT4_TEST_1_ro));
#endif // TESTDB_USE_SIGFLOAT

  _m->FLT4_TEST_2_ro = (_d[5] & (0x0FU));
#ifdef TESTDB_USE_SIGFLOAT
  _m->FLT4_TEST_2_phys = (sigfloat_t)(TESTDB_FLT4_TEST_2_ro_fromS(_m->FLT4_TEST_2_ro));
#endif // TESTDB_USE_SIGFLOAT

  _m->FLT4_TEST_3_ro = ((_d[5] >> 4) & (0x0FU));
#ifdef TESTDB_USE_SIGFLOAT
  _m->FLT4_TEST_3_phys = (sigfloat_t)(TESTDB_FLT4_TEST_3_ro_fromS(_m->FLT4_TEST_3_ro));
#endif // TESTDB_USE_SIGFLOAT

  _m->INT_TEST_1_ro = ((_d[6] >> 2) & (0x0FU));
#ifdef TESTDB_USE_SIGFLOAT
  _m->INT_TEST_1_phys = TESTDB_INT_TEST_1_ro_fromS(_m->INT_TEST_1_ro);
#endif // TESTDB_USE_SIGFLOAT

#ifdef TESTDB_USE_DIAG_MONITORS
  _m->mon1.dlc_error = (dlc_ < FLT_TEST_1_DLC);
  _m->mon1.last_cycle = GetSystemTick();
  _m->mon1.frame_cnt++;

#ifdef TESTDB_AUTO_ROLL
  _m->mon1.roll_error = (_m->RC != _m->RC_expt);
  _m->RC_expt = (_m->RC + 1) & (0x0FU);
#endif // TESTDB_AUTO_ROLL

#ifdef TESTDB_AUTO_CSM
  _m->mon1.csm_error = (((uint8_t)GetFrameHash(_d, FLT_TEST_1_DLC, FLT_TEST_1_CANID, kXor4, 1)) != (_m->CS));
#endif // TESTDB_AUTO_CSM

  FMon_FLT_TEST_1_testdb(&_m->mon1, FLT_TEST_1_CANID);
#endif // TESTDB_USE_DIAG_MONITORS

  return FLT_TEST_1_CANID;
}

#ifdef TESTDB_USE_CANSTRUCT

uint32_t Pack_FLT_TEST_1_testdb(FLT_TEST_1_t* _m, __CoderDbcCanFrame_t__* cframe)
{
  uint8_t i; for (i = 0; (i < FLT_TEST_1_DLC) && (i < 8); cframe->Data[i++] = 0);

#ifdef TESTDB_AUTO_ROLL
  _m->RC = (_m->RC + 1) & (0x0FU);
#endif // TESTDB_AUTO_ROLL

#ifdef TESTDB_AUTO_CSM
  _m->CS = 0U;
#endif // TESTDB_AUTO_CSM

#ifdef TESTDB_USE_SIGFLOAT
  _m->INT_TEST_2_ro = TESTDB_INT_TEST_2_ro_toS(_m->INT_TEST_2_phys);
  _m->Accel_ro = TESTDB_Accel_ro_toS(_m->Accel_phys);
  _m->FLT4_TEST_1_ro = TESTDB_FLT4_TEST_1_ro_toS(_m->FLT4_TEST_1_phys);
  _m->FLT4_TEST_2_ro = TESTDB_FLT4_TEST_2_ro_toS(_m->FLT4_TEST_2_phys);
  _m->FLT4_TEST_3_ro = TESTDB_FLT4_TEST_3_ro_toS(_m->FLT4_TEST_3_phys);
  _m->INT_TEST_1_ro = TESTDB_INT_TEST_1_ro_toS(_m->INT_TEST_1_phys);
#endif // TESTDB_USE_SIGFLOAT

  cframe->Data[0] |= (_m->ValTable & (0x03U)) | ((_m->Position & (0x0FU)) << 4);
  cframe->Data[1] |= ((_m->INT_TEST_2_ro & (0x7FU)) << 1);
  cframe->Data[2] |= (_m->RC & (0x0FU)) | ((_m->CS & (0x0FU)) << 4);
  cframe->Data[3] |= (_m->Accel_ro & (0xFFU));
  cframe->Data[4] |= ((_m->Accel_ro >> 8) & (0x0FU)) | ((_m->FLT4_TEST_1_ro & (0x0FU)) << 4);
  cframe->Data[5] |= (_m->FLT4_TEST_2_ro & (0x0FU)) | ((_m->FLT4_TEST_3_ro & (0x0FU)) << 4);
  cframe->Data[6] |= ((_m->INT_TEST_1_ro & (0x0FU)) << 2);

#ifdef TESTDB_AUTO_CSM
  _m->CS = ((uint8_t)GetFrameHash(cframe->Data, FLT_TEST_1_DLC, FLT_TEST_1_CANID, kXor4, 1));
  cframe->Data[2] |= ((_m->CS & (0x0FU)) << 4);
#endif // TESTDB_AUTO_CSM

  cframe->MsgId = FLT_TEST_1_CANID;
  cframe->DLC = FLT_TEST_1_DLC;
  cframe->IDE = FLT_TEST_1_IDE;
  return FLT_TEST_1_CANID;
}

#else

uint32_t Pack_FLT_TEST_1_testdb(FLT_TEST_1_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide)
{
  uint8_t i; for (i = 0; (i < FLT_TEST_1_DLC) && (i < 8); _d[i++] = 0);

#ifdef TESTDB_AUTO_ROLL
  _m->RC = (_m->RC + 1) & (0x0FU);
#endif // TESTDB_AUTO_ROLL

#ifdef TESTDB_AUTO_CSM
  _m->CS = 0U;
#endif // TESTDB_AUTO_CSM

#ifdef TESTDB_USE_SIGFLOAT
  _m->INT_TEST_2_ro = TESTDB_INT_TEST_2_ro_toS(_m->INT_TEST_2_phys);
  _m->Accel_ro = TESTDB_Accel_ro_toS(_m->Accel_phys);
  _m->FLT4_TEST_1_ro = TESTDB_FLT4_TEST_1_ro_toS(_m->FLT4_TEST_1_phys);
  _m->FLT4_TEST_2_ro = TESTDB_FLT4_TEST_2_ro_toS(_m->FLT4_TEST_2_phys);
  _m->FLT4_TEST_3_ro = TESTDB_FLT4_TEST_3_ro_toS(_m->FLT4_TEST_3_phys);
  _m->INT_TEST_1_ro = TESTDB_INT_TEST_1_ro_toS(_m->INT_TEST_1_phys);
#endif // TESTDB_USE_SIGFLOAT

  _d[0] |= (_m->ValTable & (0x03U)) | ((_m->Position & (0x0FU)) << 4);
  _d[1] |= ((_m->INT_TEST_2_ro & (0x7FU)) << 1);
  _d[2] |= (_m->RC & (0x0FU)) | ((_m->CS & (0x0FU)) << 4);
  _d[3] |= (_m->Accel_ro & (0xFFU));
  _d[4] |= ((_m->Accel_ro >> 8) & (0x0FU)) | ((_m->FLT4_TEST_1_ro & (0x0FU)) << 4);
  _d[5] |= (_m->FLT4_TEST_2_ro & (0x0FU)) | ((_m->FLT4_TEST_3_ro & (0x0FU)) << 4);
  _d[6] |= ((_m->INT_TEST_1_ro & (0x0FU)) << 2);

#ifdef TESTDB_AUTO_CSM
  _m->CS = ((uint8_t)GetFrameHash(_d, FLT_TEST_1_DLC, FLT_TEST_1_CANID, kXor4, 1));
  _d[2] |= ((_m->CS & (0x0FU)) << 4);
#endif // TESTDB_AUTO_CSM

  *_len = FLT_TEST_1_DLC;
  *_ide = FLT_TEST_1_IDE;
  return FLT_TEST_1_CANID;
}

#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_SIG_TEST_1_testdb(SIG_TEST_1_t* _m, const uint8_t* _d, uint8_t dlc_)
{
  (void)dlc_;
  _m->sig15_ro = __ext_sig__(( ((_d[1] & (0x7FU)) << 8) | (_d[0] & (0xFFU)) ), 15);
#ifdef TESTDB_USE_SIGFLOAT
  _m->sig15_phys = TESTDB_sig15_ro_fromS(_m->sig15_ro);
#endif // TESTDB_USE_SIGFLOAT

  _m->sig15_2_ro = __ext_sig__(( ((_d[3] & (0x7FU)) << 8) | (_d[2] & (0xFFU)) ), 15);
#ifdef TESTDB_USE_SIGFLOAT
  _m->sig15_2_phys = (sigfloat_t)(TESTDB_sig15_2_ro_fromS(_m->sig15_2_ro));
#endif // TESTDB_USE_SIGFLOAT

  _m->sig8_ro = __ext_sig__(( (_d[4] & (0xFFU)) ), 8);
#ifdef TESTDB_USE_SIGFLOAT
  _m->sig8_phys = TESTDB_sig8_ro_fromS(_m->sig8_ro);
#endif // TESTDB_USE_SIGFLOAT

  _m->sig_7_ro = __ext_sig__(( (_d[5] & (0x7FU)) ), 7);
#ifdef TESTDB_USE_SIGFLOAT
  _m->sig_7_phys = (sigfloat_t)(TESTDB_sig_7_ro_fromS(_m->sig_7_ro));
#endif // TESTDB_USE_SIGFLOAT

  _m->U7_TEST_1_ro = ((_d[6] >> 1) & (0x7FU));
#ifdef TESTDB_USE_SIGFLOAT
  _m->U7_TEST_1_phys = TESTDB_U7_TEST_1_ro_fromS(_m->U7_TEST_1_ro);
#endif // TESTDB_USE_SIGFLOAT

#ifdef TESTDB_USE_DIAG_MONITORS
  _m->mon1.dlc_error = (dlc_ < SIG_TEST_1_DLC);
  _m->mon1.last_cycle = GetSystemTick();
  _m->mon1.frame_cnt++;

  FMon_SIG_TEST_1_testdb(&_m->mon1, SIG_TEST_1_CANID);
#endif // TESTDB_USE_DIAG_MONITORS

  return SIG_TEST_1_CANID;
}

#ifdef TESTDB_USE_CANSTRUCT

uint32_t Pack_SIG_TEST_1_testdb(SIG_TEST_1_t* _m, __CoderDbcCanFrame_t__* cframe)
{
  uint8_t i; for (i = 0; (i < SIG_TEST_1_DLC) && (i < 8); cframe->Data[i++] = 0);

#ifdef TESTDB_USE_SIGFLOAT
  _m->sig15_ro = TESTDB_sig15_ro_toS(_m->sig15_phys);
  _m->sig15_2_ro = TESTDB_sig15_2_ro_toS(_m->sig15_2_phys);
  _m->sig8_ro = TESTDB_sig8_ro_toS(_m->sig8_phys);
  _m->sig_7_ro = TESTDB_sig_7_ro_toS(_m->sig_7_phys);
  _m->U7_TEST_1_ro = TESTDB_U7_TEST_1_ro_toS(_m->U7_TEST_1_phys);
#endif // TESTDB_USE_SIGFLOAT

  cframe->Data[0] |= (_m->sig15_ro & (0xFFU));
  cframe->Data[1] |= ((_m->sig15_ro >> 8) & (0x7FU));
  cframe->Data[2] |= (_m->sig15_2_ro & (0xFFU));
  cframe->Data[3] |= ((_m->sig15_2_ro >> 8) & (0x7FU));
  cframe->Data[4] |= (_m->sig8_ro & (0xFFU));
  cframe->Data[5] |= (_m->sig_7_ro & (0x7FU));
  cframe->Data[6] |= ((_m->U7_TEST_1_ro & (0x7FU)) << 1);

  cframe->MsgId = SIG_TEST_1_CANID;
  cframe->DLC = SIG_TEST_1_DLC;
  cframe->IDE = SIG_TEST_1_IDE;
  return SIG_TEST_1_CANID;
}

#else

uint32_t Pack_SIG_TEST_1_testdb(SIG_TEST_1_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide)
{
  uint8_t i; for (i = 0; (i < SIG_TEST_1_DLC) && (i < 8); _d[i++] = 0);

#ifdef TESTDB_USE_SIGFLOAT
  _m->sig15_ro = TESTDB_sig15_ro_toS(_m->sig15_phys);
  _m->sig15_2_ro = TESTDB_sig15_2_ro_toS(_m->sig15_2_phys);
  _m->sig8_ro = TESTDB_sig8_ro_toS(_m->sig8_phys);
  _m->sig_7_ro = TESTDB_sig_7_ro_toS(_m->sig_7_phys);
  _m->U7_TEST_1_ro = TESTDB_U7_TEST_1_ro_toS(_m->U7_TEST_1_phys);
#endif // TESTDB_USE_SIGFLOAT

  _d[0] |= (_m->sig15_ro & (0xFFU));
  _d[1] |= ((_m->sig15_ro >> 8) & (0x7FU));
  _d[2] |= (_m->sig15_2_ro & (0xFFU));
  _d[3] |= ((_m->sig15_2_ro >> 8) & (0x7FU));
  _d[4] |= (_m->sig8_ro & (0xFFU));
  _d[5] |= (_m->sig_7_ro & (0x7FU));
  _d[6] |= ((_m->U7_TEST_1_ro & (0x7FU)) << 1);

  *_len = SIG_TEST_1_DLC;
  *_ide = SIG_TEST_1_IDE;
  return SIG_TEST_1_CANID;
}

#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_EMPTY_EXT_ID_testdb(EMPTY_EXT_ID_t* _m, const uint8_t* _d, uint8_t dlc_)
{
  (void)dlc_;
  _m->ValTest = ((_d[0] >> 6) & (0x03U));
  _m->CS = (_d[1] & (0x3FU));

#ifdef TESTDB_USE_DIAG_MONITORS
  _m->mon1.dlc_error = (dlc_ < EMPTY_EXT_ID_DLC);
  _m->mon1.last_cycle = GetSystemTick();
  _m->mon1.frame_cnt++;

#ifdef TESTDB_AUTO_CSM
  _m->mon1.csm_error = (((uint8_t)GetFrameHash(_d, EMPTY_EXT_ID_DLC, EMPTY_EXT_ID_CANID, kXor8, 1)) != (_m->CS));
#endif // TESTDB_AUTO_CSM

  FMon_EMPTY_EXT_ID_testdb(&_m->mon1, EMPTY_EXT_ID_CANID);
#endif // TESTDB_USE_DIAG_MONITORS

  return EMPTY_EXT_ID_CANID;
}

#ifdef TESTDB_USE_CANSTRUCT

uint32_t Pack_EMPTY_EXT_ID_testdb(EMPTY_EXT_ID_t* _m, __CoderDbcCanFrame_t__* cframe)
{
  uint8_t i; for (i = 0; (i < EMPTY_EXT_ID_DLC) && (i < 8); cframe->Data[i++] = 0);

#ifdef TESTDB_AUTO_CSM
  _m->CS = 0U;
#endif // TESTDB_AUTO_CSM

  cframe->Data[0] |= ((_m->ValTest & (0x03U)) << 6);
  cframe->Data[1] |= (_m->CS & (0x3FU));

#ifdef TESTDB_AUTO_CSM
  _m->CS = ((uint8_t)GetFrameHash(cframe->Data, EMPTY_EXT_ID_DLC, EMPTY_EXT_ID_CANID, kXor8, 1));
  cframe->Data[1] |= (_m->CS & (0x3FU));
#endif // TESTDB_AUTO_CSM

  cframe->MsgId = EMPTY_EXT_ID_CANID;
  cframe->DLC = EMPTY_EXT_ID_DLC;
  cframe->IDE = EMPTY_EXT_ID_IDE;
  return EMPTY_EXT_ID_CANID;
}

#else

uint32_t Pack_EMPTY_EXT_ID_testdb(EMPTY_EXT_ID_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide)
{
  uint8_t i; for (i = 0; (i < EMPTY_EXT_ID_DLC) && (i < 8); _d[i++] = 0);

#ifdef TESTDB_AUTO_CSM
  _m->CS = 0U;
#endif // TESTDB_AUTO_CSM

  _d[0] |= ((_m->ValTest & (0x03U)) << 6);
  _d[1] |= (_m->CS & (0x3FU));

#ifdef TESTDB_AUTO_CSM
  _m->CS = ((uint8_t)GetFrameHash(_d, EMPTY_EXT_ID_DLC, EMPTY_EXT_ID_CANID, kXor8, 1));
  _d[1] |= (_m->CS & (0x3FU));
#endif // TESTDB_AUTO_CSM

  *_len = EMPTY_EXT_ID_DLC;
  *_ide = EMPTY_EXT_ID_IDE;
  return EMPTY_EXT_ID_CANID;
}

#endif // TESTDB_USE_CANSTRUCT

