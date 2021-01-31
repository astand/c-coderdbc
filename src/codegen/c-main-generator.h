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

  void Gen_MainHeader();
  void Gen_MainSource();
  void Gen_FMonHeader();
  void Gen_FMonSource();

  void WriteSigStructField(const SignalDescriptor_t& sig, bool bitfield, size_t pad);

  void WriteUnpackBody(const CiExpr_t* sgs);
  void WritePackStructBody(const CiExpr_t* sgs);
  void WritePackArrayBody(const CiExpr_t* sgs);

 private:
  std::vector<std::string> tmpvect;

  CSigPrinter* sigprt;

  FileWriter* fwriter;

  const FsDescriptor_t* fdesc;
};
