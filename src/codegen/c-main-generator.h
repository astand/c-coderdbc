#pragma once

#include <stdint.h>
#include "c-sigprinter.h"
#include "filewriter.h"
#include "../types/message.h"
#include "../types/outfile.h"
#include "fs-creator.h"

class CiMainGenerator {
 public:
  CiMainGenerator();

  void Generate(std::vector<MessageDescriptor_t*>& msgs, const FsDescriptor_t& fsd);

 private:

  void WriteSigStructField(const SignalDescriptor_t& sig, bool bitfield, size_t pad);

 private:
  std::vector<std::string> tmpvect;

  CSigPrinter* sigprt;

  FileWriter* fwriter;
};
