#pragma once

#include <stdint.h>
#include "filewriter.h"
#include "c-sigprinter.h"
#include "fs-creator.h"

class MonGenerator {
 public:

  MonGenerator() = default;

  uint32_t FillHeader(FileWriter& wr, std::vector<CiExpr_t*>& sigs, const GenDescriptor_t& gsett);
  uint32_t FillSource(FileWriter& wr, std::vector<CiExpr_t*>& sigs, const GenDescriptor_t& gsett);
};
