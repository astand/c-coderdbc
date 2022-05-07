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

void FMon_UTEST_2_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void FMon_EMPTY_0_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void FMon_UTEST_3_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void FMon_FLT_TEST_1_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void FMon_SIG_TEST_1_testdb(FrameMonitor_t* _mon, uint32_t msgid);
void FMon_EMPTY_EXT_ID_testdb(FrameMonitor_t* _mon, uint32_t msgid);

#endif // TESTDB_USE_DIAG_MONITORS

#ifdef __cplusplus
}
#endif
