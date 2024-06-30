// Generator version : v3.1
#pragma once

#include <stdint.h>

// when USE_SIGFLOAT enabed the sigfloat_t must be defined
// typedef double sigfloat_t;

// when USE_CANSTRUCT enabled __CoderDbcCanFrame_t__ must be defined
// #include "{header_with_can_struct}"
// typedef {can_struct} __CoderDbcCanFrame_t__;

// if you need to allocate rx and tx messages structs put the allocation macro here
// #define __DEF_{your_driver_name}__

// defualt @__ext_sig__ help types definition

typedef uint32_t ubitext_t;
typedef int32_t bitext_t;

// To provide a way to make missing control correctly you
// have to define macro @GetSystemTick() which has to
// return kind of tick counter (e.g. 1 ms ticker)

// #define GetSystemTick() __get__tick__()

// To provide a way to calculate hash (crc) for CAN
// frame's data field you have to define macro @GetFrameHash

// #define GetFrameHash(a,b,c,d,e) __get_hash__(a,b,c,d,e)

