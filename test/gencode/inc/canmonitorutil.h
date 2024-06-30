// Generator version : v3.1
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// declare here all availible checksum algorithms
typedef enum
{
  // XOR8 = 0,
  // XOR4 = 1,
  // etc

  // it is up to user to have or to skip final enum value - @CRC_ALG_COUNT
  CRC_ALG_COUNT
} DbcCanCrcMethods;

typedef struct
{
  // @last_cycle keeps tick-value when last frame was received
  uint32_t last_cycle;

  // @timeout_cycle keeps maximum timeout for frame, user responsibility
  // to init this field and use it in missing frame monitoring function
  uint32_t timeout_cycle;

  // @frame_cnt keeps count of all the received frames
  uint32_t frame_cnt;

  // setting up @roll_error bit indicates roll counting fail.
  // Bit is not clearing automatically!
  uint32_t roll_error : 1;

  // setting up @checksum_error bit indicates checksum checking failure.
  // Bit is not clearing automatically!
  uint32_t csm_error : 1;

  // setting up @cycle_error bit indicates that time was overrunned.
  // Bit is not clearing automatically!
  uint32_t cycle_error : 1;

  // setting up @dlc_error bit indicates that the actual length of
  // CAN frame is less then defined by CAN matrix!
  uint32_t dlc_error : 1;

} FrameMonitor_t;

#ifdef __cplusplus
}
#endif

