#pragma once

/* include common dbccode configurations */
#include <dbccodeconf.h>


/* ------------------------------------------------------------------------- *
  This define enables using CAN message structs with bit-fielded signals
  layout.

  Note(!): bit-feild was not tested properly. */

/* #define TESTDB_USE_BITS_SIGNAL */


/* ------------------------------------------------------------------------- *
  This macro enables using CAN message descriptive struct packing functions
  (by default signature of pack function intakes a few simple typed params
  for loading data, len, etc). To compile you need to define the struct
  __CoderDbcCanFrame_t__ which must have fields:

    u32 MsgId (CAN Frame message ID)
    u8 DLC (CAN Frame payload length field)
    u8 Data[8] (CAN Frame payload data)
    u8 IDE (CAN Frame Extended (1) / Standard (0) ID type)

  This struct definition have to be placed (or be included) in dbccodeconf.h */

/* #define TESTDB_USE_CANSTRUCT */


/* ------------------------------------------------------------------------- *
  All the signals which have values of factor != 1 or offset != 0
  will be named in message struct with posfix '_ro'. Pack to payload
  operations will be made on this signal value as well as unpack from payload.

  USE_SIGFLOAT macro makes some difference:

  1. All the '_ro' fields will have a pair field with '_phys' postfix.
  If only offset != 0 is true then the type of '_phys' signal is the same
  as '_ro' signal. In other case the type will be @sigfloat_t which
  have to be defined in user dbccodeconf.h

  2. In pack function '_ro' signal will be rewritten by '_phys' signal, which
  requires from user to use ONLY '_phys' signal for packing frame

  3. In unpack function '_phys' signal will be written by '_ro' signal.
  User have to use '_phys' signal to read physical value. */

/* #define TESTDB_USE_SIGFLOAT */


/* ------------------------------------------------------------------------- *
  Note(!) that the "canmonitorutil.h" must be accessed in include path:

  This macro adds:

  - monitor field @mon1 to message struct

  - capture system tick in unpack function and save value to mon1 field
  to provide to user better missing frame detection code. For this case
  user must provide function declared in canmonitorutil.h - GetSysTick()
  which may return 1ms uptime.

  - calling function FMon_***  (from 'fmon' driver) inside unpack function
  which is empty by default and have to be filled by user if
  tests for DLC, rolling, checksum are necessary */

/* #define TESTDB_USE_DIAG_MONITORS */


/* ------------------------------------------------------------------------- *
  When monitor using is enabled (TESTDB_USE_DIAG_MONITORS) and define below
  uncommented, additional signal will be added to message struct. ***_expt:
  expected rolling counter, to perform monitoring rolling counter sequence
  automatically (result may be tested in dedicated Fmon_*** function) */

/* #define TESTDB_AUTO_ROLL */


/* ------------------------------------------------------------------------- *
  When monitor using is enabled (TESTDB_USE_DIAG_MONITORS) and define below
  uncommented, frame checksum signal may be handled automatically.

  The signal which may be marked as checksum signal must have substring
  with next format:
    <Checksum:XOR8:3>

  where:

  - "Checksum": constant marker word

  - "XOR8": type of method, this text will be passed to GetFrameHash
  (canmonitorutil.h) function as is, the best use case is to define 'enum
  DbcCanCrcMethods' in canmonitorutil.h file with all possible
  checksum algorithms (e.g. XOR8, XOR4 etc)

  - "3": optional value that will be passed to GetFrameHash as integer value

  Function GetFrameHash have to be implemented by user

  In pack function checksum signal will be calculated automatically
  and loaded to payload

  In unpack function checksum signal is checked with calculated.
  (result may be tested in dedicated Fmon_*** function). */

/* #define TESTDB_AUTO_CSM */


/* ------------------------------------------------------------------------- *
  FMon handling model can be build in two ways: 

  1 - Default. In this case when specific frame unpack is called the 
  specific FMon_{Frame name}_{driver name} functoin will be called.
  User's code scope has to define each of these functions. Each function is
  responsible for the error handling of one frame

  2 - MONO. In this case there is only one function to perform any frame 
  monitoring. This function has to be implemented in the user's code scope.
  This function is named as FMon_MONO_{driver name}. It takes frame id
  which can be used for selection of the logic for a frame monitoring.
  This mode costs a bit more in runtime but when you often edit you DBC and you 
  have more than one project it could be more maintanable (there is
  no necessity to replace source code)

  For using MONO way uncomment line below */
/* #define TESTDB_USE_MONO_FMON */
