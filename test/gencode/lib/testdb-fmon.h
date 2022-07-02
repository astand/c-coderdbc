#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// DBC file version
#define VER_TESTDB_MAJ_FMON (1U)
#define VER_TESTDB_MIN_FMON (10U)

#include <testdb-config.h>

#ifdef TESTDB_USE_DIAG_MONITORS

#include <canmonitorutil.h>
/*
This file contains the prototypes of all the functions that will be called
from each Unpack_*name* function to detect DBC related errors
It is the user responsibility to defined these functions in the
separated .c file. If it won't be done the linkage error will happen
*/

#ifdef TESTDB_USE_MONO_FMON

void _FMon_MONO_testdb(FrameMonitor_t* _mon, uint32_t msgid);

#define FMon_UTEST_2_testdb(x, y) _FMon_MONO_testdb((x), (y))
#define FMon_EMPTY_0_testdb(x, y) _FMon_MONO_testdb((x), (y))
#define FMon_UTEST_3_testdb(x, y) _FMon_MONO_testdb((x), (y))
#define FMon_FLT_TEST_1_testdb(x, y) _FMon_MONO_testdb((x), (y))
#define FMon_SIG_TEST_1_testdb(x, y) _FMon_MONO_testdb((x), (y))
#define FMon_EMPTY_EXT_ID_testdb(x, y) _FMon_MONO_testdb((x), (y))

#else

void _FMon_UTEST_2_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void _FMon_EMPTY_0_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void _FMon_UTEST_3_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void _FMon_FLT_TEST_1_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void _FMon_SIG_TEST_1_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void _FMon_EMPTY_EXT_ID_testdb(FrameMonitor_t* _mon, uint32_t msgid);

#define FMon_UTEST_2_testdb(x, y) _FMon_UTEST_2_testdb((x), (y))
#define FMon_EMPTY_0_testdb(x, y) _FMon_EMPTY_0_testdb((x), (y))
#define FMon_UTEST_3_testdb(x, y) _FMon_UTEST_3_testdb((x), (y))
#define FMon_FLT_TEST_1_testdb(x, y) _FMon_FLT_TEST_1_testdb((x), (y))
#define FMon_SIG_TEST_1_testdb(x, y) _FMon_SIG_TEST_1_testdb((x), (y))
#define FMon_EMPTY_EXT_ID_testdb(x, y) _FMon_EMPTY_EXT_ID_testdb((x), (y))

#endif

#endif // TESTDB_USE_DIAG_MONITORS

#ifdef __cplusplus
}
#endif
