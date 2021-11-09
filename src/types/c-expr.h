#pragma once

#include <stdlib.h>
#include "message.h"
#include <vector>
#include <string>

typedef struct
{

  MessageDescriptor_t msg;

  // this field contains all expressions for converting
  // data bytes to actual signals
  std::vector<std::string> to_signals;

  // this field contains all expressions for converting
  // frame fields to data bytes
  std::vector<std::string> to_bytes;

} CiExpr_t;