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

  // another field containing expressions for converting
  // frame fields to data bytes, but includes a different expression
  // for each potential multiplexor value.
  // i.e if the master multiplexor signal is 0, the first expression
  // should be used, if it is 1, the second expression is used, etc.
  // First dimension is the byte index, second dimension is the multiplexor value.
  std::vector<std::vector<std::string>> to_bytes_mux;
  // Store the corresponding multiplexor values for each expression
  // since multiplexor values are not necessarily contiguous, we need
  // to store the values explicitly. This vector should have the same
  // size as the second dim of the to_bytes_mux and each element corresponds to the multiplexor
  // value for the corresponding expression in to_bytes_mux.
  // i.e to_bytes_mux[0 to 7].size() == mux_values.size()
  std::vector<int> mux_values; 

} CiExpr_t;