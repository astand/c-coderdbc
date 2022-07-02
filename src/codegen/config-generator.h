#pragma once

#include <stdint.h>
#include "filewriter.h"
#include "c-sigprinter.h"
#include "fs-creator.h"

class ConfigGenerator {
 public:
  void FillHeader(FileWriter& wr, const GenDescriptor_t& gsett);
};
