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

/* ----------------------------------------------------------------------------- */
// @d - buff for hash calculation
// @len - number of bytes for hash calculation
// @method - hash algorythm.
// @op - optional value
uint8_t GetFrameHash(const uint8_t* data_ptr, uint8_t len, uint32_t msgid, DbcCanCrcMethods type, uint32_t option);

/* ----------------------------------------------------------------------------- */
// this function will be called when unpacking is performing. Value will be saved
// in @last_cycle variable
uint32_t GetSystemTick(void);


#ifdef __cplusplus
}
#endif

