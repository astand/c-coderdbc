#pragma once

#include <stdint.h>
#include "c-sigprinter.h"
#include "filewriter.h"
#include "../types/message.h"
#include "../types/outfile.h"

class CiMainGenerator {
 public:
  CiMainGenerator();

  void Generate(std::vector<MessageDescriptor_t*>& msgs, std::string drvname, std::string dirpath);

 private:
  bool SetFinalPath(std::string dirpath);

  void SetCommonValues(const std::string& drvname);

  void WriteSigStructField(const SignalDescriptor_t& sig, bool bitfield, size_t pad);

 private:
  std::string work_dir_path;

  std::string workstr;

  std::string DRVNAME;

  std::string usebits_str;
  std::string usediag_str;
  std::string canframe_str;

  std::vector<std::string> tmpvect;

  CSigPrinter* sigprt;

  FileWriter* fwriter;

  OutFileDescriptor_t mhead;
  OutFileDescriptor_t mcode;
  OutFileDescriptor_t fhead;
  OutFileDescriptor_t fcode;
};
