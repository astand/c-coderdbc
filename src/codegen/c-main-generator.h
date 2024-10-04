#pragma once

#include <stdint.h>
#include <memory>
#include "c-sigprinter.h"
#include "filewriter.h"
#include "../types/message.h"
#include "../types/outfile.h"
#include "fs-creator.h"

class CiMainGenerator {
 public:
  void Generate(DbcMessageList_t& dlist, const AppSettings_t& fsd, bool mux_enabled);

 private:

  void Gen_MainHeader();
  void Gen_MainSource();
  void Gen_ConfigHeader();
  void Gen_FMonHeader();
  void Gen_FMonSource();
  void Gen_CanMonUtil();
  void Gen_DbcCodeConf();

  void WriteSigStructField(const SignalDescriptor_t& sig, bool bitfield, size_t pad);

  void WriteUnpackBody(const CiExpr_t* sgs);
  void WritePackStructBody(const CiExpr_t* sgs);
  void WritePackArrayBody(const CiExpr_t* sgs);
  void PrintPackCommonText(const std::string& arrtxt, const CiExpr_t* sgs);

 private:
  CSigPrinter sigprt;
  FileWriter fwriter;
  // Actual max DLC value from dbc list instance
  size_t val_maxDlcValueFromDbcList;
  // Macro for default initial frame's data bytes value
  std::string prt_initialDataByteValueName;
  // Macro for frame DLC validation
  std::string prt_dlcValidateMacroName;
  const AppSettings_t* fdesc;
  // Bool to turn on or off code generation based on multiplexor master signal values is enabled for multiplexed signals.
  bool is_multiplex_enabled;
};
