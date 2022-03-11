#pragma once

#include <stdint.h>

// when USE_SIGFLOAT enabed the sigfloat_t must be defined
// typedef double sigfloat_t;

// when USE_CANSTRUCT enabled __CoderDbcCanFrame_t__ must be defined
// #include "{header_with_can_struct}"
// typedef {can_struct} __CoderDbcCanFrame_t__;

// if you need to allocate rx and tx messages structs put the allocation macro here
// #define __DEF_{your_driver_name}__

