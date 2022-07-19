#include <testdb-fmon.h>

#ifdef TESTDB_USE_DIAG_MONITORS

/*
Put the monitor function content here, keep in mind -
next generation will completely clear all manually added code (!)
*/

#ifdef TESTDB_USE_MONO_FMON

void _FMon_MONO_testdb(FrameMonitor_t* _mon, uint32_t msgid)
{
  (void)_mon;
  (void)msgid;
}

#else

void _FMon_UTEST_2_testdb(FrameMonitor_t* _mon, uint32_t msgid)
{
  (void)_mon;
  (void)msgid;
}

void _FMon_EMPTY_0_testdb(FrameMonitor_t* _mon, uint32_t msgid)
{
  (void)_mon;
  (void)msgid;
}

void _FMon_UTEST_3_testdb(FrameMonitor_t* _mon, uint32_t msgid)
{
  (void)_mon;
  (void)msgid;
}

void _FMon_FLT_TEST_1_testdb(FrameMonitor_t* _mon, uint32_t msgid)
{
  (void)_mon;
  (void)msgid;
}

void _FMon_SIG_TEST_1_testdb(FrameMonitor_t* _mon, uint32_t msgid)
{
  (void)_mon;
  (void)msgid;
}

void _FMon_EMPTY_EXT_ID_testdb(FrameMonitor_t* _mon, uint32_t msgid)
{
  (void)_mon;
  (void)msgid;
}

#endif // TESTDB_USE_MONO_FMON

#endif // TESTDB_USE_DIAG_MONITORS
