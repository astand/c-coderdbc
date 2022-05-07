#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// DBC file version
#define VER_TESTDB_MAJ (1U)
#define VER_TESTDB_MIN (10U)

// include current dbc-driver compilation config
#include <testdb-config.h>

#ifdef TESTDB_USE_DIAG_MONITORS
// This file must define:
// base monitor struct
// function signature for HASH calculation: (@GetFrameHash)
// function signature for getting system tick value: (@GetSystemTick)
#include <canmonitorutil.h>

#endif // TESTDB_USE_DIAG_MONITORS


// def @UTEST_2 CAN Message (333  0x14d)
#define UTEST_2_IDE (0U)
#define UTEST_2_DLC (8U)
#define UTEST_2_CANID (0x14d)

// Value tables for @ValTest signal

#ifndef ValTest_UTEST_2_Unsupported
#define ValTest_UTEST_2_Unsupported (3)
#endif

#ifndef ValTest_UTEST_2_Fail
#define ValTest_UTEST_2_Fail (2)
#endif

#ifndef ValTest_UTEST_2_OK
#define ValTest_UTEST_2_OK (1)
#endif

#ifndef ValTest_UTEST_2_Undefined
#define ValTest_UTEST_2_Undefined (0)
#endif

// signal: @U7_TEST_1_ro
#define TESTDB_U7_TEST_1_ro_CovFactor (1)
#define TESTDB_U7_TEST_1_ro_toS(x) ( (uint8_t) ((x) - (-255)) )
#define TESTDB_U7_TEST_1_ro_fromS(x) ( ((x) + (-255)) )

typedef struct
{
#ifdef TESTDB_USE_BITS_SIGNAL

  uint32_t U28_TEST_1;                       //      Bits=28

  // This is test signal for Value Table
  //  3 : "Unsupported"
  //  2 : "Fail"
  //  1 : "OK"
  //  0 : "Undefined"
  uint8_t ValTest : 2;                       //      Bits= 2 Unit:'c'

  uint8_t U8_TEST_1;                         //      Bits= 8

  uint8_t U7_TEST_1_ro : 7;                  //      Bits= 7 Offset= -255

#ifdef TESTDB_USE_SIGFLOAT
  int16_t U7_TEST_1_phys;
#endif // TESTDB_USE_SIGFLOAT

#else

  uint32_t U28_TEST_1;                       //      Bits=28

  // This is test signal for Value Table
  //  3 : "Unsupported"
  //  2 : "Fail"
  //  1 : "OK"
  //  0 : "Undefined"
  uint8_t ValTest;                           //      Bits= 2 Unit:'c'

  uint8_t U8_TEST_1;                         //      Bits= 8

  uint8_t U7_TEST_1_ro;                      //      Bits= 7 Offset= -255

#ifdef TESTDB_USE_SIGFLOAT
  int16_t U7_TEST_1_phys;
#endif // TESTDB_USE_SIGFLOAT

#endif // TESTDB_USE_BITS_SIGNAL

#ifdef TESTDB_USE_DIAG_MONITORS

  FrameMonitor_t mon1;

#endif // TESTDB_USE_DIAG_MONITORS

} UTEST_2_t;

// def @EMPTY_0 CAN Message (352  0x160)
#define EMPTY_0_IDE (0U)
#define EMPTY_0_DLC (8U)
#define EMPTY_0_CANID (0x160)
#define EMPTY_0_CYC (101U)

typedef struct
{
#ifdef TESTDB_USE_BITS_SIGNAL

  // <Checksum:kXor8:1> test pattern
  uint8_t CS : 6;                            //      Bits= 6

  // <RollingCounter>
  uint8_t RC : 4;                            //      Bits= 4

#ifdef TESTDB_AUTO_ROLL

  uint8_t RC_expt : 4;                       //      Bits= 4

#endif // TESTDB_AUTO_ROLL

#else

  // <Checksum:kXor8:1> test pattern
  uint8_t CS;                                //      Bits= 6

  // <RollingCounter>
  uint8_t RC;                                //      Bits= 4

#ifdef TESTDB_AUTO_ROLL

  uint8_t RC_expt;                           //      Bits= 4

#endif // TESTDB_AUTO_ROLL

#endif // TESTDB_USE_BITS_SIGNAL

#ifdef TESTDB_USE_DIAG_MONITORS

  FrameMonitor_t mon1;

#endif // TESTDB_USE_DIAG_MONITORS

} EMPTY_0_t;

// def @UTEST_3 CAN Message (555  0x22b)
#define UTEST_3_IDE (0U)
#define UTEST_3_DLC (8U)
#define UTEST_3_CANID (0x22b)

// Value tables for @TestValTableID signal

#ifndef TestValTableID_UTEST_3_Description_for_the_value_0x7
#define TestValTableID_UTEST_3_Description_for_the_value_0x7 (-2)
#endif

#ifndef TestValTableID_UTEST_3_Udef
#define TestValTableID_UTEST_3_Udef (-1)
#endif

#ifndef TestValTableID_UTEST_3_Udef
#define TestValTableID_UTEST_3_Udef (6)
#endif

#ifndef TestValTableID_UTEST_3_Udef
#define TestValTableID_UTEST_3_Udef (5)
#endif

#ifndef TestValTableID_UTEST_3_Udef
#define TestValTableID_UTEST_3_Udef (4)
#endif

#ifndef TestValTableID_UTEST_3_Error
#define TestValTableID_UTEST_3_Error (3)
#endif

#ifndef TestValTableID_UTEST_3_Ok
#define TestValTableID_UTEST_3_Ok (2)
#endif

#ifndef TestValTableID_UTEST_3_State_one
#define TestValTableID_UTEST_3_State_one (1)
#endif

#ifndef TestValTableID_UTEST_3_State_1
#define TestValTableID_UTEST_3_State_1 (0)
#endif


typedef struct
{
#ifdef TESTDB_USE_BITS_SIGNAL

  uint32_t U32_TEST_1;                       //      Bits=32

  //  -2 : "Description for the value '0x7'"
  //  -1 : "Udef"
  //  6 : "Udef"
  //  5 : "Udef"
  //  4 : "Udef"
  //  3 : "Error"
  //  2 : "Ok"
  //  1 : "State one"
  //  0 : "State 1"
  uint8_t TestValTableID : 3;                //      Bits= 3

#else

  uint32_t U32_TEST_1;                       //      Bits=32

  //  -2 : "Description for the value '0x7'"
  //  -1 : "Udef"
  //  6 : "Udef"
  //  5 : "Udef"
  //  4 : "Udef"
  //  3 : "Error"
  //  2 : "Ok"
  //  1 : "State one"
  //  0 : "State 1"
  uint8_t TestValTableID;                    //      Bits= 3

#endif // TESTDB_USE_BITS_SIGNAL

#ifdef TESTDB_USE_DIAG_MONITORS

  FrameMonitor_t mon1;

#endif // TESTDB_USE_DIAG_MONITORS

} UTEST_3_t;

// def @FLT_TEST_1 CAN Message (864  0x360)
#define FLT_TEST_1_IDE (0U)
#define FLT_TEST_1_DLC (8U)
#define FLT_TEST_1_CANID (0x360)
#define FLT_TEST_1_CYC (101U)

// Value tables for @ValTable signal

#ifndef ValTable_FLT_TEST_1_Unsupported
#define ValTable_FLT_TEST_1_Unsupported (3)
#endif

#ifndef ValTable_FLT_TEST_1_Fail
#define ValTable_FLT_TEST_1_Fail (2)
#endif

#ifndef ValTable_FLT_TEST_1_OK
#define ValTable_FLT_TEST_1_OK (1)
#endif

#ifndef ValTable_FLT_TEST_1_Undefined
#define ValTable_FLT_TEST_1_Undefined (0)
#endif

// signal: @INT_TEST_2_ro
#define TESTDB_INT_TEST_2_ro_CovFactor (5)
#define TESTDB_INT_TEST_2_ro_toS(x) ( (int8_t) ((x) / (5)) )
#define TESTDB_INT_TEST_2_ro_fromS(x) ( ((x) * (5)) )
// signal: @Accel_ro
#define TESTDB_Accel_ro_CovFactor (0.100000)
#define TESTDB_Accel_ro_toS(x) ( (uint16_t) (((x) - (-100.000000)) / (0.100000)) )
#define TESTDB_Accel_ro_fromS(x) ( (((x) * (0.100000)) + (-100.000000)) )
// signal: @FLT4_TEST_1_ro
#define TESTDB_FLT4_TEST_1_ro_CovFactor (2.010000)
#define TESTDB_FLT4_TEST_1_ro_toS(x) ( (uint8_t) (((x) - (-0.010000)) / (2.010000)) )
#define TESTDB_FLT4_TEST_1_ro_fromS(x) ( (((x) * (2.010000)) + (-0.010000)) )
// signal: @FLT4_TEST_2_ro
#define TESTDB_FLT4_TEST_2_ro_CovFactor (2.010000)
#define TESTDB_FLT4_TEST_2_ro_toS(x) ( (uint8_t) (((x) - (-5.000000)) / (2.010000)) )
#define TESTDB_FLT4_TEST_2_ro_fromS(x) ( (((x) * (2.010000)) + (-5.000000)) )
// signal: @FLT4_TEST_3_ro
#define TESTDB_FLT4_TEST_3_ro_CovFactor (2.000000)
#define TESTDB_FLT4_TEST_3_ro_toS(x) ( (uint8_t) (((x) - (-10.100000)) / (2.000000)) )
#define TESTDB_FLT4_TEST_3_ro_fromS(x) ( (((x) * (2.000000)) + (-10.100000)) )
// signal: @INT_TEST_1_ro
#define TESTDB_INT_TEST_1_ro_CovFactor (9)
#define TESTDB_INT_TEST_1_ro_toS(x) ( (uint8_t) (((x) - (-11)) / (9)) )
#define TESTDB_INT_TEST_1_ro_fromS(x) ( (((x) * (9)) + (-11)) )

typedef struct
{
#ifdef TESTDB_USE_BITS_SIGNAL

  // This is just comment.
  // Next line.
  //  3 : "Unsupported"
  //  2 : "Fail"
  //  1 : "OK"
  //  0 : "Undefined"
  uint8_t ValTable : 2;                      //      Bits= 2 Unit:'e'

  uint8_t Position : 4;                      //      Bits= 4

  int8_t INT_TEST_2_ro : 7;                  //  [-] Bits= 7 Factor= 5

#ifdef TESTDB_USE_SIGFLOAT
  int16_t INT_TEST_2_phys;
#endif // TESTDB_USE_SIGFLOAT

  // <RollingCounter>
  uint8_t RC : 4;                            //      Bits= 4

  // <Checksum:kXor4:1>
  uint8_t CS : 4;                            //      Bits= 4

  uint16_t Accel_ro;                         //      Bits=12 Offset= -100.000000        Factor= 0.100000        Unit:'m/s'

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t Accel_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t FLT4_TEST_1_ro : 4;                //      Bits= 4 Offset= -0.010000          Factor= 2.010000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t FLT4_TEST_1_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t FLT4_TEST_2_ro : 4;                //      Bits= 4 Offset= -5.000000          Factor= 2.010000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t FLT4_TEST_2_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t FLT4_TEST_3_ro : 4;                //      Bits= 4 Offset= -10.100000         Factor= 2.000000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t FLT4_TEST_3_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t INT_TEST_1_ro : 4;                 //      Bits= 4 Offset= -11                Factor= 9

#ifdef TESTDB_USE_SIGFLOAT
  int8_t INT_TEST_1_phys;
#endif // TESTDB_USE_SIGFLOAT

#ifdef TESTDB_AUTO_ROLL

  uint8_t RC_expt : 4;                       //      Bits= 4

#endif // TESTDB_AUTO_ROLL

#else

  // This is just comment.
  // Next line.
  //  3 : "Unsupported"
  //  2 : "Fail"
  //  1 : "OK"
  //  0 : "Undefined"
  uint8_t ValTable;                          //      Bits= 2 Unit:'e'

  uint8_t Position;                          //      Bits= 4

  int8_t INT_TEST_2_ro;                      //  [-] Bits= 7 Factor= 5

#ifdef TESTDB_USE_SIGFLOAT
  int16_t INT_TEST_2_phys;
#endif // TESTDB_USE_SIGFLOAT

  // <RollingCounter>
  uint8_t RC;                                //      Bits= 4

  // <Checksum:kXor4:1>
  uint8_t CS;                                //      Bits= 4

  uint16_t Accel_ro;                         //      Bits=12 Offset= -100.000000        Factor= 0.100000        Unit:'m/s'

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t Accel_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t FLT4_TEST_1_ro;                    //      Bits= 4 Offset= -0.010000          Factor= 2.010000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t FLT4_TEST_1_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t FLT4_TEST_2_ro;                    //      Bits= 4 Offset= -5.000000          Factor= 2.010000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t FLT4_TEST_2_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t FLT4_TEST_3_ro;                    //      Bits= 4 Offset= -10.100000         Factor= 2.000000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t FLT4_TEST_3_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t INT_TEST_1_ro;                     //      Bits= 4 Offset= -11                Factor= 9

#ifdef TESTDB_USE_SIGFLOAT
  int8_t INT_TEST_1_phys;
#endif // TESTDB_USE_SIGFLOAT

#ifdef TESTDB_AUTO_ROLL

  uint8_t RC_expt;                           //      Bits= 4

#endif // TESTDB_AUTO_ROLL

#endif // TESTDB_USE_BITS_SIGNAL

#ifdef TESTDB_USE_DIAG_MONITORS

  FrameMonitor_t mon1;

#endif // TESTDB_USE_DIAG_MONITORS

} FLT_TEST_1_t;

// def @SIG_TEST_1 CAN Message (1911 0x777)
#define SIG_TEST_1_IDE (0U)
#define SIG_TEST_1_DLC (8U)
#define SIG_TEST_1_CANID (0x777)
// signal: @sig15_ro
#define TESTDB_sig15_ro_CovFactor (3)
#define TESTDB_sig15_ro_toS(x) ( (int16_t) (((x) - (-1024)) / (3)) )
#define TESTDB_sig15_ro_fromS(x) ( (((x) * (3)) + (-1024)) )
// signal: @sig15_2_ro
#define TESTDB_sig15_2_ro_CovFactor (1.900000)
#define TESTDB_sig15_2_ro_toS(x) ( (int16_t) (((x) - (-2500.000000)) / (1.900000)) )
#define TESTDB_sig15_2_ro_fromS(x) ( (((x) * (1.900000)) + (-2500.000000)) )
// signal: @sig8_ro
#define TESTDB_sig8_ro_CovFactor (5)
#define TESTDB_sig8_ro_toS(x) ( (int8_t) ((x) / (5)) )
#define TESTDB_sig8_ro_fromS(x) ( ((x) * (5)) )
// signal: @sig_7_ro
#define TESTDB_sig_7_ro_CovFactor (1.200000)
#define TESTDB_sig_7_ro_toS(x) ( (int8_t) (((x) - (0.000000)) / (1.200000)) )
#define TESTDB_sig_7_ro_fromS(x) ( (((x) * (1.200000)) + (0.000000)) )
// signal: @U7_TEST_1_ro
#define TESTDB_U7_TEST_1_ro_CovFactor (1)
#define TESTDB_U7_TEST_1_ro_toS(x) ( (uint8_t) ((x) - (-255)) )
#define TESTDB_U7_TEST_1_ro_fromS(x) ( ((x) + (-255)) )

typedef struct
{
#ifdef TESTDB_USE_BITS_SIGNAL

  int16_t sig15_ro;                          //  [-] Bits=15 Offset= -1024              Factor= 3

#ifdef TESTDB_USE_SIGFLOAT
  int32_t sig15_phys;
#endif // TESTDB_USE_SIGFLOAT

  int16_t sig15_2_ro;                        //  [-] Bits=15 Offset= -2500.000000       Factor= 1.900000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t sig15_2_phys;
#endif // TESTDB_USE_SIGFLOAT

  int8_t sig8_ro;                            //  [-] Bits= 8 Factor= 5

#ifdef TESTDB_USE_SIGFLOAT
  int16_t sig8_phys;
#endif // TESTDB_USE_SIGFLOAT

  int8_t sig_7_ro : 7;                       //  [-] Bits= 7 Factor= 1.200000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t sig_7_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t U7_TEST_1_ro : 7;                  //      Bits= 7 Offset= -255

#ifdef TESTDB_USE_SIGFLOAT
  int16_t U7_TEST_1_phys;
#endif // TESTDB_USE_SIGFLOAT

#else

  int16_t sig15_ro;                          //  [-] Bits=15 Offset= -1024              Factor= 3

#ifdef TESTDB_USE_SIGFLOAT
  int32_t sig15_phys;
#endif // TESTDB_USE_SIGFLOAT

  int16_t sig15_2_ro;                        //  [-] Bits=15 Offset= -2500.000000       Factor= 1.900000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t sig15_2_phys;
#endif // TESTDB_USE_SIGFLOAT

  int8_t sig8_ro;                            //  [-] Bits= 8 Factor= 5

#ifdef TESTDB_USE_SIGFLOAT
  int16_t sig8_phys;
#endif // TESTDB_USE_SIGFLOAT

  int8_t sig_7_ro;                           //  [-] Bits= 7 Factor= 1.200000

#ifdef TESTDB_USE_SIGFLOAT
  sigfloat_t sig_7_phys;
#endif // TESTDB_USE_SIGFLOAT

  uint8_t U7_TEST_1_ro;                      //      Bits= 7 Offset= -255

#ifdef TESTDB_USE_SIGFLOAT
  int16_t U7_TEST_1_phys;
#endif // TESTDB_USE_SIGFLOAT

#endif // TESTDB_USE_BITS_SIGNAL

#ifdef TESTDB_USE_DIAG_MONITORS

  FrameMonitor_t mon1;

#endif // TESTDB_USE_DIAG_MONITORS

} SIG_TEST_1_t;

// def @EMPTY_EXT_ID CAN Message (536870902 0x1ffffff6)
#define EMPTY_EXT_ID_IDE (1U)
#define EMPTY_EXT_ID_DLC (8U)
#define EMPTY_EXT_ID_CANID (0x1ffffff6)

// Value tables for @ValTest signal

#ifndef ValTest_EMPTY_EXT_ID_Unsupported
#define ValTest_EMPTY_EXT_ID_Unsupported (3)
#endif

#ifndef ValTest_EMPTY_EXT_ID_Fail
#define ValTest_EMPTY_EXT_ID_Fail (2)
#endif

#ifndef ValTest_EMPTY_EXT_ID_OK
#define ValTest_EMPTY_EXT_ID_OK (1)
#endif

#ifndef ValTest_EMPTY_EXT_ID_Undefined
#define ValTest_EMPTY_EXT_ID_Undefined (0)
#endif


typedef struct
{
#ifdef TESTDB_USE_BITS_SIGNAL

  // This is test signal for Value Table
  //  3 : "Unsupported"
  //  2 : "Fail"
  //  1 : "OK"
  //  0 : "Undefined"
  uint8_t ValTest : 2;                       //      Bits= 2 Unit:'c'

  // <Checksum:kXor8:1> test pattern
  uint8_t CS : 6;                            //      Bits= 6

#else

  // This is test signal for Value Table
  //  3 : "Unsupported"
  //  2 : "Fail"
  //  1 : "OK"
  //  0 : "Undefined"
  uint8_t ValTest;                           //      Bits= 2 Unit:'c'

  // <Checksum:kXor8:1> test pattern
  uint8_t CS;                                //      Bits= 6

#endif // TESTDB_USE_BITS_SIGNAL

#ifdef TESTDB_USE_DIAG_MONITORS

  FrameMonitor_t mon1;

#endif // TESTDB_USE_DIAG_MONITORS

} EMPTY_EXT_ID_t;

// Function signatures

uint32_t Unpack_UTEST_2_testdb(UTEST_2_t* _m, const uint8_t* _d, uint8_t dlc_);
#ifdef TESTDB_USE_CANSTRUCT
uint32_t Pack_UTEST_2_testdb(UTEST_2_t* _m, __CoderDbcCanFrame_t__* cframe);
#else
uint32_t Pack_UTEST_2_testdb(UTEST_2_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);
#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_EMPTY_0_testdb(EMPTY_0_t* _m, const uint8_t* _d, uint8_t dlc_);
#ifdef TESTDB_USE_CANSTRUCT
uint32_t Pack_EMPTY_0_testdb(EMPTY_0_t* _m, __CoderDbcCanFrame_t__* cframe);
#else
uint32_t Pack_EMPTY_0_testdb(EMPTY_0_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);
#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_UTEST_3_testdb(UTEST_3_t* _m, const uint8_t* _d, uint8_t dlc_);
#ifdef TESTDB_USE_CANSTRUCT
uint32_t Pack_UTEST_3_testdb(UTEST_3_t* _m, __CoderDbcCanFrame_t__* cframe);
#else
uint32_t Pack_UTEST_3_testdb(UTEST_3_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);
#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_FLT_TEST_1_testdb(FLT_TEST_1_t* _m, const uint8_t* _d, uint8_t dlc_);
#ifdef TESTDB_USE_CANSTRUCT
uint32_t Pack_FLT_TEST_1_testdb(FLT_TEST_1_t* _m, __CoderDbcCanFrame_t__* cframe);
#else
uint32_t Pack_FLT_TEST_1_testdb(FLT_TEST_1_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);
#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_SIG_TEST_1_testdb(SIG_TEST_1_t* _m, const uint8_t* _d, uint8_t dlc_);
#ifdef TESTDB_USE_CANSTRUCT
uint32_t Pack_SIG_TEST_1_testdb(SIG_TEST_1_t* _m, __CoderDbcCanFrame_t__* cframe);
#else
uint32_t Pack_SIG_TEST_1_testdb(SIG_TEST_1_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);
#endif // TESTDB_USE_CANSTRUCT

uint32_t Unpack_EMPTY_EXT_ID_testdb(EMPTY_EXT_ID_t* _m, const uint8_t* _d, uint8_t dlc_);
#ifdef TESTDB_USE_CANSTRUCT
uint32_t Pack_EMPTY_EXT_ID_testdb(EMPTY_EXT_ID_t* _m, __CoderDbcCanFrame_t__* cframe);
#else
uint32_t Pack_EMPTY_EXT_ID_testdb(EMPTY_EXT_ID_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);
#endif // TESTDB_USE_CANSTRUCT

#ifdef __cplusplus
}
#endif
