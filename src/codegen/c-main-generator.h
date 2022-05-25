#pragma once

#include <stdint.h>
#include "c-sigprinter.h"
#include "filewriter.h"
#include "../types/message.h"
#include "../types/outfile.h"
#include "fs-creator.h"

typedef struct {
  bool gen_rx_cbs; // generate c code with RX callback stubs
  bool gen_tx_cb; // generate c code TX callback stub
  bool can_structs; // enables CANMESSAGES_USE_CANSTRUCT macro during generation
  bool can_defined; // generate CAN structure definition
  bool struct_path_provided; // external CAN structure is provided
  std::string can_struct_path; // path to header with __CoderDbcCanFrame_t__ conversion
  std::string can_struct_def; // string defining the CAN struct
} generator_params_t;

class CiMainGenerator {
 public:
  CiMainGenerator();
  void Generate(DbcMessageList_t& dlist, const FsDescriptor_t& fsd, generator_params_t p);

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
  std::vector<std::string> tmpvect;

  CSigPrinter* sigprt;

  FileWriter* fwriter;

  const FsDescriptor_t* fdesc;

  const DbcMessageList_t* p_dlist;
};
